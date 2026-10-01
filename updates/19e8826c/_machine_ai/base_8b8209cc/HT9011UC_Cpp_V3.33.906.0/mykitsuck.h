// =============================================================================
//  mykitsuck.h  --  TMySucker (vacuum-nozzle) + TMyKitSuck (nozzle-grid) --
//                    the FULL golden-shaped kit-suck classes.
//
//  Faithful translation of golden mykitsuck.h (397 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "kitsuck".
//
//  ============================================================================
//  RELATIONSHIP TO aHotPlateSubstrate.h's TMyKitSuck/TMySucker -- READ FIRST
//  ============================================================================
//  This tree ALREADY has a class named TMyKitSuck (and TMySucker) wired into
//  the live build: aHotPlateSubstrate.h/.cpp (W6.2, 20260626-onward), grown
//  wave-by-wave into a "MINIMAL mirror" that now covers most of the surface
//  600+ already-ported call sites (ainarm9045_*.cpp, aoutarm9045_*.cpp,
//  acatchtray.cpp, acarry.cpp, atester*.cpp, ...) actually dereference, with
//  the GLOBAL OBJECTS (InArmSuck, FLCarryKit, BLCarryKit, FRCarryKit,
//  BRCarryKit, OutArmSuck, OutArm2Suck, CatchTraySuck, TestSocket, FTestSuck,
//  BTestSuck) DEFINED in aHotPlateSubstrate.cpp and linked into ht9045_sm
//  today.
//
//  THIS FILE IS A DIFFERENT, PARALLEL THING: a byte-faithful mirror of
//  golden's FULL class shape (every member, every method -- 90 TMySucker/
//  TMyKitSuck methods + 4 free functions, golden mykitsuck.cpp's complete
//  94-function surface), written under golden's OWN names (TMySucker,
//  TMyKitSuck, InArmSuck, FLCarryKit, ...) because HARD RULE 3 requires
//  golden identifiers kept byte-identical and this wave's brief requires a
//  real body for every golden function.  That is UNAVOIDABLY the same
//  identifier space aHotPlateSubstrate.h already occupies.
//
//  CONSEQUENCE, STATED PLAINLY: this header/.cpp pair is NOT #included by
//  anything yet, and NOT added to CMakeLists.txt (out of scope for this wave
//  under HARD RULE 7 -- "DO NOT modify any pre-existing file... Integration
//  is main-loop work").  If it and aHotPlateSubstrate.h were ever #included
//  in the SAME translation unit, `class TMyKitSuck { ... }` would be defined
//  twice with two DIFFERENT member layouts -- a hard redefinition error --
//  and if both .cpp files were ever linked into the same binary, the ten
//  extern TMyKitSuck globals (InArmSuck, FLCarryKit, ...) would be defined
//  TWICE, a hard "multiple definition" link error.  Neither has happened:
//  today only aHotPlateSubstrate.cpp's globals are linked into ht9045_sm;
//  this file's InArmSuck/FLCarryKit/... below are orphan globals in an
//  unbuilt translation unit.
//
//  WHY THIS SHAPE IS STILL THE RIGHT DELIVERABLE FOR THIS WAVE: the wave
//  brief is "translate mykitsuck.cpp faithfully, note the relationship, do
//  not SILENTLY duplicate" -- it does not authorize deleting or rewriting
//  aHotPlateSubstrate.h (a pre-existing file used by 600+ TUs, rule-7
//  off-limits) nor does it authorize editing CMakeLists.txt to choose a
//  winner.  Reconciling the two -- either retiring aHotPlateSubstrate's
//  MINIMAL mirror in favour of this FULL one and repointing every consumer,
//  or hand-merging the handful of members aHotPlateSubstrate added that
//  golden's real class doesn't literally spell the same way (e.g. the
//  W906-W7-L2 OffDestroy-count observability seam, which has NO golden
//  counterpart at all) -- is exactly the kind of tree-wide, multi-file
//  surgery HARD RULE 7 reserves for "main-loop work", not a single-group
//  translation wave.  This file is therefore the CANDIDATE full replacement,
//  parked at golden's own path, ready for that future integration step.
//
//  A NOTE ON THE ONE COMPILE-TIME NON-CONFLICT: both headers `#define
//  _MAX_SUCK_ROW_ITEM 4` / `_MAX_SUCK_COL_ITEM 8` with IDENTICAL replacement
//  lists, so C++'s "identical re-#define is not an error" rule means that
//  ONE pair of macros, unlike the class/global names above, would NOT
//  actually break if both headers ended up in one TU.  Noted for whoever
//  does the integration -- it is not evidence the rest is equally safe.
//
//  WAVE SCOPE -- every declaration below mirrors golden mykitsuck.h 1:1
//  (member-for-member, method-for-method); bodies are in mykitsuck.cpp.
//
//  VCL/Borland conversions: #pragma package/hdrstop dropped (golden had
//  none in the header).  `_fastcall`/`__fastcall` ctor/dtor markers dropped
//  per HARD RULE 4 (this is a NEW compiled TU with no existing __fastcall
//  declaration anywhere else to stay in lock-step with, unlike e.g.
//  MyDBIProcess below, which keeps whatever cMyDB.h already declares).
//  `TALed` (golden elec\aled.pas custom LED control) is NOT taken from
//  vclcompat/LedCore.h's `LedCore` -- that class models the FULL aled.pas
//  property set (Blink/Interval/LEDStyle/timer) that nothing in this file
//  touches; golden TMyKitSuck::SetMyLed/SetItemData/MoveSuckData/
//  MoveSuckDataDiff dereference exactly FOUR TALed members (->Visible,
//  ->TrueColor, ->Value, ->Hint).  A MINIMAL local mirror of just those four
//  (same idiom aHotPlateSubstrate.h uses throughout: "only the touched
//  surface") is declared below, named TALed (golden's own name -- confirmed
//  unused as a global-scope class anywhere else in this tree by grep before
//  writing it, so this is a fresh declaration, not a collision).
//  `TList *pSuck` uses vclcompat::TList (vclcompat/TList.h) via a local
//  `using` -- vcl_compat.h's own umbrella note (vclcompat/vcl_compat.h
//  :234-259) explains why it does NOT bring vclcompat::TList into the
//  global namespace by default (aHotPlateSubstrate.h's OWN, different,
//  narrower `class TList` shim already squats that name) and explicitly
//  recommends "add your own local `using vclcompat::TList;` in a TU that
//  does NOT also include aHotPlateSubstrate.h" -- exactly this file's
//  situation.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#ifndef MyKitSuckH
#define MyKitSuckH

