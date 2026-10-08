# SECS SV/EC 被 `#if 0` 閘住的登錄 —— 逐筆「缺什麼」

> AI(W906-P4-SECS) 20260920 由 `tools/secs_gate_census.py` 產生。
> **不要手改**；改了會與原始碼漂開。重跑那支工具即可。

P4 完成條件的最後一項：「174 筆被閘的各有『缺什麼』的紀錄」。

★ 理由**不是我判斷的**，是翻譯當下寫在每個 `#if 0` 上面的 GATE 註記；
  這份文件只是把它們蒐集起來對帳。自己重寫理由等於把翻譯者的
  現場判斷換成事後猜測。

## SV —— `SECSGEM/uHGemHT9045_SV.cpp`

閘住的區塊 **20** 個，涵蓋登錄 **135** 筆。

| 行 | GATE | 筆數 | ID | 缺什麼（原始碼裡的理由） |
|---|---|---|---|---|
| 455 | G2 | 2 | 1012, 1013 | GATE [G2] -- golden :74-75.  SVID 1012/1013 -- fMain->edTorue0 / edTorue1 (golden main.h:466/467, TEdit*): NOT members of forms/fMain.h's TfMain. |
| 459 | G3 | 3 | 1014, 1015, 1016 | GATE [G3] -- golden :76-78.  SVID 1014/1015/1016 -- iSECS_GEM_PPSIGNALTOWER_CONTROL_RED/YELLOW/GREEN: declared extern in ckernel.cpp:1276-1278, DEFINED NOWHERE in the port. |
| 481 | G19 | 2 | 1038, 1039 | GATE [G19] -- golden :96-97.  SVID 1038/1039 -- &iGrossUPH / &iNetUPH: DECLARED (cmydef.h:5905/:5904) but the only DEFINITIONS (cmydef.cpp:5993/:5992) sit inside `#if 0 // TODO(W6)` (cmydef.cpp:5816) -> unde… |
| 487 | G4 | 1 | 1041 | GATE [G4] -- golden :100-100.  SVID 1041 -- fMain->lbEPenconder (golden main.h:796, TPanel*): NOT a member of forms/fMain.h's TfMain. |
| 494 | G5 | 2 | 1047, 1048 | GATE [G5] -- golden :105-106.  SVID 1047/1048 -- fContact->lblEPValueKg: NOT a member of atester_shims.h's TfContactShim. |
| 606 | G20 | 16 | 1164, 1165, 1166, 1167, 1168, 1169 … | GATE [G20] -- golden :215-230.  SVID 1164-1179 -- &iSVByBinCount[0..15]: DECLARED (cmydef.h:5916), definition (cmydef.cpp:6000) inside the same `#if 0 // TODO(W6)` block -> undefined at LINK. |
| 625 | G21 | 1 | 1191 | GATE [G21] -- golden :232-232.  SVID 1191 -- &iSV_ErrBinCnt: DECLARED (cmydef.h:5911), definition (cmydef.cpp:5999) inside the same `#if 0 // TODO(W6)` block -> undefined at LINK. |
| 704 | G6 | 8 | 1351, 1352, 1355, 1356, 1359, 1360 … | GATE [G6] -- golden :309-316.  SVID 1351-1364 (iATC_Use_Heat_Count<=4 branch) -- fLotInfo->pl_ATCTempHead01..04 / pl_ATCRefHead01..04: NOT members of forms/fLotInfo.h's TfLotInfo. |
| 717 | G7 | 16 | 1351, 1352, 1353, 1354, 1355, 1356 … | GATE [G7] -- golden :320-335.  SVID 1351-1366 (iATC_Use_Heat_Count<=8 branch) -- fLotInfo->pl_ATCTempHead01..08 / pl_ATCRefHead01..08: ditto. |
| 773 | G8 | 4 | 2003, 2004, 2005, 2007 | GATE [G8] -- golden :374-377.  SVID 2003/2004/2005/2007 -- fContact->edForcePerDeviceKG / edForcePerPinN / edAirForceN / edForcePerDeviceN: not on TfContactShim. |
| 784 | G9 | 3 | 2018, 2019, 2020 | GATE [G9] -- golden :383-385.  SVID 2018/2019/2020 -- fContact->edAirKPA / lblReadEP / edSetKg: not on TfContactShim. |
| 792 | G10 | 1 | 2024 | GATE [G10] -- golden :389-389.  SVID 2024 -- fContact->lblDieForceEP: not on TfContactShim.  <== Eastsun 20260526 #026-1.79 |
| 831 | G11 | 9 | 2240, 2241, 2242, 2243, 2244, 2245 … | GATE [G11] -- golden :426-434.  SVID 2240-2248 -- fShowBinSelect->lblMag1..9: the fShowBinSelect global/form does not exist in the port at all. |
| 844 | G12 | 1 | 2631 | GATE [G12] -- golden :437-437.  SVID 2631 -- fContact->rgHandlerMode: not on TfContactShim.  <== Eastsun 20260526 #026-4.A8 |
| 848 | G13 | 6 | 2665, 2666, 2667, 2754, 2755, 2756 | GATE [G13] -- golden :439-445.  SVID 2665-2667 + 2754-2756 -- fTrayAssignment->edAuto1Type..edAuto6Type: the fTrayAssignment global/form does not exist in the port at all. |
| 870 | G14 | 2 | 9003, 9004 | GATE [G14] -- golden :459-460.  SVID 9003/9004 -- fCleaning->edAutoCleanAirForce_Kg / _N: not members of forms/fCleaning.h's TfCleaning (edCleaningCount, SVID 9001, IS -- and stays ACTIVE). |
| 1118 | G15 | 2 | 37200, 37201 | GATE [G15] -- golden :705-706.  SVID 37200/37201 -- fLotInfo->lbESDReportData / lbESDDecayReportData: not on TfLotInfo. |
| 1154 | G16 | 8 | 37501, 37502, 37503, 37504, 37505, 37506 … | GATE [G16] -- golden :739-746.  SVID 37501-37508 -- fObserver->lbSerialNumber01..04 / lbFirmwareNumber01..04: not on atester_shims.h's TfObserverShim.  Ifor 20170320 add SECS GEM ATC Power Supply Firmware Ve… |
| 1270 | G17 | 28 | 43300, 43301, 43302, 43303, 43304, 43305 … | GATE [G17] -- golden :853-880.  SVID 43300-43327 -- fGroundMan->labValue_0_2..labValue_3_5: the fGroundMan global/form does not exist in the port at all. |
| 1301 | G18 | 20 | 43350, 43351, 43352, 43353, 43354, 43355 … | GATE [G18] -- golden :882-901.  SVID 43350-43369 -- fSmartDiagnostic->iPushAvgTime[0..9] / iPopAvgTime[0..9]: the fSmartDiagnostic global/form does not exist in the port at all. |

