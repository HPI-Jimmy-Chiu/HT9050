> 保存來源：`.claude/skills/bcb_build/SKILL.md`，main `2db43115d`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# Borland C++ Builder 6 — 通用 Build Skill

## 環境需求

| 項目 | 預設值 | 說明 |
|------|--------|------|
| BCB6 安裝路徑 | `D:\ProgramFiles\Borland\CBuilder6` | 可透過 `%BCB_ROOT%` 覆寫 |
| 必要工具 | `bpr2mak.exe`, `make.exe`, `bcc32.exe` | 位於 `<BCB_ROOT>\Bin\` |

## 使用方式

### 1. 快速呼叫 (PowerShell)

```powershell
# 安全建構（推薦）：建構後自動清理本次新開 PowerShell 視窗
.\scripts\build_bcb_safe.ps1 "D:\Project\RDCode\HT9011UC_Code_V3.20" "HT9045.bpr"

# 基本建構 — 指定專案目錄與 .bpr 檔名
.\scripts\build_bcb.bat "D:\Project\RDCode\HT9011UC_Code_V3.20" "HT9045.bpr"

# 重新建構 (先 clean 再 build)
.\scripts\build_bcb.bat "D:\Project\RDCode\HT9011UC_Code_V3.20" "HT9045.bpr" rebuild

# 僅清除中間產物
.\scripts\build_bcb.bat "D:\Project\RDCode\HT9011UC_Code_V3.20" "HT9045.bpr" clean
```

### 2. 腳本參數說明

```
build_bcb.bat <PROJECT_DIR> <BPR_FILE> [clean|rebuild]

  PROJECT_DIR  — 專案根目錄（包含 .bpr 的資料夾）
  BPR_FILE     — BCB6 專案檔名稱（例如 HT9045.bpr）
  [模式]        — 可選：clean / rebuild（預設為增量編譯）
```

``` 
build_bcb_safe.ps1 <PROJECT_DIR> <BPR_FILE> [clean|rebuild]

  PROJECT_DIR  — 專案根目錄（包含 .bpr 的資料夾）
  BPR_FILE     — BCB6 專案檔名稱（例如 HT9045.bpr）
  [模式]        — 可選：clean / rebuild（預設為增量編譯）
  行為          — 無論成功/失敗，結束時清理本次新開的 PowerShell 視窗
```

### 2.1 速度優先建議（實戰）

```powershell
# 日常開發：優先增量編譯（最快）
make -f HT9045.mak

# 僅在以下情境才使用完整重建（較慢）
# 1) 大量合併後 2) 疑似 obj 汙染 3) 編譯行為異常
make -f HT9045.mak -B

# 有改 .bpr 時務必先重產生 Makefile，避免漏編/漏連結
bpr2mak HT9045.bpr
make -f HT9045.mak
```

### 2.2 PowerShell 視窗管理（重要）

在大型建構或自動化流程中，若使用 `Start-Process powershell ...`，可能會產生多個 PowerShell 視窗且不自動關閉。

已提供可直接使用的同步腳本：`scripts\build_bcb_safe.ps1`。
詳細清理策略請見：`references/powershell-cleanup.md`。

### 2.3 立即關閉目前已開啟的 PowerShell 視窗

若需手動一次關閉目前已開啟的 PowerShell 視窗（保留當前執行中的 shell）：

```powershell
$selfPid = $PID
Get-Process powershell -ErrorAction SilentlyContinue |
  Where-Object { $_.Id -ne $selfPid -and $_.MainWindowHandle -ne 0 } |
  Stop-Process -Force -ErrorAction SilentlyContinue
```

### 2.4 強制模式（進階）

若要「更徹底」清理 PowerShell 程序（包含沒有視窗控制代碼的背景 powershell），可使用強制模式。

⚠ 風險：可能中斷其他正在跑的自動化工作或背景腳本，請只在確認可中斷時使用。

```powershell
$selfPid = $PID
Get-Process powershell -ErrorAction SilentlyContinue |
  Where-Object { $_.Id -ne $selfPid } |
  Stop-Process -Force -ErrorAction SilentlyContinue
```

更多範例（一般/強制模式切換與風險說明）請見：`references/powershell-cleanup.md`。

### 3. 手動步驟

```powershell
$BCB  = "D:\ProgramFiles\Borland\CBuilder6"
$env:BCB  = $BCB
$env:PATH = "$BCB\Bin;" + $env:PATH

cd "D:\Project\RDCode\HT9011UC_Code_V3.20"

# Step 1: 產生 Makefile
bpr2mak HT9045.bpr

