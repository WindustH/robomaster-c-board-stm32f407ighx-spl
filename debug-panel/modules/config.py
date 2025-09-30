"""
Unified Configuration System

Handles map.json and panel.json configuration with proper data structures
"""

import json
from typing import Dict, Any, List, Optional
from dataclasses import dataclass, field
from enum import Enum


class DataType(Enum):
    """Supported data types"""
    INT8 = 1
    UINT8 = 1
    INT16 = 2
    UINT16 = 2
    INT32 = 4
    UINT32 = 4
    FLOAT = 4
    DOUBLE = 8


@dataclass
class VariableDefinition:
    """Definition of a single variable from map.json"""
    id: str
    name: str
    description: str
    type: DataType
    signed: bool
    scale: float
    unit: str
    offset: int
    field: str  # "read" or "write"


@dataclass
class GraphStyle:
    """Graph styling configuration"""
    title: str = ""
    x_label: str = "Time (s)"
    y_label: str = "Value"
    colors: List[str] = field(default_factory=lambda: ["#ff6464", "#6464ff", "#64ff64", "#ff64ff", "#ffff64"])
    grid: bool = True
    legend: bool = True
    y_min: Optional[float] = None
    y_max: Optional[float] = None


@dataclass
class ControlStyle:
    """Control styling configuration"""
    control_type: str = "slider"  # "slider", "dropdown", "spinbox"
    min_value: Optional[float] = None
    max_value: Optional[float] = None
    step: Optional[float] = None
    default_value: Optional[float] = None
    show_value: bool = True
    color: str = "#6464ff"
    options: List[Dict[str, Any]] = field(default_factory=list)


@dataclass
class GraphConfig:
    """Graph configuration from panel.json"""
    title: str
    data_sources: List[str]
    style: GraphStyle


@dataclass
class ControlConfig:
    """Control configuration from panel.json"""
    title: str
    data_source: str
    style: ControlStyle


