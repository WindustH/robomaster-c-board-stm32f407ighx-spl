"""
Unified Graph Panel

Real-time data visualization using the new configuration system
"""

import time
from typing import Dict, Any, List
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QGridLayout,
                           QLabel, QDoubleSpinBox, QCheckBox, QPushButton,
                           QScrollArea, QWidget)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor

import pyqtgraph as pg

from modules.config import MonitorConfig, GraphConfig


class GraphPanel:
    """Graph panel for real-time data visualization using the unified configuration system"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self.data_history: Dict[str, Dict[str, List]] = {}
        self.graph_widgets: Dict[str, pg.PlotWidget] = {}
        self.graph_curves: Dict[str, List[pg.PlotDataItem]] = {}
        self._create_widgets()

    def _create_widgets(self):
        """Create all graph panel widgets"""
        # Update interval control
        self.update_interval_spin = QDoubleSpinBox()
        self.update_interval_spin.setRange(0.001, 1000.0)
        self.update_interval_spin.setValue(self.config.update_interval_ms)
        self.update_interval_spin.setSuffix(" ms")
        self.update_interval_spin.setDecimals(3)

        # Time window control
        self.time_window_spin = QDoubleSpinBox()
        self.time_window_spin.setRange(0.1, 3600.0)
        self.time_window_spin.setValue(self.config.time_window_seconds)
        self.time_window_spin.setSuffix(" s")
        self.time_window_spin.setDecimals(1)

        # Auto-fit checkbox
        self.auto_fit_cb = QCheckBox("Auto Fit Y")
        self.auto_fit_cb.setChecked(True)

        # Clear history button
        self.clear_btn = QPushButton("Clear History")

    def create_graph_group(self) -> QGroupBox:
        """Create and layout the graph group box"""
        graph_group = QGroupBox("Graphs")
        layout = QVBoxLayout(graph_group)

        # Controls
        controls_layout = QHBoxLayout()
        controls_layout.addWidget(QLabel("Update Interval:"))
        controls_layout.addWidget(self.update_interval_spin)
        controls_layout.addWidget(QLabel("Time Window:"))
        controls_layout.addWidget(self.time_window_spin)
        controls_layout.addWidget(self.auto_fit_cb)
        controls_layout.addWidget(self.clear_btn)
        controls_layout.addStretch()
        layout.addLayout(controls_layout)

        # Scroll area for graphs
        scroll = QScrollArea()
        scroll_widget = QWidget()
        self.graphs_layout = QVBoxLayout(scroll_widget)
        scroll.setWidget(scroll_widget)
        scroll.setWidgetResizable(True)
        layout.addWidget(scroll)

        # Connect signals
        self.update_interval_spin.valueChanged.connect(self._on_update_interval_changed)
        self.time_window_spin.valueChanged.connect(self.update_all_graphs)
        self.auto_fit_cb.toggled.connect(self.update_all_graphs)
        self.clear_btn.clicked.connect(self.clear_history)

        # Create graphs from configuration
        self._create_graphs_from_config()

        return graph_group

    def _create_graphs_from_config(self):
        """Create graphs based on panel.json configuration"""
        for graph_id, graph_config in self.config.graphs.items():
            self._create_single_graph(graph_id, graph_config)

    def _create_single_graph(self, graph_id: str, graph_config: GraphConfig):
        """Create a single graph widget"""
        # Create plot widget
        plot_widget = pg.PlotWidget()
        plot_widget.setTitle(graph_config.title, size="12pt")
        plot_widget.setLabel('left', graph_config.style.y_label)
        plot_widget.setLabel('bottom', graph_config.style.x_label)
        plot_widget.showGrid(x=graph_config.style.grid, y=graph_config.style.grid)

        if graph_config.style.legend:
            plot_widget.addLegend()

        # Set Y-axis range if specified
        if graph_config.style.y_min is not None and graph_config.style.y_max is not None:
            plot_widget.setYRange(graph_config.style.y_min, graph_config.style.y_max)

        # Create curves for each data source
        curves = []
        for i, data_source_id in enumerate(graph_config.data_sources):
            var = self.config.get_variable_by_id(data_source_id)
            if var:
                color = graph_config.style.colors[i % len(graph_config.style.colors)]
                pen = pg.mkPen(color=color, width=2)

                if graph_config.style.legend:
                    curve = plot_widget.plot(pen=pen, name=var.name)
                else:
                    curve = plot_widget.plot(pen=pen)

                curves.append(curve)

        # Store references
        self.graph_widgets[graph_id] = plot_widget
        self.graph_curves[graph_id] = curves

        # Initialize data history
        for data_source_id in graph_config.data_sources:
            if data_source_id not in self.data_history:
                self.data_history[data_source_id] = {
                    'time': [],
                    'value': [],
                    'unit': ''
                }

        # Add to layout
        self.graphs_layout.addWidget(plot_widget)

    def update_data(self, data: Dict[str, Dict[str, Any]]):
        """Update graph data with new values"""
        current_time = time.time()

        # Update data history
        for var_id, var_data in data.items():
            if var_id in self.data_history:
                self.data_history[var_id]['time'].append(current_time)
                self.data_history[var_id]['value'].append(var_data['value'])
                self.data_history[var_id]['unit'] = var_data['unit']

                # Limit history size
                max_history = self.config.graph_history_size
                if len(self.data_history[var_id]['time']) > max_history:
                    self.data_history[var_id]['time'] = self.data_history[var_id]['time'][-max_history:]
                    self.data_history[var_id]['value'] = self.data_history[var_id]['value'][-max_history:]

        # Update all graphs
        self.update_all_graphs()

    def update_all_graphs(self):
        """Update all graphs with current data"""
        current_time = time.time()
        time_window = self.time_window_spin.value()

        for graph_id, graph_config in self.config.graphs.items():
            self._update_single_graph(graph_id, graph_config, current_time, time_window)

    def _update_single_graph(self, graph_id: str, graph_config: GraphConfig, current_time: float, time_window: float):
        """Update a single graph"""
        plot_widget = self.graph_widgets.get(graph_id)
        curves = self.graph_curves.get(graph_id, [])

        if not plot_widget or not curves:
            return

        cutoff_time = current_time - time_window

        # Update each curve in the graph
        for i, data_source_id in enumerate(graph_config.data_sources):
            if i >= len(curves):
                continue

            hist = self.data_history.get(data_source_id)
            if not hist or not hist['time']:
                continue

            # Filter data by time window
            valid_indices = [j for j, t in enumerate(hist['time']) if t >= cutoff_time]
            if not valid_indices:
                continue

            times = [hist['time'][j] - current_time for j in valid_indices]
            values = [hist['value'][j] for j in valid_indices]

            # Update curve
            curves[i].setData(times, values)

        # Auto-fit Y-axis if enabled
        if self.auto_fit_cb.isChecked():
            self._auto_fit_y_axis(plot_widget, graph_config.data_sources, cutoff_time, current_time)

    def _auto_fit_y_axis(self, plot_widget: pg.PlotWidget, data_sources: List[str], cutoff_time: float, current_time: float):
        """Auto-fit Y-axis to visible data"""
        all_values = []

        for data_source_id in data_sources:
            hist = self.data_history.get(data_source_id)
            if not hist or not hist['time']:
                continue

            # Get values within time window
            valid_values = [hist['value'][i] for i, t in enumerate(hist['time']) if t >= cutoff_time]
            all_values.extend(valid_values)

        if all_values:
            min_val = min(all_values)
            max_val = max(all_values)

            # Add some padding
            padding = (max_val - min_val) * 0.1
            if padding == 0:
                padding = 1

            plot_widget.setYRange(min_val - padding, max_val + padding)

    def clear_history(self):
        """Clear all graph history data"""
        for hist in self.data_history.values():
            hist['time'].clear()
            hist['value'].clear()

    def get_data_sources_for_graph(self, graph_id: str) -> List[str]:
        """Get data source IDs for a specific graph"""
        graph = self.config.graphs.get(graph_id)
        return graph.data_sources if graph else []

    def get_all_data_sources(self) -> List[str]:
        """Get all data source IDs used in graphs"""
        all_sources = []
        for graph in self.config.graphs.values():
            all_sources.extend(graph.data_sources)
        return list(set(all_sources))  # Remove duplicates

    def _on_update_interval_changed(self):
        """Handle update interval changes"""
        self.config.update_interval_ms = self.update_interval_spin.value()