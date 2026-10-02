# HT9045 警報代碼參考 (Alarm Code Reference)

## 版本控制

| 版本 | 日期 | 更新者 | 說明 |
|------|------|--------|------|
| V1.00 | 2026-04-01 | Steven | 初版：1551 筆警報代碼 |
| V1.01 | 2026-04-16 | Steven (AI) | 加入版本控制機制 |

> 資料來源：Excel `SECS_20260401_Steven.xlsx` — AlarmReport + AlarmCodeList_HT9045 sheets
> 程式碼版本：`HT9011UC_Code_V3.33.902.0_20260410`

## 警報報告格式 (SECS Alarm Report Format)

### 警報等級 (Alarm Class)

| Class | 名稱 | 說明 |
|:-----:|------|------|
| 1 | Jam | 機械卡料，需人員介入 |
| 2 | Event | 事件警報，軟體流程異常 |
| 3 | Message | 訊息提示，無須停機 |

### 警報代碼格式 (Alarm Code = ALID)

共 9 位數字：`XXXXXXXXXXX`

```
位置:  X    XX    XXXXXX
意義:  Class Position Jam Code
```

- **Class** (1 位)：1=Jam, 2=Event, 3=Message
- **Position** (2 位)：模組位置代碼（見下表）
- **Jam Code** (6 位)：該模組內的唯一警報編號

### 位置代碼 (Position Code)

| 代碼 | 位置 |
|:----:|------|
| 00 | Event |
| 01 | Input Arm |
| 02 | Output Arm |
| 03 | Index Unit |
| 04 | Input Shuttle |
| 05 | Output Shuttle |
| 06 | Empty Tray Arm |
| 07 | Tester I/F |
| 08 | Scanner |
| 09 | Tray Loader |
| 10 | Empty Tray |
| 11 | Tray Unloader 1 |
| 12 | Tray Unloader 2 |
| 13 | Tray Unloader 3 |
| 14 | Color Tray |
| 15 | Temp. Controller |
| 16 | System |
| 17 | Fix Tray 1 |
| 18 | Fix Tray 2 |
| 19 | Fix Tray 3 |
| 20 | ESD System |
| 21 | Process Log |
| 22 | Motion Log |
| 23 | Cassette |
| 24 | Motor |
| 25 | Auto4 |
| 26 | Auto5 |
| 27 | Auto6 |
| 28 | Fix4 |

### S5F1 警報封包格式

```
S5F1 W
  L[7]
    <A[14] Alarm Time>        // "20140526173030" (yyyyMMddHHmmss)
    <U4[1] Alarm Class>        // 1=Jam, 2=Event, 3=Message
    <U4[1] Alarm Code (ALID)>  // e.g. 101000000
    <U4[1] Alarm Type>          // 1=Set, 0=Clear
    <U4[1] Mess Type>           // 0
    <A     Alarm Text (ALTX)>   // e.g. "[M01] Motor Error --Motor Power Off Error"
    <A     Alarm Des>            // e.g. "MTrayZ"
```

### Alarm Sub Message

Sub Message 為 CSV 格式：
- 第一碼：JAM RATE 使用旗標（0 或 1）
- 其餘碼：額外子訊息

---

## 警報代碼列表 (Alarm Code List)

> 共 1551 筆警報定義

### Event

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| — | 0 | Unknown alarm code |

### Auto 1

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 111001114 | Auto1 C_Auto1LoaderZ_Select separate error |
| Event | 211001116 | There is a tray on Auto 1 (the entrance)! |
| Message | 311001117 | Auto 1 SnAuto1_Selector_Off Error!!!! |
| Message | 311001124 | Please keyin the bin code label of Auto1! |
| Message | 311001181 | Auto 1 C_Auto1_Selector_Off Error!! |

### Auto 2

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 112001214 | Auto2 C_Auto2LoaderZ_Select separate error |
| Jam | 112012003 | In arm placement arm cylinder up error |
| Event | 212001216 | There is a tray on Auto 2 (the entrance)! |
| Message | 312001217 | Auto 2 SnAuto2_Selector_Off Error!!!! |
| Message | 312001224 | Please keyin the bin code label of Auto2!. |
| Message | 312001281 | Auto 2 C_Auto2_Selector_Off Error!! |

### Auto 3

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 113001314 | Auto3 C_Auto3LoaderZ_Select separate error |
| Event | 213001316 | There is a tray on Auto 3 (the entrance)! |
| Message | 313001317 | Auto 3 SnAuto3_Selector_Off Error!!!! |
| Message | 313001324 | Please keyin the bin code label of Auto3!. |
| Message | 313001381 | Auto 3 C_Auto3_Selector_Off Error!! |

### Auto 4

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 125002501 | Auto 4 tray goes inside arrival time up error! |
| Jam | 125002502 | Auto 4 tray lock cylinder (side pusher) error! |
| Jam | 125002503 | Auto 4 tray lock cylinder (backward locker) error! |
| Jam | 125002504 | Auto 4 tray separation cylinder (upper) error |
| Jam | 125002506 | Auto 4 tray separation cylinder (lower) error |
| Jam | 125002507 | Auto 4 tray separation cylinder (middle) error |
| Jam | 125002508 | Auto 4 tray positioning error |
| Jam | 125002509 | Auto 4 tray setting error |
| Jam | 125002510 | Auto 4 tray device floating error |
| Jam | 125002511 | Tray arm put tray to Auto 4 and Auto 4 do not detect tray error! |
| Jam | 125002512 | Auto 4 tray goes outside arrival time up error! |
| Jam | 125002513 | Auto 4 tray goes up or down arrival time up error! |
| Jam | 125002514 | Auto 4 C_Auto4LoaderZ_Select separate error |
| Jam | 125002570 | Auto 4 tray miss error! |
| Event | 225002516 | There is a tray on Auto 4 (the entrance)! |
| Event | 225002530 | Auto 4 tray miss error! |
| Event | 225002551 | Auto 4 tray color detect error!! |
| Message | 325002517 | Auto 4 SnAuto4_Selector_Off Error!!!! |
| Message | 325002520 | Auto 4 tray unloader is filled with trays |
| Message | 325002521 | No tray on unloader Auto 4 |
| Message | 325002522 | Please remove the Auto 4 tray manually! |
| Message | 325002523 | Please check if there has any device on Auto 4 tray! |
| Message | 325002581 | Auto 4 C_Auto4_Selector_Off Error!! |

### Auto 5

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 126002601 | Auto 5 tray goes inside arrival time up error! |
| Jam | 126002602 | Auto 5 tray lock cylinder (side pusher) error! |
| Jam | 126002603 | Auto 5 tray lock cylinder (backward locker) error! |
| Jam | 126002604 | Auto 5 tray separation cylinder (upper) error |
| Jam | 126002606 | Auto 5 tray separation cylinder (lower) error |
| Jam | 126002607 | Auto 5 tray separation cylinder (middle) error |
| Jam | 126002608 | Auto 5 tray positioning error |
| Jam | 126002609 | Auto 5 tray setting error |
| Jam | 126002610 | Auto 5 tray device floating error |
| Jam | 126002611 | Tray arm put tray to Auto 5 and Auto 5 do not detect tray error! |
| Jam | 126002612 | Auto 5 tray goes outside arrival time up error! |
| Jam | 126002613 | Auto 5 tray goes up or down arrival time up error! |
| Jam | 126002614 | Auto 5 C_Auto5LoaderZ_Select separate error |
| Jam | 126002670 | Auto 5 tray miss error! |
| Event | 226002616 | There is a tray on Auto 5 (the entrance)! |
| Event | 226002630 | Auto 5 tray miss error! |
| Event | 226002651 | Auto 5 tray color detect error!! |
| Message | 326002617 | Auto 5 SnAuto5_Selector_Off Error!!!! |
| Message | 326002620 | Auto 5 tray unloader is filled with trays |
| Message | 326002621 | No tray on unloader Auto 5 |
| Message | 326002622 | Please remove the Auto 5 tray manually! |
| Message | 326002623 | Please check if there has any device on Auto 5 tray! |
| Message | 326002681 | Auto 5 C_Auto5_Selector_Off Error!! |

### Auto 6

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 127002701 | Auto 6 tray goes inside arrival time up error! |
| Jam | 127002702 | Auto 6 tray lock cylinder (side pusher) error! |
| Jam | 127002703 | Auto 6 tray lock cylinder (backward locker) error! |
| Jam | 127002704 | Auto 6 tray separation cylinder (upper) error |
| Jam | 127002706 | Auto 6 tray separation cylinder (lower) error |
| Jam | 127002707 | Auto 6 tray separation cylinder (middle) error |
| Jam | 127002708 | Auto 6 tray positioning error |
| Jam | 127002709 | Auto 6 tray setting error |
| Jam | 127002710 | Auto 6 tray device floating error |
| Jam | 127002711 | Tray arm put tray to Auto 6 and Auto 6 do not detect tray error! |
| Jam | 127002712 | Auto 6 tray goes outside arrival time up error! |
| Jam | 127002713 | Auto 6 tray goes up or down arrival time up error! |
| Jam | 127002714 | Auto 6 C_Auto6LoaderZ_Select separate error |
| Jam | 127002770 | Auto 6 tray miss error! |
| Event | 227002716 | There is a tray on Auto 6 (the entrance)! |
| Event | 227002730 | Auto 6 tray miss error! |
| Event | 227002751 | Auto 6 tray color detect error!! |
| Message | 327002717 | Auto 6 SnAuto6_Selector_Off Error!!!! |
| Message | 327002720 | Auto 6 tray unloader is filled with trays |
| Message | 327002721 | No tray on unloader Auto 6 |
| Message | 327002722 | Please remove the Auto 6 tray manually! |
| Message | 327002723 | Please check if there has any device on Auto 2 tray! |
| Message | 327002781 | Auto 6 C_Auto6_Selector_Off Error!! |

### Cassette

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 123002300 | Cassette arm catch cassette error! |
| Jam | 123002301 | Cassette arm catch cassette up error! |
| Jam | 123002302 | Cassette arm cassette lose! |
| Jam | 123002310 | Cassette arm place buffer 1 error! |
| Jam | 123002311 | Cassette arm place buffer 2 error! |
| Jam | 123002312 | Cassette arm place buffer 3 error! |
| Jam | 123002313 | Cassette arm place buffer 4 error! |
| Jam | 123002314 | Cassette arm place buffer 5 error! |
| Jam | 123002315 | Cassette arm place buffer 6 error! |
| Jam | 123002316 | Cassette arm place buffer 7 error! |
| Jam | 123002317 | Cassette arm place buffer 8 error! |
| Jam | 123002318 | Cassette arm place buffer 9 error! |
| Jam | 123002319 | Cassette arm place buffer 10 error! |
| Jam | 123002320 | Load port arm catch cassette error! |
| Jam | 123002321 | Load port arm place buffer 1 error! |
| Jam | 123002322 | Load port arm place load port error! |
| Jam | 123002323 | Tray bracket have tray check error! |
| Jam | 123002324 | Buffer 6 cassette have tray check error! |
| Jam | 123002325 | Tray arm catch stacked tray error! |
| Jam | 123002326 | Tray arm place stacked tray to loader error! |
| Jam | 123002327 | Tray arm catch stacked tray to empty error! |
| Jam | 123002328 | Tray arm catch stacked tray to tray bracket error! |
| Jam | 123002329 | Tray arm catch stacked tray to auto1 error! |
| Jam | 123002330 | Tray arm catch stacked tray to auto2 error! |
| Jam | 123002331 | Tray arm catch stacked tray to auto3 error! |
| Jam | 123002332 | Cassette lose! |
| Jam | 123002333 | Place cassette buffer has cassette! |
| Jam | 123002334 | Load port arm place cassette position error! |
| Jam | 123002335 | Catch stacked tray catch loader no tray |
| Jam | 123002336 | Catch stacked tray catch empty no tray |
| Jam | 123002337 | Catch stacked tray catch conversion no tray |
| Jam | 123002338 | Catch stacked tray catch Auto 1 no tray |
| Jam | 123002339 | Catch stacked tray catch Auto 3 no tray |
| Jam | 123002340 | Load port RFID read error! |
| Jam | 123002341 | Buffer 6 RFID read error! |
| Jam | 123002342 | Load port wait SECS cassette data time out! |
| Jam | 123002343 | Buffer 6 no tray, but tray data has tray, pleas check! |
| Jam | 123002344 | Buffer 6 has tray, but tray data no tray, pleas check! |
| Jam | 123002370 | Magazine tray out, please remove the tray in slot. |
| Jam | 123002371 | Magazine has tray, please remove the tray. |
| Jam | 123002372 | Magazine tray missing error. |
| Event | 223002373 | Magazine tray cover missing error |

