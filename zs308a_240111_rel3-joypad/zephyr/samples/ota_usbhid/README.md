# README (USBHID OTA)
This is an APP demo for OTA Upgrade over USBHID.
* Author: Alice Hong
* Last update: 2024-11-15
* Compatibility:
    * SDK: SDK-TAG_ZS308A_2700_v**3.17.4**
    * Board: ATS3085L_DVB_V2.0, and ATS3085L_CORE_JOYPAD_EVB_KC_V02

## I. Environment Setup
### A. Set up HOST (PC)
#### 1. Software Environment Setup
* Operating System: Win 11
* Software setup: Python 3.12.3
* Python package installation:
    ```
    pip install pyusb
    ```
#### 2. Generate ota.bin
1. Build a firmware of this app whose version you plan to upgrade to(Let's call it `new.fw`)

    For exmaple, I built two version of this sample app, and only difference is that I left a sign "_OTA UPGRADE SUCCESSFULLY :3_" in newer version to better tell whether I upgrade successfully.

    ```
    zs308a_240111_rel3\zephyr>python build.py -t -n
    ```

2. Generate ota.bin with Config Tool.

    (1) Import your firmware file, i.e. the `new.fw` file.

    (2) Click ">>" icon, then select "OTA" option.

    (3) Choose the destination you would like to save to. To apply with python app `usbhid_ota_upgrade.py`, please name it as `ota.bin`. Otherwise, you need to change `ota_file_path` in `usbhid_ota_upgrade.py`.

    (4) Put the `ota.bin` in the same folder with `usbhid_ota_upgrade.py`.

### B. Set up 3085L DEVICE
#### Build firmware before your OTA upgrade
1. Please comply with SDK-TAG_ZS308A_2700_v**3.17.4**. And also notice that, might comment out "_OTA UPGRADE SUCCESSFULLY :3_", which is somehow different from `new.fw`.

2. Build this application by:
    ```
    zs308a_240111_rel3\zephyr>python build.py -t -n
    ```
    You'll get a firmware. Let's call it `old.fw`.

3. Select your 3085L board. In my case, I chose `6. ats3085l_dvb_watch_ext_nor_max` or `4. ats3085l_core_joystick_evb_kc`.

4. Select option `5. ota_usbhid`.
5. Program the result fw file(`old.fw`) into your 3085L device.

### C. Hardware Environment Setup
1. 3085L device is connected to the HOST over USB.
2. (Optional) Console port(debug) is connected to the HOST over UART0.

### D. Run python upgrade app
Execute when 3085L device is ready.
```
zs308a_240111_rel3\zephyr\samples\ota_usbhid>python usbhid_ota_upgrade.py
```

## II. Result
You could refer to `ats3085l_core_joypad_kc_upgrade-success.log` to see full log.

1. [HOST] Console output from I. D. Run python upgrade app
    ```
    D:\sdk_workplace\zs308a_240111_rel3_ota\zephyr\samples\ota_usbhid>python usbhid_ota_upgrade.py
    File: ota.bin
    Size: 264704 Bytes
    Offset = 0, Length = 1024
    Offset = 1024, Length = 551
    Offset = 2048, Length = 4096
    Offset = 6144, Length = 4096
    Offset = 10240, Length = 4096
    Offset = 14336, Length = 4096
    Offset = 18432, Length = 4096
    Offset = 22528, Length = 4096
    ...
    Offset = 256000, Length = 4096
    Offset = 260096, Length = 4096
    Offset = 264192, Length = 512
    Upgrade successfully
    ```

2. [3085L DEVICE] recieve 0x85 0x85 command and launch ota upgrade.
    ```
    [11:29:00.167]  [112][I][os][234*]:ota input event
    [11:29:00.167]  [113][I][os][187*]:start_usbhid_upgrade, Enter update
    [11:29:00.167]  [114][I][os][1593*]:attach backend type 4
    [11:29:00.167]  [115][I][os][490*]:bind backend 4
    [11:29:00.167]  [116][I][os][1436*]:handle upgrade
    [11:29:00.167]  [117][I][os][414*]:open type 4
    [11:29:00.167]  [118][I][os][422*]:read image header
    [11:29:00.167]  [119][I][os][213*]:ota_backend_usbhid_read, offset=0, buf=0x18005ecc, size=1024
    [11:29:00.167]  [120][I][os][195*]:ota_cmd_require_image_data, offset 0x0, len 1024, buf 0x400
    [11:29:00.167]  [121][I][os][42*]:tlv_pack, buf=0x18005e48
    [11:29:00.167]  [122][I][os][42*]:tlv_pack, buf=0x18005e4f
    [11:29:00.167]  [123][I][os][101*]:ota_usbhid_send_cmd, cmd=3, buf=0x18005e3c, size=19
    [11:29:00.167]  [124][I][os][196*]:usbhid_app_main exit
    ```
3. [3085L DEVICE] Start to receive chunks of ota.bin
    ```
    [14:09:44.505]  [173][I][os][219*]:---usbhid service exit---
    [14:09:44.553]  [174][I][os][140*]:cmd 0x10002: param 1
    [14:09:44.580]  
    [14:09:44.580]  [175][I][os][55*]:ota_backend_notify_callback, Upgrade Progress: 1%
    [14:09:44.580]  [176][I][os][225*]:OTA upgrade progress ==> 1%
    [14:09:44.580]  [177][I][os][213*]:ota_backend_usbhid_read, offset=1800, buf=0x18005ecc, size=4096
    [14:09:44.580]  [178][I][os][195*]:ota_cmd_require_image_data, offset 0x1800, len 4096, buf 0x1000
    [14:09:44.580]  [179][I][os][101*]:ota_usbhid_send_cmd, cmd=3, buf=0x18005e3c, size=19
    [14:09:44.580]  [180][I][os][48*]:save bp: state 2
    [14:09:44.580]  
    [14:09:44.580]  write 0x0 -> 0x0(0x1000) (8 ms)
    [14:09:44.648]  [181][I][os][140*]:cmd 0x10002: param 3
    [14:09:44.648]  
    [14:09:44.648]  [182][I][os][55*]:ota_backend_notify_callback, Upgrade Progress: 3%
    [14:09:44.648]  [183][I][os][225*]:OTA upgrade progress ==> 3%
    [14:09:44.648]  [184][I][os][213*]:ota_backend_usbhid_read, offset=2800, buf=0x18005ecc, size=4096
    [14:09:44.648]  [185][I][os][195*]:ota_cmd_require_image_data, offset 0x2800, len 4096, buf 0x1000
    [14:09:44.648]  [186][I][os][101*]:ota_usbhid_send_cmd, cmd=3, buf=0x18005e3c, size=19
    [14:09:44.648]  write 0x1000 -> 0x1000(0x1000) (6 ms)
    [14:09:44.717]  [187][I][os][140*]:cmd 0x10002: param 4
    [14:09:44.717]  
    [14:09:44.717]  [188][I][os][55*]:ota_backend_notify_callback, Upgrade Progress: 4%
    ...
    [14:09:49.118]  [064][I][os][55*]:ota_backend_notify_callback, Upgrade Progress: 100%
    [14:09:49.118]  [065][I][os][140*]:cmd 0x10001: param 1
    [14:09:49.118]  
    [14:09:49.118]  [066][I][os][101*]:ota_usbhid_send_cmd, cmd=6, buf=0x18005e3c, size=9
    [14:09:49.118]  [067][I][os][176*]:upadte ota state:  2
    [14:09:49.118]  
    [14:09:49.118]  [068][I][os][74*]:ota_app_notify, ota state: 1->2
    [14:09:49.118]  [069][I][os][467*]:close
    [14:09:49.118]  [070][I][os][1419*]:exit
    [14:09:49.118]  [071][I][os][543*]:exit
    [14:09:49.118]  [072][I][os][517*]:exit
    [14:09:49.118]  [073][I][os][199*]:start_usbhid_upgrade, ota_upgrade_check successful. Time spent: 4664
    [14:09:49.118]  printk use cpu
    [14:09:49.118]  system reboot, type 0x2!
    [14:09:49.118]  dev power off
    [14:09:49.118]  dev power end
    ```
4. [3085L DEVICE] Reboot and upgrade
    ```
    [14:09:55.058]  I: write file app.bin size 0x37680(0x37680) to offset 0x24000 start_offset 0x0(0x0)
    [14:09:55.058]  
    [14:09:55.058]  I: find file app.bin
    [14:09:55.058]  
    [14:09:55.058]  I: found file app.bin at [1]
    [14:09:55.058]  
    [14:09:55.058]  I: ota_rx thread started
    [14:09:55.058]  
    [14:09:55.058]  I: OTA upgrade progress ==> 1%
    [14:09:55.058]  
    [14:09:55.058]  I: save bp: state 4
    [14:09:55.058]  
    [14:09:55.058]  
    [14:09:55.058]  write 0x0 -> 0x0(0x1000) (8 ms)
    [14:09:55.058]  I: OTA upgrade progress ==> 3%
    [14:09:55.058]  
    [14:09:55.058]  write 0x1000 -> 0x1000(0x1000) (8 ms)
    [14:09:55.058]  I: OTA upgrade progress ==> 4%
    [14:09:55.058]  
    [14:09:55.058]  write 0x2000 -> 0x2000(0x1000) (6 ms)
    [14:09:55.058]  I: OTA upgrade progress ==> 6%
    [14:09:55.058]  
    [14:09:55.058]  write 0x3000 -> 0x3000(0x1000) (6 ms)
    [14:09:55.058]  I: OTA upgrade progress ==> 7%
    ```
5. [3085L DEVICE] Upgrade successfully
    ```
    [14:09:56.111]  z_sys_init(3) entry(0x10029490) func:0x1000b121, grp_end=0x100294a8
    [14:09:56.111]  z_sys_init(3) entry(0x10029498) func:0x100017d5, grp_end=0x100294a8
    [14:09:56.111]  z_sys_init(3) entry(0x100294a0) func:0x10001d59, grp_end=0x100294a8
    [14:09:56.111]  [056][I][os][25*]:OTA UPGRADE SUCCESSFULLY :3
    [14:09:56.111]  [057][E][nvram_cfg][218*]:invalid param storage 0x10029548, offset 0x10, len 0x8
    [14:09:56.111]  [058][I][os][86*]:save bp: state 120
    [14:09:56.111]  
    [14:09:56.111]  [059][I][os][41*]:main, init ota app
    ```