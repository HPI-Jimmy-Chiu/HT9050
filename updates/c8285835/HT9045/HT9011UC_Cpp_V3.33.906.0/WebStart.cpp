// =============================================================================
//  WebStart.cpp  --  瀏覽器 START 的 C++ 落點
//
//  AI(W906-ST-W0) 20260915：接縫。理由與邊界全部寫在 WebStart.h 檔頭，不重複。
//  AI(W906-ST-W1) 20260915：golden main.cpp:4385-4693（309 行）忠實翻譯。
//
//  計畫書：docs/START_CAMPAIGN_PLAN.md（§5 是進度的單一出處）。
//
//  ⚠⚠ 這個函式一旦被呼叫，機台就會動。
//     MachineType.h:48 的 SOFT_SIMULTE 是註解掉的，CMakeLists.txt:1292 明寫這個
//     build 不定義它，而 wb_serve 連了完整 god-stack（CMakeLists.txt:3016 的
//     LINK_GROUP 含 ht9045_motor / ht9045_io）。沒有模擬層擋在中間。
//
// -----------------------------------------------------------------------------
//  §S3 —— ⚠⚠ 本檔已經武裝了。讀下面任何一句「行為 delta = 0」之前先看這一段。
// -----------------------------------------------------------------------------
//  AI(W906-S3-CLAIM-VOID) 20260921 23:5x：
//
//  這個檔頭原本寫著「ST-W1 結束時**仍然沒有任何人呼叫它** —— 接上分派是 S3」，
//  本檔內另有 **12 句**同樣意思的（已全部就地標註作廢，見下文）。
//
//  **那 13 句自 20260918 起全部是假的。** 當場量到的事實：
//
//      tools/wb_serve.cpp:1990   fMain = g_webMain;        <- 無條件，啟動時就做
//      tools/wb_serve.cpp:3066   g_webMain->StartFromWeb(who)
//                                <- web 的 `start.run` 指令真的呼叫它
//
//  ⇒ 瀏覽器按 START **會執行本檔完整的 1,875 行翻譯**。
//
//  ★ 為什麼這件事值得在檔頭用這麼大的篇幅講：
//    那 13 句不是**描述**，是**授權**。「行為 delta = 0」這句話的用途，
//    就是讓人（包括未來的我）放心地解掉一個閘 —— 理由是「反正沒人呼叫，
//    解了也不會發生任何事」。武裝之後，那個理由不成立了，
//    但句子還留在原地繼續發放它已經無權發放的許可。
//
//  ⇒ 下面每一處原本的宣稱都已就地標註作廢。**解閘之前要重新論證，
//    不可以再引用「行為 delta = 0」。**
//
//  ⇒ 機制面：tools/start_sites_census.py 新增了一條一致性斷言
//    （武裝狀態 vs 樹上的宣稱），兩者矛盾就回 1。它原本看不到這件事，
//    因為它的 EXCLUDE_PREFIXES 排除了 tools/ —— 而武裝就住在 tools/ 裡。
// =============================================================================
// AI(W906-ST-W5) 20260915：ATC/ATCInterface.h 【不能】include 進來。
//   它的 :170-174 做 `using vclcompat::clWhite;` 等九個顏色常數，
//   而 acatchtray_shims.h:127（以及 vclcompat/{LedCore,BtnPanelCore,TrayCore}.h）
//   是 `const TColor clWhite = ...`。兩者在同一個 TU 裡**怎麼排都撞**：
//     ATC 在後 -> ATCInterface.h:174 "clWhite is already declared in this scope"
//     ATC 在前 -> acatchtray_shims.h:127 "redeclaration of const TColor clWhite"
//   這是樹裡**既有的標頭衝突**，不是缺符號。不用調順序去繞（繞不過），
//   也不在本波改別人的標頭（那是跨檔編輯）。ATC 區塊一律閘掉，見 W5-G。

#include "WebStart.h"

#include "cmydef.h"                 // LogSoftwareOnTime, CUSTOMER_CODE, AccessLevel, SW[]
#include "cprod.h"                  // TestIF_File, LastSet, IniConfig, CosFunction
#include "common.h"                 // GetRecipePath
// ⚠ 不要同時 include cMyDB.h：它的 :129/:130 也宣告 NewRecordProcess / RecordProcess
//   **並且各自帶預設參數**，跟 acatchtray_shims.h:384 與 canary_support.h:70 撞成
//   "default argument given for parameter N"。兩邊都是同一個函式的宣告，
//   取其一即可：NewRecordProcess 走 acatchtray_shims.h（真正的本體在
//   acatchtray_shims.cpp），RecordProcess 走 canary_support.h。
#include "canary_support.h"         // ShowErrorMessage, RecordProcess
#include "cinitial.h"               // CompareTechData
#include "csystem.h"                // CheckFixTray
#include "acarry.h"                 // ResetShtMoveTimeoutWatchdog
#include "ainarm9045.h"             // AutoCalculateInArmYClosePitch
#include "Interface/InterfaceSYS.h" // SendCommand_ESD
#include "SECSGEM/SecsEventReport.h"  // EventReport（:55）—— T3 PauseFromWeb
#include "SECSGEM/SecsEventType.h"    // SECS_EVENT.DoPause（:44）—— T3 PauseFromWeb
#include "forms/fTemp_Set.h"        // fTemp_Set->InitialAddrToATC
#include "forms/fLotInfo.h"         // fLotInfo->bStartChamberBoost / CheckEventLogParameter
#include "forms/fMesSystem.h"       // fMesSystem（:704）—— T5/W4-E 的 CheckVTENGmode（:655 ACTIVE）
#include "forms/fTrayMapping.h"     // fTrayMapping->IsRunDeviceRemainLaser
#include "forms/fHotPlate.h"        // fHotPlate->CheckHotPlate8PickMode
#include "forms/fContact.h"         // fContact->fShow
#include "acatchtray_shims.h"       // fTrayMapping（golden cTrayMapping.h 的 extern 在這裡，:313）
#include "myswitch.h"               // SW[MAX_SWITCH_ITEM]（:43）
// ⚠ FTestSuck 有**兩個**標頭宣告它：mykitsuck.h:458 與 aHotPlateSubstrate.h:636，
//   而 TMyKitSuck 這個 class 在兩邊的**佈局不同**（mykitsuck.h:274 vs
//   aHotPlateSubstrate.h:365）。選錯會乾乾淨淨地連起來，然後每個欄位讀錯偏移。
//   真正的定義在 aHotPlateSubstrate.cpp:92（`TMyKitSuck FTestSuck;`），
//   aTester_Rear.cpp:11985 也記了同一件事。所以用這一個，不要改成 mykitsuck.h。
#include "aHotPlateSubstrate.h"     // FTestSuck
// AI(W906-ST-W2) 20260915 更正 ST-W1 的一個錯：
//   ST-W1 在本檔自己定了 MSG_CMD_* 六個常數，理由寫「golden MessageDef.cpp
//   尚未移植」。**那是錯的** —— MessageDef.h/.cpp 都在樹上（CMakeLists.txt:497），
//   而且 MSG_CMD_EnableBarCode 實測就在 build/libht9045_globals.a 裡。
//   當時的 grep 樣式沒配到 `extern const unsigned int MSG_CMD_EnableBarCode     ;`
//   （分號前有空白）就下了結論。
//   後果會是：對外協定的訊息代碼有兩個出處，而那正是我在 ST-W1 註解裡自己警告過的
//   漂移風險。已改用共用定義。
#include "MessageDef.h"             // MSG_CMD_*, GPIBVersionCheck, TTLRS232VerCheck
#include "mysensor.h"               // Sen[MAX_SENSOR_ITEM]（:48）
#include "atester_shims.h"          // fContact（:251，forms/fContact.h 刻意不宣告它）
#include "forms/fOffSet.h"          // fOffSet（:361）
#include "forms/fSCKART.h"          // fSCKART（:177）
#include "forms/fTesterTCP.h"       // fTesterTCP（:408）
#include "Automation/SCK_ART_Remainder.h"  // FormHS（:629，forms/fHS.h 刻意不宣告它）
// AI(W906-ST-W3) 20260915: CONTACT_NORMAL（:158）。
// 它在 BarCode/BarCode_Shuttle2_CCDScan.h:187 也有一份同值的定義；
// 兩邊都是 namespace scope 的 const int，內部連結，不會 ODR 相撞。
#include "cContact.h"               // CONTACT_NORMAL

// AI(W906-ST-W4) 20260915: iInArmWaitPosition 沒有任何標頭宣告它 ——
//   golden 的家是 ainarm2.h:219，但移植樹的 ainarm2.h **沒有**搬進來，
//   而是由 ainarm2.cpp:805 就地 `extern int iInArmWaitPosition;`。
//   實測符號確實存在（nm: `B _iInArmWaitPosition` in libht9045_sm.a），
//   所以照同一個前例就地宣告，不去改 ainarm2.h（那會是跨檔編輯）。
extern int iInArmWaitPosition;      // golden ainarm2.h:219（前例：ainarm2.cpp:805）

// AI(W906-ST-W5) 20260915
// AI(W906-ST-W5) 20260915：forms/fMesSystem.h 【不要 include】——見 W5-H。
//   引用它會把 fMesSystem.cpp 拉進連結，而那支 TU 自己帶著一個
//   未解符號 bNoRTBinFixFlag（全樹沒有定義）。
#include "forms/fShuttleMove.h"     // fShuttleMove（:109）
#include "acatchtray.h"             // MTrayXCanSafeMove（:47）
#include "aoutarm.h"                // CheckOutArmZ（:116）

// AI(W906-ST-W6) 20260915
#include "forms/fBinSel.h"          // fBinSel（:745）
#include "forms/fNote.h"            // fNote（:421）
#include "forms/fShowBinSelect.h"   // fShowBinSelect（:1133）

// AI(W906-ST-W7-A) 20260915
#include "mycylin.h"               // Cylinder[] / TMyCylinder::OnSensor（golden :32434 起）

// AI(W906-ST-W7-B) 20260915
#include "Motor/mymotor.h"          // MOT[]（:385）/ ReadPos / ScanMotorStatus / Led[] / NumberAlias / PCIL132_StopMotor
#include "Motor/HTMotor.h"          // iHomeLed（:42，enum ==1）與 HTMotor::Enable

// AI(W906-ST-W7-C) 20260915
#include "MainCalcCore.h"          // ComputeCheckOLPErrorHasErr（:404）—— 既有的純判定，不重寫一份
#include "adam6024.h"              // AI(W906-P2b-CF) 20260919: TransformFuntion（W6-G 解閘）

// AI(W906-ST-W7-D) 20260915
#include "forms/fCleaning.h"       // fCleaning（golden 的呼叫點是 fCleaning->...，保留形狀用）

// AI(W906-ST-W7-H) 20260915
#include "BarCode/BarCode_Bottom2DID.h"  // list2DByLot（:144）/ listError2DID（新增）
//   ⚠ InArmSuck / OutArmSuck 來自上面已 include 的 aHotPlateSubstrate.h:624/:627，
//     **不是** mykitsuck.h:452 —— 陷阱 #1：兩個標頭的 TMyKitSuck 佈局不同，
//     選錯會乾乾淨淨地連起來然後每個欄位讀錯偏移。
//   全樹只有一個 `class TMyCylinder`（mycylin.h:44），沒有陷阱 #5
//   的同名雙標頭問題 —— 實測過才 include。

// AI(W906-P6b-B) 20260921
#include "WebWindowRegistry.h"     // WebWindowRegistryFShowPolicy —— 取代 fShow 的那一支

// =============================================================================
//  P6-b 區塊 B：把「表單開著嗎」從行程內的 bool 換成「行程內的 bool ∪ 瀏覽器說的」
//
//  AI(W906-P6b-B) 20260921
//
//  ## 為什麼需要
//
//  golden 用 `fXxx->fShow` 問「有沒有人正在用某個診斷畫面」。移植樹的 UI 是網頁，
//  沒有人去維護那些 `fShow` 成員的值 ⇒ 今天每一個判斷讀到的一律是 false，
//  也就是「永遠沒人在教導」。那不是翻譯忠實度問題，是**訊號來源不存在**。
//
//  ⚠ 精確一點（20260921 逐支量的，我第一版寫成「全樹零個 `fShow = true`」是錯的）：
//
//  | 表單 | `fShow` 的寫入點 |
//  |---|---|
//  | `fContact`      | 只有 `forms/fContact.cpp:461 fShow=false;`（初值，golden :237） |
//  | `fShuttleMove`  | **一個都沒有** |
//  | `TrayEditForm`  | 一個都沒有 |
//  | `Zteach`        | 一個都沒有 |
//  | `fTemp_Set`     | **有一個活的**：`uTemp_Set.cpp:1153 fShow=true;` |
//
//  `fTemp_Set` 那一個**沒有被閘住、有進建置**（`CMakeLists.txt:2404`），
//  它在 `TfTemp_Set::FormShow`（`uTemp_Set.cpp:651`）裡。
//  今天它仍然不會執行，但理由是**第三級**不是第一級 ——
//  `TfTemp_Set::FormShow` 全樹**零個生產呼叫點**（只有測試與註解提到它）。
//  （memory: `symbol-exists-has-three-strengths`）
//
//  ⇒ 所以下面用**聯集**不是取代，不只是防禦性寫法：哪天有人把 `FormShow` 接起來，
//     `fTemp_Set->fShow` 就會變成一個**真的**訊號，取代式寫法會把它靜靜丟掉。
//
//  區塊 A 已經把訊號來源做好了（`WebWindowRegistry`，瀏覽器如實回報 ＋ 兩個裁決）。
//  這一塊只做一件事：把讀取點接上去。
//
//  ## ⚠ 用「聯集」不是「取代」
//
//      W906_FShow(form, member) == member || 瀏覽器說開著
//
//  兩個來源都可能是對的，而且都往「比較擋得住」的那一側倒：
//    * `member` 今天恆 false，但日後若有人真的去寫它（例如 MFC harness），
//      它就是行程內的事實，不該被網頁的答案蓋掉。
//    * 瀏覽器那一側是現在唯一活的來源。
//  取聯集與總表自己「任何一條連線說開著就是開著」的規則同向。
//
//  ## ⛔ 一定要用 FShowPolicy，不是 FShowConservative
//
//  `FShowConservative()` 是**純契約**：不可知 -> 開著。直接用它會讓
//  「沒開瀏覽器就不能啟動機台」變成硬規則 —— 那正是使用者 Q8 裁決 B 否決掉的。
//  `FShowPolicy()` 多套了兩層：
//    Q8-B  從來沒收過任何總表 -> 一律回關著（開機、沒有瀏覽器時不擋）
//    Q20-甲 瀏覽器不會回報的表單 -> 一律回關著（否則永遠擋住且無法解除）
//
//  ## ⓘ 這一塊今天在這台機器上是**惰性**的（量過，不是推論）
//
//  `D:\HT9045\web\background.html` 目前**沒有**任何 `HT9045Windows` / `ui.windows.put`
//  （`grep -c` = 0）⇒ 沒有人送總表 ⇒ `WebWindowRegistryEverAnyFrame()` 恆 false
//  ⇒ `FShowPolicy()` 對每一個表單都回 false ⇒ 每個判斷的值與接上去之前**完全相同**。
//  要它真的生效，得先把 Steven 20260919 交付包裡的 `background.html` 部署上去
//  —— 那是使用者的決定（`docs/INBOX_QUEUE.md` Q23）。
//  ⚠ 也就是說：**部署那一份網頁的當下，START 才第一次可能被視窗擋住。**
//     現場解除方式＝把那個網頁視窗關掉。
// =============================================================================
static bool W906_FShow(const char* goldenForm, bool member)
{
    return W906_FormShowing(goldenForm, member);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 改問單一函式（csystem.h:440：成員 || 頁面表規則 1～7）。已接總表的 19 處 START 判斷一起轉過來；上面那段的 Q8-B／Q20-甲由頁面表規則 3／2 承接，★ 沒回報／absent／過期且沒有 WebSocket 改成「關」（Steven Q-P1 20260928）
}

// -----------------------------------------------------------------------------
//  進度計量
//
//  為什麼是函式不是註解：註解會過期，而這棵樹的規則是「引用任何百分比都必須附
//  分母與單位」。這兩個回傳值進得了 ctest 與 wb_serve 的啟動訊息，
//  所以它不會偷偷變成謊話。
//
//  golden bool __fastcall TfMain::Start(AnsiString Func)
//      D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\main.cpp:4385-6259 = 1875 行
// -----------------------------------------------------------------------------
static const int kGoldenLines = 1875;

//  ST-W1 交付 main.cpp:4385-4693 = 309 行。
//  ST-W2 交付 main.cpp:4694-5044 = 351 行。累計 660。
//  ST-W3 交付 main.cpp:5045-5345 = 301 行。累計 961。
//  ST-W4 交付 main.cpp:5346-5646 = 301 行。累計 1262。
//  ST-W5 交付 main.cpp:5647-5958 = 312 行。累計 1574。
//  ST-W6 交付 main.cpp:5959-6259 = 301 行。累計 1875 = **golden 全長**。
//  翻譯部分到此完成。剩下的是 ST-W7：補 §5.1 的 gate（那是還債，不增加行數）。
static const int kTranslatedLines = 1875;

// -----------------------------------------------------------------------------
//  ST-W7（還債）的狀態。理由同上：**可量，不是靠記憶或文件**。
//  /st-wave 用它決定「這個戰役還能不能自動往前」，能不能就交棒給 /night-loop。
//
//    0 = 未開始
//    1 = 進行中（§5.1 還有「翻一個小函式就能解」的 gate）
//    2 = 卡住（剩下的 gate 全部需要使用者裁決，自動迴圈做不動了）
//    3 = 完成（§5.1 的 🔴 全部補完或明確裁決過）
//
//  ⚠ 2 和 3 都代表「/st-wave 不要再跑了」，但意義不同：
//    3 是做完，2 是做不下去。回報時不可以混為一談。
// -----------------------------------------------------------------------------
// AI(W906-ST-W7) 20260915: 1 -> 2。**自動迴圈做不動了，不是做完了。**
//   ST-W7 交付了 6 個（W5-A / W2-G / W1-E / W2-C / W6-H / W1-C）。
//   剩下 43 個 gate 全部需要使用者裁決或屬於別的戰役 —— 逐類清單見計畫書 §5.6。
//   ⚠ 寫 2 不是寫 3。§5.1 的還債清單**沒有清空**。
//
// AI(W906-ST-S2-fShow-B1) 20260917: 2 -> 1。**使用者裁決把它解開了。**
//   那個 2 的意思是「剩下的 gate 全部要使用者裁決」。
//   20260917 使用者就 fShow 那一家族的設計題裁決了（第 ⑧ 題，「開工」），
//   所以那個前提消失了。
//
//   ⚠ 改成 1 是**照定義**，不是為了讓自動迴圈跑得下去：
//   上面寫著 1 = 「§5.1 還有〈翻一個小函式就能解〉的 gate」。
//   實測確實如此：W6-D（:6154-6159）與 W6-E（:6169-6173）現在只差
//   一個 `RunCheckStart()` —— 全樹沒有它（csystem.cpp:9755 那個 gate 的
//   註解自己寫明「no member of either name exists on the FormsFacade TfMain」），
//   翻它一個就解兩個閘。
//
//   ⚠ 還債清單依然**沒有清空** —— 1 不是 3。本波只解了 W2-B（TrayEditForm）。
//
// AI(W906-ST-W7-RCS) 20260917 下午: 1 -> 2。**上午改成 1 的理由已經用完了。**
//   那個 1 的依據是「§5.1 還有翻一個小函式就能解的 gate」，
//   指的是 RunCheckStart()。**它翻完了**（本檔 forms/fMain.cpp 檔尾，W6-D / W6-E 已解）。
//
//   20260917 下午逐個量過剩下的 8 個，**沒有一個是自動迴圈做得動的**：
//     W5-F x2  成員齊了，但 fShow==false 在這裡是**打開保護**那一支 ⇒ 解開會
//              **新增一個擋啟動**（ASE 高雄/OP/OFF_LINE），而且 :2411 註解記著
//              下一支原本是 else if、被閘掉才變裸 if ⇒ 還要做鏈結構還原
//              → 20260923 已解（使用者裁決，依 §0.5；AI(W906-ST-W7-W5F)），else if 已還原。
//                上面兩點是後果與工作量，不是 §0.5 認的「相依不存在」。
//     W6-A     差 fTeach（全域刻意留 NULL，SIOF 迴避，forms/fTeach.cpp:67）
//     W6-B     ⚠ 通訊／硬體路徑（golden 那 8 行會開實體序列埠，見它的 gate 註解）
//     W6-C     ⚠ **擋啟動**，而且內層走不能 include 的標頭
//     W6-I     golden 的無參數多載沒有，替代品回傳極性**相反**
//     W6-L     fNote->t2DCode 是 BarCode 戰役也在等的缺件（BarCode_Shuttle2_ScanRemainder1.h:186）
//     W6-F     [RED] 解開後 SoftStart 會被設成 true
//     W2-J / W1-D  缺的遠不只一個成員
//
//   ⇒ 寫 2 不是把它改難看，是**不讓狀態碼說謊** —— 這個碼存在的唯一目的，
//   就是讓 /st-wave 知道該不該繼續。維持 1 會讓它每 25 分鐘查一遍、
//   回報「沒有不需裁決的工作」、結束 —— 那是空轉，不是進展。
//   ⚠ 仍然是 2 不是 3：還債清單**沒有清空**。使用者裁決（20260917）。
static const int kWave7State = 2;

int W906_StartGoldenLines()     { return kGoldenLines; }
int W906_StartTranslatedLines() { return kTranslatedLines; }
int W906_StartWave7State()      { return kWave7State; }

// =============================================================================
//  bool TfMainWeb::StartFromWeb(AnsiString Func)
//      golden bool __fastcall TfMain::Start(AnsiString Func)  main.cpp:4385
//
//  ⚠ 絕對不可以用「寫一行 SystemStart = true」代替整個函式。
//    那會跳過這 1,875 行檢查，也跳過 golden ckernel.cpp:526-531 的安全門互鎖
//    （門沒關就把 SystemStart 設回 false 並 StopAllMotor()）。
//    順帶：`bSystemStart` 不是啟動旗標 —— 它是 ComputeCanChangeToSocket() 的
//    傳值參數，意思是「已在運轉時不允許切換 Socket」。真正的全域在 cmydef.cpp:286。
//
//  ST-W1 只翻到 golden :4693。函式尾端因此**提早 return false**，
//  代表「檢查還沒跑完，沒有啟動」—— 與 golden 被某個檢查擋下時的回傳值一致。
//  這裡刻意不是「翻到哪裡就回 true」：一個會謊報成功的樁比沒有樁更糟。
// =============================================================================
// =============================================================================
//  TfMainWeb::CheckSLKSensor  --  golden TfMain::CheckSLKSensor()
//      main.cpp:32424-32500（77 行）  JerryYang 20160712 檢查 SLK sensor 改為函式
//
//  AI(W906-ST-W7-A) 20260915：還 §5.1 的 W5-A（🔴）。
//
//  ★ 行為 delta = 0：唯一的呼叫點在 StartFromWeb()，而 StartFromWeb() 全樹
//    零個呼叫者（S3 尚未武裝）。這是翻譯，不是解閘一段活的碼。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。上面那句不再成立。
//
//  相依逐一實測（20260915）：
//    TestIF_File.bUseSLKClamp / iSeparabilityTest   cprod.h:2175 / :2176
//    TestIF_File.iShuttleMode / iShuttle_Sel        cprod.h:1654 / :1655
//    Cylinder[]                                     mycylin.h:176（定義 mycylin.cpp:41）
//    C_SLK1_Clamp / _Unclamp / C_SLK2_*             cmydef.h:358-361（定義 cmydef.cpp:416-419，未被閘）
//    TMyCylinder::OnSensor()                        mycylin.cpp:192（活的）
//    ShowMyMessage                                  canary_support.h:80
//
//  ⚠ 關於 ShowMyMessage：移植樹這一支（canary_support.cpp:143）是**觀測樁** ——
//    記下 S1、計數、printf，然後轉給 hook。它**不**像 golden 那樣彈 modal、
//    也**不**呼叫 StopAllMotor。所以在這裡呼叫它不會停機。
//    ⛔ 但這代表：未來若有人把 ShowMyMessage 換成忠實版本，這個函式就會
//      連帶獲得「停所有馬達」的副作用。那是**那一次**變更要評估的事，
//      這裡先把事實寫下來，不要讓它變成靜默的意外。
//
//  ⚠ golden 的 Big5 中文訊息照原文翻成 UTF-8。這兩句會進 EventLog，
//    **字面是對外介面的一部分**，不要「順手改得更通順」。
// =============================================================================
bool TfMainWeb::CheckSLKSensor()                                                // golden :32424
{
    if (TestIF_File.bUseSLKClamp == true)                                       // golden :32426  JerryYang 20160526 判斷 SLK Sensor
    {
        if (TestIF_File.iSeparabilityTest == 0)                                 // golden :32428  JerryYang 20160602 安裝分離式 SLK，設 Direct contact mode
        {
            if (TestIF_File.iShuttleMode == 1)                                  // golden :32430  雙 ARM
            {
                if (TestIF_File.iShuttle_Sel == 0)                              // golden :32432  只使用 Arm1
                {
                    if (Cylinder[C_SLK1_Clamp].OnSensor() == false)             // golden :32434
                    {
                        ShowMyMessage("Please check the clamp I/O of arm1 layout kit", "請確認Arm1 layout kit clamp汽缸的I/O是否正常"); // golden :32436
                        return false;                                           // golden :32437
                    }
                }
                else if (TestIF_File.iShuttle_Sel == 1)                         // golden :32440  只使用 Arm2
                {
                    if (Cylinder[C_SLK2_Clamp].OnSensor() == false)             // golden :32442
                    {
                        ShowMyMessage("Please check the clamp I/O of arm2 layout kit", "請確認Arm2 layout kit clamp汽缸的I/O是否正常"); // golden :32444
                        return false;                                           // golden :32445
                    }
                }
            }
            else                                                                // golden :32449
            {
                if (Cylinder[C_SLK1_Clamp].OnSensor() == false)                 // golden :32451
                {
                    ShowMyMessage("Please check the clamp I/O of arm1 layout kit", "請確認Arm1 layout kit clamp汽缸的I/O是否正常"); // golden :32453
                    return false;                                               // golden :32454
                }
                else if (Cylinder[C_SLK2_Clamp].OnSensor() == false)            // golden :32456
                {
                    ShowMyMessage("Please check the clamp I/O of arm2 layout kit", "請確認Arm2 layout kit clamp汽缸的I/O是否正常"); // golden :32458
                    return false;                                               // golden :32459
                }
            }
        }
        else                                                                    // golden :32463  JerryYang 20160602 安裝分離式 SLK，設分離測試流程
        {
            if (TestIF_File.iShuttleMode == 1)                                  // golden :32465  雙 ARM
            {
                if (TestIF_File.iShuttle_Sel == 0)                              // golden :32467  只使用 Arm1
                {
                    if (Cylinder[C_SLK1_Clamp].OnSensor() == false && Cylinder[C_SLK1_Unclamp].OnSensor() == false) // golden :32469
                    {
                        ShowMyMessage("Please check the clamp I/O of arm1 layout kit", "請確認Arm1 layout kit clamp汽缸的I/O是否正常"); // golden :32471
                        return false;                                           // golden :32472
                    }
                }
                else if (TestIF_File.iShuttle_Sel == 1)                         // golden :32475  只使用 Arm2
                {
                    if (Cylinder[C_SLK2_Clamp].OnSensor() == false && Cylinder[C_SLK2_Unclamp].OnSensor() == false) // golden :32477
                    {
                        ShowMyMessage("Please check the clamp I/O of arm2 layout kit", "請確認Arm2 layout kit clamp汽缸的I/O是否正常"); // golden :32479
                        return false;                                           // golden :32480
                    }
                }
            }
            else                                                                // golden :32484
            {
                if (Cylinder[C_SLK1_Clamp].OnSensor() == false && Cylinder[C_SLK1_Unclamp].OnSensor() == false) // golden :32486
                {
                    ShowMyMessage("Please check the clamp I/O of arm1 layout kit", "請確認Arm1 layout kit clamp汽缸的I/O是否正常"); // golden :32488
                    return false;                                               // golden :32489
                }
                else if (Cylinder[C_SLK2_Clamp].OnSensor() == false && Cylinder[C_SLK2_Unclamp].OnSensor() == false) // golden :32491
                {
                    ShowMyMessage("Please check the clamp I/O of arm2 layout kit", "請確認Arm2 layout kit clamp汽缸的I/O是否正常"); // golden :32493
                    return false;                                               // golden :32494
                }
            }
        }
    }
    return true;                                                                // golden :32499
}


