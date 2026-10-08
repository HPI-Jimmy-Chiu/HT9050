# 設定發布、GPIB Aux 與 Hub／thread

來源 pin `060e5000bd9c1dd33cf05895880ddd2e0bee5914`；七個 UTF-8 來源、二十個完整 cpp 定義、三個完整 header 快照，共二十三原文片段與 hash 見 [manifest](source-manifest.json)。其中四個函式接續上輪 intake 並重新核對，其餘十六個為本輪新增選讀。

- [設定與 framing](settings.md)：PublishSettings、atomic 欄位與 pack／unpack。
- [Aux、recipe 與重啟](recipe.md)：客戶預設、INI 優先與 seed 副作用。
- [Hub 生命期](hub.md)：選擇、停止、重啟與 IsUp。
- [thread 生命期](thread.md)：非同步 Start、失敗出口、join 與例外。
- [版本、機型與未查範圍](limits.md)：共同流程與客戶差異，容量／ABI／實機分開。

回 [caller 索引](../index.md)、[初始化與有效介面](../init-selection/index.md)。本層仍屬同一 UPH Skill 的局部證據，並未完成全系統語意驗證。

## 來源定位

- [HandlerTesterSide.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerTesterSide.cpp)：blob `c2934ec58fbc8d8150c762ed99c2c33c7f164b21`。
- [HandlerGpibAux.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerGpibAux.cpp)：blob `939e8451d2d74ef500d6e3bd0d6d4f9dc57314ed`。
- [TesterCommHub.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/TesterCommHub.cpp)：blob `3bf9463c14ec6378d29b825931a832abbe34b309`。
- [TesterCommThread.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/TesterCommThread.cpp)：blob `48922d3f77dae08de6729dbdf85ae2f8ff70ff4d`。
- [TesterCommThread.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/TesterCommThread.h)：blob `e5ec5aca77bdc368966a41b2ddf16b00537a2e2d`。
- [HandlerSettings.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/HandlerSettings.h)：blob `75c736178290caf274fa2113028aee8c09d76fcd`。
- [HandlerGpibAux.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/060e5000bd9c1dd33cf05895880ddd2e0bee5914/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerGpibAux.h)：blob `1cfbaa8723e9759536de215ee3e14c7ab4a806c2`。
