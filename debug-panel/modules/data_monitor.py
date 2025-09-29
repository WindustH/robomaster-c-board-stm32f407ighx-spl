import time
import threading
import struct
from typing import Dict, Any

from PyQt5.QtCore import QObject, pyqtSignal

from modules.config import MonitorConfig, VariableConfig, VariableType
from modules.map_parser import MapParser
from modules.openocd_interface import OpenOCDInterface


class DataMonitor(QObject):
    data_updated = pyqtSignal(dict)
    connection_status = pyqtSignal(bool)

    def __init__(self, config: MonitorConfig, parser: MapParser):
        super().__init__()
        self.config = config
        self.parser = parser
        self.openocd = OpenOCDInterface(config.openocd_host, config.openocd_port)
        self.running = False
        self.read_address = None
        self.write_address = None

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

    def write_variable(self, var_config: VariableConfig, value):
        """Write value to variable"""
        if not self.write_address or not self.openocd.connected:
            return False

        try:
            # Get current offset for this variable
            offsets = self.config.calculate_motor_offsets(self.config.motor_to_monitor)
            write_mapping = {
                'PID Target': 'write_pid_target',
                'PID Kp': 'write_pid_kp',
                'PID Ki': 'write_pid_ki',
                'PID Kd': 'write_pid_kd',
                'PID Output Limit': 'write_pid_output_limit',
                'PID Mode': 'write_pid_mode'
            }

            if var_config.name not in write_mapping:
                return False

            offset = offsets[write_mapping[var_config.name]]

            if var_config.type == VariableType.FLOAT:
                data = struct.pack('<f', float(value))
            elif var_config.type == VariableType.DOUBLE:
                data = struct.pack('<d', float(value))
            elif var_config.type in [VariableType.INT8, VariableType.UINT8]:
                fmt = '<B' if not var_config.signed else '<b'
                data = struct.pack(fmt, int(value))
            elif var_config.type in [VariableType.INT16, VariableType.UINT16]:
                fmt = '<H' if not var_config.signed else '<h'
                data = struct.pack(fmt, int(value))
            elif var_config.type in [VariableType.INT32, VariableType.UINT32]:
                fmt = '<I' if not var_config.signed else '<i'
                data = struct.pack(fmt, int(value))
            else:
                return False

            self.openocd.write_memory(self.write_address + offset, data)
            return True
        except Exception as e:
            print(f"Write error: {e}")
            return False

    def _monitor_loop(self):
        """Main monitoring loop"""
        while self.running:
            try:
                # Calculate current offsets for all read variables
                offsets = self.config.calculate_motor_offsets(self.config.motor_to_monitor)

                # Calculate maximum offset needed for reading
                max_offset = max((offsets.get(var.name, 0) + var.type.value for var in self.config.read_variables), default=0)
                if max_offset == 0 or self.read_address is None:
                    time.sleep(0.1)
                    continue

                data = self.openocd.read_memory(self.read_address, max_offset)
                parsed_data = {}
                for var in self.config.read_variables:
                    try:
                        offset = offsets.get(var.name, 0)
                        if offset + var.type.value <= len(data):
                            raw_bytes = data[offset:offset + var.type.value]
                            if var.type == VariableType.FLOAT:
                                value = struct.unpack('<f', raw_bytes)[0]
                            elif var.type == VariableType.DOUBLE:
                                value = struct.unpack('<d', raw_bytes)[0]
                            elif var.type == VariableType.INT8:
                                value = struct.unpack('<b', raw_bytes)[0]
                            elif var.type == VariableType.UINT8:
                                value = struct.unpack('<B', raw_bytes)[0]
                            elif var.type == VariableType.INT16:
                                value = struct.unpack('<h', raw_bytes)[0]
                            elif var.type == VariableType.UINT16:
                                value = struct.unpack('<H', raw_bytes)[0]
                            elif var.type == VariableType.INT32:
                                value = struct.unpack('<i', raw_bytes)[0]
                            elif var.type == VariableType.UINT32:
                                value = struct.unpack('<I', raw_bytes)[0]
                            else:
                                continue

                            value = value * var.scale
                            parsed_data[var.name] = {
                                'value': value,
                                'unit': var.unit,
                                'timestamp': time.time()
                            }
                    except Exception as e:
                        print(f"Parse error for {var.name}: {e}")

                self.data_updated.emit(parsed_data)
                time.sleep(self.config.update_interval_ms / 1000.0)

            except Exception as e:
                print(f"Monitor loop error: {e}")
                time.sleep(0.5)