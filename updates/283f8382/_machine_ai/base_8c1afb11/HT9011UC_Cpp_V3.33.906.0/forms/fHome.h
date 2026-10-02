// =============================================================================
//  forms/fHome.h  --  non-VCL stand-in for golden's fHome form pointer
//
//  AI(W906-W7-L2) 20260803: FIRST home for TfHome anywhere in this migrated
//  tree (golden uhome.h:32 `class TfHome : public TForm`).  Landed by the
//  W7-L2 substrate pass that runs ahead of the translation of golden
//  ckernel.cpp.  Follows the forms/fNote.{h,cpp} convention exactly: plain
//  class, extern global pointer with the golden declaration cited, offline
//  defaults justified individually, virtual destructor.  The facade-wide
//  contract written out in forms/fMain.h binds this file too.
//
//  THAT THIS IS THE FIRST HOME IS NOT AN ASSUMPTION -- AutoClean/AutoClean.cpp
//  :253-258 already recorded it in prose ("TfHome (uhome.h) is not yet ported
//  anywhere in this tree"), and this pass re-verified it by grep before
//  writing a line.
//
//  ---------------------------------------------------------------------------
//  NO MACRO COLLISION -- CHECKED, BECAUSE THE BRIEF ASKED FOR IT SPECIFICALLY
//  ---------------------------------------------------------------------------
//  Two places in this tree already stand in for a TfHome method.  NEITHER
//  defines a macro named `fHome`, so introducing the real global below cannot
//  collide with either, and neither file is touched by this wave:
//    * csystem.cpp:2659  `#define W7C2_FHOME_SERVOOFF(f) do{(void)(f);}while(0)`
//      -- stands in for golden fHome->GaliMotorServoOff(AnsiString) (uhome.h:77).
//    * AutoClean/AutoClean.cpp:264-265  `static void W906DIAC_InitDoTestZHome(){}`
//      + `#define W906DIAC_FHOME_INITDOTESTZHOME() W906DIAC_InitDoTestZHome()`
//      -- stands in for golden fHome->InitDoTestZHome() (uhome.h:74).
//  Both are used at their call sites AS THE MACRO, never as `fHome->...`, so
//  they keep working unchanged and keep their gates.  Deliberately, this facade
//  does NOT declare GaliMotorServoOff / InitDoTestZHome / TestZTask: adding them
//  would invite someone to "retire" those two seams without translating the
//  golden bodies (uhome.cpp:4891 and the ServoOff path), which would swap two
//  honest no-ops for two silent ones.  Retiring them belongs to the wave that
//  translates uhome.cpp.  Reported to the integrator, not done here.
//  AI(W906-MT-E3b) 20260925: that wave is here for GaliMotorServoOff -- its golden
//  body (uhome.cpp:4988-5016) and InitGali_HomeTask (:646-662) are translated line for
//  line in uhome.cpp and declared below, and csystem.cpp's W7C2_FHOME_SERVOOFF now
//  calls it; so is sbAbortHomeClick (:4980-4986), its caller in MainProc's stop arm.
//  AI(W906-FLOW-2) 20260928: InitDoTestZHome / TestZTask are declared now (below) -- the golden body (uhome.cpp:4891-4906) is translated at the end of AutoClean/AutoClean.cpp, and AutoClean.cpp's W906DIAC_FHOME_INITDOTESTZHOME() calls it.
//
//  ---------------------------------------------------------------------------
//  GOLDEN ckernel.cpp CENSUS FOR fHome -- MEASURED OVER ALL 2589 LINES
//  ---------------------------------------------------------------------------
//    :372  fHome->fAbort=false;      (write) -- ScanSystemSensor, THIS FRONT
//    :373  fHome->iHomeStep=1;       (write) -- ScanSystemSensor, THIS FRONT
//    :744  else if(fHome->fShow)     (read)  -- ShowRunLed,   W7-L2 DEFERRED
//    :1073 else if(fHome->fShow)     (read)  -- ShowRunLabel, W7-L2 DEFERRED
//  Exactly 4 sites, 3 members.  All three are landed; see the per-member note
//  for why fShow is landed ahead of its consumer.
// =============================================================================
#ifndef FORMS_FHOME_H
#define FORMS_FHOME_H

