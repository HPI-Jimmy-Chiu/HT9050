// =============================================================================
//  PadInterface_St02.h  --  RS-232 operator pad (golden 0618 uPadInterface.h / .cpp, TfPadInterface + TPadRS232Thread)
//
//  AI(W906-ST02-P1) 20261005 (St02-E): card ST02-P1 (TO_STEVEN 1005 19:3x, W-80; EastSun 1005 19:1x
//  「較jimmy 先行轉 RS232 要測試了」).  Faithful translation of golden 0618 uPadInterface.cpp (962 lines) without
//  the VCL form.  Survey: docs/nb2_assist/RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md.
//
//  Two layers, so the six TMySensor / TMySwitch gates (mysensor.cpp / myswitch.cpp, library ht9045_io) get NO
//  new link dependency:
//    * this header (inline): the 31-key table (golden uPadInterface.cpp:189-243), the key state, and the four
//      methods the gates call -- IsPadButton / IsPadKey / ProcessScanKey / SendSwitchStatus(AnsiString,bool)
//      (golden :247-277, :476-490, :849-857).  Pure table look-ups; no I/O.
//    * PadInterface_St02.cpp (ht9045_sm): the protocol (Main232, ProcessReceiceData, DoScanPanelLed,
//      DoUpdataPadStatus, ProcessSendDataNew, RequestPadVersion, SendCommand, RecordCommunication), the serial
//      port owner (golden TdmTrayMotor::RS232Init / comTrayStepMotorReceiveData, Motor/TrayStepMotor.cpp:56-87 /
//      :402-513) and the serve-loop job that stands for TPadRS232Thread.
//
//  `fPadInterface` keeps golden's name so the gate lines read exactly as golden.  golden HT9045.cpp:103 creates
//  the form unconditionally (it always exists); here it is a function-local static built on first use
//  (no static-initialisation-order hazard, same object in every TU).  MinGW g++ 6.3 has no C++17 inline
//  variables, hence the inline function.
//
//  The form itself (31 TMyLed + 31 TBtnPanelLane + Memo + checkbox) is replaced by plain state:
//    TMyLed::Value / Tag           -> PAD_LED_W906::Value / Tag
//    TBtnPanelLane::Down / Tag / Enabled -> PAD_BTN_W906
//    cb_PadInterface_PadLedBling->Checked -> bPadLedBling (dfm: not checked = false)
//    Memo_PadInterface->Lines      -> MemoLines (RecordCommunication keeps golden's 1000-line rule)
//  Tags from golden uPadInterface.dfm: every Rear LED / button (and ml/sb_PadInterface_Rear) Tag=1, the front
//  ones have no Tag line = 0.
//  NOT translated (debug form only, not in the card): FormShow / FormClose / Exit / ManualSend / ClearLog1 /
//  PadButtonClick / SearchChangePageButton / FrontPowerOffMouseDown / [SendSwitchStatus(TBtnPanelLane*) -- translated by AI(W906-W155) 20261007 (St02-E), W-155: SendSwitchStatus(PAD_BTNLANE_W906*)] (the IO
//  page caller, golden iosetview.cpp) / SendCommand(vector<Byte>&) (0 callers in golden).  bShow therefore stays
//  false, as golden while the debug form is closed.
// =============================================================================
#ifndef PadInterface_St02H
#define PadInterface_St02H

#include "vclcompat/vcl_compat.h"
#include <vector>

extern bool bSafeLockStatus;                                                    // cmydef.cpp (golden: written by InitialVariable only)

//---------------------------------------------------------------------------   golden uPadInterface.h:22-43
#define PAD_ControlDLC      0x04
#define PAD_FrontControl    0x00
#define PAD_RearControl     0x01
#define PAD_LedLight        0x00
#define PAD_LedBling        0x01
#define PAD_PowerOff        0x000001                 //00000000000000001
#define PAD_PowerOn         0x000002                 //00000000000000010
#define PAD_PannelEnable    0x000004                 //00000000000000100
#define PAD_Reset           0x000008                 //00000000000001000
#define PAD_Pause           0x000010                 //00000000000010000
#define PAD_Home            0x000020                 //00000000000100000
#define PAD_Start           0x000040                 //00000000001000000
#define PAD_OneCycle        0x000080                 //00000000010000000
#define PAD_Retry           0x000100                 //00000000100000000
#define PAD_Skip            0x000200                 //00000001000000000
#define PAD_CleanOut        0x000400                 //00000010000000000
#define PAD_TrayFeed        0x000800                 //00000100000000000
#define PAD_TrayEnd         0x001000                 //00001000000000000
#define PAD_AlarmReset      0x002000                 //00010000000000000
#define PAD_SafeLock        0x004000                 //00100000000000000
#define PAD_Step            0x008000                 //01000000000000000
#define PAD_TStart          0x010000                 //10000000000000000