### Color Tray

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 114001401 | Color tray goes inside arrival time up error! |
| Jam | 114001402 | Color tray lock cylinder(side pusher) error! |
| Jam | 114001403 | Color Tray lock cylinder(backward locker) error! |
| Jam | 114001404 | Color tray separation cylinder (upper) error |
| Jam | 114001405 | Color tray separation cylinder (middle) error |
| Jam | 114001406 | Color tray separation cylinder (lower) error |
| Jam | 114001407 | Color tray separation error |
| Jam | 114001408 | Color tray pick-up error |
| Jam | 114001409 | Color tray setting error |
| Jam | 114001410 | Color tray separation (push-up) error |
| Jam | 114001411 | Tray arm put tray to Color and Color do not detect tray Error! |
| Jam | 114001412 | Color tray goes outside arrival time up error! |
| Jam | 114001413 | Color tray goes up or down arrival time up error! |
| Message | 314001420 | Color tray is filled with trays |
| Message | 314001421 | No tray on Color tray |
| Message | 314001422 | Empty Tray Drawer is filled with trays |
| Message | 314001423 | Empty Tray Drawer setting error |
| Message | 314001424 | Empty Tray Drawer tray supply error |
| Jam | 114001430 | Color tray miss error! |
| Event | 214001451 | Color Tray color detect error!! |
| Jam | 114001414 | take out color tray. |
| Event | 214001416 | There is a tray on Color (the entrance)! |
| Event | 214001487 | Tray ID Read Error on Color Stack! |
| Event | 214014061 | Color tray IC remain communication timeout! |
| Event | 214014062 | Color tray IC remain ROI check error! |
| Event | 214014063 | Color tray IC remain laser initial fail! |
| Event | 214014064 | Color tray IC remain grab error! |
| Event | 214014065 | Color tray IC remain inspection error! |
| Event | 214014066 | Color tray IC remain detect error! |

### ESD

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 220002025 | Clean ION fan. Tray arm cylinder need up safe!! |
| Event | 220002026 | Clean ION fan. In and out arm should move to decay teach position !! |
| Event | 220002077 | KASUGA fan station 1 decay test exceed the set!! : TimerESD |
| Event | 220002078 | KASUGA fan station 2 decay test exceed the set!! : TimerESD |
| Event | 220002080 | Bin Trolley Ion fan Alarm!! |
| Event | 220002081 | Bin Trolley Ion fan Power Alarm!! |
| Event | 220002098 | Loader Ion Gun Alarm!! |

### ESD System

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 220002000 | Ion fan 1 alarm. |
| Event | 220002001 | Ion fan 2 alarm. |
| Event | 220002002 | Ion fan 3 alarm. |
| Event | 220002003 | Ion fan 4 alarm. |
| Event | 220002004 | Ion fan 5 alarm. |
| Event | 220002005 | Ion fan 6 alarm. |
| Event | 220002006 | Ion fan 7 alarm. |
| Event | 220002007 | Ion fan 8 alarm. |
| Event | 220002008 | Ion fan 9 alarm. |
| Event | 220002009 | Ion fan 10 alarm. |
| Event | 220002010 | Ion fan 11 alarm(Index). |
| Event | 220002011 | Ion fan 12 alarm. |
| Event | 220002020 | Input area has ESD event error! |
| Event | 220002021 | Index area has ESD event error! |
| Event | 220002022 | Output area has ESD event error! |
| Event | 220002023 | Other area has ESD event error! |
| Event | 220002030 | SIMCO NOVX3360 Station 1 Error!! |
| Event | 220002031 | SIMCO NOVX3360 Station 2 Error!! |
| Event | 220002032 | SIMCO NOVX3360 Station 3 Error!! |
| Event | 220002033 | SIMCO NOVX3360 Station 1 Prox 1 ESD Balance Error!! |
| Event | 220002034 | SIMCO NOVX3360 Station 1 Prox 2 ESD Balance Error!! |
| Event | 220002035 | SIMCO NOVX3360 Station 1 Prox 3 ESD Balance Error!! |
| Event | 220002036 | SIMCO NOVX3360 Station 2 Prox 1 ESD Balance Error!! |
| Event | 220002037 | SIMCO NOVX3360 Station 2 Prox 2 ESD Balance Error!! |
| Event | 220002038 | SIMCO NOVX3360 Station 2 Prox 3 ESD Balance Error!! |
| Event | 220002039 | SIMCO NOVX3360 Station 3 Prox 1 ESD Balance Error!! |
| Event | 220002040 | SIMCO NOVX3360 Station 3 Prox 2 ESD Balance Error!! |
| Event | 220002041 | SIMCO NOVX3360 Station 3 Prox 3 ESD Balance Error!! |
| Event | 220002042 | SIMCO NOVX3360 Station 1 Communication Error!! |
| Event | 220002043 | SIMCO NOVX3360 Station 2 Communication Error!! |
| Event | 220002044 | SIMCO NOVX3360 Station 3 Communication Error!! |
| Event | 220002045 | SIMCO NOVX3360 Station 1 Set Error!! |
| Event | 220002046 | SIMCO NOVX3360 Station 2 Set Error!! |
| Event | 220002047 | SIMCO NOVX3360 Station 3 Set Error!! |
| Event | 220002048 | SIMCO NOVX3360 Station 1 Set to Zero Error!! |
| Event | 220002049 | SIMCO NOVX3360 Station 2 Set to Zero Error!! |
| Event | 220002050 | SIMCO NOVX3360 Station 3 Set to Zero Error!! |
| Event | 220002051 | SIMCO NOVX3360 Station 1 Set Gain Error!! |
| Event | 220002052 | SIMCO NOVX3360 Station 2 Set Gain Error!! |
| Event | 220002053 | SIMCO NOVX3360 Station 3 Set Gain Error!! |
| Event | 220002054 | ESD_3M_STATION1_ALARM! |
| Event | 220002055 | ESD_3M_STATION2_ALARM! |
| Event | 220002056 | ESD_3M_STATION3_ALARM! |
| Event | 220002057 | ESD_3M_STATION1_COMERR error! |
| Event | 220002058 | ESD_3M_STATION2_COMERR error! |
| Event | 220002059 | ESD_3M_STATION3_COMERR error! |
| Event | 220002060 | ESD_Kasuga_STATION1_ALARM!! |
| Event | 220002061 | ESD_KASUGA_Connect_Error!! |
| Event | 220002062 | SIMCO NOVX3360 Station 1 Set Decay Time Error!! |
| Event | 220002063 | SIMCO NOVX3360 Station 2 Set Decay Time Error!! |
| Event | 220002064 | SIMCO NOVX3360 Station 3 Set Decay Time Error!! |
| Event | 220002065 | SIMCO NOVX3360 Station 1 Read Decay Time Error!! |
| Event | 220002066 | SIMCO NOVX3360 Station 2 Read Decay Time Error!! |
| Event | 220002067 | SIMCO NOVX3360 Station 3 Read Decay Time Error!! |
| Event | 220002068 | SIMCO NOVX3360 Station 1 Start Decay Test Error!! |
| Event | 220002069 | SIMCO NOVX3360 Station 2 Start Decay Test Error!! |
| Event | 220002070 | SIMCO NOVX3360 Station 3 Start Decay Test Error!! |
| Event | 220002071 | SIMCO NOVX3360 Station 1 Read Decay Test Error!! |
| Event | 220002072 | SIMCO NOVX3360 Station 2 Read Decay Test Error!! |
| Event | 220002073 | SIMCO NOVX3360 Station 3 Read Decay Test Error!! |
| Event | 220002074 | SIMCO NOVX3360 Station 1 Decay Test Exceed the set!! |
| Event | 220002075 | SIMCO NOVX3360 Station 2 Decay Test Exceed the set!! |
| Event | 220002076 | SIMCO NOVX3360 Station 3 Decay Test Exceed the set!! |
| Event | 220002090 | Ion fan alarm. |
| Event | 220002091 | Ion fan condition / Ion level alarm. |
| Event | 220002092 | Ion barrier alarm. |
| Event | 220002093 | Ion barrier ion level alarm. |
| Event | 220002094 | Ion barrier condition alarm. |
| Event | 220002095 | Ion fan power or cover error!! |
| Event | 220002096 | Ion fan rotation error!! |
| Event | 220002097 | Ion fan need cleaning!! |

### Empty Tray

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 110001001 | Empty tray goes inside arrival time up error! |
| Jam | 110001002 | Empty tray lock cylinder(side pusher) error! |
| Jam | 110001003 | Empty tray lock cylinder(backward locker) error! |
| Jam | 110001004 | Empty tray separation cylinder(upper) error |
| Jam | 110001005 | Empty tray separation cylinder(middle) error |
| Jam | 110001006 | Empty tray separation cylinder(lower) error |
| Jam | 110001007 | Empty tray separation error |
| Jam | 110001008 | Empty tray pick-up error |
| Jam | 110001009 | Empty tray setting error |
| Jam | 110001010 | Empty tray separation (push-up) error |
| Jam | 110001011 | Empty tray arm put tray to Empty and Empty do not detect tray Error! |
| Jam | 110001012 | Empty tray goes outside arrival time up error! |
| Jam | 110001013 | Empty tray goes up or down arrival time up error! |
| Message | 310001020 | Empty tray 1 is filled with trays |
| Message | 310001021 | No tray on Empty Tray 1 |
| Message | 310001022 | Empty tray Drawer is filled with trays |
| Message | 310001023 | Empty tray Drawer setting error |
| Message | 310001024 | Empty tray Drawer tray supply error |
| Jam | 110001030 | Empty tray miss error! |
| Message | 310001050 | Manual tray feeder position error |
| Event | 210001051 | Empty Tray color detect error!! |
| Message | 310001052 | Must change engineer up off_line |
| Message | 310001053 | Must change engineer up off_line, product in machine. |
| Event | 210001016 | There is a tray on Empty (the entrance)! |

### Empty Tray Arm

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 106000601 | Tray pick-up error |
| Jam | 106000602 | Tray Arm hand up error! |
| Jam | 106000603 | Tray Arm hand down error! |
| Jam | 106000604 | Tray Arm chunk close error. |
| Jam | 106000605 | Tray Arm Cover Close Error! |
| Jam | 106000610 | Tray drop error |
| Event | 206000611 | Tray Arm Initial Start error |
| Event | 206000612 | ART move tray Finish |
| Jam | 106000613 | Tray Arm catch tray On is fail,check sensor on. |
| Event | 206000614 | Home Empty Tray Arm Have Tray (sensor on) ,remove tray. |
| Event | 206000615 | ART_TrayARM ON/OFF Cyclinder Sensor error |
| Jam | 106000627 | Vacuum sensor OFF error |
| Event | 206000630 | Tray Arm Position Error |
| Event | 206006300 | Tray Arm motor error |

### Fix Tray 1

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Message | 317001720 | Fix tray 1 is filled with devices. |
| Message | 317001721 | No tray on fix tray 1 |
| Message | 317001723 | Fix tray 1 will reload new tray. |
| Event | 217001731 | Fix tray floating error! |
| Event | 217001751 | Fix tray 1 color detect error!! |
| Event | 217001752 | Fix tray 1 missing error!! |
| Message | 317001710 | Fix tray 1, 2 and 3 is filled with devices. |
| Message | 317001711 | Fix tray 1 and 2 is filled with devices. |
| Message | 317001712 | Please scan the Bundle ID of Fix1! |
| Message | 317001713 | Please scan the NO ReTest BIN ID of Fix1! |
| Message | 317001724 | Please keyin the bin code label of Fix1! |

### Fix Tray 2

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 118001801 | Fix tray cylinder(forward) error |
| Jam | 118001802 | Fix tray cylinder(backward) error |
| Message | 318001820 | Fix tray 2 is filled with devices. |
| Message | 318001821 | No tray on fix tray 2 |
| Event | 218001822 | Fix tray 2 auto cleaning count over the alarm count |
| Message | 318001823 | Fix tray 2 will reload new tray. |
| Event | 218001851 | Fix tray 2 color detect error!! |
| Event | 218001852 | Fix tray 2 missing error!! |
| Message | 318001811 | Fix tray 2 and 3 is filled with devices. |
| Message | 318001812 | Please scan the Bundle ID of Fix2! |
| Message | 318001813 | Please scan the NO ReTest BIN ID of Fix2! |
| Message | 318001824 | Please keyin the bin code label of Fix2!. |

### Fix Tray 3

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 119001902 | Fix tray cylinder(forward) error |
| Jam | 119001903 | Fix tray cylinder(backward) error |
| Message | 319001920 | Fix tray 3 is filled with devices. |
| Message | 319001921 | No tray on fix tray 3 |
| Event | 219001922 | Clean kit auto cleaning count over the alarm count!! |
| Message | 319001923 | Fix tray 3 will reload new tray. |
| Event | 219001951 | Fix tray 3 color detect error!! |
| Message | 319001970 | Fix bin box is full, please open door 6 (right back) take out error bin box ic.!! |
| Jam | 119001940 | Fix tray 3 full place cylinder push (on) error!! |
| Jam | 119001941 | Fix tray 3 full place cylinder pop (off) error!! |
| Event | 219001952 | Fix tray 3 missing error!! |
| Message | 319001912 | Please scan the Bundle ID of Fix3! |
| Message | 319001913 | Please scan the NO ReTest BIN ID of Fix3! |
| Message | 319001924 | Please keyin the bin code label of Fix3!. |

### Fix Tray 4

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 228002822 | Fix tray 4 auto cleaning count over the alarm count |
| Event | 228002851 | Fix tray 4 color detect error!! |
| Event | 228002852 | Fix Tray 4 missing error!! |
| Message | 328002813 | Please scan the NO ReTest BIN ID of Fix4! |
| Message | 328002820 | Fix tray 4 is filled with devices. |
| Message | 328002821 | No tray on fix tray 4 |
| Message | 328002823 | Fix tray 4 will reload new tray. |

### Fix Tray 5

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 229002922 | Fix tray 5 auto cleaning count over the alarm count |
| Event | 229002951 | Fix tray 5 color detect error!! |
| Event | 229002952 | Fix Tray 5 missing error!! |
| Message | 329002913 | Please scan the NO ReTest BIN ID of Fix5! |
| Message | 329002920 | Fix tray 5 is filled with devices. |
| Message | 329002921 | No tray on fix tray 5 |
| Message | 329002923 | Fix tray 5 will reload new tray. |

