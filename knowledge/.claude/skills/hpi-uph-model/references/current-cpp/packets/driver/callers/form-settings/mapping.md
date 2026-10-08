# 表單與 INI 的資料映射

定位 GpibAux.cpp 的 SetFormToData、InitDataToMainForm、SaveSetupData；三個完整 body 與 enum 宣告見 [manifest](source-manifest.json)。這裡只讀碼，未呼叫保存函式。

## SetFormToData

直接將 cbDevice.Text 指派給 CommName，以 atoi 轉 cbBaudRate.Text；body 未檢查 baud 範圍或補 COM 前綴。

| 表單 ItemIndex | ByteSize | StopBits | Parity |
|---|---|---|---|
| 0 | _5 | _1 | None |
| 1 | _6 | _1_5 | Odd |
| 2 | _7 | _2 | Even |
| 3 | _8 | 無此分支 | Mark |
| 4 | 無此分支 | 無此分支 | Space |

超出相應表內分支時，該欄位不在這個 body 更新；不要把未處理索引說成驗證成功或自動清成預設。

最後呼叫 SaveSetupData。這裡沒有檢查 g_auxFramingFromRecipe、widget.Enabled 或 bCommConnect，也沒有呼叫 ApplyCommState_／重開 COM；前者是 [LoadSetupData](../tick-consumer/consumer.md) 的 UI 顯示控制，不能當作此函式本身的禁止寫入 guard。

## InitDataToMainForm

CommName／baud 與 timeout 送到表單文字，enum 依上表反向映射為 ItemIndex；未知 enum 沒有 else 去重設索引。它不正規化 CommName，也未改寫 TComm 的欄位。

Comm.h 的 enum 宣告順序為 ByteSize _5→_8、StopBits _1→_2、Parity None→Space；只驗證此來源的宣告，不稱各版本 ABI 已相容。

## SaveSetupData

在 `D:\RS232Standard\System\Setup.ini` 寫 COMPort 的 CommName、BaudRate、ByteSize、StopBits、Parity；`Detail Settng` 的 ReadIntervalTimeout 直接寫 edReadIntervalTimeout.Text，刪 IniFile，回 true。

關鍵差異：SetFormToData 不將 timeout 文字轉入 CommTester.ReadIntervalTimeout；SaveSetupData 直接保存表單文字。因此同一次表單保存與物件內 timeout 不是同一欄位更新。後續 reload、驗證、補寫預設及按鈕 caller 仍待查。

此 body 沒有回讀／比較保存結果；TIniFile 的 Write／destructor 實作未在本單元閉合，不能由 return true 宣稱磁碟已成功更新或具備交易／回復保證。
