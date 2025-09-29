# STM32F407IGHX Development Documentation

## Project Overview

This is an STM32F407IGHX motor control project with a modular architecture, featuring:
- Real-time motor control with PID controllers
- CAN bus communication for motor control
- UART communication (DMA and interrupt-based)
- Comprehensive debugging and monitoring system
- Cross-platform development tools

## Project Structure

```
stm32f407ighx/
├── src/                    # Application source code
│   ├── main.c             # Main application entry point
│   ├── def.h              # Global definitions and constants
│   ├── type.h             # Type definitions and structures
│   ├── type.c             # Type implementations
│   ├── app/               # Application layer modules
│   │   ├── mon.c          # Monitor module
│   │   ├── pid.c          # PID controller module
│   │   └── tick.c         # System tick module
│   ├── bsp/               # Board Support Package
│   │   ├── can.c          # CAN bus driver
│   │   ├── clock.c        # Clock configuration
│   │   ├── cron.c         # Scheduler/cron jobs
│   │   ├── led.c          # LED control
│   │   ├── motor.c        # Motor control interface
│   │   ├── uart.c         # UART base
│   │   ├── uart_dma.c     # UART with DMA
│   │   └── uart_it.c      # UART with interrupts
│   ├── mod/               # Module interface definitions
│   │   ├── app.h          # Application module interface
│   │   ├── app.c          # Application module implementation
│   │   ├── bsp.h          # BSP module interface
│   │   ├── bsp.c          # BSP module implementation
│   │   ├── err.h          # Error handling interface
│   │   └── err.c          # Error handling implementation
│   ├── err/               # Error handling
│   │   └── status_led.c   # Status LED error indication
│   └── utils/             # Utility functions
├── debug-panel/           # Python debug panel application
│   ├── main.py            # Debug panel entry point
│   ├── config.json        # Debug panel configuration
│   ├── modules/           # Debug panel modules
│   │   ├── config.py      # Configuration management
│   │   ├── data_monitor.py # Data monitoring logic
│   │   ├── gui.py         # GUI implementation
│   │   ├── map_parser.py  # Memory map parsing
│   │   ├── openocd_interface.py # OpenOCD communication
│   │   └── openocd_manager.py   # OpenOCD process management
│   └── change_motor.py    # Motor control script
├── core/                  # STM32 CMSIS and HAL libraries
├── startup/               # ARM startup files
├── keil-convertor/        # Keil project generator
├── build/                 # Build output directory
├── dev.py                 # Main development script
├── archive.py             # Project archiving tool
└── CMakeLists.txt         # CMake build configuration
```

## Code Style and Conventions

### Naming Conventions

- **Types**: Use lowercase abbreviations without `_t` suffix for typedefs
  ```c
  typedef struct { ... } motStat;    // Motor status
  typedef enum { ... } pidMode;      // PID mode
  typedef struct { ... } monRead;    // Monitor read structure
  typedef struct { ... } monWrite;   // Monitor write structure
  ```

- **Variables**: Use snake_case for variables and functions
  ```c
  uint32_t motor_speed;
  void update_motor_status(void);
  ```

- **Constants**: Use UPPER_CASE for macros and constants
  ```c
  #define MOTOR_COUNT 8
  #define MAX_SPEED 5000
  ```

- **Module Prefix**: Use underscore prefix for module structs
  ```c
  typedef struct { ... } _MotorMod;
  extern const _MotorMod _motor;
  ```

### File Organization

- **Type Definitions**: All type definitions go in `src/type.h`
- **Constants**: Global constants and macros in `src/def.h`
- **Module Architecture**: Each module has:
  - Interface definition in `src/mod/[module].h`
  - Implementation in `src/[category]/[module].c`
  - Module struct with function pointers

### Module Packaging Style

Modules are organized using a consistent pattern:

1. **Module Interface Structure**:
   ```c
   typedef struct {
       void (*setup)(void);
       return_type (*function_name)(parameters);
       // ... other function pointers
   } _ModuleNameMod;

   extern const _ModuleNameMod _module_name;
   ```

2. **Module Implementation**:
   - Implement all functions defined in the interface
   - Define the module instance with function pointers
   - Use static functions for internal implementation

3. **Module Usage**:
   ```c
   // Access through module interface
   _motor.setup();
   _motor.set(0, 1000);
   ```

### Type System

All custom types are defined in `src/type.h`:

- **Basic Types**: Standardized type aliases
  ```c
  typedef float f32;
  typedef double f64;
  typedef uint32_t u32;
  typedef uint8_t u8;
  // ... etc
  ```

