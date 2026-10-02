# HT9045_CONFIG 欄位速查：群組 [M] Monitor（Monitor 強制模式）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Mxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 15 個欄位，分屬 15 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| M01 | Enable monitor funciton | `cbM01` | `bM01EnableMonitorFunction` | bool | `bEnableMonitorFunction` |  | ECBool | Enable monitor funciton | ChungHung 20131009 add for SCK |
| M01-1 | Contact mode must select different speed | `cbM01_01` | `bM0101ContactModeUseDifferentSpeed` | bool | `bMonitorContactModeMustSelectDifferentSpeed` |  |  | Contact mode must select different speed | ChungHung 20131009 add for SCK |
| M01-2 | Site yield different must on | `cbM01_02` | `bM0102SiteYieldDifferentMustOn` | bool | `bMonitorSiteYieldDifferentMustOn` |  |  | Site yield different must on | ChungHung 20131009 add for SCK |
| M01-3 | Site yield continue fail by socket must on | `cbM01_03` | `bM0103ContinueFailBySocketMustOn` | bool | `bMonitorSiteYieldContinueFailBySocketMustOn` |  |  | Site yield continue fail by socket must on | ChungHung 20131009 add for SCK |
| M01-4 | Site yield continue fail by head must on | `cbM01_04` | `bM0104ContinueFailByHeadMustOn` | bool | `bMonitorSiteYieldContinueFailByHeadMustOn` |  |  | Site yield continue fail by head must on | ChungHung 20131009 add for SCK |
| M01-5 | In/Out Arm device check must on | `cbM01_05` | `bM0105InOutArmDeviceCheckMustOn` | bool | `bMonitorInOutArmDeviceCheckMustOn` |  |  | In/Out Arm device check must on | ChungHung 20131009 add for SCK |
| M01-6 | Index destory check must on | `cbM01_06` | `bM0106IndexDeviceCheckDestoryMustOn` | bool | `bMonitorIndexArmDeviceCheckDestoryMustOn` |  |  | Index destory check must on | ChungHung 20131009 add for SCK |
| M01-7 | Auto speed must on | `cbM01_07` | `bM0107AutoSpeedMustOn` | bool | `bMonitorAutoSpeedMustOn` |  |  | Auto speed must on | ChungHung 20131009 add for SCK |
| M01-8 | Every first device have initial delay time | `cbM01_08` | `bM0108EveryFirstDeviceMustOn` | bool | `bMonitorEveryFirstDeviceMustEnabled` |  |  | Every first device have initial delay time | ChungHung 20141210 add for SCK want to add Monitor every first device have delay time |
| M01-9 | If handler RTC off check piggy back function | `cbM01_09` | `bM0109RTCOffCheckYieldPiggyBack` | bool | `bM0109RTCOffCheckYieldPiggyBack` |  |  | If handler RTC off check piggy back function | ChungHung 20150613 add for SCK want to check and show message |
| M01-10 | Disable [I12] function | `cbM01_10` | `bM1010Disable_I12` | bool | `bM1010Disable_I12` |  |  | Disable  function | Steven 20160727 : For SCK |
| M01-12 | Enable auto clean function | `cbM01_12` | `bM1012EnableAutoClean` | bool | `bM1012EnableAutoClean` |  |  | Enable auto clean function | Steven 20160727 : For SCK |
| M01-13 | Enable 2DID function | `cbM01_13` | `bM1013Enable2DID` | bool | `bM1013Enable2DID` |  |  | Enable 2DID function | Steven 20160727 : For SCK |
| M01-14 | Enable ATC function | `cbM01_14` | `bM1014EnableATC` | bool | `bM1014EnableATC` |  |  | Enable ATC function | Steven 20191129 : For SCK |
| M1015 |  | — | `bM1015EnableBottom2DID` | bool | — |  | — |  | JerryYang 20250120 : add |

---


## 未綁定 UI 元件的欄位

_無。所有 `M##` 開頭的欄位均已在主表中列出。_
