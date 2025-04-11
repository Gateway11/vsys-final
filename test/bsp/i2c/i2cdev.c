/*
 * ALSA SoC Texas Instruments i2cdev Quad-Channel Audio Amplifier
 *
 * Copyright (C) 2016-2017 Texas Instruments Incorporated - https://www.ti.com/
 *	Author: Andreas Dannenberg <dannenberg@ti.com>
 *	Andrew F. Davis <afd@ti.com>
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>

static int i2cdev_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
    u32 *raw_data;
    int num_regs, index = 0;
	int ret;
    
    ret = of_property_count_u32_elems(dev->of_node, "write-regs");
    if (ret < 0) {
        dev_err(dev, "Failed to read write-regs property\n");
        return ret;
    }

    num_regs = ret;
    raw_data = kmalloc_array(num_regs, sizeof(u32), GFP_KERNEL);
    if (!raw_data)
        return -ENOMEM;

    ret = of_property_read_u32_array(dev->of_node, "write-regs", raw_data, num_regs);
    if (ret < 0) {
        dev_err(dev, "Failed to read write-regs array\n");
        kfree(raw_data);
        return ret;
    }

    uint8_t *write_regs = kmalloc(num_regs, GFP_KERNEL);
    for (int i = 0; i < num_regs; i++) {
        write_regs[i] = (uint8_t)(raw_data[i] & 0xFF);
    }

    while (index < num_regs) {
#if 1
        pr_info("cnt=%d, addr=0x%02X, ", write_regs[index], write_regs[index + 1]);
        for (int i = 2; i <= write_regs[index]; i++) {
            pr_cont("0x%02X ", write_regs[index + i]);
        }
        pr_cont("\n");
#endif
        client->addr = write_regs[index + 1];
        ret = i2c_master_send(client, write_regs + index + 2, write_regs[index] - 1);
        if (ret < 0) {
            dev_warn(dev, "Failed to write i2c addr=0x%02X, reg=0x%02X\n",
                    write_regs[index + 1], write_regs[index + 2]);
            goto exit;
        }
        index += 1 + write_regs[index];
    }   

exit:
    kfree(raw_data);
    kfree(write_regs);
    return 0;
}

static void i2cdev_i2c_remove(struct i2c_client *client)
{
}

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id i2cdev_of_ids[] = {
    { .compatible = "lenovo,i2ctest", },
    { },
};
MODULE_DEVICE_TABLE(of, i2cdev_of_ids);
#endif

static const struct i2c_device_id i2cdev_i2c_ids[] = {
    { "i2cdev", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, i2cdev_i2c_ids);

static struct i2c_driver i2cdev_i2c_driver = {
    .driver = {
        .name = "i2cdev",
        .of_match_table = of_match_ptr(i2cdev_of_ids),
    },
    .probe = i2cdev_i2c_probe,
    .remove = i2cdev_i2c_remove,
    .id_table = i2cdev_i2c_ids,
};
#if 0
module_i2c_driver(i2cdev_i2c_driver);
#else

static int __init i2cdev_init(void)
{
    return i2c_add_driver(&i2cdev_i2c_driver);
}

static void __exit i2cdev_exit(void)
{
    i2c_del_driver(&i2cdev_i2c_driver);
}

subsys_initcall(i2cdev_init);
module_exit(i2cdev_exit);
#endif

MODULE_DESCRIPTION("ASoC i2cdev driver");
MODULE_AUTHOR("Xiang Dai <daixiang5@lenovo.com>");
MODULE_LICENSE("GPL v2");