### Fix Tray 6

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 230003022 | Fix tray 6 auto cleaning count over the alarm count |
| Event | 230003051 | Fix tray 6 color detect error!! |
| Event | 230003052 | Fix Tray 6 missing error!! |
| Message | 330003013 | Please scan the NO ReTest BIN ID of Fix6! |
| Message | 330003020 | Fix tray 6 is filled with devices. |
| Message | 330003021 | No tray on fix tray 6 |
| Message | 330003023 | Fix tray 6 will reload new tray. |

### Index Unit

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 103000301 | Device pick-up error at Arm 1 |
| Jam | 103000302 | Device pick-up error at Arm 2 |
| Jam | 103000303 | Device drop error (Arm 1) |
| Jam | 103000304 | Device drop error (Arm 2) |
| Jam | 103000305 | Device drop to Shuttle error (Arm 1)!! |
| Jam | 103000306 | Device drop to Shuttle error (Arm 2)!! |
| Event | 203000310 | Socket has IC error! |
| Event | 203000311 | Auto contact test : Reject bin received error. |
| Jam | 103000312 | Clean pad pick-up error at Arm 1 |
| Jam | 103000313 | Clean pad pick-up error at Arm 2 |
| Jam | 103000314 | Clean pad drop error (Arm 1) |
| Jam | 103000315 | Clean pad drop error (Arm 2) |
| Jam | 103000316 | Handler hang!! Try one cycle, home, reset, or reset program |
| Jam | 103000317 | Handler hang!! Try one cycle, home, reset, or reset program |
| Event | 203000320 | Index arm initial start error |
| Event | 203000321 | Index, contact force over error |
| Event | 203000322 | Socket have Ic |
| Event | 203000323 | Socket Detect Device Floting error |
| Jam | 103000327 | Index vacuum sensor OFF error |
| Event | 203000330 | RTC Vision program off error!! |
| Message | 303000331 | Please Auto Hight to get ROI Model! |
| Message | 303000332 | Please Check ROI for RTC or Auto Hight to Get FullView. |
| Event | 203000333 | RTC Light Fail! |
| Event | 203000334 | RTC Start Error! |
| Event | 203000335 | RTC Arm1 Error! |
| Event | 203000336 | RTC Arm2 Error! |
| Event | 203000337 | RTC FullT Time Out Error. |
| Event | 203000338 | RTC Home Error! |
| Event | 203000339 | RTC Wait Receive Over Time Error! |
| Event | 203000340 | RTC Please Check Encode! |
| Event | 203000341 | RTC Grab TimeOut Error! |
| Event | 203000342 | RTC HalfView Socket Have Devices Error! |
| Event | 203000343 | RTC FullView Index Check Error! |
| Event | 203000344 | RTC Feedback Check Error! |
| Event | 203000345 | RTC Trigger Check Error! |
| Event | 203000346 | RTC FullView Socket Have Devices Error! |
| Event | 203000347 | RTC program not exist!! |
| Jam | 103000350 | By Head yield different |
| Event | 203000360 | Arm connector drop |
| Jam | 103000371 | Arm 1 SLK clamp error |
| Jam | 103000372 | Arm 2 SLK clamp error |
| Jam | 103000373 | Arm 1 SLK unclamp error |
| Jam | 103000374 | Arm 2 SLK unclamp error |
| Jam | 103000375 | Socket clamp Error |
| Jam | 103000376 | Socket unclamp Error |
| Event | 203003300 | Index arm Y1 motor error |
| Event | 203003301 | Index arm Z1 motor error |
| Event | 203003302 | Index arm Z2 motor error |
| Event | 203003303 | Index arm Y2 motor error |
| Event | 203000307 | Index arm Z1 home sensor position has been changed error! |
| Event | 203000308 | Index arm Z2 home sensor position has been changed error! |
| Event | 203000309 | Reading load cell error |
| Event | 203000324 | Warning! Contact force per arm over than 20kgf |
| Event | 203000325 | Warning! Contact force per arm less than 5kgf |
| Event | 203000329 | EP of die force is not enough alarm! |
| Event | 203000348 | Galil command error. It happend 3 times in 10 mins !! |
| Event | 203000349 | Index Motor Servo Alarm !! |
| Event | 203000351 | All site fail |
| Event | 203000352 | Empty socket check has double device |
| Event | 203000353 | Empty socket check time out |
| Event | 203000354 | RTC Alarm Arm 1 NG |
| Event | 203000355 | RTC Alarm Arm 2 NG |
| Event | 203000356 | RTC Alarm CCD Stop Sensor ON,Error type time out. |
| Event | 203000361 | Index arm 1 contact torque monitor error. |
| Event | 203000362 | Index arm 2 contact torque monitor error. |
| Event | 203000377 | Index arm 1 compare SLK height over setting value, please check the EP system! |
| Event | 203000378 | Index arm 2 compare SLK height over setting value, please check the EP system! |
| Event | 203003314 | Index arm Y1 home sensor position has been changed error! |
| Event | 203003315 | Index arm Y2 home sensor position has been changed error! |
| Event | 203003316 | Index arm Y1 position error, need to press home |
| Event | 203003317 | Index arm Y2 position error, need to press home |
| Event | 203003500 | Index arm Y1 up two speed: |
| Event | 203003501 | Index arm Y2 up two speed: |
| Event | 203003502 | Index arm Z1 home sensor error!! |
| Event | 203003503 | Index arm Z2 home sensor error!! |
| Event | 203003600 | MCU arm 1 read A800701START error!! |
| Event | 203003601 | MCU arm 2 read A800702START error!! |
| Event | 203003602 | MCU arm 1 read 00A900601STOP error!! |
| Event | 203003603 | MCU arm 2 read 00A900602STOP error!! |
| Event | 203003604 | MCU arm 1 driver broken 00FF00801ERRXXX error!! |
| Event | 203003605 | MCU arm 2 driver broken 00FF00802ERRXXX error!! |
| Event | 203003606 | Clean Fan tray arm Cylinder need up Safe !! |
| Event | 203003607 | Clean Fan In out arm need move Decay Teach position !! |
| Event | 203003608 | Index Arm 1 No9 |
| Event | 203003609 | Index Arm 2 No9 |
| Message | 303000328 | Contract mode need open chamber door push Z1. |

### Input Arm

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Message | 301000101 | Device pick-up error on the tray |
| Jam | 101000109 | Device pick-up error on Hot Plate |
| Jam | 101000110 | Device pick-up error on Clean Kit |
| Jam | 101000111 | Device pick-up error on Shuttle |
| Jam | 101000112 | Device pick up error on input rotate kit! |
| Jam | 101000114 | Close site pick-up IC error, take out ic. |
| Jam | 101000115 | Clean pad pick-up error on shuttle |
| Event | 201000119 | Loading count is less than ART setting count |
| Event | 201000120 | Loading count is more than ART setting count |
| Jam | 101000126 | In arm Device drop error |
| Jam | 101000127 | In arm Vacuum sensor OFF error |
| Jam | 101000128 | Clean pad drop error |
| Event | 201000132 | In arm initial start error |
| Event | 201000150 | In arm search hot plate data error |
| Event | 201000151 | In arm place hot plate x or y < 0 error |
| Event | 201000152 | Tray parameter error, motor will out of limit |
| Event | 201000153 | In arm pick hot plate data error |
| Event | 201000154 | In arm X axis motor will out of limit! |
| Event | 201000155 | In arm Y axis motor will out of limit! |
| Message | 301000156 | In arm search hot plate data error, hot plate format not support! |
| Event | 201000157 | In arm sucker not at home error! |
| Event | 201000158 | In arm destroy error, please take away the device! |
| Jam | 101000160 | Hot Plate 1 device height error by Laser!! |
| Jam | 101000161 | Hot Plate 2 device height error by Laser!! |
| Jam | 101000162 | Hot Plate 1 device lose error by Laser!! |
| Jam | 101000163 | Hot Plate 2 device lose error by Laser!! |
| Event | 201000164 | Input arm laser read error, please check COM port connection!! |
| Event | 201000165 | Hot plate 1 laser read error, please check sensor position!! |
| Event | 201000166 | Hot plate 2 laser read error, please check sensor position!! |
| Event | 201000170 | Device superfloat at hot plate error! |
| Event | 201001300 | In arm X motor error |
| Event | 201001301 | In arm Y motor error |
| Event | 201001302 | In arm pitch motor error |
| Event | 201001303 | In arm Z A motor error |
| Event | 201001304 | In arm Z B motor error |
| Event | 201001305 | In arm Z C motor error |
| Event | 201001306 | In arm Z D motor error |
| Event | 201001307 | In arm Z E motor error |
| Event | 201001308 | In arm Z F motor error |
| Event | 201001309 | In arm Z G motor error |
| Event | 201001310 | In arm Z H motor error |
| Jam | 101000159 | In arm place to hot plate make double IC error! |
| Event | 201000121 | Please collection the count of ART! |
| Event | 201000122 | Tray map data has no device, please remove device |
| Event | 201000123 | In arm Y pitch home error !! |
| Event | 201000131 | In arm auto height fail. |
| Event | 201000173 | In arm area |
| Event | 201000174 | No.9 picker of In arm device pick-up error |
| Event | 201000175 | Placement box has device, need to clear box |
| Event | 201000176 | No.9 In arm Device pick up error for Shuttle1 |
| Event | 201000177 | No.9 In arm Device pick up error for Shuttle2 |
| Event | 201000178 | No.9 Device drop error |
| Event | 201001311 | In arm auto height fail. |
| Message | 301000102 | Abnormal auto tray end. Please check devices count. |
| Message | 301000103 | In arm device pick-up error on the tray, please check and remove tray |
| Message | 301000104 | In arm device pick-up error on the tray_Consecutive |
| Message | 301000105 | Device pick-up error on the tray |
| Message | 301000106 | Device pick-up error on the tray |
| Message | 301000149 | In arm pick from loader position <0 error! |
| Message | 301000171 | In arm X CW/CCW sensor on error |
| Message | 301000172 | In arm Y CW/CCW sensor on error |

### Input Shuttle

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 104000401 | In shuttle 1 device floating error. |
| Jam | 104000402 | In shuttle 1 device superfluous error. |
| Jam | 104000403 | In shuttle 1 device condition error. |
| Jam | 104000404 | In shuttle 2 device floating error. |
| Jam | 104000405 | In shuttle 2 device superfluous error. |
| Jam | 104000406 | In shuttle 2 device condition error. |
| Jam | 104000407 | In shuttle 1 Sensor Broken.! |
| Jam | 104000408 | In shuttle 2 Sensor Broken.! |
| Jam | 104000410 | In shuttle 2 Clean PAD floating error |
| Jam | 104000411 | In shuttle 1 device height error by Laser!! |
| Jam | 104000412 | In shuttle 2 device height error by Laser!! |
| Event | 204000413 | In shuttle laser read error, please check COM port connection!! |
| Event | 204000414 | In shuttle 1 Laser read error, please check sensor position!! |
| Event | 204000415 | In shuttle 2 Laser read error, please check sensor position!! |
| Jam | 104000450 | In shuttle 1 Rotate Fail! |
| Jam | 104000451 | In shuttle 2 Rotate Fail! |
| Jam | 104000452 | In rotate devices floating error!! |
| Jam | 104000453 | In rotate rotation fail error!! |
| Jam | 104000460 | In Shuttle 1 2DID error! |
| Jam | 104000461 | In Shuttle 2 2DID error! |
| Event | 204000462 | 2DID communication time out! |
| Event | 204000463 | In shuttle 1 CCD 1 or 2 exposure time out! |
| Event | 204000464 | In shuttle 2 CCD 3 or 4 exposure time out! |
| Event | 204000465 | In shuttle 1 check have duplicate 2DID error! |
| Event | 204000466 | In shuttle 2 check have duplicate 2DID error! |
| Event | 204000467 | In shuttle 1 check have duplicate 2DID in lot error! |
| Event | 204000468 | In shuttle 2 check have duplicate 2DID in lot error! |
| Event | 204000469 | In shuttle 1 exposure position error! |
| Event | 204000470 | In shuttle 2 exposure position error! |
| Event | 204000471 | In shuttle 1 check by lot 2DID error! |
| Event | 204000472 | In shuttle 2 check by lot 2DID error! |
| Jam | 104000473 | In shuttle 1 device deviation error |
| Jam | 104000474 | In shuttle 2 device deviation error |
| Event | 204000475 | 2DID consecutive failure error! |
| Event | 204000476 | In shuttle 1 need remove ic! |
| Event | 204000477 | In shuttle 2 need remove ic! |
| Jam | 104000478 | In Shuttle 1 IC float error! |
| Jam | 104000479 | In Shuttle 2 IC float error! |
| Jam | 104000480 | In shuttle 1 has device from Index Arm. |
| Jam | 104000481 | In shuttle 2 has device from Index Arm. |
| Event | 204000490 | In shuttle Initial Start error |
| Event | 204004300 | In shuttle 1 motor error |
| Event | 204004301 | In shuttle 2 motor error |
| Jam | 104000416 | In shuttle 1 clean pad floating error. |
| Jam | 104000417 | In shuttle 2 clean pad floating error. |
| Jam | 104000484 | Buttom 2D ID Data error. |
| Jam | 104000495 | 2DID Mapping process fail,In Shuttle 1 2DID error! |
| Jam | 104000496 | 2DID Mapping process fail,In Shuttle 2 2DID error! |
| Jam | 104000497 | 2DID Mapping process fail,Bottom 2DID error! |
| Event | 204000482 | No LotCheckData file |
| Event | 204000483 | 2DID Controlled Yield Alarm! |
| Event | 204000485 | Shuttle 1 2DID is not on the allow list! |
| Event | 204000486 | Shuttle 2 2DID is not on the allow list! |
| Event | 204000487 | In shuttle 1 No.9 |
| Event | 204000488 | In shuttle 2 No.9 |
| Event | 204000489 | Tray Arm need Up.check sensor up |
| Event | 204000491 | In shuttle 1 No9 Pre Check Over Count |
| Event | 204000492 | In shuttle 2 No9 Pre Check Over Count |
| Event | 204000493 | In shuttle 1 sensor is by pass |
| Event | 204000494 | In shuttle 2 sensor is by pass |
| Event | 204004200 | In shuttle 1 Barcode Compare Error!! |
| Event | 204004201 | In shuttle 2 Barcode Compare Error!! |
| Event | 204004202 | In shuttle 1 Remote Barcode Compare Error!! |
| Event | 204004203 | In shuttle 2 Remote Barcode Compare Error!! |
| Event | 204004204 | In shuttle 1 Barcode Check Sum Error!! |
| Event | 204004205 | In shuttle 2 Barcode Check Sum Error!! |
| Event | 204004206 | In shuttle 1 Pin 1 inspection Error!! |
| Event | 204004207 | In shuttle 2 Pin 1 inspection Error!! |

