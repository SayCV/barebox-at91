/*
 * Copyright (C) 2013 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * GPLv2 only
 */

#ifndef __SAMA5D3_BOOTMODE_H__
#define __SAMA5D3_BOOTMODE_H__

#include <mach/sama5d3.h>

#define SAMA5D3_BOOT_FROM_GPBR		(SAMA5D3_BASE_GPBR + 4)

#define SAMA5D3_BOOT_FROM_KEY		(0xba << 24)

#define SAMA5D3_BOOT_FROM		(0xf << 0)
#define		SAMA5D3_BOOT_FROM_SPI		(0 << 0)
#define		SAMA5D3_BOOT_FROM_MCI		(1 << 0)
#define		SAMA5D3_BOOT_FROM_SMC		(2 << 0)
#define		SAMA5D3_BOOT_FROM_TWD		(3 << 0)

#define SAMA5D3_BOOT_FROM_INTERFACE	(0xf << 4)
#define 	SAMA5D3_BOOT_FROM_INTERFACE_SHIFT	4
#define		SAMA5D3_BOOT_FROM_INTERFACE_0	(0 << 4)
#define		SAMA5D3_BOOT_FROM_INTERFACE_1	(1 << 4)
#define		SAMA5D3_BOOT_FROM_INTERFACE_2	(2 << 4)
#define		SAMA5D3_BOOT_FROM_INTERFACE_3	(3 << 4)
#define		SAMA5D3_BOOT_FROM_INTERFACE_4	(4 << 4)

#define SAMA5D3_BOOT_FROM_MEDIA_TYPE	(0xf << 8)
#define		SAMA5D3_BOOT_FROM_MEDIA_SD	(0 << 8)
#define		SAMA5D3_BOOT_FROM_MEDIA_MMC	(1 << 8)
#define		SAMA5D3_BOOT_FROM_MEDIA_EMMC	(2 << 8)
#define		SAMA5D3_BOOT_FROM_MEDIA_AT25	(0 << 8)
#define		SAMA5D3_BOOT_FROM_MEDIA_AT45	(1 << 8)

#define SAMA5D3_BOOT_FROM_CS		(0xf << 12)
#define 	SAMA5D3_BOOT_FROM_CS_SHIFT	12
#define		SAMA5D3_BOOT_FROM_CS_0		(0 << 12)
#define		SAMA5D3_BOOT_FROM_CS_1		(1 << 12)
#define		SAMA5D3_BOOT_FROM_CS_2		(2 << 12)
#define		SAMA5D3_BOOT_FROM_CS_3		(3 << 12)
#define		SAMA5D3_BOOT_FROM_CS_4		(4 << 12)

#endif /* __SAMA5D3_BOOTMODE_H__ */
