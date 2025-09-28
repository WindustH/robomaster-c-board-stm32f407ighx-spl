#!/usr/bin/env python3
"""
Compact STM32 development script for building, flashing, and debugging.
"""

import sys
import subprocess
import shutil
import argparse
import os
from pathlib import Path
from typing import List, Optional

# Configuration
PROJECT_NAME = "stm32f407ighx"
TARGET_CFG = "stm32f4x.cfg"
OPENOCD_INTERFACES = ["stlink.cfg", "cmsis-dap.cfg", "jlink.cfg", "stlink-v2.cfg", "stlink-v2-1.cfg"]
DEFAULT_BUILD_TYPE = "Release"


class Color:
    """ANSI color codes."""

    HEADER = "\033[95m"
    BLUE = "\033[94m"
    CYAN = "\033[96m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    RED = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"

    @classmethod
    def disable(cls):
        for attr in dir(cls):
            if not attr.startswith("__") and isinstance(getattr(cls, attr), str):
                setattr(cls, attr, "")


# Disable colors on non-TTY
if not sys.stdout.isatty() or os.name == "nt":
    Color.disable()


def print_header(message: str):
    print(f"\n{Color.BOLD}{Color.CYAN}=== {message} ==={Color.ENDC}\n")


def print_status(message: str, status: str = "info"):
    status_colors = {
        "success": Color.GREEN,
        "error": Color.RED,
        "warning": Color.YELLOW,
        "info": Color.BLUE,
    }
    color = status_colors.get(status.lower(), Color.BLUE)
    icons = {"success": "✅", "error": "❌", "warning": "⚠️", "info": "ℹ️"}
    icon = icons.get(status.lower(), "")
    print(f"{color}{icon} {message}{Color.ENDC}")


def print_command(command: List[str]):
    print(f"{Color.BLUE}▶ {Color.BOLD}{' '.join(command)}{Color.ENDC}")


