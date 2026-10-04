// =============================================================================
//  BinDisplay/MyBinDisp.h  --  TMyBinDispCtrl: the Bin-display controller's
//                               pure-DATA-LAYER base class (getters/setters/
//                               flags/queues), plus a hardware-free offline
//                               stand-in child so the base class is
//                               instantiable.
//
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618\BinDisplay\MyBinDisp.h
//              (196 lines, cp950 -- 0 U+FFFD on decode, verified this wave:
//              `python3 -c "open('MyBinDisp.h','rb').read().decode('cp950')"`)
//  Golden .cpp: HT9011UC_Code_V3.33.906.0_20260618\BinDisplay\MyBinDisp.cpp
//               (3169 lines, cp950 -- 0 U+FFFD, same command)
//  Recon this wave is built from: docs/RECON_BinDisCtrl.md (2026-08-20).
//  Translator: AI(W906-BinDisp-WA) 20260820
//
//  =========================== WAVE SCOPE ===================================
//  This wave translates ONLY the data-layer subset of TMyBinDispCtrl the
//  recon identified as covering every one of the 17 known golden production
//  call sites (cShowBinSelect.cpp/cBinSel.cpp/cSortCT.cpp/csystem.cpp/
//  acatchtray.cpp/Motor/myMN200motor.cpp/database.cpp/main.cpp -- see recon
//  S2 table). In this HEADER that is:
//    ctor/dtor, the 10 one-line getters/setters (golden :145-154), GetRunStatus
//    (:156-170), SetComPort/SetComPort2/SetComParity (:172-180), ProcessStopStart
//    (:182-195), CommBinReceiveData/CommBinReceiveData2 (:199-269),
//    InstalledUnit (:273-280), WriteTargetBin/WriteTargetCount (:606-646),
//    ShowCommLog (:701-720 -- see NOTE A), StartFlash/IsAnyFlashing/
//    ProcessFlash/ClearAutoChangingWarn/FlashPro (:3078-3168), plus every
//    protected field those methods touch (golden :34-159 minus what NOTE B
//    below excludes).
//
//  NOTE A -- ShowCommLog was not in the recon's own line-item list (which
//  enumerated CommBinReceiveData as ":199-269, does not call CommBin, can be
//  translated as-is") but CommBinReceiveData's OWN body (golden :248) calls
//  it, and ShowCommLog (golden :701-720) is itself pure data/logging (no
//  CommBin, no hardware) -- so it is a REQUIRED transitive dependency of an
//  in-scope method, not a scope expansion. Confirmed no other in-scope method
//  needs `command_TFT_Input`/`command_TFT_Font` (golden :727+): every call
//  site of those two is inside the WriteBin_TFT/SetFontXxx_TFT hardware-
//  protocol family (golden :890-1681), all out of this wave's scope (see
//  GATE (1) below) -- verified by grep over the cp950-decoded golden .cpp,
//  2026-08-20.
//
//  NOTE B -- HEADER MEMBERS DELIBERATELY OMITTED (not merely deferred bodies,
//  actually absent from this header, unlike GATE (1)/(2) which keep the
//  declaration and gate only the body):
//    * `TDataModule3` (golden :21-32) and `extern PACKAGE TDataModule3
//      *DataModule3;` -- recon S1 established this is a DESIGN-TIME VCL
//      datamodule the standalone BinDispTester.bpr harness owns; production
//      code never touches it (`HSys.BinDisCtrl`'s concrete instance owns its
//      own CommBin/CommBin2 members, golden :123-124). Its ONLY appearance in
//      anything this wave touches is inside Timer1Timer's case-1 hardware-
//      bring-up branch (golden :332,336), which is GATEd (see GATE (1)) --
//      so nothing in this header needs it. Omitting it means this header
//      does not need a `TDataModule` VCL base class this tree has no port of.
//    * `command_TFT_Input`/`command_TFT_Font` (golden :146-147, public) --
//      see NOTE A; both are unused unless/until the WriteBin_TFT hardware-
//      protocol family lands (GATE (2)).
//
//  =========================== GATE REGISTER ================================
//  GATE (1) -- Timer1Timer (golden :284-604, ~320 lines) is DECLARED here
//    (protected, matches golden :46) but its .cpp body is an intentional
//    empty stub, not a translation. Rationale: golden's Timer1Timer is the
//    hardware-bring-up STATE MACHINE that (a) reaches TDataModule3 (NOTE B),
//    (b) dispatches into the pure-virtual hardware-protocol hooks
//    (DoStartSetBin/DoStartSetColor/DoStartGetStatus/DoOnce/DoOnceTFT/DoCycle/
//    DoCycleTFT/InitialTask) that this wave's offline child class answers
//    with golden's OWN base-class defaults (`{return false;}`) or a no-op
//    (InitialTask), and (c) opens/polls real COM ports via GetCOMPortStatus
//    (not ported). The recon's own line-item budget (~350-360 body lines)
//    excludes these ~320 lines outright -- translating them faithfully would
//    ~double the wave and pull in the very hardware-protocol surface (recon
//    S3) that is explicitly parked for a future "UI/hardware wave" pending a
//    physical Bin display to validate against. ProcessStopStart's ctor-time
//    behaviour (bFirstInit branch calling InitialTask()) is NOT affected by
//    this gate -- that call is real and reaches the offline child's no-op
//    override, exactly per recon S2's own accounting ("InitialTask() have
//    something to call").
//    ctor still does `Timer1->OnTimer=Timer1Timer;` (golden :93) faithfully:
//    the assignment compiles and stores the callback (TU-local ht9045_bindisp
//    ::TTimer's OnTimer is a real std::function slot), it is simply never
//    invoked by anything in this build -- no message pump ticks it, matching
//    every other TU-local TTimer stand-in already in this tree (see
//    PowerSavingMode.h's [H1] note: "nothing in this build pumps a Windows
//    message queue ... assigning ->OnTimer stores the callback and NOTHING
//    EVER CALLS IT").
//  GATE (2) -- The entire TMyBinDispHT9046 hardware-protocol override family
//    (golden :657-3077, ~2400 lines: WriteBin/WriteBin2/WriteColor/
//    WriteColor2/ReadVersion/ReadVersion2/ReadVersion_TFT/
//    MagazineWriteBin_HTA18/_BT008/_TFT/MagazineWriteBinFont_TFT/
//    SetNoBackGround_TFT/SetBackGround_TFT/SetFontBin_TFT/SetFontBinWord_TFT/
//    SetFontEA_TFT/SetFontCount_TFT/WriteBinWord_TFT/WriteEA_TFT/
//    WriteCount_TFT/WriteBin_TFT/DoOnce/DoOnceTFT/DoCycle/DoCycleTFT/
//    InitialTask[REAL body]/command_TFT_Input/command_TFT_Font) is NOT
//    translated -- per recon S3, zero known production call sites need it,
//    and there is no physical Bin display on the experiment rig to validate
//    a byte-level RS-232/TFT protocol against. See TMyBinDispOffline below
//    for the no-op stand-in that satisfies C++'s "cannot instantiate an
//    abstract class" rule instead.
//  GATE (3) -- `Spcomm.pas`/`TComm`'s real hardware bring-up (StartComm
//    against a physical COM port) is reachable only through GATE (1)'s
//    stubbed Timer1Timer, so it is dormant in this wave regardless of
//    vclcompat/Comm.h's own real capability (see RECON CORRECTION below).
//
//  ======================= RECON CORRECTION (2026-08-20) =====================
//  docs/RECON_BinDisCtrl.md S1/S3 states (04:05 grep, excluding build dirs)
//  that this tree "has never touched Spcomm.pas/spcomm.hpp" and that a
//  "minimal TComm facade (StopComm() only)" would need to be written fresh
//  for this wave. THAT IS WRONG: `vclcompat/Comm.h` + `vclcompat/Comm.cpp`
//  is a COMPLETE, SIM-capable `Spcomm::TComm` (StartComm/StopComm/
//  WriteCommData/CommName/BaudRate/Parity/ByteSize/StopBits/
//  ReadIntervalTimeout/OnReceiveData, all real, with an automatic
//  simulation-mode fallback when no physical port can be opened -- see that
//  header's own banner), already wired into CMakeLists.txt (ht9045_sm target,
//  line ~153) and already consumed by 4+ translated files (OmronLaser/
//  LaserSensor.h/.cpp among them) since git commit 199272d, 2026-06-26 --
//  eight weeks before this recon's "no implementation" grep. The recon's
//  grep command (`grep -rn "TComm\b" ... `) should have matched
//  `vclcompat/Comm.h:126 class TComm : public TComponent` and did not; this
//  translation does not repeat that command and instead verifies directly
//  (`git log -- vclcompat/Comm.h`, `grep -rn "class TComm" vclcompat/`,
//  both re-run 2026-08-20 immediately before writing this file). CONSEQUENCE:
//  step 4 of the recon's own translation-path proposal ("TComm 的最小
//  facade... 20-30 行") is UNNECESSARY -- `CommBin`/`CommBin2` below are typed
//  `Spcomm::TComm*` directly, no new facade class is added by this wave, and
//  no shared header is touched (satisfies the task's own "勿動共用頭檔"
//  instruction by there being nothing left to add to it).
//
//  =========================== DEVIATION / GOLDEN ODDITY =====================
//  (a) FlashPro/ClearAutoChangingWarn (golden :3129-3168) are MEMBER methods
//      of TMyBinDispCtrl that reach through the GLOBAL singleton
//      `HSys.BinDisCtrl->...` instead of `this->...` to call GetColorNow/
//      StartFlash on themselves. Since `HSys.BinDisCtrl` IS `this` whenever
//      these methods run on the live production singleton, this is
//      behaviourally a self-call via an unnecessary global indirection --
//      but it is golden's own code (not a translation artifact) and is kept
//      VERBATIM, not "fixed" to `this->GetColorNow(...)`/`this->
//      StartFlash(...)`. See MyBinDisp.cpp's matching comment at the
//      translated call sites.
//  (b) golden's ctor (:39-129) leaves several scalar/array fields untouched,
//      relying on BCB6's TObject::NewInstance zero-filling the raw instance
//      memory BEFORE the constructor body runs (the same VCL semantic
//      Public/MyStringList.h's own `ZeroInitVclFields()` helper already
//      documents and reproduces for a different class in this tree). This
//      port adds an equivalent private `ZeroInitVclFields()` helper (NO
//      golden counterpart -- see its doc comment on this class) called as
//      the FIRST statement of the translated ctor, covering every field
//      golden's ctor body never assigns: bSetColor[]/bGetStatus[]/
//      iCountTFT[]/bFlashCom/BinDispRecv2/SendBuffer/BinDispCom2Buffer/
//      BinDispCom2Buffer2/Com2ReceiveByte/Receivelen/iOnceTask family/Addr/
//      MagAddr/CommBin/CommBin2. Without it, this port's plain-C++ base
//      would leave those raw members at INDETERMINATE values instead of
//      golden's actual zero/false/NULL -- a genuine fidelity gap, not a
//      style choice.
//  (c) `ComPort` defaults to the STRING "4" (golden :86 `ComPort=4;`, using
//      AnsiString's int-formatting assignment -- ported AnsiString supports
//      the identical `operator=(int)` per vclcompat/AnsiString.h:85, verified
//      this wave), while `ComPort2` is never assigned in the ctor at all and
//      so defaults to "" (AnsiString's own default constructor, not the
//      zero-fill helper -- class-typed members are not raw/POD, so C++
//      already default-constructs them regardless of (b) above). This
//      asymmetry (ComPort seeded, ComPort2 not) is golden's own and is kept.
//
//  =========================== VCL / TOOLCHAIN ADAPTATIONS ===================
//  * `<Classes.hpp>`/`"SPComm.hpp"` (BCB6-only) -> `vclcompat/vcl_compat.h`,
//    which already transitively provides AnsiString/TObject/Word AND
//    `vclcompat/Comm.h`'s `Spcomm::TComm`/`Spcomm::TParity`/`Spcomm::Word`
//    (see RECON CORRECTION above). Types are spelled qualified
//    (`Spcomm::TComm`, `Spcomm::TParity`, `Spcomm::Word`) rather than relying
//    on Comm.h's own `using namespace Spcomm;`, matching the established
//    precedent in OmronLaser/LaserSensor.h (the tree's other TComm consumer)
//    -- bare enum VALUES (`None`) are still used unqualified where golden
//    does, also matching that same file's precedent
//    (`CommLaserOutArm->Parity=None;`, LaserSensor.cpp:938).
//  * `TTimer` (golden :41, Timer1) has no tree-wide port (verified again this
//    wave: `grep -rn "class TTimer" vclcompat/` -- 0 hits, same result
//    forms/fShowBinSelect.h's own banner already recorded 2026-08-18). This
//    tree's established answer is a per-file guarded TU-local stand-in with
//    exactly the member set the consumer touches (Interval/Enabled/OnTimer
//    as a std::function, matching Automation/uRENESAS_Server.h's [G3] and
//    PowerSavingMode.h's [H1] -- NOT the bare-`Enabled`-only shape used by
//    ATCInterface.h/LaserSensor.h, since this ctor also writes ->Interval and
//    ->OnTimer). Following PowerSavingMode.h's [H1] "TRAP 5" precedent
//    exactly (two headers, one global class name `TTimer`, different member
//    layouts, is an ODR/sizeof landmine the moment both land in one TU), this
//    stand-in lives in its OWN namespace `ht9045_bindisp` rather than at
//    global scope -- zero ODR surface with ATCInterface.h/LaserSensor.h/
//    uRENESAS_Server.h/PowerSavingMode.h's own TTimers, by construction.
//  * `__fastcall`/`__closure`/`__published` dropped throughout (golden marks
//    every method golden itself marks `__fastcall`); VCL properties become
//    plain public/protected data members, matching this tree's blanket
//    convention.
//  * `WORD`/`Pointer` (golden CommBinReceiveData(2) params, :199-200/:252-253)
//    -> `Spcomm::Word`/`void*`, matching OmronLaser/LaserSensor.h's identical
//    OnReceiveData-handler signature precedent
//    (`void CommLaser1ReceiveData(vclcompat::TObject*, void*, Spcomm::Word)`).
//  * `TMyStringList` (slBinDispLog) is only forward-declared here (matches
//    cmydef.h's own tree-wide forward-decl -- the complete type is needed
//    only where `new`/methods are called, i.e. the .cpp, matching
//    handlerlog.cpp's established `#include "Public/MyStringList.h"`
//    precedent).
//
//  Big5/UTF-8: golden is cp950 with Chinese comments; this translation is
//  UTF-8, comments carried over in Traditional Chinese where they annotate
//  behaviour (matching this tree's per-file convention of preserving golden
//  comments' language rather than translating them to English).
// =============================================================================
#ifndef MyBinDispH
#define MyBinDispH

