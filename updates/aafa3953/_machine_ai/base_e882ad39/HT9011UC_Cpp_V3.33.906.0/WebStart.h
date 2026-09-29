// =============================================================================
//  WebStart.h  --  瀏覽器 START 的 C++ 落點（ST-W0 骨架）
//
//  AI(W906-ST-W0) 20260915。計畫書 docs/START_CAMPAIGN_PLAN.md。
//
//  使用者 20260915 裁決：「我就是要讓它動，執行」。這一檔是那件事的容器，
//  但**它現在還是空的** —— ST-W0 只建接縫，一行翻譯都還沒有。
//
// =============================================================================
//  為什麼在樹根，不在 WebBridge/
// =============================================================================
//  `ht9045_webbridge` 只 link `ht9045_public`（CMakeLists.txt:2979），刻意不碰
//  god-stack —— 那是「零機台碼」的承重牆。這個檔要呼叫機台碼，所以跟
//  WebBridgeTags.cpp / WebAuth.cpp / WebBridgeRecipeDoc.cpp 同一家族：樹根的
//  TU，直接編進 `add_executable(wb_serve ...)`，不進任何 static library。
//  （順帶：因為是 add_executable 的直接來源而不是 archive 成員，
//    它不會踩到「編過但沒被抽出」的 archive-extraction 陷阱。）
//
// =============================================================================
//  ★★★ 為什麼**不** override TfMain::Start() ★★★
// =============================================================================
//  這是本檔最重要的一段。第一版設計是「TfMainWeb override Start()，
//  wb_serve 把 fMain 指過來」。**那是錯的，而且錯得很危險。**
//
//  ⚠⚠ AI(W906-DUET-REF) 20260921 重新量測。**結論完全存活；下面每一個行號都漂了，
//  而且原本的盤點有兩個實質錯誤。** 這是這棵樹第三次踩同一個坑（見
//  memory never-mechanically-shift-line-citations），所以這次連量法一起寫下來。
//
//  ⚠ **不要再用人工 grep 維護這張清單。** 這棵樹已經量錯三次，第三次是我自己：
//  20260921 我只 grep `fMain->Start(`，結果**漏掉 11 個** —— 那些 W7C1_FMAIN_START
//  呼叫裡，只有碰巧在行尾帶了 `// golden fMain->Start("...")` 註解的 6 個被命中，
//  沒帶註解的 DoOneCycleFinishCheck 5 個與 DoART_AfterCleanOut 6 個完全沒出現。
//
//  量法 ＝ 跑這支（它一次跑就抓到上面那個錯）：
//      python tools/start_sites_census.py
//      python tools/start_sites_census.py --check 34 30 4    ← CI 用（ctest START_SitesCensus），數字變了就紅燈（AI(W906-R80) 20260927：原為 33 30 3，HandlerGpibMsg.cpp:717 其實被同檔 `#define W906_REMOTE_START_WIRED 0` 閘住；AI(W906-SENSORSCAN) 20260924：原為 32 29 3，cSensorScan.cpp 新增一個活的呼叫點，見 :68）
//  它做兩件目視做不到的事：(1) 同時比對直接呼叫與包裝巨集，且**去掉註解後才比對**；
//  (2) 對每個候選**整檔線性累積 #if 0 深度**，而不是往回找最近的 #if ——
//  GATE G01 橫跨 212 行（4521-4733），往回看只會看到 4182 那個已經 #endif 掉的。
//
//  實測（20260921，工具產出）：生產碼共 **32 個**呼叫點（另有 tests/ 1 個不計），
//  其中 **3 個**在 #if 0 裡、**29 個是活的**。（AI(W906-R80) 20260927 重量：**34 個／活 30／閘 4** —— 第 4 個閘＝TesterComm/Handler/HandlerGpibMsg.cpp:717 GPIB START，工具現在把「同檔 #define 成 0 的巨集」也當 #if 0。AI(W906-SENSORSCAN) 20260924 重量：**33 個／活 30／閘 3** —— 多的那一個是 cSensorScan.cpp 的 fMain->Start("AMR")，見 :68；下方「29」的論證同樣適用於 30）
//
//  被閘住的 3 個（不是原本寫的 2 個）：
//    Command.cpp:16305              "RemoteControl Start"    SAFETY-GATE(W906-FW-CMD-C)
//    Command.cpp:17056              "TCP Command Start!!"    SAFETY-GATE(W906-FW-CMD-C)
//    SECSGEM/uHGemHT9045.cpp:4722   "SECS GEM RCMD : START"  ← **在 GATE G01 內**
//        ^ 原本的盤點把這一個列為「活的」第一個例子。它從 0b95c18（20260809，
//          PT-W5b）起就被 `#if 0 // GATE G01`（4521-4733）包住，比那份盤點早了
//          五週。Command.cpp 那兩個則是 ece9626（20260820）加的，也早於盤點 ——
//          所以不是「後來才變的」，是當時就沒量對。
//
//  活的 29 個，依觸發原因分六群：
//    SECSGEM/uHGemHT9045.cpp:5810 :5815     "SECS GEM RCMD : REMOTE_START"
//                            :7882 :7900 :7913  "SECS GEM RCMD : START[ Auto Start]"
//        ^ host 端下 RCMD 就能啟動機台。裸的 if(SystemStart==false) 包著，沒有 gate。
//    csystem.cpp:5807 5821 5914 5967 6003 6073   DoCleanOutFinishCheck 1-6
//        ^ Clean-out 完成後自動重啟。
//    csystem.cpp:7275 7368 7937 7966 7970        DoOneCycleFinishCheck 1-5
//        ^ One-cycle 結束後自動重啟。★ 20260915 與 20260921 兩次盤點都漏了這一群。
//    csystem.cpp:8601 8622 8641 8647 8664 8675   DoART_AfterCleanOut 1-6
//        ^ ART（Auto Retest）清機後自動重啟。★ 同上，兩次都漏。
//        （以上三群都經 W7C1_FMAIN_START 巨集，該巨集的註解自己寫
//          「the macros dispatch to the real fMain」）
//    ainarm9045.cpp:10717 10735 10796 10812
//        ^ InArm 取料錯誤後自動重啟。
//    Automation/uRENESAS_Server.cpp:1590 :1831、Automation/automation.cpp:1981
//        ^ AGV／自動化流程。  ＋ cSensorScan.cpp 的 fMain->Start("AMR")（TfMain::ProcessSensorScan，golden main.cpp:14251；KYEC AMR：TrayForm.bEnableAMR 且 RunInfo.bLotStart 且 bAMRReceiveStart；AI(W906-SENSORSCAN) 20260924 照翻加入，同樣落在基底空函式）
//
//  override 一次就把這 29 個全部武裝。那不是「讓網頁的 START 會動」，
//  那是「所有可能啟動機台的路徑同時開始運作」。
//  forms/fMain.cpp:276 那句 "W7-C1: offline do NOT auto re-start" 防的就是這個。
//
//  所以：`TfMain::Start()` **保持原樣**（virtual void，空函式，呼叫點照舊
//  全部惰性）。真正的啟動序列放在**新方法** `StartFromWeb()`，只有 wb_serve 的
//  指令分派會叫它。爆炸半徑剛好是一個呼叫者。
//
//  要讓 SECS/GEM 或 clean-out 自動重啟也能動，是**另外 29 個獨立決定**，
//  每一個都要單獨裁決。不要用一個 override 把它們一起打開。
//
//  ⚠ 但「逐一裁決 29 次」本身是個壞形狀，而且這張清單已經被量錯三次 ——
//  光是「維護一份正確的呼叫點清單」我們就證明了自己做不到。
//  20260920 的 Duet3D 借鑑分析（docs/DUET3D_REFERENCE_ANALYSIS.md 提案 P12）
//  指出結構解：DSF 讓**所有來源**的指令走同一條有 stage 的管線，
//  互鎖與操作權檢查放在 Pre stage 做一次。那才是這個問題的答案 ——
//  不是補 29 個 gate，而是讓它們全部經過同一個入口，清單就不必人工維護。
//  該提案標為要使用者裁決。
//
// -----------------------------------------------------------------------------
//  ⚠⚠ 還有第二條繞道：直接寫 SoftStart，根本不經過 Start()
// -----------------------------------------------------------------------------
//  20260915 ST-W6 稽核的新發現（**不是 ST-W6 造成的，是既有狀態**）：
//
//    SECSGEM/uHGemHT9045.cpp:5504 / :5538 / :7837 / :7919   `SoftStart = true;`
//    （AI(W906-DUET-REF) 20260921 重量：舊註解寫的 :5489/:5523/:7797/:7879 已漂，
//      現在分別指到一個 `}`、一個 `}`、一句註解、一個 `else` —— 照著看會看錯地方。
//      四處**全部仍是活的**。另有 :5885 `SoftStart=false;`。）
//
//  ⚠ 但 `SoftStart=true` 全樹 10 處，**不是 10 條啟動路徑**。它是一個共用的
//  加速斜坡旗標，逐一讀過的分類（tools/start_sites_census.py 第二份報告 + 人工判讀）：
//      SECSGEM 4 處        S2F42 host command 遠端啟動        ← 真的是遠端啟動
//      WebStart.cpp 3 處   我們自己翻譯的 StartFromWeb 落點     ← 就是這個落點本身
//      csystem 2 處        AccelateTask 斜坡（:16789 慢速 10 秒 → :16813 換生產速度）
//                                                            ← 不是冷啟動
//      forms/fMain.cpp 1 處 馬達上電後的 HOME 序列（下一行就是 iHome=1）← 不是冷啟動
//  ⇒ 引用這個數字時務必連分類一起引。工具只會數，語意要人讀。
//
//  那四行走的是 S2F42 host command 路徑。它們不呼叫 fMain->Start()，而是直接設
//  SoftStart —— 也就是說上面那套呼叫點盤點**盤不到它們**，要看第二份報告。
//
//  ⚠ 20260917 更正：下游今天是**斷的**。ckernel.cpp:816 `if(SoftStart==true)`
//  → :1015 `SystemStart=true` 確實存在，但 SoftStart 沒有活的讀者（收據在檔頭尾端）。
//  ⊙ 那四行仍然是地雷：接上那個 tick 呼叫的那一刻，它們就活了。
//
//  ⛔ **S3（武裝）之前必須先裁決這四行。** 不要在這個檔裡動它們 ——
//     那是 SECS/GEM 的地盤，且屬安全關鍵，要使用者在場。
//     這裡只負責讓它被看見，不讓它被忘掉。
//
//  ---- 20260917：「下游斷了」的收據 ----------------------------------
//  SoftStart 全樹唯一的讀者是 ckernel.cpp:816，在 ScanSystemSensor() 裡。
//  ScanSystemSensor() 只有兩個呼叫點，兩個今天都到不了那一支：
//    * ckernel.cpp:4010 —— 活的，但在 while(PopUpAlarm(...)) 警報彈窗迴圈裡，
//      而 :4009 前一行先做 SoftStart=false;
//    * csystem.cpp:782 —— **死的**。它同時在 `#ifdef DEBUG_TRY_CATCH` 和
//      csystem.cpp:598 那個 `#if 0 // GOLDEN VERBATIM`（收到 :2989）裡面，
//      **承重的是外層那個 `#if 0`**。:599 的 MainProc() 是被閘掉的那份；
//      活的 MainProc() 在 :3003，它自己用 `#if 0 // TODO(W7)` 明列
//      ScanSystemSensor 未翻，只呼叫 DoAllProcess / DoOneCycleFinishCheck /
//      DoCleanOutFinishCheck。
//  ⇒ S3（武裝）還缺一個每個 tick 的 ScanSystemSensor() 呼叫，
//    不是「只差一個旗標」。
//
// =============================================================================
//  fMain 要不要重指
// =============================================================================
//  要。golden 的 Start() 本體是 TfMain 的成員函式，直接寫 `spbUserName->Caption
//  = "Operator"` 這種語法，動的是 fMain 自己的狀態。如果 TfMainWeb 是另一個物件，
//  那些寫入就落在一份沒人看的副本上。
//
//  所以 S3（武裝）那一步要做的是 `fMain = &theWebImpl`，**但不 override
//  Start()** —— 同一個物件、同一份狀態，而 29 個活的舊呼叫點拿到的仍是基底的空函式。
//
//  ⚠ ST-W0 **不提供**重指函式。重指屬於 S3，而 S3 要使用者站在機台旁邊。
//    這一波結束後，這個類別沒有任何人建構、沒有任何人呼叫。
// =============================================================================
#ifndef WEBSTART_H
#define WEBSTART_H

