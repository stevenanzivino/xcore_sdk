// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "driver_instances.h"
#include "example_pipeline/example_pipeline.h"

void gpio_start() {
#if appconfGPIO_RPC_ENABLED
  {
    rtos_gpio_rpc_config(gpio_ctx, appconfGPIO_RPC_PORT,
                         appconfGPIO_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(0)
  {
    rtos_gpio_start(gpio_ctx);
    rtos_printf("Started GPIO driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void i2c_start() {
#if appconfI2C_RPC_ENABLED
  {
    rtos_i2c_master_rpc_config(i2c_master_ctx, appconfI2C_MASTER_RPC_PORT,
                               appconfI2C_MASTER_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(0)
  {
    rtos_i2c_master_start(i2c_master_ctx);
    rtos_printf("Started I2C driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void spi_start() {
#if appconfSPI_RPC_ENABLED
  {
    rtos_spi_master_rpc_config(spi_master_ctx, appconfSPI_RPC_PORT,
                               appconfSPI_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(0)
  {
    rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES - 1);
    rtos_printf("Started SPI driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void i2s_start() {
#if appconfI2S_RPC_ENABLED
  {
    rtos_i2s_rpc_config(i2s_ctx, appconfI2S_RPC_PORT,
                        appconfI2S_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(1)
  {
    rtos_i2s_start(i2s_ctx,
                   rtos_i2s_mclk_bclk_ratio(appconfAUDIO_CLOCK_FREQUENCY,
                                            EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE),
                   I2S_MODE_I2S, 3.0 * MIC_DUAL_FRAME_SIZE,
                   3.0 * MIC_DUAL_FRAME_SIZE, 0);
    rtos_printf("Started I2S driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void mics_start() {
#if appconfMIC_ARRAY_RPC_ENABLED && !I2S_ADC_ENABLED
  {
    rtos_mic_array_rpc_config(mic_array_ctx, appconfMIC_ARRAY_RPC_PORT,
                              appconfMIC_ARRAY_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(1)
  {
    const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
        appconfPDM_CLOCK_FREQUENCY, EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE);

    rtos_mic_array_start(
        mic_array_ctx, pdm_decimation_factor,
        rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
        rtos_mic_array_fir_compensation(pdm_decimation_factor),
        3.0 * MIC_DUAL_FRAME_SIZE, 0);
    rtos_printf("Started mic array driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void flash_start() {
#if appconfQSPI_FLASH_RPC_ENABLED
  {
    rtos_qspi_flash_rpc_config(qspi_flash_ctx, appconfQSPI_RPC_PORT,
                               appconfQSPI_RPC_HOST_TASK_PRIORITY);
  }
#endif
#if ON_TILE(0)
  {
    rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES - 1);
    rtos_printf("Started QSPI driver on tile %d\n", THIS_XCORE_TILE);
  }
#endif
}

void platform_start(void) {
  rtos_intertile_start(intertile1_ctx);
  rtos_intertile_start(intertile2_ctx);

  gpio_start();
  spi_start();
  flash_start();
  i2c_start();
  i2s_start();
  mics_start();
}
