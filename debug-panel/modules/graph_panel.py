"""
Graph Panel Module

Handles the real-time graph plotting and visualization components.
"""

from typing import Dict, Any, List, Optional, Callable
import time
import numpy as np

from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QLabel,
                           QDoubleSpinBox, QCheckBox, QPushButton)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor

import pyqtgraph as pg

from modules.config import MonitorConfig, VariableConfig


class GraphPanel:
    """Graph panel for real-time data visualization"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self.data_history: Dict[str, Dict[str, List]] = {}
        self.plots: Dict[str, Dict[str, Any]] = {}
        self.expanded_graph_callback: Optional[Callable] = None
        self._create_widgets()

    def _create_widgets(self) -> None:
        """Create graph panel widgets"""
        self.auto_zoom_cb = QCheckBox("Auto Zoom Y")
        self.auto_zoom_cb.setChecked(True)
        self.clear_btn = QPushButton("Clear History")

        # Time window controls
        self.time_window_spin = QDoubleSpinBox()
        self.time_window_spin.setRange(0.1, 3600.0)
        self.time_window_spin.setValue(10.0)
        self.time_window_spin.setSuffix(" s")
        self.time_window_spin.setDecimals(1)
        self.time_window_spin.setToolTip("Time window to display (seconds)")

    def create_graph_group(self) -> QGroupBox:
        """Create and layout the graph group box"""
        graph_group = QGroupBox("Real-time Data")
        layout = QVBoxLayout(graph_group)

        # Controls layout
        controls_layout = QHBoxLayout()
        controls_layout.addWidget(QLabel("Time Window:"))
        controls_layout.addWidget(self.time_window_spin)
        controls_layout.addWidget(self.auto_zoom_cb)
        controls_layout.addWidget(self.clear_btn)
        controls_layout.addStretch()
        layout.addLayout(controls_layout)

        # Graph layout
        self.plot_widget = pg.GraphicsLayoutWidget()
        layout.addWidget(self.plot_widget)

        return graph_group

    def create_plots(self) -> None:
        """Create plots for all visible variables"""
        visible_vars = [var for var in self.config.read_variables if var.graph.visible]
        for i, var in enumerate(visible_vars):
            self._create_single_plot(var, i)

    def _create_single_plot(self, var: VariableConfig, index: int) -> None:
        """Create a single plot for a variable"""
        title = var.graph.title or f"{var.name} ({var.unit})"
        plot = self.plot_widget.addPlot(row=index//2, col=index%2, title=title)
        plot.setLabel('left', var.name, units=var.unit)
        plot.setLabel('bottom', 'Time', units='s')
        plot.showGrid(x=True, y=True)

        # Make title clickable if callback is set
        if self.expanded_graph_callback:
            plot.titleLabel.mousePressEvent = lambda event, v=var: self.expanded_graph_callback(v)
            plot.titleLabel.setCursor(Qt.PointingHandCursor)
            plot.titleLabel.setToolTip(f"Click to open expanded view of {var.name}")

        color = QColor(var.graph.color)
        curve = plot.plot(pen=pg.mkPen(color=color, width=2))

        self.plots[var.name] = {'plot': plot, 'curve': curve}
        self.data_history[var.name] = {'time': [], 'values': []}

        # Set Y range if specified
        if var.graph.y_min is not None and var.graph.y_max is not None:
            plot.setYRange(var.graph.y_min, var.graph.y_max)

    def set_expanded_graph_callback(self, callback: Callable) -> None:
        """Set callback function for expanded graph creation"""
        self.expanded_graph_callback = callback

    def update_graphs(self, data: Dict[str, Any]) -> None:
        """Update all graphs with new data"""
        current_time = time.time()
        time_window = self.time_window_spin.value()

        for var_name, var_data in data.items():
            if var_name in self.data_history:
                self._update_single_graph(var_name, var_data, current_time, time_window)

    def _update_single_graph(self, var_name: str, var_data: Any, current_time: float, time_window: float) -> None:
        """Update a single graph with new data"""
        hist = self.data_history[var_name]
        hist['time'].append(current_time)
        hist['values'].append(var_data['value'])

        # Filter data by time window
        cutoff_time = current_time - time_window
        valid_indices = [i for i, t in enumerate(hist['time']) if t >= cutoff_time]

        if valid_indices:
            hist['time'] = [hist['time'][i] for i in valid_indices]
            hist['values'] = [hist['values'][i] for i in valid_indices]
        else:
            # Keep at least the latest point
            hist['time'] = hist['time'][-1:]
            hist['values'] = hist['values'][-1:]

        # Also limit by history size
        max_size = self.config.graph_history_size
        if len(hist['time']) > max_size:
            hist['time'] = hist['time'][-max_size:]
            hist['values'] = hist['values'][-max_size:]

        # Update plot if exists
        if var_name in self.plots:
            self._update_plot_display(var_name, hist, time_window)

    def _update_plot_display(self, var_name: str, hist: Dict[str, List], time_window: float) -> None:
        """Update the visual display of a plot"""
        times = np.array(hist['time'])
        values = np.array(hist['values'])

        if len(times) > 0:
            # Use relative time starting from the oldest point in the window
            rel_times = times - times[0]
            self.plots[var_name]['curve'].setData(rel_times, values)

            # Set X range to show the time window
            plot = self.plots[var_name]['plot']
            plot.setXRange(0, time_window, padding=0)

            # Auto range only Y axis if enabled
            if self.auto_zoom_cb.isChecked():
                plot.enableAutoRange(axis=pg.ViewBox.YAxis)
            else:
                plot.disableAutoRange(axis=pg.ViewBox.YAxis)

    def clear_history(self) -> None:
        """Clear all graph history data"""
        for hist in self.data_history.values():
            hist['time'].clear()
            hist['values'].clear()
        for plot_info in self.plots.values():
            plot_info['curve'].clear()

    def update_motor_graph_titles(self, motor_id: int) -> None:
        """Update graph titles to reflect current motor"""
        import re
        for var in self.config.read_variables:
            if var.graph and var.graph.title:
                if var.name.startswith('motor_'):
                    var.graph.title = re.sub(r'Motor \d+', f'Motor {motor_id}', var.graph.title)
                elif var.name.startswith('pid_'):
                    var.graph.title = re.sub(r'Motor \d+ PID', f'Motor {motor_id} PID', var.graph.title)

        # Recreate plots with updated titles
        self.plot_widget.clear()
        self.create_plots()