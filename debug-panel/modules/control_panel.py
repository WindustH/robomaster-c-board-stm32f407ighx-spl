"""
Unified Control Panel

Variable writing and control interface using the new configuration system
"""

from typing import Dict, Any, Callable, Optional
from PyQt5.QtWidgets import (QGroupBox, QVBoxLayout, QHBoxLayout, QGridLayout,
                           QScrollArea, QWidget, QDoubleSpinBox, QSpinBox,
                           QCheckBox, QPushButton, QLabel, QSlider, QComboBox)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor

from modules.config import MonitorConfig, ControlConfig, VariableDefinition, ControlStyle


class ControlPanel:
    """Control panel for variable writing and control using the unified configuration system"""

    def __init__(self, config: MonitorConfig):
        self.config = config
        self.control_widgets: Dict[str, Dict[str, Any]] = {}
        self._create_widgets()

    def _create_widgets(self):
        """Create all control panel widgets"""
        # Save defaults button
        self.save_defaults_btn = QPushButton("Save as Defaults")
        self.load_defaults_btn = QPushButton("Load Defaults")

    def create_control_group(self) -> QGroupBox:
        """Create and layout the control group box"""
        control_group = QGroupBox("Control Panel")
        layout = QVBoxLayout(control_group)

        # Save/load defaults
        defaults_layout = QHBoxLayout()
        defaults_layout.addWidget(self.save_defaults_btn)
        defaults_layout.addWidget(self.load_defaults_btn)
        defaults_layout.addStretch()
        layout.addLayout(defaults_layout)

        # Scroll area for controls
        scroll = QScrollArea()
        scroll_widget = QWidget()
        self.controls_layout = QVBoxLayout(scroll_widget)
        scroll.setWidget(scroll_widget)
        scroll.setWidgetResizable(True)
        layout.addWidget(scroll)

        # Connect signals
        self.save_defaults_btn.clicked.connect(self._save_defaults)
        self.load_defaults_btn.clicked.connect(self._load_defaults)

        # Create controls from configuration
        self._create_controls_from_config()

        return control_group

    def _create_controls_from_config(self):
        """Create controls based on panel.json configuration"""
        for control_id, control_config in self.config.controls.items():
            self._create_single_control(control_id, control_config)

    def _create_single_control(self, control_id: str, control_config: ControlConfig):
        """Create a single control widget"""
        var = self.config.get_variable_by_id(control_config.data_source)
        if not var:
            print(f"Warning: Variable {control_config.data_source} not found for control {control_id}")
            return

        # Create control group
        control_group = QGroupBox(control_config.title)
        control_layout = QVBoxLayout(control_group)

        # Create control widget based on type
        control_widget = self._create_control_widget(control_config, var)
        if not control_widget:
            return

        # Apply button
        apply_btn = QPushButton("Apply")

        # Value display (if enabled)
        value_label = None
        if control_config.style.show_value:
            value_label = QLabel("Value: 0")
            control_layout.addWidget(value_label)

        control_layout.addWidget(control_widget)
        control_layout.addWidget(apply_btn)

        # Store control information
        self.control_widgets[control_id] = {
            'group': control_group,
            'control': control_widget,
            'button': apply_btn,
            'value_label': value_label,
            'config': control_config,
            'variable': var
        }

        # Connect signals
        if control_config.style.show_value and value_label:
            self._connect_value_display(control_id, control_widget, value_label)

        # Add to layout
        self.controls_layout.addWidget(control_group)

    def _create_control_widget(self, control_config: ControlConfig, var: VariableDefinition):
        """Create appropriate control widget based on control type"""
        style = control_config.style

        if style.control_type == "slider":
            return self._create_slider_control(style, var)
        elif style.control_type == "dropdown":
            return self._create_dropdown_control(style, var)
        elif style.control_type == "spinbox":
            return self._create_spinbox_control(style, var)
        else:
            print(f"Unknown control type: {style.control_type}")
            return None

    def _create_slider_control(self, style: ControlStyle, var: VariableDefinition):
        """Create a slider control"""
        slider = QSlider(Qt.Horizontal)

        # Set range
        min_val = style.min_value if style.min_value is not None else 0
        max_val = style.max_value if style.max_value is not None else 100
        step = style.step if style.step is not None else 1

        # Convert to integer steps for slider
        num_steps = int((max_val - min_val) / step)
        slider.setMinimum(0)
        slider.setMaximum(num_steps)

        # Set default value
        default_val = style.default_value if style.default_value is not None else min_val
        default_step = int((default_val - min_val) / step)
        slider.setValue(default_step)

        # Store conversion parameters
        slider.min_value = min_val
        slider.max_value = max_val
        slider.step = step

        return slider

    def _create_dropdown_control(self, style: ControlStyle, var: VariableDefinition):
        """Create a dropdown control"""
        combo = QComboBox()

        if style.options:
            for option in style.options:
                combo.addItem(option['label'], option['value'])
        else:
            # Default options for common types
            if var.type.name in ['INT8', 'UINT8']:
                for i in range(256):
                    combo.addItem(str(i), i)

        # Set default value
        default_val = style.default_value if style.default_value is not None else 0
        index = combo.findData(default_val)
        if index >= 0:
            combo.setCurrentIndex(index)

        return combo

    def _create_spinbox_control(self, style: ControlStyle, var: VariableDefinition):
        """Create a spinbox control"""
        if var.type.name in ['FLOAT', 'DOUBLE']:
            spinbox = QDoubleSpinBox()
            spinbox.setDecimals(3)
        else:
            spinbox = QSpinBox()

        # Set range
        if style.min_value is not None:
            spinbox.setMinimum(style.min_value)
        if style.max_value is not None:
            spinbox.setMaximum(style.max_value)

        # Set step
        if style.step is not None:
            spinbox.setSingleStep(style.step)

        # Set default value
        if style.default_value is not None:
            spinbox.setValue(style.default_value)

        return spinbox

    def _connect_value_display(self, control_id: str, control_widget, value_label: QLabel):
        """Connect control widget to value display"""
        if isinstance(control_widget, QSlider):
            control_widget.valueChanged.connect(
                lambda value: self._update_slider_value_display(control_id, value)
            )
        elif isinstance(control_widget, QComboBox):
            control_widget.currentIndexChanged.connect(
                lambda: self._update_combo_value_display(control_id)
            )
        elif isinstance(control_widget, (QSpinBox, QDoubleSpinBox)):
            control_widget.valueChanged.connect(
                lambda value: self._update_spinbox_value_display(control_id, value)
            )

    def _update_slider_value_display(self, control_id: str, slider_value: int):
        """Update value display for slider"""
        ctrl_info = self.control_widgets.get(control_id)
        if not ctrl_info or not ctrl_info['value_label']:
            return

        slider = ctrl_info['control']
        min_val = getattr(slider, 'min_value', 0)
        step = getattr(slider, 'step', 1)

        actual_value = min_val + (slider_value * step)
        ctrl_info['value_label'].setText(f"Value: {actual_value:.3f}")

    def _update_combo_value_display(self, control_id: str):
        """Update value display for combo box"""
        ctrl_info = self.control_widgets.get(control_id)
        if not ctrl_info or not ctrl_info['value_label']:
            return

        combo = ctrl_info['control']
        current_value = combo.currentData()
        ctrl_info['value_label'].setText(f"Value: {current_value}")

    def _update_spinbox_value_display(self, control_id: str, value):
        """Update value display for spinbox"""
        ctrl_info = self.control_widgets.get(control_id)
        if not ctrl_info or not ctrl_info['value_label']:
            return

        ctrl_info['value_label'].setText(f"Value: {value}")

    def get_control_value(self, control_id: str) -> Optional[float]:
        """Get the current value from a control"""
        ctrl_info = self.control_widgets.get(control_id)
        if not ctrl_info:
            return None

        control = ctrl_info['control']
        var = ctrl_info['variable']

        if isinstance(control, QSlider):
            min_val = getattr(control, 'min_value', 0)
            step = getattr(control, 'step', 1)
            return min_val + (control.value() * step)

        elif isinstance(control, QComboBox):
            return control.currentData()

        elif isinstance(control, (QSpinBox, QDoubleSpinBox)):
            return control.value()

        return None

    def set_control_value(self, control_id: str, value: float):
        """Set the value of a control"""
        ctrl_info = self.control_widgets.get(control_id)
        if not ctrl_info:
            return

        control = ctrl_info['control']

        if isinstance(control, QSlider):
            min_val = getattr(control, 'min_value', 0)
            step = getattr(control, 'step', 1)
            slider_value = int((value - min_val) / step)
            control.setValue(slider_value)

        elif isinstance(control, QComboBox):
            index = control.findData(value)
            if index >= 0:
                control.setCurrentIndex(index)

        elif isinstance(control, (QSpinBox, QDoubleSpinBox)):
            control.setValue(value)

    def connect_apply_buttons(self, apply_callback: Callable):
        """Connect all apply buttons to the given callback"""
        for control_id, ctrl_info in self.control_widgets.items():
            ctrl_info['button'].clicked.connect(
                lambda checked, cid=control_id: apply_callback(cid)
            )

    def _save_defaults(self):
        """Save current control values as defaults"""
        for control_id, ctrl_info in self.control_widgets.items():
            current_value = self.get_control_value(control_id)
            if current_value is not None:
                ctrl_info['config'].style.default_value = current_value

        # Save to panel.json
        self.config.save_panel_config()
        print("Default values saved to panel.json")

    def _load_defaults(self):
        """Load default values into controls"""
        for control_id, ctrl_info in self.control_widgets.items():
            default_value = ctrl_info['config'].style.default_value
            if default_value is not None:
                self.set_control_value(control_id, default_value)

        print("Default values loaded from panel.json")