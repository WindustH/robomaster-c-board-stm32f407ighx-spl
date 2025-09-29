"""
Main GUI Module

Provides the main application window that orchestrates all UI components.
"""

import sys
import time
from typing import Dict, Any

from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout,
                           QWidget, QSplitter, QMessageBox, QDialog, QLabel,
                           QDoubleSpinBox, QCheckBox)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor

import pyqtgraph as pg

from modules.config import MonitorConfig
from modules.map_parser import MapParser
from modules.data_monitor import DataMonitor
from modules.openocd_manager import OpenOCDManager
from modules.connection_panel import ConnectionPanel
from modules.graph_panel import GraphPanel
from modules.control_panel import ControlPanel


class ExpandedGraphWindow(QDialog):
    """Dialog window for expanded graph view"""

    def __init__(self, parent=None, var_name: str = None, var_config=None, data_history: Dict = None):
        super().__init__(parent)
        self.var_name = var_name
        self.var_config = var_config
        self.data_history = data_history
        self.time_window = parent.graph_panel.time_window_spin.value() if parent else 10.0

        self._init_ui()

    def _init_ui(self) -> None:
        """Initialize the expanded graph UI"""
        self.setWindowTitle(f"Expanded View: {self.var_name}")
        self.setGeometry(200, 200, 800, 600)

        layout = QVBoxLayout(self)

        # Controls
        controls_layout = QHBoxLayout()

        # Time window control
        controls_layout.addWidget(QLabel("Time Window:"))
        self.time_window_spin = QDoubleSpinBox()
        self.time_window_spin.setRange(0.1, 3600.0)
        self.time_window_spin.setValue(self.time_window)
        self.time_window_spin.setSuffix(" s")
        self.time_window_spin.setDecimals(1)
        self.time_window_spin.valueChanged.connect(self.update_graph)

        self.auto_zoom_cb = QCheckBox("Auto Zoom Y")
        self.auto_zoom_cb.setChecked(True)
        self.auto_zoom_cb.toggled.connect(self.update_graph)

        controls_layout.addWidget(self.time_window_spin)
        controls_layout.addWidget(self.auto_zoom_cb)
        controls_layout.addStretch()

        layout.addLayout(controls_layout)

        # Graph
        self.plot_widget = pg.PlotWidget()
        title = self.var_config.graph.title or f"{self.var_name} ({self.var_config.unit})"
        self.plot_widget.setTitle(title, size="14pt")
        self.plot_widget.setLabel('left', self.var_name, units=self.var_config.unit)
        self.plot_widget.setLabel('bottom', 'Time', units='s')
        self.plot_widget.showGrid(x=True, y=True)

        color = QColor(self.var_config.graph.color)
        self.curve = self.plot_widget.plot(pen=pg.mkPen(color=color, width=3))

        layout.addWidget(self.plot_widget)

        # Update graph with current data
        self.update_graph()

    def update_graph(self) -> None:
        """Update the expanded graph with current data"""
        if not self.data_history or self.var_name not in self.data_history:
            return

        hist = self.data_history[self.var_name]
        if not hist['time']:
            return

        current_time = time.time()
        time_window = self.time_window_spin.value()

        # Filter data by time window
        cutoff_time = current_time - time_window
        valid_indices = [i for i, t in enumerate(hist['time']) if t >= cutoff_time]

        if valid_indices:
            times = np.array([hist['time'][i] for i in valid_indices])
            values = np.array([hist['values'][i] for i in valid_indices])
        else:
            # Keep at least the latest point
            times = np.array(hist['time'][-1:])
            values = np.array(hist['values'][-1:])

        if len(times) > 0:
            # Use relative time starting from the oldest point in the window
            rel_times = times - times[0]
            self.curve.setData(rel_times, values)

            # Set X range to show the time window
            self.plot_widget.setXRange(0, time_window, padding=0)

            # Auto range only Y axis if enabled
            if self.auto_zoom_cb.isChecked():
                self.plot_widget.enableAutoRange(axis=pg.ViewBox.YAxis)
            else:
                self.plot_widget.disableAutoRange(axis=pg.ViewBox.YAxis)

    def update_data(self, data_history: Dict) -> None:
        """Update the data history from parent"""
        self.data_history = data_history
        self.update_graph()


