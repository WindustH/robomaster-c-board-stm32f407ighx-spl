"""
Connection Panel Module

Handles the connection settings and motor selection UI components.
"""

from typing import Optional, Callable
from PyQt5.QtWidgets import (QGroupBox, QGridLayout, QLabel, QLineEdit,
                           QSpinBox, QCheckBox, QPushButton)
from PyQt5.QtCore import Qt

from modules.config import MonitorConfig


class ConnectionPanel:
    """Connection panel for OpenOCD settings and motor selection"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self._create_widgets()

    def _create_widgets(self) -> None:
        """Create all connection panel widgets"""
        # Connection settings
        self.host_edit = QLineEdit()
        self.port_edit = QSpinBox()
        self.port_edit.setRange(1, 65535)
        self.map_file_edit = QLineEdit()
        self.openocd_config_edit = QLineEdit()
        self.auto_start_cb = QCheckBox("Auto-start OpenOCD")
        self.connect_btn = QPushButton("Connect")
        self.status_label = QLabel("Disconnected")
        self.save_config_btn = QPushButton("Save Config")

        # Motor selection
        self.motor_select = QSpinBox()
        self.motor_select.setRange(0, 7)
        self.motor_select.setValue(1)
        self.motor_select.setToolTip("Select motor to monitor (0-7)")
        self.motor_apply_btn = QPushButton("Apply Motor")
        self.motor_status_label = QLabel("Motor 1")

    def create_connection_group(self) -> QGroupBox:
        """Create and layout the connection group box"""
        conn_group = QGroupBox("Connection & Files")
        conn_layout = QGridLayout(conn_group)

        # Add widgets to layout
        conn_layout.addWidget(QLabel("Host:"), 0, 0)
        conn_layout.addWidget(self.host_edit, 0, 1)
        conn_layout.addWidget(QLabel("Port:"), 0, 2)
        conn_layout.addWidget(self.port_edit, 0, 3)
        conn_layout.addWidget(QLabel("MAP:"), 1, 0)
        conn_layout.addWidget(self.map_file_edit, 1, 1, 1, 3)
        conn_layout.addWidget(QLabel("OpenOCD Config:"), 2, 0)
        conn_layout.addWidget(self.openocd_config_edit, 2, 1)
        conn_layout.addWidget(self.auto_start_cb, 2, 2, 1, 2)
        conn_layout.addWidget(QLabel("Motor:"), 3, 0)
        conn_layout.addWidget(self.motor_select, 3, 1)
        conn_layout.addWidget(self.motor_apply_btn, 3, 2)
        conn_layout.addWidget(self.motor_status_label, 3, 3)
        conn_layout.addWidget(self.connect_btn, 4, 0)
        conn_layout.addWidget(self.save_config_btn, 4, 1)
        conn_layout.addWidget(self.status_label, 4, 2, 1, 2)

        return conn_group

    def load_config_into_ui(self) -> None:
        """Load configuration values into UI widgets"""
        self.host_edit.setText(self.config.openocd_host)
        self.port_edit.setValue(self.config.openocd_port)
        self.map_file_edit.setText(self.config.map_file)
        self.openocd_config_edit.setText(self.config.openocd_config_file or "")
        self.auto_start_cb.setChecked(self.config.auto_start_openocd)
        self.motor_select.setValue(self.config.motor_to_monitor)
        self.motor_status_label.setText(f"Motor {self.config.motor_to_monitor}")

    def save_config_from_ui(self) -> None:
        """Save UI widget values to configuration"""
        self.config.openocd_host = self.host_edit.text()
        self.config.openocd_port = self.port_edit.value()
        self.config.map_file = self.map_file_edit.text()
        self.config.openocd_config_file = self.openocd_config_edit.text() or None
        self.config.auto_start_openocd = self.auto_start_cb.isChecked()
        self.config.motor_to_monitor = self.motor_select.value()

    def update_connection_status(self, connected: bool) -> None:
        """Update connection status display"""
        self.status_label.setText("Connected" if connected else "Disconnected")
        self.status_label.setStyleSheet("color: green" if connected else "color: red")

    def update_motor_status(self, motor_id: int) -> None:
        """Update motor status display"""
        self.motor_status_label.setText(f"Motor {motor_id}")