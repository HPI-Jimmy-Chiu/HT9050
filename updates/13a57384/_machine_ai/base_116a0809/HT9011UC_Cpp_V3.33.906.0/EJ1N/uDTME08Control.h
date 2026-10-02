// =============================================================================
//  uDTME08Control.h  --  Panasonic DTME08 8-channel-per-station Modbus/TCP
//                         temperature controller driver (up to 4 stations,
//                         32 channels total).
//
//  Faithful translation of golden EJ1N/uDTME08Control.h (145 lines, BCB6,
//  Big5/cp950 -- this header carries no Chinese comments; the Chinese text
//  in the //-comments is JimmyChiu/Steven attribution prose, all ASCII
//  except the em-dash-like "：" nowhere present -- confirmed by decoding
//  the raw bytes as cp950: every comment line is plain ASCII).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 group "ej1n".  Second EJ1N/ file to land in
//  this wave (after uSocketServerClient.cpp/.h).
//
//  ROLE: uDTME08Info is a plain per-channel value/status record (32 of them
//  live in mapDTMInfo, one per physical channel across all 4 stations).
//  uDTME08Control owns a uSocketClient transport + a uModbusCommand frame
//  codec and drives a family of 2-phase (bIsFirst true=send, false=poll-
//  receive) Do* state steppers -- DoGetSV/DoGetPV/DoGetStatus (reads) and
//  DoSetValue+its 5 DoSetXxx callers (writes) -- exactly the same
//  "GetClientEncodeXxx -> SendCommandSocket -> wait for iReceiveLen ->
//  DecodeToListIntTCP" shape repeated per operation.
//
//  WAVE SCOPE -- ACTIVE vs GATED:
//   ACTIVE (faithful, verbatim): every member of uDTME08Info; every member
//     of uDTME08Control EXCEPT GetSettingFilePath (GATE 1 below).
//   GATE (#if 0): 1 -- registered below, re-cited at its own definition site
//     in the header (it is a header inline, same as golden).
//
//  GATE REGISTER
//   (1) `AnsiString GetSettingFilePath(){return
//       ExtractFilePath(Application->ExeName);}` (golden :54) --
//       `Application` (the VCL TApplication singleton, `->ExeName`) has NO
//       port anywhere in this tree. This is an ALREADY-ESTABLISHED gate:
//       cpublic.cpp:2147 gates 3 whole functions on the identical missing
//       surface ("TODO(GA1-B3): blocked by Application->ExeName..."), and
//       database.cpp:401 hits the same wall independently. ACTIVE default:
//       golden's OWN commented-out fallback, sitting right next to the gated
//       line in golden itself (golden :55, `//    AnsiString
//       GetSettingFilePath(){return "D:\\HT9045\\system\\";}`) -- this is
//       the golden AUTHOR's own documented substitute for this exact
//       situation, not a value this translation invented. Re-cited at the
//       definition below.
//
//  VCL/Borland conversions: `typedef void (__closure *TPointVoidShowMsg)
//  (AnsiString);` (golden :9) -- __closure has no standard-C++ spelling;
//  translated to `std::function<void(AnsiString)>`, the SAME choice EJ1N
//  sibling BarCode_Helpers.h already made for its own (textually identical)
//  TPointVoidShowMsg typedef -- redeclaring the same typedef name to the
//  same underlying type in two headers is legal C++ (not an ODR conflict)
//  even if some future TU ends up including both.  `byte` (golden :67)
//  resolves via <windows.h> -> rpcndr.h, same path every other EJ1N unit
//  uses.  `std::map<int,uDTME08Info>` (golden :138) needs <map>, included
//  explicitly below for this header's self-containment (MachineDefine.h
//  also carries it, but golden's own header did not rely on that -- this
//  port keeps the header self-contained per house style).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8 (in
//  practice, this golden header's comments are all plain ASCII English/
//  pinyin names -- confirmed, nothing to transliterate). Final gate: ZERO
//  U+FFFD.
// =============================================================================

// BCB6 ORIGINAL header (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #ifndef uDTME08ControlH / #define uDTME08ControlH
//   #include "uSocketServerClient.h"
// =============================================================================

//---------------------------------------------------------------------------