#include "forms/fMain.h"

// -----------------------------------------------------------------------------
//  TfMainWeb -- 照 W7 架構計畫 D3 的 `TfXxxImpl : public TfXxx` 形狀。
//  D3 原本規劃的 impl 在 MFC 那一側（ui/FormsFacadeMfc.cpp），但 MFC 已自產品
//  移除（7b86cfd），而現在的 UI 是 web。所以 impl 在這裡。
//
//  FACADE CONTRACT（forms/fMain.h 檔頭）第 1 條說得很清楚：
//  forms/fMain.cpp 的空函式是 PERMANENT OFFLINE IMPLEMENTATION，不是鷹架；
//  真實實作用 impl 子類覆寫。這個類別就是那個 impl —— 只是它**刻意不覆寫
//  Start()**，理由見上面那一大段。
// -----------------------------------------------------------------------------
class TfMainWeb : public TfMain
{
public:
    TfMainWeb() : TfMain() {}
    virtual ~TfMainWeb() {}

    // ---------------------------------------------------------------------
    //  golden bool __fastcall TfMain::Start(AnsiString Func)
    //      main.cpp:4385-6259（1,875 行）
    //
    //  ★ 這**不是** override。名字刻意不同，就是為了不讓那 29 個活的
    //    `fMain->Start(...)` 綁到這裡來（數字見檔頭；用
    //    tools/start_sites_census.py 重量，不要人工 grep）。
    //
    //  回傳值照 golden：true = 啟動流程走完；false = 被某個檢查擋下。
    //  golden 有十幾條 early-return false 的路徑，回傳值是承重的
    //  （移植樹現有的呼叫點沒有一個看回傳值 —— 20260921 重驗：對全部 32 個
    //    呼叫點（含被閘住的）搜 `if/while/return/=/&&/|| /!` 帶 Start 呼叫的樣式，
    //    命中 0 筆，全部是獨立敘述 —— 所以基底那支維持 void 不需要動，改動面積是零）。
    //
    //  ST-W0 現況：**本體是空的**，一律回 false。
    //  進度看 W906_StartTranslatedLines()，不要看這段註解 —— 註解會過期。
    // ---------------------------------------------------------------------
    bool StartFromWeb(AnsiString Func);

