> 保存來源：`.claude/skills/ht9045-index-flow/references/child-state-machines.md`，main `e184ef205`。以下保留原文；原文中的機型／版本與「裁決、提案、已實作」仍依原標註。當前實作狀態先看 [共同與差異](../common.md)。

<!-- preserved-content:start -->
# Index 子狀態機詳細 Case 清單

## DoTestYFront() (`aTester_Front.cpp`)

> Task variable: `int &Task = iTestYFrontTask`

- 前置/掉料/安全段：`1, 50, 52, 55, 56, 57, 60, 61, 62, 64, 65, 66, 67, 68, 70, 72, 73, 75, 7500, 76, 78, 79, 80`
- 中段主測試路徑：`81, 82, 84, 90, 95, 97, 100, 105, 106, 108, 109, 110, 115, 116, 130, 140, 200, 209, 2091, 2092, 210, 211, 212, 213, 215, 220, 230, 240`
- 後段處理：`300, 310, 311, 312, 313, 314, 315, 316, 320, 325, 326, 330, 340, 350`
- 擴充/特例：`1000, 1100, 1200, 1300, 2000, 12000, 12010, 14000, 16000, 16100, 17900, 18000, 18100, 18500`

---

## DoTestYRear() (`aTester_Rear.cpp`)

> Task variable: `int &Task = iTestYRearTask`

- 前中段：`1,2,3,50,52,55,56,57,60,61,62,64,65,66,67,68,70,72,73,75,7500,76,78,79,80,81,82,84,90,95,97,100,105,106,108,109,110,115,130,140,200,209,2091,2092,210,211,212,213,215,220,230,240,300,310,311,312,313,314,315,316,320,325,326,330,340,350`
- 延伸段（Rear 特有大分支）：`1000,1100,1200,1300,2000,10500,11000,11002,11010,11015,11016,11017,11020,11030,11035,11040,11041,11050,11060,11080,11090,11095,11096,11100,11110,12000,12010,14000,16000,16100,17900,18000,18100,18500`

---

## DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

> Task variable: `int &Task = iTestTwoArm32SiteTask`

- 初始與掉料：`1, 50, 52, 55, 60, 61, 62, 64, 65, 66, 67, 68`
- 主測試段：`81, 82, 84, 85, 90, 95, 97, 100, 105, 106, 108, 110, 120, 130, 200, 209, 210, 220, 221, 225, 226, 230, 240, 241, 250`
- 後段：`300, 310, 311, 312, 313, 314, 315, 316, 320, 325, 330, 340`
- 結尾：`2000, 12000, 12010`

---

## DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

> Task variable: `int &Task = iFrontTestDestroyICTask`

- Case: `1, 100, 200, 309, 310, 320, 500, 550, 600, 650, 700, 750, 800`
- 用途：Front 測試頭殘 IC 清除（PlaceToShuttleFirst 路徑）

---

## DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

> Task variable: `int &Task = iRearTestDestroyICTask`

- Case: `1, 100, 200, 309, 310, 320, 500, 550, 600, 650, 700, 750, 800`
- 用途：Rear 測試頭殘 IC 清除

---

## DoFTestSuckTestIC() (`aTester_Front.cpp`)

> Task variable: `int &Task = iFTestSuckTestICTask`

- Case: `1, 100, 200, 2200, 2400, 2410, 2500, 2600, 2700, 3000, 3050, 3100, 5000, 5050, 5100, 5300, 5350, 5400, 5450, 5500, 6000, 6100, 6200, 6300, 6500, 6600, 7000, 7100`
- 用途：AutoSiteMap/測試驗證流程（Front）

---

## DoBTestSuckTestIC() (`aTester_Rear.cpp`)

> Task variable: `int &Task = iBTestSuckTestICTask`

- Case: `1, 100, 200, 2200, 2400, 2410, 2500, 2600, 2700, 3000, 3050, 3100, 5000, 5050, 5100, 5300, 5350, 5400, 5450, 5500, 6000, 6100, 6200, 6300, 6500, 6600, 7000, 7100`
- 用途：AutoSiteMap/測試驗證流程（Rear）

---

## DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

> Task variable: `int &Task = iTestSuckTestIC_TwoArm32Site_Task`

- Case: `1, 2200, 2400, 2500, 2600, 2700, 3000, 3100, 5000, 5100, 5200, 5300, 5400, 5500, 6000, 6100`
- 用途：32-site 兩臂 AutoSiteMap/測試驗證流程

---

## IndexEveryTimeCheckEP() (`atester.cpp`)

> Task variable: `int &Task = iIndexEveryTimeCheckEPTask`

- Case: `1 → 100 → 200 → 300`
- 用途：每次 Index 前 EP 充氣壓力確認與告警重試

<!-- preserved-content:end -->
