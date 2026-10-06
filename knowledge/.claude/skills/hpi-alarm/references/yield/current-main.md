# 目前 main 的 Yield 路徑

唯讀基準 main `7deec0f7604e893b58b6cce975c049d284ab8474`，未執行機台、告警或 C++ 測試；以下只代表靜態來源碼核對。

## 呼叫者已存在

HT9011UC_Cpp_V3.33.906.0/MainTimersSt02.cpp::St02OnTimer3 會呼叫 MainTimer3.cpp::W906_Timer3Timer；handler 有 InitialOK 的提前返回。W906_Timer3Timer 實際呼叫 CheckBySiteYieldAlarm、CheckBySiteByArmYieldAlarm、CheckLowYieldAlarm、CheckLowYieldAlarmByTotal、CheckIntervalLowYieldAlarmBySite、CheckIntervalLowYieldAlarmByTotal、CheckLowYieldAlarmSpecial、CheckByPickerYieldAlarm 共八個方法。

atester_ProcessCount.cpp 的 PCW7_YIELDMON_CALCSITEYIELD 呼叫 fYieldMonitoring->CalculateSiteYield；PC_YIELDMON_CLEARCOUNT 呼叫同一 instance 的 ClearYieldCount。該檔舊註解說 Timer3 driver 沒移植／ClearYieldCount 尚無成員，已不能當此 main 的結論；函式存在及有呼叫也不代表現場所有 gate／條件已通過。

## 告警處置與客戶

atester_ProcessCount.cpp::DoLowYieldAlarm 保留 CosFunction.bSmartAutoClean、PC_CHECKSMARTAUTOCLEAN、bYieldAlmNeedOneCycle、bContinueFailNeedAlarmDirectly、bLowYieldDoOneCycle 的處置分流；但目前 PC_CHECKSMARTAUTOCLEAN 明確展開成 false，這兩個呼叫不能當 Smart Auto Clean 已接通的證據。CC_JCET 再依 IniConfig.bI05LowYieldForcedOneCycle 決定可否 Retry；CC_Greatek／CC_AMKOR_China／CC_QUALCOMM／bKoreaFunction 有 Retry 路徑；special low-yield 可用 kcode==0，仍須按告警宿主的 stop 規則解讀。

bOneCycle 分支設定 bNeedOneCycleByYieldAlm 並呼叫 BtnOneCycleClick，但 slLowYieldAlarm 的 TStringList 記錄仍在 #if 0；不可整理為記錄功能全量完成。ret 預設 K_ONECYCLE，函式最後亦依 ret 呼叫 BtnOneCycleClick，不能假定每次都 ShowErrorMessage 或只觸發一次。

## E-034：RT 自動關 site

uYieldMonitoring.cpp::DoRTAutoSocketOff 在 SCKART 四條件成立時直接返回；其餘依 CosFunction.bAutoCloseSiteWhenRT、TestIF_File.iAutoCloseSiteWhenRT、IniConfig.bA09_ByArmCloseSite 走 Auto Head／Auto Socket。低良率標記目前確實使用 bLowYieldCloseSite= true，之後呼叫 DoAutoCloseSite(2)，是 E-034 採 0625／V912 的既有實作，區分於 0618 比較後丟棄的舊行為。

Auto Socket arm-1-only 分支目前寫 dYield[1]，後續卻比較 dYield[0]；來源碼明標 golden asymmetry。本次保留並說明，不在 Skill 整理中改程式或宣稱已修正。

## 機型分流

Yield 的共同演算法按站點、TestSocket、Shuttle／NN／FT／RT及客戶旗標分流，不能直接把 HT9050 的軸／門數套到其他機台。同一 Skill 的 [機型差異](../machines/index.md)另列 PCI1203 扭力逾時告警；那是不同 trigger，不是 Yield 門檻或計數的替代。
