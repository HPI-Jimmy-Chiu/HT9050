# SECS/GEM SV / EC 對照表（自動產生）

> 由 [scripts/extract_secs_sv_ec.py](../../../../../../scripts/extract_secs_sv_ec.py) 從 V899 原始碼
> `SECSGEM/uHGemHT9045_SV.cpp` 與 `SECSGEM/uHGemHT9045_EC.cpp` 自動擷取，
> **請勿手動編輯 CSV**，原始碼變動後重跑腳本即可同步。

## 檔案

| 檔案 | 內容 | 筆數（V899） |
|------|------|------|
| `sv_table.csv` | 全部 Status Variable（唯讀） | 772 |
| `ec_table.csv` | 全部 Equipment Constant（可讀寫） | 1671 |

## 欄位說明

### sv_table.csv
| 欄位 | 說明 |
|------|------|
| `SVID` | SV 唯一識別碼（int） |
| `Type` | 資料型態（ASCII_TYPE / INT_4_TYPE / FT_8_TYPE / BOOLEAN_TYPE） |
| `Name` | SECS 可見變數名稱 |
| `Unit` | 單位（可空） |
| `Pointer` | 資料來源（變數、TEdit*、TLabel*、TPanel* 等） |
| `Comment` | 註解 |

### ec_table.csv
| 欄位 | 說明 |
|------|------|
| `ECID` | EC 唯一識別碼（int） |
| `Type` | 資料型態 |
| `Name` | SECS 可見變數名稱 |
| `Unit` | 單位（可空） |
| `Pointer` | 資料來源 |
| `Max` / `Min` / `Default` | 上限 / 下限 / 預設值（字串，可空） |
| `Comment` | 註解 |

## 如何查詢

- **查 ID → 變數**：在 CSV 搜尋 SVID/ECID 數字。
- **查名稱 → ID**：搜尋 `Name` 欄關鍵字（如 `Temperature`、`Contact`、`Lot`）。
- **查某全域變數被哪個 SV/EC 暴露**：搜尋 `Pointer` 欄（如 `LastSet.iTester`）。
- CSV 為 `utf-8-sig`，可直接用 Excel 開啟，中文註解不亂碼。

## 重新產生（原始碼異動後）

```powershell
python scripts/extract_secs_sv_ec.py
# 指定其他版本資料夾：
python scripts/extract_secs_sv_ec.py --code-folder HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422
```

腳本會解析多行呼叫、忽略字串內逗號，輸出至本資料夾的 `sv_table.csv` / `ec_table.csv`。
