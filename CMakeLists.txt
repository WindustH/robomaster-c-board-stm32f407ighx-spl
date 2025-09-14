# CMakeLists.txt for STM32F407 Project
cmake_minimum_required(VERSION 3.20)

# Project information
project(stm32f407ighx C ASM)

# --- Target Definition ---
set(TARGET ${PROJECT_NAME})
add_executable(${TARGET} "") # Create the target, sources will be added later
set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

# --- MCU Specific Definitions ---
set(CPU cortex-m4)
set(FPU fpv4-sp-d16)
set(FLOAT_ABI hard)
set(MCU_DEVICE STM32F407xx) # Renamed to avoid conflict with potential CMake variables

# --- Build Configuration ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "No build type specified, defaulting to 'Debug'")
endif()

# --- File Paths and Sources ---
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F407IGHX_FLASH.ld)

# Directories
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/inc)
set(DRIVER_DIR ${CMAKE_SOURCE_DIR}/libs)
set(CMSIS_DIR ${DRIVER_DIR}/CMSIS)
set(STDPERIPH_DIR ${DRIVER_DIR}/STM32F4xx_StdPeriph_Driver)
set(STARTUP_DIR ${CMAKE_SOURCE_DIR}/startup)

# Source Files
# Note: Using file(GLOB...) is convenient but not always robust. If you add new files,
# you may need to re-run CMake manually for them to be detected.
file(GLOB_RECURSE APP_SOURCES ${SOURCE_DIR}/*.c)

set(DRIVER_SOURCES
    ${CMSIS_DIR}/system_stm32f4xx.c
    ${STDPERIPH_DIR}/src/misc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dma.c
    ${STDPERIPH_DIR}/src/stm32f4xx_gpio.c
    ${STDPERIPH_DIR}/src/stm32f4xx_rcc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_tim.c
    ${STDPERIPH_DIR}/src/stm32f4xx_usart.c
)

set(ASM_SOURCES
    ${STARTUP_DIR}/startup_stm32f407ighx.s
)

target_sources(${TARGET}
    PRIVATE
    ${APP_SOURCES}
    ${DRIVER_SOURCES}
    ${ASM_SOURCES}
)

# --- Include Directories ---
target_include_directories(${TARGET}
    PUBLIC
    ${INCLUDE_DIR}
    ${CMSIS_DIR}/Include
    ${CMSIS_DIR}/Device/ST/STM32F4xx/Include
    ${STDPERIPH_DIR}/inc
)

# --- Compile Options and Definitions ---
# Common flags for all build types
target_compile_options(${TARGET}
    PRIVATE
    -mcpu=${CPU}
    -mthumb
    -mfpu=${FPU}
    -mfloat-abi=${FLOAT_ABI}
    -Wall
    -ffunction-sections # Place each function in its own section
    -fdata-sections # Place each data item in its own section
)

# Build-type specific flags
target_compile_options(${TARGET}
    PRIVATE
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-O2>
)

# Common definitions for all build types
target_compile_definitions(${TARGET}
    PRIVATE
    STM32F40_41xxx
    ${MCU_DEVICE}
    USE_STDPERIPH_DRIVER
)

# Build-type specific definitions
target_compile_definitions(${TARGET}
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# --- Linker Options ---
target_link_options(${TARGET}
    PRIVATE
    -mcpu=${CPU}
    -mthumb
    -mfpu=${FPU}
    -mfloat-abi=${FLOAT_ABI}
    --specs=nano.specs
    --specs=nosys.specs
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/${TARGET}.map,--cref
    -Wl,--gc-sections # Garbage collect unused sections
)

# --- Post-build Steps ---
# Create .bin and .hex files after linking
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET}.elf ${TARGET}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET}.elf ${TARGET}.hex
    COMMENT "Generating .bin and .hex files"
)

# Print size information after building
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${TARGET}.elf
    COMMENT "Printing size information"
)