#include "vclcompat/vcl_compat.h"   // AnsiString (self-contained: usable without relying on include order)
#include "vclcompat/BtnPanelCore.h" // vclcompat::TColor + cl* colour constants (golden Graphics.hpp)
#include "vclcompat/TList.h"        // vclcompat::TList -- see banner note above (deliberately NOT global by default)
#include "cmydef.h"                 // NULL_IC/HAS_IC/... IC-state constants, TYPE_A/TYPE_B, TColor (global typedef), INDEX_SUCKER_TYPE; transitively: MachineType.h (eIOType), cprod.h (CosFunction), cpublic.h
#include "myTimer.h"                // TQPF_Timer
#include "Public/MyProductionRecord.h"  // TMyProductionRecord

// using vclcompat::TList;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）             // local alias -- see banner "RELATIONSHIP" note (aHotPlateSubstrate.h's OWN TList shim is NOT visible from this TU)
// using vclcompat::clBlack;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// using vclcompat::clGreen;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// using vclcompat::clRed;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// using vclcompat::clBlue;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// using vclcompat::clLime;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// using vclcompat::clWhite;  // AI(W906-A4-6) 20260924: 不再注入全域 —— 移到 mykitsuck.cpp:199（本標頭改由 186 支檔經 aHotPlateSubstrate.h 看到，全域 using 會跟 HTEditList.h／aHotPlateSubstrate.h 的 TList、uHGemEquipment.h／acatchtray_shims.h 的顏色常數撞名）
// golden VCL Graphics.hpp clAqua ($00FFFF00 = R0 G255 B255 in the 0x00BBGGRR
// layout) -- not in vclcompat/BtnPanelCore.h's small shared palette, so
// declared here, once, guarded (same guard idiom BtnPanelCore.h itself uses
// for its own shared TColor block).
#ifndef HT9045_MYKITSUCK_CLAQUA
#define HT9045_MYKITSUCK_CLAQUA
// const TColor clAqua = 0x00FFFF00;  // AI(W906-A4-6) 20260924: 移到 mykitsuck.cpp:200（static）—— 全域定義會跟 cObserver.cpp:179 的 clAqua 撞名
#endif

