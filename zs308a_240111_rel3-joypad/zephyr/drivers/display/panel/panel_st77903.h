/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef PANEL_ST77903_DRIVER_H__
#define PANEL_ST77903_DRIVER_H__

#define ST77903_CMD_NOP				0x00
#define ST77903_CMD_SWRESET			0x01 /* Software Reset */
#define ST77903_CMD_RDDID			0x04 /* Read Display ID */
#define ST77903_CMD_RDDST			0x09 /* Read Display Status */
#define ST77903_CMD_RDDPM			0x0A /* Read Display Power Mode */
#define ST77903_CMD_RDDMADCTL		0x0B /* Read Display MADCTL */
#define ST77903_CMD_RDDCOLMOD		0x0C /* Read Display Pixel Format */
#define ST77903_CMD_RDDIM			0x0D /* Read Display Image Mode */
#define ST77903_CMD_RDDSM			0x0E /* Read Display Signal Mode */
#define ST77903_CMD_RDDSDR			0x0F /* Read Display Self-Diagnostic Result */

#define ST77903_CMD_SLPIN			0x10 /* Sleep In */
#define ST77903_CMD_SLPOUT			0x11 /* Sleep Out */
#define ST77903_CMD_INVOFF			0x20 /* Display Inversion Off */
#define ST77903_CMD_INVON			0x21 /* Display Inversion On */
#define ST77903_CMD_DISPOFF			0x28 /* Display Off */
#define ST77903_CMD_DISPON			0x29 /* Display On */

//#define ST77903_CMD_CASET			0x2A /* Set Column Start Address */
//#define ST77903_CMD_RASET			0x2B /* Set Row Start Address */
#define ST77903_CMD_RAMWR			0x2C /* Memory Write */
#define ST77903_CMD_RAMWRC			0x3C /* Memory Continuous Write */

#define ST77903_CMD_TEOFF			0x34 /* Tearing Effect Line OFF */
#define ST77903_CMD_TEON			0x35 /* Tearing Effect Line ON */
#define ST77903_CMD_MADCTL			0x36 /* Memory Data Access Control */
#define ST77903_CMD_IDMOFF			0x38 /* Idle Mode Off */
#define ST77903_CMD_IDMON			0x39 /* Idle Mode On */
#define ST77903_CMD_COLMOD			0x3A /* Interface Pixel Format */

#define ST77903_CMD_STE				0x44 /* Set Tear Scanline */
#define ST77903_CMD_TESLRD			0x45 /* Read Scanline */
#define ST77903_CMD_HS				0x60 /* Horizontal SYNC Command */
#define ST77903_CMD_VS				0x61 /* Vertical SYNC Command */

#define ST77903_CMD_WRIDMC			0x90 /* Write two-color idle Mode color */
#define ST77903_CMD_RDIDMC			0x91 /* Read two-color idle Mode color */

#define ST77903_CMD_CK				0xF0 /* Command Key */
#define ST77903_CMD_ECFC			0xB0 /* Entry Code Function Control */
#define ST77903_CMD_FRC1			0xB1 /* Frame Rate Control 1 */
#define ST77903_CMD_GSC				0xB2 /* Gate Scan Control */
#define ST77903_CMD_VDMDC			0xB3 /* Video Mode Display Control */
#define ST77903_CMD_TCMDC			0xB4 /* Two color Mode Display Control */
#define ST77903_CMD_BPC				0xB5 /* Blank Porch Control */
#define ST77903_CMD_DISCN			0xB6 /* Display Function Control */
#define ST77903_CMD_EMSET			0xB7 /* Entry Mode Set */

#define ST77903_CMD_PWR				0xC0 /* Power Control */
#define ST77903_CMD_PWR1			0xC1 /* Power Control 1 */
#define ST77903_CMD_PWR2			0xC2 /* Power Control 2 */
#define ST77903_CMD_PWR3			0xC3 /* Power Control 3 */
#define ST77903_CMD_VCOMCTL			0xC5 /* Vcom Control */
#define ST77903_CMD_VMF1OFS			0xD6 /* Vcom Offset 1 */
#define ST77903_CMD_VMF2OFS			0xD7 /* Vcom Offset 2 */

#define ST77903_CMD_PGC				0xE0 /* Positive Gamma Control */
#define ST77903_CMD_NGC				0xE1 /* Negative Gamma Control */
#define ST77903_CMD_ANAMODE			0xE5 /* Analog System Control */

#define ST77903_CMD_DTRCON			0xD9 /* Dithering Control */
#define ST77903_CMD_SRECON			0xDE /* SRE Control */
#define ST77903_CMD_RLCMODE			0xC8 /* Run-length Control */

#define ST77903_CMD_RGBIF			0xA0 /* RGB Interface Control */
#define ST77903_CMD_RDFCS			0xAA /* Read First Checksum */
#define ST77903_CMD_RDCCS			0xAF /* Read Continue Checksum */

#define ST77903_CMD_RDID1			0xDA /* Read ID1 Code */
#define ST77903_CMD_RDID2			0xDB /* Read ID2 Code */
#define ST77903_CMD_RDID3			0xDC /* Read ID3 Code */

#define ST77903_RD_CMD(cmd)			((0xDD << 24) | ((uint32_t)(cmd) << 8))
#define ST77903_WR_CMD(cmd)			((0xDE << 24) | ((uint32_t)(cmd) << 8))

#endif /* PANEL_ST77903_DRIVER_H__ */
