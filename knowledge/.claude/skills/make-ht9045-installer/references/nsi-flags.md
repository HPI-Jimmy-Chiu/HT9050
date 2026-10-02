# NSI 旗標設定參考

> 修改 `D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI.nsi` 頂端的 `!define` 行。
> **建議使用腳本**（`scripts\set_nsi_flags.py`）切換，不要手動編輯 NSI。

---

## CustomVersion（客製版前綴）

> 切換前，確認原始碼 `MachineType.h` 中對應 `#define` 已啟用，與 NSI 旗標必須一致。

| `!define CustomVersion` | `MachineType.h` `#define` | 說明 |
|------------------------|---------------------------|------|
| `""` | （無）| 標準版（預設） |
| `"TF-AMD_"` | `AMD_Version` | AMD 客製版 |
| `"KL_"` | `HiSilicon` | KL（海思）客製版 |
| `"MTK_"` | `MTK_Version` | MTK 客製版 |
| `"EVAN_"` | `FOR_EVAN` | EVAN 客製版 |
| `"QROVO_"` | — | Qorvo 客製版 |

---

## HandlerType（機型前綴）

| 值 | 說明 |
|----|------|
| `"HT9xxx"` | 通用型（預設，`HT9045_MUI.nsi`） |
| `"HT9045"` | Win10 版預設（`HT9045_MUI_Win10.nsi`） |
| `"HT1032"` | HT1032 機型 |
| `"HT9011UC"` | HT9011UC 機型 |

---

## BetaVersion（Beta 後綴）

> `_EurekaLog` 與 `_CodeGuard` 是編譯器層級設定，需先在 `HT9045.bpr` 確認對應選項已啟用（Linker / Compiler 頁籤）。

| `!define BetaVersion` | 對應 `MachineType.h` / `HT9045.bpr` 設定 | 說明 |
|-----------------------|------------------------------------------|------|
| `""` | （無）| 正式版（預設） |
| `"_BETA"` | `#define BETA_VERSION`（`MachineType.h`） | Beta 測試版 |
| `"_EurekaLog"` | `HT9045.bpr` 啟用 EurekaLog 連結設定 | EurekaLog 除錯版 |
| `"_CodeGuard"` | `HT9045.bpr` 啟用 CodeGuard 選項 | CodeGuard 版 |

---

## set_nsi_flags.py 使用方式

路徑：`d:\.github\skills\make-ht9045-installer\scripts\set_nsi_flags.py`

```powershell
# 切換為 AMD 客製 + Beta
python "d:\.github\skills\make-ht9045-installer\scripts\set_nsi_flags.py" "TF-AMD_" "_BETA"

# 還原為標準版
python "d:\.github\skills\make-ht9045-installer\scripts\set_nsi_flags.py" "" ""
```

**自動補全規則**：
- `CustomVersion` 結尾無 `_` → 自動補上（`a` → `a_`）
- `BetaVersion` 開頭無 `_` → 自動補上（`b` → `_b`）
- 每次執行完整替換旗標區塊，不會累積多餘注解行