// =============================================================================
//  TfMainWeb::CheckInOutArmZHomeSensor  --  golden TfMain::CheckInOutArmZHomeSensor()
//      main.cpp:32339-32376（38 行）  Frank 20160612 add Check In/Out Arm Home Sensor Function
//
//  AI(W906-ST-W7-B) 20260915：還 §5.1 的 W2-G（🔴）。
//
//  ★★ 這一支會下運動指令（PCIL132_StopMotor -> Motor->DecStop()）。
//     照翻的理由與 S3 之前的審查義務寫在 WebStart.h 的宣告旁，不在這裡重複。
//     一句話版本：閘掉「停手臂」會變成 fail-dangerous。
//
//  ★ 行為 delta = 0：唯一呼叫點在 StartFromWeb()，而它全樹零個呼叫者。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
//
//  相依逐一實測（20260915，用一支編譯探針一次驗完，rc=0 零診斷）：
//    InArmSuck / OutArmSuck              aHotPlateSubstrate.h:624 / :627（★ 不是 mykitsuck.h）
//    .iMotRow / .iMotCol / Suck[][].iMotNo  同上標頭的 TMyKitSuck / TMySucker
//    MOT[]                               Motor/mymotor.h:385（TTrayMotor : public TMyMotor，:318）
//    ReadPos / ScanMotorStatus / Led[] / NumberAlias   Motor/mymotor.h:171 / :168 / :176 / :136
//    PCIL132_StopMotor()                 Motor/mymotor.cpp:1278（活的，走 Motor->DecStop()）
//    iHomeLed                            Motor/HTMotor.h:42（enum == 1）
//    USE_PICKER_COUNT / ep16Picker       MachineType.h
//    InOutArmPickerUseMotor / eptUseMotCyn  cmydef.h / MachineType.h
//    MInArmZA / MOutArmZA / MInArmX / MInArmY   cmydef.h
//    CosFunction.bCheckInOutArmZHomeSensor  CosFunction.h:215（W2-G 的閘門旗標）
//
//  ⚠ golden 的 Big5 中文訊息照原文轉 UTF-8。會進 EventLog，字面是對外介面。
// =============================================================================
bool TfMainWeb::CheckInOutArmZHomeSensor()                                      // golden :32339  Frank 20160612
{
    int iMotNoIn, iMotNoOut;                                                    // golden :32341
    for (int i = 0; i < InArmSuck.iMotRow; i++)                                 // golden :32342
    {
        for (int j = 0; j < InArmSuck.iMotCol; j++)                             // golden :32344
        {
            iMotNoIn  = (USE_PICKER_COUNT == ep16Picker && InOutArmPickerUseMotor == eptUseMotCyn) ? MInArmZA  : InArmSuck.Suck[i][j].iMotNo;  // golden :32346
            iMotNoOut = (USE_PICKER_COUNT == ep16Picker && InOutArmPickerUseMotor == eptUseMotCyn) ? MOutArmZA : OutArmSuck.Suck[i][j].iMotNo; // golden :32347
            if (MOT[iMotNoIn].ReadPos() >= 0)                                   // golden :32348
            {
                MOT[iMotNoIn].ScanMotorStatus();                                // golden :32350
                if (MOT[iMotNoIn].Motor->Enable && MOT[iMotNoIn].Led[iHomeLed] == false) // golden :32351
                {
                    MOT[MInArmX].PCIL132_StopMotor();                           // golden :32353  ★ 運動指令
                    MOT[MInArmY].PCIL132_StopMotor();                           // golden :32354  ★ 運動指令
                    ShowMyMessage(MOT[iMotNoIn].NumberAlias + " Home sensor error, if suck is down, maybe sensor fail!",  // golden :32355
                                  MOT[iMotNoIn].NumberAlias + "歸零sensor錯誤了，如果吸嘴在下方，可能是sensor壞掉");      // golden :32356
                    return false;                                               // golden :32357
                }
            }

            if (MOT[iMotNoOut].ReadPos() >= 0)                                  // golden :32361
            {
                MOT[iMotNoOut].ScanMotorStatus();                               // golden :32363
                if (MOT[iMotNoOut].Motor->Enable && MOT[iMotNoOut].Led[iHomeLed] == false) // golden :32364
                {
                    //AI(W906-P14) 20260923: 偏離 golden，使用者裁決 P14-乙（只在 V906 修，V912 刻意不改以維持出貨相容）。
                    //   golden :32366-32367 在出料側也停 MInArmX／MInArmY（筆誤）；出料 Z（iMotNoOut）出問題時該停的是
                    //   出料手臂的 XY —— 與上面入料分支（入料 Z → 停 MInArmX／Y）對稱。
                    MOT[MOutArmX].PCIL132_StopMotor();                          // golden :32366 是 MInArmX  ★ 運動指令
                    MOT[MOutArmY].PCIL132_StopMotor();                          // golden :32367 是 MInArmY  ★ 運動指令
                    ShowMyMessage(MOT[iMotNoOut].NumberAlias + " Home sensor error, if suck is down, maybe sensor fail!", // golden :32368
                                  MOT[iMotNoOut].NumberAlias + "歸零sensor錯誤了，如果吸嘴在下方，可能是sensor壞掉");     // golden :32369
                    return false;                                               // golden :32370
                }
            }
        }
    }
    return true;                                                                // golden :32375
}

//  ⓘ AI(W906-P14) 20260923: P14 裁決乙：V906 已修，V912 維持原樣。
//    （原註記：golden 出料側 :32366-32367 失敗時停的仍是 MInArmX／MInArmY、不是 OutArm —— 看起來是筆誤，
//     但它是 2016 年至今的出貨行為，改它要使用者決定。使用者 20260923 下班前裁決「乙」＝只在 V906 修。）
//    ⚠ 只在「出料吸嘴在下方＋原點 sensor 失效」時觸發，模擬觸發不到 —— 靜態確認，未實測觸發。


// =============================================================================
//  TfMainWeb::CheckOLPError  --  golden TfMain::CheckOLPError()
//      main.cpp:33986-34012（27 行）  Sam 20230921：Bin 設定錯誤不能啟動
//
//  AI(W906-ST-W7-C) 20260915：**部分**還 §5.1 的 W1-E（🔴）。
//
//  ★ 為什麼是部分：judgement 與 dialog 的相依狀態不同
//    判定  ComputeCheckOLPErrorHasErr()  MainCalcCore.h:404   ✅ 已存在
//    對話框 ShowMyMessagePWD()            golden mymessbox.h:52 ❌ 全樹未移植
//  所以照本戰役的規矩「能部分閘就不要整段閘」：判定翻、對話框閘。
//
//  ⚠ 不用「改叫 ShowMyMessage」來代替 —— 那支是觀測樁而且沒有密碼確認，
//    換一支不同語意的函式是替 golden 選一條路，不是翻譯。
//
//  ⚠⚠ 解閘後的實際行為：OLP Bin 設定有錯 → **拒絕啟動，但畫面沒有任何訊息**。
//    這是刻意的取捨（fail-safe 勝於 fail-open），理由寫在 WebStart.h 的宣告旁。
//    ⛔ S3 之前要補 ShowMyMessagePWD，否則現場會遇到「按 START 沒反應也沒提示」。
//
//  ★ 行為 delta = 0：唯一呼叫點在 StartFromWeb()，全樹零個呼叫者。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
// =============================================================================
bool TfMainWeb::CheckOLPError()                                                 // golden :33986  Sam 20230921
{
    // golden :33988-34010 的整個迴圈，純判定的部分由 MainCalcCore 既有函式承擔。
    // 它自己的檔頭（MainCalcCore.h:390-403）寫明：忠實保留 >=3 門檻與 0..9 全掃，
    // 不提早 break —— 跟 golden 的迴圈形狀一對一。
    const bool bHasErr = ComputeCheckOLPErrorHasErr(LastSet.OLPSetBinErr);      // golden :33988-34010

    // SAFETY-GATE(W906-ST-W7-C-DLG) 🟡 golden :33993-34007 `ShowMyMessagePWD()`
    //   golden 對三個索引各發一個不同的訊息（i==0 CATEGORY_REQUEST /
    //   i==1 BINDEFINE 或「pass 與 fail 在同一欄」/ i==2 FIXTRAYDEFINE）。
    //   `ShowMyMessagePWD`（golden mymessbox.h:52）全樹未移植 ——
    //   PowerSavingMode.cpp:937 的 GATE (3) 是同一個原因。
    //   🟡：golden 不看它的回傳值，所以它不參與判定；閘掉不改變擋不擋啟動。
    //   閘掉的後果：擋下來了，但操作員不知道是哪一項 Bin 設定出錯、要找 PE。
#if 0
    for (int i = 0; i < 10; i++)                                                // golden :33989
    {
        if (LastSet.OLPSetBinErr[i] >= 3)                                       // golden :33991
        {
            if (i == 0)                                                         // golden :33993
            {
                ShowMyMessagePWD("CATEGORY_REQUEST has command error", "Please call PE to check command", NULL, true, false, true); // golden :33995
            }
            else if (i == 1)                                                    // golden :33997
            {
                if (LastSet.OLPSetBinErr[i] == 4)                               // golden :33999
                    ShowMyMessagePWD("Categoty definie has pass and fail at same field", "Please call PE to check bin setting", NULL, true, false, true); // golden :34000
                else
                    ShowMyMessagePWD("BINDEFINE_REQUEST has command error", "Please call PE to check command", NULL, true, false, true); // golden :34002
            }
            else if (i == 2)                                                    // golden :34004
            {
                ShowMyMessagePWD("FIXTRAYDEFINE_REQUEST has command error", "Please call PE to check command", NULL, true, false, true); // golden :34006
            }
        }
    }
#endif

    return bHasErr;                                                             // golden :34011
}


// =============================================================================
//  W906ST_CheckSmartAutoCleanCanStart  --  golden TfCleaning::CheckSmartAutoCleanCanStart()
//      AutoClean/uCleaning.cpp:2879-2915（37 行）
//      Sam 20250916：Alarm 後需要清除資料才能 Start
//
//  AI(W906-ST-W7-D) 20260915：還 §5.1 的 W2-C（🔴）。
//
//  ★ 為什麼是**自由函式**而不是 TfCleaning 的方法
//    golden 這一支是 `TfCleaning` 的成員，呼叫點是 `fCleaning->CheckSmartAutoCleanCanStart()`。
//    移植樹的 `forms/fCleaning.h` **沒有宣告過它**（實測），而 FACADE CONTRACT 規定
//    facade 的本體是「PERMANENT OFFLINE IMPLEMENTATION」，真實實作要用 impl 子類；
//    而把 `fCleaning` 重指到 impl 屬於 S3（武裝），本戰役硬停止線明文禁止。
//
//    ⚠ 但這裡有一個**比落點更重要的事實**：golden 用到的每一個狀態，
//      在移植樹**都是全域**，不是 TfCleaning 的成員 —— 實測（編譯探針 rc=0，零診斷）：
//        IniConfig.bEnableAutoCleanFunction / TestIF.iAutoClean_Function /
//        TestIF.bACSmart / TestIF.iACSmart_Count / TestIF.iACSmart_Count_CTF /
//        iACSmartCount_CTF / iACSmartCount / sACRecAlarmCode / sACRecEPortCode /
//        K_RETRY / MMInterface
//      全部在檔案範圍直接取得。所以「成員 vs 自由函式」在這裡**不改變讀到的東西**。
//
//    這也是這棵樹既有的慣例：`MainCalcCore` 就是把 TfMain 的方法抽成自由函式
//    （見該檔 banner）。前綴 `W906ST_` 照 `AutoRetest.cpp:338` 的命名前例。
//
//  ⚠ 注意 `TestIF` 不是 `TestIF_File` —— 本檔別處大量使用後者，兩者是不同的結構。
//    golden 這一支讀的是 `TestIF`，照翻。
//
//  ★ 行為 delta = 0：唯一呼叫點在 StartFromWeb()，全樹零個呼叫者。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
// =============================================================================
static bool W906ST_CheckSmartAutoCleanCanStart()                                // golden uCleaning.cpp:2879  Sam 20250916
{
    if (IniConfig.bEnableAutoCleanFunction &&                                   // golden :2881
        TestIF.iAutoClean_Function         &&                                   // golden :2882
        TestIF.bACSmart                    &&                                   // golden :2883
        TestIF.iACSmart_Count     != 0     &&                                   // golden :2884
        TestIF.iACSmart_Count_CTF != 0)                                         // golden :2885
    {
        bool bCanStart = true;                                                  // golden :2887

        if (iACSmartCount_CTF == 1)                                             // golden :2889
            bCanStart = true;                                                   // golden :2890
        else if (iACSmartCount_CTF <= TestIF.iACSmart_Count_CTF)                // golden :2891
            bCanStart = true;                                                   // golden :2892
        else
            bCanStart = false;                                                  // golden :2894

        if (bCanStart)                                                          // golden :2896
        {
            if (iACSmartCount == 1)                                             // golden :2898
                bCanStart = true;                                               // golden :2899
            else if (iACSmartCount <= TestIF.iACSmart_Count)                     // golden :2900
                bCanStart = true;                                               // golden :2901
            else
                bCanStart = false;                                              // golden :2903
        }

        if (bCanStart == false)                                                 // golden :2906  Sam 20250916 : Alarm 後需要清除資料才能 Start
        {
            ShowErrorMessage(sACRecAlarmCode, K_RETRY, MMInterface, false, sACRecEPortCode); // golden :2908
            // golden :2909 是被註解掉的 ShowMyMessage("Please Reset AI AutoClean","")
            //   —— 照原樣保留為註解，不要「順手啟用」。
        }

        return bCanStart;                                                       // golden :2912
    }
    return true;                                                                // golden :2914
}


// =============================================================================
//  TfMainWeb::SET_ESD_Tri_Temp  --  golden TfMain::SET_ESD_Tri_Temp(int)
//      main.cpp:34528-34550（23 行）  Ztex 2023.04.19 Add HT-1032 TriTemp Function
//
//  AI(W906-ST-W7-G) 20260915：還 §5.1 的 W6-H（🟡）。
//
//  相依實測（編譯探針 rc=0，零診斷）：
//    USE_NOVX3360 / Tri_Temp_Machine            cmydef.h
//    Tempture_Hot / Tempture_AmbientHot         cprod.h
//    Temperature.fWorkTemperBase                cprod.h
//    SendCommand_ESD / ESD_Temperature*         Interface/InterfaceSYS.h（本體 .cpp:532，真實作）
//
//  ★ 行為 delta = 0：唯一呼叫點在 StartFromWeb()，全樹零個呼叫者。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
//
//  ⚠ golden 的 else-if 與 else 兩支**做同一件事**（都送 ESD_TemperatureAmbient，
//    :34543 與 :34547）。看起來可以合併，但那是 golden 的原樣 —— 照翻不合併，
//    因為合併之後就看不出「常溫熱」曾經是一個被分開處理的情況。
//
//  ⚠ golden 的 Hot 分支有一個**沒有涵蓋的區間**：fWorkTemperBase 在 10~40 之間時
//    三個條件都不成立，**一個命令都不送**。照翻並在此註明；那是 golden 的行為，
//    不是我漏翻。改它要使用者決定。
// =============================================================================
void TfMainWeb::SET_ESD_Tri_Temp(int iTemperature)                              // golden :34528  Ztex 2023.04.19
{
    if (USE_NOVX3360 == true && Tri_Temp_Machine == 1)                          // golden :34530
    {
        if (iTemperature == Tempture_Hot)                                       // golden :34532
        {
            if (Temperature.fWorkTemperBase >= 40 && Temperature.fWorkTemperBase <= 130) // golden :34534
                SendCommand_ESD(ESD_TemperatureHot);                            // golden :34535
            else if (Temperature.fWorkTemperBase > 130)                         // golden :34536
                SendCommand_ESD(ESD_TemperatureSuperHot);                       // golden :34537
            else if (Temperature.fWorkTemperBase < 10)                          // golden :34538
                SendCommand_ESD(ESD_TemperatureCold);                           // golden :34539
            // ⚠ golden 沒有 else —— 10~40 這一段不送任何命令。照翻。
        }
        else if (iTemperature == Tempture_AmbientHot)                           // golden :34541
        {
            SendCommand_ESD(ESD_TemperatureAmbient);                            // golden :34543
        }
        else                                                                    // golden :34545
        {
            SendCommand_ESD(ESD_TemperatureAmbient);                            // golden :34547  與上一支相同，照翻不合併
        }
    }
}


// =============================================================================
//  W906ST_Read2DIDList  --  golden TfBarCode::Read2DIDList()
//      BarCode/BarCode.cpp:8209-8246（38 行）  Steven 20190604：2DID 不在 List 內的另外分 bin
//
//  AI(W906-ST-W7-H) 20260915：還 §5.1 的 W1-C（🟡）。
//  ★ 這是 §5.5 那張分類表裡**最後一個**「相依齊全、可自動做」的 gate。
//
//  落點是自由函式，理由同 ST-W7-D：golden 是 `TfBarCode` 的成員，
//  而移植樹的 `TfBarCode` 沒有這個成員，重指 shim 屬 S3。
//  兩個清單物件都是裸全域（`BarCode/BarCode_Bottom2DID.h:144` 與新增的
//  `listError2DID`），所以換 host 不改變讀寫的對象。
//
//  ⚠⚠ **今天它餵不到任何人，這是已知且刻意的**：
//    兩個消費者 `b2DIDIsInsideList`（golden BarCode.cpp:8248）與
//    `b2DIDIsInsideToErrorBin`（golden :8268）在移植樹**零個定義**，
//    唯四的呼叫點（`atester.cpp:1745/1758/2824/2845`）全部在
//    golden-verbatim 的 `#if 0` 內。
//    仍然翻，理由是：gate 的債要誠實還掉，而且消費者落地那天清單已經是熱的。
//    **不要因為「沒人讀」就把它當成沒做完** —— 也不要當成它已經生效。
//
//  相依實測（編譯探針 rc=0）：`LastSet.iTester` / `OFF_LINE` /
//    `TestIF_File.bEnableBarCode` / `.bSearch2DIDByLot` / `.b2DIDNotExist2Error` /
//    `asSearch2DIDByLot`（common.h:123）/ `as2DIDSortToError` / `FileExists` /
//    `ShowMyMessage`（canary_support.h:80）。
//
//  ★ 行為 delta = 0：唯一呼叫點在 StartFromWeb()，全樹零個呼叫者。
//    ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
// =============================================================================
static void W906ST_Read2DIDList()                                               // golden BarCode.cpp:8209  Steven 20190604
{
    list2DByLot->Clear();                                                       // golden :8211
    listError2DID->Clear();                                                     // golden :8212

    if (LastSet.iTester != OFF_LINE ||                                          // golden :8214
        TestIF_File.bEnableBarCode == false)                                    // golden :8215
    {
        return;                                                                 // golden :8217
    }

    if (TestIF_File.bSearch2DIDByLot)                                           // golden :8220  Frank 20170316 (wei) add Search 2DID By Lot
    {
        if (FileExists(asSearch2DIDByLot))                                      // golden :8222
        {
            list2DByLot->LoadFromFile(asSearch2DIDByLot);                       // golden :8224
        }
        else
        {
            // golden :8228 的註解原文是「由Frank新增」—— 空的 else，照翻不補。
        }
        list2DByLot->Sort();                                                    // golden :8230  ★ 在 if/else 之外，兩條路都會 Sort
    }
    else if (TestIF_File.b2DIDNotExist2Error)                                   // golden :8232  JerryYang 20231218 : 2DID 黑名單功能
    {
        if (FileExists(as2DIDSortToError))                                      // golden :8234
        {
            listError2DID->LoadFromFile(as2DIDSortToError);                     // golden :8236
            listError2DID->Sort();                                              // golden :8237  ★ 這一支的 Sort 在 if 內，與上面不對稱
        }
        else
        {
            AnsiString s = "";                                                  // golden :8241
            s.sprintf("Can not found the 2DID file in path %s", as2DIDSortToError); // golden :8242
            ShowMyMessage(s);                                                   // golden :8243
        }
    }
}


// ===========================================================================
//  AI(W906-T5-W4E) 20260919  —— 解 SAFETY-GATE(W906-ST-W4-E)
//  golden: bool TfMain::NETDownloadDataCheck()   main.cpp:30425-30792（368 行）
//
//  它做的事：把 FTP/RMS 下載回來的那一份配方（`*_NET`）跟機台本機那一份
//  （`*_File` / 全域）**逐欄比對**，任何一欄不同就 ShowMyMessage 並回 false，
//  由呼叫端擋下啟動。沒有任何寫入，純比較。
//
//  逐行翻譯，零個閘 —— 368 行的相依在這棵樹全部齊備（見 WebStart.h 的說明）。
// ===========================================================================
bool TfMainWeb::NETDownloadDataCheck()                                          // golden :30425
{
    AnsiString Str = "", Str2;                                                  // golden :30427
    bool bContactHasErr = false;                                                // golden :30428

    if (CUSTOMER_CODE == CC_SCC ||                                              // golden :30429
        (CosFunction.bHiSiliconFunction == true &&                              // golden :30430
         CUSTOMER_CODE == CC_AMD_M))                                            // golden :30431  Ifor 20200727 add TF-AMD RMS Data Check
    {
        if (IniConfig.bEnableFTP == false && IniConfig.bEnableRms == false)     // golden :30433
            return true;                                                        // golden :30434

        if (bFTPDownloadSetupFile == false && IniConfig.bEnableFTP == true)     // golden :30436
        {
            ShowMyMessage("Need Download FTP Setup File!!");                    // golden :30438
            return false;                                                       // golden :30439
        }
    }
    else                                                                        // golden :30442
    {
        if (CUSTOMER_CODE != CC_ASE_M)                                          // golden :30444
        {
            if (IniConfig.bEnableFTP == false)                                  // golden :30446
                return true;                                                    // golden :30447

            if (IniConfig.bVTESTFunction == true &&                             // golden :30449
                fMesSystem->CheckVTENGmode(fLotInfo->edtSysLotID->Text) == true)// golden :30450
                return true;                                                    // golden :30451

            if (bFTPDownloadSetupFile == false)                                 // golden :30453
            {
                ShowMyMessage("Need Download FTP Setup File!!");                // golden :30455
                return false;                                                   // golden :30456
            }
        }
        else                                                                    // golden :30459
        {
            if (IniConfig.bN13_EnableARMSFunction == false)                     // golden :30461  Ifor 20250303 Fix ASEM ARM 異常報警
                return true;                                                    // golden :30462
        }
    }

    if (SYS_SetupFile != DownloadWorkFile_NET)                                  // golden :30466
    {
        ShowMyMessage("Work File Check Error!!");                               // golden :30468
        return false;                                                           // golden :30469
    }

    if (LastSet.iTemperature != LastSetTemperature_NET)                         // golden :30472
    {
        ShowMyMessage("Temperature Mode Check Error!!");                        // golden :30474
        return false;                                                           // golden :30475
    }

    if (Temperature.fWorkTemperBase != Temperature_NET.fWorkTemperBase ||       // golden :30478
        Temperature.fSoakTime       != Temperature_NET.fSoakTime)               // golden :30479
    {
        ShowMyMessage("Temperature Default or Soak Time Check Error!!");        // golden :30481
        return false;                                                           // golden :30482
    }

    for (int j = 0; j < 8; j++)                                                 // golden :30485  ChungHung 20141002 3->5 / Ifor 20170417 5->8
    {
        for (int i = 0; i < 15; i++)                                            // golden :30487  QQQ
        {
            if (BinSelect[j].iCatDataT3Pos[i]     != BinSelect_NET[j].iCatDataT3Pos[i] ||      // golden :30489
                BinSelect[j].iDBContact[i]        != BinSelect_NET[j].iDBContact[i] ||         // golden :30490
                BinSelect[j].iStackDefFailCate[i] != BinSelect_NET[j].iStackDefFailCate[i] ||  // golden :30491
                BinSelect[j].bFailure[i]          != BinSelect_NET[j].bFailure[i])             // golden :30492
            {
                ShowMyMessage("Binning Data Check Error!!");                    // golden :30494
                return false;                                                   // golden :30495
            }
        }
    }

    if (HotPlateForm_File.XPitch       != HotPlateForm_NET.XPitch ||            // golden :30500
        HotPlateForm_File.YPitch       != HotPlateForm_NET.YPitch ||            // golden :30501
        HotPlateForm_File.XStart       != HotPlateForm_NET.XStart ||            // golden :30502
        HotPlateForm_File.YStart       != HotPlateForm_NET.YStart ||            // golden :30503
        HotPlateForm_File.XDivision    != HotPlateForm_NET.XDivision ||         // golden :30504
        HotPlateForm_File.YDivision    != HotPlateForm_NET.YDivision ||         // golden :30505
        HotPlateForm_File.ZDepth       != HotPlateForm_NET.ZDepth ||            // golden :30506
        HotPlateForm_File.iPlateSelect != HotPlateForm_NET.iPlateSelect)        // golden :30507
    {
        ShowMyMessage("Hot Plate Form Data Check Error!!");                     // golden :30509
        return false;                                                           // golden :30510
    }

    if (TrayForm.LodareType != TrayForm_NET.LodareType)                         // golden :30513
    {
        ShowMyMessage("Tray Form Data Check Error!!");                          // golden :30515
        return false;                                                           // golden :30516
    }

    TRAY_TYPE_PARA tmpTrayPara[eTrayCount];                                     // golden :30519
    tmpTrayPara[eAuto1] = TrayForm.Auto[eAuto1];                                // golden :30520
    tmpTrayPara[eAuto2] = TrayForm.Auto[eAuto2];                                // golden :30521
    tmpTrayPara[eAuto3] = TrayForm.Auto[eAuto3];                                // golden :30522
    tmpTrayPara[eAuto4] = TrayForm.Auto[eAuto4];                                // golden :30523
    tmpTrayPara[eAuto5] = TrayForm.Auto[eAuto5];                                // golden :30524
    tmpTrayPara[eAuto6] = TrayForm.Auto[eAuto6];                                // golden :30525
    tmpTrayPara[eFix1 ] = TrayForm.Auto[eFix1 ];                                // golden :30526
    tmpTrayPara[eFix2 ] = TrayForm.Auto[eFix2 ];                                // golden :30527
    tmpTrayPara[eFix3 ] = TrayForm.Auto[eFix3 ];                                // golden :30528
    tmpTrayPara[eFix4 ] = TrayForm.Auto[eFix4 ];                                // golden :30529
    tmpTrayPara[eFix5 ] = TrayForm.Auto[eFix5 ];                                // golden :30530
    tmpTrayPara[eFix6 ] = TrayForm.Auto[eFix6 ];                                // golden :30531

    // ⓘ golden 這裡把 *_NET 的 Auto[] 也搬進一個**同樣叫 TrayPara_NET 的區域陣列**。
    //   它不是全域 —— 全樹 grep `TrayPara_NET` 會 0 命中，那是正確的。
    TRAY_TYPE_PARA TrayPara_NET[eTrayCount];                                    // golden :30533
    TrayPara_NET[eAuto1] = TrayForm_NET.Auto[eAuto1];                           // golden :30534
    TrayPara_NET[eAuto2] = TrayForm_NET.Auto[eAuto2];                           // golden :30535
    TrayPara_NET[eAuto3] = TrayForm_NET.Auto[eAuto3];                           // golden :30536
    TrayPara_NET[eAuto4] = TrayForm_NET.Auto[eAuto4];                           // golden :30537
    TrayPara_NET[eAuto5] = TrayForm_NET.Auto[eAuto5];                           // golden :30538
    TrayPara_NET[eAuto6] = TrayForm_NET.Auto[eAuto6];                           // golden :30539
    TrayPara_NET[eFix1 ] = TrayForm_NET.Auto[eFix1 ];                           // golden :30540
    TrayPara_NET[eFix2 ] = TrayForm_NET.Auto[eFix2 ];                           // golden :30541
    TrayPara_NET[eFix3 ] = TrayForm_NET.Auto[eFix3 ];                           // golden :30542
    TrayPara_NET[eFix4 ] = TrayForm_NET.Auto[eFix4 ];                           // golden :30543
    TrayPara_NET[eFix5 ] = TrayForm_NET.Auto[eFix5 ];                           // golden :30544
    TrayPara_NET[eFix6 ] = TrayForm_NET.Auto[eFix6 ];                           // golden :30545

    for (int i = 0; i < iFixRight; i++)                                         // golden :30547
    {
        if (Prod.iTrayType[i] != tNotUse &&                                     // golden :30549
            Prod.iTrayType[i] != tTrayBox)                                      // golden :30550
        {
            if (tmpTrayPara[i].XPitch    != TrayPara_NET[i].XPitch ||           // golden :30552
                tmpTrayPara[i].YPitch    != TrayPara_NET[i].YPitch ||           // golden :30553
                tmpTrayPara[i].XStart    != TrayPara_NET[i].XStart ||           // golden :30554
                tmpTrayPara[i].YStart    != TrayPara_NET[i].YStart ||           // golden :30555
                tmpTrayPara[i].XDivision != TrayPara_NET[i].XDivision ||        // golden :30556
                tmpTrayPara[i].YDivision != TrayPara_NET[i].YDivision)          // golden :30557
            {
                ShowMyMessage("Tray Form Data Check Error!!");                  // golden :30559
                return false;                                                   // golden :30560
            }
        }
    }

    for (int i = 0; i < 4; i++)                                                 // golden :30565
    {
        for (int j = 0; j < 8; j++)                                             // golden :30567
        {
            if (TestIF_File.iSiteMap[i][j] != TestIF_NET.iSiteMap[i][j])        // golden :30569
            {
                ShowMyMessage("Site Mapping Data Check Error!!");               // golden :30571
                return false;                                                   // golden :30572
            }
        }
    }

    if (TestIF_File.bEnableMRTMode != TestIF_NET.bEnableMRTMode)                // golden :30578  Ifor 20170418 (wei) add MRT Mode Check
    {
        ShowMyMessage("MRT Mode Check Error!!");                                // golden :30580
        return false;                                                           // golden :30581
    }

    if (TestIF_File.iTestMode != TestIF_NET.iTestMode)                          // golden :30584
    {
        ShowMyMessage("Test Site Mode Check Error!!");                          // golden :30586
        return false;                                                           // golden :30587
    }

    if (TestIF_File.iTestType != TestIF_NET.iTestType)                          // golden :30590
    {
        ShowMyMessage("Test Interface Mode Check Error!!");                     // golden :30592
        return false;                                                           // golden :30593
    }

    if (TestIF_File.iGpibMode != TestIF_NET.iGpibMode)                          // golden :30596
    {
        ShowMyMessage("GPIB Type Check Error!!");                               // golden :30598
        return false;                                                           // golden :30599
    }

    if (TestIF_File.iGpibAddress != TestIF_NET.iGpibAddress)                    // golden :30602
    {
        ShowMyMessage("GPIB Address Check Error!!");                            // golden :30604
        return false;                                                           // golden :30605
    }

    if (TestIF_File.iRs232Mode != TestIF_NET.iRs232Mode)                        // golden :30608
    {
        ShowMyMessage("RS-232 Type Check Error!!");                             // golden :30610
        return false;                                                           // golden :30611
    }

    // golden :30614-30615  Ifor 20181023 add SCC 要求 FTP 資料卡控加入 Contact 相關資料
    if (CUSTOMER_CODE == CC_CYUEAN ||                                           // golden :30616  Steven 20231220
        CUSTOMER_CODE == CC_SCC    ||                                           // golden :30617
        CUSTOMER_CODE == CC_JCET   ||                                           // golden :30618
        CUSTOMER_CODE == CC_SJ_Semiconductor ||                                 // golden :30619  Steven 20220623 Add SJSM
        CUSTOMER_CODE == CC_SJ_Semiconductor_OS ||                              // golden :30620
        CUSTOMER_CODE == CC_XINITECH            ||                              // golden :30621
        (CosFunction.bHiSiliconFunction == true && CUSTOMER_CODE == CC_AMD_M))  // golden :30622  Steven 20190111 JCET add contact 資料卡控
    {
        // golden :30624  Steven 20190904：將 Alarm 分開表示
        if (DeviceForm_File.VacuumMode != DeviceForm_NET.VacuumMode)            // golden :30625
        {
            bContactHasErr = true;                                              // golden :30627
            Str2.sprintf("VacuumMode %d : %d", DeviceForm_File.VacuumMode, DeviceForm_NET.VacuumMode);  // golden :30628
            Str = Str + ", " + Str2;                                            // golden :30629
        }

        if (DeviceForm_File.ContactMode != DeviceForm_NET.ContactMode)          // golden :30632
        {
            bContactHasErr = true;                                              // golden :30634
            Str2.sprintf("ContactMode %d : %d", DeviceForm_File.ContactMode, DeviceForm_NET.ContactMode);  // golden :30635
            Str = Str + ", " + Str2;                                            // golden :30636
        }

        if (DeviceForm_File.DummyMode != DeviceForm_NET.DummyMode)              // golden :30639
        {
            bContactHasErr = true;                                              // golden :30641
            // ⚠ GOLDEN 缺陷，照翻：這一條比的是 DummyMode，但訊息字面印的是
            //   "ContactMode"（golden :30642 逐字如此）。操作員看到兩則
            //   "ContactMode ..." 會以為同一個欄位被報了兩次。
            //   照翻而非修正 —— 改字面等於改對外介面（那是 ShowMyMessage 的
            //   S1，會進 EventLog）。要修請走使用者裁決。
            Str2.sprintf("ContactMode %d : %d", DeviceForm_File.DummyMode, DeviceForm_NET.DummyMode);  // golden :30642
            Str = Str + ", " + Str2;                                            // golden :30643
        }

        if (DeviceForm_File.DropWait != DeviceForm_NET.DropWait)                // golden :30646
        {
            bContactHasErr = true;                                              // golden :30648
            Str2.sprintf("DropWait %d : %d", DeviceForm_File.DropWait, DeviceForm_NET.DropWait);  // golden :30649
            Str = Str + ", " + Str2;                                            // golden :30650
        }

        if (DeviceForm_File.DropSpeed != DeviceForm_NET.DropSpeed)              // golden :30653
        {
            bContactHasErr = true;                                              // golden :30655
            Str2.sprintf("DropSpeed %d : %d", DeviceForm_File.DropSpeed, DeviceForm_NET.DropSpeed);  // golden :30656
            Str = Str + ", " + Str2;                                            // golden :30657
        }

        // golden :30659-30660  Ifor 20210112 ==>
        if (IniConfig.bD58UseArm1PickPlaceArm2Test == true && TestIF_File.bArm1PickPlaceArm2Test == true)  // golden :30661  Steven 20211124 功能沒開時不比對
        {
            if (DeviceForm_File.UpWait != DeviceForm_NET.UpWait)                // golden :30663
            {
                bContactHasErr = true;                                          // golden :30665
                Str2.sprintf("UpWait %d : %d", DeviceForm_File.UpWait, DeviceForm_NET.UpWait);  // golden :30666
                Str = Str + ", " + Str2;                                        // golden :30667
            }

            if (DeviceForm_File.UpSpeed != DeviceForm_NET.UpSpeed)              // golden :30670
            {
                bContactHasErr = true;                                          // golden :30672
                Str2.sprintf("UpSpeed %d : %d", DeviceForm_File.UpSpeed, DeviceForm_NET.UpSpeed);  // golden :30673
                Str = Str + ", " + Str2;                                        // golden :30674
            }
        }
        // golden :30677-30678  <== / Ifor 20210112

        if (DeviceForm_File.bShuttleWaitingOutSiteChamber != DeviceForm_NET.bShuttleWaitingOutSiteChamber)  // golden :30679
        {
            bContactHasErr = true;                                              // golden :30681
            Str2.sprintf("bShuttleWaitingOutSiteChamber %d : %d", DeviceForm_File.bShuttleWaitingOutSiteChamber, DeviceForm_NET.bShuttleWaitingOutSiteChamber);  // golden :30682
            Str = Str + ", " + Str2;                                            // golden :30683
        }

        if (DeviceForm_File.bSuckShuttleDeviceAfterTested != DeviceForm_NET.bSuckShuttleDeviceAfterTested)  // golden :30686
        {
            bContactHasErr = true;                                              // golden :30688
            Str2.sprintf("bSuckShuttleDeviceAfterTested %d : %d", DeviceForm_File.bSuckShuttleDeviceAfterTested, DeviceForm_NET.bSuckShuttleDeviceAfterTested);  // golden :30689
            Str = Str + ", " + Str2;                                            // golden :30690
        }

        if (DeviceForm_File.bIndexUpSpeed != DeviceForm_NET.bIndexUpSpeed)      // golden :30693
        {
            bContactHasErr = true;                                              // golden :30695
            Str2.sprintf("bIndexUpSpeed %d : %d", DeviceForm_File.bIndexUpSpeed, DeviceForm_NET.bIndexUpSpeed);  // golden :30696
            Str = Str + ", " + Str2;                                            // golden :30697
        }

        if (DeviceForm_File.bPurgeBeforePickShuttle != DeviceForm_NET.bPurgeBeforePickShuttle)  // golden :30700
        {
            bContactHasErr = true;                                              // golden :30702
            Str2.sprintf("bPurgeBeforePickShuttle %d : %d", DeviceForm_File.bPurgeBeforePickShuttle, DeviceForm_NET.bPurgeBeforePickShuttle);  // golden :30703
            Str = Str + ", " + Str2;                                            // golden :30704
        }

        if (DeviceForm_File.iPurgeBeforePickShuttleTime != DeviceForm_NET.iPurgeBeforePickShuttleTime)  // golden :30707
        {
            bContactHasErr = true;                                              // golden :30709
            Str2.sprintf("iPurgeBeforePickShuttleTime %d : %d", DeviceForm_File.iPurgeBeforePickShuttleTime, DeviceForm_NET.iPurgeBeforePickShuttleTime);  // golden :30710
            Str = Str + ", " + Str2;                                            // golden :30711
        }

        if (DeviceForm_File.iPurgeBeforePickShuttleInterval != DeviceForm_NET.iPurgeBeforePickShuttleInterval)  // golden :30714
        {
            bContactHasErr = true;                                              // golden :30716
            Str2.sprintf("iPurgeBeforePickShuttleInterval %d : %d", DeviceForm_File.iPurgeBeforePickShuttleInterval, DeviceForm_NET.iPurgeBeforePickShuttleInterval);  // golden :30717
            Str = Str + ", " + Str2;                                            // golden :30718
        }

        // ⓘ golden 的欄位名就是 `iPurgeBdforePickShuttleOffSet`（Bdfore，打字錯誤），
        //   照翻 —— 它是結構成員名，改了就對不上 cprod.h。
        if (DeviceForm_File.iPurgeBdforePickShuttleOffSet != DeviceForm_NET.iPurgeBdforePickShuttleOffSet)  // golden :30721
        {
            bContactHasErr = true;                                              // golden :30723
            Str2.sprintf("iPurgeBdforePickShuttleOffSet %d : %d", DeviceForm_File.iPurgeBdforePickShuttleOffSet, DeviceForm_NET.iPurgeBdforePickShuttleOffSet);  // golden :30724
            Str = Str + ", " + Str2;                                            // golden :30725
        }

        if (DeviceForm_File.bPickShuttleDeviceTogether != DeviceForm_NET.bPickShuttleDeviceTogether)  // golden :30728
        {
            bContactHasErr = true;                                              // golden :30730
            Str2.sprintf("bPickShuttleDeviceTogether %d : %d", DeviceForm_File.bPickShuttleDeviceTogether, DeviceForm_NET.bPickShuttleDeviceTogether);  // golden :30731
            Str = Str + ", " + Str2;                                            // golden :30732
        }

        if (bContactHasErr)                                                     // golden :30735
        {
            ShowMyMessage("Contact parameter Data Check Error!!", Str, Str);    // golden :30737
            return false;                                                       // golden :30738
        }

        if (DeviceForm_File.iHeadDeviceCT != DeviceForm_NET.iHeadDeviceCT ||    // golden :30741
            DeviceForm_File.dKitDiameter  != DeviceForm_NET.dKitDiameter  ||    // golden :30742
            DeviceForm_File.iPinCT        != DeviceForm_NET.iPinCT        ||    // golden :30743
            DeviceForm_File.ForcePerPinN  != DeviceForm_NET.ForcePerPinN  ||    // golden :30744
            DeviceForm_File.ForcePerPinG  != DeviceForm_NET.ForcePerPinG  ||    // golden :30745
            DeviceForm_File.XDimension    != DeviceForm_NET.XDimension    ||    // golden :30746
            DeviceForm_File.YDimension    != DeviceForm_NET.YDimension)         // golden :30747
        {
            ShowMyMessage("Contact Force Data Check Error!!");                  // golden :30749
            return false;                                                       // golden :30750
        }
    }
    // golden :30753-30754  <== / Ifor 20181023

    if (CUSTOMER_CODE == CC_JCET ||                                             // golden :30756  Steven 20190111 JCET add yield 資料卡控
        CUSTOMER_CODE == CC_SJ_Semiconductor ||                                 // golden :30757  Steven 20220623 Add SJSM
        CUSTOMER_CODE == CC_SJ_Semiconductor_OS ||                              // golden :30758
        CUSTOMER_CODE == CC_XINITECH)                                           // golden :30759
    {
        if (TestIF_File.bLowYieldAlarmByBin               != TestIF_NET.bLowYieldAlarmByBin               ||  // golden :30761
            TestIF_File.dLowYieldLimit                    != TestIF_NET.dLowYieldLimit                    ||  // golden :30762
            TestIF_File.iLowYieldCount                    != TestIF_NET.iLowYieldCount                    ||  // golden :30763
            TestIF_File.dFailAlarmSiteYield               != TestIF_NET.dFailAlarmSiteYield               ||  // golden :30764
            TestIF_File.iFailAlarmSiteYieldDifferentCount != TestIF_NET.iFailAlarmSiteYieldDifferentCount ||  // golden :30765
            TestIF_File.dFailAlarmSiteYieldCmp            != TestIF_NET.dFailAlarmSiteYieldCmp            ||  // golden :30766
            TestIF_File.iFailAlarmSiteYieldCmpCount       != TestIF_NET.iFailAlarmSiteYieldCmpCount       ||  // golden :30767
            TestIF_File.bFailAlarmLowYieldByTotal         != TestIF_NET.bFailAlarmLowYieldByTotal         ||  // golden :30768
            TestIF_File.dLowYieldLimitByTotal             != TestIF_NET.dLowYieldLimitByTotal             ||  // golden :30769
            TestIF_File.iLowYieldCountByTotal             != TestIF_NET.iLowYieldCountByTotal             ||  // golden :30770
            TestIF_File.bFailAlarmLowYield                != TestIF_NET.bFailAlarmLowYield                ||  // golden :30771
            TestIF_File.bFailAlarmSiteYieldDifferent      != TestIF_NET.bFailAlarmSiteYieldDifferent      ||  // golden :30772
            TestIF_File.iFailAlarmSiteYield               != TestIF_NET.iFailAlarmSiteYield               ||  // golden :30773
            TestIF_File.bContinuousPass                   != TestIF_NET.bContinuousPass                   ||  // golden :30774
            TestIF_File.iContinuousPassBinCount           != TestIF_NET.iContinuousPassBinCount           ||  // golden :30775
            TestIF_File.bContinuousPassBySocket           != TestIF_NET.bContinuousPassBySocket           ||  // golden :30776
            TestIF_File.iContinuousPassBinCountBySocket   != TestIF_NET.iContinuousPassBinCountBySocket   ||  // golden :30777
            TestIF_File.bContinuousLoader                 != TestIF_NET.bContinuousLoader                 ||  // golden :30778
            TestIF_File.iContinuousLoaderCount            != TestIF_NET.iContinuousLoaderCount            ||  // golden :30779
            TestIF_File.bContinuousContact                != TestIF_NET.bContinuousContact                ||  // golden :30780
            TestIF_File.iContinuousContactCount           != TestIF_NET.iContinuousContactCount           ||  // golden :30781
            TestIF_File.bContsFailBySocket                != TestIF_NET.bContsFailBySocket                ||  // golden :30782
            TestIF_File.iContsFailSocketAlarmCT           != TestIF_NET.iContsFailSocketAlarmCT           ||  // golden :30783
            TestIF_File.bContsFailByHead                  != TestIF_NET.bContsFailByHead                  ||  // golden :30784
            TestIF_File.iContsFailHeadAlarmCT             != TestIF_NET.iContsFailHeadAlarmCT)                // golden :30785
        {
            ShowMyMessage("Yield Data Check Error!!");                          // golden :30787
            return false;                                                       // golden :30788
        }
    }
    return true;                                                                // golden :30791
}

