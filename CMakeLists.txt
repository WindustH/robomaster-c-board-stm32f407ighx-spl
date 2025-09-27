# cmake file for stm32f407 project
# usage: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-none-eabi.cmake
cmake_minimum_required(VERSION 3.16)

# project information
project(stm32f407ighx C ASM)

# target definition
set(TARGET ${PROJECT_NAME})
add_executable(${TARGET})
set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

# project specific configuration
set(MCU_DEVICE STM32F407xx)
set(HSE_FREQ 12000000)

# build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "no build type specified, defaulting to 'Debug'")
endif()

# file paths and directories
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F407IGHX_FLASH.ld)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(DRIVER_DIR ${CMAKE_SOURCE_DIR}/core)
set(CMSIS_DIR ${DRIVER_DIR}/CMSIS)
set(HAL_DIR ${DRIVER_DIR}/STM32F4xx_HAL_Driver)
set(STARTUP_DIR ${CMAKE_SOURCE_DIR}/startup)

# application source files
file(GLOB_RECURSE APP_SOURCES ${SOURCE_DIR}/*.c)

# hal driver sources
set(DRIVER_SOURCES
    # cmsis system files
    ${CMSIS_DIR}/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

    # hal driver sources
    ${HAL_DIR}/Src/stm32f4xx_hal.c
    ${HAL_DIR}/Src/stm32f4xx_hal_rcc.c
    ${HAL_DIR}/Src/stm32f4xx_hal_gpio.c
    ${HAL_DIR}/Src/stm32f4xx_hal_dma.c
    ${HAL_DIR}/Src/stm32f4xx_hal_uart.c
    ${HAL_DIR}/Src/stm32f4xx_hal_tim.c
    ${HAL_DIR}/Src/stm32f4xx_hal_tim_ex.c
    ${HAL_DIR}/Src/stm32f4xx_hal_cortex.c
    ${HAL_DIR}/Src/stm32f4xx_hal_flash.c
    ${HAL_DIR}/Src/stm32f4xx_hal_flash_ex.c
    ${HAL_DIR}/Src/stm32f4xx_hal_pwr.c
    ${HAL_DIR}/Src/stm32f4xx_hal_pwr_ex.c
    ${HAL_DIR}/Src/stm32f4xx_hal_exti.c
    ${HAL_DIR}/Src/stm32f4xx_hal_can.c
)

# assembly startup files
set(ASM_SOURCES
    ${STARTUP_DIR}/startup_stm32f407ighx.s
)

# add all source files to target
target_sources(${TARGET}
    PRIVATE
    ${APP_SOURCES}
    ${DRIVER_SOURCES}
    ${ASM_SOURCES}
)

# include directories
target_include_directories(${TARGET}
    PUBLIC
    ${SOURCE_DIR}
    ${INCLUDE_DIR}
    ${CMSIS_DIR}/Include
    ${CMSIS_DIR}/Device/ST/STM32F4xx/Include
    ${HAL_DIR}/Inc
)

# common compiler options
target_compile_options(${TARGET}
    PRIVATE
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
    -fstack-usage
    -fdiagnostics-color=always
    -fno-diagnostics-show-option
)

# build-type specific compiler options
target_compile_options(${TARGET}
    PRIVATE
    $<$<CONFIG:Debug>:-Og -g3 -DDEBUG>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
    $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
)

# preprocessor definitions
# sysclk calculation: 12mhz / 12 * 336 / 2 = 168mhz
# usb clock: sysclk / 7 = 48mhz
target_compile_definitions(${TARGET}
    PRIVATE
    STM32F40_41xxx
    ${MCU_DEVICE}
    USE_HAL_DRIVER
    HSE_VALUE=${HSE_FREQ}
)

# linker options
target_link_options(${TARGET}
    PRIVATE
    --specs=nano.specs
    --specs=nosys.specs
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/${TARGET}.map,--cref
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)
