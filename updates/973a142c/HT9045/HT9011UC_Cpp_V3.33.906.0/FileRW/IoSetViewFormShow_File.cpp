// ===========================================================================
//  FileRW/IoSetViewFormShow_File.cpp -- S12 second-type bridge (display only): golden Tfiosetview::FormShow's screen half.
//
//  AI(W906-IOSV-FORMSHOW) 20261002.  NOT generated -- hand-written by the rule FileRW/TeachFormShow_File.cpp follows:
//      widget->Prop = rhs;   ->   J.SetProp("widget", rhs);    control flow and the right-hand sides copied verbatim.
//  EastSun 20261001「請檢查每個頁面元件」「不是只有檢查按鈕喔 我說的是所有元件」: the every-component check (compK ioS) found
//  that the web IO page applied none of golden FormShow's Visible / TabVisible / Caption / Enabled assignments -- every group
//  showed as drawn in iosetview.dfm (Color tray, 2nd loader, AOI, OTD, Ion fan 5-IO captions, Catch tray, Neg. air, TTL map...)
//  whatever this machine's options are.
//
//  Source: golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\iosetview.cpp (V906, Big5, read only):
//    ctor :49-91 (:64-65 only), FormShow :236-1023, ShowSuckMode :1313-1669, ShowShuttleSensor :1671-1758,
//    LabSiteMap :2861-2983, Hide_1032_IO :3907-3963, Timer1Timer :185-196 (btnSwHeaterFan, evaluated at open).
//  Served as GET /api/form/HW.IoSetView.html (FormJson.cpp BridgePageJson); page/ht9045_iosetview_formshow_c.js applies it
//  on load and on every window open (golden FormShow runs at every Show).
//
//  NOT copied (each on purpose, so a GET never changes the machine):
//    :306-307 SW[SwFMotorBreaker].Off() / SW[SwBMotorBreaker].Off()         (machine outputs: the Index Z brakes)
//    :313-329 MOT[].Gali_ReadPos / NewRecordProcess                           (card reads + log)
//    :364 bStartTTLOut, :370 bOutDataChange, :372 bCheckEMG, :387 fShow, :716-717 ResetIndexSuck/Destroy, :725 bChangeSuckStatus,
//    :749-750 Timer1/2 Enabled, :1000 SetTechDataToProd                        (form / engine state)
//    :366-368 SetCompomentIO / SetCompomentHint / SetPanelElable               (the per-Alias binding: the page's ioBindStatus
//                                                                               and ht9045_io_do.js / ht9045_io_widgets.js; SetPanelElable
//                                                                               only acts in SOFT_SIMULTE or iControlPanelMode==1)
//    :394-397 SW[SwCCDLight].On()/Off(), :420 ADAM_WriteVoltage(0), :429 ADAM_DirectWriteData   (machine outputs)
//    :399-402 / :416 / :424 / :454-457 tbarIndexEP / tbarIndexEP2 / tbarLoaderEP Position / Max   (track-bar range: no bridge prop)
//    :718-719 Left / Top, :1005-1006 grpWinWayGroup Top / Left, :55-62 palArm*_16 Top / Left      (geometry)
//    :802 btIPSetting->Click() (imgOther picture from BmpPath)                (file side effect; the page shows its own image)
//    :832-975 btnC_*->OnClick=BtnPanelClick                                    (event rewiring: C++ io.btnPanelClick does it,
//                                                                               JsonBridge/IoBtnPanelClick.cpp kZ table)
//    :757-766 tsLoader..tsTool ->Enabled by AccessLevel / LevelSet, :365 tsTool->TabVisible by AccessLevel
//                                                                             (login-level locks: EastSun 20260929 "IO畫面一律不要卡控，
//                                                                               讓我測試", JsonBridge/IoBtnPanelClick.cpp W906_IO_PAGE_NO_GUARDS)
//    :1692 InitShuttleThreadParameter() (ShowShuttleSensor's first line)      (rewrites SThreadPara, which the shuttle thread
//                                                                               uses; golden can only run it with the machine stopped --
//                                                                               the IO form cannot open while SystemStart -- a web GET can)
//  tsIOTable :248: golden shows it for NewIO_MN200 / PCI_P64C64; this tree adds PCI1203_IO to every such test (Jimmy 20260925
//    ruling A, cinitial.cpp:505 / database.cpp:1213 AI(W906-IOWEB-P4)) -- so does this line.
// ===========================================================================
#include "JsonBridge/FormBridge.h"
#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "cprod.h"
#include "LastSet.h"
#include "myswitch.h"
#include "mysensor.h"
#include "mycylin.h"
#include "mykitsuck.h"
#include "csystem.h"
#include "Motor/mymotor.h"

namespace ht9045 {
namespace formbridge {

namespace f_Tfiosetview {

static const long kclUsed = 0x00393A3E, kclUnused = 0x00DDDDDC;   // golden ShowSuckMode TColor(0x00393A3E) / TColor(0x00DDDDDC)
static const long kclLime = 0x0000FF00, kclBtnFace = (long)0x8000000F;   // VCL clLime / clBtnFace (clSystemColor | COLOR_BTNFACE)
static const int kG9004_M204 = 0xA7;   // Motor/vendor/CMNet.h:20 #define G9004_M204 (that header drags the Syntek typedefs in; cinitial.cpp:3679-3690)

// golden ctor :67-86 Index_8 / Index_16 / InArm_8 / InArm_16 / OutArm_8 / OutArm_16 (the panels ShowSuckMode paints)
static const char* const kIndex8[2][2][4] = {
    { { "palArm1_Aa_Name", "palArm1_Ab_Name", "palArm1_Ac_Name", "palArm1_Ad_Name" },
      { "palArm1_Ba_Name", "palArm1_Bb_Name", "palArm1_Bc_Name", "palArm1_Bd_Name" } },
    { { "palArm2_Aa_Name", "palArm2_Ab_Name", "palArm2_Ac_Name", "palArm2_Ad_Name" },
      { "palArm2_Ba_Name", "palArm2_Bb_Name", "palArm2_Bc_Name", "palArm2_Bd_Name" } } };
static const char* const kIndex16[2][2][8] = {
    { { "palArm1_Aa_16_name", "palArm1_Ab_16_name", "palArm1_Ac_16_name", "palArm1_Ad_16_name", "palArm1_Ae_16_name", "palArm1_Af_16_name", "palArm1_Ag_16_name", "palArm1_Ah_16_name" },
      { "palArm1_Ba_16_name", "palArm1_Bb_16_name", "palArm1_Bc_16_name", "palArm1_Bd_16_name", "palArm1_Be_16_name", "palArm1_Bf_16_name", "palArm1_Bg_16_name", "palArm1_Bh_16_name" } },
    { { "palArm2_Aa_16_name", "palArm2_Ab_16_name", "palArm2_Ac_16_name", "palArm2_Ad_16_name", "palArm2_Ae_16_name", "palArm2_Af_16_name", "palArm2_Ag_16_name", "palArm2_Ah_16_name" },
      { "palArm2_Ba_16_name", "palArm2_Bb_16_name", "palArm2_Bc_16_name", "palArm2_Bd_16_name", "palArm2_Be_16_name", "palArm2_Bf_16_name", "palArm2_Bg_16_name", "palArm2_Bh_16_name" } } };
static const char* const kInArm8[2][4]  = { { "palInArm_A", "palInArm_C", "palInArm_E", "palInArm_G" }, { "palInArm_B", "palInArm_D", "palInArm_F", "palInArm_H" } };
static const char* const kOutArm8[2][4] = { { "palOutArm_A", "palOutArm_C", "palOutArm_E", "palOutArm_G" }, { "palOutArm_B", "palOutArm_D", "palOutArm_F", "palOutArm_H" } };
static const char* const kInArm16[2][8]  = { { "labInArmAa", "labInArmAb", "labInArmAc", "labInArmAd", "labInArmAe", "labInArmAf", "labInArmAg", "labInArmAh" },
                                             { "labInArmBa", "labInArmBb", "labInArmBc", "labInArmBd", "labInArmBe", "labInArmBf", "labInArmBg", "labInArmBh" } };
static const char* const kOutArm16[2][8] = { { "labOutArmAa", "labOutArmAb", "labOutArmAc", "labOutArmAd", "labOutArmAe", "labOutArmAf", "labOutArmAg", "labOutArmAh" },
                                             { "labOutArmBa", "labOutArmBb", "labOutArmBc", "labOutArmBd", "labOutArmBe", "labOutArmBf", "labOutArmBg", "labOutArmBh" } };

static bool InRange(int v, int n) { return v >= 0 && v < n; }   // golden indexes the arrays unchecked; the bridge must not

// golden iosetview.cpp:1313  Tfiosetview::ShowSuckMode(int iSel)
static void B_ShowSuckMode(FormState& J, int iSel)
{
    J.SetVisible("palArm2_Ab", true);                                                    // :1315
    J.SetVisible("palArm1_Ab", true);
    if(iSel==-1)                                                                         // :1318 全部顯示為使用中
    {
        for(int i=0; i<2; i++)
            for(int j=0; j<4; j++)
            {
                J.SetColor(kIndex8[0][i][j], kclUsed);  J.SetColor(kIndex8[1][i][j], kclUsed);
                J.SetCaption(kIndex8[0][i][j], IndexSuckName[i][j]);  J.SetCaption(kIndex8[1][i][j], IndexSuckName[i][j]);
                J.SetColor(kInArm8[i][j], kclUsed);  J.SetColor(kOutArm8[i][j], kclUsed);
            }
        for(int i=0; i<2; i++)
            for(int j=0; j<8; j++)
            {
                J.SetColor(kIndex16[0][i][j], kclUsed);  J.SetColor(kIndex16[1][i][j], kclUsed);
                J.SetCaption(kIndex16[0][i][j], IndexSuckName[i][j]);  J.SetCaption(kIndex16[1][i][j], IndexSuckName[i][j]);
                J.SetColor(kInArm16[i][j], kclUsed);  J.SetColor(kOutArm16[i][j], kclUsed);
            }
    }
    else                                                                                 // :1346 全部顯示為未使用
    {
        for(int i=0; i<2; i++)
            for(int j=0; j<4; j++)
            {
                J.SetColor(kIndex8[0][i][j], kclUnused);  J.SetColor(kIndex8[1][i][j], kclUnused);
                J.SetCaption(kIndex8[0][i][j], AnsiString(""));  J.SetCaption(kIndex8[1][i][j], AnsiString(""));
                J.SetColor(kInArm8[i][j], kclUnused);  J.SetColor(kOutArm8[i][j], kclUnused);
            }
        for(int i=0; i<2; i++)
            for(int j=0; j<8; j++)
            {
                J.SetColor(kIndex16[0][i][j], kclUnused);  J.SetColor(kIndex16[1][i][j], kclUnused);
                J.SetCaption(kIndex16[0][i][j], AnsiString(""));  J.SetCaption(kIndex16[1][i][j], AnsiString(""));
                J.SetColor(kInArm16[i][j], kclUnused);  J.SetColor(kOutArm16[i][j], kclUnused);
            }
    }
    if(iSel==-1)                                                                         // :1375
        return;

    int iR, iC;
    for(int i=0; i<FTestSuck.iShtRow; i++)                                               // :1379
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            iR=FTestSuck.Suck[i][j].iMyRow;
            iC=FTestSuck.Suck[i][j].iMyCol;
            if(InRange(iR, 2) && InRange(iC, 8) && InRange(i, 4) && InRange(j, 8))
            {
                if(iC<4)
                {
                    J.SetColor(kIndex8[0][iR][iC], kclUsed);
                    J.SetCaption(kIndex8[0][iR][iC], IndexSuckName[i][j]);
                }
                J.SetColor(kIndex16[0][iR][iC], kclUsed);
                J.SetCaption(kIndex16[0][iR][iC], IndexSuckName[i][j]);
            }
            iR=BTestSuck.Suck[i][j].iMyRow;
            iC=BTestSuck.Suck[i][j].iMyCol;
            if(InRange(iR, 2) && InRange(iC, 8) && InRange(i, 4) && InRange(j, 8))
            {
                if(iC<4)
                {
                    J.SetColor(kIndex8[1][iR][iC], kclUsed);
                    J.SetCaption(kIndex8[1][iR][iC], IndexSuckName[i][j]);
                }
                J.SetColor(kIndex16[1][iR][iC], kclUsed);
                J.SetCaption(kIndex16[1][iR][iC], IndexSuckName[i][j]);
            }
        }
    }

