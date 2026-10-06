> 保存來源：`.claude/skills/ht9045-inarm-flow/references/InArm_Debug_Methodology.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# InArm 8-Picker 模式問題解決方法論

## 案例背景 (2026-03-27)
- **機台**：HT9045 IC Test Handler（1x4 site, QFN 6x6）
- **症狀**：選擇 8 吸嘴模式（iUseSuckMode=8），但 InArm 行為仍為 1x4（前排或後排單行取放）
- **根本原因**：配置標誌 `IniConfig.b1x4Use8Suck` 在 VTEST 客戶函數中未被設置為 true
- **解決方案**：在 CosFunction.cpp 第 2334 行添加 `IniConfig.b1x4Use8Suck = true;`

---

## 關鍵洞察

### 1. 症狀 ≠ 根本原因
- **表面症狀**：InArm 一次只取一行（前排 或 後排）
- **假設 #1**：iUseSuckMode 被降級了 → **排除**（用戶確認 setupDowngrade 機制未觸發）
- **假設 #2**：LoaderXPitch 超過 40mm 閾值 → **排除**（確認 9.2mm << 40mm）
- **假設 #3**：iPickRow==1 而非 2 → **表面症狀**，不是根本原因
- **真正根因**：上游配置標誌缺失 → 導致 ChangeUseSuckMode() 強制降級模式

### 2. 配置標誌的雙重角色
配置標誌不僅控制顯示/UI，而且是 **功能 Gate**（控制點）：
```cpp
// ChangeUseSuckMode() case QualSite1X4 (ckernel.cpp:288)
if(i8PickerHPMode==iHPWideHP && IniConfig.b1x4Use8Suck)  // <- Gate here
{
    if(HotPlateForm.XDivision in {4,6,8,12})
        break;  // 保持 iUseSuckMode=8
    else
        TestIF.iUseSuckMode=4;  // <- 降階到 4
}
else
    TestIF.iUseSuckMode=4;  // <- VTEST 未設標誌時一定降階
```

### 3. 調試追蹤策略（按優先順序）
1. **確認初始設定**：驗證 setup.inf / Gerneral.ini / 全局變量 是否符合預期
2. **排除降階條件**：檢查所有會強制降級模式的條件（如 ChangeUseSuckMode）
3. **驗證狀態機邏輯**：確認 dispatch 和 branching 邏輯在接收正確模式時是否工作
4. **追溯初始化**：若狀態機邏輯正確，問題必在上游初始化層
5. **定位客戶特定配置**：檢查 CosFunction.cpp 的 FUNC_CC_* 客戶函數

### 4. 客戶特定函數的完整性檢查
VTEST/各客戶配置函數需要檢查：
- `iUseSuckMode` 相關的配置標誌（如 `b1x4Use8Suck`）
- 功能特性開關（如 `bAutoSiteMappingUseHotPlate`）
- 硬體選項（如 `bUseAxxGPicker`）

**模板**：若其他客戶功能中有某標誌，檢查新客戶配置是否也設了

---

## 具體除錯工序

### 步驟 1：驗證 iUseSuckMode 初始值
搜尋 `TestIF_File.iUseSuckMode =` 於 ainarm2.cpp 附近（初始化）
搜尋 `iUseSuckMode=` 於 ckernel.cpp（所有降級點）

### 步驟 2：檢查降級條件
若 iUseSuckMode 被設為 4，追蹤：
- 是否來自 ChangeUseSuckMode() 的 case branch
- case 內的條件判定（個別檢查三個 if 條件）

### 步驟 3：若條件不符，向上追溯配置
搜尋 `b1x4Use8Suck =` 找所有設置點
讀取 CosFunction.cpp 對應客戶函數中的設置

### 步驟 4：確認客戶函數完整性
檢查 FUNC_CC_* 函數是否缺少相關標誌設置

### 步驟 5：檢查後續邏輯是否正確
一旦 iUseSuckMode 正確，驗證：
- `DoInArm_9045_Type()` dispatch 邏輯
- `SearchLoadTrayUpDown_9045()` branching（需要 iPickRow==2 AND 料盤條件）
- `SetPickerCount(2,4,...)` 設置 iPickRow

---

## 核心代碼位置備忘

| 功能 | 檔案 | 行數 | 用途 |
|------|------|------|------|
| 降級邏輯 | ckernel.cpp | 288-310 | **ChangeUseSuckMode() case QualSite1X4** |
| 客戶配置 | CosFunction.cpp | 2265-2334 | VTEST_Funtion() 設置 ~40 個標誌 |
| 狀態機分派 | ainarm9045.cpp | 1599 | DoInArm_9045_Type() 選擇変體 |
| 分株選擇 | ainarm9045.cpp | 6177 | SearchLoadTrayUpDown_9045() 雙行檢查 |
| 8-吸嘴変体 | ainarm9045_1x4_8_Hot.cpp | 32 | SetPickerCount(2,4,1,4,1,0,0) 設 iPickRow=2 |
| Sucker 初始化 | mykitsuck.cpp | 205-217 | SetPickerCount() 唯一設置 iPickRow 的地方 |

---

## 通用除錯模式（未來應用）

### 症狀特徵：「功能已選但不生效」
- 可能原因：配置標誌 gate 未開啟
- 除錯方向：配置層 → 降級檢查 → 狀態機邏輯

### 預防檢查清單
- [ ] 新增客戶函數時，複製其他客戶的標誌設置清單
- [ ] 版本發布前，掃描 CosFunction.cpp 確認所有 FUNC_CC_* 函數的標誌完整性
- [ ] 機台行為異常時，優先檢查 ChangeUseSuckMode() 的降級邏輯
- [ ] 配置標誌應與代碼邏輯一起審查，不能只看 INI 檔案

<!-- preserved-content:end -->
