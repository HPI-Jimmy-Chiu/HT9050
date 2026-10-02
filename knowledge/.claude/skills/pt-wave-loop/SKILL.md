---
name: pt-wave-loop
description: HT9045 V906 純翻譯戰役（PT CAMPAIGN）的自動波次政策。定義最終目標、選標的規則、驗收 gate、停止條件、硬邊界，以及六個已付代價的陷阱。Use when：執行 /pt-wave、規劃下一個翻譯波次、判斷某個波次能不能自動做完、要引用完成度百分比。關鍵字：PT-W, PT CAMPAIGN, 純翻譯, 波次, census, 完成度, 驗收 gate, 失敗集合, archive extraction, absence claim
---

> **//Steven 20260915** — 本檔於 2026-09-15 更新。
> 當日完整變更紀錄：`D:\HT9045\CHANGES_20260915_Steven.md`


# PT 波次迴圈政策（HT9045 V906）

## 最終目標（可量測，不是口號）

**把 golden 的 C/C++ 全部翻成標準 C++ 並且編得起來。**

- 範圍 = golden `HT9045.bpr` 的 `<OBJFILES>`，289 個單元
- `.dfm` **資源檔**不手翻（走 `tools/dfm2rc`）；表單的 `.cpp` **要翻**
- 「編得起來」= Debug 與 Release 都 build exit 0，**且** ctest 失敗集合不超過
  `docs/PT_CAMPAIGN_PLAN.md` §7 那 6 個常駐項

**現況一律用腳本量，不引用任何記憶中的數字**：

```
cd HT9011UC_Cpp_V3.33.906.0 && python tools/census/census.py --detail
```

20260809 基準：非表單 73.3%（缺 89,973 行）／表單 4.6%（缺 249,761 行）／全案 43.2%。
引用任何百分比**必須同時講分母（表單／非表單／全部）與單位（code 行）**，否則兩個都對的
數字會看起來互相矛盾（這個錯誤犯過一次）。

## 本迴圈的範圍（使用者 20260809 決定）

**只做非表單：43 個「有鏡射檔但沒翻完」的檔，89,973 行。**
**到表單邊界就停下來回報**，不要自己決定 facade 策略——那是計畫書 §4 自己標記的
「整個戰役最大的單一未決項」，107 個表單單元會各自發明 widget 存取方式。

## 選標的政策

1. 跑 census，取 `mirrored but INCOMPLETE` 清單，**缺口大的優先**。
2. **三個例外要提前拉，因為它們擋住別的東西生效**：
   - `cinitial.cpp`（`InitialMotorParameter`，golden :3392）＋ `Motor/mymotor.cpp`
     （48 個 ACTIVE Gali_ stub）→ 解 Sim-motor 耦合，讓已交付的 4,694 行 Galil 驅動
     真的有呼叫者。
   - `SECSGEM/uHGemHT9045.cpp` → 它是 `AddSV`/`AddEC` 兩個 override 宣告的家，
     少了它 PT-W4 交付的 SV/EC 字典永遠是 gated 的自由函式。
3. 一波抓 1–7 個單元，**看行數不看檔數**：單波 golden 行數控制在 ~15k 以內
   （PT-W3 18 檔 23k 行過大，收斂花了整場）。

## 一個波次的七步

0. **`git status` 先跑，不是先讀 RESUME。** 這條規則四次撿到別人（或自己）留在樹上
   未 commit 的在製工作。
1. **census 選標的**（上面的政策）。
2. **翻譯 workflow**：agent **只准新增／附加自己那幾個鏡射檔**。
   不准碰 `CMakeLists.txt`、既有檔、`tests/`、vendor 標頭、V899 目錄。
   同一檔多個 agent 時**一律 append-only 並互相對帳**。
