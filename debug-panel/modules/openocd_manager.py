import subprocess
import os
import signal
import socket
import time
from typing import Optional


class OpenOCDManager:
    def __init__(self, config_file: Optional[str] = None, interface: str = "cmsis-dap", target: str = "stm32f4x"):
        self.config_file = config_file
        self.interface = interface
        self.target = target
        self.process = None
        self.host = "localhost"
        self.port = 4444

    def start(self) -> bool:
        """Start OpenOCD process"""
        try:
            # Check if OpenOCD is already running
            if self.is_running():
                print("OpenOCD is already running")
                return True

            # Build OpenOCD command
            cmd = ["openocd"]
            if self.config_file:
                cmd.extend(["-f", self.config_file])
            else:
                # Use default configuration
                cmd.extend(["-f", f"interface/{self.interface}.cfg"])
                cmd.extend(["-f", f"target/{self.target}.cfg"])

            print(f"Starting OpenOCD: {' '.join(cmd)}")
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid
            )

            # Wait for OpenOCD to start
            time.sleep(2)

            # Check if process is still running
            if self.process.poll() is not None:
                stdout, stderr = self.process.communicate()
                print(f"OpenOCD failed to start: {stderr}")
                return False

            print("OpenOCD started successfully")
            return True

        except Exception as e:
            print(f"Failed to start OpenOCD: {e}")
            return False

    def stop(self):
        """Stop OpenOCD process"""
        if self.process:
            try:
                # Send SIGTERM to the process group
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
                self.process.wait(timeout=5)
                print("OpenOCD stopped")
            except subprocess.TimeoutExpired:
                # Force kill if it doesn't terminate gracefully
                os.killpg(os.getpgid(self.process.pid), signal.SIGKILL)
                print("OpenOCD force stopped")
            except Exception as e:
                print(f"Error stopping OpenOCD: {e}")
            finally:
                self.process = None

    def is_running(self) -> bool:
        """Check if OpenOCD is running and accepting connections"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex((self.host, self.port))
            sock.close()
            return result == 0
        except:
            return False

    def wait_for_connection(self, timeout: int = 10) -> bool:
        """Wait for OpenOCD to be ready"""
        start_time = time.time()
        while time.time() - start_time < timeout:
            if self.is_running():
                return True
            time.sleep(0.5)
        return False