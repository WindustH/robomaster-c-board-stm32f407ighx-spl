"""
Main Debug Panel

Unified debug panel with new configuration system using map.json and panel.json
"""

import sys
import os
from typing import Dict, Any

from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout,
                           QWidget, QSplitter, QTabWidget, QMessageBox, QLabel, QPushButton)
from PyQt5.QtCore import Qt

from modules.config import MonitorConfig
from modules.map_parser import MapParser
from modules.data_monitor import DataMonitor
from modules.openocd_interface import OpenOCDInterface
from modules.graph_panel import GraphPanel
from modules.control_panel import ControlPanel


class MotorMonitorGUI(QMainWindow):
    """Main GUI class for unified debug panel"""

    def __init__(self):
        super().__init__()

        # Initialize configuration
        self.config = MonitorConfig()
        self.parser = MapParser(self.config.map_file)
        self.openocd = OpenOCDInterface(self.config.openocd_host, self.config.openocd_port)
        self.data_monitor = DataMonitor(self.config, self.parser)

        # Initialize panels
        self.graph_panel = GraphPanel(self.config)
        self.control_panel = ControlPanel(self.config)

        self._init_ui()
        self._connect_signals()

    def _init_ui(self) -> None:
        """Initialize the main UI"""
        self.setWindowTitle("STM32F407 Motor Control Debug Panel")
        self.setGeometry(100, 100, 1400, 900)

        # Create central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Main layout
        main_layout = QVBoxLayout(central_widget)

        # Connection status
        self.status_label = QLabel("Disconnected")
        self.status_label.setStyleSheet("QLabel { color: red; font-weight: bold; }")
        main_layout.addWidget(self.status_label)

        # Connect button
        self.connect_btn = QPushButton("Connect")
        main_layout.addWidget(self.connect_btn)

        # Splitter for graph and control panels
        splitter = QSplitter(Qt.Horizontal)

        # Graph panel
        graph_group = self.graph_panel.create_graph_group()
        splitter.addWidget(graph_group)

        # Control panel
        control_group = self.control_panel.create_control_group()
        splitter.addWidget(control_group)

        splitter.setSizes([700, 300])
        main_layout.addWidget(splitter)

        # Set monitored variables based on current graph configuration
        self._update_monitored_variables()

    def _connect_signals(self) -> None:
        """Connect all signals and slots"""
        # Connection button
        self.connect_btn.clicked.connect(self._connect_to_target)

        # Data monitor signals
        self.data_monitor.data_updated.connect(self._on_data_updated)
        self.data_monitor.connection_status.connect(self._on_connection_status)

        # Control panel signals
        self.control_panel.connect_apply_buttons(self._on_control_apply)

    def _connect_to_target(self) -> None:
        """Connect to target device"""
        try:
            # Connect to OpenOCD
            if not self.openocd.connect():
                QMessageBox.warning(self, "Connection Error", "Failed to connect to OpenOCD")
                return

            # Connect data monitor
            if self.data_monitor.start():
                print("Connected to target device")
            else:
                QMessageBox.warning(self, "Connection Error", "Failed to connect to target device")

        except Exception as e:
            QMessageBox.critical(self, "Connection Error", f"Failed to connect: {e}")

    def _on_connection_status(self, connected: bool) -> None:
        """Handle connection status changes"""
        if connected:
            self.status_label.setText("Connected")
            self.status_label.setStyleSheet("QLabel { color: green; font-weight: bold; }")
            self.connect_btn.setText("Disconnect")
        else:
            self.status_label.setText("Disconnected")
            self.status_label.setStyleSheet("QLabel { color: red; font-weight: bold; }")
            self.connect_btn.setText("Connect")

    def _on_data_updated(self, data: Dict[str, Dict[str, Any]]) -> None:
        """Handle new data from data monitor"""
        self.graph_panel.update_data(data)

    def _on_control_apply(self, control_id: str) -> None:
        """Handle control apply button click"""
        try:
            value = self.control_panel.get_control_value(control_id)
            if value is not None:
                control_config = self.config.controls.get(control_id)
                if control_config:
                    success = self.data_monitor.write_variable(control_config.data_source, value)
                    if success:
                        print(f"Applied {value} to {control_config.data_source}")
                    else:
                        print(f"Failed to write to {control_config.data_source}")
        except Exception as e:
            print(f"Control apply error: {e}")

    def _update_monitored_variables(self) -> None:
        """Update which variables to monitor based on graph configuration"""
        monitored_vars = self.graph_panel.get_all_data_sources()
        self.data_monitor.set_monitored_variables(monitored_vars)

    def closeEvent(self, event) -> None:
        """Handle application close"""
        self.data_monitor.stop()
        self.openocd.disconnect()
        event.accept()


def main():
    """Main application entry point"""
    app = QApplication(sys.argv)

    # Set application style
    app.setStyle('Fusion')

    # Create and show main window
    window = MotorMonitorGUI()
    window.show()

    # Start application
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()