class MotorMonitorGUI(QMainWindow):
    """Main application window for STM32 Motor Monitor"""

    def __init__(self):
        super().__init__()
        self.config = MonitorConfig()
        self.parser = MapParser()
        self.monitor = DataMonitor(self.config, self.parser)
        self.openocd_manager = OpenOCDManager(
            config_file=self.config.openocd_config_file,
            interface=self.config.openocd_interface,
            target=self.config.openocd_target
        )
        self.expanded_windows: Dict[str, ExpandedGraphWindow] = {}

        # Initialize UI components
        self.connection_panel = ConnectionPanel(self.config)
        self.graph_panel = GraphPanel(self.config)
        self.control_panel = ControlPanel(self.config)

        self._init_ui()
        self._setup_connections()
        self._load_config_into_ui()

    def _init_ui(self) -> None:
        """Initialize the main UI layout"""
        self.setWindowTitle("STM32 Motor Monitor")
        self.setGeometry(100, 100, 1200, 800)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # Add connection panel
        conn_group = self.connection_panel.create_connection_group()
        main_layout.addWidget(conn_group)

        # Create splitter for graph and control panels
        splitter = QSplitter()
        main_layout.addWidget(splitter)

        # Add graph panel
        graph_widget = self.graph_panel.create_graph_group()
        splitter.addWidget(graph_widget)

        # Add control panel
        control_widget = self.control_panel.create_control_group()
        splitter.addWidget(control_widget)

        splitter.setSizes([800, 400])

        # Set up expanded graph callback and create plots
        self.graph_panel.set_expanded_graph_callback(self.open_expanded_graph)
        self.graph_panel.create_plots()

    def _setup_connections(self) -> None:
        """Set up all signal-slot connections"""
        # Connection panel signals
        self.connection_panel.connect_btn.clicked.connect(self._toggle_connection)
        self.connection_panel.save_config_btn.clicked.connect(self._save_current_config)
        self.connection_panel.motor_apply_btn.clicked.connect(self._change_motor)

        # Graph panel signals
        self.graph_panel.clear_btn.clicked.connect(self._clear_history)

        # Control panel signals
        self.control_panel.connect_apply_buttons(self._apply_write_value)

        # Data monitor signals
        self.monitor.data_updated.connect(self._update_graphs)
        self.monitor.connection_status.connect(self._update_connection_status)

    def _load_config_into_ui(self) -> None:
        """Load configuration values into all UI panels"""
        self.connection_panel.load_config_into_ui()
        self.graph_panel.time_window_spin.setValue(self.config.time_window_seconds)

    def _save_current_config(self) -> None:
        """Save current UI settings to configuration"""
        self.connection_panel.save_config_from_ui()
        self.config.time_window_seconds = self.graph_panel.time_window_spin.value()
        self.config.save_config()
        QMessageBox.information(self, "Saved", "Configuration saved to config.json")

    def _toggle_connection(self) -> None:
        """Toggle OpenOCD connection"""
        if self.connection_panel.connect_btn.text() == "Connect":
            self._connect_to_target()
        else:
            self._disconnect_from_target()

    def _connect_to_target(self) -> None:
        """Connect to target device via OpenOCD"""
        # Save current settings
        self.connection_panel.save_config_from_ui()

        # Update OpenOCD manager with new settings
        self.openocd_manager.config_file = self.config.openocd_config_file
        self.openocd_manager.interface = self.config.openocd_interface
        self.openocd_manager.target = self.config.openocd_target

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
            self.connection_panel.connect_btn.setText("Disconnect")
        else:
            QMessageBox.warning(self, "Connection Error",
                              "Failed to connect to OpenOCD or parse symbol files")

    def _disconnect_from_target(self) -> None:
        """Disconnect from target device"""
        self.monitor.stop()
        self.connection_panel.connect_btn.setText("Connect")

    def _update_connection_status(self, connected: bool) -> None:
        """Update connection status display"""
        self.connection_panel.update_connection_status(connected)

    def _update_graphs(self, data: Dict[str, Any]) -> None:
        """Update all graphs with new data"""
        self.graph_panel.update_graphs(data)

        # Update expanded windows
        for window in self.expanded_windows.values():
            window.update_data(self.graph_panel.data_history)

    def _apply_write_value(self, var_name: str) -> None:
        """Apply write value to target device"""
        value = self.control_panel.get_write_value(var_name)
        if value is None:
            return

        var_config = self.control_panel.write_controls[var_name]['config']
        if self.monitor.write_variable(var_config, value):
            print(f"Wrote {value} to {var_name}")
        else:
            QMessageBox.warning(self, "Write Error", f"Failed to write to {var_name}")

    def _change_motor(self) -> None:
        """Change the motor being monitored"""
        motor_id = self.connection_panel.motor_select.value()

        # Update config with new motor selection
        self.config.motor_to_monitor = motor_id
        self.config.update_motor_offsets(motor_id)

        # Update UI
        self.connection_panel.update_motor_status(motor_id)
        self.graph_panel.update_motor_graph_titles(motor_id)

        # Save config
        self.config.save_config()

        print(f"Changed monitored motor to motor {motor_id}")

    def _clear_history(self) -> None:
        """Clear all graph history data"""
        self.graph_panel.clear_history()

    def open_expanded_graph(self, var_config) -> None:
        """Open expanded graph window for the specified variable"""
        var_name = var_config.name

        # If window already exists, bring it to front
        if var_name in self.expanded_windows:
            window = self.expanded_windows[var_name]
            window.raise_()
            window.activateWindow()
            return

        # Create new expanded window
        window = ExpandedGraphWindow(
            parent=self,
            var_name=var_name,
            var_config=var_config,
            data_history=self.graph_panel.data_history
        )

        # Track the window and clean up when closed
        self.expanded_windows[var_name] = window
        window.finished.connect(lambda: self.expanded_windows.pop(var_name, None))

        window.show()

    def closeEvent(self, event):
        """Clean up OpenOCD when application closes"""
        self.monitor.stop()
        self.openocd_manager.stop()
        event.accept()