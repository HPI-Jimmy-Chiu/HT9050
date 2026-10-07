# 客戶碼／功能閘的局部查證

來源main2db V906 ElaSchedule.cpp的JobEnabled／CustomerCodeAllows函式；第三欄列實際runtime條件，此次未核對相應INSTALL_／USE_編譯閘。只查兩函式正文，不代表全caller／完整生效條件。

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_CC_或runtime） | 行為差異一句話 | 來源（906／912、912-only） | 相關機台 |
|---|---|---|---|---|---|
| 851 | JobEnabled，JOB_N25_3／4／5；CustomerCodeAllows，EL_UPLOAD_JAMWEEK／SUMMARY／EVENTLOG | c.n25_3／4／5、n25Host；自動需EffectiveO10且SIM自動不跑 | N25工作限制851，自己的功能閘與主機再核對；手動不能直接推成不傳輸 | 906正文局部核對；912未核對 | 本次未查Model分派，HT9050／其他Handler依實際配置 |
| 915／919 | JobEnabled，JOB_O19_VTEST；CustomerCodeAllows，EL_VTEST_MTBF_SUM | c.o19Week／o19WeekDay；自動EffectiveO10 | VTEST週報有客戶碼與週期閘，不套用非VTEST的Handler O19 | 906正文局部核對；912未核對 | 同上，未證機台啟用 |
| 868 | CustomerCodeAllows，EL_UPLOAD_CHIPADV_LOTEND | 本函式只有cust碼；N34功能閘／lot-end caller另待查 | 此函式限制868，不等於N34整條流程已接通 | 906正文局部核對；912未核對 | 同上，未證機台啟用 |

SPIL CSV、VTEST檔名、933／967 Jam預設、956 Production_Log欄位等既有差異按[保存原文](ela/original-entry.md)的版本／日期保持，本次不新增已核對列。沒有本表列到的客戶不能推論沒有功能或差異。[客戶總索引](../../hpi-customer-features/SKILL.md)另保留詞法候選／人工局部證據界線。
