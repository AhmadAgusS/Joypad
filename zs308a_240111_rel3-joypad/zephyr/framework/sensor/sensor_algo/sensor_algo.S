/******************** (C) COPYRIGHT 2020 Actions-semi ********************
 * File Name          : sensor_algo.s
 * Author             : Lark Team
 * Version            : V1.0
 * Date               : 2021-05-25
 * Description        : Include sensor algorithm bin. 
 *************************************************************************/
#include <autoconf.h>
                .section .sensor_rom,"a"
				.align 12
				.global __sensor_algo_start
				.global __sensor_algo_end
__sensor_algo_start:
#if defined(CONFIG_SENSOR_ALGO_MOTION_CYWEE)
#if CONFIG_SENSOR_RAM_CODE_SIZE > 0
				.incbin "motion_cywee_ramx.bin"
#else
				.incbin "motion_cywee.bin"
#endif
#elif defined(CONFIG_SENSOR_ALGO_MOTION_SILAN)
				.incbin "motion_silan.bin"
#else
				.incbin "motion_cywee.bin"
#endif
__sensor_algo_end:
				.align 12
				.end
                PRESERVE8
                END