#define _MAX_SUCK_ROW_ITEM 4
#define _MAX_SUCK_COL_ITEM 8

// ---------------------------------------------------------------------------
//  TALed -- MINIMAL local mirror of golden's custom LED control (elec\aled.pas
//  TALed : TGraphicControl).  See banner above for why this is not
//  vclcompat::LedCore.  Exactly the four members TMyKitSuck's LED-touching
//  methods dereference.  Plain public fields (no property get/set) -- same
//  idiom as aHotPlateSubstrate.h's TfLedValue (`bool Value;` plain field).
// ---------------------------------------------------------------------------
class TALed
{
public:
    bool       Visible;    // golden TGraphicControl::Visible (read by SetMyLed/All_HasIC)
    bool       Value;      // golden TALed::Value (LED lit/unlit; read+written)
    TColor     TrueColor;  // golden TALed::TrueColor (written per-IC-state)
    AnsiString Hint;       // golden TGraphicControl::Hint (written: nozzle position label)

    TALed() : Visible(true), Value(false), TrueColor(vclcompat::clBlack) {}
};

class TMySucker
{
    private:
        int OnTask;
        int OffTask;
        int OnOff;
        int RetryCT;
        int rct;
        TQPF_Timer tSuckTimer;

        bool bSuckOK;                                                           //ChungHung 20110901 add
        bool bDestroyOK;                                                        //ChungHung 20110901 add

        TQPF_Timer  TOn;                                                        // 真空開啟
        TQPF_Timer  TOff;                                                       // 真空破壞
        TQPF_Timer  TOnDelay;
        TQPF_Timer  TOffDelay;

        TQPF_Timer  TAgainDestroyDelay;                                         //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
        int ract;                                                               //ChungHung 20130413 add 針對 阿凡達IC 無法Relase

        void DoOnIO(bool bOn);                                                  //Steven 20230721 : 統一真空開關控制, 使用Private是防止外部使用, 外部操作原本的function
        void DoOffIO(bool bOn);                                                 //Steven 20230721 : 統一破壞開關控制

    protected:
    public:
        TMySucker();
        ~TMySucker();
        DWORD VacuumOnTime;
        DWORD VacuumOffTime;
        int   VacuumOnTimeBuffer[20];
        int   VacuumOffTimeBuffer[20];
        DWORD VacuumOnTimeRecord;
        DWORD VacuumOffTimeRecord;

        AnsiString SensorName;
        AnsiString OnPortName;
        AnsiString OffPortName;
        AnsiString SuckerName;
        bool fAlarm;
        bool Error;
        bool Enable;
        bool Status;
        bool AlarmEnable;
        int  OnAlarmTime;
        int  OffAlarmTime;
        int  OnDelayTime;
        int  OffDelayTime;

        AnsiString OnUsing;
        int  OnRing;
        int  OnIP;
        int  OnPort;                                                            // 真空開啟
        int  OnBit;
        int  OnType;                                                            // A or B Type
        int  OnISABase;
        bool OnEnable;

        AnsiString OffUsing;
        int  OffRing;
        int  OffIP;
        int  OffPort;                                                           // 真空破壞
        int  OffBit;
        int  OffType;                                                           // A or B Type
        int  OffISABase;
        bool OffEnable;

