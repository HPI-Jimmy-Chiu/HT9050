# typed writer 與 Change Log hook 接線

[common_ChangeLog.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common_ChangeLog.cpp)、[cpublic.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/cpublic.cpp)、[common.cpp 固定正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.cpp)；完整所選原文、介面／歷史與 byte／body hash 見 [manifest](source-manifest.json)。

[common.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/common.h) 的五個 W906_ChangeLogFn typedef 保留 bool／int／unsigned long／AnsiString／double 實參，Str1／Str2／bHasChange 用 reference。double 額外接 caller 的 AnsiString Str。common.cpp 的五個 hook global 常數初值都是 0；未裝上時 writer 跳過 hook。W906_InstallChangeLogHooks 直接把五個指標指向對應 W906_ChangeLog_*，沒有 guard、mutex 或生命週期管理。

[wb_serve 啟動所選正文](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/25163d8e4e2df6f64f531bb1c8894fcd1d7b5fd0/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp) 的 startup: LoadMachineConfig() 同列依序 InitialMemory、InitCommonString、W906_InstallChangeLogHooks，後續才呼叫 LoadMachineConfig。這是所選 wb_serve source 接線證據，並非所有 executable、build variant 或現場初始化均已執行。原 comment／header 的舊行號與 golden 說法保存在 manifest，不用舊行號定位活文件。

## 五個 writer 的既有 context

WriteIniData 在 OpenIniFile 失敗時記 RecordProcess 並 return；成功則讀舊值，以 false 初始化 bHasChange，非 null hook 才呼叫。hook 在 INI write 的 try／catch 之前；hook throw 不能由下方 write catch 攔住。hook 先記 change 再寫 INI，bHasChange 不是儲存成功回執；RecordChangeLogByLot 的舊 tail 仍在 #if 0。

double writer 先 Str.sprintf("%0.4f", Value)，ReadFloat 舊值，再把 ret／Value／Str 都傳給 hook；WriteString 使用這份四位小數 Str。Read* 的 default 使用傳入新值，讀取結果／缺鍵與實際 backend 仍沿既有 INI 文檔，不在本輪執行。

五個 writer 已完成來源只核正文 hash。字串 hook 沿 [既有 W906_ChangeLog_Str](../sysutils-numeric/callers.md)，本輪不重算；四 numeric hook 與名稱／caption helper 則保存完整新原文。

RecordChangeLogProcess 接收 Str1.c_str()／Str2.c_str() 後才把 bHasChange 設 true；未記錄的分支不把外部 flag 清掉。資料庫落點／同步、lot logging、thread 與傳入物件有效性未閉合。banner 的當年 stand-in／未寫入描述只當歷史，不冒稱今日 backend 現況。

回 [numeric gate](values.md)、[未驗項](limits.md)。
