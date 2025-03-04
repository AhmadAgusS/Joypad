Please comply dsp_codec_bank with SDK version: 3.17.3

Demo Functionality:
1. Generate btMUSIC.dsp with Demo CEVA Project: dsp_codec_bank
(1) You'll be able to build dsp_codec_bank.a
(2) Double click on build_all_dsp.bat to conver dsp_codec_bank.a to adMUSIC.dsp and copy to destination.
(you might need to modify SDK_KDFS_DIR in the script file)

2. Copy btMUSIC.dsp to zephyr\samples\test_codec\fs_sdfs\sdfs_k (if you did step 1.(2), please skip this step.)

3. Build Firmware (using sample/test_codec)
(1) python build.py -t -n
(2) Board type: ats3085l_dvb_watch_ext_nor_max
(3) Application: test_codec
(4) Functionality of test_codec: 
There're mic_in.pcm and sco_in.pcm stored in disk, and they serve as input files.
Through adMUSIC.dsp, it will generate spk_out.pcm and sco_out.pcm.

4. Test Firmware built with test_codec:
(1) Power on EVB
(2) Short press on/off button to enter Engineer Mode. In this mode, EVB will start to do streaming thing. Might need to wait for a while.
(Don't go to next step until there's no message comming out like "[19:20:27.737]  [176][I][os][127*]:dec in stream len 530944, out stream len 336896")
(3) After seeing EVB is not doing streaming thing, please reboot EVB.
(4) You'll see spk_out.pcm and sco_out.pcm 

5. How you could do in your CEVA project:
	1. Decode
		static int pcm_decode(struct decoder_data *data) 
		{
			/* do your decoder here */
		}
	
	
	2. Encode
		static int pcm_encode(struct encoder_data *encoder_data)
		{
			/* do your encoder here */
	
		}

6. How you could do some modification on sample/test_codec, so that you could develop your application based on this sample code:
	Change:
	init_param.output_stream = spk_out_stream;
	To:
    init_param.output_stream = NULL, (which means sending output stream to DAC as default)

	Change:
	init_param.capture_input_stream = mic_in_stream;
    To:
    init_param.capture_input_stream = NULL, (which means receiving stream from ADC output as default)

	You'll need to handle bluetooth data flow (reference: stream of sco)