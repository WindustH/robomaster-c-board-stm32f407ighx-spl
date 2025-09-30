"""
Connection Configuration Panel

Handles OpenOCD connection settings and configuration
"""

from typing import Dict, Any
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QGridLayout,
                           QLabel, QLineEdit, QSpinBox, QCheckBox, QPushButton,
                           QMessageBox)
from PyQt5.QtCore import Qt

from modules.config import MonitorConfig


class ConnectionPanel:
    """Panel for OpenOCD connection configuration"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self._create_widgets()

    def _create_widgets(self):
        """Create connection configuration widgets"""
        # OpenOCD host
        self.host_edit = QLineEdit(self.config.openocd_host)

        # OpenOCD port
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(self.config.openocd_port)

        # Auto-start OpenOCD
        self.auto_start_cb = QCheckBox("Auto-start OpenOCD")
        self.auto_start_cb.setChecked(self.config.auto_start_openocd)

        # Map file
        self.map_file_edit = QLineEdit(self.config.map_file)
        self.map_file_edit.setPlaceholderText("map.json")

        # Connect button
        self.connect_btn = QPushButton("Connect")

        # Save config button
        self.save_config_btn = QPushButton("Save Configuration")

    def create_connection_group(self) -> QGroupBox:
        """Create and layout the connection group box"""
        connection_group = QGroupBox("Connection Configuration")
        layout = QVBoxLayout(connection_group)

        # Connection settings grid
        settings_layout = QGridLayout()

        settings_layout.addWidget(QLabel("OpenOCD Host:"), 0, 0)
        settings_layout.addWidget(self.host_edit, 0, 1)

        settings_layout.addWidget(QLabel("OpenOCD Port:"), 1, 0)
        settings_layout.addWidget(self.port_spin, 1, 1)

        settings_layout.addWidget(QLabel("Map File:"), 2, 0)
        settings_layout.addWidget(self.map_file_edit, 2, 1)

        layout.addLayout(settings_layout)

        # Auto-start checkbox
        layout.addWidget(self.auto_start_cb)

        # Button layout
        button_layout = QHBoxLayout()
        button_layout.addWidget(self.connect_btn)
        button_layout.addWidget(self.save_config_btn)
        button_layout.addStretch()

        layout.addLayout(button_layout)

        return connection_group

    def save_config_from_ui(self):
        """Save configuration from UI to config object"""
        self.config.openocd_host = self.host_edit.text()
        self.config.openocd_port = self.port_spin.value()
        self.config.auto_start_openocd = self.auto_start_cb.isChecked()
        self.config.map_file = self.map_file_edit.text()

    def load_config_into_ui(self):
        """Load configuration from config object to UI"""
        self.host_edit.setText(self.config.openocd_host)
        self.port_spin.setValue(self.config.openocd_port)
        self.auto_start_cb.setChecked(self.config.auto_start_openocd)
        self.map_file_edit.setText(self.config.map_file)

    def update_connection_status(self, connected: bool):
        """Update connection status display"""
        if connected:
            self.connect_btn.setText("Disconnect")
        else:
            self.connect_btn.setText("Connect")