/*
  dm9000.c: Version 1.2 12/15/2003

	A Davicom DM9000 ISA NIC fast Ethernet driver for Linux.
	Copyright (C) 1997  Sten Wang

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

  (C)Copyright 1997-1998 DAVICOM Semiconductor,Inc. All Rights Reserved.

V0.11	06/20/2001	REG_0A bit3=1, default enable BP with DA match
	06/22/2001 	Support DM9801 progrmming
	 	 	E3: R25 = ((R24 + NF) & 0x00ff) | 0xf000
		 	E4: R25 = ((R24 + NF) & 0x00ff) | 0xc200
     		R17 = (R17 & 0xfff0) | NF + 3
		 	E5: R25 = ((R24 + NF - 3) & 0x00ff) | 0xc200
     		R17 = (R17 & 0xfff0) | NF

v1.00               	modify by simon 2001.9.5
	                change for kernel 2.4.x

v1.1   11/09/2001      	fix force mode bug

v1.2   03/18/2003       Weilun Huang <weilun_huang@davicom.com.tw>:
			Fixed phy reset.
			Added tx/rx 32 bit mode.
			Cleaned up for kernel merge.

--------------------------------------

       12/15/2003       Initial port to u-boot by Sascha Hauer <saschahauer@web.de>

TODO: Homerun NIC and longrun NIC are not functional, only internal at the
      moment.
*/

#include <common.h>
#include <command.h>
#include <driver.h>
#include <clock.h>
#include <miiphy.h>
#include <net.h>
#include <init.h>
#include <asm/io.h>

#include "dm9000.h"

/* Board/System/Debug information/definition ---------------- */

#define DM9801_NOISE_FLOOR	0x08
#define DM9802_NOISE_FLOOR	0x05

/* #define CONFIG_DM9000_DEBUG */

#ifdef CONFIG_DM9000_DEBUG
#define DM9000_DBG(fmt,args...) printf(fmt ,##args)
#else				/*  */
#define DM9000_DBG(fmt,args...)
#endif				/*  */

/* DM9000 network board routine ---------------------------- */

#define DM9000_outb(d,r) ( *(volatile u8 *)r = d )
#define DM9000_outw(d,r) ( *(volatile u16 *)r = d )
#define DM9000_outl(d,r) ( *(volatile u32 *)r = d )
#define DM9000_inb(r) (*(volatile u8 *)r)
#define DM9000_inw(r) (*(volatile u16 *)r)
#define DM9000_inl(r) (*(volatile u32 *)r)

#ifdef CONFIG_DM9000_DEBUG
static void
dump_regs(void)
{
	DM9000_DBG("\n");
	DM9000_DBG("NCR   (0x00): %02x\n", DM9000_ior(0));
	DM9000_DBG("NSR   (0x01): %02x\n", DM9000_ior(1));
	DM9000_DBG("TCR   (0x02): %02x\n", DM9000_ior(2));
	DM9000_DBG("TSRI  (0x03): %02x\n", DM9000_ior(3));
	DM9000_DBG("TSRII (0x04): %02x\n", DM9000_ior(4));
	DM9000_DBG("RCR   (0x05): %02x\n", DM9000_ior(5));
	DM9000_DBG("RSR   (0x06): %02x\n", DM9000_ior(6));
	DM9000_DBG("ISR   (0xFE): %02x\n", DM9000_ior(ISR));
	DM9000_DBG("\n");
}
#endif				/*  */

static u8 DM9000_ior(int reg)
{
	DM9000_outb(reg, DM9000_IO);
	return DM9000_inb(DM9000_DATA);
}

static void DM9000_iow(int reg, u8 value)
{
	DM9000_outb(reg, DM9000_IO);
	DM9000_outb(value, DM9000_DATA);
}