// AI(W906-HOME-C1) 20260920: 本檔原本刻意不 include 任何東西（見下面那段註解）。
// 現在加了 <vector> 與兩個**前置宣告**（TLabel / TEdit），因為 golden 的
// `THomeClass`（uhome.h:18）本來就住在同一個標頭裡，而 `TfHome::HomeClass`
// 是 `vector<THomeClass *>`（uhome.h:79）。只前置宣告、不 include 版面標頭，
// 所以「ckernel_shims.cpp 可以 include 本檔而不把 widget 型別拖進那個 TU」
// 這個原始約束仍然成立 —— 只有 forms/fHome.cpp 自己需要完整型別。
// ⚠ 只前置宣告，**不做 `using vclcompat::TEdit;`** —— `vclcompat/Controls.h`
//   自己在檔尾（:535/:539）已經有同名的 using-declaration，本檔再寫一次會在
//   兩者同時被 include 的 TU 裡重複宣告。成員一律寫完整的 `vclcompat::TEdit *`。
#include <vector>

namespace vclcompat { class TLabel; class TEdit; class TListBox; }
// AI(W906-MT-E3b) 20260925: forward declaration only (same rule as the three above) --
//   GaliMotorServoOff takes golden's `AnsiString sFunc` by value; a declaration may name an
//   incomplete parameter type, and every caller already has the full type.
namespace vclcompat { class AnsiString; }

// ===========================================================================
//  THomeClass -- golden uhome.h:18-32 `class THomeClass : public TComponent`
//
//  每一顆馬達在歸零畫面上的一列：名稱標籤、位置欄、狀態燈，加上**承重的兩個
//  整數** `THomeOrder`（這顆馬達屬於哪一個歸零階段）與 `THomeFlag`
//  （這顆還沒歸好嗎）。golden 的 `ProcessMotorHome` case 600（uhome.cpp:3020）
//  的迴圈條件就是這兩個：
//      if(fHome->HomeClass[i]->THomeFlag &&
//         fHome->HomeClass[i]->THomeOrder==MotorTask)
//
//  ⚠ 不繼承 TComponent：golden 繼承它只為了 `new TLabel(this)` 的
//  ownership（BCB 的元件樹會在父元件解構時連帶釋放子元件）。本樹沒有元件樹，
//  解構由 `TfHome::FormDestroy()`（golden uhome.cpp:378-393）逐一 delete，
//  那一段是照翻的。
//
//  ⚠ `ledHome` 沒有落地 —— **缺相依**，不是省略：
//  golden 的型別是 `TALed`（`D:\HT9045\elec\Component\aled.pas`），本樹的
//  對應物是 `vclcompat/LedCore.h` 的 `LedCore`，那是一個**沒有 TControl 基底**
//  的 frameworkless core：沒有 `Left`/`Top`，也沒有 `Value`/`TrueColor` 的
//  property 語法（只有 SetValue/SetTrueColor）。把 `LedCore` 硬塞進來會讓
//  `InitialHomeClass()` 尾端那段版面迴圈（golden :356-375）與
//  `TfHome::ShowLed()`（golden :664-680）都變成半翻半改。
//  ⇒ 整個 LED 欄位閘住，`ShowLed()` 的本體一併閘住並在那裡寫明。
//  **純顯示，零控制流** —— golden 的 `ShowLed` 不回傳值、不設任何旗標。
//  UN-GATE：等 vclcompat 出現 TControl 派生的 TALed（LedCore + Left/Top）。
// ===========================================================================
class THomeClass
{
public:
    vclcompat::TLabel *labName;         // golden uhome.h:22
    vclcompat::TEdit  *edPos;           // golden uhome.h:23
    // TALed *ledHome;                  // golden uhome.h:24 -- GATE，見上面的橫幅
    int     HomeOrder;                  // golden uhome.h:25
    int     index;                      // golden uhome.h:26
    int     THomeOrder;                 // golden uhome.h:27
    int     THomeFlag;                  // golden uhome.h:28
    bool    Visible;                    // golden uhome.h:29

    THomeClass(int MotNo, int Order, bool bView=true);   // golden uhome.h:21
    ~THomeClass();
};