    for(int i=0; i<OutArmSuck.iPickRow; i++)                                             // :1405
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            iR=OutArmSuck.Suck[i][j].iMyRow;
            iC=OutArmSuck.Suck[i][j].iMyCol;
            if(!InRange(iR, 2) || !InRange(iC, 8)) continue;
            if(j<4 && iC<4)                                                              // golden tests j<4 and indexes [iC] (:1411-1413)
                J.SetColor(kOutArm8[iR][iC], kclUsed);
            J.SetColor(kOutArm16[iR][iC], kclUsed);
        }
    }

    // :1419-1668 the In Arm pickers in use, per iInArmType (golden's own table, one row per arm type)
    struct InUse { int type; int n8; int r8[8], c8[8]; int n16; int r16[8], c16[8]; };
    static const InUse kIn[] = {
        { e9045_1x2_2_13, 2, {0,0}, {0,2}, 2, {0,0}, {0,4} },                            // :1432
        { e9045_1x2_2_14, 2, {0,0}, {0,3}, 2, {0,0}, {0,6} },                            // :1439
        { e9045_1x2_4_Hot, 4, {0,0,1,1}, {0,2,0,2}, 4, {0,0,1,1}, {0,4,0,4} },           // :1446
        { e9045_1x3_2_14, 2, {0,0}, {0,3}, 2, {0,0}, {0,6} },                            // :1457
        { e9045_1x3_4, 6, {0,0,0,1,1,1}, {0,1,2,0,1,2}, 6, {0,0,0,1,1,1}, {0,2,4,0,2,4} },   // :1464
        { e9045_1x4_2_14, 2, {0,0}, {0,3}, 2, {0,0}, {0,6} },                            // :1479
        { e9045_1x4_1_Ac, 1, {0}, {0}, 1, {0}, {0} },                                    // :1486
        { e9045_1x4_4_13, 4, {0,0,1,1}, {0,2,0,2}, 4, {0,0,1,1}, {0,4,0,4} },            // :1491
        { e9045_1x4_4_Back, 4, {1,1,1,1}, {0,1,2,3}, 4, {1,1,1,1}, {0,2,4,6} },          // :1502
        { e9045_1x4_4, 4, {0,0,0,0}, {0,1,2,3}, 4, {0,0,0,0}, {0,2,4,6} },               // :1513
        { e9045_1x4_8_Hot, 8, {0,0,0,0,1,1,1,1}, {0,1,2,3,0,1,2,3}, 8, {0,0,0,0,1,1,1,1}, {0,2,4,6,0,2,4,6} },   // :1524
        { e9045_2x1_2_13, 2, {0,0}, {0,2}, 2, {0,0}, {0,4} },                            // :1543
        { e9045_2x2_4_12, 4, {0,0,1,1}, {0,1,0,1}, 4, {0,0,1,1}, {0,2,0,2} },            // :1550
        { e9045_2x2_4_13, 4, {0,0,1,1}, {0,2,0,2}, 4, {0,0,1,1}, {0,4,0,4} },            // :1561
        { e9045_2x2_4_14, 4, {0,0,1,1}, {0,3,0,3}, 4, {0,0,1,1}, {0,6,0,6} },            // :1572
        { e9045_2x2_8_Hot, 8, {0,0,0,0,1,1,1,1}, {0,1,2,3,0,1,2,3}, 8, {0,0,0,0,1,1,1,1}, {0,2,4,6,0,2,4,6} },   // :1583
        { e9045_2x3_6_14, 2, {0,0}, {0,3}, 2, {0,0}, {0,6} },                            // :1602
        { e9045_2x3_6, 6, {0,0,0,1,1,1}, {0,1,2,0,1,2}, 6, {0,0,0,1,1,1}, {0,2,4,0,2,4} },   // :1609
        { e9045_2x4_4_13, 4, {0,0,1,1}, {0,2,0,2}, 4, {0,0,1,1}, {0,4,0,4} },            // :1624
        { e9045_2x4_4_14, 4, {0,0,1,1}, {0,3,0,3}, 4, {0,0,1,1}, {0,6,0,6} },            // :1635
    };
    if(iInArmType==e9045_1x1_1)                                                          // :1419
    {
        if(Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck)                    // :1421
        {
            J.SetColor(kInArm8[0][1], kclUsed);
            J.SetColor(kInArm16[0][2], kclUsed);
        }
        else
        {
            J.SetColor(kInArm8[0][0], kclUsed);
            J.SetColor(kInArm16[0][0], kclUsed);
        }
        return;
    }
    if(iInArmType==e9045_2x4_8 || iInArmType==e9045_2x5_8 || iInArmType==e9045_2x6_8 ||   // :1646
       iInArmType==e9045_2x8_8 || iInArmType==e9045_2x8_32)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++) J.SetColor(kInArm8[i][j], kclUsed);
            for(int j=0; j<8; j+=2) J.SetColor(kInArm16[i][j], kclUsed);
        }
        return;
    }
    for(unsigned k=0; k<sizeof(kIn)/sizeof(kIn[0]); ++k)
    {
        if(iInArmType!=kIn[k].type) continue;
        for(int n=0; n<kIn[k].n8; ++n)  J.SetColor(kInArm8[kIn[k].r8[n]][kIn[k].c8[n]], kclUsed);
        for(int n=0; n<kIn[k].n16; ++n) J.SetColor(kInArm16[kIn[k].r16[n]][kIn[k].c16[n]], kclUsed);
        break;
    }
}

