// SPDX-License-Identifier: GPL-2.0
/*
 * ALSA SoC Texas Instruments rt5640 Quad-Channel Audio Amplifier
 *
 * Copyright (C) 2016-2017 Texas Instruments Incorporated - https://www.ti.com/
 *	Author: Andreas Dannenberg <dannenberg@ti.com>
 *	Andrew F. Davis <afd@ti.com>
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>

//#include "rt5640.h"

/* Define how often to check (and clear) the fault status register (in ms) */
#define RT5640_FAULT_CHECK_INTERVAL 2000

struct rt5640_data {
	struct device *dev;
	struct regmap *regmap;
};

//static const struct reg_default rt5640_reg_defaults[] = {
//#define RT5640_MISC_CTRL1		0x01
//	{ RT5640_MISC_CTRL1,		0x32 },
//};

static const struct regmap_config rt5640_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,

//	.max_register = RT5640_MISC_CTRL1,
//	.reg_defaults = rt5640_reg_defaults,
//	.num_reg_defaults = ARRAY_SIZE(rt5640_reg_defaults),
//	.cache_type = REGCACHE_RBTREE,
};

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id rt5640_of_ids[] = {
	{ .compatible = "realtek,rt5640-lite", },
	{ },
};
MODULE_DEVICE_TABLE(of, rt5640_of_ids);
#endif

static int rt5640_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct rt5640_data *rt5640;
	int ret;
    
	rt5640 = devm_kzalloc(dev, sizeof(*rt5640), GFP_KERNEL);
	if (!rt5640)
		return -ENOMEM;
	dev_set_drvdata(dev, rt5640);

	rt5640->dev = dev;

	rt5640->regmap = devm_regmap_init_i2c(client, &rt5640_regmap_config);
	if (IS_ERR(rt5640->regmap)) {
		ret = PTR_ERR(rt5640->regmap);
		dev_err(dev, "unable to allocate register map: %d\n", ret);
		return ret;
	}

    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x00 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x00, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);

    //sleep 0.1
    usleep_range(1000, 1001);
    
	return 0;
}

static int rt5640_i2c_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct rt5640_data *rt5640 = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&rt5640->fault_check_work);

	return 0;
}

static const struct i2c_device_id rt5640_i2c_ids[] = {
	{ "rt5640", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rt5640_i2c_ids);

static struct i2c_driver rt5640_i2c_driver = {
	.driver = {
		.name = "rt5640",
		.of_match_table = of_match_ptr(rt5640_of_ids),
	},
	.probe = rt5640_i2c_probe,
	.remove = rt5640_i2c_remove,
	.id_table = rt5640_i2c_ids,
};
module_i2c_driver(rt5640_i2c_driver);

MODULE_DESCRIPTION("ASoC RT5640/RT5639 driver");
MODULE_AUTHOR("Johnny Hsu <johnnyhsu@realtek.com>");
MODULE_LICENSE("GPL v2");
