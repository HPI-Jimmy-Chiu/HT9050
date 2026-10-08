# 版本、機型、來源與待查

同題沿 [HT9050／其他 Handler UPH 機型樹](../../../../../machines/index.md)。HT9050 Hot／Ambient、site／每批與 Soak 口徑仍在原機型頁；所選 converter body 沒有 MachineTypeChoice／CustomerCode，只能確認這批 V906 共用符號，不能證明全部機型／客戶 caller 的輸入、recipe、path 與現場值相同。

Aux COM／timeout 客戶分流沿 [consumer](../tick-consumer/consumer.md) 與 [表單映射](../form-settings/mapping.md)。磁碟／memory 的 found 與保存狀態沿 [INI binding](../ini-core/binding.md)，跟本層數值／文字轉換分開；這些函式沒有完成 SOFT_SIMULTE／真機、runtime 或 HT9050 工作檔驗證。

V906 UTF-8／C++17 的四來源 pin、byte/blob hash 與十六片段完整保存。V912 Big5／BCB6、header 的 OLE/golden 註解及 assignInt 的 portable 註解是另一路相容性證據；本輪靜態核對不當作其實測。活文件按 function／變數定位，原文註解中的行號只作保存內容。

這次是將已讀 intake 文件化：12 完整 cpp、2 inline header、1 完整 header、1 常數。沒有重讀已完成主題或把全 TDateTime.h 內每個 inline 算成新 cpp body；AnsiString.h 僅選兩個定義，未宣稱全 header 已查完。

仍待 AnsiString 的其他 constructor／c_str／IsEmpty、TStrings 容器、完整 caller／reload／shutdown、輸入容量與 ABI、標準函式庫／locale、BCB6 負日期及格式契約、V912 機型／客戶矩陣、作用中 HT9050 與實機 UPH；S8 全部發布語意仍未結案。保留既有 Skill、metadata、相容入口、資源與 W-44 裁決正文；沒有改 source、runtime 或機台快照。
