// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/mic_array/api/rtos_mic_array.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/i2s/api/rtos_i2s.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"

#define I2C_TILE 1
#define PIPELINE_TILE 0
#define GPIO_TILE 0
#define WIFI_TILE 1 /* Uses SPI, GPIO, and QSPI */
#define QSPI_FLASH_TILE 1

extern rtos_intertile_t *intertile1_ctx;
extern rtos_intertile_t *intertile2_ctx;
extern rtos_gpio_t *gpio_ctx;
extern rtos_qspi_flash_t *qspi_flash_ctx;
extern rtos_i2c_master_t *i2c_master_ctx;
extern rtos_spi_master_t *spi_master_ctx;
extern rtos_spi_master_device_t *wifi_device_ctx;
extern rtos_i2s_t *i2s_ctx;
extern rtos_mic_array_t *mic_array_ctx;

#endif /* DRIVER_INSTANCES_H_ */