// No include is needed: every member below is a plain int/bool.  (fNote.h
// includes forms/FormWidgets.h only because it owns a TEdit*.)  Keeping this
// header dependency-free lets ckernel_shims.cpp -- and the ckernel.cpp that
// follows -- include it without dragging widget types into their TU.

class TfHome
{
public:
    // -----------------------------------------------------------------------
    //  [DATA] golden uhome.h:69 `int iHomeStep;` -- the Home form's own home
    //  sequence cursor, switched on by golden ProcessMotorHome (the free
    //  function opening at uhome.cpp:1180 `bool ProcessMotorHome(bool Flag2)`;
    //  e.g. :2164 =2, :2281 =3, :2335 =4, :4819 =1300).  Counted this pass
    //  rather than estimated: uhome.cpp contains 135 `iHomeStep=` assignments.
    //  OFFLINE VALUE 1, which is GOLDEN'S OWN CTOR VALUE (uhome.cpp:109
    //  `iHomeStep =1;` inside `__fastcall TfHome::TfHome(TComponent* Owner)`
    //  at uhome.cpp:106), not an invented default.
    //  WHICH BRANCH DOES IT SELECT?  Stated honestly: NONE, inside ckernel.cpp.
    //  Golden ckernel.cpp only ever WRITES this member (:373) and never reads
    //  it, so no arm of ScanSystemSensor turns on its value.  The value still
    //  is not arbitrary: 1 is simultaneously golden's ctor value AND the value
    //  golden writes at every "restart the home sequence" site (uhome.cpp:400,
    //  :711, :1222, :1696, :2530, :3189, :4151 among others), i.e. "sequence
    //  parked at step 1, nothing in flight".  Choosing 1 makes the pre-SoftStart
    //  state identical to the post-:373 state, which is what golden gets on a
    //  freshly constructed form.
    // -----------------------------------------------------------------------
    int  iHomeStep;    int TestZTask = 0;   //AI(W906-FLOW-2) 20260928: golden uhome.h:68 `int TestZTask;` -- written only by InitDoTestZHome (golden uhome.cpp:4893), read only by State Record (golden main.cpp:9936 QueueTaskList[198]).  0 = golden's value before the first write (BCB zero-fills a TForm; golden ctor uhome.cpp:106-113 never sets it)

    // -----------------------------------------------------------------------
    //  [DATA] golden uhome.h:70 `bool fShow;` -- "the Home dialog is displayed",
    //  i.e. a homing sequence is on screen and running.
    //  OFFLINE VALUE false, GOLDEN'S OWN CTOR VALUE (uhome.cpp:110
    //  `fShow =false;`).
    //  BRANCHES THAT false SELECTS -- both verified by reading the golden lines:
    //    * ckernel.cpp:744 `else if(fHome->fShow) { RunState=LED_Homeing; }`
    //      false SKIPS the LED_Homeing tower-light state and lets ShowRunLed
    //      fall through to the FT-CT reply arm (:748), the Auto-Retest arm
    //      (:753) and the OFF_LINE alarm arm (:758).  true would have pinned
    //      the tower light to "Homing" and masked all three.
    //    * ckernel.cpp:1073 `else if(fHome->fShow) { fMain->ShowNowStatus(...,
    //      "Homing"); CoolTime=0; }` false SKIPS the "Homing" status caption
    //      and the CoolTime reset, falling through to the ATC-self-test arm at
    //      :1078.  true would have re-zeroed CoolTime on every tick.
    //  LANDED AHEAD OF ITS CONSUMER, ON PURPOSE, AND SAID SO PLAINLY: both
    //  readers live in ShowRunLed / ShowRunLabel, which the W7-L2 recon
    //  DEFERRED, so nothing in this wave reads it.  It is landed anyway for the
    //  same reason forms/fNote.h gives for its own AlarmType: it is the only
    //  other fHome member golden ckernel.cpp touches, it is a plain bool with a
    //  real golden ctor value, and forms/ files are edited one-owner-at-a-time
    //  -- omitting it guarantees a second serialised edit to this same file.
    // -----------------------------------------------------------------------
    bool fShow;

