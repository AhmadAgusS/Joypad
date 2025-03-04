/*
 * Copyright (c) 2017 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief SPICACHE profile interface for Actions SoC
 */

#ifndef __SPICACHE_H__
#define __SPICACHE_H__

#include <zephyr/types.h>

#define SPI0_BASE_ADDR 0x10000000
#define SPI0_BASE_END_ADDR 0x14000000
#define SPI0_UNCACHE_ADDR 0x14000000
#define SPI0_UNCACHE_END_ADDR 0x18000000


#define SPI1_BASE_ADDR 0x18000000
#define SPI1_BASE_END_ADDR 0x1C000000
#define SPI1_UNCACHE_ADDR 0x1C000000
#define SPI1_UNCACHE_END_ADDR 0x20000000


typedef enum __SPI_CACHE_OPS
{
    SPI_CACHE_FLUSH              = 0x01,
    SPI_CACHE_INVALIDATE         = 0x02,
    SPI_WRITEBUF_FLUSH           = 0x03,
    SPI_CACHE_FLUSH_ALL          = 0x04,
	SPI_CACHE_INVALID_ALL  	     = 0x05,
	SPI_CACHE_FLUSH_INVALID  	 = 0x06,
	SPI_CACHE_FLUSH_INVALID_ALL  = 0x07,

}SPI_CACHE_OPS;

#ifdef CONFIG_SOC_SPICACHE_PROFILE
struct spicache_profile {
	int spi_id;

	/* must aligned to 64-byte */
	uint32_t start_addr;
	uint32_t end_addr;
	uint32_t limit_addr;

	/* timestamp, ms */
	int64_t start_time;
	int64_t end_time;

	/* hit/miss in user address range */
	uint32_t hit_cnt;
	uint32_t miss_cnt;

	/* hit/miss in all address range */
	uint32_t total_hit_cnt;
	uint32_t total_miss_cnt;

	/* spi1 cache miss/hit count */
	uint32_t dma_hit_cnt;
	uint32_t dma_miss_cnt;
};

int spicache_profile_start(struct spicache_profile *profile);
int spicache_profile_stop(struct spicache_profile *profile);
int spicache_profile_get_data(struct spicache_profile *profile);

#endif

void spi1_cache_ops(SPI_CACHE_OPS ops, void* addr, int size);
void spi1_cache_ops_wait_finshed(void);

#define buf_is_psram(buf)  ((((uint32_t)buf) >= SPI1_BASE_ADDR) && (((uint32_t)buf) < SPI1_BASE_END_ADDR))
#define buf_is_psram_un(buf)  ((((uint32_t)buf) >= SPI1_UNCACHE_ADDR) && (((uint32_t)buf) < SPI1_UNCACHE_END_ADDR))

#define buf_is_nor(buf)  ((((uint32_t)buf) >= SPI0_BASE_ADDR) && (((uint32_t)buf) < SPI0_BASE_END_ADDR))
#define buf_is_nor_un(buf)  ((((uint32_t)buf) >= SPI0_UNCACHE_ADDR) && (((uint32_t)buf) < SPI0_UNCACHE_END_ADDR))


extern void * cache_to_uncache(void *vaddr);

extern void * uncache_to_cache(void *paddr);


#endif	/* __SPICACHE_H__ */
