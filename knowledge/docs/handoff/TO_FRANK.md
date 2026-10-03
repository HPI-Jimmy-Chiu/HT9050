# 給 Frank：工作卡與回答（Jimmy／筆電 → Frank）

> **這個檔只有 Jimmy 這邊（Jimmy 本人或筆電的 Claude）會寫。** Frank 請不要改這個檔；回覆請寫在你自己開的分支
> `v906/frank-handoff` 的 `docs/handoff/FROM_FRANK.md`（聊天寫 `CHAT_FRANK.md`；這支分支只放這兩個檔，不開 MR、不合 main）。兩個檔各自只有一個寫者，git 合併永遠不會衝突。
> 開始：20261001（Jimmy 11:3x：「Frank01要接關於流程控制的導入，尤其是Index和shuttle動作，可以派工給她處理」）。規則比照 `TO_IFOR.md` §0、`TO_KEVIN.md` §0。
> **機台名**：Frank 的 Claude 叫 **Frank01**（之後多一台就叫 Frank02，比照 St01／St02、Ifor01）；每列寫日期時間（YYYYMMDD HH:MM）。

## 0. 規則

1. **每次開工先 `git fetch`**，讀這個檔 §1，以及 `TO_STEVEN.md` §1、`TO_IFOR.md` §1、`TO_KEVIN.md` §1（別人正在改的不要碰）。
2. **開工前先認領**：在 `v906/frank-handoff` 的 `FROM_FRANK.md` §1 寫「我接 F-xx、會動哪些檔（行號）」，先推一顆小 commit 再做。做完寫 §2（commit／分支、驗證）；問題寫 §3。筆電的回答寫本檔 §4。
3. 程式碼走 `v906/frank-*` 工作分支＋MR（目標 `main`）；筆電跑兩組態全新 gate（出貨＋模擬）綠了才合。
4. **參考原文**：9050 專屬的流程（Index、Shuttle、9050 Tray 臂）以 **Frank 的 910 樹**為準；其他一律以 golden `HT9011UC_Code_V3.33.906.0_20260618` 為準（`HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261001.md` 第 0 條：照原文補齊接上，包括運動／IO）。
   要「改得跟原文不一樣」（新設計、修原文的怪處）先在 §3 問 Jimmy。翻譯照移植樹的規矩（C++17、UTF-8、`//AI(W906-<代號>) YYYYMMDD: 描述`、golden 怪的照翻並註明；見根目錄 `CLAUDE.md`「編輯 V906 C++ 移植版」）。
5. 真實檔照「備份→驗證→刪備份」（`tools/webprobe/wbrun_guard.py`）。commit 作者用你自己的公司信箱。急的事打電話給 Jimmy。

## 1. 我們正在改的檔（這些先不要動）

| 誰 | 檔 |
|---|---|
| 筆電（第十七批，gate 中） | `MainTimerSegments.cpp`（新）、`forms/fMain_SetLotState.cpp`（新）、`WebBridgeTags.cpp:626`、`CMakeLists.txt` 兩行、`csystem.cpp:4052`／`:9770`、`AutoRetest.cpp:278`、`Automation/SCK_ART_Remainder.cpp:337`、`forms/fMain.cpp:553`、`forms/fAGV.h:34`、tests |
| 筆電（下一批） | `csystem.cpp` 的 W7C1／g4 替身區（`:2576-2584`、`:6369`）與呼叫點（`:3106-3117`、`:7505-7515`、`:7650-7682`）；`RecordSafeDoorStates`（`:16544-16552`、`:21388-21640`） |
| Ifor（I-01） | 加熱鏈：golden `Index16Heater`／`IndexHeatMode`／`HotplateHeatMode` 與出貨組態的 `THeaterThread`（認領後的行號看 `FROM_IFOR.md` §1） |
| St02 | 冷卻風扇（`csystem.cpp` G04／G07／G08／G12／G12b、`:24787-25031`、`uHeaterThread.cpp:540`）；H-008（`vclcompat/ClientSocket.*`） |
| Kevin | `TO_KEVIN.md` §1（0929 Jimmy：「Index 流程交給 Kevin」——F-01 開工前請先在 §3 跟 Kevin 對一下分工，避免兩邊做同一段 Index） |

## 2. 須知（筆電 1001 11:2x 量的，不用回）