//AI(W906-T7) 20260923: golden TfMain::CheckARTSetupFile()（main.cpp:32211-32238）。純判定、無副作用；
//  本體早就以參數化的 ComputeCheckARTSetupFile 翻好（MainCalcCore.cpp:364，極性與七個 Pos() 檢查逐字相同，
//  有 9 支測試），這裡只負責把 golden 讀的那幾個值原樣餵進去 —— 不重寫判定。
//  值的來源（T7 動手前逐一量過，都有人維護）：
//    USE_AUTO_RETEST／IniConfig.bA10_AutoReTest —— 開機 LoadMachineConfig
//    BinSelect[FT_ART] —— 開機 fBinSel->ReadFile()（tools/wb_serve.cpp，golden main.cpp:1014 的形狀）
//    iTestBinCount —— cmydef.cpp 初值 16，執行期由 BinSel 維護
//    fMain->cbSetupFileName->Text —— 開機 `= GetLastOpenFN()`（tools/wb_serve.cpp，golden main.cpp:9440），換工作檔時由遠端命令改
static bool CheckARTSetupFile()                                                 // golden main.cpp:32211
{
    return ComputeCheckARTSetupFile(USE_AUTO_RETEST,
                                    IniConfig.bA10_AutoReTest,
                                    BinSelect[FT_ART].bAutoRetest,
                                    BinSelect[FT_ART].iCatDataT3Pos,
                                    iTestBinCount,
                                    fMain->cbSetupFileName->Text);
}

bool TfMainWeb::StartFromWeb(AnsiString Func)
{
    static int iStartIn = 0;                                                    // golden :4387
    { extern bool W906_PageStartAllowed(const char*); if (!W906_PageStartAllowed(Func.c_str())) { RecordProcess(AnsiString("W906 PAGETAB: START refused -- no HMI screen connected (") + Func + ")"); return false; } }   //AI(W906-PAGETAB-Q51) 20260928 [W906] Steven Q-P1 20260928「關閉沒畫面的時候, c++不可以system start」：一個 HMI 畫面都沒有（沒有 WebSocket、或主畫面 fMain 的網頁答案是關）就拒絕，照 golden 的 early-return false（不記 MES2110、不動 iStartIn）。本函式是 start.run（tools/wb_serve.cpp:5679）與告警框 START（:6048，含告警框裡的面板 START 鍵）唯一的落點；判斷在 WebPageTable.cpp PageStartAllowed。放在原本的空白行，不移動行號
    LogSoftwareOnTime("HT9045, Start(0)");                                      // golden :4389

    // golden :4391-4392 的兩個 static。ST-W1/W2/W3 都沒有用到，ST-W4 才第一次讀
    //（bflag 在 :5407/:5417/:5423/:5438，sRecipe 在 :5627/:5630），所以到這一波才加。
    static bool bflag = false;                                                  // golden :4391
    static AnsiString sRecipe = "";                                             // golden :4392

    // AI(W906-ST-W4) 20260915: bflag 的**唯一讀取點**（golden :5407）在
    // SAFETY-GATE(W906-ST-W4-F) 裡，所以現在只剩 golden :5423/:5438 兩個寫入，
    // -Wunused-but-set-variable 會紅。
    // ★ 不刪那兩個寫入 —— 它們是 golden 的行為，W4-F 解閘後就要用到；
    //   刪掉會讓解閘的人以為狀態機本來就沒有這一段。
    // 所以明確標註「知道它現在沒人讀」，而不是靠關警告。
    (void)bflag;

    if (iStartIn != 0)                                                          // golden :4394
        return false;                                                           // golden :4395

    NewRecordProcess("MES2110", "START pressed", Func);                         // golden :4397
    iStartIn = 1;                                                               // golden :4398
    ResetShtMoveTimeoutWatchdog();                                              // golden :4399  AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : 任何 Start 入口清 watchdog

    // golden :4401-4406 的區域變數。只保留已翻到的範圍用得到的，理由同上。
    int ret = 0, iMode = 0, iFix = 0;                                                   // golden :4401（ret 在 ST-W2 :4699、iMode 在 ST-W4 :5451 用）
    bool bFlag = false;                                                         // golden :4402（ST-W4 :5447 用）
    bool bHasAlm = false;                                                       // golden :4404（ST-W6 :6053 用）
    AnsiString str = "";                                                        // golden :4405（ST-W6 :6058 用）
    bool bArmAllSiteClose[2] = { true, true };                                  // golden :4403
    AnsiString Str = "";                                                        // golden :4405（ST-W3 :5126 用）
    AnsiString Str2 = "";                                                       // golden :4405（ST-W2 :5027 用）

    fTemp_Set->InitialAddrToATC();                                              // golden :4408  Steven 20241105 : 更新轉換矩陣

    if (LastSet.iTester == OFF_LINE)                                            // golden :4410  jou 20211101 : 保持在上面
        SendCommand_ESD(ESD_TestOFFLine);                                       // golden :4411  kevin 20180820
    else
        SendCommand_ESD(ESD_TestOnLine);                                        // golden :4413  kevin 20180816

    if (INSTALL_OCR != eocrUninstal || BAR_CODE_INSTALL != ebctUninstall)       // golden :4415
    {
        if (TestIF_File.bOcrFunction || TestIF_File.bEnableBarCode)             // golden :4417  Steven 20160912 : 避免沒開啟 2D function
            fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);                          // golden :4418
        else
            fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);                         // golden :4420
    }
    else if (INSTALL_OCR == eocrUninstal && BAR_CODE_INSTALL == ebctUninstall)  // golden :4422
    {
        fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);                             // golden :4424
    }

    if (TestIF_File.b2DUsePinInspection)                                        // golden :4427  Ifor 20240528 add:Pin1 Function
        fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);                         // golden :4428
    else
        fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);                        // golden :4430

    // -- golden :4432-4439 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-A) golden :4434 `patFunc->CheckMachineStationOnStart()`
    //   `patFunc`（CC_PANTHER 客戶物件）全樹不存在 —— 實測 forms/fMain.h 與
    //   cmydef.h 都沒有。這是**擋啟動**的檢查：golden 在它回 false 時
    //   `iStartIn=0; return false;`。
    //   閘掉的後果是「CC_PANTHER 機台少一道啟動前站點檢查」。這台不是
    //   CC_PANTHER，所以現況無影響；但要出貨給 PANTHER 之前必須補。
#if 0
    if (CUSTOMER_CODE == CC_PANTHER)                                            // golden :4432
    {
        if (patFunc->CheckMachineStationOnStart() == false)                     // golden :4434
        {
            iStartIn = 0;                                                       // golden :4436
            return false;                                                       // golden :4437
        }
    }
#endif

    // -- golden :4441-4460 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-B) golden :4445/:4449/:4454/:4458
    //   `COM2->SendCommToVision(COM2->rtAUTOTEACHON, ...)` 等四個 RTC 視覺列舉
    //   （rtAUTOTEACHON / rtAUTOTEACHOFF / rtSTEPASIDEMODEON / rtSTEPASIDEMODEOFF）
    //   在移植樹不存在。這是**唯讀方向的通知**（告訴視覺系統現在的模式），
    //   不擋啟動，所以閘掉不會讓 Start 誤判成功。
    //   後果：裝了 RTC 的機台，視覺端不會收到 auto-teach / step-aside 模式切換。
#if 0
    if (REAL_TIME_CCD == true && !COM2->bCCDDummyRum)                           // golden :4441  Ifor 20230908 add:RTC 讓位功能 By Setup File
    {
        if (bRTCAutoTuning)                                                     // golden :4443  Sam 20240711 : RTC Auto Tuning
            COM2->SendCommToVision(COM2->rtAUTOTEACHON, true);                  // golden :4445
        else
            COM2->SendCommToVision(COM2->rtAUTOTEACHOFF, true);                 // golden :4449

        if (TestIF_File.bUseRTCStepAsideMode == true)                           // golden :4452
            COM2->SendCommToVision(COM2->rtSTEPASIDEMODEON, true);              // golden :4454
        else
            COM2->SendCommToVision(COM2->rtSTEPASIDEMODEOFF, true);             // golden :4458
    }
#endif

    if (CosFunction.bFTPFunction)                                               // golden :4462  Ifor 20231101 add:FTP Function
    {
        if (IniConfig.bEnableFTP == true)                                       // golden :4464
            fMain->SendMSG_CMD(MSG_CMD_EnableFTPFunction);                      // golden :4466
        else
            fMain->SendMSG_CMD(MSG_CMD_DisableFTPFunction);                     // golden :4470
    }

    if (CompareTechData() == false)                                             // golden :4474  Steven 20220221 : 針對 Teaching 值做防呆
    {
        iStartIn = 0;                                                           // golden :4476
        return false;                                                           // golden :4477
    }

    LogSoftwareOnTime("HT9045, Start(1)");                                      // golden :4480

    AutoCalculateInArmYClosePitch(true, false);                                 // golden :4482  Steven 20190314 : for Y-Pitch

    // AI(W906-ST-W7-H) 20260915: W1-C 已還債 —— golden BarCode.cpp:8209-8246 那 38 行
    //   翻好了（見本檔上方 W906ST_Read2DIDList），gate 解除。
    //   ⚠ 兩個消費者尚未移植，所以今天載入的清單沒有人讀 —— 見該函式檔頭。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    W906ST_Read2DIDList();                                                      // golden :4483  Steven 20190604 : 2DID 不在 List 內的另外分 bin

    if (fLotInfo->bStartChamberBoost)                                           // golden :4485  Steven 20191128 : Chamber Boost Function
    {
        iStartIn = 0;                                                           // golden :4487
        RecordProcess("Pause by chamber boost function");                       // golden :4488
        return false;                                                           // golden :4489
    }

#ifndef SOFT_SIMULTE                                                            // golden :4492
    if (IniConfig.bA01PressStartAutoSwitchToOperatorMode)                       // golden :4493  JerryYang 20170705 (Steven) for JCET 按 start 要切為 OP 權限
    {
        AccessLevel = 0;                                                        // golden :4495
        // SAFETY-GATE(W906-ST-W1-K) golden :4496 `spbUserName->Caption`
        //   forms/fMain.h 沒有 spbUserName（cCounterClear.cpp:419 的 GATE(CC3)
        //   註記同一件事，那邊也是整段 #if 0）。
        //   ★ 只閘這一行，不閘整段：AccessLevel=0 與 ChangeLevelAttr() 是
        //     真正的行為（權限降級），spbUserName 只是把降級結果顯示出來。
        //     整段閘掉會讓 JCET 按 START 不再切成 OP 權限 —— 那是行為變更。
#if 0
        spbUserName->Caption = "Operator";                                      // golden :4496
#endif
        fMain->ChangeLevelAttr();                                               // golden :4497
    }

    if (CUSTOMER_CODE == CC_SIGURD_HUKOU ||                                     // golden :4500  KaiChen 20191128：矽格-湖口，軟體重啟時檢查所有 Tray
        CUSTOMER_CODE == CC_SIGURD_SUZHOU)                                      // golden :4501
    {
        if (bCheckTrayBySoftWareOpen == false)                                  // golden :4503
        {
            if (CheckFixTray() == false)                                        // golden :4505
            {
                iStartIn = 0;                                                   // golden :4507
                return false;                                                   // golden :4508
            }
        }
    }
#endif                                                                          // golden :4512

    // -- golden :4513-4526 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-D) golden :4521/:4522
    //   `fTrayMapping->tsLoaderDeviceCheck` 與 `fTrayMapping->PageControl3`
    //   兩個 widget 在 forms/fTrayMapping.h 不存在（那是換頁籤的顯示動作）。
    //   ⚠ 但 **`return false` 那一條不可以一起閘掉** —— 它是擋啟動的，
    //     閘掉會讓 CC_ASE_CL 在該檢殘留 device 時照樣啟動。
    //     所以這裡只閘「換頁籤」那兩行，檢查與 return 照翻。
    if (CUSTOMER_CODE == CC_ASE_CL)                                             // golden :4513
    {
        if (fTrayMapping->IsRunDeviceRemainLaser() && bNeedDoRemainCheck == true) // golden :4515
        {
            ShowMyMessage("請先手動執行Color空tray檢,確認Color tray上有殘留device可正常檢出"); // golden :4517
            // SAFETY-GATE(W906-ST-W1-D) golden :4518-4523
            //   forms/fTrayMapping.h 的 facade 沒有 fShow / Show / PageControl1 /
            //   PageControl3 / tsLoaderDeviceCheck / TabSheet2 —— 整段是
            //   「把 Tray Mapping 視窗叫出來並切到該頁籤」的顯示動作。
            //   ★ 只閘顯示。下面那個 return false 是**擋啟動**的，
            //     閘掉會讓 CC_ASE_CL 在該檢殘留 device 時照樣啟動。
#if 0
            if (fTrayMapping->fShow == false)                                   // golden :4518
            {
                fTrayMapping->Show();                                           // golden :4520
                fTrayMapping->PageControl1->ActivePage = fTrayMapping->tsLoaderDeviceCheck; // golden :4521
                fTrayMapping->PageControl3->ActivePage = fTrayMapping->TabSheet2;           // golden :4522
            }
#endif
            return false;                                                       // golden :4524  ★ 擋啟動，不可閘
        }
    }

    if (CUSTOMER_CODE == CC_ASE_CL)                                             // golden :4528  KaiHuang 20200728 : For ASE-CL A168 產品不能開燈測試
    {
        SW[SwCCDLight].Off();                                                   // golden :4530  ★ 真正的 IO 動作，照翻
        // SAFETY-GATE(W906-ST-W1-L) golden :4531-4532 `spbLight`
        //   forms/fMain.h 沒有 spbLight。純顯示（把燈已關這件事反映到按鈕上）。
        //   ★ 同樣只閘顯示，不閘 SW[SwCCDLight].Off() —— 那一行才是「關燈」。
#if 0
        spbLight->Tag = 0;                                                      // golden :4531
        spbLight->Caption = "Light OFF";                                        // golden :4532
#endif
    }

    // -- golden :4535-4542 ----------------------------------------------------
    // AI(W906-ST-W7-C) 20260915: W1-E 已**部分**還債 —— CheckOLPError() 的判定翻好了
    //   （見本檔上方），所以擋啟動的那一半解閘，golden :4535-4541 原文恢復。
    //   ⚠ 只有三個 ShowMyMessagePWD 對話框仍被閘在該函式內
    //     （SAFETY-GATE(W906-ST-W7-C-DLG)）—— 擋得住，但不會告訴操作員為什麼。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    if (CosFunction.bOLPFunction)                                               // golden :4535  Sam 20230921 : Bin 設定錯誤不能啟動
    {
        if (CheckOLPError())                                                    // golden :4537
        {
            iStartIn = 0;                                                       // golden :4539
            return false;                                                       // golden :4540
        }
    }

    if (fLotInfo->CheckEventLogParameter() == false)                            // golden :4544  Steven 20181224 : For ASE-CL
    {
        iStartIn = 0;                                                           // golden :4546
        return false;                                                           // golden :4547
    }

    if (CUSTOMER_CODE == CC_SIGURD_ChungXing && bInitailQtyByLowYield)          // golden :4550  Sam 20230728 : 發生 LowYield 後重新 Start 清除 Sort Count
    {
        bInitailQtyByLowYield = false;                                          // golden :4552
        fMain->Clarn_Data(11, "LowYield Start Clear");                          // golden :4553
    }

    // -- golden :4556-4560 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-F) golden :4558/:4560 `lbl2DSort->Visible`
    //   forms/fMain.h 沒有 lbl2DSort 這個 widget。純顯示（2D sort 標籤的可見性），
    //   不擋啟動，也不影響任何判斷。
#if 0
    if (CosFunction.bCanUseSearch2DIDByLot &&                                   // golden :4556  Steven 20220811 : ATK request 2D sort display.
        TestIF_File.bEnableBarCode && TestIF_File.bSearch2DIDByLot && LastSet.iTester == OFF_LINE)
        lbl2DSort->Visible = true;                                              // golden :4558
    else
        lbl2DSort->Visible = false;                                             // golden :4560
#endif

    if (IniConfig.bA09_ByArmCloseSite && IniConfig.bA09_1_AutoCloseArm)         // golden :4562  Steven 20220819 : 單 Arm Site 全關時就把 Arm 關了
    {
        if (bUseTwoArm32Site == false)                                          // golden :4564
        {
            for (int z = 0; z < 2; z++)                                         // golden :4566
            {
                bArmAllSiteClose[z] = true;                                     // golden :4568
                for (int i = 0; i < FTestSuck.iShtRow; i++)                     // golden :4569
                {
                    for (int j = 0; j < FTestSuck.iShtCol; j++)                 // golden :4571
                    {
                        if (bTestSiteUse[z][i][j])                              // golden :4573
                        {
                            bArmAllSiteClose[z] = false;                        // golden :4575
                        }
                    }
                }
            }
        }

        if (bArmAllSiteClose[0] == true)                                        // golden :4582
        {
            TestIF_File.iShuttleMode = 1;                                       // golden :4584
            TestIF_File.iShuttle_Sel = 1;                                       // golden :4585
        }
        else if (bArmAllSiteClose[1] == true)                                   // golden :4587
        {
            TestIF_File.iShuttleMode = 1;                                       // golden :4589
            TestIF_File.iShuttle_Sel = 0;                                       // golden :4590
        }

        AnsiString szDir = GetRecipePath() + "\\HandlerCondition.Data";         // golden :4593
        WriteIniData(szDir, "Configuration", "Shuttle Mode",    TestIF_File.iShuttleMode); // golden :4594
        WriteIniData(szDir, "Configuration", "Shuttle1 Cancel", TestIF_File.iShuttle_Sel); // golden :4595
    }

    // -- golden :4598-4604 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-G) 🟡 golden :4602 `CheckEmployeeID(strShowAUTHORITY)`
    //   golden mymessbox.cpp:1353-1369（17 行）。SECS/GEM 員工 ID 確認。
    //   🟡：不擋啟動（golden 沒有用它的回傳值）。
    //
    //   ⚠⚠ **20260915 更正：原本寫「ST-W7 補」，那個判斷是錯的。**
    //   它只有 17 行，但**行數不是難度** —— 它是 `ShowModal()` 家族，
    //   本戰役最硬的一類（見 `docs/FSHOW_WEB_VISIBILITY_DESIGN.md` §6）。
    //
    //   golden 的流程是：把提示字寫進 fPassword 的兩個 Label → **ShowModal()
    //   阻塞等操作員輸入帳密** → 讀 edUserName/edPassword → `EventReport(
    //   SECS_EVENT.BarcodeReaderEnter)` 把憑證回報給 host。
    //
    //   移植樹 `forms/fPassword.h:337` 是 **`void ShowModal() {}`**（離線 no-op），
    //   同檔 :164-172 自己寫明理由：
    //     「golden's modal loop does not exist headless … the web layer owns show/dismiss」
    //
    //   ⇒ 照翻的後果**比閘掉糟得多**：ShowModal 立刻返回、讀到空的帳號密碼、
    //     然後**對 host 送出一個帶空白憑證的 SECS 事件** ——
    //     等於向 host 謊報「操作員已通過身分確認」。
    //     這不是「少做一件事」，是**做錯一件對外的事**。
    //
    //   相依現況（編譯探針實測 20260915）：`fPassword` / `edUserName` / `edPassword`
    //   都在；缺 `Label6`、`EventReport`、`SECS_EVENT`。
    //   但**就算把那三個補齊也不該解閘** —— 真正的阻塞點是 ShowModal 的往返語意。
    //
    //   ⛔ 歸屬：跟 §5.2 的 fShow 同一個設計題，而且是其中最難的「往返」那一種。
    //     要等 web 層的 show/dismiss 通道，屬 FW-W 指令通道，不是本戰役。
#if 0
    if (IniConfig.bN07_EnableEmployeeIdCheak == true)                           // golden :4598  Ifor 20180227 (Steven) SECS GEM Confirm the Employee ID
    {
        if (bWaitSecsGemReply == true)                                          // golden :4600
        {
            CheckEmployeeID(strShowAUTHORITY);                                  // golden :4602
        }
    }
#endif

    // -- golden :4606-4621 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-H) golden :4613 `fFTPClient->DownloadUpdateAutomatically()`
    //   移植樹沒有 fFTPClient 的這個方法。
    //   ⚠ 這是**擋啟動**的路徑：golden 在 bUpdateAutomatically 為真時
    //     `iStartIn=0; return false;`（有更新包要裝就不讓開機台）。
    //     閘掉的後果：開了 N32 自動更新的機台，會在有待裝更新時照樣啟動。
