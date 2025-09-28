import socket
import struct
from typing import Optional


class OpenOCDInterface:
    def __init__(self, host: str = "localhost", port: int = 4444):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False

    def connect(self) -> bool:
        """Connect to OpenOCD server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            return True
        except Exception as e:
            print(f"Failed to connect to OpenOCD: {e}")
            return False

    def disconnect(self):
        """Disconnect from OpenOCD server"""
        if self.socket:
            self.socket.close()
        self.connected = False

    def send_command(self, command: str) -> str:
        """Send command to OpenOCD and get response"""
        if not self.connected or self.socket is None:
            return ""

        try:
            self.socket.send(f"{command}\n".encode())
            response = self.socket.recv(4096).decode()
            return response.strip()
        except Exception as e:
            print(f"OpenOCD command error: {e}")
            return ""

    def read_memory(self, address: int, size: int) -> bytes:
        """Read memory from target"""
        command = f"mdh 0x{address:08x} {size//2}"
        response = self.send_command(command)

        data = bytearray()
        try:
            lines = response.split('\n')
            for line in lines:
                if ':' in line:
                    hex_values = line.split(':')[1].strip().split()
                    for hex_val in hex_values:
                        if hex_val.startswith('0x'):
                            hex_val = hex_val[2:]
                        val = int(hex_val, 16)
                        data.extend(struct.pack('<H', val))
        except Exception as e:
            print(f"Memory read parsing error: {e}")

        return bytes(data[:size])

    def write_memory(self, address: int, data: bytes):
        """Write memory to target"""
        for i in range(0, len(data), 4):
            chunk = data[i:i+4]
            if len(chunk) < 4:
                chunk += b'\x00' * (4 - len(chunk))
            value = struct.unpack('<I', chunk)[0]
            command = f"mww 0x{address + i:08x} 0x{value:08x}"
            self.send_command(command)