- **共用區交付包的 7z 密碼（Jimmy 1001 13:4x：「未來統一用一個密碼」「所有人都要知道密碼，這不是機密，也不會有任何風險，否則無法多人協作」）：`〔交付 7z 密碼：已遮，不放 GitHub（RULINGS_20261001 第 40 條）〕`**。以後所有共用區（`U:\共用區\`）的交付包都用這一組。例外：9/25～10/01 打的包（例 0930 的 K-01 兩包 `U:\共用區\HT-9050\K01_golden0618_HT9050snapshot_20260930\`）用的是退役的那組 `〔交付 7z 密碼：已遮，不放 GitHub（RULINGS_20261001 第 40 條）〕`。這兩組只寫在交接檔（公司 GitLab）；公開的 GitHub 機台包不放。
**你的 910 樹**（`D:\HT9045\Staterecord\HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch`，Jimmy 1001 11:0x 放的副本）：
- SVN `…/SourceCode/SVN` 的 `HT9011UC_Code_V3.20`，基準是 r909／r910（Steven 提交）。**9050 的改動全部是本地還沒提交的**：29 支檔（+6,340／−256 行、134 個 hunk）＋3 支沒進版控的新檔
  （`atester_FinePitch.cpp` 3,040 行、`atester_FinePitch.h`、`SmartSetup.cpp` 空檔）。新增行的署名：Eastsun 227 行（0818～0820），其他人零星。
- 流程邏輯集中在：`acatchtray.cpp` +546、`asendic_Loader.cpp` +293、`acarry.cpp` +284（飛梭 `Do_Auto_InSH`／`Do_Auto_OutSH`）、`csystem.cpp` +215、`cmydef.*` +276、`main.h` +141、`cinitial.cpp` +101；
  Index：`MoveIndexZ`、`CheckIndexStatusERRSH1`、`atester_FinePitch.cpp`（csystem.cpp 已引用）；9050 Tray 臂：`DoCatchFromLoader_9050`、`DoCatchFromEmpty_9050`、`DoPlaceTrayToEmpty_9050`、`DoPlaceTrayToAuto_9050`、`DoPlaceToBuffer_9050`、`DoLoadNewICTray_9050`、`DoLoad_9050`。
- **新機型 `Type_HT9050 = 800`**（`MachineType.h`），流程用 `MachineTypeChoice==Type_HT9050` 判斷（26 處）。⚠ 移植樹現在把 HT9050 當 `Type_HT9046_LS`＋PCIE-1203（RULINGS_20260926 第 25 條）——兩者怎麼合，Jimmy 會裁（見 §3 F-01b 前提 ①）。
- 畫面：`iosetview.dfm` +2,687、`main.dfm` +1,244 行。移植樹的畫面是網頁（Steven 團隊的範圍），**F-01 不做網頁**，只在盤點裡列出來。

**環境沒有對齊（這是 F-01a 第 2 項要解的）**：

| | 你那次模擬（state record `D:\HT9045\Staterecord\2026-10-01 10_50_09`） | git 裡的 HT9050（`machines/HT9050/`，機台與筆電用） |
|---|---|---|
| 工單 | `9378_1952DFT4_H9046_22_025_V01` | `FT005054_9050`（第 1 臂、Shuttle 1；說明 `FT005054_9050_DIFF.md`／`_VERIFY.md`） |
| 卡別 | `MOTION_CARD_TYPE=1`、`IO_CARD_TYPE=1`、`INDEX_MOTION_CARD=0` | `MOTION_CARD_TYPE=0`、`IO_CARD_TYPE=4`（PCIE-1203 IO）、`INDEX_MOTION_CARD=0` |
| 機型 | `Type_HT9050`（800） | 移植樹當 `Type_HT9046_LS` |
| git 裡有的 | — | IO 表、馬達表、`Pci1203Axis.ini`／`Pci1203Io.ini`、工單；**沒有** `Gerneral.ini`、`config.ini`、教導資料 |

## 3. 工作卡

### F-01a　9050 流程導入的盤點與環境對齊（**現在就能做，唯讀**）

1. **134 個 hunk 的盤點**：逐支檔分類（流程邏輯／畫面／建置／設定），每個流程 hunk 寫出它在移植樹的對應位置（移植樹的檔逐行標著「golden xxx.cpp:NNNN」＝906 的行號，所以是 910→906→V906），
   以及它依賴、但 906／V906 沒有的東西（906→910 的公司改動）。交付：`HT9011UC_Cpp_V3.33.906.0/docs/FLOW9050_PORT_LEDGER.md`（MR；格式可參考 `docs/MG_PORT_LEDGER.md`）。
2. **環境對齊提案**：列出你那次模擬用的工單與 `system`／`config` 設定、跟 `machines/HT9050/` 的差異，提一份「大家共用的 9050 模擬組」該放哪些檔（工單、`Gerneral.ini`、`config.ini`、IO／馬達表、教導資料），
   以及哪些值是「真機才要」（1203 卡別）、哪些是流程分支會看的（機型、工單參數）。寫在 `FROM_FRANK.md` §3，等 Jimmy 裁。
3. **參考軌跡**：你那次 state record 的 `Task_ListWithTime.csv`／`Task.xls`／`DecisionVariables.csv`／`MainFormSnapshot.txt` 是 V906 之後要對照的基準；說明那一輪的操作步驟（從開機到一輪跑完按了什麼），寫在 §2 或 §3。

### F-01b　Index 與 Shuttle 流程照你的 910 翻進 V906（**Jimmy 1001 13:2x 已裁 ①②：可以開工**，先在 FROM_FRANK §1 認領行號）

- 前提（**1001 13:2x 已裁，見 §4**）：① 機型＝**新增 `Type_HT9050`**（照你的 26 處判斷搬）；② 共用模擬組＝**你那份工單 `9378_1952DFT4_H9046_22_025_V01`**，F-01a ② 做成 `machines/HT9050/` 的檔。~~原文：① 機型：照你的樹新增 `Type_HT9050`（建議），還是維持移植樹的 `Type_HT9046_LS`；② 共用的 9050 模擬組（工單與設定）定版、放進 `machines/HT9050/`。~~
- 範圍：Index（`MoveIndexZ`、`CheckIndexStatusERRSH1`、`atester_FinePitch.cpp`）與 Shuttle（`Do_Auto_InSH`／`Do_Auto_OutSH`）優先，再來 9050 Tray 臂流程；每個狀態機附 ctest。
- 驗收：用共用模擬組在 V906 模擬組態跑一輪，StateRecord 的 `Task_ListWithTime` 在 Index／Shuttle 的 Task 序列跟你的參考軌跡一致（不一致的逐項列原因）＋兩組態 gate 綠。
- 不要動：Ifor 的 I-01（加熱）、St02 的冷卻風扇與 H-008、筆電 §1 的檔；網頁畫面（Steven 團隊）。

### F-02　把你的 910 樹放上 git：獨立參考分支（Jimmy 1001 13:2x「照建議」＝NIGHT_REPORT §0 第 25 項 A）

- 分支 `ref/frank-910-9050`，**orphan**（跟 main 沒有共同歷史）。**永遠不開 MR、不合 main**，也不會進 GitHub 機台包；用途是讓 910 的改動有版本、`git diff` 看得出每次改了什麼，翻譯時逐行對照。
- 第 1 顆＝**SVN 原版**：你那台工作副本的 BASE（例：`svn export -r BASE <工作副本> <暫存目錄>`），放在分支根目錄 `HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch/`。
- 第 2 顆＝**你目前的工作副本**（含沒進 SVN 的 `atester_FinePitch.cpp`／`atester_FinePitch.h`／`SmartSetup.cpp`）。之後你每給一次新快照就再加一顆，commit 訊息寫日期與這次改了什麼。
- 不放：`.svn`、建置產物（`*.obj`、`*.exe`、`*.tds`、`*.il?`、`__history`）、State Record 與執行期資料（`system`／`config`／`IniData`）。
- **位元組不變**：Big5、CRLF 照原樣，不要轉碼；分支根目錄放一個 `.gitattributes`，內容 `* -text`，讓 git 不改換行。
- 推之前掃權杖／私鑰／7z 密碼；推完在 FROM_FRANK §2 寫分支名、兩顆 commit 的 hash 與檔數。排在 F-01a 之後、F-01b 之前或同時都可以。

## 4. 回答

| 時間 | 你的問題 | 回答 |
|---|---|---|
| 20261001 12:0x | 11:38 §3 兩題：開 TO_FRANK.md 派工、把 `v906/frank-handoff` 加進夜間迴圈 | **兩件都已經做了**（main `84e1cb6b`，11:35，跟你 11:38 那顆剛好錯過）：本檔就是派工（§3 的 F-01a 現在就可以做，唯讀；F-01b 等 Jimmy 定機型與共用模擬組），夜間迴圈第 5b 步已經讀 `v906/frank-handoff`。**請在 FROM_FRANK §1 認領 F-01a 再開工。** 環境：你那台沒有 `C:\MinGW`（g++ 6.3.0，唯一能重現 BCB6 x87 算術的 oracle）⇒ F-01b 開始改 C++ 時，要嘛照 `.claude/skills/pt-wave-loop/SKILL.md`「建置環境：從零裝起來的實測步驟」裝 MinGW.org 6.3.0＋CMake 4.0.2（mingw-get 0.6.2），要嘛只用 WinLibs 16.2 做 `-fsyntax-only`、由筆電跑兩組態 gate 合你的 MR。F-01a 是唯讀，不受影響。 |
| 20261001 12:3x | 12:16 §2 F-01a ① MR !32（`v906/frank-flow9050-ledger` `79fdf5ce`，只新增 `docs/FLOW9050_PORT_LEDGER.md`） | **收到，謝謝。** 只有文件，跟筆電第十八批（gate 中）一起合進 main。§0 的 ①（V906 沒有 9050 的 26 個氣缸＋75 個感測器定義，`machines/HT9050/IO_Table.csv` 已在用那些名字）正好解釋了筆電普查 (e) 的 E-IO-* 202 列（「IO 表比程式新」）——那些名字是 910 樹的 9050 定義，會跟 `Type_HT9050`（NIGHT_REPORT §0 第 23 項）一起決定怎麼進來。 |
| 20261001 12:3x | 11:53 F-01 與 Kevin K-02 的分工（A／B／C） | **轉 Jimmy 決定**（NIGHT_REPORT §0 第 29 項，筆電建議 A：K-02 改成審 910 的 Index、F-01b 照 910 翻，只留一份 Index）。F-01a 不受影響，照做。 |
| 20261001 13:0x | MR !32 | **已合進 main**（跟筆電第十八批一起；只有文件，推之前掃過權杖／密碼 0 處）。GitLab 會自動把 MR !32 標成已合併。 |
| 20261001 13:2x | NIGHT_REPORT §0 第 23～26、29 項（Jimmy 1001 13:2x「照建議」） | ① **機型：新增 `Type_HT9050`**（照你的 26 處判斷搬；取代 RULINGS_20260926 第 25 條「HT9050 當 HT9046_LS」的暫行做法）——F-01b 一起做，你盤點 §0 ① 的 26 個氣缸＋75 個感測器定義（上限 321／900）也在這一項裡。② **共用模擬組＝你那份工單 `9378_1952DFT4_H9046_22_025_V01`**，放 `machines/HT9050/`（工單、`Gerneral.ini`、`config.ini`、IO／馬達表、教導資料，加參考軌跡 2026-10-01 10_50_09 的純文字部分；`machines/` 不進 GitHub 包），`FT005054_9050` 留著當量產工單——F-01a ② 照這個做成檔案、開 MR。③ **910 樹上 git**：新卡 **F-02**（§3）。④ 之後 9050 流程**直接在 V906 開發**，910 樹凍結當參考（UI 的 .dfm 交給 Steven 那邊做網頁）。⑤ **Index 只留一份**：Kevin 的 K-02 改成「審 910 的 Index」，F-01b 照 910 翻（筆電同時通知 Kevin）。⇒ **F-01b 可以開工**，先在 FROM_FRANK §1 認領行號。 |
| 20261001 14:3x | 13:48 §3 三題；14:14 §1 三列（F-02、`sim_9378`、盤點更正） | (1) **照你 14:0x 的回答定案**（Jimmy 1001 14:2x：「這部分Frank有回覆，你確認他的訊息」）：Loader 的 Z（`MLoaderZ`）、Empty 的 Z（`MEmptyZ`）是馬達，Auto1～3 的 Z 是氣缸＋馬達複合 ⇒ **F-01b 照 910 的馬達流程翻**；RULINGS_20260930 第 2、3 條改寫成 RULINGS_20261001 第 22 條（隨筆電第十九批的文件進 main）。機台正本 `machines/HT9050/Mot_Table.csv`（M35／M36／M38～M40 的 Enable 與 1203 軸號）等 ES02（EastSun 的筆電）上機確認後由筆電改（TO_ES02 E-03）；你的 `sim_9378/` 用它自己的表，不受影響。(2)(3) 照你 14:14 寫的（第 12 條：9378 常溫、檔案取自 10_50_09）。H003（只有 Shuttle1）、`Do_Auto_OutSH` 的 `SystemNG`（恢復檢查）收到。**§1 三列都同意**：F-02 照你寫的三顆（Eastsun 那 76 支手動備份先不放）；`machines/HT9050/sim_9378/` 放子目錄、不蓋正本；盤點更正跟模擬組一起推，筆電合。`machines/` 不進 GitHub 機台包（打包只取 906 樹與 `web`），但它在公司 GitLab，推之前照樣掃權杖／密碼。 |
| 20261001 15:5x | 14:5x §3 Frank 補充機構（分盤是 Z 軸馬達，跟 Tray 臂交接時觸發氣缸） | **跟 910 程式對過：一致**（Jimmy 1001 15:4x：「需要從他寫的BCB版本對齊答案，有任何訊息不清楚必須和Frank問清楚，不要猜，程式邏輯和他的描述必須一致，如果有矛盾處必須釐清」；對的是你推的 `ref/frank-910-9050` `beba23ee`）。逐層升降全是馬達：`acatchtray.cpp` `DoCatchFromLoader_9050`（:2439）`MOT[MLoaderZ].MotorMove(Prod.TrayZ_Up／Down[層])`、`DoCatchFromEmpty_9050`（:2536）與 `DoPlaceTrayToEmpty_9050`（:3969）`MOT[MEmptyZ]`、`DoPlaceTrayToAuto_9050`（:4070）`MOT[MAuto1Z～MAuto3Z]`、`csystem.cpp` `DoReceiveAllToBottom_9050`（:7626）五座 Z；交接才動氣缸：各區 `EdgeClip`／`EdgePush`（先收）、`C_TrayZ_Selector`（Loader）／`C_EmptyLoaderZ_Select`／`C_Auto1～3LoaderZ_Select`、`C_MobileTrayTableSelect`；9050 流程不看 `*_Z_USE_MOTOR`。⇒ RULINGS_20261001 第 22 條照這個寫（隨筆電第二十批的文件進 main）。**讀程式時有 3 處看不懂，請 Frank 本人回答**（回答前 F-01b 照 910 原樣翻、在註解寫明「待 Frank 確認」，不要自己改）：見下一列。 |
| 20261001 15:5x | （筆電問 Frank 本人，三題） | **Q1** `DoPlaceToBuffer_9050` case 200（`acatchtray.cpp`:4294）呼叫 `DoPlaceTrayToAuto_9050(0)`，但那支函式是從 1 起算（:4072 `idx = iAutoTarget - 1`；:4073 `idx<0` 就 `return 0`），所以這條路永遠不會完成；旁邊註解寫「按照之前架構移到auto 1」——本來要的是 `(1)` 嗎？還是 9050 實際不會走到這條（`CatchTraySuck.Item[0][0]` 不是 1、2 的時候）？**Q2** 層數計數：`iEmptyLayerCount_9050`（`acatchtray.cpp`:2529）與 `iAuto1～3LayerCount_9050`（`cmydef.cpp:6109-6111`）初值都是 -1，全樹除了放盤時 `++`（Empty :4007／:4024、Auto :4169／:4184，一次放盤加兩次）、`DoReceiveAllToBottom_9050` 收完設回 -1（`csystem.cpp:7691`）之外，**沒有地方設定**（Loader 有偵測：`asendic_Loader.cpp` :2632／:2681／:2699／:2728；`iEmptyLayerCountDetect_9050` 宣告了但沒用到）。所以第一次放到空的 Auto／Empty 疊時，`Up[-1]`、`Down[-1]` 會被拿來當位置；`DoCatchFromEmpty_9050`（:2536-2603）也完全不改這個計數（Loader 取盤在 case 20 會 `++`）。請說明：這個計數代表什麼（第幾層？還是位置表的索引？）、-1 本來打算在哪裡被設成正確的值、放一次盤為什麼加兩次、Empty 取盤要不要改計數。**Q3** `DoPlaceTrayToAuto_9050` case 700（`acatchtray.cpp`:4222-4223）兩行都是 `MOT[MOutArmY].fCanMove=true;`，可是 case 1（:4116-4119）在 `TRAY_ARM_MODE==eAboveCoveyor` 時把 `MOutArmX`、`MOutArmY` 都關掉了——第一行本來是 `MOutArmX` 嗎？（不然那個模式下放完 Auto 盤之後出料臂 X 就一直不能動；HT9050 是不是 `eAboveCoveyor`？） |
| 20261001 18:1x | 16:07 §1 F-01b 第 A 批（Shuttle＋4 個小分支，全包在 `Type_HT9050` 裡，零行為變化） | **可以**。`acarry.cpp`／`acarry.h`、`csystem.cpp` `DoAllProcess` :1901-1916、`csystem_predicates.cpp` :309／:316、`aoutarm9045.cpp` :4290、`ainarm9045.cpp` :4147-4158、`ainarm2.cpp` :4090 都是筆電的檔，第二十批（18:1x 上 main）與第二十一批都沒有動這些行。從新的 main 開分支。 |
| 20261001 18:1x | 14:38 §3「910 的 9050 程式另外核對出的缺陷」(a)～(e) 與三題；16:07 範圍盤點 (1)～(5) | ① **筆電先更正**：15:5x 寫給 Frank 本人的 Q1（`DoPlaceTrayToAuto_9050(0)`）、Q2（層數 -1）就是你 14:38 的 (b)、(a)——是筆電漏讀了你 14:38 那幾列，不是新發現；**Q3（`DoPlaceTrayToAuto_9050` case 700 兩行都是 `MOutArmY`）是新的**。② 修不修 (a)～(e)、Mot_Table 那幾軸要不要開、12 處 `\|\| Type_HT9050` 留不留、GPIB 橋接誰加、機台的 `Model` 字串、C 批放哪裡——**都是 Jimmy 的決定，已轉**（NIGHT_REPORT §0 第 34 項）。回覆前照你寫的：A 批先做；(a)～(e) 照 910 原樣翻、註解寫「待 Frank 本人說明／待 Jimmy 定」，不先修。 |
| 20261001 18:1x | 17:19 §3 Frank 本人 17:1x 的回答（①～⑤）；A 批抓到的 910 死碼 | **收到，謝謝**。③（照 2×4 拿掉 12 處 `\|\| Type_HT9050`）與 ⑤（`InitTestYFPTask` 的呼叫點放 `InitAllProcessTask()`）**已轉 Jimmy**（NIGHT_REPORT §0 第 34 項，筆電建議兩項都同意；③ 要連同 GRID 的 ChangeSite 風險與 2×8 的 ctest 一起改）。回覆前：③ 先不拿（現況）；⑤ C 批照你寫的做，只差呼叫點那一行。死碼兩題（`Do_Auto_OutSH` case 100 的殘料檢查永遠不執行；Y1 用 `Tech.iHT9040TestY1_Front` 還是 `Prod.TestY1_Front`）等 Frank 本人回——回覆前照 910 原文翻、ctest 照原文釘，同意。⚠ **15:5x 筆電直接問 Frank 本人的 Q1～Q3 還沒看到回答**（17:19 那五題是你 16:07 的題目）：Q1 `DoPlaceTrayToAuto_9050(0)`（函式從 1 起算）、Q2 Empty／Auto 層數計數器從 -1 開始沒人設、Q3 case 700 兩行都是 `MOutArmY`——請 Frank 本人回在 FROM_FRANK §3（`docs/handoff/WAITING_REPLIES.md` W-01）。 |
| 20261001 19:0x | ⏰ 追問（第 1 次；`docs/handoff/WAITING_REPLIES.md` W-01；**用信**） | 15:5x 問 Frank 本人的 Q1～Q3 到 19:00 還沒看到回答 ⇒ 照 Jimmy 1001 17:0x 的授權（「如果今天晚上七點前還沒收到回覆，可直接擬稿並且寄給他，要求他在他的AI端寫清楚」），**19:02 已寄信到 Frank 本人信箱**（三題原文照抄本檔 15:5x 那一列）。請 Frank 把答案寫在 FROM_FRANK §3、每題註明是他本人的說明（像 17:1x、18:0x 那樣附原話）。 |
| 20261001 19:0x | 18:10 §2 F-01b 第 A 批（`v906/frank-f01b-shuttle` `64170d56`，MR !63）；18:55 Frank 本人 18:0x 的兩題 | **收到**，MR !63 排進筆電第二十二批（兩組態全新 gate 綠了才合）。Frank 的答案：① `Do_Auto_OutSH` 的殘料檢查照 910 不執行（9050 用別的方式檢查）——照 910 保留＋註解，同意；② `CheckIndexStatusERRSH1()` 改用 `Prod.TestY1_Front`——**偏離 910、Frank 決定**，筆電會在 NIGHT_REPORT 跟 Jimmy 說明（不擋合併）。 |
| 20261001 19:5x | 19:29 §3 Frank 本人 19:2x 第一輪（Q1～Q3） | **收到，謝謝**。Q1「這條會走到，目前還在修正中」⇒ V906 這段照你寫的等 Frank 修好再照新版翻（D 批），在那之前照 910 原樣、註解寫「待 Frank 修正」；Q2（白話重述）、Q3（收尾那一行是不是 `MOutArmX`）由你繼續問 Frank，有答案寫 §3，筆電照答案翻。 |
| 20261001 20:2x | MR !63（F-01b 第 A 批） | **已合進 main**（第二十二批，筆電兩組態 gate b22a 1001 19:52-20:23 (HEAD 6f83a410): SHIP 4 = baseline, SIM 19 = baseline；`Flow9050_Shuttle` 在 gate 裡通過）＝GitHub 第 109 包。`csystem.cpp` 在 DoAllProcess :1898 之後的行號往下移 16 行（已在 TO_STEVEN 告知 St01／St02）。B 批（切換機型＋IO 定義）照 NIGHT_REPORT §0 第 34 項等 Jimmy。 |
| 20261002 00:3x | 🔎 覆核意見（NB2 R126（`v906/nb2-assist` `32779f33` 的 `docs/nb2_assist/README.md` R126）；MR !63 F-01b 第 A 批：**正確**，跟 910 逐句一致，只有你同意並註記的 `Prod.TestY1_Front` 一處不同） | 上機前要知道的兩件（今天不會跑，`Type_HT9050` 還沒有人指派）：① 唯一的編碼器互鎖 `CheckIndexStatusERRSH1`（Index Y1 在前、Z1 低於安全高度就不准飛梭動）**沒有任何測試看得到它壞掉**：SIM 下讀編碼器一律回 false，SHIP 測試又把軸設成 Enable=false。② H093 只做了一半：910 對 `Type_HT9050` 叫 `DoTestHeadMotorFP()`，移植樹仍對所有機種叫 `DoTestHeadMotor()`（`csystem.cpp:1937`）——910 的飛梭狀態機配 906 的 Index 狀態機，這個組合誰都沒跑過。⇒ NB2 建議：**`database.cpp:517` 改成 `Type_HT9050` 要等 B／C 批（`DoTestHeadMotorFP`）落地之後**，而且先補一個 SHIP 組態的互鎖測試。不用回，排 B／C 批時照這個順序 |
| 20261002 05:5x | ❓ **問題（W-11）：HT9050 的 Teach 頁要用什麼範圍** | 網頁 Teach 頁現在幾乎沒有小鍵盤範圍。我們想照 golden `uteach.cpp` `setEditLimitClick` 補，但你的 HT9050 軌跡（`machines/HT9050/sim_9378/teach.ini`）有好幾個值落在 golden 範圍外：Auto1～3 的 Y＝-72735／-72746／-72695（golden -53000～-59000）、Fix1～3 Y 約 -18500（golden -7000～-13000）、InSht1Y -53699／OutSht1Y -53632（golden -35000～-41000）。你的 910 樹 `uteach.cpp` 這段跟 906 一樣。請教三件：①HT9050 上教導值是移動馬達後按 Set，還是也會手打？②910 在 HT9050 上手打這些值時，是不是也會被夾掉（有沒有遇過）？③HT9050 各軸的行程／軟體極限是多少（`Mot_Table.csv` 的 PSoftLimit／NSoftLimit 能不能直接當範圍）？不急，回在 FROM_FRANK 就好。 |
| 20261002 09:3x | ❓ **再確認一次（W-12；Jimmy 1002 09:1x「2X4這問題請重複問她，double check」）** | Frank 1001 17:1x 說 **HT9050 的 Carry kit 與 Index 吸嘴是 2×4**，所以 V906 自己加的 12 處 `|| Type_HT9050`（走 2×8）要照 910 拿掉。Jimmy 要動手前再跟 Frank 本人確認一次：①入料／出料飛梭的 Carry kit 是 **2 列 × 4 欄**嗎？②Index（測試臂）的吸嘴也是 **2 × 4**？③確定照 910 拿掉那 12 處（9050 走 2×4、不走 2×8）？請 Frank01 轉問 Frank 本人，答案（附原話）寫在 FROM_FRANK §3。在確認之前 12 處先不動。其他照 Frank 的說法（Jimmy 已同意 §0 第 34 項）。 |
| 20261002 11:1x | ⏰ 追問（第 1 次；`WAITING_REPLIES` W-11） | 05:5x 問的 **HT9050 Teach 頁範圍**還沒看到回答：①教導是移動馬達後按 Set，還是會手打數值；②910 手打時會不會被夾；③各軸行程（能不能用 `Mot_Table` 的軟體極限當範圍）。⚠ **更新**：機台端 1002 06:4x 已照 EastSun 的裁決 C，在機台上做了「Teach 每一欄照 golden 9046LS 臂的範圍」（機台 cpp 0117 TEACH-KB；機台自己量到這台 11 個在用的教導位置會被夾）；筆電這輪整合（GitHub 第 125 包）**沒收**這一項，等 Jimmy 決定（他 09:1x 選的是 A：Teach 先不夾，之後用馬達軟體極限）。你的答案就是他決定的依據。 |
| 20261002 15:3x | ⏰ 追問（第 2 次；`WAITING_REPLIES` W-11） | 05:5x 問、11:1x 追過一次的 **HT9050 Teach 頁範圍**還沒看到回答：①教導是移動馬達後按 Set，還是會手打數值；②910 手打時會不會被夾；③各軸行程（能不能用 `Mot_Table` 的軟體極限當範圍）。這三題現在是 NIGHT_REPORT §0 第 51 項（機台照 EastSun 裁決 C 做的 TEACH-KB 要不要收）的依據。寫在 FROM_FRANK §3 就好；一句「不知道／要問 Frank 本人」也行。再 4 小時沒回，筆電會請 Jimmy 決定要不要直接問 Frank。 |
| 20261002 21:4x | 📌 Jimmy 1002 20:2x（`RULINGS_20261002.md` 第 21 條）：**HT9050 的開發／測試一律用機台推上來的工作檔**（工單＋機台參數） | 原話：「機台端有透過github把工單和機台設定檔放上去，你放到gitlab後，未來要求其他人要協助開發或測試時，都要用此工作檔，這樣才能有效同步問題」。機台 1002 19:29 的快照（GitHub `machine/integ-ioweb` `ca828068`）筆電已逐位元組放進 GitLab main `machines/HT9050/snapshot/`（`c84209ad`，694 檔；目前工單 IOWEB_TEST_R003，料盤 7×17、熱盤 8×16）。Frank01：**之後重現、量測、測試 HT9050 的行為，先照 `machines/HT9050/snapshot/SNAPSHOT_SOURCE.md` 裝好**（先備份 → 複製到自己的 `D:\HT9045\system\`／`config\`／`IniData\Data\` → 比 MD5 → 做完還原），回報時寫明用的是哪一版（`git log -1 --format=%h -- machines/HT9050/snapshot`）。機種身分不在快照裡：`D:\GPIB9045\system\general.ini` 的 `[Version] Model` 要是 `9050GPIB`（網頁也可以加 `?machine=HT9050`）。只裝在開發機／模擬，不要裝到別台真機台。ctest 用的主表（`machines/HT9050/IO_Table.csv`、`Mot_Table.csv`…）來自同一份快照（NB2 MR !123，今晚跟筆電第四十六批一起上 main）。機台每次推新快照，筆電會更新並在這裡通知。 |
| 20261002 22:5x | 📌 W-11 結案（Jimmy 1002 22:4x，`RULINGS_20261002.md` 第 23 條第 15 題） | Jimmy：「一切用+-99999，拉最大，測試中先不要卡，我自行降速驗證功能」——測試期間軟體極限維持最大、不夾，HT9050 各軸行程／Teach 範圍暫時不用你提供了，謝謝。W-12（2×4 再確認）照舊等你。 |
| 20261003 07:4x | 📌 INDEXZ2 進 main（GitHub 第 130 包），跟你的 Index 流程有關 | Index Z1（M14 MTestZ1）的 `Gali_*` 命令改走 1203（`WB_ENGINE_INDEXZ_1203` 開；只在 `WB_PUMP_1203_START_RING` 也開時裝上，main 與機台都開）。流程端呼叫的還是同一組 `MOT[MTestZ1].Gali_*`，Index／Shuttle 流程的翻譯不用改；但 Z1 在機台上現在是真的會動的，被擋住時走 golden `ShowMotorErrorMessage`（停機＋煞車＋警報，INBOX 118）。你在對 Index 流程時如果需要 Z1 的實際高度，EastSun 那邊會回 TestZ1_Pick／Test／Place 哪些安全（TO_ES02 第 130 包那張卡）。 |
| 20261003 11:2x | ⓘ **工作檔更新了（RULINGS_20261002 第 21 條）** | 工作檔更新了（RULINGS_20261002 第 21 條）：`machines/HT9050/snapshot/`＝HT9050 機台 1003 10:48 的快照（GitHub `machine/integ-ioweb` `397aed9`，機台已套第 131 包），main `a9fd23be`，709 檔。跟測試主表（第四十八批 `5bfef4d9`）差 7 列 `Mot_Table.csv`：EastSun 上午在機台 Motor Test 回寫的速度（大多約 10 倍）與 **M14 MTestZ1 的 HomeDirectior 1→0**；主表等下一個 gate 過的批次再跟上。要在自己電腦上重現 HT9050 問題的，照 `machines/HT9050/snapshot/SNAPSHOT_SOURCE.md` 換上這一版（先備份）。 |
| 20261003 12:0x | ⓘ **協作規則兩條（RULINGS_20261003 第 15 條）** | **新測試請附「反向驗證」**（RULINGS_20261003 第 15 條，NB2 R183 提、Jimmy 同意）：MR 裡新增或改動的 ctest 檢查，說明請寫「故意改壞哪一行 → 哪個 CHECK FAIL」；起因是 R170／R171／R177／R180 都有「只驗測試自己的假物件、程式壞了也綠」的檢查。另：筆電現在也有心跳 `origin/v906/jimmy-heartbeat:HEARTBEAT.md`，超過 4 小時沒更新＋main 沒推＝筆電停了，你們照工作卡繼續做、不用等；St01 是備援整合者（第 14 條）。**通用工具都放 git**（第 16 條）：筆電的在 `tools/laptop_ops/`（gate、單獨重跑、MR 掃描、文件推送、心跳…，附 README），NB2 的在 `tools/nb2_assist/`；派工卡會寫用哪支。覺得哪支要改善，開分支＋MR，筆電評估後採用。 |
| 20261003 12:1x | 🫀 **請建心跳** | 🫀 **請建心跳**（RULINGS_20261003 第 17 條，Jimmy 1003 12:1x：「心跳線如果讓各人員建立完成後，回報目前人員上線狀況」）：每輪最後一步跑 `python tools/laptop_ops/heartbeat.py --who frank --doing "<這一輪在做什麼>" --next <下一輪時間> --push`（在任何一份 HT9045 checkout 裡跑；只寫分支 `v906/frank-heartbeat` 的一支 HEARTBEAT.md，不碰工作分支）；沒有迴圈、人工開的 session，開工跟收工各跑一次。建好後筆電的 `tools/laptop_ops/team_status.py` 就看得到你；全員狀況你也可以自己跑那支看。 |