- **Structures**: Well-defined data structures using lowercase abbreviations
  ```c
  typedef struct {
      i16 i;  // Current
      i16 v;  // Velocity
      i16 th; // Position
      u8 T;   // Temperature
  } motStat;

  typedef struct {
      f32 kp;           // Proportional gain
      f32 ki;           // Integral gain
      f32 kd;           // Derivative gain
      f32 dt;           // Time step
      f32 output_limit; // Output saturation limit
      pidMode mode;     // Control mode
      f32 integral;     // Integral accumulator
      f32 prev_error;   // Previous error
      f32 target;       // Target value
      u8 enabled;       // Controller enabled flag
  } pidStat;

  typedef struct {
      motStat motors[8];  // Motor status for all 8 motors
      pidStat pids[8];    // PID status for all 8 motors
      u32 timestamp;      // System timestamp
  } monRead;

  typedef struct {
      f32 pid_targets[8];        // PID target values
      u8 pid_enabled[8];         // PID enable/disable flags
      f32 pid_kp[8];             // PID proportional gains
      f32 pid_ki[8];             // PID integral gains
      f32 pid_kd[8];             // PID derivative gains
      f32 pid_output_limit[8];   // PID output limits
      pidMode pid_mode[8];       // PID control modes
      u8 use_pid_control;        // PID control flag
  } monWrite;
  ```

- **Function Pointers**: For callback and module interfaces
  ```c
  typedef void (*proc)(void);
  ```

## Module Documentation

### Application Modules (`src/mod/app.h`)

#### PID Module (`_PidMod`)
- **setup()**: Initialize PID controller with parameters
- **update()**: Update PID calculation
- **reset()**: Reset PID state
- **enable()/disable()**: Control PID operation
- **set_target()**: Set target value
- **set_kp()/set_ki()/set_kd()**: Tune PID parameters
- **status()**: Get current PID status

#### Tick Module (`_TickMod`)
- **update()**: Update system tick
- **get()**: Get current tick count

#### Monitor Module (`_MonMod`)
- **update()**: Update monitor data
- **read**: Pointer to `monRead` structure
- **write**: Pointer to `monWrite` structure

#### Application Module (`_AppMod`)
- **init()**: Initialize all application modules
- **pid**: PID module interface
- **mon**: Monitor module interface
- **tick**: Tick module interface

**Usage Example:**
```c
app.init();                    // Initialize all application modules
app.pid.setup(0, 1.0, 0.1, 0.01, 0.001, 1000, PID_VELOCITY);
app.pid.enable(0);
app.pid.set_target(0, 1000.0);
app.tick.get();               // Get current system tick
app.mon.update();             // Update monitor data
```

### BSP Modules (`src/mod/bsp.h`)

#### UART DMA Module (`_UartDmaMod`)
- **setup()**: Initialize UART with DMA
- **send_dat()**: Send data buffer
- **send_str()**: Send string
- **bind_rx_callback()**: Set receive callback
- **unbind_rx_callback()**: Remove receive callback

**Usage Example:**
```c
volatile buf uart_rx_buf;
_uart_dma.setup(&uart_rx_buf);
_uart_dma.bind_rx_callback(my_rx_handler);
_uart_dma.send_str("Hello World");
```

#### UART IT Module (`_UartItMod`)
- **setup()**: Initialize UART with interrupts
- **send_byte()**: Send single byte
- **send_str()**: Send string
- **has_new_byte()**: Check for received data
- **read_byte()**: Read received byte

**Usage Example:**
```c
_uart_it.setup();
_uart_it.send_str("Hello");
if (_uart_it.has_new_byte()) {
    u8 data = _uart_it.read_byte();
}
```

#### UART Module (`_UartMod`)
- **setup()**: Initialize UART base configuration
- **dma**: DMA-based UART interface
- **it**: Interrupt-based UART interface

**Usage Example:**
```c
bsp.uart.setup();
bsp.uart.dma.setup(&uart_rx_buf);
bsp.uart.it.setup();
```

#### LED Module (`_LedMod`)
- **setup()**: Initialize LED
- **show()**: Set LED color/state

**Usage Example:**
```c
_led.setup();
_led.show(0x00FF0000);  // Red
_led.show(0x0000FF00);  // Green
_led.show(0x000000FF);  // Blue
```

#### Clock Module (`_ClockMod`)
- **setup()**: Configure system clock

