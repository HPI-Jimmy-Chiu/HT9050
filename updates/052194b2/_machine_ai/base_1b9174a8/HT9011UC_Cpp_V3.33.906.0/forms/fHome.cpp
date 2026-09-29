// =============================================================================
//  forms/fHome.cpp  --  definitions for the fHome facade
//
//  AI(W906-W7-L2) 20260803: first home for TfHome; see forms/fHome.h for the
//  golden citations, the macro-collision check and the per-member offline
//  justifications.  Follows forms/fNote.cpp exactly.
// =============================================================================
#include "forms/fHome.h"
// AI(W906-HOME-C2) 20260920: 完整型別 —— fHome.h 只前置宣告 TLabel/TEdit/TListBox，
//   而下面的 ctor 要 `new vclcompat::TListBox()`，前置宣告不夠。
#include "vclcompat/Controls.h"

// The initialiser list is in DECLARATION ORDER (iHomeStep, fShow, fAbort) so
// there is no -Wreorder under the -Wall -Wextra this library is built with.
// Every value is read straight out of golden's own constructor, `__fastcall
// TfHome::TfHome(TComponent* Owner)` at uhome.cpp:106-113 -- iHomeStep=1
// (uhome.cpp:109), fShow=false (uhome.cpp:110), fAbort=false (uhome.cpp:111).
// Golden's fourth ctor statement, `ledHome->Visible=false;` (uhome.cpp:112), is
// pure VCL widget state with no member on this facade and is deliberately not
// carried over.
TfHome::TfHome() : iHomeStep(1), fShow(false), fAbort(false),
                   // AI(W906-HOME-C2) 20260920: ListBox1 在 golden 是 .dfm 元件，
                   //   由 VCL 在表單載入時建立。本樹沒有表單載入，所以在 ctor 建。
                   //   golden 的表單是在 HT9045.cpp:175 CreateForm 時無條件建立的，
                   //   所以「fHome 活著 ⇒ ListBox1 活著」在兩邊是同一個事實。
                   ListBox1(new vclcompat::TListBox()) {}

// Golden creates this form UNCONDITIONALLY at startup -- HT9045.cpp:175
// `Application->CreateForm(__classid(TfHome), &fHome);`, inside the flat
// unguarded CreateForm list at HT9045.cpp:166-245 (re-read in full; there is no
// enclosing if/config gate on any entry in that list).  So `fHome` is non-NULL
// for the whole life of the program after WinMain, which is precisely why
// golden's ckernel.cpp:372 and :373 dereference it with no null guard.  A live
// instance here is therefore the FAITHFUL offline state, not a convenience: a
// NULL global would turn those two unguarded golden writes into a segfault.
// Same allocate-at-load idiom as forms/fNote.cpp.
TfHome *fHome = new TfHome();

// =============================================================================
//  AI(W906-HOME-C1) 20260920 —— 歸零狀態機的資料層（golden uhome.cpp 的前段）
//
//  ## 為什麼這一塊現在才來
//
//  P0-4 量到歸零真正擋在 `CheckMotorHome()`（csystem.cpp:328）：它要求
//  TOTAL_MOTOR 顆馬達全部 `HomeFlag==1`。寫 `HomeFlag` 的
//  `TMyMotor::MotorHome()`（Motor/mymotor.cpp:1105/1132/1160）**早就翻好
//  而且是活的**，只是全樹沒有人呼叫它 —— 呼叫它的是 golden
//  `ProcessMotorHome()` case 600 的迴圈，而那個迴圈吃的就是下面這個
//  `HomeClass` 向量。所以資料層是狀態機的前置，先落地這一塊。
//
//  ## 本次落地的 golden 範圍
//
//      THomeClass::THomeClass        uhome.cpp:61-104
//      TfHome::InitialHomeClass      uhome.cpp:115-376   （173 列表 + 版面迴圈）
//      TfHome::FormDestroy           uhome.cpp:378-393
//      TfHome::ShowLed               uhome.cpp:664-680
//      TfHome::ResetAllMotorLed      uhome.cpp:682-686
//      TfHome::ShowMotorHomePos      uhome.cpp:688-718
//
//  `ProcessMotorHome()` 本體（uhome.cpp:1180-4842，**3,663 行**）不在本次，
//  它是下一波。
//
//  ## 這一塊**不改變任何執行期行為**
//
//  `InitialHomeClass()` 今天沒有呼叫者（golden 在 `TfHome::FormShow` /
//  `FormCreate` 那條 VCL 路徑上叫它，本樹沒有那條路徑）。落地它只是讓
//  下一波的 `ProcessMotorHome()` 有東西可以吃。
//  ⇒ 交付這一顆 commit 之後，`csystem.cpp` 的 Q13 假 seam 仍然照舊，
//    機台行為一格都沒動。這是刻意的：資料層和狀態機分開兩顆 commit，
//    出問題時才分得出是哪一邊。
//
//  ## 閘（只有一個，而且是純顯示）
//
//  `GATE(W906-HOME-C1-LEDHOME)` —— golden 的 `ledHome` 型別是 `TALed`
//  （`D:\HT9045\elec\Component\aled.pas`）。本樹的對應物 `vclcompat/LedCore.h`
//  的 `LedCore` **沒有 TControl 基底**（沒有 Left/Top），property 也是
//  setter 式（SetValue/SetTrueColor）而不是 `->Value=`。
//  ⇒ LED 欄位與 `ShowLed()` 的本體整段閘住。
//  **golden 的 `ShowLed` 不回傳值、不設任何旗標 —— 零控制流，純顯示。**
//  UN-GATE：等 vclcompat 出現 TControl 派生的 TALed。
// =============================================================================
#include <cstdio>
#include <cstdlib>