3. **主迴圈整併**（序列做，不委派）：CMakeLists 落點、stub 退役、過期 gate 前提複驗。
4. **驗收 gate —— 分兩級，先判斷這一波有沒有改行為**（20260810 使用者定案）。
   > **數字必須在最後一次整併之後量。** `set_source_files_properties` 的 `COMPILE_FLAGS`
   > 是無條件附加的，改它會同時影響 Debug——前一輪的數字就過期了。
   > 「build dir 是全新的」≠「量的是最終樹」。

   **4a. 零行為變更的波次**（例如「golden 原文塞進 `#if 0`、不碰任何活的碼」）：
   **用 preprocessed 比對 + 一次 Debug build，不跑全新 Debug+Release。**

   ```
   git show HEAD:<path> > /tmp/head.cpp
   FLAGS="-std=c++17 -E -DMN200DLL_EXPORTS -DDLLDIR_EX -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 \
          -I. -IMotor -IMotor/vendor -IEtherCAT/vendor -Ithird_party/sqlite3 -ISECSGEM"
   # 兩邊都跑 g++ $FLAGS，輸出剝掉 '^# ' line marker 再比
   ```

   **驗收線：每一行差異都必須是空行。** 那代表編譯器看到的碼完全相同——
   **這比測試套件更強**，因為它不是「測不出差異」而是「沒有差異」。
   然後補**唯一還沒排除的風險**：Release `-O3` 在變大的檔上 OOM
   （`uHGemHT9045_EC.cpp` 曾因此被釘 `-O1`）。
   **注意：跑 Debug build 不能覆蓋這個風險**——20260810 第一版政策寫成「跑一次 Debug build」，
   但風險在 Release，這是寫錯的。正確做法是**直接對那個檔單獨編一次 `-O3`**（約 1 分鐘）：
   ```
   C:/MinGW/bin/g++.exe -std=c++17 -O3 -DNDEBUG -c <上面同一組 -D/-I> <檔> -o tmp.o
   ```
   再跑一次 Debug build 確認沒有編譯退化（這一項是回歸檢查，不是 OOM 檢查）。
   約 5 分鐘取代 50 分鐘。**省下來的時間是拿來開下一波的，不是拿來閒置的。**

   坑：`git show` 輸出 LF、工作檔可能是 CRLF；比對前兩邊都要剝 `\r`，
   否則會回報「完全不相同」（這條害主迴圈誤判過兩次）。

   **4b. 有行為變更的波次**：**全新 build dir、Debug 與 Release 各一次**，
   失敗集合逐項相同且 ⊆ §7 那 6 個。這一級不得簡化。
5. **commit**：一顆 commit 一件事（見下面「行為變更」）。
6. **DEVLOG**：`docs/DEVLOG.md` 加一節 + 更新 🔖 RESUME（下一步要具體到檔名行號）。
7. **回報**：交付行數、兩組 ctest 數字、失敗集合、下一個標的。

## 建置環境：從零裝起來的實測步驟（20260914）

一台只有 clone 下來的樹、沒有任何工具鏈的機器，要跑 `build.bat` 需要兩樣：

**① MinGW.org GCC 6.3.0 必須在 `C:\MinGW`**（`build.bat:37` 寫死 `MINGW_BIN`）。
版本不可替換——它是唯一能重現 BCB6 x87 算術的編譯器。內網備份區沒有現成的，
從 MinGW.org 官方途徑裝：

```
mingw-get 0.6.2  (SourceForge, Installer/mingw-get/)  解壓到 C:\MinGW
C:\MinGW\bin\mingw-get.exe update
C:\MinGW\bin\mingw-get.exe install mingw32-gcc-g++ mingw32-make
C:\MinGW\bin\mingw-get.exe install msys-base        # build.bat:91 的 tee.exe
```

依賴（binutils 2.28、mingwrt 5.0.2、w32api、libstdc++、libatomic）由 mingw-get 自己解，
不要手動拼湊。裝完 `g++ --version` 應該是 `(MinGW.org GCC-6.3.0-1) 6.3.0`。
⚠ **最新的 mingw-get 是 0.6.2 不是 0.6.3**，URL 用 SourceForge RSS
（`/projects/mingw/rss?path=/Installer/mingw-get`）查，不要猜。

