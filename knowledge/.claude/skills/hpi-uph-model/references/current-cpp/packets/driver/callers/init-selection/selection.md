# 有效TestType與bridge啟動

定位HandlerTesterSide.cpp的OffLineGpibWay、EffectiveBridgeTestType、StartBridgeProgram；完整body見 [manifest](source-manifest.json)。

| function／條件 | 實際選讀body |
| --- | --- |
| OffLineGpibWay | LastSet.iTester!=ON_LINE 且（TestIF.iTestType==RS232_MODE 或 TestIF_File.iTestType==TTL_MODE） |
| EffectiveBridgeTestType | 上述成立就GPIB_MODE，否則TestIF.iTestType |
| StartBridgeProgram／hub | hub==0直接false；其餘先算有效t，呼叫PublishSettings |
| CurrentTestType()!=t或!IsUp() | 先W906_GpibAuxBeforeBridgeStart(t) |
| CurrentTestType()!=t | 返回hub->SelectTestType(t) |
| type相同但!IsUp() | 返回hub->Restart()；其餘true |

判斷是「非ON_LINE」，不只精確OFF_LINE；RS232看TestIF，TTL看TestIF_File，不可自行改寫成同一份recipe欄位或假設兩份永遠同步。本body沒有CUSTOMER_CODE或MachineTypeChoice分支；機型／客戶差異先查實際設定與上層caller。

[factory](../factory.md) 中GPIB／TCPIP註冊GpibEngine、RS232／TTL註冊Rs232Engine，與這裡有效t的選擇接續判讀。PublishSettings／GpibAux、Hub的SelectTestType／Restart、Rs232Engine全路徑尚未在本單元查完；不能因有效t==GPIB便保證不開COM、無NI呼叫或實際使用SimGpibDriver。

OffLineGpibWay原文註解說明20260926的歷史Off-Line裁決與模擬流程，本層保存它；driver指標仍依 [Start選擇](../../lifecycle/selection.md) 的注入／NI條件，建置宏與LastSet狀態不是同一件事。返回true也不代表外部Tester就緒或整段機台流程已實測。

回 [本層索引](index.md)、[界線](limits.md)。
