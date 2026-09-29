// =============================================================================
//  TfAOILaserScan.cpp  --  generic AOI/laser-scanner client family (Loader
//                          tray residual-IC check + tray cell-map read-back)
//
//  Faithful translation of golden TfAOILaserScan.cpp (1398 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "aoilaser".
//
//  ROLE: see TfAOILaserScan.h's own banner for the class-level picture
//  (TAOISocket / TfAOI / TfTrayDeviceCheckFromLoader / TfTrayMapFromLoader).
//  This file is 100% state-machine + protocol-string plumbing; it owns no
//  file-scope global (every field lives on one of the 4 classes).
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim -- all 54 golden functions with a
//  compiled body; every branch, magic number and golden comment preserved).
//  golden line ranges below cite the FUNCTION BODY span (open brace to close
//  brace) in the golden .cpp:
//    TAOISocket::TAOISocket                              golden :18-23
//    TAOISocket::~TAOISocket                              golden :25-28
//    TAOISocket::ReceiveData                              golden :30-37
//    TAOISocket::DoSendCommand                            golden :39-63
//    TAOISocket::SendCommandSocket                        golden :65-79
//    TAOISocket::DoCommuncation                           golden :81-84
//    TAOISocket::IsCommActive                             golden :86-89
//    TfAOI::TfAOI(AnsiString)                             golden :92-110
//    TfAOI::~TfAOI                                        golden :112-117
//    TfAOI::GetAOI_Command                                golden :119-141
//    TfAOI::GetLoadFileName                                golden :143-146
//    TfAOI::GetLotMO                                       golden :148-151  (GATE 2)
//    TfAOI::GetErrorMsg                                    golden :153-156
//    TfAOI::GetTrayData                                    golden :158-167
//    TfAOI::ReceiveData                                    golden :169-175
//    TfAOI::DoSendCommand                                  golden :177-201
//    TfAOI::SendCommandSocket                              golden :203-214
//    TfAOI::DoCommuncation                                 golden :216-221
//    TfAOI::IsCommActive                                   golden :223-226
//    TfAOI::GetXC_YC                                       golden :228-233
//    TfAOI::SetAndBackUpLoaderYSpeed                       golden :235-241
//    TfAOI::RestoreLoaderYSpeedRaw                         golden :243-247
//    TfAOI::DoMoveOut                                      golden :249-323
//    TfAOI::DoMoveIn                                       golden :325-408
//    TfAOI::DoSetLot                                       golden :411-445
//    TfAOI::DoAutoTuning                                   golden :447-536
//    TfAOI::SetMoveTrack                                   golden :538-541
//    TfAOI::GetMoveTrack                                   golden :543-546
//    TfAOI::GetInSensor                                    golden :548-561
//    TfAOI::GetOutSensor                                   golden :563-576
//    TfTrayDeviceCheckFromLoader::ctor                     golden :579-581
//    TfTrayDeviceCheckFromLoader::dtor                     golden :583-585
//    TfTrayDeviceCheckFromLoader::DoSettingBeforeMotorMove golden :587-672
//    TfTrayDeviceCheckFromLoader::DoGetValueAfterMotorArrival golden :674-714
//    TfTrayDeviceCheckFromLoader::DoAOI_LoadFile           golden :716-750
//    TfTrayDeviceCheckFromLoader::DoAOI_ClearBuffer        golden :752-787
//    TfTrayDeviceCheckFromLoader::DoAOI_Function           golden :789-823
//    TfTrayDeviceCheckFromLoader::DoGetValue               golden :825-869
//    TfTrayDeviceCheckFromLoader::DoGoldenImage            golden :871-905
//    TfTrayDeviceCheckFromLoader::DoGetStatus              golden :907-941
//    TfTrayMapFromLoader::ctor                             golden :944-946
//    TfTrayMapFromLoader::dtor                             golden :948-951  (GATE 1)
//    TfTrayMapFromLoader::DoAOI_LoadFile                   golden :953-987
//    TfTrayMapFromLoader::DoAOI_ClearBuffer                golden :989-1024 (GATE 1)
//    TfTrayMapFromLoader::DoAOI_Function                   golden :1026-1060
//    TfTrayMapFromLoader::DoGetValue                       golden :1062-1128 (GATE 1)
//    TfTrayMapFromLoader::DoCreateFile                     golden :1130-1169
//    TfTrayMapFromLoader::DoGoldenImage                    golden :1171-1205
//    TfTrayMapFromLoader::DoGetStatus                      golden :1207-1241
//    TfTrayMapFromLoader::AnalysisRowCol                   golden :1243-1254
//    TfTrayMapFromLoader::AnalysisTrayData                 golden :1256-1281 (GATE 1)
//    TfTrayMapFromLoader::HasNullIC                        golden :1283-1294 (GATE 1)
//    TfTrayMapFromLoader::DoSettingBeforeMotorMove         golden :1296-1353
//    TfTrayMapFromLoader::DoGetValueAfterMotorArrival      golden :1355-1397
//
//  NOT TRANSLATED, AND WHY (not a gap in the 54-function count above):
//  golden's own header declares a SECOND TfAOI ctor, `TfAOI();` (no-arg,
//  TfAOILaserScan.h golden :49) -- but golden's OWN .cpp never defines it
//  anywhere (grepped the whole golden .cpp this wave for `TfAOI::TfAOI()`:
//  zero hits; the only defined ctor is the AnsiString-arg one at :92). Golden
//  itself only ever constructs TfAOI-derived objects via the AnsiString-arg
//  base (TfTrayDeviceCheckFromLoader :579 and TfTrayMapFromLoader :944 both
//  write `: TfAOI(asname)`), so golden's own linker never needs the no-arg
//  ctor either. This is a bodyless declaration in GOLDEN, not a translation
//  gap: rule 5 ("every golden function must get a real body") binds functions
//  golden itself defines. Declared in the port header exactly as golden wrote
//  it (byte-identical surface); left undefined here exactly as golden left it
//  undefined, for exactly the same reason (never called). If some future wave
//  ever constructs a bare `TfAOI x;` this will surface as a normal link error,
//  the same day it would surface in a from-scratch BCB6 rebuild of golden.
//
//  GATE REGISTER
//  -------------
//  (1) TTMyTray incomplete substrate -- see TfAOILaserScan.h's own GATE (1)
//      for the full argument (VCL custom-control widget, zero compiled
//      definition anywhere in this port tree, deferred tree-wide to W7-C4).
//      Five call sites gated, each individually, all inside
//      TfTrayMapFromLoader (the ONLY class that ever dereferences a TTMyTray*
//      -- TfTrayDeviceCheckFromLoader never touches one):
//        (1a) ~TfTrayMapFromLoader()          golden :950  `delete trayMapIC;`
//             WHY false is not even attempted here: trayMapIC is EXTERNALLY
//             OWNED -- never `new`'d by this class, only handed in via
//             SetTrayIC() from (not-yet-translated) TfTrayMapping::
//             InitialAOIGroup's `mtTrayMap` (golden cTrayMapping.cpp :5409).
//             Even in golden this delete is a double-free candidate the day
//             TfTrayMapping's own dtor also frees mtTrayMap; that is a
//             pre-existing golden lifetime question, not something this
//             mechanical translation resolves either way. Faithful default:
//             do nothing (no delete attempted against a pointer this class
//             does not own, of a type this build cannot even destruct).
//        (1b) DoAOI_ClearBuffer()             golden :994  `trayMapIC->ClearCell();`
//             Faithful default: no-op (nothing to clear without a widget).
//        (1c) DoGetValue()                    golden :1104-1105
//             `trayMapIC->XItem=iGetCol; trayMapIC->YItem=iGetRow;`
//             Faithful default: dropped (nothing observes them offline).
//             NOTE: the very next golden line, :1106
//             `AnalysisTrayData(trayMapIC,tsAnalysis->Strings[4]);`, is NOT
//             itself gated -- AnalysisTrayData is a real function THIS SAME
//             file defines (see 1d below); calling it needs no gate, only
//             its own body does.
//        (1d) AnalysisTrayData(TTMyTray*,AnsiString) golden :1259-1279 (whole
//             body: reads mTray->YItem/->XItem, writes
//             mTray->SetCellColorIndex() per cell). Faithful default: no-op
//             (params suppressed with (void) casts -- there is no grid
//             widget to paint offline).
//        (1e) HasNullIC(TTMyTray*)             golden :1285-1293 (whole body:
//             reads mTray->YItem/->XItem/->GetCellData()). Faithful default:
//             `false`, matching TfAOI's OWN base-class default one level up
//             (TfAOILaserScan.h golden :127 `virtual bool HasNullIC(TTMyTray*
//             mTray){return false;}`) -- "no null IC detected" is already
//             golden's own answer when a concrete override has nothing better
//             to say.
//      BEHAVIOUR DELTA, STATED PLAINLY: on a real machine with
//      CosFunction.bTrayMapFromLoader enabled and a live laser scanner, golden
//      paints/reads the physical tray-map grid through these five sites; this
//      build never does, and TfTrayMapFromLoader::HasNullIC always reports
//      "no null IC" regardless of what the scanner actually saw. Retire this
//      gate the moment vclcompat::TrayCore (W7-C4) lands a complete TTMyTray
//      with real ClearCell/SetCellColorIndex/GetCellData/XItem/YItem.
//  (2) TfAOI::GetLotMO()  golden :150  `return (fLotInfo->ed_PIOEEMO->Text);`
//      -- `ed_PIOEEMO` (golden uLotInfo.h:601, `TEdit *ed_PIOEEMO;`) has not
//      been ported onto forms/fLotInfo.h. Grepped the whole port tree this
//      wave for `ed_PIOEEMO`: the only hit outside golden itself is the dfm2rc
//      layout dump (tools/dfm2rc/layout_out/uLotInfo_layout.gen.cpp), which is
//      generated metadata, not a compiled member. Adding the member is out of
//      THIS wave's scope (forms/fLotInfo.h is a pre-existing file -- rule 7
//      forbids touching it from this narrow, single-golden-unit wave).
//      Faithful default: "" (empty AnsiString) -- an unknown/blank Lot MO,
//      the same "nothing known yet" reading vclcompat::TEdit's own
//      Text-defaults-to-"" convention gives every other unset TEdit member on
//      this facade (see forms/FormWidgets.h's DEFAULT-VALUE RULE citation).
//      BEHAVIOUR DELTA: TfAOI::DoSetLot's outgoing "SL,<MO>" command (golden
//      :416) carries an empty MO string instead of the operator-entered lot
//      MO until ed_PIOEEMO is ported.
//
//  VCL/Borland conversions:
//   * `#pragma hdrstop` (golden :3) / `#pragma package(smart_init)` (golden
//     :16) dropped.
//   * `socketClient->SetReceiveFunc(ReceiveData);` (golden :22, :97) --
//     ReceiveData is a bound-`this` member closure in golden (BCB6 binds it
//     implicitly); TPointVoidReceive here is `std::function<void(char*,int)>`
//     (EJ1N/uSocketServerClient.h), which needs the binding spelled out. Same
//     idiom EJ1N/uDTME08Control.cpp:108 already uses for the identical
//     pattern: `[this](char* cGet,int iLen){ReceiveData(cGet,iLen);}`.
//   * `asCommand.WideCharBufSize()` (golden :68, :206) -- real VCL
//     AnsiString::WideCharBufSize() returns `(Length()+1)*sizeof(WideChar)`
//     (a BYTE count sized for a wide-char conversion buffer, not the ANSI
//     byte length -- almost certainly a copy-paste artefact in golden's own
//     SendCommand call, since the buffer handed to SendCommand is the ANSI
//     `c_str()`, not a wide buffer). Not "fixed" here: this port's AnsiString
//     shim has no WideCharBufSize member, so the exact golden ARITHMETIC is
//     reproduced inline as `(asCommand.Length()+1)*(int)sizeof(wchar_t)`,
//     preserving golden's odd (over-long) send-length value byte for byte.
//   * `__FUNC__` (golden :319, :339, etc.) -- BCB6 builtin; this port's
//     canary_support.h already `#define __FUNC__ __func__`.
//   * `#include "MyMotor.h"` (golden :10) -> "Motor/mymotor.h" (this tree's
//     established path for that unit -- see acatchtray.cpp:86 for the
//     identical mapping); `#include "main.h"` -> "forms/fMain.h";
//     `#include "uLotInfo.h"` -> "forms/fLotInfo.h" (both per cprod.cpp:151-152's
//     established mapping).
//   * `__fastcall` / `__property`: none in this file -- golden has none
//     either (no VCL base class anywhere in this unit).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "TfAOILaserScan.h"
//   #include "main.h"
//   #include "uLotInfo.h"
//   #include "cprod.h"
//   #include "MyMotor.h"
//   #include "asendic.h"
//   #include "mysensor.h"
//   #include "acatchtray.h"                //RogerYang 20250830 : Add殘料檢(台車)
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "TfAOILaserScan.h"         // this unit's own contract