// golden iosetview.cpp:1671  Tfiosetview::ShowShuttleSensor()
static void B_ShowShuttleSensor(FormState& J)
{
    if(LastSet.iOutShuttleSensorMode==0)                                                 // :1673
    {
        J.SetVisible("mmoOutSht", true);
        J.SetVisible("mmoOutSht2", false);
    }
    else
    {
        J.SetVisible("mmoOutSht", false);
        J.SetVisible("mmoOutSht2", true);
        /* golden :1682-1683 mmoOutSht2->Top/Left=mmoOutSht's (geometry) */
    }

    AnsiString Str1="", Str2="", Str3="", Str4="";
    static const char* const pInShuttleLink[9]={"plInShuttleLink1", "plInShuttleLink2", "plInShuttleLink3", "plInShuttleLink4", "plInShuttleLink5",
                                                 "plInShuttleLink6", "plInShuttleLink7", "plInShuttleLink8", "plInShuttleLink9"};
    static const char* const pOutShuttleLink[7]={"plOutShuttleLink1", "plOutShuttleLink2", "plOutShuttleLink3", "plOutShuttleLink4", "plOutShuttleLink5",
                                                  "plOutShuttleLink6", "plOutShuttleLink7"};
    /* golden :1692 InitShuttleThreadParameter(); -- NOT copied (file header): SThreadPara as the engine has it.
       Its only screen lines are golden cinitial.cpp:15012-15047 fiosetview->labUseSensor1/2->Caption=Str1 -- computed here from
       the same SThreadPara fields (the port's InitShuttleThreadParameter writes them into a shim nobody shows, cinitial.cpp:13253) */
    {
        AnsiString StrU;
        if(SThreadPara.iScanSensor==1)                                                   // cinitial.cpp:15012
        {
            if(SThreadPara.bExeShuttleThread)
                StrU="Use Y Sensor with Thread";
            else if(SThreadPara.bOutYUseLatch)
                StrU="Use Y Sensor with Latch";
            else
                StrU="Use Z Sensor with Latch";
        }
        else                                                                             // cinitial.cpp:15034 2顆scan sensor
        {
            if(SThreadPara.bUseM204Mode)
                StrU="Use Z Sensor with Latch";
            else
                StrU="Use Z Sensor with Thread";
        }
        J.SetCaption("labUseSensor1", StrU);                                             // cinitial.cpp:15046
        J.SetCaption("labUseSensor2", StrU);                                             // cinitial.cpp:15047 (golden writes Str1 into both)
    }

    for(int i=0; i<8; i++)                                                               // :1694
    {
        if(SThreadPara.bUseInShtSen[i]==true)
            J.SetColor(pInShuttleLink[i], kclLime);
        else
            J.SetColor(pInShuttleLink[i], kclBtnFace);
    }

    if(MachineTypeChoice==Type_HT9045 ||                                                 // :1702
       MachineTypeChoice==Type_HT9045_12Site)
    {
        if(ENABLE_OUT_SHUTTLE_SENEOR)
        {
            for(int i=0; i<6; i++)
            {
                if(SThreadPara.bUseInShtSen[i]==true)
                    J.SetColor(pOutShuttleLink[i+1], kclLime);
                else
                    J.SetColor(pOutShuttleLink[i+1], kclBtnFace);
            }
        }

        J.SetVisible("grpInShuttle", true);
        J.SetVisible("grpOutShuttle", ENABLE_OUT_SHUTTLE_SENEOR);
        J.SetVisible(pInShuttleLink[8-1], false);
        J.SetVisible(pInShuttleLink[9-1], (IN_SHT_LAST_SENSOR==1));
        J.SetVisible("MyLedInShuttleNumR8", false);
        J.SetVisible("MyLedInShuttleNumR9", (IN_SHT_LAST_SENSOR==1));
        J.SetVisible("MyLedInShuttleNumF8", false);
        J.SetVisible("MyLedInShuttleNumF9", (IN_SHT_LAST_SENSOR==1));
    }
    else
    {
        J.SetVisible("grpInShuttle", true);                                              // :1727
        J.SetVisible("grpOutShuttle", false);
        J.SetVisible("MyLedInShuttleNumR8", true);
        J.SetVisible("MyLedInShuttleNumR9", true);
        J.SetVisible("MyLedInShuttleNumF8", true);
        J.SetVisible("MyLedInShuttleNumF9", true);
    }

    J.SetVisible("lbOutShuttle1", (ENABLE_OUT_SHUTTLE_SENEOR==true));                      // :1735
    J.SetVisible("lbOutShuttle2", (ENABLE_OUT_SHUTTLE_SENEOR==true));

    for(int i=0; i<FLCarryKit.iShtCol && i<10; i++)                                      // :1738 (i<10: SThreadPara.iInShSenIndex[2][10])
    {
        const int a=SThreadPara.iInShSenIndex[0][i], b=SThreadPara.iInShSenIndex[1][i];
        if(InRange(a, MAX_SENSOR_ITEM)) Str1+=Sen[a].Name+AnsiString(", ");
        if(InRange(b, MAX_SENSOR_ITEM)) Str2+=Sen[b].Name+AnsiString(", ");
    }

    if(ENABLE_OUT_SHUTTLE_SENEOR==true &&                                                // :1744
       FLCarryKit.iShtCol<6)
    {
        for(int i=0; i<FLCarryKit.iShtCol; i++)
        {
            const int a=SThreadPara.iOutShSenIndex[0][i], b=SThreadPara.iOutShSenIndex[1][i];
            if(InRange(a, MAX_SENSOR_ITEM)) Str3+=Sen[a].Name+AnsiString(", ");
            if(InRange(b, MAX_SENSOR_ITEM)) Str4+=Sen[b].Name+AnsiString(", ");
        }
    }

    J.SetCaption("lbInShuttle1", Str1);                                                  // :1754
    J.SetCaption("lbInShuttle2", Str2);
    J.SetCaption("lbOutShuttle1", Str3);
    J.SetCaption("lbOutShuttle2", Str4);
}

// golden iosetview.cpp:2861  Tfiosetview::LabSiteMap()
static void B_LabSiteMap(FormState& J)
{
    int iCH[2][4];
    AnsiString asString[9]= {"Aa", "Ab", "Ac", "Ad", "Ba", "Bb", "Bc", "Bd", "X"};
    AnsiString asString1[9]={"Aa", "Ab", "Ac", "Ba", "Bb", "Bc", "X", "X", "X"};
    static const char* const kOut[8] = { "lblTTL_Aa", "lblTTL_Ab", "lblTTL_Ac", "lblTTL_Ad", "lblTTL_Ba", "lblTTL_Bb", "lblTTL_Bc", "lblTTL_Bd" };
    static const char* const kIn[8]  = { "lblTTLInAa", "lblTTLInAb", "lblTTLInAc", "lblTTLInAd", "lblTTLInBa", "lblTTLInBb", "lblTTLInBc", "lblTTLInBd" };

    if((TTLCfg.iCateBitLength==_8Bit || TTLCfg.iCateBitLength==_10Bit ||                 // :2868
        TTLCfg.iCateBitLength==_10BitPE || TTLCfg.iCateBitLength==_10BitPO))
    {
        for(int k=0; k<8; k++) { J.SetVisible(kOut[k], false); J.SetVisible(kIn[k], false); }   // :2871-2887
        return;
    }
    switch(TestIF_File.iTestMode)                                                        // :2891
    {
        case SingleSite:
        case DualSite:
        case TriSite1X3:
        case QualSite1X4:
        case DualSite2x1:
        case QualSite2X2:
        case _8Site1X4:
            for(int i=0; i<2; i++)
                for(int j=0; j<4; j++)
                    iCH[i][j]=(TestIF_File.iSiteMap[i][j]>0 && TestIF_File.iSiteMap[i][j]<=8) ? TestIF_File.iSiteMap[i][j]-1 : 8;   // golden: >0 ? -1 : 8 (unchecked above 8)
            {
                const int o[8] = { iCH[0][0], iCH[0][1], iCH[0][2], iCH[0][3], iCH[1][0], iCH[1][1], iCH[1][2], iCH[1][3] };   // :2913-2929
                for(int k=0; k<8; k++) { J.SetCaption(kOut[k], asString[o[k]]); J.SetCaption(kIn[k], asString[o[k]]); }
            }
            break;
        case _6Site2X3:
            for(int i=0; i<2; i++)
                for(int j=0; j<4; j++)
                    iCH[i][j]=(TestIF_File.iSiteMap[i][j]>0 && TestIF_File.iSiteMap[i][j]<=8) ? TestIF_File.iSiteMap[i][j]-1 : 8;
            {
                const int o[8] = { iCH[0][0], iCH[1][0], iCH[0][1], iCH[0][3], iCH[0][2], iCH[1][2], iCH[1][1], iCH[1][3] };   // :2943-2959
                for(int k=0; k<8; k++) { J.SetCaption(kOut[k], asString1[o[k]]); J.SetCaption(kIn[k], asString1[o[k]]); }
            }
            break;
        case _32Site4X8N:                                                                // :2961
        case _32Site4X8M:                                                                // :2971
            for(int k=0; k<8; k++) J.SetCaption(kOut[k], AnsiString("Aa"));
            break;
    }
}

