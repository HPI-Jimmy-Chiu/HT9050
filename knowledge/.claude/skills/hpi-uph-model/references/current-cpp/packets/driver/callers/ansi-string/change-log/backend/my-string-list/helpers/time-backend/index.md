# MyStringList時間下層：取時與TDateTime入口

承接 [全域／物件時間](../time.md)。這層補齊先前尚未保存的TDateTime取時與薄轉呼叫，既有日期序列、Decode及formatter完整正文只重核context，不重算新完成。

| 問題 | 文件 |
| --- | --- |
| Windows／POSIX取樣、Now／Date／Time | [時鐘來源](clock.md) |
| 日期序列、EncodeDate與Yesterday界線 | [序列與日期](serial.md) |
| TimeToStr／FormatString轉呼叫 | [格式入口](format-entry.md) |
| 保存清單、機型／版本、待續 | [範圍](limits.md) |
| 7個新cpp與2region；12個既有context | [原文清單](source-manifest.json) |

## 完成口徑

新完成nowSerialWithMs、Now、Date、Time、EncodeDate、TimeToStr、TDateTime::FormatString共7個cpp正文，加2個CPP原始banner／include region，共9新摘錄、2來源。

既有10個cpp正文、完整TDateTime.h、epoch constant共12個context逐字／body核對；不算新完成或新full header。TDateTime.cpp完整原文另存於清單source_archives，逐byte核對並保留所有註解，亦不額外算摘錄或函式。合計17個cpp正文在新單元與既有日期單元皆有完整證據；這僅是此檔選定來源的覆蓋，沒有擴成全域caller、UPH公式或完整VCL相容性完成。

各頁以function／變數定位；本輪僅靜態文件，未呼叫機台、writer、clock、runtime、test、build、BCB6或913 oracle。