#include "vclcompat/vcl_compat.h"   // AnsiString/TObject/Word + Spcomm::TComm/TParity (vclcompat/Comm.h, transitively included)
#include "MachineType.h"            // eBinDispName (eBinDispLoader..eBinDispTotal) / MAX_BIN_UNIT / TEST_MAX_BIN
#include "myTimer.h"                // TQPF_Timer (BinDisDelay/BinDisCycleDelay/FlashTimer[])
#include <functional>               // std::function -- ht9045_bindisp::TTimer's OnTimer slot

// golden cmydef.h-style forward decl (cmydef.h:15 `class TMyStringList;`) --
// only a pointer is needed in THIS header; the complete type is required only
// where slBinDispLog is constructed/called (BinDisplay/MyBinDisp.cpp), which
// pulls in Public/MyStringList.h directly (see that .cpp's own banner).
class TMyStringList;

// ---------------------------------------------------------------------------
// [H1] TTimer stand-in -- see "VCL / TOOLCHAIN ADAPTATIONS" above and
// PowerSavingMode.h's [H1] "TRAP 5" note for why this MUST be namespaced
// rather than declared at global scope like ATCInterface.h/LaserSensor.h's
// (member-incompatible) global `class TTimer`. Member set matches golden's
// touched surface exactly: ->Interval (:94), ->Enabled (:96, dtor :137),
// ->OnTimer (:93), constructed with an owner argument (:92, golden passes
// NULL).
// ---------------------------------------------------------------------------
namespace ht9045_bindisp {
class TTimer
{
public:
    int  Interval;
    bool Enabled;
    std::function<void(TObject* Sender)> OnTimer;
    explicit TTimer(void* /*Owner*/) : Interval(0), Enabled(false) {}
};
} // namespace ht9045_bindisp

