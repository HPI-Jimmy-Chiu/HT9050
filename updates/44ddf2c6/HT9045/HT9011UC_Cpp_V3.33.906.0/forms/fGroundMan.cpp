// =============================================================================
//  forms/fGroundMan.cpp  --  TfGroundMan bodies that fit the ht9045_forms
//  link diet (vclcompat + ht9045_globals + ht9045_core only -- every ACTIVE
//  line below stays inside that diet, verified per DEPENDENCY AUDIT in
//  forms/fGroundMan.h).
//
//  AI(W906-FW3-LGM1) 20260828: new file, wave FW3-LGM1.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/GroundMan/
//  GroundMan.cpp (1,667 lines, cp950, 0 U+FFFD, this wave). Full banner --
//  ROLE, FIELD SCOPING (why most of the golden class is not declared),
//  WAVE SCOPE table (9/18 delivered), GATE REGISTER (GM-0..GM-4), GOLDEN
//  NOTES, DEVIATIONS, DEPENDENCY AUDIT, CMAKE STATUS -- lives in
//  forms/fGroundMan.h; this file carries per-statement pointer-back
//  comments only.
//
//  CMAKE STATUS -- NOT WIRED by this wave. See forms/fGroundMan.h.
// =============================================================================
#include "forms/fGroundMan.h"
#include "database.h"      // HSys (:357), .iGroundManScanPoint (:291)
#include "cmydef.h"         // CUSTOMER_CODE (:3181), AccessLevel (:3503),
                             // iDefHonPrecLevel (:3588), InitialOK (:220),
                             // USE_GROUND_MAN (:3031), N_INTEGER (:289)
#include "MachineType.h"    // CC_KYEC_LEE (:292), CC_SIGURD_PeiXing (:319),
                             // CC_SIGURD_HUKOU (:314)
#include "forms/fQwertyKey.h"   // fQwertyKey global + ShowQwertyKey
#include <cstring>           // memcpy (calc_crc)

// golden GroundMan.cpp:20 -- constructed via `new` (header GLOBAL POINTER
// note); ctor-safe, see below.
TfGroundMan *fGroundMan = new TfGroundMan();