**② CMake 4.0.2**（開發機同版）：GitHub `Kitware/CMake` release 的
`cmake-4.0.2-windows-x86_64.zip`，解壓到 `C:\CMake`，把 `C:\CMake\bin` 加進
**使用者** PATH（用 `[Environment]::SetEnvironmentVariable(...,'User')`，
**不要用 `setx PATH`** —— 它有 1024 字元截斷會毀掉 PATH）。
專案宣告 `cmake_minimum_required(VERSION 3.10)`，在 CMake 4.0 的下限 3.5 之上，相容。

**驗證不要只看 `--version`**：跑一次專案真實原始碼的語法檢查——
`g++ -std=c++17 -fsyntax-only cMyDB.cpp` 應該無輸出（`CMakeLists.txt:1185` 的註解
自己這樣聲稱）。⚠ 別用 `#include <optional>` 當 smoke test：GCC 6.3 的 libstdc++
沒有它（GCC 7 才有），但**專案完全沒用到** `<optional>/<variant>/<string_view>/
<any>/<filesystem>`，用的是 C++17 的語言特性而非那批函式庫。

## ctest 基準是 5 個，不是 6 個

權威來源是 `tools/gateverdict.sh` 的 `RESIDENT`：

```
GA1_ReadGeneralIni  IniFiles  config_db  config_loaders  ini_helpers
```

`dfm2rc_idempotent` **已被移出基準**（`gateverdict.sh:30`：「目前實測為通過，故不列入；
若它再度失敗會以 EXTRA 出現，那正是我們要看到的」）。
`D:\HT9045\docs\SOURCE_AND_BUILD.md` 原本寫 6 個，20260914 已更正。

**20260914 新增實測**：`WB_DataLayer` 與 `WB_Tags` 在本機失敗，與常駐五項**同一類**
（`docs/PT_CAMPAIGN_PLAN.md §7`：測試直接讀活的 `D:/HT9045/system` 與 `config` 並
斷言某一台機器的值，而磁碟上是另一台的快照）：

```
LoadMachineConfig() 回報成功，但
  IniConfig.sMachineType = ""   sGPIBMachineID = ""   RMSTesterID = ""
  CosFunction non-zero bytes = 0 / 488
```

它們是 FW-TEMP 波次新增的測試，晚於 §7 那次查證，所以**從未被加進 `RESIDENT`**。
已用 A/B 對照排除「config 被覆蓋」這個懷疑：把 `config\LastSet.ini` 還原成覆蓋前的
版本重跑，兩個測試以完全相同的方式失敗。
**要不要把它們加進 `RESIDENT` 是人的決定**（§7 建議的正解是改讀版控的 fixture），
不要自己加。

---

## 行為變更的處理（使用者 20260809 決定）

**自動做，但單獨一顆 commit、單獨量。**

「行為變更」= 解閘（`#if 0` → live）、退役 ACTIVE stub、掛 driver、改預設值。
波次 N 做純翻譯＋整併，波次 N+1 做行為變更，各自跑完整 Debug+Release gate。
理由：這次退役 60 個 stub 讓 out-arm 路徑第一次真的執行、CosFunction 解閘讓 163 個
客戶設定檔生效——混在翻譯 commit 裡就無法回溯歸因。

## 不准自行停下（20260810 使用者定案）

**預設是「繼續」，不是「回報後等待」。** 這條是使用者明確要求的：
除非達到最終目標並明講，否則持續推進。

**規則一：回合結束前不准是閒著的。**
如果沒有任何背景工作在跑，**就不准結束回合** —— 要嘛開下一波，要嘛做主迴圈自己
能做的整併／量測／文件。`ScheduleWakeup` 只當「有東西在跑」時的保險，
**不當節奏器**。

