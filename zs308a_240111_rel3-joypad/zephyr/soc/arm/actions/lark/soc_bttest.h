/*
 * Copyright (c) 2021 Actions Semiconductor Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef	_ACTIONS_SOC_BTTEST_H_
#define	_ACTIONS_SOC_BTTEST_H_

#ifndef _ASMLANGUAGE

/**
 * @brief Run bt fcc test
 *
 * This function is used to run bt fcc test
 *
 * @param mode 0: uart test mode, for pcba
 *           1: bt test mode, for demo, only test bt tx power...
 *          other value: invalid
 *
 * @return 0 if succsess.
 * @return 1 if failed.
 *
 * @note: 
 *		if uart mode, baudrate is set to 115200
 *		if bt mode, use share memory to set param as below:
 *		TEST_MODE: 0x0106A000, 0: BR/EDR Test; 1: BLE Test;
 *		BLE_MODE:  0x0106A004, 0: BLE_1M; 1: BLE_2M;
 *		BT_Channel: 0x0106A008, 0~79;
 *		TX_Power: 0x0106A00C,  index in power table;
 *		TX_Mode: 0x0106A010, 9:DH1; 10:DH2_2M; 11:DH1_3M; 13:DH3; 14:2DH3; 15:3DH3; 16:DH5; 17:2DH5; 18: 3DH5; 19: SingleTone
 *		Payload_Mode: 0x0106A014, 0:PN9; 1:PN15; 2:all 0; 3:all 1; 4:00001111; 5:01010101; 6:0,1,2...0xFF
 *		Execute_Mode: 0x0106A018, 0:transmit one packet; 1: continue transmission
 */
extern int fcc_test_main(uint8_t mode);

#endif /* _ASMLANGUAGE */

#endif /* _ACTIONS_SOC_BTTEST_H_	*/