// golden MyBinDisp.h:11-19 (global scope in golden, kept global here).
enum eBinDispStatus                                                             //Sam 20240604 : 新增 BinDisplay TFT
{
    eBDP_Initial    =0,
    eBDP_GetStatus  =1,
    eBDP_ColorSet   =2,
    eBDP_BinSet     =3,
    eBDP_BinRun     =4,
    eBDP_DispErr    =5
};

// ---------------------------------------------------------------------------
//  TMyBinDispCtrl  (golden :34-159)
//  Pure data-layer base class: getters/setters/queues/flags only. The four
//  hardware-protocol pure virtuals (WriteBin/WriteColor/WriteBin2/
//  WriteColor2 + ReadVersion/ReadVersion2 + InitialTask) exist so a concrete
//  subclass can plug real hardware in later (GATE (2)); TMyBinDispOffline
//  below answers them all with no-ops so THIS class is instantiable today.
// ---------------------------------------------------------------------------
class TMyBinDispCtrl
{
private:
    // AI(W906-BinDisp-WA) 20260820: NO golden counterpart. Reproduces BCB6's
    // TObject::NewInstance zero-fill-before-ctor semantic for every raw
    // scalar/array field golden's own ctor body (MyBinDisp.cpp :39-129) never
    // assigns. See this header's own banner, DEVIATION (b), for the full
    // field list and rationale. Same idiom as Public/MyStringList.h's
    // ZeroInitVclFields()/BindProperties() pair.
    void ZeroInitVclFields();

protected:  // User declarations
    int    Addr;
    int    MagAddr;                                                             //JerryYang 20230515 : Magazine BIN DISP
    ht9045_bindisp::TTimer *Timer1;

