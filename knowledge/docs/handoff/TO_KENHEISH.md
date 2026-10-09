# 給 KenHeish：工作卡與回答（Jimmy／筆電 → KenHeish）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** 回覆請寫在你的分支 `v906/kenheish-handoff` 的 `docs/handoff/FROM_KENHEISH.md`（分支請你從 main 開）。
> 兩個檔各自只有一個寫者，git 合併永遠不會衝突。
> 開始：20261007 13:3x（KenHeish 來信、Jimmy 轉：「需要派工或要問我高雄版 HT9046LS／ASEKH、golden 裡 KenHsieh 署名段落的原意時，請建 docs/handoff/TO_KENHEISH.md，我回在 v906/kenheish-handoff 的 FROM_KENHEISH.md」）。筆電的夜間迴圈每 20 分鐘讀一次 `v906/kenheish-handoff`；急事打電話給 Jimmy。

## 0. 規則

1. **你的環境**：你那台的 `D:\HT9045` 是 BCB／SVN 量產環境——**不做 V906 上機或模擬測試，也不跑 `tools/machine_sync/machine_sync.py apply`**（AGENTS.md「測試前先同步機台快照」那條不適用於你）。別人請你量 V906 的東西時，直接回「不在我的環境」。
2. **主要找你的事**：高雄版 HT9046LS／ASEKH 的功能原意；golden（`HT9011UC_Code_V3.33.906.0_20260618`，Big5）裡 `KenHsieh` 署名段落為什麼這樣寫。回答請附「檔名:行號」並註明哪一棵樹（例〔g906〕〔V912〕〔HT9046LS〕）。
3. **讀這個檔**：`git fetch origin` 後用 `git show origin/main:docs/handoff/TO_KENHEISH.md` 讀（主資料夾可能落後 main）。
4. **先認領再做**：要接卡就在 FROM_KENHEISH §1 寫一行、先推再做；做完寫 §2，問題寫 §3；筆電的回答寫在這個檔的 §4。
5. **程式（如果有）**：V906 C++ 走 `v906/kenheish-<主題>` 分支＋MR 到 main，由筆電跑出貨＋模擬兩組態 gate；BCB 樹（V912 等）先在 FROM §3 說，不要直接推 main。
6. 給人看的文字用繁體中文；不要寫任何帳密、權杖、7z 密碼（交接檔會鏡像到公開的 GitHub）。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第 85 批 gate 跑中） | 機台 cpp 0267～0277（`WebMotorAccess.cpp`、`WebMotorAccessLive.cpp`、`EtherCAT/Pci1203*.cpp`）、St02 !300（`forms/fNote_ShowError.cpp`、`tools/wb_serve.cpp`、`tools/wb_dialog_mailbox.h`、`web/page/ht9045_alarm_motionview.js`）、`tests/CMakeLists.txt` |

## 2. 須知（不用回，知道就好）

- 分工：動作流程（Index／Shuttle／Tray／HOME）先照 Steven 本人的話、其次 Frank（RULINGS_20261006 第 20 條）；機台事實問 EastSun；客戶碼與你署名段落的「原意」問你。
- 名字：你的帳號是 KenHeish，golden 署名是 KenHsieh——兩個我們都認得是你。

## 3. 工作卡

（目前沒有。要問 HT9046LS／ASEKH 或 KenHsieh 署名段落時加在這裡。）

## 4. 回答

| 時間 | 事項 | 內容 |
|---|---|---|
| 20261007 13:3x | ✅ **通道開好了**（Jimmy 轉來你的信） | 筆電的夜間迴圈會讀 `v906/kenheish-handoff` 的 FROM_KENHEISH.md。心跳可選（人工 session 不強制）：`python tools/laptop_ops/heartbeat.py --who kenheish --doing "<在做什麼>" --push`。 |
| 20261007 16:3x | **回 §3 Q-1（同一台電腦 BCB 與 V906 並存）** | ①**只讀程式＋編譯是安全的**：編譯只寫 `Obj/`，不碰 `D:\HT9045\system`、`config`、`MDB`。今天起 `build.bat`（不帶參數）只建 wb_serve、不編測試（RULINGS_20261007 第 7 條），要編測試用 `build.bat gate`（另一個目錄）。②**在那台跑 wb_serve 或 ctest 之前要先保護真檔**：V906 的 ctest 量過會寫進 `D:\HT9045\system`（工單的溫度、AutoClean 計數、`machinerecord.dat` 等）。你說的「先備份 system／config／CFG／IniData／MDB、跑完還原」就是對的做法；樹裡的 `tools/realfile_guard.py`（`snap <名字>` → 跑 → `check <名字>`，有變就 `restore`，沒變就 `drop`）就是把這件事自動化，它的根目錄寫死 `D:\HT9045`——在你那台正好就是要保護的 BCB 環境，可以直接用。③**把機台根目錄做成可設定**：目前沒有排；現成的縫只有 `W906_GENERAL_INI_PATH`、`W906_AUTH_PATH` 和測試用的 `W906_HT9045LOG_ROOT`，`asSystemPath`／`asDBPath`／`D:\HT9045_Log` 都還寫死。要做會動到全樹每一個寫死的 `D:\HT9045` 路徑，等你真的要在那台跑 V906 時說一聲，我們開卡估範圍。 |
| 20261009 07:1x | **（St02-M 請筆電轉告，不用決定）SVN r913 少了你的 Barcode2SetupData 三支檔**（W-192） | St02 1009 06:5x 量到：SVN `HT9011UC_Code_V3.20` r913 的 `HT9045.cpp`:122（USEFORM）／:284（CreateForm）與 `HT9045.bpr`:154／:193／:602 都引用 `Barcode2SetupData`，但 `Barcode2SetupData.cpp`／`.h`／`.dfm` 不在 r913 的改動清單、SVN HEAD 也找不到（`svn log -v`／`svn ls`／`svn cat` 確認）⇒ **從 SVN 取出 r913 的人編不起來**。GitLab `ht9045_913` 有這三支（ST 組用的 golden 不受影響）。r913 是 JerryYang 提交的，同一件事也寫在 TO_JERRY；你或他補 `svn add` 都可以，補好在 FROM_KENHEISH 回一行（哪個 revision）就好。 |
| 20261009 12:3x | ⏰ **追問（第 1 次；W-192）** | 07:1x 的 W-192（SVN r913 少了你的 Barcode2SetupData 三支檔，請補 `svn add`）還沒看到回覆；你或 Jerry 其中一位補上就好，回在 `v906/kenheish-handoff` 的 FROM_KENHEISH.md（一行 SVN 版號）。 |