#if 0
    if (CosFunction.bDownloadUpdateAutomatically &&                             // golden :4606
        IniConfig.bN32_CheckAtInitailStart)                                     // golden :4607  Sam 20220824 : FTP 自動下載安裝更新包
    {
        #ifndef SOFT_SIMULTE
        if (LastSet.iRunStartMode == rsmInitialStart ||                         // golden :4610
            LastSet.iRunStartMode == rsmCInitialRetest)                         // golden :4611  Sam 20221018 : Initail RT 也要
        {
            fFTPClient->DownloadUpdateAutomatically();                          // golden :4613
            if (bUpdateAutomatically)                                           // golden :4614
            {
                iStartIn = 0;                                                   // golden :4616
                return false;                                                   // golden :4617
            }
        }
        #endif
    }
#endif

    // -- golden :4623-4685 ----------------------------------------------------
    // SAFETY-GATE(W906-ST-W1-I) golden :4633/:4649/:4660/:4675 `fLotInfo->CheckingCheckList()`
    //   golden uLotInfo.cpp:12413（10 行）尚未移植。
    //   ⚠ 這整段是**擋啟動**的（Sigurd FTP Automation 的比對清單，回傳非 "OK"
    //     就 `iStartIn=0; ShowErrorMessage("WAR16333",...); return false;`）。
    //     閘掉的後果：開了 A32 FTP Automation 的矽格機台，比對清單不合也照樣啟動。
    //     ST-W7 必須補 —— 這一條是本波所有 gate 裡影響最大的。
#if 0
    if (IniConfig.bA32EnableFTPAutomation == true &&                            // golden :4623  KaiChen 20190530：Sigurd FTP Automation
        fContact->fShow == false)                                               // golden :4624  KaiChen 20200603：Contact 頁面中不檢查
    {
        #ifndef SOFT_SIMULTE
        fLotInfo->ShowInformation(false);                                       // golden :4627  KaiChen 20191111：權限卡關顯示
        if (CUSTOMER_CODE == CC_SIGURD_ChungXing &&                             // golden :4628
            IniConfig.bA32_3For93KFunction == false)                            // golden :4629  Sam 20220620
        {
            if (LastSet.iRunStartMode == rsmContinuStart)                       // golden :4631
            {
                AnsiString asR = fLotInfo->CheckingCheckList();                 // golden :4633
                if (asR != "OK")
                {
                    iStartIn = 0;
                    ShowErrorMessage("WAR16333", 0, MMSystem, false, asR);      // golden :4637
                    return false;
                }
            }
        }
        else if (CUSTOMER_CODE == CC_SIGURD_PeiXing ||                          // golden :4642
                 (CUSTOMER_CODE == CC_SIGURD_ChungXing &&
                  IniConfig.bA32_3For93KFunction == true))
        {
            if (LastSet.iRunStartMode == rsmInitialStart ||                     // golden :4646
                LastSet.iRunStartMode == rsmCInitialRetest)                     // golden :4647
            {
                AnsiString asR = fLotInfo->CheckingCheckList();                 // golden :4649
                if (asR != "OK")
                {
                    iStartIn = 0;
                    ShowErrorMessage("WAR16333", 0, MMSystem, false, asR);      // golden :4653
                    return false;
                }
            }
            else if (LastSet.iRunStartMode == rsmContinuStart ||                // golden :4657
                     LastSet.iRunStartMode == rsmContinuRetest)
            {
                AnsiString asR = fLotInfo->CheckingCheckList(true);             // golden :4660
                if (asR != "OK")
                {
                    iStartIn = 0;
                    ShowErrorMessage("WAR16333", 0, MMSystem, false, asR);      // golden :4664
                    return false;
                }
            }
        }
        else
        {
            if (LastSet.iRunStartMode == rsmContinuStart  ||                    // golden :4671
                LastSet.iRunStartMode == rsmContinuRetest ||
                LastSet.iRunStartMode == rsmInitialStart)
            {
                AnsiString asR = fLotInfo->CheckingCheckList();                 // golden :4675
                if (asR != "OK")
                {
                    iStartIn = 0;
                    ShowErrorMessage("WAR16333", 0, MMSystem, false, asR);      // golden :4679
                    return false;
                }
            }
        }
        #endif
    }
#endif

    // SAFETY-GATE(W906-ST-W1-J) golden :4688 `sbEngSite->Down = false`
    //   forms/fMain.h 沒有 sbEngSite（第三組工程師用的開關 SITE 按鈕）。
    //   純 UI 狀態，不影響判斷。
#if 0
    sbEngSite->Down = false;                                                    // golden :4688  Alick 20160926 add for 第三組工程師用開關 SITE
#endif
    bSiteUseEE = false;                                                         // golden :4689
    ShowTestHeadComp(false);                                                    // golden :4690
    i8PickerHPMode = fHotPlate->CheckHotPlate8PickMode();                       // golden :4691  JerryYang 20161010 判斷 HotPlate Offset 10mm
    LogSoftwareOnTime("HT9045, Start(2)");                                      // golden :4692

    // =========================================================================
    //  ST-W2  golden main.cpp:4694-5044（351 行）
    //  AI(W906-ST-W2) 20260915
    // =========================================================================

    // SAFETY-GATE(W906-ST-W2-A) golden :4694-4695 `FormBarcodeReader`
    //   ⚠ 20260916 重驗：原本寫的理由「全樹沒有 FormBarcodeReader」**已經過期**。
    //   它有：`BarcodeReader.cpp:40` 定義 `TFormBarcodeReader *FormBarcodeReader;`，
    //   `CMakeLists.txt:2354` 自 20260824 起就把 `BarcodeReader.cpp` 編進去，
    //   而 `BarcodeReader.h:75/:102/:108` 的 class／`bShow`／`Close()` 都齊全。
    //   **閘仍然要留，但理由是另一個**：唯一的建構點是 `FormsBootstrap.cpp:203`
    //   的 `InitForms()`，而 `FormsBootstrap.cpp:129` 記著「NOTHING CALLS
    //   InitForms() YET」—— 20260916 用 `git grep InitForms` 重驗**仍然成立**
    //   （全樹只有定義與註解，零呼叫點）。所以這個指標在執行期是 **NULL**，
    //   解閘等於在 CC_KYEC_LEE 按 START 時 NULL 解參考。
    //   ⇒ 要開這個閘，先決定「誰來呼叫 InitForms()」。那是使用者的題目。
    //   純視窗關閉動作（KYEC 要求按 START 時把 BarCode 視窗強制關掉，避免
    //   ALARM 時視窗卡死）。不擋啟動。
    //   後果：CC_KYEC_LEE 機台按 START 時 BarCode 視窗不會自動關。
#if 0
    if (CUSTOMER_CODE == CC_KYEC_LEE && FormBarcodeReader->bShow)               // golden :4694  Ifor 20161229 (Steven)
        FormBarcodeReader->Close();                                             // golden :4695
#endif

    if (CUSTOMER_CODE == CC_UNISEM_M)                                           // golden :4697  Ifor 20171018 (wei) : UNISEM Start 時判斷壓力
    {
        ret = 0;                                                                // golden :4699
        if (DeviceForm_File.dPress > 20)                                        // golden :4700
        {
            ret = ShowErrorMessage("WAR0324", K_RETRY | K_SKIP, MMSystem);      // golden :4702
        }
        else if (DeviceForm_File.dPress < 5)                                    // golden :4704
        {
            ret = ShowErrorMessage("WAR0325", K_RETRY | K_SKIP, MMSystem);      // golden :4706
        }

        if (ret == K_RETRY)                                                     // golden :4709
        {
            iStartIn = 0;                                                       // golden :4711
            return false;                                                       // golden :4712
        }
    }

    // AI(W906-ST-S2-fShow-B1) 20260917: SAFETY-GATE(W906-ST-W2-B) **已解開**。
    //   原因消失了：舊註解「全樹沒有 TrayEditForm」已過期，現在有了
    //   （acatchtray_shims.h 的 TTrayEditForm_Facade，兩個成員的 stand-in）。
    //
    //   ⚠ 解閘前先問了「false 是 golden 的正常路徑，還是保護路徑？」——
    //   這裡是**正常路徑**：離線沒有視窗開著，所以 `fShow == true` 不成立、
    //   整個 if 不執行，與閘住時的行為**完全相同**。差別只在它從此編得過、
    //   被型別檢查過，而不是一段沒人編過的文字。
    //
    //   （同一波裡 W5-F / W6-D / W6-E 沒有跟著解：W5-F 的 false 是**保護**那一支，
    //   解開會新增一個擋啟動；W6-D / W6-E 還缺 RunCheckStart()，全樹不存在。
    //   見 docs/ST_FSHOW_SIMPLE_PROPOSAL.md。）
    if (W906_FShow("TrayEditForm", TrayEditForm->fShow) == true)                                            // golden :4716  JerryYang 20240111
        TrayEditForm->Close();                                                  // golden :4717

    bUse8Picker = false;                                                        // golden :4719  Steven 20220927 : 整合 8 吸嘴 auto clean
    if (IniConfig.bE43AutoCleanUseHotplate)                                     // golden :4720
    {
        bUse8Picker = true;                                                     // golden :4722
    }
    else if (MachineTypeChoice == Type_HT9046_LS ||                             // golden :4724
             MachineTypeChoice == Type_HT1032)                                  // golden :4725
    {
        // golden :4727/:4729-4730 是三行被作者自己註解掉的舊分支，照原樣不還原。
        bUse8Picker = true;                                                     // golden :4728
    }

#ifndef SOFT_SIMULTE                                                            // golden :4733
    // AI(W906-ST-W7-D) 20260915: W2-C 已還債 —— golden uCleaning.cpp:2879 那 37 行翻好了
    //   （見本檔上方 W906ST_CheckSmartAutoCleanCanStart），gate 解除。
    //   ⚠ 呼叫的是自由函式而不是 `fCleaning->...`：移植樹的 TfCleaning facade
    //     沒有宣告過這個方法，而 golden 用到的狀態在移植樹全部是全域（實測），
    //     所以換一個 host 不改變讀到的東西。理由詳見該函式檔頭。
    //   ⚠ golden 這裡**沒有** `iStartIn = 0;`（跟其他擋啟動點不同），照翻不補。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    if (W906ST_CheckSmartAutoCleanCanStart() == false)                          // golden :4734
        return false;                                                           // golden :4735

    // SAFETY-GATE(W906-ST-W2-D) golden :4737-4755 `fPMAlarmInterFace` / `PMAlarm_SYS`
    //   ⚠ **擋啟動**（A19 PM Alarm：到了 PM 停機日就不讓開機台）。
    //   全樹沒有 fPMAlarmInterFace 這個介面物件。
    //   閘掉的後果：開了 A19 的機台，到了 PM 停機日照樣能啟動，
    //   而且 PM 警告訊息也不會跳。
#if 0
    if (IniConfig.bA19UsePMAlarmFunction == true &&                             // golden :4737  2015.05.26 Mylin PM Alarm
        HasICUnderMachine() == false &&                                         // golden :4738
        fAutoTeach->IsRun() == false)                                           // golden :4739  JimmyChiu 20211020 : Auto alignment mode
    {
        if (fPMAlarmInterFace->PMAlarm_SYS.IsStopDate() == true)                // golden :4741
        {
            iStartIn = 0;                                                       // golden :4743
            ShowMyMessage("PM Stop When Lot Start! Please PM Machine");         // golden :4744
            return false;                                                       // golden :4745
        }
        else if (fPMAlarmInterFace->PMAlarm_SYS.IsAlarmDate() == true)          // golden :4747
        {
            if (bIsShowPMAlarmMessage == true)                                  // golden :4749  wei 20160225 Mylin Modify PM Alarm Show One Time
            {
                ShowMyMessage("PM Alarm When Lot Start! Please PM Machine");    // golden :4751
                bIsShowPMAlarmMessage = false;                                  // golden :4752
            }
        }
    }
#endif

    // SAFETY-GATE(W906-ST-W2-E) golden :4757-4769 `fProductionInfo->CheckOEE_WhenStart()`
    //   ⚠ **擋啟動**（JimmyChiu 20220125 集中於 OEE 判斷）。
    //   forms 的 fProductionInfo facade 沒有 CheckOEE_WhenStart。
    //   閘掉的後果：開了 OEE 的機台（memory：bOEEFunction 只有超豐開），
    //   OEE 檢查不通過也照樣啟動。
#if 0
    if (CosFunction.bOEEFunction)                                               // golden :4757
    {
        AnsiString asErrorMsg;                                                  // golden :4759
        if (fProductionInfo->CheckOEE_WhenStart(asErrorMsg) == false)           // golden :4760
        {
            if (asErrorMsg != "")                                               // golden :4762
            {
                iStartIn = 0;                                                   // golden :4764
                ShowMyMessage(asErrorMsg);                                      // golden :4765
                return false;                                                   // golden :4766
            }
        }
    }
#endif
#endif                                                                          // golden :4770

    if (TestIF_File.iTestType == TCP_IP_MODE &&                                 // golden :4772  wei 20211027 open short TCP/IP
        LastSet.iTester == ON_LINE)                                             // golden :4773  Steven 20231113
    {
        if (fTesterTCP->bConnectOK == false)                                    // golden :4775
        {
            iStartIn = 0;                                                       // golden :4777
            ShowMyMessage("TCP/IP does not connect!! Please check TCP/IP connect!!"); // golden :4778
            return false;                                                       // golden :4779
        }
    }
    // SAFETY-GATE(W906-ST-W2-H) golden :4782-4831 `bFind`  [RETIRED 20260928 AI(W906-GB-P8) 20260928 (St02-E helper): see :1721]
    //   ⚠ **擋啟動**。`bFind` 全樹不存在（只有 bFindPickICFail / bFindProgram）。
    //   golden 用它判斷 GPIB / RS232 / TTL-RS232 板子有沒有被找到，
    //   找不到就 ShowMyMessage("... no execute!!") 並 return false；
    //   版本不對（bGpibRS232Error）也擋。
    //   閘掉的後果：**ON_LINE 模式下 GPIB / RS232 沒接好也能按 START**，
    //   而且板子版本不符也不會被擋。這一條的影響面比其他 gate 大 ——
    //   它涵蓋 TCP_IP / GPIB / RS232 / TTL 四種 Tester 介面。
    //   連帶閘掉 CheckTTLBoardBitMode()（golden :4826，該函式本身是存在的）。
//#if 0   AI(W906-GB-P8) 20260928 (St02-E helper): gate retired (B4) -- its reason no longer holds: golden bFind is THandlerTesterSide::bFind, read here as W906_TesterBridgeFound() (forms/fMain.h:1419, ledger T10, the seat atester GetTesterResult already reads), bGpibRS232Error cmydef.h:3732, CheckTTLBoardBitMode csystem.h:292, golden 906_0625_Steven main.cpp:4782-4831 (912 :4988-5037)
    else if ((TestIF_File.iTestType == TCP_IP_MODE &&                           // golden :4782
              LastSet.iTester != ON_LINE) ||                                    // golden :4783
             ((TestIF_File.iTestType == GPIB_MODE ||                            // golden :4784
               TestIF_File.iTestType == RS232_MODE) &&                          // golden :4785
              LastSet.iTester == ON_LINE))                                      // golden :4786
    {
        if (W906_TesterBridgeFound() == false)                                                     // golden :4788  AI(W906-GB-P8) 20260928 (St02-E helper): golden bFind
        {
            iStartIn = 0;                                                       // golden :4790
            if (TestIF_File.iTestType == RS232_MODE)                            // golden :4791
                ShowMyMessage("RS232 no execute!! Please Check RS232 !!");       // golden :4792
            else
                ShowMyMessage("GPIB no execute!! Please Check GPIB !!");         // golden :4794

            return false;                                                       // golden :4796
        }

        if (bGpibRS232Error)                                                    // golden :4799  wei 20150617 Add version control
        {
            iStartIn = 0;                                                       // golden :4801
            if (TestIF_File.iTestType == RS232_MODE)                            // golden :4802
                ShowMyMessage("RS232 Version Error!! Please Check RS232 Version!! Must be V" + AnsiString(GPIBVersionCheck) + ".00"); // golden :4803
            else
                ShowMyMessage("GPIB Version Error!! Please Check GPIB Version !! Must be V" + AnsiString(GPIBVersionCheck) + ".00");  // golden :4805
            return false;                                                       // golden :4806
        }
    }
    else if (TestIF_File.iTestType == TTL_MODE &&                               // golden :4809  Isaac 20210511 : TTLRS232 板子版本檢查
             (TTL_CARD_TYPE == 2 || TTL_CARD_TYPE == 3))                        // golden :4810
    {
        if (W906_TesterBridgeFound() == false)                                                     // golden :4812  AI(W906-GB-P8) 20260928 (St02-E helper): golden bFind
        {
            iStartIn = 0;                                                       // golden :4814
            ShowMyMessage("TTL RS232 no execute!! Please Check TTL RS232 !!");  // golden :4815
            return false;                                                       // golden :4816
        }

        if (bGpibRS232Error)                                                    // golden :4819
        {
            iStartIn = 0;                                                       // golden :4821
            ShowMyMessage("TTL RS232 Version Error!! Please Check TTLBoard Version!! Must be " + AnsiString(TTLRS232VerCheck)); // golden :4822
            return false;                                                       // golden :4823
        }

        if (CheckTTLBoardBitMode() == false)                                    // golden :4826
        {
            iStartIn = 0;                                                       // golden :4828
            return false;                                                       // golden :4829
        }
    }

//#endif  // SAFETY-GATE(W906-ST-W2-H) 結束  AI(W906-GB-P8) 20260928 (St02-E helper): B4

    // SAFETY-GATE(W906-ST-W2-I) golden :4833-4837 `FormHS->CheckCanRunStart_HS()`
    //   ⚠ **擋啟動**（Ifor 20160818 整合海思 Start 判斷）。
    //   移植樹的 FormHS 是 W5SckArtRem_FormHSStub，沒有 CheckCanRunStart_HS。
    //   閘掉的後果：海思（HiSilicon）流程的啟動前判斷不執行。
#if 0
    if (FormHS->CheckCanRunStart_HS())                                          // golden :4833
    {
        iStartIn = 0;                                                           // golden :4835
        return false;                                                           // golden :4836
    }
#endif

    // SAFETY-GATE(W906-ST-W2-F) golden :4839-4842 `fSortCT->PageControl1`
    //   forms 的 fSortCT facade 沒有 PageControl1。純換頁籤
    //   （KYEC 要求 Start 時切到 Sort Count 頁面）。不擋啟動。
#if 0
    if (CUSTOMER_CODE == CC_KYEC_LEE)                                           // golden :4839  Ifor 20170413 (wei)
    {
        fSortCT->PageControl1->ActivePageIndex = 0;                             // golden :4841
    }
#endif

    // AI(W906-ST-W7-B) 20260915: W2-G 已還債 —— CheckInOutArmZHomeSensor() 翻好了
    //   （見本檔上方），所以這個 🔴 gate 解除。golden :4844-4851 原文恢復。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    //   ⚠ 這條路徑通往運動指令（見該函式的檔頭），S3 之前要單獨審。
    if (CosFunction.bCheckInOutArmZHomeSensor == true)                          // golden :4844
    {
        if (!CheckInOutArmZHomeSensor())                                        // golden :4846
        {
            iStartIn = 0;                                                       // golden :4848
            return false;                                                       // golden :4849
        }
    }

    LogSoftwareOnTime("HT9045, Start(3)");                                      // golden :4853

    // SAFETY-GATE(W906-ST-W2-J) golden :4855-4879 `fAutoTeach` / `fOffSet->fShow`
    //   ⚠ **擋啟動**。`fAutoTeach` 全樹不存在 —— forms/fOffSet.h:154-155 就明文
    //   記載「全樹沒有 class TfAutoTeach、沒有任何 extern ... fAutoTeach 宣告」。
    //   TfOffSet 也沒有 fShow / Show。
    //   ⚠ 這裡**不能只閘掉條件**：golden 的 else 分支會把 LastSet.bNeedSetupTeach
    //   清成 false，那是狀態寫入。拿掉條件、留 else，或反過來，都是發明行為。
    //   所以整段閘掉。
    //   閘掉的後果：開了 A30 Setup Teach 的機台，(a) ON_LINE 狀態下沒確認完
    //   setup teach 點位也能按 START；(b) 未啟用 A30 時 bNeedSetupTeach 不會被清。
#if 0
    if (IniConfig.bA30SetupTeachFunction &&                                     // golden :4855  JerryYang 20180921 Setup Teach 功能
        fAutoTeach->IsRun() == false)                                           // golden :4856  JimmyChiu 20211020
    {
        if (LastSet.bNeedSetupTeach)                                            // golden :4858
        {
            if (fContact->fShow == false)                                       // golden :4860
            {
                if (LastSet.iTester == ON_LINE)                                 // golden :4862
                {
                    iStartIn = 0;                                               // golden :4864
                    ShowMyMessage("請先切至Offline並確認所有setup teach點位");   // golden :4865
                    return false;                                               // golden :4866
                }

                if (fOffSet->fShow == false)                                    // golden :4869
                {
                    fOffSet->Show();                                            // golden :4871
                }
            }
        }
    }
    else
    {
        LastSet.bNeedSetupTeach = false;                                        // golden :4878
    }