static u16 phy_read(int reg)
{
	u16 val;

	/* Fill the phyxcer register into REG_0C */
	DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);
	DM9000_iow(DM9000_EPCR, 0xc);	/* Issue phyxcer read command */
	udelay(100);		/* Wait read complete */
	DM9000_iow(DM9000_EPCR, 0x0);	/* Clear phyxcer read command */
	val = (DM9000_ior(DM9000_EPDRH) << 8) | DM9000_ior(DM9000_EPDRL);

	/* The read data keeps on REG_0D & REG_0E */
	DM9000_DBG("phy_read(%d): %d\n", reg, val);
	return val;
}

static void phy_write(int reg, u16 value)
{

	/* Fill the phyxcer register into REG_0C */
	DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	DM9000_iow(DM9000_EPDRL, (value & 0xff));
	DM9000_iow(DM9000_EPDRH, ((value >> 8) & 0xff));
	DM9000_iow(DM9000_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);		/* Wait write complete */
	DM9000_iow(DM9000_EPCR, 0x0);	/* Clear phyxcer write command */
	DM9000_DBG("phy_write(reg:%d, value:%d)\n", reg, value);
}

int dm9000_check_id(void)
{
	u32 id_val;
	id_val = DM9000_ior(DM9000_VIDL);
	id_val |= DM9000_ior(DM9000_VIDH) << 8;
	id_val |= DM9000_ior(DM9000_PIDL) << 16;
	id_val |= DM9000_ior(DM9000_PIDH) << 24;
	if (id_val == DM9000_ID) {
		printf("dm9000 i/o: 0x%x, id: 0x%x \n", CONFIG_DM9000_BASE,
		       id_val);
		return 0;
	} else {
		printf("dm9000 not found at 0x%08x id: 0x%08x\n",
		       CONFIG_DM9000_BASE, id_val);
		return -1;
	}
}

static void dm9000_reset(void)
{
	DM9000_DBG("resetting\n");
	DM9000_iow(DM9000_NCR, NCR_RST);
	udelay(1000);		/* delay 1ms */
}

int dm9000_probe(struct device_d *dev)
{
//        struct eth_device *ndev = dev->type_data;

	printf("dm9000_eth_init()\n");

	/* RESET device */
	dm9000_reset();
	dm9000_check_id();

	/* Program operating register */
	DM9000_iow(DM9000_NCR, 0x0);	/* only intern phy supported by now */
	DM9000_iow(DM9000_TCR, 0);	/* TX Polling clear */
	DM9000_iow(DM9000_BPTR, 0x3f);	/* Less 3Kb, 200us */
	DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));	/* Flow Control : High/Low Water */
	DM9000_iow(DM9000_FCR, 0x0);	/* SH FIXME: This looks strange! Flow Control */
	DM9000_iow(DM9000_SMCR, 0);	/* Special Mode */
	DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);	/* clear TX status */
	DM9000_iow(DM9000_ISR, 0x0f);	/* Clear interrupt status */

        return 0;
}


int dm9000_eth_open(struct eth_device *ndev)
{
        int lnk, i = 0, ctl;

	/* Activate DM9000 */
	DM9000_iow(DM9000_GPCR, 0x01);	/* Let GPIO0 output */
	DM9000_iow(DM9000_GPR, 0x00);	/* Enable PHY */
	DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);	/* RX enable */
	DM9000_iow(DM9000_IMR, IMR_PAR);	/* Enable TX/RX interrupt mask */

        phy_write(0, 0x8000);	/* PHY RESET */

	ctl = phy_read(PHY_BMCR);

	if (ctl < 0)
		return ctl;

	ctl |= (PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);

	/* Don't isolate the PHY if we're negotiating */
	ctl &= ~(PHY_BMCR_ISO);

	phy_write(PHY_BMCR, ctl);

        while (!(phy_read(1) & 0x20)) {	/* autonegation complete bit */
		udelay(1000);
		i++;
		if (i == 5000) {
			printf("could not establish link\n");
                        break;
		}
	}

	/* see what we've got */
	lnk = phy_read(17) >> 12;
	printf("operating at ");
	switch (lnk) {
	case 1:
		printf("10M half duplex ");
		break;
	case 2:
		printf("10M full duplex ");
		break;
	case 4:
		printf("100M half duplex ");
		break;
	case 8:
		printf("100M full duplex ");
		break;
	default:
		printf("unknown: %d ", lnk);
		break;
	}
	printf("mode\n");
	return 0;
}