    TQPF_Timer BinDisDelay;
    TQPF_Timer BinDisCycleDelay;

    void Timer1Timer(TObject *Sender);                                          // GATE (1) -- see banner; body is an intentional empty stub
    void CommBinReceiveData(TObject *Sender, void *Buffer, Spcomm::Word BufferLength);
    void CommBinReceiveData2(TObject *Sender, void *Buffer, Spcomm::Word BufferLength);
    bool  bHasUnitArray[MAX_BIN_UNIT];                                          // 個別的顯示器是否有安裝
    bool  bHasUnit;                                                             // 如果都沒有裝顯示器就會=false
    bool  bSliding[MAX_BIN_UNIT];                                               // 判斷該位置是否需要跳號
    bool  bSliding_Cnt[MAX_BIN_UNIT];                                           //Sam 20240604 : 新增 BinDisplay TFT
    bool  bStopProcess;                                                         // 是否要停止Timer
    bool  bSetBin[MAX_BIN_UNIT];                                                // 那些顯示器要開始設定Bin
    int   iSetBin[MAX_BIN_UNIT][TEST_MAX_BIN];                                  // 顯示器要設定的Bin            //Steven 20140402 : Fixed 記憶體破壞
    bool  bSetColor[MAX_BIN_UNIT];                                              // 那些顯示器要開始設定Bin
    bool  bGetStatus[MAX_BIN_UNIT];
    int   iSetColor[MAX_BIN_UNIT];                                              // 顯示器要設定的Color
    bool  bStartSetBin;                                                         // 開始進行設定顯示器Bin
    bool  bStartSetColor;                                                       // 開始進行設定顯示器Color
    double dDelaySec;                                                           // Bin輪替的時間    //Sam 20240604 : 顯示器輪巡時間改為 double
    AnsiString sUnitName[MAX_BIN_UNIT];                                         // 紀錄 Log 使用    //Sam 20240604 : 新增 BinDisplay TFT
    int iSetCount[MAX_BIN_UNIT];                                                // 顯示 IC Count 使用
    int iCountTFT[MAX_BIN_UNIT];                                                // Bin輪巡使用
    bool bStartOnce;                                                            // 開始進行設定TFT顯示器設定
    bool bStartCycle;                                                           // 開始進行 TFT 顯示器
    int   iVersion[MAX_BIN_UNIT];                                               // 顯示器的版本
    int   iBinNow[MAX_BIN_UNIT];                                                // 顯示器目前的Bin
    int   iColorNow[MAX_BIN_UNIT];                                              // 顯示器目前的的Color
    bool  bHasError[MAX_BIN_UNIT];                                              // 顯示器是否有出錯
    int   iRusStatus;                                                           // 顯示目前在執行的動作