### Motion Log

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 222002200 | Index Position Error |
| Event | 222002201 | Gail Command |
| Event | 222002202 | Gail TC1 |
| Event | 222002203 | Gail Command Err |
| Event | 222002204 | In Arm Z Homing |
| Event | 222002205 | Out Arm Z Homing |
| Event | 222002206 | Auto homing |
| Event | 222002207 | Do process motor home start. |
| Event | 222002208 | Do process motor home finish. |
| Event | 222002209 | In arm Z home finish |
| Event | 222002210 | Out arm Z home finish |
| Event | 222002211 | In arm Z home fail |
| Event | 222002212 | Out arm Z home fail |

### Output Arm

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 102000201 | Device pick-up error on output shuttle 1 |
| Jam | 102000202 | Device pick-up error on output shuttle 2 |
| Jam | 102000203 | Device drop error |
| Event | 202000204 | Device drop at output arm. |
| Jam | 102000210 | Device pick up error on output rotate kit! |
| Jam | 102000211 | Device pick-up error on auto 1 |
| Jam | 102000212 | Device pick-up error on auto 2 |
| Jam | 102000213 | Device pick-up error on auto 3 |
| Jam | 102000214 | Device pick-up error on fix 1 |
| Jam | 102000215 | Device pick-up error on fix 2 |
| Jam | 102000216 | Device pick-up error on fix 3 |
| Jam | 102000217 | Out arm Vacuum sensor OFF error |
| Event | 202000226 | Out arm initial start error |
| Event | 202000254 | Out arm X axis motor will out of limit! |
| Event | 202000255 | Out arm Y axis motor will out of limit! |
| Event | 202000256 | Output shuttle detect bin data miss, will auto place to interface error bin. |
| Event | 202000257 | Out arm sucker not at home error! |
| Event | 202000258 | Out arm destroy error, please take away the device! |
| Event | 202002300 | Out arm X motor error |
| Event | 202002301 | Out arm Y motor error |
| Event | 202002302 | Out arm pitch motor error |
| Event | 202002303 | Out arm Z A motor error |
| Event | 202002304 | Out arm Z B motor error |
| Event | 202002305 | Out arm Z C motor error |
| Event | 202002306 | Out arm Z D motor error |
| Event | 202002307 | Out arm Z E motor error |
| Event | 202002308 | Out arm Z F motor error |
| Event | 202002309 | Out arm Z G motor error |
| Event | 202002310 | Out arm Z H motor error |
| Jam | 102000232 | Out arm device pick-up error on auto 4 |
| Jam | 102000233 | Out arm device pick-up error on auto 5 |
| Jam | 102000234 | Out arm device pick-up error on auto 6 |
| Jam | 102000235 | Out arm device pick-up error on fix 4 |
| Jam | 102000236 | Out arm device pick-up error on fix 5 |
| Jam | 102000237 | Out arm device pick-up error on fix 6 |
| Event | 202000227 | Out arm Y pitch home error !! |
| Event | 202000231 | Out arm auto height fail. |
| Event | 202002350 | Out arm 2DID duplicate error |
| Message | 302000271 | Out arm X CW/CCW sensor on error |
| Message | 302000272 | Out arm Y CW/CCW sensor on error |

### Output Shuttle

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 105000501 | Out Shuttle 1 device floating error |
| Jam | 105000504 | Out Shuttle 2 device floating error |
| Jam | 105000508 | Device lose at Out Shuttle 1. |
| Jam | 105000509 | Device lose at Out Shuttle 2. |
| Jam | 105000540 | Device lose at Out Shuttle 1. |
| Jam | 105000550 | Device lose at Out Shuttle 2. |
| Jam | 105000551 | Out shuttle 2 rotate fail! |
| Jam | 105000552 | Out rotate devices floating error!! |
| Jam | 105000553 | Out rotate rotation fail error!! |
| Jam | 105000560 | Device superfluous at Output Shuttle 1. |
| Jam | 105000570 | Device superfluous at Output Shuttle 2. |
| Jam | 105000580 | Out Shuttle1 Barcode Error! |
| Jam | 105000581 | Out Shuttle2 Barcode Error! |
| Jam | 105000590 | Out Shuttle 1 Rotate Fail! |
| Jam | 105000591 | Out Shuttle 2 Rotate Fail! |
| Event | 205000592 | Out Shuttle Initial Start error |
| Event | 205005300 | Out Shuttle 1 |
| Event | 205005301 | Out Shuttle 2 |

### Process Log

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Message | 321002101 | Change Shuttle Mode |
| Message | 321002102 | Change Shuttle Select |
| Message | 321002103 | Use Suck Mode Change |
| Message | 321002104 | Change Handler Use |
| Message | 321002105 | Close Open Site |
| Message | 321002106 | Test Site Assign |
| Message | 321002107 | Change Start Mode |
| Message | 321002108 | Program Start |
| Message | 321002109 | Program Close |
| Message | 321002110 | START pressed |
| Message | 321002111 | PAUSE pressed |
| Message | 321002112 | HOME pressed |
| Message | 321002113 | RESET pressed |
| Message | 321002114 | CLEAN OUT pressed |
| Message | 321002115 | ONE CYCLE pressed |
| Message | 321002116 | ALARM RESET pressed |
| Message | 321002117 | POWER OFF pressed |
| Message | 321002118 | TRAY FEED pressed |
| Message | 321002119 | TRAY END pressed |
| Message | 321002120 | SKIP pressed |
| Message | 321002121 | RETRY pressed |
| Message | 321002122 | HOME & Retry pressed |
| Message | 321002123 | Auto Training pressed |
| Message | 321002124 | Fix pressed |
| Message | 321002125 | START pressed |
| Message | 321002126 | LOCK pressed |
| Message | 321002127 | HOME Push Z1 pressed |
| Message | 321002130 | Temperature Wait |
| Message | 321002131 | Temperature OK |
| Message | 321002132 | Change work tepmearture and soak time |
| Message | 321002140 | ======== Operator login ======== |
| Message | 321002141 | ======== Engineer login ======== |
| Message | 321002142 | ======== Supervisor login ======== |
| Message | 321002143 | ======== Hontech login ======== |
| Message | 321002144 | ======== USER login ======== |
| Message | 321002145 | XXXX  Tester MANUAL MODE  XXXX |
| Message | 321002146 | XXXX  Tester OFF-Line  XXXX |
| Message | 321002147 | VVVV  Tester ON-Line  VVVV |
| Message | 321002148 | VVVV  Run Mode : REALLY  VVVV |
| Message | 321002149 | XXXX  Run Mode : DUMMY  XXXX |
| Message | 321002150 | XXXX  Run Mode : TRAY ONLY  XXXX |
| Message | 321002151 | Change Hot Mode |
| Message | 321002152 | Change Ambient Hot Mode |
| Message | 321002153 | Change Ambient Mode |
| Message | 321002154 | Change Ambient/Hot/AmbientHot Mode |
| Message | 321002155 | Change to Off_Line |
| Message | 321002156 | Change to Manual Sort Mode by iTester Button |
| Message | 321002157 | Change to On_Line |
| Message | 321002158 | Change Set Up File |
| Message | 321002170 | Enter Contact |
| Message | 321002171 | Enter Bin |
| Message | 321002172 | Enter Tester I/F |
| Message | 321002173 | Enter Tray Form |
| Message | 321002174 | Enter Plate Form |
| Message | 321002175 | Enter Tray Assignment |
| Message | 321002176 | Enter Load / Unload |
| Message | 321002177 | Enter Set Up |
| Message | 321002178 | Enter Yield Monitoring |
| Message | 321002179 | Enter Builder |
| Message | 321002180 | Enter Start Condition |
| Message | 321002181 | Enter Counter Select |
| Message | 321002182 | Enter Counter Clear |
| Message | 321002183 | Enter Tower Light |
| Message | 321002184 | Enter Security |
| Message | 321002185 | Enter Configuration |
| Message | 321002186 | Enter DIO Form |
| Message | 321002187 | Enter Speed |
| Message | 321002188 | Enter Offset |
| Message | 321002189 | Enter Teach Form |
| Message | 321002190 | Enter IO |
| Message | 321002191 | Enter ATC Form |
| Message | 321002192 | Enter CCD TCPIP Form |
| Message | 321002193 | Enter CC-Link Form |
| Message | 321002194 | Enter Auto Clean Form |
| Message | 321002195 | Enter Omron Temperature Form |
| Message | 321002196 | Enter Sensor Latch Form |
| Message | 321002197 | Enter SECS GEM Form |
| Message | 321002198 | Enter OCR Form |
| Message | 321002199 | Enter Auto Temp. Form |
| Message | 321021100 | Enter Dynamic Temperature Form |
| Message | 321021101 | Enter 2D Bar Code Form |
| Message | 321021102 | Enter QA Mode Form |
| Message | 321021103 | Enter Rotate Form |
| Message | 321021104 | Enter Air Conditioner Form |
| Message | 321021105 | Enter PM Alarm Form |
| Message | 321021106 | Enter Monitor View |
| Message | 321021107 | Enter Shuttle Maintain |
| Message | 321021108 | Enter Tray Edit Form |
| Message | 321021109 | Enter Temp. Offset |
| Message | 321002128 | POWER ON pressed |

### Scanner

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 208000801 | IC Lead Scanner mode setting error |
| Event | 208000802 | IC Lead Scanner error |
| Event | 208000803 | IC Lead Scanner consecutive failure |
| Jam | 108000804 | IC Lead Scanne time out error |
| Event | 208000805 | IC Lead Scanne failure rate alarm error |
| Event | 208000811 | Socket consecutive scanning failure |
| Event | 208000821 | Arm1: consecutive scanning failure |
| Event | 208000829 | Arm2: consecutive scanning failure |
| Event | 208000860 | Continuous Start error(Scanner Mode) |
| Event | 208000861 | Start Mode Select error |
| Event | 208000870 | Scanner BD total counter error |
| Event | 208000871 | Scanner BD site 1 counter error |
| Event | 208000872 | Scanner BD site 2 counter error |
| Event | 208000873 | Scanner BD total conti counter error |
| Event | 208000874 | Scanner BD site 1 conti counter error |
| Event | 208000875 | Scanner BD site 2 conti counter error |
| Event | 208000876 | Scanner BD fail error |
| Event | 208000877 | Scanner BD fail error |
| Event | 208000878 | Scanner BD fail error |
| Event | 208000879 | Scanner BD fail error |
| Event | 208000880 | AOI scan continuous fail by site |
| Event | 208000881 | AOI scan continuous fail by arm 1 |
| Event | 208000882 | AOI scan continuous fail by arm 2 |
| Event | 208000883 | AOI scan reply time out |
| Event | 208000884 | Top AOI scan continuous fail by site |
| Event | 208000885 | Top AOI scan continuous fail by arm 1 |
| Event | 208000886 | Top AOI scan continuous fail by arm 2 |
| Event | 208000887 | Top AOI scan reply time out |
| Event | 208000888 | AOI error |
| Event | 208000889 | Device remain in tray |
| Event | 208000890 | Loader tray is not filled with IC |

