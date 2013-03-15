/*
 * Copyright (C) 2013 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * GPLv2 only
 */

#ifndef __BOOTMODE_H__
#define __BOOTMODE_H__

extern struct at91_boot_mode at91_soc_boot_mode;

struct at91_boot_mode {
	u16 from;
	u16 interface;
	u16 media;
	u16 cs;
};

enum at91_soc_boot_from {
	AT91_BOOT_FROM_UNKNOWN,
	AT91_BOOT_FROM_SPI,
	AT91_BOOT_FROM_MCI,
	AT91_BOOT_FROM_SMC,
	AT91_BOOT_FROM_TWD,
};

enum at91_soc_boot_media {
	AT91_BOOT_MEDIA_UNKNOWN,
	AT91_BOOT_MEDIA_SD,
	AT91_BOOT_MEDIA_MMC,
	AT91_BOOT_MEDIA_EMMC,
	AT91_BOOT_MEDIA_AT45,
	AT91_BOOT_MEDIA_AT25,
	AT91_BOOT_MEDIA_NAND,
	AT91_BOOT_MEDIA_NOR,
};

#ifdef CONFIG_AT91_BOOTMODE
void at91_bootmode_device_register(void);

#define at91_boot_from_spi()	(at91_soc_boot_mode.from == AT91_BOOT_FROM_SPI)
#define at91_boot_from_mci()	(at91_soc_boot_mode.from == AT91_BOOT_FROM_MCI)
#define at91_boot_from_smc()	(at91_soc_boot_mode.from == AT91_BOOT_FROM_SMC)
#define at91_boot_from_twd()	(at91_soc_boot_mode.from == AT91_BOOT_FROM_TWD)

#define at91_boot_media_sd()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_SD)
#define at91_boot_media_mmc()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_MMC)
#define at91_boot_media_emmc()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_EMMC)
#define at91_boot_media_at45()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_AT45)
#define at91_boot_media_at25()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_AT25)
#define at91_boot_media_nand()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_NAND)
#define at91_boot_media_nor()	(at91_soc_boot_mode.media == AT91_BOOT_MEDIA_NOR)
#else
static void inline void at91_bootmode_device_register(void) {}

#define at91_boot_from_spi()	(0)
#define at91_boot_from_mci()	(0)
#define at91_boot_from_smc()	(0)
#define at91_boot_from_twd()	(0)

#define at91_boot_media_sd()	(0)
#define at91_boot_media_mmc()	(0)
#define at91_boot_media_emmc()	(0)
#define at91_boot_media_at45()	(0)
#define at91_boot_media_at25()	(0)
#define at91_boot_media_nand()	(0)
#define at91_boot_media_nor()	(0)
#endif

#endif /* __BOOTMODE_H__ */
