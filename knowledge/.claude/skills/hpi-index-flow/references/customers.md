# Index 客戶條件索引

下列是原文件已記錄的條件，尚未逐客戶核對 V906／V912。目前原版資料有 V897、V899、golden 等不同版本，不能用整理後的路徑推定已移植完成。

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_GIGAS | Contact Test case3050／Pick gate | bContactTestICDropGuard；CUSTOMER_CODE | 原版 contact guard 依實際 HAS_IC 分流，解除前須確認真實持料 | [V899 原案例](history/ht9045-contact-pick-interlock/index.md)，906／912 未核對 | 原文件機型，依實機確認 |
| CC_SIGURD_HUKOU | DoTestY contact 高度 | CUSTOMER_CODE | 原版記錄 contact 高度補償 | [原 Index 入口](history/ht9045-index-flow/index.md)，906／912 未核對 | 原文件 HT9045 系列 |
| CC_JCET／CC_KYEC_LEE | EP 回授判定 | iD26_3FixValueOrPercentage | 原版值為1時走百分比判定 | [原 EP 第6節](ep/pressure-feedback.md)，V906 live stub，912 未核對 | 有對應 EP 安裝的機台 |

UPH／CSV 客戶白名單保留在 [UPH 詳細資料](flow/uph-record.md)，由量測分支導覽；本表沒有把每個紀錄分流當成 HT9050 的有效功能。