### System

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 216001601 | System power error |
| Event | 216001602 | Motor power error |
| Event | 216001603 | Air not enough |
| Event | 216001604 | Negative Pressure Air not enough |
| Event | 216001605 | The air of electronic air regulator (EP) is not enough! |
| Event | 216001606 | Negative Pressure Air too low(Vacuum pump) |
| Message | 316001607 | Autoclean Start |
| Message | 316001608 | Autoclean Finish |
| Event | 216001609 | Ground error |
| Message | 316001611 | Safe door 1 is opened |
| Message | 316001612 | Safe door 2 is opened |
| Message | 316001613 | Safe door 3 is opened |
| Message | 316001614 | Safe door 4 is opened |
| Message | 316001615 | Safe door 5 is opened |
| Message | 316001616 | Safe door 6 is opened |
| Message | 316001617 | Safe door 7 is opened |
| Message | 316001618 | Safe door 8 is opened |
| Message | 316001619 | Safe door 9 is opened |
| Message | 316001620 | Safe door 10 is opened |
| Message | 316001625 | Heater door 1  is opened |
| Message | 316001626 | Heater door 2  is opened |
| Message | 316001627 | Heater door 3  is opened |
| Event | 216001630 | Emergency is pressed down (Front , Left) |
| Event | 216001631 | Emergency is pressed down (Front , Right) |
| Event | 216001632 | Emergency is pressed down (Rear , Left) |
| Event | 216001633 | Emergency is pressed down (Rear ,Right) |
| Event | 216001634 | ServonOff is break |
| Event | 216001635 | Could not execute a program in the controller. |
| Event | 216001636 | Initial Index Motion Card Fail! |
| Event | 216001637 | Heater fan can not run |
| Event | 216001638 | Motor control error, check control box! |
| Event | 216001639 | Motor encoder error, check encoder cable! |
| Message | 316001640 | One cycle finish |
| Message | 316001641 | Reset Finish!! |
| Message | 316001642 | Clean Out Finish!! |
| Message | 316001643 | [Tray Feed] OK!! |
| Message | 316001644 | [Tray End] OK!! |
| Message | 316001645 | Must finish [One Cycle]!! |
| Message | 316001646 | Must finish [Clean out]!! |
| Event | 216001647 | Comulacation with tester not finish!! |
| Message | 316001648 | QA mode finished! |
| Message | 316001649 | Tester Low Yield Stop Handler! |
| Message | 316001650 | SECS GEM Connection Fail!! |
| Message | 316001651 | [Pass Bin Tray Feed] OK!! |
| Message | 316001652 | Please choose reset mode. |
| Message | 316001653 | One cycle Finish!(After yield alarm) |
| Event | 216001672 | User Name Already exists |
| Message | 316001673 | ADD User Finish |
| Message | 316001674 | Delete User Finish |
| Message | 316001675 | Modify User Finish |
| Event | 216001676 | Insufficient privileges |
| Event | 216001677 | User Name or Password Error |
| Event | 216001678 | User Name or Password need KeyIn |
| Message | 316001680 | Please enter the device name and download set up file first!! |
| Event | 216001681 | File Read error!! |
| Event | 216001682 | File Save error!! |
| Event | 216001683 | Server Connection Fail!! |
| Event | 216001684 | Download file from server fail! |
| Event | 216001685 | Upload file to server fail! |
| Event | 216001686 | Unzip file fail! |
| Message | 316001687 | Unzip file Ok! |
| Message | 316001688 | Download file from server OK! |
| Message | 316001689 | Upload file to server OK! |
| Event | 216001690 | No SYN-TEK Master Card!!! |
| Event | 216001691 | No SYN-TEK DIO Module!!! |
| Event | 216001692 | Open SYN-TEK DIO Module Fail!!! |
| Event | 216001693 | Load SYN-TEK Motion Module Config Fail!!! |
| Event | 216001694 | No PISO DIO Card!!!! |
| Event | 216001695 | No PISO DIO Module!!!! |
| Event | 216001696 | Load PISO DIO Module Fail!!! |
| Event | 216001697 | No Contect Motion Card!!!! |
| Event | 216001698 | Load Contect Motion Config Fail!!! |
| Message | 316001699 | Home Check hotplate have any tray or other tooling and push Z1 continue. |
| Event | 216016100 | Motor not home alarm! |
| Event | 216016101 | Motor error! |
| Event | 216016102 | Auto clean must use ARM1 |
| Event | 216016103 | Auto clean must use ARM2 |
| Event | 216016104 | Tray feed, please take out fix tray: |
| Event | 216016105 | Arm1 pick place arm2 test do not use auto clean!! |
| Event | 216016106 | Temperature file (Temperature.Data) not exist!! |
| Event | 216016107 | Temperature data is null!! |
| Event | 216016108 | Sock time data is null!! |
| Event | 216016109 | CC-Link connect error |
| Event | 216016300 | Top view time out!! |
| Event | 216016301 | Bottom view time out!! |
| Event | 216016302 | 4 side view time out!! |
| Event | 216016303 | AOI Fail!! |
| Jam | 116016304 | Device pick-up error on AOI Kit!! |
| Event | 216016305 | AOI Continuous Fail BGAViewBySite Over Setting. |
| Event | 216016306 | AOI Continuous Fail BGAViewByArm1 Over Setting. |
| Event | 216016307 | AOI Continuous Fail BGAViewByArm2 Over Setting. |
| Event | 216016308 | AOI Continuous Fail PADViewBySite Over Setting. |
| Event | 216016309 | AOI Continuous Fail PADViewByArm1 Over Setting. |
| Event | 216016310 | AOI Continuous Fail PADViewByArm2 Over Setting. |
| Event | 216016311 | PAD AOI Fail!! |
| Event | 216016312 | BGA AOI Fail!! |
| Jam | 116016130 | Tester side pusher push error! |
| Jam | 116016131 | Tester side pusher pop error! |
| Jam | 116016337 | Tester side push cylinder has push error! |
| Jam | 116016338 | Tester side push cylinder has pop error! |
| Event | 216001610 | Dry air not enough |
| Event | 216016110 | SECS GEM run check time out! |
| Event | 216016116 | 2DID function is OFF! |
| Event | 216016117 | 2DID function : Check duplicate code by lot is OFF! |
| Event | 216016118 | MD5 check fail! |
| Event | 216016120 | Loader tray ID no respond read error |
| Event | 216016121 | Empty or Color tray ID no respond read error |
| Event | 216016122 | Load tray need take out tray manually. |
| Event | 216016123 | The three-hour inspection time is up. Please send materials for inspection |
| Event | 216016124 | Please send the first batch of materials for inspection. |
| Event | 216016125 | Safe door is opened. |
| Event | 216016126 | Auto site mapping function is OFF! |
| Event | 216016132 | Run Check function has been turn off! |
| Event | 216016140 | Emergency is pressed down. (PLC) |
| Event | 216016150 | EtherCAT PCI-1203 open card fail. |
| Event | 216016151 | EtherCAT NU-EC1 connection error. |
| Event | 216016152 | EtherCAT ring 1 was disconnected. |
| Event | 216016153 | EtherCAT AMP count error. |
| Event | 216016160 | Fix tray data has been clear, please check there is no device in Fix1/2/3. |
| Event | 216016313 | Please check the division of clean kit, the number of clean pad is not enough. |
| Event | 216016314 | Temperature exceeds 3 Sigma standard deviation. |
| Event | 216016315 | Teach Z pick-up pos offset data over 3mm. |
| Event | 216016316 | Don't use Dummy IC mode run Auto Z. |
| Event | 216016317 | Auto teach cannot close Site. |
| Event | 216016318 | Bin box X limit over range |
| Event | 216016319 | Teach Z, X Y pos offset data over 5mm. |
| Event | 216016320 | Get XML file for 2DID fail. |
| Event | 216016321 | Upload test result to server fail! |
| Event | 216016322 | EP controller return value error. |
| Event | 216016323 | Double EP controller return value error. |
| Event | 216016324 | Arm1 dual force EP air too low. |
| Event | 216016325 | Arm2 dual force EP air too low. |
| Event | 216016326 | PPSELECT load file error. Please check work file |
| Event | 216016327 | Tester send LOTORDER 1. Please check tester!! |
| Event | 216016329 | Un-expected unit ID read! |
| Event | 216016333 | Please inform field engineer to check params.!! |
| Event | 216016334 | Index cycle time is out of range. |
| Event | 216016335 | Empty or Color tray ID no respond read error |
| Event | 216016336 | Tray ID repeat |
| Event | 216016339 | Tray ID Read Duplicate Error! |
| Event | 216016435 | Shuttle 1 floodgate door must open ! |
| Event | 216016436 | Shuttle 2 floodgate door must open ! |
| Event | 216016446 | Chamber Dry Air not enough |
| Event | 216016500 | Local Recipe Switching Failed!! |
| Message | 316001621 | Safe door 11 is opened |
| Message | 316001622 | Safe door 12 is opened |
| Message | 316001623 | Safe door 13 is opened |
| Message | 316001624 | Safe door 14 is opened |
| Message | 316001628 | Heater door 4  is opened |
| Message | 316001629 | Safe door 15 is opened |
| Message | 316001654 | One cycle finish!(RTC model reset) |
| Message | 316001655 | ART loading devices finish. |
| Message | 316001656 | One cycle finish!(GPIB Command PAUSE) |
| Message | 316001657 | Hatchway safe door 1 is opened |
| Message | 316001658 | Hatchway safe door 2 is opened |
| Message | 316001659 | Hatchway safe door 3 is opened |
| Message | 316001660 | Hatchway safe door 4 is opened |
| Message | 316001661 | Hatchway safe door 5 is opened |
| Message | 316001662 | Hatchway safe door 6 is opened |
| Message | 316001663 | Hatchway safe door 7 is opened |
| Message | 316001664 | Hatchway safe door 8 is opened |
| Message | 316001665 | Hatchway safe door 9 is opened |
| Message | 316001666 | Hatchway safe door 10 is opened |
| Message | 316001667 | Hatchway safe door 11 is opened |
| Message | 316001668 | Safe Door 6 (fix tray PnP door) is opened |
| Message | 316016111 | Please input the quantity of EQC. |
| Message | 316016112 | Please insert lot ID and lot count! |
| Message | 316016113 | Index arm 1 water leakage alarm! |
| Message | 316016114 | Index arm 2 water leakage alarm! |
| Message | 316016115 | Plate water leakage alarm! |
| Message | 316016119 | Chiller water leakage alarm! |
| Message | 316016334 | Loader Optical Gate Abnormal Alarm!! |
| Message | 316016429 | Magazine safe door is opened |
| Message | 316016430 | Magazine Safe door 1 is opened. |
| Message | 316016431 | Magazine Safe door 2 is opened. |
| Message | 316016432 | Magazine Safe door 3 is opened. |
| Message | 316016433 | SnOHTIntoLoadPort safe door is opened. |
| Message | 316016434 | SnOHTIntoLoadPort1 safe door is opened. |
| Message | 316016437 | SysErr_EnhaustAir_VentDoorOpen |
| Message | 316016438 | SysErr_EnhaustAir_VentDoorClose |
| Message | 316016439 | SysErr_EnhaustAir_FanRunDetectOff |
| Message | 316016440 | SysErr_EnhaustAir_FanRunDetectOn |

