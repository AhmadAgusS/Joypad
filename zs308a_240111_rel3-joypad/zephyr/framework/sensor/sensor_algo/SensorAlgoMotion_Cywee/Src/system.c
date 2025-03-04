/*******************************************************************************
 * @file    system.c
 * @author  MEMS Application Team
 * @version V1.0
 * @date    2021-5-25
 * @brief   sensor algorithm data
*******************************************************************************/

/******************************************************************************/
//includes
/******************************************************************************/
#include <stdint.h>

/******************************************************************************/
//constants
/******************************************************************************/
 /*mbrec mem layout*/
 /*
 0x1000000-0x1000200 //sram 1KB save boot_info(0x20) +NANDID
 0x1000020-0x1000400  // NANDID
 */
#define BOOT_INFO_SRAM_ADDR	0x1000000

/******************************************************************************/
//typedef
/******************************************************************************/
/*!
 * struct boot_info_t
 * @brief The boot infomation that transfer from bootloader to system.
 */
typedef struct {
	uint32_t mbrec_phy_addr; /* The physical address of MBREC that current system running */
	uint32_t param_phy_addr; /* The physical address of PARAM that current system using */
	uint32_t system_phy_addr; /* The physical address of SYSTEM that current using */
	uint32_t reboot_reason; /* Reboot reason */
	uint32_t nand_id_offs; /* nand id table offset */
	uint32_t nand_id_len; /* nand id table length */
	uint32_t watchdog_reboot : 1; /* The reboot event that occured from bootrom watchdog expired */
	uint32_t is_mirror : 1; /* The indicator of launching system is a mirror partition */
} boot_info_t;


/******************************************************************************/
//function
/******************************************************************************/
void printk(const char *fmt, ...)
{
	(void)(fmt);
}

const boot_info_t *soc_boot_get_info(void)
{
	return (const boot_info_t *)BOOT_INFO_SRAM_ADDR;
}

uint32_t sys_read32(unsigned int addr)
{
    return *(volatile unsigned int *)addr;
}

void z_impl_k_busy_wait(unsigned int usec_to_wait)
{
    unsigned int cycles_to_wait, current_cycles, start_cycles;
    start_cycles = sys_read32(0x4000C100+0x48); //  //sys_read32(T2_CNT);
    cycles_to_wait = usec_to_wait*32;
    for (;;) {
        current_cycles = sys_read32(0x4000C100+0x48);
        /* this handles the rollover on an unsigned 32-bit value */
        if ((current_cycles - start_cycles) >= cycles_to_wait) {
            break;
        }
    }
}
