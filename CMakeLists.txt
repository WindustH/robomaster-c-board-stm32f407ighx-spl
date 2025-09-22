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
set(STDPERIPH_DIR ${DRIVER_DIR}/STM32F4xx_StdPeriph_Driver)
set(STARTUP_DIR ${CMAKE_SOURCE_DIR}/startup)

# application source files
file(GLOB_RECURSE APP_SOURCES ${SOURCE_DIR}/*.c)

# driver source files
set(DRIVER_SOURCES
    # cmsis system files
    ${CMSIS_DIR}/system_stm32f4xx.c

    # stdperiph driver sources
    ${STDPERIPH_DIR}/src/misc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_adc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_can.c
    ${STDPERIPH_DIR}/src/stm32f4xx_cec.c
    ${STDPERIPH_DIR}/src/stm32f4xx_crc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_cryp.c
    ${STDPERIPH_DIR}/src/stm32f4xx_cryp_aes.c
    ${STDPERIPH_DIR}/src/stm32f4xx_cryp_des.c
    ${STDPERIPH_DIR}/src/stm32f4xx_cryp_tdes.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dac.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dbgmcu.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dcmi.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dfsdm.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dma.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dma2d.c
    ${STDPERIPH_DIR}/src/stm32f4xx_dsi.c
    ${STDPERIPH_DIR}/src/stm32f4xx_exti.c
    ${STDPERIPH_DIR}/src/stm32f4xx_flash.c
    ${STDPERIPH_DIR}/src/stm32f4xx_flash_ramfunc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_fmpi2c.c
    ${STDPERIPH_DIR}/src/stm32f4xx_fsmc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_gpio.c
    ${STDPERIPH_DIR}/src/stm32f4xx_hash.c
    ${STDPERIPH_DIR}/src/stm32f4xx_hash_md5.c
    ${STDPERIPH_DIR}/src/stm32f4xx_hash_sha1.c
    ${STDPERIPH_DIR}/src/stm32f4xx_i2c.c
    ${STDPERIPH_DIR}/src/stm32f4xx_iwdg.c
    ${STDPERIPH_DIR}/src/stm32f4xx_lptim.c
    ${STDPERIPH_DIR}/src/stm32f4xx_ltdc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_pwr.c
    ${STDPERIPH_DIR}/src/stm32f4xx_qspi.c
    ${STDPERIPH_DIR}/src/stm32f4xx_rcc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_rng.c
    ${STDPERIPH_DIR}/src/stm32f4xx_rtc.c
    ${STDPERIPH_DIR}/src/stm32f4xx_sai.c
    ${STDPERIPH_DIR}/src/stm32f4xx_sdio.c
    ${STDPERIPH_DIR}/src/stm32f4xx_spdifrx.c
    ${STDPERIPH_DIR}/src/stm32f4xx_spi.c
    ${STDPERIPH_DIR}/src/stm32f4xx_syscfg.c
    ${STDPERIPH_DIR}/src/stm32f4xx_tim.c
    ${STDPERIPH_DIR}/src/stm32f4xx_usart.c
    ${STDPERIPH_DIR}/src/stm32f4xx_wwdg.c
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
    ${STDPERIPH_DIR}/inc
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
    USE_STDPERIPH_DRIVER
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