class ProjectManager:
    """Manages STM32 project tasks."""

    def __init__(self, build_dir: str = "build"):
        self.project_dir = Path(__file__).parent.resolve()
        self.build_dir = self.project_dir / build_dir
        self.elf_file = self.build_dir / f"{PROJECT_NAME}.elf"
        self.toolchain_file = self.project_dir / "toolchain-arm-none-eabi.cmake"
        self.openocd_process = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.cleanup()

    def cleanup(self):
        if self.openocd_process and self.openocd_process.poll() is None:
            print_status("Terminating OpenOCD process...", "warning")
            self.openocd_process.terminate()
            try:
                self.openocd_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.openocd_process.kill()
            self.openocd_process = None

    def _run_command(
        self,
        command: List[str],
        cwd: Optional[Path] = None,
        check: bool = True,
        capture_output: bool = False,
    ):
        print_command(command)
        try:
            return subprocess.run(
                command,
                cwd=cwd or self.project_dir,
                check=check,
                stdout=sys.stdout if not capture_output else subprocess.PIPE,
                stderr=sys.stderr if not capture_output else subprocess.PIPE,
                text=True,
            )
        except FileNotFoundError as e:
            print_status(f"Command not found: {e.filename}", "error")
            sys.exit(1)
        except subprocess.CalledProcessError as e:
            print_status(f"Command failed with exit code {e.returncode}", "error")
            if check:
                sys.exit(1)
            raise

    def _find_interface(self) -> Optional[str]:
        """Detect connected programmer using lsusb, similar to dev.sh"""
        try:
            # Run lsusb to detect connected devices
            result = subprocess.run(["lsusb"], capture_output=True, text=True, check=True)
            lsusb_output = result.stdout

            # Check for specific device IDs, similar to dev.sh
            if "0483:3748" in lsusb_output:
                print_status("Detected ST-Link v2", "success")
                return "stlink-v2.cfg"
            elif "0483:374b" in lsusb_output:
                print_status("Detected ST-Link v2-1", "success")
                return "stlink-v2-1.cfg"
            elif "0d28:0204" in lsusb_output:
                print_status("Detected CMSIS-DAP", "success")
                return "cmsis-dap.cfg"
            else:
                # Fall back to the original method if no known device is found
                print_status("No known programmer detected via lsusb, trying OpenOCD test...", "warning")
                for interface in OPENOCD_INTERFACES:
                    test_cmd = ["openocd", "-f", f"interface/{interface}", "-c", "exit"]
                    try:
                        result = self._run_command(test_cmd, check=False, capture_output=True)
                        if result.returncode == 0:
                            print_status(f"Found interface: {interface}", "success")
                            return interface
                    except Exception:
                        continue
        except Exception as e:
            print_status(f"Failed to run lsusb: {e}", "warning")
            # Fall back to the original method
            for interface in OPENOCD_INTERFACES:
                test_cmd = ["openocd", "-f", f"interface/{interface}", "-c", "exit"]
                try:
                    result = self._run_command(test_cmd, check=False, capture_output=True)
                    if result.returncode == 0:
                        print_status(f"Found interface: {interface}", "success")
                        return interface
                except Exception:
                    continue
        return None

    def build(self, build_type: str = DEFAULT_BUILD_TYPE):
        print_header(f"Building Project ({build_type})")
        self.build_dir.mkdir(exist_ok=True)

        configure_cmd = [
            "cmake",
            "-S",
            self.project_dir.as_posix(),
            "-B",
            self.build_dir.as_posix(),
            f"-DCMAKE_TOOLCHAIN_FILE={self.toolchain_file.as_posix()}",
            f"-DCMAKE_BUILD_TYPE={build_type}",
            "-GNinja",
        ]
        self._run_command(configure_cmd)

        build_cmd = ["cmake", "--build", self.build_dir.as_posix(), "--parallel"]
        self._run_command(build_cmd)

        if not self.elf_file.exists():
            print_status("ELF file not generated!", "error")
            sys.exit(1)

        # Run keil converter to generate keil project files
        self._run_keil_converter()

        print_status("Build completed successfully", "success")

    def _run_keil_converter(self):
        """Run the Keil converter script to generate Keil project files."""
        print_header("Generating Keil Project Files")
        keil_converter_path = self.project_dir / "keil-convertor" / "to-keil.py"

        if not keil_converter_path.exists():
            print_status(f"Keil converter script not found at {keil_converter_path}", "warning")
            return

        try:
            # Run the keil converter script
            cmd = [sys.executable, keil_converter_path.as_posix()]
            self._run_command(cmd, cwd=keil_converter_path.parent)
            print_status("Keil project files generated successfully", "success")
        except subprocess.CalledProcessError:
            print_status("Failed to generate Keil project files", "error")
        except Exception as e:
            print_status(f"Error running Keil converter: {str(e)}", "error")

    def flash(self, interface: Optional[str] = None):
        if not self.elf_file.exists():
            print_status("ELF file not found. Building first...", "warning")
            self.build()

        print_header("Flashing Target")
        if interface is None:
            interface = self._find_interface()
            if interface is None:
                print_status("No compatible programmer found!", "error")
                sys.exit(1)

        flash_cmd = [
            "openocd",
            "-f",
            f"interface/{interface}",
            "-f",
            f"target/{TARGET_CFG}",
            "-c",
            f"program {self.elf_file.as_posix()} verify reset exit",
        ]

        try:
            self._run_command(flash_cmd)
            print_status("Device flashed successfully", "success")
        except subprocess.CalledProcessError:
            print_status("Flashing failed!", "error")
            sys.exit(1)

    def debug(self):
        print_header("Starting Debug Session")
        interface = self._find_interface()
        if interface is None:
            print_status("No compatible debug interface found!", "error")
            sys.exit(1)

        openocd_cmd = [
            "openocd",
            "-f",
            f"interface/{interface}",
            "-f",
            f"target/{TARGET_CFG}",
            "-c",
            "gdb_port 3333",
        ]

        try:
            self.openocd_process = subprocess.Popen(
                openocd_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )

            gdb_cmd = [
                "arm-none-eabi-gdb",
                "-ex",
                "target extended-remote localhost:3333",
                "-ex",
                "monitor reset halt",
                "-ex",
                "load",
                self.elf_file.as_posix(),
            ]
            self._run_command(gdb_cmd)

        except Exception as e:
            print_status(f"Debug session failed: {str(e)}", "error")
        finally:
            self.cleanup()

    def clean(self):
        print_header("Cleaning Project")
        to_remove = [
            self.build_dir,
            self.project_dir / "CMakeCache.txt",
            self.project_dir / "CMakeFiles",
        ]

        for path in to_remove:
            if path.exists():
                if path.is_dir():
                    shutil.rmtree(path)
                else:
                    path.unlink()
                print_status(f"Removed: {path}", "info")
        print_status("Clean completed", "success")

    def rebuild(self, build_type: str = DEFAULT_BUILD_TYPE):
        self.clean()
        self.build(build_type)


