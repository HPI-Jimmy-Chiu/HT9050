// =============================================================================
//  aoutarm.h  --  Out-arm ENGINE public surface (mirror of golden aoutarm.h).
//
//  Faithful translation of golden aoutarm.h (173 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W4-aoutarm) 20260808
//  Translation wave: W906-PT-W4 (last tier of genuinely-missing non-form units).
//
//  ROLE: the header half of golden aoutarm.cpp -- it publishes the out-arm
//  ENGINE's file-scope cursors (OutArmTask / iPickFromShuttle1/2Task /
//  iPlaceToAuto/Fix/SortShtTask / the 13 Magazine + catch-tray cursors), the
//  per-picker active/position grids (bOutArmSuckActive / iOutArmXPosition /
//  iOutArmYPosition), the close-pitch geometry doubles (dOutArmXPitch_1Step /
//  _MovePitch / iOutArmXStep / iOutArmYStep) and 60-odd free functions that the
//  27 per-site aoutarm9045_<cfg> variants + acarry / acatchtray / csystem call.
//  It also declares class TMyQASample and its single global instance QASam.
//
//  WAVE SCOPE -- transcribed VERBATIM except for the two deviations in the GATE
//  REGISTER below.  Every extern/prototype keeps golden's order, spelling,
//  default arguments and trailing Big5 comment.
//
//  GATE REGISTER -- 2 deviations from golden text, both in this header:
//   (H1) golden aoutarm.h:5  `#include "mykitsuck.h"`  -- GATED #if 0.
//        WHY THE #else IS FAITHFUL: this header's ONLY use of TMyKitSuck is the
//        pointer parameter of `CheckOutputShuttleDataMiss(TMyKitSuck *P,int,int)`
//        (golden aoutarm.h:151), for which an incomplete type is sufficient in
//        C++.  The #else arm therefore forward-declares `class TMyKitSuck;` and
//        adds `MachineType.h` (for MAX_ARM_Row / MAX_ARM_Col, which golden got
//        transitively through mykitsuck.h) + `vclcompat/vcl_compat.h` (AnsiString,
//        for SendDataToASE / OutArmPickShuttleAlarm / SaveDataCurrentlyTrayData).
//        WHY IT MUST BE GATED RATHER THAN COPIED: this port tree has TWO
//        DIFFERENT classes named TMyKitSuck with DIFFERENT LAYOUTS --
//        mykitsuck.h:274 and aHotPlateSubstrate.h:365 -- and 14 colliding global
//        names (InArmSuck / OutArmSuck / F,B L,R CarryKit / F,BTestSuck / ...).
//        The 177 TUs that actually compile against those globals use the
//        aHotPlateSubstrate.h layout (that is where the `extern TMyKitSuck
//        OutArmSuck;` those TUs see is declared, and mykitsuck.cpp is NOT
//        registered in CMakeLists.txt as of this wave -- see the .cpp banner's
//        absence-claim list).  A header that unconditionally pulled mykitsuck.h
//        would silently give any future includer the WRONG member offsets: it
//        would compile and it would link, and every field access would read the
//        wrong bytes.  BEHAVIOUR DELTA ON A REAL MACHINE: none.  No declaration
//        in this header needs a complete TMyKitSuck, and aoutarm.cpp itself
//        includes aHotPlateSubstrate.h for the complete type.  Retire this gate
//        when the tree collapses its two TMyKitSuck definitions into one.
//   (H2) golden aoutarm.h:160 `__fastcall TMyQASample();` -- the `__fastcall`
//        is DROPPED (this wave's contract: __fastcall/__published/#pragma
//        package go).  Kept in lockstep with the definition in aoutarm.cpp
//        (golden :3580), so there is no decl/def calling-convention mismatch of
//        the kind vclcompat/vcl_compat.h:24-50 warns about.  BEHAVIOUR DELTA:
//        none -- TMyQASample::TMyQASample is only ever called by the static-init
//        of `QASam` inside aoutarm.cpp; no other TU takes its address.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: UTF-8, ZERO U+FFFD, ZERO CRLF.
// =============================================================================

//---------------------------------------------------------------------------
#ifndef aoutarmH
#define aoutarmH
//---------------------------------------------------------------------------
//AI(W906-PT-W4-aoutarm) 20260808: GATE (H1) -- see GATE REGISTER above.
#if 0
#include "mykitsuck.h"                                                          //RogerYang 20250522 Add for 9046AU
#else
#include "MachineType.h"                                                      //AI(W906-PT-W4-aoutarm) 20260808: (H1) MAX_ARM_Row / MAX_ARM_Col, which golden got transitively via mykitsuck.h
#include "vclcompat/vcl_compat.h"                                             //AI(W906-PT-W4-aoutarm) 20260808: (H1) AnsiString, likewise transitive in golden
class TMyKitSuck;                                                             //AI(W906-PT-W4-aoutarm) 20260808: (H1) incomplete type is enough for the CheckOutputShuttleDataMiss pointer parameter
#endif
//#include "cprod.h"              //kevin 20140325
//#include "MachineDefine.h"      //kevin 20140325
#include "myTimer.h"