#include "cmydef.h"                 // MOT[] / TOTAL_MOTOR / FIRST_HOME.. / 機型旗標
#include "MachineType.h"
#include "Motor/mymotor.h"          // TMyMotor::ReadPos / Gali_ReadPos / PCIL132_SetPos
// ⛔⛔ **絕對不要** `#include "Motor/myGALILmotor.h"` ——
//   AI(W906-HOME-C1) 20260920 實測：加了它，`StopAllMotor()`（無參數）會綁到
//   `myGALILmotor.h:78` 的 `StopAllMotor(bool bIndexCanStop=true)`，於是連結器
//   為了解它而把 `myGALILmotor.cpp.obj` 拉進來 —— 然後出現
//   **609 個 `multiple definition of TMyMotor::Gali_*`**。
//
//   根因（**本樹既有的結構問題，不是本波造成的**）：
//     * `Motor/mymotor.cpp:1507-1530` 有一整批 `Gali_*` 的**樁**
//       （註解寫 "declared for ABI; all bodies gated TODO(W6-Galil)"）
//     * `Motor/myGALILmotor.cpp` 有同樣那批的**真本體**
//     兩者同在 `libht9045_motor.a`。GNU ld 只在需要解某個符號時才抽取成員，
//     所以只要沒有任何 TU 同時逼出這兩個 .obj，衝突就一直潛伏著。
//     ⇒ 這正是 memory「武裝沒編過的碼會壞別條線」的形狀：
//       我只是在 ShowMotorHomePos 裡照翻了一行 `StopAllMotor()`，
//       就把一個潛伏的 609 重複符號問題叫醒了。
//   紀錄在 docs/PLAN_START_TO_RUN.md，**不在這一波修** —— 修它要決定
//   「樁退休之後誰提供 Galil 符號」，那是 Galil 波次的事。
//
//   ⚠ 改用 `aHotPlateSubstrate.h:943` 的 `extern void StopAllMotor();`
//     （本樹其餘每一個呼叫端都是走這一條，例如 csystem.cpp）。
//     **它的定義是 `aHotPlateSubstrate.cpp:1173 void StopAllMotor() {}` ——
//     一個 no-op。** 也就是說 `ShowMotorHomePos` 偵測到 Z 軸位置異常時，
//     「停掉所有馬達」這一步在本樹今天**不會真的停**；
//     同一個分支裡的 `SoftStop=true` / `SystemStart=false` /
//     `iHomeStep=1` / `ShowMyMessage` 都是真的，所以歸零仍然會中止。
//     這是本樹既有的狀態，不是本波新增的落差。
#include "vclcompat/Controls.h"     // vclcompat::TLabel / vclcompat::TEdit
#include "canary_support.h"         // ShowMyMessage
// MyDBIProcess 的兩個宣告在本樹並存（cMyDB.h:81 是三參數的 __fastcall 版，
// aHotPlateSubstrate.h:944 是兩參數版）。golden uhome.cpp:390 用的是兩參數形式，
// 所以沿用後者 —— 與 forms/fHS.h:141 / forms/fMotorTest.h:289 記錄的既有慣例同一條路。
extern void MyDBIProcess(AnsiString S1, AnsiString S2);
// 同理，`StopAllMotor` 點名 extern，逐字照抄 aHotPlateSubstrate.h:943 的宣告。
// 不 include 那個標頭是因為它會與本檔已經帶進來的 cmydef.h/Motor/mymotor.h
// 撞上兩份 TMyKitSuck / TMySucker（見 uhome.cpp 檔頭的 ODR 說明）。
extern void StopAllMotor();

