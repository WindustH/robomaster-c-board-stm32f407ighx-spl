"""
Data Source Configuration Panel

Allows configuration of which data sources to monitor and display
"""

from typing import List, Dict, Any
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QGridLayout,
                           QLabel, QListWidget, QListWidgetItem, QPushButton,
                           QCheckBox, QScrollArea, QWidget, QMessageBox)
from PyQt5.QtCore import Qt

from modules.config import MonitorConfig


class DataSourcePanel:
    """Panel for data source configuration"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self._create_widgets()

    def _create_widgets(self):
        """Create data source configuration widgets"""
        # Available variables list
        self.available_vars_list = QListWidget()

        # Selected variables list
        self.selected_vars_list = QListWidget()

        # Add/remove buttons
        self.add_btn = QPushButton(">>")
        self.remove_btn = QPushButton("<<")

        # Apply button
        self.apply_btn = QPushButton("Apply Configuration")

        # Refresh button
        self.refresh_btn = QPushButton("Refresh")

    def create_data_source_group(self) -> QGroupBox:
        """Create and layout the data source group box"""
        data_source_group = QGroupBox("Data Source Configuration")
        layout = QVBoxLayout(data_source_group)

        # Lists layout
        lists_layout = QHBoxLayout()

        # Available variables
        available_layout = QVBoxLayout()
        available_layout.addWidget(QLabel("Available Variables:"))
        available_layout.addWidget(self.available_vars_list)

        # Button column
        button_layout = QVBoxLayout()
        button_layout.addStretch()
        button_layout.addWidget(self.add_btn)
        button_layout.addWidget(self.remove_btn)
        button_layout.addStretch()

        # Selected variables
        selected_layout = QVBoxLayout()
        selected_layout.addWidget(QLabel("Selected Variables:"))
        selected_layout.addWidget(self.selected_vars_list)

        lists_layout.addLayout(available_layout)
        lists_layout.addLayout(button_layout)
        lists_layout.addLayout(selected_layout)

        layout.addLayout(lists_layout)

        # Button layout
        button_layout = QHBoxLayout()
        button_layout.addWidget(self.apply_btn)
        button_layout.addWidget(self.refresh_btn)
        button_layout.addStretch()

        layout.addLayout(button_layout)

        # Connect signals
        self.add_btn.clicked.connect(self._add_selected)
        self.remove_btn.clicked.connect(self._remove_selected)
        self.apply_btn.clicked.connect(self._apply_configuration)
        self.refresh_btn.clicked.connect(self._refresh_lists)

        # Initial population
        self._refresh_lists()

        return data_source_group

    def _refresh_lists(self):
        """Refresh the variable lists"""
        self.available_vars_list.clear()
        self.selected_vars_list.clear()

        # Get all read variables
        read_vars = self.config.get_read_variables()
        write_vars = self.config.get_write_variables()

        # Get currently monitored variables from graphs
        monitored_vars = set()
        for graph in self.config.graphs.values():
            monitored_vars.update(graph.data_sources)

        # Populate lists with read variables
        for var in read_vars:
            # Add indicator for read variables
            item_text = f"[READ] {var.name} ({var.id})"
            if var.id in monitored_vars:
                self.selected_vars_list.addItem(item_text)
            else:
                self.available_vars_list.addItem(item_text)

        # Add write variables to available list with clear indication
        for var in write_vars:
            item_text = f"[WRITE] {var.name} ({var.id})"
            self.available_vars_list.addItem(item_text)

    def _add_selected(self):
        """Add selected variables to monitored list"""
        selected_items = self.available_vars_list.selectedItems()
        for item in selected_items:
            # Extract variable ID from item text
            var_id = self._extract_var_id(item.text())
            if var_id:
                # Add to selected list
                self.selected_vars_list.addItem(item.text())
                # Remove from available list
                self.available_vars_list.takeItem(self.available_vars_list.row(item))

    def _remove_selected(self):
        """Remove selected variables from monitored list"""
        selected_items = self.selected_vars_list.selectedItems()
        for item in selected_items:
            # Extract variable ID from item text
            var_id = self._extract_var_id(item.text())
            if var_id:
                # Add to available list
                self.available_vars_list.addItem(item.text())
                # Remove from selected list
                self.selected_vars_list.takeItem(self.selected_vars_list.row(item))

    def _extract_var_id(self, item_text: str) -> str:
        """Extract variable ID from list item text"""
        # Format: "[READ/WRITE] Variable Name (var_id)"
        if "(" in item_text and ")" in item_text:
            return item_text.split("(")[1].rstrip(")")
        return ""

    def _apply_configuration(self):
        """Apply the current configuration to graphs"""
        try:
            # Get selected variable IDs
            selected_vars = []
            for i in range(self.selected_vars_list.count()):
                item = self.selected_vars_list.item(i)
                var_id = self._extract_var_id(item.text())
                if var_id:
                    selected_vars.append(var_id)

            if not selected_vars:
                QMessageBox.warning(None, "Warning", "No variables selected for monitoring")
                return

            # Update graphs configuration
            # For now, we'll create a single graph with all selected variables
            # In a more advanced version, this could be configurable
            self.config.graphs.clear()

            if selected_vars:
                # Create a default graph with all selected variables
                from modules.config import GraphStyle, GraphConfig

                style = GraphStyle(
                    title="Custom Graph",
                    y_label="Value",
                    x_label="Time (s)",
                    colors=["#ff6464", "#6464ff", "#64ff64", "#ff64ff", "#ffff64"],
                    grid=True,
                    legend=True
                )

                graph = GraphConfig(
                    title="Custom Graph",
                    data_sources=selected_vars,
                    style=style
                )
                self.config.graphs["custom_graph"] = graph

            # Save configuration
            self.config.save_panel_config()

            QMessageBox.information(None, "Success", "Configuration applied successfully")

        except Exception as e:
            QMessageBox.critical(None, "Error", f"Failed to apply configuration: {e}")

    def get_monitored_variables(self) -> List[str]:
        """Get list of currently monitored variable IDs"""
        monitored_vars = []
        for i in range(self.selected_vars_list.count()):
            item = self.selected_vars_list.item(i)
            var_id = self._extract_var_id(item.text())
            if var_id:
                monitored_vars.append(var_id)
        return monitored_vars