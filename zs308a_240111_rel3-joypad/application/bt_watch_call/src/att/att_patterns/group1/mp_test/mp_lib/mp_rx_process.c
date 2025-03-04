#include "ap_autotest_rf_rx.h"
#include "mp_lark.h"


void mp_rx_packet_dealwith(struct mp_test_stru *mp_manager, unsigned int packet_counter)
{
    int8 x;
    int16 y;
    uint8 i,j;
    int32 rssi_avg = 0;
    int32 cfo_avg = 0;
    int16 cfo_real = 0;
    uint8 index = mp_manager->mp_valid_packets_cn % CFO_RESULT_NUM;

    for(i=0; i< packet_counter;i++)
    {
        for(j=1; j< packet_counter -i;j++)
        {
            if(mp_manager->rssi_tmp[j-1] > mp_manager->rssi_tmp[j])
            {
                x = mp_manager->rssi_tmp[j];
                mp_manager->rssi_tmp[j]=mp_manager->rssi_tmp[j-1];
                mp_manager->rssi_tmp[j-1]=x;
            }

            if(mp_manager->cfo_tmp[j-1] > mp_manager->cfo_tmp[j])
            {
                y = mp_manager->cfo_tmp[j];
                mp_manager->cfo_tmp[j] = mp_manager->cfo_tmp[j-1];
                mp_manager->cfo_tmp[j-1] = y;
            }
        }
    }

    for( i = 1 ; i < (packet_counter - 1) ; i++)
    {
        rssi_avg += mp_manager->rssi_tmp[i];
        cfo_avg += mp_manager->cfo_tmp[i];

		LOG_I("rssi %d avg %d\n", mp_manager->rssi_tmp[i], rssi_avg);
    }

    mp_manager->rssi_val[index] = rssi_avg / (packet_counter - 2);

    cfo_real = (int16)(cfo_avg / (packet_counter - 2));

	mp_manager->cfo_val[index] = cfo_real;

    mp_manager->mp_valid_packets_cn++;
    LOG_I("----rssi v:%d cfo v :%d---- idx %d\n",mp_manager->rssi_val[index],mp_manager->cfo_val[index], index);

}


int mp_analyse_cfo_result(struct mp_test_stru *mp_manager, int16_t cfo, int16_t rssi, unsigned int packet_counter)
{
	int index;

	index = mp_manager->mp_packets_cn;
	index %= packet_counter;

	mp_manager->rssi_tmp[index] = rssi;
	mp_manager->cfo_tmp[index] = cfo;
	mp_manager->mp_packets_cn++;

	LOG_D("recv pkg cfo %d rssi %d\n", cfo, rssi);

	if((mp_manager->mp_packets_cn % packet_counter) == 0){
		mp_rx_packet_dealwith(mp_manager, packet_counter);
		if(mp_manager->mp_valid_packets_cn == CFO_RESULT_NUM){
			mp_manager->mp_valid_packets_cn = 0;
			if(mp_manager->mp_rx_data_ready == 0){
				mp_manager->mp_rx_data_ready = 1;
			}
		}

		return 0;
	}

	return -1;

}

int mp_bt_packet_receive_start(void)
{
	return mp_btc_rx_begin();
}

int mp_bt_packet_receive_stop(void)
{
	mp_btc_rx_stop();

	udelay(100);

	mp_btc_clear_hci_rx_buffer();

	return 0;
}

int mp_bt_packet_receive_process(unsigned int channel, unsigned int packet_counter)
{
	int ret;
	uint16_t cfo_data[2];
	struct mp_test_stru *mp_test = &g_mp_test;

	u32_t cur_time = k_uptime_get();


	while(1){
		ret = mp_btc_read_hci_data((uint8_t *)&cfo_data, sizeof(cfo_data));

		if(ret == sizeof(cfo_data)){
			ret = mp_analyse_cfo_result(mp_test, cfo_data[0], cfo_data[1], packet_counter);

			if(ret == 0){
				break;
			}
		}

		if((k_uptime_get() - cur_time) > 1000){
			att_buf_printf("recv pkg timeout %d...\n", (k_uptime_get() - cur_time));
			cur_time = k_uptime_get();
		}
	}

	return ret;
}


bool att_bttool_rx_begin(u8_t channel)
{
	mp_btc_rx_init(channel);

	return true;
}

bool att_bttool_rx_stop(void)
{
	mp_btc_rx_stop();

	return true;
}

int att_bttool_rx_report(mp_rx_report_t *rx_report)
{
	int ret;

	ret = mp_btc_read_hci_data((uint8_t *)rx_report, sizeof(mp_rx_report_t));

	if(ret == sizeof(mp_rx_report_t)){
		return ret;
	}

	return 0;

}

int mp_bt_rx_report(mp_rx_report_t *rx_report)
{
	att_bttool_rx_stop();

	mp_btc_get_report();

	return att_bttool_rx_report(rx_report);
}