### Temp. Controller

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 215001500 | Hotplate1 temperature OverLow! |
| Event | 215001501 | Hotplate2 temperature OverLow! |
| Event | 215001502 | Shuttle1 temperature OverLow! |
| Event | 215001503 | Shuttle2 temperature OverLow! |
| Event | 215001504 | Head12 temperature OverLow! |
| Event | 215001505 | Head34 temperature OverLow! |
| Event | 215001506 | Head56 temperature OverLow! |
| Event | 215001507 | Head78 temperature OverLow! |
| Event | 215001508 | Socket temperature OverLow! |
| Event | 215001509 | Chamber temperature OverLow! |
| Event | 215001510 | CCD temperature OverLow! |
| Event | 215001511 | Aa 1 temperature OverLow! |
| Event | 215001512 | Ab 1 temperature OverLow! |
| Event | 215001513 | Ac 1 temperature OverLow! |
| Event | 215001514 | Ad 1 temperature OverLow! |
| Event | 215001515 | Ba 1 temperature OverLow! |
| Event | 215001516 | Bb 1 temperature OverLow! |
| Event | 215001517 | Bc 1 temperature OverLow! |
| Event | 215001518 | Bd 1 temperature OverLow! |
| Event | 215001519 | Aa 2 temperature OverLow! |
| Event | 215001520 | Ab 2 temperature OverLow! |
| Event | 215001521 | Ac 2 temperature OverLow! |
| Event | 215001522 | Ad 2 temperature OverLow! |
| Event | 215001523 | Ba 2 temperature OverLow! |
| Event | 215001524 | Bb 2 temperature OverLow! |
| Event | 215001525 | Bc 2 temperature OverLow! |
| Event | 215001526 | Bd 2 temperature OverLow! |
| Event | 215001527 | Heat Gun 1 temperature OverLow! |
| Event | 215001528 | Heat Gun 2 temperature OverLow! |
| Event | 215001529 | Dut1 temperature OverLow! |
| Event | 215001530 | Dut2 temperature OverLow! |
| Event | 215001531 | Dut3 temperature OverLow! |
| Event | 215001532 | Dut4 temperature OverLow! |
| Event | 215001533 | Ae 1 temperature OverLow! |
| Event | 215001534 | Af 1 temperature OverLow! |
| Event | 215001535 | Ag 1 temperature OverLow! |
| Event | 215001536 | Ah 1 temperature OverLow! |
| Event | 215001537 | Be 1 temperature OverLow! |
| Event | 215001538 | Bf 1 temperature OverLow! |
| Event | 215001539 | Bg 1 temperature OverLow! |
| Event | 215001540 | Bh 1 temperature OverLow! |
| Event | 215001541 | Ae 2 temperature OverLow! |
| Event | 215001542 | Af 2 temperature OverLow! |
| Event | 215001543 | Ag 2 temperature OverLow! |
| Event | 215001544 | Ah 2 temperature OverLow! |
| Event | 215001545 | Be 2 temperature OverLow! |
| Event | 215001546 | Bf 2 temperature OverLow! |
| Event | 215001547 | Bg 2 temperature OverLow! |
| Event | 215001548 | Bh 2 temperature OverLow! |
| Event | 215001549 | 2D reader temperature OverLow! |
| Event | 215001550 | Hotplate1 temperature OverHigh! |
| Event | 215001551 | Hotplate2 temperature OverHigh! |
| Event | 215001552 | Shuttle1 temperature OverHigh! |
| Event | 215001553 | Shuttle2 temperature OverHigh! |
| Event | 215001554 | Head12 temperature OverHigh! |
| Event | 215001555 | Head34 temperature OverHigh! |
| Event | 215001556 | Head56 temperature OverHigh! |
| Event | 215001557 | Head78 temperature OverHigh! |
| Event | 215001558 | Socket temperature OverHigh! |
| Event | 215001559 | Chamber temperature OverHigh! |
| Event | 215001560 | CCD temperature OverHigh! |
| Event | 215001561 | Aa 1 temperature OverHigh! |
| Event | 215001562 | Ab 1 temperature OverHigh! |
| Event | 215001563 | Ac 1 temperature OverHigh! |
| Event | 215001564 | Ad 1 temperature OverHigh! |
| Event | 215001565 | Ba 1 temperature OverHigh! |
| Event | 215001566 | Bb 1 temperature OverHigh! |
| Event | 215001567 | Bc 1 temperature OverHigh! |
| Event | 215001568 | Bd 1 temperature OverHigh! |
| Event | 215001569 | Aa 2 temperature OverHigh! |
| Event | 215001570 | Ab 2 temperature OverHigh! |
| Event | 215001571 | Ac 2 temperature OverHigh! |
| Event | 215001572 | Ad 2 temperature OverHigh! |
| Event | 215001573 | Ba 2 temperature OverHigh! |
| Event | 215001574 | Bb 2 temperature OverHigh! |
| Event | 215001575 | Bc 2 temperature OverHigh! |
| Event | 215001576 | Bd 2 temperature OverHigh! |
| Event | 215001577 | Heat Gun 1 temperature OverHigh! |
| Event | 215001578 | Heat Gun 2 temperature OverHigh! |
| Event | 215001579 | Dut1 temperature OverHigh! |
| Event | 215001580 | Dut2 temperature OverHigh! |
| Event | 215001581 | Dut3 temperature OverHigh! |
| Event | 215001582 | Dut4 temperature OverHigh! |
| Event | 215001583 | Ae 1 temperature OverHigh! |
| Event | 215001584 | Af 1 temperature OverHigh! |
| Event | 215001585 | Ag 1 temperature OverHigh! |
| Event | 215001586 | Ah 1 temperature OverHigh! |
| Event | 215001587 | Be 1 temperature OverHigh! |
| Event | 215001588 | Bf 1 temperature OverHigh! |
| Event | 215001589 | Bg 1 temperature OverHigh! |
| Event | 215001590 | Bh 1 temperature OverHigh! |
| Event | 215001591 | Ae 2 temperature OverHigh! |
| Event | 215001592 | Af 2 temperature OverHigh! |
| Event | 215001593 | Ag 2 temperature OverHigh! |
| Event | 215001594 | Ah 2 temperature OverHigh! |
| Event | 215001595 | Be 2 temperature OverHigh! |
| Event | 215001596 | Bf 2 temperature OverHigh! |
| Event | 215001597 | Bg 2 temperature OverHigh! |
| Event | 215001598 | Bh 2 temperature OverHigh! |
| Event | 215001599 | 2D reader temperature OverHigh! |
| Event | 215015180 | Temperature Over High or Low Can not Start |
| Event | 215015181 | CCD over temperature |
| Event | 215015182 | Temperature Over Error! |
| Event | 215015183 | ATC Waiting TSD time out |
| Event | 215015190 | Temperature mode : High. Please turn on the temperature. |
| Message | 315015191 | Temperature mode : High |
| Message | 315015192 | Temperature mode : Ambient |
| Message | 315015193 | Enter power saving mode |
| Event | 215015200 | ATC system alarm: Water tank leaking!! |
| Event | 215015201 | ATC system alarm: Water tank over high limit!! |
| Event | 215015202 | ATC system alarm: Water tank over low limit!! |
| Event | 215015203 | ATC system alarm: Water flow error!! |
| Event | 215015204 | ATC system alarm: [ARM] Water flow error!! |
| Event | 215015205 | ATC system alarm: Water temperature too high!! |
| Event | 215015206 | ATC system alarm: Temperature over setting !! |
| Event | 215015207 | ATC system alarm: Tc thermocouple continuous error |
| Event | 215015208 | ATC system alarm: Ts thermocouple over setting |
| Event | 215015209 | ATC system alarm: Ts thermocouple continuous error |
| Event | 215015210 | ATC system alarm: Chiller circulating fluid pressure too high!! |
| Event | 215015211 | ATC system alarm: Chiller no power!! |
| Event | 215015212 | ATC system alarm: Low level in tank!! |
| Event | 215015213 | ATC system alarm: Over high temperature in chiller!! |
| Event | 215015214 | ATC system alarm: Over low temperature in chiller!! |
| Event | 215015215 | ATC system alarm: Circulating fluid discharge pressure rise!! |
| Event | 215015216 | ATC system alarm: Circulating fluid discharge pressure drop!! |
| Event | 215015217 | ATC system alarm: <Chiller can not running!!> |
| Event | 215015218 | ATC system alarm: <ALM01>GPUTJ board alarm/INT thermal couple error |
| Event | 215015219 | ATC system alarm: <ALM02>PV over max temp error. |
| Event | 215015220 | ATC system alarm: <ALM03>GPUTJ board sensor mode error (TJ/TC) |
| Event | 215015221 | ATC system alarm: <ALM04>GPUTJ board communication error |
| Event | 215015222 | ATC system alarm: <ALM05>ATC IPC is idle |
| Event | 215015223 | ATC system alarm: <ALM06>Water valve motor error |
| Event | 215015224 | ATC system alarm: NI module initial fail!! |
| Event | 215015225 | ATC system alarm: Read recipe file error!! |
| Event | 215015226 | ATC system alarm: DA module connect fail!! |
| Event | 215015227 | ATC system alarm: ATC3.1 module connect fail!! |
| Event | 215015228 | ATC system alarm: ATC command time out!! |
| Event | 215015229 | ATC system alarm: Receive #SETFAIL!! |
| Event | 215015230 | ATC system alarm: Network disconnect!! |
| Event | 215015231 | ATC system alarm: Unknown alarm code!! |
| Event | 215015232 | ATC system alarm: Unknown command code!! |
| Event | 215015233 | ATC system alarm: Write five same temperature |
| Event | 215015234 | ATC system alarm: Chiller has not started error |
| Event | 215015235 | ATC system alarm: Chiller connect error |
| Event | 215015236 | ATC system alarm: Water flow error |
| Event | 215015237 | ATC system alarm: Water temperature sensor over high |
| Event | 215015238 | ATC system alarm: ATC power supply off error |
| Event | 215015239 | ATC system alarm: ATC disable sSite |
| Event | 215015240 | ATC system alarm: Temperature below error |
| Event | 215015241 | ATC system alarm: Leak water |
| Event | 215015242 | ATC system alarm: Temperature over setting !! |
| Event | 215015300 | ATC alarm: ATC temperature sensor always same error |
| Event | 215015301 | ATC alarm: ATC temperature alarm by temperature over setting degree and continuous setting time alarm |
| Event | 215015302 | ATC alarm: ATC temperature alarm by maximum peak alarm |
| Event | 215015303 | ATC alarm: ATC temperature alarm by temperature difference over setting alarm |
| Event | 215015304 | ATC alarm: ATC temperature over error |
| Event | 215015305 | ATC alarm: ATC temperature below error |
| Event | 215015306 | ATC alarm: ATC temperature refer sensor always same error |
| Event | 215015307 | ATC alarm: ATC temperature sensor error |
| Event | 215015308 | ATC alarm: ATC temperature refer sensor error |
| Event | 215015309 | ATC alarm: ATC connect error, Please confirm whether to open? and connect ATC |
| Event | 215015310 | ATC alarm: ATC ambient temperature setting error range 25~30!! |
| Event | 215015311 | ATC alarm: ATC function not yet started!! Please enable ATC active! |
| Event | 215015312 | ATC alarm: ATC version error!! Please check ATC software version!! Must for HS version! |
| Event | 215015313 | ATC alarm: ATC Self-Test Result Fail (+5C) |
| Event | 215015314 | ATC alarm: ATC Self-Test Result Fail (-5C) |
| Event | 215015315 | ATC alarm: ATC Self-Test Result Fail (+0C) |
| Event | 215015316 | ATC alarm: ATC Self-Test Result Fail Exception |
| Event | 215015317 | ATC alarm: ATC Self-Test Result Success |
| Event | 215015318 | ATC alarm: ATC Self-Test Time Out |
| Event | 215015319 | ATC alarm: ATC temperature index always same error |