    // ---------------------------------------------------------------------
    //  T3：golden bool __fastcall TfMain::Pause(AnsiString Func)
    //      main.cpp:6325-6379（55 行），kevin 20141108
    //
    //  ⚠ 跟 StartFromWeb 一樣，這是**新方法**，不是 TfMain::Pause() 的 override。
    //  理由同 ST 戰役那條，而且這裡的數字更大：forms/fMain.cpp:233 記著
    //  BtnPauseClick 之外還有 **~40 個狀態機呼叫點**。把 golden 的本體填進基底
    //  那支，等於一次把那 40 個點全部武裝成「真的會 SoftStop=true; StopAllMotor();」。
    //  基底 TfMain::Pause() 維持 golden-faithful 的 `return false;`（離線永不暫停）。
    //
    //  鏈路（與 START 對稱）：
    //      START  SoftStart=true -> ckernel.cpp:816  -> :1015 SystemStart=true
    //      PAUSE  SoftStop =true -> ckernel.cpp:1046 -> :1050 SystemStart=false
    //  兩邊都由 pump tick（PumpTick -> MainProc）驅動，所以 SystemStart 是
    //  **下一個 tick** 才翻，不是這支回來就翻。
    // ---------------------------------------------------------------------
    bool PauseFromWeb(AnsiString Func);