# Step 2: 執行編譯
make -f HT9045.mak
```

## 建構流程

```
.bpr  ──bpr2mak──►  .mak  ──make──►  .obj (Obj/)  ──ilink32──►  .exe
```

## 注意事項

- **.bpr 內的路徑為硬編碼**：`PROJECT` (輸出 EXE) 與 `OBJFILES` 目錄須確保存在，否則 ilink32 會失敗。
- **Obj 目錄**：BCB6 預設將 `.obj` 輸出至 `.bpr` 中 `OBJFILES` 指定的路徑（通常為 `..\Obj\`）。若不存在請先手動建立。
- **PCH 目錄**：`.bpr` 中 `-H=` 旗標指定的 PCH 快取目錄（例如 `D:\HT9045\Obj\`）也必須事先建立，否則每個編譯單元都會產生 W8058 警告。
- **Precompiled Header**：若 `.bpr` 啟用 PCH，第一次編譯較慢；`clean` 後會重新產生。
- **只在必要時重建**：`-B` 會強制全檔重編，HT9045 大專案耗時明顯增加；平時優先增量編譯。
- **避免不必要的 `.h` 變更**：標頭檔異動會觸發大量重編，若僅調整實作，優先改 `.cpp`。
- **`.bpr` 變更後一定要先跑 `bpr2mak`**：新增/移除 `.cpp` 後若未重產生 `.mak`，容易出現 `Unresolved external`，造成反覆重編。
- **必須在專案目錄執行 `make`**：工作目錄錯誤可能導致 linker 回應檔 (`MAKE0000.@@@`) 讀取失敗，浪費整輪編譯時間。
- **同時間只跑一個建構流程**：並行 build 容易造成 PCH/obj I/O 競爭（例如 W8058），增加整體編譯時間與不穩定性。
- **PowerShell 視窗清理**：若建構流程會開新 PowerShell 視窗，請以 `try/finally` 在結束時關閉新開視窗（成功/失敗都執行），避免殘留視窗。
- **BCB_ROOT 覆寫**：腳本尊重 `%BCB_ROOT%` 環境變數，若不設則使用預設路徑。
- **Arm 變體重複符號警告**：HT9011UC 等大型專案中，多個 Arm 變體 `.obj` 之間可能產生 `Public symbol defined in both module` 的 linker 警告，這是預期行為（inline 函式重複定義），不影響連結結果。

## HT9011UC 專案首次建構前置步驟

```powershell
# 1. 建立所有必要目錄
New-Item -ItemType Directory -Force "D:\Project\RDCode\Obj"  # OBJFILES 目錄
New-Item -ItemType Directory -Force "D:\HT9045\Obj"          # PCH 快取目錄
New-Item -ItemType Directory -Force "D:\HT9045\EXE"          # EXE 輸出目錄

# 2. 執行建構
.\scripts\build_bcb.bat "D:\Project\RDCode\HT9011UC_Code_V3.20" "HT9045.bpr"
```

## 常見錯誤排查

| 錯誤訊息 | 原因 | 解法 |
|----------|------|------|
| `bpr2mak not found` | PATH 未包含 BCB Bin | 確認 BCB_ROOT 正確 |
| `Fatal: Unable to open file '*.obj'` | Obj 目錄不存在 | `mkdir D:\Project\RDCode\Obj` |
| `Fatal: Unable to open file 'MAKE0000.@@@'` | 非專案目錄執行 `make` 或暫存回應檔路徑失效 | 切換到 `.bpr/.mak` 所在目錄後重跑 |
| 建構後殘留多個 PowerShell 視窗 | 使用 `Start-Process powershell` 後未清理子程序 | 用 `try/finally` 記錄前後 PID，最後 `Stop-Process` 清掉新開視窗 |
| 一般模式清不乾淨 | 有無視窗的背景 powershell 程序 | 使用「強制模式」：關閉所有 powershell（排除當前 PID） |
| `Error: Unresolved external` | 缺少 .lib 或 .dll | 核對 .bpr 中 LIBRARIES 設定 |
| `Fatal: Unable to open file 'ADAMTCPBC.LIB'`（或其他 `LIBFILES` 裡的 lib） | `.gitignore:61` 排除 `*.lib`，**16 支廠商 import lib 不會跟著 git 到新機器**（20260930 新筆電 V912 首建踩到；編譯 294 單元全過，只在連結失敗） | 從 golden `HT9011UC_Code_V3.33.906.0_20260618` 的**相同相對路徑**複製（`ADAMTCPbc.lib`、`sqlite3.lib`、`Motor\`×6、`CCLink\`、`EJ1N\`、`SECSGEM\`、`CanBus\`、`Public\`×3、`EtherCAT\`，清單見 `.bpr` 的 `LIBFILES`）；檔案被 ignore，不會進 git |
| 建置後 git 顯示 `.bpr` 被改（`PATHCPP` 變成 `Motor;Motor;Motor…`） | `bpr2mak` 第一次讀專案時，把 `PATHCPP` 依編譯單元重算成「每單元一筆」並寫回（內容相同時不再寫） | 語意沒變，**不要 commit**；確認 diff 只有這一行後 `git restore -- <樹>/HT9045.bpr` |
| 剛裝完 BCB6 沒重開機，`bcc32` 報 `Unable to execute command 'ilink32.exe'` | PATH 還沒生效 | `build_bcb.bat` 會自己把 `%BCB_ROOT%\Bin` 加進 PATH，**不用重開機**；手動測試時自己補 PATH |
| `Out of memory` | PCH 過大 | 增加 Windows 虛擬記憶體 |

## Reference

- `references/powershell-cleanup.md`：PowerShell 視窗清理策略（一般模式/強制模式/風險）

<!-- preserved-content:end -->