int dm9000_eth_send (struct eth_device *ndev, volatile void *packet, int length)
{
	char *data_ptr;
	u32 tmplen, i;
	uint64_t tmo;
	DM9000_DBG("eth_send: length: %d\n", length);
	for (i = 0; i < length; i++) {
		if (i % 8 == 0)
			DM9000_DBG("\nSend: 02x: ", i);
		DM9000_DBG("%02x ", ((unsigned char *) packet)[i]);
	} DM9000_DBG("\n");

	/* Move data to DM9000 TX RAM */
	data_ptr = (char *) packet;
	DM9000_outb(DM9000_MWCMD, DM9000_IO);

#ifdef CONFIG_DM9000_USE_8BIT
	/* Byte mode */
	for (i = 0; i < length; i++)
		DM9000_outb((data_ptr[i] & 0xff), DM9000_DATA);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_16BIT
	tmplen = (length + 1) / 2;
	for (i = 0; i < tmplen; i++)
		DM9000_outw(((u16 *) data_ptr)[i], DM9000_DATA);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_32BIT
	tmplen = (length + 3) / 4;
	for (i = 0; i < tmplen; i++)
		DM9000_outl(((u32 *) data_ptr)[i], DM9000_DATA);

#endif				/*  */

	/* Set TX length to DM9000 */
	DM9000_iow(DM9000_TXPLL, length & 0xff);
	DM9000_iow(DM9000_TXPLH, (length >> 8) & 0xff);

	/* Issue TX polling command */
	DM9000_iow(DM9000_TCR, TCR_TXREQ);	/* Cleared after TX complete */

	/* wait for end of transmission */
	tmo = get_time_ns();
	while (DM9000_ior(DM9000_TCR) & TCR_TXREQ) {
		if (is_timeout(tmo, 5 * SECOND)) {
			printf("transmission timeout\n");
			break;
		}
	}
	DM9000_DBG("transmit done\n\n");
	return 0;
}

void dm9000_eth_halt (struct eth_device *ndev)
{
	printf("eth_halt\n");

	phy_write(0, 0x8000);	/* PHY RESET */
	DM9000_iow(DM9000_GPR, 0x01);	/* Power-Down PHY */
	DM9000_iow(DM9000_IMR, 0x80);	/* Disable all interrupt */
	DM9000_iow(DM9000_RCR, 0x00);	/* Disable RX */
}