    // ---------------------------------------------------------------------
    //  ST-W7-A：golden bool TfMain::CheckSLKSensor()
    //      main.cpp:32424-32500（77 行），JerryYang 20160712
    //
    //  為什麼放在 TfMainWeb 而不是 MainCalcCore：
    //    MainCalcCore.h:113-118 自己寫明它「STILL OUT OF SCOPE」，理由是它讀
    //    全域 Cylinder[] 硬體陣列（ht9045_sm substrate），而 MainCalcCore 是
    //    刻意保持 UI-free / globals-free 的 TU。**那個理由對 MainCalcCore 成立，
    //    對這裡不成立** —— WebStart.cpp 是 add_executable(wb_serve) 的直接來源，
    //    本來就連著整個 god-stack，Cylinder[] 就在手上（mycylin.h:176 宣告、
    //    mycylin.cpp:41 定義）。所以「不能放那裡」不等於「不能翻」。
    //
    //  也不放進 forms/fMain.cpp：計畫書 §3.5 的裁決 ——
    //  facade 的空函式是 PERMANENT OFFLINE IMPLEMENTATION，不是鷹架。
    //
    //  ⚠ 這**不是** override：`TfMain` 沒有宣告過 CheckSLKSensor，
    //    全樹也沒有第二個呼叫點（實測：只有 WebStart.cpp 自己）。
    //    所以爆炸半徑仍然是一個呼叫者。
    // ---------------------------------------------------------------------
    // ---------------------------------------------------------------------
    //  T5/W4-E：golden bool TfMain::NETDownloadDataCheck()
    //      main.cpp:30425-30792（368 行），Joye/wei 2013-12-03 KYEC FTP
    //
    //  ⚠ **擋啟動**：ON_LINE 時 FTP 下載的配方與本機配方比對不過就不讓開機台。
    //  呼叫點在 StartFromWeb（golden :5550-5563），本身包在
    //  `#ifndef SOFT_SIMULTE` 裡 —— 也就是模擬組態下不會被呼叫，
    //  只有出貨組態會走到。
    // ---------------------------------------------------------------------
    bool NETDownloadDataCheck();

