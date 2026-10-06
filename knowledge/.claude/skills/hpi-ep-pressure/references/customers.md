# EP客戶條件索引

客戶條件與機型、EP型態及一般設定分開。

| 客戶碼 | 函式／Task | 開關（INSTALL_/USE_/FUNC_CC_） | 行為差異一句話 | 來源（906/912,912-only） | 相關機台 |
|---|---|---|---|---|---|
| CC_GIGAS | ADAM_ReturnValueCheck | EP_Install=3／5；live gate另查 | 原回授電壓異常檢查排除GIGAS | V906 Pressure helper條件已核對；golden依[原排錯](adam/references/troubleshooting.md) | 配有回授的EP，非9050專屬 |
| CC_KYEC_LEE | KpaTransferKG／MultiTransferKG及TransformFuntion | EP模式與力量表另查 | 原特殊缸徑2.8→3.0、5.8→6.0分支；不可套所有客戶 | V906 helper／TransformFuntion分支已核對；golden見原排錯 | 原配置／案例 |

百分比容差、ASE等其他分支由原程式地圖與當前source再核對；本表不宣稱已盤點全部客戶。
