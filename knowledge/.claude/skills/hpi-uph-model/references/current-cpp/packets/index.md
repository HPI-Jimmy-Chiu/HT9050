# UPH 相關封包與命令宣告

pin `4ad100003f07f348fdbc8447521ce3774c3e4c88`。四份 `MessageDef.h/.cpp` 的選定宣告與巨集展開保存於 [manifest](source-manifest.json)：V906 UTF-8／V912 cp950，四個完整 VM／MV 宣告、十三個來源片段；沒有函式 body 或實機驗證。

| 問題 | 路由 |
| --- | --- |
| VM／MV 欄位、容量宣告與四個全域符號 | [欄位與符號](fields.md) |
| MSG_CMD_UPH 及 V912 命令表展開 | [命令宣告](commands.md) |
| HT9050／其他 Handler 共用項與版本差異 | [版本與機型界線](versions.md) |
| ABI、driver、caller、客戶與部署待查 | [查證界線](limits.md) |

固定來源：

- [V906 MessageDef.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/4ad100003f07f348fdbc8447521ce3774c3e4c88/HT9011UC_Cpp_V3.33.906.0/MessageDef.h)：`utf-8`，blob `8a57025d5b589fd8315dd77d14805e8f1f82e00c`。
- [V906 MessageDef.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/4ad100003f07f348fdbc8447521ce3774c3e4c88/HT9011UC_Cpp_V3.33.906.0/MessageDef.cpp)：`utf-8`，blob `d3be2fe69aaa869c7f8b282f66e31abdd227c1fa`。
- [V912 MessageDef.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/4ad100003f07f348fdbc8447521ce3774c3e4c88/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MessageDef.h)：`cp950`，blob `6a3f6cc9c991fc2928b50a41d640fa7f3d776ba6`。
- [V912 MessageDef.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/4ad100003f07f348fdbc8447521ce3774c3e4c88/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/MessageDef.cpp)：`cp950`，blob `e64780763cf3187fc37f40a82eb6fff4544c5a7a`。

回 [目前 C++ 索引](../index.md)、[Command consumer](../consumers/command/index.md)。活正文以版本／檔案、符號與欄位定位；原始註解保存作歷史。

[driver wrapper 子樹](driver/index.md)補 V906 指標設定／讀取及 ibwrt 狀態刷新五個選定 body；V912 driver、所有 caller／生命週期與送達仍待查。