    // -----------------------------------------------------------------------
    //  [DATA] golden uhome.h:71 `bool fAbort;` -- "the operator pressed Abort
    //  Home"; golden sets it true in sbAbortHomeClick (uhome.cpp:4983
    //  `fAbort=true;`) and clears it at every home (re)start.
    //  OFFLINE VALUE false, GOLDEN'S OWN CTOR VALUE (uhome.cpp:111
    //  `fAbort =false;`), and also the value golden's own reset path writes
    //  (uhome.cpp:4848).
    //  WHICH BRANCH DOES IT SELECT?  Again honestly: none in ckernel.cpp --
    //  golden ckernel.cpp:372 only WRITES it.  Its readers are all inside
    //  uhome.cpp's home loop, which has no ported home.  false is still the
    //  only defensible value: it means "no abort pending", which is exactly the
    //  state ScanSystemSensor itself establishes at :372 before starting.
    // -----------------------------------------------------------------------
    bool fAbort;

    // -----------------------------------------------------------------------
    //  AI(W906-HOME-C1) 20260920: 歸零狀態機真正的資料結構
    //
    //  [DATA] golden uhome.h:79 `vector<THomeClass *>HomeClass;`
    //
    //  為什麼現在才落地：P0-4 量到（見 docs/PLAN_START_TO_RUN.md）歸零真正
    //  擋在 `CheckMotorHome()`（csystem.cpp:328），它要求 TOTAL_MOTOR 顆
    //  馬達全部 `HomeFlag==1`；而寫 `HomeFlag` 的 `TMyMotor::MotorHome()`
    //  （Motor/mymotor.cpp:1105/1132/1160）**早就翻好而且是活的**，只是
    //  沒有人呼叫它。呼叫它的是 golden `ProcessMotorHome` case 600 的迴圈，
    //  而那個迴圈的條件就是這個 vector。
    //
    //  ⚠ golden 自己的契約：case 600（uhome.cpp:3030）寫的是
    //      if(fHome->HomeClass[i]->THomeFlag && ...)   // i 來自 MOT[] 的迴圈
    //      ret=MOT[i].MotorHome(flag1);
    //    也就是**假設 `HomeClass[i]->index == i`**。這不是本移植加的假設，
    //    是 golden 原文的形狀。`InitialHomeClass()` 尾端那段
    //      for(int i=HomeClass.size(); i<TOTAL_MOTOR; i++)
    //    （golden :343-347，註解寫「修正歸零例外」）就是在保證
    //    `size() >= TOTAL_MOTOR`，讓那個索引不會越界。
    //    ⇒ tests/test_homeclass.cpp 把這個契約變成會當掉的斷言。
    // -----------------------------------------------------------------------
    std::vector<THomeClass *> HomeClass;

    // -----------------------------------------------------------------------
    //  AI(W906-HOME-C2) 20260920: 歸零的逐行訊息清單
    //
    //  [DATA] golden uhome.h 的 `TListBox *ListBox1;`（.dfm 元件）。
    //  golden `ProcessMotorHome()` 在 35 處寫它，全部都是
    //      sprintf(str, "M%02d home finish.", i+1);
    //      fHome->ListBox1->Items->Insert(0, str);
    //  這類「第 N 顆馬達歸好了」的進度行。
    //
    //  **落地而不是閘掉的理由**：`vclcompat::TListBox`（vclcompat/Controls.h:397）
    //  本來就在，而且它的 `Items` 是真的 `TStringList`，`Insert(int, AnsiString)`
    //  是真的會存進去（TStringList.h:201，0-based，與 VCL 同）。
    //  ⇒ 閘掉 35 行只會換來 35 個「這裡少一行字」的註解；落地則讓 golden 原文
    //    逐字成立，而且那些字串**可以被測試觀察**。
    //
    //  ⚠ 它不會出現在任何畫面上 —— 本樹沒有視窗。存的是值，不是版面。
    //    這與 `vclcompat/Controls.h` 對 Left/Top 的既有說法一致。
    //
    //  ⚠ 20260919 使用者裁決過「回 HOME 顯示畫面是為了讓使用者知道現在軸回
    //    home 進度」。這個清單正是那個進度的資料來源；把它存起來，
    //    web HMI 之後要顯示歸零進度時就有東西可取。
    // -----------------------------------------------------------------------
    vclcompat::TListBox *ListBox1;