## EC —— `SECSGEM/uHGemHT9045_EC.cpp`

閘住的區塊 **13** 個，涵蓋登錄 **39** 筆。

| 行 | GATE | 筆數 | ID | 缺什麼（原始碼裡的理由） |
|---|---|---|---|---|
| 481 | — | 1 | 1554 | GATE g1 -- golden :78-78 (M:edCustomerDevice); see GATE REGISTER above |
| 485 | — | 1 | 1581 | GATE g2 -- golden :80-80 (M:sInfo_Customer); see GATE REGISTER above |
| 489 | — | 13 | 1583, 1584, 1585, 1586, 1587, 1588 … | GATE g3 -- golden :82-94 (M:sInfo_CurrQty,M:sInfo_CustDevGup,M:sInfo_CustLotID,M:sInfo_DeviceName,M:sInfo_HandlerID,M:sInfo_OperatorID,M:sInfo_ProgramName,M:sInfo_ReportCnt,M:sInfo_Stage,M:sInfo_Step,M:sInfo… |
| 506 | — | 1 | 1596 | GATE g4 -- golden :97-97 (M:edtBarcodeRecipe); see GATE REGISTER above  ==> Eastsun 20260527 整合#027-1.MR.EC1597 bBarCodeMultiRecipe SECS EC :KYEC |
| 605 | — | 1 | 2632 | GATE g5 -- golden :194-194 (M:chkShuttle); see GATE REGISTER above |
| 760 | — | 1 | 3540 | GATE g6 -- golden :347-347 (M:tSiteMap); see GATE REGISTER above |
| 1358 | — | 6 | 9511, 9512, 9513, 9514, 9515, 9516 | GATE g16 -- golden :924-929 (M:chkAutoCleanMode1,M:chkAutoCleanMode2,M:chkAutoCleanMode3,M:chkAutoCleanMode4,M:chkAutoCleanMode5,M:chkAutoCleanMode6); see GATE REGISTER above |
| 1372 | — | 1 | 9532 | GATE g17 -- golden :936-936 (M:edContactTime); see GATE REGISTER above |
| 1377 | — | 1 | 9535 | GATE g18 -- golden :939-939 (M:edPinSingleN); see GATE REGISTER above |
| 1409 | — | 6 | 9601, 9602, 9671, 9672, 9689, 9690 | GATE g19 -- golden :969-974 (M:edACContactCleanHeight,M:edACContactShiftHeight,M:edShuttle1PickOffset,M:edtShuttle1PlaceOffset,M:edtShuttle2XOffset,M:edtShuttle2YOffset); see GATE REGISTER above |
| 1688 | — | 3 | 16111, 16112, 16113 | GATE g28 -- golden :1230-1232 (M:DutNum,M:RotateKit_PitchX,M:RotateKit_PitchY); see GATE REGISTER above |
| 2180 | — | 3 | 37490, 37491, 37492 | GATE g29 -- golden :1720-1722 (fStartCondition); see GATE REGISTER above |
| 2213 | — | 1 | 38002 | GATE g30 -- golden :1751-1751 (M:pnlLoader); see GATE REGISTER above |

---

**合計 174 筆被閘。**

⚠ 沒有理由的列（`（原始碼未寫理由）`）是真正要補的 —— 
它們是「閘了但沒說為什麼」，而那正是 §0.5 不允許的那種閘。
