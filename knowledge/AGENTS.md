# AGENTS.md — HT9045 自動化測試機台控制軟體

## GPT / Codex 專案入口（20261006）

本檔是 Codex 的專案入口。共用知識沿用本 repo 的 `CLAUDE.md`、`.claude/agents/` 與 `.claude/skills/`；不建立第二套技能鏡像，也不重建已退役的 `.agents/skills/`。

### 開工與規則讀取

- 先確認 repo、目前分支、`git status --short` 與 remote；fetch 後比較 `HEAD` 與 `origin/main`。有未提交改動時保留原狀，不自動 stash、reset 或換掉使用者分支；需要最新基準時用獨立工作樹，或用 `git show origin/main:<路徑>` 讀正本。
- 讀最新 `CLAUDE.md` 的協作方式、Agent 分流與本次主題相關規則。其 `@AGENTS.md` 是 Claude 的 import 寫法；讀到時本檔已載入，不要循環重讀。
- 修改前讀 `.claude/ops/write-boundary-policy.json` 並遵守唯讀與備份邊界。Claude 的 `.claude/settings.json` hooks 不代表已在 Codex 執行，不得宣稱有自動攔截保護。
- 使用者當前指示優先於 repo 的歷史流程；遇到矛盾先確認版本、裁決日期與適用角色，勿把 Jimmy 筆電／ST01／ST02 的權責套給另一角色。
- 對使用者的說明及文件用繁體中文；專案知識、裁決、驗證結果記在 repo 的既有文件位置，避免只存在個人記憶。

### 按工作主題讀取

下列 agent 檔在 Codex 中先作為工作規則與技能索引讀取；只有本次任務需要且已獲授權時才委派子代理，不把 Claude 的工具名稱當成 Codex 已安裝的工具。

| 工作主題 | 規則入口（相對本 repo） |
| --- | --- |
| HT9045 程式與版本判斷 | `.claude/agents/ht9045-agent.md` |
| V912 量產維護，Big5 / BCB6 / pre-C++11 | `.claude/agents/ht9045-v912.md` |
| V899 客戶機台對照，唯讀 | `.claude/agents/ht9045-v899.md` |
| V906 C++ 移植與 Web HMI，UTF-8 / C++17 / CMake | `.claude/agents/ht9045-v906.md` |
| HT9050 硬體與機台事實，先查證再改程式 | `.claude/agents/ht9050-agent.md` |
| 交接、代跑、todo / done、協作紀錄 | `.claude/agents/co-work-agent.md` |
| 客訴與週報 | `.claude/agents/case-coordinator.md`、`.claude/agents/weekly-report.md` |
| RD5 入口網站，repo `9050motionview` | 該 repo 的 `AGENTS.md`、`.claude/agents/rd5-portal-agent.md` |

- 技能權威仍是 `.claude/skills/<name>/SKILL.md`。先讀所選 agent 的技能表，再讀適用技能及必要 references；清冊 `docs/ops/registers/skill-registry.md` 用來找入口，實際檔案與最新裁決才是依據。
- 若 session 的技能清單沒有列出 repo 技能，直接讀上述 `SKILL.md` 並遵循適用流程；不要假設 `.claude/skills/` 自動被 Codex 註冊。Claude 的 `Skill`、`Task`、斜線指令與 `/loop` 不是可直接照抄的 Codex API；先讀對應檔案，再用本 session 可用工具執行已授權的工作。
- 本文下方 BCB6 / Big5 規則適用 BCB6 樹；V906 移植樹依其 agent 規則使用 C++17 / UTF-8。V906 建置先讀 `cpp_build`，BCB6 建置先讀 `bcb_build`；文件入口修改只需檢查差異與引用，不啟動機台或執行期設定測試。
- HT9050 執行期測試遵守下方機台快照同步、備份與還原流程。模擬／真機依建置期 `SOFT_SIMULTE`，不可重新加入已退場的 `--dry` 啟動參數；硬體操作須符合當次授權與機台現場條件。
- 推送依使用者授權與對應 repo 流程，只提交本次檔案。跨同事整合走 MR；RD5 入口網站走分支與 MR 給 ST02-M。HT9045 main 的機台包交付規則仍依最新 `CLAUDE.md`，不可將私有 repo 歷史推到公開 GitHub。