// ---------------------------------------------------------------------------
//  THomeClass -- golden uhome.cpp:61-104
//
//  ⚠ golden 的 `: TComponent(Owner)` 不帶過來：那是 BCB 的元件樹 ownership，
//    唯一的作用是「父元件解構時連帶釋放 labName/edPos/ledHome」。本樹沒有
//    元件樹，所以改由 `~THomeClass()` 明確 delete，並由
//    `TfHome::FormDestroy()`（golden :378，照翻）刪 vector 裡的每一個。
//    ⇒ 語意等價，而且沒有多出 golden 沒有的行為。
//
//  ⚠ golden 在 `Visible==false` 時**完全不建立** labName/edPos/ledHome ——
//    也就是那三個指標是**未初始化的野指標**（THomeClass 沒有初始化串列）。
//    這是 golden 自己的缺陷，不是本移植的。本樹把它們初始化成 NULL：
//    這是**唯一一處刻意偏離 golden**，理由是「未初始化指標」在 golden 那邊
//    靠「所有讀取點都先問 Visible」而僥倖成立（uhome.cpp:666 / :691 都有問），
//    在這裡照抄等於把一顆未爆彈搬進來，而且**行為完全相同**：
//    Visible==false 時沒有人碰它們。
//    ⇒ 偏離的是「未定義」變「定義為 NULL」，不是任何一條可觀察的路徑。
// ---------------------------------------------------------------------------
THomeClass::THomeClass(int MotNo, int Order, bool bView)
    : labName(0), edPos(0),
      HomeOrder(Order), index(MotNo), THomeOrder(Order), THomeFlag(1), Visible(bView)
{
    HomeOrder   =Order;                                                         // golden :63
    index       =MotNo;                                                         // golden :64
    Visible     =bView;                                                         // golden :65
    THomeOrder  =HomeOrder;                                                     // golden :66
    THomeFlag   =1;                                                             // golden :67
    AnsiString str;

    if(Visible==true)
    {
        labName                     =new vclcompat::TLabel();                   // golden :71 `new TLabel(this)`
        //GATE(W906-HOME-C1-WIDGETCFG) golden :72-79 —— labName 的 Parent/Name/Color/
        //  Height/Font 在 vclcompat::TLabel 上沒有對應成員（它只有 Caption/Color/
        //  Left/Top/Visible）。純外觀，零控制流。UN-GATE：等 vclcompat::TLabel 長出
        //  Parent/Name/Font。
        labName     ->Caption       =MOT[MotNo].NumberAlias;                    // golden :80  ← 有成員，照翻

        edPos               =new vclcompat::TEdit();                            // golden :82 `new TEdit(this)`
        //GATE(W906-HOME-C1-WIDGETCFG) golden :83-87 —— edPos 的 Parent/Name/Height/
        //  Width/ReadOnly 同上。
        edPos   ->Text      =AnsiString(0);                                     // golden :88 `edPos->Text=0;`（golden 靠 AnsiString(int) 隱式轉換）

        //GATE(W906-HOME-C1-LEDHOME) golden :90-100 —— 整個 ledHome（TALed）。見檔頭。
    }
}

// ---------------------------------------------------------------------------
//  ~THomeClass -- golden 沒有這個解構子（TComponent 的元件樹代勞）。
//  本樹沒有元件樹，所以明確釋放。見上面的橫幅。
// ---------------------------------------------------------------------------
THomeClass::~THomeClass()
{
    delete labName;
    delete edPos;
}