> 為什麼要寫死：20260809-10 那場停下來的時間，**絕大多數不是被停止條件擋住的，
> 是自己排出來的空檔** —— commit 完成後習慣性「回報＋排 20-30 分鐘喚醒＋結束回合」，
> 而當下沒有任何東西在跑。停止條件全場只擋了一次。
> 待辦量從來不是問題（當時還有約 4,200 行隱藏缺口＋67,187 行 census 缺口＋6 個 task）。

**規則二：這些情況預先授權，同一回合內做完，不要停下等。**

| 情況 | 授權動作 |
|---|---|
| ctest 多出的失敗，根因是「測試期望值照鷹架校準」 | **同一回合內重新校準並繼續**，但必須附 golden 行號證據，並把失去覆蓋的項目寫進測試自己的 NOT COVERED 區 |
| 選標的、切波、分組、整併、stub 退役、fidelity 修正、量測、文件 | 全部自動 |
| 波次 agent 死掉（API / 額度） | 用 `resumeFromRunId` 重派；若是額度，改做主迴圈能做的事，不要空等 |

**規則三：真正要停的只有這些。**

| 條件 | 動作 |
|---|---|
| **安全關鍵行為變更**（解閘馬達煞車／安全門互鎖／把「未完成」變「完成」的 stub／動到真機動作） | **累積到佇列，繼續下一個非安全項目**。不停下、不等待，但**也不自己做**。等使用者在場一次過 |
| 到達表單邊界（非表單全部翻完） | 停、回報，等使用者定 facade 策略 |
| 運算額度耗盡 | 寫完 DEVLOG + RESUME 再停 |

> 安全界線的理由不是信心不足，是**量測會錯**：那一場主迴圈自己的量測錯了五次
> （census gate map 自我指涉、字串常數被當呼叫點、census 從不比本體大小、
> 兩次 CRLF/LF 弄壞比對）。純翻譯量錯只浪費時間；安全解閘量錯會動到機器。

## 冷啟動／中斷後恢復（20260810 加入）

**任何一次新的 invocation（額度重置後、當機後、cron 心跳觸發）都照這個順序做，不要憑記憶接續：**

1. `git log --oneline -5` —— 看真正 commit 到哪裡。
2. `git status --porcelain -- HT9011UC_Cpp_V3.33.906.0/` —— 樹上有沒有未 commit 的在製工作。
   有就先收完（驗證 → 驗收 → commit），**不要疊新波**。
3. 檢查有沒有背景 workflow / build / ctest 還在跑。**有就不要介入**，尤其不要開第二個
   波次去改同一批檔案（PT-W6b 四個 agent 共用一個檔就造成過誤判與整波作廢）。
4. 讀 `docs/DEVLOG.md` 檔尾的 🔖 RESUME，從那裡的「下一步」開始。
5. **不要重跑已完成的波次。** RESUME 與 git log 對不上時，相信 git log。

### 兩條續跑路徑，故意重複

| 機制 | 性質 | 失效方式 |
|---|---|---|
| `ScheduleWakeup` | **鏈**：每輪武裝下一輪 | **一個環節斷掉就整條死**（額度中斷、或忘記武裝） |
| `CronCreate` 心跳 | **時間表**：固定間隔觸發 | 漏掉一次不影響下一次；但 **session 結束就消失** |

所以**兩個都要有**：每輪照樣武裝 `ScheduleWakeup`（有東西在跑時當保險），
另外掛一個守衛式 cron 心跳（每 20 分鐘，先檢查有沒有在跑，有就不介入）。

**兩者都是 session-scoped。** session 一結束（關掉 client、機器重開）兩者都沒了，
沒有任何機制能跨 session 自動續跑——這一點不要對使用者含糊。

### 不准為了問問題而停下（20260810 使用者追加）

**使用者明確要求：不要問他怎麼做，撞到額度就等解除後自己繼續。**
所以「停下來問」不是一個可選動作。被卡住時的正確行為是：