struct PAD_LED_W906 { bool Value; int Tag; PAD_LED_W906() : Value(false), Tag(0) {} };              // TMyLed: Value (TALed default false), Tag (dfm)
struct PAD_BTN_W906 { bool Down; int Tag; bool Enabled; PAD_BTN_W906() : Down(false), Tag(0), Enabled(false) {} };   // TBtnPanelLane
struct PAD_BTNLANE_W906 { AnsiString Alias; bool Down; };   // AI(W906-W155) 20261007 (St02-E): what golden SendSwitchStatus(TBtnPanelLane *bpPtr) reads of the IO page button (Alias, Down); occupies the old blank line
//===========================================================================   golden uPadInterface.h:45-54
typedef struct
{
    PAD_LED_W906    *mlEvent;
    PAD_BTN_W906    *btnEvent;
    AnsiString      PadName;
    int             iData;
    AnsiString      InputName;

    void SetItem(PAD_LED_W906 *_mlEvent, PAD_BTN_W906 *_btnEvent, AnsiString _PadName, int _iData, AnsiString _InputName)
    {                                                                           // golden uPadInterface.cpp:51-59
        mlEvent =_mlEvent;
        btnEvent=_btnEvent;
        PadName =_PadName;
        iData   =_iData;
        InputName=_InputName;
        btnEvent->Enabled=true;
    }
} PAD_PTR;

//---------------------------------------------------------------------------   golden uPadInterface.h:67-214
class TfPadInterface
{
private:
    enum ePadName                                                               //Jimmychiu 20230718 : Fixed for 三面板控制
    {   epn_SnFKPowerOff=0,     epn_SnFKPowerOn=1,      epn_SnFrontPadActive=2, epn_SnFKReset=3,        epn_SnFKPause=4,
        epn_SnFKHome=5,         epn_SnFKStart=6,        epn_SnFKOneCycle=7,     epn_SnFKRetry=8,        epn_SnFKSkip=9,
        epn_SnFKCleanOut=10,    epn_SnFKTrayFeed=11,    epn_SnFKTrayEnd=12,     epn_SnFKAlarmReset=13,  epn_SnRKPowerOff=14,
        epn_SnRKPowerOn=15,     epn_SnRKReset=16,       epn_SnRKPause=17,       epn_SnRKHome=18,        epn_SnRKStart=19,
        epn_SnRKOneCycle=20,    epn_SnRKRetry=21,       epn_SnRKSkip=22,        epn_SnRKCleanOut=23,    epn_SnRKTrayFeed=24,
        epn_SnRKTrayEnd=25,     epn_SnRKAlarmReset=26,  epn_SnRKSafeLock=27,    epn_SnRKManualStep=28,  epn_SnRKManualTStart=29,
        epn_SnRearPadActive=30, epn_Total
    };
    AnsiString  aParaPath;                                                      // golden :191, assigned only (dead in golden too)
    AnsiString  aReciveData;
    AnsiString  aSendData;
    void PadWriteDataToFile(AnsiString cFilePath, AnsiString cData);           // PadInterface_St02.cpp

public:
    TfPadInterface() : bRs232Ok(false), CheckPadItem(0), bShow(false), bRequestVer(false), bScanSwitch(false),
                       bSendSwitchStatusing(false), bPadLedBling(false)
    {                                                                           // golden :152-157 (SearchChangePageButton = form only)
        for(int i=0; i<32; i++) bPadStatus[i]=false;                            // golden relied on VCL zero-fill (survey T6)
        InitialVariable();
    }

    bool                    bRs232Ok;                                           // golden never initialises it (VCL zero-fill): false
    std::vector<AnsiString> CommReceiveList;                                    // golden TStringList; filled by the port drain (main thread)
    std::vector<int>        CommReceiveLength;
    std::vector<AnsiString> SendData;                                           // golden vector<String>
    std::vector<AnsiString> RequestData;
    PAD_PTR                 PadItem[32];
    int                     CheckPadItem;
    bool                    bPadStatus[32];
    bool                    bShow;
    bool                    bRequestVer;
    bool                    bScanSwitch;                                        //KenHsieh 20211224 : 新增按鈕狀態掃描
    bool                    bSendSwitchStatusing;

    // the form's controls (see the file header)
    PAD_LED_W906            ml[32];
    PAD_BTN_W906            sb[32];
    bool                    bPadLedBling;                                       // cb_PadInterface_PadLedBling->Checked
    std::vector<AnsiString> MemoLines;                                          // Memo_PadInterface->Lines