class MonitorConfig:
    """Main configuration class handling both map.json and panel.json"""

    def __init__(self, map_file: str = "map.json", panel_file: str = "panel.json"):
        self.map_file = map_file
        self.panel_file = panel_file

        # Connection settings
        self.openocd_host = "localhost"
        self.openocd_port = 4444
        self.auto_start_openocd = True
        self.update_interval_ms = 1
        self.time_window_seconds = 10.0
        self.graph_history_size = 1000

        # Structure names
        self.read_struct_name = "monitor_read_data"
        self.write_struct_name = "monitor_write_data"

        # Data structures
        self.variables: Dict[str, VariableDefinition] = {}
        self.graphs: Dict[str, GraphConfig] = {}
        self.controls: Dict[str, ControlConfig] = {}

        self._load_config()

    def _load_config(self):
        """Load configuration from JSON files"""
        try:
            # Load map.json
            with open(self.map_file, 'r') as f:
                map_data = json.load(f)
                self._parse_map_data(map_data)

            # Load panel.json
            with open(self.panel_file, 'r') as f:
                panel_data = json.load(f)
                self._parse_panel_data(panel_data)

        except FileNotFoundError as e:
            print(f"Configuration file not found: {e}")
        except json.JSONDecodeError as e:
            print(f"Invalid JSON in configuration: {e}")
        except Exception as e:
            print(f"Error loading configuration: {e}")

    def _parse_map_data(self, map_data: Dict[str, Any]):
        """Parse map.json data into VariableDefinition objects"""
        self.variables.clear()

        # Update structure names if provided
        if 'read_struct_name' in map_data:
            self.read_struct_name = map_data['read_struct_name']
        if 'write_struct_name' in map_data:
            self.write_struct_name = map_data['write_struct_name']

        # Parse variables array
        for var_data in map_data.get('variables', []):
            try:
                var_id = var_data.get('id', '')
                var = VariableDefinition(
                    id=var_id,
                    name=var_data.get('name', var_id),
                    description=var_data.get('description', ''),
                    type=DataType[var_data['type']],
                    signed=var_data.get('signed', True),
                    scale=var_data.get('scale', 1.0),
                    unit=var_data.get('unit', ''),
                    offset=var_data['offset'],
                    field=var_data['field']
                )
                self.variables[var_id] = var
            except (KeyError, ValueError) as e:
                print(f"Error parsing variable {var_id}: {e}")

    def _parse_panel_data(self, panel_data: Dict[str, Any]):
        """Parse panel.json data into GraphConfig and ControlConfig objects"""
        self.graphs.clear()
        self.controls.clear()

        # Parse graphs
        for graph_id, graph_data in panel_data.get('graphs', {}).items():
            try:
                style_data = graph_data.get('style', {})
                style = GraphStyle(
                    title=style_data.get('title', graph_data.get('title', '')),
                    x_label=style_data.get('x_label', 'Time (s)'),
                    y_label=style_data.get('y_label', 'Value'),
                    colors=style_data.get('colors', ["#ff6464", "#6464ff", "#64ff64", "#ff64ff", "#ffff64"]),
                    grid=style_data.get('grid', True),
                    legend=style_data.get('legend', True),
                    y_min=style_data.get('y_min'),
                    y_max=style_data.get('y_max')
                )

                graph = GraphConfig(
                    title=graph_data.get('title', graph_id),
                    data_sources=graph_data.get('data_sources', []),
                    style=style
                )
                self.graphs[graph_id] = graph
            except Exception as e:
                print(f"Error parsing graph {graph_id}: {e}")

        # Parse controls
        for control_id, control_data in panel_data.get('controls', {}).items():
            try:
                style_data = control_data.get('style', {})
                style = ControlStyle(
                    control_type=style_data.get('control_type', 'slider'),
                    min_value=style_data.get('min_value'),
                    max_value=style_data.get('max_value'),
                    step=style_data.get('step'),
                    default_value=style_data.get('default_value'),
                    show_value=style_data.get('show_value', True),
                    color=style_data.get('color', "#6464ff"),
                    options=style_data.get('options', [])
                )

                control = ControlConfig(
                    title=control_data.get('title', control_id),
                    data_source=control_data['data_source'],
                    style=style
                )
                self.controls[control_id] = control
            except Exception as e:
                print(f"Error parsing control {control_id}: {e}")

    def get_variable_by_id(self, var_id: str) -> Optional[VariableDefinition]:
        """Get variable definition by ID"""
        return self.variables.get(var_id)

    def get_read_variables(self) -> List[VariableDefinition]:
        """Get all read variables"""
        return [var for var in self.variables.values() if var.field == "read"]

    def get_write_variables(self) -> List[VariableDefinition]:
        """Get all write variables"""
        return [var for var in self.variables.values() if var.field == "write"]

    def add_graph(self, graph_id: str, title: str, data_sources: List[str], style: GraphStyle):
        """Add a new graph configuration"""
        self.graphs[graph_id] = GraphConfig(title=title, data_sources=data_sources, style=style)

    def add_control(self, control_id: str, title: str, data_source: str, style: ControlStyle):
        """Add a new control configuration"""
        self.controls[control_id] = ControlConfig(title=title, data_source=data_source, style=style)

    def save_panel_config(self):
        """Save current panel configuration to panel.json"""
        try:
            panel_data = {
                'graphs': {},
                'controls': {}
            }

            # Serialize graphs
            for graph_id, graph in self.graphs.items():
                panel_data['graphs'][graph_id] = {
                    'title': graph.title,
                    'data_sources': graph.data_sources,
                    'style': {
                        'title': graph.style.title,
                        'x_label': graph.style.x_label,
                        'y_label': graph.style.y_label,
                        'colors': graph.style.colors,
                        'grid': graph.style.grid,
                        'legend': graph.style.legend,
                        'y_min': graph.style.y_min,
                        'y_max': graph.style.y_max
                    }
                }

            # Serialize controls
            for control_id, control in self.controls.items():
                panel_data['controls'][control_id] = {
                    'title': control.title,
                    'data_source': control.data_source,
                    'style': {
                        'control_type': control.style.control_type,
                        'min_value': control.style.min_value,
                        'max_value': control.style.max_value,
                        'step': control.style.step,
                        'default_value': control.style.default_value,
                        'show_value': control.style.show_value,
                        'color': control.style.color,
                        'options': control.style.options
                    }
                }

            with open(self.panel_file, 'w') as f:
                json.dump(panel_data, f, indent=2)

        except Exception as e:
            print(f"Error saving panel configuration: {e}")
            raise

    def validate_configuration(self) -> List[str]:
        """Validate configuration and return list of errors"""
        errors = []

        # Validate graphs
        for graph_id, graph in self.graphs.items():
            for data_source in graph.data_sources:
                if data_source not in self.variables:
                    errors.append(f"Graph '{graph_id}': Unknown data source '{data_source}'")
                elif self.variables[data_source].field != "read":
                    errors.append(f"Graph '{graph_id}': Data source '{data_source}' is not a read variable")

        # Validate controls
        for control_id, control in self.controls.items():
            if control.data_source not in self.variables:
                errors.append(f"Control '{control_id}': Unknown data source '{control.data_source}'")
            elif self.variables[control.data_source].field != "write":
                errors.append(f"Control '{control_id}': Data source '{control.data_source}' is not a write variable")

        return errors