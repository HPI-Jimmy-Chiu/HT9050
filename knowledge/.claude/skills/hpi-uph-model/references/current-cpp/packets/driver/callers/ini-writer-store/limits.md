# 機型、版本與下一步

本單元仍屬同一 [HT9050／其他 Handler UPH 機型樹](../../../../../machines/index.md)。所選 25 個 cpp body 沒有 MachineTypeChoice／CustomerCode；只說明這批 V906 共用 INI 符號，不能據此認定所有機型的 path、recipe、客戶設定或現場值相同。

- HT9050 的 Hot／Ambient、site／batch 單位仍沿機型樹；Aux COM／timeout 客戶分流仍沿 [consumer](../tick-consumer/consumer.md) 與 [表單映射](../form-settings/mapping.md)，本層不把它們改成共用常數。
- Fast／磁碟 profile 與 memory store 的 trim、header、'#'、空 key、value／引號、列舉及保存格式分開；Windows TouchSame 與非 Windows 路徑也分開。這些平台分支不是 SIM／真機分支。
- V906 UTF-8 原文與完整函式 hash 已保存；V912 Big5／BCB6 系統 profile、歷史 header 的 contract 與來源註解測試紀錄不是本輪結果。原文註解內行號只保留為歷史定位；活文件依函式與變數定位。
- 本輪沒有讀現場 INI／工單或機台設定，沒有更改 SOFT_SIMULTE、runtime、W-44 裁決或原 Skill 原文／metadata／資源／相容入口。文件驗證不用 machine snapshot apply／restore。

下一步：typed API、日期／浮點與磁碟列舉 wrappers、完整 caller／reload／shutdown、名稱／value 輸入限制、容量 ABI、V912 機型／客戶矩陣及實機 UPH。沒有宣告 S8 全部發布語意或公用 INI 全檔查證完成。
