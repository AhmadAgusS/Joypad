/*
 * Copyright (c) 2021 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Generate SPI NAND ID table binary.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "id_tbl.h"

#ifndef ID_TABLE_NAME
#define ID_TABLE_NAME "nand_id.bin"
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_array)  (sizeof(_array) / sizeof(_array[0]))
#endif

#define MAX_ID_TABLE_NUMBER (8)

//nand_id.bin list(7)
#define   GD5F1GQ5UEYIG
//#define   W25N01KVxxIR
//#define   MX35LF1GE4AB
//#define   DS35Q1GB
//#define   F35SQA001G
//#define   F50L1G41LB
//#define   FM25S01A

//ready to use list(9)
//#define W25N01GVZEIG
//#define W25N02KB
//#define W25N512GVxIG
//#define MX35LF2GE4AD
//#define MX35UF2GE4AD
#define DS35Q1GA
//#define F35SQA512M
//#define KANY1D4S2WD
//#define GD5F1GQ7UEYIG

static const struct FlashChipInfo FlashChipInfoTbl[] =
{
    /* =================================================================================== */
#ifdef GD5F1GQ5UEYIG
    /* GD5F1GQ5UEYIG 2KB PageSize, 128MB SPINand */
    {
        {0xc8, 0x51, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "GD5F1GQ5UEYIG",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef W25N01GVZEIG
    /* W25N01GVZEIG 2KB PageSize, 128MB SPINand */
    {
        {0xef, 0xaa, 0x21, 0x00, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "W25N01GVZEIG",
        11,                                                  /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef W25N01KVxxIR
    /* W25N01KVxxIR 2KB PageSize, 128MB SPINand */
    {
        {0xef, 0xae, 0x21, 0x00, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "W25N01KVxxIR",
        13,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef W25N02KB
    /* W25N02KB 2KB PageSize, 256MB SPINand */
    {
        {0xef, 0xaa, 0x22, 0x00, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        2048,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "W25N02KV",
        12,                                                  /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef W25N512GVxIG
    /* W25N512GVxIG/IT 2KB PageSize, 64MB SPINand */
    {
        {0xef, 0xaa, 0x20, 0x00, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        512,                                                /* BlkNumPerDie */
        492,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "W25N512GVxIG",
        10,                                                  /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef MX35LF1GE4AB
    /* MX35LF1GE4AB 2KB PageSize, 128MB SPINand */
    {
        {0xc2, 0x12, 0xc2, 0x12, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "MX35LF1GE4AB",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef MX35LF2GE4AD
    /* MX35LF2GE4AD 2KB PageSize, 256MB SPINand */
    {
        {0xc2, 0x26, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        2048,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "MX35LF2GE4AD",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef MX35UF2GE4AD
    /* MX35UF2GE4AD 2KB PageSize, 1.8V, 256MB SPINand */
    {
        {0xc2, 0xa6, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        2048,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "MX35UF2GE4AD",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef FM25S01A
    /* FM25S01A 2KB PageSize, 128MB SPINand */
    {
        {0xa1, 0xe4, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "FM25S01A",
        13,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef DS35Q1GA
    /* DS35Q1GA 2KB PageSize, 128MB SPINand */
    {
        {0xe5, 0x71, 0xe5, 0x71, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                                /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "DS35Q1GA",
        12,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef DS35Q1GB
    /* DS35Q1GB 2KB PageSize, 128MB SPINand */
    {
        {0xe5, 0xf1, 0xe5, 0xf1, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        32,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                                /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x70,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "DS35Q1GB",
        12,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef F35SQA001G
    /* F35SQA001G 2KB PageSize, 128MB SPINand */
    {
        {0xcd, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "F35SQA001G",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef F35SQA512M
    /* F35SQA512M 2KB PageSize, 64MB SPINand */
    {
        {0xcd, 0x70, 0x70, 0xcd, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        512,                                                /* BlkNumPerDie */
        492,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "F35SQA512M",
        10,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef F50L1G41LB
    /* F50L1G41LB 2KB PageSize, 128MB SPINand */
    {
        {0xc8, 0x01, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x10,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "F50L1G41LB",
        12,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef KANY1D4S2WD
    /* KANY1D4S2WD 2KB PageSize, 128MB SPINand */
    {
        {0x1, 0x15, 0x1, 0x15, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x20,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "KANY1D4S2WD",
        12,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef GD5F1GQ7UEYIG
    /* GD5F1GQ7UEYIG 2KB PageSize, 128MB SPINand */
    {
        {0xc8, 0x91, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        1,                                                  /* ECCBits */
        0x30,                                               /* eccStatusMask*/
        0x20,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "GD5F1GQ7UEYIG",
        12,                                                 /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
#ifdef XT26G01C
    /* XT26G01C 2KB PageSize, 128MB SPINand, Not support now! */
    {
        {0x0b, 0x11, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},   /* ChipID */
        1,                                                  /* ChipCnt */
        1,                                                  /* DieCntPerChip */
        1,                                                  /* PlaneCntPerDie */
        4,                                                  /* SectNumPerPPage */
        16,                                                 /* SpareBytesPerSector */
        90,                                                 /* Frequence */
        64,                                                 /* PageNumPerPBlk */
        1024,                                               /* BlkNumPerDie */
        984,                                                /* DefaultLBlkNumPer1024PBlk */
        2048,                                               /* userMetaOffset*/
        16,                                                 /* userMetaShift*/
        4,                                                  /* userMetaSize*/
        4,                                                  /* ECCBits */
        0xf0,                                               /* eccStatusMask*/
        0x80,                                               /* eccStatusErrVal*/
        16,                                                 /* readAddressBits*/
        /**
         * read mode:
         * 0x0:1xIO(0x03), cpu trans; 0x10:1xIO(0x03), dma trans;
         * 0x1:2xIO-quad(0xbb), cpu trans; 0x11:2xIO-quad(0xbb), dma trans;
         * 0x2:2xIO-dual(0x3b), cpu trans; 0x12:2xIO-dual(0x3b), dma trans;
         * 0x4:4xIO-quad(0x6b), cpu trans; 0x14:4xIO-quad(0x6b), dma trans;
         * 0x8:4xIO-dual(0xeb), cpu trans; 0x18:4xIO-dual(0xeb), dma trans. the most fast.
         */

        0x14,                                               /* readMode 0x10*/
        /**
         * write mode:
         * 0x0:1xIO(0x02), cpu trans; 0x10:1xIO(0x02), dma trans;
         * 0x4:4xIO-quad(0x32), cpu trans; 0x14:4xIO-quad(0x32), dma trans;
         */
        0x14,                                               /* writeMode 0x10*/
        FLASH_CHIP_INFO_VERSION,
        "XT26G01C",
        13,                                                  /* delaychain */
        {0},                                                /* Reserved, 12Bytes */
    },
#endif
};

uint32_t checksum(void *buffer, uint32_t len)
{
    uint32_t *pdat = buffer;
    uint32_t ck32 = 0;

    while (len > 0) {
        ck32 += *pdat++;
        len -= 4;
    }

    return ck32;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int ret, i;
    struct NandIdTblHeader TblHeader;

    uint32_t id_num = ARRAY_SIZE(FlashChipInfoTbl);

    fprintf(stdout, "Generate nand id table: %s\n", ID_TABLE_NAME);

    if ((id_num == 0) || (id_num > MAX_ID_TABLE_NUMBER)) {
        fprintf(stderr, "invalid flash id number %d", id_num);
        return -EINVAL;
    }

    TblHeader.num = id_num;
    //Notice: magic value is stabled and can't be changed!
    TblHeader.magic = 0x53648673;
    fprintf(stdout, "sizeof(chiptbl) = %ld\n", sizeof(FlashChipInfoTbl));
    TblHeader.checksum = checksum((void *)FlashChipInfoTbl, sizeof(FlashChipInfoTbl));

    for (i = 0; i < id_num; i++)
        fprintf(stdout, "[%d] chipid:0x%lx chipname:%s\n",
            i, *(uint64_t *)FlashChipInfoTbl[i].ChipID, FlashChipInfoTbl[i].FlashMark);

    fp = fopen(ID_TABLE_NAME, "wb");
    if (!fp) {
        fprintf(stderr, "failed to open id table:%s, error=%s", ID_TABLE_NAME, strerror(errno));
        return -errno;
    }

    ret = fwrite(&TblHeader, sizeof(TblHeader), 1, fp);
    ret = fwrite(FlashChipInfoTbl, sizeof(FlashChipInfoTbl), 1, fp);
    if (ret != 1) {
        fprintf(stderr, "write nand id table failed, error=%s", strerror(errno));
        fclose(fp);
        return -errno;
    }

    fclose(fp);

    return 0;
}