    bool CheckSLKSensor();

    // ---------------------------------------------------------------------
    //  ST-W7-B：golden bool TfMain::CheckInOutArmZHomeSensor()
    //      main.cpp:32339-32376（38 行），Frank 20160612
    //
    //  ⚠⚠ 這一支跟 CheckSLKSensor **不同級**：它裡面有
    //      MOT[MInArmX].PCIL132_StopMotor();
    //      MOT[MInArmY].PCIL132_StopMotor();
    //    那是**真的運動指令** —— 移植樹 Motor/mymotor.cpp:1278 的本體會走到
    //    `Motor->DecStop()`（減速停止），不是樁。
    //
    //  為什麼還是照翻，而不是把 StopMotor 那兩行閘掉：
    //    golden 的語意是「偵測到 Z 軸原點感測器不對 -> **先把手臂停下來** -> 報訊息 -> 回 false」。
    //    閘掉 StopMotor 會變成「偵測到異常卻讓手臂繼續跑」——
    //    那是 **fail-dangerous**，比整段不執行更糟。
    //    本戰役的規矩是「閘掉『做不到的事』可以，替 golden『選一條路』不行」；
    //    這裡兩個相依都在、都能連，沒有做不到的事。
    //
    //  ⛔ **S3（武裝）之前必須把這一條放進審查清單**：一旦 StartFromWeb() 被掛上
    //    分派，Z 軸原點感測器異常就會讓 InArmX / InArmY 收到停止命令。
    //    那是正確且安全的方向（停，不是動），但它**是一個真的機台動作**，
    //    不可以在使用者不在場時第一次被觸發。計畫書 §6 有列。
    //
    //  ⚠ 同樣不是 override：`TfMain` 沒有宣告過它，全樹也沒有第二個呼叫點。
    // ---------------------------------------------------------------------
    bool CheckInOutArmZHomeSensor();

