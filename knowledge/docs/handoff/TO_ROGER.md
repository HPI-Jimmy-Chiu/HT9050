# 給 RogerYang：工作卡與回答（Jimmy／筆電 → RogerYang）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** 回覆請寫在你的分支 `v906/roger-handoff` 的 `docs/handoff/FROM_ROGER.md`（你 1006 19:41 開的）。
> 兩個檔各自只有一個寫者，git 合併永遠不會衝突。
> 開始：20261006 21:0x（RogerYang：「需要派工或回答我的時候，請建 docs/handoff/TO_ROGER.md」）。筆電的夜間迴圈每 20 分鐘讀一次 `v906/roger-handoff`；急事打電話給 Jimmy。

## 0. 規則

1. **開工前 `git fetch origin`**，用 `git show origin/main:<路徑>` 讀這個檔、`CLAUDE.md`、`AGENTS.md`（主資料夾 `D:\HT9045` 可能落後 main）。
2. **先認領再做**：FROM_ROGER §1 寫一行（卡號、會動的檔），先推再開工（你已經這樣做了）。做完寫 §2，問題寫 §3；筆電的回答寫在這個檔的 §4。
3. **程式走分支＋MR**：`v906/roger-<主題>`，MR 開到 main。筆電整批跑出貨＋模擬兩組態 gate，綠了才合，合完推 GitHub 機台更新包。新測試請附反向驗證（故意弄壞一處，測試要紅）。
4. 照 `HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261001.md` 第 0 條：golden 會做、移植樹是空的或被閘住的，照 golden 補齊接上；翻譯以忠實優先。golden＝`HT9011UC_Code_V3.33.906.0_20260618`（Big5）。
5. 建置一律走 `build.bat`（先讀 `.claude/skills/cpp_build/SKILL.md`）。ctest 會寫真實檔：跑之前 `python tools/realfile_guard.py snap <名稱>`，跑完 `check <名稱>`，沒變再 `drop <名稱>`。
6. 給人看的文字用繁體中文；行號引用寫「檔名:行號」並註明哪一棵樹（你的〔移植〕〔g906〕〔V912〕寫法很好，照用）。
7. 心跳（人工 session 不強制）：`python tools/laptop_ops/heartbeat.py --who roger --doing "<在做什麼>" --push`（推到 `v906/roger-heartbeat`）。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第 81 批 gate 跑中） | 機台 cpp 0240～0245（`MyPLC/MyPLC_IO_Modbus.cpp`、`Motor/myGALILmotor.cpp`、`BinDisplay/BinDispBringUp_St02.cpp`、`Gerneral.ini` 讀的 SafePlcModel）、Jerry !266、NB2-1 !272、Ifor01 !273、St02 !274／!277、Frank01 !275 |
| 筆電（第 82 批，第 81 批跑完接著跑） | Ifor01 !276、St02 !278／!279（`TempCtrl/TriTemp.cpp`）、機台 web 0125（`web/js/pci1203/*`、`web/JSON/Alarm*`） |
| St02（認領中） | `forms/fSCKART.h`／`.cpp`、`TesterComm/Handler/HandlerGpibMsg.cpp`（ART lot 狀態 G2／G4／G6） |

## 2. 須知（不用回，知道就好）

- **POOL-2（`#if 0`）的候選清單誤判很多**：Ifor01 1006 15:1x 抽查，他看的 45 個只有 3 個真的能解；你 20:4x 又抓到一類（整支檔沒連進 wb_serve）。
  認領前先查 `docs/handoff/IF0_CENSUS_20261006_linked.tsv`（那支檔兩個組態都要是 `linked`），再逐一對 golden 0618；普查上標 ⛔ 的不要照單開。
- 交接檔裡問到 golden 你署名的段落，我們會直接在這裡問你原意（你 §0 說可以）。上機量測仍照各自的流程（機台事實問 EastSun，動作流程照 RULINGS_20261006 第 20 條先 Steven 再 Frank）。

## 3. 工作卡（不急；接之前先在 FROM_ROGER §1 認領）

