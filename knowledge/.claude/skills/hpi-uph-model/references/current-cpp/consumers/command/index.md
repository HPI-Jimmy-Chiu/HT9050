# V906／V912 Command UPH consumer

pin `1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5`；[manifest](source-manifest.json)保存兩Command.cpp（V906 UTF-8／V912 cp950）、四完整函式文字／六body hash及四call區段。GetAll完整body只保存hash、尚未查完。

- [表格、單筆／GetAll與版本](flow.md)：`TfMain::UPHStrings`、`TfMain::GetUPH`、`TfMain::GetAll`。
- [客戶／機型與剩餘](limits.md)：值口徑、caller／transport／counter界線。
- 固定source：[V906 Command](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5/HT9011UC_Cpp_V3.33.906.0/Command.cpp)、[V912 Command](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/Command.cpp)。

原始註解／舊行號只保存作來源歷史，活正文以function／欄位／MSG定位。回[consumer樹](../index.md)，沒有傳送指令或執行runtime。

[轉送與bridge子樹](transport/index.md)補V906註冊／清除、兩版本地包裝及出口差異；接收端／送達與完整dispatcher仍待補。