    // ---------------------------------------------------------------------
    //  ST-W7-C：golden bool TfMain::CheckOLPError()
    //      main.cpp:33986-34012（27 行），Sam 20230921「Bin 設定錯誤不能啟動」
    //
    //  ★ 這一支是**部分**翻譯，而且是刻意的：
    //    - **判定**（有沒有任何 OLPSetBinErr[i] >= 3）→ 翻，用移植樹既有的
    //      `ComputeCheckOLPErrorHasErr()`（MainCalcCore.h:404）。
    //    - **三個對話框**（golden 對 i==0 / 1 / 2 各發一個）→ 閘掉，
    //      因為 `ShowMyMessagePWD`（golden mymessbox.h:52）全樹未移植
    //      （`PowerSavingMode.cpp:937` 就是為同一個理由閘住的）。
    //
    //  ⚠⚠ 這樣做的後果要講清楚：解閘之後，OLP Bin 設定有錯時
    //    **機台會拒絕啟動，但操作員看不到任何訊息**。
    //    我仍然選擇這樣，理由是本戰役自己的判準：
    //      閘著 = fail-open（設定錯了照樣跑，會出壞品）
    //      解閘 = fail-safe（不跑，但沒說為什麼）
    //    ST-W5 的稽核已經因為 fail-open 修正過一次分類（W5-B），同一個標準。
    //
    //  ⛔ **S3 之前必須把 `ShowMyMessagePWD` 補上**，否則現場會遇到
    //    「按 START 沒反應、也沒有任何提示」。這一條進 §6 的 S3 必審清單。
    // ---------------------------------------------------------------------
    bool CheckOLPError();

    // ---------------------------------------------------------------------
    //  ST-W7-G：golden void TfMain::SET_ESD_Tri_Temp(int)
    //      main.cpp:34528-34550（23 行），Ztex 2023.04.19 Add HT-1032 TriTemp
    //
    //  ⚠ 它會**對外送命令**（`SendCommand_ESD`）。之所以仍然照翻而不歸類成
    //    「對外動作、不能翻」，是因為**這條通道在本檔已經是活的**：
    //    `WebStart.cpp` 的活碼裡已有 4 個 `SendCommand_ESD` 呼叫
    //    （`:510` `ESD_TestOFFLine` / `:512` `ESD_TestOnLine` /
    //      `:2581` 與 `:2756` `ESD_SYSTEM_START`），全部不在 `#if 0` 內。
    //    再加第 5 個與本戰役既有立場一致，不是新開一條對外通道。
    //
    //    ⚠ 對照 W1-G（`CheckEmployeeID`）：那一支之所以**不能**翻，
    //      是因為它的對外訊息**攜帶 ShowModal 收集不到的憑證** ——
    //      送出去的內容是錯的。這裡送出去的是 `Temperature.fWorkTemperBase`，
    //      一個本檔別處已經在讀的真實值。**差別在「內容對不對」，不在「有沒有對外」。**
    // ---------------------------------------------------------------------
    void SET_ESD_Tri_Temp(int iTemperature);
};

// -----------------------------------------------------------------------------
//  已翻譯的行數 / golden 總行數。讓「翻到哪了」是可量的，不是靠讀 commit 訊息。
//  ST-W1..W6 每完成一波就更新 WebStart.cpp 裡的那個常數。
// -----------------------------------------------------------------------------
int W906_StartTranslatedLines();
int W906_StartGoldenLines();
// ST-W7 還債的狀態：0=未開始 1=進行中 2=卡住(等裁決) 3=完成
// /st-wave 用它決定要不要交棒給 /night-loop。2 與 3 都停，但意義不同。
int W906_StartWave7State();

#endif // WEBSTART_H