（目前沒有指定給你的卡。你說要自己從 POOL-2 挑——§4 21:0x 列了四支已確認連進 wb_serve 的 LOW 檔可以參考。）

## 4. 回答你的問題

| 時間 | 主題 | 內容 |
|---|---|---|
| 20261006 21:0x | ✅ **W-62 B1 原點極性：收到，謝謝**——已轉給 St01（TO_STEVEN §4 21:0x） | 你說明的 golden 設計（極性只有一層、交給卡片：`InitMotor` 依 Mot_Table SensorType 寫 `CFG_AxOrgLogic`，程式端反相 20250425 拿掉；9046AU 泓格步進六軸實機驗證過）解開了 B1.3「卡片會不會照 OrgLogic 翻」的疑問，記下來了。⚠ **HT9050 這邊 10/05 已經有答案，M3 不用再量**：`docs/handoff/ST01_W62_ANSWERS_20261005.md` B 節開頭的 ⛔ 更正（NB2-1 U20，1005 06:3x）——機台 `machine_log/oplog_20261004.txt` 每次開機 InitMotor 對 18 軸寫 `CFG_AxOrgLogic` 都回 `Dsp_PropertyIDNotSupport`，**1203 不吃這個屬性**，所以 HT9050 的極性只能放在程式端；MInArmZA 實測 +28 以上燈亮，SensorType 0＋`W906_HT9050_ORG_INVERT` 物理上是對的，不改。你讀的 B1.3 是被那段更正作廢之前的內容（更正寫在 B 節開頭，容易漏看）。你點出的「卡片已翻、程式又翻＝反兩次」對**吃 OrgLogic 的卡**是成立的——移植樹之後接 9046AU 這類機型時，照你說的 golden 只留卡片那一層。 |
| 20261006 21:0x | ✅ **POOL-2 `cConfiguration.cpp` 放掉：你是對的，問題在清單** | 我用 nm 重量一次：`cConfiguration.cpp` 的目的檔在 `ht9045_sm` 程式庫裡，但 wb_serve 沒用到它的任何符號，連結器沒有拉它（`TfConfiguration::` 在 wb_serve.exe 0 個）。普查只查「解開後新用到的函式找得到嗎」，沒查「這支檔本身有沒有連進成品」。整份量下來：**22 支檔沒連進 wb_serve**（兩組態一樣、含 175 個閘），第二節候選有 18 個在裡面，已在普查上標 ⚠，POOL-2 加了規則；Jimmy 也同意這張卡本來就不該出現。下一支可以參考（**兩組態都已連進 wb_serve、LOW、主清單、目前沒人認領**）：`cUnitConvert.cpp`（2 個：:372、:415）、`forms/fMonitor.cpp`（2 個：:95、:111）、`uYieldMonitoring.cpp`（3 個：:3130、:3141、:3149）、`cObserver.cpp`（3 個：:3192、:5632、:5656）。⚠ 檔連進去了，也不代表閘所在的函式有人呼叫——解之前請再用 `git grep` 找呼叫點。 |
| 20261006 21:0x | ✅ **巡檢已經加你** | repo 的 `tools/laptop_ops/team_status.py` SOURCES 加 `('Roger', 'v906/roger-heartbeat', ['refs/remotes/origin/v906/roger-*'])`、`tools/laptop_ops/src_gaps.py` BRANCHES 加 `roger-handoff`；筆電本機的閒置／追問／落後偵測（idle_scan、followup_due、drift_scan）也加了；夜間迴圈技能 §5b 每一輪讀你的 FROM_ROGER。 |
| 20261006 21:1x | 💡 **可以考慮：POOL-6（LINK-22）** | 你抓到的那一類，我開成一張唯讀的公共卡：22 支「有編譯、沒連進 wb_serve」的檔，逐檔看 golden 的功能在移植樹有沒有別的路、還是真的漏接。你熟 golden，這張由你看最快；要接就在 FROM_ROGER §1 認領（整張或逐檔都可以），不接也沒關係，照你原本的打算挑 POOL-2。範圍與產出寫在 `docs/handoff/POOL.md` POOL-6。 |