    bool       bFlash[MAX_BIN_UNIT];                                            //Eastsun 20260513 : 閃爍功能
    int        iFlashColorA[MAX_BIN_UNIT];                                      //Eastsun 20260513 : 閃爍功能
    int        iFlashColorB[MAX_BIN_UNIT];                                      //Eastsun 20260513 : 閃爍功能
    int        iFlashPeriodMs[MAX_BIN_UNIT];                                    //Eastsun 20260513 : 閃爍功能
    bool       bFlashShowA[MAX_BIN_UNIT];                                       //Eastsun 20260513 : 閃爍功能
    TQPF_Timer FlashTimer[MAX_BIN_UNIT];                                        //Eastsun 20260513 : 閃爍功能
    void ProcessFlash(void);                                                    //Eastsun 20260513 : 閃爍功能
    bool bFlashCom;                                                             //Eastsun 20260513 : 閃爍功能

    char SendBuffer[1024];                                                      // 欲對顯示器通訊之資料
    bool BinDispRecv;                                                           // 有從顯示器通訊之 Comm Port收到一筆資料
    bool BinDispRecv2;                                                          // 有從顯示器通訊之 Comm Port收到一筆資料
    char BinDispCom2Buffer[1024];                                               // 有從顯示器通訊之 Comm Port收到一筆資料之內容
    char BinDispCom2Buffer2[1024];                                              // 有從顯示器通訊之 Comm Port收到一筆資料之內容
    AnsiString  ComPort;                                                        // 顯示器通訊之 Comm Port
    AnsiString  ComPort2;                                                       // 顯示器通訊之 Comm Port
    Spcomm::TParity ComParity;                                                  // 顯示器通訊之 Parity
    unsigned char Com2ReceiveByte[1024];
    int Receivelen;
    int iOnceTask;                                                              //Sam 20240604 : 新增 BinDisplay TFT
    int iOnceTFTTask;
    int iCycleTask;
    int iCycleTFTTask;
    int iStartSetBinTask;
    int iStartSetColorTask;
    int iStartGetStatusTask;
    int iBinDispCtrlTask;
    int iTotalInstalledUnit;

