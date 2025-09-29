"""
Unified Data Monitor

Handles data acquisition and processing using the new configuration system
"""

import time
import threading
import struct
from typing import Dict, Any, List, Optional
from PyQt5.QtCore import QObject, pyqtSignal

from modules.config import MonitorConfig, VariableDefinition
from modules.map_parser import MapParser
from modules.openocd_interface import OpenOCDInterface


class DataMonitor(QObject):
    """Main data monitor class using the unified configuration system"""

    data_updated = pyqtSignal(dict)  # {data_source_id: {value, unit, timestamp}}
    connection_status = pyqtSignal(bool)

    def __init__(self, config: MonitorConfig, parser: MapParser):
        super().__init__()
        self.config = config
        self.parser = parser
        self.openocd = OpenOCDInterface(config.openocd_host, config.openocd_port)
        self.running = False
        self.read_address = None
        self.write_address = None

        # Track which variables to monitor
        self.monitored_variables: List[str] = []

    def set_monitored_variables(self, variable_ids: List[str]):
        """Set which variables to monitor"""
        self.monitored_variables = variable_ids

    def start(self):
        """Start monitoring"""
        self.read_address = self.parser.get_symbol_address(self.config.read_struct_name)
        self.write_address = self.parser.get_symbol_address(self.config.write_struct_name)

        if not self.read_address:
            print(f"Could not find address for {self.config.read_struct_name}")
            return False

        if not self.openocd.connect():
            self.connection_status.emit(False)
            return False

        self.connection_status.emit(True)
        self.running = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop)
        self.monitor_thread.daemon = True
        self.monitor_thread.start()
        return True

    def stop(self):
        """Stop monitoring"""
        self.running = False
        self.openocd.disconnect()
        self.connection_status.emit(False)

    def write_variable(self, data_source_id: str, value: float) -> bool:
        """Write value to variable using data source ID"""
        if not self.write_address or not self.openocd.connected:
            return False

        try:
            var = self.config.get_variable_by_id(data_source_id)
            if not var or var.field != "write":
                print(f"Invalid write variable: {data_source_id}")
                return False

            # Pack data based on variable type
            data = self._pack_value(value, var)
            if not data:
                return False

            # Write to memory
            self.openocd.write_memory(self.write_address + var.offset, data)
            return True

        except Exception as e:
            print(f"Write error for {data_source_id}: {e}")
            return False

    def _pack_value(self, value: float, var: VariableDefinition) -> bytes:
        """Pack value into bytes based on variable type"""
        try:
            if var.type.name == "FLOAT":
                return struct.pack('<f', float(value))
            elif var.type.name == "DOUBLE":
                return struct.pack('<d', float(value))
            elif var.type.name in ["INT8", "UINT8"]:
                fmt = '<B' if not var.signed else '<b'
                return struct.pack(fmt, int(value))
            elif var.type.name in ["INT16", "UINT16"]:
                fmt = '<H' if not var.signed else '<h'
                return struct.pack(fmt, int(value))
            elif var.type.name in ["INT32", "UINT32"]:
                fmt = '<I' if not var.signed else '<i'
                return struct.pack(fmt, int(value))
            else:
                print(f"Unsupported type: {var.type.name}")
                return b''
        except Exception as e:
            print(f"Packing error: {e}")
            return b''

    def _monitor_loop(self):
        """Main monitoring loop"""
        while self.running:
            try:
                if not self.monitored_variables or self.read_address is None:
                    time.sleep(0.1)
                    continue

                # Calculate required memory range
                max_offset = 0
                for var_id in self.monitored_variables:
                    var = self.config.get_variable_by_id(var_id)
                    if var:
                        max_offset = max(max_offset, var.offset + var.type.value)

                if max_offset == 0:
                    time.sleep(0.1)
                    continue

                # Read memory
                data = self.openocd.read_memory(self.read_address, max_offset)
                parsed_data = {}

                # Parse each monitored variable
                for var_id in self.monitored_variables:
                    var = self.config.get_variable_by_id(var_id)
                    if not var:
                        continue

                    try:
                        if var.offset + var.type.value <= len(data):
                            raw_bytes = data[var.offset:var.offset + var.type.value]
                            value = self._unpack_value(raw_bytes, var)

                            if value is not None:
                                parsed_data[var_id] = {
                                    'value': value * var.scale,
                                    'unit': var.unit,
                                    'timestamp': time.time()
                                }
                    except Exception as e:
                        print(f"Parse error for {var_id}: {e}")

                self.data_updated.emit(parsed_data)
                time.sleep(self.config.update_interval_ms / 1000.0)

            except Exception as e:
                print(f"Monitor loop error: {e}")
                time.sleep(0.5)

    def _unpack_value(self, raw_bytes: bytes, var: VariableDefinition):
        """Unpack bytes to value based on variable type"""
        try:
            if var.type.name == "FLOAT":
                return struct.unpack('<f', raw_bytes)[0]
            elif var.type.name == "DOUBLE":
                return struct.unpack('<d', raw_bytes)[0]
            elif var.type.name == "INT8":
                return struct.unpack('<b', raw_bytes)[0]
            elif var.type.name == "UINT8":
                return struct.unpack('<B', raw_bytes)[0]
            elif var.type.name == "INT16":
                return struct.unpack('<h', raw_bytes)[0]
            elif var.type.name == "UINT16":
                return struct.unpack('<H', raw_bytes)[0]
            elif var.type.name == "INT32":
                return struct.unpack('<i', raw_bytes)[0]
            elif var.type.name == "UINT32":
                return struct.unpack('<I', raw_bytes)[0]
            else:
                return None
        except Exception as e:
            print(f"Unpacking error for {var.type.name}: {e}")
            return None

    def get_variable_info(self, data_source_id: str) -> Dict[str, Any]:
        """Get information about a variable"""
        var = self.config.get_variable_by_id(data_source_id)
        if not var:
            return {}

        return {
            'id': var.id,
            'name': var.name,
            'description': var.description,
            'type': var.type.name,
            'signed': var.signed,
            'scale': var.scale,
            'unit': var.unit,
            'offset': var.offset,
            'field': var.field
        }