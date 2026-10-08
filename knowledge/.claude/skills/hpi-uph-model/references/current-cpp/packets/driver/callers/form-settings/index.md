# Aux 表單、INI 保存與 DCB 設定

同一 UPH Skill 的局部延伸：三個 UTF-8 來源、七個完整 cpp 定義及 Comm.h 完整快照；pin `f8b7785b514020628307e15ec59c403b6d501f46`、原文與 hash 見 [manifest](source-manifest.json)。四個函式接續上一輪 intake 並重新核對，其餘三個新增選讀。

- [建立與所有權](ownership.md)：Aux／TComm 初值、widget 別名與釋放。
- [表單映射與保存](mapping.md)：framing enum、timeout 文字及 INI 寫入界線。
- [DCB 與 timeout](comm-state.md)：ApplyCommState_、平台分流及未回報的設定結果。
- [版本、機型與待查](limits.md)：共用入口、差異及歷史 header 的適用界線。

回 [caller 索引](../index.md)、[tick／設定 consumer](../tick-consumer/index.md)。本次只讀來源，沒有建立物件、寫 INI、開 COM 或啟動機台；完整 caller、容量／ABI、V912、實機 UPH 與 S8 語意仍待續。

## 來源定位

- [GpibAux.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f8b7785b514020628307e15ec59c403b6d501f46/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibAux.cpp)：blob `54c045def600083e8b643d0f877cafbca3eccc17`。
- [Comm.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f8b7785b514020628307e15ec59c403b6d501f46/HT9011UC_Cpp_V3.33.906.0/vclcompat/Comm.cpp)：blob `2aabfcc00f8debf78993a33fcd1ec73c3d17335d`。
- [Comm.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f8b7785b514020628307e15ec59c403b6d501f46/HT9011UC_Cpp_V3.33.906.0/vclcompat/Comm.h)：blob `5c81f8875241aceb41c316022d95a0be55a0702d`。