    //======================================================================== golden :189-243
    void InitialVariable()
    {
        aParaPath   = "D:\\HT9045\\System\\PadInterfacePara.ini";

        bShow      =false;
        bRequestVer=false;
        bScanSwitch=true;                                                       //KenHsieh 20211224 : 新增按鈕狀態掃描

        bSendSwitchStatusing=false;

        CommReceiveList.clear();
        CommReceiveLength.clear();
        bSafeLockStatus=false;

        for(int i=0; i<31; i++)                                                 // dfm Tag: rear LEDs / buttons 1, front 0
        {
            ml[i].Tag=(i>=epn_SnRKPowerOff) ? 1 : 0;
            sb[i].Tag=(i>=epn_SnRKPowerOff) ? 1 : 0;
        }
        PadItem[ 0].SetItem(&ml[ 0], &sb[ 0], "SwFKPowerOff"        , PAD_PowerOff      ,"SnFKPowerOff");
        PadItem[ 1].SetItem(&ml[ 1], &sb[ 1], "SwFKPowerOn"         , PAD_PowerOn       ,"SnFKPowerOn");
        PadItem[ 2].SetItem(&ml[ 2], &sb[ 2], "SwFrontActiveLed"    , PAD_PannelEnable  ,"SnFrontPadActive");   //KenHsieh 20211221 : Front新增Enable燈號
        PadItem[ 3].SetItem(&ml[ 3], &sb[ 3], "SwFKReset"           , PAD_Reset         ,"SnFKReset");
        PadItem[ 4].SetItem(&ml[ 4], &sb[ 4], "SwFKPause"           , PAD_Pause         ,"SnFKPause");
        PadItem[ 5].SetItem(&ml[ 5], &sb[ 5], "SwFKHome"            , PAD_Home          ,"SnFKHome");
        PadItem[ 6].SetItem(&ml[ 6], &sb[ 6], "SwFKStart"           , PAD_Start         ,"SnFKStart");
        PadItem[ 7].SetItem(&ml[ 7], &sb[ 7], "SwFKOneCycle"        , PAD_OneCycle      ,"SnFKOneCycle");
        PadItem[ 8].SetItem(&ml[ 8], &sb[ 8], "SwFKRetry"           , PAD_Retry         ,"SnFKRetry");
        PadItem[ 9].SetItem(&ml[ 9], &sb[ 9], "SwFKSkip"            , PAD_Skip          ,"SnFKSkip");
        PadItem[10].SetItem(&ml[10], &sb[10], "SwFKCleanOut"        , PAD_CleanOut      ,"SnFKCleanOut");
        PadItem[11].SetItem(&ml[11], &sb[11], "SwFKTrayFeed"        , PAD_TrayFeed      ,"SnFKTrayFeed");
        PadItem[12].SetItem(&ml[12], &sb[12], "SwFKTrayEnd"         , PAD_TrayEnd       ,"SnFKTrayEnd");
        PadItem[13].SetItem(&ml[13], &sb[13], "SwFKAlarmReset"      , PAD_AlarmReset    ,"SnFKAlarmReset");
        PadItem[14].SetItem(&ml[14], &sb[14], "SwRKPowerOff"        , PAD_PowerOff      ,"SnRKPowerOff");
        PadItem[15].SetItem(&ml[15], &sb[15], "SwRKPowerOn"         , PAD_PowerOn       ,"SnRKPowerOn");
        PadItem[16].SetItem(&ml[16], &sb[16], "SwRKReset"           , PAD_Reset         ,"SnRKReset");
        PadItem[17].SetItem(&ml[17], &sb[17], "SwRKPause"           , PAD_Pause         ,"SnRKPause");
        PadItem[18].SetItem(&ml[18], &sb[18], "SwRKHome"            , PAD_Home          ,"SnRKHome");
        PadItem[19].SetItem(&ml[19], &sb[19], "SwRKStart"           , PAD_Start         ,"SnRKStart");
        PadItem[20].SetItem(&ml[20], &sb[20], "SwRKOneCycle"        , PAD_OneCycle      ,"SnRKOneCycle");
        PadItem[21].SetItem(&ml[21], &sb[21], "SwRKRetry"           , PAD_Retry         ,"SnRKRetry");
        PadItem[22].SetItem(&ml[22], &sb[22], "SwRKSkip"            , PAD_Skip          ,"SnRKSkip");
        PadItem[23].SetItem(&ml[23], &sb[23], "SwRKCleanOut"        , PAD_CleanOut      ,"SnRKCleanOut");
        PadItem[24].SetItem(&ml[24], &sb[24], "SwRKTrayFeed"        , PAD_TrayFeed      ,"SnRKTrayFeed");
        PadItem[25].SetItem(&ml[25], &sb[25], "SwRKTrayEnd"         , PAD_TrayEnd       ,"SnRKTrayEnd");
        PadItem[26].SetItem(&ml[26], &sb[26], "SwRKAlarmReset"      , PAD_AlarmReset    ,"SnRKAlarmReset");
        PadItem[27].SetItem(&ml[27], &sb[27], "SwRKSafeLock"        , PAD_SafeLock      ,"SnRKSafeLock");       //SnRKCoverOpen  SnSafeLock   //KenHsieh 20211228 : 區分實體IO與通訊面板
        PadItem[28].SetItem(&ml[28], &sb[28], "SwRKManualStep"      , PAD_Step          ,"SnRKManualStep");
        PadItem[29].SetItem(&ml[29], &sb[29], "SwRKManualTStart"    , PAD_TStart        ,"SnRKManualTStart");
        PadItem[30].SetItem(&ml[30], &sb[30], "SwRearActiveLed"     , PAD_PannelEnable  ,"SnRearPadActive");    //KenHsieh 20211221 : Front新增Enable燈號
        CheckPadItem=31;                                                        //KenHsieh 20211221 : Front新增Enable燈號，30->31
    }
    //======================================================================== golden :247-260
    bool IsPadButton(AnsiString aName)
    {
        bool bFind=false;

        for(int i=0; i<CheckPadItem; i++)
        {
            if(AnsiString(PadItem[i].PadName)==aName)
            {
                bFind=true;
                break;
            }
        }
        return bFind;
    }
    //======================================================================== golden :264-277
    bool IsPadKey(AnsiString aName)
    {
        bool bFind=false;

        for(int i=0; i<CheckPadItem; i++)
        {
            if(AnsiString(PadItem[i].InputName)==aName)
            {
                bFind=true;
                break;
            }
        }
        return bFind;
    }
    //======================================================================== golden :476-490
    void SendSwitchStatus(AnsiString aName, bool Type)
    {
        if(bShow)
            return;

        for(int i=0; i<CheckPadItem; i++)
        {
            if(AnsiString(PadItem[i].PadName)==aName)                           //  && bPadStatus[i] != Type )
            {
                bPadStatus[i]=Type;
            }
        }
    }
    //======================================================================== golden :849-857
    //  golden quirk kept: Pos (not ==), and TMySwitch::Status passes an Sw… name that no Sn… InputName contains,
    //  so a pad button's Status() reads false (survey §2).
    bool ProcessScanKey(AnsiString aSenName)
    {
        for(int i=0; i<CheckPadItem; i++)
        {
            if(AnsiString(PadItem[i].InputName).Pos(aSenName)>0)
                return PadItem[i].mlEvent->Value;
        }
        return false;
    }

