// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_MASTER_DEV_CONF_DEFAULTS_H_
#define SPI_MASTER_DEV_CONF_DEFAULTS_H_

/* SPI defaults */

#ifndef SPICONF_BUFFER_LEN
#define SPICONF_BUFFER_LEN       4096
#endif

#ifndef SPICONF_RX_ONLY_CHAR
#define SPICONF_RX_ONLY_CHAR    0x00
#endif

#ifndef SPICONF_INTERBYTE_DELAY_ENABLE
#define SPICONF_INTERBYTE_DELAY_ENABLE 1
#endif

#ifndef SPICONF_MIN_CS_TO_DATA_DELAY_NS
#define SPICONF_MIN_CS_TO_DATA_DELAY_NS 500
#endif

#endif /* SPI_MASTER_DEV_CONF_DEFAULTS_H_ */
