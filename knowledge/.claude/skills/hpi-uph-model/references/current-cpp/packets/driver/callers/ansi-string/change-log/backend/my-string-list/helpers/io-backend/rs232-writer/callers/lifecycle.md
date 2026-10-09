# 關閉順序、Bin log 與路徑

[上層](index.md)。以 function／變數定位，不以移動中的行號做操作指令。

## 兩份 list 的關閉行為

`Rs232Ui.cpp::FormDestroy` 執行 `slRS232Log->Clear()` 後 delete 並設 NULL。
`Rs232Bridge.h` 的 TMyStringList 繼承 TStringList，但緩衝另放 `MyList`，
class 沒有宣告自訂 Clear。這個 Clear 不能直接當成 MyList 已清空；
`Rs232Support.cpp::~TMyStringList` 仍先 MySaveToFile，再 MyList->Clear／delete。
析構 catch(...) 沒有回報；關閉未必代表成功保存最後緩衝。

`Rs232Engine.cpp::Stop` 若尚未 started 就 return；未 closed 則 DoClose，再 Teardown。
DoClose 走 `FormClose`，其中關閉通訊後 `SaveBinData`，再釋放 panels／barcode lists。
Teardown 呼叫 FormDestroy（例外吞掉）後 delete fRS232Main、清 token／mailbox／started。
`Rs232Ui.cpp::~TfRS232Main` 僅在 slRS232Log 非 NULL 時 delete，
因此正常 FormDestroy 已設 NULL 的路徑不會再 delete 同一個 log。
Start catch 及 engine 異常終結也可直接 Teardown；不能將全部終止等同正常 Stop→FormClose。
本輪只核對正文，不啟停 engine 或 COM／socket。

## Bin memo 另外走 SaveToFile

`Rs232Log.cpp::AddBinData` 以 iUseRS232Mode 選 MemoBinData_TTL 或 MemoBinData；
在加入新字串前、所選 memo Count>10000 時呼叫 SaveBinData，再加入新行。
mode 2／TTL 行含毫秒，其他 standard 分支不含毫秒。
此函式與 SaveBinData 都沒有自行 GetTimeInfo；System* 是其他呼叫留下的時間，
不能稱為保存瞬間新取樣。

`SaveBinData` 的空 memo 早退只明確涵蓋 mode 0 及 >=InterfaceType_TTL；
其他 standard mode 即使空 memo 仍可走保存分支。TTL 寫 BinLog_TTL，其他寫 BinLog；
月資料夾為 YYYY_MM，檔名含日期、時分秒及 Rs232_BinData／TTL_Rs232_BinData。
先 MyForceDirectories，接 `Memo…->Lines->SaveToFile` 再 Clear；
無外層成功回執，也沒有將錯誤結果傳回 AddBinData／FormClose。
此處是 vclcompat TStringList backend，與 rs232std 裸 writer 分開；
backend 本體已有前批文件，本輪不重算其整理完成，完整異常／磁碟效果未執行。

## INI override 沒有覆蓋 log 路徑

`Rs232Engine.cpp::OverrideIniPaths` 只存 setupIni／handlerGeneralIni；
Start 在 ResetRs232Globals 後將它們套到 IniFileName／asHGeneralPath。
本輪讀到的 slRS232Log 建構與 BinLog／BinLog_TTL 字面路徑仍是 `D:RS232Log` 下，
不能把「INI 已 redirect」當成 log 也隔離。其他外部覆寫或完整部署設定仍待查。
文內路徑是分析對象，沒有建立／寫入這些 runtime 資料夾。
