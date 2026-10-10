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
| 20261006 21:5x | 📋 **工作卡 W-128：HT9050 料盤 28 個疊盤 Z 教點從哪裡來（唯讀，第一步）** | 背景：`cprod_9050.inc` 裡 9050 專用的料盤 Z 欄位（Loader 的 `TrayZ_Up／Down／Home`、Empty、Auto1～Auto3 各自的 Up／Down／Home、初始探層、額外抬升）沒有正式賦值，但 `asendic_Loader.cpp` 的 `DoLoad_9050`（Task 600／1000，約 :2771、:2789、:2856、:2879）直接拿來當移動目標（NB2-GPT 的 `docs/handoff/HT9050_TEACH_READINESS.md` 第二列）。Frank01 在做 9050 料盤流程（F9050-FIX3），這一塊等人接。**請你先唯讀查**：①910-9050（`HT9011UC_Code_V3.33.910.0_20260820_HT9050`，EastSun 定義版）與 golden 0618 裡這些值在哪裡賦值（讀 teach.ini 的哪個區段／鍵，或由別的值算出來）；②Teach 頁（`HW.teach`）有沒有對應的教導欄位；③移植樹要照 910 翻的話要動哪些檔、大約多少行。產出寫在你的 FROM_ROGER §2（或另開 `docs/handoff/W128_TRAYZ_SOURCE_<日期>.md`）；**先不改程式**，結論出來再決定誰改（可能就是你）。接之前在 FROM_ROGER §1 認領；不接也請回一句。（另一張可以選的：POOL-7 W-79 (b)，見 `docs/handoff/POOL.md`。） |
| 20261007 09:3x | ⏰ **追問（第 1 次；W-134）：W-128 第一步（唯讀）** | HT9050 料盤 28 個疊盤 Z 教點在 910-9050／golden 從哪裡賦值、Teach 頁有沒有欄位、照 910 翻要動哪些檔。⚠ 機台 cpp 0250 TRAYZ-PITCH（第 83 批）已改成「基準層 − 層數×pitch」＋Teach 頁新區塊，請一併對照它跟 golden 的差異。 |
| 20261007 13:4x | ⏰ **追問（第 2 次；W-134）** | W-128 第一步（唯讀）：HT9050 料盤 28 個疊盤 Z 教點在 910-9050 版／golden 是在哪裡賦值的（檔名:行號）？你有空時回在 FROM_ROGER §2 就好。 |
| 20261008 11:3x | ✅ **W-134（W-128 第一步）收到，結案，謝謝** | 你 11:0x 的結論照收：910 沒有賦值；golden `TrayZ_Up` 是站別不是層別（910 的 `TrayZ_Up[層]` 就算不越界也對錯站）；機台的 `W906_TrayZ9050`（基準－層×pitch，1008 起 pitch＝Tray Form 厚度×100）已取代，舊欄位不讀。剩下的上機量測（base／pitch／Home）歸 EastSun；站別／層別那段已轉 Frank01。 |
| 20261008 21:2x | ✅ **MR !347（POOL-2 `cSpeed.cpp`:290，GATE S3 留著、過期的理由改正）收到**，排第 121 批（試合乾淨）。 | 謝謝。 |
| 20261008 22:1x | ✅ **MR !347 進 main**（第 121 批 `e6c54331`＝第 204 包） | gate b121a：兩組態只有固定失敗（兩組態都是 4 支，489 支測試；模擬組態的 cmake 探測檔被鎖兩次，第三次重跑綠）。 |
| 20261009 07:3x | **（St02-M 請筆電轉告）912↔913 差異表裡跟你有關的「913 可能的 bug」**（W-194；Steven 本人 1009 06:2x：「如果是遇到 913 有 bug 也是要通知」） | 表在 `docs/handoff/ST02_V912_VS_V913_20261009.md`（＋`_files.tsv`／`_functions.tsv`／`_r913_log_items.tsv`；St02-E 1009 07:1x 做的，筆電搬一份進 main，行號都是 913 樹）。都是讀程式推出來的，請你判斷：①**中** `aoutarm9045.cpp`:893-895 `UseFix3Cylinder` 新算的 `hFix3Gap`／`bFix3Frozen`（凍結超過 5 秒不算氣缸時間）整個檔都沒用到，像沒做完，r913 log 也沒寫。②**低** `automation.cpp`:3478-3486 `HANARMSCheckLinkBeforeStart` 在 `Start()` 裡最多 3 秒 ProcessMessages＋Sleep；註解假設 Start 只從按鍵來，但有遠端／自動重啟的呼叫端，從 GPIB／SECS 觸發時可能重入或卡住（推論，沒測）。③**低** `fVATMesFileSys.cpp` `CheckLotInfor` 的越界保護只蓋 1264／1297／1331，1348／1357 還是把 MES 字串 atoi 直接當索引。④**log 沒寫的行為改變**：HANA RMS 預設 port 6670 → 14140。在 FROM_ROGER 回「①沒做完／刻意…」這樣一行就好。 |
| 20261009 12:3x | ⏰ **追問（第 1 次；W-194）** | 07:3x 的 W-194（912↔913 差異表裡跟你有關的「913 可能的 bug」：UseFix3Cylinder 沒用到的 hFix3Gap／bFix3Frozen、HANARMSCheckLinkBeforeStart 在 Start 裡 ProcessMessages、CheckLotInfor 用 atoi 當索引、HANA RMS port 6670→14140）還沒看到回覆；逐項回「刻意／是 bug」就好，回在 `v906/roger-handoff` 的 FROM_ROGER.md。另外：移植樹要不要照 golden 在按 START 時等 RMS 連線（最多 3 秒）正等 Jimmy 決定（#151），你當初為什麼在 Start 裡先確認 RMS 連線，寫一兩句對他很有幫助。 |
| 20261009 16:3x | ⏰ **追問（第 2 次；W-194）——不急** | 07:3x 的 W-194（912↔913 差異表裡跟你有關的「913 可能的 bug」）還沒看到回覆；逐項回「刻意／是 bug」就好，回在 `v906/roger-handoff` 的 FROM_ROGER.md。（#151 那題已改請 Steven 決定，你不用回那段。） |
| 20261009 20:0x | **（告知，不用回）TESNA 的 913 事件紀錄重複寫入，V906 照 Steven 修掉了** | 913 `MyStringList.cpp`:409-432、GetFileName :527-529／:645：N10（JCET 每日上傳）開＋`iN10UploadProductMethod==0`＋`iO15_SaveFilePeriod==8` 時，主檔被 N10 分支改成 TByDay，`bDupYMDFile` 仍為 true，「每天多存一份」的路徑跟主檔一模一樣 ⇒ 每列寫兩次、卡料統計變兩倍。Steven 1009 19:4x：「每月跟每天不影響，各存一行」，V906 修掉（標 [W906]）。BCB 量產版要不要也修，你決定。 |
| 20261010 22:2x | 📏 **提醒：你最近推的分支落後 main 40 以上 包——測試前先更新到 main 最新版**（RULINGS_20261006 第 12 條） | `drift_scan.py` 22:2x 量：`v906/roger-pool2-speed-s3`（10-08 19:58）的基底 `d70b021d` 落後 40 包以上。回來開工時先 `git fetch` 再從 main 最新版開分支（或合 main），再照 AGENTS.md 先跑 `machine_sync.py check`；main 現在是第 255 包。不用回。 |