//---------------------------------------------------------------------------
// AI(W906-FW3-LGM1) 20260828: ctor PORT SPLIT -- fields + this object's own
// `new`'d widget stand-ins (in-class initializers, header) + the golden
// ctor statements that touch ONLY this object's own fields/widgets (no
// other global dereferenced). The HSys-dependent iUseGndBoard computation
// and the ReadGroundOffset() call (golden :40-51, :136) move to Init() --
// see header CTOR/Init() SPLIT.
TfGroundMan::TfGroundMan()
{
    iGroundMasterTask = 1;     // golden :33
    bRs232Ok          = false; // golden :34
    fShow             = false; // golden :35
    // golden :36-38 (bGroundManReset/bGroundManResetByStart/bUseOffset) --
    // NOT translated: no ACTIVE consumer this wave (see header FIELD
    // SCOPING), fields not declared.

    // golden :53-59, :61-85 (bOpenClose[][] init) -- NOT translated: field
    // not declared, no ACTIVE consumer.

    // golden :86-124 -- asShowName[][] literal assignments, translated
    // verbatim. golden's preceding `ZeroMemory(asShowName,...)` (:31) is
    // dropped, not translated -- see header DEVIATION (D-5).
    asShowName[0][0]="InArm";
    asShowName[0][1]="OutArm";
    asShowName[0][2]="Arm1";
    asShowName[0][3]="Arm2";
    asShowName[0][4]="HP1";
    asShowName[0][5]="HP2";
    asShowName[0][6]="SH1";
    asShowName[0][7]="SH2";

    //KaiChen 20200707 ：修改 GroundMan 對點顯示的位置
    //==>
    asShowName[1][0]="InArm_A";
    asShowName[1][1]="InArm_C";
    asShowName[1][2]="InArm_E";
    asShowName[1][3]="InArm_G";
    asShowName[1][4]="InArm_B";
    asShowName[1][5]="InArm_D";
    asShowName[1][6]="InArm_F";
    asShowName[1][7]="InArm_H";

    asShowName[2][0]="OutArm_A";
    asShowName[2][1]="OutArm_C";
    asShowName[2][2]="OutArm_E";
    asShowName[2][3]="OutArm_G";
    asShowName[2][4]="OutArm_B";
    asShowName[2][5]="OutArm_D";
    asShowName[2][6]="OutArm_F";
    asShowName[2][7]="OutArm_H";
    //<==
    //KaiChen 20200707 ：修改 GroundMan 對點顯示的位置

    asShowName[3][0]="Loader";  //JerryYang 20210428 : Add 28點接地功能
    asShowName[3][1]="Empty";
    asShowName[3][2]="Color";
    asShowName[3][3]="Auto1";
    asShowName[3][4]="Auto2";
    asShowName[3][5]="Auto3";
    asShowName[3][6]="Ch6";
    asShowName[3][7]="Ch7";

    asGroundVaule="NA";   // AI(W906-W2-3) 20260926: golden :125 -- now declared (RecordTimeInfo reads it).  Was the comment: golden :125 (asGroundVaule="NA";), :126-131 (sCMD[]=...) -- NOT
    // translated: neither field is declared, no ACTIVE consumer this wave.   [20260926: asGroundVaule half STALE, see the line above; sCMD[] still not translated]

    // golden :132-133 (bInternalOhm/bVersion) -- NOT translated: no ACTIVE
    // consumer, fields not declared.

    Timer1->Interval = 30;   // golden :134 -- this object's own widget, ctor-safe

    // golden :135 (iMachineOhmRetry=0;) -- NOT translated: field not declared.
}
//---------------------------------------------------------------------------
// AI(W906-FW3-LGM1) 20260828: the ctor's deferred remainder -- golden :40-51
// (HSys-dependent iUseGndBoard) + :136 (ReadGroundOffset(), GATE (GM-1)).
// Not called anywhere yet (no wiring this wave) -- must run once, before
// FormShow, whenever a future wave wires this form's lifecycle.
void TfGroundMan::Init()
{
    // AI(W906-FW3-LGM1) 20260828: GATE (GM-0) -- golden :40-51 branches on
    // the GLOBAL `HSys.iGroundManScanPoint` (database.h:357/:291). HSys's
    // only definition is database.cpp, which CMakeLists.txt:871 places in
    // ht9045_db (`target_link_libraries(ht9045_db PUBLIC vclcompat
    // ht9045_globals)`, CMakeLists.txt:892 -- ht9045_globals/ht9045_core do
    // NOT link back to ht9045_db). ht9045_forms links ONLY
    // vclcompat+ht9045_globals+ht9045_core (CMakeLists.txt:822-829) and
    // HSys is NOT one of the four verified cross-diet exceptions
    // (RecordProcess/ShowMyMessage/ShowErrorMessage/MyDBIProcess) -- this
    // is the FIRST forms/*.cpp file that would otherwise reference HSys
    // (`grep "HSys\." forms/*.cpp` -- 0 hits elsewhere, this wave). Gated
    // whole; iUseGndBoard stays at its field default (0) until a future
    // wave either adds an ht9045_forms->ht9045_db edge or mirrors
    // iGroundManScanPoint into something ht9045_forms can reach. See
    // header GATE REGISTER (GM-0).
#if 0
    if(HSys.iGroundManScanPoint==0)  //JerryYang 20210428 : Add 28點接地功能
    {
        iUseGndBoard=1;
    }
    else if(HSys.iGroundManScanPoint==1)
    {
        iUseGndBoard=3;
    }
    else
    {
        iUseGndBoard=4;
    }
#endif

    // AI(W906-FW3-LGM1) 20260828: GATE (GM-1) -- ReadGroundOffset (golden
    // GroundMan.cpp:1572-1621) is EXCLUDED this wave (CheckAndReadIniData
    // write-back into the shared D:\HT9045\system\GroundMan.ini). Not
    // declared/defined; the call is gated in place. See header GATE
    // REGISTER (GM-1).
#if 0
    ReadGroundOffset();
#endif
}
//---------------------------------------------------------------------------
void TfGroundMan::FormShow()                                                   // golden :139-226 -- `TObject *Sender` dropped, never read; GATE (GM-4) inside
{
    Left=100;   //Steven 20220407 : 避免視窗跑到畫面外
    Top=100;
    TLabel *LabelShowPtr[4][8]={
                                {labCH_0_0, labCH_0_1, labCH_0_2, labCH_0_3,
                                labCH_0_4, labCH_0_5, labCH_0_6, labCH_0_7},
                                {labCH_1_0, labCH_1_1, labCH_1_2, labCH_1_3,
                                labCH_1_4, labCH_1_5, labCH_1_6, labCH_1_7},
                                {labCH_2_0, labCH_2_1, labCH_2_2, labCH_2_3,
                                labCH_2_4, labCH_2_5, labCH_2_6, labCH_2_7},  //JerryYang 20210428 : Add 28點接地功能
                                {labCH_3_0, labCH_3_1, labCH_3_2, labCH_3_3,
                                labCH_3_4, labCH_3_5, labCH_3_6, labCH_3_7}
                                };

    // AI(W906-FW3-LGM1) 20260828: GATE (GM-4) -- golden :154-206, the
    // HSys.iGroundManScanPoint three-way branch. Same HSys/ht9045_db
    // link-boundary reason as GATE (GM-0) (see Init()) -- this file cannot
    // reach ht9045_db. Every widget this block ALONE would touch
    // (gbBoard0-3, labCH_0_0/labCH_0_1's ->Visible writes, led_0_0/led_0_1,
    // labValue_0_0/labValue_0_1, labCount_0_0/labCount_0_1,
    // labBoardOhrm/2/3/4, labBoardVersion/2/3/4) is consequently NOT
    // declared on this class -- see header FIELD SCOPING. Gated whole;
    // every board group box/label stays at its construction-time default
    // (Visible=false) until a future wave resolves GATE (GM-0)/(GM-4)
    // together (same missing dependency, should open in the same change).
    // golden :185-196's own commented-out sibling lines (labCH_0_0 etc.,
    // iGroundManScanPoint==2 branch) are kept as literal comments below,
    // matching golden's own dead-but-visible authorial intent (header
    // GOLDEN NOTE (G-GM2)).
#if 0
    if(HSys.iGroundManScanPoint==0) // 8點接地
    {
        gbBoard0->Visible=true;
        gbBoard1->Visible=false;
        gbBoard2->Visible=false;
        gbBoard3->Visible=false;
        labBoardOhrm->Visible=true;
        labBoardVersion->Visible=true;
    }
    else if(HSys.iGroundManScanPoint==1) // 22點接地
    {
        gbBoard0->Visible=true;
        gbBoard1->Visible=true;
        gbBoard2->Visible=true;
        gbBoard3->Visible=false;
        labCH_0_0->Visible=false;
        labCH_0_1->Visible=false;
        led_0_0->Visible=false;
        led_0_1->Visible=false;
        labValue_0_0->Visible=false;
        labValue_0_1->Visible=false;
        labCount_0_0->Visible=false;
        labCount_0_1->Visible=false;

        labBoardOhrm->Visible=true;
        labBoardOhrm2->Visible=true;
        labBoardOhrm3->Visible=true;
        labBoardVersion->Visible=true;
        labBoardVersion2->Visible=true;
        labBoardVersion3->Visible=true;
    }
    else if(HSys.iGroundManScanPoint==2) //JerryYang 20210428 : Add 28點接地功能
    {
        gbBoard0->Visible=true;
        gbBoard1->Visible=true;
        gbBoard2->Visible=true;
        gbBoard3->Visible=true;
//        labCH_0_0->Visible=false;
//        labCH_0_1->Visible=false;
//        led_0_0->Visible=false;
//        led_0_1->Visible=false;
//        labValue_0_0->Visible=false;
//        labValue_0_1->Visible=false;

        labBoardOhrm->Visible=true;
        labBoardOhrm2->Visible=true;
        labBoardOhrm3->Visible=true;
        labBoardOhrm4->Visible=true;
        labBoardVersion->Visible=true;
        labBoardVersion2->Visible=true;
        labBoardVersion3->Visible=true;
        labBoardVersion4->Visible=true;
    }
#endif

    for(int i=0; i<iUseGndBoard; i++)   // ACTIVE, but permanently 0 iterations
    {                                    // while GATE (GM-0) is closed --
        for(int j=0; j<8; j++)          // iUseGndBoard's only writer is
        {                                 // gated. See header GATE REGISTER
            LabelShowPtr[i][j]->Caption=asShowName[i][j];   // (GM-0).
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        edContinuous_Time->Enabled=false;
        edOccurrences->Enabled=false;
    }

    btnMaintenanceMode->Visible=(CUSTOMER_CODE==CC_SIGURD_PeiXing && AccessLevel==iDefHonPrecLevel);     //Sam 20250122 : 矽格北興新增維護模式，不需要警報一直顯示紅色

    PageControl1->ActivePageIndex=0;
    fShow=true;
}
//---------------------------------------------------------------------------
void TfGroundMan::FormClose()                                                  // golden :228-245 -- `TObject *Sender, TCloseAction &Action` dropped, neither read
{
    bool bReStart=true;

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU || CUSTOMER_CODE==CC_SIGURD_PeiXing)              //Sam 20220107 : 矽格中控室通訊口不夠要求可以取消 Ground 功能
        bReStart=false;
    else
        bReStart=true;

    if(bReStart)
         ReStart();  //Sam 20220107 : 防止重新啟動 Initail Start 要重跑 GroundMan  //Ifor 20201112 add: 避免 Ground 被關視窗後失效 -- GATE (GM-3), see header
    btnMaintenanceMode->Down=false;                                             //Sam 20250122 : 矽格北興新增維護模式，不需要警報一直顯示紅色

    fShow=false;
    //這一段請保留在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);    //Steven 20100629
}
//---------------------------------------------------------------------------
AnsiString TfGroundMan::calc_crc(unsigned char *buf, int length)                // golden :525-549 -- pure computation, no comm/file/global deref beyond params
{
    unsigned short crc = 0xFFFF;
    int i, j;
    unsigned char LSB;
    AnsiString asCrc;

    unsigned char cbuf2[11];

    memcpy(cbuf2, buf, sizeof(cbuf2));

    for(i=0; i<11; i++)
    {
        crc^=cbuf2[i];
        for(j=0; j<8; j++)
        {
            LSB= crc & 1;
            crc=crc >> 1;
            if (LSB)
                crc^=0xA001;
        }
    }
    asCrc.sprintf("%d,%d", ((crc&0xFF00)>>8), ((crc&0x0FF)));
    return asCrc;
}
//---------------------------------------------------------------------------
void TfGroundMan::Timer1Timer()                                                // golden :551-575 -- `TObject *Sender` dropped, never read
{
    static bool bRunTimer1=false;                                           //jou 20240310 : timer加上保作

    if(InitialOK==false)
    {
        bRunTimer1=false;
        return;
    }

    if(bRunTimer1==true)
    {
        return;
    }

    bRunTimer1=true;

    if(USE_GROUND_MAN==1 && bRs232Ok)
    {
        // AI(W906-FW3-LGM1) 20260828: GATE (GM-2) -- DoGroundMasterMonitor
        // (golden :577-1416, 840 lines) is EXCLUDED this wave (RS232 comm +
        // StopAllMotor() machine action at golden :1295). Not declared/
        // defined; the call is gated in place. SAFE WHILE GATED: bRs232Ok
        // is set true only by Init_GM_RS232 (also excluded), so this branch
        // never executes today regardless of the gate -- see header GATE
        // REGISTER (GM-2).
#if 0
        DoGroundMasterMonitor();
#endif
    }
    labStatus->Caption=(AnsiString)iGroundMasterTask;

    bRunTimer1=false;
}
//---------------------------------------------------------------------------
void TfGroundMan::sbtExitClick()                                               // golden :1471-1478 -- `TObject *Sender` dropped, never read
{
    sbtExit->Down=false;
    Close();
    fShow=false;
}
//---------------------------------------------------------------------------
void TfGroundMan::edOccurrencesMouseDown()                                      // golden :1623-1628 -- see header DEVIATION (D-3): Sender-> edOccurrences
{
    fQwertyKey->ShowQwertyKey(edOccurrences, N_INTEGER, 1, true, 1, 10);
}
//---------------------------------------------------------------------------
void TfGroundMan::edContinuous_TimeMouseDown()                                   // golden :1629-1634 -- see header DEVIATION (D-3): Sender-> edContinuous_Time
{
    fQwertyKey->ShowQwertyKey(edContinuous_Time, N_INTEGER, 5, true, 5, 50);
}
//---------------------------------------------------------------------------
void TfGroundMan::ReStart()                                                     // golden :1635-1651 -- GATE (GM-3), see header
{
#ifdef SOFT_SIMULTE
    return;
#else
    // AI(W906-FW3-LGM1) 20260828: GATE (GM-3) -- comGM->StopComm() +
    // Init_GM_RS232() are both EXCLUDED this wave (RS232 comm). Neither
    // `comGM` nor `Init_GM_RS232` is declared, so the whole conditional is
    // gated in place -- ReStart() is a no-op while this gate is closed
    // (matches golden's own SOFT_SIMULTE branch shape above). See header
    // GATE REGISTER (GM-3).
#if 0
    if(USE_GROUND_MAN)
    {
        comGM->StopComm();
        bRs232Ok=false;
        Init_GM_RS232();
    }
#endif
#endif
}
//---------------------------------------------------------------------------
