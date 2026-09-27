// =============================================================================
//  cSecurity.cpp  --  TfSecurity + TMySecurity (golden cSecurity.cpp, 1,772
//                      lines) + the free function ChangePassword
//
//  AI(W906-FW-SecCC) 20260819: new file. See forms/fSecurity.h for the full
//  translation-status table, CTOR SAFETY argument, GATE REGISTER, CONSUMER
//  REGISTER and STUB COLLISION search. This file only carries per-statement
//  pointer-back comments to that banner.
// =============================================================================
#include "forms/fSecurity.h"

#include "vclcompat/vcl_compat.h"   // AnsiString, FileExists
#include "common.h"                  // INIFileGeneral, CheckAndReadIniData/WriteIniData
#include "canary_support.h"            // ShowErrorMessage

// S19: targeted forward declarations instead of `#include "cMyDB.h"` --
// cMyDB.h and canary_support.h both declare RecordProcess/MyDBIProcessNew
// with CONFLICTING default arguments (illegal to specify twice) and
// MyDBIProcessNew's __fastcall marking differs between the two headers.
// uYieldMonitoring.cpp already hit and documented this exact conflict this
// wave ("including cMyDB.h+canary_support.h in one TU errors either order,
// so no TU ever saw both decls"; -fsyntax-only reproduces it verbatim here
// too). Fixing either shared header is out of this wave's write boundary
// (forms/fSecurity.h, forms/fCounterClear.h, cSecurity.cpp, cCounterClear.cpp,
// tests/test_security_core.cpp, tests/test_counterclear_core.cpp only).
// These three signatures are copied verbatim (including __fastcall/no-
// __fastcall) from cMyDB.h:81/:114/:124 -- the real bodies already exist and
// link against cMyDB.h's own declarations elsewhere in the tree.
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2="");
void GetJameCodeOfAxis(int iAxis, TComboBox *ComboBox);
void __fastcall GetAlarmCodeList(TStringGrid *strGrid);
#include "cprod.h"                      // LevelSet, WriteData/ReadData
#include "cpublic.h"
#include "cmydef.h"                      // CUSTOMER_CODE + CC_*, AccessLevel, MMSystem, iDef*Level
#include "MachineType.h"                  // CheckRange<T>
#include "Config.h"                        // IniConfig
#include "CosFunction.h"                     // CosFunction
#include "forms/fMain.h"                      // GATE (SEC4) evidence only (fMain->AlarmUnitMap absent)

#include <cstdlib>   // atoi