extern bool bOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col];
extern bool bOutArmSuckActiveBack[MAX_ARM_Row][MAX_ARM_Col];
extern bool bOutArmManualStepPress;
//extern bool bOutArmZNeedCheck[4];
extern int OutArmTask;
extern int iDoOutArmAfterPlaceToAutoTask;                                       //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
extern int iOutArmPlaceOrder;
extern int iOutArmPickOrder;                                                    //JerryYang 20221215 : add Magazine
extern int iWhichAuto;
extern int iWhichBuff;                                                          //JerryYang 20221215 : add Magazine
extern int iWhichMag;
extern int iScannerFailCount[4];
extern int iScannerContinueFail;
extern int iScanCount;
extern bool bOutArmXOverLimit;                                                  //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題

extern TQPF_Timer DoPickFromShuttle1Delay;
extern TQPF_Timer DoPickFromShuttle2Delay;
extern TQPF_Timer DoPlaceToAutoDelay;

extern bool bPickShuttleError;

extern bool bOverTray;                                                          //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上

void InitOutArmTask();
void DoOutArm();
bool MoveOutArmXY_ToAuto(int Type);
bool MoveOutArmXY_ToManual(int Type);
bool IsOutArmSuckHasAuto(int type);
int GetOutArmPlaceToAutoMaxSpin();
int GetOutArmPlaceToManualMaxSpin();
bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                           //Ifor 20250926 add:Fix Tray 取放時移開Out Arm
bool MoveOutArmXY_ToShuttleAlarmArea();
bool MoveOutArm2XYToDecayTeach();                                               //Ifor 20151210 新增OurArm Move Decay
bool MoveOutArmXY_ToShtMidArea();
extern int OutArmTask;
extern int iPickFromShuttle1Task;
extern int iPickFromShuttle2Task;
extern int iPlaceToSortShtTask;                                                 //RogerYang 20250513 Add for 9046AU
extern int iPlaceToAutoTask;
extern int iPlaceToFixTask;
extern int iFix3CanFullTask;                                                    //ChungHung 20140313 add Fix3 can Full Tray
bool MoveOutArmToAutoSafe();
bool CheckOutArmZ(bool bMessage);
void SetFixTrayMiddleDtata();

bool CheckTesterZ(int WhichShuttle);
bool CheckShuttleICPos(int WhichShuttle);
int  CheckOutArmCleanOut(int Task=50);
int  CheckBin(int &ct, int iShuttle);                                           //kevin 20160906 void->int
bool MoveOutArmZ_Shuttle_Place();
bool CheckOutArmInitState();
void InitOutArmPickFromAuto1Task();
//bool IsOutArmNeedMoveToWait();
bool IsCatchTrayReadySupplyNewTray();
void InitOutArmInspectTask();
void InitPlaceToAutoTask();
void InitPickFromShuttle1Task();
void PickFromShuttle(int ibwhichKit);                                           //kevin 20210612 add int iwhichKit=0
void OutArmAddSpeed();
void OutArmSubSpeed();
bool NeedScan();
void InitPickFromShuttle2Task();
void SetFixTrayFullIC();
void initDoPlaceOutRotateTask();
bool DoPlaceOutRotate();

extern int iOutArmInspectTask;
extern int iOutArmPickFromAuto1Task;
extern int iPlaceToOutRotateTask;
extern bool bPlaceToAuto1IsFull;

void DoOutArm_Scanner();
void SearchBinBoxUpDown(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol);     //Steven 20230601 : 重新整理Bin Box
int Find_OutArm_PickerMaxUseCountOnTime(int iUseSuck, int &iRow, int &iCol);
int Find_OutArm_PickerBufferMaxUseCountOnTime(int iUseSuck, int &iStep, int &iRow, int &iCol, bool bSearchLast);        //JerryYang 20221215 : Magazine把fix區當buffer區功能
void Find_OutArm_Single(int iUseSuck, int &iRow, int &iCol);
extern int iOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col];
extern int iOutArmYPosition;
void AdjustOutArmClosePitchCondition(int iw, bool &bCanPick2ICAtOnceTime);
bool CheckOutSuckICFallDown(bool AutoClean=false);

bool DetectColorTrayFail(int Index);

extern bool bCarryControlOutarm1;                                               //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag
extern bool bCarryControlOutarm2;

void SetOutArmHome();                                                           //jou 981203 alarm時，out arm z motor home