#include "forms/fMain.h"            // fMain->cbSetupFileName->Text (was "main.h")
#include "forms/fLotInfo.h"         // fLotInfo (was "uLotInfo.h") -- ed_PIOEEMO not yet ported, see GATE (2)
#include "cprod.h"                  // Prod (iMLoaderYCarPos/iMLoaderYSurePos), UserDefForm_File[4] (TRAY_TYPE_PARA)
#include "Motor/mymotor.h"          // MOT[] (TTrayMotor: .Tray.XItem/.YItem, GetSpeed/SetSpeed/ReadPos/MotorMove) -- was "MyMotor.h"
#include "asendic.h"                // TrayMoveIn / TrayMoveOut
#include "mysensor.h"               // Sen[] (TMySensor::Enable / IsOn)
#include "acatchtray.h"             //RogerYang 20250830 : Add殘料檢(台車) -- golden's own citation; no symbol from this header is referenced by this unit's ACTIVE bodies (kept for provenance/parity with golden's own include list)
#include "cmydef.h"                 // sensor index constants (SnLoaderSureTray etc.), HAS_OCR_NG, INSTALL_OCR_YMot, MLoaderY, MMTrayY -- golden reached these transitively; this port includes them directly
#include "MachineType.h"            // eOCRYMotType (eocrYMotInstal) -- golden reached this transitively too
#include "canary_support.h"         // __FUNC__
//---------------------------------------------------------------------------
TAOISocket::TAOISocket()
{
    iCommandTask=0;
    socketClient=new uSocketClient();
    //AI(W906-PT-W3) 20260807: golden `socketClient->SetReceiveFunc(ReceiveData);`
    //  (golden :22) -- see banner ADAPTATION note (bound-`this` closure).
    socketClient->SetReceiveFunc([this](char* cGet,int iLen){ReceiveData(cGet,iLen);});
}
//---------------------------------------------------------------------------
TAOISocket::~TAOISocket()
{
    delete socketClient;
}
//---------------------------------------------------------------------------
void TAOISocket::ReceiveData(char* cGet,int iLen)
{
    asReceiveMsg=AnsiString(cGet);
    if(asReceiveMsg.Length()>iLen)
        asReceiveMsg=asReceiveMsg.SubString(0,iLen);
    if(RecordMsg)
        RecordMsg("[Receive]"+asReceiveMsg);
}
//---------------------------------------------------------------------------
AnsiString TAOISocket::DoSendCommand(bool bIsFirst,AnsiString asCommand)
{
    if(bIsFirst)
    {
        iCommandTask=1;
        return "";
    }
    switch(iCommandTask)
    {
        case 1:
            //send command
            SendCommandSocket(asCommand);
            iCommandTask=100;
            break;
        case 100:
            if(asReceiveMsg=="")
            {/*wait receive*/}
            else
            {
                return asReceiveMsg;
            }
            break;
    }
    return "";
}
//---------------------------------------------------------------------------
void TAOISocket::SendCommandSocket(AnsiString asCommand)
{
    asReceiveMsg="";
    //AI(W906-PT-W3) 20260807: golden `asCommand.WideCharBufSize()` (golden :68)
    //  -- see banner ADAPTATION note; reproduces the exact golden byte-count
    //  arithmetic ((Length()+1)*sizeof(WideChar)) with this port's AnsiString.
    //  const_cast: this port's AnsiString::c_str() returns const char* (BCB6's
    //  own AnsiString::c_str() returned non-const char*); same fix as
    //  cprod.cpp:1734 / cSocket.cpp:520 for the identical BCB6-vs-port gap.
    bflag=socketClient->SendCommand(const_cast<char*>(asCommand.c_str()),(asCommand.Length()+1)*(int)sizeof(wchar_t));
    if(RecordMsg)
    {
        if(bflag)
            RecordMsg("[Send]"+asCommand);
        else
        {
            RecordMsg("[Send Error]"+asCommand+"[IP]"+socketClient->GetSocketAddress()+"[Port]"+socketClient->GetSocketPort());
        }
    }
    Sleep(5);
}
//---------------------------------------------------------------------------
bool TAOISocket::DoCommuncation()
{
    return socketClient->DoOpenCommuncation();
}
//---------------------------------------------------------------------------
bool TAOISocket::IsCommActive()
{
    return socketClient->IsConnected();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TfAOI::TfAOI(AnsiString asname)
{
    socketClient=new uSocketClient();
    tsAnalysis=new TStringList();
    socketAOI=new TAOISocket();
    //AI(W906-PT-W3) 20260807: golden `socketClient->SetReceiveFunc(ReceiveData);`
    //  (golden :97) -- see banner ADAPTATION note (bound-`this` closure).
    socketClient->SetReceiveFunc([this](char* cGet,int iLen){ReceiveData(cGet,iLen);});
    bSimulationAOICommand=false;
    iTDCTask=0;
    iCommandTask=0;
    iCommandDelayTime=90;
    asName=asname;
    asTempStr="";
    SetMoveTrack(eMT_Loader);
    bNeedCheckInitialStart=false;
    bEnableRetryCount=false;
    iRemainICLaserRetryCount=0;
    bRemainICLaserAutoSkip=false;
    iActiveDelayTime=90;
}
//---------------------------------------------------------------------------
TfAOI::~TfAOI()
{
    delete tsAnalysis;
    delete socketClient;
    delete socketAOI;
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetAOI_Command(eAOI_Command eaoi)
{
    switch(eaoi)
    {
        case eAOI_LoadFile:
            return "LF";
        case eAOI_ClearBuffer:
            return "E9";
        case eAOI_GrabIC:
            return "E1";
        case eAOI_GetValueIC:
            return "GV";
        case eAOI_SetLot:
            return "SL";
        case eAOI_CreateFile:
            return "CF";
        case eAOI_SetGoldenImage:
            return "E2";
        case eAOI_GetStatus:
            return "GS";
    }
    return AnsiString("Not Find Mapping Command");
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetLoadFileName()
{
    return fMain->cbSetupFileName->Text;
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetLotMO()
{
    //AI(W906-PT-W3) 20260807 GATE (2): golden `return (fLotInfo->ed_PIOEEMO->Text);`
    //  (golden :150) -- see file banner GATE (2). ed_PIOEEMO not yet ported
    //  onto forms/fLotInfo.h. Faithful default: "" (unknown/blank Lot MO).
#if 0
    return (fLotInfo->ed_PIOEEMO->Text);
#else
    return "";
#endif
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetErrorMsg(AnsiString asFunName,AnsiString asErrorType,AnsiString asContent)
{
    return AnsiString().sprintf("[Error][%s][%s]%s",asFunName,asErrorType,asContent);
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetTrayData()
{
    return AnsiString().sprintf("%d_%d_%0.2f_%0.2f_%0.2f_%0.2f",
                                UserDefForm_File[0].XDivision,
                                UserDefForm_File[0].YDivision,
                                UserDefForm_File[0].XStart,
                                UserDefForm_File[0].YStart,
                                UserDefForm_File[0].XPitch,
                                UserDefForm_File[0].YPitch);
}
//---------------------------------------------------------------------------
void TfAOI::ReceiveData(char* cGet,int iLen)
{
    asReceiveMsg=AnsiString(cGet);
    if(asReceiveMsg.Length()>iLen)
        asReceiveMsg=asReceiveMsg.SubString(0,iLen);
    RecordMsgLaser("[Receive]"+asReceiveMsg);
}
//---------------------------------------------------------------------------
AnsiString TfAOI::DoSendCommand(bool bIsFirst,AnsiString asCommand)
{
    if(bIsFirst)
    {
        iCommandTask=1;
        return "";
    }
    switch(iCommandTask)
    {
        case 1:
            //send command
            SendCommandSocket(asCommand);
            iCommandTask=100;
            break;
        case 100:
            if(asReceiveMsg=="")
            {/*wait receive*/}
            else
            {
                return asReceiveMsg;
            }
            break;
    }
    return "";
}
//---------------------------------------------------------------------------
void TfAOI::SendCommandSocket(AnsiString asCommand)
{
    asReceiveMsg="";
    //AI(W906-PT-W3) 20260807: golden `asCommand.WideCharBufSize()` (golden
    //  :206) -- see banner ADAPTATION note. const_cast: see TAOISocket::
    //  SendCommandSocket's own note above (same AnsiString::c_str() gap).
    bflag=socketClient->SendCommand(const_cast<char*>(asCommand.c_str()),(asCommand.Length()+1)*(int)sizeof(wchar_t));
    if(bflag)
        RecordMsgLaser("[Send]"+asCommand);
    else
    {
        RecordMsgLaser("[Send Error]"+asCommand+"[IP]"+socketClient->GetSocketAddress()+"[Port]"+socketClient->GetSocketPort());
    }
    Sleep(5);
}
//---------------------------------------------------------------------------
bool TfAOI::DoCommuncation()
{
    if(bSimulationAOICommand)
        return true;
    return socketClient->DoOpenCommuncation();
}
//---------------------------------------------------------------------------
bool TfAOI::IsCommActive()
{
    return socketClient->IsConnected();
}
//---------------------------------------------------------------------------
AnsiString TfAOI::GetXC_YC()
{
    iRow=MOT[MMTrayY].Tray.YItem;
    iCol=MOT[MMTrayY].Tray.XItem;
    return IntToStr(iCol)+"_"+IntToStr(iRow); //XC_YC
}
//---------------------------------------------------------------------------
int TfAOI::SetAndBackUpLoaderYSpeed(int iLoaderYCWSpeedPerc)                    //RogerYang 20250906 : Add for MLoaderY
{
    //AI(ht9045-v899) 20260505: 只記錄當下瞬間速度供 log 查看，不再用它做還原 (ReadSpeed 是瞬間速度不是設定值，會把馬達設成 0/2 龜速)
    int iSnapshotSpd=MOT[MLoaderY].GetSpeed();
    MOT[MLoaderY].SetSpeed(iLoaderYCWSpeedPerc);
    return iSnapshotSpd;
}
//---------------------------------------------------------------------------
void TfAOI::RestoreLoaderYSpeedRaw(int iRawSpeed)                               //AI(ht9045-v899) 20260505: 參數忽略 (歷史遺留)，永遠還原為 100% 預設量產速度，避免任何後續 Home/Teach 受影響
{
    //AI(ht9045-v899) 20260505: 修正前版 bug — 原本把 ReadSpeed() 當設定值還原 → 馬達被設成 0 或 2 → Home 反向撞擊、form 關閉龜速漂移
    MOT[MLoaderY].SetSpeed(100);
}
//---------------------------------------------------------------------------
bool TfAOI::DoMoveOut(bool bIsFirst,AnsiString &asError)
{
    #ifdef SOFT_SIMULTE
    return true;
    #else
    if(bIsFirst)
    {
        iTDCTask=1;
        asError="";
        return true;
    }

    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //RogerYang 20250830 : Add殘料檢(台車)
            {
                iTDCTask=500;
            }
            else
            {
                iTDCTask=1000;
                TrayMoveOut(false, GetMoveTrack());
            }
//            return false;
        case 500:
            if(MOT[MLoaderY].ReadPos()<=Prod.iMLoaderYCarPos-5 ||               //RogerYang 20251028 : Add
               MOT[MLoaderY].ReadPos()>=Prod.iMLoaderYCarPos+5)
            {
                iTDCTask=600;
            }
            else
            {
                iTDCTask=9999;
            }
            return false;
        case 600:
            //AI(ht9045-v899) 20260505: 與 case 500 判斷的目標位置 (CarPos) 對齊；原本判斷 CarPos 卻 MoveTo OCRPos，導致到位後仍不滿足 ±5 條件 → 死迴圈一直跑到 OCRPos
            if(MOT[MLoaderY].MotorMove(Prod.iMLoaderYCarPos))
            {
                iTDCTask=500;
            }
            return false;
        case 1000:
            TrayMoveOut(true, GetMoveTrack(), -99999);
            iTDCTask=2000;
            return false;
        case 2000:
            TrayMoveOut(true, GetMoveTrack(), -99999);
            if(GetOutSensor())
            {
                TrayArrivalDelay.SetSecAndOn(3);
                iTDCTask=3000;
            }
            return false;
        case 3000:
            TrayMoveOut(true, GetMoveTrack(), -99999);
            if(TrayArrivalDelay.Off() && GetOutSensor())
            {
                TrayMoveOut(false, GetMoveTrack());
            }
            else
                return false;
            iTDCTask=9999;
            return false;
        case 9999: //finish
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
    #endif
}
//---------------------------------------------------------------------------
bool TfAOI::DoMoveIn(bool bIsFirst,AnsiString &asError)
{
    #ifdef SOFT_SIMULTE
    return true;
    #else
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        asError="";
        return true;
    }

    if(AOIDelay.Off())
        asError=AnsiString(__FUNC__)+" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());

            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //RogerYang 20250830 : Add殘料檢(台車)
            {
                iTDCTask=500;
            }
            else
            {
                iTDCTask=1000;
                TrayMoveIn(false, GetMoveTrack());
            }
//            return false;
        case 500:
            if(MOT[MLoaderY].ReadPos()<=Prod.iMLoaderYSurePos-5 ||              //RogerYang 20251028 : Add
               MOT[MLoaderY].ReadPos()>=Prod.iMLoaderYSurePos+5)
            {
                iTDCTask=600;
            }
            else
            {
                iTDCTask=9999;
            }
            return false;
        case 600:
            if(MOT[MLoaderY].MotorMove(Prod.iMLoaderYSurePos))                  //RogerYang 20250830 : Add殘料檢(台車)
            {
                iTDCTask=500;
            }
            return false;
        case 1000:
            TrayMoveIn(true, GetMoveTrack(), 99999);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=2000;
            return false;
        case 2000:
            TrayMoveIn(true, GetMoveTrack(), 99999);
            if(GetInSensor())                                                   //jimmychiu 20220414 delete SnLoaderPreDete
            {
                htDetectICFloatingDelay.SetSecAndOn(iLD_TrayArrivalDely());     //Sam 20200420 : 到了在滑一下。
                AOIDelay.SetSecAndOn(CommandDelay());
            }
            else
            {
                TrayMoveIn(true, GetMoveTrack(), 99999);
                return false;
            }
            iTDCTask=3000;
            return false;
        case 3000:
            TrayMoveIn(true, GetMoveTrack(), 99999);
            if(htDetectICFloatingDelay.Off() && GetInSensor())
            {
                TrayMoveIn(false, GetMoveTrack(), 99999);                       //Loader 進料停止
                iTDCTask=9999;
            }
            return false;
        case 9999: //finish
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
    #endif
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool TfAOI::DoSetLot(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_SetLot)+","+GetLotMO(); //SL,Abc1234
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="SL,1,Abc1234";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //SL,1,Abc1234
            if(tsAnalysis->Count==3)
            {
                if(tsAnalysis->Strings[1]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfAOI::DoAutoTuning(bool bIsFirst,AnsiString &asError)
{
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        asError=AnsiString("");
        return true;
    }

    if(AOIDelay.Off())
    {
        asError=__FUNC__;
        asError+=" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    }

    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=1000;
            return false;
        case 1000:
            if(DoCommuncation())
            {
                iTDCTask=2000;
            }
            return false;
        case 2000:
            DoAOI_LoadFile(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3000;
            return false;
        case 3000:
            if(DoAOI_LoadFile(false))
            {
                iTDCTask=4000;
            }
            return false;
        case 4000:
            DoCreateFile(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=5000;
            return false;
        case 5000:
            if(DoCreateFile(false))
            {
                iTDCTask=6000;
            }
            return false;
        case 6000:
            DoGoldenImage(true, 0);                                             //RogerYang 20250830 : 殘料檢變更格式
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=7000;
            return false;
        case 7000:
            if(DoGoldenImage(false, 0))                                         //RogerYang 20250830 : 殘料檢變更格式
            {
                iTDCTask=8000;
                ttLoaderCarEdgePushDelay.SetSecAndOn(LoaderCarEdgePushDelay());
            }
            return false;
        case 8000:
//            DoGetStatus(true);
//            AOIDelay.SetSecAndOn(CommandDelay());
//            iTDCTask=9000;
//            return false;
//        case 9000:
//            if(DoGetStatus(false))
//            {
//              iTDCTask=10000;
//                ttLoaderCarEdgePushDelay.SetSecAndOn(LoaderCarEdgePushDelay());
//            }
//            return false;
//        case 10000:
            if(ttLoaderCarEdgePushDelay.Off())
            {
                iTDCTask=9999;
            }
            return false;
        case 9999: //finish
            return true;
        default:
            asError=__FUNC__;
            asError+="，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
}
//---------------------------------------------------------------------------
void TfAOI::SetMoveTrack(int iTrack)
{
    iMovetrack=iTrack;
}
//---------------------------------------------------------------------------
int TfAOI::GetMoveTrack()
{
    return iMovetrack;
}
//---------------------------------------------------------------------------
bool TfAOI::GetInSensor()
{
    switch(iMovetrack)
    {
        case eMT_Loader:
            return Sen[SnLoaderSureTray].Enable && Sen[SnLoaderSureTray].IsOn();
        case eMT_Auto1:
            return Sen[SnAuto1TrayDetect].IsOn();
        case eMT_Color:
            return Sen[SenColorHasTray].IsOn();
        default:
            return false;
    }
}
//---------------------------------------------------------------------------
bool TfAOI::GetOutSensor()
{
    switch(iMovetrack)
    {
        case eMT_Loader:
            return Sen[SnLoaderCarHasTray].IsOn();
        case eMT_Auto1:
            return Sen[SnAuto1PreDete].IsOn();
        case eMT_Color:
            return (Sen[SenColorCarHasTray].IsOn() && Sen[SenColorCCWDete].IsOn());
        default:
            return false;
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TfTrayDeviceCheckFromLoader::TfTrayDeviceCheckFromLoader(AnsiString asname) : TfAOI(asname)
{
}
//---------------------------------------------------------------------------
TfTrayDeviceCheckFromLoader::~TfTrayDeviceCheckFromLoader()
{
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoSettingBeforeMotorMove(bool bIsFirst,AnsiString &asError)
{
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        asError=AnsiString("");
        return true;
    }

    if(AOIDelay.Off())
        asError=AnsiString(__FUNC__)+" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=1000;
            return false;
        case 1000:
            if(DoCommuncation())
            {
                iTDCTask=2000;
            }
            return false;
        case 2000:
            DoAOI_LoadFile(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3000;
            return false;
        case 3000:
            if(DoAOI_LoadFile(false))
            {
                iTDCTask=3500;
            }
            return false;
        case 3500:
            DoSetLot(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3700;
            return false;
        case 3700:
            if(DoSetLot(false))
            {
                iTDCTask=4000;
            }
            return false;
        case 4000:
            DoAOI_ClearBuffer(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=5000;
            return false;
        case 5000:
            if(DoAOI_ClearBuffer(false))
            {
                iTDCTask=6000;
            }
            return false;
        case 6000:
            DoAOI_Function(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=7000;
            return false;
        case 7000:
            if(DoAOI_Function(false))
            {
                iTDCTask=8000;
//                Cylinder[C_LoaderCarEdgePush].On();
                ttLoaderCarEdgePushDelay.SetSecAndOn(LoaderCarEdgePushDelay());
            }
            return false;
        case 8000:
            if(ttLoaderCarEdgePushDelay.Off())
            {
//                Cylinder[C_LoaderCarEdgePush].Off();
                iTDCTask=9999;
            }
            return false;
        case 9999: //finish
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoGetValueAfterMotorArrival(bool bIsFirst,AnsiString &asError)
{
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        asError=AnsiString("");
        return true;
    }

    if(AOIDelay.Off())
        asError=AnsiString(__FUNC__)+" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=1000;
            return false;
        case 1000:
            if(DoCommuncation())
                iTDCTask=2000;
            return false;
        case 2000:
            DoGetValue(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3000;
            return false;
        case 3000:
            if(DoGetValue(false))
                iTDCTask=9999;
            return false;
        case 9999: //finish
            socketClient->Close();
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoAOI_LoadFile(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_LoadFile)+","+GetFunctionCode()+","+GetLoadFileName();
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="LF,5,1,Filename";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //LF,5,1,Filename
            if(tsAnalysis->Count>3)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoAOI_ClearBuffer(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        bIsRemainIC=false;
        asSendMsg=GetAOI_Command(eAOI_ClearBuffer)+","+GetFunctionCode()+","+GetXC_YC(); //E9,5,XC_YC
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E9,5,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E9,5,1
            if(tsAnalysis->Count>2)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoAOI_Function(bool bIsFirst, int iIndex)     //RogerYang 20250830 : 殘料檢變更格式
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GrabIC)+","+GetFunctionCode()+","+"0"; //E1,IC_Remain,Index
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E1,5,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E1,5,Result
            if(tsAnalysis->Count>2)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoGetValue(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GetValueIC)+","+GetFunctionCode(); //GV,5
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="GV,5,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //GV,5,1
            if(tsAnalysis->Count>2)
            {
                if(tsAnalysis->Strings[2]=="1") //1:Pass
                {
                    bIsRemainIC=false;
                    return true;
                }
                else if(tsAnalysis->Strings[2]=="2") //2:取像中 or 檢測中
                    return false;
                else if(tsAnalysis->Strings[2]=="3") //3:殘料檢出
                {
                    bIsRemainIC=true;
                    return true;
                }
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoGoldenImage(bool bIsFirst, int iIndex)      //RogerYang 20250830 : 殘料檢變更格式
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_SetGoldenImage)+","+GetFunctionCode()+","+IntToStr(iIndex); //RogerYang 20250830 : 殘料檢變更格式 //E2,5,Index
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E2,5,1," + IntToStr(iIndex);                          //RogerYang 20250830 : 殘料檢變更格式
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E2,5,1
            if(tsAnalysis->Count==3)                                            //RogerYang 20250830 : 殘料檢變更格式
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayDeviceCheckFromLoader::DoGetStatus(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GetStatus)+","+GetFunctionCode(); //GS,5
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="GS,5,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //GS,5,1
            if(tsAnalysis->Count==3)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TfTrayMapFromLoader::TfTrayMapFromLoader(AnsiString asname) : TfAOI(asname)
{
}
//---------------------------------------------------------------------------
TfTrayMapFromLoader::~TfTrayMapFromLoader()
{
    //AI(W906-PT-W3) 20260807 GATE (1a): golden `delete trayMapIC;` (golden
    //  :950) -- see file banner GATE (1). trayMapIC is externally owned (see
    //  banner); faithful default is to attempt nothing.
#if 0
    delete trayMapIC;
#endif
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoAOI_LoadFile(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_LoadFile)+","+GetFunctionCode()+","+GetLoadFileName();
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="LF,4,1,Filename";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //LF,4,1,Filename
            if(tsAnalysis->Count>3)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoAOI_ClearBuffer(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        //AI(W906-PT-W3) 20260807 GATE (1b): golden `trayMapIC->ClearCell();`
        //  (golden :994) -- see file banner GATE (1). Faithful default: no-op.
#if 0
        trayMapIC->ClearCell();
#endif
        asSendMsg=GetAOI_Command(eAOI_ClearBuffer)+","+GetFunctionCode()+","+GetXC_YC(); //E9,4,3_5
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E9,4,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E9,4,1
            if(tsAnalysis->Count>2)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoAOI_Function(bool bIsFirst, int iIndex)             //RogerYang 20250830 : 殘料檢變更格式
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GrabIC)+","+GetFunctionCode()+","+"0"; //E1,4,Index
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E1,4,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E1,4,1
            if(tsAnalysis->Count>2)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoGetValue(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GetValueIC)+","+GetFunctionCode(); //GV,4
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            AnsiString asDatas="";
            for(int i=0;i<iRow*iCol;i++)asDatas+=IntToStr((i<24?1:0));
            asReceiveMsg="GV,4,1,"+GetXC_YC()+","+asDatas;
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;
            if(tsAnalysis->Count>3)
            {
                if(tsAnalysis->Count>4)  //GV,4,1,3_5,111111111111000
                {
                    if(tsAnalysis->Strings[2]=="1") //1:Pass
                    {
                        int iGetCol,iGetRow;
                        AnalysisRowCol(tsAnalysis->Strings[3],iGetCol,iGetRow);
                        if(iGetCol*iGetRow==0)
                        {
                            RecordErrorMsgLaser("[Error]["+AnsiString(__FUNC__)+"][AOI Col or Row is Zero]"+asReceiveMsg);
                            return true;
                        }

                        if(iRow!=iGetRow || iCol!=iGetCol)
                        {
                            RecordErrorMsgLaser("[Error]["+AnsiString(__FUNC__)+"][AOI Col or Row is Not Mapping Now Setting Size]"+asReceiveMsg);
                            return true;
                        }
                        //AI(W906-PT-W3) 20260807 GATE (1c): golden
                        //  `trayMapIC->XItem=iGetCol; trayMapIC->YItem=iGetRow;`
                        //  (golden :1104-1105) -- see file banner GATE (1).
                        //  Faithful default: dropped. AnalysisTrayData() below
                        //  is NOT itself gated -- it is a real function this
                        //  same file defines; its OWN body gates the TTMyTray
                        //  touches it makes (see GATE (1d)).
#if 0
                        trayMapIC->XItem=iGetCol;
                        trayMapIC->YItem=iGetRow;
#endif
                        AnalysisTrayData(trayMapIC,tsAnalysis->Strings[4]);
                        return true;
                    }
                    else     //0:CCD ERR
                        RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
                }
                else                     //GV,4,2
                {
                    if(tsAnalysis->Strings[2]=="2") //2:取像未完成
                    {
                        return false;
                    }
                    else     //0:CCD ERR
                        RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
                }
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoCreateFile(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        /*
        CF,4,3_5_30_30_25_25
             XC_YC_XP_YP_W_H
             XC、YC表示IC數量，XP、YP表是間距，W、H表示IC大小(單位:mm)
        */
        asSendMsg=GetAOI_Command(eAOI_CreateFile)+","+GetFunctionCode()+","+GetTrayData(); //CF,4,3_5_30_30_25_25
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="CF,4,1,0";                                            //RogerYang 20250830 : 殘料檢變更格式
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;   //CF,4,1,ScanCount
            if(tsAnalysis->Count==4)                                            //RogerYang 20250830 : 殘料檢變更格式
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoGoldenImage(bool bIsFirst, int iIndex)              //RogerYang 20250830 : 殘料檢變更格式
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_SetGoldenImage)+","+GetFunctionCode()+","+IntToStr(iIndex); //E2,4,Index  //RogerYang 20250830 : 殘料檢變更格式
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="E2,4,1,"+IntToStr(iIndex);                            //RogerYang 20250830 : 殘料檢變更格式
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //E2,4,1
            if(tsAnalysis->Count==3)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoGetStatus(bool bIsFirst)
{
    if(bIsFirst)
    {
        DoSendCommand(true,"");
        asSendMsg=GetAOI_Command(eAOI_GetStatus)+","+GetFunctionCode(); //GS,4
        return true;
    }
    else
    {
        if(bSimulationAOICommand)
        {
            asReceiveMsg="GS,4,1";
            RecordMsgLaser("[Receive]"+asReceiveMsg);
        }
        else
            asReceiveMsg=DoSendCommand(false,asSendMsg);
        if(asReceiveMsg==""){/*wait receive*/}
        else
        {
            tsAnalysis->CommaText=asReceiveMsg;  //GS,4,1
            if(tsAnalysis->Count==3)
            {
                if(tsAnalysis->Strings[2]=="1")
                    return true;
                else
                    RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetAOIReturnError(),asReceiveMsg));
            }
            else//Error
                RecordErrorMsgLaser(GetErrorMsg(__FUNC__,GetReceiveWrongCommand(),asReceiveMsg));
            return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
void TfTrayMapFromLoader::AnalysisRowCol(AnsiString asInput,int &icow,int &irow)
{
    itemp=asInput.Pos("_");
    if(itemp==0)
    {
        icow=0;
        irow=0;
        return;
    }
    icow=atoi(asInput.SubString(0,itemp-1).c_str());
    irow=atoi(asInput.SubString(itemp+1,asInput.Length()).c_str());
}
//---------------------------------------------------------------------------
void TfTrayMapFromLoader::AnalysisTrayData(TTMyTray* mTray,AnsiString asRawData)
{
    //AI(W906-PT-W3) 20260807 GATE (1d): golden body (golden :1259-1279) reads
    //  mTray->YItem/->XItem and writes mTray->SetCellColorIndex() per cell --
    //  every statement below touches the incomplete TTMyTray type; see file
    //  banner GATE (1). Faithful default: no-op (nothing to paint offline).
#if 0
    int iIC=0;
    int iRow=mTray->YItem;
    int iCol=mTray->XItem;
    if(iCol==0)return;
    for(int i=0;i<asRawData.Length();i++)
    {
        if(iRow<=(i/iCol))
            return;
        //iIC=asRawData.SubString(i+1,1)=="1"?1:0;
        iIC=StrToIntDef(asRawData.SubString(i+1,1), 0);                         //RogerYang 20250907 : 新增翹料
        switch(iIC)
        {
            case 0:
                mTray->SetCellColorIndex(i%iCol, i/iCol, 0);
            break;
            case 1:
                mTray->SetCellColorIndex(i%iCol, i/iCol, 1);
            break;
            case 2:
                mTray->SetCellColorIndex(i%iCol, i/iCol, HAS_OCR_NG);
            break;
        }
    }
#else
    (void)mTray;
    (void)asRawData;
#endif
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::HasNullIC(TTMyTray* mTray)
{
    //AI(W906-PT-W3) 20260807 GATE (1e): golden body (golden :1285-1293) reads
    //  mTray->YItem/->XItem/->GetCellData() -- see file banner GATE (1).
    //  Faithful default: false, matching TfAOI's OWN base-class default
    //  (TfAOILaserScan.h golden :127).
#if 0
    for(int i=0;i<mTray->YItem;i++)
    {
        for(int j=0;j<mTray->XItem;j++)
        {
            if(mTray->GetCellData(i,j)==0)
                return true;
        }
    }
    return false;
#else
    (void)mTray;
    return false;
#endif
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoSettingBeforeMotorMove(bool bIsFirst,AnsiString &asError)
{
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        asError="";
        return true;
    }

    if(AOIDelay.Off())
        asError=AnsiString(__FUNC__)+" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=1000;
            return false;
        case 1000:
            if(DoCommuncation())
                iTDCTask=2000;
            return false;
        case 2000:
            DoAOI_LoadFile(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3000;
            return false;
        case 3000:
            if(DoAOI_LoadFile(false))
                iTDCTask=4000;
            return false;
        case 4000:
            DoAOI_ClearBuffer(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=5000;
            return false;
        case 5000:
            if(DoAOI_ClearBuffer(false))
                iTDCTask=6000;
            return false;
        case 6000:
            DoAOI_Function(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=7000;
            return false;
        case 7000:
            if(DoAOI_Function(false))
                iTDCTask=9999;
            return false;
        case 9999: //finish
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
}
//---------------------------------------------------------------------------
bool TfTrayMapFromLoader::DoGetValueAfterMotorArrival(bool bIsFirst,AnsiString &asError)
{
    if(bIsFirst)
    {
        iTDCTask=1;
        AOIDelay.SetSecAndOn(CommandDelay());
        return true;
    }

    if(AOIDelay.Off())
    {
        asError=AnsiString(__FUNC__)+" TimeOut "+IntToStr(CommandDelay())+" sec step="+IntToStr(iTDCTask);
    }

    if(asError!="")
        return true;
    switch(iTDCTask)
    {
        case 1:
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=1000;
            return false;
        case 1000:
            if(DoCommuncation())
                iTDCTask=2000;
            return false;
        case 2000:
            DoGetValue(true);
            AOIDelay.SetSecAndOn(CommandDelay());
            iTDCTask=3000;
            return false;
        case 3000:
            if(DoGetValue(false))
                iTDCTask=9999;
            return false;
        case 9999: //finish
            socketClient->Close();
            return true;
        default:
            asError=AnsiString(__FUNC__)+"，Get Result wrong step="+IntToStr(iTDCTask);
            return true;
    }
}
//---------------------------------------------------------------------------
