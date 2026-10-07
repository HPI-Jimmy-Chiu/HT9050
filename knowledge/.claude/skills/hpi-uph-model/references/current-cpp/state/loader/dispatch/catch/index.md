# HT9050 取料 caller 與本地狀態

pin `1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5`；[manifest](source-manifest.json)保存V906 `acatchtray.cpp`一source、兩完整函式文字／三body hash，`DoCatchTray`只讀短入口與case250，完整caller尚未讀完。

- [取料狀態與返回值](flow.md)：`InitialCatchFromLoader_9050`、`DoCatchFromLoader_9050`、`iCatchFromLoader9050Task`。
- [適用與剩餘](limits.md)：一般Handler分流、callee／安全／計數界線。
- [固定source](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/1cdd21bef8a55ab06f415840ae5fdee4b1c16bd5/HT9011UC_Cpp_V3.33.906.0/acatchtray.cpp)。

manifest原始註解中的行號只保留作來源歷史；活正文用function、Task／case及變數定位。沒有執行機構或runtime。回[Loader dispatch](../index.md)。
