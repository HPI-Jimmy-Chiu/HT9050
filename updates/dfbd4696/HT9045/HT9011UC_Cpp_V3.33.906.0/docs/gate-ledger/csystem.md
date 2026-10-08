# 閘冊：`csystem.cpp`

> AI(W906-ST-LEDGER) 20260924，由 `tools/gate_ledger_emit.py` 從就地註解抽出，
> 主迴圈逐條覆核。**理由一律照碼上原文搬運，沒有的不補。**
>
> 相異 tag **60** 個（有具名相依 31／沒有 29），共 107 處。

## §1 有具名相依的（31）

| tag | 嚴重度 | 行 | `#if 0`? | 缺的相依（碼上原文） |
|---|---|---|---|---|
| `g3-G01` | — | 9980,9989 | ✅ | golden csystem.cpp —— 5690  --  fMain->MemoIndexPosLog->Lines->Add -- the Index-4-axis-home completion log line. |
| `g3-G02` | — | 10124,10134 | ✅ | golden csystem.cpp —— 5867  --  fSCKART->ClearAlarmCode() at RT-Start entry. |
| `g3-G03` | — | 10398,10409 | ✅ | golden csystem.cpp —— 6131  --  fAGV->bATK_AMR_DoHostLotStart / bATKAMR_GET_LOTORDER0_Ready reset (ATK-AMR). |
| `g3-G04` | — | 10478,10486 | ✅ | golden csystem.cpp —— 6199  --  fLotInfo->labLotTrayCount_KYEC->Caption="0" (KYEC AMR tray-count label). |
| `g3-G05` | — | 10528,10566 | ✅ | golden csystem.cpp —— 6241  --  the SCKART / SPIL manual lot-count entry block (GPIB message fill + fNote lot-count/lot-ID edit boxes + fSCKART->AccessFile). |
| `g3-G06` | — | 10584,10591 | ✅ | golden csystem.cpp —— 6281  --  fSCKART->ClearAlarmCode() at Initial-Start entry. |
| `g3-G07` | — | 10592,10603 | ✅ | golden csystem.cpp —— 6282  --  LotSummary.ClearAllData() (SCKART branch). |
| `g3-G08` | — | 10622,10628 | ✅ | golden csystem.cpp —— 6301  --  LotSummary.ClearAllData() (SPIL / ASE-CL branch). |
| `g3-G09` | — | 10669,10678 | ✅ | golden csystem.cpp —— 6342  --  fTrayMapping->ClearTrayIDByLot(). |
| `g3-G10` | — | 10686,10695 | ✅ | golden csystem.cpp —— 6350  --  fOmron->ClearOmronLog(). |
| `g3-G11` | — | 10750,10762 | ✅ | golden csystem.cpp —— 6405  --  ZeroMemory(fYieldMonitoring->iAlarmSiteYieldCmpCnt, ...) -- the LowYieldAutoSiteOff 'alarm N times before closing the site' counter. |
| `g3-G12` | — | 10849,10872 | ✅ | golden csystem.cpp —— 6492  --  the eight yield / bin-select interval-counter resets (fYieldMonitoring x7 + fShowBinSelect x1). |
| `g3-G13` | — | 11112 | — | RETIRED —— TMyBinDispCtrl's |
| `g3-G15` | — | 11485,11495 | ✅ | golden csystem.cpp —— 7108  --  fBarCode->Write_Device_Info_By_Tray (CC_ASE_KaohSiung branch). |
| `g3-G16` | — | 11503,11512 | ✅ | golden csystem.cpp —— 7116  --  fBarCode->Write_Device_Info_By_Tray (bBarcodeTrayRecFile branch). |
| `g3-G17` | — | 11814 | — | RETIRED —— paired |
| `g3-G18` | — | 11875,11883 | ✅ | golden csystem.cpp —— 7470  --  fMain->ImpParaCheck->Clear() -- GM work-file parameter-compare list reset. |
| `g3-G19` | — | 12596,12607 | ✅ | golden csystem.cpp —— 8181  --  fYieldMonitoring->ClearAutoSiteOffStatus(). |
| `g3-G20` | — | 12618,12631 | ✅ | golden csystem.cpp —— 8192  --  the fMain->RENESAS_Server->bReturn41Flag work-complete unlock of the work-file list. |
| `g3-G21` | — | 12673,12691 | ✅ | golden csystem.cpp —— 8235  --  the AMR.CheckTrayFeed() latch of LastSet.bAMRTrayFeedWait + fLotInfo->RefreshAMR(). |
| `g3-G22` | — | 12708,12744 | ✅ | golden csystem.cpp —— 8256  --  the whole Auto-loop 'stack has fewer than 16 bins -> make the operator remove the tray' block (MES1124/1224/1324 + the AUTO-has-tray follow-up message). |
| `g3-G23` | — | 12757,12788 | ✅ | golden csystem.cpp —— 8288  --  the matching Fix-loop 'stack has fewer than 16 bins' block (MES1724..MES2224 + the Fix-tray-detect sensor confirmation). |
| `W906-HOME-W1-CONTACTFN` | — | 31460,31468 | ❌ lifted | W-152 20261007 (RULINGS_20261007 #9): HT9050 PCI1203 Index Z1 only -- live `if(W906_IndexZLive1203()) fContactForm->DoTestContactFunction();` (csystem.cpp:31468); SIM / other machines: no call (AI(W906-W156) 20261007 (St02-E): hand edit, line count kept) |
| `W906-HOME-W1-CONTACTMODE` | — | 31480,31487 | ❌ lifted | W-152 MODE 20261007 (c26ddd62): lifted with CONTACTFN -- live `if(W906_IndexZLive1203() && W906_ContactModeNormalHook) W906_ContactModeNormalHook();` (csystem.cpp:31487) (AI(W906-W156) 20261007 (St02-E)) |
| `W906-HOME-W1-RTCBLOCK` | — | 30924,30939,30969 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-AUTOTEACHPOS` | — | 30982,30988,30990 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-AUTOALIGN` | — | 31017,31024,31028 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-BARCODEARM` | — | 31032,31040,31061 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-SHUTTLEMOVE` | — | 31071,31076,31078 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-OFFSETARM` | — | 31109,31120,31163 | ✅ | 缺相依，見上面的就地註解 |
| `W906-HOME-W1-SPEEDCLOSE` | — | 31174,31183,31186 | ✅ | 缺相依，見上面的就地註解 |

## §2 ⚠ 抽取器抽不出理由的（29）—— 要人看一眼

⚠⚠ **這一節不等於「碼上沒寫理由」。** 20260924 波 2 實測：三筆落在這裡的，
三筆**碼上都寫了**，只是寫法本支認不得（寫在 `#if 0` 那一行的尾巴、
或寫成散文而沒用 `缺相依：`）。把它們當成 §0.5 違規回報會**誣賴人**。

所以這一節的正確讀法是：**本支抽不出來，請人去讀那幾行**。
讀完若確認碼上真的沒寫，那時它才是 §0.5 的待辦 ——
依 §0.5，加閘的唯一合法理由是「相依不存在」，一個說不出缺什麼的閘
**不是**可以補一句話蓋過去的空格。

| tag | 行 | `#if 0`? |
|---|---|---|
| `W906-T6-FTPMODAL` | 8963 | — |
| `W906-T6-A37LOTINFO` | 9017 | — |
| `W906-T6-GROUNDMAN` | 9077,9168 | — |
| `W906-T6-FIXAOI` | 9162 | — |
| `W906-T6-CONTALARM` | 9176 | — |
| `W906-T6-ARTINPUT` | 9345 | — |
| `W906-T6-BCRECFILE` | 9386 | — |
| `W906-T6-EQCQTY` | 9464 | — |
| `W906-T6-ALED` | 9480,30255,32122,32261 | — |
| `W906-T6-UTACPPSEL` | 9546 | — |
| `W906-T6-2DIDMAP` | 9585 | — |
| `W906-T6-COM2TORQUE` | 29988 | ✅ |
| `W906-T6-E84SHOW` | 30024 | — |
| `W906-T6-VERIFYMOT` | 30125 | — |
| `W906-T6-SOCKETKR` | 30190,31357,32040,32384 | — |
| `W906-T6-AUTOTEACH` | 30302 | — |
| `W906-T6-SCCRTM` | 30326 | — |
| `W906-T6-HISINAME` | 30429,30483 | — |
| `W906-T6-OEE` | 30552 | — |
| `W906-T6-ATC30RESEND` | 30568 | — |
| `W906-T6-ROTZERO` | 31369 | — |
| `W906-T6-RTCSTART` | 31942 | — |
| `W906-T6-OCRSTART` | 32031 | — |
| `W906-T6-HSPREALARM` | 32076 | — |
| `W906-T6-TEMPTAJOFS` | 32134 | — |
| `W906-T6-HPTORQUE` | 32159,32180 | — |
| `W906-T6-OCREND` | 32251 | — |
| `W906-T6-ABORTHOME` | 32320 | — |
| `W906-T6-PANATIME` | 32335 | — |
