> 保存來源：`.claude/skills/ht9045-config/references/config-ui-naming.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# cConfiguration UI 元件放置與命名慣例

> 來源：`cConfiguration.h`（__published 區段）、`cConfiguration.dfm`

---

## 1. 頁籤層次結構

```
fConfiguration（主視窗）
└── PageControl1（主分頁）
    ├── tsSoftSimu     ← Soft Simulate Speed
    ├── tsTempComm     ← 溫控通訊（Comm）
    ├── tsConfig       ← 主設定（Config）
    │   └── pcConfig（群組分頁）
    │       ├── tsA00  ← A [Function]
    │       ├── tsb00  ← B [Report]
    │       ├── tsC00  ← C [Hardware]
    │       ├── tsD00  ← D [Index]
    │       ├── tsE00  ← E [In/Out Arm]
    │       ├── tsF00  ← F [Shuttle]
    │       ├── tsG00  ← G [Visible]
    │       ├── tsI00  ← I [Tester]
    │       ├── tsL00  ← L [Temperature]
    │       ├── tsM00  ← M [Monitor]
    │       ├── tsN00  ← N [Network]
    │       ├── tsO00  ← O [Count]
    │       ├── tsP00  ← P [Tray]
    │       └── tsSearchFunction ← 全文搜尋
    ├── tsTrayData     ← Tray 定義表
    └── tsHPData       ← Hot Plate 定義表
```

---

## 2. 群組內佈局模式

每個字母群組使用下列兩種佈局之一：

| 模式 | 使用群組 | 說明 |
|------|---------|------|
| **PageControl 模式** | A、B、C、D、E、F、L、N、O、P | `pcXXX00` 包含多個子 TabSheet |
| **Panel 模式** | G、I、M | `pal_X` 單一面板，不再分頁 |

每個字母群組都有一個 **`MemoX`**（TMemo）用於顯示功能說明備忘：  
`MemoA`, `MemoB`, `MemoC`, `MemoD`, `MemoE`, `MemoF`, `MemoG`, `MemoI`, `MemoL`, `MemoM`, `MemoN`, `MemoO`, `MemoP`

---

## 3. 子 TabSheet 命名規則

| 規則 | 範例 | 適用情境 |
|------|------|---------|
| `ts` + 字母 + `_NN`（流水編號） | `tsA_00`, `tsA_01`, `tsI_01`, `tsI_20`, `tsI_30`, `tsL_00`, `tsL_10`, `tsO_00`, `tsO_11` | 依序排列的子頁 |
| `ts` + 字母 + `_Desc`（功能描述） | `tsD_Preasure`, `tsD_Contact`, `tsD_Mode`, `tsD_Other`, `tsE_XYScale` | 有明確功能主題的子頁 |
| `tsNNN`（N 群組按功能編號） | `tsN05`, `tsN06`, `tsN07`, `tsN08`, `tsN09`…`tsN24` | N 群組以功能代碼命名子頁 |
| `tsF01`, `tsF21`, `tsE50` | 同上 | F / E 個別功能頁 |

---

## 4. UI 元件命名前綴對照表

| 前綴 | VCL 型別 | 說明 | 範例 |
|------|---------|------|------|
| `cb` | TCheckBox | 主要勾選框（最常見） | `cbD41`, `cbA32`, `cbI21` |
| `chk` | TCheckBox | 次要勾選框（部分功能） | `chkN12`, `chkO16`, `chkI29_1` |
| `ed` | TEdit | 數值 / 文字輸入框 | `edD44`, `edN06_HostName`, `edA22_2` |
| `edt` | TEdit | 次要輸入框（部分功能） | `edtN14_1`, `edtO06_Local`, `edtA12_1` |
| `lbled` | TLabeledEdit | 帶標籤的輸入框 | `lbledtN23_1_URL`, `lbledtN24` |
| `lab` | TLabel | 欄位標籤（多數） | `labD25_1`, `labN06_Host`, `labL11_1` |
| `lbl` | TLabel | 欄位標籤（部分） | `lblA16`, `lblD01_1`, `lblN07_7` |
| `tb` | TTrackBar | 滑桿（EP Load Rate） | `tbD25_Index60mm`, `tbD25_Index40mm_NS` |
| `rg` | TRadioGroup | 單選群組 | `rgF07`, `rgI22`, `rgL10`, `rgN09_4` |
| `co` | TComboBox | 下拉選單（主要） | `coD03`, `coD22`, `coI20`, `coI25` |
| `cbb` | TComboBox | 下拉選單（次要） | `cbbO15_1`, `cbbD47` |
| `gb` | TGroupBox | 分組框（功能區塊） | `gbD25`, `gbL11`, `gbN06_FTP`, `gbM01` |
| `grp` | TGroupBox | 分組框（次要） | `grpA20`, `grpD47`, `grpN14`, `grpN22` |
| `pal_` | TPanel | 群組主容器面板 | `pal_G`, `pal_I`, `pal_D1`, `pal_L1` |
| `pnl` | TPanel | 區塊子面板 | `pnlF14`, `pnlN14_15`, `pnlN22` |
| `pc` | TPageControl | 群組主分頁控制 | `pcA00`, `pcD00`, `pcF00`, `pcN00` |
| `pgc` | TPageControl | 功能內分頁控制 | `pgcN14_1`, `pgcN23`, `pgI00` |
| `ts` | TTabSheet | 分頁 | `tsA00`, `tsD_Contact`, `tsN06` |
| `ud` | TUpDown | 上下計數器 | `udD46` |
| `dt` / `dtp` | TDateTimePicker | 日期時間選擇器 | `dtO06_LastDate`, `dtpO06NextTime` |
| `img` | TImage | 圖片元件 | `imgI37_3` |
| `strngrd` | TStringGrid | 字串表格 | `strngrdAutoSaveLog`, `strngrdTray` |
| `sb` / `spb` | TSpeedButton | 功能按鈕 | `spbA25_RunExecutFilePathChoice`, `sbExit` |
| `btn` | TButton | 一般按鈕 | `btnAdd1000`, `btnN15ESDForm` |
| `bt` | TButton | 一般按鈕（部分） | `btD47`, `btN06_TesterList` |

---

## 5. M 群組的特殊命名

M01 群組的子 CheckBox 使用 **`cbM01_NN`** 的複合格式，代表第 N 項強制開關：

```
cbM01           ← M01 總開關（Monitor Function Enable）
cbM01_01        ← M0101 Contact Mode Use Different Speed
cbM01_02        ← M0102 Site Yield Different Must On
cbM01_03        ← M0103 Continue Fail By Socket Must On
...
cbM01_14        ← M01 最後一個強制開關項目
```

---

## 6. 新增元件的命名原則

1. **CheckBox**：`cb` + 功能代碼（含字母群組與數字）  
   例：新增 N 群組第 99 項 → `cbN99_NewFunction`

2. **Edit（數值輸入）**：`ed` + 功能代碼  
   例：`edN99_Value`；若有多個欄位則加後綴 `_1`、`_2` 或描述詞 `_Host`、`_Path`

3. **Label（標籤）**：`lab` + 功能代碼（較短）或 `lbl` + 功能代碼（較長）  
   例：`labN99_1`, `lblN99_Description`

4. **GroupBox（功能區）**：`grp` + 功能代碼  
   例：`grpN99`

5. **子 TabSheet**（群組有多子頁時）：`ts` + 字母 + `_NN` 或 `_Desc`  
   例：新增 A 群組第 4 個子頁 → `tsA_04` 或 `tsA_NewFeature`

<!-- preserved-content:end -->
