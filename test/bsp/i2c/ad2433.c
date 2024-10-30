// SPDX-License-Identifier: GPL-2.0
/*
 * ALSA SoC Texas Instruments ad2433 Quad-Channel Audio Amplifier
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

struct ad2428_data *ad2428_global_data = NULL;

struct ad2433_data {
	struct device *dev;
	struct regmap *regmap;
	struct delayed_work fault_check_work;
};

static void ad2433_fault_check_work(struct work_struct *work)
{
}

static const struct regmap_config ad2433_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
};

a2b_HResult a2b_pal_I2cWriteFunc(a2b_Handle hnd,
        uint16_t addr, uint16_t nWrite,
        const uint8_t* wBuf)
{
  a2b_HResult nReturnValue = 0, regCnt = nWrite - 1, i;
  struct i2c_msg msg[1];

  msg[0].addr = addr;
  msg[0].flags = 0;
  msg[0].len = nWrite;
  msg[0].buf = (uint8_t*)wBuf;

  nReturnValue = i2c_transfer(ad2428_global_data->client->adapter, msg, ARRAY_SIZE(msg));
#ifdef A2B_PRINT_CONSOLE
  for (i = 0; i < regCnt; i++) {
    pr_alert("i2c-%d write device(%#x) reg=0x%02x val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
        AUDIO_I2C_INSTANCE, addr, wBuf[0] + i, wBuf[i + 1], PRINTF_BYTE_TO_BINARY_INT8(wBuf[i + 1]), regCnt);
  }
#endif
  if(nReturnValue != ARRAY_SIZE(msg)) {
    pr_err("i2c-%d write device(%#x) reg=0x%02x error\n", AUDIO_I2C_INSTANCE, addr, wBuf[0]);
    return -1;
  }
  (void)i;
  return A2B_RESULT_SUCCESS;
}

a2b_HResult a2b_pal_I2cWriteReadFunc(a2b_Handle hnd, uint16_t addr, uint16_t nWrite,
        const uint8_t* wBuf, uint16_t nRead, uint8_t* rBuf)
{
  a2b_HResult nReturnValue = A2B_RESULT_SUCCESS;
  struct i2c_msg msg[2];

  msg[0].addr = addr;
  msg[0].flags = 0;
  msg[0].len = nWrite;
  msg[0].buf = (uint8_t*)wBuf;
  msg[1].flags = I2C_M_RD;
  msg[1].addr = addr;
  msg[1].len = nRead;
  msg[1].buf = rBuf;

  nReturnValue = i2c_transfer(ad2428_global_data->client->adapter, msg, ARRAY_SIZE(msg));

  if(nReturnValue != ARRAY_SIZE(msg)) {
    pr_alert("i2c-%d read device(%#x) reg=0x%02x error\n", AUDIO_I2C_INSTANCE, addr, rBuf[0]);
    return -1;
  }
#ifdef A2B_PRINT_CONSOLE
  for (i = 0; i < nRead; i++) {
    pr_err("i2c-%d write device(%#x) reg=0x%02x val=0x%02x ("PRINTF_BINARY_PATTERN_INT8") cnt=%d\n",
        AUDIO_I2C_INSTANCE, addr, wBuf[0] + i, rBuf[i], PRINTF_BYTE_TO_BINARY_INT8(rBuf[i]), nRead);
  }
#endif
  return A2B_RESULT_SUCCESS;
}

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id ad2433_of_ids[] = {
	{ .compatible = "lenovo,ad2433", },
	{ },
};
MODULE_DEVICE_TABLE(of, ad2433_of_ids);
#endif

static int ad2433_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct ad2433_data *ad2433;
	int ret;
    
	ad2433 = devm_kzalloc(dev, sizeof(*ad2433), GFP_KERNEL);
	if (!ad2433)
		return -ENOMEM;
	dev_set_drvdata(dev, ad2433);

	ad2433->dev = dev;

	ad2433->regmap = devm_regmap_init_i2c(client, &ad2433_regmap_config);
	if (IS_ERR(ad2433->regmap)) {
		ret = PTR_ERR(ad2433->regmap);
		dev_err(dev, "unable to allocate register map: %d\n", ret);
		return ret;
	}

    ad2428_global_data = ad2428;

	INIT_DELAYED_WORK(&ad2433->fault_check_work, ad2433_fault_check_work);
	//schedule_delayed_work(&ad2433->fault_check_work, msecs_to_jiffies(ad2433_FAULT_CHECK_INTERVAL));

	return 0;
}

static int ad2433_i2c_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct ad2433_data *ad2433 = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&ad2433->fault_check_work);

	return 0;
}

static const struct i2c_device_id ad2433_i2c_ids[] = {
	{ "ad2433", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ad2433_i2c_ids);

static struct i2c_driver ad2433_i2c_driver = {
	.driver = {
		.name = "ad2433",
		.of_match_table = of_match_ptr(ad2433_of_ids),
	},
	.probe = ad2433_i2c_probe,
	.remove = ad2433_i2c_remove,
	.id_table = ad2433_i2c_ids,
};
module_i2c_driver(ad2433_i2c_driver);

MODULE_DESCRIPTION("ASoC AD2433 driver");
MODULE_AUTHOR("Johnny Hsu <johnnyhsu@realtek.com>");
MODULE_LICENSE("GPL v2");