**Usage Example:**
```c
_clock.setup();  // Configures system clock to 168MHz
```

#### Cron Module (`_CronMod`)
- **setup()**: Initialize scheduler
- **add_job()**: Add periodic job
- **remove_job()**: Remove job

**Usage Example:**
```c
_cron.setup();
u8 job_id = _cron.add_job(my_periodic_function);
_cron.remove_job(job_id);
```

#### CAN Module (`_CanMod`)
- **setup()**: Initialize CAN bus
- **send()**: Send CAN message
- **read()**: Read CAN message
- **bind_rx_callback()**: Set receive callback
- **unbind_rx_callback()**: Remove receive callback

**Usage Example:**
```c
_can.setup();
_can.bind_rx_callback(my_can_handler);

canTxH header = {.StdId = 0x123, .DLC = 8};
u8 data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
_can.send(&header, data, NULL);
```

#### Motor Module (`_MotorMod`)
- **setup()**: Initialize motor control
- **set()**: Set motor current
- **status()**: Get motor status
- **send_ctrl_signal()**: Send control signals
- **update_status()**: Update motor status from CAN

**Usage Example:**
```c
_motor.setup();
_motor.set(0, 1000);  // Set motor 0 to 1000mA
motStat status = _motor.status(0);
_motor.send_ctrl_signal();  // Send control signals to all motors
```

#### Error Module (`_ErrMod`)
- **setup()**: Initialize error handling system

**Usage Example:**
```c
_err.setup();  // Initialize error handling (status LED, etc.)
```

#### BSP Module (`_BspMod`)
- **init()**: Initialize all BSP modules
- **uart**: UART module interface
- **led**: LED module interface
- **clock**: Clock module interface
- **cron**: Cron module interface
- **can**: CAN module interface
- **motor**: Motor module interface

**Usage Example:**
```c
bsp.init();                    // Initialize all BSP modules
bsp.clock.setup();             // Configure system clock
bsp.led.setup();               // Initialize LED
bsp.cron.setup();              // Initialize scheduler
bsp.can.setup();               // Initialize CAN bus
bsp.motor.setup();             // Initialize motor control
bsp.can.bind_rx_callback(bsp.motor.update_status);  // Bind motor status update

// Add periodic jobs
bsp.cron.add_job(app.tick.update);
bsp.cron.add_job(bsp.motor.send_ctrl_signal);
bsp.cron.add_job(app.mon.update);
```

### Module Initialization Sequence

**Complete System Initialization:**
```c
int main() {
  // 1. Initialize BSP modules
  bsp.init();

  // 2. Configure system clock
  bsp.clock.setup();

  // 3. Initialize peripherals
  bsp.led.setup();
  bsp.cron.setup();
  bsp.can.setup();
  bsp.motor.setup();

  // 4. Set up communication callbacks
  bsp.can.bind_rx_callback(bsp.motor.update_status);

  // 5. Initialize application modules
  app.init();

  // 6. Configure initial motor settings
  bsp.motor.set(6, 1000);  // Set motor 6 to 1000mA
  bsp.led.show(0xffffffff);  // Turn on LED

  // 7. Add periodic tasks to scheduler
  bsp.cron.add_job(app.tick.update);
  bsp.cron.add_job(bsp.motor.send_ctrl_signal);
  bsp.cron.add_job(app.mon.update);

  // 8. Main loop
  while (true) {
    // System idle or background tasks
  }
}
```

### Module Dependencies

- **BSP Modules** depend on hardware initialization
- **Application Modules** depend on BSP modules being initialized
- **CAN Module** provides data to **Motor Module** via callback
- **Cron Module** schedules periodic execution of other modules
- **Monitor Module** reads data from **Motor** and **PID** modules

## Development Tools

### dev.py - Main Development Script

A comprehensive Python script for building, flashing, and debugging:

```bash
# Build project
python dev.py build

# Build with specific type
python dev.py build --debug
python dev.py build --release

# Flash to device
python dev.py flash

# Start debug session
python dev.py debug

# Clean build artifacts
python dev.py clean

# Rebuild project
python dev.py rebuild
```

**Features:**
- Automatic programmer detection (ST-Link, CMSIS-DAP, J-Link)
- CMake-based build system
- OpenOCD integration for flashing and debugging
- Keil project file generation
- Color-coded output and status reporting

### Debug Panel

A PyQt5-based application for real-time monitoring and control:

```bash
# Run debug panel
cd debug-panel
python main.py
```