void TfHome::InitialHomeClass()                                                 // golden uhome.cpp:115  (__fastcall 去掉：本樹不是 VCL)
{
    static bool bInitial=false;

    if(bInitial==true)
        return ;

    bInitial=true;

    AnsiString Str;

    HomeClass.push_back(new THomeClass(MInArmX       , SECOND_HOME));           //Stven 20120823 Start : Home元件改用Vector新增
    HomeClass.push_back(new THomeClass(MInArmY       , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MInArmPitch   , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MInArmZA      , FIRST_HOME));
    HomeClass.push_back(new THomeClass(MInArmZB      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZC      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZD      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZE      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZF      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZG      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZH      , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInShuttle1   , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MInShuttle2   , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MTestY1       , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MTestZ1       , FIRST_HOME));
    HomeClass.push_back(new THomeClass(MTestZ2       , FIRST_HOME));
    HomeClass.push_back(new THomeClass(MTestY2       , SECOND_HOME, (USE_INDEX_ARM_AXES==IndexArm_4_Axis)));            //Jimmychiu 20221123 : Add for HT9016C

    HomeClass.push_back(new THomeClass(MOutShuttle1  , SECOND_HOME, false));
    HomeClass.push_back(new THomeClass(MOutShuttle2  , SECOND_HOME, false));
    HomeClass.push_back(new THomeClass(MOutArmX      , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MOutArmY      , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MOutArmPitch  , SECOND_HOME));
    HomeClass.push_back(new THomeClass(MOutArmZA     , FIRST_HOME));
    HomeClass.push_back(new THomeClass(MOutArmZB     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZC     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZD     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZE     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZF     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZG     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZH     , FIRST_HOME, (InOutArmPickerUseMotor==eptUseMot)));
    if(TRAY_ARM_MODE==eUnderCoveyor)
        HomeClass.push_back(new THomeClass(MTrayX, SECOND_HOME));
    else
        HomeClass.push_back(new THomeClass(MTrayX, THREE_HOME));

    HomeClass.push_back(new THomeClass(MInArmPitchY  , SECOND_HOME, (USE_IN_Y_IS_AUTO_PITCH==true)));                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
    HomeClass.push_back(new THomeClass(MInArmPitchX2 , SECOND_HOME, (USE_IN_Y_IS_AUTO_PITCH==true)));
    HomeClass.push_back(new THomeClass(MOutArmPitchY , SECOND_HOME, (USE_OUT_Y_IS_AUTO_PITCH==true)));
    HomeClass.push_back(new THomeClass(MOutArmPitchX2, SECOND_HOME, (USE_OUT_Y_IS_AUTO_PITCH==true)));
    HomeClass.push_back(new THomeClass(MLoaderZ      , SECOND_HOME, LOAD_Z_USE_MOTOR[0]));
    HomeClass.push_back(new THomeClass(MEmptyZ       , SECOND_HOME, LOAD_Z_USE_MOTOR[1]));
    HomeClass.push_back(new THomeClass(MColorZ       , SECOND_HOME, LOAD_Z_USE_MOTOR[2]));
    HomeClass.push_back(new THomeClass(MAuto1Z       , SECOND_HOME, (LOAD_Z_USE_MOTOR[3] || USE_LdUldCassetteMode)));   //Ifor 20251216 add:Boat Carrier
    HomeClass.push_back(new THomeClass(MAuto2Z       , SECOND_HOME, (LOAD_Z_USE_MOTOR[4] || USE_LdUldCassetteMode)));   //Ifor 20251216 add:Boat Carrier
    HomeClass.push_back(new THomeClass(MAuto3Z       , SECOND_HOME, LOAD_Z_USE_MOTOR[5]));
    HomeClass.push_back(new THomeClass(MInRotateKit  , SECOND_HOME, (USE_ROTATE_KIT && iRotate_Type!=eCynRotate)));
    HomeClass.push_back(new THomeClass(MOutRotateKit , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e1MotRotate || iRotate_Type==e8MotRotate || iRotate_Type==e1MotRotate1Dut || iRotate_Type==eInOutArm1Motor))));  //add One sucker with rotate //Steven 20170705 (wei) : 修正Rotate回Home頁面顯示問題
    HomeClass.push_back(new THomeClass(MAOIKit       , SECOND_HOME, USE_AOI_Inspection));
    if(USE_LdUldCassetteMode==1)                                                //Ifor 20251216 add:Boat Carrier
        HomeClass.push_back(new THomeClass(MLoaderY  , FIRST_HOME, USE_LdUldCassetteMode));
    else
        HomeClass.push_back(new THomeClass(MLoaderY      , SECOND_HOME, (INSTALL_OCR_YMot==eocrYMotInstal)));           //RogerYang 20250909 : 只要是Motor就要回home
    HomeClass.push_back(new THomeClass(MEmptyY       , SECOND_HOME, false));
    HomeClass.push_back(new THomeClass(MColorY       , SECOND_HOME, false));
    if(USE_LdUldCassetteMode==1)                                                //Ifor 20251216 add:Boat Carrier
    {
        HomeClass.push_back(new THomeClass(MAuto1Y   , FIRST_HOME, USE_LdUldCassetteMode));
        HomeClass.push_back(new THomeClass(MAuto2Y   , FIRST_HOME, USE_LdUldCassetteMode));
    }
    else
    {
        HomeClass.push_back(new THomeClass(MAuto1Y       , SECOND_HOME, false));
        HomeClass.push_back(new THomeClass(MAuto2Y       , SECOND_HOME, false));
    }

    HomeClass.push_back(new THomeClass(MAuto3Y       , SECOND_HOME, false));
    HomeClass.push_back(new THomeClass(MInArmZAe     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));                      //Steven 20230323 : For HT1032
    HomeClass.push_back(new THomeClass(MInArmPitchX3 , SECOND_HOME, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)));
    HomeClass.push_back(new THomeClass(MInArmPitchX4 , SECOND_HOME, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)));
    HomeClass.push_back(new THomeClass(MInArmZAf     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmPitchX3, SECOND_HOME, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)));
    HomeClass.push_back(new THomeClass(MOutArmPitchX4, SECOND_HOME, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)));
    if(USE_LdUldCassetteMode==1)
        HomeClass.push_back(new THomeClass(MTrayZ        , SECOND_HOME,  (USE_LdUldCassetteMode)));                     //Ifor 20251216 add:Boat Carrier
    else
        HomeClass.push_back(new THomeClass(MTrayZ        , FIRST_HOME,  (TRAY_ARM_MODE==eUnderCoveyor)));               //Ifor 20251216 add:Boat Carrier
    HomeClass.push_back(new THomeClass(MOutSortAa    , FIRST_HOME, (USE_OUT_SORT_ARM==iOutSortX40mm)));                 //Steven 20240822 : For HT-9046AU
    HomeClass.push_back(new THomeClass(MOutSortAb    , FIRST_HOME, (USE_OUT_SORT_ARM==iOutSortX40mm)));
    HomeClass.push_back(new THomeClass(MInArmXScale  , THREE_HOME, false));
    HomeClass.push_back(new THomeClass(MInArmYScale  , THREE_HOME, false));
    HomeClass.push_back(new THomeClass(MOutArmXScale , THREE_HOME, false));
    HomeClass.push_back(new THomeClass(MOutArmYScale , THREE_HOME, false));
    HomeClass.push_back(new THomeClass(MShuttle1Pitch, SECOND_HOME, AUTO_SENSOR_INSTALL));                              //wei 20160914 Auto Shuttle Sensor
    HomeClass.push_back(new THomeClass(MShuttle2Pitch, SECOND_HOME, AUTO_SENSOR_INSTALL));                              //wei 20160914 Auto Shuttle Sensor
    //Steven 20170329 (Wei) : Add individual rotate motor
    //==>
    HomeClass.push_back(new THomeClass(MInRotateB    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));             //JerryYang 20170605 (wei) 修正回Home頁面顯示問題  //Steven 20260316 : Fix operator precedence
    HomeClass.push_back(new THomeClass(MInRotateC    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate))));
    HomeClass.push_back(new THomeClass(MInRotateD    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate))));
    HomeClass.push_back(new THomeClass(MInRotateE    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));  //Steven 20260316 : Fix operator precedence
    HomeClass.push_back(new THomeClass(MInRotateF    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));             //Steven 20260316 : Fix operator precedence
    HomeClass.push_back(new THomeClass(MInRotateG    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate))));
    HomeClass.push_back(new THomeClass(MInRotateH    , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate))));
    HomeClass.push_back(new THomeClass(MOutRotateB   , SECOND_HOME, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    HomeClass.push_back(new THomeClass(MOutRotateC   , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    HomeClass.push_back(new THomeClass(MOutRotateD   , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    HomeClass.push_back(new THomeClass(MOutRotateE   , SECOND_HOME, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    HomeClass.push_back(new THomeClass(MOutRotateF   , SECOND_HOME, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    HomeClass.push_back(new THomeClass(MOutRotateG   , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    HomeClass.push_back(new THomeClass(MOutRotateH   , SECOND_HOME, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    //<==
    //Steven 20170329 (Wei) : Add individual rotate motor

    HomeClass.push_back(new THomeClass(MLightScale   , SECOND_HOME, USE_TRAY_ROBOT));                                   //Steven 20170330 (Wei) : For HT-9046LM
    HomeClass.push_back(new THomeClass(MInArmZAg     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZAh     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MArmAlignment , SECOND_HOME, false));
    HomeClass.push_back(new THomeClass(MLoadHingeR   , SECOND_HOME, USE_LOADER_HINGE));                                 //Steven 20170330 (Wei) : For TSMC
    HomeClass.push_back(new THomeClass(MLoadHingeZ   , SECOND_HOME, USE_LOADER_HINGE));                                 //Steven 20170330 (Wei) : For TSMC
    HomeClass.push_back(new THomeClass(MPreciser     , SECOND_HOME, USE_PRECISER));                                     //Frank 20180410 (Steven) : InArm Preciser Station
    //JerryYang 20180213 add motor
    //==>
    HomeClass.push_back(new THomeClass(MInArmZBe     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZBf     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZBg     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MInArmZBh     , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZAe    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZAf    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZAg    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZAh    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZBe    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZBf    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZBg    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    HomeClass.push_back(new THomeClass(MOutArmZBh    , FIRST_HOME,  (USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMot)));
    //<==
    //JerryYang 20180213 add motor

    HomeClass.push_back(new THomeClass(MOutSortX     , SECOND_HOME,  (USE_OUT_SORT_ARM==iOutSortX40mm)));               //Steven 20240822 : For HT-9046AU
    HomeClass.push_back(new THomeClass(MCaselevatorZ , FIRST_HOME,  false));    //Sam 20190112 LM

    HomeClass.push_back(new THomeClass(MCasArmX      , SECOND_HOME, USE_MR_SYSTEM));
    HomeClass.push_back(new THomeClass(MCasArmZ      , FIRST_HOME,  USE_MR_SYSTEM));
    HomeClass.push_back(new THomeClass(MTrayBracketZ , FIRST_HOME,  USE_MR_SYSTEM));
    HomeClass.push_back(new THomeClass(MStackedTrayX , SECOND_HOME, USE_MR_SYSTEM));
    HomeClass.push_back(new THomeClass(MStackedTrayZ , FIRST_HOME,  USE_MR_SYSTEM));

    HomeClass.push_back(new THomeClass(MUnloadRobotZ , THREE_HOME,  (USE_MR_SYSTEM==2)));
    HomeClass.push_back(new THomeClass(MOutSortY     , SECOND_HOME, (USE_OUT_SORT_ARM==iOutSortX40mm)));                //Steven 20240822 : For HT-9046AU

    HomeClass.push_back(new THomeClass(MCCDX         , THREE_HOME,  USE_FINE_PITCH));
    HomeClass.push_back(new THomeClass(MCCDY         , THREE_HOME,  USE_FINE_PITCH));
    HomeClass.push_back(new THomeClass(MCCDZ         , SECOND_HOME, USE_FINE_PITCH));

    //Frank 20210612 : Flipper Function
    //==>
    HomeClass.push_back(new THomeClass(MInFlipper1   , THREE_HOME,   USE_DEVICE_FLIPPER));
    HomeClass.push_back(new THomeClass(MInFlipper2   , THREE_HOME,   USE_DEVICE_FLIPPER));
    HomeClass.push_back(new THomeClass(MInFlipper3   , THREE_HOME,   USE_DEVICE_FLIPPER));
    HomeClass.push_back(new THomeClass(MOutFlipper1  , THREE_HOME,   USE_DEVICE_FLIPPER));
    HomeClass.push_back(new THomeClass(MOutFlipper2  , THREE_HOME,   USE_DEVICE_FLIPPER));
    HomeClass.push_back(new THomeClass(MOutFlipper3  , THREE_HOME,   USE_DEVICE_FLIPPER));
    //<==
    //Frank 20210612 : Flipper Function

    HomeClass.push_back(new THomeClass(MLdCarRotArm           , FIRST_HOME, USE_LD_Rot_Arm));
    if(USE_LdUldCassetteMode==1)                                                //Ifor 20251216 add:Boat Carrier
    {
        HomeClass.push_back(new THomeClass(MLoaderY_CCW       , FIRST_HOME,   USE_LdUldCassetteMode));
        HomeClass.push_back(new THomeClass(MAuto1Y_CCW        , FIRST_HOME,   USE_LdUldCassetteMode));
        HomeClass.push_back(new THomeClass(MAuto2Y_CCW        , FIRST_HOME,   USE_LdUldCassetteMode));
    }
    else
    {
        HomeClass.push_back(new THomeClass(MLoaderY_CCW           , SECOND_HOME,  false));
        HomeClass.push_back(new THomeClass(MAuto1Y_CCW            , SECOND_HOME,  false));
        HomeClass.push_back(new THomeClass(MAuto2Y_CCW            , SECOND_HOME,  false));
    }

    HomeClass.push_back(new THomeClass(MAuto3Y_CCW            , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MAuto4Y_CCW            , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MAuto5Y_CCW            , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MAuto6Y_CCW            , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(M1_3R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_4X         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_4Y         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_4R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));

    HomeClass.push_back(new THomeClass(M1_5X         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_5Y         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_5R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_6X         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_6Y         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_6R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_7X         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_7Y         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_7R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_8X         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_8Y         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(M1_8R         , SECOND_HOME,  (MachineTypeChoice==Type_HT502)));
    HomeClass.push_back(new THomeClass(MMagazine     , THREE_HOME,    AUTO3_IS_MAGAZINE));                              //JerryYang 20220909 : add magazine
    HomeClass.push_back(new THomeClass(MCatchMgzTray , SECOND_HOME,   AUTO3_IS_MAGAZINE));                              //JerryYang 20220909 : add magazine
    HomeClass.push_back(new THomeClass(MMagYTrayOut  , SECOND_HOME,   false));  //JerryYang 20220909 : add magazine
    HomeClass.push_back(new THomeClass(MFix3Full     , SECOND_HOME,  (FIX3_FULL_PLACE==Fix3K_UseStepperMotor)));        //JimmyChiu 20220927 : Stepper Motor Control in Fix3
    HomeClass.push_back(new THomeClass(MAuto4Z       , SECOND_HOME,  LOAD_Z_USE_MOTOR[6]));                             //Steven 20230907 : For HT-9011UC
    HomeClass.push_back(new THomeClass(MAuto5Z       , SECOND_HOME,  LOAD_Z_USE_MOTOR[7]));
    HomeClass.push_back(new THomeClass(MAuto6Z       , SECOND_HOME,  LOAD_Z_USE_MOTOR[8]));
    HomeClass.push_back(new THomeClass(MAuto4Y       , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MAuto5Y       , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MAuto6Y       , SECOND_HOME,  false));

    HomeClass.push_back(new THomeClass(MTopAOIArmX, FIRST_HOME,   false)); //USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));  //Ian 20230823 Top AOI Function
    HomeClass.push_back(new THomeClass(MTopAOIArmY, FIRST_HOME,   false)); //USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));  //Ian 20230823 Top AOI Function
    HomeClass.push_back(new THomeClass(MTopAOIArmR, SECOND_HOME,  false)); // USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));  //Ian 20230823 Top AOI Function
    HomeClass.push_back(new THomeClass(MTopAOICCDZ, FIRST_HOME,    USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));  //Ian 20230823 Top AOI Function
    HomeClass.push_back(new THomeClass(MTopAOIElevZ1 , SECOND_HOME,  false));
    HomeClass.push_back(new THomeClass(MTopAOIElevZ2 , SECOND_HOME,  false));

    HomeClass.push_back(new THomeClass(MOutSortPitchX, SECOND_HOME,  (USE_OUT_SORT_ARM==iOutSortX40mm)));               //Steven 20240822 : For HT-9046AU
    HomeClass.push_back(new THomeClass(MOutSortSht   , SECOND_HOME,  (USE_OUT_SORT_ARM==iOutSortX40mm)));
    HomeClass.push_back(new THomeClass(MLoad2Z       , SECOND_HOME,  (USE_2nd_LOADER==eartInstall)));                   //RogerYang 20250331 : For HT-9046AU
    HomeClass.push_back(new THomeClass(MLoad2Y      , SECOND_HOME,  false));

    HomeClass.push_back(new THomeClass(MInSh1LtcSenZ1, SECOND_HOME, In_Shuttle_Auto_Latch));                            //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    HomeClass.push_back(new THomeClass(MInSh1LtcSenZ2, SECOND_HOME, In_Shuttle_Auto_Latch));                            //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    HomeClass.push_back(new THomeClass(MInSh2LtcSenZ1, SECOND_HOME, In_Shuttle_Auto_Latch));                            //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    HomeClass.push_back(new THomeClass(MInSh2LtcSenZ2, SECOND_HOME, In_Shuttle_Auto_Latch));                            //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料

    for(int i=HomeClass.size(); i<TOTAL_MOTOR; i++)                             //Steven 20221115 : 修正歸零例外
    {
        Str.sprintf("[M%d]", i);
        HomeClass.push_back(new THomeClass(i, THREE_HOME,  false));
    }

    int iLPitch =270,   iTPitch =30;
    int iLabelL =5,     iLabelT =8;
    int iLedL   =170,   iLedT   =8;
    (void)iLedL; (void)iLedT;   // 只有被 GATE(W906-HOME-C1-LEDHOME) 閘掉的兩行用到；
                                // 保留 golden 的宣告與數值，解閘時原地就能用。
    int iEditL  =195,   iEditT  =8;
    int iMaxRowItem=15;
    int ColItem=0, RowItem=0;

    for(unsigned int i=0; i<HomeClass.size(); i++)
    {
        if(HomeClass[i]->Visible)
        {
            HomeClass[i]->labName->Top  =iLabelT+iTPitch*RowItem;
            HomeClass[i]->labName->Left =iLabelL+iLPitch*ColItem;
//GATE(W906-HOME-C1-LEDHOME) golden :363  HomeClass[i]->ledHome->Top  =iLedT  +iTPitch*RowItem;
//GATE(W906-HOME-C1-LEDHOME) golden :364  HomeClass[i]->ledHome->Left =iLedL  +iLPitch*ColItem;
            HomeClass[i]->edPos->Top    =iEditT +iTPitch*RowItem;
            HomeClass[i]->edPos->Left   =iEditL +iLPitch*ColItem;
            RowItem++;

            if(RowItem>=iMaxRowItem)
            {
                RowItem=0;
                ColItem++;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  TfHome::FormDestroy -- golden uhome.cpp:378-393
//
//  簽章去掉 `TObject *Sender`（VCL 事件簽章，本樹沒有事件來源）與
//  `__fastcall`。golden 的 `vec_clr(HomeClass)` 是一個把 vector 換成空 vector
//  以真正釋放容量的慣用法；本樹用 `std::vector<THomeClass *>().swap(...)`
//  表達同一件事（vec_clr 本身是 golden 的巨集，沒有翻進來）。
//  golden 尾端的 `LogSoftwareOffTime("TfHome, FormDestroy")` 沒有落地 ——
//  GATE(W906-HOME-C1-OFFTIME)，缺相依：`LogSoftwareOffTime` 全樹 0 命中。
//  行為：軟體關閉時間不會被記一筆。純記錄。
// ---------------------------------------------------------------------------
void TfHome::FormDestroy()
{
    try
    {
        for(std::vector<THomeClass *>::iterator iter=HomeClass.begin(); iter!=HomeClass.end(); ++iter)
        {
            delete *iter;                                                       // golden :384
        }
        std::vector<THomeClass *>().swap(HomeClass);                            // golden :386 `vec_clr(HomeClass);`
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfHome::FormDestroy");                       // golden :390
    }
    //GATE(W906-HOME-C1-OFFTIME) golden :392  LogSoftwareOffTime("TfHome, FormDestroy");
}

// ---------------------------------------------------------------------------
//  TfHome::ShowLed -- golden uhome.cpp:664-680
//
//  ⚠ 本體整段閘住（GATE(W906-HOME-C1-LEDHOME)）。golden 的本體是四行對
//  `ledHome->TrueColor` / `->Value` 的賦值，**不回傳值、不設任何旗標、
//  不影響任何控制流** —— 它的呼叫端（case 600 / case 650 / ResetAllMotorLed）
//  都只是在畫面上把某一顆馬達的燈變綠/紅/黃。
//  ⇒ 閘住的後果是「歸零畫面上的燈不會亮」，不是「歸零流程改變」。
//  UN-GATE：等 vclcompat 出現 TControl 派生的 TALed。
// ---------------------------------------------------------------------------
void TfHome::ShowLed(int index, int attr)
{
    (void)index; (void)attr;
#if 0 // GATE(W906-HOME-C1-LEDHOME): 缺相依，見上面的就地註解
    if(HomeClass[index]->Visible==true)
    {
        if(attr)
        {
            if(     attr==1) HomeClass[index]->ledHome->TrueColor=clLime;
            else if(attr==2) HomeClass[index]->ledHome->TrueColor=clRed;
            else if(attr==3) HomeClass[index]->ledHome->TrueColor=clYellow;
            HomeClass[index]->ledHome->Value=true;
        }
        else
        {
            HomeClass[index]->ledHome->Value=false;
        }
    }
#endif // GATE(W906-HOME-C1-LEDHOME)
}

// ---------------------------------------------------------------------------
//  TfHome::ResetAllMotorLed -- golden uhome.cpp:682-686
//  照翻。它呼叫的 ShowLed 目前是 no-op（見上），所以整支也是 no-op ——
//  但形狀留著，ShowLed 一解閘它就跟著活。
// ---------------------------------------------------------------------------
void TfHome::ResetAllMotorLed()
{
    for(int i=0; i<TOTAL_MOTOR; i++)
        ShowLed(i, 0);                                                          // golden :685
}

// ---------------------------------------------------------------------------
//  TfHome::ShowMotorHomePos -- golden uhome.cpp:688-718
//
//  ★ **這一支是承重的，不是顯示。** 名字叫 Show，但它的 `return false`
//  會讓 case 600 的迴圈 `if(!fHome->ShowMotorHomePos(i)) return true;`
//  直接中止整個歸零，並且 `SoftStop=true` / `SystemStart=false` /
//  `iHomeStep=1`。它檢查的是：Arm 的 Z 軸歸零後位置若 >5000，
//  代表歸零感應器有問題 —— 那是真的會撞機的狀況。
//  ⇒ 完整翻譯，一格都不閘。
// ---------------------------------------------------------------------------
bool TfHome::ShowMotorHomePos(int index)
{
    AnsiString S1, S2;
    if(HomeClass[index]->Visible==true)
    {
        if(index==MTestY1 || index==MTestZ1 || index==MTestZ2 || index==MTestY2)
            HomeClass[index]->edPos->Text=MOT[index].Gali_ReadPos();            // golden :694
        else
            HomeClass[index]->edPos->Text=MOT[index].ReadPos();                 // golden :696

        if((index>=MInArmZA && index<=MInArmZH) ||
           (index>=MOutArmZA && index<=MOutArmZH))                              //Steven 20091120 Start : Check Suck Z Home Position
        {
            if(atoi(HomeClass[index]->edPos->Text.c_str())>5000)
            {
                StopAllMotor();
                fHome->ShowLed(index, 2);
                S1.sprintf("M%2d position error, please check the home sensor!!", index);
                S2.sprintf("M%2d 位置錯誤，請檢查歸零感應器!!", index);
                ShowMyMessage(S1, S2, "ShowMotorHomePos");
                MOT[index].PCIL132_SetPos(0);
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return false;
            }
        }
    }
    return true;
}