    //虛擬函式，需要在子類別實做
    virtual void WriteBin  (int Addr, int Command, short value)=0;              // 對address=Index顯示器送出欲設定之Bin
    virtual void WriteColor(int Addr, short value)=0;                           // 對address=Index顯示器送出欲設定之顏色
    virtual void WriteBin2  (int Addr, int Command, short value)=0;             // 對address=Index顯示器送出欲設定之Bin
    virtual void WriteColor2(int Addr, short value)=0;                          // 對address=Index顯示器送出欲設定之顏色

    virtual void ReadVersion(int Addr)=0;                                       // 讀取顯示器的狀態
    virtual void ReadVersion2(int Addr)=0;                                      // 讀取顯示器的狀態
    virtual bool DoStartSetBin()    {return false;};                            // 開始Bin的顯示
    virtual bool DoStartSetColor()  {return false;};                            // 開始設定顯示器顏色
    virtual bool DoStartGetStatus() {return false;};                            // 確認顯示器的狀態
    virtual void InitialTask()=0;                                               //Sam 20240604 : 新增 BinDisplay TFT
    virtual bool DoOnce()                               {return false;};
    virtual bool DoOnceTFT(bool bReset, int iSetType)   {return false;};
    virtual bool DoCycle()                              {return false;};
    virtual bool DoCycleTFT(bool bReset, int iWriteType){return false;};

public:
    TMyBinDispCtrl();
    virtual ~TMyBinDispCtrl();
    AnsiString Alias[MAX_BIN_UNIT];
    Spcomm::TComm  *CommBin;
    Spcomm::TComm  *CommBin2;
    void ProcessStopStart(bool Value)  ;                                        // 啟動或停止跳號
    void SetComPort(AnsiString port)   ;                                        // 設定顯示器群組是利用那一個COM PORT
    void SetComPort2(AnsiString port)  ;                                        // 設定顯示器群組是利用那一個COM PORT
    void SetComParity(Spcomm::TParity Parity)  ;                                // 設定顯示器群組是利用那一個Parity
    bool UnitHasInstall(int Index)     ;                                        // 確認該顯示器使是否有啟用
    void CloseUnit(int Index)          ;                                        // 關閉該顯示器
    void OpenUnit(int Index)           ;                                        // 開啟該顯示器
    void SetDelayTime(double Sec)      ;                                        // 設定多久輪替一次 //Sam 20240604 : 顯示器輪巡時間改為 double
    double  GetDelayTime()             ;                                        // 取得輪替的時間
    int  GetTotalInstalledUnit()       ;                                        // 取得總共安裝了幾個顯示器
    int  GetColorNow(int Index)        ;                                        // 顯示器目前的顏色
    int  GetBinNow(int Index)          ;                                        // 顯示器目前的字元
    bool GerErrNow(int Index)          ;                                        // 顯示器是否有出錯
    AnsiString GetRunStatus()          ;                                        // 回傳目前在執行的動作

    void InstalledUnit(int Index);                                              // 設定address=Index 的顯示器是有存在的
    void WriteTargetBin(int Index, int *bin, int color);                        // 將address=Index顯示器設定文字與顏色
    void WriteTargetCount(int Index, int iCount);                               // 設定 IC 數量 //Sam 20240604 : 新增 BinDisplay TFT
    bool InitialOK;                                                             //判斷主控端是否已經準備完成
    bool bFirstInit;
    void ShowCommLog(unsigned char ucLog[], int Address, AnsiString sFun);      //Sam 20240604 : 新增 BinDisplay TFT -- see NOTE A (required by CommBinReceiveData)
    TMyStringList *slBinDispLog;                                                //Steven 20210910 : Bin顯示器的log
    AnsiString sReadBuffer;                                                     //kevin 20170711 (Steven) fix define
    AnsiString sReadBuffer2;                                                    //kevin 20170711 (Steven) fix define
    AnsiString sReadBufferMag;
    //==> Eastsun 20260513 KYEC: AutoChanging flash public API
    void StartFlash(int Index, int colorA, int colorB, int periodMs);
    bool IsAnyFlashing(void);
    int iOldTimerInterval;
    void ClearAutoChangingWarn(int AutoTarget);
    void FlashPro(int AutoTarget);
    //<== Eastsun 20260513
};

