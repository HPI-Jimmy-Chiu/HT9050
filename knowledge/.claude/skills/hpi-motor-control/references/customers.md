# Motor條件索引

本批沒有加入未核對的CUSTOMER_CODE功能。機型／driver分派、PICKER與HPP配置、歷史客戶案例分別維持原標示；不等同所有客戶共有。

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| 未核對客戶碼分支；類別狀態規則 | 新動作gate設計 | 依類別提供的狀態，不以機型／卡名開關 | Steven Q119要求各種motor class共用允許／拒絕原則，資料路由另分派 | [原裁決](control/original-entry.md)，實作需逐工作卡核對 | 多種軸卡／EtherCAT／MotionNet |

具體客戶案例與driver單位見各原文，不把研究或統計樣本轉成當前啟用清單。