**Features:**
- Real-time data monitoring via OpenOCD
- Configurable variable mapping
- Live graphing of motor parameters
- PID parameter tuning
- Motor control interface
- Memory map parsing

**Debug Panel Modules:**

#### GUI Module (`modules/gui.py`)
- **MotorMonitorGUI**: Main GUI class for debug panel
- Real-time data display and control interface
- Graph plotting for motor parameters
- PID parameter tuning interface

#### Data Monitor Module (`modules/data_monitor.py`)
- **DataMonitor**: Handles data acquisition and processing
- OpenOCD communication for memory reading/writing
- Data structure parsing and validation
- Real-time data buffering

#### Config Module (`modules/config.py`)
- **MonitorConfig**: Configuration management
- Variable mapping and type definitions
- Graph configuration and styling
- OpenOCD connection settings

#### Map Parser Module (`modules/map_parser.py`)
- Memory map parsing from firmware.map
- Symbol address resolution
- Structure member offset calculation

#### OpenOCD Interface (`modules/openocd_interface.py`)
- **OpenOCDInterface**: Low-level OpenOCD communication
- TCL command execution
- Memory read/write operations
- Connection management

#### OpenOCD Manager (`modules/openocd_manager.py`)
- **OpenOCDManager**: Process management for OpenOCD
- Automatic OpenOCD startup/shutdown
- Interface detection and configuration
- Error handling and recovery

**Configuration:**
- Edit `debug-panel/config.json` for custom variable mapping
- Configure OpenOCD connection settings
- Define read/write variables and their memory offsets

**Usage Example:**
```bash
# Start debug panel
cd debug-panel
python main.py

# The panel will automatically:
# 1. Load configuration from config.json
# 2. Start OpenOCD if auto_start_openocd is enabled
# 3. Connect to target device
# 4. Begin real-time monitoring of configured variables
```

### Motor Configuration Tool (`change_motor.py`)

Helper script to easily change which motor to monitor in the debug panel:

```bash
# Change monitored motor
cd debug-panel
python change_motor.py <motor_id>

# Examples
python change_motor.py 0    # Monitor motor 0
python change_motor.py 3    # Monitor motor 3
python change_motor.py 7    # Monitor motor 7
```

**Features:**
- Automatically calculates memory offsets for specified motor
- Updates config.json with new motor monitoring configuration
- Updates graph titles to reflect current motor
- Handles all motor-related variables (current, velocity, position, temperature, PID parameters)

**What it updates:**
- `motor_to_monitor` configuration field
- Memory offsets for all motor-related variables
- Graph titles to show current motor number
- PID parameter offsets for read/write operations

**Usage Example:**
```bash
cd debug-panel
python change_motor.py 2
# Output:
# ✓ Successfully updated config.json to monitor motor 2
# Updated offsets:
#   motor_current: 16
#   motor_velocity: 18
#   motor_position: 20
#   motor_temperature: 22
#   ...
```

### Archive Tool (`archive.py`)

Create project archives at specific commit states:

```bash
# Create archive
python archive.py

# Create archive with custom output path
python archive.py --output /path/to/archive
```

### Keil Converter (`keil-convertor/to-keil.py`)

Generate Keil MDK project files from CMake configuration:

```bash
# Generate Keil project files
cd keil-convertor
python to-keil.py
```

## Build System

### CMake Configuration

- **Toolchain**: `toolchain-arm-none-eabi.cmake`
- **Build Types**: Debug, Release, MinSizeRel, RelWithDebInfo
- **Compiler**: arm-none-eabi-gcc
- **Linker**: Custom linker script `STM32F407IGHX_FLASH.ld`

### Build Process

1. **Configure**:
   ```bash
   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-none-eabi.cmake
   ```

2. **Build**:
   ```bash
   cmake --build build --parallel
   ```

3. **Output**: `build/stm32f407ighx.elf`

### Dependencies

Required tools:
- CMake (>= 3.16)
- arm-none-eabi toolchain
- OpenOCD
- Python 3.8+
- Ninja (recommended)

## Debugging and Monitoring

### Memory Layout

- **Read Structure**: `monitor_read_t` - System state data
- **Write Structure**: `monitor_write_t` - Control parameters
- **Memory Map**: Parsed from firmware.map file

### OpenOCD Integration

- **Port**: 4444 (TCL), 3333 (GDB)
- **Interfaces**: ST-Link, CMSIS-DAP, J-Link
- **Target**: stm32f4x

### GDB Debugging