//---------------------------------------------------------------------------
// AI(W906-FW-SecCC) 20260819: global homecomed unconditionally -- the SIOF
// guard lives INSIDE the ctor, wrapping only the risky statement group. See
// forms/fSecurity.h CTOR SAFETY.
TfSecurity *fSecurity = new TfSecurity();
int iMaxLevelItem = 0;                                                           // golden cSecurity.cpp:19 (file-scope global) -- stays 0 while GATE (SEC1) is closed
int iBit8 = 0;                                                                    // golden cSecurity.cpp:20 (file-scope global, DISTINCT from TfSecurity::iBit8 the member -- golden bug B10, see below)
TObject *DefaultImg = 0;                                                          // golden `Graphics::TBitmap *DefaultImg;` -- retyped, see GATE (SEC1) (Graphics::TBitmap has no port; this pointer is never assigned a real value while SEC1 stays closed)
//---------------------------------------------------------------------------
//  B10 (golden bug, cosmetic/no behavioural impact): golden declares BOTH a
//  file-scope global `int iBit8;` (cSecurity.cpp:20) AND a same-named public
//  MEMBER `int iBit8;` on TfSecurity (cSecurity.h:145). Inside any TfSecurity
//  member function, unqualified `iBit8` resolves to the MEMBER (ordinary C++
//  name hiding), so the file-scope global is permanently unreachable from
//  this class's own code -- and neither one is ever read or written by any
//  translated method (grepped golden cSecurity.cpp in full this wave: zero
//  uses of either `iBit8`). Both are carried here for structural fidelity
//  only; this note exists so a future reader does not mistake the shadowing
//  for a bug THIS translation introduced.
//---------------------------------------------------------------------------
TfSecurity::TfSecurity()                                                          // golden :23-239
{                                                                               //Steven 20130611 : 權限設定改為表格式方式
    // GATE (SEC1): see forms/fSecurity.h banner -- DefaultImg assignment +
    // the whole 178-entry mySecurityPal population + the iMaxLevelItem/
    // SetVisible loop. forms/fMain.h has ZERO `TSpeedButton *sbXXX` members
    // (confirmed 20260819); `Graphics::TBitmap` has no port anywhere.
    // mySecurityPal stays empty; iMaxLevelItem (file-scope global above)
    // stays 0 -- see forms/fSecurity.h "EMERGENT BEHAVIOUR" for what that
    // does to Insufficient() today.
#if 0
    DefaultImg=fMain->sbSetting->Glyph;

    mySecurityPal.push_back(new TMySecurity("[00] Main - Tools",                                    fMain->sbSetting->Glyph,        sbMain));
    mySecurityPal.push_back(new TMySecurity("[01] Main - Config",                                   fMain->sbConfig->Glyph,         sbMain));
    mySecurityPal.push_back(new TMySecurity("[02] Main - Offset",                                   fMain->sbOffset->Glyph,         sbMain));
    mySecurityPal.push_back(new TMySecurity("[03] Main - Speed",                                    fMain->sbSpeed->Glyph,          sbMain));
    mySecurityPal.push_back(new TMySecurity("[04] Main - IO",                                       fMain->sbIO->Glyph,             sbMain));
    mySecurityPal.push_back(new TMySecurity("[05] Main - Message",                                  fMain->sbMessage->Glyph,        sbMain));
    mySecurityPal.push_back(new TMySecurity("[06] Main - Exit",                                     fMain->sbCloseProgram->Glyph,   sbMain));
    mySecurityPal.push_back(new TMySecurity("[07] Main - Hot Mode",                                 fMain->sbTempOffset->Glyph,     sbMain));
    mySecurityPal.push_back(new TMySecurity("[08] Main - Tester On/Off line",                       fMain->sbTester->Glyph,         sbMain));
    mySecurityPal.push_back(new TMySecurity("[09] Main - Setup File",                               fMain->sbTester->Glyph,         sbMain));                   //kevin 20180917 add setup file
    mySecurityPal.push_back(new TMySecurity("[10] Main - Site Select",                              fMain->sbTester->Glyph,         sbMain));
    mySecurityPal.push_back(new TMySecurity("[11] Main - Real/Dummy",                               fMain->sbStartMode->Glyph,      sbMain));
    mySecurityPal.push_back(new TMySecurity("[12] Main - Start Mode",                               fMain->sbStartMode->Glyph,      sbMain));
    mySecurityPal.push_back(new TMySecurity("[13] Main - Auto Teach Close",                         fMain->sbStartMode->Glyph,      sbMain));                   //JimmyChiu 20211020 : Auto alignment mode
    mySecurityPal.push_back(new TMySecurity("[14] Tools - Tray Form",                               fMain->sbTrayForm->Glyph,       sbTools));
    mySecurityPal.push_back(new TMySecurity("[15] Tools - Plate Form",                              fMain->sbPlateForm->Glyph,      sbTools));
    mySecurityPal.push_back(new TMySecurity("[16] Tools - Tray Assign",                             fMain->sbTrayAssign->Glyph,     sbTools));
    mySecurityPal.push_back(new TMySecurity("[17] Tools - Temp.OffSet",                             fMain->sbTempOffset->Glyph,     sbTools));
    mySecurityPal.push_back(new TMySecurity("[18] Tools - Contact",                                 fMain->sbContact->Glyph,        sbTools));
    mySecurityPal.push_back(new TMySecurity("[19] Tools - Test IF",                                 fMain->sbTester->Glyph,         sbTools));
    mySecurityPal.push_back(new TMySecurity("[20] Tools - Bin",                                     fMain->sbBin->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[21] Tools - Set Up",                                  fMain->sbSetup->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[22] Tools - Load/Unld",                               fMain->sbLdUld->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[23] Config - Builder",                                fMain->sbBuilder->Glyph,        sbConfig));
    mySecurityPal.push_back(new TMySecurity("[24] Config - Start Mode",                             fMain->sbStartMode->Glyph,      sbConfig));
    mySecurityPal.push_back(new TMySecurity("[25] Config - C.Select",                               fMain->sbSelete->Glyph,         sbConfig));
    mySecurityPal.push_back(new TMySecurity("[26] Config - C.Clear",                                fMain->sbClear->Glyph,          sbConfig));
    mySecurityPal.push_back(new TMySecurity("[27] TestIF - InterFace Type",                         fMain->sbTester->Glyph,         sbSetup));                  //JimmyChiu 20211228 : InterFace Type
    mySecurityPal.push_back(new TMySecurity("[28] Config - Tower Light",                            fMain->sbTowerLight->Glyph,     sbConfig));
    mySecurityPal.push_back(new TMySecurity("[29] Config - Password",                               fMain->sbPassword->Glyph,       sbConfig));
    mySecurityPal.push_back(new TMySecurity("[30] Config - Configuration",                          fMain->sbConfiguration->Glyph,  sbConfig));
    mySecurityPal.push_back(new TMySecurity("[31] Config - DIO Setting",                            fMain->sbDioSet->Glyph,         sbConfig));
    mySecurityPal.push_back(new TMySecurity("[32] Main - Temperature Deg Setup",                    fMain->spbSet->Glyph,           sbMain));
    mySecurityPal.push_back(new TMySecurity("[33] Set Offset Limit",                                fMain->sbOffset->Glyph,         sbOther));
    mySecurityPal.push_back(new TMySecurity("[34] Tool Alarm",                                      DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[35] Alarm - Trouble Shooting",                        DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[36] Alarm - Low Yield Alarm",                         fMain->sbYield->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[37] Tools - CCD",                                     fMain->sbCCD->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[38] Contact - Contact Force",                         fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[39] Tools - Yield Monitoring",                        fMain->sbYield->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[40] Setup - Shuttle Mode",                            fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[41] Main - Change SetUp File",                        fMain->sbStartMode->Glyph,      sbMain));
    mySecurityPal.push_back(new TMySecurity("[42] Config - SECS\\GEM",                              fMain->sbSecsGem->Glyph,        sbConfig));
    mySecurityPal.push_back(new TMySecurity("[43] Tools - Auto Clean",                              fMain->sbAutoClean->Glyph,      sbTools));
    mySecurityPal.push_back(new TMySecurity("[44] Tools - ATC",                                     fMain->sbATC->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[45] Tools - Sensor Adj.",                             fMain->sbShuttleSensor->Glyph,  sbTools));
    mySecurityPal.push_back(new TMySecurity("[46] Config - Sensor Latch",                           fMain->sbSensorLatch->Glyph,    sbConfig));
    mySecurityPal.push_back(new TMySecurity("[47] Config - Omron Temp.",                            fMain->sbOmron->Glyph,          sbConfig));
    mySecurityPal.push_back(new TMySecurity("[48] Tools - OCR",                                     fMain->sbOCR->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[49] Config - Auto Temp.",                             fMain->sbAutoTemp->Glyph,       sbConfig));
    mySecurityPal.push_back(new TMySecurity("[50] Temp.OffSet - Temp. Mode",                        fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[51] Temp.OffSet - Default Offset",                    fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[52] Temp.OffSet - Base Point",                        fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[53] Temp.OffSet - User Offser",                       fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[54] Temp.OffSet - Single Limits",                     fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[55] Temp.OffSet - Hot Mode",                          fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[56] Temp.OffSet - Amb Mode",                          fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[57] Temp.OffSet - Index Mode",                        fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[58] IO - STACK 1",                                    fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[59] IO - STACK 2",                                    fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[60] IO - SUCKER",                                     fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[61] IO - SHUTTLE",                                    fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[62] IO - KEY PAD",                                    fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[63] IO - SYSTEM",                                     fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[64] IO - INDEX",                                      fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[65] IO - TTL",                                        fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[66] IO - TOOLS",                                      fMain->sbIO->Glyph,             sbIO));
    mySecurityPal.push_back(new TMySecurity("[67] Configure - A[Function]",                         fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[68] Configure - C[Hardware]",                         fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[69] Configure - D[Index]",                            fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[70] Configure - E[In/Out Arm]",                       fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[71] Configure - F[Shuttle]",                          fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[72] Configure - G[Visible]",                          fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[73] Configure - I[Tester]",                           fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[74] Configure - L[Temperature]",                      fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[75] Configure - O[Count]",                            fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[76] Configure - N[Network]",                          fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[77] Configure - P[Tray]",                             fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[78] Configure - Tray Data",                           fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[79] Configure - Hot Plate Data",                      fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[80] Tools - Dyna. Temperature",                       fMain->sbDynamicTemp->Glyph,    sbTools));
    mySecurityPal.push_back(new TMySecurity("[81] Yield - Yield Alarm",                             fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[82] Yield - Piggy-Back Funtions",                     fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[83] Yield - Alarm",                                   fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[84] Yield - Failure rate Alarm",                      fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[85] Tools - QA Mode",                                 fMain->sbQAMode->Glyph,         sbTools));
    mySecurityPal.push_back(new TMySecurity("[86] Config - Motion View",                            fMain->sbMotionView->Glyph,     sbConfig));
    mySecurityPal.push_back(new TMySecurity("[87] Main - Teaching",                                 fMain->sbTeaching->Glyph,       sbMain));
    mySecurityPal.push_back(new TMySecurity("[88] Tools - BarCode",                                 fMain->sbBarCode->Glyph,        sbTools));
    mySecurityPal.push_back(new TMySecurity("[89] Tools - Rotate",                                  fMain->sbRotate->Glyph,         sbTools));
    mySecurityPal.push_back(new TMySecurity("[90] Config - Air Con.",                               fMain->spbAirConditioner->Glyph,sbConfig));
    mySecurityPal.push_back(new TMySecurity("[91] TestIF - Test Time",                              fMain->sbTester->Glyph,         sbSetup));
    mySecurityPal.push_back(new TMySecurity("[92] Contact - Contact parameter",                     fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[93] Contact - Handler Mode",                          fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[94] Setup - Site Map",                                fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[95] Setup - Shuttle Mode",                            fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[96] Configure - M[Monitor]",                          fMain->sbConfiguration->Glyph,  sbConfiguration));
    mySecurityPal.push_back(new TMySecurity("[97] Other - Auto Clean Clear Count",                  fMain->sbAutoClean->Glyph,      sbOther));
    mySecurityPal.push_back(new TMySecurity("[98] Other - QA Mode Device Count",                    fMain->sbQAMode->Glyph,         sbOther));
    mySecurityPal.push_back(new TMySecurity("[99] Tools - Socket",                                  fMain->sbSocket_ASE_KR->Glyph,  sbTools));
    mySecurityPal.push_back(new TMySecurity("[100] Tools - Laser Sensor",                           fMain->spbLaser->Glyph,         sbTools));
    mySecurityPal.push_back(new TMySecurity("[101] Contact - Height",                               fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[102] Other - Loader Tray Mode",                       fMain->sbTester->Glyph,         sbOther));
    mySecurityPal.push_back(new TMySecurity("[103] Contact - Test Socket IC check",                 fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[104] Other - Tray Edit",                              fMain->sbTrayForm->Glyph,       sbOther));
    mySecurityPal.push_back(new TMySecurity("[105] Main - Big Fan",                                 fMain->spbFan->Glyph,           sbMain));
    mySecurityPal.push_back(new TMySecurity("[106] Temp.OffSet - Initial Temp Offset",              fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[107] Other - Yield Count Clean",                      fMain->sbContact->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[108] Other - Bin Clean Count",                        fMain->sbContact->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[109] Contact - Sensor Adjustment",                    fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[110] PM Alarm - User PM Alarm Level",                 fMain->sbPMAlarm->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[111] PM Alarm - Item Setup",                          fMain->sbPMAlarm->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[112] PM Alarm - Visible Setup",                       fMain->sbPMAlarm->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[113] PM Alarm - Visible Executive",                   fMain->sbPMAlarm->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[114] ATC Control",                                    fMain->sbATC->Glyph,            sbOther));
    mySecurityPal.push_back(new TMySecurity("[115] Tray - Load/Unload",                             fMain->sbLdUld->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[116] Tray - Knocker",                                 fMain->sbLdUld->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[117] Yield - Alarm4",                                 fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[118] Yield - Yield Alarm Enable",                     fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[119] Yield - Piggy-Back Funtions Enable",             fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[120] Yield - Alarm Enable",                           fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[121] Yield - Alarm4 Enable",                          fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[122] Other - Shuttle Sensor Move",                    fMain->sbContact->Glyph,        sbOther));
    mySecurityPal.push_back(new TMySecurity("[123] TestIF -Use initial start delay in socket",      fMain->sbTester->Glyph,         sbSetup));
    mySecurityPal.push_back(new TMySecurity("[124] Temp.OffSet - EOT Extra Temp Offset",            fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[125] Yield - Alarm Control Access",                   fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[126] Yield - Failure Count",                          fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[127] Config - Monitor View",                          fMain->sbMonitorView->Glyph,    sbConfig));
    mySecurityPal.push_back(new TMySecurity("[128] PE Model Control",                               DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[129] Yield - ART User Fix",                           fMain->spbAutoRetest->Glyph,    sbYield));
    mySecurityPal.push_back(new TMySecurity("[130] Yield - ART Setting",                            fMain->spbAutoRetest->Glyph,    sbYield));
    mySecurityPal.push_back(new TMySecurity("[131] TestIF - Start Delay",                           fMain->sbTester->Glyph,         sbSetup));
    mySecurityPal.push_back(new TMySecurity("[132] Bin - Double Contact",                           fMain->sbBin->Glyph,            sbSetup));
    mySecurityPal.push_back(new TMySecurity("[133] Speed - Auto Speed",                             fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[134] Speed - [Index Arm] Wait Time",                  fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[135] Speed - [Index Arm] Destroy Time and Count",     fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[136] Speed - [Input Arm] Function for small package", fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[137] Speed - [In Arm] Release Delay",                 fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[138] Speed - [In Arm] Retry Count and Wait Time",     fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[139] Speed - [In Arm] Destroy Time and Count",        fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[140] Speed - [In Arm] Two Speed Move Down",           fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[141] Speed - [Out Arm] Function for small package",   fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[142] Speed - [Out Arm] Retry Count and Wait Time",    fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[143] Speed - [Out Arm] Destroy Time and Count",       fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[144] Speed - [Out Arm] Destroy Check",                fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[145] Speed - [Out Arm] Two Speed Move Down",          fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[146] Speed - [Tray Arm] Wait Time",                   fMain->sbSpeed->Glyph,          sbOther));
    mySecurityPal.push_back(new TMySecurity("[147] Speed - Auto Skip",                              fMain->sbSpeed->Glyph,          sbMain));
    mySecurityPal.push_back(new TMySecurity("[148] Tools - [Bin] Yield Control",                    fMain->sbBin->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[149] Tools - [Bin] Tray Setting",                     fMain->sbBin->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[150] Tools - RPDefault",                              fMain->sbSpeed->Glyph,          sbTools));
    mySecurityPal.push_back(new TMySecurity("[151] Setup - ERMS Selection",                         fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[152] Contact - Contact Force Calibration",            fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[153] Yield - Alarm5 Option",                          fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[154] Yield - Alarm5 Value",                           fMain->sbYield->Glyph,          sbYield));
    mySecurityPal.push_back(new TMySecurity("[155] Setup - 12/16 Site Direct Heater Layout",        fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[156] Other - Pass Download MOFile",                   fMain->sbStartMode->Glyph,      sbOther));
    mySecurityPal.push_back(new TMySecurity("[157] Other - Check Online/Real Status",               fMain->sbStartMode->Glyph,      sbOther));
    mySecurityPal.push_back(new TMySecurity("[158] Temp.OffSet - Active ATC Cooling",               fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[159] Setup - Socket sensor",                          fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[160] Other - Unloader Tray Edit",                     fMain->sbTrayForm->Glyph,       sbOther));
    mySecurityPal.push_back(new TMySecurity("[161] Tools - Barcode Sub-Function",                   fMain->sbBarCode->Glyph,        sbTools));
    mySecurityPal.push_back(new TMySecurity("[162] Yield - Break ART",                              fMain->spbAutoRetest->Glyph,    sbYield));
    mySecurityPal.push_back(new TMySecurity("[163] Contact Count Alarm Edit Permission",            DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[164] Other - Server on/off line",                     DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[165] Setup - Arm 1 pick, arm 2 test function",        fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[166] Other - Level for continuous alarm [O17]",       fMain->sbConfiguration->Glyph,  sbOther));
    mySecurityPal.push_back(new TMySecurity("[167] Setup - Enable auto site mapping function",      fMain->sbSetup->Glyph,          sbSetup));
    mySecurityPal.push_back(new TMySecurity("[168] Other - Default Recipe Setting",                 DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[169] Other - Default Recipe Reply",                   DefaultImg,                     sbOther));
    mySecurityPal.push_back(new TMySecurity("[170] Tools - ScanAOI",                                fMain->sbCCD->Glyph,            sbTools));
    mySecurityPal.push_back(new TMySecurity("[171] Contact - Contact height",                       fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[172] Main - Disable Site Map",                        fMain->spbAutoRetest->Glyph,    sbMain));
    mySecurityPal.push_back(new TMySecurity("[173] Contact - Release height",                       fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[174] Tools - Magazine",                               fMain->sbMagazine->Glyph,       sbTools));
    mySecurityPal.push_back(new TMySecurity("[175] Temp.OffSet - Kit Temp. offset",                 fMain->sbTempOffset->Glyph,     sbTemp));
    mySecurityPal.push_back(new TMySecurity("[176] Contact - RTC Auto Tuning",                      fMain->sbContact->Glyph,        sbContact));
    mySecurityPal.push_back(new TMySecurity("[177] [I49] - Offline clean out all ic",               fMain->spbAutoRetest->Glyph,    sbYield));
    mySecurityPal.push_back(new TMySecurity("[178] Tools - Tray Function",                          fMain->spbTrayMapping->Glyph,   sbTools));
    //目前為256, 超過時候要多修改AccessLevel的陣列大小----------------------------------------------

    iMaxLevelItem=mySecurityPal.size();
    for(int i=0; i<iMaxLevelItem; i++)
    {
        mySecurityPal[i]->SetVisible(true);
    }
#endif

    // AI(W906-FW-SecCC) 20260819: SIOF guard -- see forms/fSecurity.h CTOR
    // SAFETY. GetLevelSet()/the Jam-code-populate loop/AddAlarmList() all
    // read cross-TU globals (CosFunction/IniConfig/CUSTOMER_CODE) that golden
    // guarantees are already constructed (this form is built inside WinMain,
    // strictly after every other global). INIFileGeneral!=0 reproduces that
    // precondition instead of running these at static init (common.h:51;
    // cObserver.cpp:428 / cShowBinSelect.cpp:181 precedent, same sentinel).
    if (INIFileGeneral != 0)
    {
        GetLevelSet();                                                              //Steven 20140222 : 統一LevelSet檔案改為Function
        FileNameJam000="D:\\HT9045\\Error\\English\\JAM0000.dat";
        if(FileExists(FileNameJam000)==false)                                       //Steven 20140222 End: Alarm Code設定權限
        {
            for(int i=0; i<cbJamArea->Items->Count; i++)
            {
                cbJamArea->ItemIndex=i;
                cbJamArea->Refresh();
                GetJameCodeOfAxis(cbJamArea->ItemIndex+1, cbJamCode);

                for(int j=0; j<cbJamCode->Items->Count; j++)
                {
                    cbJamCode->ItemIndex=j;
                    cbJamCode->Refresh();
                    cbJamLang->ItemIndex=0;
                    cbJamLang->Refresh();
                    JamArea=cbJamArea->Text;
                    JamCode=cbJamCode->Text.SubString(1, cbJamCode->Text.AnsiPos("  :")-1);
                    ChangeJamMessage(false);
                    ChangeJamMessage();
                }
            }
        }
        AddAlarmList();                                                             //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    }
}
//---------------------------------------------------------------------------
void TfSecurity::FormDestroy(TObject *Sender)                                    // golden :241-258
{
    try
    {
        for(std::vector<TMySecurity *>::iterator iter=mySecurityPal.begin(); iter!=mySecurityPal.end(); ++iter)
        {
            delete *iter;
        }
        mySecurityPal.clear();                                                     // golden `vec_clr(mySecurityPal);`
        DefaultImg=0;
        // B11 (golden bug, protective-by-accident): golden sets DefaultImg=
        // NULL then immediately `delete DefaultImg;` -- deletes NULL (a
        // documented C++ no-op), NOT the real object DefaultImg pointed at.
        // DefaultImg is a BORROWED pointer (golden :26, `fMain->sbSetting->
        // Glyph`, owned by fMain's TSpeedButton) -- had this line actually
        // deleted the real pointer, it would double-free/dangle a widget
        // fMain still owns. The bug's practical effect is therefore
        // protective (a genuine delete-of-real-object here would be the
        // actual bug); translated verbatim regardless, per project policy.
        delete DefaultImg;                                                      //Steven 20160108 : release memory
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfSecurity::FormDestroy");
    }
    LogSoftwareOffTime("TfSecurity, FormDestroy");                              //Steven 20210526 : 紀錄關機的時間
}
//---------------------------------------------------------------------------
void TfSecurity::FormShow(TObject *Sender)                                       // golden :260-436
{
    GetLevelSet();                                                              //Steven 20140222 : 統一LevelSet檔案改為Function

    for(int i=0; i<iMaxLevelItem && i<(int)mySecurityPal.size(); i++)          //[W906] 20260927 偏離 golden（跟 FormClose :511 同一個保護，decisions R62）：golden iMaxLevelItem==mySecurityPal.size()（:210）；移植樹 W906_SecurityBoot 設 180 但 mySecurityPal 在 GATE (SEC1) 是空的 —— 原句 i=0 就讀空 vector 的 [0]。目前沒有人呼叫 FormShow（WebSecurityJam 刻意避開），這是預防
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE &&
           (i==35  ||
            i==104 ||
            i==114 ||
            i==128))
        {
            mySecurityPal[i]->SetEnabled(false);
        }
        else if(i==163)                                                         //Contact Count Alarm Edit Permission
        {
            mySecurityPal[i]->SetEnabled(false);
        }
        mySecurityPal[i]->SetLevel(LevelSet.AccessLevel[i]);
    }

    // NOTE: golden also sets `Left=50; Top=10;` here (TForm position) -- no
    // form-level geometry is modelled by this facade, same posture as
    // forms/fContactCT.h / forms/fCounterClear.h.

    #ifndef SOFT_SIMULTE
    SecurityPalVisible();                                                       //Steven 20250430 : 開起來
    #endif

    cbUnlockPassWord->Visible   =CosFunction.bUseAlarmUnlockPassWord;           //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
    cbIncludeMTBA->Visible      =CosFunction.bIncludeMTBA;                      //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
    chkCheckContAlarm->Visible  =CosFunction.bConAlarmNeedKeyInPassword;        //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量
    chkO17->Visible             =CosFunction.bConAlarmInTimeLevelUp;            //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限

    cbAddAlarmLog->Visible         =CUSTOMER_CODE==CC_PTI;                      //Sam 20210611 : Alarm Log 可以自訂該不需要記 Log 名稱

    cbN27AlarmSel->Visible          =CosFunction.bUseAlarmLogXml;
    cbN27AlarmSelByArea->Visible    =CosFunction.bUseAlarmLogXml;
    cbN27AddBoard->Visible          =(CosFunction.bUseAlarmLogXml &&
                                      CUSTOMER_CODE==CC_SIGURD_ChungXing);      //Sam 20210911 :  南特中興系統要求顯示增加開啟
    chkTCPAlarm->Visible            =CosFunction.bEnableHandlerResultServer;    //Sam 20230426 : 通知客戶系統 Handler 已經密碼鎖定
    cbContAlarmNotUpload->Visible   =CosFunction.bOLPFunction;                  //Sam 20231116 : 新增 Alarm 不需上傳伺服器
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        if(rgJamLevel->Columns<=4)
        {
            rgJamLevel->Columns=5;
            if(CUSTOMER_CODE==CC_KYEC_LEE)                                      //wei 20160505 增加PE權限
                rgJamLevel->Items->Insert(0, "Operator");
            else
                rgJamLevel->Items->Insert(0, "Open");
        }

        switch(AccessLevel)
        {
            case 4:                                                             //HonPrec
                btnHonPrec->Visible=true;
                sbSupervisor->Visible=true;
                sbEngineer->Visible=true;
                PageControl1->Visible=true;
                break;
            case 3:                                                             //Supervisor
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=true;
                sbEngineer->Visible=true;

                if(IniConfig.bSPILFunction==true ||                             //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                   CUSTOMER_CODE==CC_SCS)                                       //jou 2015-08-27 SCS 要求 Jam Level 要可以自訂級別
                    PageControl1->Visible=false;
                else
                    PageControl1->Visible=true;
                break;
            case 2:                                                             //Engineer
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=false;
                sbEngineer->Visible=true;
                PageControl1->Visible=false;
                break;
            case 1:                                                             //OP
            case 0:                                                             //Open
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=false;
                sbEngineer->Visible=false;
                PageControl1->Visible=false;
                break;
        }
    }
    else
    {
        switch(AccessLevel)
        {
            case 3:                                                             //HonPrec
                btnHonPrec->Visible=true;
                sbSupervisor->Visible=true;
                sbEngineer->Visible=true;
                PageControl1->Visible=true;
                break;
            case 2:                                                             //Supervisor
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=true;
                sbEngineer->Visible=true;

                if(IniConfig.bSPILFunction==true ||                             //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                   CUSTOMER_CODE==CC_SCS)                                       //jou 2015-08-27 SCS 要求 Jam Level 要可以自訂級別
                    PageControl1->Visible=false;
                else
                    PageControl1->Visible=true;
                break;
            case 1:                                                             //Engineer
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=false;
                sbEngineer->Visible=true;
                PageControl1->Visible=false;
                break;
            case 0:                                                             //OP
                btnHonPrec->Visible=false;
                sbSupervisor->Visible=false;
                sbEngineer->Visible=false;
                PageControl1->Visible=false;
                break;
        }
    }

    PageControl1->ActivePageIndex=0;
    // GATE (SEC-TAB): golden :382-414's ScrollBox/panel-arrangement block --
    // needs `TScrollBox` (sbMain/sbTools/.../sbOther are TPanel stand-ins,
    // S12) AND `->ControlCount`/`->Controls[i]` (no TPageControl/TTabSheet/
    // TPanel exposes a child-control list in vclcompat). Its entire purpose
    // is positioning the (always-empty this wave, GATE SEC1) mySecurityPal
    // panels -- see forms/fSecurity.h banner.
#if 0
    int iPitch=52, iStart=2;                                                    //重新排列位置 Start
    for(int iP=0; iP<PageControl1->ControlCount; iP++)                          //找出所有TabSheet
    {
        TControl *P=PageControl1->Controls[iP];
        TTabSheet *TabPtr=dynamic_cast <TTabSheet *>(P);
        if(TabPtr!=NULL)
        {
            for(int j=0; j<TabPtr->ControlCount; j++)                           //找出TabSheet裡面的ScrollBox
            {
                TControl *S=TabPtr->Controls[j];
                TScrollBox *sbPtr=dynamic_cast <TScrollBox *>(S);
                if(sbPtr!=NULL)                                                 //找出所有ScrollBox裡面的所有Panel並排好
                {
                    sbPtr->VertScrollBar->Position=0;                           //jou 2012-11-07 修正畫面顯示問題
                    iStart=2;
                    for(int k=0; k<sbPtr->ControlCount; k++)
                    {
                        TControl *pal=sbPtr->Controls[k];
                        TPanel *palPtr=dynamic_cast <TPanel *>(pal);
                        if(palPtr!=NULL)
                        {
                            if(palPtr->Visible==true)
                            {
                                palPtr->Top=iStart;
                                iStart+=iPitch;
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    cbJamNeedRed->Visible=(IniConfig.bAlarmMustRedColor==true);                 //Steven 20111116 : 特殊Alarm需要改紅底
    cbJamArea->ItemIndex=0;                                                     //Steven 20140222 Start: Alarm Code設定權限
    cbJamArea->Refresh();
    GetJameCodeOfAxis(cbJamArea->ItemIndex+1, cbJamCode);
    cbJamCode->ItemIndex=0;
    cbJamCode->Refresh();
    cbJamLang->ItemIndex=0;
    cbJamLang->Refresh();
    JamArea=cbJamArea->Text;
    JamCode=cbJamCode->Text.SubString(1, cbJamCode->Text.AnsiPos("  :")-1);
    ChangeJamMessage(false);

    btnOperator->Visible=CosFunction.bSecurityHave5Level;                       //jou 2014-06-19 Security Have 5 Level
    rgMachineStatusBit8->Visible=(CUSTOMER_CODE!=CC_SCK);
    chkAlarmAfterFullTray->Visible=(CosFunction.bNeedAlarmAfterUnloaderFull &&
                                    cbJamArea->Text=="02 Output Arm");          //Jimmychiu 20240902 : Need Alarm After Unloader Full
    fShow=true;
}
//---------------------------------------------------------------------------
void TfSecurity::FormClose()                                                     // golden :438-468 (Sender/TCloseAction dropped, see forms/fSecurity.h banner)
{   extern bool W906_FormCloseSkipPassword;                                    //[W906] 20260927 (Steven 團隊): 定義在本檔檔尾；Q24 排後的 SavePassword／ReadPassword 開關（見 :532/:535）
    for(int i=0; i<iMaxLevelItem && i<(int)mySecurityPal.size(); i++)          //[W906] 偏離 golden：Steven 20260927 Q30=B 範圍保護。golden iMaxLevelItem==mySecurityPal.size()（:210）；移植樹 W906_SecurityBoot 設 180 但 mySecurityPal 在 GATE (SEC1) 是空的 —— 原句 i=0 就讀空 vector 的 [0]（未定義行為、實際是讀位址 0 當機）。保護後 0 次：網頁的值已由 WebLevelSet.cpp 放進 LevelSet
    {
        LevelSet.AccessLevel[i]=mySecurityPal[i]->GetLevel();
        mySecurityPal[i]->SetEnabled(false);                                    //wei 20150803 防止顯示錯線
        mySecurityPal[i]->SetEnabled(true);
    }

    if(LevelSet.AccessLevel[87]<iDefSupervisorLevel)                            //Steven 20120830 : Teaching按鈕權限
        LevelSet.AccessLevel[87]=iDefSupervisorLevel;

    if(LevelSet.AccessLevel[129]>LevelSet.AccessLevel[130])                     //Steven 20161201 : For SCK 93K ART
        LevelSet.AccessLevel[129]=LevelSet.AccessLevel[130];

    if(CUSTOMER_CODE!=CC_SIGURD_PeiXing)                                        //Alick 20160901 modify for 矽格北興
    {
        if(LevelSet.AccessLevel[86]<iDefEngineerLevel)                          //Steven 20120830 : Motion View按鈕權限
            LevelSet.AccessLevel[86]=iDefEngineerLevel;
    }

    SaveJamLevel();
    if(W906_FormCloseSkipPassword==false) SavePassword();                      //[W906] 20260927：網頁存檔路徑（WebLevelSet.cpp）暫時跳過 —— Q24=B「照 golden 呼叫，但排後」；旗標預設 false＝golden 原樣
    SetLevelSet();                                                              //Steven 20140222 : 統一LevelSet檔案改為Function

    if(W906_FormCloseSkipPassword==false) ReadPassword();                      //[W906] 20260927：同 :532
    fShow=false;
}
//---------------------------------------------------------------------------
// GATE (SEC3): entire body -- see forms/fSecurity.h banner. Every statement
// indexes `mySecurityPal[N]` for a fixed N up to 178; with the vector empty
// (GATE SEC1) any such index is undefined behaviour, not a compile error --
// gated as a unit rather than left to fault the first time FormShow's
// `#ifndef SOFT_SIMULTE SecurityPalVisible();#endif` runs.
//---------------------------------------------------------------------------
void TfSecurity::SecurityPalVisible()                                            // golden :470-571
{
#if 0
    mySecurityPal[ 9]->SetVisible(true);                                        //kevin 20180917
    mySecurityPal[27]->SetVisible(CUSTOMER_CODE==CC_Greatek);                   //JimmyChiu 20211228 : InterFace Type
    mySecurityPal[33]->SetVisible(false);
    mySecurityPal[34]->SetVisible(false);
    mySecurityPal[35]->SetVisible(IniConfig.bSPILFunction==false &&
                                  CUSTOMER_CODE!=CC_SCS);
    mySecurityPal[36]->SetVisible(false);
    mySecurityPal[37]->SetVisible(IniConfig.bEnableCCDUSETCPIP ||
                                  REAL_TIME_CCD ||
                                  USE_Scanner_AOI_Inspection ||
                                  USE_Top_Scanner_AOI_Inspection);
    mySecurityPal[40]->SetVisible(IniConfig.bShuttleModeAccseeLevel);
    mySecurityPal[41]->SetVisible(CosFunction.bSetupFileNameControlByLevel);
    mySecurityPal[42]->SetVisible(IniConfig.bEnable_SECS_GEM);
    mySecurityPal[43]->SetVisible(IniConfig.bEnableAutoCleanFunction);
    mySecurityPal[44]->SetVisible((ATC_SYSTEM==eATCHonPrecType));
    mySecurityPal[45]->SetVisible(SHUTTLE_SENSOR_TYPE==eSensorCCLink ||
                                  SHUTTLE_SENSOR_TYPE==eSensorCCLink3 ||
                                  UseCanBusOrEtherCAT());
    mySecurityPal[46]->SetVisible((SYN_TEK_MOTION_MODULE==G9004_M204));
    mySecurityPal[47]->SetVisible((USE_16_HEATER==eht16HeaterEJ1N ||
                                   USE_16_HEATER==eht32HeaterEJ1N ||
                                   USE_16_HEATER==eht16HeaterDTME08 ||
                                   USE_16_HEATER==eht32HeaterDTME08));
    mySecurityPal[48]->SetVisible(INSTALL_OCR);
    mySecurityPal[49]->SetVisible(CosFunction.bAutoKTemp);
    mySecurityPal[80]->SetVisible(IniConfig.bC04EnableTestTempIC);
    mySecurityPal[83]->SetVisible(CUSTOMER_CODE!=CC_ASE_CL);
    mySecurityPal[84]->SetVisible(false);
    mySecurityPal[85]->SetVisible(IniConfig.bQAMode);
    mySecurityPal[88]->SetVisible(BAR_CODE_INSTALL!=ebctUninstall);
    mySecurityPal[89]->SetVisible(USE_ROTATE_KIT==1);
    mySecurityPal[90]->SetVisible(USE_AIR_CONDITIONER==2);
    mySecurityPal[95]->SetVisible(false);
    mySecurityPal[96]->SetVisible(CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[97]->SetVisible(IniConfig.bEnableAutoCleanFunction);
    mySecurityPal[98]->SetVisible(IniConfig.bQAMode);
    mySecurityPal[99]->SetVisible(IniConfig.bSocketCommunication);
    mySecurityPal[100]->SetVisible(USE_LASER_DISTANCE);
    mySecurityPal[116]->SetVisible(CosFunction.bKnockerSetBySetupFile);
    mySecurityPal[110]->SetVisible(CosFunction.bUsePMAlarmFunction);
    mySecurityPal[111]->SetVisible(CosFunction.bUsePMAlarmFunction);
    mySecurityPal[112]->SetVisible(CosFunction.bUsePMAlarmFunction);
    mySecurityPal[113]->SetVisible(CosFunction.bUsePMAlarmFunction);
    mySecurityPal[117]->SetVisible(CosFunction.bYieldAlarm4);
    mySecurityPal[118]->SetVisible(CUSTOMER_CODE==CC_Greatek);
    mySecurityPal[119]->SetVisible(CUSTOMER_CODE==CC_Greatek);
    mySecurityPal[120]->SetVisible(CUSTOMER_CODE==CC_Greatek);
    mySecurityPal[121]->SetVisible(CosFunction.bYieldAlarm4);
    mySecurityPal[122]->SetVisible(CUSTOMER_CODE==CC_Greatek);
    mySecurityPal[125]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[126]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[128]->SetVisible(CosFunction.bUsePEModelFunction);
    mySecurityPal[129]->SetVisible(CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);
    mySecurityPal[130]->SetVisible(CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);
    mySecurityPal[131]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[132]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[133]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[134]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[135]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[136]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[137]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[138]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[139]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[140]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[141]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[142]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[143]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[144]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[145]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[146]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[147]->SetVisible(CUSTOMER_CODE==CC_ASE_CL);
    mySecurityPal[148]->SetVisible(CosFunction.bUseYieldControlFunction);
    mySecurityPal[149]->SetVisible(IniConfig.bKoreaFunction);
    mySecurityPal[150]->SetVisible(CosFunction.bRecipeParameterDefault);
    mySecurityPal[151]->SetVisible(IniConfig.bEnableErms);
    mySecurityPal[152]->SetVisible(WEIGHT_CALIBRATION ||
                                   CosFunction.bUseDynamicKitDiameter);
    mySecurityPal[158]->SetVisible(CUSTOMER_CODE==CC_Microchip_Phil);
    mySecurityPal[159]->SetVisible(IniConfig.bC08_SocketSensor);
    mySecurityPal[160]->SetVisible(CosFunction.bUnloaderEditTrayLevelSet);
    mySecurityPal[161]->SetVisible(CUSTOMER_CODE==CC_SCK);
    mySecurityPal[162]->SetVisible(CUSTOMER_CODE==CC_TERAPOWER &&
                                   CosFunction.bUseSCKART &&
                                   USE_AUTO_RETEST==eartInstall &&
                                   IniConfig.bA10_AutoReTest);
    mySecurityPal[163]->SetVisible(CosFunction.bUseHeadContactCount);
    mySecurityPal[164]->SetVisible(CUSTOMER_CODE==CC_Murata);
    mySecurityPal[165]->SetVisible(IniConfig.bD58UseArm1PickPlaceArm2Test);
    mySecurityPal[166]->SetVisible(CosFunction.bConAlarmInTimeLevelUp);
    mySecurityPal[167]->SetVisible(CosFunction.bI21EnableASMByRecipe);
    mySecurityPal[168]->SetVisible(CosFunction.bRecipeParameterDefault);
    mySecurityPal[169]->SetVisible(CosFunction.bRecipeParameterDefault);
    mySecurityPal[170]->SetVisible(USE_AOI_Inspection ||
                                   USE_Scanner_AOI_Inspection);
#endif
}
//---------------------------------------------------------------------------
bool TfSecurity::Insufficient(int iType, bool bAlarm)                            // golden :573-597
{
    if(iType==-1)                                                               //hontech權限
    {
        if(AccessLevel<iDefHonPrecLevel)                                        //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
        {
            return false;
        }
        return true;
    }

    if(iType<0)
        return false;
    if(iType>iMaxLevelItem)                                                     // iMaxLevelItem==0 while GATE (SEC1) is closed -- see forms/fSecurity.h "EMERGENT BEHAVIOUR"
        return false;

    if(AccessLevel<LevelSet.AccessLevel[iType])
    {
        if(bAlarm==true)
            ShowErrorMessage("WAR1676", 0, MMSystem);                           //Insufficient privileges

        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
//  ChangePassword (free function, golden cSecurity.cpp:599-811, file-local
//  in golden too -- never declared in cSecurity.h, called only from this
//  file's own click handlers)
//---------------------------------------------------------------------------
// GATE (SEC7): entire body -- see forms/fSecurity.h banner. `fLogin`
// (TfLogin) has no facade or stub anywhere in this tree (0 hits for
// "class TfLogin"/"fLogin;", 0 files named login.h outside generated
// tools/dfm2rc artifacts). Every branch dereferences fLogin->..., so there
// is no partial evaluation. DEFAULT: no-op (the change-password dialog does
// not run) -- same "no modal dialog surface yet" posture as GATE (C2)/S13.
//---------------------------------------------------------------------------
void ChangePassword(int iLevel)                                                  // golden :599-811
{
    (void)iLevel;
#if 0
    AnsiString asLevel=iLevel;
    int iUseID=iLevel-1;
    bool bChange=false;

    TStringList *List, *ListChange;
    AnsiString str, asName, asLevel1, asPass, asList;
    char cStr[256];
    char dest[20];

    if(FileExists(pwPath))                                                      //2012-01-03    Dell modify
    {
        if(IniConfig.bPasswordSecret)                                           //jou 2013-01-04 Password Txt 加密
        {
            bChange=CheckAndReadIniDataGeneral("Password", "Change", false);
            if(bChange==false)
            {
                ListChange=new TStringList;
                ListChange->Clear();
            }
        }

        List=new TStringList;
        List->Clear();
        List->LoadFromFile(pwPath);                                             //2012-01-03    Dell modify
        int index, icount;

        fLogin->labUserName->Visible=true;
        fLogin->edUserName->Visible=true;
        fLogin->rgLoginOption->Visible=true;
        fLogin->cbLoginUserName->Visible=true;
        fLogin->cbLoginUserName->Clear();
        for(int i=0; i<List->Count; i++)
        {
            strncpy(cStr, List->Strings[i].c_str(), sizeof(cStr));

            SplitStrByDotSpaceOnly(cStr, dest, 20);                             //Name
            asName=dest;
            SplitStrByDotSpaceOnly(cStr, dest, 20);                             //Level
            asLevel1=dest;
            if(atoi(dest)==atoi(asLevel.c_str()))
            {
                fLogin->cbLoginUserName->Items->Add(asName);
            }

            if(IniConfig.bPasswordSecret && bChange==false)                     //jou 2013-01-04 Password Txt 加密
            {
                SplitStrByDotSpaceOnly(cStr, dest, 20);                         //Password
                asPass=dest;
                asPass=EncodeStr(asPass);

                asList.sprintf("%s %s %s", asName, asLevel1, asPass);
                ListChange->Add(asList);
            }
        }

        if(IniConfig.bPasswordSecret && bChange==false)                         //jou 2013-01-04 Password Txt 加密
        {
            WriteIniDataGeneral("Password","Change",true);
            List->Clear();
            delete List;
            ListChange->SaveToFile(pwPath);
            ListChange->Clear();
            delete ListChange;
            ShowMyMessage("Password Secret finish!!");
            return;
        }

        fLogin->ShowModal();
        fLogin->labUserName->Visible=false;
        fLogin->edUserName->Visible=false;
        fLogin->rgLoginOption->Visible=false;
        fLogin->cbLoginUserName->Visible=false;
        fLogin->edUserName->Text=fLogin->edUserName->Text.Trim();
        fLogin->edLoginOldPassword->Text=fLogin->edLoginOldPassword->Text.Trim();

        if(IniConfig.bPasswordSecret && bChange==false)
        {
            ListChange->Clear();
            delete ListChange;                                                  //Steven 20160912 : Add delete for save memory
        }

        if(fLogin->edUserName->Text.IsEmpty())
        {
            ShowErrorMessage("WAR1678", 0, MMSystem);                           //User Name or Password need KeyIn
            List->Clear();
            delete List;                                                        //Steven 20160912 : Add delete for save memory
            return;
        }
        else if(fLogin->edLoginOldPassword->Text.IsEmpty())
        {
            if(fLogin->rgLoginOption->ItemIndex !=1)                            //2012-01-03    Dell modify 刪除不需要password
            {
                ShowErrorMessage("WAR1678", 0, MMSystem);                       //User Name or Password need KeyIn
                List->Clear();
                delete List;                                                    //Steven 20160912 : Add delete for save memory
                return;
            }
        }

        switch(fLogin->rgLoginOption->ItemIndex)
        {
            case 0:                                                             //New
                for(int i=0; i<List->Count; i++)
                {
                    index=List->Strings[i].Pos(fLogin->edUserName->Text);
                    if(index==1)
                    {
                        ShowErrorMessage("WAR1672", 0, MMSystem);               //User Name Already exists
                        List->Clear();
                        delete List;                                            //Steven 20160912 : Add delete for save memory
                        return;
                    }
                }

                if(IniConfig.bPasswordSecret)                                   //jou 2013-01-04 Password Txt 加密
                {
                    asPass=EncodeStr(fLogin->edLoginOldPassword->Text);
                    str   =fLogin->edUserName->Text+" "+asLevel+" "+asPass;
                }
                else
                {
                    str   =fLogin->edUserName->Text+" "+asLevel+" "+fLogin->edLoginOldPassword->Text;
                }
                List->Add(str);
                ShowErrorMessage("MES1673", 0, MMSystem);                       //New User Finish
                break;
            case 1:                                                             //Delete
                str  =fLogin->edUserName->Text+" "+asLevel;                     //2012-01-03    Dell modify 刪除不需要password
                index=-1;
                for(int i=0; i<List->Count; i++)
                {
                    icount=List->Strings[i].Pos(str);
                    if(icount>0)
                    {
                        index=i;
                        break;
                    }
                }

                if(Application->MessageBox("Sure delete??", "", MB_YESNO+MB_ICONQUESTION+MB_TOPMOST)==IDNO)
                {
                    break;
                }

                if(index!=-1)
                {
                    List->Delete(index);
                    ShowErrorMessage("MES1674", 0, MMSystem);                   //Delete User Finish
                }
                else
                {
                    ShowErrorMessage("WAR1677", 0, MMSystem);                   //UserName or PassWord Error
                }
                break;
            case 2:                                                             //Edit
                if(IniConfig.bPasswordSecret)                                   //jou 2013-01-04 Password Txt 加密
                {
                    asPass=EncodeStr(fLogin->edLoginOldPassword->Text);
                    str   =fLogin->edUserName->Text+" "+asLevel+" "+asPass;
                }
                else
                {
                    str   =fLogin->edUserName->Text+" "+asLevel+" "+fLogin->edLoginOldPassword->Text;
                }

                index=List->IndexOf(str);
                if(index!=-1)
                {
                    List->Delete(index);

                    if(IniConfig.bPasswordSecret)                               //jou 2013-01-04 Password Txt 加密
                    {
                        asPass=EncodeStr(fLogin->edLoginNewPassword->Text);
                        str   =fLogin->edUserName->Text+" "+asLevel+" "+asPass;
                    }
                    else
                    {
                        str   =fLogin->edUserName->Text+" "+asLevel+" "+fLogin->edLoginOldPassword->Text;
                    }
                    List->Add(str);
                    ShowErrorMessage("MES1675", 0, MMSystem);                   //Modify User Finish
                }
                else
                {
                    ShowErrorMessage("WAR1677", 0, MMSystem);                   //UserName or PassWord Error
                }
                break;
        }

        List->SaveToFile(pwPath);                                               //2012-01-03    Dell modify
        List->Clear();
        delete List;
    }
    else
    {
        if(iLevel>=iDefHonPrecLevel)                                            //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
            return;

        fLogin->ShowModal();

        if(fLogin->edLoginNewPassword->Text.IsEmpty())                          //jou 2014-07-06 修正未輸入密碼就存檔錯誤
        {
            ShowErrorMessage("WAR1678", 0, MMSystem);                           //User Name or Password need KeyIn
            return;
        }

        strncpy(USER.ID[iUseID]      , fLogin->edUserName->Text.c_str(), sizeof(USER.ID[iUseID]));
        strncpy(USER.PassWord[iUseID], fLogin->edLoginNewPassword->Text.c_str(), sizeof(USER.PassWord[iUseID]));        //Steven 20120608 : edLoginOldPassword --> edLoginNewPassword
        SavePassword();
    }
#endif
}
//---------------------------------------------------------------------------
void TfSecurity::SecurityExitClick(TObject *Sender)                              // golden :813-816
{
    // DEVIATION: golden `Close();` -- see forms/fSecurity.h banner
    // (ATCInterface.cpp:446 precedent).
    FormClose();                                                                //[W906] 20260927 Q30=B：網頁 Exit（WebLevelSet.cpp W906_LevelSetPut）接到這裡；越界保護在 FormClose 迴圈 :512
}
//---------------------------------------------------------------------------
//  B9 (golden bug): golden's TMySecurity ctor initializer is
//  `: TComponent(Owner)` -- but `Owner` is NOT a parameter of THIS
//  constructor (unlike TfSecurity's own ctor, which does take one).
//  Unqualified `Owner` at that point resolves (ordinary C++ member-function-
//  scope lookup, valid even inside a mem-initializer-list) to the INHERITED
//  `TComponent::Owner` -- which in real BCB6 VCL is a PROPERTY, not a
//  parameter or variable, backed by `FOwner`. Reading a base subobject's own
//  property to initialize THAT SAME base subobject, before it has been
//  constructed, is reading memory that has not been initialized yet --
//  practically, on a freshly `operator new`-allocated block, this observes
//  whatever bit pattern the allocator happened to hand back (commonly zero
//  on Windows for a first-touch page, but not guaranteed by the language).
//  This port's TComponent (vclcompat/Comm.h) exposes `Owner()` as a member
//  FUNCTION, not a property -- so a verbatim `: TComponent(Owner)` would not
//  even compile here (a function name does not implicitly convert to
//  `TComponent*`). Translated as `TComponent(0)`, matching golden's most
//  likely REAL runtime behaviour (freshly allocated memory reads back as
//  NULL far more often than not) rather than inventing a different value.
//  Moot in practice this wave: TMySecurity is never constructed at all
//  while GATE (SEC1) stays closed (mySecurityPal.push_back is the only
//  caller, and it is gated).
//---------------------------------------------------------------------------
TMySecurity::TMySecurity(AnsiString Caption, TControl *Sender)                   // golden :818-852
    : TComponent(0), index(0), Panel(0), SpeedButton(0), Visible(false), RadioGroup(0)
{
    // GATE (SEC10): entire body -- see forms/fSecurity.h banner. Panel/
    // RadioGroup/SpeedButton geometry (->Left/->Top/->Width/->Height) and
    // Name do not exist on vclcompat's TPanel/TRadioGroup/TSpeedButton.
    // TMySecurity is unreachable this wave (GATE SEC1) regardless.
#if 0
    Panel=new TPanel(this);
    Panel->Left     =0;
    Panel->Top      =0;
    Panel->Height   =50;
    Panel->Color    =TColor(0x00C2B8A6);
    Panel->BevelInner=bvNone;
    Panel->BevelOuter=bvNone;
    RadioGroup=new TRadioGroup(this);
    RadioGroup->Parent=Panel;
    SpeedButton=new TSpeedButton(this);
    SpeedButton->Parent=Panel;
    SpeedButton->Glyph=Img;
    AnsiString s="";
    int iSt=0, iEd=0,iSn=0;
    s=Caption;
    if(Caption.Length()>6)                                                      //Sam 20250827 : 修正 Change Log 疑似問題
    {
        iSt=s.Pos("[");
        iEd=s.Pos("]");
        if(iSt>0 && iEd>0)
        {
            s=s.SubString(iSt+1,iEd-2);
            iSn=atoi(s.c_str());
        }
    }
    else
    {
        iSn=0;
    }
    SetCaption(Caption, iSn);
    SetParent(Sender);                                                          //Steven 20130611 : 權限設定改為表格式方式
    Panel->Width    =Panel->Parent->Width-50;
#else
    (void)Caption;
    (void)Sender;
#endif
}
//---------------------------------------------------------------------------
TMySecurity::~TMySecurity()                                                      // golden :854-858 (empty in golden too)
{
//    delete RadioGroup;
//    delete SpeedButton;
}
//---------------------------------------------------------------------------
void TMySecurity::SetParent(TControl *Sender)                                    // golden :860-902
{
    // GATE (SEC10): entire body -- see forms/fSecurity.h banner.
#if 0
    Panel->Parent=Sender;
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
    {
        RadioGroup->Columns=5;
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //wei 20160505 增加PE權限
        {
            RadioGroup->Items->Add("Operator");
            RadioGroup->Items->Add("Engineer");
            RadioGroup->Items->Add("PEngineer");
            RadioGroup->Items->Add("Supervisor");
            RadioGroup->Items->Add("HonPrec");
        }
        else
        {
            RadioGroup->Items->Add("Open");
            RadioGroup->Items->Add("Operator");
            RadioGroup->Items->Add("Engineer");
            RadioGroup->Items->Add("Supervisor");
            RadioGroup->Items->Add("HonPrec");
        }
    }
    else
    {
        RadioGroup->Columns=4;
        RadioGroup->Items->Add("Operator");
        RadioGroup->Items->Add("Engineer");
        RadioGroup->Items->Add("Supervisor");
        RadioGroup->Items->Add("HonPrec");
    }
    RadioGroup->ItemIndex=0;
    RadioGroup->Width=458;
    RadioGroup->Height=42;
    RadioGroup->Top=1;
    RadioGroup->Left=310;
    SpeedButton->Width=300;
    SpeedButton->Height=42;
    SpeedButton->Top=4;
    SpeedButton->Left=7;
    SpeedButton->Layout=blGlyphLeft;
    SpeedButton->Margin=4;
#else
    (void)Sender;
#endif
}
//---------------------------------------------------------------------------
void TMySecurity::SetPosition(int Top, int Left)                                 // golden :904-908
{
    // GATE (SEC10): entire body -- see forms/fSecurity.h banner.
#if 0
    Panel->Top=Top;
    Panel->Left=Left;
#else
    (void)Top;
    (void)Left;
#endif
}
//---------------------------------------------------------------------------
void TMySecurity::SetCaption(AnsiString Caption, int index)                      // golden :910-920
{
    // GATE (SEC10): entire body -- see forms/fSecurity.h banner (Name does
    // not exist on TPanel/TRadioGroup/TSpeedButton; RadioGroup has no
    // Caption member either).
#if 0
    this->index=index;
    this->Caption=Caption;
    Panel->Name="MySecurity_Panel_"+AnsiString(index);
    Panel->Caption="";
    RadioGroup->Name="MySecurity_RadioGroup_"+AnsiString(index);
    RadioGroup->Caption="";
    SpeedButton->Name="MySecurity_SpeedButton_"+AnsiString(index);
    SpeedButton->Caption=Caption;
#else
    (void)Caption;
    (void)index;
#endif
}
//---------------------------------------------------------------------------
AnsiString TMySecurity::GetCaption()                                             // golden :922-925
{
    return SpeedButton->Caption;
}
//---------------------------------------------------------------------------
void TfSecurity::btnHonPrecClick(TObject *Sender)                                // golden :927-933
{
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
        ChangePassword(4);
    else
        ChangePassword(3);
}
//---------------------------------------------------------------------------
void TfSecurity::sbSupervisorClick(TObject *Sender)                              // golden :935-941
{
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
        ChangePassword(3);
    else
        ChangePassword(2);
}
//---------------------------------------------------------------------------
void TfSecurity::sbEngineerClick(TObject *Sender)                                // golden :943-949
{
    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19 Security Have 5 Level
        ChangePassword(2);
    else
        ChangePassword(1);
}
//---------------------------------------------------------------------------
void TfSecurity::cbJamAreaChange(TObject *Sender)                                // golden :951-962
{
    GetJameCodeOfAxis(cbJamArea->ItemIndex+1, cbJamCode);
    cbJamCode->ItemIndex=0;
    cbJamCode->Refresh();
    cbJamLang->ItemIndex=0;
    cbJamLang->Refresh();
    ChangeJamMessage();
    AnsiString sJamArea=cbJamArea->Text;
    chkAlarmAfterFullTray->Visible=(CosFunction.bNeedAlarmAfterUnloaderFull &&
                                    sJamArea=="02 Output Arm");                 //Jimmychiu 20240902 : Need Alarm After Unloader Full
}
//---------------------------------------------------------------------------
void TfSecurity::cbJamCodeChange(TObject *Sender)                                // golden :964-969
{
    cbJamLang->ItemIndex=0;
    cbJamLang->Refresh();
    ChangeJamMessage();
}
//---------------------------------------------------------------------------
void TfSecurity::cbJamLangChange(TObject *Sender)                                // golden :971-974
{
    ChangeJamMessage();
}
//---------------------------------------------------------------------------
void TfSecurity::ChangeJamMessage(bool bSave)                                    // golden :976-1060
{
    AnsiString FileName;

    if(bSave)
        SaveJamLevel();

    JamLang=cbJamLang->ItemIndex;
    JamArea=cbJamArea->Text;
    JamCode=cbJamCode->Text.SubString(1, cbJamCode->Text.AnsiPos("  :")-1);

    // GATE (SEC5): `RichEditJamCode->Font->Charset/Name` -- TControl (which
    // TMemo, the S11 stand-in, inherits through TCustomEdit) has no `Font`
    // member (vclcompat/Controls.h:213-223). Purely cosmetic on-screen font
    // selection; the file path chosen below is unaffected.
    if(JamLang==0)                                                              //English
    {
        FileName.sprintf("D:\\HT9045\\Error\\English\\%s.dat", JamCode);
    }
    else if(JamLang==1)                                                         //Chinese
    {
        FileName.sprintf("D:\\HT9045\\Error\\Chinese\\%s.dat", JamCode);
    }
    else if(JamLang==2)                                                         //Korean
    {
        FileName.sprintf("D:\\HT9045\\Error\\Korea\\%s.dat", JamCode);
    }
    else if(JamLang==3)                                                         //Singapore
    {
        FileName.sprintf("D:\\HT9045\\Error\\Singapore\\%s.dat", JamCode);
    }

    RichEditJamCode->Clear();
    if(FileExists(FileName))
        RichEditJamCode->Lines->LoadFromFile(FileName);

    cbJamNeedRed->Checked=GetJemRed(JamArea, JamCode);
    rgJamLevel->ItemIndex=GetJamLevel(JamArea, JamCode);
    cbSilentMode->Checked=GetJemSilent(JamArea, JamCode);                       //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
    rgMachineStatusBit8->ItemIndex=GetBit8(JamArea, JamCode);                   //Isaac 20170922 (Steven) : ATP Machine Status Bit8 Issue

    if(CosFunction.bUseAlarmUnlockPassWord==true)                               //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
        cbUnlockPassWord->Checked=GetJemUnlockPassWord(JamArea, JamCode);       //Ifor 20170214 (wei) add 可以自訂Alarm是否要有解除密碼

    if(CosFunction.bIncludeMTBA==true)                                          //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
        cbIncludeMTBA->Checked=GetJemIncludeMTBA(JamArea, JamCode);

    if(CosFunction.bUseAlarmLogXml)
    {
        cbN27AlarmSelByArea->Checked=GetN27AlarmSel(JamArea, "");
        cbN27AlarmSel->Checked=GetN27AlarmSel(JamArea, JamCode);
        cbN27AddBoard->Checked=GetN27AddBoard(JamArea, JamCode);                //Sam 20210911 :  南特中興系統要求顯示增加開啟
    }

    if(CosFunction.bOLPFunction)                                                //Sam 20231116 : 新增 Alarm 不需上傳伺服器
    {
        cbContAlarmNotUpload->Checked=GetContAlarmNotUpload(JamArea, JamCode);
    }

    if(CosFunction.bConAlarmNeedKeyInPassword)
        chkCheckContAlarm->Checked=GetJemContiAlarm(JamArea, JamCode);          //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量

    if(CosFunction.bConAlarmInTimeLevelUp)                                      //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
        chkO17->Checked=GetO17ContiAlarm(JamArea, JamCode);                     //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量

    if(CosFunction.bEnableHandlerResultServer)                                  //Sam 20230426 : 通知客戶系統 Handler 已經密碼鎖定
        chkTCPAlarm->Checked=GetJemTCPAlarm(JamArea, JamCode);
    chkAlarmAfterFullTray->Checked=GetAlarmAfterUnloaderFull(JamArea, JamCode);                                         //Jimmychiu 20240902 : Need Alarm After Unloader Full

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20210611 : Alarm Log 可以自訂該不需要記 Log 名稱
    {
        if(JamCode.Pos("WAR")!=0 || JamCode.Pos("JAM")!=0)                      //Sam 20210823 :  JAM 也要自訂量。
        {
            cbAddAlarmLog->Visible=true;
            cbAddAlarmLog->Checked=GetAddAlarmLog(JamArea, JamCode);
        }
        else
        {
            cbAddAlarmLog->Visible=false;
            cbAddAlarmLog->Checked=false;
        }
    }
}
//---------------------------------------------------------------------------
void TfSecurity::SaveJamLevel()                                                  // golden :1062-1202 //Steven 20140222 : Alarm Code需要有權限編輯
{
    AnsiString FileName;
    if(JamArea!="" && JamCode!="")
    {
        labMustCheck_35->Visible=false;
        if(IniConfig.bSPILFunction==true ||                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction  //Steven 20140430 : 矽品要求類自訂
           CUSTOMER_CODE==CC_SCS ||                                             //jou 2015-08-27 SCS 要求 Jam Level 要可以自訂級別
           CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20170413 (wei) add ASE_KH
        {
        }
        else if(CUSTOMER_CODE==CC_AMKOR_China ||
                CUSTOMER_CODE==CC_QUALCOMM)                                     //JerryYang 20170412 (Steven) add QUALCOMM
        {
            if(JamCode=="WAR07301" ||                                           //Socket consecutive failure
               JamCode=="WAR07321" ||                                           //Arm1: consecutive failure
               JamCode=="WAR07329" ||                                           //Arm2: consecutive failure
               JamCode=="JAM0508" || JamCode=="JAM0509")                        //Device lose at Output Shuttle
            {
                cbJamNeedRed->Checked=true;
                labMustCheck_35->Visible=true;
                if(rgJamLevel->ItemIndex<LevelSet.AccessLevel[35])
                {
                    rgJamLevel->ItemIndex=LevelSet.AccessLevel[35];
                }
            }
        }
        else if(CUSTOMER_CODE==CC_SCC)
        {
            if(JamCode=="WAR07301" ||                                           //Socket consecutive failure
               JamCode=="WAR07321" ||                                           //Arm1: consecutive failure
               JamCode=="WAR07329" ||                                           //Arm2: consecutive failure
               JamCode=="JAM0508"  || JamCode=="JAM0509" ||                     //kevin 20140612 可以設定 權限   //Device lose at Output Shuttle
               JamCode=="JAM0303"  || JamCode=="JAM0304" ||                     //Device drop error (Arm 1) & Device drop error (Arm 2)   //Jou 20140604 Add for SCC
               JamCode=="WAR0310"  ||                                           //Socket has IC error!                                    //Jou 20140604 Add for SCC
               JamCode=="JAM0314"  || JamCode=="JAM0315")                       //JerryYang 20160516 add JAM0314 JAM0315
            {
                cbJamNeedRed->Checked=true;
                labMustCheck_35->Visible=true;
                if(rgJamLevel->ItemIndex<LevelSet.AccessLevel[35])
                {
                    rgJamLevel->ItemIndex=LevelSet.AccessLevel[35];
                }
            }
        }
        else
        {
            if(JamCode=="JAM0203" || JamCode=="WAR0310" || JamCode=="WAR0346" ||
               JamCode=="WAR0343" || JamCode=="JAM0301" || JamCode=="JAM0302" ||
               JamCode=="WAR0701" || JamCode=="WAR0702" || JamCode=="WAR0703" || JamCode=="WAR0705" ||                  //Steven 20150709 : Add for Amkor China
               JamCode=="JAM0508" || JamCode=="JAM0509" || JamCode=="WAR07301" ||
               JamCode=="JAM0312" || JamCode=="JAM0313")                        //JerryYang 20160516 add JAM0312 JAM0313
            {
                labMustCheck_35->Visible=true;
                if(rgJamLevel->ItemIndex<LevelSet.AccessLevel[35])
                {
                    rgJamLevel->ItemIndex=LevelSet.AccessLevel[35];
                }
            }
        }
    }
    else
    {
        return;                                                                 //kevin 20190529 add  JamCode == NULL
    }
    WriteIniData(FileNameJam000, JamArea, JamCode+" Red", cbJamNeedRed->Checked);
    WriteIniData(FileNameJam000, JamArea, JamCode+" Silent", cbSilentMode->Checked);                                    //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
    WriteIniData(FileNameJam000, JamArea, JamCode, rgJamLevel->ItemIndex);
    WriteIniData(FileNameJam000, JamArea, JamCode+" Bit8", rgMachineStatusBit8->ItemIndex);                             //Isaac 20170922 (Steven) : ATP Machine Status Bit8 Issue
    if(CosFunction.bUseAlarmUnlockPassWord==true)                               //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" UnlockPassWord", cbUnlockPassWord->Checked);                    //Ifor 20170214 add 可以自訂Alarm是否要有解除密碼
    }

    if(CosFunction.bIncludeMTBA==true)                                          //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
    {
        if(CUSTOMER_CODE==CC_ASE_CL ||
           CUSTOMER_CODE==CC_TERAPOWER)
        {
            WriteIniData(FileNameJam000, JamArea, JamCode+" IncludeMTBF", cbIncludeMTBA->Checked);
        }
        else
        {
            WriteIniData(FileNameJam000, JamArea, JamCode+" IncludeMTBA", cbIncludeMTBA->Checked);
        }
    }

    if(CosFunction.bUseAlarmLogXml)
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" bN27AddBoard", cbN27AddBoard->Checked);                         //Sam 20210911 :  南特中興系統要求顯示增加開啟
        WriteIniData(FileNameJam000, JamArea, JamCode+" bN27AlarmSel", cbN27AlarmSel->Checked);
        WriteIniData(FileNameJam000, JamArea, "bN27AlarmSel", cbN27AlarmSelByArea->Checked);
    }

    if(CosFunction.bOLPFunction)                                                //Sam 20231116 : 新增 Alarm 不需上傳伺服器
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" ContAlarmNotUpload", cbContAlarmNotUpload->Checked);
    }

    if(CosFunction.bConAlarmNeedKeyInPassword && JamCode.Pos("JAM")>0)          //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" ContiAlarm", chkCheckContAlarm->Checked);
    }

    if(CosFunction.bConAlarmInTimeLevelUp==true)                                //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" O17ContiAlarm", chkO17->Checked);
    }

    if(CUSTOMER_CODE==CC_PTI)                                                   //Sam 20210611 : Alarm Log 可以自訂該不需要記 Log 名稱
    {
         WriteIniData(FileNameJam000, JamArea, JamCode+" GetAddAlarmLog", cbAddAlarmLog->Checked);
    }

    if(CosFunction.bEnableHandlerResultServer)                                  //Sam 20230426 : 通知客戶系統 Handler 已經密碼鎖定
    {
        WriteIniData(FileNameJam000, JamArea, JamCode+" TCPAlarm", chkTCPAlarm->Checked);
    }

    WriteIniData(FileNameJam000, JamArea, JamCode+" AlarmAfterUnloaderFull", chkAlarmAfterFullTray->Checked);           //Jimmychiu 20240902 : Need Alarm After Unloader Full

    if(JamLang==0)                                                              //English
    {
        FileName.sprintf("D:\\HT9045\\Error\\English\\%s.dat", JamCode);
    }
    else if(JamLang==1)                                                         //Chinese
    {
        FileName.sprintf("D:\\HT9045\\Error\\Chinese\\%s.dat", JamCode);
    }
    else if(JamLang==2)                                                         //Korean
    {
        FileName.sprintf("D:\\HT9045\\Error\\Korea\\%s.dat", JamCode);
    }
    else if(JamLang==3)                                                         //Singapore
    {
        FileName.sprintf("D:\\HT9045\\Error\\Singapore\\%s.dat", JamCode);
    }

    if(RichEditJamCode->Lines->Count!=0)
        RichEditJamCode->Lines->SaveToFile(FileName);
}
//---------------------------------------------------------------------------
int TfSecurity::GetJamLevel(AnsiString sJamArea, AnsiString sJamCode)            // golden :1204-1297 //Steven 20140222 : 取得Alarm Code需要有權限
{
    int iLevel=0;
    AnsiString asStr="";
    if(sJamArea!="" && sJamCode!="")
    {
        labMustCheck_35->Visible=false;
        asStr=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode, AnsiString("0"));
        iLevel=asStr.ToIntDef(0);                                               //jou 20240607 : 修正alarm權限設定異常
        if(CUSTOMER_CODE==CC_GIGAS && sJamCode=="JAM0201")                      //Richard 20220830 : 全智要求Jam0201 Operator可解除
        {
            iLevel=0;
        }

//        if(IniConfig.bSPILFunction==true ||                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction   //Steven 20140430 : 矽品要求類自訂
//           CUSTOMER_CODE==CC_SCS ||                                             //jou 2015-08-27 SCS 要求 Jam Level 要可以自訂級別
        if(SPIL_FOR_QLE==1)                                                     //KevinCheng 20260521 : 全部jamCode權限
        {
        }
        else if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20170413 (wei) add ASE_KH
        {
            if(sJamCode=="WAR0349" || sJamCode=="WAR16132")                     //JerryYang 20240111 : add  //JerryYang 20250423 : add WAR16132
            {
                if(iLevel<LevelSet.AccessLevel[35])
                {
                    iLevel=LevelSet.AccessLevel[35];
                }
            }
        }
        else if(CUSTOMER_CODE==CC_AMKOR_China ||                                //Steven 20150709 : Add for Amkor China
                CUSTOMER_CODE==CC_QUALCOMM)                                     //JerryYang 20170412 (Steven) add QUALCOMM
        {
            if(sJamCode=="WAR0701" || sJamCode=="JAM0301" ||
               sJamCode=="WAR0702" || sJamCode=="JAM0703" || JamCode=="WAR0705" ||
               sJamCode=="JAM0302" || sJamCode=="JAM0303" ||
               sJamCode=="JAM0304" || sJamCode=="JAM0305" ||
               sJamCode=="JAM0306" || sJamCode=="JAM0306" ||
               sJamCode=="WAR0310" || sJamCode=="WAR0343" ||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" ||
               sJamCode=="JAM0314" || sJamCode=="JAM0315")                      //JerryYang 20160516 add JAM0312~0315
            {
                labMustCheck_35->Visible=true;
                if(iLevel<LevelSet.AccessLevel[35])
                {
                    iLevel=LevelSet.AccessLevel[35];
                }
            }
        }
        else if(CUSTOMER_CODE==CC_SCC)
        {
            if(sJamCode=="WAR07301" ||                                          //Socket consecutive failure
               sJamCode=="WAR07321" ||                                          //Arm1: consecutive failure
               sJamCode=="WAR07329" ||                                          //Arm2: consecutive failure
               sJamCode=="JAM0508"  || sJamCode=="JAM0509" ||                   //kevin 20140612 可以設定 權限  //Device lose at Output Shuttle
               sJamCode=="JAM0303"  || sJamCode=="JAM0304" ||                   //Device drop error (Arm 1) & Device drop error (Arm 2)    //Jou 20140604 Add for SCC
               sJamCode=="WAR0310" )                                            //Socket has IC error!                                     //Jou 20140604 Add for SCC
            {
                labMustCheck_35->Visible=true;
                if(iLevel<LevelSet.AccessLevel[35])
                {
                    iLevel=LevelSet.AccessLevel[35];
                }
            }
        }
        else if(CUSTOMER_CODE==CC_ASE_CL)
        {
            if(sJamCode=="WAR16126" || sJamCode=="WAR1685" ||  //WAR16118->WAR16126 Steven 20260331
               sJamCode.AnsiPos("WAR24")>0)                                     //JerryYang 20240626 : Song要求限制FALALARM都要Engineer以上權限才能解除
            {
                if(iLevel<1)
                {
                    iLevel=1;
                }
            }
        }
        else
        {
            if(sJamCode=="JAM0203" || sJamCode=="WAR0310" || sJamCode=="WAR0346" ||
               sJamCode=="WAR0343" || sJamCode=="JAM0301" || sJamCode=="JAM0302" ||
               sJamCode=="WAR0701" || sJamCode=="WAR0702" || sJamCode=="WAR0703" || JamCode=="WAR0705" ||               //Steven 20150709 : Add for Amkor China
               sJamCode=="JAM0508" || sJamCode=="JAM0509" || sJamCode=="WAR07301"||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" || sJamCode=="WAR0471" || sJamCode=="WAR0472" )               //JerryYang 20160516 add JAM0312 JAM0313  //wei 20160823  Lot check 錯誤需輸入密碼
            {
                labMustCheck_35->Visible=true;
                if(iLevel<LevelSet.AccessLevel[35])
                {
                    iLevel=LevelSet.AccessLevel[35];
                }
            }
        }
    }

    return iLevel;
}
//---------------------------------------------------------------------------
int TfSecurity::GetBit8(AnsiString sJamArea, AnsiString sJamCode)                // golden :1299-1305 //Isaac 20170922 (Steven) : ATP Machine Status Bit8 Issue
{
    // golden declares a LOCAL `int iBit8=0;` here, same spelling as both the
    // file-scope global and the class member (B10) -- a THIRD shadow, legal
    // C++ (ordinary block-scope shadowing), kept verbatim.
    int iBit8=0;
    iBit8=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" Bit8", 0);

    return iBit8;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemSilent(AnsiString sJamArea, AnsiString sJamCode)          // golden :1307-1316 //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
{
    bool bNeedSilent=false;

    if(sJamArea!="" && sJamCode!="")
    {
        bNeedSilent=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" Silent", false);
    }
    return bNeedSilent;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemUnlockPassWord(AnsiString sJamArea, AnsiString sJamCode)  // golden :1318-1327 //Ifor 20170214 (wei) add 可以自訂Alarm是否要有解除密碼
{
    bool bUnlockPassWord=false;

    if(sJamArea!="" && sJamCode!="")
    {
        bUnlockPassWord=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" UnlockPassWord", false);
    }
    return bUnlockPassWord;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemIncludeMTBA(AnsiString sJamArea, AnsiString sJamCode)     // golden :1329-1358 //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
{
    bool bIncludeMTBA=false;

    if(sJamArea!="" && sJamCode!="")
    {
        if(CUSTOMER_CODE==CC_ASE_CL ||
           CUSTOMER_CODE==CC_TERAPOWER)
        {
            bIncludeMTBA=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" IncludeMTBF", true);
        }
        else
        {
            if(sJamCode.Pos("JAM")>0 &&
               (sJamArea=="01 Input Arm" ||
                sJamArea=="02 Output Arm" ||
                sJamArea=="03 Index Unit" ||
                sJamArea=="04 Input Shuttle" ||
                sJamArea=="05 Output Shuttle"))
            {
                bIncludeMTBA=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" IncludeMTBA", true);
            }
            else
            {
                bIncludeMTBA=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" IncludeMTBA", false);
            }
        }
    }
    return bIncludeMTBA;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemContiAlarm(AnsiString sJamArea, AnsiString sJamCode)      // golden :1360-1369 //Steven 20200513 : 新增alarm輸入密碼後alarm要可以自訂量
{
    bool bContiAlarm=false;

    if(sJamArea!="" && sJamCode.Pos("JAM")>0)
    {
        bContiAlarm=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" ContiAlarm", true);
    }
    return bContiAlarm;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetO17ContiAlarm(AnsiString sJamArea, AnsiString sJamCode)      // golden :1371-1380 //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
{
    bool bContiAlarm=false;

    if(sJamArea!="")
    {
        bContiAlarm=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" O17ContiAlarm", false);
    }
    return bContiAlarm;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetAddAlarmLog(AnsiString sJamArea, AnsiString sJamCode)        // golden :1382-1391 //Sam 20210611 : Alarm Log 可以自訂該不需要記 Log 名稱
{
    bool bContiAlarm=false;

    if(sJamArea!="")
    {
        bContiAlarm=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" GetAddAlarmLog", false);
    }
    return bContiAlarm;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemTCPAlarm(AnsiString sJamArea, AnsiString sJamCode)        // golden :1393-1401 //Sam 20230426 : 通知客戶系統 Handler 已經密碼鎖定
{
    bool bTCPAlarm=false;
    if(sJamArea!="")
    {
        bTCPAlarm=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" TCPAlarm", false);
    }
    return bTCPAlarm;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetAlarmAfterUnloaderFull(AnsiString sJamArea, AnsiString sJamCode)                                    // golden :1403-1411 //Jimmychiu 20240902 : Need Alarm After Unloader Full
{
    bool bAlarm=false;
    if(sJamArea=="02 Output Arm")
    {
        bAlarm=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" AlarmAfterUnloaderFull", false);
    }
    return bAlarm;
}
//---------------------------------------------------------------------------
AnsiString TfSecurity::GetJamArea(AnsiString sJamArea)                           // golden :1413-1416
{
    // GATE (SEC4): `fMain->AlarmUnitMap[sJamArea]` -- forms/fMain.h has no
    // `AlarmUnitMap` member (0 hits, grepped this wave). No known caller
    // exists yet anywhere in the port tree (grepped this wave) -- this
    // default has no observable consumer today.
#if 0
    return fMain->AlarmUnitMap[sJamArea];                                       //Steven 20231127 : 整理Alarm Unit
#else
    (void)sJamArea;
    return AnsiString("");
#endif
}
//---------------------------------------------------------------------------
bool TfSecurity::GetJemRed(AnsiString sJamArea, AnsiString sJamCode)             // golden :1418-1464 //Steven 20140222 : 取得Alarm畫面是否要紅底
{
    bool bNeedRed=false;

    if(sJamArea!="" && sJamCode!="")
    {
        cbJamNeedRed->Enabled=true;
        bNeedRed=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" Red", false);
        if(IniConfig.bSPILFunction==true ||                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction  //Steven 20140430 : 矽品要求類自訂
           CUSTOMER_CODE==CC_SCS)                                               //jou 2015-08-27 SCS 要求 Jam Level 要可以自訂級別
        {
            ;
        }
        else if(CUSTOMER_CODE==CC_AMKOR_China || CUSTOMER_CODE==CC_QUALCOMM)    //JerryYang 20170412 (Steven) add QUALCOMM
        {
            if(sJamCode=="WAR0701" || sJamCode=="JAM0301" ||
               sJamCode=="WAR0702" || sJamCode=="WAR0703" ||                    //Steven 20150709 : Add for Amkor China
               sJamCode=="JAM0302" || sJamCode=="JAM0303" ||
               sJamCode=="JAM0304" || sJamCode=="JAM0305" ||
               sJamCode=="JAM0306" || sJamCode=="JAM0306" ||
               sJamCode=="WAR0310" || sJamCode=="WAR0343" ||
               sJamCode=="JAM0312" || sJamCode=="JAM0313" ||
               sJamCode=="JAM0314" || sJamCode=="JAM0315")                      //JerryYang 20160516 add JAM0312~JAM0315
            {
                bNeedRed=true;
                cbJamNeedRed->Enabled=false;
            }
        }
        else if(CUSTOMER_CODE==CC_SCC)
        {
            if(sJamCode=="WAR07301" ||                                          //Socket consecutive failure
               sJamCode=="WAR07321" ||                                          //Arm1: consecutive failure
               sJamCode=="WAR07329" ||                                          //Arm2: consecutive failure
               sJamCode=="JAM0508" ||
               sJamCode=="JAM0509")                                             //Device lose at Output Shuttle
            {
                bNeedRed=true;
                cbJamNeedRed->Enabled=false;
            }
        }
        else
        {
            ;
        }
    }
    return bNeedRed;
}
//---------------------------------------------------------------------------
void TfSecurity::GetLevelSet()                                                   // golden :1466-1501 //Steven 20140222 : 統一LevelSet檔案改為Function
{
    extern const char* W906_LevelSetPath(); AnsiString FileName=W906_LevelSetPath();   //[W906] 20260927 測試縫（decisions R66）：golden V912 :1476 字面值 "d:\\HT9045\\system\\levelset.dat"；環境變數 W906_LEVELSET_PATH 沒設＝原字面值（本檔檔尾）
    // S16: AnsiString::c_str() returns `const char*` in this port
    // (vclcompat/AnsiString.h:92); cprod.h:3239-3240 declares WriteData/
    // ReadData taking non-const `char*` (a stricter signature than real
    // BCB6, whose looser C++ dialect accepted this without a cast). const_
    // cast bridges the two without touching cprod.h.
    if(FileExists(FileName)==false)
    {
        // AI(W906-FW-SecCC-integrate) 20260819: GATE (SEC-W1) -- shared
        // machine-config WRITE (system\levelset.dat) gated per the campaign
        // write boundary (the wave's own report exempted it as "small
        // per-machine file"; integration enforces the rule instead: system\
        // writes are gated, no size exception). Behavioural delta ONLY on a
        // machine where the file is missing: golden would seed it from the
        // in-memory defaults then read it straight back -- the gated port
        // skips both (the read below stays inside this same missing-file
        // else-shape), leaving LevelSet.AccessLevel[] at its current
        // in-memory values, which are exactly the bytes golden would have
        // round-tripped. On a machine where the file exists (every real
        // one), the two are identical.
        //AI(W906-FRW-S64) 20260926: GATE (SEC-W1) 解除 —— golden V912 cSecurity.cpp:1477-1480 原樣（檔案不在就用記憶體現值建檔）；上面 0819 的閘註解留作歷史
        WriteData(const_cast<char*>(FileName.c_str()), (char *)&LevelSet.AccessLevel[0], sizeof(LevelSet));
        //AI(W906-FRW-S64) 20260926: （原 #endif）
    }
    //AI(W906-FRW-S64) 20260926: 原 `else` 拿掉 —— golden :1481 ReadData 無條件（上一段保證檔案在）；下面 0819 的「read moved under else」註解是歷史
    // AI(W906-FW-SecCC-integrate) 20260819: read moved under `else` as part
    // of GATE (SEC-W1) -- golden reads unconditionally because it just
    // guaranteed the file exists; with the seeding write gated, reading a
    // missing file would be new (un-golden) behaviour, so the read now runs
    // exactly when golden's read had a file to read.
    ReadData(const_cast<char*>(FileName.c_str()), (char *)&LevelSet.AccessLevel[0], sizeof(LevelSet));

    for(int i=0; i<256; i++)
    {
        if((CUSTOMER_CODE==CC_KYEC_LEE) &&                                      //Ifor 20160825 add PE模式大鎖設定第2種可修改
           (i==35 || i==114 || i==128 || i==104))                               //Ifor 20160914 京元電子要求 Trouble Shooting大鎖設定第2
        {                                                                       //Ifor 20170203 (Steven) 京元電子要求ATC Control大鎖設定第2種可修改
            if(i==104)
            {
                LevelSet.AccessLevel[i]=3;                                      //Ifor 20200330 :add KYEC 京元要求大鎖設定最高權限
            }
            else
            {
                LevelSet.AccessLevel[i]=2;
            }
        }
        else if(i==163)
        {
            LevelSet.AccessLevel[i]=3;                                          //Ifor 20200114 add:Life Time Edit Permission 大鎖設定Hontech權限才可修改
        }
        else
        {
            if(CosFunction.bSecurityHave5Level==true)                           //jou 2014-06-19 Security Have 5 Level
                LevelSet.AccessLevel[i]=CheckRange(LevelSet.AccessLevel[i], 0, 4);                                      //Steven 20110907 : 檢查範圍
            else
                LevelSet.AccessLevel[i]=CheckRange(LevelSet.AccessLevel[i], 0, 3);                                      //Steven 20110907 : 檢查範圍
        }
    }
}
//---------------------------------------------------------------------------
void TfSecurity::SetLevelSet()                                                   // golden :1503-1507 //Steven 20140222 : 統一LevelSet檔案改為Function
{
    extern const char* W906_LevelSetPath(); AnsiString FileName=W906_LevelSetPath();   //[W906] 20260927 測試縫（decisions R66）：golden V912 :1513 字面值 "d:\\HT9045\\system\\levelset.dat"；同 GetLevelSet
    // AI(W906-FW-SecCC-integrate) 20260819: GATE (SEC-W2) -- shared
    // machine-config WRITE (system\levelset.dat), gated per the campaign
    // write boundary (same rule enforcement as GATE (SEC-W1) above). The
    // in-memory LevelSet.AccessLevel[] mutation that callers make BEFORE
    // calling this persists for the process; only the disk flush is gated.
    //AI(W906-FRW-S64) 20260926: GATE (SEC-W2) 解除 —— golden V912 :1514 原樣。觸發者：網頁權限表存檔 WebLevelSet.cpp（golden FormClose :465）；SECS S125F4 仍在 SECSGEM/uHGemHT9045.cpp GATE [L1]（交 Jimmy）
    WriteData(const_cast<char*>(FileName.c_str()), (char *)&LevelSet.AccessLevel[0], sizeof(LevelSet));                 // S16, see GetLevelSet above
    //AI(W906-FRW-S64) 20260926: （原 #endif）
    (void)FileName;
}
//---------------------------------------------------------------------------
void TfSecurity::spbImportClick(TObject *Sender)                                 // golden :1509-1577
{
    TStringList *MyStrList=new TStringList;
    AnsiString Str;
    AnsiString MyJamArea;
    AnsiString MyJamCode;
    AnsiString Level;
    AnsiString RedBg;
    AnsiString Silent="0";
    AnsiString Message;
    AnsiString UnlockPassWord="0";
    // golden shadows the private member `int MachineStatusBit8;` with a
    // LOCAL `AnsiString MachineStatusBit8;` here -- legal C++ (different
    // type, block scope), kept verbatim.
    AnsiString MachineStatusBit8;
    int iPos;

    // S13: OpenDialog1->Execute() hard-returns false (no modal dialog
    // surface) -- the body below is verbatim golden but structurally dead
    // until a real file picker exists.
    if(OpenDialog1->Execute())
    {
        MyStrList->LoadFromFile(OpenDialog1->FileName);

        for(int i=1; i<MyStrList->Count; i++)
        {
            Str=MyStrList->Strings[i];                                          //Steven 20180404 (Jou) : Fixed for CSV被Excel存檔之後, 有逗點的欄位會讀取異常的問題
            Str=StringReplace(Str, "\"", "", TReplaceFlags()<<rfReplaceAll);    //Isaac 20180427 (Steven) Jam cord csv file format changed & no work properly,把"去掉
            iPos=Str.AnsiPos(",");
            MyJamArea=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            iPos=Str.AnsiPos(",");
            MyJamCode=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            iPos=Str.AnsiPos(",");
            Level=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            iPos=Str.AnsiPos(",");
            RedBg=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            iPos=Str.AnsiPos(",");
            Silent=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            iPos=Str.AnsiPos(",");
            UnlockPassWord=Str.SubString(1, iPos-1);
            Str=Str.SubString(iPos+1, Str.Length());
            if(CUSTOMER_CODE!=CC_SCK)                                           //Isaac 20180413 (Steven) 少客戶碼，SCK不驗要
            {
                iPos=Str.AnsiPos(",");
                MachineStatusBit8=Str.SubString(1, iPos-1);
            }

            WriteIniData(FileNameJam000, MyJamArea, MyJamCode, Level);
            WriteIniData(FileNameJam000, MyJamArea, MyJamCode+" Silent", Silent);                                       //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
            WriteIniData(FileNameJam000, MyJamArea, MyJamCode+" Red", RedBg);
            if(CUSTOMER_CODE!=CC_SCK)                                           //Isaac 20180413 (Steven) 少客戶碼，SCK不驗要
            {
                WriteIniData(FileNameJam000, MyJamArea, MyJamCode+" Bit8", MachineStatusBit8);
            }

            if(CosFunction.bUseAlarmUnlockPassWord==true)                       //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
            {
                WriteIniData(FileNameJam000, MyJamArea, MyJamCode+" UnlockPassWord", UnlockPassWord);                   //Ifor 20170214 add 可以自訂Alarm是否要有解除密碼
            }
        }
    }
    ChangeJamMessage(false);

    spbImport->Down=false;

    MyStrList->Clear();
    delete MyStrList;                                                           //Steven 20160912 : Add delete for save memory
}
//---------------------------------------------------------------------------
void TfSecurity::spbExportClick(TObject *Sender)                                 // golden :1579-1657
{
    AnsiString MyJamArea;
    AnsiString MyJamCode;
    AnsiString Level;
    AnsiString RedBg;
    AnsiString Message;
    AnsiString CommaText;
    AnsiString OldJamCode="";

    TStringList *MyStrList=new TStringList;

    // S13: SaveDialog1->Execute() hard-returns false, see spbImportClick.
    if(SaveDialog1->Execute())
    {
        if(CUSTOMER_CODE==CC_SCK)                                               //Isaac 20180413 (Steven) 少客戶碼，SCK不驗要
        {
            CommaText.sprintf("JamArea, JamCode, JamLevel, JamNeedRed, SilentMode, UnlockPassWord, Message");
        }
        else
        {
            CommaText.sprintf("JamArea, JamCode, JamLevel, JamNeedRed, SilentMode, UnlockPassWord, MachineStatusBit8, Message");
        }
        MyStrList->Add(CommaText);

        for(int i=0; i<cbJamArea->Items->Count; i++)                            //至少要有輸出一次
        {
            cbJamArea->ItemIndex=i;
            cbJamArea->Refresh();
            GetJameCodeOfAxis(cbJamArea->ItemIndex+1, cbJamCode);

            for(int j=0; j<cbJamCode->Items->Count; j++)
            {
                cbJamCode->ItemIndex=j;
                cbJamCode->Refresh();
                cbJamLang->ItemIndex=0;
                cbJamLang->Refresh();
                JamArea=cbJamArea->Text;
                JamCode=cbJamCode->Text.SubString(1, cbJamCode->Text.AnsiPos("  :")-1);
                Message=cbJamCode->Text.SubString(cbJamCode->Text.AnsiPos(":")+1, cbJamCode->Text.Length());
                ChangeJamMessage(false);

                if(OldJamCode!=JamCode)
                {
                    OldJamCode=JamCode;
                    if(CUSTOMER_CODE==CC_SCK)                                   //Isaac 20180413 (Steven) 少客戶碼，SCK不驗要
                    {
                        CommaText.sprintf("\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\"",                           //Isaac 20180427 (Steven) Jam cord csv file format changed & no work properly,export出來時都加上引號
                                           JamArea,
                                           JamCode,
                                           rgJamLevel->ItemIndex,
                                           (cbJamNeedRed->Checked)?1:0,
                                           (cbSilentMode->Checked)?1:0,         //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
                                           (cbUnlockPassWord->Checked)?1:0,
                                           Message);                            //Steven 20170331 (wei) : 這個請保留在最後一位
                    }
                    else
                    {
                        CommaText.sprintf("\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\"",                    //Isaac 20180427 (Steven) Jam cord csv file format changed & no work properly,export出來時都加上引號
                                           JamArea,
                                           JamCode,
                                           rgJamLevel->ItemIndex,
                                           (cbJamNeedRed->Checked)?1:0,
                                           (cbSilentMode->Checked)?1:0,         //Steven 20150423 : SCK要求可以自訂Alarm是否要有靜音器
                                           (cbUnlockPassWord->Checked)?1:0,
                                           (rgMachineStatusBit8->ItemIndex)?1:0,
                                           Message);                            //Steven 20170331 (wei) : 這個請保留在最後一位
                    }
                    MyStrList->Add(CommaText);
                }
            }
        }

        MyStrList->SaveToFile(SaveDialog1->FileName);
    }

    spbExport->Down=false;
    MyStrList->Clear();
    delete MyStrList;                                                           //Steven 20160912 : Add delete for save memory
}
//---------------------------------------------------------------------------
void TfSecurity::btnOperatorClick(TObject *Sender)                               // golden :1659-1662
{
    ChangePassword(1);                                                          //jou 2014-06-19 Security Have 5 Level
}
//---------------------------------------------------------------------------
void TfSecurity::AddAlarmList()                                                  // golden :1664-1676
{
    if(CosFunction.bStatisticsJamCount==false)
    {
        tsStatisticsJam->TabVisible=false;
        return;
    }

    GetAlarmCodeList(sgStatisticsJam);                                          //Steven 20200331 : Alarm code list改用文字檔
    // GATE (SEC6): `sgStatisticsJam->ColWidths[7]=50;` -- vclcompat's
    // TStringGrid does not model ColWidths (see forms/fSecurity.h banner).
    // Purely cosmetic column width.
    sgStatisticsJam->Cells[7][0]="Count";
    ClearAllJamCount();
}
//---------------------------------------------------------------------------
void TfSecurity::ClearAllJamCount()                                              // golden :1678-1689
{
    if(CosFunction.bStatisticsJamCount==false)
    {
        return;
    }

    for(int i=1; i<sgStatisticsJam->RowCount; i++)
    {
        sgStatisticsJam->Cells[7][i]="0";
    }
}
//---------------------------------------------------------------------------
bool TfSecurity::AddJamCount(AnsiString asJamCode)                               // golden :1691-1717
{
    if(CosFunction.bStatisticsJamCount==false)
    {
        return false;
    }

    int iCount=0;
    for(int i=1; i<sgStatisticsJam->RowCount; i++)
    {
        if(sgStatisticsJam->Cells[2][i]==asJamCode)
        {
            iCount=atoi(sgStatisticsJam->Cells[7][i].c_str())+1;
            if(iCount>=5)
            {
                sgStatisticsJam->Cells[7][i]="0";
                return true;
            }
            else
            {
                sgStatisticsJam->Cells[7][i]=iCount;
                return false;
            }
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetN27AlarmSel(AnsiString sJamArea, AnsiString sJamCode)        // golden :1719-1731
{
    bool bN27AlarmSel=false;

    if(sJamArea!="")
    {
        if(sJamCode=="")
            bN27AlarmSel=CheckAndReadIniData(FileNameJam000, sJamArea, "bN27AlarmSel", true);
        else
            bN27AlarmSel=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" bN27AlarmSel", true);
    }
    return bN27AlarmSel;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetN27AddBoard(AnsiString sJamArea, AnsiString sJamCode)        // golden :1733-1742 //Sam 20210911 :  南特中興系統要求顯示增加開啟
{
    bool bN27AddBoard=false;

    if(sJamArea!="" && sJamCode!="")
    {
        bN27AddBoard=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" bN27AddBoard", false);
    }
    return bN27AddBoard;
}
//---------------------------------------------------------------------------
bool TfSecurity::GetContAlarmNotUpload(AnsiString sJamArea, AnsiString sJamCode)                                        // golden :1744-1752 //Sam 20231116 : 新增 Alarm 不需上傳伺服器
{
    bool bContAlarmNotUpload=false;
    if(sJamArea!="" && sJamCode!="")
    {
        bContAlarmNotUpload=CheckAndReadIniData(FileNameJam000, sJamArea, sJamCode+" ContAlarmNotUpload", false);
    }
    return bContAlarmNotUpload;
}
//---------------------------------------------------------------------------
AnsiString TfSecurity::GetPasswoard()                                            // golden :1754-1771 //Sam 20220106 : 取出密碼
{
    AnsiString sPathName="";
    AnsiString asPassword="16943420";
    // GATE (SEC9): the whole `if(IniConfig.bSIGURDFunction)` block writes
    // under D:\HT9045\system\ (SG_PW.ini) -- on the task brief's explicit
    // write-gate list. DEFAULT: return golden's own pre-set literal below.
#if 0
    if(IniConfig.bSIGURDFunction)
    {
        sPathName.sprintf("D:\\HT9045\\system");
        MyForceDirectories(sPathName);
        sPathName.sprintf("D:\\HT9045\\system\\SG_PW.ini");
        if(FileExists(sPathName)==false)                                        //檢查檔案
        {
            WriteIniData(sPathName, "SG",    "PW",       (AnsiString)16943420);
        }
        asPassword=CheckAndReadIniData(sPathName, "SG",    "PW",    (AnsiString)16943420);
    }
#endif

    return asPassword;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Steven 20260924 (S12)：GATE (SEC1) 拆出「權限查表」那一半。
//
//  golden TfSecurity 建構子（V912 cSecurity.cpp:26-215）：180 筆 mySecurityPal.push_back（全部無條件）→
//  :210 iMaxLevelItem=mySecurityPal.size() → :215 GetLevelSet()（讀 system\levelset.dat 進 LevelSet）。
//  Insufficient(iType)（:574）只看 iMaxLevelItem 與 LevelSet.AccessLevel[iType]，不碰 mySecurityPal 本身。
//  SEC1 關著時 iMaxLevelItem＝0 → Insufficient(任何 iType>0) 一律 false：C 路／golden 表單橋照 golden 轉出來的
//  `grp->Enabled=fSecurity->Insufficient(115,false)` 全部變成停用，任何等級都改不到（實測 Setup.Ld_ULd.html）。
//  這裡只補 golden 的那個數字與 GetLevelSet；mySecurityPal（按鈕圖示、面板）仍在 SEC1／SEC10 裡。
//  ⚠ 數字＝V912 golden 的筆數。golden 加一筆權限項目時要跟著改（本檔 #if 0 裡那份是 V906 的 179 筆）。
//  呼叫端：tools/wb_serve.cpp 開機（golden WinMain CreateForm(TfSecurity) 的時機，早於任何表單開頁）。
//---------------------------------------------------------------------------
#include <cstdio>   // printf（放在這裡而不是檔頭：檔頭加一行會位移本檔全部行號，文件以行號引用 cSecurity.cpp）
void W906_SecurityBoot()
{
    iMaxLevelItem = 180;                                                        // golden V912 cSecurity.cpp:210（180 筆 push_back）
    fSecurity->GetLevelSet();                                                   // golden V912 cSecurity.cpp:215
    { extern void W906_SecurityJamBoot(); W906_SecurityJamBoot(); }             //Steven 團隊 20260926 (Status.Security Jam)：golden 建構子 :216-239 的 Jam 那一半（見檔尾）
    std::printf("security: iMaxLevelItem=%d (golden V912 count), levelset.dat loaded -- GATE (SEC1) lookup half lifted\n",
                iMaxLevelItem);
}
//---------------------------------------------------------------------------
//  Steven 20260924 (S12 LevelSet)：網頁 Status.Security.html 存檔（WS system.levels.put 寫完 levelset.dat 之後）  //AI(W906-FRW-S64) 20260926: 已由 WebLevelSet.cpp W906_LevelSetPut 取代（wb_serve 換臂後本函式無人呼叫；換臂前舊臂仍呼叫，SEC-W2 解除後這裡的 SetLevelSet 會把正規化後的記憶體寫回，與 golden 相同）
//  ＝ golden TfSecurity::FormClose（V912 cSecurity.cpp:439-468）的 LevelSet 那一段：
//    :441-446 LevelSet.AccessLevel[i]=mySecurityPal[i]->GetLevel() —— 網頁送的值已寫進檔案，這裡 GetLevelSet() 讀回
//              （含 golden GetLevelSet 的範圍檢查與強制值：[163]=3、KYEC_LEE 的 35/114/128/104）
//    :448-458 三條鉗制（[87]≥Supervisor、[129]≤[130]、非矽格北興時 [86]≥Engineer）
//    :465     SetLevelSet() 寫回
//  ⚠ 沒做：:462 SaveJamLevel()（Alarm／Jam 權限，這一頁沒有）、:463 SavePassword()／:466 ReadPassword()（密碼，
//     這一頁沒有；login.dat 不經網頁）。回傳被鉗制改掉的索引，給頁面顯示。
//---------------------------------------------------------------------------
// golden FormClose :448-458 的三條鉗制，對任一份 256 格的等級表（回傳被改掉的索引，逗號分隔）。
// 審查第 8 輪 M-1：WS system.levels.put 在寫檔**之前**先對「檔案現值＋網頁送的值」套這三條，鉗制後的值經原本
// 已授權的寫檔路徑（--allow-system-write）一起寫下去 —— 檔案與記憶體一致，也不必解 GATE (SEC-W2)。
std::string W906_SecurityClampLevels(int* lv)
{
    std::string clamped;
    if(lv[87]<iDefSupervisorLevel)                                              //Steven 20120830 : Teaching按鈕權限
    { lv[87]=iDefSupervisorLevel; clamped += "87,"; }

    if(lv[129]>lv[130])                                                         //Steven 20161201 : For SCK 93K ART
    { lv[129]=lv[130]; clamped += "129,"; }

    if(CUSTOMER_CODE!=CC_SIGURD_PeiXing)                                        //Alick 20160901 modify for 矽格北興
    {
        if(lv[86]<iDefEngineerLevel)                                            //Steven 20120830 : Motion View按鈕權限
        { lv[86]=iDefEngineerLevel; clamped += "86,"; }
    }
    if (!clamped.empty()) clamped.erase(clamped.size() - 1);
    return clamped;
}

std::string W906_SecurityFormCloseLevels()
{
    fSecurity->GetLevelSet();                                                   // 網頁寫好的檔（已鉗制）讀回記憶體（含 golden 的範圍檢查與強制值）
    const std::string clamped = W906_SecurityClampLevels(&LevelSet.AccessLevel[0]);   // 正常為空（寫檔前已鉗過）
    // golden :465 SetLevelSet()：//AI(W906-FRW-S64) 20260926: GATE (SEC-W2) 已解除，這裡會真的寫檔（整塊記憶體，＝golden）
    fSecurity->SetLevelSet();                                                   //Steven 20140222 : 存取LevelSet檔案改為Function
    return clamped;
}

//---------------------------------------------------------------------------
//  Steven 團隊 20260926 (Status.Security Jam 分頁)：golden TfSecurity 建構子的 Jam 那一半（V912 cSecurity.cpp:217-239）。
//
//  為什麼要另外跑：本檔 :62 的建構子在 static init 時就跑（`TfSecurity *fSecurity = new TfSecurity();`），
//  :269 的 `if (INIFileGeneral != 0)` 守衛在那個時間點一定是假 —— 所以 golden :217 `FileNameJam000=...` 從來沒執行，
//  FileNameJam000 是空字串，**所有** Jam getter（GetJamLevel／GetJemSilent／GetJemRed／GetBit8…）都走
//  OpenIniFile("") 失敗分支回預設值（common.cpp OpenIniFile :325），從不讀 Error\English\JAM0000.dat。
//  這裡在 golden CreateForm(TfSecurity) 的時機（W906_SecurityBoot，wb_serve 開機）補跑。
//
//  (1) DFM 設計期狀態（替身沒有 .dfm 載入器）：cSecurity.dfm:692-872 / :1017-1025 的 Items／ItemIndex／Text／Visible。
//  (2) golden :217-238 原樣：設 FileNameJam000；檔案不存在時逐區逐碼 ChangeJamMessage(false)+ChangeJamMessage() 建檔。
//  (3) golden :239 AddAlarmList()：bStatisticsJamCount=false → tsStatisticsJam 隱藏；否則 sgStatisticsJam 由 AlarmCodeList.txt 填表、Count 歸 0。
//  golden 換配方（main.cpp DoReadLastData :9264-9441）不碰 Jam 設定 —— getter 每次都直接讀檔，沒有記憶體快取，所以不接 W906_DoReadLastData。
//---------------------------------------------------------------------------
void W906_SecurityJamBoot()
{
    TfSecurity* f = fSecurity;
    // ---- (1) DFM ----
    static const char* const kArea[31] = {                                      // cSecurity.dfm:728-759
        "01 Input Arm", "02 Output Arm", "03 Index Unit", "04 Input Shuttle", "05 Output Shuttle", "06 Empty Tray Arm",
        "07 Tester I/F", "08 Scanner", "09 Tray Loader", "10 Empty Tray", "11 Tray Unloader 1", "12 Tray Unloader 2",
        "13 Tray Unloader 3", "14 Color Tray", "15 Temp. Controller", "16 System", "17 Fix Tray 1", "18 Fix Tray 2",
        "19 Fix Tray 3", "20 ESD System", "21 Process Log", "22 Motion Log", "23 Message", "24 Motor",
        "25 Tray Unloader 4", "26 Tray Unloader 5", "27 Tray Unloader 6", "28 Fix Tray 4", "29 Fix Tray 5", "30 Fix Tray 6",
        "31 Cylinder" };
    if (f->cbJamArea->Items->Count == 0)
        for (int i = 0; i < 31; ++i) f->cbJamArea->Items->Add(kArea[i]);
    f->cbJamArea->ItemIndex = 0;  f->cbJamArea->Text = "01 Input Arm";            // dfm:723/:726
    static const char* const kLang[4] = { "English", "Chinese", "Korean", "Singapore" };   // dfm:791-795
    if (f->cbJamLang->Items->Count == 0)
        for (int i = 0; i < 4; ++i) f->cbJamLang->Items->Add(kLang[i]);
    f->cbJamLang->ItemIndex = -1; f->cbJamLang->Text = "English";                 // dfm:789（沒有 ItemIndex＝-1）
    static const char* const kLv[4] = { "Operator", "Engineer", "Supervisor", "HonPrec" };  // dfm:704-708
    if (f->rgJamLevel->Items->Count == 0)
        for (int i = 0; i < 4; ++i) f->rgJamLevel->Items->Add(kLv[i]);
    f->rgJamLevel->ItemIndex = 0;                                                // dfm:703
    if (f->rgMachineStatusBit8->Items->Count == 0) { f->rgMachineStatusBit8->Items->Add("0"); f->rgMachineStatusBit8->Items->Add("1"); }   // dfm:867-869
    f->rgMachineStatusBit8->ItemIndex = 0;                                       // dfm:866
    f->tsJamCode->TabVisible = true; f->tsStatisticsJam->TabVisible = true;      // VCL TTabSheet.TabVisible 預設 true（dfm:347/:1014 沒寫）；vclcompat 預設 false
    // vclcompat TControl 預設 Visible=false、Enabled=false（Controls.h:256）；VCL/dfm 預設兩者都是 true —— Jam 分頁元件補回 dfm 預設，
    // 之後的可見性由 golden FormShow :289-301/:417/:429-432 決定（沒補的話 rgJamLevel／cbSilentMode 永遠看不到、網頁送的值被忽略）
    {
        TControl* const kDfmDefault[] = { f->rgJamLevel, f->cbJamArea, f->cbJamCode, f->cbJamLang, f->RichEditJamCode, f->rgMachineStatusBit8,
            f->cbJamNeedRed, f->cbSilentMode, f->cbUnlockPassWord, f->cbIncludeMTBA, f->chkCheckContAlarm, f->chkO17, f->cbAddAlarmLog,
            f->cbN27AddBoard, f->cbN27AlarmSel, f->cbN27AlarmSelByArea, f->chkTCPAlarm, f->cbContAlarmNotUpload, f->chkAlarmAfterFullTray,
            f->spbImport, f->spbExport, f->labJamArea, f->labJamCode, f->Label1, f->labLang };
        for (TControl* c : kDfmDefault) { c->Visible = true; c->Enabled = true; }
    }
    f->labMustCheck_35->Visible = false;                                         // dfm:690
    f->sgStatisticsJam->RowCount = 60;                                           // dfm:1024-1025（ColCount=8 已在 fSecurity.h）

    // ---- (2) golden :217-238 ----
    f->FileNameJam000="D:\\HT9045\\Error\\English\\JAM0000.dat";
    if(FileExists(f->FileNameJam000)==false)                                    //Steven 20140222 End: Alarm Code設定權限
    {
        for(int i=0; i<f->cbJamArea->Items->Count; i++)
        {
            f->cbJamArea->ItemIndex=i;
            f->cbJamArea->Refresh();
            GetJameCodeOfAxis(f->cbJamArea->ItemIndex+1, f->cbJamCode);

            for(int j=0; j<f->cbJamCode->Items->Count; j++)
            {
                f->cbJamCode->ItemIndex=j;
                f->cbJamCode->Refresh();
                f->cbJamLang->ItemIndex=0;
                f->cbJamLang->Refresh();
                f->JamArea=f->cbJamArea->Text;
                f->JamCode=f->cbJamCode->Text.SubString(1, f->cbJamCode->Text.AnsiPos("  :")-1);
                f->ChangeJamMessage(false);
                f->ChangeJamMessage();
            }
        }
    }
    // ---- (3) golden :239 ----
    f->AddAlarmList();                                                          //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
    std::printf("security: Jam boot (golden cSecurity.cpp:217-239) FileNameJam000=%s exists=%d statisticsTab=%d\n",
                f->FileNameJam000.c_str(), FileExists(f->FileNameJam000) ? 1 : 0, f->tsStatisticsJam->TabVisible ? 1 : 0);
}
//---------------------------------------------------------------------------
//  //AI(W906-FRW-S64F) 20260927 (Steven 團隊)：FormClose（:510）的 SavePassword／ReadPassword 開關。
//  預設 false＝golden 原樣（FormClose :463-467 照跑）。網頁權限表存檔（WebLevelSet.cpp W906_LevelSetPut，Q30=B 經
//  SecurityExitClick→FormClose）在呼叫期間設成 true：Steven 20260927 Q24=B「照 golden 呼叫，但不急、排後」——
//  SavePassword 會用記憶體的 USER 整個蓋掉 d:\HT9045\system\login.dat（字面值，WebLogin.cpp 的 W906_LOGINDAT_PATH 測試縫轉不到）。
//  排到 Q24 時：WebLevelSet.cpp 拿掉 SkipPasswordGuard 那一行即可，這裡不用改。
//---------------------------------------------------------------------------
bool W906_FormCloseSkipPassword = false;
//---------------------------------------------------------------------------
//AI(W906-FRW-S64) 20260927 [W906] 不在 golden：levelset.dat 路徑的測試縫（Steven 團隊，S64 追加題 5／decisions R66）。
//  golden V912 cSecurity.cpp:1476（GetLevelSet）、:1513（SetLevelSet）路徑是字面值；WebLevelSet.cpp 的 kLevelSetPath（備份、驗證、讀檔）也改用這支，
//  三處一定落在同一個檔。環境變數 W906_LEVELSET_PATH 沒設或空字串＝golden 字面值（出貨行為不變）；ctest／e2e 設成暫存檔，才不會寫到機台真檔。
//  放檔尾（不放檔頭）：檔頭加一行會位移本檔全部行號，文件以行號引用 cSecurity.cpp。
const char* W906_LevelSetPath()
{
    const char* e=std::getenv("W906_LEVELSET_PATH");
    return (e!=0 && e[0]!=0) ? e : "d:\\HT9045\\system\\levelset.dat";
}
