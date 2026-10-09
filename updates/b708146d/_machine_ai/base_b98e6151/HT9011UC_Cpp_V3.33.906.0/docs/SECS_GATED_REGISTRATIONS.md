# SECS SV/EC 被 `#if 0` 閘住的登錄 —— 逐筆「缺什麼」

> AI(W906-P4-SECS) 20260920 由 `tools/secs_gate_census.py` 產生。
> **不要手改**；改了會與原始碼漂開。重跑那支工具即可。

P4 完成條件的最後一項：「174 筆被閘的各有『缺什麼』的紀錄」。

★ 理由**不是我判斷的**，是翻譯當下寫在每個 `#if 0` 上面的 GATE 註記；
  這份文件只是把它們蒐集起來對帳。自己重寫理由等於把翻譯者的
  現場判斷換成事後猜測。

## SV —— `SECSGEM/uHGemHT9045_SV.cpp`

閘住的區塊 **13** 個，涵蓋登錄 **78** 筆。

| 行 | GATE | 筆數 | ID | 缺什麼（原始碼裡的理由） |
|---|---|---|---|---|
| 487 | G4 | 1 | 1041 | GATE [G4] -- golden :100-100.  SVID 1041 -- fMain->lbEPenconder (golden main.h:796, TPanel*): NOT a member of forms/fMain.h's TfMain. |
| 494 | G5 | 2 | 1047, 1048 | GATE [G5] -- golden :105-106.  SVID 1047/1048 -- fContact->lblEPValueKg: NOT a member of atester_shims.h's TfContactShim. |
| 625 | G21 | 1 | 1191 | GATE [G21] -- golden :232-232.  SVID 1191 -- &iSV_ErrBinCnt: DECLARED (cmydef.h:5911), definition (cmydef.cpp:5999) inside the same `#if 0 // TODO(W6)` block -> undefined at LINK. |
| 704 | G6 | 8 | 1351, 1352, 1355, 1356, 1359, 1360 … | GATE [G6] -- golden :309-316.  SVID 1351-1364 (iATC_Use_Heat_Count<=4 branch) -- fLotInfo->pl_ATCTempHead01..04 / pl_ATCRefHead01..04: NOT members of forms/fLotInfo.h's TfLotInfo. |
| 717 | G7 | 16 | 1351, 1352, 1353, 1354, 1355, 1356 … | GATE [G7] -- golden :320-335.  SVID 1351-1366 (iATC_Use_Heat_Count<=8 branch) -- fLotInfo->pl_ATCTempHead01..08 / pl_ATCRefHead01..08: ditto. |
| 773 | G8 | 4 | 2003, 2004, 2005, 2007 | GATE [G8] -- golden :374-377.  SVID 2003/2004/2005/2007 -- fContact->edForcePerDeviceKG / edForcePerPinN / edAirForceN / edForcePerDeviceN: not on TfContactShim. |
| 784 | G9 | 3 | 2018, 2019, 2020 | GATE [G9] -- golden :383-385.  SVID 2018/2019/2020 -- fContact->edAirKPA / lblReadEP / edSetKg: not on TfContactShim. |
| 792 | G10 | 1 | 2024 | GATE [G10] -- golden :389-389.  SVID 2024 -- fContact->lblDieForceEP: not on TfContactShim.  ==> Eastsun 20260526 #026-1.79 Ifor 20220218 add:KYEC 要求新增Dual Force 開關      HGemPtr->SetSVDataPointer(2022 , HTyp… |
| 831 | G11 | 9 | 2240, 2241, 2242, 2243, 2244, 2245 … | GATE [G11] -- golden :426-434.  SVID 2240-2248 -- fShowBinSelect->lblMag1..9: the fShowBinSelect global/form does not exist in the port at all. |
| 844 | G12 | 1 | 2631 | GATE [G12] -- golden :437-437.  SVID 2631 -- fContact->rgHandlerMode: not on TfContactShim.  <== Eastsun 20260526 #026-4.A8 |
| 870 | G14 | 2 | 9003, 9004 | GATE [G14] -- golden :459-460.  SVID 9003/9004 -- fCleaning->edAutoCleanAirForce_Kg / _N: not members of forms/fCleaning.h's TfCleaning (edCleaningCount, SVID 9001, IS -- and stays ACTIVE). |
| 1118 | G15 | 2 | 37200, 37201 | GATE [G15] -- golden :705-706.  SVID 37200/37201 -- fLotInfo->lbESDReportData / lbESDDecayReportData: not on TfLotInfo. |
| 1270 | G17 | 28 | 43300, 43301, 43302, 43303, 43304, 43305 … | GATE [G17] -- golden :853-880.  SVID 43300-43327 -- fGroundMan->labValue_0_2..labValue_3_5: the fGroundMan global/form does not exist in the port at all.   //AI(W906-POOL2-SV) 20261008 (St02-E, claim): reaso… |

## EC —— `SECSGEM/uHGemHT9045_EC.cpp`

閘住的區塊 **8** 個，涵蓋登錄 **32** 筆。

| 行 | GATE | 筆數 | ID | 缺什麼（原始碼裡的理由） |
|---|---|---|---|---|
| 485 | — | 1 | 1581 | GATE g2 -- golden :80-80 (M:sInfo_Customer); see GATE REGISTER above |
| 489 | — | 13 | 1583, 1584, 1585, 1586, 1587, 1588 … | GATE g3 -- golden :82-94 (M:sInfo_CurrQty,M:sInfo_CustDevGup,M:sInfo_CustLotID,M:sInfo_DeviceName,M:sInfo_HandlerID,M:sInfo_OperatorID,M:sInfo_ProgramName,M:sInfo_ReportCnt,M:sInfo_Stage,M:sInfo_Step,M:sInfo… |
| 605 | — | 1 | 2632 | GATE g5 -- golden :194-194 (M:chkShuttle); see GATE REGISTER above |
| 764 | — | 1 | 3540 | GATE g6 -- golden :347-347 (M:tSiteMap); see GATE REGISTER above |
| 1362 | — | 6 | 9511, 9512, 9513, 9514, 9515, 9516 | GATE g16 -- golden :924-929 (M:chkAutoCleanMode1,M:chkAutoCleanMode2,M:chkAutoCleanMode3,M:chkAutoCleanMode4,M:chkAutoCleanMode5,M:chkAutoCleanMode6); see GATE REGISTER above |
| 1376 | — | 1 | 9532 | GATE g17 -- golden :936-936 (M:edContactTime); see GATE REGISTER above |
| 1413 | — | 6 | 9601, 9602, 9671, 9672, 9689, 9690 | GATE g19 -- golden :969-974 (M:edACContactCleanHeight,M:edACContactShiftHeight,M:edShuttle1PickOffset,M:edtShuttle1PlaceOffset,M:edtShuttle2XOffset,M:edtShuttle2YOffset); see GATE REGISTER above |
| 2184 | — | 3 | 37490, 37491, 37492 | GATE g29 -- golden :1720-1722 (fStartCondition); see GATE REGISTER above |

---

**合計 110 筆被閘。**

⚠ 沒有理由的列（`（原始碼未寫理由）`）是真正要補的 —— 
它們是「閘了但沒說為什麼」，而那正是 §0.5 不允許的那種閘。
