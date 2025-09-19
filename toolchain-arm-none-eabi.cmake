# toolchain-arm-none-eabi.cmake
#
# CMake toolchain file for ARM Cortex-M processors, specifically targeting
# a Cortex-M4 with hard float.
#
# This file is intended to be used with the GNU Arm Embedded Toolchain.
# To use it, invoke CMake with:
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-none-eabi.cmake

# --- System and Processor Configuration ---
# These settings define the target architecture for the cross-compilation.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# --- Language Standard Configuration ---
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Toolchain Executables ---
# Defines the prefix for the toolchain commands (e.g., arm-none-eabi-gcc).
# Assumes the toolchain binaries are in the system's PATH.
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Specify the cross-compilers and other essential tools.
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")
set(CMAKE_GDB ${TOOLCHAIN_PREFIX}gdb CACHE INTERNAL "debugger")

# --- Core Compilation Flags ---
# These flags are fundamental to targeting the specific MCU architecture.
# They are set here to be applied globally to all C and ASM files.
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "Initial C flags")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "Initial ASM flags")
set(CMAKE_EXE_LINKER_FLAGS_INIT "" CACHE STRING "Initial Linker flags")

# --- Auto-detect toolchain system include directories ---
# Find system include directories for the ARM GCC toolchain and add them as system includes
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -E -Wp,-v -xc /dev/null
    RESULT_VARIABLE _GCC_RESULT
    ERROR_VARIABLE _GCC_OUTPUT
    OUTPUT_QUIET
)

if(NOT _GCC_RESULT)
    string(REGEX REPLACE ".*#include <...> search starts here:\n" "" _SEARCH_PATHS "${_GCC_OUTPUT}")
    string(REGEX REPLACE "\nEnd of search list..*" "" _SEARCH_PATHS "${_SEARCH_PATHS}")
    string(REPLACE "\n" ";" _SEARCH_PATHS_LIST "${_SEARCH_PATHS}")

    set(TOOLCHAIN_SYSTEM_INCLUDE_DIRS "")
    foreach(path ${_SEARCH_PATHS_LIST})
        string(STRIP "${path}" path)
        if(IS_DIRECTORY "${path}")
            list(APPEND TOOLCHAIN_SYSTEM_INCLUDE_DIRS "${path}")
        endif()
    endforeach()

    if(TOOLCHAIN_SYSTEM_INCLUDE_DIRS)
        message(STATUS "Auto-detected ARM GCC system includes: ${TOOLCHAIN_SYSTEM_INCLUDE_DIRS}")
        # Add as system includes to C flags
        foreach(dir ${TOOLCHAIN_SYSTEM_INCLUDE_DIRS})
            set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -isystem ${dir}")
        endforeach()
    endif()
endif()

# --- CMake Behavior Configuration ---
# This is necessary to ensure CMake can successfully compile a simple test program
# during its initial checks.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands")

# Configure search paths for libraries and headers. This tells CMake to only
# look in the toolchain's sysroot for libraries and headers, and not on the
# host system, which is critical for cross-compilation.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)