```bash
# Start debug session
arm-none-eabi-gdb build/stm32f407ighx.elf
(gdb) target extended-remote localhost:3333
(gdb) load
(gdb) monitor reset halt
```

## Motor Control System

### Supported Motors
- Up to 8 motors via CAN bus
- Real-time PID control
- Current, velocity, and position control modes
- Temperature monitoring

### Communication Protocol
- CAN bus for motor communication
- UART for debug and monitoring
- Custom protocol for motor status and control

## Development Workflow

### Typical Development Cycle

1. **Code Changes**: Modify source files in `src/`
2. **Build**: `python dev.py build`
3. **Flash**: `python dev.py flash`
4. **Monitor**: Run debug panel for real-time monitoring
5. **Debug**: Use `python dev.py debug` for GDB session

### Testing

- Use debug panel for real-time parameter monitoring
- Monitor motor status and PID performance
- Test communication protocols via UART and CAN
- Verify system timing with tick module

### Troubleshooting

- Check programmer connection with `lsusb`
- Verify OpenOCD configuration
- Monitor system logs in debug panel
- Use status LED for error indication

## Configuration Files

### def.h - System Configuration
- DMA buffer sizes
- Clock frequencies
- Timer configurations
- System constants

### type.h - Data Structures
- Motor status structures
- PID controller parameters
- Communication protocols
- Monitor data structures

### debug-panel/config.json
- OpenOCD connection settings
- Variable mapping
- Graph configurations
- Update intervals

## Best Practices

### Code Organization
- Keep module interfaces clean and focused
- Use consistent naming conventions
- Document all public interfaces
- Separate concerns between BSP and application layers

### Memory Management
- Use DMA for high-speed communication
- Minimize dynamic memory allocation
- Use fixed-size buffers where possible
- Monitor stack usage

### Real-time Considerations
- Keep interrupt handlers short
- Use DMA for data transfer
- Implement proper synchronization
- Monitor system timing

### Error Handling
- Use status LED for visual feedback
- Implement proper error codes
- Log errors via UART when possible
- Graceful degradation where appropriate

## Development Status and TODO

### ⚠️ Documentation Maintenance
**IMPORTANT: Update this documentation after significant development changes**

After making changes to:
- Module interfaces or functionality
- Type definitions
- Development tools
- Project structure

Please update this `dev.md` file to reflect the current state of the project.

### 🔧 Modules Under Development

#### Error Module (`_ErrMod`)
**Status**: ⚠️ **INCOMPLETE / UNDER DEVELOPMENT**

**Current Implementation:**
- Only has basic `setup()` function
- No error handling logic implemented
- Status LED error indication exists but not integrated

**TODO:**
- [ ] Implement comprehensive error detection
- [ ] Add error logging via UART
- [ ] Integrate status LED error codes
- [ ] Add error recovery mechanisms
- [ ] Implement error callback system
- [ ] Add error severity levels
- [ ] Create error reporting interface

#### UART Modules
**Status**: ⚠️ **PARTIALLY IMPLEMENTED**

**TODO:**
- [ ] Complete UART DMA receive buffer daemon
- [ ] Implement UART command handling
- [ ] Add UART data validation
- [ ] Implement flow control

#### Application Command Handler
**Status**: ⚠️ **NOT IMPLEMENTED**

**TODO:**
- [ ] Implement command parsing for UART communication
- [ ] Add command response system
- [ ] Create command validation
- [ ] Implement command history

### 🚧 Future Development Areas

#### Enhanced Motor Control
- [ ] Implement advanced motor control algorithms
- [ ] Add motor calibration routines
- [ ] Implement motor safety limits
- [ ] Add motor fault detection

#### System Monitoring
- [ ] Add system health monitoring
- [ ] Implement watchdog timer
- [ ] Add temperature monitoring
- [ ] Create system statistics

#### Communication Protocols
- [ ] Implement additional communication protocols
- [ ] Add protocol versioning
- [ ] Create communication encryption
- [ ] Implement data compression

### 📝 Documentation TODO

- [ ] Add API reference for all modules
- [ ] Create troubleshooting guide
- [ ] Add performance optimization tips
- [ ] Document hardware dependencies
- [ ] Create migration guide for future versions

## Contributing Guidelines

### When to Update Documentation
- After adding new modules or features
- When changing module interfaces
- After modifying type definitions
- When adding new development tools
- After significant refactoring

### Documentation Standards
- Keep examples up to date with current code
- Document all public interfaces
- Include usage examples for new features
- Update TODO sections as tasks are completed
- Maintain consistent formatting throughout