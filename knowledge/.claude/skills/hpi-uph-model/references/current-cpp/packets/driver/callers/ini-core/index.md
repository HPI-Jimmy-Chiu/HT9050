# Aux 設定的 INI 讀寫共用層

V906 來源 pin `b6792353e02f55e6da8ef4647db486a960697d8d`；IniFiles.cpp／IniFiles.h 的 22 個完整 cpp 定義、完整 header、W906RdSpan 與 kW906BcbReadStringBuf 共 25 份原文片段與 hash 見 [manifest](source-manifest.json)。其中六個 cpp 與 header 接續前輪 intake，其餘十六個 cpp、本次 struct／常數已完整選讀；不是整個 cpp 檔或全部 caller 的驗證。

- [綁定、memory 與保存時機](binding.md)：writeThrough_、FileName、constructor／destructor、UpdateFile。
- [磁碟讀取與解析](read.md)：found、預設字串、2048 buffer、section／key、parseIntDef。
- [寫入與失敗界線](write.md)：probe、had、out、fopen／fwrite、未閉合的 Fast callee。
- [機型、版本與未查範圍](limits.md)：HT9050／其他 Handler 共用層，客戶／平台與現場資料分開。

回 [Aux 表單映射](../form-settings/mapping.md)、[Aux 設定 consumer](../tick-consumer/consumer.md)、[caller 索引](../index.md)。本層僅靜態查證，未呼叫 INI 保存或機台程式。