def check_dependencies():
    required_tools = ["cmake", "openocd", "arm-none-eabi-gcc", "arm-none-eabi-gdb"]
    missing_tools = [tool for tool in required_tools if shutil.which(tool) is None]

    if missing_tools:
        print_status(f"Missing tools: {', '.join(missing_tools)}", "error")
        print_status("Please install the missing tools.", "warning")
        return False
    return True


def main():
    if not check_dependencies():
        sys.exit(1)

    parser = argparse.ArgumentParser(description="STM32 Project Manager")
    parser.add_argument(
        "-v", "--version", action="version", version="STM32 Project Manager v1.0"
    )

    subparsers = parser.add_subparsers(dest="command", required=True, title="commands")

    # Build command
    build_parser = subparsers.add_parser("build", help="Build the project")
    build_parser.add_argument(
        "--debug", action="store_true", help="Build in Debug mode"
    )
    build_parser.add_argument(
        "--release", action="store_true", help="Build in Release mode"
    )
    build_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Build-release command
    build_release_parser = subparsers.add_parser("build-release", help="Build in Release mode")
    build_release_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Build-debug command
    build_debug_parser = subparsers.add_parser("build-debug", help="Build in Debug mode")
    build_debug_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Flash command
    flash_parser = subparsers.add_parser("flash", help="Flash the device")
    flash_parser.add_argument(
        "--interface", choices=OPENOCD_INTERFACES, help="Programmer interface"
    )
    flash_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Debug command
    debug_parser = subparsers.add_parser("debug", help="Start debug session")
    debug_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Clean command
    clean_parser = subparsers.add_parser("clean", help="Clean build artifacts")
    clean_parser.add_argument("--build-dir", default="build", help="Build directory")

    # Rebuild command
    rebuild_parser = subparsers.add_parser("rebuild", help="Clean and rebuild")
    rebuild_parser.add_argument(
        "--debug", action="store_true", help="Rebuild in Debug mode"
    )
    rebuild_parser.add_argument(
        "--release", action="store_true", help="Rebuild in Release mode"
    )
    rebuild_parser.add_argument("--build-dir", default="build", help="Build directory")

    args = parser.parse_args()
    build_dir = getattr(args, "build_dir", "build")

    with ProjectManager(build_dir) as manager:
        try:
            if args.command == "build":
                build_type = (
                    "Debug"
                    if args.debug
                    else ("Release" if args.release else DEFAULT_BUILD_TYPE)
                )
                manager.build(build_type)
            elif args.command == "build-release":
                manager.build("Release")
            elif args.command == "build-debug":
                manager.build("Debug")
            elif args.command == "flash":
                manager.flash(args.interface)
            elif args.command == "debug":
                manager.debug()
            elif args.command == "clean":
                manager.clean()
            elif args.command == "rebuild":
                build_type = (
                    "Debug"
                    if args.debug
                    else ("Release" if args.release else DEFAULT_BUILD_TYPE)
                )
                manager.rebuild(build_type)
        except KeyboardInterrupt:
            print_status("\nOperation cancelled by user", "warning")
            sys.exit(130)
        except Exception as e:
            print_status(f"Error: {str(e)}", "error")
            sys.exit(1)


if __name__ == "__main__":
    main()