        AnsiString SenUsing;
        int  SenRing;
        int  SenIP;
        int  SenPort;
        int  SenBit;
        int  SenType;
        int  SenISABase;

        int  iManualOffTask;

        bool Suck();                                                            //真空電磁閥 ON  破壞電磁閥 OFF
        bool Destroy();                                                         //真空電磁閥 OFF 破壞電磁閥 ON
        void On();                                                              //真空電磁閥 ON  破壞電磁閥 OFF
        void Off();                                                             //真空電磁閥 OFF 破壞電磁閥 ON
        void OnSuck();                                                          //真空電磁閥 ON
        bool OffSuck();                                                         //真空電磁閥 OFF
        void OnDestroy();                                                       //破壞電磁閥 ON
        void OffDestroy();                                                      //破壞電磁閥 OFF
        void Normal();                                                          //恢復正常

        void Reset();
        void ReStart();
        bool GetStatus();
        bool Sensor();
        bool GetOnBit();
        bool GetOffBit();

        double GetIOValue();                                                    //Sam 20230210 : 新增 VacuumUnit 通訊模組
        double GetIOValueThread();
        bool SetIOValueThread(double dKpa);
        bool FinshFullMotion();
        void ResetSuckTask();
        void SetRetryCount(int);
        void CheckIsFallDown();
        bool bIsRotate;
        void PushOnTime();
        void PushOffTime();
        bool RealTimeRefreshVacuumOnOffTime;
        bool bNeedSuck;
        bool bNeedDestroy;
        void SetNeedSuck(bool Value)        {bNeedSuck=Value;}
        void SetNeedDestroy(bool Value)     {bNeedDestroy=Value;}
        bool GetNeedSuckStatus()            {return bNeedSuck;}
        bool GetNeedDestroyStatus()         {return bNeedDestroy;}
        void InitSuckFlag()                 {SetNeedSuck(false);SetNeedDestroy(false);}

        bool IsSuckFinish();
        bool IsDestroyFinish();

        int  DestroyAgainCount;                                                 //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
        double DestroyAgainTime;                                                //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
        int  iNozzleEvent;                                                      //Klutter 20210817 方便修正吸嘴回home異常事件       //Steven 20210825 : 吹氣完成才可以歸零
        int  ISABase;                                                           //Nickliu 20230306 add IO Use ISABase

        AnsiString sRecordStartOffDelayTime;                                    //Sam 20220329 : Record Index Air On Time
        AnsiString sRecordEndOffDelayTime;
        int  iRecordOffDelayTime;
        int  iMotNo;                                                            //Steven for HT1032
        int  iMyRow;                                                            //物理Row
        int  iMyCol;                                                            //物理Col
        AnsiString sName;                                                       //Alarm用, 顯示吸嘴的位置
        double GetVaccum();  int W906_GetOffDestroyCount() const; void W906_ResetOffDestroyCount(); void W906_OffDestroySeamHit(); unsigned long W906_OffDestroyStamp; int W906_OffDestroyRaw;  // AI(W906-A4-6) 20260924: 移植樹獨有的觀測接縫（原在 aHotPlateSubstrate.h 精簡鏡像）—— 每個吸嘴 OffDestroy() 的呼叫次數，epoch 式 O(1) 重設；golden StopAllDestroy 只呼叫 OffDestroy，沒有這個計數就無法驗證它碰到哪幾個吸嘴。只計數、不影響 IO  （原行尾註解：Sam 20230210 : 新增 VacuumUnit 通訊模組）
};

