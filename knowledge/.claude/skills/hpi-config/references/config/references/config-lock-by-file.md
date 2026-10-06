> 保存來源：`.claude/skills/ht9045-config/references/config-lock-by-file.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Lock by File 機制參考

> 控制檔案：`AuthPath + "config.ini"` 的 `[Specific]` 區段
> 讀取函式：`ReadLockByFile()`
> 更新函式：`ChangeCBListProperty()`

---

## 機制說明

`Lock by File` 讓特定功能的 UI 狀態（勾選 / 可操作）由外部 `config.ini` 控制，而非用一般設定介面存檔。適用於需要在現場由維護人員鎖定、客戶端無法自行修改的功能。

```
AuthPath + "config.ini"   [Specific] 區段
  Key_Active   = true/false   ← UI CheckBox 是否勾起（功能是否啟用）
  Key_Enabled  = true/false   ← UI CheckBox 是否可被使用者操作
```

- `_Active = true` + `_Enable = false`：功能強制開啟，用戶端不能改
- `_Active = false` + `_Enable = false`：功能強制關閉，用戶端不能改
- `_Active = xxx` + `_Enable = true`：功能預設值，用戶端可自行切換

---

## Lock by File 欄位對照表

| `[Specific]` Key | IniConfig 欄位 | 功能說明 | 實作版本 |
|-----------------|----------------|----------|---------|
| `D41_Active` | `bD41_Active` | D41 Socket Position Check 啟用 | Steven 20140627 |
| `D41_Enabled` | `bD41_Enable` | D41 使用者可修改 | Steven 20140627 |
| `D42_Active` | `bD42_Active` | D42 Index Pick Shuttle Pause 啟用 | JerryYang 20160220 |
| `D42_Enable` | `bD42_Enable` | D42 使用者可修改 | JerryYang 20160220 |
| `D44_Active` | `bD44_Active` | D44 Check Index IC Destroy 啟用 | JerryYang 20160220 |
| `D44_Enable` | `bD44_Enable` | D44 使用者可修改 | JerryYang 20160220 |
| `F06_Active` | `bF06_Active` | F06 Initial IC Check 啟用 | Steven 20140627 |
| `F06_Enabled` | `bF06_Enable` | F06 使用者可修改 | Steven 20140627 |
| `F11_Active` | `bF11_Active` | F11 Out Sht Front/Rear Sensor 啟用 | Sam 20240202 |
| `F11_Enable` | `bF11_Enable` | F11 使用者可修改 | Sam 20240202 |
| `F26_Enable` | `bF26_Enable` | F26 Out Sht Jam Skip/Retry 可操作 | Sam 20220527 |
| `I06_Active` | `bI06_Active` | I06 功能啟用（矽格北興） | Sam 20220527 |
| `I06_Enable` | `bI06_Enable` | I06 使用者可修改 | Sam 20220527 |
| `P24_Active` | `bP24_Active` | P24 Skip Event 移除盤啟用（矽格） | JerryYang 20160425 |
| `P24_Enable` | `bP24_Enable` | P24 使用者可修改 | JerryYang 20160425 |
| `RTC_Active` | `bRTC_Active` | RTC 功能啟用 | Sam 20240311 |
| `RTC_Enable` | `bRTC_Enable` | RTC 使用者可修改 | Sam 20240311 |

---

## 程式碼位置

### 讀取

```cpp
// cConfiguration.cpp - ReadLockByFile()
void TfConfiguration::ReadLockByFile()
{
    AnsiString sPath = AuthPath + "config.ini";

    // D41
    IniConfig.bD41_Active = ReadIniData(sPath, "Specific", "D41_Active", false);
    IniConfig.bD41_Enable = ReadIniData(sPath, "Specific", "D41_Enabled", true);

    // F06
    IniConfig.bF06_Active = ReadIniData(sPath, "Specific", "F06_Active", false);
    IniConfig.bF06_Enable = ReadIniData(sPath, "Specific", "F06_Enabled", true);

    // RTC
    IniConfig.bRTC_Active = ReadIniData(sPath, "Specific", "RTC_Active", false);
    IniConfig.bRTC_Enable = ReadIniData(sPath, "Specific", "RTC_Enable", true);
    // ... 其他同理
}
```

### 套用到 UI

```cpp
// ChangeCBListProperty() - 依 IniConfig 欄位調整 CheckBox 狀態
if(CosFunction.bLockD41ByFile)
{
    cbD41_Active->Checked = IniConfig.bD41_Active;
    cbD41_Active->Enabled = IniConfig.bD41_Enable;
}
```

---

## 新增 Lock by File 功能的標準步驟

1. 在 `Config.h` 對應群組新增 `bXxx_Active` 與 `bXxx_Enable` 欄位
2. 在 `ReadLockByFile()` 中新增讀取：
   ```cpp
   IniConfig.bXxx_Active = ReadIniData(sPath, "Specific", "Xxx_Active", false);
   IniConfig.bXxx_Enable = ReadIniData(sPath, "Specific", "Xxx_Enabled", true);
   ```
3. 在 `ChangeCBListProperty()` 新增 UI 同步：
   ```cpp
   if(CosFunction.bLockXxxByFile) {
       cbXxx_Active->Checked = IniConfig.bXxx_Active;
       cbXxx_Active->Enabled = IniConfig.bXxx_Enable;
   }
   ```
4. 如需透過 `HTEditList` 管理（可從 UI 儲存），在 `InitConfigEdtList_ItemX()` 也要加對應行
5. 在部署時，手動建立 `AuthPath/config.ini` 的 `[Specific]` 區段並寫入初始值

<!-- preserved-content:end -->
