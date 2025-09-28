#!/usr/bin/env python3
"""
Helper script to easily change which motor to monitor in the debug panel.

Usage:
    python change_motor.py <motor_id>

Example:
    python change_motor.py 0    # Monitor motor 0
    python change_motor.py 3    # Monitor motor 3
    python change_motor.py 7    # Monitor motor 7
"""

import json
import sys
import os

# Structure sizes
MOTOR_STATUS_SIZE = 8  # sizeof(motStat) - i16(2) + i16(2) + i16(2) + u8(1) + padding(1) = 8 bytes
PID_STATUS_SIZE = 28   # sizeof(pidStat) - 7 floats (4 bytes each) + u8(1) + padding(3) = 28 bytes

def calculate_offsets(motor_id):
    """Calculate offsets for the specified motor ID"""

    # Motor status offsets (monitor_read_data.motors[motor_id])
    motor_base_offset = motor_id * MOTOR_STATUS_SIZE

    # PID status offsets (monitor_read_data.pids[motor_id])
    pid_base_offset = 64 + (motor_id * PID_STATUS_SIZE)  # 64 = sizeof(motStat[8])

    # Write variable offsets (monitor_write_data)
    write_pid_target_offset = motor_id * 4  # 4 bytes per float
    write_pid_enable_offset = 32 + motor_id  # 1 byte per bool
    write_motor_current_offset = 64 + (motor_id * 2)  # 2 bytes per int16

    return {
        'motor_current': motor_base_offset + 0,
        'motor_velocity': motor_base_offset + 2,
        'motor_position': motor_base_offset + 4,
        'motor_temperature': motor_base_offset + 6,

        'pid_kp': pid_base_offset + 0,
        'pid_ki': pid_base_offset + 4,
        'pid_kd': pid_base_offset + 8,
        'pid_target': pid_base_offset + 28,  # target is at offset 28 in pidStat
        'pid_enabled': pid_base_offset + 56,  # enabled is at offset 56 in pidStat

        'write_pid_target': write_pid_target_offset,
        'write_pid_enable': write_pid_enable_offset,
        'write_motor_current': write_motor_current_offset
    }

def update_config(motor_id):
    """Update the config.json file with new motor offsets"""

    config_path = 'config.json'

    if not os.path.exists(config_path):
        print(f"Error: {config_path} not found!")
        return False

    # Load current config
    with open(config_path, 'r') as f:
        config = json.load(f)

    # Calculate new offsets
    offsets = calculate_offsets(motor_id)

    # Update motor_to_monitor
    config['motor_to_monitor'] = motor_id

    # Update read variables
    for var in config['read_variables']:
        if var['name'] in offsets:
            var['offset'] = offsets[var['name']]
            # Update graph titles
            if var['name'].startswith('motor_'):
                var['graph']['title'] = var['graph']['title'].replace('Motor ', f'Motor {motor_id} ')
            elif var['name'].startswith('pid_'):
                var['graph']['title'] = var['graph']['title'].replace('PID ', f'Motor {motor_id} PID ')

    # Update write variables
    write_mapping = {
        'pid_target': 'write_pid_target',
        'pid_enable': 'write_pid_enable',
        'motor_current': 'write_motor_current'
    }

    for var in config['write_variables']:
        if var['name'] in write_mapping:
            var['offset'] = offsets[write_mapping[var['name']]]

    # Save updated config
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)

    return True

def main():
    if len(sys.argv) != 2:
        print("Usage: python change_motor.py <motor_id>")
        print("motor_id: 0-7 (motor index to monitor)")
        sys.exit(1)

    try:
        motor_id = int(sys.argv[1])
        if motor_id < 0 or motor_id > 7:
            print("Error: motor_id must be between 0 and 7")
            sys.exit(1)
    except ValueError:
        print("Error: motor_id must be a number between 0 and 7")
        sys.exit(1)

    print(f"Changing monitored motor to motor {motor_id}...")

    if update_config(motor_id):
        print(f"✓ Successfully updated config.json to monitor motor {motor_id}")
        print("\nUpdated offsets:")
        offsets = calculate_offsets(motor_id)
        for name, offset in offsets.items():
            print(f"  {name}: {offset}")
    else:
        print("✗ Failed to update config.json")
        sys.exit(1)

if __name__ == '__main__':
    main()