Codex 入口方式參考：[OpenAI 官方 AGENTS.md 說明](https://developers.openai.com/codex/guides/agents-md)。

## 專案概述

HT9045 是一套用於半導體 / 電子元件自動化測試的 **Handler 控制軟體**，負責管理自動化取放臂、多站點測試、溫度控制、Shuttle 機構、Socket 測試及生產追蹤等功能。

- **語言**：C++（Borland C++ Builder 6.0 / VCL 框架）
- **輸出**：HT9045.exe（Windows 桌面應用程式）
- **專案檔**：`*.bpr`（BCB 專案檔）
- **UI 框架**：Delphi VCL（`.dfm` 表單檔案）
- **資料庫**：SQLite3（運行時資料）、Access MDB（歷史資料）
- **通訊協定**：SECS/GEM、TCP/IP、CAN Bus、EtherCAT、CC-Link、Modbus TCP、GPIB、RS-232

## 目錄結構

```
HT9045/
├── HT9011UC_Code_V3.33.912.0_20260908_Jimmy/            ← 目前預設 AI 修改目標版本原始碼（公司最新出貨版）
├── HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422/   ← 前一個量產維護版；20260909 起唯讀（只讀不改，修正一律出 V912）
│   ├── ARMS/          # 機械臂控制（取放臂 in/out/index/tray）
│   ├── ATC/           # ATC 系統主控
│   ├── Motor/         # 馬達控制（Hontech、MN200、SMC、Syntek、EtherCAT）
│   ├── TempCtrl/      # 溫控模組（DT4848、KT4H、TMC401、UT100、WT404）
│   ├── Automation/    # 自動化流程、AGV/AMR、MES 整合
│   ├── SECSGEM/       # SECS/GEM 半導體設備通訊
│   ├── CanBus/        # CAN Bus 通訊
│   ├── EtherCAT/      # EtherCAT 通訊
│   ├── CCLink/        # CC-Link 工業網路
│   ├── MyPLC/         # PLC Modbus TCP 介面
│   ├── Public/        # 共用工具（cJSON、MD5、記憶體管理）
│   ├── BarCode/       # 條碼讀取系統
│   ├── Interface/     # 各類介面 API
│   ├── HotPlate/      # 熱板控制
│   ├── Monitor/       # 系統監控 UI
│   └── ...            # 其他模組（400+ 原始檔）
├── backup/            # 歷史版本備份
├── system/            # 運行時設定檔（.ini, .dat, .DB, .csv）
├── config/            # 應用程式設定
├── CFG/               # 額外設定
├── EXE/               # 編譯後執行檔與 DLL
├── hklib/             # 硬體驅動程式庫
├── bmp/               # UI 圖片資源
├── MDB/               # Access 資料庫檔案
├── SECS/              # SECS/GEM 相關設定
├── IniData/           # INI 資料檔
└── Error/             # 錯誤紀錄
```

## 命名慣例

| 類別 | 前綴 / 規則 | 範例 |
|------|------------|------|
| 類別（Class） | `c` 前綴 | `cKernel`, `cSocket`, `cMyDB`, `cConfiguration` |
| 表單（Form） | `f` 或 `u` 前綴 | `fMain`, `fLotInfo`, `uTeach`, `uMotorTest` |
| 機械臂模組 | `a` 前綴 + 機型 + 站點配置 | `ainarm9045_1x1`, `aoutarm9045_2x4` |
| 測試模組 | `atester` 前綴 | `atester9045` |
| 全域定義 | `MachineDefine.h`, `MachineType.h`, `cmydef.h` , `cprod.h`,`cpublic.h`| — |

## 版本管理慣例

- 版本格式：`V{主版本}.{次版本}.{修訂號}.{小修訂}_{日期}`
- 開發者分支以 `_{開發者名稱}` 後綴命名
- 範例：`HT9011UC_Code_V3.33.897.0_20260304_RogerYang`
- 版本歷史記錄在 `Ver.txt`
- 目前預設 AI 修改目標版本：`HT9011UC_Code_V3.33.912.0_20260908_Jimmy`（20260909 起，公司 20260908 出貨版）
- 前一個量產維護版 `HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422`
  **已於 20260909 唯讀**（使用者裁決；已加入 `readonlyRoots`，hook 實測回 deny）。
  未結案的週報 row 3/6/8/9 一律改由 V912 判斷與交付 —— 可行性已查證：四件涉及的機種
  （Type_HT9046LS / Type_HT9045 / Type_HT9046A）與程式區域在 V912 都有對應。
  **唯讀只擋「改」不擋「讀」**：分析跑 899.x 的機台時照樣要開 V899 對照，那是機台
  實際在跑的碼；只是修正與交付一律出 V912。
  若要重開：從 `readonlyRoots` 移除**並**加回 `allowedWriteRoots`，兩份清單都要動
  （`confirmOutsideAllowed:false` 會讓「兩份都沒列到」變成靜默放行）。
- 除上述兩棵之外，其他版本原始碼目錄（`HT9011UC_Code_*`、`HT9046LS_Code_*`、
  `HT9011UC_Cpp_*_noBuild`、`HT9011UC_CSharp_*`）一律唯讀，除非使用者明確指定切換目標版本。
  權威清單在 `.claude/ops/write-boundary-policy.json`，不在本文（1005 從 `.github/ops` 搬過來）。

## 建置方式

- **IDE**：Borland C++ Builder 6.0
- **專案檔**：`HT9045.bpr`（位於原始碼目錄根層）
- **輸出目標**：`HT9045.exe`
- **外部函式庫**：`hklib/` 目錄中的 `.lib` 檔，以及 `EXE/` 目錄中的 `.dll` 檔
- **關鍵依賴**：
  - `CSMC.lib`（伺服馬達控制）
  - `DMC32.dll`（Galil 運動控制器）
  - `MN200DLL.dll`（三菱）
  - `sqlite3.lib`（資料庫）
  - `KeyProDLL.dll`（授權驗證）

## 設定檔體系

| 檔案 | 用途 |
|------|------|
| `setup.inf` | 機台基本設定（測試板/Socket 型號） |
| `CurrentSetupData.txt` | 目前啟用的設定組態 |
| `system/Gerneral.ini` | 通用系統參數 |
| `system/ATC.ini` | ATC 主控設定 |
| `system/*.dat` | 機台參數、偏移量、教導資料 |
| `system/*.DB` | SQLite 資料庫（感測器、馬達、缸體、料盤） |
| `system/*.csv` | IO 對應表、馬達表、語系檔 |

> **機台專屬設定檔的分享方式**（AI(W906-MACHINES) 20260923，Jimmy 裁決）：
> `system/`、`config/` 是機台執行期目錄，每台內容不同，**不進版控**（`.gitignore:115`）。
> 某台機的專屬檔要保存歷史或分享給別人時，放在 **`machines/<機台代號>/`**
> （例：`machines/HT9050/IO_Table.csv`），**不是**放進 `system/`。
> 規則、安裝與更新步驟見 `machines/README.md` 與各機台資料夾的 `README.md`。
> 從 `machines/` 複製到 `system/` 屬於改動執行期設定，要先得到使用者同意並先備份。

## ★ 懷疑是「HT9050 機台設定或工單」的問題：先比對機台快照，再下結論（Jimmy 20261005，所有人、所有 AI）

> AI(W906-MACHSYNC) 20261005，`HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261005.md` 第 4 條。起因：同事的 AI 回報「機台參數設定有問題」，
> 實際確認是**它電腦上的機台參數跟機台端不一樣**。Jimmy 原話：「如果發現是機台設定或工單問題，優先上github確認工單和機台設定是否同步，
> 沒有同步就先同步再檢查，如果已經同步，那就是真問題的機率高，可提出討論」。
>
> ⚠ **每一次測試前都先做，不用問**（Jimmy 1005 19:2x，在 NB2 對話裡：「一定要更新」「未來其他同事要測試，也不要問，必須更新才能接著測試」；RULINGS_20261005 第 6 條）：工作樹先更新到 GitLab main 最新版，再跑下面第 1～2 步；沒做就不准接著測。

1. **先比對**：`python tools/machine_sync/machine_sync.py check`
   （先比 GitHub `HT9050` 分支 `machine/integ-ioweb`——機台約每 30 分鐘把 `machine_params/`＋`workorder/` 推上去；
   連不到 GitHub 自動改比 GitLab main 的 `machines/HT9050/snapshot/`，並印出那份是機台幾點拍的）。
2. **`NOT SYNCED`（exit 1）⇒ 先同步再看**：`machine_sync.py apply --yes`（自動先備份、複製、逐檔比 MD5）。
   同步前量到的現象**不能**當成機台問題回報。這條規則就是 Jimmy 對「開發機／模擬機同步 HT9050 工作檔」的同意（上面「先得到使用者同意」那句在這個情況已經給了）；**別台真機台一律不裝**。
3. **`SYNCED`（exit 0）⇒ 真問題的機率高，提出討論**：回報時附上工具印的 `machine snapshot` 兩行（來源、機台拍照時間）。
4. 做完驗證要還原：`machine_sync.py restore <apply 印的備份資料夾>`（還原、比 MD5、刪備份）。

細節：`tools/machine_sync/README.md`；放回位置與機種身分（`D:\GPIB9045\system\general.ini` 的 `[Version] Model`）：`machines/HT9050/snapshot/SNAPSHOT_SOURCE.md`。
GitLab 那份照 RULINGS_20261002 第 21 條是**單向**的（機台 → GitHub → 筆電鏡像 → GitLab）：要改機台設定，請 EastSun 在機台上改、推上來，不要改 GitLab 那份。

## 機台 Layout

機台物理配置示意圖請參考：`IMG/BMP/InOutArmOffset_6.bmp`

> 其他 Layout 變體圖位於 `IMG/BMP/` 目錄下：
> - `InOutArmOffset.bmp` — 標準版 Layout
> - `InOutArmOffset_6.bmp` — 6-site Layout（目前使用）
> - `InOutArmOffset_AU.bmp` — AU 版 Layout

### 機台各模組組成

| 模組 | 說明 | 馬達 | 真空產生器 |
|------|------|------|-----------|
| 入料手臂（InArm） | 從 Loader 取 IC 放至入料飛梭 | `MInArmX`, `MInArmY`, `MInArmPitch`, `InArmZIndex` | `InArmSuck` |
| 出料手臂（OutArm） | 從出料飛梭取 IC 放至出料區 | `MOutArmX`, `MOutArmY`, `MOutArmPitch`, `OutArmZIndex` | `OutArmSuck` |
| 入料飛梭（InShuttle） | 將 IC 從入料手臂送至 Index 測試站 | `MInShuttle1`, `MInShuttle2` | `FLCarryKit`, `BLCarryKit` |
| 下壓模組（Index） | 將 IC 壓入 Socket 進行測試 | `MTestY1`, `MTestZ1`, `MTestY2`, `MTestZ2` | `FTestSuck`, `BTestSuck` |
| 出料飛梭（OutShuttle） | 將測試完成的 IC 從 Index 送至出料手臂 | `MOutShuttle1`, `MOutShuttle2` | `FRCarryKit`, `BRCarryKit` |
| Tray 搬運手臂（TrayArm） | 搬移料盤（Loader / Empty / Color / Auto） | `MTrayX` | `CatchTraySuck` |
| 入料區（Loader） | 供料料盤堆疊區 | — | — |
| 空盤區（Empty / Color） | 空料盤回收區（含顏色分類盤） | — | — |
| 自動出料區（Auto） | 依測試結果自動分類出料（`Auto1`～`Auto6`） | — | — |
| 手動出料區（Fix） | 手動指定分類出料（`Fix1`～`Fix6`） | — | — |
| 加熱盤（HotPlate） | 測試前對 IC 進行預熱（`HotPlate1`, `HotPlate2`） | — | — |
| Auto Clean 區 | 自動清潔 Socket 接點區域 | — | — |
| 入料旋轉站 | 入料側 IC 方向旋轉校正站 | — | — |
| 出料旋轉站 | 出料側 IC 方向旋轉校正站 | — | — |

## 機台硬體配置對照

| 用途 | 設定檔 | 讀取來源 |
|------|--------|----------|
| 機台硬體配置 | `system/Gerneral.ini` | `cinitial.cpp` |
| 馬達配置 | `system/Mot_Table.csv` | `cinitial.cpp` |
| IO 配置 | `system/IO_Table.csv` | `cinitial.cpp` |

- 以上三個設定檔均由 `cinitial.cpp` 讀取，修改設定檔前請先閱讀該檔案以確認欄位格式與順序。
- 機台全域變數定義集中於以下標頭檔：`MachineType.h`、`cprod.h`、`cpublic.h`、`cmydef.h`

## 硬體控制類別

| 標頭檔 | 說明 |
|--------|------|
| `myio.h` | IO 基本控制（底層 port 讀寫） |
| `MyLaneIo.h` | IO 進階控制類別（`TLaneIO`），支援多 Ring/IP/Port 架構 |
| `mysensor.h` | Sensor 類別（`TMySensor`），對應感測器輸入點 |
| `myswitch.h` | Switch 類別（`TMySwitch`），對應輸出開關點 |
| `mycylin.h` | 氣缸類別（`TMyCylinder`），每個氣缸含 2 個 switch（輸出）與 2 個 sensor（到位感測） |
| `MyKitSuck.h` | 真空產生器類別（`TMySucker`），每個真空產生器含 2 個 switch（On/Off）與 1 個 sensor（真空感測） |
| `Motor/mymotor.h` | 馬達類別，封裝各廠牌馬達控制介面 |

## 多執行緒架構

系統使用多執行緒進行即時控制：
- `uHeaterThread` — 溫控執行緒
- `uShuttleThread` — Shuttle 移動執行緒
- `ScanBtnThread` — 按鈕掃描執行緒
- `HThreadCtrl/` — 自訂執行緒控制模組

## 編碼注意事項

- **原始碼編碼**：原始碼預設使用 BIG5，新增或修改檔案時請保持 BIG5 編碼，避免中文註解或字串亂碼
- **SVN 中繼資料保護**：原始碼目錄下所有 `.svn` 資料夾一律不進行搜尋與修改
- **報告檔名規範**：報告生成時，檔名開頭須加上 `RD5軟體` 前綴，且檔案名稱格式須包含 `YYYYMMDD_HHMMSS`
- **BCB 6.0 限制**：不支援 C++11 以上語法，使用 VCL 的 `AnsiString` 而非 `std::string`
- **表單檔案**：`.dfm` 為 Delphi 格式，不可手動亂改格式
- **硬體相依性**：許多模組直接操作硬體 I/O，修改前須確認實體機台配置
- **多站點配置**：機械臂模組依站點配置（1x1, 1x2, 2x4, 等）有不同實作，修改時注意影響範圍
- **設定檔格式**：大量使用 INI / DAT / CSV 格式，欄位順序與格式敏感，修改需謹慎
- **SECS/GEM 通訊**：遵循 SEMI 標準，變數命名和訊息格式需符合規範

## 驅動程式與定義檔（禁止修改）

以下檔案視為第三方或硬體驅動介面檔，且同時為專案的「定義檔」：幾乎所有的全域變數、資料結構與硬體介面定義都儲存在這些檔案中，請視為唯讀（僅可讀取與引用），不得修改。

- `CCLink/Mdfunc.h`
- `EtherCAT/AdvMotApi.h`
- `EtherCAT/AdvMotDev.h`
- `EtherCAT/AdvMotDrv.h`
- `EtherCAT/AdvMotErr.h`
- `EtherCAT/AdvMotPropID.h`
- `Motor/CMNETError.h`
- `Motor/CMNet.h`
- `Motor/CSmc.h`
- `Motor/CSmcdef.h`
- `Motor/DMCCOM.H`
- `Motor/dmcdrc.h`
- `Motor/dmcdrco.h`
- `Motor/DMCMLIB.H`
- `Motor/Mc88x1p_DLL.h`
- `Motor/mn200.h`
- `Motor/PCI_L112.h`
- `Motor/PCI_L122_Err.h`
- `Motor/PCI_L122.H`
- `Motor/PCI_L132.H`
- `MR/DMT.h`
- `Public/ADSMOD.h`
- `Public/HTKeyPro.h`
- `ADAMTCP.h`
- `sqlite3.h`

## 技能自動載入規則

完整的技能觸發對照表由 workspace config 的 `<skills>` 區段管理，不在此重複。

- 技能檔案位置：`.claude/skills/<name>/SKILL.md`
  （Steven 20260918：原本寫 `.agents/skills/`，該目錄已退場 —— 理由見 `CLAUDE.md` 的同日註）
- 技能清冊與盤點：`docs/ops/registers/skill-registry.md`
- 技能數量依目前版本的 `.claude/skills/*/SKILL.md` 實檔為準，不在入口維護固定數字；涵蓋機構流程、通訊、溫控、Web HMI、建置、協作與資料分析等領域。
