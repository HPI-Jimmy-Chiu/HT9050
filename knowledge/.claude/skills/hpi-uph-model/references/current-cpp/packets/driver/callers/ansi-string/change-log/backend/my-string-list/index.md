# 檔案型 TMyStringList：buffer、writer 與 filename

這層承接 [Change Log後端](../index.md)，查證 V906 `Public/MyStringList.cpp` 與同名 header 的完整檔案型 accumulator。它使用內部 `MyList`，繼承的基底清單與 `TesterComm/Rs232` 的同名類別要分開。

| 問題 | 文件 |
| --- | --- |
| property proxy、初始化、批次容量與時間來源 | [property／buffer](properties-buffer.md) |
| 主要追加、HANA、同資料夾／Lot鏡像及失敗清空 | [writer](writer.md) |
| enum順序、時段、08:00班別、FT／RT與固定檔名 | [filename](filename.md) |
| 指定檔名、行數／插入、SG Jam與2D mapping | [專用writer](specialized.md) |
| 版本、機型、歷史註解與待查callee | [界線](limits.md) |
| 完整原文、cpp分段保存、source byte與body hash | [保存清單](source-manifest.json) |

## 靜態範圍

來源pin由保存清單記錄。32完整cpp定義含建構／解構、9setter、buffer／writer／filename與free function；完整header含enum、proxy與原註解，另12外部註解／同名介面region，共45新摘錄、4來源。`TStringList::GetText()` 是已完成單元的context重核，不算新完成。

原cpp以definition、歷史context和separator padding完整重組；header逐字保存。活文件以function／變數定位，原文中的行號及golden敘述保留作歷史，不當成新913或BCB6對照。

## 同題機型讀法

| 軸 | 適用性 |
| --- | --- |
| HT9050與其他Handler共同項 | 選定Public類別內無按MachineType選writer的分支；只有caller實際採用這個類別時，才套本層buffer與保存契約。這不證明每台機已啟用同一log。 |
| 客戶／功能差異 | ASE KaohSiung初始週期、SIGURD時間列、Cypress offline、FT／RT、HANA旗標、N10班別與QUALCOMM mapping各自判斷，見下層。 |
| 執行期 | `D:`路徑是程式字面輸出；本輪不讀寫或複製機台設定，不把呼叫存在當成上機成功。 |
| 版本 | V906 C++17／UTF-8現行原文；V912／V899／最新913與部署機台的對應需另附實際來源。 |

僅文件整理／靜態查證，未執行C++、build、test、writer、刪檔、runtime或實機。

## 目錄與時間helper補充（20261009）

[MyForceDirectories／filesystem／全域時間](helpers/index.md)：11完整cpp／12region共23原文、11來源；上層當時helper待續範圍由此補充。既有common writer／Decode只重核context；Now內部、OS落盤／鎖檔、通訊／upload／其餘caller與版本實機仍待續。

## 專用 writer 的 caller 延伸（20261009）

[Handler 警報插入與 RS232 同名方法](specialized-callers/index.md)：新增 3 完整 CPP、6來源；行數取樣／雙 flush／sLastFileName 與 SPIL 欄位、RS232 內部緩衝差異已定位。3既有callee只重核，upload／notice ack 與部署實機另續。