1. **選安全預設值並繼續**，判準依序是：
   (a) 保持現行行為不變 → (b) 可逆 → (c) 讓樹維持編得起來。
2. **把選擇與被否決的替代方案寫進 commit 訊息或 DEVLOG**，讓使用者事後可以審。
   決策要可追溯，但不要卡住等他回答。
3. **安全關鍵項目的處置是「不做」，不是「停下來問」** ——
   累積到佇列、繼續下一個非安全項目。這兩件事常被混為一談。

> 20260810 的實例：PT-W7a 的整併撞上 50 個 Galil 碰撞（task #10，安全）。
> 當時我列了三個選項請使用者裁決 —— **那是錯的做法**。
> 正確做法是照判準自己選（繼續找抽出需求，零風險、可逆、不動安全叢集），
> 而那條路後來只花一次 `ld -Map` 就解決了：`SetGaliRate` 一個呼叫點。
> **如果當時停下來等回答，會白等一整段時間，而且答案根本不需要使用者。**

### 額度中斷時會發生什麼（實測，不是猜的）

| 東西 | 額度期間 | 解除後 |
|---|---|---|
| **背景 OS 進程**（`cmake --build`、`ctest`） | **照跑不停** —— 它們不經過 API | 結果已在磁碟，心跳讀 log 即可 |
| 子 agent（workflow） | 會死（20260809 實測「You've hit your session limit」） | 用 `resumeFromRunId` 回放，已完成的從 cache 取 |
| `ScheduleWakeup` 鏈 | 該輪失敗 → **鏈斷** | 不會自己回來 |
| **cron 心跳** | 那次觸發失敗 | **下一次照排程觸發** ← 這是真正的復原路徑 |

**所以額度中斷不需要任何人介入**：gate 會自己跑完，心跳會自己回來讀結果。
唯一的前提是 **session 沒有結束**（client 不能關、機器不能重開）——
cron 與喚醒都是 session-scoped，磁碟上沒有任何東西。

## 硬邊界（永不觸碰）

- `.dfm` 不手改（Delphi 格式，走 dfm2rc）
- `HT9011UC_Code_V3.33.899.0_*` 與其他版本目錄唯讀
- `Motor/vendor/`、`EtherCAT/vendor/` 唯讀（AGENTS.md 禁改清單）
- `system/`、`config/`、`CFG/`、`setup.inf` 預設只讀
- golden 樹（`HT9011UC_Code_V3.33.906.0_20260618`）唯讀
- **golden 是 Big5**：用 `encoding='cp950'` 讀、UTF-8 寫、**逐檔偵測並保持 EOL**
  （這棵樹是混合的：`cmydef.cpp`/`csystem_shims.cpp`/`acatchtray_shims.cpp`/
  `csystem_predicates.cpp` 是 CRLF，其餘 bare-LF；硬統一會造出上千行假 churn）
- 交付前檢查：UTF-8 可解、**零 U+FFFD**、EOL 未變

## 六個已付代價的陷阱（波次自檢必跑）

### 1. 「build 綠」證明不了接上了 — archive-extraction，五種形狀

static archive 成員**只有在解析某個仍未定義的符號時才會被抽出**。所以一個檔可以編譯、
進 archive、然後永遠不被連結，而 build 全綠。

1. **沒人引用** → `nm --undefined-only lib*.a`
2. **stub 先滿足需求**（最陰險，`nm --undefined-only` **看不到**）→ 用
   `nm --defined-only` 對兩個 object 取交集，問「我的新 object 定義的符號有沒有別人需要，
   而那個需要是不是已經被某個 stub 吃掉了」
3. **註冊在錯的 archive** → 會 configure、會編、`-fsyntax-only` 也過，然後靜默
4. **`static` 影子** → 內部連結所以乾淨，只在同 TU 開始 include 真宣告時才爆
5. **`tests/` 裡的 TU-local stand-in** → 測試安靜地量空函式