    // ---- PadInterface_St02.cpp (protocol + port; ht9045_sm) ----
    void ResetComm();                                                           // golden :378-383
    bool OpenCommPort();                                                        // golden :281-315
    bool CloseCommPort();                                                       // golden :319-334
    void SendCommand(AnsiString sData);                                         // golden :492-510
    void ProcessReceiceData();                                                  // golden :696-749
    void DoScanPanelLed(int iAddress, int iKey);                                // golden :545-665
    void DoUpdataPadStatus(int iAddress, int iKey);                             // golden :667-694
    void Main232();                                                             // golden :861-938
    bool ProcessSendDataNew();                                                  // golden :751-847
    void RequestPadVersion();   void SendSwitchStatus(PAD_BTNLANE_W906 *bpPtr);   // golden :396-415 / AI(W906-W155) 20261007 (St02-E): SendSwitchStatus golden :417-474 (body PadInterface_St02.cpp EOF)
    void RecordCommunication(AnsiString aTitle, AnsiString Command);            // golden :356-376
};

inline TfPadInterface* W906_PadInterfaceObject()                                // golden HT9045.cpp:103 CreateForm (always exists)
{
    static TfPadInterface o;
    return &o;
}
#define fPadInterface (W906_PadInterfaceObject())

// ---- port owner + serve-loop job (PadInterface_St02.cpp) ----
void W906_PadPortRS232Init(AnsiString ComPort);                                 // golden TdmTrayMotor::RS232Init (Motor/TrayStepMotor.cpp:56-87)
void W906_RS232InitPadCheck(bool *flag);                                        // golden rs232.cpp:221-234 (TCOM2::RS232Init)
void W906_PadThreadTick();                                                      // TPadRS232Thread::RS232ThreadProcess (golden :36-40) incl. the receive drain
bool W906_PadIoPoint(const AnsiString& alias, bool* on);                        // AI(W906-W155) 20261007 (St02-E): the IO page Panel squares (golden iosetview.cpp ScanLed :2778-2782); PadInterface_St02.cpp EOF; occupies the old blank line
#endif // PadInterface_St02H