// ---------------------------------------------------------------------------
//  TMyBinDispOffline  -- NOT a golden class. Recon-mandated no-op concrete
//  stand-in (recon S3 step 2, "~20-40 行") so TMyBinDispCtrl (abstract: 7
//  pure virtuals) is instantiable without any of the GATE (2) hardware
//  protocol. Deliberately named to NOT resemble golden's real
//  `TMyBinDispHT9046` (which this is not, and must never be mistaken for --
//  it implements ZERO of the real RS-232/TFT protocol). Every override is a
//  pure no-op; unused override parameters are left unnamed to stay
//  -Wextra-clean (this is new infrastructure, not a translation, so it is
//  held to this tree's "-Wall -Wextra clean" bar for non-translated code).
// ---------------------------------------------------------------------------
class TMyBinDispOffline : public TMyBinDispCtrl
{
protected:
    virtual void WriteBin  (int /*Addr*/, int /*Command*/, short /*value*/) {}
    virtual void WriteColor(int /*Addr*/, short /*value*/)                  {}
    virtual void WriteBin2  (int /*Addr*/, int /*Command*/, short /*value*/) {}
    virtual void WriteColor2(int /*Addr*/, short /*value*/)                  {}
    virtual void ReadVersion(int /*Addr*/)  {}
    virtual void ReadVersion2(int /*Addr*/) {}
    virtual void InitialTask() {}                                              // GATE (1)/(2): real body needs hardware bring-up; no-op offline
public:
    TMyBinDispOffline() {}
    virtual ~TMyBinDispOffline() {}
};

#endif

// =============================================================================
//  APPENDED SECTION -- TMyBinDispHT9046: the real hardware-protocol subclass.
//  Translator: AI(W906-FW-BINDISP3) 20260824
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618\BinDisplay\MyBinDisp.h
//              :160-194 (subclass declaration; cp950, 0 U+FFFD on decode,
//              re-verified this wave with the same command as the base banner).
//  Method bodies: this directory's MyBinDisp.cpp, appended section (golden
//              .cpp :648-3068 -- see that section's own GOLDEN LINE MAP).
//
//  WHY THIS SECTION SITS AFTER THE BASE GUARD'S #endif: this wave's edit
//  boundary for this file is APPEND-ONLY, so the subclass cannot be inserted
//  before the existing #endif. The section carries its own include guard
//  (MyBinDispHT9046_AppendH), so repeated inclusion stays safe: the first
//  include defines base+subclass, later includes skip both via their
//  respective guards; TMyBinDispCtrl is always complete at this point because
//  it is defined above in this same file.
//
//  ================= GATE-REGISTER UPDATES (supersede the base banner) ========
//  * GATE (2) above is LIFTED for the TMyBinDispHT9046 method family: all of
//    golden .cpp :657-3068 is now translated (28 subclass methods + the 2
//    hoisted command_TFT_* helpers + the golden TU-scope protocol state; see
//    the .cpp appended banner). The GATE (2) text above is kept as history
//    (this file is append-only); THIS note is the current state.
//  * GATE (1) (Timer1Timer stub) REMAINS. Timer1Timer is still the ONLY code
//    path that calls DoStartSetBin/DoStartSetColor/DoStartGetStatus/DoOnce/
//    DoCycle (verified over the cp950-decoded golden .cpp 2026-08-24: each of
//    those five is invoked only from Timer1Timer's dispatcher, golden
//    :284-604), so every hardware-touching body in the appended .cpp section
//    has ZERO runtime callers in this build. Combined with the instantiation
//    fact below, translating the COM-port writes verbatim carries zero
//    execution risk today.
//  * GATE (3) REMAINS and is now the load-bearing one: the only instantiation
//    in this tree stays `new TMyBinDispOffline` (database.cpp:225, user ruling
//    20260824 -- the experiment rig HAS a color-bin panel, but bring-up waits
//    for machine-side validation). NOTHING news TMyBinDispHT9046 yet; this
//    class is translated-and-parked for that bring-up wave.
//  * NOTE B above is PARTIALLY superseded: command_TFT_Input/command_TFT_Font
//    are now translated -- but as members of THIS subclass, not the base (see
//    DEVIATION (d) just below). TDataModule3 (incl. golden .cpp :21
//    `TDataModule3 *DataModule3;`, ctor :32-35, and DataModuleDestroy
//    :3070-3073) stays omitted: still design-time harness surface, still
//    unreferenced by anything ported.
//
//  =========================== DEVIATION (d) ==================================
//  command_TFT_Input / command_TFT_Font are declared PUBLIC members of
//  TMyBinDispCtrl in golden (MyBinDisp.h:146-147). The base class body above
//  is frozen by this wave's append-only boundary, so they are HOISTED onto
//  this subclass instead (public, same access as golden). Behaviourally
//  identical by construction: every golden call site of either function is
//  inside a TMyBinDispHT9046 method (verified 2026-08-24 by grep over the
//  cp950-decoded golden .cpp -- command_TFT_Input called at :894/:912/:930/
//  :950, command_TFT_Font at :995/:1028/:1059/:1091, zero call sites outside
//  golden :866-1094), and neither body touches any member field (both are
//  pure buffer formatters over their char* out-param plus the TU-scope
//  iAddArrayTFT table). A future non-append wave MAY move them back onto the
//  base verbatim.
//
//  ZeroInitVclFields note: golden's TMyBinDispHT9046 declares NO data members
//  (only virtuals, golden :161-194), so there is nothing for a subclass
//  ZeroInitVclFields to zero -- the base ctor's helper already covers the
//  whole object. Function-local `static` state inside the .cpp bodies is
//  zero-initialized by C++ static-storage rules, same as BCB6.
// =============================================================================
#ifndef MyBinDispHT9046_AppendH
#define MyBinDispHT9046_AppendH

