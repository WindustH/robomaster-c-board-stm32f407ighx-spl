# toolchain-arm-none-eabi.cmake
#
# cmake toolchain file for arm cortex-m processors, specifically targeting
# cortex-m4 with hard float support for stm32f4 series
#
# usage: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-none-eabi.cmake

# system and processor configuration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# language standard configuration
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# mcu specific configuration
set(CPU cortex-m4)
set(FPU fpv4-sp-d16)
set(FLOAT_ABI hard)

# toolchain executables
set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")
set(CMAKE_GDB ${TOOLCHAIN_PREFIX}gdb CACHE INTERNAL "debugger")

# core compilation flags for cortex-m4
set(CPU_FLAGS "-mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI}")

# set initial flags
set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "initial c flags")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "initial asm flags")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "initial linker flags")

# cmake behavior configuration
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "export compile commands")

# configure search paths for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