extern bool bUseOnebyOne;                                                       //2010-12-07
void ShowOutputShuttleDataMiss();                                               //Steven 20110527
void IfUseOnebyOne(int iAuto);                                                  //Steven 20111101 : 整合成function
void OutArmAddBlockPitch(int &nowX, int &nowY, int iRow, int iCol);
void SendDataToASE(AnsiString Data);                                            //kevin 20161228 add
void TransferAutoRatio(int iWhichAuto, int *iXPos, int *iYPos);                 //Steven 20141029 : Auto的軟體齒輪比
void TransferOutShuttleRatio(int iShuttle, int *iXPos, int *iYPos, int Row, int Col);                                   //Steven 20141029 : Out Shuttle的軟體齒輪比
int OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart);                                        //Steven 20141121 : 整合OutArm吸取異常
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iShtR, int iShtC, int iSuckR, int iSuckC);                       //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog

bool UnloaderTrayYDirForArmYPitch(int iW);
int AutoCalculateOutArmXClosePitch(int iWhichAuto, bool bUseDeviceDinemsion=false);
int AutoCalculateOutArmYClosePitch(int iWhichAuto);
extern bool GetOutArmToUnLoaderPosition(int bCanPick2ICAtOnceTime, int iOutPutTray, int &iXPos, int &iYPos, int iR, int iC, bool RealMove);
extern double dOutArmXPitch_1Step;                                              //1吸嘴之間的X-Pitch
extern double dOutArmXPitch_MovePitch;                                          //張到最開的X-Pitch
extern int    iOutArmXStep;                                                     //Out Arm在Tray X一次要跳幾格
extern int    iOutArmYStep;                                                     //Out Arm在Tray Y一次要跳幾格
bool SaveDataCurrentlyTrayData(AnsiString iUnloader);

//kevin 20160822 error bin box Start--------------------------------------------
bool CheckUseFixBinBoxFunction();
bool CheckBinBoxIsFull(bool Alarm,int detectWhichAuto);                         //kevin 20160822
int SearchTrayToPlace_CateR();                                                  //Steven 20161221 : Cate R for ART
bool DoOutArmXYToPlaceFixBinBox(bool HasYPitch);
bool ErrorBinBoxDetect(int iError);                                             //kevin 20160906 error bin box判斷 次數及計數
//------------------------------------------------------------------------------
extern bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin);
void OutArmAddSpeedDisplay();                                                   //KaiChen 20171225 (Steven)：Add Speed Display
void OutArmSubSpeedDisplay();                                                   //KaiChen 20171225 (Steven)：Add Speed Display

bool CheckDuplicateBarCode();                                                   //jou 20191008 : Barcode duplicate file by out arm
bool bOutShtwaitPick();                                                         //kevin 20210602 Out arm sht on wait

extern int iPlaceToMagazineBufferTask;                                          //JerryYang 20221215 : add Magazine
extern int iPickFromMagazineBufferTask;

extern int iPlaceToMagazineTrayTask;
extern int iAuto3MagazineTask;
extern int iMagazineInitCheckTask;
extern int iMagazineUpDownTask;
extern int iCatchTrayGetTrayTask;
extern int iCatchTrayReceiveTrayTask;
extern int iMagazineTrayFeedTask;
extern int iCatchTrayGetNewTrayTask;                                            // 2012.11.05 , Joye , HT-7046
extern int iCatchTrayChangeTrayTask;                                            // 2012.11.05 , Joye , HT-7046
extern int iCatchMagazineTrayOut;
extern int iDoMagazineScanHasTrayTask;

bool MoveOutArm2XYToShuttle2Wait();
bool CalOneByOneCount(int iAuto);                                               //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞

bool CheckOutputShuttleDataMiss(TMyKitSuck *P, int iR, int iC);                 //RogerYang 20250516 Add for 9046AU
void InitPlaceToSortShtTask();                                                  //RogerYang 20250609 Add for 9046AU

//---------------------------------------------------------------------------
class TMyQASample
{
    private:
    protected:
    public:
        TMyQASample();                                            //AI(W906-PT-W4-aoutarm) 20260808: GATE (H2) -- golden `__fastcall TMyQASample();`
        int iCount[4][8];
        int iMinX;
        int iMinY;
        int iCurrCount;
        void ClearCount();
        void FindDeviceToQA();
};
extern TMyQASample QASam;
extern int iOutArmAdditionalFunctionTask;
extern void InitDoOutArmAdditionalFunction();
void SetOutArmNeedDestory(int iRow, int iCol, int iShtRow, int iShtCol, bool bPlace=false);                             //RogerYang 20250516 Add for 9046AU
void GetTrayDirection(int iWhichAuto, int &Direction, int &ixStart, int &ixEnd, int &ixStep, int &iyStart, int &iyEnd, int &iyStep);
#endif