// golden iosetview.cpp:3907  Tfiosetview::Hide_1032_IO()
static void B_Hide_1032_IO(FormState& J)
{
    J.SetTabVisible("tsStack1_Above", false);                                            // :3909
    J.SetTabVisible("tsStack2_Above", false);
    J.SetTabVisible("tsStack3_Above", false);
    J.SetTabVisible("tsStack1_Under", false);
    J.SetTabVisible("tsStack2_Under", false);
    J.SetTabVisible("tsStack3_Under", false);
    J.SetTabVisible("tsStack1_Cassette", false);
    J.SetTabVisible("tsStack2_Cassette", false);
    if(USE_LdUldCassetteMode==1)                                                         // :3917
    {
        J.SetActivePage("pgcStack1", "tsStack1_Cassette");
        J.SetActivePageIndex("pgcStack1", 2);
        J.SetActivePage("pgcStack2", "tsStack2_Cassette");
        J.SetActivePageIndex("pgcStack2", 2);
    }
    else
    {
        J.SetActivePage("pgcStack1", (TRAY_ARM_MODE==eAboveCoveyor)?"tsStack1_Above":"tsStack1_Under");   // :3926
        J.SetActivePageIndex("pgcStack1", (TRAY_ARM_MODE==eAboveCoveyor)?0:1);
        J.SetActivePage("pgcStack2", (TRAY_ARM_MODE==eAboveCoveyor)?"tsStack2_Above":"tsStack2_Under");
        J.SetActivePageIndex("pgcStack2", (TRAY_ARM_MODE==eAboveCoveyor)?0:1);
    }

    J.SetActivePage("pgcStack3", (TRAY_ARM_MODE==eAboveCoveyor)?"tsStack3_Above":"tsStack3_Under");       // :3932
    J.SetActivePageIndex("pgcStack3", (TRAY_ARM_MODE==eAboveCoveyor)?0:1);
    J.SetVisible("grpTempOver", (Tri_Temp_Machine==1));                                  // :3934

    static const char* const kHatch[] = { "pnlSnSafeDoor1Hatchway", "pnlSnSafeDoor2Hatchway", "pnlSnSafeDoor3Hatchway", "pnlSnSafeDoor4Hatchway",
        "pnlSnSafeDoor6Hatchway", "pnlSnSafeDoor7Hatchway", "pnlSnSafeDoor8Hatchway", "pnlSnSafeDoor10Hatchway", "pnlSnSafeDoor11Hatchway",
        "pnlSnSafeDoorForFixTray" };
    for(unsigned k=0; k<sizeof(kHatch)/sizeof(kHatch[0]); ++k) J.SetVisible(kHatch[k], (Tri_Temp_Machine==1));   // :3936-3945

    J.SetVisible("btnC_SafeDoor8Lock_1032", (MachineTypeChoice==Type_HT1032));           // :3947
    J.SetVisible("grpTriTempAir", (MachineTypeChoice==Type_HT1032));
    J.SetVisible("gbExhaustAir", (MachineTypeChoice==Type_HT1032));
    static const char* const kFlood[] = { "btnC_Shuttle1Floodgate", "ledC_Shuttle1Floodgate_On", "ledC_Shuttle1Floodgate_Off",
        "btnC_Shuttle2Floodgate", "ledC_Shuttle2Floodgate_On", "ledC_Shuttle2Floodgate_Off",
        "btnC_OutShuttle1Floodgate", "ledC_OutShuttle1Floodgate_On", "ledC_OutShuttle1Floodgate_Off",
        "btnC_OutShuttle2Floodgate", "ledC_OutShuttle2Floodgate_On", "ledC_OutShuttle2Floodgate_Off" };
    for(unsigned k=0; k<sizeof(kFlood)/sizeof(kFlood[0]); ++k) J.SetVisible(kFlood[k], (SHUTTLE_FLOODGATE==1));   // :3950-3962
}

