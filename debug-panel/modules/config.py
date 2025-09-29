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
class GraphConfig:
    color: str = "#ff6464"  # Default red
    visible: bool = True
    y_min: Optional[float] = None
    y_max: Optional[float] = None
    title: Optional[str] = None  # If None, use var.name


@dataclass
class VariableConfig:
    name: str
    offset: int
    type: VariableType
    signed: bool = True
    scale: float = 1.0
    unit: str = ""
    min_val: Optional[float] = None
    max_val: Optional[float] = None
    writable: bool = False
    graph: GraphConfig = None

    def __post_init__(self):
        if self.graph is None:
            self.graph = GraphConfig()


class MonitorConfig:
    def __init__(self, config_file: str = "config.json"):
        self.config_file = config_file

        # Connection settings
        self.openocd_host = "localhost"
        self.openocd_port = 4444

        # OpenOCD settings
        self.auto_start_openocd = True
        self.openocd_config_file = None
        self.openocd_interface = "cmsis-dap"  # Changed from stlink-v2 to cmsis-dap
        self.openocd_target = "stm32f4x"

        # File settings
        self.map_file = "firmware.map"

        # Monitoring settings
        self.update_interval_ms = 100
        self.graph_history_size = 1000

        # Struct names - now fully configurable
        self.read_struct_name = "monitor_read"
        self.write_struct_name = "monitor_write"

        # Variables - now fully configurable via config.json
        self.read_variables: List[VariableConfig] = []
        self.write_variables: List[VariableConfig] = []

        self.load_config()

    def _graph_config_to_dict(self, gc: GraphConfig) -> dict:
        return asdict(gc)

    def _dict_to_graph_config(self, data: dict) -> GraphConfig:
        return GraphConfig(**data)

    def _var_config_to_dict(self, var: VariableConfig) -> dict:
        d = asdict(var)
        d['type'] = var.type.name
        d['graph'] = self._graph_config_to_dict(var.graph)
        return d

    def _dict_to_var_config(self, data: dict) -> VariableConfig:
        var_type = VariableType[data['type']]
        graph = self._dict_to_graph_config(data.get('graph', {}))
        # Remove 'graph' and 'type' so they don't interfere with VariableConfig init
        clean_data = {k: v for k, v in data.items() if k not in ['graph', 'type']}
        return VariableConfig(
            **clean_data,
            type=var_type,
            graph=graph
        )

    def load_config(self):
        try:
            with open(self.config_file, 'r') as f:
                config = json.load(f)

            # Load all basic settings
            for key in ['read_struct_name', 'write_struct_name', 'openocd_host',
                       'openocd_port', 'update_interval_ms', 'graph_history_size',
                       'map_file', 'auto_start_openocd',
                       'openocd_config_file', 'openocd_interface', 'openocd_target']:
                if key in config:
                    setattr(self, key, config[key])

            # Load read variables
            if 'read_variables' in config:
                self.read_variables = [
                    self._dict_to_var_config(v) for v in config['read_variables']
                ]
            else:
                # Default read variables if not specified
                self.read_variables = [
                    VariableConfig("motor_speed", 0, VariableType.FLOAT, scale=1.0, unit="RPM",
                                  graph=GraphConfig(color="#ff6464")),
                    VariableConfig("temperature", 12, VariableType.FLOAT, scale=1.0, unit="°C",
                                  graph=GraphConfig(color="#ffaa00")),
                ]

            # Load write variables
            if 'write_variables' in config:
                self.write_variables = [
                    self._dict_to_var_config(v) for v in config['write_variables']
                ]
            else:
                # Default write variables if not specified
                self.write_variables = [
                    VariableConfig("target_speed", 0, VariableType.FLOAT, writable=True,
                                  min_val=0, max_val=5000, unit="RPM"),
                    VariableConfig("enable_motor", 12, VariableType.UINT8, writable=True,
                                  min_val=0, max_val=1, unit=""),
                ]

        except FileNotFoundError:
            print(f"Config file {self.config_file} not found. Using defaults and creating new config.")
            self.save_config()
        except Exception as e:
            print(f"Error loading config: {e}")

    def save_config(self):
        config = {
            # Connection settings
            'openocd_host': self.openocd_host,
            'openocd_port': self.openocd_port,

            # OpenOCD settings
            'auto_start_openocd': self.auto_start_openocd,
            'openocd_config_file': self.openocd_config_file,
            'openocd_interface': self.openocd_interface,
            'openocd_target': self.openocd_target,

            # File settings
            'map_file': self.map_file,

            # Monitoring settings
            'update_interval_ms': self.update_interval_ms,
            'graph_history_size': self.graph_history_size,

            # Struct names
            'read_struct_name': self.read_struct_name,
            'write_struct_name': self.write_struct_name,

            # Variables
            'read_variables': [self._var_config_to_dict(v) for v in self.read_variables],
            'write_variables': [self._var_config_to_dict(v) for v in self.write_variables],
        }

        # Ensure directory exists
        os.makedirs(os.path.dirname(self.config_file) if os.path.dirname(self.config_file) else '.', exist_ok=True)

        with open(self.config_file, 'w') as f:
            json.dump(config, indent=2, fp=f)