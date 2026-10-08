# GPIB Aux 的設定與連線 consumer

定位 `TesterComm/Gpib/GpibAux.cpp` 的 `LoadSetupData`、`OpenTesterComm`、`CloseTesterComm`；完整原文見 [manifest](source-manifest.json)。本檔與 Rs232/Rs232Comm.cpp 的同名類別方法須用來源路徑區分。

## LoadSetupData

先從 `D:\RS232Standard\System\Setup.ini` 讀 COM3、baud 9600、ByteSize `_7`、StopBits `_1`、Parity `Even`；`Detail Settng` 的 ReadIntervalTimeout 預設 70。符號 ordinal 不能自行換成另一套編碼。

本 body 定義 `W906_P6_GPIB_PORT_FROM_RECIPE 1`。`HsUnpackAuxFraming(HsGpibAuxFraming().load(), ...)` 成功才覆寫 baud／byte／stop／parity；CommName 與 timeout 保持前述 INI 值。四欄 snapshot 的 pack／unpack 限制見 [設定](../settings-hub/settings.md)。

呼叫 InitDataToMainForm 後，四個 framing 控制項的 Enabled 設為 `!g_auxFramingFromRecipe`，刪 IniFile，回 true。此 body 沒有直接寫 INI；更新表單按鈕與 SetFormToData 的寫入路徑仍待查，不能因 UI disabled 就宣稱所有 caller 都無法改設定。

## OpenTesterComm

七個不同客戶預設啟用：CC_HONPREC_QC、CC_MAXIM_THAILAND、CC_MAXIM、CC_Microchip_Thai／Phil／China、CC_SCC；原 body 的 THAILAND 重複列保存在 manifest。其他客戶預設 false，均由 `CheckAndReadIniData` 讀 general.ini 的 OpenTesterComm.RS232；helper 完整副作用未查。

- false 時隱藏 RS232 tab 並回 false。
- 已連線時先 CloseTesterComm；其回傳值沒有用來擋下後續開啟。
- 先用 `\\.\` 前綴組 CommName，CreateFileA 探測後 CloseHandle，沒有檢查探測結果；**這發生在 IsSimMode／StartComm 之前**。
- 記錄啟動前 `IsSimMode()` 為 bSimRequested，StartComm 後若已是 SIM 且先前值為 false，就 StopComm 並 throw；catch 記失敗、bCommConnect=false、回 false。
- 成功路徑設 bCommConnect=true 並記成功；這個標記不等於 tester handshake 或成功測試。

`bSimRequested` 是此處給啟動前 getter 值的名稱；明確 SIM 與底層 bSimForced 的關係見 [TComm](transport.md)。明確 SIM 也不能消除先前的探測語句，故不可把這支函式作為「零 COM 接觸」的證明。

## CloseTesterComm

StopComm 返回後才清 bCommConnect 並記成功；catch 記失敗、回 false，沒有在 catch 強制清旗標。StopComm 的有限等待與未檢查的等待結果見 [TComm](transport.md)。
