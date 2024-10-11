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
	struct delayed_work fault_check_work;
};

static void rt5640_fault_check_work(struct work_struct *work)
{
	struct rt5640_data *rt5640 = container_of(work, struct rt5640_data,
						    fault_check_work.work);
	struct device *dev = rt5640->dev;
	unsigned int reg;
	int ret;

#define RT5640_CHANNEL_FAULT		0x10
	ret = regmap_read(rt5640->regmap, RT5640_CHANNEL_FAULT, &reg);
	if (ret < 0) {
		dev_err(dev, "failed to read CHANNEL_FAULT register: %d\n", ret);
		goto out;
	}

out:
	/* Schedule the next fault check at the specified interval */
	schedule_delayed_work(&rt5640->fault_check_work,
			      msecs_to_jiffies(RT5640_FAULT_CHECK_INTERVAL));
}

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
    
    //#PR-3Dh: ADC/DAC RESET Control, Enable ADC and DAC Clock Generators
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x3D"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x003d);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x36 0x00"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x3600);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-12h:
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x12"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0012);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x0A 0xA8"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x0aa8);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-14h:
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x14"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0014);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x0A 0xAA"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x0aaa);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-20h:
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x20"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0020);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x61 0x10"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x6110);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-21h:
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x21"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0021);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0xE0 0xE0"
	ret = regmap_write(rt5640->regmap, 0x6c, 0xe0e0);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
   
    //#PR-23h:
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x23"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0023);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x18 0x04"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x1804);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-FAh: General Control 1, MCLK Detection ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0xFA -w "0x3C 0x00"
	ret = regmap_write(rt5640->regmap, 0xfa, 0x3c00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Dh: Class-D Amp Output Control, AC+DC ratio gain control
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8D -w "0xA8 0x00"
	ret = regmap_write(rt5640->regmap, 0x8d, 0xa800);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Eh: HP Amp Control 1, Power On Soft Generator
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8E -w "0x00 0x04"
	ret = regmap_write(rt5640->regmap, 0x8e, 0x0004);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Fh: HP Amp Control 2, Set to POR value
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8F -w "0x11 0x00"
	ret = regmap_write(rt5640->regmap, 0x8f, 0x1100);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-61h: Power Management Control 1, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x61 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x61, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-62h: Power Management Control 2, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x62 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x62, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-63h: Power Management Control 3, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x63, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-64h: Power Management Control 4, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x64 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x64, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-65h: Power Management Control 5, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x65 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x65, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-66h: Power Management Control 6, reset
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x66 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x66, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-FAh: General Control 1, I2S Clock Gating enable
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0xFA -w "0x34 0x01"
	ret = regmap_write(rt5640->regmap, 0xfa, 0x3401);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-2Eh: DSP_PATH2
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x2E -w "0x0C 0x00"
	ret = regmap_write(rt5640->regmap, 0x2e, 0x0c00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-63h: Power Management Control 3, VREF1 Power and fastmode, MBIAS, VREF2 Power and fastmode, LD02 Controls enable
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0xE8 0x1C"
	ret = regmap_write(rt5640->regmap, 0x63, 0xe81c);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-64h: Power Management Control 4, MICBIAS1 Power Control ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x64 -w "0x08 0x00"
	ret = regmap_write(rt5640->regmap, 0x64, 0x0800);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-19h: DACL1/R1 Digital Volume
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x19 -w "0xAF 0xAF"
	ret = regmap_write(rt5640->regmap, 0x19, 0xafaf);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-1Ah: DACL2/R2 Digital Volume
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x1A -w "0xAF 0xAF"
	ret = regmap_write(rt5640->regmap, 0x1a, 0xafaf);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-1Bh: DACL2/R2 Mute/Unmute Control
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x1B -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x1b, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-29h: Stereo ADC to DAC Digital Mixer Control, Mute I2S1 to DAC L/R channels
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x29 -w "0xC0 0xC0"
	ret = regmap_write(rt5640->regmap, 0x29, 0xc0c0);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    //#Bootup registers end
    
    //sleep 0.5
    usleep_range(5000, 5001);
    
    //#Playback registers start
    //#MX-29h: Stereo ADC to DAC Digital Mixer Control, Unmute I2S1 to DAC L/R channels
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x29 -w "0x80 0x80"
	ret = regmap_write(rt5640->regmap, 0x29, 0x8080);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-2Ah: Stereo DAC Digital Mixer Control, Unmute Controls for DACL1/DACR1 to Stereo DAC Left/Right Mixer
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x2A -w "0x14 0x14"
	ret = regmap_write(rt5640->regmap, 0x2a, 0x1414);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-2Ch: DAC Digital Mixer Control, DACL1 -> DACMIXL & DACR1 -> DACMIXR
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x2C -w "0x22 0x00"
	ret = regmap_write(rt5640->regmap, 0x2c, 0x2200);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-45h: HPOMIX Control, Unmute Control for DAC1 to HPOMIX
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x45 -w "0xA0 0x00"
	ret = regmap_write(rt5640->regmap, 0x45, 0xa000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-70h: I2S1 Digital Interface Control, Slave Mode and I2S1 <= BCLK1/LRCK1/DACDAT1/ADCDAT1
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x70 -w "0x00 0x00"
	ret = regmap_write(rt5640->regmap, 0x70, 0x0000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-73h: ADC/DAC Clock Control 1, I2S1 Master Mode Clock Relative of BCLK and LRCK set to 16Bits
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x73 -w "0x01 0x14"
	ret = regmap_write(rt5640->regmap, 0x73, 0x0114);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-61h: Power Management Control 1, I2S1, DAC1L, DAC1R power ON + I2S1 Digital Interface Power ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x61 -w "0x98 0x07"
	ret = regmap_write(rt5640->regmap, 0x61, 0x9807);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-63h: Power Management Control 3, Left and Right Headphone Amp Power Control ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0xE8 0xFC"
	ret = regmap_write(rt5640->regmap, 0x63, 0xe8fc);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-66h: Power Management Control 6, SPKVOLL SPKVOLR HPOVOLL HPKVOLR Power ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x66 -w "0x03 0x00"
	ret = regmap_write(rt5640->regmap, 0x66, 0x0300);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-D3h: Wind Filter Control – Enable
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0xD3 -w "0x2A 0x20"
	ret = regmap_write(rt5640->regmap, 0xd3, 0x2a20);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-24h: Charge pump int reg1
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x24"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0024);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x02 0x20"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x0220);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Fh: HP Amp Control 2, Set HP Depop mode to 2
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8F -w "0x31 0x00"
	ret = regmap_write(rt5640->regmap, 0x8f, 0x3100);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Eh: HP Amp Control 1, Charge Pump Power Control ON and HP Amp All Power On Control ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8E -w "0x00 0x09"
	ret = regmap_write(rt5640->regmap, 0x8e, 0x0009);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-77h: HP DCC INT1
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x77"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0077);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x9F 0x00"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x9f00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-63h: Power Management Control 3, VREF1/2 Fast Mode Control to slow
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0xA8 0xD4"
	ret = regmap_write(rt5640->regmap, 0x63, 0xa8d4);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-63h: Power Management Control 3, Improve HP Amp Driving enable
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0xA8 0xF4"
	ret = regmap_write(rt5640->regmap, 0x63, 0xa8f4);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //sleep 0.01
    usleep_range(100, 101);
    
    //#MX-63h: Power Management Control 3, VREF1/2 Fast Mode Control to fast
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x63 -w "0xE8 0xFC"
	ret = regmap_write(rt5640->regmap, 0x63, 0xe8fc);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Fh: HP Amp Control 2, Set HP Depop Mode to Mode 1 and Depop Mode 1 Control ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8F -w "0x11 0x40"
	ret = regmap_write(rt5640->regmap, 0x8f, 0x1140);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-91h: HP Amp Control, Set HP Charge Pump Mode Selection to Middle voltage mode
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x91 -w "0x0E 0x00"
	ret = regmap_write(rt5640->regmap, 0x91, 0x0e00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
   
    //#MX-90h:HP Amp Control 3, Set HP Depop Mode to 3 and control ON
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x90 -w "0x07 0x37"
	ret = regmap_write(rt5640->regmap, 0x90, 0x0737);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-37h: M AMP INT REG2
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x37"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0037);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x1C 0x00"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x1c00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-8Eh: HP Amp Control 1, Charge Pump Power Control OFF, Power On Soft Generator
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x8E -w "0x00 0x05"
	ret = regmap_write(rt5640->regmap, 0x8e, 0x0005);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#PR-24h: Charge pump int reg1
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6A -w "0x00 0x24"
	ret = regmap_write(rt5640->regmap, 0x6a, 0x0024);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x6C -w "0x04 0x20"
	ret = regmap_write(rt5640->regmap, 0x6c, 0x0420);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //sleep 0.01
    usleep_range(100, 101);
    
    //#MX-02h: Headphone Output Control, Unmute Control for Left and Right Headphone Output Ports
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x02 -w "0x48 0x48"
	ret = regmap_write(rt5640->regmap, 0x02, 0x4848);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    //#Playback registers end
    
    //#Capture registers start
    //#MX-62h: Power Management Control 2, Stereo ADC Digital Filter Poweron +i2s
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x62 -w "0x80 0x00"
	ret = regmap_write(rt5640->regmap, 0x62, 0x8000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-64h: Power Management Control 4, MICBIAS1 MIC BST1 and MIC BST2 Power ON -mic specific
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x64 -w "0x98 0x00"
	ret = regmap_write(rt5640->regmap, 0x64, 0x9800);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-65h: Power Management Control 5, RECMIXL & RECMIXR Power ON -on the mic points
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x65 -w "0x0C 0x00"
	ret = regmap_write(rt5640->regmap, 0x65, 0x0c00);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-3Ch: RECMIXL Control 2
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x3C -w "0x00 0x6F"
	ret = regmap_write(rt5640->regmap, 0x3c, 0x006f);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-3Dh: MICBST1 to RECMIXL
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x3D -w "0x00 0x7D"
	ret = regmap_write(rt5640->regmap, 0x3d, 0x007d);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-3Eh: MICBST2 to RECMIXR
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x3E -w "0x00 0x6F"
	ret = regmap_write(rt5640->regmap, 0x3e, 0x006f);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-27h: Stereo ADC Digital Mixer Control, Unmute ADC L/R channels
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x27 -w "0x30 0x20"
	ret = regmap_write(rt5640->regmap, 0x27, 0x3020);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-0Dh: IN1 Input Control Differential Mode enabled
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x0D -w "0x80 0x00" #0x8080 for Differential MIC on board
	ret = regmap_write(rt5640->regmap, 0x0d, 0x8000);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-0Eh: IN2 Input Control Differential Mode enabled
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x0E -w "0x08 0x00" #0840 - DIFF
	ret = regmap_write(rt5640->regmap, 0x0e, 0x0800);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);
    
    //#MX-73h: ADC/DAC Clock Control 1, ADC Over sample rate to 64Fs
    //i2ccmd -d /dev/i2c-6 -a 0x1c -o 0x73 -w "0x01 0x15"
	ret = regmap_write(rt5640->regmap, 0x73, 0x0115);
	dev_err(dev, "------------------------ %d, ret: %d\n", __LINE__, ret);

	INIT_DELAYED_WORK(&rt5640->fault_check_work, rt5640_fault_check_work);
	//schedule_delayed_work(&rt5640->fault_check_work, msecs_to_jiffies(RT5640_FAULT_CHECK_INTERVAL));

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
