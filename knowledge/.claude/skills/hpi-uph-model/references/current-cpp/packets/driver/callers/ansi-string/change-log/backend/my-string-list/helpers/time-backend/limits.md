# 時鐘下層保存與待續

[上層](index.md)；[原文清單](source-manifest.json)。來源pin、2個Git blob、byte／UTF-8摘錄與body hash在清單；7新cpp的外層preprocessor stack為空，nowSerialWithMs內部_WIN32兩支完整保存。

12個既有context與41個歷史manifest逐字／body或blob重核；完整CPP archive保存原文與歷史T6-CCSIR註記，不額外增加完成數。父time.md只append子路由，原metadata、資源、相容入口及前2單元68原文保留。本輪沒有新canonical主題，維持22已整合主題。

## 機型／版本共同項與差異

| 軸 | 適用界線 |
| --- | --- |
| HT9050／HT9045／其他Handler | 選定TDateTime正文沒有MachineType或客戶分支；只對採用此V906 shim的caller適用。不是所有部署版本都用此實作的證據。 |
| 客戶／班別 | N10 08:00／20:00及SG Jam昨天日期仍由上層filename／writer決定；時鐘factory不新增班別或UPH扣除策略。 |
| 平台 | _WIN32取SYSTEMTIME毫秒欄位；非_WIN32傳0毫秒。部署平台與實際clock／encoding要另核，不能泛化兩支一致。 |
| 版本 | 本輪V906 C++17／UTF-8；V912／V899與最新913無新比對。檔頭BCB6／Delphi對齊說明及歷史golden用法保留為原作者說明，未升格成驗證結果。 |
| Runtime | 未同步或讀寫機台設定、改時區／時間、取機台log、建立／刪檔或呼叫IO。 |

## 待續

- OS／CRT檔案API規格、實際encoding／partial write／sharing／持久性；時間API規格、時區／DST、單調耗時與錯誤／併發實測。
- MyStringList其餘完整caller、HANA form生命週期、bChangeFile／sPrevFileName／Lotfile上傳及buffer併發。
- RS232獨立writer、upload／通訊、其他機台版本；最新913／V912／客戶V899來源與UPH／S8公式及實機證據。

只有Skill references與交付文件的靜態查證；不修改來源／driver／定義檔或執行期，不建置或啟動機台。