class TMyKitSuck{
    private:
    protected:
    public:
    TALed *pLed[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    bool   bLed[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];

    TMyKitSuck();
    ~TMyKitSuck();

    TMySucker Suck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iMotRow;
    int  iMotCol;
    int  iMaxRow;
    int  iMaxCol;
    int  iMaxCnt;
    int  iPickRow;
    int  iPickCol;
    int  iShtRow;
    int  iShtCol;
    int  iShtCnt;
    int  iPickStep;                                                             //Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3
    int  iPickKitStep;
    int  iShtKitStep;
    int  iXStep;                                                                //X pitch要設幾格
    int  iYStep;                                                                //Y pitch要設幾格
    int  iModeX;                                                                //針對NON-Standard的X排列要用哪一種

    int  Item[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    bool bNeedCheck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichSite[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichAuto[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iNeedRotAng[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                   //預計旋轉的角度   //Steven 20170425 (wei) : Add rotate motor
    int  iCurrRotAng[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                   //目前產品的角度
    bool bQATray[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iCleanCount[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    bool bFliped[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichIndex[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iNeedSuck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                     //0:不需要吸, 其他:IC type

    bool bPass[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    bool bNeedReTest[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iBinData[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iBinDataBackUp[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
    bool bNeedRotate[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichSht;
    int  iWhichKit;
    int  iWhichTray;
    int  iWhichShtPickFor32;                                                    //Steven 20140512: For HT-9047
    int  iWhichKitPickFor32;                                                    //Steven 20140512: For HT-9047
    int  HotCount;

    AnsiString cDeviceInfAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];   //Ifor 20170925 (Steven) : add
    AnsiString cReDeviceInfAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; //Ifor 20170925 (Steven) : add
    int  iNeedRotAngAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];        //Ifor 20170925 (Steven) : add 預計旋轉的角度
    int  iCurrRotAngAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];        //Ifor 20170925 (Steven) : add 目前產品的角度
    int  iWhichShuttleAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichKitAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iHotCountAutoSitemapping[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iWhichShuttleBackup;
    int  iWhichKitBackup;                                                       //wei 20171211 (jou) InArmSuck.iWhichKit-->InArmSuck.iWhichKitBackup

    bool bScan[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    int  iAOIStation[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection

    AnsiString cDeviceInf[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];              // 2012.06.01 Q_Q BarCode Mode
    AnsiString cReDeviceInf[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];            // 2012.06.01 Q_Q BarCode Mode
    AnsiString cSBin[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                   //Steven 20220120 : Amlogic需要收SBIN
    bool b2DIDNG[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                       //Steven 20200611 : for Murata, 2DID NG不測試
    int iAOIResult[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                     //Sam 20240325 : 新增 DamageTrayMapping 功能
    TMyProductionRecord PordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];

    bool bTrayMapNGData[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                //KenHsieh 20220923 : add Tray Map Throw IC Function

    TQPF_Timer  TSoakTime;                                                      //jou 2012-06-29 Index Pick up need wait Soak Time

    int iAutoCleanRecX[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                 //jou 2013-03-13 Auto Clean Record X
    int iAutoCleanRecY[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];                 //jou 2013-03-13 Auto Clean Record Y

    void SetMyLed(int iRow, int iCol, TALed *ledPtr);                           //設定燈號
    void SetItemAmount(int iRow, int iCol);                                     //設定數量
    void SetMotorCount(int iRow, int iCol);
    void SetPickerCount(int _iPickRow, int _iPickCol, int _iShtRow, int _iShtCol, int _iPickStep, int _iKitStep, int _iShtStep);
    void SetPickerCount(int _iPickRow, int _iPickCol);

    void ResetAll();                                                            //重置SuckTask
    bool CheckLedStatus();
    void CheckVaccumIsIniaialON(int iRow, int iCol, bool &flag);                // if sensor Off  ,off vacuum ,flag not chage
                                                                                // if        On   ,if has not Real IC then off vacuum,flag=true
                                                                                //                 else do not change
    void CheckVaccumIsON_AboveSocket(int X, int Y, bool &flag);                 // if sensor Off  ,off vacuum ,flag not chage

    //兩個資料交換----------------
    void MoveAllItem(class TMyKitSuck &Source);
    void MoveSingalItem(class TMyKitSuck &Source, int iWhichSuck);
    void CopyFrom (class TMyKitSuck &Source);
    void MergeSuck(class TMyKitSuck &Source1, class TMyKitSuck &Source2, bool InsertCol=false);
    void SplitSuck(class TMyKitSuck &Source1, class TMyKitSuck &Source2, bool InsertCol=false);
    void MergeSuck1Row(class TMyKitSuck &Source1, class TMyKitSuck &Source2);
    void SplitSuck1Row(class TMyKitSuck &Source1, class TMyKitSuck &Source2);
    void MoveSuckData(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR=-1, int TargetC=-1);
    void MoveSuckDataDiff(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC);
    void CopyFromTray(int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot, int TrayR, int TrayC, int iTrayData, int iTarget=-1, bool bHP2Inarm=false);   //Steven 20150714 : Manual Sort
    void CopyToTray  (int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot, int TrayR, int TrayC, int iTrayData, int iTarget=-1);   //Steven 20150714 : Manual Sort

    //設定資料----------------
    void SetAll(int Type);
    void SetAllToNullIC();                                                      //kevin 20150505
    void SetAllRealIC2InterfaceBin();
    void SetAllHASIC2ErrorBin();
    void SetUnuseToNullIC();                                                    //Steven 20241017 : 清除沒用到的資料
    void SetUnuseAndHasNullICToNullIC();                                        //Steven 20241017 : 清除沒用到的資料 與 HAS_NULL_IC資料
    void SetNullIcToHasNullIc();
    void SetHasNullIcToNullIc();                                                //Steven 20160524 : ADD SetHasNullIcToNullIc
    void SetItemData(int iRow, int iCol, int data, int iTarget=-1);

    //清除資料----------------
    void ClearAll();
    void ClearAllError();

    //檢查內容---------------------
    bool FindNoIC();                                                            //判斷是否有空位置  kevin 20120531
    bool HasIC();
    bool HasRealIC();
    bool HasRealIC_Left(int iRow, int Mid);
    bool HasRealIC_Right(int iRow, int Mid);
    bool HAS_NO_IC();
    bool HasDefineIC(int IC_TYPE);                                              //Steven 20130620 : 改成Function
    bool RowHasDefineIC(int iRow, int IC_TYPE);                                 //Steven 20221005 : 針對上下排判斷有沒有指定的IC
    bool HasType(int);
    bool HasNotTestYet();
    bool Tested();
    bool NoIC();

    bool All_HAS_NULL_IC();
    bool All_HasIC();
    bool ShtAll_HasICType(int IC_TYPE1, int IC_TYPE2);
    bool ArmAll_HasICType(int IC_TYPE1, int IC_TYPE2);
    bool UseSiteFullIC();                                                       //Ifor 20161215 add 判斷Use Site 有無NULL_IC
    bool UseSiteHasIC();
    bool UseSiteNoIC();

    int  CountRealIC();

    bool AlreadyTest();
    bool AlreadyTestNotIncludeErrorBin();                                       //Steven 20200611 : for Murata, 2DID NG不測試
    bool PartAlreadyTest();

    bool RightSideNoIC(int MiddleValue);
    bool LeftSideNoIC(int MiddleValue);
    bool ArmLeftSideHaveRealIC(int MiddleValue);
    bool ArmRightSideHaveRealIC(int MiddleValue);
    bool ArmUpSideNoIC();
    bool ArmDownSideNoIC();
    bool ArmRow0HaveRealIC();
    bool ArmRow1HaveRealIC();
    bool ArmUpSideHaveRealIC(bool left=true);                                   //ChungHung 20130708 add left=true
    bool ArmDownSideHaveRealIC(bool left=true);                                 //ChungHung 20130708 add left=true;
    bool ArmUpSideAllTypeIC(int IC_TYPE, int iOffset, int iCol);                //Steven 20220930 : CheckSpecialSiteMode run Auto Clean
    bool ArmDownSideAllTypeIC(int IC_TYPE, int iOffset, int iCol);              //Steven 20220930 : CheckSpecialSiteMode run Auto Clean

    bool IsShtSuckFinish();
    bool IsShtDestroyFinish();
    bool IsPickSuckFinish();
    bool IsPickDestroyFinish();

    bool CheckDestoryFinish();                                                  //Steven 20210825 : 吹氣完成才可以歸零

    bool bAlreadyRotate;                                                        //Steven 20210609 : 拆出Precisor, Rotator, Bottom CCD, Die Clean
    bool bAlreadyPreciser;
    bool bAlreadyDieClean;
    bool bAlready2DID;
    bool bAlreadyAOI;
    bool bAlreadyFlipper;                                                       //Frank 20210612 : Flipper Function
    bool bAlreadyFixAI;                                                         //Sam 20211220 : 拆出 Fix AI AOI
    bool bAlreadyThrowIC;  void SetType1ToType2ByPickCol(int Type1, int Type2);  // AI(W906-A4-6) 20260924: 移植樹獨有（原在精簡鏡像）—— golden 只有 ainarm9045_2x4_16.cpp 的呼叫點、沒有定義；保留讓該檔編得起來，本體見 mykitsuck.cpp 檔尾  （原行尾註解：KenHsieh 20220923 : add Tray Map Throw IC Function）
};

extern TMyKitSuck InArmSuck;
extern TMyKitSuck FLCarryKit;
extern TMyKitSuck FRCarryKit;
extern TMyKitSuck BLCarryKit;
extern TMyKitSuck BRCarryKit;
extern TMyKitSuck OutSht3Kit;                                                   //Steven 20240822 : For HT-9046AU
extern TMyKitSuck FTestSuck;
extern TMyKitSuck BTestSuck;
extern TMyKitSuck OutArmSuck;
extern TMyKitSuck OutArm2Suck;                                                  //Steven 20240822 : For HT-9046AU
extern TMyKitSuck CatchTraySuck;
extern TMyKitSuck TestSocket;
extern TMyKitSuck CheckKitSuck;                                                 //20111130  Dell
extern TMyKitSuck AOIKit;                                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern TMyKitSuck InArmPlaceSuck;                                               //JimmyChiu 20220908 add Pickup Error Placement
//---------------------------------------------------------------------------
extern TMyKitSuck FTestSuckBackup;
extern TMyKitSuck BTestSuckBackup;

extern TMyKitSuck InArmSuckBackup;
extern TMyKitSuck OutArmSuckBackup;

extern TMyKitSuck *ptrInSHT;                                                    //Auto Clean Kit use
extern TMyKitSuck ptrInSHTBackup;                                               //Auto Clean Kit use
extern TMyKitSuck *ptrOutSHT;
extern TMyKitSuck *ptr2DIDSHT;
extern TMyKitSuck *ptrMulti2DIDSHT;

extern vclcompat::TList *pSuck;  // AI(W906-A4-6) 20260924: 完整名稱（全域 using 已移除）
extern TMySucker *pTempSuck;

extern void ClearAllManualSuckTask();
extern void InitialSuckerName();
extern void SetAllRealIC2InterfaceBin();
extern void CopyKitSuck(TMyKitSuck *Source, TMyKitSuck *Target);
extern void CopySuck(TMySucker *Source , TMySucker *Target);
extern void CopyInitSuck(TMyKitSuck *Source, TMyKitSuck *Target, int SourceR, int SourceC, int TargetR, int TargetC);
extern TMyProductionRecord OutArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];   //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
extern TMyProductionRecord SortArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];  //RogerYang 20250515 Add for 9046AU
// AI(W906-A4-6) 20260924: 觀測接縫的全域（定義在 aHotPlateSubstrate.cpp:66-75，原宣告在已退役的精簡鏡像區段內）
#ifndef HT9045_SUCKER_W7L2_SEAM_GLOBALS
#define HT9045_SUCKER_W7L2_SEAM_GLOBALS
extern unsigned long W906_TMySucker_OffDestroyEpoch;
extern long          W906_TMySucker_OffDestroyTotal;
void W906_TMySucker_OffDestroy_ResetAll();
#endif
#endif
