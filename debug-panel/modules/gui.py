import sys
import time
import numpy as np

from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout,
                           QWidget, QPushButton, QLabel, QLineEdit, QSpinBox,
                           QDoubleSpinBox, QCheckBox, QComboBox, QGroupBox,
                           QTabWidget, QTextEdit, QMessageBox, QSplitter,
                           QGridLayout, QScrollArea, QColorDialog)
from PyQt5.QtCore import QTimer, pyqtSignal, QObject, QThread, Qt
from PyQt5.QtGui import QFont, QColor

import pyqtgraph as pg

from modules.config import MonitorConfig, VariableType
from modules.elf_parser import ElfMapParser
from modules.data_monitor import DataMonitor
from modules.openocd_manager import OpenOCDManager


class MotorMonitorGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.config = MonitorConfig()
        self.parser = ElfMapParser()
        self.monitor = DataMonitor(self.config, self.parser)
        self.openocd_manager = OpenOCDManager(
            config_file=self.config.openocd_config_file,
            interface=self.config.openocd_interface,
            target=self.config.openocd_target
        )
        self.data_history = {}
        self.plots = {}

        self.init_ui()
        self.setup_connections()
        self.load_config_into_ui()

    def init_ui(self):
        self.setWindowTitle("STM32 Motor Monitor")
        self.setGeometry(100, 100, 1200, 800)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # Connection group
        conn_group = QGroupBox("Connection & Files")
        conn_layout = QGridLayout(conn_group)

        self.host_edit = QLineEdit()
        self.port_edit = QSpinBox()
        self.port_edit.setRange(1, 65535)
        self.elf_file_edit = QLineEdit()
        self.map_file_edit = QLineEdit()
        self.openocd_config_edit = QLineEdit()
        self.auto_start_cb = QCheckBox("Auto-start OpenOCD")
        self.connect_btn = QPushButton("Connect")
        self.status_label = QLabel("Disconnected")
        self.save_config_btn = QPushButton("Save Config")

        conn_layout.addWidget(QLabel("Host:"), 0, 0)
        conn_layout.addWidget(self.host_edit, 0, 1)
        conn_layout.addWidget(QLabel("Port:"), 0, 2)
        conn_layout.addWidget(self.port_edit, 0, 3)
        conn_layout.addWidget(QLabel("ELF:"), 1, 0)
        conn_layout.addWidget(self.elf_file_edit, 1, 1)
        conn_layout.addWidget(QLabel("MAP:"), 1, 2)
        conn_layout.addWidget(self.map_file_edit, 1, 3)
        conn_layout.addWidget(QLabel("OpenOCD Config:"), 2, 0)
        conn_layout.addWidget(self.openocd_config_edit, 2, 1)
        conn_layout.addWidget(self.auto_start_cb, 2, 2, 1, 2)
        conn_layout.addWidget(self.connect_btn, 3, 0)
        conn_layout.addWidget(self.save_config_btn, 3, 1)
        conn_layout.addWidget(self.status_label, 3, 2, 1, 2)

        main_layout.addWidget(conn_group)

        # Splitter
        splitter = QSplitter()
        main_layout.addWidget(splitter)

        graph_widget = self.create_graph_widget()
        splitter.addWidget(graph_widget)

        control_widget = self.create_control_widget()
        splitter.addWidget(control_widget)

        splitter.setSizes([800, 400])

    def load_config_into_ui(self):
        self.host_edit.setText(self.config.openocd_host)
        self.port_edit.setValue(self.config.openocd_port)
        self.elf_file_edit.setText(self.config.elf_file)
        self.map_file_edit.setText(self.config.map_file)
        self.openocd_config_edit.setText(self.config.openocd_config_file or "")
        self.auto_start_cb.setChecked(self.config.auto_start_openocd)

    def create_graph_widget(self):
        graph_group = QGroupBox("Real-time Data")
        layout = QVBoxLayout(graph_group)

        controls_layout = QHBoxLayout()
        self.auto_zoom_cb = QCheckBox("Auto Zoom")
        self.auto_zoom_cb.setChecked(True)
        self.clear_btn = QPushButton("Clear History")
        controls_layout.addWidget(self.auto_zoom_cb)
        controls_layout.addWidget(self.clear_btn)
        controls_layout.addStretch()
        layout.addLayout(controls_layout)

        self.plot_widget = pg.GraphicsLayoutWidget()
        layout.addWidget(self.plot_widget)

        # Create plots for each read variable
        for i, var in enumerate(self.config.read_variables):
            title = var.graph.title or f"{var.name} ({var.unit})"
            plot = self.plot_widget.addPlot(row=i//2, col=i%2, title=title)
            plot.setLabel('left', var.name, units=var.unit)
            plot.setLabel('bottom', 'Time', units='s')
            plot.showGrid(x=True, y=True)

            color = QColor(var.graph.color)
            curve = plot.plot(pen=pg.mkPen(color=color, width=2))
            self.plots[var.name] = {'plot': plot, 'curve': curve}
            self.data_history[var.name] = {'time': [], 'values': []}

            # Set Y range if specified
            if var.graph.y_min is not None and var.graph.y_max is not None:
                plot.setYRange(var.graph.y_min, var.graph.y_max)

        return graph_group

    def create_control_widget(self):
        control_group = QGroupBox("Control Panel")
        layout = QVBoxLayout(control_group)

        scroll = QScrollArea()
        scroll_widget = QWidget()
        scroll_layout = QVBoxLayout(scroll_widget)

        self.write_controls = {}

        for var in self.config.write_variables:
            if var.writable:
                var_group = QGroupBox(f"{var.name} ({var.unit})")
                var_layout = QVBoxLayout(var_group)

                if var.type in (VariableType.FLOAT, VariableType.DOUBLE):
                    control = QDoubleSpinBox()
                    control.setDecimals(3)
                    if var.min_val is not None:
                        control.setMinimum(var.min_val)
                    if var.max_val is not None:
                        control.setMaximum(var.max_val)
                elif var.type == VariableType.UINT8 and var.max_val == 1:
                    control = QCheckBox("Enabled")
                else:
                    control = QSpinBox()
                    if var.min_val is not None:
                        control.setMinimum(int(var.min_val))
                    if var.max_val is not None:
                        control.setMaximum(int(var.max_val))

                apply_btn = QPushButton("Apply")
                var_layout.addWidget(control)
                var_layout.addWidget(apply_btn)

                self.write_controls[var.name] = {
                    'control': control,
                    'button': apply_btn,
                    'config': var
                }
                apply_btn.clicked.connect(lambda checked, v=var.name: self.apply_write_value(v))

                scroll_layout.addWidget(var_group)

        scroll.setWidget(scroll_widget)
        scroll.setWidgetResizable(True)
        layout.addWidget(scroll)
        return control_group

    def setup_connections(self):
        self.connect_btn.clicked.connect(self.toggle_connection)
        self.clear_btn.clicked.connect(self.clear_history)
        self.save_config_btn.clicked.connect(self.save_current_config)
        self.monitor.data_updated.connect(self.update_graphs)
        self.monitor.connection_status.connect(self.update_connection_status)

    def save_current_config(self):
        self.config.openocd_host = self.host_edit.text()
        self.config.openocd_port = self.port_edit.value()
        self.config.elf_file = self.elf_file_edit.text()
        self.config.map_file = self.map_file_edit.text()
        self.config.openocd_config_file = self.openocd_config_edit.text() or None
        self.config.auto_start_openocd = self.auto_start_cb.isChecked()
        self.config.save_config()
        QMessageBox.information(self, "Saved", "Configuration saved to config.json")

    def toggle_connection(self):
        if self.connect_btn.text() == "Connect":
            self.config.openocd_host = self.host_edit.text()
            self.config.openocd_port = self.port_edit.value()
            self.config.elf_file = self.elf_file_edit.text()
            self.config.map_file = self.map_file_edit.text()
            self.config.openocd_config_file = self.openocd_config_edit.text() or None
            self.config.auto_start_openocd = self.auto_start_cb.isChecked()

            # Update OpenOCD manager with new settings
            self.openocd_manager.config_file = self.config.openocd_config_file
            self.openocd_manager.interface = self.config.openocd_interface
            self.openocd_manager.target = self.config.openocd_target

            self.parser.elf_file = self.config.elf_file
            self.parser.map_file = self.config.map_file
            self.parser.parse_map_file()

            # Auto-start OpenOCD if enabled
            if self.config.auto_start_openocd and not self.openocd_manager.is_running():
                if not self.openocd_manager.start():
                    QMessageBox.warning(self, "OpenOCD Error",
                                      "Failed to start OpenOCD. Please check your configuration.")
                    return
                # Wait for OpenOCD to be ready
                if not self.openocd_manager.wait_for_connection():
                    QMessageBox.warning(self, "OpenOCD Error",
                                      "OpenOCD started but is not responding. Please check the configuration.")
                    return

            if self.monitor.start():
                self.connect_btn.setText("Disconnect")
            else:
                QMessageBox.warning(self, "Connection Error",
                                  "Failed to connect to OpenOCD or parse symbol files")
        else:
            self.monitor.stop()
            self.connect_btn.setText("Connect")

    def update_connection_status(self, connected):
        self.status_label.setText("Connected" if connected else "Disconnected")
        self.status_label.setStyleSheet("color: green" if connected else "color: red")

    def update_graphs(self, data):
        current_time = time.time()
        for var_name, var_data in data.items():
            if var_name in self.data_history:
                hist = self.data_history[var_name]
                hist['time'].append(current_time)
                hist['values'].append(var_data['value'])

                max_size = self.config.graph_history_size
                if len(hist['time']) > max_size:
                    hist['time'] = hist['time'][-max_size:]
                    hist['values'] = hist['values'][-max_size:]

                if var_name in self.plots:
                    times = np.array(hist['time'])
                    values = np.array(hist['values'])
                    if len(times) > 0:
                        rel_times = times - times[0]
                        self.plots[var_name]['curve'].setData(rel_times, values)
                        if self.auto_zoom_cb.isChecked():
                            self.plots[var_name]['plot'].autoRange()

    def apply_write_value(self, var_name):
        if var_name not in self.write_controls:
            return
        ctrl_info = self.write_controls[var_name]
        control = ctrl_info['control']
        var_config = ctrl_info['config']

        if isinstance(control, QCheckBox):
            value = 1 if control.isChecked() else 0
        else:
            value = control.value()

        if self.monitor.write_variable(var_config, value):
            print(f"Wrote {value} to {var_name}")
        else:
            QMessageBox.warning(self, "Write Error", f"Failed to write to {var_name}")

    def clear_history(self):
        for hist in self.data_history.values():
            hist['time'].clear()
            hist['values'].clear()
        for plot_info in self.plots.values():
            plot_info['curve'].clear()

    def closeEvent(self, event):
        """Clean up OpenOCD when application closes"""
        self.monitor.stop()
        self.openocd_manager.stop()
        event.accept()