#endif  // SAFETY-GATE(W906-ST-W2-J) 結束

    if (IniConfig.bO18SafeDoorOnOffDurationDetect)                              // golden :4881  JerryYang 20210112 : 安全門長時間未開啟要跳 alarm
    {
        int iY, iM, iD, iH, iLastTime, iNowTime;                                // golden :4883
        AnsiString str, sTime;                                                  // golden :4884

        iNowTime = SystemHour + ((SystemDate - 1) * 24) + ((SystemMonth - 1) * 30 * 24) + ((SystemYear - 1) * 12 * 30 * 24); // golden :4886
        for (int i = 0; i < MAX_SAFE_DOOR_CNT; i++)                             // golden :4887  JerryYang 20230704 : 整合安全門 15->MAX_SAFE_DOOR_CNT
        {
            if (Sen[iSafeDoor[i]].IsOn() == true)                               // golden :4889
            {
                if (iSafeDoor[i] == SnSafeDoor1 || iSafeDoor[i] == SnSafeDoor2 || iSafeDoor[i] == SnSafeDoor3 || // golden :4891
                    iSafeDoor[i] == SnSafeDoor6 || iSafeDoor[i] == SnSafeDoor7 || iSafeDoor[i] == SnSafeDoor8)   // golden :4892
                {
                    sTime = ReadIniData(asGeneralPath, "LastSafeDoorOpen", Sen[iSafeDoor[i]].Name, AnsiString("2020-12-20 00")); // golden :4894
                    iY = atoi(sTime.SubString(1, 4).c_str());                   // golden :4895
                    iM = atoi(sTime.SubString(6, 2).c_str());                   // golden :4896
                    iD = atoi(sTime.SubString(9, 2).c_str());                   // golden :4897
                    iH = atoi(sTime.SubString(12, 2).c_str());                  // golden :4898
                    iLastTime = iH + ((iD - 1) * 24) + ((iM - 1) * 30 * 24) + ((iY - 1) * 12 * 30 * 24); // golden :4899
                    if ((iNowTime - iLastTime) >= IniConfig.iO18SafeDoorOnOffDurationHour) // golden :4900
                    {
                        iStartIn = 0;                                           // golden :4902
                        str.sprintf("%s超過%d小時未開啟安全門", Sen[iSafeDoor[i]].Name, IniConfig.iO18SafeDoorOnOffDurationHour); // golden :4903
                        ShowMyMessage(str);                                     // golden :4904
                        return false;                                           // golden :4905
                    }
                }
            }
        }

        if (Sen[SnHeaterDoor].Enable == true && Sen[SnHeaterDoor].IsOn() == true) // golden :4911
        {
            sTime = ReadIniData(asGeneralPath, "LastSafeDoorOpen", Sen[SnHeaterDoor].Name, AnsiString("2020-12-20 00")); // golden :4913
            iY = atoi(sTime.SubString(1, 4).c_str());                           // golden :4914
            iM = atoi(sTime.SubString(6, 2).c_str());                           // golden :4915
            iD = atoi(sTime.SubString(9, 2).c_str());                           // golden :4916
            iH = atoi(sTime.SubString(12, 2).c_str());                          // golden :4917
            iLastTime = iH + ((iD - 1) * 24) + ((iM - 1) * 30 * 24) + ((iY - 1) * 12 * 30 * 24); // golden :4918
            if ((iNowTime - iLastTime) >= IniConfig.iO18SafeDoorOnOffDurationHour) // golden :4919
            {
                iStartIn = 0;                                                   // golden :4921
                str.sprintf("%s (chamber door)超過%d小時未開啟安全門", Sen[SnHeaterDoor].Name, IniConfig.iO18SafeDoorOnOffDurationHour); // golden :4922
                ShowMyMessage(str);                                             // golden :4923
                return false;                                                   // golden :4924
            }
        }

        if (Sen[SnHeaterDoor2].Enable == true && Sen[SnHeaterDoor2].IsOn() == true) // golden :4928
        {
            sTime = ReadIniData(asGeneralPath, "LastSafeDoorOpen", Sen[SnHeaterDoor2].Name, AnsiString("2020-12-20 00")); // golden :4930
            iY = atoi(sTime.SubString(1, 4).c_str());                           // golden :4931
            iM = atoi(sTime.SubString(6, 2).c_str());                           // golden :4932
            iD = atoi(sTime.SubString(9, 2).c_str());                           // golden :4933
            iH = atoi(sTime.SubString(12, 2).c_str());                          // golden :4934
            iLastTime = iH + ((iD - 1) * 24) + ((iM - 1) * 30 * 24) + ((iY - 1) * 12 * 30 * 24); // golden :4935
            if ((iNowTime - iLastTime) >= IniConfig.iO18SafeDoorOnOffDurationHour) // golden :4936
            {
                iStartIn = 0;                                                   // golden :4938
                str.sprintf("%s (chamber door)超過%d小時未開啟安全門", Sen[SnHeaterDoor2].Name, IniConfig.iO18SafeDoorOnOffDurationHour); // golden :4939
                ShowMyMessage(str);                                             // golden :4940
                return false;                                                   // golden :4941
            }
        }

        if (Sen[SnHeaterDoor3].Enable == true && Sen[SnHeaterDoor3].IsOn() == true) // golden :4945  Steven 20191016 : For ATC3.3 第三個加熱門
        {
            sTime = ReadIniData(asGeneralPath, "LastSafeDoorOpen", Sen[SnHeaterDoor3].Name, AnsiString("2020-12-20 00")); // golden :4947
            iY = atoi(sTime.SubString(1, 4).c_str());                           // golden :4948
            iM = atoi(sTime.SubString(6, 2).c_str());                           // golden :4949
            iD = atoi(sTime.SubString(9, 2).c_str());                           // golden :4950
            iH = atoi(sTime.SubString(12, 2).c_str());                          // golden :4951
            iLastTime = iH + ((iD - 1) * 24) + ((iM - 1) * 30 * 24) + ((iY - 1) * 12 * 30 * 24); // golden :4952
            if ((iNowTime - iLastTime) >= IniConfig.iO18SafeDoorOnOffDurationHour) // golden :4953
            {
                iStartIn = 0;                                                   // golden :4955
                str.sprintf("%s (chamber door)超過%d小時未開啟安全門", Sen[SnHeaterDoor3].Name, IniConfig.iO18SafeDoorOnOffDurationHour); // golden :4956
                ShowMyMessage(str);                                             // golden :4957
                return false;                                                   // golden :4958
            }
        }
    }

    if (IniConfig.bI40_bStartProductOnLine &&                                   // golden :4963  kevin 20180517
        AccessLevel < iDefEngineerLevel &&                                      // golden :4964  jou 2014-06-19 Security Have 5 Level
        bOneCycleOperateChangeON_line)                                          // golden :4965  kevin 20140407 生產前 OP OFF_LINE 強制 On line
    {
        iStartIn = 0;                                                           // golden :4967
        ShowErrorMessage("MES1053", 0, MMSystem, false, "Main--Start");         // golden :4968  Steven 20150202 : MES1051 --> MES1053
        return false;                                                           // golden :4969
    }

    if (bOneCycleOperateChangeON_line &&                                        // golden :4972
        iOff_LINE_Mode == 1 &&                                                  // golden :4973
        LastSet.iTester == ON_LINE)                                             // golden :4974  kevin 20140411 必須是 OFF_LINE
    {
        if (W906_FShow("fContact", fContact->fShow) == false && W906_FShow("fTemp_Set", fTemp_Set->fShow) == false)              // golden :4976
        {
            iStartIn = 0;                                                       // golden :4978
            ShowErrorMessage("MES1052", 0, MMSystem, false, "Main--Start");     // golden :4979
            return false;                                                       // golden :4980
        }
    }

    if (CosFunction.bSortingBy2DList == true &&                                 // golden :4984
        LastSet.iTester == _2D_SORT &&                                          // golden :4985
        TestIF_File.bSortingBy2DIDList == true)                                 // golden :4986  Frank 20221122 : 2DID sorting for ATK
    {
        if (IniConfig.bSPILFunction == true ||                                  // golden :4988
            CUSTOMER_CODE == CC_SJ_Semiconductor)                               // golden :4989
        {
            if (fLotInfo->cbRunMode->Text == "" ||                              // golden :4991
                fLotInfo->cbRunMode->ItemIndex == -1)                           // golden :4992  JerryYang 20230322
            {
                iStartIn = 0;                                                   // golden :4994
                ShowMyMessage("Please select the run mode in lot info!!");      // golden :4995
                return false;                                                   // golden :4996
            }
        }

        if (IniConfig.bA66_2D_Sort)                                             // golden :5000  JerryYang 20260201 : modify log
        {
            if (fLotInfo->edtCusStep->Text == "")                               // golden :5002
            {
                iStartIn = 0;                                                   // golden :5004
                ShowMyMessage("Please key in the Cus. step in lot info!!");     // golden :5005
                return false;                                                   // golden :5006
            }
        }

        if (RunInfo.bLotStart == false)                                         // golden :5010
        {
            iStartIn = 0;                                                       // golden :5012
            ShowMyMessage("Please Press Lot Start!!");                          // golden :5013
            return false;                                                       // golden :5014
        }
        else if (IniConfig.bVTESTFunction == true)                              // golden :5016  RogerYang 20250604 偉測不可複測 bin 功能
        {
            if (fLotInfo->CheckNoRetestBinFlag() == false)                      // golden :5018
            {
                iStartIn = 0;                                                   // golden :5020
                return false;                                                   // golden :5021
            }
        }

        // SAFETY-GATE(W906-ST-W2-K) golden :5025-5043 `fSCKART->iInfo_MultiLotCnt`
        //   ⚠ **擋啟動**（2DID 來源檔不存在就不讓開機台）。
        //   TfSCKART facade 沒有 iInfo_MultiLotCnt。
        //   ⚠ 這是 if / else-if / else 三岔，**只閘中間那個條件等於替 golden 選路**，
        //   所以整組（選檔 + FileExists 檢查）一起閘。
        //   閘掉的後果：2D sorting 模式下，SortBy2DID_*.csv 不存在也能按 START。
        //   注意上面那幾道檢查（cbRunMode / edtCusStep / RunInfo.bLotStart /
        //   CheckNoRetestBinFlag）**沒有**被閘，仍然照常擋。
#if 0
        if (IniConfig.iN23DownloadMethod == 1)                                  // golden :5025  Net Drive
        {
            Str2.sprintf("%s\\SortBy2DID_%s.csv", "D:\\HT9045_Log\\2D_SortList", fLotInfo->edtSysLotID->Text); // golden :5027
        }
        else if (fSCKART->iInfo_MultiLotCnt == 0)                               // golden :5029
        {
            Str2.sprintf("%s\\SortBy2DID_%s.csv", "D:\\HT9045_Log\\2D_SortList", fSCKART->sLotID); // golden :5031
        }
        else
        {
            Str2.sprintf("%s\\SortBy2DID_%s.csv", "D:\\HT9045_Log\\2D_SortList", sTotalLotID); // golden :5035
        }

        if (FileExists(Str2) == false)                                          // golden :5038  Steven 20160505 : 加上保護
        {
            iStartIn = 0;                                                       // golden :5040
            ShowMyMessage("2DID source file is missing!!");                     // golden :5041
            return false;                                                       // golden :5042
        }
#endif  // SAFETY-GATE(W906-ST-W2-K) 結束
    }

    // =========================================================================
    //  ST-W3  golden main.cpp:5045-5345（301 行）
    //  AI(W906-ST-W3) 20260915
    //
    //  ⚠ 結構：golden :5045 是 `else if`，接的是上面那個
    //     `if (CosFunction.bSortingBy2DList == true && ...)`（golden :4984）。
    //     所以 ST-W2 的收尾 return 拿掉了，這一段直接續上去。
    //     這也是為什麼波次邊界不能只看行數 —— 要看控制流在哪裡斷得開。
    // =========================================================================
    else if (TestIF_File.b2DIDAllowList)                                        // golden :5045  JerryYang 20241104 : 支援 2DID 白名單功能
    {
        // SAFETY-GATE(W906-ST-W3-A) golden :5047-5055 `fBarCode->list2DWhitle`
        //   ⚠ **擋啟動**。TfBarCode 沒有 list2DWhitle（2DID 白名單清單物件）。
        //   閘掉的後果：開了 2DID 白名單、且 N23 下載方式 = 2 的機台，
        //   白名單是空的也能按 START —— 等於白名單功能整個失效。
        //   ⚠ 這裡**只能整段閘**：golden 是 if/else 二岔，把 if 那支拿掉會讓
        //   else 那支（Lot Start / run mode 檢查）在 N23==2 時也跑，那是改行為。
        //   所以連 else 一起閘，整個 b2DIDAllowList 區塊都不執行。
#if 0
        if (IniConfig.iN23DownloadMethod == 2)                                  // golden :5047  JerryYang 20250320
        {
            if (fBarCode->list2DWhitle->Count == 0)                             // golden :5049
            {
                iStartIn = 0;                                                   // golden :5051
                ShowMyMessage("No 2DID white list!!");                          // golden :5052
                return false;                                                   // golden :5053
            }
        }
        else
        {
            if (RunInfo.bLotStart == false)                                     // golden :5058
            {
                iStartIn = 0;                                                   // golden :5060
                ShowMyMessage("Please Press Lot Start!!");                      // golden :5061
                return false;                                                   // golden :5062
            }
            else if (fLotInfo->cbRunMode->Text == "" ||                         // golden :5064  Steven 20250811 : Add run mode check
                     fLotInfo->cbRunMode->ItemIndex == -1)                      // golden :5065  JerryYang 20230322
            {
                iStartIn = 0;                                                   // golden :5067
                fLotInfo->sbSECSLotEnd->Down = true;                            // golden :5068
                ShowMyMessage("Please select the run mode in lot info!!");      // golden :5069
                return false;                                                   // golden :5070
            }
            else if (IniConfig.bVTESTFunction == true)                          // golden :5072  RogerYang 20250604 偉測不可複測 bin 功能
            {
                if (fLotInfo->CheckNoRetestBinFlag() == false)                  // golden :5074
                {
                    iStartIn = 0;                                               // golden :5076
                    return false;                                               // golden :5077
                }
            }
        }
#endif  // SAFETY-GATE(W906-ST-W3-A) 結束
    }

    if (CosFunction.bRMSNoNeedToDownloadEveryTime == false &&                   // golden :5083  Steven 20240926 : RMS 不要每次下載
        IniConfig.bShowLotInfo &&                                               // golden :5084
        IniConfig.bEnableRms == true &&                                         // golden :5085
        LastSet.bHasDownloadFile == false)                                      // golden :5086
    {
        iStartIn = 0;                                                           // golden :5088
        ShowErrorMessage("MES1680", 0, MMSystem, false, "Main--Start");         // golden :5089
        return false;                                                           // golden :5090
    }

    if (IniConfig.bEnable_SECS_GEM == true &&                                   // golden :5093  JerryYang 20190709 / Steven 20190723
        IniConfig.bSECS_GEM_OneCycle &&                                         // golden :5094
        CosFunction.bSECS_GEM_OneCycle &&                                       // golden :5095
        bSECSGEMConnectionFail)                                                 // golden :5096  wei 20150817 SECSGEM 斷線顯示 MES1650
    {
        if (bSECSGEMbyPass == false)                                            // golden :5098
        {
#ifndef SOFT_SIMULTE                                                            // golden :5100
            iStartIn = 0;                                                       // golden :5101
            ShowErrorMessage("MES1650", 0, MMSystem, false, "Main--Start");     // golden :5102  kevin 20140412
            return false;                                                       // golden :5103
#endif                                                                          // golden :5104
        }
    }

    LogSoftwareOnTime("HT9045, Start(4)");                                      // golden :5108

    if (W906_FShow("fContact", fContact->fShow) == true &&                                              // golden :5110
        iContactMode != CONTACT_NORMAL)                                         // golden :5111  Steven 20250328 : auto height 不看 lot id
    {
        ;                                                                       // golden :5113  golden 就是一個空敘述，照翻
    }
    else if (CUSTOMER_CODE == CC_KYEC_LEE ||                                    // golden :5115
             CUSTOMER_CODE == CC_KYEC_CHEN ||                                   // golden :5116
             CUSTOMER_CODE == CC_KYEC_JCTHIU ||                                 // golden :5117
             CUSTOMER_CODE == CC_KYEC_XILINX)                                   // golden :5118
    {
        if (((IniConfig.bEnable_SECS_GEM == true && iTrayFeed != 1) ||          // golden :5120
             IniConfig.bN10Enable_FTPUpLoadLog == true) &&                      // golden :5121
            W906_FShow("fContact", fContact->fShow) == false)                                           // golden :5122
        {
            AnsiString sLotID = fLotInfo->edtSysLotID->Text;                    // golden :5124
            AnsiString sOpID  = fLotInfo->edtSysOperatorID->Text;               // golden :5125
            Str.sprintf("Please Check LotID Length(Need %d Digits)!!", IniConfig.iLotIDLength); // golden :5126  Frank 20170531 (Steven) LotID 7 碼

            if (sLotID == "" || sOpID == "")                                    // golden :5128
            {
                iStartIn = 0;                                                   // golden :5130
                ShowMyMessage("Please Enter LotID and Operator ID!!");          // golden :5131
                return false;                                                   // golden :5132
            }
            else if (sLotID.Length() != IniConfig.iLotIDLength)                 // golden :5134
            {
                iStartIn = 0;                                                   // golden :5136
                ShowMyMessage(Str);                                             // golden :5137
                return false;                                                   // golden :5138
            }
            else if (RunInfo.bLotStart == false)                                // golden :5140
            {
                iStartIn = 0;                                                   // golden :5142
                ShowMyMessage("Please Press Lot Start!!");                      // golden :5143
                return false;                                                   // golden :5144
            }
        }
    }
    else if (((CUSTOMER_CODE == CC_TSMC_TAINAN &&                               // golden :5148
               IniConfig.bEnable_SECS_GEM == true &&                            // golden :5149
               IniConfig.bN07_EnableSecsLotCheck) ||                            // golden :5150
              CosFunction.bLotStartLockCriticalPara) &&                         // golden :5151  JerryYang 20220311 : ATP 鎖定 Critical parameter
             W906_FShow("fContact", fContact->fShow) == false)                                          // golden :5152  wei 20160517 TSMC lot 卡關
    {
        if (RunInfo.bLotStart == false)                                         // golden :5154
        {
            iStartIn = 0;                                                       // golden :5156
            ShowMyMessage("Please press Lot Check!!");                          // golden :5157
            return false;                                                       // golden :5158
        }
    }
    else if (CUSTOMER_CODE == CC_SCC ||                                         // golden :5161  Steven 20200306 : SCC 按 Lot End 要清空
             CUSTOMER_CODE == CC_JCET ||                                        // golden :5162
             CUSTOMER_CODE == CC_SJ_Semiconductor ||                            // golden :5163
             CUSTOMER_CODE == CC_AMKOR_China)                                   // golden :5164  Steven 20250819 : Add Amkor China
    {
        // golden :5165 是一行被作者註解掉的 `iTrayFeed!=1`，照原樣不還原。
        if (fLotInfo->edtSysLotID->Text == "" ||                                // golden :5167
            fLotInfo->edtSysOperatorID->Text == "")                             // golden :5168
        {
            iStartIn = 0;                                                       // golden :5170
            ShowMyMessage("Please Enter LotID and Operator ID!!");              // golden :5171
            return false;                                                       // golden :5172
        }
        else if (fLotInfo->cbRunMode->Text == "")                               // golden :5174
        {
            iStartIn = 0;                                                       // golden :5176
            ShowMyMessage("Please select run mode!!");                          // golden :5177
            return false;                                                       // golden :5178
        }
        else if (RunInfo.bLotStart == false)                                    // golden :5180
        {
            iStartIn = 0;                                                       // golden :5182
            ShowMyMessage("Please Press Lot Start!!");                          // golden :5183
            return false;                                                       // golden :5184
        }
    }
    else if (USE_RFID_READER)                                                   // golden :5187  Steven 20220713 : RFID Reader for SJSEMI
    {
        // ⚠ golden 這一支**只顯示訊息，不 return false、也不重設 iStartIn**。
        //   看起來像缺陷（提示完照樣往下跑，而且 iStartIn 留在 1），
        //   但那是 golden 的行為，照翻並註明。改它要使用者決定。
        if (fLotInfo->edtSysLotID->Text == "" ||                                // golden :5189
            fLotInfo->edtSysOperatorID->Text == "")                             // golden :5190
        {
            ShowMyMessage("Please Enter LotID and Operator ID!!");              // golden :5192
        }
        else if (fLotInfo->pnlLoader->Caption == "")                            // golden :5194  RogerYang 20260601 : add else
        {
            ShowMyMessage("Please Scan RFID of Loader!!");                      // golden :5196
        }
    }
    else if (CUSTOMER_CODE == CC_Murata ||                                      // golden :5199  Steven 20200612 : Add for Murata
             (CUSTOMER_CODE == CC_SIGURD_ChungXing &&                           // golden :5200  Ifor 20170505 (wei) 矽格中興廠 LotID 卡關
              IniConfig.bEnable_SECS_GEM == false) ||                           // golden :5201  Sam 20250401 : 中興開 SECSGEM 時不卡
             IniConfig.bVTESTFunction == true ||                                // golden :5202  jou 20200326 : VTest lot start 卡關
             CUSTOMER_CODE == CC_TSI ||                                         // golden :5203
             CUSTOMER_CODE == CC_CYUEAN ||                                      // golden :5204  Steven 20221225 : add for CyuEan
             ((IniConfig.bO23_InputLotIDByBarcode == true ||                    // golden :5205  Steven 20241224 : LotID 只能用 Barcode
               CosFunction.bHiSiliconFunction == true) &&                       // golden :5206
              (CUSTOMER_CODE == CC_SIGURD_PeiXing ||                            // golden :5207
               CUSTOMER_CODE == CC_JCET ||                                      // golden :5208
               CUSTOMER_CODE == CC_SCC ||                                       // golden :5209  Ifor 20170911 add SCC
               CUSTOMER_CODE == CC_AMD_M)))                                     // golden :5210  Ifor 20200915 : add TF AMD KL 版
    {
        // ====================================================================
        //  （已解閘）W906-Q29-LOTID golden :5212-5266
        //  AI(W906-Q29) 20260921：使用者 17:1x 裁決「乙」—— 保持閘住，但改成正式閘門。
        //
        //  緣由：這 59 行先前是被**裸註解**（逐行 `//`）掉的，沒寫理由。
        //  那種形式三個月後沒人分得出是暫時還是刻意。
        //
        //  ## ★ 缺的相依（這才是合法的加閘理由，不是「怕它擋住」）
        //
        //  `fLotInfo->edtSysLotID` / `edtSysOperatorID` 全樹**唯一的寫入點**是
        //  `tools/wb_serve.cpp:3088-3089` 的 `lot.start` 指令；
        //  而 `D:\HT9045\web` 底下 **665 個檔裡 `lot.start` 出現 0 次**
        //  （20260921 逐檔讀量，不是遞迴 grep —— 這棵樹的遞迴 grep 有回假空的前科）。
        //  ⇒ 瀏覽器端**沒有「Lot Start」這一步**，所以這兩個欄位永遠是空的，
        //     這個檢查永遠擋住。
        //
        //  ## ⚠⚠ 閘住期間的後果（波及的不只 CyuEan）
        //
        //  上面那個 `if`（本檔 :2140-2151）的條件同時涵蓋：
        //    CC_SIGURD_ChungXing（且 SECSGEM 關）/ bVTESTFunction / CC_TSI / CC_CYUEAN /
        //    bO23_InputLotIDByBarcode|bHiSiliconFunction 搭
        //    CC_SIGURD_PeiXing|CC_JCET|CC_SCC|CC_AMD_M
        //  ⇒ **七個以上的客戶組態，在閘住期間不再檢查工單號與操作員代號。**
        //  工單號是半導體廠的追溯要求，不是顯示欄位。
        //  同時被閘掉的還有：Lot Start 未按的檢查、VTEST 的 retest bin 檢查、
        //  以及 RT/FT 模式一致性檢查（golden :5234-5266）。
        //
        //  ## ★ UN-GATE 條件（寫死，不要靠記憶）
        //
        //  **Q27 落地後立即解閘** —— 網頁端送得出 `lot.start`、
        //  `edtSysLotID` 拿得到值的那一刻。把這個 `#if 0` 拿掉即可，
        //  裡面是 golden 的逐字原文，不需要重新翻譯。
        //  驗收：空著 LotID 按 START 要回得到
        //  「Please Enter LotID and Operator ID!!」而不是放行。
        //
        //  ## ★ 20260921 20:0x 解閘 —— UN-GATE 條件達成
        //
        //  閘的理由是「相依不存在：瀏覽器端沒有送出 `lot.start` 的入口」。
        //  那個相依現在**存在了**：
        //    * `web/page/ht9045_lotstart.js`（UI）➕ `ht9045_recipe_client.js`
        //      的 `lotStart`（送出）已落地
        //    * `tools/webprobe/q27_lotstart_probe.py` 端到端 **5 PASS / 0 FAIL**：
        //      ack 回 `RunInfo.bLotStart == true`，而且 realfile_guard 確認
        //      `config\config.ini` 的 `[Lot Info]` 真的被寫（驗完已還原）
        //
        //  ★ 按 §0.5，到這一刻**繼續閘住才是不合法的** ——
        //    加閘的唯一合法理由是「相依不存在」，而它已經不成立。
        //
        //  ⚠⚠ 現場影響：**START 在沒按 Lot Start 之前會被擋住**，
        //    訊息是「Please Enter LotID and Operator ID!!」。
        //    解法：控制面板下方的 **Lot Start** 區塊填兩個欄位再按。
        //    這是 golden 的行為，不是新增的限制。
        // ====================================================================
        if (fLotInfo->edtSysLotID->Text == "" ||                                // golden :5212
            fLotInfo->edtSysOperatorID->Text == "")                             // golden :5213
        {
            iStartIn = 0;                                                       // golden :5215
            ShowMyMessage("Please Enter LotID and Operator ID!!");              // golden :5216
            return false;                                                       // golden :5217
        }
        else if (RunInfo.bLotStart == false)                                    // golden :5219
        {
            iStartIn = 0;                                                       // golden :5221
            ShowMyMessage("Please press Lot Start!!");                          // golden :5222
            return false;                                                       // golden :5223
        }
        else if (IniConfig.bVTESTFunction == true)                              // golden :5225  RogerYang 20250604
        {
            if (fLotInfo->CheckNoRetestBinFlag() == false)                      // golden :5227
            {
                iStartIn = 0;                                                   // golden :5229
                return false;                                                   // golden :5230
            }
        }

        if (IniConfig.bVTESTFunction == true)                                   // golden :5234
        {
            if (fLotInfo->cbRunMode->Text.Pos("RT") > 0)                        // golden :5236
            {
                if (iRunStartMode != RT)                                        // golden :5238
                {
                    if (IniConfig.bI21EnableASM &&                              // golden :5240
                        iRunStartMode == rsmAutoSiteMap)                        // golden :5241  Steven 20221226 : For V-Test
                    {
                        // golden :5242-5243 是空的，照翻
                    }
                    else
                    {
                        iStartIn = 0;                                           // golden :5246
                        ShowMyMessage("Run RT Start mode error!!");             // golden :5247
                        return false;                                           // golden :5248
                    }
                }
            }
            else
            {
                if (iRunStartMode != FT)                                        // golden :5254
                {
                    if (IniConfig.bI21EnableASM &&                              // golden :5256
                        iRunStartMode == rsmAutoSiteMap)                        // golden :5257  Steven 20221226 : For V-Test
                    {
                        // golden :5258-5259 是空的，照翻
                    }
                    else
                    {
                        iStartIn = 0;                                           // golden :5262
                        ShowMyMessage("Run FT Start mode error!!");             // golden :5263
                        return false;                                           // golden :5264
                    }
                }
            }
        }
    }
    else if (IniConfig.bA38_SLT_Summary)                                        // golden :5270  JerryYang 20220923 : add for SLT lot summary
    {
        if (fLotInfo->edtSysLotID->Text == "" ||                                // golden :5272
            fLotInfo->edtSysOperatorID->Text == "")                             // golden :5273
        {
            iStartIn = 0;                                                       // golden :5275
            ShowMyMessage("Please Enter LotID and Operator ID!!");              // golden :5276
            return false;                                                       // golden :5277
        }
        else if (fLotInfo->edtJobSeq->Text == "")                               // golden :5279
        {
            iStartIn = 0;                                                       // golden :5281
            ShowMyMessage("Please Enter Job Sequence!");                        // golden :5282
            return false;                                                       // golden :5283
        }
        else if (RunInfo.bLotStart == false)                                    // golden :5285
        {
            iStartIn = 0;                                                       // golden :5287
            ShowMyMessage("Please Press Lot Start!!");                          // golden :5288
            return false;                                                       // golden :5289
        }
        else if (IniConfig.bVTESTFunction == true)                              // golden :5291  RogerYang 20250604
        {
            if (fLotInfo->CheckNoRetestBinFlag() == false)                      // golden :5293
            {
                iStartIn = 0;                                                   // golden :5295
                return false;                                                   // golden :5296
            }
        }
    }
    else if (CUSTOMER_CODE == CC_LEADYO)                                        // golden :5300  KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    {
        if (fLotInfo->edtSysLotID->Text == "")                                  // golden :5302
        {
            iStartIn = 0;                                                       // golden :5304
            ShowMyMessage("Please Enter LotID!!");                              // golden :5305
            return false;                                                       // golden :5306
        }
        else if (RunInfo.bLotStart == false)                                    // golden :5308
        {
            iStartIn = 0;                                                       // golden :5310
            ShowMyMessage("Please press Lot Start!!");                          // golden :5311
            return false;                                                       // golden :5312
        }
    }
    else if (CUSTOMER_CODE == CC_TSMC_TAINAN)                                   // golden :5315  wei 20160923 Tray Feed 後下一次開始需要 Clean Count
    {
        bInitialCleanCount = CheckAndReadIniDataGeneral("System", "bInitialCleanCount", 0); // golden :5317
        if (bInitialCleanCount)                                                 // golden :5318
        {
            Clarn_Data(1, "TSMC_InitialCleanCount");                            // golden :5320
        }
        bInitialCleanCount = false;                                             // golden :5322
        WriteIniDataGeneral("System", "bInitialCleanCount", 0);                 // golden :5323
    }

    if (CUSTOMER_CODE == CC_JCET &&                                             // golden :5326
        TestIF_File.bSpiroxTesterLotEnd == true)                                // golden :5327  JerryYang 20170706 (Steven)
    {
        if (bNeedClearSortCount)                                                // golden :5329
        {
            bNeedClearSortCount = false;                                        // golden :5331
            Clarn_Data(1, "JCET_NeedClearSortCount");                           // golden :5332
        }
    }

    LogSoftwareOnTime("HT9045, Start(5)");                                      // golden :5336

    if (CosFunction.bRTC_ROICount == true)                                      // golden :5338  jou 20171201 (Steven) : RTC ROI 確認數量
    {
        if (REAL_TIME_CCD == true && !COM2->bCCDDummyRum)                       // golden :5340
        {
            bRealCCDROICountCheck = true;                                       // golden :5342
        }
    }

    // =========================================================================
    //  ST-W4  golden main.cpp:5346-5646（301 行）
    //  AI(W906-ST-W4) 20260915
    // =========================================================================

    // SAFETY-GATE(W906-ST-W4-A) 🔴 golden :5346-5356 `ATCAmbientTemperCheck()`
    //   ⚠ **擋啟動**。golden：常溫／常溫熱模式 + ATC 主動冷卻時，檢查 ATC 環溫設定
    //   是否在 25~30 範圍內，不在就 WAR15310 並擋住。
    //   閘掉的後果：開了 ATC 主動冷卻的機台，環溫設定超出 25~30 也照樣啟動。
    //
    //   ⚠⚠ **20260915 更正：原本寫「該函式在移植樹不存在」，那是錯的。**
    //   實測（ST-W7-D 查訪）：
    //     ✅ 純計算**已經存在** —— `ComputeATCAmbientTemperCheck(double, int, int)`
    //        （MainCalcCore.h:318，忠實保留 ==0 sentinel 與 -5 的二段判斷）
    //     ✅ `IniConfig.dATCAmbientTemperature`、`ATC_SYSTEM` 都拿得到
    //     ❌ **缺的是第三個參數 `iATC_MODE_TYPE` 的來源**
    //   `ATC/ATC_Handler_Side.h` 在移植樹不存在（實測 include 失敗）；
    //   唯一的替身是 `acarry_shims.h:112` 的 `TATC_InterfaceFormShim::iATC_MODE_TYPE`，
    //   而它**只在 ctor 設成 0，全樹再無任何寫入**（`acarry_shims.cpp:72`）。
    //
    //   為什麼**不**拿那個 shim 來湊：
    //     `iATC_MODE_TYPE` 恆為 0 ⇒ `(mode==33||35||61)` 恆假 ⇒ 永遠走 else ⇒
    //     環溫一超出 25~30 就一律擋住。對「新 ATC 系統 + mode 33/35/61 且環溫在
    //     -5~25 之間」的機台，**golden 會放行而我們會擋** —— 那是替 golden 選路。
    //   ⚠ 這跟 W1-E（ST-W7-C）的部分解閘**不同級**，差別要講清楚：
    //     W1-E 是**忠實算出判定、只省略輸出**（三個對話框）。
    //     這裡若照做，是**用捏造的輸入去算判定** —— 省略輸出 ≠ 捏造輸入。
    //
    //   歸屬（陷阱 #6 的第三類）：這個狀態**不在 web、也不在 C++** ——
    //   它是 ATC 控制器回報的模式，整條 ATC 介面尚未移植。
    //   所以它不屬 §5.2 的 fShow 家族，要等 ATC 介面移植，是獨立的一大塊。
#if 0
    if ((LastSet.iTemperature == Tempture_Ambient ||                            // golden :5346
         LastSet.iTemperature == Tempture_AmbientHot) &&                        // golden :5347  kevin 20181011 : add 恆溫控制
        Temperature.bATCActiveCooling)                                          // golden :5348  wei 20151013
    {
        if (ATCAmbientTemperCheck() == false)                                   // golden :5350
        {
            iStartIn = 0;                                                       // golden :5352
            ShowErrorMessage("WAR15310", 0, MMATC_Handler, false, "Main--Start"); // golden :5353
            return false;                                                       // golden :5354
        }
    }
#endif

    if (LastSet.iRealDummy == DUMMY)                                            // golden :5358  Ifor 20160503 避免 No Device/No Tray 模式下 loader 內有 tray
    {
        if (Sen[SnLoaderSureTray].IsOn())                                       // golden :5360  Jimmychiu 20220414 delete SnLoaderPreDete
        {
            iStartIn = 0;                                                       // golden :5362
            ShowErrorMessage("MES0922", 0, MMTrayY, false, "Main--Start");      // golden :5363
            iInArmWaitPosition = 0;                                             // golden :5364  Ifor 20210209 add:旗標未清除導致 Tray Arm 不作動
            return false;                                                       // golden :5365
        }
    }

    // SAFETY-GATE(W906-ST-W4-B) golden :5369-5387 `NETDownloadDataCheck()` / `fARMS` / `pnARMSTitle`
    //   ⚠ **擋啟動**（ASE-M 的 ARMS 功能：伺服器下載失敗時要驗密碼才能本機跑）。
    //   三個相依都不在：NETDownloadDataCheck（golden main.cpp:30425，368 行，
    //   計畫書 §2 列為「先閘」）、fARMS 這個介面物件、fLotInfo->pnARMSTitle 這個 widget。
    //   閘掉的後果：CC_ASE_M 且開了 N13 ARMS 的機台，伺服器資料沒下載成功也不會
    //   要求密碼，直接放行；而且畫面不會顯示 RUN LOCAL / RUN SERVER。
#if 0
    if (CUSTOMER_CODE == CC_ASE_M)                                              // golden :5369  Ifor 20170621 (wei) add ARMS Function
    {
        if (IniConfig.bN13_EnableARMSFunction)                                  // golden :5371
        {
            if (NETDownloadDataCheck() == false)                                // golden :5373
            {
                if (fARMS->CheckPassword(aARMSPassWordPath) == false)           // golden :5375
                {
                    iStartIn = 0;                                               // golden :5377
                    return false;                                               // golden :5378
                }
                fLotInfo->pnARMSTitle->Caption = "RUN LOCAL";                   // golden :5380
            }
            else
            {
                fLotInfo->pnARMSTitle->Caption = "RUN SERVER";                  // golden :5384
            }
        }
    }
