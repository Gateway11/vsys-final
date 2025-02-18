/*
 * A2B24XX driver
 *
 * Copyright 2023 Analog Devices Inc.
 *  Author: ADI Automotive Software Team, Bangalore
 *
 * Licensed under the GPL-2.
 */

#ifndef __SOUND_SOC_CODECS_A2B24XX_H__
#define __SOUND_SOC_CODECS_A2B24XX_H__

#include <linux/regmap.h>

#define A2B_MASTER_ADDR                 0x68
#define A2B_SLAVE_ADDR                  0x69

#define A2B_MASTER_NODE                   -1
#define A2B_INVALID_NODE                  -2

struct device;

enum a2b24xx_type {
	A2B24XX,

};

int a2b24xx_probe(struct device *dev, struct regmap *regmap,
	enum a2b24xx_type type, void (*switch_mode)(struct device *dev));

int a2b24xx_remove(struct device *dev);

extern const struct regmap_config a2b24xx_regmap_config;

enum a2b24xx_clk_id {
	A2B24XX_SYSCLK,
};

enum a2b24xx_sysclk_src {
	A2B24XX_SYSCLK_SRC_MCLK,
	A2B24XX_SYSCLK_SRC_LRCLK,
};

#endif