// Borland RTL System.hpp `Byte` (= unsigned char), needed by command_TFT_Font's
// golden signature. Same file-local global-typedef precedent as
// Interface/InterfaceSYS.h:36 and MyPLC/MyPLC_IO_Modbus.h:32 (a TU that sees
// more than one of these is fine -- identical typedef redefinition is legal).
typedef unsigned char Byte;

//----------------------------------------------------------------------------
// golden MyBinDisp.h:161-194 -- member list and order verbatim; __fastcall
// dropped per this tree's blanket convention (see base banner).
//----------------------------------------------------------------------------
class TMyBinDispHT9046:public TMyBinDispCtrl
{
    private:
    protected:
        virtual void WriteBin(int Addr, int Command, short Value);
        virtual void WriteBin2(int Addr, int Command, short Value);
        virtual void MagazineWriteBin_HTA18(int index,int ivalue);              //JerryYang 20230515 : 二位數BIN DISP, HTA18
        virtual void MagazineWriteBin_BT008(int index,int ivalue);              //JerryYang 20230515 : 三位數BIN DISP, HT-BT008
        virtual void MagazineWriteBin_TFT(int index,int ivalue, bool bmulti=false);   //JerryYang 20231106 : TFT
        virtual void MagazineWriteBinFont_TFT(int index, int iColor, bool binitial=false, bool bmulti=false);   //JerryYang 20231106 : TFT
        virtual void WriteColor(int Addr, short Value);
        virtual void WriteColor2(int Addr, short Value);
        virtual void ReadVersion(int Addr);
        virtual void ReadVersion2(int Addr);
        virtual void ReadVersion_TFT(int index);
        virtual bool DoStartSetBin();
        virtual bool DoStartSetColor();
        virtual bool DoStartGetStatus();
        virtual void InitialTask();                                             //Sam 20240604 : 新增 BinDisplay TFT
        virtual void SetNoBackGround_TFT(int index);                            //清空背景
        virtual void SetBackGround_TFT(int index);                              //設定背景
        virtual void SetFontBin_TFT(int index, int iColor, int iValue);         //設定 Bin 別字型
        virtual void SetFontBinWord_TFT(int index, int iColor, int iValue);     //設定 "Bin" 字型
        virtual void SetFontEA_TFT(int index, int iColor, int iValue);          //設定 "EA" 字型
        virtual void SetFontCount_TFT(int index, int iColor, int iValue);       //設定 "Count" 字型
        virtual void WriteBinWord_TFT(int index,int ivalue);                    //顯示 "Bin"
        virtual void WriteEA_TFT(int index,int ivalue);                         //顯示 "EA"
        virtual void WriteCount_TFT(int index,int ivalue, int iCount);          //顯示 Count 數量
        virtual void WriteBin_TFT(int index,int ivalue);                        //顯示Bin別 By TFT
        virtual bool DoOnce();
        virtual bool DoOnceTFT(bool bReset, int iSetType);
        virtual bool DoCycle();
        virtual bool DoCycleTFT(bool bReset, int iWriteType);

    public:
        // DEVIATION (d) -- golden BASE-class public members (MyBinDisp.h:
        // 146-147), hoisted onto this subclass; see the section banner above.
        void command_TFT_Input(char *cStr, int index, int iDisplabel, AnsiString sValue);
        void command_TFT_Font (char *cStr, int index, int iDisplabel, Byte iXPos, Byte iYPos, Byte iWidth, Byte iHeigh, Byte iFontSize, Byte iFill, int iColor);
};
#endif
