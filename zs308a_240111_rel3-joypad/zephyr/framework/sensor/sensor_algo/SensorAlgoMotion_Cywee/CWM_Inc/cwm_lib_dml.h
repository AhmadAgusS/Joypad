#ifndef __CWM_LIB_DML_H__
#define __CWM_LIB_DML_H__

#define SCL_DML_DRV_ENABLE              400
#define SCL_DML_DRV_INIT                401
#define SCL_DML_DRV_HW_CONFIG           402
#define SCL_DML_DRV_AG_CONFIG           403
#define SCL_DML_DRV_M_CONFIG            404
#define SCL_DML_DEBUG                   405
#define SCL_DML_GET_LIB_INFO            406
#define SCL_DML_GET_INITED_LIST         407
#define SCL_DML_DRV_AG_STATUS           408
#define SCL_DML_DRV_ANYM_CONFIG         409
#define SCL_DML_DRV_AG_PERF_CONFIG      410
#define SCL_DML_DRV_M_PERF_CONFIG       411
#define SCL_DML_DRV_AG_EXT_CONFIG       412
#define SCL_DML_DRV_M_EXT_CONFIG        413
#define SCL_DML_DRV_AG_REG_ACCESS       414
#define SCL_DML_DRV_M_REG_ACCESS        415

#define SCL_DML_AUTO_RANGE_CONFIG       420

int CWM_Dml_LibInit(void);
int CWM_Dml_process(void);

#endif
