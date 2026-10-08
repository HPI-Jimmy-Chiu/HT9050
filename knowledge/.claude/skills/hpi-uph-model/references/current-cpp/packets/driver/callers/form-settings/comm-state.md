# TComm::ApplyCommState_ 的套用界線

定位 `vclcompat/Comm.cpp` 的 ApplyCommState_，完整 body 與 Comm.h 快照見 [manifest](source-manifest.json)。StartComm 的呼叫點已在 [transport](../tick-consumer/transport.md) 查證；本單元沒有套用任何 COM 設定。

body 只在 `_WIN32` 編譯：hFile 無效早退，GetCommState 失敗也早退。讀到 DCB 後才覆寫下列欄位：

| 輸入 | DCB 映射 |
|---|---|
| BaudRate | 直接指定 dcb.BaudRate |
| ByteSize | _5／_6／_7 為 5／6／7 bits；_8 與 default 為 8 |
| Parity | Odd／Even／Mark／Space 各自映射；None 與 default 為 NOPARITY |
| StopBits | _1_5／_2 各自映射；_1 與 default 為 ONESTOPBIT |
| ParityCheck | dcb.fParity 的 true／false |
| XonXoffFlow | fOutX／fInX；fBinary=true |

另固定關閉 CTS／DSR flow 與 DSR sensitivity，DTR／RTS 設 enable，fTXContinueOnXoff=true。Parity enum 與 ParityCheck 是不同欄位：Aux 的 Even 初值不等於此處 fParity 已啟用。

呼叫 SetCommState 後不檢查回傳值。COMMTIMEOUTS 先清 0，再設：

- ReadIntervalTimeout 非零用原值，零用 MAXDWORD。
- ReadTotalTimeoutMultiplier=0；ReadTotalTimeoutConstant 非零 interval 時為 50，否則為 0。
- WriteTotalTimeoutMultiplier=0；WriteTotalTimeoutConstant=2000。

SetCommTimeouts 的回傳值也未檢查；void body 沒有將設定成功／失敗回報給 StartComm。旗標、timeout 參數與原註解的 reader 行為不能取代 driver／實機證據，也不能直接換成 UPH 的固定開銷。

Comm.h 宣告這些設定為普通公開欄位；這七個選讀 body 中，SetFormToData 沒有呼叫本 helper。整個系統是否另有重開／套用 caller，仍待續查。
