"""
Data Source Panel Module

Allows configuration of data sources for graphs and controls.
"""

from typing import Dict, Any, List, Optional
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QGridLayout,
                           QLabel, QComboBox, QPushButton, QLineEdit,
                           QListWidget, QListWidgetItem, QSplitter, QScrollArea,
                           QWidget, QColorDialog, QDoubleSpinBox, QSpinBox,
                           QCheckBox)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor

from modules.config_v2 import MonitorConfigV2, GraphConfig, ControlConfig, GraphStyle, ControlStyle


class DataSourcePanel:
    """Panel for configuring data sources for graphs and controls"""

    def __init__(self, config: MonitorConfigV2):
        self.config = config
        self._create_widgets()

    def _create_widgets(self):
        """Create all data source panel widgets"""
        # Graph configuration widgets
        self.graph_list = QListWidget()
        self.graph_title_edit = QLineEdit()
        self.graph_data_sources_list = QListWidget()
        self.available_variables_list = QListWidget()
        self.add_data_source_btn = QPushButton("Add Data Source")
        self.remove_data_source_btn = QPushButton("Remove Data Source")
        self.new_graph_btn = QPushButton("New Graph")
        self.delete_graph_btn = QPushButton("Delete Graph")
        self.save_graph_btn = QPushButton("Save Graph")

        # Control configuration widgets
        self.control_list = QListWidget()
        self.control_title_edit = QLineEdit()
        self.control_data_source_combo = QComboBox()
        self.control_type_combo = QComboBox()
        self.control_min_spin = QDoubleSpinBox()
        self.control_max_spin = QDoubleSpinBox()
        self.control_step_spin = QDoubleSpinBox()
        self.control_default_spin = QDoubleSpinBox()
        self.control_color_btn = QPushButton("Choose Color")
        self.new_control_btn = QPushButton("New Control")
        self.delete_control_btn = QPushButton("Delete Control")
        self.save_control_btn = QPushButton("Save Control")

    def create_data_source_group(self) -> QGroupBox:
        """Create and layout the data source configuration group box"""
        data_source_group = QGroupBox("Data Source Configuration")
        main_layout = QVBoxLayout(data_source_group)

        # Splitter for graph and control configuration
        splitter = QSplitter(Qt.Horizontal)

        # Graph configuration side
        graph_widget = self._create_graph_config_widget()
        splitter.addWidget(graph_widget)

        # Control configuration side
        control_widget = self._create_control_config_widget()
        splitter.addWidget(control_widget)

        splitter.setSizes([400, 400])
        main_layout.addWidget(splitter)

        # Save all button
        save_all_btn = QPushButton("Save All Configuration")
        save_all_btn.clicked.connect(self._save_all_config)
        main_layout.addWidget(save_all_btn)

        # Load initial data
        self._load_graph_list()
        self._load_control_list()
        self._load_available_variables()
        self._load_write_variables()

        return data_source_group

    def _create_graph_config_widget(self) -> QWidget:
        """Create graph configuration widget"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # Graph list and actions
        graph_list_layout = QHBoxLayout()
        graph_list_layout.addWidget(QLabel("Graphs:"))
        graph_list_layout.addWidget(self.new_graph_btn)
        graph_list_layout.addWidget(self.delete_graph_btn)
        layout.addLayout(graph_list_layout)
        layout.addWidget(self.graph_list)

        # Graph configuration
        config_layout = QGridLayout()
        config_layout.addWidget(QLabel("Title:"), 0, 0)
        config_layout.addWidget(self.graph_title_edit, 0, 1)

        config_layout.addWidget(QLabel("Data Sources:"), 1, 0)
        config_layout.addWidget(self.graph_data_sources_list, 1, 1)

        config_layout.addWidget(QLabel("Available Variables:"), 2, 0)
        config_layout.addWidget(self.available_variables_list, 2, 1)

        data_source_btn_layout = QHBoxLayout()
        data_source_btn_layout.addWidget(self.add_data_source_btn)
        data_source_btn_layout.addWidget(self.remove_data_source_btn)
        config_layout.addLayout(data_source_btn_layout, 3, 1)

        config_layout.addWidget(self.save_graph_btn, 4, 1)

        layout.addLayout(config_layout)

        # Connect signals
        self.graph_list.currentItemChanged.connect(self._on_graph_selected)
        self.new_graph_btn.clicked.connect(self._new_graph)
        self.delete_graph_btn.clicked.connect(self._delete_graph)
        self.add_data_source_btn.clicked.connect(self._add_data_source)
        self.remove_data_source_btn.clicked.connect(self._remove_data_source)
        self.save_graph_btn.clicked.connect(self._save_graph)

        return widget

    def _create_control_config_widget(self) -> QWidget:
        """Create control configuration widget"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # Control list and actions
        control_list_layout = QHBoxLayout()
        control_list_layout.addWidget(QLabel("Controls:"))
        control_list_layout.addWidget(self.new_control_btn)
        control_list_layout.addWidget(self.delete_control_btn)
        layout.addLayout(control_list_layout)
        layout.addWidget(self.control_list)

        # Control configuration
        config_layout = QGridLayout()
        config_layout.addWidget(QLabel("Title:"), 0, 0)
        config_layout.addWidget(self.control_title_edit, 0, 1)

        config_layout.addWidget(QLabel("Data Source:"), 1, 0)
        config_layout.addWidget(self.control_data_source_combo, 1, 1)

        config_layout.addWidget(QLabel("Control Type:"), 2, 0)
        config_layout.addWidget(self.control_type_combo, 2, 1)

        config_layout.addWidget(QLabel("Min Value:"), 3, 0)
        config_layout.addWidget(self.control_min_spin, 3, 1)

        config_layout.addWidget(QLabel("Max Value:"), 4, 0)
        config_layout.addWidget(self.control_max_spin, 4, 1)

        config_layout.addWidget(QLabel("Step:"), 5, 0)
        config_layout.addWidget(self.control_step_spin, 5, 1)

        config_layout.addWidget(QLabel("Default Value:"), 6, 0)
        config_layout.addWidget(self.control_default_spin, 6, 1)

        config_layout.addWidget(QLabel("Color:"), 7, 0)
        config_layout.addWidget(self.control_color_btn, 7, 1)

        config_layout.addWidget(self.save_control_btn, 8, 1)

        layout.addLayout(config_layout)

        # Connect signals
        self.control_list.currentItemChanged.connect(self._on_control_selected)
        self.new_control_btn.clicked.connect(self._new_control)
        self.delete_control_btn.clicked.connect(self._delete_control)
        self.control_color_btn.clicked.connect(self._choose_control_color)
        self.save_control_btn.clicked.connect(self._save_control)

        # Setup control type combo
        self.control_type_combo.addItems(["slider", "dropdown", "spinbox"])

        return widget

    def _load_graph_list(self):
        """Load graphs into the list widget"""
        self.graph_list.clear()
        for graph_id in self.config.graphs:
            item = QListWidgetItem(graph_id)
            item.setData(Qt.UserRole, graph_id)
            self.graph_list.addItem(item)

    def _load_control_list(self):
        """Load controls into the list widget"""
        self.control_list.clear()
        for control_id in self.config.controls:
            item = QListWidgetItem(control_id)
            item.setData(Qt.UserRole, control_id)
            self.control_list.addItem(item)

    def _load_available_variables(self):
        """Load available read variables"""
        self.available_variables_list.clear()
        for var in self.config.get_read_variables():
            item = QListWidgetItem(f"{var.name} ({var.id})")
            item.setData(Qt.UserRole, var.id)
            self.available_variables_list.addItem(item)

    def _load_write_variables(self):
        """Load write variables for control data source"""
        self.control_data_source_combo.clear()
        for var in self.config.get_write_variables():
            self.control_data_source_combo.addItem(f"{var.name} ({var.id})", var.id)

    def _on_graph_selected(self, current, previous):
        """Handle graph selection"""
        if not current:
            return

        graph_id = current.data(Qt.UserRole)
        graph = self.config.graphs.get(graph_id)
        if graph:
            self.graph_title_edit.setText(graph.title)
            self._load_graph_data_sources(graph.data_sources)

    def _on_control_selected(self, current, previous):
        """Handle control selection"""
        if not current:
            return

        control_id = current.data(Qt.UserRole)
        control = self.config.controls.get(control_id)
        if control:
            self.control_title_edit.setText(control.title)

            # Set data source
            index = self.control_data_source_combo.findData(control.data_source)
            if index >= 0:
                self.control_data_source_combo.setCurrentIndex(index)

            # Set control type
            index = self.control_type_combo.findText(control.style.control_type)
            if index >= 0:
                self.control_type_combo.setCurrentIndex(index)

            # Set style values
            self.control_min_spin.setValue(control.style.min_value or 0)
            self.control_max_spin.setValue(control.style.max_value or 100)
            self.control_step_spin.setValue(control.style.step or 1)
            self.control_default_spin.setValue(control.style.default_value or 0)

    def _load_graph_data_sources(self, data_sources: List[str]):
        """Load data sources for the selected graph"""
        self.graph_data_sources_list.clear()
        for source_id in data_sources:
            var = self.config.get_variable_by_id(source_id)
            if var:
                item = QListWidgetItem(f"{var.name} ({source_id})")
                item.setData(Qt.UserRole, source_id)
                self.graph_data_sources_list.addItem(item)

    def _new_graph(self):
        """Create a new graph"""
        graph_id = f"graph_{len(self.config.graphs) + 1}"
        graph_style = GraphStyle(colors=["#ff6464"])
        self.config.add_graph(graph_id, "New Graph", [], graph_style)
        self._load_graph_list()

    def _delete_graph(self):
        """Delete the selected graph"""
        current = self.graph_list.currentItem()
        if current:
            graph_id = current.data(Qt.UserRole)
            del self.config.graphs[graph_id]
            self._load_graph_list()

    def _new_control(self):
        """Create a new control"""
        control_id = f"control_{len(self.config.controls) + 1}"
        control_style = ControlStyle(control_type="slider")
        self.config.add_control(control_id, "New Control", "", control_style)
        self._load_control_list()

    def _delete_control(self):
        """Delete the selected control"""
        current = self.control_list.currentItem()
        if current:
            control_id = current.data(Qt.UserRole)
            del self.config.controls[control_id]
            self._load_control_list()

    def _add_data_source(self):
        """Add selected variable as data source"""
        current = self.available_variables_list.currentItem()
        graph_current = self.graph_list.currentItem()
        if current and graph_current:
            var_id = current.data(Qt.UserRole)
            graph_id = graph_current.data(Qt.UserRole)

            graph = self.config.graphs.get(graph_id)
            if graph and var_id not in graph.data_sources:
                graph.data_sources.append(var_id)
                self._load_graph_data_sources(graph.data_sources)

    def _remove_data_source(self):
        """Remove selected data source"""
        current = self.graph_data_sources_list.currentItem()
        graph_current = self.graph_list.currentItem()
        if current and graph_current:
            var_id = current.data(Qt.UserRole)
            graph_id = graph_current.data(Qt.UserRole)

            graph = self.config.graphs.get(graph_id)
            if graph and var_id in graph.data_sources:
                graph.data_sources.remove(var_id)
                self._load_graph_data_sources(graph.data_sources)

    def _choose_control_color(self):
        """Choose color for control"""
        color = QColorDialog.getColor()
        if color.isValid():
            self.control_color_btn.setText(color.name())

    def _save_graph(self):
        """Save current graph configuration"""
        current = self.graph_list.currentItem()
        if current:
            graph_id = current.data(Qt.UserRole)
            graph = self.config.graphs.get(graph_id)
            if graph:
                graph.title = self.graph_title_edit.text()
                # Data sources are already updated in the list

    def _save_control(self):
        """Save current control configuration"""
        current = self.control_list.currentItem()
        if current:
            control_id = current.data(Qt.UserRole)
            control = self.config.controls.get(control_id)
            if control:
                control.title = self.control_title_edit.text()
                control.data_source = self.control_data_source_combo.currentData()
                control.style.control_type = self.control_type_combo.currentText()
                control.style.min_value = self.control_min_spin.value()
                control.style.max_value = self.control_max_spin.value()
                control.style.step = self.control_step_spin.value()
                control.style.default_value = self.control_default_spin.value()
                control.style.color = self.control_color_btn.text()

    def _save_all_config(self):
        """Save all configuration to panel.json"""
        self.config.save_panel_config()
        print("Configuration saved to panel.json")