#endif

    if (CosFunction.bRTCHalfViewAutoVerify &&                                   // golden :5389
        IniConfig.bD36EnableRTCAutoModelVerify &&                               // golden :5390
        iContactMode == CONTACT_NORMAL &&                                       // golden :5391
        REAL_TIME_CCD == true &&                                                // golden :5392
        !COM2->bCCDDummyRum &&                                                  // golden :5393
        bNeedWaitContactTestAutoVerify)                                         // golden :5394
    {
        iStartIn = 0;                                                           // golden :5396
        ShowMyMessage("Must to do the auto verification after Calibration RTC model"); // golden :5397
        return false;                                                           // golden :5398
    }

    if (CUSTOMER_CODE == CC_JCET ||                                             // golden :5401
        CUSTOMER_CODE == CC_Murata)                                             // golden :5402  Steven 20200910 : add Murata
    {
#ifndef SOFT_SIMULTE                                                            // golden :5404
        if (LastSet.iTester == OFF_LINE)                                        // golden :5405  Frank 20170221 (wei) add for JCET
        {
            // SAFETY-GATE(W906-ST-W4-F) golden :5407-5419 `ShowMyMessageBox_YES_NO`
            //   ⚠ **擋啟動**（JCET / Murata：OFF_LINE 跑要先確認）。
            //   該函式是已知未移植的 W7-UI modal（Automation/AGV_E84.h:58 等多處記載）。
            //   ⚠ cinitial.cpp 有個回固定 "NO" 的替代品 W8N2_ShowMyMessageBox_YES_NO
            //     （forms/fLotInfo.h:1346 提到），**不採用** —— 那會讓這個確認框
            //     永遠答 NO，等於 JCET/Murata 永遠不能 OFF_LINE 啟動。那是改行為。
            //   ★ **只閘內層**：外面的 `else { bflag = false; }`（golden :5423）
            //     是 ON_LINE 時的狀態寫入，保留。
            //   閘掉的後果：JCET / Murata 機台 OFF_LINE 啟動時不再跳確認框，直接放行。
#if 0
            if (bflag == false)                                                 // golden :5407
            {
                ret = ShowMyMessageBox_YES_NO("Sure To OffLine Running", "確定要OffLine測試？"); // golden :5409
                if (ret == 2)                                                   // golden :5410
                {
                    iStartIn = 0;                                               // golden :5412
                    return false;                                               // golden :5413
                }
                else
                {
                    bflag = true;                                               // golden :5417
                }
            }
#endif
        }
        else
        {
            bflag = false;                                                      // golden :5423
        }
#endif                                                                          // golden :5425

        if (IniConfig.bA23CheckLotNoInSLTReport &&                              // golden :5427  JerryYang 20170706 重新啟用 A23
            iContactMode == CONTACT_NORMAL &&                                   // golden :5428
            fLotInfo->edtSysLotID->Text == "")                                  // golden :5429
        {
            iStartIn = 0;                                                       // golden :5431
            ShowMyMessage("請輸入Lot No");                                       // golden :5432
            return false;                                                       // golden :5433
        }
    }
    else
    {
        bflag = false;                                                          // golden :5438
    }

    if (CosFunction.bAutoRetestGPIBmode == true)                                // golden :5441  jou 2015-10-02 Auto Retest GPIB mode
    {
        if (LastSet.iRunStartMode == rsmInitial_ART ||                          // golden :5443
            LastSet.iRunStartMode == rsmContinuStart_ART ||                     // golden :5444
            LastSet.iRunStartMode == rsmContinuRetest_ART)                      // golden :5445
        {
            bFlag = false;                                                      // golden :5447
            if (CosFunction.bUseSCKART)                                         // golden :5448  Steven 20161214 (wei) : For SCK ART
            {
                if (LastSet.iTester == OFF_LINE)                                // golden :5450
                    iMode = OffT;                                               // golden :5451
                else
                    iMode = FT;                                                 // golden :5453
            }
            else
            {
                iMode = FT_ART;                                                 // golden :5457
            }

            for (int i = eAuto1; i <= iAutoRight; i++)                          // golden :5460
            {
                if (BinSelect[iMode].bAutoRetest[i] == true)                    // golden :5462
                {
                    for (int j = 0; j < iTestBinCount; j++)                     // golden :5464
                    {
                        if (BinSelect[iMode].iCatDataT3Pos[j] == (i + 1))       // golden :5466
                        {
                            bFlag = true;                                       // golden :5468
                        }
                    }
                }
            }

            if (bFlag == false)                                                 // golden :5474
            {
                iStartIn = 0;                                                   // golden :5476
                ShowMyMessage("Need setting Retest on Combine bin in ART_Normal bin page"); // golden :5477
                return false;                                                   // golden :5478
            }

            // golden :5481-5484 是四行被作者註解掉的 hanaART 呼叫，照原樣不還原。
        }
    }
    // SAFETY-GATE(W906-ST-W4-C) golden :5487-5499 `CheckARTSetupFile()`
    //   ⚠ **擋啟動**（KYEC 的 ART 模式：沒開 A10 或工作檔不支援就不讓跑）。
    //   golden 那個函式在移植樹不存在。
    //   ⚠ 這是 `else if`，接在上面 `if (CosFunction.bAutoRetestGPIBmode)` 之後。
    //     整支閘掉不會影響前一支的行為（前一支有自己的 return），
    //     但會讓 CC_KYEC_LEE 走 ART 模式時完全不做這道檢查。
    //   閘掉的後果：CC_KYEC_LEE 在 ART 起始／續跑／續複測模式下，
    //   即使 A10 沒開或工作檔不支援 ART，也照樣啟動。
    //   ✅ AI(W906-T7) 20260923 **已解**（使用者 20260923 17:4x 裁決「b 排在 T6 之後」）。
    //     上面「golden 那個函式在移植樹不存在」是過期的 absence claim：純判定早以 ComputeCheckARTSetupFile
    //     落地（MainCalcCore.cpp:364，MainCalcCore.h:22 自己寫 RESOLVED）。本檔在 StartFromWeb 前加了
    //     檔案層的 CheckARTSetupFile() 把 golden 讀的五個值原樣餵進去（值的來源逐一量過，見那支函式的註解）。
    //     極性與副作用照 golden 對過：golden 的 CheckARTSetupFile 本身無副作用、回 true＝可跑 ART；
    //     副作用（iStartIn=0、ShowMyMessage、return false）全在這個呼叫端，照原文保留（W6-I 栽在「替代品極性相反、
    //     少了副作用」—— 這裡兩者都沒有）。
    else if (CUSTOMER_CODE == CC_KYEC_LEE &&                                    // golden :5487
             (LastSet.iRunStartMode == rsmInitial_ART ||                        // golden :5488
              LastSet.iRunStartMode == rsmContinuStart_ART ||                   // golden :5489
              LastSet.iRunStartMode == rsmContinuRetest_ART))                   // golden :5490
    {
        if (IniConfig.bA10_AutoReTest == false ||                               // golden :5492  wei 20160504 沒開 ART 無法動作
            CheckARTSetupFile() == false)                                       // golden :5493  wei 20160115 確認 Setup File 是否可跑 ART
        {
            iStartIn = 0;                                                       // golden :5495
            ShowMyMessage("No Run ART Mode. Please Check Setup File or Open ART Mode.!!"); // golden :5496
            return false;                                                       // golden :5497
        }
    }

    if (IniConfig.bSPILFunction && TestIF_File.iSCKART_SortMode == 0)           // golden :5501  JerryYang 20220923 : ART bin setting 保護
    {
        if (LastSet.iRunStartMode == rsmInitial_ART ||                          // golden :5503
            LastSet.iRunStartMode == rsmContinuStart_ART ||                     // golden :5504
            LastSet.iRunStartMode == rsmAutoRetest)                             // golden :5505
        {
            for (int i = 0; i < iTestBinCount; i++)                             // golden :5507
            {
                int Pos = Prod.iT6PosCate[i];                                   // golden :5509
                if (Pos > ePosNoUse)                                            // golden :5510
                {
                    if (Pos > ePosAuto3 || Prod.bART6Tray[Pos - 1] == false)    // golden :5512  FT 不需 Retest 的 BIN，在 RT 必須是相同 TRAY
                    {
                        // ⚠ golden 這個條件 `Pos != Prod.iT6PosCate[i]` **恆為 false**
                        //   —— Pos 就是在 :5509 從 Prod.iT6PosCate[i] 取出來的，
                        //   中間沒有任何東西改過它。所以整個錯誤分支是死碼。
                        //   照翻並註明；這是 golden 的缺陷，改它要使用者決定。
                        if (Pos != Prod.iT6PosCate[i])                          // golden :5514
                        {
                            iStartIn = 0;                                       // golden :5516
                            bBigMyMessage = true;                               // golden :5517
                            ShowMyMessage("ART Bin settung檢查異常!!\r\n良品與不可重測Bin在FT與RT必須設定在相同Unload軌道上.\r\n請重新執行EAP2S下載分類機Recipe , 如還是發生相同異常請通知PE確認Bin別設定", // golden :5518
                                          "ART Bin setting check error!!\r\nPass Bin and No-Retest Bin must set to the same unloader on FT & RT Bin setting.\r\nPlease use EAP2S re-download the handler recipe.\r\nIf the same error happen again please call PE check the handler bin setting."); // golden :5519
                            return false;                                       // golden :5520
                        }
                    }
                }
            }
        }
    }

    // AI(W906-R28TORQ) 20260925: SAFETY-GATE(W906-ST-W4-D) 解閘 —— COM2（TCOM2Shim）現在有 iWriteAndCheckMotorTorqueDelay
    //   （golden rs232.h:174，扭力子系統在 rs232.cpp；使用者 20260925 裁決第 6 條）。On() 讓扭力寫入狀態機的計時從按 START
    //   這一刻重新起算（golden 註解「避免暫停時 Torque 會 Time Out」）。不影響 Start 的判斷結果。
    //   原閘理由：「COM2 沒有這個成員…缺了它，暫停時可能出現 Torque Time Out。」
    COM2->iWriteAndCheckMotorTorqueDelay.On();                                  // golden :5528  Steven 20130701

    if (IniConfig.bHighModeCanOffTemp &&                                        // golden :5530
        Temperature.iMachineTempMode == 0 &&                                    // golden :5531
        LastSet.iTemperature == Tempture_Ambient)                               // golden :5532  Steven 20110524
    {
        iStartIn = 0;                                                           // golden :5534
        ShowErrorMessage("WAR15190", 0, MMSystem, false, "Main--Start");        // golden :5535
        return false;                                                           // golden :5536
    }

    LogSoftwareOnTime("HT9045, Start(6)");                                      // golden :5539

    // SAFETY-GATE(W906-ST-W4-G) golden :5541-5548 `CCDInterfaceForm->iIdentificationStatus`
    //   ⚠ **擋啟動**。TCCDInterfaceFormShim 沒有 iIdentificationStatus
    //   （CCD TCP/IP 的辨識狀態，2 = Fail）。
    //   閘掉的後果：開了 bEnableCCDUSETCPIP 的機台，CCD 辨識失敗也照樣啟動。
#if 0
    if (IniConfig.bEnableCCDUSETCPIP)                                           // golden :5541  Eliot 2010_1209
    {
        if (CCDInterfaceForm->iIdentificationStatus == 2)                       // golden :5543  2:Fail
        {
            iStartIn = 0;                                                       // golden :5545
            return false;                                                       // golden :5546
        }
    }
#endif

    // AI(W906-T5-W4E) 20260919: 這裡本來是 SAFETY-GATE(W906-ST-W4-E)，
    //   閘掉的理由是「缺的是 golden main.cpp:30425 那 368 行」。
    //   那 368 行已於 20260919 翻完（見本檔 TfMainWeb::NETDownloadDataCheck()），
    //   所以閘的前提消失，呼叫點解開。
    //
    //   ⓘ 注意它仍然包在 `#ifndef SOFT_SIMULTE` 裡（golden 本來就這樣寫）：
    //   模擬組態下不會被呼叫，只有出貨組態會走到這條比對。
    if (CosFunction.bUseFTPDownloadDataCheck == true &&                         // golden :5550  Ifor 20180125
        bEnablePEModel == false)                                                // golden :5551  Ifor 20160824 PE 工程模式下不比對
    {
        #ifndef SOFT_SIMULTE
        if (LastSet.iTester == ON_LINE)                                         // golden :5554
        {
            if (NETDownloadDataCheck() == false)                                // golden :5556  jou 2015-01-21 KYEC 說 Off-line FTP 不比對
            {
                iStartIn = 0;                                                   // golden :5558
                return false;                                                   // golden :5559
            }
        }
        #endif
    }

    if (USE_IN_OUT_ARM_Y_PITCH == iXPitchManual635)                             // golden :5565  jou 2012-05-15 : 選擇 Y Pitch 機構模式
    {
        if (TestIF.iARM_Y_PITCH == 6000)                                        // golden :5567
        {
            if (InArmZSafe(DETECT_SENSOR_FLAG) == -1 && Sen[SenInArmYPitch60].IsOff())   // golden :5569
            {
                iStartIn = 0;                                                   // golden :5571
                ShowMyMessage("Please Adjust In Arm Y pitch set to 60mm", "請調整 In Arm Y pitch 機構至 60mm 的位置。"); // golden :5572
                return false;                                                   // golden :5573
            }

            if (OutArmZSafe(DETECT_SENSOR_FLAG) == -1 && Sen[SenOutArmYPitch60].IsOff()) // golden :5576
            {
                iStartIn = 0;                                                   // golden :5578
                ShowMyMessage("Please Adjust Out Arm Y pitch set to 60mm", "請調整 Out Arm Y pitch 機構至 60mm 的位置。"); // golden :5579
                return false;                                                   // golden :5580
            }
        }
        else
        {
            if (InArmZSafe(DETECT_SENSOR_FLAG) == -1 && Sen[SenInArmYPitch60].IsOn())    // golden :5585
            {
                iStartIn = 0;                                                   // golden :5587
                ShowMyMessage("Please Adjust In Arm Y pitch set to 63.5mm", "請調整 In Arm Y pitch 機構至 63.5mm 的位置。"); // golden :5588
                return false;                                                   // golden :5589
            }

            if (OutArmZSafe(DETECT_SENSOR_FLAG) == -1 && Sen[SenOutArmYPitch60].IsOn())  // golden :5592
            {
                iStartIn = 0;                                                   // golden :5594
                ShowMyMessage("Please Adjust Out Arm Y pitch set to 63.5mm", "請調整 Out Arm Y pitch 機構至 63.5mm 的位置。"); // golden :5595
                return false;                                                   // golden :5596
            }
        }
    }

    // SAFETY-GATE(W906-ST-W4-H) golden :5601-5611 `ShowMyMessageBox_YES_NO`
    //   ⚠ **擋啟動**（magazine 空 tray 是否補滿的確認）。
    //   同 W4-F，缺的是未移植的 W7-UI modal。
    //   ⚠ 這一段**整段閘**：它唯一的內容就是那個確認框，
    //     沒有像 W4-F 那樣有另一半的狀態寫入要保留。
    //   閘掉的後果：裝 magazine 的機台在起始／起始複測模式下，
    //   不再詢問空 tray 是否補滿，直接放行。
#if 0
    if (AUTO3_IS_MAGAZINE == 1 && TrayForm.iMagTraySource == 1 &&               // golden :5601  JerryYang 20220909 : add magazine
        (LastSet.iRunStartMode == rsmInitialStart ||                            // golden :5602
         LastSet.iRunStartMode == rsmCInitialRetest))                           // golden :5603
    {
        ret = ShowMyMessageBox_YES_NO("Do you fill up the magazine empty tray?", "是否已將Magazine空tray補滿?"); // golden :5605
        if (ret == 2)                                                           // golden :5606
        {
            iStartIn = 0;                                                       // golden :5608
            return false;                                                       // golden :5609
        }
    }
#endif

    if (AUTO3_IS_MAGAZINE == 1)                                                 // golden :5613
    {
        if (Tech.iMagazineTray1Pos == 0 ||                                      // golden :5615
            Tech.iCatchMazTray_Front == 0 ||                                    // golden :5616
            Tech.iCatchMazTray_Rear == 0)                                       // golden :5617
        {
            iStartIn = 0;                                                       // golden :5619
            ShowMyMessage("Magazine Teach position can not be 0.");             // golden :5620
            return false;                                                       // golden :5621
        }
    }

    if (IniConfig.bD18_AutoHeightWhenChangeRecipe)                              // golden :5625  Steven 20221013 : 更換工作檔時提示有沒有做 Auto Height
    {
        if (sRecipe != cbSetupFileName->Text)                                   // golden :5627
        {
            iStartIn = 0;                                                       // golden :5629
            sRecipe = cbSetupFileName->Text;                                    // golden :5630
            ShowMyMessage("Did you check Auto Height?", "Please check contact height after change recipe."); // golden :5631
            return false;                                                       // golden :5632
        }
    }

    if (CUSTOMER_CODE == CC_SIGURD_HUKOU &&                                     // golden :5636
        fHeaterOK == false &&                                                   // golden :5637
        W906_FShow("fContact", fContact->fShow) == true &&                                              // golden :5638  JerryYang 20151230
        LastSet.iTemperature == Tempture_Hot &&                                 // golden :5639
        iContactMode != CONTACT_NORMAL)                                         // golden :5640
    {
        iStartIn = 0;                                                           // golden :5642
        ShowMyMessage("Can not auto height during heating", "加熱尚未完成，不能Auto Height。"); // golden :5643
        return false;                                                           // golden :5644
    }

    // =========================================================================
    //  ST-W5  golden main.cpp:5647-5958（312 行）
    //  AI(W906-ST-W5) 20260915
    //
    //  ⚠ 這一段開始有**真正的機台動作**（不只是檢查）：
    //     :5885-5890  ATC 上線 / 冷機開關 / SetRunATC
    //     :5900       ATC 7.0 送 ATC_RUN
    //     :5908       關閉 GPIB 程式
    //     :5918       省電模式計時器重啟
    //     :5877       Handler 停機時間 latch
    //     :5938       ScanFixTrayStatus
    //  它們仍然只在 StartFromWeb() 被呼叫時才跑，而目前沒有人呼叫它。
    // =========================================================================

    if (CosFunction.bRecipeCheck &&                                             // golden :5647  JerryYang 20151201 Amkor：工作檔不同要先做 Height Calibration
        bHeightCalibrationFinish == false &&                                    // golden :5648
        iContactMode == CONTACT_NORMAL)                                         // golden :5649
    {
        iStartIn = 0;                                                           // golden :5651
        ShowMyMessage("Please do height calibration first", "請先做height calibration"); // golden :5652
        return false;                                                           // golden :5653
    }

    if (IniConfig.bA68_AutoLoadUnload)                                          // golden :5656  JerryYang 20250521 : For AMR
    {
        if (Sen[SnLoaderTrayHasTray_AGV].Enable == false)                       // golden :5658
        {
            iStartIn = 0;                                                       // golden :5660
            ShowMyMessage("Please keyin the sensor SnLoaderTrayHasTray_AGV in database!", "請於database輸入SnLoaderTrayHasTray_AGV!"); // golden :5661
            return false;                                                       // golden :5662
        }

        for (int i = 0; i < iAutoCnt; i++)                                      // golden :5665  JerryYang 20251020 : add
        {
            if (Sen[SnAutoTrayHasTray[i]].Enable == false)                      // golden :5667
            {
                iStartIn = 0;                                                   // golden :5669
                Str.sprintf("Please keyin the sensor SnAuto%dTrayHasTray in database!", i + 1); // golden :5670
                ShowMyMessage(Str);                                             // golden :5671
                return false;                                                   // golden :5672
            }
        }
    }

    // AI(W906-ST-W7-A) 20260915: W5-A 已還債 —— CheckSLKSensor() 翻好了（見本檔上方），
    //   所以這個 🔴 gate 解除。golden :5677-5681 原文恢復。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    if (CheckSLKSensor() == false)                                              // golden :5677
    {
        iStartIn = 0;                                                           // golden :5679
        return false;                                                           // golden :5680
    }

    if (IniConfig.bSPILFunction == true &&                                      // golden :5683  JerryYang 20250115 : SPIL 耀仁要求卡控
        LastSet.iTester == ON_LINE &&                                           // golden :5684
        LastSet.iRealDummy == REALLY &&                                         // golden :5685
        (iContactMode != CONTACT_AUTO_GET_HEIGHT &&                             // golden :5686
         iContactMode != CONTACT_MANUAL_GET_HEIGHT))                            // golden :5687  JerryYang 20250826 : fix 保護
    {
        if (IniConfig.bD30EnableSiteModeSelect &&                               // golden :5689
            TestIF_File.iShuttleMode == 1)                                      // golden :5690
        {
            if (TestIF_File.iShuttle_Sel == 0)                                  // golden :5692  Front Arm Only
            {
                if (DeviceForm_File.IndexContact[0] == -50)                     // golden :5694
                {
                    // ⚠ golden 這三支**都沒有** iStartIn=0 就直接 return false。
                    //   跟同檔其他 early-return 不一致（那些都先歸零），
                    //   結果是下一次呼叫會被 :4394 的 `if(iStartIn!=0) return false;`
                    //   直接擋掉。看起來是缺陷，但照翻並註明。
                    ShowMyMessage("Contact height is -50mm, please do auto height calibration!"); // golden :5696
                    return false;                                               // golden :5697
                }
            }
            else if (TestIF_File.iShuttle_Sel == 1)                             // golden :5700  Rear Arm Only
            {
                if (DeviceForm_File.IndexContact[1] == -50)                     // golden :5702
                {
                    ShowMyMessage("Contact height is -50mm, please do auto height calibration!"); // golden :5704
                    return false;                                               // golden :5705
                }
            }
        }
        else
        {
            if (DeviceForm_File.IndexContact[0] == -50 || DeviceForm_File.IndexContact[1] == -50) // golden :5711
            {
                ShowMyMessage("Contact height is -50mm, please do auto height calibration!"); // golden :5713
                return false;                                                   // golden :5714
            }
        }
    }

    if (IniConfig.bNewResetFunction && bResetIsPressed)                         // golden :5719  Steven 20130625 : 新的 Reset 方式
    {
        bResetShuttle = false;                                                  // golden :5721
        bResetInArm = false;                                                    // golden :5722
        bResetInArmTask = false;                                                // golden :5723
        bResetOutArm = false;                                                   // golden :5724
        bResetOutArmTask = false;                                               // golden :5725
        bResetSortArm = false;                                                  // golden :5726  RogerYang 20250515 Add for 9046AU
        bResetSortArmTask = false;                                              // golden :5727
        bResetIndexArm = false;                                                 // golden :5728
        bResetIndexArm1Pick = false;                                            // golden :5729
        bResetIndexArm2Pick = false;                                            // golden :5730
        bResetLoadTray = false;                                                 // golden :5731
        bResetUnLoadTray[0] = false;                                            // golden :5732
        bResetUnLoadTray[1] = false;                                            // golden :5733
        bResetUnLoadTray[2] = false;                                            // golden :5734

        // SAFETY-GATE(W906-ST-W5-B) golden :5736-5776 `fMain->cbRunStartMode`
        //   forms/fMain.h 沒有 cbRunStartMode 這個 widget（Run Start Mode 下拉選單）。
        //   ⚠ 這一段是「新 Reset 方式」依**目前選的啟動模式**決定要重設哪些軸：
        //     起始／起始複測 -> 清 fAllMotorHome
        //     續跑／續複測   -> 把 Shuttle/InArm/OutArm/SortArm/IndexArm/Tray
        //                      的 reset 旗標打開，並把 InArm 上的 HAS_HOT_IC
        //                      改成 HAS_NULL_IC（標記需要 destroy）
        //   ⚠ **只能整段閘**：兩支分岔要讀同一個 widget 才知道走哪邊，
        //     少了它就無從判斷，挑一邊等於替 golden 選路。
        //   ⚠⚠ **這是 🔴（擋啟動側），不是「只是旗標沒設」** ——
        //   20260915 對抗式稽核指出、我自己對 golden 複驗屬實，原本的分類是錯的。
        //   追下去：
        //     golden :5739  fAllMotorHome = false;      ← 被這個 gate 吞掉的那一行
        //     golden :6123  if (fAllMotorHome == false) ← 全 Start() 裡唯一的讀取點
        //     golden :6164  Home("Home by Start");
        //     golden :6165  iStartIn = 0;
        //     golden :6166  return false;
        //   （實測：fAllMotorHome 在 main.cpp:4385-6259 內只出現這兩次，一寫一讀。）
        //   所以 :5739 正是逼出 :6166 那條 return false 的輸入。閘掉之後，
        //   **Initial Start / Initial Retest 在 fAllMotorHome 已經是 true 時，
        //   不會走「先回原點、return false」那條路，而是一路往下真的開跑** ——
        //   方向是 fail-open，不是少做事。
        //   :6123 落在 ST-W6 範圍，所以今天還是潛伏的；但分類現在就要標 🔴，
        //   否則 ST-W6 落地時沒有人會回頭看這裡。
        //
        //   閘掉的其餘後果：開了 bNewResetFunction 的機台，按過 Reset 之後再 Start，
        //   所有 reset 旗標維持在上面那批 false（:5721-5734 照常執行），
        //   也就是「續跑模式應該要重設的那些軸不會被重設」。
        //   ★ 更正：原本這裡還寫了「bResetIsPressed 不會被放掉」，**那是錯的** ——
        //     golden :5958 的 `bResetIsPressed = false;` 是無條件的，而且已經翻進去了。
        //   ★ ST-W7 要優先補 cbRunStartMode 這個 widget。
        //   ✅ AI(W906-T7) 20260923 **已解**（使用者 20260923 17:4x 裁決「b 排在 T6 之後」）。
        //     上面「forms/fMain.h 沒有 cbRunStartMode」是過期的 absence claim：W906-P10（20260921）已補
        //     （forms/fMain.h:306 宣告、forms/fMain.cpp:103 `new TfLotInfoRunMode()`＝vclcompat::TComboBox）。
        //     值從哪來（「符號存在」有三級，這裡三級都量過）：寫者是 RunStartMode.cpp 的 SetRunStartMode()
        //     （`fMain->cbRunStartMode->Text=StartModeName[Mode];`，照 golden；那支已取代舊的空樁 —— nm 看得到
        //     `T SetRunStartMode(eRunStartMode, AnsiString)`）。模式一變就寫。
        //     ⚠ 唯一缺口：golden 開機時 FormShow→SetStartModeData→SetRunStartMode 會先把它種好，wb_serve 開機沒走那條
        //       （T5 已記錄）。所以在第一次 SetRunStartMode 之前 Text 是空字串 —— 兩個分岔都不成立，
        //       等於跟閘著時一樣；之後就照 golden。解閘不會比閘著更差。列晨報：補開機的 SetStartModeData。

        if (fMain->cbRunStartMode->Text == StartModeName[rsmInitialStart] ||    // golden :5736
            fMain->cbRunStartMode->Text == StartModeName[rsmCInitialRetest])    // golden :5737
        {
            fAllMotorHome = false;                                              // golden :5739
            bResetIsPressed = false;                                            // golden :5740
        }
        else if (fMain->cbRunStartMode->Text == StartModeName[rsmContinuStart] || // golden :5742
                 fMain->cbRunStartMode->Text == StartModeName[rsmContinuRetest])  // golden :5743
        {
            bResetIsPressed = false;                                            // golden :5745

            bResetShuttle = true;                                               // golden :5747
            bResetInArm = true;                                                 // golden :5748
            bResetInArmTask = false;                                            // golden :5749
            bResetOutArm = true;                                                // golden :5750
            bResetOutArmTask = false;                                           // golden :5751
            bResetSortArm = true;                                               // golden :5752  RogerYang 20250515
            bResetSortArmTask = false;                                          // golden :5753
            bResetIndexArm = true;                                              // golden :5754
            bResetIndexArm1Pick = true;                                         // golden :5755
            bResetIndexArm2Pick = true;                                         // golden :5756
            bResetLoadTray = true;                                              // golden :5757
            bResetUnLoadTray[0] = true;                                         // golden :5758
            bResetUnLoadTray[1] = true;                                         // golden :5759
            bResetUnLoadTray[2] = true;                                         // golden :5760

            if (InArmZSafe(DETECT_ALL_FLAG) == -1)                              // golden :5762
            {
                for (int i = 0; i < InArmSuck.iMaxRow; i++)                     // golden :5764
                {
                    for (int j = 0; j < InArmSuck.iMaxCol; j++)                 // golden :5766
                    {
                        if (InArmSuck.Item[i][j] == HAS_HOT_IC)                 // golden :5768
                        {
                            InArmSuck.SetItemData(i, j, HAS_NULL_IC);           // golden :5770
                            bInArmCheckDestroyACT[i][j] = true;                 // golden :5771  jou 981130 確認 device 確實 destroy 完成
                        }
                    }
                }
            }
        }
        // （原 `#endif  // SAFETY-GATE(W906-ST-W5-B) 結束` —— AI(W906-T7) 20260923 已解，上面 #if 0 一併移除）
        bLampReset = false;                                                     // golden :5777
        iReset = 0;                                                             // golden :5778
    }

    LogSoftwareOnTime("HT9045, Start(7)");                                      // golden :5781

    if (bCarRecordTimeStart == true)                                            // golden :5783  2013-12-09 wei
    {
        CarRecord.Set0_1SecAndOn(IniConfig.iCarRecordDelayTime * 600);          // golden :5785
        bCarRecordTimeStart = false;                                            // golden :5786
        bCarRecordTimeEnd = true;                                               // golden :5787
    }

    if (CosFunction.bTempLess30degShowLight ||                                  // golden :5790  jou 2014-12-19
        CosFunction.bTempHeaterOkShowLight)                                     // golden :5791  jou 20180529
    {
        bTempLess30degShowLightFlag = false;                                    // golden :5793
        bTempHeaterOkShowLightFlag = false;                                     // golden :5794
    }

    // SAFETY-GATE(W906-ST-W5-F) golden :5802 / :5929 `TfShuttleMove::fShow`
    //   TfShuttleMove facade 沒有 fShow。它在 golden 裡的角色是
    //   「Shuttle Move 畫面開著的時候放行」的例外條件。
    //   ⚠ 兩處都**只能整段閘**，因為 fShow 是 && 條件鏈的一環，
    //     拿掉它等於把那個例外變成無條件成立或無條件不成立 —— 兩者都是改行為。
    //   閘掉的後果：
    //     :5797-5813  CC_ASE_KaohSiung 的 OP 權限 OFF_LINE 卡關**不執行**
    //                 （本來 OP 在 OFF_LINE 且非 AutoOneCycle 時不准 Start）
    //     :5929-5939  Auto 流道有 tray 盤的檢查、以及 ASE 的 ScanFixTrayStatus
    //                 **都不執行** —— 流道上有盤也照樣啟動
    // AI(W906-ST-W7-W5F) 20260923: **已解**（兩處都解）。使用者裁決「a 解閘」，依 §0.5。
    //   20260917 自評的兩個不解理由 ——「解開會多一道擋啟動」與「要還原被閘掉才變裸 if
    //   的 else-if 鏈」—— 是後果與工作量，不是相依不存在；§0.5 只認後者
    //   （計畫書 §5.5 的「§0.5 裁決題」）。上面「facade 沒有 fShow」已過期。
    //   相依逐項重量過（20260923）：fContact->fShow（forms/fContact.h:1482）、
    //   fTemp_Set->fShow（forms/fTemp_Set.h:505）、Zteach->fShow（aHotPlateSubstrate.h:1206
    //   的 facade，W6-D 已在用）、fShuttleMove->fShow（forms/fShuttleMove.h:134）。
    //   四個都照 P6-b 慣例包 W906_FShow（行程內的 bool ∪ 瀏覽器說的）。
    //   下一支 golden :5814 的 `else if` 一併還原。
    //   ⓘ 只有 CUSTOMER_CODE==CC_ASE_KaohSiung 走得到。MES1052 是 kcode==0 的通知，
    //     經 AI(W906-Q30-KZERO) 不進 wb_serve 的等待迴圈，不會卡住 tick。
    if (CUSTOMER_CODE == CC_ASE_KaohSiung)                                      // golden :5797  kevin 20180522
    {
        if (W906_FShow("fContact", fContact->fShow) == false &&                 // golden :5799
            W906_FShow("fTemp_Set", fTemp_Set->fShow) == false &&               // golden :5800
            W906_FShow("Zteach", Zteach->fShow) == false &&                     // golden :5801
            W906_FShow("fShuttleMove", fShuttleMove->fShow) == false)           // golden :5802  KEVIN 20210803 fShuttleMove 不卡權限
        {
            if (AccessLevel == 0 &&                                             // golden :5804
                LastSet.iTester == OFF_LINE &&                                  // golden :5805
                bIsAutoOneCycle == false)                                       // golden :5806  kevin 20180926 AutoClean OP mode error
            {
                iStartIn = 0;                                                   // golden :5808
                ShowErrorMessage("MES1052", 0, MMSystem, false, "Main--Start"); // golden :5809
                return false;                                                   // golden :5810
            }
        }
    }
    else if (CUSTOMER_CODE == CC_GIGAS &&                                       // golden :5814
             IniConfig.bC08_1_CheckSocketSensorDetectON == true)                // golden :5815  Isaac 20201130
    {
        if (TestIF_File.bEnSocketSensor == false)                               // golden :5817
        {
            // ⚠ golden 用的是 `#ifdef SOFT_SIMULTE`（不是 ifndef）。
            //   本 build 不定義 SOFT_SIMULTE，所以這一段在出貨組態下**不會編進去** ——
            //   也就是 CC_GIGAS 的 Socket Sensor 檢查在真機上根本沒作用。
            //   那是 golden 的樣子，照翻。
            #ifdef SOFT_SIMULTE                                                 // golden :5819
            iStartIn = 0;                                                       // golden :5820
            ShowMyMessage("SocketSensor Detect Function is OFF. Please turn ON!"); // golden :5821
            return false;                                                       // golden :5822
            #endif                                                              // golden :5823
        }
    }

    if (IniConfig.bL46_AStreamErrorCompressOnecycle == true &&                  // golden :5827
        iAStreamErrorCompressOnecycle == 2)                                     // golden :5828  Ztex 2024.10.01
    {
        iStartIn = 0;                                                           // golden :5830
        ShowMyMessage("Please wait for the compressor reset", "請等待壓縮機重製"); // golden :5831
        return false;                                                           // golden :5832
    }

    #ifndef SOFT_SIMULTE                                                        // golden :5835
    if (bEject)                                                                 // golden :5836  JerryYang 20251020 : 渠梁半清機功能
    {
        for (int i = iFixMin; i <= iFixMax; i++)                                // golden :5838
        {
            iFix = iAutoIndex[i];                                               // golden :5840
            if (i == eFix1 && MachineTypeChoice == Type_HT9046_LS &&            // golden :5841  kevin 20220915 Rotate 有使用 Fix 1 就不能用
                USE_ROTATE_KIT == 1 && tRotate.ActiveRotate == 1)               // golden :5842  JerryYang 20191029
            {
                // golden :5843-5844 是空的，照翻
            }
            else
            {
                bFlag = Sen[SnFixedTrayDetect[iFix]].IsOff();                   // golden :5847
                if (bFlag == false)                                             // golden :5848
                {
                    ShowMyMessage("Please remove the FIX tray!", "請取下FIX區的TRAY盤!"); // golden :5850
                    iStartIn = 0;                                               // golden :5851
                    return false;                                               // golden :5852
                }
            }
        }
    }
    #endif                                                                      // golden :5857

    if (HasICUnderMachine() == false)                                           // golden :5859  Steven 20200811 : 加上 Site Map 與 Bin Assign 的 log
    {
        RecordProcess(WriteSiteMapData(false));                                 // golden :5861
        RecordProcess(WriteBinMap(false));                                      // golden :5862
    }

    // SAFETY-GATE(W906-ST-W5-H) golden :5865-5869 `fMesSystem->CheckLotInfor()`
    //   ⚠ **擋啟動**（jou 20200409：VTest MES system 的批次資訊檢查）。
    //   forms/fMesSystem.h:667 把它標成 **GATE (L-2)** —— golden :1037-1638
    //   共 602 行，刻意未翻譯。
    //
    //   ★ 這個 gate 的成因跟本波其他幾個不同，值得記：
    //     它**不是**編譯期就看得到的。-fsyntax-only 一路綠（宣告在標頭裡），
    //     是 ld 才報 undefined reference。而且連鎖反應：
    //       WebStart.cpp 引用 TfMesSystem::CheckLotInfor()
    //         -> 連結器去 libht9045_forms.a 抽出 fMesSystem.cpp.obj
    //           -> **暴露出那支 TU 自己的未解符號 `bNoRTBinFixFlag`**
    //   ⚠⚠ bNoRTBinFixFlag 在**連結層沒有定義**。
    //     也就是 fMesSystem.cpp 一直帶著一個壞掉的相依，只是從來沒有任何 target
    //     連結它，所以沒人發現。下一個把它連進去的人會再撞一次。
    //
    //   ⚠ AI(W906-T5-W5H) 20260919 補充 —— **不要只用 git grep 判這一條**。
    //     源碼裡看起來「有定義」：
    //         cmydef.cpp:6150   bool bNoRTBinFixFlag[3];
    //     20260919 我看到那一行，判定「這個註解過期了，阻塞溶解了」。**判錯了。**
    //     那一行在 cmydef.cpp 的 TODO(W6) `#if 0` 區塊**裡面**，被編掉。
    //     用預處理器判死活（不是自己寫區塊掃描器 —— 簡易掃描器會被巢狀
    //     `#ifdef` 的 `#endif` 騙走，我同一天也踩過）：
    //         g++ -E cmydef.cpp  ->  bNoRTBinFixFlag 只出現 1 次，
    //                                而且是標頭來的 `extern bool bNoRTBinFixFlag[3];`
    //         nm --defined-only cmydef.cpp.obj  ->  0 個
    //         nm --defined-only libht9045_*.a / wb_serve.exe  ->  0 個
    //     ⇒ 阻塞**仍然成立**。
    //
    //   ⓘ 解法有先例，而且很小：cmydef.cpp 那個 TODO(W6) 閘的慣例是
    //     **一次解一個定義**，每次在 `#endif` 後面註明「誰綁它」與安全理由
    //     （既有例子：bTRAYCHKNG、bNeedReportBundleID、sBinCode_ATK）。
    //     `bool bNoRTBinFixFlag[3]` 是 plain data、沒有會碰 NULL 全域的 ctor，
    //     與那幾個同一類（PT_CAMPAIGN_PLAN.md section 8）。
    //     ⛔ 但**不要提前解**：樹的規則是「有東西綁它才解」，
    //     而在 CheckLotInfor 翻好之前沒有東西綁它。兩件事要一起做。
    //
    //   閘掉的後果：開了 VTest MES 的機台，批次資訊檢查不過也照樣啟動。
    // AI(W906-T5-W5H) 20260919: 這裡本來是 SAFETY-GATE(W906-ST-W5-H)，
    //   閘的理由是 golden :1037-1638 那 602 行沒有翻。本波翻完了
    //   （forms/fMesSystem.cpp 的 TfMesSystem::CheckLotInfor），所以前提消失。
    //
    //   ⓘ 順帶把該閘註解裡兩件已經不成立/會誤導的事更正掉：
    //     * bNoRTBinFixFlag 的連結阻塞已解（cmydef.cpp 依該檔慣例解閘一個定義，
    //       見 T5/W4-E 那顆 commit）。
    //     * 「gated SOLELY on TestSocket.iShtRow/.iShtCol -- same ht9045_sm
    //       boundary」那句是對的，而且 forms/fMesSystem.h:297 自己就寫著
    //       "a future ht9045_sm-reaching wave should look here first" ——
    //       wb_serve 同時連結 ht9045_forms 與 ht9045_sm，本波就是那個 wave。
    //
    //   ⚠ 這條檢查**會擋啟動**：開了 VTest MES 的機台，批次資訊比對不過就不讓開。
    //     那正是它存在的目的。
    if (fMesSystem->CheckLotInfor() == false)                                   // golden :5865  jou 20200409 : VTest Mes system
    {
        iStartIn = 0;                                                           // golden :5867
        return false;                                                           // golden :5868
    }

    // ★ golden 在這裡**讀** SystemStart 當守門（已在跑就不再 Start）。
    //   這是整個 Start() 裡唯一碰 SystemStart 的地方，而且是唯讀。
    //   真正把它設成 true 的是 golden ckernel.cpp:517（移植樹 ckernel.cpp:1015），不在本函式內。
    if (SystemStart)                                                            // golden :5871
    {
        iStartIn = 0;                                                           // golden :5873
        return false;                                                           // golden :5874
    }

    lHandlerStopTime.LatchCycleTime(true);                                      // golden :5877  jou 2014-09-21 Show Handler Stop Time

    // AI(W906-ST-W5-G-note) 20260916：補上一條這個 gate 一直沒寫、而下一個人一定會問的事。
    //
    //   被閘的區塊裡有 `bRunATC = true;`（golden :5891）。本樹對「狀態寫入」的既有慣例
    //   是**盡量留在閘外**（見 W1-K 的 AccessLevel、W1-L 的 SW[SwCCDLight].Off()、
    //   W5-C 的 bReOpenGpib）。這裡**刻意不留**，理由是方向：
    //
    //     csystem.cpp:19041  bRunATC=false;  **LIVE**（G25a 的 banner :19050 明寫保持 ACTIVE）
    //     csystem.cpp:21439  bRunATC=false;  GATED（G06 整塊閘掉）
    //     csystem.cpp:9768   bRunATC=true;   **GATED**
    //   ⇒ 全樹**沒有任何活的 `bRunATC = true`**。而它會直接送給 SECS host：
    //     SECSGEM/uHGemHT9045_SV.cpp:491  SVID 1044（BOOLEAN_TYPE）-> &bRunATC
    //     SECSGEM/uHGemHT9045_SV.cpp:492  SVID 1045（ASCII_TYPE）  -> &bRunATC
    //
    //   把 `=true` 留在閘外，等於在**從來沒有叫 ATC 跑**的情況下對 host 宣稱它在跑。
    //   那跟 W1-G（CheckEmployeeID 會送出空白憑證）是同一類錯：
    //   **不是少做一件事，是做錯一件對外的事。** 所以整塊一起閘，不要「順手把狀態寫入救回來」。
    //
    //   ⚠ 上面那三行的死活，20260916 用**前處理器**確認過，不是靠掃描器：
    //       g++ -std=c++17 -E <同 build 的 -D/-I> csystem.cpp
    //       -> `bRunATC=true`  在預處理輸出裡出現 **0** 次
    //          `bRunATC=false` 出現 **1** 次（就是 :19041 那個）
    //     這是權威答案：**全樹沒有任何會被編譯的 `bRunATC = true`**。
    //
    //   ⚠⚠ 記一下我怎麼繞遠路的，因為這條記憶已經寫過了：
    //     「用編譯器判死活，不要自己寫掃描器」。我今晚**違反了兩次**才繞回去 ——
    //     第一版逐行數深度，栽在 `/* ... #ifdef ... #endif*/` 這種**被註解掉的
    //     `#if`/`#endif` 對**（開頭的 `/*` 讓 `#if` 不被算，結尾的 `#endif*/`
    //     卻被算成減 1），深度變負之後真正的 `#if 0` 只回到 0，於是被閘的碼判成 LIVE。
    //     第二版「改良成剝掉區塊註解再數」，結果 flag 卡住，把 11,932/31,970 行
    //     誤判成註解內，連 `:17612` 那個**活的** `#ifdef SOFT_SIMULTE` 都沒數到，
    //     兩版對 2,103 行的判定相反。
    //     **更精密的工具不等於更正確的工具。** 最後是 `g++ -E` 與樹自己的 GATE banner
    //     互相印證才定案。
    // SAFETY-GATE(W906-ST-W5-G) golden :5879-5901 ATC 介面
    //   ⚠ **這一段是真正的機台動作**，不是檢查：
    //     :5886  ATCInterfaceForm->OnLine()          ATC 上線
    //     :5888  ATCChillerSwitch(true)              開冷機
    //     :5890  SetRunATC(true)                     讓 ATC 開始跑
    //     :5900  SendCommToATC7(ATC_RUN, ...)        ATC 7.0 送 RUN
    //   閘掉的**不是**擋啟動的檢查，而是「Start 時該把 ATC 帶起來」這件事。
    //   後果：裝鴻勁型 ATC 且開主動冷卻的機台，按 Start 之後 ATC 不會自動上線／
    //   開冷機／開始運轉 —— golden 的註解寫明它存在的理由是
    //   「Alarm 之後會暫停 ATC，所以要重新 Start」。
    //   ★ 成因是**標頭衝突不是缺符號**（見檔頭那段），所以 ST-W7 要解的是
    //     衝突本身，不是去翻一個函式。
    //
    //   ⚠⚠ 這個 gate 裡有**一行不是因為缺符號而被閘的**，必須寫明，否則日後解閘
    //     的人會誤判：`golden :5891  bRunATC = true;`。
    //     bRunATC 在移植樹是**活的**（cmydef.h:4091 extern / cmydef.cpp:4263 定義），
    //     本檔已經 include cmydef.h，外層三個條件也都不碰 ATC 標頭 ——
    //     技術上它可以留在外面。
    //     **刻意一起閘掉**，理由：bRunATC 有真正的消費者
    //     （SECSGEM/uHGemHT9045_SV.cpp:238 把 SVID 1045 直接註冊到 &bRunATC，
    //       另有 PowerSavingMode.cpp / csystem.cpp 數處）。
    //     在 ATC 根本沒被叫起來（:5886/:5888/:5890 全被閘）的情況下把它設成 true，
    //     等於**對外謊報機台狀態**，比少做事糟。
    //     這是本檔「保留狀態寫入」慣例（W1-K/L、W4-F、W5-C）的**唯一例外**。