### 2. absence-claim 會在同一波內過期

「X 全樹沒有 port」的 gate 前提，會被兄弟 agent 之後落地的檔弄成假的。
GATE REGISTER 的 absence-claim 要**附量測指令與時間**，並在**收工時重跑**；
主迴圈整併時逐條複驗。「降級值剛好等價」不算沒事——缺陷是假前提本身。

### 3. 前提死掉不代表答案就是退役

兩次遇到：`MyMessageBox`（真的有 port，但在 ht9045_sm，而 ht9045_sm 連結 ht9045_motor，
為一個 bool 反轉相依不值得）、`WriteLastDataFile`（可連結，但它 CreateFile 寫死的
`D:\HT9045\system\lastdata.dat`，退役會讓 ctest 覆蓋機台真實存檔）。
**要重新問一次「為什麼它該是 gated」，把真答案寫進去。**

### 4. static-init ctor 不可碰 §8 那 18 個 NULL 全域

翻譯單元用 `X *x = new X();` 慣例時 ctor 在 `main()` 前跑。facade 的 ctor 只塞欄位所以
安全，**翻譯單元的 ctor 不是**：`fLaserSensor` 的 ctor 呼叫 `InitLaserEdtList()`
（17 行全是 `elLaser->Add(...)`，而 `elLaser` 是 golden `main.cpp:1483` 才 new 的）
→ **134 個 ctest 有 88 個 SEGFAULT**。stand-in 也救不了（跨 TU 動態初始化無順序保證）。
處置：在呼叫點加 `if(指標)` 並留 GA-3 hand-off。

### 5. 同名 class 兩個標頭 — 「編過」和「連上」都不是證據

`TMyKitSuck`（`mykitsuck.h:274` vs `aHotPlateSubstrate.h:365`，佈局不同）、
`TInLaserCheck`（`aHotPlateSubstrate.h:836` vs `OmronLaser/LaserSensorInArm.h:34`）。
選錯標頭會**乾乾淨淨地連起來，然後每個欄位讀錯偏移**。
include 某個標頭拿全域物件時，要確認**它實際連到的那個定義在哪個 .cpp**、佈局是不是同一個。

### 6. facade 缺成員時，先問「那個狀態在新架構下由誰持有」

**編譯器只會告訴你「符號不存在」。它不會告訴你那是不是該補的東西。**

20260915 ST 戰役實例（使用者指出，我自己沒接起來）：
golden 大量用 `fXxx->fShow`（VCL 的「這個表單現在顯示著嗎」）當**例外條件** ——
「這個畫面開著的時候不要卡權限 / 不做 run check / 不檢查流道」。
移植樹的 facade 有些有 `fShow` 有些沒有，我一律當成「缺一個欄位」處理，
還把「補 `fShuttleMove->fShow`」寫進待辦。

**那個框法是錯的。** 產品方向是 **UI = web、底層 = C++**（20260812 定案），
所以「哪一頁開著」這個狀態**不在 C++ 這一側，在瀏覽器**。
補一個 `bool fShow` 進 facade 會得到一個**永遠讀到 false 的樁** ——
等於「那些畫面永遠沒開」，所有例外都不成立，每一個都是**靜默的行為偏離**。
`forms/fShuttleMove.h:35-40` 自己就警告過：
「Adding it would be inventing an offline default (i.e. a silent branch selection)」。

規模：ST 戰役實測有 **10 個 gate** 卡在 `fShow`，佔還債清單一大半。

**所以缺 facade 成員時，先問這一句：**

> 這個成員在 golden 裡代表什麼**狀態**？那個狀態在新架構下**由誰持有**？

| 答案 | 處置 |
|---|---|
| C++ 仍持有 | 補成員（這才是原本的預設處置） |
| **web / 別的行程持有** | **不要補樁。閘著，並把它升級成設計題交給使用者** |
| 沒有人持有 | 重新設計，不是翻譯題 |

