# HT9045_CONFIG 欄位速查：群組 [G] Visible（UI 顯示控制）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Gxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 27 個欄位，分屬 25 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| G01 | Show test rate | `cbG01` | `bG01Show_TestRate` | bool | `bG01Show_TestRate` | 35400 | ECBool | Show test rate |  |
| G04 | Show fail alarm count | `cbG04` | `bG04ShowFailAlarmCount` | bool | `bG04ShowFailAlarmCount` | 35401 | ECBool | Show fail alarm count |  |
| G05 | Show motor speed | `cbG05` | `bG05ShowSpeedMessage` | bool | `bG05ShowSpeedMessage` | 35402 | ECBool | Show motor speed |  |
| G06 | Home push Z1 start initial | `cbG06` | `bG06HomeinitialCheckZ1` | bool | `bHomeinitialCheckZ1` | 35403 | ECBool | Home push Z1 start initial | kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認 |
| G07 | Support multi color for error bin | `cbG07` | `bG07MultiColorForFailBin` | bool | `bSupportMultiColorForFailBin` | 35404 | ECBool | Support multi color for error bin | Steven 20160310 : 改成有顏色的fail bin |
| G08 | Show '#39'Are you sure'#39' message after alarm. | `cbG08` | `bG08VisibleAreyousure` | bool | `bVisibleAreyousure` | 35405 | ECBool | Show 'Are you sure' message after alarm. | wei 20160413 顯示Are you sure |
| G09 | Need password when edit site map | `cbG09` | `bG09NeedPasswordWhenEditSiteMap` | bool | `bNeedPasswordWhenEditSiteMap` | 35406 | ECBool | Need password when edit site map | JerryYang 20160425 修改Site map需要密碼 |
| G10 | Show immediate UPH | `cbG10` | `bG10ShowImmediateUPH` | bool | `bG10ShowImmediateUPH` | 35407 | ECBool | Show immediate UPH | Steven 20160727 : Show immediate UPH |
| G11 | ASE Report  record | `cbG11` | `bG11ASEReport` | bool | `bG11ASEReport` | 35408 | ECBool | ASE Report  record | kevin 20170306 |
| G12 | Contract high manual send test message | `cbG12` | `bG12ContractModeManualMessage` | bool | `bG12ContractModeManualMessage` | 35409 | ECBool | Contract high manual send test message | kevin 20180221 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE |
| G13 | Show Temp. offset on contact page. | `cbG13` | `bG13ShowTempOffsetOnContact` | bool | `bG13ShowTempOffsetOnContact` |  | ECBool | Show Temp. offset on contact page. |  |
| G14 | Start-up warning                            sec | `cbG14` | `bG14UseStartSoundAlarm` | bool | `bG14UseStartSoundAlarm` |  | ECBool | Start-up warning sec | kevin 20201116  Start 發出聲音 不動 5sec |
| G14 | Start-up warning                            sec | `edG14` | `iG14StartWarrTime` | int | `iStartWarrTime` |  | ECBool | Start-up warning sec |  |
| G15 | Load Input Count | `cbG15` | `bG15LoadInputCount` | bool | `bG15LoadInputCount` |  | ECBool | Load Input Count | kevin 20211106 輸入顆數達成就 Clean out |
| G15 | Load Input Count | — | `iG15KeyInTotal` | int | — |  | — | Load Input Count | kevin 20211106 輸入抽測數量     //Steven 20230308 : 沒用到Mark |
| G16 | Bin Display has communication error, need to alarm. | `chkG16` | `bG16BinDispNeedAlarm` | bool | `bG16BinDispNeedAlarm` |  | ECBool | Bin Display has communication error, need to alarm. | Steven 20211130 : JSCC要求Bin顯示器異常要alarm |
| G17 | Load CCD map for Ase-Kh | `cbG17` | `b17bUseLoadCCDTrayMap` | bool | `b17bUseLoadCCDTrayMap` |  | ECBool | Load CCD map for Ase-Kh | kevin 20220610 ASE_KH call load CCD 拍照 |
| G17-1 | Load CCD map for tray end Ase-Kh | `cbG17_1` | `b17bUseLoadCCDTrayMapTrayend` | bool | `b17bUseLoadCCDTrayMapTrayend` |  | ECBool | Load CCD map for tray end Ase-Kh | kevin 20230530 trayend 拍照 |
| G18 | Unload put empty tray | `edG18Auto1Count` | `iUnloaderTrayCount` | int[] | `iAuto1Count` | -- | -- | Unload put empty tray | kevin 20220610 add  Auto 123 判斷滿TRAY 補空TRAY |
| G18-1 | Use Tray Map | `cbG18_1` | `b18bUseAutoTrayMap` | bool | `b18bUseAutoTrayMap` |  | ECBool | Use Tray Map | kevin 20220610 AUTO 123 先放空盤，數量 |
| G18-2 | Double unload tray | `cbG18_2` | `b18bDoubleUnloadTray` | bool | `b18bDoubleUnloadTray` |  | ECBool | Double unload tray | kevin 20220610 add Unload Tray 2  倍 的設定 |
| G19 | Shuttle sensor add Autoclean  Parmameter | `cbG19` | `b19InSHAutoCleanPA` | bool | `b19InSHAutoCleanPA` |  | ECBool | Shuttle sensor add Autoclean  Parmameter | kevin 20230530 add Autoclean shuttle define new 參數 |
| G20 | Fix full tray wait on manual position when using AGV robot | `cbG20` | `b20FixfullWaitpos` | bool | `b20FixfullWaitpos` |  | ECBool | Fix full tray wait on manual position when using AGV robot | kevin 20230816 out arm move Wait |
| G21 | Color full tray no alarm (XP,Telix) | `cbG21` | `b21colorfulltray` | bool | `b21colorfulltray` |  | ECBool | Color full tray no alarm (XP,Telix) | kevin 20230918 colortray no Alarm |
| G22 | Notice takeout tray after contact mode. | `cbG22` | `bG22NoticeTakeoutTray` | bool | `bG22NoticeTakeoutTray` |  | ECBool | Notice takeout tray after contact mode. | JerryYang 20231218 : G22提醒人員取tray功能 |
| G23 | Disable Show Function Status. | `cbG23` | `bG23DiasbleFuncStatusView` | bool | `bG23DiasbleFuncStatusView` |  |  | Disable Show Function Status. | RogerYang 20250728 Vtest設開關決定要不要顯示View |
| G24 | Disable Show SECS/GEM Status. | `cbG24` | `bG24DisableSECSGEMStatus` | bool | `bG24DisableSECSGEMStatus` |  |  | Disable Show SECS/GEM Status. | RogerYang 20251222 : 丁曉東要求新增選項(G24)是否顯示 |

---


## 未綁定 UI 元件的欄位

_無。所有 `G##` 開頭的欄位均已在主表中列出。_
