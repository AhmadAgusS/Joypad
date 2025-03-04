/*
 * Copyright (c) 2017 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief SPICACHE profile interface for Actions SoC
 */

#include <errno.h>
#include <kernel.h>
#include "soc.h"
#include "spicache.h"

#define SPI1_CACHE_SIZE (1024*32)


#ifdef CONFIG_SOC_SPICACHE_PROFILE
static void spicache_update_profile_data(struct spicache_profile *profile)
{
	if (!profile->spi_id) {
		profile->hit_cnt = sys_read32(SPICACHE_RANGE_ADDR_HIT_COUNT);
		profile->miss_cnt = sys_read32(SPICACHE_RANGE_ADDR_MISS_COUNT);
		profile->total_hit_cnt = sys_read32(SPICACHE_TOTAL_HIT_COUNT);
		profile->total_miss_cnt = sys_read32(SPICACHE_TOTAL_MISS_COUNT);
	} else {
		profile->hit_cnt = sys_read32(SPI1_CACHE_M4F_HIT_COUNT);
		profile->miss_cnt = sys_read32(SPI1_CACHE_M4F_MISS_COUNT);
		profile->dma_hit_cnt = sys_read32(SPI1_CACHE_DMA_HIT_COUNT);
		profile->dma_miss_cnt = sys_read32(SPI1_CACHE_DMA_MISS_COUNT);
	}
}

int spicache_profile_get_data(struct spicache_profile *profile)
{
	if (!profile)
		return -EINVAL;

	spicache_update_profile_data(profile);

	return 0;
}

int spicache_profile_stop(struct spicache_profile *profile)
{
	if (!profile)
		return -EINVAL;

	profile->end_time = k_cycle_get_32();
	spicache_update_profile_data(profile);

	if (!profile->spi_id)
		sys_write32(sys_read32(SPICACHE_CTL) & ~(1 << 4), SPICACHE_CTL);
	else
		sys_write32(sys_read32(SPI1_CACHE_CTL) & ~(1 << 4), SPI1_CACHE_CTL);

	return 0;
}

int spicache_profile_start(struct spicache_profile *profile)
{
	if (!profile)
		return -EINVAL;

	if (!profile->spi_id) {
		sys_write32(profile->start_addr, SPICACHE_PROFILE_ADDR_START);
		sys_write32(profile->end_addr, SPICACHE_PROFILE_ADDR_END);
	} else {
		sys_write32(profile->start_addr, SPI1_CACHE_PROFILE_ADDR_START);
		sys_write32(profile->end_addr, SPI1_CACHE_PROFILE_ADDR_END);
	}

	profile->start_time = k_cycle_get_32();

	if (!profile->spi_id)
		sys_write32(sys_read32(SPICACHE_CTL) | (1 << 4), SPICACHE_CTL);
	else
		sys_write32(sys_read32(SPI1_CACHE_CTL) | (1 << 4), SPI1_CACHE_CTL);

	return 0;
}
#endif

void spi1_cache_ops_wait_finshed(void)
{
	while(sys_test_bit(SPI1_CACHE_OPERATE, 0));

}

void spi1_cache_ops(SPI_CACHE_OPS ops, void* addr, int size)
{
	uint32_t end;
	uint32_t off = (uint32_t)addr;
	uint32_t op;

	if(off < SPI1_BASE_ADDR)
		return;

	spi1_cache_ops_wait_finshed();

	switch(ops){
		case SPI_CACHE_FLUSH:
			if(size >= SPI1_CACHE_SIZE){
				sys_write32(0x11, SPI1_CACHE_OPERATE);
				return;
			}else{
				op = 0x15; // address mode
			}
			break;
		case SPI_CACHE_INVALIDATE:
			op = 0x0d; // invalid // address mode
			break;
		case SPI_WRITEBUF_FLUSH:
			sys_write32(0x1f, SPI1_CACHE_OPERATE);
			spi1_cache_ops_wait_finshed();
			return;
		case SPI_CACHE_FLUSH_ALL://flush all
			sys_write32(0x11, SPI1_CACHE_OPERATE);
			return ;
		case SPI_CACHE_INVALID_ALL: //invalid all
			sys_write32(0x9, SPI1_CACHE_OPERATE);
			spi1_cache_ops_wait_finshed();
			return ;
		case SPI_CACHE_FLUSH_INVALID:
			if(size >= SPI1_CACHE_SIZE){  //flush and invalid all
				sys_write32(0x19, SPI1_CACHE_OPERATE);
				//spi1_cache_ops_wait_finshed();
				return;
			}else{
				op = 0x1d; // flush & invalid // address mode
			}
			break;
		case SPI_CACHE_FLUSH_INVALID_ALL:
			sys_write32(0x19, SPI1_CACHE_OPERATE); //flush and invalid all
			//spi1_cache_ops_wait_finshed();
			return ;
		default:
			return;
	}

	/*address mode operate*/
	off = (off - SPI1_BASE_ADDR);
	end = off + size;
	off &= (~0x1f);

	/*not flush all*/
	while(off < end) {
		sys_write32(op|off, SPI1_CACHE_OPERATE);
		off+= 0x20;
		//while(sys_test_bit(SPI1_CACHE_OPERATE, 0));
		spi1_cache_ops_wait_finshed();
	}

}


void * cache_to_uncache(void *vaddr)
{
	void *pvadr = NULL;
	if(buf_is_nor(vaddr)){
		pvadr = (void *) (SPI0_UNCACHE_ADDR + (((uint32_t)vaddr) - SPI0_BASE_ADDR));
	}else if (buf_is_psram(vaddr)){
		pvadr = (void *) (SPI1_UNCACHE_ADDR + (((uint32_t)vaddr) - SPI1_BASE_ADDR));
	}
	return pvadr;
}

void * uncache_to_cache(void *paddr)
{
	void *vaddr = NULL;
	if(buf_is_nor_un(paddr))
		vaddr = (void *) (SPI0_BASE_ADDR + (((uint32_t)paddr) - SPI0_UNCACHE_ADDR));

	else if (buf_is_psram_un(paddr)){
		vaddr = (void *) (SPI1_BASE_ADDR + (((uint32_t)paddr) - SPI1_UNCACHE_ADDR));
	}

	return vaddr;

}




