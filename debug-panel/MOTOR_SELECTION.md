# Motor Selection Helper

This directory contains a helper script to easily change which motor you want to monitor in the debug panel.

## Quick Start

To change which motor to monitor, run:

```bash
python change_motor.py <motor_id>
```

Where `<motor_id>` is a number from 0 to 7.

## Examples

```bash
# Monitor motor 0
python change_motor.py 0

# Monitor motor 3
python change_motor.py 3

# Monitor motor 7
python change_motor.py 7
```

## What This Does

- Updates `config.json` with the correct memory offsets for the selected motor
- Updates graph titles to show the correct motor number
- Sets the `motor_to_monitor` field in the config
- All changes are automatically applied to the debug panel

## Structure Layout

The monitor module exposes:
- **Motor status**: Current, velocity, position, temperature for all 8 motors
- **PID status**: KP, KI, KD, target, enabled flags for all 8 motors
- **Control**: PID targets, enable flags, direct current control

The script automatically calculates the correct memory offsets based on the structure sizes:
- `motStat`: 8 bytes per motor
- `pidStat`: 28 bytes per motor

## Notes

- Motor IDs are 0-indexed (0 = motor 1, 1 = motor 2, etc.)
- The system supports monitoring all 8 motors simultaneously
- The debug panel focuses on one motor for easier visualization
- You can switch motors at any time without restarting the debug panel