### Temperature

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 215015100 | Hot plate 1 temperature over upper limit! |
| Event | 215015101 | Hot plate 2 temperature over upper limit! |
| Event | 215015102 | Shuttle 1 temperature over upper limit! |
| Event | 215015103 | Shuttle 2 temperature over upper limit! |
| Event | 215015104 | Head 12 temperature over upper limit! |
| Event | 215015105 | Head 34 temperature over upper limit! |
| Event | 215015106 | Head 56 temperature over upper limit! |
| Event | 215015107 | Head 78 temperature over upper limit! |
| Event | 215015108 | Socket temperature over upper limit! |
| Event | 215015109 | Chamber temperature over upper limit! |
| Event | 215015110 | CCD temperature over upper limit! |
| Event | 215015111 | Aa 1 temperature over upper limit! |
| Event | 215015112 | Ab 1 temperature over upper limit! |
| Event | 215015113 | Ac 1 temperature over upper limit! |
| Event | 215015114 | Ad 1 temperature over upper limit! |
| Event | 215015115 | Ba 1 temperature over upper limit! |
| Event | 215015116 | Bb 1 temperature over upper limit! |
| Event | 215015117 | Bc 1 temperature over upper limit! |
| Event | 215015118 | Bd 1 temperature over upper limit! |
| Event | 215015119 | Aa 2 temperature over upper limit! |
| Event | 215015120 | Ab 2 temperature over upper limit! |
| Event | 215015121 | Ac 2 temperature over upper limit! |
| Event | 215015122 | Ad 2 temperature over upper limit! |
| Event | 215015123 | Ba 2 temperature over upper limit! |
| Event | 215015124 | Bb 2 temperature over upper limit! |
| Event | 215015125 | Bc 2 temperature over upper limit! |
| Event | 215015126 | Bd 2 temperature over upper limit! |
| Event | 215015127 | Heat gun 1 temperature over upper limit! |
| Event | 215015128 | Heat gun 2 temperature over upper limit! |
| Event | 215015129 | Dut 1 temperature over upper limit! |
| Event | 215015130 | Dut 2 temperature over upper limit! |
| Event | 215015131 | Dut 3 temperature over upper limit! |
| Event | 215015132 | Dut 4 temperature over upper limit! |
| Event | 215015133 | Ae 1 temperature over upper limit! |
| Event | 215015134 | Af 1 temperature over upper limit! |
| Event | 215015135 | Ag 1 temperature over upper limit! |
| Event | 215015136 | Ah 1 temperature over upper limit! |
| Event | 215015137 | Be 1 temperature over upper limit! |
| Event | 215015138 | Bf 1 temperature over upper limit! |
| Event | 215015139 | Bg 1 temperature over upper limit! |
| Event | 215015140 | Bh 1 temperature over upper limit! |
| Event | 215015141 | Ae 2 temperature over upper limit! |
| Event | 215015142 | Af 2 temperature over upper limit! |
| Event | 215015143 | Ag 2 temperature over upper limit! |
| Event | 215015144 | Ah 2 temperature over upper limit! |
| Event | 215015145 | Be 2 temperature over upper limit! |
| Event | 215015146 | Bf 2 temperature over upper limit! |
| Event | 215015147 | Bg 2 temperature over upper limit! |
| Event | 215015148 | Bh 2 temperature over upper limit! |
| Event | 215015149 | 2D reader temperature over upper limit! |
| Event | 215015150 | L/B temperature over upper limit! |
| Event | 215015151 | ESD temperature over upper limit! |
| Event | 215015152 | CCD 2 temperature over upper limit! |
| Event | 215015153 | ATC hot air 1 temperature over upper limit! |
| Event | 215015154 | ATC hot air 2 temperature over upper limit! |
| Event | 215015155 | Out Shuttle 1 temperature over upper limit! |
| Event | 215015156 | Out Shuttle 2 temperature over upper limit! |
| Event | 215015157 | Base 1 temperature over upper limit! |
| Event | 215015158 | Base 2 temperature over upper limit! |
| Event | 215015159 | Base 3 temperature over upper limit! |
| Event | 215015160 | Base 4 temperature over upper limit! |
| Event | 215015161 | Base 5 temperature over upper limit! |
| Event | 215015162 | Base 6 temperature over upper limit! |
| Event | 215015163 | Hot plate 2-1 temperature over upper limit! |
| Event | 215015164 | Hot plate 2-2 temperature over upper limit! |
| Event | 215015165 | Shuttle 2-1 temperature over upper limit! |
| Event | 215015166 | Shuttle 2-2 temperature over upper limit! |
| Event | 215015167 | Door 1 temperature over upper limit! |
| Event | 215015168 | Door 2 temperature over upper limit! |
| Event | 215015169 | L/B Up temperature over upper limit! |
| Event | 215015170 | L/B Down temperature over upper limit! |
| Event | 215015194 | Temperature offset exceeds the limit of the machine!! |
| Event | 215015195 | Hot gun 1 air blow not enough. |
| Event | 215015196 | Hot gun 2 air blow not enough. |
| Event | 215015197 | Hot gun 1 air blow too hight. |
| Event | 215015198 | Hot gun 2 air blow too hight. |
| Event | 215015243 | TJ current source error, please check 100uA current board ! |
| Event | 215015244 | Recipe file not exist!! |
| Event | 215015245 | ATC system alarm: Chiller cannot communication!! |
| Event | 215015246 | ATC system alarm: Undefined Serial number!! |
| Event | 215015247 | ATC system alarm: Serial number does not match record!! |
| Event | 215015248 | ATC system alarm: The serial number conflicts with other channels!! |
| Event | 215015249 | TJ current source error, please check 100uA current board ! |
| Event | 215015250 | TJ signal reading is abnormal, please check TJ board ! |
| Event | 215015251 | Power supply thermocouple continuous error!! |
| Event | 215015252 | INT2 thermal couple error!! |
| Event | 215015253 | PV over min temp error!! |
| Event | 215015254 | Tj temp error!! |
| Event | 215015255 | Water flow pressure over high limit!! |
| Event | 215015256 | Water flow low limit!! |
| Event | 215015257 | Handler EMG button is pressed!! |
| Event | 215015258 | Adapter box condensation alarm!! |
| Event | 215015259 | Not enough dry air!! |
| Event | 215015260 | Head water temperature over high limit!! |
| Event | 215015261 | Water flow pressurization control box alarm!! |
| Event | 215015262 | Heating error!! |
| Event | 215015263 | Temperature over low error!! |
| Event | 215015264 | TJ over max temp error!! |
| Event | 215015265 | Water valve box has water leakage alarm! |
| Event | 215015266 | Water flow is 0 and continuous 10 sec!! |
| Event | 215015267 | Water flow is less than the setting minimum flow!! |
| Event | 215015268 | Air cooling system network disconnect!! |
| Event | 215015269 | Air cooling air flow not enough!! |
| Event | 215015270 | Air cooling system compressor is abnormal!! |
| Event | 215015271 | Chiller PCW pressure low limit alarm !! |
| Event | 215015272 | Chiller tank level fault alarm!! |
| Event | 215015273 | Chiller pump overload alarm!! |
| Event | 215015274 | Chiller compressor 1 overload alarm!! |
| Event | 215015275 | Chiller refrigerant high or low pressure alarm!! |
| Event | 215015276 | Chiller refill liquids warning!! |
| Event | 215015277 | Chiller phase error alarm!! |
| Event | 215015278 | Chiller flow low limit alarm!! |
| Event | 215015279 | Chiller temperature controller communication error!! |
| Event | 215015280 | Chiller compressor cut off(low limit)!! |
| Event | 215015281 | Chiller heater cut off(high limit)!! |
| Event | 215015282 | Chiller temperature sensor alarm!! |
| Event | 215015283 | Chiller safely module disable alarm!! |
| Event | 215015284 | Chiller tank temperature over range alarm!! |
| Event | 215015285 | Chiller supply pressure is too high!! |
| Event | 215015286 | Chiller inverter fault alarm!! |
| Event | 215015287 | Chiller circulating fluid discharge pressure too high!! |
| Event | 215015288 | Chiller circulating fluid discharge pressure too low!! |
| Event | 215015289 | Chiller EMO button pressed.!! |
| Event | 215015290 | Chiller compressor 1 overload alarm!! |
| Event | 215015291 | The AVP2 proportional valve of the chiller abnormally!! |
| Event | 215015292 | High level in chiller tank!! |
| Event | 215015293 | Chiller flow signal is abnormal!! |
| Event | 215015294 | Chiller heats up or cools down timeout!! |
| Event | 215015295 | Chiller pump pressure is too high or not enough!! |
| Event | 215015296 | Chiller dry air pressure is too high or not enough!! |
| Event | 215015297 | Chiller vaporizer temperature is too high or too low!! |
| Event | 215015320 | Please check setup name non KL!!! |
| Event | 215015321 | ATC alarm: Please check work temperature does not comply File name! |
| Event | 215015322 | ATC alarm: This setup file must use ATC system!!! Please check again |
| Event | 215015323 | TriTempSysErr: Shuttle 1 motor move over setting times |
| Event | 215015324 | TriTempSysErr: Shuttle 2 motor move over setting times |
| Event | 215015326 | TriTempSysErr: Temp over detect |
| Event | 215015327 | SwTriTempSafeDoor6Lock : Off -> On, SafeLock On |
| Event | 215015328 | TriTempSysErr: Floodgate closed |
| Event | 215015329 | TriTempSysErr: Floodgate open |
| Event | 215015330 | TriTempSysErr: shuttle 1 floodgate not close |
| Event | 215015331 | TriTempSysErr: Shuttle 2 floodgate not close |
| Event | 215015332 | TriTempSysErr: Shuttle 1 floodgate close error |
| Event | 215015333 | TriTempSysErr: Shuttle 2 floodgate close error |
| Event | 215015334 | Manual defrost function is invalid # Machine is running |
| Event | 215015335 | Manual defrost function is invalid # Please check ATC system connection status |
| Event | 215015336 | Manual defrost function is invalid # Please log in for executing Engineer defrost function |
| Event | 215015337 | Manual defrost function is invalid # Working file is not low temperature(<26)) |
| Event | 215015338 | TriTempSysErr: Defrost is terminated by manual |
| Event | 215015339 | Manual defrost function is invalid # Heating time out |
| Event | 215015340 | TriTempSysErr: Defrost finish for manual |
| Event | 215015341 | TriTempSysErr: Fix door open waiting dry fail out humidity alarm |
| Event | 215015342 | TriTempSysErr: Fix door open waiting dry fail dew point |
| Event | 215015343 | TriTempSysErr: In arm or index area door open |
| Event | 215015344 | TriTempSysErr: In arm or index area small door open |
| Event | 215015345 | TriTempSysErr: Docking area open sensor off |
| Event | 215015346 | TriTempSysErr: Dry air not enough |
| Event | 215015347 | Please check Tester dry air not enough |
| Event | 215015348 | TriTempSysErr: DewPointDetectOnIndexArm1 |
| Event | 215015349 | TriTempSysErr: DewPointDetectOnIndexArm2 |
| Event | 215015350 | TriTempSysErr: InIonBarAirNotEnough |
| Event | 215015351 | TriTempSysErr: OutIonBarAirNotEnough |
| Event | 215015352 | TriTempSysErr_HumidityAnomaly1Detect |
| Event | 215015353 | SysErr_LowTemperaturetOverSetTime |
| Event | 215015354 | Manual fast cool down function is fail. Machine is running. |
| Event | 215015355 | Manual fast cool down function is fail. Please check ATC system connection status. |
| Event | 215015356 | Manual fast cool down function is fail. Please login to Engineer level for executing this function. |
| Event | 215015357 | Manual fast cool down function is fail. Working File is not temperature range. |
| Event | 215015358 | Manual fast cool down function is fail. Dew point must below range. |
| Event | 215015359 | Manual fast cool down function is fail. In arm humidity anomaly. |
| Event | 215015360 | Manual fast cool down function is fail. Out arm humidity anomaly. |
| Event | 215015361 | Manual fast cool down function is fail. Please check ATC System Connection status. |
| Event | 215015362 | Manual use air cooler to cool down function is fail. Machine is running. |
| Event | 215015363 | Manual use air cooler to cool down function is fail. Refrigerant system open. |
| Event | 215015364 | Manual use air cooler to cool down function is fail. Please check ATC system cnnection status. |
| Event | 215015365 | Manual use air cooler to cool down function is fail. Please login to Engineer level for executing this function. |
| Event | 215015366 | Manual use air cooler to cool down function is fail. Working file is not temperature range. |
| Event | 215015367 | Manual use air cooler to cool down function is fail. Shuttle 1 floodgate need to open. |
| Event | 215015368 | Manual use air cooler to cool down function is fail. Shuttle 2 floodgate need to open. |
| Event | 215015369 | Manual use air cooler to cool down function is fail. Docking sensor was off. |
| Event | 215015370 | Manual use air cooler to cool down function is fail. Temperature is within the set range. |
| Event | 215015371 | Manual use air cooler to cool down function is fail. ATC was disconnected. |
| Event | 215015372 | Manual use air cooler to cool down function is fail. Cooling is complete. |
| Event | 215015373 | Manual change kit function is fail. Machine is running. |
| Event | 215015374 | Manual change kit function is fail. Please check ATC system must at disConnect status. |
| Event | 215015375 | Manual change kit function is fail. Please login to Engineer level for executing this function. |
| Event | 215015376 | Manual change kit function is fail. Must finish [Clean out]. |
| Event | 215015377 | Manual change kit function. Change kit is complete. |
| Event | 215015378 | Out shuttle 1 temperature read error. |
| Event | 215015379 | Out shuttle 2 temperature read error. |
| Event | 215015380 | Index 1 temperature read error. |
| Event | 215015381 | Index 2 temperature read error. |
| Event | 215015382 | Index 3 temperature read error. |
| Event | 215015383 | Index 4 temperature read error. |
| Event | 215015384 | Base 1 temperature read error. |
| Event | 215015385 | Base 2 temperature read error. |
| Event | 215015386 | Base 3 temperature read error. |
| Event | 215015387 | Base 4 temperature read error. |
| Event | 215015388 | Base 5 temperature read error. |
| Event | 215015389 | Base 6 temperature read error. |
| Event | 215015390 | Over temperature |
| Event | 215015402 | Air stream air volume not enough! |
| Event | 215015403 | Soak time setting error! |
| Event | 215015404 | Work temperature setting error! |
| Message | 315015401 | Chamber boost function finish. |

### Tester I/F

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 207000701 | Low Yield Alarm! |
| Event | 207000702 | Site Yield Different over setting! |
| Event | 207000703 | Arm Site Yield Different over setting! |
| Event | 207000704 | No BARCODE? COMMAND ERROR!! |
| Event | 207000705 | Low Yield Alarm!(By Total) |
| Event | 207000706 | Interval Total Yield Difference Alert |
| Event | 207000707 | Site To Site Yield Alert! |
| Event | 207000708 | Head To Head Yield Alert! |
| Event | 207000709 | Site yield% Over Alert! |
| Event | 207000710 | GPIB Wakeup TimeOut |
| Event | 207000711 | DUTCHK ECHO NG!! Please check site mapping!! |
| Event | 207000712 | Alarm 4 Continue Alert! |
| Event | 207000713 | Site To Site Yield Alert |
| Message | 307000714 | EOT monitor time continue Over 5 time. |
| Event | 207000715 | No receive data,take out arm all IC |
| Event | 207000716 | Tester Time out,No receive data,take out arm all IC |
| Event | 207007100 | Change Clean Pad |
| Event | 207007301 | Socket consecutive failure |
| Event | 207007318 | RS232 data format error!! |
| Event | 207007319 | TTL data format error!! |
| Event | 207007320 | GPIB data format error!! |
| Event | 207007321 | Arm1: consecutive failure |
| Event | 207007322 | Consecutive Pass Error! |
| Event | 207007323 | Bin yield too high error! |
| Event | 207007324 | Loading count exceed seting value |
| Event | 207007325 | Contact count exceed seting value |
| Event | 207007329 | Arm2: consecutive failure |
| Event | 207007331 | Special Bin Socket Consecutive Failure |
| Event | 207007332 | Arm1: Special Bin Consecutive Failure |
| Event | 207007333 | Arm2: Special Bin Consecutive Failure |
| Event | 207007334 | Test resilt: all site are same specific fail bin |
| Event | 207007352 | Tester time up error |
| Jam | 107007352 | Tester time up error |
| Event | 207007356 | Category setting error |
| Event | 207007357 | Category yield over limit |
| Event | 207007358 | Category count over failure |
| Event | 207007359 | Test bin error, take out all IC!! |
| Event | 207007400 | Auto site map can not close site!! |
| Event | 207007401 | Barcode Function No Open! |
| Event | 207000717 | O/S alarm yield over setting! |
| Event | 207000718 | Continuous site yield different over setting! |
| Event | 207000719 | Continuous low yield alarm! |
| Event | 207000720 | O/S alarm yield warning! |
| Event | 207000721 | Low yield alarm!(By Site) |
| Event | 207000722 | Previous yield difference warning! |
| Event | 207000723 | Previous yield difference alarm! |
| Event | 207000724 | Low yield special 1 |
| Event | 207000725 | Low yield special 2 |
| Event | 207000726 | Arm 1 site yield difference over setting! |
| Event | 207000727 | Arm 2 site yield difference over setting! |
| Event | 207000729 | Continuously lower alarm in noraml area!(By Adaptive) |
| Event | 207000730 | Continuously lower alarm in min area!(By Adaptive) |
| Event | 207000732 | Test arm 2DID is not on the allow list! |
| Event | 207007317 | Test flow of RS232 error - BA without CE command!! |
| Event | 207007326 | Test result shows closed site has bin error!! |
| Event | 207007327 | Test flow of GPIB error - binon without 0x41! |
| Event | 207007328 | Test flow of GPIB error - binon without fullsite command!! |
| Event | 207007335 | By bin site to site yield compare over setting! |
| Event | 207007336 | Arm 1: by bin site to site yield compare over setting! |
| Event | 207007337 | Arm 2: by bin site to site yield compare over setting! |
| Event | 207007360 | ART special bin alarm! |
| Event | 207007361 | ART FT low yield! |
| Event | 207007362 | SECS GEM consecutive failure! |
| Event | 207007402 | Site Mapping Check Fail! Must Pass Bin,need Do again! |
| Event | 207007403 | Initial Site Mapping Detect Continue Fail! |
| Event | 207007404 | Idle Site Mapping Detect Continue Fail! |
| Event | 207007405 | Interval Site Mapping Detect Continue Fail! |
| Event | 207007406 | Manual Site Mapping Detect Continue Fail! |
| Event | 207007451 | Barcode Pin1 Function Enabled! |
| Message | 307000731 | STOP from tester, waiting for user to operate. |
| Message | 307000732 | LB Undocking, requires QA to confirm the compensation documents. |
| Message | 307000733 | PAUSE from tester, waiting for RESUME. |
| Message | 307000736 | Received Tester PAUSE Request > Refer To Internal System Error Message! |
| Message | 307007399 | Tester loading program ready. |

