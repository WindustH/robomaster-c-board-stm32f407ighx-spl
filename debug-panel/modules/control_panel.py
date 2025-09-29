"""
Control Panel Module

Handles the control interface for writing variables to the target.
"""

from typing import Dict, Any, Callable
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QScrollArea, QWidget,
                           QDoubleSpinBox, QSpinBox, QCheckBox, QPushButton)

from modules.config import MonitorConfig, VariableConfig, VariableType


class ControlPanel:
    """Control panel for variable writing and control"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self.write_controls: Dict[str, Dict[str, Any]] = {}

    def create_control_group(self) -> QGroupBox:
        """Create and layout the control group box"""
        control_group = QGroupBox("Control Panel")
        layout = QVBoxLayout(control_group)

        scroll = QScrollArea()
        scroll_widget = QWidget()
        scroll_layout = QVBoxLayout(scroll_widget)

        self._create_write_controls(scroll_layout)

        scroll.setWidget(scroll_widget)
        scroll.setWidgetResizable(True)
        layout.addWidget(scroll)

        return control_group

    def _create_write_controls(self, layout: QVBoxLayout) -> None:
        """Create controls for all writable variables"""
        for var in self.config.write_variables:
            if var.writable:
                self._create_single_control(var, layout)

    def _create_single_control(self, var: VariableConfig, layout: QVBoxLayout) -> None:
        """Create a single control for a writable variable"""
        var_group = QGroupBox(f"{var.name} ({var.unit})")
        var_layout = QVBoxLayout(var_group)

        control = self._create_control_widget(var)
        apply_btn = QPushButton("Apply")

        self.write_controls[var.name] = {
            'control': control,
            'button': apply_btn,
            'config': var
        }

        var_layout.addWidget(control)
        var_layout.addWidget(apply_btn)
        layout.addWidget(var_group)

    def _create_control_widget(self, var: VariableConfig) -> Any:
        """Create appropriate control widget based on variable type"""
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

        return control

    def get_write_value(self, var_name: str) -> Any:
        """Get the current value from a write control"""
        if var_name not in self.write_controls:
            return None

        ctrl_info = self.write_controls[var_name]
        control = ctrl_info['control']

        if isinstance(control, QCheckBox):
            return 1 if control.isChecked() else 0
        else:
            return control.value()

    def connect_apply_buttons(self, apply_callback: Callable) -> None:
        """Connect all apply buttons to the given callback"""
        for var_name, ctrl_info in self.write_controls.items():
            ctrl_info['button'].clicked.connect(
                lambda checked, v=var_name: apply_callback(v)
            )