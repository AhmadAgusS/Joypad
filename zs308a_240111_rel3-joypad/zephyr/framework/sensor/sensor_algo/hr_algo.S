/******************** (C) COPYRIGHT 2020 Actions-semi ********************
 * File Name          : hr_algo.s
 * Author             : Lark Team
 * Version            : V1.0
 * Date               : 2021-05-25
 * Description        : Include sensor algorithm bin. 
 *************************************************************************/
#include <autoconf.h>
                .section .hr_rom,"a"
				.align 12
				.global __hr_algo_start
				.global __hr_algo_end
__hr_algo_start:
#if defined(CONFIG_SENSOR_ALGO_HR_GH30X)
				.incbin "hr_gh30x.bin"
#elif defined(CONFIG_SENSOR_ALGO_HR_VCARE)
				.incbin "hr_ghVcare.bin"
#elif defined(CONFIG_SENSOR_ALGO_HR_HX3605)
				.incbin "hr_hx3605.bin"
#elif defined(CONFIG_SENSOR_ALGO_HR_HX3690)
				.incbin "hr_hx3690.bin"
#else
				.incbin "hr_gh30x.bin"
#endif
__hr_algo_end:
				.align 12
				.end
                PRESERVE8
                END