### Tray Arm

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 106000606 | Tray arm cover up fail! |
| Jam | 106000616 | Check loader sensor SnLoaderSureTray and SnLoaderPreDete, tray arm catch tray error |
| Jam | 106000617 | Color tray ID NULL,take out tray |
| Jam | 106000619 | catch Load tray Lose,take out tray |
| Jam | 106000620 | Take out Color tray |
| Event | 206000618 | Index need take out IC |
| Message | 306000650 | Tray arm - Pick tray from Loader |
| Message | 306000651 | Tray arm - Pick tray from Empty |
| Message | 306000652 | Tray arm - Pick tray from Color |
| Message | 306000653 | Tray arm - Pick tray from Auto 1 |
| Message | 306000654 | Tray arm - Pick tray from Auto 2 |
| Message | 306000655 | Tray arm - Pick tray from Auto 3 |
| Message | 306000656 | Tray arm - Pick tray from Auto 4 |
| Message | 306000657 | Tray arm - Pick tray from Auto 5 |
| Message | 306000658 | Tray arm - Pick tray from Auto 6 |
| Message | 306000670 | Tray arm - place tray to Loader |
| Message | 306000671 | Tray arm - place tray to Empty |
| Message | 306000672 | Tray arm - place tray to Color |
| Message | 306000673 | Tray arm - place tray to Auto 1 |
| Message | 306000674 | Tray arm - place tray to Auto 2 |
| Message | 306000675 | Tray arm - place tray to Auto 3 |
| Message | 306000676 | Tray arm - place tray to Auto 4 |
| Message | 306000677 | Tray arm - place tray to Auto 5 |
| Message | 306000678 | Tray arm - place tray to Auto 6 |

### Tray Loader

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 109000901 | Loader tray goes inside arrival time up error! |
| Jam | 109000902 | Loader tray lock cylinder(side pusher) error! |
| Jam | 109000903 | Loader tray lock cylinder(backward locker) error! |
| Jam | 109000904 | Tray separation cylinder(upper) error |
| Jam | 109000905 | Tray separation cylinder(middle) error |
| Jam | 109000906 | Tray separation cylinder(lower) error |
| Jam | 109000907 | Tray separation error |
| Jam | 109000908 | Tray pick-up error |
| Jam | 109000909 | Tray setting error |
| Jam | 109000911 | Tray arm put tray to Loader and Loader do not detect tray Error! |
| Jam | 109000912 | Loader tray goes outside arrival time up error! |
| Jam | 109000913 | Loader tray goes up or down arrival time up error! |
| Event | 209000914 | Loader Tray was lost Please check Sensor |
| Event | 209000915 | Loader_Car Tray was lost Please check Sensor |
| Message | 309000920 | There is no tray |
| Message | 309000921 | Loader tray exceed limit.Remove tray!! |
| Message | 309000922 | Please remove the loader tray manually! |
| Message | 309000923 | Please check if there has any device on loader tray! |
| Jam | 109000929 | Loader tray miss error! |
| Event | 209000930 | OCR Vision program off error!! |
| Event | 209000931 | OCR no golsen sample error!! |
| Event | 209000932 | OCR no ROI data error!! |
| Event | 209000933 | OCR no Match data error!! |
| Event | 209000934 | OCR start time out error!! |
| Event | 209000935 | OCR start not ready error!! |
| Event | 209000940 | OCR inspection NG!! |
| Event | 209000941 | OCR inspection analysis Error!! |
| Event | 209000942 | OCR inspection grab time out!! |
| Event | 209000943 | OCR inspection read image error!! |
| Event | 209000944 | OCR inspection time out!! |
| Event | 209000951 | Loader Tray color detect error!! |
| Event | 209000957 | OCR PGM Time Out!! |
| Event | 209000958 | OCR PGM NG!! |
| Event | 209000959 | OCR PGM OK or NG Time Out!! |
| Event | 209000960 | OCR PGM NG!! |
| Event | 209000961 | Loader Tray input will over Total Tray |
| Event | 209000962 | Loader Tray input below Total Tray |
| Event | 209000963 | Load No Tray ART |
| Event | 209000964 | Run tray modal Load have IC,manual take out fix tray,put new load tray |
| Event | 209000997 | OCR Inspect NG! |
| Event | 209000998 | OCR NO IC! |
| Event | 209000999 | OCR Consecutive NO IC! |
| Event | 209001000 | OCR Word Type Error! |
| Jam | 109009102 | Loader tray device floating error! |
| Event | 209000916 | There is a tray on Loader (the entrance)! |
| Event | 209000952 | Tray in loader can not retest!! Please remove it manually |
| Event | 209000953 | Un-expected Bundle ID read! |
| Event | 209000970 | Tray ID communication time out! |
| Event | 209000971 | Tray ID exposure time out! |
| Event | 209000973 | Tray ID exposure result error! |
| Event | 209000974 | Tray ID data no exist! |
| Event | 209000975 | Tray ID data count no OK! |
| Event | 209000976 | Tray map route Z: disk fail! |
| Event | 209000980 | Tray map communication time out! |
| Event | 209000981 | Tray map exposure time out! |
| Event | 209000982 | Tray map exposure position error! |
| Event | 209000983 | Tray map exposure Result error! |
| Event | 209000990 | Device remain communication time out! |
| Event | 209000991 | Device remain exposure time out! |
| Event | 209000992 | Device remain exposure position error! |
| Event | 209000993 | Device remain exposure result error! |
| Event | 209000994 | Tray has device remain! |
| Event | 209000995 | OCR barcode is not in the file. |
| Event | 209000996 | OCR check by Lot is duplicate error. |
| Event | 209009100 | OCR word type error! |
| Event | 209009101 | Auto tray end enable has IC or have loss device issue. Please check loader tray! |
| Event | 209009103 | Loader tray CCD read data error! |
| Event | 209009104 | Catch loader tray CCD read data time out! |
| Event | 209009105 | Loader tray CCD has IC ! |
| Event | 209009106 | <TrayMap>Trigger CCD grab. |
| Event | 209009107 | <AutoMove>Auto 1 tray finish, call AGV |
| Event | 209009108 | <AutoMove>Auto 2 tray finish, call AGV |
| Event | 209009109 | <AutoMove>Auto 3 tray finish, call AGV |
| Event | 209009110 | <TrayMap>Tray End CCD Grab. |
| Event | 209009111 | Abnormal EP traffic Alarm |
| Event | 209009112 | OCR word length error! |
| Event | 209009113 | OCR disconnect error. |
| Event | 209009200 | Loader tray color is not allowed |
| Message | 309000917 | Loader SnLoaderUpSafedetect Error!!!! |
| Message | 309009222 | This tray is high risk with residual IC on this tray, should remove input tray manaully. |

### Tray Unloader 1

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 111001101 | Auto 1 tray goes inside arrival time up error! |
| Jam | 111001102 | Auto 1 tray lock cylinder(side pusher) error! |
| Jam | 111001103 | Auto 1 tray lock cylinder(backward locker) error! |
| Jam | 111001104 | Auto 1 tray separation cylinder(upper) error |
| Jam | 111001106 | Auto 1 tray separation cylinder(lower) error |
| Jam | 111001107 | Auto 1 tray separation cylinder(push-up) error |
| Jam | 111001108 | Auto 1 tray positioning error |
| Jam | 111001109 | Auto 1 tray setting error |
| Jam | 111001110 | Auto 1 tray Device floating error |
| Jam | 111001111 | Tray arm put tray to Auto 1 and Auto 1 do not detect tray Error! |
| Jam | 111001112 | Auto 1 tray goes outside arrival time up error! |
| Jam | 111001113 | Auto 1 tray goes up or down arrival time up error! |
| Message | 311001120 | Auto 1 tray Unloader is filled with trays |
| Message | 311001121 | No tray on Unloader Auto 1 |
| Message | 311001122 | Please remove the Auto 1 tray manually! |
| Message | 311001123 | Please check if there has any device on Auto 1 tray! |
| Event | 211001130 | Auto 1 tray miss error! |
| Jam | 111001131 | Auto 1 tray arrival time up error |
| Jam | 111001132 | Auto 1 tray lock cylinder (forward) error |
| Jam | 111001133 | Auto 1 tray lock cylinder (backward) error |
| Jam | 111001134 | Auto 1 tray separation cylinder (upper) error |
| Jam | 111001135 | Auto 1 tray separation cylinder (middle) error |
| Jam | 111001136 | Auto 1 tray separation cylinder (lower) error |
| Jam | 111001137 | Auto 1 tray separation cylinder (push-up) error |
| Jam | 111001138 | Auto 1 tray positioning error |
| Jam | 111001139 | Auto 1 tray setting error |
| Jam | 111001140 | Auto 1 tray separation (push-up) error |
| Event | 211001151 | Auto 1 Tray color detect error!! |
| Jam | 111001161 | Auto 1 tray arrival time up error |
| Jam | 111001162 | Auto 1 tray lock cylinder (forward) error |
| Jam | 111001163 | Auto 1 tray lock cylinder (backward) error |
| Jam | 111001164 | Auto 1 tray separation cylinder (upper) error |
| Jam | 111001165 | Auto 1 tray separation cylinder (middle) error |
| Jam | 111001166 | Auto 1 tray separation cylinder (lower) error |
| Jam | 111001167 | Auto 1 tray separation error |
| Jam | 111001168 | Auto 1 tray pick-up error |
| Jam | 111001169 | Auto 1 tray setting error |
| Jam | 111001130 | Auto 1 tray miss error! |

### Tray Unloader 2

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 112001201 | Auto 2 tray goes inside arrival time up error! |
| Jam | 112001202 | Auto 2 tray lock cylinder(side pusher) error! |
| Jam | 112001203 | Auto 2 tray lock cylinder(backward locker) error! |
| Jam | 112001204 | Auto 2 tray separation cylinder (upper) error |
| Jam | 112001206 | Auto 2 tray separation cylinder (lower) error |
| Jam | 112001207 | Auto 2 tray separation (push-up) error |
| Jam | 112001208 | Auto 2 tray positioning error |
| Jam | 112001209 | Auto 2 tray setting error |
| Jam | 112001210 | Auto 2 tray Device floating error |
| Jam | 112001211 | Tray arm put tray to Auto 2 and Auto 2 do not detect tray Error! |
| Jam | 112001212 | Auto 2 tray goes outside arrival time up error! |
| Jam | 112001213 | Auto 2 tray goes up or down arrival time up error! |
| Message | 312001220 | Auto 2 tray Unloader is filled with trays |
| Message | 312001221 | No tray on Unloader Auto 2 |
| Message | 312001222 | Please remove the Auto 2 tray manually! |
| Message | 312001223 | Please check if there has any device on Auto 2 tray! |
| Event | 212001230 | Auto 2 tray miss error! |
| Event | 212001251 | Auto 2 Tray color detect error!! |
| Jam | 112001270 | Auto 2 tray miss error! |

### Tray Unloader 3

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Jam | 113001301 | Auto 3 tray goes inside arrival time up error! |
| Jam | 113001302 | Auto 3 tray lock cylinder(side pusher) error! |
| Jam | 113001303 | Auto 3 tray lock cylinder(backward locker) error! |
| Jam | 113001304 | Auto 3 tray separation cylinder (upper) error |
| Jam | 113001306 | Auto 3 tray separation cylinder (lower) error |
| Jam | 113001307 | Auto 3 tray separation (push-up) error |
| Jam | 113001308 | Auto 3 tray positioning error |
| Jam | 113001309 | Auto 3 tray setting error |
| Jam | 113001310 | Auto 3 tray Device floating error |
| Jam | 113001311 | Tray arm put tray to Auto 3 and Auto 3 do not detect tray Error! |
| Jam | 113001312 | Auto 3 tray goes outside arrival time up error! |
| Jam | 113001313 | Auto 3 tray goes up or down arrival time up error! |
| Message | 313001320 | Auto 3 tray Unloader is filled with trays |
| Message | 313001321 | No tray on Unloader Auto 3 |
| Message | 313001322 | Please remove the Auto 3 tray manually! |
| Message | 313001323 | Please check if there has any device on Auto 3 tray! |
| Event | 213001330 | Auto 3 tray miss error! |
| Event | 213001351 | Auto 3 Tray color detect error!! |
| Jam | 113001330 | Auto 3 tray miss error! |

### Unit 00

| Class | ALID | Alarm Text |
|:-----:|------|------------|
| Event | 200000000 | Uknown alarm code! |
