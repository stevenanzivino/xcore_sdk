cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_XUD_HIL TRUE)
set(USE_I2C_HIL TRUE)
set(USE_I2S_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)

set(USE_RTOS_GPIO_DRIVER TRUE)
set(USE_RTOS_I2C_DRIVER TRUE)
set(USE_RTOS_I2S_DRIVER FALSE)
set(USE_RTOS_INTERTILE_DRIVER TRUE)
set(USE_RTOS_MIC_ARRAY_DRIVER FALSE)
set(USE_RTOS_RPC_DRIVER TRUE)
set(USE_RTOS_SPI_DRIVER FALSE)
set(USE_RTOS_USB_DRIVER TRUE)
set(USE_RTOS_SWMEM_DRIVER FALSE)
set(USE_RTOS_QSPI_FLASH_DRIVER FALSE)
set(USE_RTOS_TRACE_DRIVER FALSE)
set(USE_RTOS_WIFI_DRIVER FALSE)

set(USE_DEVICE_CONTROL TRUE)
set(USE_TINYUSB TRUE)

## Import XMOS RTOS platform configuration.
## Must be done after setting the configuration options.
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(device_control)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX C ASM)

set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/board_init.c"
    "src/usb_descriptors.c"
)

set(APP_INCLUDES
    "src"
)

if(${BOARD} STREQUAL "XCORE200-MIC-ARRAY")
    set(SYSCLK_FREQ 500)
else()
    set(SYSCLK_FREQ 700)
endif()

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    XUD_CORE_CLOCK=${SYSCLK_FREQ}
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