// golden iosetview.cpp:236  Tfiosetview::FormShow(TObject *Sender) -- the screen half
static void B_FormShow(FormState& J)
{
    // ctor :64-65 (the ctor runs once at CreateForm; FormShow never touches these two again)
    J.SetVisible("btnSwHeatGun", (INSTALL_HEAT_GUN>0));
    J.SetVisible("btnSwCDAGun", (INSTALL_HEAT_GUN>1));

    /* :238-247 tsLoader..tsTool ->Enabled=true and :757-766 the AccessLevel locks: NOT copied (file header, EastSun 20260929) */
    J.SetTabVisible("tsIOTable", (IO_CARD_TYPE==NewIO_MN200 ||                            // :248
                                  IO_CARD_TYPE==PCI_P64C64 || IO_CARD_TYPE==PCI1203_IO));   // + PCI1203_IO: Jimmy 20260925 ruling A (file header)
    J.SetTabVisible("tsSafePLC", IsSafePLCIOInstall());                                   // :251  AI(W906-W217) 20261010 (Ifor01): golden 913 iosetview.cpp:353

    static const char* const tempBtn[] = { "btnSwBMotorBreaker", "btnSwFMotorBreaker", "btnSwMotorRelay", "btnSwServerON",   // :253-255
                                           "btnIndexArm1SuckMode1", "btnIndexArm1SuckMode2",
                                           "btnIndexArm2SuckMode1", "btnIndexArm2SuckMode2" };

    if(MachineTypeChoice==Type_HT9045)                                                   // :258
    {
        J.SetVisible("palArm2_A_16", false);
        J.SetVisible("palArm2_B_16", false);
        J.SetVisible("palArm1_A_16", false);
        J.SetVisible("palArm1_B_16", false);
    }
    else
    {
        if(MachineTypeChoice==Type_HT9045_12Site)                                        // :267
        {
            J.SetVisible("palArm2_Ag_16", false);
            J.SetVisible("palArm2_Ah_16", false);
            J.SetVisible("palArm2_Bg_16", false);
            J.SetVisible("palArm2_Bh_16", false);
            J.SetVisible("palArm1_Ag_16", false);
            J.SetVisible("palArm1_Ah_16", false);
            J.SetVisible("palArm1_Bg_16", false);
            J.SetVisible("palArm1_Bh_16", false);
        }
        J.SetVisible("palArm2_A", false);                                                // :278
        J.SetVisible("palArm2_B", false);
        J.SetVisible("palArm1_A", false);
        J.SetVisible("palArm1_B", false);
    }
    J.SetVisible("palTTL8Site", CosFunction.bTTLCanUse8Site);                            // :283

    if(AUTO_EMPTY_COLOR!=0)                                                              // :285
    {
        J.SetVisible("grpManual", false);
    }
    else
    {
        J.SetVisible("grpEmpty", false);
        J.SetVisible("grpColor", false);
    }

    J.SetVisible("grpLoader2_Under", (USE_2nd_LOADER!=eartUninstall));                   // :295
    J.SetVisible("grpLoader2", (USE_2nd_LOADER!=eartUninstall));
    J.SetVisible("pnlOutArm2Suck", (USE_OUT_SORT_ARM!=eartUninstall));

    J.SetTabVisible("tsUnLoader2", (AUTO_EMPTY_COLOR>=3));                               // :299
    J.SetVisible("grpAuto6", (AUTO_EMPTY_COLOR==4));
    J.SetVisible("grpAuto6_Under", (AUTO_EMPTY_COLOR==4));

    J.SetVisible("grpAOI", (USE_AOI_Inspection || USE_Fix_AI_CCD));                      // :303
    J.SetVisible("gbTopBottomAOI", (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));

    /* :306-307 SW[SwFMotorBreaker].Off(); SW[SwBMotorBreaker].Off(); -- NOT copied (machine outputs) */
    /* :309-329 MOT[].Gali_ReadPos / NewRecordProcess -- NOT copied */

    J.SetVisible("btnSwInArmZBreaker", (USE_PICKER_COUNT==ep1Picker));                   // :322
    J.SetVisible("btnSwOutArmZBreaker", (USE_PICKER_COUNT==ep1Picker));

    if(MachineTypeChoice==Type_HT9045 || MachineTypeChoice==Type_HT9045_12Site)          // :331
    {
        J.SetVisible("ledSnInPutSHT2S8", false);
        J.SetVisible("ledSnInPutSHT1S8", false);
        J.SetVisible("ledSnInPutSHT2S7", (IN_SHT_LAST_SENSOR!=1));
        J.SetVisible("ledSnInPutSHT1S7", (IN_SHT_LAST_SENSOR!=1));
        J.SetVisible("ledSnInPutSHT2S9", (IN_SHT_LAST_SENSOR==1));
        J.SetVisible("ledSnInPutSHT1S9", (IN_SHT_LAST_SENSOR==1));
        if(ENABLE_OUT_SHUTTLE_SENEOR==false)                                             // :341
        {
            J.SetVisible("palOutShuttleSensor2", false);
            J.SetVisible("palOutShuttleSensor1", false);
        }
        else
        {
            J.SetVisible("palOutShuttleSensor2", true);
            J.SetVisible("palOutShuttleSensor1", true);
        }
    }
    else
    {
        J.SetVisible("ledSnInPutSHT2S8", true);                                          // :355
        J.SetVisible("ledSnInPutSHT1S8", true);
        J.SetVisible("ledSnInPutSHT2S9", true);
        J.SetVisible("ledSnInPutSHT1S9", true);
        J.SetVisible("palOutShuttleSensor2", false);
        J.SetVisible("palOutShuttleSensor1", false);
    }

    /* :364-372 bStartTTLOut / SetCompomentIO / SetCompomentHint / SetPanelElable / bOutDataChange / bCheckEMG -- NOT copied */

    if(TestIF_File.iTestMode<_8Site2X4)                                                  // :374
    {
        J.SetVisible("chkShowIndexAll", true);
        J.SetChecked("chkShowIndexAll", false);
    }
    else
    {
        J.SetVisible("chkShowIndexAll", false);
        J.SetChecked("chkShowIndexAll", false);
    }

    B_ShowSuckMode(J, TestIF_File.iTestMode);                                            // :385
    B_ShowShuttleSensor(J);                                                              // :386

    if(CUSTOMER_CODE==CC_Greatek)                                                        // :389
    {
        J.SetVisible("btnClose", true);
    }
    /* :394-397 SW[SwCCDLight].On()/Off() -- NOT copied (machine output); :399-402 track-bar Position -- not a bridge prop */

    if(IndexHasIC())                                                                     // :404
    {
        J.SetEnabled("bplEpSwitch1", false);
        J.SetEnabled("bplEpSwitch2", false);
        J.SetEnabled("tbarIndexEP", false);
        for(int k=0; k<8; k++)
            J.SetEnabled(tempBtn[k], false);
    }
    else
    {
        if(EP_Install>0)                                                                 // :414
        {
            J.SetEnabled("tbarIndexEP", true);
            J.SetEnabled("bplEpSwitch1", true);
            J.SetEnabled("bplEpSwitch2", true);
            /* :420 ADAM_WriteVoltage(0); :429 ADAM_DirectWriteData(0, 1) -- NOT copied (machine outputs) */
        }
        else
        {
            J.SetEnabled("tbarIndexEP", false);
            J.SetEnabled("bplEpSwitch1", false);
            J.SetEnabled("bplEpSwitch2", false);
        }
        for(int k=0; k<8; k++)                                                           // :438
            J.SetEnabled(tempBtn[k], true);
    }

    J.SetEnabled("btnSwTjSignal01", true);                                               // :442
    J.SetEnabled("btnSwTjSignal02", true);
    J.SetEnabled("btnSwTjSignal03", true);
    J.SetEnabled("btnSwTjSignal04", true);
    J.SetEnabled("btnSwTjSignal05", true);
    J.SetEnabled("btnSwTjSignal06", true);
    J.SetEnabled("btnSwTjSignal07", true);
    J.SetEnabled("btnSwTjSignal08", true);
    J.SetVisible("btnSwAirOff", SW[SwAirOff].Enable);                                    // :450

    if(USE_CKD_FCM_CleanAir)                                                             // :452
    {
        J.SetVisible("grpLoaderEP", true);
        J.SetCaption("lblLoadEpAlarm", AnsiString("0"));                                 // :457 =tbarLoaderEP->Position (just set to 0)
    }
    else
    {
        J.SetVisible("grpLoaderEP", false);
    }
    J.SetVisible("btnSwLoaderAirClean_U", SW[SwLoaderAirClean].Enable);                  // :463
    J.SetVisible("btnSwLoaderAirClean", SW[SwLoaderAirClean].Enable);

    J.SetVisible("btnSwLoad2AirClean_U", SW[SwLoad2AirClean].Enable);                    // :466
    J.SetVisible("btnSwLoad2AirClean", SW[SwLoad2AirClean].Enable);

    #ifdef SOFT_SIMULTE
    //為了手冊撰寫須全部顯示出來                                                          // :469
        J.SetVisible("btnSwTesterAirCooling", true);
        J.SetVisible("bplEpSwitch1", true);
        J.SetVisible("bplEpSwitch2", true);
        J.SetVisible("btnSwDutHeaterCoolFan", true);
        J.SetVisible("btnSwShuttleCooling", true);
        J.SetVisible("gbRealCCD", true);
        J.SetVisible("grpATCAlarm", true);
        J.SetVisible("btnpnlnIndexIONFan", true);
        J.SetVisible("myldlnSenInArmYPitch", true);
        J.SetVisible("lblSenInArmYPitch60", true);
        J.SetVisible("myldlnSenOutArmYPitch60", true);
        J.SetVisible("lblSenOutArmYPitch60", true);
        J.SetVisible("bplFanDirection", true);
        J.SetVisible("gbSLKClamp", true);
        J.SetVisible("gbSocketClamp", true);
        J.SetVisible("btnSwIonFanClean", true);
        J.SetVisible("labIonFanClean", true);
        J.SetVisible("gbATCTJSwitch", true);
    #else
        J.SetVisible("btnSwTesterAirCooling", SW[SwTesterAirCooling].Enable);            // :490
        J.SetVisible("bplEpSwitch1", SW[SwEpArm1].Enable);
        J.SetVisible("bplEpSwitch2", SW[SwEpArm2].Enable);
        J.SetVisible("btnSwDutHeaterCoolFan", SW[SwDutHeaterCoolFan].Enable);
        J.SetVisible("btnSwShuttleCooling", SW[SwShuttleCooling].Enable);
        J.SetVisible("btnSwAirConditioner", SW[SwHotplateCooling].Enable);
        J.SetVisible("gbRealCCD", REAL_TIME_CCD);
        J.SetVisible("grpATCAlarm", (ATC_SYSTEM==eATCSiliconType || ATC_SYSTEM==eWinWay) && Temperature.bATCActiveCooling==true);
        J.SetVisible("grpATC70", (ATC_SYSTEM!=eNonChamber && ATC_SYSTEM!=eATCSiliconType && ATC_SYSTEM!=eATCUninstall));
        J.SetVisible("btnpnlnIndexIONFan", (ATC_SYSTEM>eATC60));
        J.SetVisible("myldlnSenInArmYPitch", (USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635));
        J.SetVisible("lblSenInArmYPitch60", (USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635));
        J.SetVisible("myldlnSenOutArmYPitch60", (USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635));
        J.SetVisible("lblSenOutArmYPitch60", (USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635));
        J.SetVisible("lblFix3Lock", (FIX3_FULL_PLACE==Fix3K_UseCylinder46LA));
        J.SetVisible("ledSnFix3Lock", (FIX3_FULL_PLACE==Fix3K_UseCylinder46LA));
        J.SetVisible("btnC_FixTray_FullPlace", (FIX3_FULL_PLACE==Fix3K_UseCylinder || FIX3_FULL_PLACE==Fix3K_UseCylinder46LA));
        J.SetVisible("ledC_FixTray_FullPlace_On", (FIX3_FULL_PLACE==Fix3K_UseCylinder || FIX3_FULL_PLACE==Fix3K_UseCylinder46LA));
        J.SetVisible("ledC_FixTray_FullPlace_Off", (FIX3_FULL_PLACE==Fix3K_UseCylinder || FIX3_FULL_PLACE==Fix3K_UseCylinder46LA));
        J.SetVisible("labFix3FullPlace", (FIX3_FULL_PLACE==Fix3K_ShortShuttle || FIX3_FULL_PLACE==Fix3K_UseStepperMotor));
        J.SetVisible("ledFix3FullPlace", (FIX3_FULL_PLACE==Fix3K_ShortShuttle || FIX3_FULL_PLACE==Fix3K_UseStepperMotor));
        J.SetVisible("gbSLKClamp", (INSTALL_SOCKET_CLAMP));
        J.SetVisible("gbSocketClamp", (INSTALL_SOCKET_CLAMP));
        {
            static const char* const kPre[] = { "ledSnLoaderIsPreAlarm", "ledSnEmptyIsPreAlarm", "ledSnColorIsPreAlarm", "ledSnAuto1IsPreAlarm",
                "ledSnAuto2IsPreAlarm", "ledSnAuto3IsPreAlarm", "ledSnAuto4IsPreAlarm", "ledSnAuto5IsPreAlarm", "ledSnAuto6IsPreAlarm",
                "ledSnLoaderIsPreAlarm_U", "ledSnEmptyIsPreAlarm_U", "ledSnColorIsPreAlarm_U", "ledSnAuto1IsPreAlarm_U", "ledSnAuto2IsPreAlarm_U",
                "ledSnAuto3IsPreAlarm_U", "ledSnAuto4IsPreAlarm_U", "ledSnAuto5IsPreAlarm_U", "ledSnAuto6IsPreAlarm_U" };
            for(unsigned k=0; k<sizeof(kPre)/sizeof(kPre[0]); ++k)                       // :514-531
                J.SetVisible(kPre[k], (CUSTOMER_CODE==CC_KYEC_CHEN || CUSTOMER_CODE==CC_HONPREC_QC));
        }
        J.SetVisible("gbOCR", (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR));       // :532
        J.SetVisible("gbDoubleEP", (INSTALL_DOUBLE_EP==1 || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI));
        J.SetVisible("grpArm2EP", (EP_Install==5));
        J.SetVisible("btnSwIonFanClean", ((USE_AutoCleanIonFan && SW[SwIonFanClean].Enable==true) || CUSTOMER_CODE==CC_HONPREC_QC));
        J.SetVisible("labIonFanClean", ((USE_AutoCleanIonFan && SW[SwIonFanClean].Enable==true) || CUSTOMER_CODE==CC_HONPREC_QC));
        J.SetVisible("gbATCTJSwitch", (SW[SwTjSignal01].Enable || SW[SwTjSignal02].Enable || SW[SwTjSignal05].Enable || SW[SwTjSignal06].Enable));
        J.SetVisible("btnSwTjSignal01", (SW[SwTjSignal01].Enable));                      // :538
        J.SetVisible("btnSwTjSignal02", (SW[SwTjSignal02].Enable));
        J.SetVisible("btnSwTjSignal03", (SW[SwTjSignal03].Enable));
        J.SetVisible("btnSwTjSignal04", (SW[SwTjSignal04].Enable));
        J.SetVisible("btnSwTjSignal05", (SW[SwTjSignal05].Enable));
        J.SetVisible("btnSwTjSignal06", (SW[SwTjSignal06].Enable));
        J.SetVisible("btnSwTjSignal07", (SW[SwTjSignal07].Enable));
        J.SetVisible("btnSwTjSignal08", (SW[SwTjSignal08].Enable));
        J.SetVisible("pnlFix45", (AUTO_EMPTY_COLOR>=3));                                 // :546
    #endif

    J.SetVisible("gbIonFanPower", (ION_FAN_TYPE==e2IoforOne));                           // :549
    if(ION_FAN_TYPE==e1IOforOne || ION_FAN_TYPE==e2IoforOne)
    {
        J.SetCaption("lblIonFan01", AnsiString("Ion Fan 1"));
        J.SetCaption("lblIonFan02", AnsiString("Ion Fan 2"));
        J.SetCaption("lblIonFan03", AnsiString("Ion Fan 3"));
        J.SetCaption("lblIonFan04", AnsiString("Ion Fan 4"));
        J.SetCaption("lblIonFan05", AnsiString("Ion Fan 5"));
        J.SetVisible("lblIonFan06", true);
        J.SetVisible("lblIonFan07", true);
        J.SetVisible("lblIonFan08", true);
        J.SetVisible("lblIonFan09", true);
        J.SetVisible("lblIonFan10", true);
        J.SetVisible("lblIonFan11", true);
        J.SetVisible("ledIonFan06", true);
        J.SetVisible("ledIonFan07", true);
        J.SetVisible("ledIonFan08", true);
        J.SetVisible("ledIonFan09", true);
        J.SetVisible("ledIonFan10", true);
        J.SetVisible("ledIonFan11", true);
    }
    else if(ION_FAN_TYPE==eUnInstallIonFan)                                              // :570
    {
        J.SetVisible("grpIonFanStatus", false);
    }
    else if(ION_FAN_TYPE==e5IOforAll)                                                    // :574
    {
        J.SetCaption("lblIonFan01", AnsiString("IonFanAlarm"));
        J.SetCaption("lblIonFan02", AnsiString("IonFanLevelAlarm"));
        J.SetCaption("lblIonFan03", AnsiString("IonBarrierAlarm"));
        J.SetCaption("lblIonFan04", AnsiString("IonBarrierLevelAlarm"));
        J.SetCaption("lblIonFan05", AnsiString("IonBarrierConditionAlarm"));
        J.SetVisible("lblIonFan06", false);
        J.SetVisible("lblIonFan07", false);
        J.SetVisible("lblIonFan08", false);
        J.SetVisible("lblIonFan09", false);
        J.SetVisible("lblIonFan10", false);
        J.SetVisible("lblIonFan11", false);
        J.SetVisible("ledIonFan06", false);
        J.SetVisible("ledIonFan07", false);
        J.SetVisible("ledIonFan08", false);
        J.SetVisible("ledIonFan09", false);
        J.SetVisible("ledIonFan10", false);
        J.SetVisible("ledIonFan11", false);
    }
    J.SetVisible("ledIonFan13", false);                                                  // :594
    J.SetVisible("ledIonFan14", false);
    J.SetVisible("ledSnSystemPower", false);

    bool bUseArtCatchTray=(Cylinder[C_TrayCover].Enable ||                               // :598
                           USE_CATCH_TRAY_MODEL==2 ||
                           USE_CATCH_TRAY_MODEL==3 ||
                           USE_CATCH_TRAY_MODEL==4);

    if(TRAY_ARM_MODE==eUnderCoveyor)                                                     // :603
    {
        J.SetTabVisible("tsUnderArm", true);
        J.SetTabVisible("tsRTArm", false);
        J.SetTabVisible("tsTrayArm", false);
        J.SetVisible("bplC_TrayX_UpDown", false);
        J.SetVisible("ledC_TrayX_UpDown_Off", false);
        J.SetVisible("lbTrayArmSafePos", false);
        J.SetVisible("ledSnTrayArmSafePos", false);
    }
    else if(IniConfig.bC03UseCatchTray==true)                                            // :613
    {
        J.SetTabVisible("tsUnderArm", false);
        J.SetTabVisible("tsRTArm", bUseArtCatchTray);
        J.SetTabVisible("tsTrayArm", !bUseArtCatchTray);
        J.SetVisible("bplC_CatchTray_Fix", !bUseArtCatchTray);
        J.SetVisible("btnC_CatchTray_FixOn", bUseArtCatchTray);
        J.SetVisible("btnC_CatchTray_FixOff", bUseArtCatchTray);
        J.SetVisible("ledC_CatchTray_Fix_On", !bUseArtCatchTray);
        J.SetVisible("ledC_CatchTray_Fix_Off", !bUseArtCatchTray);
        J.SetVisible("bplC_TurnTrayArm", bUseArtCatchTray);
        J.SetVisible("ledC_CatchTray_FixOn_On", bUseArtCatchTray);
        J.SetVisible("ledC_CatchTray_FixOff_On", bUseArtCatchTray);
        J.SetVisible("btnC_TurnTrayArmLock", bUseArtCatchTray);
        J.SetVisible("ledC_TurnTrayArm_On", bUseArtCatchTray);

        J.SetVisible("myldlnSnCatchTrayFix1On", (USE_CATCH_TRAY_MODEL==3));              // :629
        J.SetVisible("myldlnSnCatchTrayFix2On", (USE_CATCH_TRAY_MODEL==3));
        J.SetVisible("ledSnTrayCover", (USE_CATCH_TRAY_MODEL==3));
        J.SetVisible("bplC_TrayCover", (USE_CATCH_TRAY_MODEL==2));
        J.SetVisible("ledC_TrayCover_On", (USE_CATCH_TRAY_MODEL==2));

        J.SetVisible("ledSnCatchTrayFix1On", true);                                      // :635
        J.SetVisible("ledSnCatchTrayFix2On", true);
        J.SetVisible("palTrayArm", false);
        J.SetVisible("bplCatchSuck_On", false);
        J.SetVisible("bplCatchSuck_Off", false);
        J.SetVisible("ledCatchSuck", false);
    }
    else
    {
        J.SetTabVisible("tsUnderArm", false);                                            // :644
        J.SetTabVisible("tsTrayArm", true);
        J.SetTabVisible("tsRTArm", false);
        J.SetVisible("bplC_CatchTray_Fix", false);
        J.SetVisible("ledC_CatchTray_Fix_On", false);
        J.SetVisible("ledC_CatchTray_Fix_Off", false);
        J.SetVisible("ledSnCatchTrayFix1On", false);
        J.SetVisible("ledSnCatchTrayFix2On", false);
        J.SetVisible("bplC_TurnTrayArm", false);
        J.SetVisible("btnC_TurnTrayArmLock", false);
        J.SetVisible("ledC_TurnTrayArm_On", false);
        J.SetVisible("palTrayArm", true);
        J.SetVisible("bplCatchSuck_On", true);
        J.SetVisible("bplCatchSuck_Off", true);
        J.SetVisible("ledCatchSuck", true);
        J.SetVisible("bplC_TrayCover", (USE_AUTO_RETEST==eartInstall));
        J.SetVisible("ledC_TrayCover_On", (USE_AUTO_RETEST==eartInstall));
        J.SetVisible("ledSnTrayCover", false);
    }

    J.SetVisible("grpShtRotate", (IniConfig.bRotateShNeedCheck && IniConfig.bHaveRotateShuttle));   // :664

    if(NUMBER_PANEL_TYPE==2)                                                             // :666
    {
        static const char* const kBin[] = { "btnSwLoaderBin", "btnSwEmpty1Bin", "btnSwEmpty2Bin", "btnSwAuto1Bin", "btnSwAuto2Bin",
            "btnSwAuto3Bin", "btnSwFix1Bin", "btnSwFix2Bin", "btnSwFix3Bin", "btnSwFix4Bin", "btnSwFix5Bin", "btnSwFix6Bin" };
        for(unsigned k=0; k<sizeof(kBin)/sizeof(kBin[0]); ++k) J.SetVisible(kBin[k], true);
    }

    if(CUSTOMER_CODE==CC_HONPREC_QC)                                                     // :682
    {
        J.SetEnabled("btnAllVacuum", true);
        J.SetEnabled("btnAllDestory", true);
        J.SetVisible("btnAllVacuum", true);
        J.SetVisible("btnAllDestory", true);
        J.SetVisible("grpShtRotate", true);
    }

    if(INDEX_SUCKER_TYPE==1)                                                             // :691
    {
        J.SetVisible("ledNegAir1", true);
        J.SetVisible("lblNegAir1", true);
        if(Sen[SnNegativePressureAir2].Enable==false)
        {
            J.SetVisible("ledNegAir2", false);
            J.SetVisible("lblNegAir2", false);
        }
        else
        {
            J.SetVisible("ledNegAir2", true);
            J.SetVisible("lblNegAir2", true);
        }
    }
    else
    {
        J.SetVisible("ledNegAir1", false);
        J.SetVisible("lblNegAir1", false);
        J.SetVisible("ledNegAir2", false);
        J.SetVisible("lblNegAir2", false);
    }

    J.SetVisible("grpTesterDryAir", Sen[SnTesterDryAir].Enable);                         // :714
    /* :716-719 ResetIndexSuck / ResetIndexDestroy / Left / Top -- NOT copied */
    if(SYN_TEK_MOTION_MODULE==kG9004_M204)                                               // :720
        J.SetVisible("btnLatchCheck", true);
    else
        J.SetVisible("btnLatchCheck", false);

    B_LabSiteMap(J);                                                                     // :727
    J.SetVisible("bplC_HotplateVibration", (Cylinder[C_HotplateVibration].Enable==true));   // :728

    J.SetVisible("grpIndexChangeToque", USE_IO_CHANGE_TOQUE);                            // :730

    J.SetVisible("btnSwCCDCooling", (REAL_TIME_CCD || ATC_SYSTEM>eATC60));               // :732
    if(REAL_TIME_CCD)
        J.SetCaption("btnSwCCDCooling", AnsiString("CCD Cooling"));
    else
        J.SetCaption("btnSwCCDCooling", AnsiString("Ion Air"));

    J.SetVisible("gbSocketSensor", IniConfig.bC08_SocketSensor);                         // :743

    J.SetVisible("gbOneTouchDocking", (USE_OTD==1?true:false));                          // :745
    J.SetVisible("gbOneTouchDocking_2", (USE_OTD==2?true:false));
    J.SetVisible("gbOneTouchDocking_3", ((USE_OTD==0 && IniConfig.bA05UseAutoDocking)?true:false));

    J.SetVisible("btnSwTesterPower", SW[SwTesterPower].Enable);                          // :752
    J.SetVisible("grpGroundMan", Sen[SnGroundMan].Enable);                               // :754

    B_Hide_1032_IO(J);                                                                   // :769
    J.SetVisible("grpTesterDocking", IniConfig.bEnable_SECS_GEM);                        // :770
    J.SetVisible("gbRotateKIT", (USE_ROTATE_KIT==1));
    J.SetVisible("gbPreciser", USE_PRECISER);                                            // :773

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                  // :775
    {
        J.SetCaption("lblSenAutoDocking360KG", AnsiString("480KG"));
        J.SetCaption("lblC_DockXAxisOff_On_360", AnsiString("480KG OFF"));
        J.SetCaption("lblC_DockXAxisOn_On_360", AnsiString("480KG ON"));
        J.SetVisible("ASEBuffer", true);
    }

    if(INDEX_PRESS_TYPE==e500KG && USE_OTD==2)                                           // :784
    {
        J.SetCaption("lblSenAutoDocking240KG", AnsiString("500KG Y"));
        J.SetCaption("lblSenAutoDocking360KG", AnsiString("500KG X"));
        J.SetCaption("lblC_DockYAxisOn_On_240", AnsiString("500KG Y ON"));
        J.SetCaption("lblC_DockYAxisOff_On_240", AnsiString("500KG Y OFF"));
        J.SetCaption("lblC_DockXAxisOn_On_360", AnsiString("500KG X ON"));
        J.SetCaption("lblC_DockXAxisOff_On_360", AnsiString("500KG X OFF"));
    }

    if(IniConfig.bP35TrayArm)                                                            // :794
    {
        J.SetVisible("ledSnTrayArmSafePos", true);
        J.SetVisible("lbTrayArmSafePos", true);
    }

    J.SetVisible("grpWaterLeakageSensor", (ATC_SYSTEM!=eATCUninstall));                  // :800
    J.SetVisible("gbAOA", MACHINE_HAS_AUTO_ALIGNMENT_CCD);
    /* :802 btIPSetting->Click() -- NOT copied (imgOther picture from BmpPath) */
    J.SetVisible("grpHotGunFlow", HotGunFlowEnable);                                     // :803
    if(INSTALL_DOUBLE_EP==1 || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                       // :805
    {
        J.SetVisible("gbDoubleEP", true);
    }

    {   // :811-828 every lblXxxZ ("Z not homed") hidden first
        static const char* const kZ[] = { "lblLoaderZ", "lblEmptyZ", "lblColorZ", "lblAuto1Z", "lblAuto2Z", "lblAuto3Z", "lblAuto4Z", "lblAuto5Z", "lblAuto6Z",
            "lblLoaderZ_U", "lblEmptyZ_U", "lblColorZ_U", "lblAuto1Z_U", "lblAuto2Z_U", "lblAuto3Z_U", "lblAuto4Z_U", "lblAuto5Z_U", "lblAuto6Z_U" };
        for(unsigned k=0; k<sizeof(kZ)/sizeof(kZ[0]); ++k) J.SetVisible(kZ[k], false);
    }
    {   // :830-985 a lane whose Z is a motor: its two buttons Enabled and "Z not homed" shown while MOT[].HomeFlag==false.
        //   (the OnClick=BtnPanelClick rewiring of a non-motor lane is behaviour, done by C++ io.btnPanelClick's kZ table)
        struct ZLane { int k; bool needsAEC; const char* a; const char* b; const char* aU; const char* bU; const char* lbl; const char* lblU; int mot; };
        const ZLane kL[] = {
            { 0, false, "btnC_Load_Middle", "btnC_Load_Up", "btnC_Load_Middle_U", "btnC_Load_Up_U", "lblLoaderZ", "lblLoaderZ_U", MLoaderZ },              // :830
            { 1, true,  "btnC_Empty_Middle", "btnC_Empty_Up", "btnC_Empty_Middle_U", "btnC_Empty_Up_U", "lblEmptyZ", "lblEmptyZ_U", MEmptyZ },           // :847
            { 2, true,  "btnC_Color_Middle", "btnC_Color_Up", "btnC_Color_Middle_U", "btnC_Color_Up_U", "lblColorZ", "lblColorZ_U", MColorZ },           // :864
            { 3, false, "btnC_Auto1_Selector", "btnC_Auto1_Up", "btnC_Auto1_Selector_U", "btnC_Auto1_Up_U", "lblAuto1Z", "lblAuto1Z_U", MAuto1Z },      // :881
            { 4, false, "btnC_Auto2_Selector", "btnC_Auto2_Up", "btnC_Auto2_Selector_U", "btnC_Auto2_Up_U", "lblAuto2Z", "lblAuto2Z_U", MAuto2Z },      // :898
            { 5, false, "btnC_Auto3_Selector", "btnC_Auto3_Up", "btnC_Auto3_Selector_U", "btnC_Auto3_Up_U", "lblAuto3Z", "lblAuto3Z_U", MAuto3Z },      // :915
            { 6, false, "btnC_Auto4_Selector", "btnC_Auto4_Up", "btnC_Auto4_Selector_U", "btnC_Auto4_Up_U", "lblAuto4Z", "lblAuto4Z_U", MAuto4Z },      // :936
            { 7, false, "btnC_Auto5_Selector", "btnC_Auto5_Up", "btnC_Auto5_Selector_U", "btnC_Auto5_Up_U", "lblAuto5Z", "lblAuto5Z_U", MAuto5Z },      // :953
            { 8, false, "btnC_Auto6_Selector", "btnC_Auto6_Up", "btnC_Auto6_Selector_U", "btnC_Auto6_Up_U", "lblAuto6Z", "lblAuto6Z_U", MAuto6Z } };   // :970
        for(unsigned i=0; i<sizeof(kL)/sizeof(kL[0]); ++i)
        {
            const bool motor = LOAD_Z_USE_MOTOR[kL[i].k] && (!kL[i].needsAEC || AUTO_EMPTY_COLOR!=0);
            if(!motor) continue;
            J.SetEnabled(kL[i].a, true);  J.SetEnabled(kL[i].b, true);
            J.SetEnabled(kL[i].aU, true); J.SetEnabled(kL[i].bU, true);
            J.SetVisible(kL[i].lbl, (MOT[kL[i].mot].HomeFlag==false));
            J.SetVisible(kL[i].lblU, (MOT[kL[i].mot].HomeFlag==false));
            if(kL[i].k==5 && IniConfig.bVTESTFunction==true && USE_AUTO_RETEST==eartUninstall)   // :930
                J.SetVisible("btnC_Auto3_Up", false);
        }
    }

    J.SetVisible("ledSnTesterAlarm", IniConfig.bVTESTFunction);                          // :987
    J.SetVisible("lblSnTesterAlarm", IniConfig.bVTESTFunction);

    J.SetVisible("BtnChamboCoolDown", SW[SwAutoCoolDown].Enable);                        // :990
    J.SetTabVisible("tsOption1", (USE_E84_Sensor ||                                      // :991
                                  AUTO3_IS_MAGAZINE==1 ||
                                  USE_AOI_Inspection ||
                                  USE_Fix_AI_CCD));

    J.SetVisible("ledSnLoaderTrayHasTray_AGV", (USE_E84_Sensor || USE_COVER_TRAYID!=tCIDNotUse));   // :996
    J.SetVisible("ledSnEmptyTrayHasTray_AGV", USE_E84_Sensor);
    J.SetVisible("ledSnColorTrayHasTray_AGV", USE_E84_Sensor);
    /* :1000 SetTechDataToProd() -- NOT copied */

    if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true)                       // :1002
    {
        J.SetVisible("grpWinWayGroup", true);
        /* :1005-1006 Top=152 / Left=112 (geometry) */
    }
    else
    {
        J.SetVisible("grpWinWayGroup", false);
    }
    J.SetVisible("sb_IO_CommunicationPad", iControlPanelMode);                           // :1012
    J.SetVisible("grpPickUpErrorPlacement", (USE_InPlacement==eartInstall));
    J.SetVisible("grpMagazine", (AUTO3_IS_MAGAZINE==1));
    J.SetVisible("grpColorSensor", (USE_COLORSENSOR_MUN!=eCSMUN_Uninstall));
    J.SetVisible("pnlTriTemp", (MachineTypeChoice==Type_HT1032));
    J.SetVisible("pnl8PickerInArm", (USE_PICKER_COUNT!=ep16Picker));
    J.SetVisible("pnl16PickerInArm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnl8PickerOutArm", (USE_PICKER_COUNT!=ep16Picker));
    J.SetVisible("pnl16PickerOutArm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gpSafePLC", (IsSafePLCIOInstall()));   //AI(W906-W217) 20261010 (Ifor01): golden 913 iosetview.cpp:1136
    J.SetVisible("grpLoaderFunc", (USE_LD_Rot_Arm==1));                                  // :1022

    // golden Timer1Timer :185-196 (every 50 ms while the form is open): evaluated here once per open
    if(IniConfig.bIOFormCanControlHeaterFan)
    {
        if(bHeaterDoorIsOpen[0] || bHeaterDoorIsOpen[1] ||
           bHeaterDoorIsOpen[2] || bHeaterDoorIsOpen[3])
            J.SetVisible("btnSwHeaterFan", true);
        else
            J.SetVisible("btnSwHeaterFan", false);
    }
}