#ifndef uDTME08ControlH
#define uDTME08ControlH
//---------------------------------------------------------------------------
#include "EJ1N/uSocketServerClient.h"
#include <map>
#include <functional>
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//AI(W906-PT-W2) 20260807: golden `typedef void (__closure *TPointVoidShowMsg)
//  (AnsiString);` -- __closure has no standard-C++ spelling; translated per
//  vcl_compat.h's documented convention (same choice EJ1N sibling
//  BarCode_Helpers.h already made for its own TPointVoidShowMsg).
typedef std::function<void(AnsiString)> TPointVoidShowMsg;
#define swap16(x) ((((x) & 0xFF)<<8) | (((x)>>8) & 0xFF))
//---------------------------------------------------------------------------
#define SendCommLength 200
//---------------------------------------------------------------------------
class uDTME08Info
{
private:
public: // User declarations
    uDTME08Info()
    {
        InitialDatas();
    }
    void InitialDatas()
    {
        PV=SV=0.0;
        SetSV=SensorType=CycleTime=ControlActionOut2=0;
        RunAutoTunning=RealAutoTunning=Alarm3=Alarm2=IsCelsius=IsFahrenheit=Alarm1=IsRunOutput2=IsRunOutput1=IsRunAutoTuning=false;
        Event="";
    }
    double PV;
    AnsiString Event;
    double SV;
    int SetSV;
    void SetSettingSV(double dvalue){SetSV=(int)(dvalue*10);}
    bool RunAutoTunning;
    bool RealAutoTunning;
    int SensorType;
    int CycleTime;
    void SetCycleTime(double dvalue){CycleTime=dvalue*10;}
    int ControlActionOut2;
    //status
    bool Alarm3;
    bool Alarm2;
    bool IsCelsius;
    bool IsFahrenheit;
    bool Alarm1;
    bool IsRunOutput2;
    bool IsRunOutput1;
    bool IsRunAutoTuning;
};
class uDTME08Control
{
private:    // User declarations
    uSocketClient *socketClient;
    //AI(W906-PT-W2) 20260807: GATE (1) -- golden
    //  `return ExtractFilePath(Application->ExeName);` (golden :54).
    //  Application->ExeName has no port anywhere in this tree (already-
    //  established gate, cpublic.cpp:2147 / database.cpp:401). ACTIVE
    //  default is golden's OWN commented-out fallback (golden :55). See
    //  this header's own banner GATE (1).
    AnsiString GetSettingFilePath()
    {
#if 0
        return ExtractFilePath(Application->ExeName);
#else
        return "D:\\HT9045\\system\\";
#endif
    }
//    AnsiString GetSettingFilePath(){return "D:\\HT9045\\system\\";}
    AnsiString GetSettingFileName(){return "DTME08_Control.ini";}
    AnsiString GetSettingFileNameWithPath(){return GetSettingFilePath()+GetSettingFileName();}
    bool bflag;
    //SendComm
    char strSendUse[SendCommLength];
    int iCommandLen;
    //Receive
    char* cReceive;
    int iReceiveLen;
    AnsiString tempStr;
    int tempInt;
    byte tempByte;
    int GetMinReceiveLen(){return 9+GetChannelNumberPerStation()*2;}
    void GetStatus(int iInput,uDTME08Info *uinfo);
    void ClearClientComm();
    bool bDebug;
public: // User declarations
    //enum
    enum eSetSensorType
    {
        esstKType=0,
        esstPT100=12,
    };
    enum eFunctionCode
    {
        efc_SetSV=0,
        efc_SetAT,
        efc_GetSV,
        efc_GetPV,
        efc_GetStatus,
        efc_SetSensorType,
        efc_SetCycleTime,
        efc_SetControlActionOut2,
    };
    enum eAutoTune
    {
        eatTrunStop=0,
        eatTrunRunning,
    };
    enum eControlAction
    {
        ecaHeating=0,
        ecaCooling,
        ecaDisable,
    };
    //
    uDTME08Control();
    ~uDTME08Control();
    void Initialization();
    TPointVoidShowMsg RecordMsg;
    TPointVoidShowMsg RecordErrorMsg;
    int GetMaxStationNumber(){return 4;}
    int GetChannelNumberPerStation(){return 8;}
    //socket
    void ReceiveData(char* cGet,int iLen);
    bool IsConnected(){return socketClient->IsConnected();}
    bool DoCommuncation(){return socketClient->DoOpenCommuncation();}
    void DoSocketClose(){socketClient->Close();}
    AnsiString GetSocketAddress(){return socketClient->GetSocketAddress();}
    AnsiString GetSocketPort(){return socketClient->GetSocketPort();}
    void SetSocketAddress(AnsiString asValue);
    void SetSocketPort(AnsiString asValue);
    void ReadSettingFile();
    void WriteSettingFile();
    void SendCommandSocket(char* cSet,int iLen);
    void AnalysisChannelStatus(int iInput,bool **arrBool,int iBoolLen);
    //
    int GetStationCode(int istate);
    AnsiString Chararr2Hexstring(char* cstr,int iNum);
    int GetFunctionCode(eFunctionCode fc);
    //
    bool DoGetSV(bool bIsFirst,int iStation);
    bool DoGetPV(bool bIsFirst,int iStation);
    bool DoGetStatus(bool bIsFirst,int iStation);
    //
    bool DoSetValue(eFunctionCode fc,bool bIsFirst,int iStation);
    bool DoSetSV(bool bIsFirst,int iStation);
    bool DoSetAT(bool bIsFirst,int iStation);
    bool DoSetSensorType(bool bIsFirst,int iStation);
    bool DoSetCycleTime(bool bIsFirst,int iStation);
    bool DoSetControlActionOut2(bool bIsFirst,int iStation);
    //
    std::map<int,uDTME08Info> mapDTMInfo;
    int GetSizeDTMEInfo(){return mapDTMInfo.size();}
    void InitialmapDTMInfoList();
    AnsiString ErrorCodeDescription(byte bdata);
    bool ReceiveInitial();
};
#endif
