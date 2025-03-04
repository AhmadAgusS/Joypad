# APP: joypad
This is an demo APP, and verified only on ATS3085L Joystick_EVB_KC_V01

## Funtionalities
There're 4 main functionalities we demonstrate in joypad application: **usb_hid**, **bt_spp**, **bt_hid**, **bt_ble_vnd_gatt**
There would be three APP(bt_hid, bt_spp, and usb_hid) switching in sequece when you push "MENU" button.

## Verification Steps
### A. bt_hid
0. The default APP is bt_hid, so you don't need to push MENU button.
1. Connect with your mobile phone via bluetooth. You'll need to seach name start with "LARK_*****"
2. Push Black button(a.k.a Left Action Button1) to have volume up
3. Push White button(a.k.a Right Action Button1) to have volume down

    (This functionality was verified on iphone 13 mini.)

### B. bt_spp
0. Push MENU button, to switch to bt_spp (you could double check on UART0 log message)

1. You'll need to build another APP(bt_spp_thruput) on 3085 that will connect automatically to assigned BTSPP device and send packet.

    (1) Let's say we have two devices: A device (run joypad APP), and B device (run bt_spp_thruput)
    
    (2) You'll need to know **BT Name**(i.e. LARK_[DEVICE_MAC]) and **UUID** (please check: test_spp_uuid contents in bt_spp_main.c) of A device.
    
    (3) Modify bt_spp_thruput APP: `src/bt_spp/app_br.c`
    **TARGET_DEV_NAME** as BT Name of device A you got in 1.(2)
    
    (4) Modify bt_spp_thruput APP: `src/bt_spp/app_br.c`
    **app_spp_uuid_reg** (2nd one, since it's NOT a STANDARD_SPP, i.e. STATDARD_SPP==0)

2. Build bt_spp_thuput APP

3. Device B's log will be like:
    ```
    [18:11:34.614]  [121][I][os][404]: Thruput clock=1797422: Rx=0 bytes, Tx=10 bytes,
    [18:11:35.615]  [122][I][os][404]: Thruput clock=1798423: Rx=0 bytes, Tx=0 bytes,
    [18:11:36.616]  [123][I][os][404]: Thruput clock=1799424: Rx=0 bytes, Tx=0 bytes,
    [18:11:37.421]  [1800:230]TX: 02 00 08 12 00 0e 00 41 00 53 ef 15 d5 01 02 03 04
    [18:11:37.421]  [1800:230] 05 06 07 08 09 31
    [18:11:37.616]  [124][I][os][404]: Thruput clock=1800425: Rx=0 bytes, Tx=10 bytes,
    [18:11:38.619]  [125][I][os][404]: Thruput clock=1801426: Rx=0 bytes, Tx=0 bytes,
    [18:11:39.620]  [126][I][os][404]: Thruput clock=1802427: Rx=0 bytes, Tx=0 bytes,
    [18:11:40.422]  [1803:231]TX: 02 00 08 12 00 0e 00 41 00 53 ef 15 d6 01 02 03 04
    [18:11:40.422]  [1803:231] 05 06 07 08 09 31
    [18:11:40.620]  [127][I][os][404]: Thruput clock=1803428: Rx=0 bytes, Tx=10 bytes,
    [18:11:41.621]  [128][I][os][404]: Thruput clock=1804429: Rx=0 bytes, Tx=0 bytes,
    [18:11:42.621]  [129][I][os][404]: Thruput clock=1805430: Rx=0 bytes, Tx=0 bytes,
    [18:11:43.426]  [1806:232]TX: 02 00 08 12 00 0e 00 41 00 53 ef 15 d7 01 02 03 04
    ```
    ```
    [18:11:51.367]  [138][I][os][345]: Rx: channel:129 ,len 4 byte
    [18:11:51.545]  [1814:353]RX: 02 00 28 0c 00 08 00 41 00 51 ef 09 12 00 00 04 eb
    [18:11:51.545]  [1814:353]TX: 02 00 08 09 00 05 00 41 00 53 ff 01 04 2d
    [18:11:51.545]  [139][I][os][345]: Rx: channel:129 ,len 4 byte
    [18:11:51.631]  [140][I][os][404]: Thruput clock=1814439: Rx=8 bytes, Tx=0 bytes,
    [18:11:52.220]  [1815:028]RX: 02 00 28 0c 00 08 00 41 00 51 ef 09 12 00 00 20 eb
    ```
4. Device A(Target Device, running joypad application)'s log will be like:
    ```
    [17:43:03.491]  [010][I][bat_drv][160*]:current avg: 5ma
    [17:43:03.576]  [011][I][bat_ctl][940*]:cv check: 240ma-120ma-48ma, real: 5ma
    [17:43:03.887]  i&i:0 18 3001ms
    [17:43:03.887]  RX: 02 00 28 12 00 0e 00 41 00 53 ef 15 9a 01 02 03 04
    [17:43:03.887]   05 06 07 08 09 31
    [17:43:03.887]  Rx: channel:128 ,len 10 byte
    [17:43:04.782]  [012][I][bat_drv][160*]:current avg: 5ma
    [17:43:04.869]  [013][I][bat_ctl][940*]:cv check: 240ma-120ma-48ma, real: 5ma
    [17:43:06.067]  [014][I][bat_drv][160*]:current avg: 5ma
    [17:43:06.160]  [015][I][bat_ctl][940*]:cv check: 240ma-120ma-48ma, real: 5ma
    [17:43:06.432]  <CT-D> lwa: 0x0
    [17:43:06.432]  <CT-I> 0RSSI:-66 38 214
    [17:43:06.432]  <CT-W>RX: 02 00 28 12 00 0e 00 41 00 53 ef 15 9b 01 02 03 04
    [17:43:06.888]   05 06 07 08 09 31
    [17:43:06.888]  Rx: channel:128 ,len 10 byte
    [17:43:06.888]  TX: 02 00 08 09 00 05 00 41 00 51 ff 01 04 f7
    ```
    ```
    [17:42:48.471]  MSG_KEY_INPUT: 20000012 
    [17:42:48.471]  TX: 02 00 08 0c 00 08 00 41 00 51 ef 09 12 00 00 20 eb
    [17:42:48.471]  [228][I][os][86*]:Send data len 4
    [17:42:48.471]  
    [17:42:48.584]  MSG_KEY_INPUT: 4000012 
    [17:42:48.584]  TX: 02 00 08 0c 00 08 00 41 00 51 ef 09 12 00 00 04 eb
    [17:42:48.584]  [229][I][os][86*]:Send data len 4
    ```
### C. usb_hid

0. Push MENU button, to switch to usb_hid (you could double check on UART0 log message)

1. Make sure your device is connected with your PC via USB port.

2. Push Black button(a.k.a Left Action Button1) to have volume up. (See whether volume of your PC changes or not)

3. Push White button(a.k.a Right Action Button1) to have volume down. (See whether volume of your PC changes or not)

### D. bt_ble_vnd_gatt
Test this functionalities with nRF Connect APP.

0. Download [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile) APP

1. You could follow some exercise steps, according to the [tutorial](https://academy.nordicsemi.com/courses/bluetooth-low-energy-fundamentals/lessons/lesson-4-bluetooth-le-data-exchange/topic/gatt-operations/).