# toolchain-arm-none-eabi.cmake
#
# CMake toolchain file for ARM Cortex-M processors.
#
# This file is intended to be used with the GNU Arm Embedded Toolchain.

# Set the target system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set the toolchain prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Specify the cross-compilers and tools
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc) # Use gcc for assembly files as well
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")
set(CMAKE_DEBUGGER ${TOOLCHAIN_PREFIX}gdb CACHE INTERNAL "debugger")

# Set compiler flags for the initial CMake compiler test.
# This is necessary to ensure CMake can successfully compile a simple test program.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_ASM_FLAGS_INIT "" CACHE STRING "Initial ASM flags for compiler test")
set(CMAKE_CXX_FLAGS_INIT "" CACHE STRING "Initial CXX flags for compiler test")

# Configure search paths for libraries and headers.
# This tells CMake to only look in the toolchain's sysroot for libraries and headers,
# and not on the host system.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
