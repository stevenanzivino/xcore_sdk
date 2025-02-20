cmake_minimum_required(VERSION 3.14)

## Import hil source
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL TRUE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

set(APP_NAME i2s_master_external_clock_test)

set(APP_COMPILER_FLAGS
    "-O3"
    "-g"
    "-report"
    "-target=XCORE-AI-EXPLORER"
)

set(APP_SOURCES
    "src/main.c"
)

set(APP_INCLUDES
    "src"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(OUTS 0 1 2 3 4)
set(INS 0 1 2 3 4)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

foreach(out ${OUTS})
    foreach(in ${INS})
        set(TARGET_NAME_NO_EXT "${APP_NAME}_smoke_${out}${in}")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                   PRIVATE
                                       NUM_OUT=${out}
                                       NUM_IN=${in}
                                       SMOKE=1
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})


        set(TARGET_NAME_NO_EXT "${APP_NAME}_nightly_${out}${in}")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                   PRIVATE
                                       NUM_OUT=${out}
                                       NUM_IN=${in}
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
    endforeach()
endforeach()