#if 0
    if (ATC_SYSTEM == eATCHonPrecType &&                                        // golden :5879  Steven 20120725 : Alarm 後會暫停 ATC，要重新 Start
        (TestIF_File.iTestMode == DualSite ||                                   // golden :5880
         TestIF_File.iTestMode == SingleSite))                                  // golden :5881
    {
        if (Temperature.bATCActiveCooling == true)                              // golden :5883
        {
            if (ATCInterfaceForm->IsOnLine() == false)                          // golden :5885
                ATCInterfaceForm->OnLine();                                     // golden :5886
            if (ATCInterfaceForm->ATC_SYS.IsChillerRun() == false)              // golden :5887
                ATCInterfaceForm->ATCChillerSwitch(true);                       // golden :5888
            if (ATCInterfaceForm->ATC_SYS_PAL[0]->bATCRunSetting == false)      // golden :5889
                ATCInterfaceForm->SetRunATC(true);                              // golden :5890
            bRunATC = true;                                                     // golden :5891  ChungHung 20160118 add for Hisi V102
        }
    }

    if (ATC_SYSTEM == eATCHonPrecType &&                                        // golden :5895
        Temperature.bATC70Active == true &&                                     // golden :5896  Steven 20151111 : For ATC 7.0
        (LastSet.iTemperature == Tempture_Hot ||                                // golden :5897
         LastSet.iTemperature == Tempture_AmbientHot))                          // golden :5898  kevin 20181011 恆溫
    {
        ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");                      // golden :5900
    }
#endif  // SAFETY-GATE(W906-ST-W5-G) 結束

    if (bReOpenGpib == true &&                                                  // golden :5903
        IniConfig.bI07ResetGPIBAfterOneCycleCleanOut == true)                   // golden :5904  Steven 20101013
    {
        // SAFETY-GATE(W906-ST-W5-C) golden :5906-5909 `fMain->CloseGpibProgram()`
        //   forms/fMain.h 沒有這個方法。
        //   golden 的用意是「每次 OneCycle 或 CleanOut 後要重開 GPIB」——
        //   先關掉，之後別處會再開。
        //   閘掉的後果：開了 I07 的機台，OneCycle/CleanOut 後 GPIB 不會被關，
        //   也就不會重開；原本要修的「GPIB 斷線」問題會留著。
        //   ★ 下面的 bReOpenGpib = false（:5910）**保留** —— 那是狀態寫入，
        //     閘掉會讓這個旗標永遠掛著。
#if 0
        if (IndexHasIC() == false)                                              // golden :5906  Steven 20110609
        {
            fMain->CloseGpibProgram(__FUNC__);                                  // golden :5908
        }
#endif
        bReOpenGpib = false;                                                    // golden :5910
    }
    else
    {
        bReOpenGpib = false;                                                    // golden :5914
    }

    // SAFETY-GATE(W906-ST-W5-D) golden :5917-5918 `tPSM`
    //   省電模式的計時器物件全樹不存在。不擋啟動。
    //   閘掉的後果：開了 bPowerSaveFunction 的機台，按 Start 不會重啟省電倒數
    //   （可能在運轉中進入省電）。
#if 0
    if (IniConfig.bPowerSaveFunction)                                           // golden :5917  Dell 20110418
        tPSM.Restart();                                                         // golden :5918
#endif

    // golden :5920-5926 是七行被作者註解掉的 Galil 速度設定，照原樣不還原。

    // SAFETY-GATE(W906-ST-W5-E) golden :5927 `bStartKeyPressCheck`
    //   這個旗標全樹不存在。golden 的註解寫「for 安全門未關按 Start 時
    //   IndexArm 會先動作」，看起來像安全相關。
    //   ★ **但它是 🟡 不是 🔴** —— 20260915 對抗式稽核指出、我自己複驗屬實。
    //     原本我因為那句「安全門」字樣就標成 🔴，那是被**歷史成因的註解**帶偏，
    //     沒有去看它實際做什麼。
    //   實際的機制在 golden csystem.cpp:4657-4672：讀到它為 true 時做一次性的
    //   Galil VS/SP 速度補寫。**沒有 return、沒有 StopAllMotor、沒有動 SystemStart。**
    //   ⚠ 而且**移植樹裡那個唯一的讀取點本來就已經被閘了** ——
    //     csystem.cpp:18960 `#if 0 // GATE G22`，該處 banner 自己寫明
    //     「the one-shot Galil VS/SP speed re-issue ... is not performed」。
    //   所以這個 gate 今天的行為 delta 是 **0**。  ⚠ AI(W906-INDEXZ-1203) 20260930：已過期 —— 見下方 #if 1 那一行（G22 解閘後 delta 不再是 0）。
    //   標 🔴 會把它排進 ST-W7 的優先序，擠掉真正 fail-open 的 W5-A / W5-F。
#if 1   //AI(W906-INDEXZ-1203) 20260930: review round 2 A -- SAFETY-GATE(W906-ST-W5-E) LIFTED to golden. Both premises above are gone: the member exists (forms/fMain.h, NSDMI false = golden ctor main.cpp:1692) and its one reader, csystem.cpp GATE G22 (golden :4657-4672), is lifted in the same change. DELTA from today: after this START, DoSystem's next pass with SystemStart still true (i.e. after ScanSystemSensor's safe-door check and CountMotorPowerDelay) sends golden's ONE "VS<v>;SP<s>,<s>,<s>,<s>;" to MOT[MTestY1] -- on HT9050 with the Index Z1 route that string resumes a Z1 move halted by VS0;SP0 (the route's only resume; its old SystemStart rule resumed before the door check); with no route / no Galil card it is logged (QueueGalilCmd) and answered 0 (golden no-card branch)
    bStartKeyPressCheck = true;                                                 // golden :5927  Steven 20110309
#endif

    // SAFETY-GATE(W906-ST-W5-F) 第二處。理由同上。
    // AI(W906-ST-W7-W5F) 20260923: **已解**，同第一處（使用者裁決「a 解閘」）。
    //   ⓘ CheckAutoHasTray(false)（csystem.cpp:12646）沒有對話框：只有安全門沒關時回 true
    //     （靜默擋啟動，golden 同），另外依 Auto 流道 sensor 開關側推／壓 tray 氣缸。
    //   ⓘ ScanFixTrayStatus()（csystem.cpp:16498）只有 ASE 高雄會叫；內有 K_RETRY 告警，
    //     會等操作員回答，與本函式其他 K_RETRY 告警同一套機制。
    if (W906_FShow("fShuttleMove", fShuttleMove->fShow) == false)              // golden :5929  KEVIN 20210803 / kevin 20130416 使用空 TRAY
    {
        if (CheckAutoHasTray(false) == true)                                    // golden :5931  jou 2010-01-25 : 確認 Auto 流道上是否有 tray 盤
        {
            iStartIn = 0;                                                       // golden :5933
            return false;                                                       // golden :5934  Steven 20250423 : Door not close
        }

        if (CUSTOMER_CODE == CC_ASE_KaohSiung)                                  // golden :5937
            ScanFixTrayStatus();                                                // golden :5938  kevin 20171026 (wei)
    }

    if (LastSet.iRealDummy != DUMMY)                                            // golden :5941
    {
        if (CheckOutArmZ(false) == false && MTrayXCanSafeMove())                // golden :5943  Out Arm Z 在上面，且 Tray Arm 在上面
        {
            if (UserDefForm[0].ZDepth < 1500 &&                                 // golden :5945  Steven 20200723 : 太厚的 Tray 不能檢查置偏
                Sen[SnUnLoaderFloating].Enable &&                               // golden :5946
                Sen[SnUnLoaderFloating].IsOn())                                 // golden :5947  Steven 20110725 : Unloader 置偏偵測
            {
                iStartIn = 0;                                                   // golden :5949
                ShowErrorMessage("JAM1109", K_RETRY, MMAuto1, true, "Main--Start"); // golden :5950
                return false;                                                   // golden :5951  JerryYang 20230204
            }
        }
    }

    bNeedCheckIndexToque = true;                                                // golden :5956  jou 2012-06-12 吸 shuttle 時需檢測 Torque
    bResetIsPressed = false;                                                    // golden :5958  Steven 20130625

    // =========================================================================
    //  ST-W6  golden main.cpp:5959-6259（301 行）—— **最後一波翻譯**
    //  AI(W906-ST-W6) 20260915
    //
    //  ⚠⚠⚠ 這一波之後，這個函式就有**成功路徑**了（golden :6258 return true）。
    //     而且它裡面有 golden :6196 / :6207 / :6230 的 `SoftStart = true;` ——
    //     **那才是真正的啟動觸發**：kernel tick 看到 SoftStart==true 就會跑
    //     ckernel.cpp 的「啟動檢查」區塊，然後 golden ckernel.cpp:517（移植樹是 :1015）設 SystemStart=true。
    //
    //     也就是說：**從這一波起，唯一擋在機台動作之前的東西，就是「沒有人呼叫
    //     StartFromWeb()」這件事本身。** 掛上 wb_serve 分派（S3）之前，
    //     這個函式仍然是死碼；掛上之後就是真的。
    //     S3 要使用者站在機台旁邊，而且要先補完 §5.1 的 🔴。
    // =========================================================================

    if (CUSTOMER_CODE == CC_ASE_KaohSiung)                                      // golden :5959
    {
        // SAFETY-GATE(W906-ST-W6-A) golden :5961-5973 `TfShuttleMove::fShow` / `fTeach`
        //   同 W5-F：TfShuttleMove facade 沒有 fShow，另外 fTeach 也不在。
        //   ⚠ **擋啟動**：OP 權限在 OFF_LINE 時不准 Start（MES1052）。
        //   閘掉的後果：ASE 高雄的機台，OP 在 OFF_LINE 也能按 START。
        //   ⚠ 只能整段閘 —— fShow 是 && 條件鏈的一環，補預設值就是替 golden 選路。
#if 0
        if (fContact->fShow == false &&                                         // golden :5961
            fTemp_Set->fShow == false &&                                        // golden :5962
            Zteach->fShow == false &&                                           // golden :5963  KenHsieh 20211111 : AOA teach 不卡權限
            fShuttleMove->fShow == false &&                                     // golden :5964  KEVIN 20210803
            fTeach->fShow == false)                                             // golden :5965
        {
            if (AccessLevel == 0 && LastSet.iTester == OFF_LINE)                // golden :5967  kevin 20140411 必須是 OFF_LINE
            {
                iStartIn = 0;                                                   // golden :5969
                ShowErrorMessage("MES1052", 0, MMSystem, false, "Main--Start"); // golden :5970
                return false;                                                   // golden :5971
            }
        }
#endif

        // SAFETY-GATE(W906-ST-W6-I) golden :5975-5983 `CheckSiteMapState()`
        //   ⚠ **擋啟動**（AutoSiteMap 模式下不能在主畫面關 site）。
        //   golden 的無參數多載在 main.cpp:31148-31172，移植樹沒有。
        //   ⚠ 移植樹**有一個部分替代品** `ComputeCheckSiteMapState()`
        //     （MainCalcCore.h:384，說明在 :350-383），但**不能直接拿來用**：
        //       (a) 它的回傳極性與 golden **相反**（回的是 raw bHasErr，
        //           golden 是 bHasErr 時回 false）
        //       (b) golden 那支還會發 MyDBIProcess("System", "AutoSiteMap Do not close Site")
        //           與 ShowErrorMessage("WAR07400", ...)，那兩個 MainCalcCore.h:375-378
        //           自己寫明 "OUT OF SCOPE here and belong in a not-yet-translated wrapper"。
        //     自己補那個 wrapper 等於在波次中途寫新行為，不是翻譯。列入 ST-W7。
        //   閘掉的後果：ASE 高雄的機台跑 AutoSiteMap 模式時，
        //   即使在主畫面關掉了 site 也照樣啟動。
#if 0
        if (IniConfig.bUseAutoSiteMapping &&                                    // golden :5975
            LastSet.iRunStartMode == rsmAutoSiteMap)                            // golden :5976  kevin 20150115
        {
            if (CheckSiteMapState())                                            // golden :5978  AutoSiteMap 不能在主畫面關 site
            {
                iStartIn = 0;                                                   // golden :5980
                return false;                                                   // golden :5981
            }
        }
#endif
    }

    fBinSel->cbUseMRTMode->Enabled = false;                                     // golden :5986  Ifor 20170414 (wei) Start 後不可修改 MRT 模式

    SendCommand_ESD(ESD_SYSTEM_START);                                          // golden :5988  Steven 20140722

    LogSoftwareOnTime("HT9045, Start(8)");                                      // golden :5990

    // SAFETY-GATE(W906-ST-W6-B) golden :5992-5995 `fBarCode->spbStartComClick()`
    //   ⚠ 20260917 重量：舊註解寫「ST-W7 補（只有 9 行）」，**那是低估**。不擋啟動。
    //   golden fAOI.cpp:3884 那 8 行踩的是**通訊／硬體路徑**：Sender 跨型別轉成
    //   TSpeedButton* 讀 ->Tag、TopAOIComm/AOIComm->StopComm()、再呼叫 RS232Init()。見計畫書 §5.1。
#if 0
    if (TestIF_File.bEnableBarCode && InputShuttleHasIC() == false)             // golden :5992  Steven 20151218 : Add for 2d code
    {
        fBarCode->spbStartComClick(this);                                       // golden :5994
    }
#endif
    // SAFETY-GATE(W906-ST-W6-L) golden :5996-5997 `fNote->t2DCode`
    //   TfNote facade 沒有 t2DCode 這個 widget（2D code 的顯示格陣）。
    //   純顯示 —— 它把入料吸嘴的列/行數寫進那個格陣的尺寸。不擋啟動、不影響判斷。
    //   閘掉的後果：Note 畫面的 2D code 格陣尺寸不會跟著吸嘴配置更新。
#if 0
    fNote->t2DCode->XItem = InArmSuck.iShtRow;                                  // golden :5996
    fNote->t2DCode->YItem = InArmSuck.iShtCol;                                  // golden :5997
#endif

    if (CUSTOMER_CODE == CC_KYEC_LEE &&                                         // golden :5999
        ((CosFunction.bUseMRTMode && bCleanSkipICCount) ||                      // golden :6000
         (USE_AUTO_RETEST == eartInstall && IniConfig.bA10_AutoReTest && bCleanSkipICCount) || // golden :6001
         (CosFunction.bUseARTSortCount == true && bCleanSkipICCount)))          // golden :6002  Frank 20160819 / Ifor 20170413 (wei)
    {
        for (int i = 1; i < 21; i++)                                            // golden :6004
        {
            fShowBinSelect->StrARTSkipICCount->Cells[1][i] = "";                // golden :6006
        }
        bCleanSkipICCount = false;                                              // golden :6008
    }

    if (TestIF_File.i2DIDFormat == eAMD)                                        // golden :6011  JerryYang 20200422
    {
        HHandler2Gpib.iStatus[0] = eAMD;                                        // golden :6013
    }
    else if (TestIF_File.i2DIDFormat == eIntel)                                 // golden :6015
    {
        HHandler2Gpib.iStatus[0] = eIntel;                                      // golden :6017
    }
    else
    {
        HHandler2Gpib.iStatus[0] = eStandard;                                   // golden :6021
    }
    fMain->SendMSG_CMD(MSG_CMD_2DIDFormat);                                     // golden :6023

    SocketAirCoolingClear();                                                    // golden :6025  jou 2016-04-28
    // golden :6026 是一行被作者註解掉的 RespondASECom，照原樣不還原。
    iLogLightScaleCount_InArmX1 = 0;                                            // golden :6027  KaiChen 20171228：Log Light Scale Data
    iLogLightScaleCount_InArmX2 = 0;                                            // golden :6028
    iLogLightScaleCount_InArmY1 = 0;                                            // golden :6029
    iLogLightScaleCount_InArmY2 = 0;                                            // golden :6030
    iLogLightScaleCount_OutArmX1 = 0;                                           // golden :6031
    iLogLightScaleCount_OutArmX2 = 0;                                           // golden :6032
    iLogLightScaleCount_OutArmY1 = 0;                                           // golden :6033
    iLogLightScaleCount_OutArmY2 = 0;                                           // golden :6034

    if (CosFunction.bEnableHandlerResultServer == true &&                       // golden :6036  Isaac 20170613 (wei) TeraPower TCP Command
        bHandlerResultConnect == false)                                         // golden :6037
    {
        HanderTcpIp();                                                          // golden :6039
        bHandlerResultConnect = true;                                           // golden :6040
    }
    // SAFETY-GATE(W906-ST-W6-J) golden :6042-6046 `TCPCommandServer` / `TeraTCPResultServer` -- LIFTED by W10 (AI(W906-W10) 20260927 (St02-E))
    //   AI(W906-W10) 20260927 (St02-E)：兩個伺服器物件存在了（forms/fMain.h:1009），else 這一支照 golden 跑（:3482／:3488 改成註解）。以下三行是解閘前的說明：
    //   ⚠ 只閘 else 這一支 —— 上面那支（HanderTcpIp() + bHandlerResultConnect=true）
    //     的相依都在，保留。兩支互斥，閘掉 else 不會讓 if 那支在不該跑時跑。
    //   閘掉的後果：關掉 TeraPower TCP 功能時，那兩個伺服器不會被停用
    //   （本來就沒被啟用過，所以今天的行為 delta 約等於 0，但語意上少了一次收尾）。
//#if 0   AI(W906-W10) 20260927 (St02-E): lifted
    else if (CosFunction.bEnableHandlerResultServer == false)                   // golden :6042
    {
        TCPCommandServer->Active = false;                                       // golden :6044
        TeraTCPResultServer->Active = false;                                    // golden :6045
    }
//#endif

    if (IniConfig.bP53_ForcedScanBinCodeOfUnloader &&                           // golden :6048
        LastSet.iTester == ON_LINE &&                                           // golden :6049
        iContactMode == CONTACT_NORMAL &&                                       // golden :6050
        IniConfig.bA65_BundleIDList == false)                                   // golden :6051  JerryYang 20240111 : add P53 function
    {
        bHasAlm = false;                                                        // golden :6053
        for (int i = 0; i < 3; i++)                                             // golden :6054  MES1124, MES1224, MES1324
        {
            if (fMain->IsStackHasLess16Bin(i) && sStackBinTemp[i] == "")        // golden :6056  JerryYang 20250429
            {
                str.sprintf("MES%d24", i + 11);                                 // golden :6058
                ShowErrorMessage(str, K_RETRY, MMAuto1_Car + i, false, "Process_START"); // golden :6059
                bHasAlm = true;                                                 // golden :6060
            }
        }

        for (int i = 0; i < 3; i++)                                             // golden :6064
        {
            if (i == 0 && MachineTypeChoice == Type_HT9046_LS &&                // golden :6066  kevin 20220915
                USE_ROTATE_KIT == 1 && tRotate.ActiveRotate == 1)               // golden :6067  JerryYang 20191029
            {
                // golden :6068-6069 是空的，照翻
            }
            else
            {
                if (fMain->IsStackHasLess16Bin(i + 3) && sStackBinTemp[i + 3] == "") // golden :6072
                {
                    if (Sen[SnFixedTray1Detect + i].IsOff() &&                  // golden :6074
                        LastSet.iRealDummy != DUMMY)                            // golden :6075  JerryYang 20241118
                    {
                        iStartIn = 0;                                           // golden :6077
                        str.sprintf("MES%d21", 17 + i);                         // golden :6078  MES1721, MES1821, MES1921
                        ShowErrorMessage(str, K_RETRY, MManualTray1 + i);       // golden :6079
                        return false;                                           // golden :6080
                    }

                    str.sprintf("MES%d24", i + 17);                             // golden :6083  MES1724, MES1824, MES1924
                    ShowErrorMessage(str, K_RETRY, MManualTray1 + i, false, "Process_START"); // golden :6084  Sam 20200630
                    bHasAlm = true;                                             // golden :6085
                }
            }
        }

        if (bHasAlm == true)                                                    // golden :6090
        {
            iStartIn = 0;                                                       // golden :6092
            return false;                                                       // golden :6093
        }
    }

    // SAFETY-GATE(W906-ST-W6-C) golden :6097-6108 `fBarCode->RunCheckBarcodeByServerData()`
    //   ⚠ **擋啟動**（從伺服器下載 2DID 檔失敗就不讓開機台）。
    //   TfBarCode 沒有 RunCheckBarcodeByServerData；
    //   而且內層的 fMesSystem->Get2DIDFromServer 也走 W5-H 那個不能 include 的標頭。
    //   閘掉的後果：開了「從伺服器讀 2DID」的機台，下載失敗也照樣啟動。
#if 0
    if (fBarCode->RunCheckBarcodeByServerData() == true)                        // golden :6097  Jimmychiu 20230925
    {
        if (fMesSystem->Get2DIDFromServer(TestIF_File.asMes2DID_URL) == true)   // golden :6099
        {
        }
        else
        {
            iStartIn = 0;                                                       // golden :6104
            ShowMyMessage("Download 2DID file from server fail!");              // golden :6105
            return false;                                                       // golden :6106
        }
    }
#endif

    // SAFETY-GATE(W906-ST-W6-K) golden :6110-6113 `fMain->machineTime`
    //   forms/fMain.h 沒有 machineTime 這個成員（機台運作狀態紀錄的計時器）。
    //   不擋啟動。
    //   閘掉的後果：CC_PANTHER 的機台，Start 時不會恢復運作時間計時 ——
    //   運轉時數統計會少算。
