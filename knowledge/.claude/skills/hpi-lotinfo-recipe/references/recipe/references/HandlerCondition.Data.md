> 保存來源：`.claude/skills/ht9045-recipe/references/HandlerCondition.Data.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# HandlerCondition.Data — 機台運行條件設定

**路徑：** `D:\HT9045\IniData\Data\[工作檔名]\HandlerCondition.Data`
**模組：** [uLotInfo.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uLotInfo.cpp)
**結構體：** `TestSocket`、`ArmSpeed[]`、`SHSpeed` 等多個全域變數
**讀/寫：** `ReadIniData()` / `WriteIniData()`

---

## 欄位定義

| 節點 | 欄位 | 型態 | 值域/說明 |
|------|------|------|----------|
| `[Configuration]` | Handling Mode | int | 1～9；進料/出料/Shuttle 組合模式 |
| | Site Aa～Hh | int | 各 Socket 序號；0=關, 1～N=啟用並指定編號 |
| | Shuttle1/2 Cancel | int | 0=使用, 1=不使用 |
| | Shuttle Mode | int | 0=一般, 1=特殊模式 |
| | X/Y Pitch | float | Shuttle 間距 (pulse，一般 0) |
| | Test Mode | string | 文字表示："2-Site", "4-Site" 等 |
| | Use Suck Mode | int | 0=標準, 1=特殊, 2=自訂（影響吸嘴真空策略） |
| | Search Last Mode | int | 0=一般, 1=搜索最後板位 |
| | iAutoClean_Function | int | 0=禁用, 1=啟用 AutoClean |
| | iAutoClean_Mode | int | 41=標準清潔模式；詳見 AutoClean 文件 |
| | iAutoClean_IntervalContact | int | 清潔間隔的接觸次數 |
| | iAutoClean_DeveicePices | int | 每次清潔裝置件數 |
| | iAutoClean_MotorSpeed[0~3] | int | 清潔時各馬達速度 (%) |
| | iAutoClean_ContactMode | int | 0=一般接觸, 其他同 Contact 模式 |
| | iAutoClean_Tray | int | AutoClean 使用的托盤編號 |
| | iAutoCleanShuttle | int | AutoClean 使用的 Shuttle 編號 |
| | NS7000/NS8000 Change Socket | int | NS5000/7000/8000 換燒座功能 |
| | Rotate Shuttle | int | 0=不旋轉, 1=旋轉 Shuttle 後再放 |
| | Real Time CCD | int | 0=關, 1=開啟即時 CCD 辨識 |
| | In Out Arm Y Pitch | int | InArm/OutArm Y 軸間距 (pulse) |
| | SocketSensor | int | Socket 感測器啟用 |

---

## 範例

```ini
[Configuration]
Handling Mode=5
Site Aa=1
Site Ab=2
Shuttle1 Cancel=0
Shuttle2 Cancel=1
X Pitch=160.00
Test Mode=2-Site
Use Suck Mode=2
iAutoClean_Function=0
iAutoClean_Mode=41
iAutoClean_IntervalContact=15
iAutoClean_MotorSpeed[0]=10
```

---

## 注意事項

- `configByRecipe.ini` 中若有同名欄位，會**覆蓋**此檔的設定值
- `Handling Mode` 決定整體機台配置（Shuttle 數量/InArm 數量/OutArm 組合）
- `iAutoClean_Mode=41` 為標準清潔模式；非 41 值需查閱 AutoClean 文件
- `Use Suck Mode=2` = 自訂真空策略，受 IniConfig 中的 EPControl 設定影響

---

## 關聯程式碼

- 讀寫實作：[uLotInfo.cpp](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uLotInfo.cpp) — `ReadIniData()`, `WriteIniData()`
- Site 配置對應：[uLotInfo.h](file:///d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\uLotInfo.h)
- AutoClean 流程：ht9045-inarm-flow Skill

<!-- preserved-content:end -->
