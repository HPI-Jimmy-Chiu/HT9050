> 保存來源：`.claude/skills/ht9045-general-ini/references/main-mapping.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# main.cpp — Gerneral.ini 存取對應表

> 原始檔：`HT9045/HT9011UC_Code_V3.33.898.0_20260313_Steven/main.cpp`
> 主要位於 `TfMain()` 建構函式 + `FormShow()` + 散布於各事件處理

## 目錄

- [遷移模式說明](main-mapping.md#遷移模式說明)
- [System Section — 遷移寫入 + 讀取](main-mapping.md#system-section--遷移寫入--讀取)
- [VENDER Section](main-mapping.md#vender-section)
- [Record Section](main-mapping.md#record-section)
- [In Arm Section](main-mapping.md#in-arm-section)
- [Password Section](main-mapping.md#password-section)
- [Version Section](main-mapping.md#version-section)
- [AOA 校正寫入（~30 Key）](main-mapping.md#aoa-校正寫入30-key)
- [main 獨有 Key](main-mapping.md#main-獨有-key)

---

## 遷移模式說明

main.cpp 使用 `CheckIniData()` 函式檢查 INI Key 是否存在，不存在時寫入預設值：

```cpp
if(CheckIniData(asGeneralPath, "System", "KEY_NAME") == false)
{
    iTemp = <預設值或彈窗詢問>;
    WriteIniDataGeneral("System", "KEY_NAME", iTemp);
    Variable = iTemp;
}
else
{
    Variable = CheckAndReadIniDataGeneral("System", "KEY_NAME", DefaultValue);
}
```

此模式確保舊版 INI 檔升級時不會因缺少 Key 而異常。

---

## System Section — 遷移寫入 + 讀取

| Key | 變數 | 預設值 | 操作 | 遷移方式 |
|-----|------|--------|------|----------|
| EP_Install | EP_Install | 0 or 1 | Read+Write | Dialog 詢問使用者 |
| EP_MAXKPA | EP_MAXKPA | 499.0 | Read | 僅 EP_Install≠0 時讀取 |
| EP_MAXA | EP_MAXAFB | 5.013 | Read | 僅 EP_Install≠0 時讀取 |
| EP_MINMPA | EP_MINMPA | 0.001 | Read | 僅 EP_Install≠0 時讀取 |
| EP_MINA_FeedBack | EP_MinAFB | 0.908 | Read | 僅 EP_Install≠0 時讀取 |
| EPDual_MAXKPA | EPDual_MAXKPA | 899.0 | Read | **main 獨有** — 雙 EP |
| EPDual_MAXAFB | EPDual_MAXAFB | 4.905 | Read | **main 獨有** — 雙 EP |
| EPDual_MINMPA | EPDual_MINMPA | 0.001 | Read | **main 獨有** — 雙 EP |
| EPDual_MinAFB | EPDual_MinAFB | 0.968 | Read | **main 獨有** — 雙 EP |
| INOUT_ARM_PICKER_USE_MOTOR | InOutArmPickerUseMotor | 1 | Read+Write | 遷移 + 讀後自動修正（讀到 0 時強制寫回 1） |
| SUPPORT_2_EMPTY_EMPTY | SUPPORT_2_EMPTY_EMPTY | 0 or 1 | Read+Write | Dialog 詢問 7-Track Empty Unloader |
| ION_FAN_TYPE | ION_FAN_TYPE | 1 | Read+Write | Dialog 詢問 |
| INDEX_SUCKER_TYPE | INDEX_SUCKER_TYPE | 0 or 1 | Read+Write | Dialog 詢問負壓系統 |
| IndexTimeSet | bIndexTimeSet | 0 | Read+Write | 自動寫入預設 0 |
| bContaceTorque | bContaceTorque | 0 | Read+Write | **main 獨有** — 自動寫入，TSMC 接觸力矩 |
| bUse_NewAutoCleanForm | bUse_NewAutoCleanForm | 0 | Read+Write | **main 獨有** — 自動寫入 |
| bUseNewCleanModeKit | bUseNewCleanModeKit | 0 | Read+Write | **main 獨有** — 自動寫入 |
| SetupFileCheckList | asSetupFileCheckList | "HisiATC_..." | Read | **main 獨有** — Setup 檔名 |
| bInitialCleanCount | bInitialCleanCount | 0 | Read+Write | **main 獨有** — 讀後自動重置（CC_TSMC_TAINAN） |
| bHasEnteredPEModel | bHasEnteredPEModel | (bool) | Write | PE 模式追蹤（散布於事件處理） |

## VENDER Section

| Key | 變數 | 預設值 | 操作 | 說明 |
|-----|------|--------|------|------|
| ASEK15UsePW | bASEK15UsePW | 0 | Read+Write | **main 獨有** — 遷移寫入 |
| ASEK15PassWord | ASEK15PassWord | "27025312" | Read+Write | **main 獨有** — 遷移寫入 |
| HONTECH | HonPrecPassword | "27025312" | Read | **main 獨有** — 舊密碼讀取 |
| HONPREC | HonPrecPassword | "27025312" | Read+Write | **main 獨有** — 密碼同步（HONTECH→HONPREC） |

## Record Section

| Key | 變數 | 值 | 操作 | 說明 |
|-----|------|-----|------|------|
| Program Close | (直接值) | 0 | Write | **main 獨有** — 程式啟動時寫 0 |
| Program Close | (直接值) | 1 | Write | **main 獨有** — 程式關閉時寫 1 |

## In Arm Section

| Key | 變數 | 預設值 | 操作 | 說明 |
|-----|------|--------|------|------|
| ZSafePos | ZSafePos | 20 (SCK) / 50 (其他) | Read | **main 獨有** — 依 CUSTOMER_CODE 條件預設 |

## Password Section

| Key | 變數 | 預設值 | 操作 | 說明 |
|-----|------|--------|------|------|
| Change | bChange | false | Read | **main 獨有** — 密碼變更旗標 |

## Version Section

| Key | 變數 | 值 | 操作 | 說明 |
|-----|------|-----|------|------|
| Ver | asHandlerVersion | (版本字串) | Write | ATC 6.0 關閉時更新版本 |

---

## AOA 校正寫入（~30 Key）

main.cpp 在 CCD 校正完成後，將光學對位偏移值寫入 `[System]`。這些 Key 在 database.cpp 中被讀取，但僅在 main.cpp 中被寫入。

| Key 模式 | 數量 | 說明 |
|----------|------|------|
| AOA_InArm_Loader_X/Y | 2 | InArm Loader 偏移 |
| AOA_InArm_Shuttle1/2_X/Y | 4 | InArm Shuttle 偏移 |
| AOA_InArm_Hotplate1/2_X/Y | 4 | InArm Hotplate 偏移 |
| AOA_OutArm_Auto1-6_X/Y | 12 | OutArm Auto 偏移（Auto4-6 條件） |
| AOA_OutArm_Fix1-6_X/Y | 12 | OutArm Fix 偏移（Fix4-6 條件） |
| AOA_OutArm_Shuttle1/2_X/Y | 4 | OutArm Shuttle 偏移 |

> 注意：這些 AOA Key 由 main.cpp **寫入**、database.cpp **讀取**，HandlerSys 不參與。

---

## main 獨有 Key

以下 Key **僅在 main.cpp 中處理**，database.cpp 與 HandlerSys 均不涉及：

| Section | Key | 說明 |
|---------|-----|------|
| System | EPDual_MAXKPA | 雙 EP 最大 KPA |
| System | EPDual_MAXAFB | 雙 EP 最大 Ampere |
| System | EPDual_MINMPA | 雙 EP 最小 MPA |
| System | EPDual_MinAFB | 雙 EP 最小 Ampere |
| System | bContaceTorque | 接觸力矩（TSMC） |
| System | bUse_NewAutoCleanForm | 新清潔表單 |
| System | bUseNewCleanModeKit | 新清潔模式 Kit |
| System | SetupFileCheckList | Setup 驗證檔名 |
| System | bInitialCleanCount | 清潔計數重置（TSMC TAINAN） |
| System | IndexTimeSet | Index 時間設定 |
| In Arm | ZSafePos | Z 安全位置 |
| VENDER | ASEK15UsePW | ASEK15 密碼啟用 |
| VENDER | ASEK15PassWord | ASEK15 密碼值 |
| VENDER | HONTECH | 舊密碼（讀取用） |
| VENDER | HONPREC | 主密碼（同步寫入） |
| Record | Program Close | 程式啟閉旗標 |
| Password | Change | 密碼變更旗標 |

<!-- preserved-content:end -->
