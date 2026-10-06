> 保存來源：`.claude/skills/ht9045-customer-code-manager/references/source-map.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# customer-code-manager 源碼參照

> 版本基準：HT9011UC_Code_V3.33.900.0_20260331

## 需修改的檔案（插入新 CC_ 時）

| 檔案 | 功能 | 關鍵位置 |
|------|------|----------|
| `MachineType.h` | `#define CC_xxx nnn` 定義區 | L124–L370（依數值升序插入）|
| `CosFunction.cpp` | `CustomerFunctionSelect()` Switch-Case | L3624（函數入口）|
| `HandlerSys.cpp` | `SaveSystemSet()` / `LoaderSystemSet()` | L535（寫入）/ L124（讀取）|
| `HandlerSys.dfm` | `edtCustomerCode`（TEdit UI 元件） | L316 |

## MachineType.h — CC_ 定義區間

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\MachineType.h
CC_ 起始行：L124   #define CC_HONPREC_QC  0
CC_ 結尾行：L370   #define CC_HTML_Monitor  99999
插入規則：依數值升序插入（找最後一個值 < 新代碼的 #define 之後）
```

### 現有代碼範圍

| 範圍 | 說明 |
|------|------|
| 0 | CC_HONPREC_QC（保留） |
| 729 – 808 | 鴻勁興業代理客戶 |
| 810 – 899 | HPI / TeraTech / JB-Elite 代理客戶 |
| 900 – 998 | 直銷大客戶 |
| 999 | CC_QUALCOMM |
| 99999 | CC_HTML_Monitor |

## CosFunction.cpp — CustomerFunctionSelect()

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\CosFunction.cpp
函數入口：L3624
觸發時機：程式啟動 database.cpp::L335、工作檔切換 cprod.cpp::L2937
作用：依 CUSTOMER_CODE 全域值套用客戶特定預設值（IO 配置、功能旗標等）
新增 case 格式：
  case CC_XXX_YYY:
      // 設定說明
      break;
```

## HandlerSys.cpp — CUSTOMER_CODE 讀寫

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\HandlerSys.cpp
讀取（UI ← INI）：L124  edtCustomerCode->Text = CheckAndReadIniDataGeneral("System","CUSTOMER_CODE",0)
寫入（UI → INI）：L535  CUSTOMER_CODE = atoi(edtCustomerCode->Text)
                  L536  WriteIniDataGeneral("System","CUSTOMER_CODE",CUSTOMER_CODE)
```

## HandlerSys.dfm — UI 元件

```
路徑：d:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\HandlerSys.dfm
edtCustomerCode：TEdit，L316
tsCustomerCode：TTabSheet，L5941
```

## GPIB 同步（跨專案）

新增 CC_ 時須同步更新：
- `d:\GPIB9045\GPIB_Code_32Site_V12.13.900.0_20260331\cmydef.h`
  （依數值升序插入 `#define CC_xxx nnn`）

<!-- preserved-content:end -->
