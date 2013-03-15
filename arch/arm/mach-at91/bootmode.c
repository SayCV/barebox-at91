/*
 * Copyright (C) 2013 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * GPLv2 only
 */

#include <common.h>
#include <mach/bootmode.h>

struct at91_boot_mode at91_soc_boot_mode;

static const char *soc_boot_from[] = {
	[AT91_BOOT_FROM_SPI]		= "spi",
	[AT91_BOOT_FROM_MCI]		= "mci",
	[AT91_BOOT_FROM_SMC]		= "smc",
	[AT91_BOOT_FROM_TWD]		= "twd",
	[AT91_BOOT_FROM_UNKNOWN]	= "Unknown"
};

static const char *soc_boot_media[] = {
	[AT91_BOOT_MEDIA_SD]		= "SD",
	[AT91_BOOT_MEDIA_MMC]		= "MMC",
	[AT91_BOOT_MEDIA_EMMC]		= "eMMC",
	[AT91_BOOT_MEDIA_AT45]		= "at45",
	[AT91_BOOT_MEDIA_AT25]		= "at25",
	[AT91_BOOT_MEDIA_NAND]		= "NAND",
	[AT91_BOOT_MEDIA_NOR]		= "NOR",
	[AT91_BOOT_MEDIA_UNKNOWN]	= "Unknown"
};

void at91_bootmode_device_register(void)
{
	struct device_d *dev;
	const char *name;
	char str[16];

	dev = add_generic_device_res("bootloc", DEVICE_ID_SINGLE, NULL, 0, NULL);

	if (at91_soc_boot_mode.from > ARRAY_SIZE(soc_boot_from))
		name = soc_boot_from[AT91_BOOT_FROM_UNKNOWN];
	else
		name = soc_boot_from[at91_soc_boot_mode.from];

	dev_add_param_fixed(dev, "from", (char*)name);

	if (at91_soc_boot_mode.media > ARRAY_SIZE(soc_boot_media))
		name = soc_boot_media[AT91_BOOT_MEDIA_UNKNOWN];
	else
		name = soc_boot_media[at91_soc_boot_mode.media];

	dev_add_param_fixed(dev, "media", (char*)name);
	sprintf(str, "%d", at91_soc_boot_mode.interface);
	dev_add_param_fixed(dev, "interface", str);
	sprintf(str, "%d", at91_soc_boot_mode.cs);
	dev_add_param_fixed(dev, "cs", str);

}
