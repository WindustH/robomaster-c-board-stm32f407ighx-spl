#!/usr/bin/env python3
"""
STM32 Motor Monitor - Debug Panel

A PyQt5-based application for monitoring and controlling STM32 motor control systems
via OpenOCD and GDB server.
"""

import sys

from modules.gui import MotorMonitorGUI
from PyQt5.QtWidgets import QApplication


def main():
    """Main application entry point"""
    app = QApplication(sys.argv)
    app.setStyle('Fusion')

    window = MotorMonitorGUI()
    window.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()