    // -----------------------------------------------------------------------
    //  AI(W906-HOME-C2) 20260920: ★ `Show()` / `Close()` 是**承重的**，不是版面
    //
    //  golden 的歸零狀態機有 **16 處**問 `fHome->fShow`，其中 case 600
    //  （uhome.cpp:3021，真正驅動馬達的那個迴圈）第一行就是
    //      if(fHome->fShow==false) { SoftStop=true; break; }
    //  ⇒ `fShow` 是 false 的話，**整個歸零一步都跑不動**，而且會 SoftStop。
    //
    //  在 golden，`fShow` 由 VCL 事件維護：
    //      TfHome::FormShow  (uhome.cpp:4844)  Panel2->Visible=false; fShow=true;  fAbort=false;
    //      TfHome::FormClose (uhome.cpp:4868)  fShow=false; Timer1->Enabled=false;
    //  而事件由 `fHome->Show()`（uhome.cpp:2368，case 300）與 `Close()`
    //  （uhome.cpp:3548，case 1250）觸發。
    //
    //  本樹沒有 VCL 事件，所以把「Show 會造成什麼狀態改變」直接寫進方法裡。
    //  ⚠ 這不是自由發揮 —— 兩個方法的本體逐行對應 golden 那兩個事件處理器，
    //    只有沒有對應成員的那兩行（Panel2 / Timer1）閘住，見 uhome.cpp 的定義。
    //  ⚠ 若把它們寫成 no-op（「反正沒有視窗」），case 600 會在第一個 tick
    //    就 SoftStop —— 那正是「解 gate 前先查值從哪來」那條 memory 的反例。
    // -----------------------------------------------------------------------
    void Show();                        // golden TForm::Show -> FormShow (uhome.cpp:4844)
    void Close();                       // golden TForm::Close -> FormClose (uhome.cpp:4868)
    void RotateCheckClear();            // golden uhome.cpp:4851（清 SnRotateCheck，完整翻譯）

    void InitialHomeClass();            // golden uhome.h:82 / uhome.cpp:115
    void FormDestroy();                 // golden uhome.cpp:378（去掉 TObject* Sender）
    void ShowLed(int index, int attr);  // golden uhome.cpp:664
    void ResetAllMotorLed();            // golden uhome.cpp:682
    bool ShowMotorHomePos(int index);   // golden uhome.cpp:688

    // AI(W906-MT-E3b) 20260925: the motor-power-OFF path (golden uhome.h, bodies in uhome.cpp).
    //   Callers in golden: uMotorTest btnMotorPowerClick / FormShow, csystem CheckMotorPowerShutDown
    //   ([Power Off] key) and DoOneCycleFinishCheck, sbAbortHomeClick, aTester_Front/Rear,
    //   PowerSavingMode.  golden does NOT clear any HomeFlag here (lead default 20260925: kept).
    void InitGali_HomeTask();  void InitDoTestZHome();   // golden uhome.h:76, body uhome.cpp:646-662  //AI(W906-FLOW-2) 20260928: InitDoTestZHome = golden uhome.h:74, body uhome.cpp:4891-4906 translated at the end of AutoClean/AutoClean.cpp
    void GaliMotorServoOff(vclcompat::AnsiString sFunc); // golden uhome.h:77, body uhome.cpp:4988-5016  //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
    // AI(W906-MT-E3b) 20260925: golden `void __fastcall sbAbortHomeClick(TObject *Sender)` (body
    //   uhome.cpp:4980-4986) -- the Abort Home button: GaliMotorServoOff + fAbort=true + Close().
    //   Sender is `void *` (the forms/fMain.h convention, e.g. BtnPauseClick): golden's only engine
    //   caller passes the form itself (`fHome->sbAbortHomeClick(fHome)`, csystem.cpp MainProc's stop
    //   arm) and TfHome is not a TObject here.  Sender is unused, as in golden.
    void sbAbortHomeClick(void *Sender);

    TfHome();
    virtual ~TfHome() {}
};

// golden: extern PACKAGE TfHome *fHome;   (uhome.h:85)
extern TfHome *fHome;  extern int W906_HomeLedState[];   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): ShowLed's attr per HomeClass slot (0 off / 1 lime / 2 red / 3 yellow, golden uhome.cpp:664-680), read by the native HW.home window (ui/native/NativeFormsWbServe.cpp HomeSnapshot); defined in forms/fHome.cpp:505

#endif // FORMS_FHOME_H