#if 0
    if (CUSTOMER_CODE == CC_PANTHER)                                            // golden :6110
    {
        fMain->machineTime.Resume();                                            // golden :6112  Jimmychiu 20250916 : 機台運作狀態紀錄
    }
#endif

    // golden :6115-6121 是七行被作者註解掉的 P63 停機計時器，照原樣不還原。

    // ★★ golden :6123 —— 這就是 ST-W5 的 W5-B gate 指向的那個讀取點。
    //    W5-B 閘掉了 golden :5739 的 `fAllMotorHome = false;`，
    //    所以在 Initial Start 且 fAllMotorHome 已經是 true 的情況下，
    //    **這個 if 不成立、下面的 Home + return false 不會執行** —— 沒 Home 就往下跑。
    //    那條路現在具體化了。ST-W7 必須補 cbRunStartMode 把 W5-B 解閘。
    //    ✅ AI(W906-T7) 20260923: W5-B 已解（見上面那格）—— :5739 的 `fAllMotorHome = false;` 回來了，
    //       Initial Start／Initial Retest 會照 golden 先走這裡的 Home + return false。
    //       （前提是 cbRunStartMode->Text 已被 SetRunStartMode 種過，見 W5-B 那格的缺口說明。）
    if (fAllMotorHome == false)                                                 // golden :6123  jou 20211029 : SoftStart=true 保持在最下面
    {
        if (IniConfig.bG06HomeinitialCheckZ1 && bHomeUnlock == false)           // golden :6125  kevin 20131218
        {
            bHomeinitialCheckPushZ1 = true;                                     // golden :6127
        }
        SendCommand_ESD(ESD_SYSTEM_START);                                      // golden :6129  Steven 20140722

        if (IniConfig.bEnable_SECS_GEM == true && IniConfig.bRCMDStart == true) // golden :6131  Steven 20141006 : SECS GEM Remote Start
        {
            if (IniConfig.bSPILFunction == true)                                // golden :6133
            {
                bHomeByStart = true;                                            // golden :6135
            }
            else
            {
                bHomeByStart = false;                                           // golden :6139
            }
        }
        else if (IniConfig.bN29_ParameterCheckForGMTest == true &&              // golden :6142
                 bPhysicalStart == true)                                        // golden :6143  Steven 20220311 : GM Test 工作檔比對
        {
            bHomeByStart = false;                                               // golden :6145
        }
        else if (CUSTOMER_CODE == CC_ChipMos_ZHUBEI &&                          // golden :6147
                 IniConfig.bN25_1_EnableStartControl)                           // golden :6148  Steven 20210413 : 南茂的自動 Start
        {
            bHomeByStart = false;                                               // golden :6150
        }
        else
        {
            // SAFETY-GATE(W906-ST-W6-D) 🟡 golden :6154-6159 `fShuttleMove->fShow` / `Zteach->fShow`
            //   同 W5-F / W6-A。條件鏈少一環就得替 golden 選路，所以整個 if 閘掉。
            //   🟡 的理由：`RunCheckStart()` 在 golden 這裡是**無回傳值的通知**，
            //     它不 return false、也不改本函式後續的控制流；下一行 :6160
            //     的 `bHomeByStart = true;` 照樣執行、:6164 照樣 Home。
            //     （20260915 補：原本只有文字沒有標籤，第三份稽核點名。）
            //   ⚠ 只閘**條件與 RunCheckStart()**，下面的 `bHomeByStart = true;`
            //     （golden :6160）**保留** —— 那是無條件的狀態寫入，
            //     跟 W5-C 保留 `bReOpenGpib = false;` 同一個前例。
            //   閘掉的後果：開了 SECS/GEM 的機台，Home by Start 之前不做 Run Check。
            // AI(W906-ST-W7-RCS) 20260917: **已解**。唯一阻塞是 RunCheckStart()，現在翻好了
            // （forms/fMain.cpp 檔尾）；fShuttleMove->fShow 與 Zteach->fShow 在 ST 批次 1 已備齊。
            if (IniConfig.bEnable_SECS_GEM == true &&                           // golden :6154  Steven 20140528
                W906_FShow("fShuttleMove", fShuttleMove->fShow) == false &&                                 // golden :6155  JerryYang 20160901
                W906_FShow("Zteach", Zteach->fShow) == false)                                         // golden :6156  JerryYang 20170123 (Steven)
            {
                RunCheckStart();                                                // golden :6158  Ifor 20151208
            }
            bHomeByStart = true;                                                // golden :6160
        }

        lHandlerStopTime.LatchCycleTime(true);                                  // golden :6163  jou 2014-09-21
        Home("Home by Start");                                                  // golden :6164  ★ 真正的回原點動作
        iStartIn = 0;                                                           // golden :6165
        return false;                                                           // golden :6166
    }

    // SAFETY-GATE(W906-ST-W6-E) golden :6169-6173 `fShuttleMove->fShow`
    //   同上。不擋啟動（只是少做一次 Run Check）。
    // AI(W906-ST-W7-RCS) 20260917: **已解**。同 W6-D —— 唯一阻塞是 RunCheckStart()，
    //   現在翻好了（forms/fMain.cpp 檔尾）；fShuttleMove->fShow 在 ST 批次 1 已備齊。
    if (IniConfig.bEnable_SECS_GEM == true &&                                   // golden :6169  Steven 20141006
        W906_FShow("fShuttleMove", fShuttleMove->fShow) == false)                                           // golden :6170  JerryYang 20160901
    {
        RunCheckStart();                                                        // golden :6172  Ifor 20151208
    }

    // golden :6175 是一行被作者註解掉的 ChangeStateUploadServer，照原樣不還原。

    // ★★★ 底下這一整串 if/else-if 鏈是**真正的啟動觸發**：
    //     `SoftStart = true;`（:6196 / :6207 / :6230）會讓 kernel tick 進入
    //     ckernel.cpp 的「啟動檢查」區塊，然後 golden ckernel.cpp:517（移植樹是 :1015）設 SystemStart=true。
    //     `Timer6->Enabled = true;` 那幾支是走 SECS/GEM 的 Remote Start 路徑。
    //
    // SAFETY-GATE(W906-ST-W6-F) 🔴 golden :6177-6232 `fShuttleMove->fShow`
    //   ★★ 標 🔴 而不是 🟡，儘管 golden 這一段沒有 `return false`。
    //     §5.1 的 🔴/🟡 判準是「golden 有沒有用它擋啟動」，而這一段是
    //     **唯一會把 SoftStart 設成 true 的地方** —— 閘掉它不是「少一個檢查」，
    //     是「啟動根本不發生」。用 🟡（純顯示）標會嚴重低估。
    //     實測（20260915，第三份稽核）：本檔三個 `SoftStart = true` 全部在這個
    //     #if 0 內；下游鏈路是活的（ckernel.cpp:816 `if(SoftStart==true)`
    //     → :1015 `SystemStart=true`），所以刀口就在這裡。
    //   ⚠⚠ 這一段**五個**分支（if / else-if ×3 / else）裡有**兩個**要讀
    //     fShuttleMove->fShow（:6179 / :6202），第四支還多讀一個
    //     fContact->fShow（:6204）。facade 兩個成員都沒有。
    //     （20260915 更正：原文寫「四個分支裡有三個」，兩個數字都錯；
    //       引的行號 :6179/:6202 本身是對的。第三份稽核抓到。）
    //   ⚠ **只能整段閘**：這是 if / else-if × 3 / else 的五岔，
    //     少一個條件就無法決定走哪一支，而每一支的結果都不同
    //     （Timer6 遠端啟動 / 什麼都不做 / SoftStart=true / 蜂鳴器後才 SoftStart）。
    //     挑任何一支都是替 golden 選路。
    //   ★★ AI(W906-ST-S3-B1) 20260918 **已解閘**；六行語句改 //MARKED。上兩行
    //     「facade 兩個成員都沒有」**過期**（forms/fShuttleMove.h:134、
    //     atester_shims.h:157 都有）；編譯器問出的是 Timer6/ImpParaCheck/
    //     iAutoStartTask/tAutoStart…。⇒ 三個 SoftStart=true 全存活。
#if 1   // AI(W906-ST-S3-B1) 20260918: UN-GATED（還原＝改回 #if 0）。分支結構完整保留。
    if (IniConfig.bEnable_SECS_GEM == true &&                                   // golden :6177
        (IniConfig.bRCMDStart == true || bSECSPause == true) &&                 // golden :6178  JerryYang 20250120
        W906_FShow("fShuttleMove", fShuttleMove->fShow) == false)                                           // golden :6179
    {
        bPhysicalStart = true;                                                  // golden :6181
        //MARKED(W906-ST-S3-B1) 20260918 Timer6->Enabled = true;     // golden :6182  SECS/GEM 遠端啟動
    }
    else if (CUSTOMER_CODE == CC_ASE_CL && bSECSPause)                          // golden :6184  JerryYang 20260414 : ASE-CL 才要 Lock
    {
        // golden :6185-6186 是空的，照翻
    }
    else if (CosFunction.RunCheckWhenRecPause)                                  // golden :6187  JerryYang 20250120
    {
        if (bNeedDoRunCheck == true)                                            // golden :6189
        {
            bPhysicalStart = true;                                              // golden :6191
            //MARKED(W906-ST-S3-B1) 20260918 Timer6->Enabled = true; // golden :6192  SECS/GEM 遠端啟動
        }
        else
        {
            SoftStart = true;                                                   // golden :6196  ★ 啟動觸發
        }
    }
    else if ((CUSTOMER_CODE == CC_ChipMos_ZHUBEI &&                             // golden :6199
              IniConfig.bN25_1_EnableStartControl) ||                           // golden :6200  Steven 20210413
             (IniConfig.bN29_ParameterCheckForGMTest == true &&                 // golden :6201
              W906_FShow("fShuttleMove", fShuttleMove->fShow) == false))                                    // golden :6202  Steven 20220311
    {
        if ((W906_FShow("fContact", fContact->fShow) == true && iContactMode != CONTACT_NORMAL) ||      // golden :6204  Steven 20210519
            (iTrayFeed == 1))                                                   // golden :6205  Jimmychiu 20250327
        {
            SoftStart = true;                                                   // golden :6207  ★ 啟動觸發
        }
        else
        {
            if (HasICUnderMachine() == false)                                   // golden :6211
            {
                //MARKED(W906-ST-S3-B1) 20260918 ImpParaCheck->Clear();  // golden :6213  GM 比對清單，純顯示
            }
            //MARKED(W906-ST-S3-B1) 20260918 iAutoStartTask = 1;     // golden :6215  AutoStart 任務游標
            bPhysicalStart = true;                                              // golden :6216
            //MARKED(W906-ST-S3-B1) 20260918 Timer6->Enabled = true; // golden :6217  SECS/GEM 遠端啟動
            //MARKED(W906-ST-S3-B1) 20260918 tAutoStartTimeOutTimer.SetSecAndOn(TestIF_File.iInitialMaxTime); // golden :6218  AutoStart 逾時
        }
    }
    else
    {
        if (IniConfig.bG14UseStartSoundAlarm)                                   // golden :6223  kevin 20201116 Start 發出聲音不動 5sec
        {
            RunStartLowSpeedBuzzer(true);                                       // golden :6225
            bStartMoveSpeed = true;                                             // golden :6226  kevin 20201116
        }
        else
        {
            SoftStart = true;                                                   // golden :6230  ★ 啟動觸發
        }
    }
#endif

    // ---- W6-G：20260919 **已解閘**（P2b）------------------------------------
    //
    //AI(W906-P2b-CF) 20260919: 這裡原本是 `SAFETY-GATE(W906-ST-W6-G)`，
    // 閘住的理由是 `TransformFuntion()`（golden adam6024.cpp:1038，759 行）沒有翻。
    // 沒翻的理由（RULINGS §C11.2）是那 759 行裡有 **44 個 `fContactForce->…`**，
    // 而那個 VCL 表單刻意未移植。
    //
    // P2a（commit 096984f）把四張力量表搬進 `ContactForceTables()` 並接上
    // wb_serve 的 bring-up，P2b 把 759 行翻進 `adam6024.cpp` ——
    // 44 個引用全部有家了，替換是機械的。⇒ 閘的前提消失，解閘。
    //
    // ⚠ 解閘之後這裡會呼叫 `ADAM_DirectWriteData(iInputValue, 0, 0)`（golden :6252）。
    //   那**是**對外寫 IO（EP 類比輸出＝下壓力道）。三件事要知道：
    //     1. 在這棵樹它是 no-op 樁（`atester_shims.cpp:326`），今天寫不出去。
    //     2. 同一支函式在 `asendic_Loader.cpp:482/486/847/890` 有**四個未閘**的
    //        呼叫點 —— 閘住這一個卻放行那四個本來就不一致（RULINGS §C11.5）。
    //     3. ⛔ 樁換成真本體的那一天，這個呼叫點會跟著活起來。
    //        那一天要重讀本段，並確認 `LoadContactForceTables()` 真的載過表
    //        （空表會讓換算拿到預設值 —— `ContactForce.h:319` 叫它
    //         "the silent-30.0 defect"）。
    //
    // ⚠ `TransformFuntion` 本身**不寫 IO**：實測 ADAM 寫 0 / 讀 0 / 馬達 0 /
    //   氣缸 0 / 檔案 0，只有一個 ShowMyMessage。它是純換算。
    //   RULINGS §C11.5 已更正過相反的敘述。
    // ==> Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 begin                       // golden :6233
    if (INSTALL_DOUBLE_EP == 1 || INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)         // golden :6234
    {
        int iInputValue = 0;                                                    // golden :6236
        double dInPutdefault = 0.0;                                             // golden :6237
        if (CUSTOMER_CODE == CC_KYEC_LEE && bEnable_KLT_Function == false && DeviceForm_File.bUseDieForce == false) // golden :6238
        {
            if (DeviceForm_File.dDieForceKitDiameter <= 2)                      // golden :6240
                dInPutdefault = 8;                                              // golden :6241
            else
                dInPutdefault = (DeviceForm_File.dDieForceKitDiameter - 2) * 15; // golden :6243
            iInputValue = TransformFuntion(dInPutdefault, true);                // golden :6244
        }
        else
        {
            iInputValue = TransformFuntion(DeviceForm_File.DoubleForce, true);  // golden :6248
        }

        if (W906_FShow("fContact", fContact->fShow) == false)                                           // golden :6251
            ADAM_DirectWriteData(iInputValue, 0, 0);                            // golden :6252
    }
    // <== Eastsun 20260525 INSTALL_DOUBLE_EP_3 整合 end                         // golden :6254

    iStartIn = 0;                                                               // golden :6255

    LogSoftwareOnTime("HT9045, Start(End)");                                    // golden :6256

    // AI(W906-ST-W7-G) 20260915: W6-H 已還債 —— SET_ESD_Tri_Temp() 翻好了
    //   （見本檔上方），gate 解除。golden :6257 原文恢復。
    //   ⚠ 這一行會對 ESD 送命令，但同一條通道在本檔已經有 4 個活的呼叫點
    //     （:510 / :512 / :2581 / :2756），不是新開的對外通道。
    //   行為 delta = 0（StartFromWeb 仍然零個呼叫者）。
    //   ⚠ 20260921 作廢：S3 已於 20260918 武裝，見檔頭 §S3。delta 不再是 0。
    SET_ESD_Tri_Temp(LastSet.iTemperature);                                     // golden :6257  Ztex 2023.04.19
    return true;                                                                // golden :6258  ★ golden 的成功路徑
}


// ===========================================================================
//  AI(W906-T3-PAUSE) 20260918
//  golden: bool __fastcall TfMain::Pause(AnsiString Func)  main.cpp:6325-6379
//
//  55 行，逐行翻譯。相依全部量過（見下），四個閘全是顯示或整合待辦，
//  **沒有一個擋得住暫停本身** —— 承重的那一行是 :6347 的 `SoftStop=true;`，
//  而它零相依。
//
//  ⚠ 這支會讓機台停下來（StopAllMotor()）。它是 S3 武裝面的一部分。
// ===========================================================================
bool TfMainWeb::PauseFromWeb(AnsiString Func)                                   // golden :6325
{
    bStartMoveSpeed = false;                                                    // golden :6327  Steven 20231018 : Fixed for G14
    NewRecordProcess("MES2111", "PAUSE pressed", Func);                         // golden :6328

    // golden :6330 「wei 20150825 移到外面」
    // ⓘ golden :6333 的第三個 disjunct 沒有加括號，靠 && 比 || 緊。這裡補上
    //   括號，語意逐位元相同，只是讓「a || b || (c && d)」讀得出來。
    if ((IniConfig.bEnable_SECS_GEM == true && IniConfig.bRCMDStart == true && bPhysicalStart == true) ||   // golden :6331
        (CUSTOMER_CODE == CC_ChipMos_ZHUBEI && IniConfig.bN25_1_EnableStartControl && bPhysicalStart == true) ||  // golden :6332
        (IniConfig.bN29_ParameterCheckForGMTest == true && bPhysicalStart == true))                        // golden :6333
    {
        bPhysicalStart = false;                                                 // golden :6335
    }

    if (SystemStart)                                                            // golden :6338
    {
        if (IniConfig.bEnable_SECS_GEM == true)                                 // golden :6340  Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoPause);                                    // golden :6341

        SendCommand_ESD(ESD_SYSTEM_STOP);                                       // golden :6343  Steven 20140722
        lHandlerStopTime.LatchCycleTime(true);                                  // golden :6344  jou 2014-09-21
    }

    // ★ golden :6347 的原註解：「很重要!!Alarm後沒下SoftStop流程會繼續跑」
    //   這是整支唯一承重的一行。ckernel.cpp:1046「暫停檢查」看到它，
    //   在 :1049/:1050 把 SoftStop 清掉並把 SystemStart 設為 false。
    SoftStop = true;                                                            // golden :6347  jou 20180102 (Steven)
    StopAllMotor();                                                             // golden :6348  0801

    fSCKART->AccessFile(false, -1);                                             // golden :6350

    // SAFETY-GATE(W906-T3-PAUSE-1) 🟡 golden :6351 `fStartCondition->WritePickerCount()`
    //   forms/fStartCondition.h:268/:466 兩處自己寫著 INTEGRATION-PENDING --
    //   NO `extern TfStartCondition *fStartCondition;` here。指標在 Command.cpp
    //   用得到（:10250 的 fShow），但這個 method 沒有可 include 的宣告。
    //   🟡：它記的是吸嘴真空次數計數（JerryYang 20220331），純統計。
    //   閘掉的後果：按 PAUSE 時那一次的 picker count 不會落檔，
    //   下次開機讀到的計數會少掉這一批。不影響停機本身。

    // SAFETY-GATE(W906-T3-PAUSE-2) 🟡 golden :6353-6354 `bShowInOutAlarm` / `lblInOutAlarm`
    //   兩個都不是 TfMain 的成員 —— csystem.cpp:16852 的既有 GATE G27a 與
    //   :16858 的 G32 已經記過同一件事（「fMain->bShowInOutAlarm / lblInOutAlarm
    //   not TfMain members」）。這裡不是新發現，是同一個洞的第三個呼叫點。
    //   🟡：純顯示（清掉 In/Out 警告字樣）。
    //   閘掉的後果：暫停後主畫面那行 In/Out 警告字樣不會被清空，
    //   會停在暫停前的最後一則。web UI 這一側該由 tag 表達。

    // SAFETY-GATE(W906-T3-PAUSE-3) 🟡 golden :6355 `UpdateTaskList()`
    //   golden main.cpp:6381-6400，本體是把 QueueTaskList[] 寫進
    //   `sgTaskList->Cells[j+1][i]` —— 一個 VCL TStringGrid。全樹 0 命中。
    //   🟡：純顯示，而且在 web UI 樹裡它本來就不該是表格寫入。
    //   閘掉的後果：Task List 分頁停在暫停前的內容。正解是發成 tag，
    //   不是把 TStringGrid 搬進來。

    if (IniConfig.bG14UseStartSoundAlarm)                                       // golden :6356  kevin 20201116
    {
        RunStartLowSpeedBuzzer(true);                                           // golden :6358
        bStartMoveSpeed = false;                                                // golden :6359
    }

    if (IniConfig.bUseAutoSiteMapping == true && IniConfig.bI21EnableASM == true)          // golden :6362  Jimmychiu 20230707
    {
        if (IniConfig.bI50_EnableAutoSiteMappingTrigger == true)                           // golden :6364
        {
            if (IniConfig.bI50_Pause == true)                                              // golden :6366
            {
                RecordProcess("Trigger Auto Site Map after PAUSE [I50]");                  // golden :6368
                // ⚠ INERT（不是閘，是行為缺口）：SetRunStartMode 在這棵樹是
                //   `void SetRunStartMode(int) {}`（aHotPlateSubstrate.cpp:1074），
                //   一個 no-op 樁。翻它是忠實的 —— golden 會改 run start mode，
                //   這裡什麼都不會發生。
                //   ⓘ 順帶一提：正因為它是 no-op，這一行**碰不到**
                //   LastSet.iRunStartMode，所以週末計畫 §0.4「不解凍
                //   iRunStartMode」沒有被違反。等那支樁換成真本體的那一天，
                //   這個呼叫點要跟著重審。
                //   ⚠ AI(W906-T7) 20260923: 「那一天」已經到了 —— 空樁早被 RunStartMode.cpp 的真本體取代
                //     （nm：`T SetRunStartMode(eRunStartMode, AnsiString)`），上面「INERT／碰不到 iRunStartMode」都已過期：
                //     這一行現在會照 golden 把 LastSet.iRunStartMode 改成 rsmAutoSiteMap 並寫 cbRunStartMode->Text。
                //     重審結論：照 golden 保留（這是翻譯，不是新行為）；週末計畫 §0.4 的「不解凍」前提已不成立，列晨報。
                SetRunStartMode(rsmAutoSiteMap);                                           // golden :6369
            }
        }
    }

    // SAFETY-GATE(W906-T3-PAUSE-4) 🟡 golden :6374-6377 `fMain->machineTime.Pause()`
    //   CC_PANTHER（鴻谷科技）專屬的機台運作狀態紀錄，Jimmychiu 20250916。
    //   `machineTime` 是 TMachineTimeManager，宣告在 ProductionInfo/uPAT_Function.h，
    //   而 PAT_Function 那一族尚未整合。使用者 20260918 已裁決：
    //   「缺 PAT_Function 沒關係，和這有關的先 Mark」。
    //   🟡：只有 CUSTOMER_CODE==CC_PANTHER 會走到，且純紀錄。
    //   閘掉的後果：鴻谷的機台稼動時間紀錄少了 PAUSE 這個轉折點。

    return true;                                                                // golden :6378  kevin 20141108
}


// =============================================================================
//  AI(W906-FTP-START) 20260928 [W906]（Steven 團隊 St01）R120-F：FTP 下載畫面「下載成功就解開 START」的 C++ 半邊
//
//  golden void __fastcall TfFTPClient::plSLoadClick(TObject *Sender)
//      V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:862 起，下載後段 :1005-1022 與 :1064-1084
//
//  上面 NETDownloadDataCheck 讀的兩樣東西 —— bFTPDownloadSetupFile 與 *_NET —— 在 golden 由這個按鈕的後段寫：
//    :1005-1022  下載結果 → bFTPDownloadSetupFile（成功 true／失敗 false），成功時順便清 PE 模式旗標
//    :1064-1084  下載、換配方、存檔都做完之後，把「新配方的值」抄進 *_NET（開機那一份在 FileRW/MainBoot.cpp 檔尾）
//  確安（CC_CYUEAN）結批時把 bFTPDownloadSetupFile 清成 false（forms/fLotInfo.cpp:7087，R120＝A），
//  golden 是操作員到 FTP 畫面重新下載一次就解開；移植樹原本沒有這一步，只能重開程式。
//
//  ⚠ 今天這兩支**沒有呼叫者**（安裝座，不是接上）：移植樹沒有 plSLoadClick，也沒有網頁的 FTP 下載畫面／指令
//    （KYECFTP\ 只有 LoadFileFormServer2 等四支傳輸函式，KYECFTP/FTPClient_Transfer.h:98-101；
//    D:\HT9045\web\page\ 沒有 FTP 下載頁；St02 的 origin/v906/steven-gpib-widget 也沒有）。頁面與指令是 W62（St02）。
//    之後做下載指令的人，照 golden 順序呼叫：
//      LoadFileFormServer2(sRootPath+IniConfig.FtpDownloadPath, 名稱)          golden :998-1001（Sigurd 走 FTPAutomation 路徑）
//      DownloadPasswordFormServer()                                           golden :1003（移植樹沒有；見下方 ⚠）
//      W906_FTPClient_DownloadResult(名稱, FTP_DownloadFail)                   golden :1005-1022  ← 本檔
//      （KYEC_LEE 海思版再抓一次比對清單）                                     golden :1024-1028
//      RecordProcess("Download file : "+DownloadWorkFile_NET)                 golden :1029-1030
//      cbSetupFileName 換成 名稱+"_NET"、cbSetupFileNameChange、btSaveSetupFile  golden :1031-1047
//      SECS EventReport(DownLoadRecipeByFTPOK／NG)                            golden :1053-1062
//      W906_FTPClient_DownloadRecordNetData()                                 golden :1064-1084  ← 本檔（一定在換配方之後：抄的是新配方）
//    宣告照本樹慣例在呼叫處寫 extern（WebStart.h 不加，免得移動那個檔的行號）：
//      extern void W906_FTPClient_DownloadResult(const AnsiString&, bool);
//      extern void W906_FTPClient_DownloadRecordNetData();
//
//  ⚠ golden 看起來不對的地方（照翻，不修；要改由 Steven／Jimmy 決定）：
//    :1007 讀的 FTP_DownloadFail 是 :1003 DownloadPasswordFormServer() 留下的值（golden FTPClient.cpp:4468-4556 會把它
//    設成 false 或 true），不是 :1000 工作檔下載的結果。CosFunction.PassworDownloadByFTP 與 IniConfig.bFtpPasswordDownload
//    都開時（:4470-4472），工作檔下載失敗、密碼本下載成功也會當成成功（START 解開，但工作檔沒換）；反過來工作檔成功、
//    密碼本連不上會當成失敗。任一個沒開時 DownloadPasswordFormServer 第一行就 return，FTP_DownloadFail 保持工作檔的結果。
//    呼叫者照 golden 順序傳「那一刻的 FTP_DownloadFail」，這兩支就跟 golden 一樣。
//    另一處（不影響 START，給做記錄的人）：:1029 記錄的 DownloadWorkFile_NET 在那一刻還是**上一次**的值
//    （這次的要到 :1068 才寫），所以 "Download file : " 那一行印的是上一次下載（或開機快照）的檔名。照 golden。
//
//  為什麼 FTP_DownloadFail 用參數傳，不在這裡直接讀：它定義在 KYECFTP/FTPClient_Transfer.cpp:346（ht9045_kyecftp），
//    wb_serve 目前沒有連進那個單元（nm build\wb_serve.exe 20260928：FTP_DownloadFail／LoadFileFormServer2 0 筆）。
//    在這裡引用它，連結器會第一次把整個 FTP 傳輸單元抽進 wb_serve —— 本次不想改連結內容。之後的下載指令本身
//    就會呼叫 LoadFileFormServer2，由它傳進來。
// =============================================================================
#include <cstdio>

void W906_FTPClient_DownloadResult(const AnsiString& sServerWaferName, bool bFTP_DownloadFail)
{
    SYS_SetupFile = sServerWaferName;                                           // golden :1005  SYS_SetupFile=edtServerWaferName->Text;  //20140103 wei

    if (bFTP_DownloadFail == true)                                              // golden :1007  jou 2014-04-16 ftp download fail fix
    {
        // golden :1009-1010 `iErrorBySECSGEM=3; iErrorByGPIB=3;` 是 TfFTPClient 的成員（SECS／GPIB 遠端下載的回覆碼），
        //   屬於下載指令那一側（它自己回覆遠端），這裡不做。
        bFTPDownloadSetupFile = false;                                          // golden :1011
    }
    else
    {
        bFTPDownloadSetupFile = true;                                           // golden :1015  ★ R120-F：確安結批清掉的記號在這裡設回
        bInitNeedDownloadFTP  = false;                                          // golden :1016  JerryYang 20200416 艾科要求切initial start按start要強制download recipe
        if (bEnablePEModel == false)                                            // golden :1017  Ifor 20160824 add 非PE工程模式底下才可清除旗標
        {
            bHasEnteredPEModel = false;                                         // golden :1019  Ifor 20160823 FTP檔案下載後關閉PE模式判斷旗標
            WriteIniDataGeneral("System", "bHasEnteredPEModel", bHasEnteredPEModel); // golden :1020  Ifor 20160824 避免 PE模式修改後程式被關閉上傳
            // ⚠ 上一行寫 D:\HT9045\system\Gerneral.ini [System] bHasEnteredPEModel（golden 同；量產機共用檔）。
        }
    }

    std::printf("WebStart: golden FTPClient.cpp:1005-1022 download result \"%s\" FTP_DownloadFail=%d -> bFTPDownloadSetupFile=%d "
                "bInitNeedDownloadFTP=%d bHasEnteredPEModel=%d (bEnablePEModel=%d)\n",
                sServerWaferName.c_str(), (int)bFTP_DownloadFail, (int)bFTPDownloadSetupFile,
                (int)bInitNeedDownloadFTP, (int)bHasEnteredPEModel, (int)bEnablePEModel);
}

void W906_FTPClient_DownloadRecordNetData()
{
    bool bRecorded = false;
    if (CosFunction.bUseFTPDownloadDataCheck == true &&                         // golden :1064  Ifor 20170727 (wei) add 比對下載資料 by 矽格中興廠
        bFTPDownloadSetupFile == true)                                          // golden :1065  Ifor 20180125 : Use FTP Download Data Check
    {                                                                           // golden :1066  jou 20180130 (Steven) : FTP download check 要code保持在最下面
        // record data
        DownloadWorkFile_NET    = SYS_SetupFile;                                // golden :1068
        LastSetTemperature_NET  = LastSet.iTemperature;                         // golden :1069
        TrayForm_NET            = TrayForm;                                     // golden :1070
        HotPlateForm_NET        = HotPlateForm_File;                            // golden :1071
        Temperature_NET         = Temperature;                                  // golden :1072
        TestIF_NET              = TestIF_File;                                  // golden :1073

        // golden :1075 `if(CUSTOMER_CODE==CC_SCC)` 被作者註解掉（Ifor 20181023）⇒ 下面這個區塊無條件執行，照原樣保留大括號。
        {
            DeviceForm_NET      = DeviceForm_File;                              // golden :1077
        }

        for (int i = 0; i < 8; i++)                                             // golden :1080  ChungHung 20141002 3->5 / Ifor 20170418 MRT Mode 5 -> 8
        {
            BinSelect_NET[i]    = BinSelect[i];                                 // golden :1082
        }
        bRecorded = true;
    }

    std::printf("WebStart: golden FTPClient.cpp:1064-1084 record download data -> %s (DownloadWorkFile_NET=\"%s\", "
                "LastSetTemperature_NET=%d)\n",
                bRecorded ? "recorded *_NET" : "not recorded (bUseFTPDownloadDataCheck or bFTPDownloadSetupFile is false)",
                DownloadWorkFile_NET.c_str(), LastSetTemperature_NET);
}
