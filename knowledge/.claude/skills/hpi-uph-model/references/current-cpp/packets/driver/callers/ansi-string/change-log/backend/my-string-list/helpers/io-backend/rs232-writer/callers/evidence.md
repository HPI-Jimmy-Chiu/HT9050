# 證據、版本及未查項

[上層](index.md)；[完整來源與摘錄 hash](source-manifest.json)。

## 字面普查和呼叫圖分開

釘在 `7f1e937f2dcecba0c1937dea3731d90132f22803`，讀 V906 樹 2049 個 .cpp／.h／.hpp／.cc／.cxx／.c，
排除 .svn 及非原始碼副檔名；[普查](writer-census.json) 逐檔記 blob／SHA256。
以保留位置的遮罩移除一般 comment／字串／字元 literal，再找
`WriteDataToFile(`（84 次）與 `rs232std::WriteDataToFile(`（0 次；容許 :: 周圍空白）。
TesterComm/Rs232 子樹只有 Rs232Bridge.h 的宣告與 Rs232Globals.cpp 的定義各一次；
本輪實際讀到的通訊與 Bin 保存函式也沒有呼叫該裸 writer。
這增加字面覆蓋範圍，並非證明 linker 不可達或完整 caller 不存在。
function pointer、宏、alias、raw literal 特殊格式、外部 consumer 與建置組態未解析。
immutable pin 上的命中行號僅為普查 metadata；活文件仍以 function／變數定位。

同一子樹遮罩後沒有 upload／ftp 詞彙命中；不據此宣稱系統沒有上傳。
網路 send、Handler 外部服務、其他路徑與後續 upload 尚未連起完整呼叫圖。

## 同名、機型與版本矩陣

| 範圍 | 本輪定位／界線 |
|---|---|
| V906 tester-side RS232Standard | rs232std::TMyStringList／裸 writer；來源 banner 記歷史 Rev12.13.902.0_20260410，本輪核移植正文，未獨立讀外部 golden |
| V906 Handler 共用 | `Public/MyStringList.cpp`、common.cpp writer 及 AnsiString overload 是另一份實作；[既有 Handler 整理](../../../../index.md) 保留，不用同名直接套用 |
| V912 Handler／V899 客戶現場碼 | 不由 tester 歷史 902 名稱推定對等；V899 保留唯讀分析，修正交付依 V912 入口，本輪沒有做對應驗證 |
| 新改動 golden 913 | Steven 最新裁決仍適用新 source 比對；此單元不是將 tester 902 重新指定成 Handler golden，913 差異未比 |
| HT9050 與其他 Handler | 共用的「畫面／緩衝／磁碟／量測分開」讀法可用；哪台啟用哪種 tester／mode、客戶與 runtime 路徑需個別證據，未稱每台都有此 log |

## 已完成與下一步

已保存 21 完整 C++ function＋2 region＝23 摘錄、7 source 路徑；
351 行的表單析構拆成兩頁，接回正文不丟內容。舊 writer 及 45 份歷史 manifest 保留，
canonical Skill 入口／metadata、原文、HTML 資源與相容入口不改。
引用、錨點、原文 hash／完整函式邊界與前批保存由本單元驗證腳本檢查。

後續：各 tester 模式完整入口／checkbox、緩衝同時存取、exception reentry、
MyInsertToFile／GetLastLine caller、upload／HTTP／FTP 服務、部署 CRT／磁碟錯誤、
容量與客戶機型對照，以及 UPH S8 實機／校正。此輪沒有 C++ 建置、程式執行、
機台測試、runtime 寫入、寄信或自己合併。