⚠ **補樁比閘著更糟**：閘著是「這段不執行」，補樁是「這段執行了但判斷永遠錯」。
前者在報告裡看得見，後者看不見。

這一條不只 ST 戰役 —— **FW（dfm→web）整個戰役會一直撞到**，
因為它的本質就是把 VCL 的 UI 狀態搬到瀏覽器。

### 附帶：Debug 綠不代表 Release 綠

`uHGemHT9045_EC.cpp`（~1,817 statement 擠一個函式）在 -O3/-O2 讓 32-bit cc1plus OOM，
-O1 才過（單獨編也 OOM，不是 -j 壓力）。計畫書 §5.3 要求兩種建法都量就是為了這一類。

### ★ 附帶：這棵樹的遞迴 grep 不只是慢 —— **它會回報假的「查無」**

`grep -rn` / `find` / `git ls-files | xargs grep` 對樹根跑**一定逾時**（實測 10 次）。
**20260915 進一步量測，結果比「慢」嚴重得多：**

```
grep -rn --exclude-dir={.svn,.git,build,Obj912,Out912,__pycache__,node_modules,rc_out,ir_out} \
     "ComputeCheckOLPErrorHasErr" .        # 這個符號確實存在（MainCalcCore.h:404）
-> 耗時 111 秒、命中 0 筆、exit 0
```

**加了排除清單也沒用**（這棵樹光是體積就撐爆它），而且最危險的是：
逾時被殺掉之後，管線末端的 `wc -l` / `head` 照樣 **exit 0**，
整條指令看起來像**乾乾淨淨地「這個符號不存在」**。

> **所以「我 grep 過了，沒有」這句話，在這棵樹上不是證據。**
> 它跟「逾時被砍掉」長得一模一樣。
> 這也是為什麼 gate 註解裡的 absence-claim 必須附**量測指令**（見陷阱 #2）——
> 而那個指令不可以是遞迴 grep。

**唯一可靠的做法：**

| 要做的事 | 用什麼 |
|---|---|
| 全樹找符號 / 字串 | **Grep 工具（ripgrep）** —— `glob` 過濾、`head_limit` 限量 |
| 已知檔案內找 | `grep -n PATTERN <明確路徑>`（不加 `-r`） |
| 「這個符號到底連進去沒有」 | **`nm`**，不是 grep |
| 「這個相依存不存在、型別對不對」 | **寫一支編譯探針 `-fsyntax-only`**，不是 grep |

⚠ Bash 環境**沒有 `rg`**（20260915 實測 PATH 上找不到，也沒有可用的隨附二進位），
所以「改用 rg」不是一個選項 —— **只能用 Grep 工具**。

### 附帶：`nm` 輸出是 CRLF

正則用 `$` 搭 `re.M` **不會**匹配（`$` 在 `\n` 前，`\r` 留下），會得到「0 undefined」的
假答案。要 splitlines + strip。參考 `scratchpad/link_closure.py` 的寫法。

## agent 的論證比程式碼更常錯

一次稽核 12 條 findings 有 11 條是引用造假或 off-by-N，不是邏輯錯。所以：
- 引用行號類的 finding **要逐條開 golden 對字面**再改
- 「絕對宣稱」（全樹沒有／零消費者）預設為偽，自己重跑 grep
- 稽核 agent **唯讀**；BLOCKING 那批主迴圈自己複驗

## 相關文件

- `HT9011UC_Cpp_V3.33.906.0/docs/PT_CAMPAIGN_PLAN.md` — §2 量法、§3 現況、§4 波次、§5 驗收、§6 作業規則、§7 那 6 個常駐失敗、§8 那 18 個 NULL 全域
- `HT9011UC_Cpp_V3.33.906.0/docs/DEVLOG.md` — 每波記錄，最新 🔖 RESUME 在檔尾
- `HT9011UC_Cpp_V3.33.906.0/tools/census/census.py` — 唯一權威的完成度量法