int dm9000_eth_rx (struct eth_device *ndev)
{
	u8 rxbyte, *rdptr = (u8 *) NetRxPackets[0];
	u16 RxStatus, RxLen = 0;
	u32 tmplen, i;
#ifdef CONFIG_DM9000_USE_32BIT
	u32 tmpdata;
#endif

	/* Check packet ready or not */
	DM9000_ior(DM9000_MRCMDX);	/* Dummy read */
	rxbyte = DM9000_inb(DM9000_DATA);	/* Got most updated data */
	if (rxbyte == 0)
		return 0;

	/* Status check: this byte must be 0 or 1 */
	if (rxbyte > 1) {
		DM9000_iow(DM9000_RCR, 0x00);	/* Stop Device */
		DM9000_iow(DM9000_ISR, 0x80);	/* Stop INT request */
		DM9000_DBG("rx status check: %d\n", rxbyte);
	}
	DM9000_DBG("receiving packet\n");

	/* A packet ready now  & Get status/length */
	DM9000_outb(DM9000_MRCMD, DM9000_IO);

#ifdef CONFIG_DM9000_USE_8BIT
	RxStatus = DM9000_inb(DM9000_DATA) + (DM9000_inb(DM9000_DATA) << 8);
	RxLen = DM9000_inb(DM9000_DATA) + (DM9000_inb(DM9000_DATA) << 8);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_16BIT
	RxStatus = DM9000_inw(DM9000_DATA);
	RxLen = DM9000_inw(DM9000_DATA);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_32BIT
	tmpdata = DM9000_inl(DM9000_DATA);
	RxStatus = tmpdata;
	RxLen = tmpdata >> 16;

#endif				/*  */
	DM9000_DBG("rx status: 0x%04x rx len: %d\n", RxStatus, RxLen);

	/* Move data from DM9000 */
	/* Read received packet from RX SRAM */
#ifdef CONFIG_DM9000_USE_8BIT
	for (i = 0; i < RxLen; i++)
		rdptr[i] = DM9000_inb(DM9000_DATA);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_16BIT
	tmplen = (RxLen + 1) / 2;
	for (i = 0; i < tmplen; i++)
		((u16 *) rdptr)[i] = DM9000_inw(DM9000_DATA);

#endif				/*  */
#ifdef CONFIG_DM9000_USE_32BIT
	tmplen = (RxLen + 3) / 4;
	for (i = 0; i < tmplen; i++)
		((u32 *) rdptr)[i] = DM9000_inl(DM9000_DATA);

#endif				/*  */
	if ((RxStatus & 0xbf00) || (RxLen < 0x40)
	    || (RxLen > DM9000_PKT_MAX)) {
		if (RxStatus & 0x100) {
			printf("rx fifo error\n");
		}
		if (RxStatus & 0x200) {
			printf("rx crc error\n");
		}
		if (RxStatus & 0x8000) {
			printf("rx length error\n");
		}
		if (RxLen > DM9000_PKT_MAX) {
			printf("rx length too big\n");
			dm9000_reset();
		}
	} else {

		/* Pass to upper layer */
		DM9000_DBG("passing packet to upper layer\n");
		NetReceive(NetRxPackets[0], RxLen);
		return RxLen;
	}
	return 0;
}

static u16 read_srom_word(int offset)
{
	DM9000_iow(DM9000_EPAR, offset);
	DM9000_iow(DM9000_EPCR, 0x4);
	udelay(200);
	DM9000_iow(DM9000_EPCR, 0x0);
	return (DM9000_ior(DM9000_EPDRL) + (DM9000_ior(DM9000_EPDRH) << 8));
}

static int dm9000_get_mac_address(struct eth_device *eth, unsigned char *adr)
{
	int i;

        for (i = 0; i < 3; i++)
		((u16 *) adr)[i] = read_srom_word(i);

        return 0;
}

static int dm9000_set_mac_address(struct eth_device *eth, unsigned char *adr)
{
	int i, oft;
printf("dm9000_set_mac_address\n");
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		DM9000_iow(oft, adr[i]);
	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		DM9000_iow(oft, 0xff);

#if 0
	for (i = 0; i < 5; i++)
		printf ("%02x:", adr[i]);
	printf ("%02x\n", adr[5]);
#endif
	return -0;
}

struct eth_device dm9000_eth = {
	.open = dm9000_eth_open,
	.send = dm9000_eth_send,
	.recv = dm9000_eth_rx,
	.halt = dm9000_eth_halt,
	.get_mac_address = dm9000_get_mac_address,
	.set_mac_address = dm9000_set_mac_address,
};

static struct driver_d dm9000_driver = {
        .name  = "dm9000",
        .probe = dm9000_probe,
        .type  = DEVICE_TYPE_ETHER,
        .type_data = &dm9000_eth,
};

static int dm9000_init(void)
{
        register_driver(&dm9000_driver);
        return 0;
}

device_initcall(dm9000_init);