static void Display(FormState& J)
{
    B_FormShow(J);
}

// AI(W906-IOSV-SHOWALL) 20261002: golden chkShowIndexAllClick (iosetview.cpp:1760-1766), the one handler on this form that only
//   repaints: every Index / InArm / OutArm sucker panel drawn "in use" with the Index sucker names, and the box hides itself.
//   WS form.event {form:"Tfiosetview", control:"chkShowIndexAll", event:"click"}; page/ht9045_iosetview_formshow_c.js sends it.
static void E_chkShowIndexAllClick(FormState& J)
{
    // :1762 if(fShow==false) return; -- fShow is true while the form is shown (FormShow :387); the page sends this from the open
    //   window only (RunEvent's chain check: chkShowIndexAll is visible only when FormShow :374 made it so)
    B_ShowSuckMode(J, -1);                                                               // :1764
    J.SetVisible("chkShowIndexAll", false);                                              // :1765
}
// golden DFM (iosetview.dfm): chkShowIndexAll and its containers have no Visible / Enabled lines = both true at design time
static const EventGuard kChain_chkShowIndexAll[] = {
    { "chkShowIndexAll", true, true }, { "tsIndexVacuum", true, true }, { "pgcVacuum", true, true },
    { "pnlVacuum", true, true }, { "tsSucker", true, true }, { "PC_IOSET", true, true } };
static const EventDesc kEvents[] = {
    { "chkShowIndexAll", "click", "iosetview.cpp:1760 Tfiosetview::chkShowIndexAllClick", &E_chkShowIndexAllClick,
      kChain_chkShowIndexAll, (int)(sizeof(kChain_chkShowIndexAll) / sizeof(kChain_chkShowIndexAll[0])) } };

}  // namespace f_Tfiosetview

extern const BridgeDesc kBridge_Tfiosetview = {
    "HW.IoSetView.html", "Tfiosetview", "iosetview.cpp",
    &f_Tfiosetview::Display, nullptr, nullptr,   // display only: no save bridge (the IO table saves through the engine's sysGrid)
    "",
    "",
    f_Tfiosetview::kEvents, (int)(sizeof(f_Tfiosetview::kEvents) / sizeof(f_Tfiosetview::kEvents[0])),   // AI(W906-IOSV-SHOWALL) 20261002; the IO buttons go through io.btnPanelClick
    ""
};

}  // namespace formbridge
}  // namespace ht9045
