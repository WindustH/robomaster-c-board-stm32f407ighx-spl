import json
import os
from typing import Dict, Any, List, Optional
from dataclasses import dataclass, asdict
from enum import Enum


class VariableType(Enum):
    INT8 = 1
    INT16 = 2
    INT32 = 4
    UINT8 = 1
    UINT16 = 2
    UINT32 = 4
    FLOAT = 4
    DOUBLE = 8


@dataclass
class GraphStyle:
    colors: List[str]
    y_min: Optional[float] = None
    y_max: Optional[float] = None
    y_label: str = ""
    x_label: str = "Time (s)"
    legend: bool = True
    grid: bool = True


@dataclass
class ControlStyle:
    control_type: str  # "slider", "dropdown", "spinbox"
    color: str = "#000000"
    min_value: Optional[float] = None
    max_value: Optional[float] = None
    step: Optional[float] = None
    default_value: Optional[float] = None
    show_value: bool = True
    options: Optional[List[Dict[str, Any]]] = None


@dataclass
class VariableDefinition:
    id: str
    name: str
    description: str
    type: VariableType
    signed: bool
    scale: float
    unit: str
    offset: int
    field: str  # "read" or "write"


@dataclass
class GraphConfig:
    id: str
    title: str
    data_sources: List[str]
    style: GraphStyle


@dataclass
class ControlConfig:
    id: str
    title: str
    data_source: str
    style: ControlStyle


class MonitorConfigV2:
    def __init__(self, config_dir: str = "."):
        self.config_dir = config_dir

        # Connection settings
        self.openocd_host = "localhost"
        self.openocd_port = 4444
        self.auto_start_openocd = True
        self.openocd_config_file = None
        self.openocd_interface = "cmsis-dap"
        self.openocd_target = "stm32f4x"

        # File settings
        self.map_file = "../build/stm32f407ighx.map"

        # Monitoring settings
        self.update_interval_ms = 100
        self.graph_history_size = 1000
        self.time_window_seconds = 10.0

        # Struct names
        self.read_struct_name = "monitor_read_data"
        self.write_struct_name = "monitor_write_data"

        # Data structures
        self.variables: Dict[str, VariableDefinition] = {}
        self.graphs: Dict[str, GraphConfig] = {}
        self.controls: Dict[str, ControlConfig] = {}

        self.load_config()

    def load_config(self):
        """Load configuration from map.json and panel.json"""
        try:
            # Load map.json
            map_path = os.path.join(self.config_dir, "map.json")
            with open(map_path, 'r') as f:
                map_data = json.load(f)

            # Load struct names
            self.read_struct_name = map_data.get('read_struct_name', self.read_struct_name)
            self.write_struct_name = map_data.get('write_struct_name', self.write_struct_name)

            # Load variables
            for var_data in map_data.get('variables', []):
                var_def = VariableDefinition(
                    id=var_data['id'],
                    name=var_data['name'],
                    description=var_data['description'],
                    type=VariableType[var_data['type']],
                    signed=var_data['signed'],
                    scale=var_data['scale'],
                    unit=var_data['unit'],
                    offset=var_data['offset'],
                    field=var_data['field']
                )
                self.variables[var_data['id']] = var_def

            # Load panel.json
            panel_path = os.path.join(self.config_dir, "panel.json")
            with open(panel_path, 'r') as f:
                panel_data = json.load(f)

            # Load graphs
            for graph_data in panel_data.get('graphs', []):
                style_data = graph_data['style']
                graph_style = GraphStyle(
                    colors=style_data['colors'],
                    y_min=style_data.get('y_min'),
                    y_max=style_data.get('y_max'),
                    y_label=style_data.get('y_label', ''),
                    x_label=style_data.get('x_label', 'Time (s)'),
                    legend=style_data.get('legend', True),
                    grid=style_data.get('grid', True)
                )
                graph_config = GraphConfig(
                    id=graph_data['id'],
                    title=graph_data['title'],
                    data_sources=graph_data['data_sources'],
                    style=graph_style
                )
                self.graphs[graph_data['id']] = graph_config

            # Load controls
            for control_data in panel_data.get('controls', []):
                style_data = control_data['style']
                control_style = ControlStyle(
                    control_type=style_data['control_type'],
                    color=style_data.get('color', '#000000'),
                    min_value=style_data.get('min_value'),
                    max_value=style_data.get('max_value'),
                    step=style_data.get('step'),
                    default_value=style_data.get('default_value'),
                    show_value=style_data.get('show_value', True),
                    options=style_data.get('options')
                )
                control_config = ControlConfig(
                    id=control_data['id'],
                    title=control_data['title'],
                    data_source=control_data['data_source'],
                    style=control_style
                )
                self.controls[control_data['id']] = control_config

        except FileNotFoundError as e:
            print(f"Config file not found: {e}")
        except Exception as e:
            print(f"Error loading config: {e}")

    def get_variable_by_id(self, var_id: str) -> Optional[VariableDefinition]:
        """Get variable definition by ID"""
        return self.variables.get(var_id)

    def get_read_variables(self) -> List[VariableDefinition]:
        """Get all read variables"""
        return [v for v in self.variables.values() if v.field == "read"]

    def get_write_variables(self) -> List[VariableDefinition]:
        """Get all write variables"""
        return [v for v in self.variables.values() if v.field == "write"]

    def get_variable_offset(self, var_id: str) -> Optional[int]:
        """Get offset for a variable by ID"""
        var = self.get_variable_by_id(var_id)
        return var.offset if var else None

    def save_panel_config(self):
        """Save current panel configuration to panel.json"""
        panel_data = {
            "graphs": [],
            "controls": []
        }

        # Save graphs
        for graph in self.graphs.values():
            graph_data = {
                "id": graph.id,
                "title": graph.title,
                "data_sources": graph.data_sources,
                "style": {
                    "colors": graph.style.colors,
                    "y_min": graph.style.y_min,
                    "y_max": graph.style.y_max,
                    "y_label": graph.style.y_label,
                    "x_label": graph.style.x_label,
                    "legend": graph.style.legend,
                    "grid": graph.style.grid
                }
            }
            panel_data["graphs"].append(graph_data)

        # Save controls
        for control in self.controls.values():
            control_data = {
                "id": control.id,
                "title": control.title,
                "data_source": control.data_source,
                "style": {
                    "control_type": control.style.control_type,
                    "color": control.style.color,
                    "min_value": control.style.min_value,
                    "max_value": control.style.max_value,
                    "step": control.style.step,
                    "default_value": control.style.default_value,
                    "show_value": control.style.show_value,
                    "options": control.style.options
                }
            }
            panel_data["controls"].append(control_data)

        panel_path = os.path.join(self.config_dir, "panel.json")
        with open(panel_path, 'w') as f:
            json.dump(panel_data, indent=2, fp=f)

    def add_graph(self, graph_id: str, title: str, data_sources: List[str], style: GraphStyle):
        """Add a new graph configuration"""
        graph_config = GraphConfig(
            id=graph_id,
            title=title,
            data_sources=data_sources,
            style=style
        )
        self.graphs[graph_id] = graph_config

    def add_control(self, control_id: str, title: str, data_source: str, style: ControlStyle):
        """Add a new control configuration"""
        control_config = ControlConfig(
            id=control_id,
            title=title,
            data_source=data_source,
            style=style
        )
        self.controls[control_id] = control_config