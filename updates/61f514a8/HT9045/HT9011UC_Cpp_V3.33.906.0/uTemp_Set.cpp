// =============================================================================
//  uTemp_Set.cpp  --  TfTemp_Set definitions (see forms/fTemp_Set.h for the
//  full ROLE / WAVE SCOPE / GATE REGISTER / SAFETY GATE REGISTER / DEPENDENCY
//  AUDIT -- read that banner FIRST, it is not repeated in full here).
//
//  Faithful translation of golden uTemp_Set.cpp (6,970 lines, BCB6, cp950).
//  Translator: AI(W906-FW3-TempSet-WA) 20260820.
//  Translation wave: FW-3 batch 3 continuation, uTemp_Set Wave A.
//  Golden read start-to-finish this wave (6,970/6,970 lines, 100%) before any
//  line below was written.
//
//  WAVE SCOPE TABLE (golden span, this file's status) -- see facade header
//  for the full GATE REGISTER/SAFETY GATE REGISTER this table cross-references.
//  --------------------------------------------------------------------------
//   ctor                                :95-408    translated -> Init() (ctor empty)
//   SetBasePointIMG                     :410-419   GATE(G-Image), reduces to no-op
//   rb1PointClick                       :421-426   translated
//   FormCreate                          :428-431   translated
//   FormShow                            :433-1321  translated (huge)
//   SetTempPanelCaption                 :1323-1974 translated (pure Caption)
//   ReadTempFile                        :1976-3143 translated, GATE(G-RGColor x1, dep-fMain->palIndivisual x1)
//   DoIniDataToForm                     :3145-3669 translated, GATE(G-FTestIF)
//   UpDateEdit                          :3671-4101 translated, GATE(G-RGColor x2, G-Image, G-Align)
//   edSoakTimeKeyPress                  :4103-4107 translated
//   edSoakTimeClick                     :4109-4121 translated, GATE(G-Barcode); G-Qwerty OPENED
//   edJamSoakTimeClick                  :4123-4135 translated, GATE(G-Barcode); G-Qwerty OPENED
//   edLHP1MouseDown                     :4137-4151 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   edLHeatGun1MouseDown                :4153-4157 DEVIATION sig, G-Qwerty OPENED
//   FormClose                           :4159-4169 DEVIATION sig, translated
//   edLHP1KeyUp                         :4171-4175 DEVIATION sig, G-Qwerty OPENED
//   edLowBaseMouseDown                  :4177-4193 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   rgIndexHeatModeClick                :4195-4199 translated
//   spbSaveClick                        :4201-4528 translated, SAFETY GATE (S2)
//   SaveSetupFile                       :4532-5099 SAFETY GATE (S1), whole body
//   sbtExitClick                        :5101-5124 translated, GATE(ATC ->Click x2)
//   btClearAllClick                     :5126-5135 translated
//   DisableOffset                       :5137-5148 translated
//   pgcTempOffsetChange                 :5150-5188 translated
//   rgTemperatureModeClick              :5190-5223 translated, GATE(G-Barcode)
//   edTargetHP1MouseDown                :5225-5231 DEVIATION sig, G-Qwerty OPENED
//   DisplayTargetTempEdit               :5233-5311 translated
//   cbEnableIndividualModeClick         :5313-5316 translated
//   edAmbHotGuartbentClick              :5318-5331 translated, GATE(G-Barcode); G-Qwerty OPENED
//   rbATC70ActiveOnClick                :5333-5354 translated
//   rbATCActiveOnClick                  :5356-5377 translated
//   edtATCInPC1MouseDown                :5379-5396 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   edAtcFileNameMouseDown              :5398-5402 DEVIATION sig, G-Qwerty OPENED
//   edATCAmbTempMouseDown               :5404-5428 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   edArm1OffsetMouseDown               :5430-5437 DEVIATION sig, translated+G-Qwerty OPENED
//   edATCTestTimeOffsetMouseDown        :5439-5443 DEVIATION sig, G-Qwerty OPENED
//   edATCTempAlwaysSameAlarmMouseDown   :5445-5449 DEVIATION sig, G-Qwerty OPENED
//   edATCChillerTempMouseDown           :5451-5472 DEVIATION sig, G-Qwerty OPENED (x4)
//   edATCInitialOffset1MouseDown        :5474-5488 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   edArm1NoFullsiteOffset_1MouseDown   :5490-5495 DEVIATION sig, G-Qwerty OPENED
//   edTSDTimeOutMouseDown               :5497-5501 DEVIATION sig, G-Qwerty OPENED
//   SendATCSelfTest                     :5503-5548 SAFETY GATE (S5), whole body
//   sbSafeTestATCClick                  :5550-5553 translated (calls S5)
//   CheckTempSettingChange              :5555-5615 translated (pure compare)
//   MaxTempSetting                      :5619-5685 translated (pure calc)
//   MinTempSetting                      :5687-5721 translated (pure calc)
//   edAmbTempClick                      :5728-5734 translated, G-Qwerty OPENED
//   edtIdleTime_LongClick                :5736-5747 translated, G-Qwerty OPENED
//   edtBoostOffset_LongClick            :5749-5752 G-Qwerty OPENED
//   edtBoostOffsetClick                 :5754-5757 G-Qwerty OPENED
//   edtInitialDelay_1Click              :5759-5762 G-Qwerty OPENED
//   btnSortClick                        :5764-5781 translated
//   edLimitHP1MouseDown                 :5783-5787 DEVIATION sig, G-Qwerty OPENED
//   edtLBTimeOutClick                   :5789-5792 G-Qwerty OPENED
//   FormDestroy                         :5794-5821 translated
//   ShowLineOnTop                       :5823-5854 GATE(G-Align), whole body
//   SaveRemoteTempOffset                :5856-5973 translated; SAFETY GATE (S3) OPENED 20260927 (Steven W9=A)
//   SaveRemoteTempOffsetFromGPIB        :5975-6059 translated + 912 :6142-6163 reload (W9=A)
//   ReadRemoteTempOffset                :6061-6115 translated (reads the recipe; the SetATCOffset push stays gated)
//   edTJTempRange_HighClick             :6117-6120 G-Qwerty OPENED
//   edAmbGuardbandClick                 :6122-6125 G-Qwerty OPENED
//   edTJTempRange_LowClick              :6127-6130 G-Qwerty OPENED
//   ControlATC60AirFlow                 :6132-6160 SAFETY GATE (S13, partial)
//   edtThresholdClick                   :6162-6165 G-Qwerty OPENED
//   edDewPointRangeMouseDown            :6167-6170 DEVIATION sig, G-Qwerty OPENED
//   edDewPointAlarmIntervalMouseDown    :6172-6175 DEVIATION sig, G-Qwerty OPENED
//   btnSameAsArm1Click                  :6177-6186 translated
//   edtChamberBoostTimeClick            :6188-6191 G-Qwerty OPENED
//   edtChamberBoostOffsetClick          :6193-6196 G-Qwerty OPENED
//   edTempOffsetCountClick              :6198-6201 G-Qwerty OPENED
//   edTempDownContactDelayClick         :6203-6206 G-Qwerty OPENED
//   edtHeatGunTempATCMouseDown          :6208-6222 DEVIATION sig, GATE(G-Barcode); G-Qwerty OPENED
//   chkTempCalByRecipeClick             :6225-6233 translated
//   FormShortCut                        :6235-6242 OMITTED (TWMKey no port, see header)
//   edLBTempAlarmTimeMouseDown          :6244-6248 DEVIATION sig, G-Qwerty OPENED
//   edtLBAirOnTempMouseDown             :6250-6254 DEVIATION sig, G-Qwerty OPENED
//   edtATCPIDOffset_MinPClick           :6256-6259 G-Qwerty OPENED
//   edFFC_Arm1TimeOn_01Click            :6261-6264 G-Qwerty OPENED
//   edtATC_HotGunTimeClick              :6266-6269 G-Qwerty OPENED
//   edtLBTempMinClick                   :6271-6274 G-Qwerty OPENED
//   rgBasePointClick                    :6276-6279 translated
//   edtSetTJ_OffsetClick                :6281-6284 G-Qwerty OPENED
//   edtSetTJ_SlopeChange                :6286-6289 G-Qwerty OPENED
//   sgTjMapMouseDown                    :6291-6309 GATE(G-Grid), whole body
//   rgTjMapTypeClick                    :6311-6321 translated
//   edTempReadyRangeMouseDown           :6323-6327 DEVIATION sig, G-Qwerty OPENED
//   InitialAddrToATC                    :6329-6662 translated (pure data calc)
//   edATCOfsTimeMouseDown               :6664-6668 DEVIATION sig, G-Qwerty OPENED
//   SwitchOnOffRefrigeratorNo           :6672-6683 SAFETY GATE (S7, partial)
//   UseCloseSiteTemperatureCtrl         :6685-6688 translated (golden "No Use")
//   SetSingleWorkTemperature            :6690-6698 SAFETY GATE (S9, partial)
//   SetCloseSiteTemperature             :6700-6706 translated (golden "No Use")
//   cbSelectDeviceChange                :6709-6727 translated
//   btnDefrostStartClick                :6729-6768 SAFETY GATE (S11, partial)
//   btnDefrostEndClick                  :6770-6776 SAFETY GATE (S12, whole tail)
//   CheckVerSupportFunction             :6778-6814 translated (no gate, per brief)
//   tmr_ATC_DeforstTimer                :6816-6838 translated (delegates, see S17)
//   ATC_Online_Or_offline               :6840-6848 translated, GATE(G-Delegate)
//   Tri_Temp_Low_Temperature_Use_Heater :6850-6871 translated (Click() inert, see S18)
//   Tri_Temp_High_Temperature_UseNo_Heater :6873-6889 translated (Click() inert, see S19)
//   Handler_Send_To_ATC_DewPoint        :6891-6894 SAFETY GATE (S14), whole body
//   edt_SetAirstreamTemperatureRang_SocketClick :6897-6901 G-Qwerty OPENED
//   edtSetTempature2AirMachineClick     :6903-6929 translated, G-Qwerty OPENED
//   edt_AirVolumeLmt_IndexClick         :6931-6934 G-Qwerty OPENED
//   btn_DefrostAllUseStartClick         :6936-6943 SAFETY GATE (S15), whole body
//   edt_Defrost_Time_Too_LowerClick     :6945-6948 G-Qwerty OPENED
//   btn_DefrostAllUseEndClick           :6951-6956 SAFETY GATE (S16), whole body
//   cbATCReferTempSensorClick           :6958-6969 translated
//   ATC_Power / CheckAirMachineStatus   -- declared, no golden body exists (see header)
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"     // de-VCL'd include hub: vclcompat umbrella + portable STL
#pragma hdrstop

#include "forms/fTemp_Set.h"
// AI(W906-FW-SIG-W18) 20260826: 22 支 handler 回填 golden 完整簽章。
#include "vclcompat/ShiftState.h"

#include "MachineType.h"       // tc* channel enum, e*/CC_* constants
#include "cmydef.h"            // Temperature.*-adjacent globals, UN150Read[]/bUT150*[]/asGPIBTempShow[]/
                               //   iTempOverShowAlarmT[]/fQwertyKey/N_INTEGER/N_DOUBLE/CUSTOMER_CODE/TColor/
                               //   SystemStart/iHome/dTempMax/dTempMin/K_RETRY/MMSystem/ShowErrorMessage/...
#include "cprod.h"             // Temperature / TestIF / TestIF_File / DeviceForm_File / InputLimit / tATCData
#include "Config.h"            // IniConfig
#include "CosFunction.h"       // CosFunction
#include "LastSet.h"           // LastSet.iTemperature / SaveLastSetIni
#include "cAuthority.h"        // AccessLevel / LevelSet / bAuthCriticalPara / GetLimitAuth / iDefSupervisorLevel/iDefEngineerLevel
#include "forms/fSecurity.h"   // fSecurity
#include "aHotPlateSubstrate.h" // global TList (listNormal/listArm1/listArm2 backing) + canonical TMyKitSuck InArmSuck + MOT[]
#include "ATC/ATCInterface.h"  // TATCInterfaceForm *ATCInterfaceForm ("old" ATC interface)
#include "acarry_shims.h"      // TATC_InterfaceForm *ATC_InterfaceForm ("new" ATC interface) -- golden
                               //   ATC/ATC_Handler_Side.h is NOT ported as a standalone header in this
                               //   tree; acarry_shims.h already carries this exact global (same include
                               //   cTemperFrom.cpp itself uses for ATC_InterfaceForm).
#include "TempCtrl/TriTemp.h"  // Tri_Temp_Machine / fTriMachineSelectDeviceDefrost_Start|End / Total_Compressor /
                               //   RefrigeratorUserModeState / TriTemp_Ch / bALLTempFirstReady /
                               //   bcheckSuperHotTemperatureKitTiming
#include "forms/fMain.h"       // fMain
#include "forms/fLotInfo.h"    // fLotInfo
#include "atester_shims.h"     // fContact (TfContactShim)
#include "database.h"          // ATKRecipeInfo
#include "common.h"            // OnlyNumberAndDotInPut / MyForceDirectories / FileExists / CopyFile / DeleteFile
#include "canary_support.h"    // RecordProcess / ShowErrorMessage / ShowMyMessage (sim bodies, golden note.h/cMyDB.h/mymessbox.h homes)
#include "csystem.h"           // HasICUnderMachine / IndexHasIC / ShuttleHasIC
#include "cinitial.h"          // InitialHeaterDoor
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT
#include "SECSGEM/SecsEventReport.h" // EventReport(unsigned)
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
#include "BarcodeReader.h"  // AI(W906-FW-BARCODE2) 20260825: Barcode_Reader real since FW-BARCODE1 (e7b4bf8) -- rgTemperatureModeClick guard live

// AI(W906-FW3-TempSet-WA) 20260820: LOCALLY PROVIDED, matching this tree's
// established per-TU convention for these exact golden constants (see
// cContact.h's own note, "its OWN local copy of CONTACT_NORMAL/
// CONTACT_AUTO_GET_HEIGHT/CONTACT_TEST"; BarCode/BarCode_Shuttle2_CCDScan.h:189
// already carries an identical `const int CONTACT_TEST = 3;`) -- golden
// cContact.cpp:77. Only the one value SaveRemoteTempOffsetFromGPIB compares
// against is carried here.
const int CONTACT_TEST = 3;

// AI(W906-FW3-TempSet-WA) 20260820: LOCALLY PROVIDED -- golden ATC/
// ATC_Handler_Side.h:19-31 (`#define`d ints), NOT ported as a shared include
// (see this file's own ATC_InterfaceForm/acarry_shims.h note above). FIVE
// sibling files already carry their own identical TU-local mirror for the 3
// values THEY need (aTester_Front.cpp/aTester_Rear.cpp/cUnitConvert.cpp/
// csystem.cpp/uHeaterThread.cpp, all citing golden ATC_TYPE_33/35/61); this
// file needs a wider set (every ATC_MODE_TYPE this dialog branches on),
// values verified directly against golden ATC_Handler_Side.h this wave.
#define ATC_TYPE_20 20
#define ATC_TYPE_32 32
#define ATC_TYPE_33 33
#define ATC_TYPE_35 35
#define ATC_TYPE_36 36
#define ATC_TYPE_60 60
#define ATC_TYPE_61 61
#define ATC_TYPE_70 70

// MyDBIProcess -- golden home aHotPlateSubstrate.h:924. Forward-declared
// locally rather than pulling in the whole header a second time under a
// different name, matching this tree's widely-used established idiom for
// this exact function (MyTempPanel.cpp:53, automation.cpp:72,
// CCLink/MyCCLink.cpp:36, OmronLaser/LaserSensor.cpp,
// SECSGEM/uHGemEquipment.cpp:93 all do the identical one-line forward decl).
extern void MyDBIProcess(AnsiString S1, AnsiString S2);

// AI(W906-FW3-TempSet-WA) 20260820: ATC/ATCInterface.h:171 already promotes
// `vclcompat::clLime` into global scope (`using vclcompat::clLime;`) but NOT
// its sibling `clRed` (vclcompat/LedCore.h:57) -- this file needs both
// (sbtExitClick's ATC-online-Caption/Color bookkeeping). Same minimal fix.
using vclcompat::clRed;

// fDynamicTemp -- golden `TfDynamicTemp *fDynamicTemp;` (DynamicTemp.cpp,
// this file's sibling wave) is declared/DEFINED in that .cpp, NOT in
// forms/fDynamicTemp.h itself (see that header's own closing note: "widgets
// go in the header, file-scope globals stay in the .cpp that owns them").
// Forward-declared here so this TU can reference the same global.
extern TfDynamicTemp *fDynamicTemp;

// FTestIF -- GATE(G-FTestIF): golden TFTestIF (cTesterIF.h) has no facade in this port. DoIniDataToForm's call (:3365) stays gated; SaveSetupFile's (:5384) re-reads only
// the [InitialMode] block -- the one this form writes -- through UTS_ReadTestIFFile_InitialMode (end of file; same subset as FileRW/Temperature.gen.inc TS_ReadTestIFFile_InitialMode). AI(W906-I01C) 20261002.
#include "forms/fHS.h"   // AI(W906-I01C) 20261002 (Ifor01): TFormHS::CheckTempOffset (forms/fHS.cpp:804, golden HS_Function.cpp:3958) for spbSaveClick's WAR15194 check
static TFormHS* UTS_FormHS() { static TFormHS* p = new TFormHS(); return p; }   // AI(W906-I01C): golden FormHS -- the global name is a stub's (Automation/SCK_ART_Remainder.h:629); same private instance as FileRW/Temperature.gen.inc:68 (CheckTempOffset keeps its state in function statics)
static void UTS_ReadTestIFFile_InitialMode();   // AI(W906-I01C): end of file (golden cTesterIF.cpp:563-570 + :713-779)
//

//------------------------------------------------------------------------------
// golden :47 -- global, ZERO-initialized (NOT `= new TfTemp_Set()`), per this
// tree's static-init-order rule.
TfTemp_Set *fTemp_Set;

// golden :48-64 -- fTempOffSet[][] row-index constants, canonical home (see
// forms/fTemp_Set.h's own note on the 3 pre-existing file-local mirrors).
const int LowBase=0;
const int MidBase=1;
const int HigBase=2;
const int UserOffSet=3;
const int SingleLimit=4;
const int TargetTemp=5;                                                         //Steven 20140924 : 各個加熱區獨立有自己的設定值
const int AmbientHotLow=6;                                                      //kevin 20140918 恆溫控制 低  offset
const int AmbientHotMid=7;                                                      //kevin 20140918 恆溫控制 中 offset
const int InitTempOffset=8;                                                     //Steven 20141117 : 起測時溫度要補Offset
const int TestOverTimeTempOffset=9;                                             //20160312 : 距離上次測試時間超過所設定時間要補Offset
const int TempEditTotal=10;                                                     //kevin 20140918 恆溫控制 5->7 -- NOT extern-declared anywhere (golden's own header keeps this one commented out); internal linkage is correct, matches golden's intent.
const int KitLowBase=11;                                                        //JerryYang 20221003 Analog要求新增Kit三點offset
const int KitMidBase=12;
const int KitHigBase=13;
const int KitAmbientHotLow=14;
const int KitAmbientHotMid=15;
const int SHigBase=16;                                                          //Ztex 2024.07.27 Add 6 Point Temperature Offset

// golden :70-93 -- ATC/FFC/Zone/MultiSensor edit-array seeds. FILE-SCOPE
// globals in golden too (NOT TfTemp_Set members) -- Init() populates them.
TEdit *ATCOffsetEdit[32];
TEdit *ATCPackageOffsetEdit[3];                                                 //Ifor 20190306 : add ATC Package Offset 三點校正
TEdit *ATCPackageTempEdit[3];                                                   //Ifor 20190306 : add ATC Package Offset 三點校正
TEdit *ATC_FFCOffsetOnTimeEdit[2][10];                                          //Ifor 20190730 : add FFC Temperature Offset
TEdit *ATC_FFCOffsetOffTimeEdit[2][10];                                         //Ifor 20190730 : add FFC Temperature Offset
TEdit *ATC_FFCOffsetEdit[2][10];                                                //Ifor 20190730 : add FFC Temperature Offset
TCheckBox *ATC_FFCPointUse[2][10];                                              //Ifor 20190730 : add FFC Temperature Offset
TStringList *sATC_CH_Tj=new TStringList();                                      //JerryYang 20231128 : 傳送ATC site map
TCheckBox *ZoneTempUse[4];                                                      //wei 20240617 Multi Zone
TEdit *ZoneTempSetting[4];                                                      //wei 20240617 Multi Zone

const int Refrigerator_Num=8;                                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
int iATCRefrigeratorHasUse[Refrigerator_Num+1];
const int iATC_MAX_CHANNEL=64;
bool bATCChannelHasUse[iATC_MAX_CHANNEL];
bool iUseCloseSiteTempCtrl[iATC_MAX_CHANNEL];
double fCloseSiteTemp=0;
int iTotalSize=0;                                                               //Ztex 2023.04.19 Add HT-1032 TriTemp Function

TCheckBox *MultiSensorOffsetUse[4];                                             //Ifor 20251203 add:ATC Multi Sensor Offset
TEdit *ATC_MultiSensorOffsetEdit[32];                                           //Ifor 20251203 add:ATC Multi Sensor Offset

//------------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: ctor is EMPTY -- all of golden's ctor
// logic (:95-408) moves to Init() below, per this wave's "ctor only fields"
// convention (widget members are NSDMI'd in forms/fTemp_Set.h). Not called
// by anything yet this wave.
//------------------------------------------------------------------------------
TfTemp_Set::TfTemp_Set()
{
}

//------------------------------------------------------------------------------
// golden ctor body :95-408, verbatim (this wave's Init(), called later by an
// integration wave once). See facade header banner for why this logic lives
// here instead of the ctor.
//------------------------------------------------------------------------------
void TfTemp_Set::Init()
{
    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        asTempCtrl[tcShuttle1]="In Sht 1-1";                                    //Temp_Set 改變元件名稱
        asTempCtrl[tcShuttle2]="In Sht 1-2";
        asTempCtrl[tcShuttle3]="In Sht 2-1";
        asTempCtrl[tcShuttle4]="In Sht 2-2";
        asTempCtrl[tcATCHotAir1]="Index";
        asTempCtrl[tcATCHotAir2]="Socket";

        asTempCtrl[tcHotPlate1]="Plate 1-1";
        asTempCtrl[tcHotPlate2]="Plate 1-2";
        asTempCtrl[tcHotPlate3]="Plate 2-1";
        asTempCtrl[tcHotPlate4]="Plate 2-2";

        asTempCtrl[tcAa1] = "Aa1";
        asTempCtrl[tcAb1] = "Ab1";
        asTempCtrl[tcAc1] = "Ac1";
        asTempCtrl[tcAd1] = "Ad1";
        asTempCtrl[tcAe1] = "Ae1";
        asTempCtrl[tcAf1] = "Af1";
        asTempCtrl[tcAg1] = "Ag1";
        asTempCtrl[tcAh1] = "Ah1";

        asTempCtrl[tcAa2] = "Aa2";
        asTempCtrl[tcAb2] = "Ab2";
        asTempCtrl[tcAc2] = "Ac2";
        asTempCtrl[tcAd2] = "Ad2";
        asTempCtrl[tcAe2] = "Ae2";
        asTempCtrl[tcAf2] = "Af2";
        asTempCtrl[tcAg2] = "Ag2";
        asTempCtrl[tcAh2] = "Ah2";

        asTempCtrl[tcBa1] = "Ba1";
        asTempCtrl[tcBb1] = "Bb1";
        asTempCtrl[tcBc1] = "Bc1";
        asTempCtrl[tcBd1] = "Bd1";
        asTempCtrl[tcBe1] = "Be1";
        asTempCtrl[tcBf1] = "Bf1";
        asTempCtrl[tcBg1] = "Bg1";
        asTempCtrl[tcBh1] = "Bh1";

        asTempCtrl[tcBa2] = "Ba2";
        asTempCtrl[tcBb2] = "Bb2";
        asTempCtrl[tcBc2] = "Bc2";
        asTempCtrl[tcBd2] = "Bd2";
        asTempCtrl[tcBe2] = "Be2";
        asTempCtrl[tcBf2] = "Bf2";
        asTempCtrl[tcBg2] = "Bg2";
        asTempCtrl[tcBh2] = "Bh2";

        asTempCtrl[tcDoor1] = "In Door";                                        //Ztex 2023.10.23 Add Index Door Heater
        asTempCtrl[tcDoor2] = "Out Door";                                       //Ztex 2023.10.23 Add Index Door Heater
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]=new TMyTempPanel(asTempCtrl[i], i);
    }
    listNormal=new TList();
    listArm1=new TList();
    listArm2=new TList();

    listNormal->Add(myTempPal[tcHotPlate1]);
    listNormal->Add(myTempPal[tcHotPlate2]);
    listNormal->Add(myTempPal[tcHotPlate3]);                                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    listNormal->Add(myTempPal[tcHotPlate4]);                                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    listNormal->Add(myTempPal[tcShuttle1]);
    listNormal->Add(myTempPal[tcShuttle2]);
    listNormal->Add(myTempPal[tcShuttle3]);                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    listNormal->Add(myTempPal[tcShuttle4]);                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    listNormal->Add(myTempPal[tcHead1]);
    listNormal->Add(myTempPal[tcHead2]);
    listNormal->Add(myTempPal[tcHead3]);
    listNormal->Add(myTempPal[tcHead4]);
    listNormal->Add(myTempPal[tcSocket]);
    listNormal->Add(myTempPal[tcChamber]);
    if(Index_ESDAir)                                                            //kevin 20200210 add index ESD temp
        listNormal->Add(myTempPal[tcIndexESD]);

    listNormal->Add(myTempPal[tcOutSht1]);                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    listNormal->Add(myTempPal[tcOutSht2]);                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    listArm1->Add(myTempPal[tcAa1]);
    listArm1->Add(myTempPal[tcBa1]);
    listArm1->Add(myTempPal[tcAb1]);
    listArm1->Add(myTempPal[tcBb1]);
    listArm1->Add(myTempPal[tcAc1]);
    listArm1->Add(myTempPal[tcBc1]);
    listArm1->Add(myTempPal[tcAd1]);
    listArm1->Add(myTempPal[tcBd1]);
    listArm1->Add(myTempPal[tcAe1]);
    listArm1->Add(myTempPal[tcBe1]);
    listArm1->Add(myTempPal[tcAf1]);
    listArm1->Add(myTempPal[tcBf1]);
    listArm1->Add(myTempPal[tcAg1]);
    listArm1->Add(myTempPal[tcBg1]);
    listArm1->Add(myTempPal[tcAh1]);
    listArm1->Add(myTempPal[tcBh1]);

    listArm2->Add(myTempPal[tcAa2]);
    listArm2->Add(myTempPal[tcBa2]);
    listArm2->Add(myTempPal[tcAb2]);
    listArm2->Add(myTempPal[tcBb2]);
    listArm2->Add(myTempPal[tcAc2]);
    listArm2->Add(myTempPal[tcBc2]);
    listArm2->Add(myTempPal[tcAd2]);
    listArm2->Add(myTempPal[tcBd2]);
    listArm2->Add(myTempPal[tcAe2]);
    listArm2->Add(myTempPal[tcBe2]);
    listArm2->Add(myTempPal[tcAf2]);
    listArm2->Add(myTempPal[tcBf2]);
    listArm2->Add(myTempPal[tcAg2]);
    listArm2->Add(myTempPal[tcBg2]);
    listArm2->Add(myTempPal[tcAh2]);
    listArm2->Add(myTempPal[tcBh2]);

    myTempPal[tcHeatGun1]->SetParent(tsHeatGun);
    myTempPal[tcHeatGun2]->SetParent(tsHeatGun);
    // AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Align) -- golden
    // `->palTemp->Align=alBottom;` (here and throughout this file).
    // vclcompat::TPanel/TControl carry no `Align` (pure Z-order/paint
    // property; see facade header's G-Align note). Gated inline rather than
    // omitted, so the golden intent stays legible.
#if 0 // GATE(G-Align)
    myTempPal[tcHeatGun1]->palTemp->Align=alBottom;
    myTempPal[tcHeatGun2]->palTemp->Align=alBottom;
#endif // GATE(G-Align)

    myTempPal[tcDUT1]->SetParent(tsDUT);
    myTempPal[tcDUT2]->SetParent(tsDUT);
    myTempPal[tcDUT3]->SetParent(tsDUT);
    myTempPal[tcDUT4]->SetParent(tsDUT);

    myTempPal[tcBase1]->SetParent(tsDUT);
    myTempPal[tcBase2]->SetParent(tsDUT);
    myTempPal[tcBase3]->SetParent(tsDUT);
    myTempPal[tcBase4]->SetParent(tsDUT);
    myTempPal[tcBase5]->SetParent(tsDUT);
    myTempPal[tcBase6]->SetParent(tsDUT);

    myTempPal[tcDoor1]->SetParent(tsDUT);                                       //Ztex 2023.10.23 Add Index Door Heater
    myTempPal[tcDoor2]->SetParent(tsDUT);                                       //Ztex 2023.10.23 Add Index Door Heater

    for(int i=0; i<listNormal->Count; i++)
    {
        TMyTempPanel *Temp;
        Temp=(TMyTempPanel*)listNormal->Items[i];
        Temp->SetParent(thNormal);
    }

    for(int i=0; i<listArm1->Count; i++)
    {
        TMyTempPanel *Temp;
        Temp=(TMyTempPanel*)listArm1->Items[i];
        Temp->SetParent(tsArm1);
    }

    for(int i=0; i<listArm2->Count; i++)
    {
        TMyTempPanel *Temp;
        Temp=(TMyTempPanel*)listArm2->Items[i];
        Temp->SetParent(tsArm2);
    }

    if(CosFunction.bUseOldATCTempOffset==true)                                  //JerryYang 20190123 ATC Offset UI使用舊版
    {
    }
    else
    {
        for(int j=0; j<tcTotalCount; j++)
        {
            myTempPal[j]->SetIndexTag(-1);
        }
        myTempPal[tcAa1]->SetIndexTag(0);
        myTempPal[tcAb1]->SetIndexTag(1);
        myTempPal[tcAc1]->SetIndexTag(2);
        myTempPal[tcAd1]->SetIndexTag(3);
        myTempPal[tcAe1]->SetIndexTag(4);
        myTempPal[tcAf1]->SetIndexTag(5);
        myTempPal[tcAg1]->SetIndexTag(6);
        myTempPal[tcAh1]->SetIndexTag(7);

        myTempPal[tcBa1]->SetIndexTag(8);
        myTempPal[tcBb1]->SetIndexTag(9);
        myTempPal[tcBc1]->SetIndexTag(10);
        myTempPal[tcBd1]->SetIndexTag(11);
        myTempPal[tcBe1]->SetIndexTag(12);
        myTempPal[tcBf1]->SetIndexTag(13);
        myTempPal[tcBg1]->SetIndexTag(14);
        myTempPal[tcBh1]->SetIndexTag(15);

        myTempPal[tcAa2]->SetIndexTag(16);
        myTempPal[tcAb2]->SetIndexTag(17);
        myTempPal[tcAc2]->SetIndexTag(18);
        myTempPal[tcAd2]->SetIndexTag(19);
        myTempPal[tcAe2]->SetIndexTag(20);
        myTempPal[tcAf2]->SetIndexTag(21);
        myTempPal[tcAg2]->SetIndexTag(22);
        myTempPal[tcAh2]->SetIndexTag(23);

        myTempPal[tcBa2]->SetIndexTag(24);
        myTempPal[tcBb2]->SetIndexTag(25);
        myTempPal[tcBc2]->SetIndexTag(26);
        myTempPal[tcBd2]->SetIndexTag(27);
        myTempPal[tcBe2]->SetIndexTag(28);
        myTempPal[tcBf2]->SetIndexTag(29);
        myTempPal[tcBg2]->SetIndexTag(30);
        myTempPal[tcBh2]->SetIndexTag(31);
        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            myTempPal[tcHotPlate1]->SetIndexTag(32);                            //Temp_Set 新增動態宣告元件
            myTempPal[tcHotPlate2]->SetIndexTag(33);
            myTempPal[tcShuttle1]->SetIndexTag(34);
            myTempPal[tcShuttle2]->SetIndexTag(35);
            myTempPal[tcHotPlate3]->SetIndexTag(36);
            myTempPal[tcHotPlate4]->SetIndexTag(37);
            myTempPal[tcShuttle3]->SetIndexTag(38);
            myTempPal[tcShuttle4]->SetIndexTag(39);
        }

        ATCOffsetEdit[ 0]=edtATCOffset_01;    ATCOffsetEdit[ 1]=edtATCOffset_02;    ATCOffsetEdit[ 2]=edtATCOffset_03;    ATCOffsetEdit[ 3]=edtATCOffset_04;
        ATCOffsetEdit[ 4]=edtATCOffset_05;    ATCOffsetEdit[ 5]=edtATCOffset_06;    ATCOffsetEdit[ 6]=edtATCOffset_07;    ATCOffsetEdit[ 7]=edtATCOffset_08;
        ATCOffsetEdit[ 8]=edtATCOffset_09;    ATCOffsetEdit[ 9]=edtATCOffset_10;    ATCOffsetEdit[10]=edtATCOffset_11;    ATCOffsetEdit[11]=edtATCOffset_12;
        ATCOffsetEdit[12]=edtATCOffset_13;    ATCOffsetEdit[13]=edtATCOffset_14;    ATCOffsetEdit[14]=edtATCOffset_15;    ATCOffsetEdit[15]=edtATCOffset_16;
        ATCOffsetEdit[16]=edtATCOffset_17;    ATCOffsetEdit[17]=edtATCOffset_18;    ATCOffsetEdit[18]=edtATCOffset_19;    ATCOffsetEdit[19]=edtATCOffset_20;
        ATCOffsetEdit[20]=edtATCOffset_21;    ATCOffsetEdit[21]=edtATCOffset_22;    ATCOffsetEdit[22]=edtATCOffset_23;    ATCOffsetEdit[23]=edtATCOffset_24;
        ATCOffsetEdit[24]=edtATCOffset_25;    ATCOffsetEdit[25]=edtATCOffset_26;    ATCOffsetEdit[26]=edtATCOffset_27;    ATCOffsetEdit[27]=edtATCOffset_28;
        ATCOffsetEdit[28]=edtATCOffset_29;    ATCOffsetEdit[29]=edtATCOffset_30;    ATCOffsetEdit[30]=edtATCOffset_31;    ATCOffsetEdit[31]=edtATCOffset_32;
    }
    //Ifor 20190306 : add ATC Package Offset 三點校正
    //==>
    ATCPackageOffsetEdit[0]=edtATC_PackageOffset_01;
    ATCPackageOffsetEdit[1]=edtATC_PackageOffset_02;
    ATCPackageOffsetEdit[2]=edtATC_PackageOffset_03;

    ATCPackageTempEdit[0]=edtATC_PackageTemp_01;
    ATCPackageTempEdit[1]=edtATC_PackageTemp_02;
    ATCPackageTempEdit[2]=edtATC_PackageTemp_03;
    //<==
    //Ifor 20190306 : add ATC Package Offset 三點校正
    //Ifor 20190730 : add FFC Temperature Offset
    //==>
    //Arm1 On Time
    ATC_FFCOffsetOnTimeEdit[0][0]=edFFC_Arm1TimeOn_01;  ATC_FFCOffsetOnTimeEdit[0][1]=edFFC_Arm1TimeOn_02;  ATC_FFCOffsetOnTimeEdit[0][2]=edFFC_Arm1TimeOn_03;
    ATC_FFCOffsetOnTimeEdit[0][3]=edFFC_Arm1TimeOn_04;  ATC_FFCOffsetOnTimeEdit[0][4]=edFFC_Arm1TimeOn_05;  ATC_FFCOffsetOnTimeEdit[0][5]=edFFC_Arm1TimeOn_06;
    ATC_FFCOffsetOnTimeEdit[0][6]=edFFC_Arm1TimeOn_07;  ATC_FFCOffsetOnTimeEdit[0][7]=edFFC_Arm1TimeOn_08;  ATC_FFCOffsetOnTimeEdit[0][8]=edFFC_Arm1TimeOn_09;
    ATC_FFCOffsetOnTimeEdit[0][9]=edFFC_Arm1TimeOn_10;
    //Arm2 On Time
    ATC_FFCOffsetOnTimeEdit[1][0]=edFFC_Arm2TimeOn_01;  ATC_FFCOffsetOnTimeEdit[1][1]=edFFC_Arm2TimeOn_02;  ATC_FFCOffsetOnTimeEdit[1][2]=edFFC_Arm2TimeOn_03;
    ATC_FFCOffsetOnTimeEdit[1][3]=edFFC_Arm2TimeOn_04;  ATC_FFCOffsetOnTimeEdit[1][4]=edFFC_Arm2TimeOn_05;  ATC_FFCOffsetOnTimeEdit[1][5]=edFFC_Arm2TimeOn_06;
    ATC_FFCOffsetOnTimeEdit[1][6]=edFFC_Arm2TimeOn_07;  ATC_FFCOffsetOnTimeEdit[1][7]=edFFC_Arm2TimeOn_08;  ATC_FFCOffsetOnTimeEdit[1][8]=edFFC_Arm2TimeOn_09;
    ATC_FFCOffsetOnTimeEdit[1][9]=edFFC_Arm2TimeOn_10;
    //Arm1 Off Time
    ATC_FFCOffsetOffTimeEdit[0][0]=edFFC_Arm1TimeOff_01;ATC_FFCOffsetOffTimeEdit[0][1]=edFFC_Arm1TimeOff_02;ATC_FFCOffsetOffTimeEdit[0][2]=edFFC_Arm1TimeOff_03;
    ATC_FFCOffsetOffTimeEdit[0][3]=edFFC_Arm1TimeOff_04;ATC_FFCOffsetOffTimeEdit[0][4]=edFFC_Arm1TimeOff_05;ATC_FFCOffsetOffTimeEdit[0][5]=edFFC_Arm1TimeOff_06;
    ATC_FFCOffsetOffTimeEdit[0][6]=edFFC_Arm1TimeOff_07;ATC_FFCOffsetOffTimeEdit[0][7]=edFFC_Arm1TimeOff_08;ATC_FFCOffsetOffTimeEdit[0][8]=edFFC_Arm1TimeOff_09;
    ATC_FFCOffsetOffTimeEdit[0][9]=edFFC_Arm1TimeOff_10;
    //Arm2 Off Time
    ATC_FFCOffsetOffTimeEdit[1][0]=edFFC_Arm2TimeOff_01;ATC_FFCOffsetOffTimeEdit[1][1]=edFFC_Arm2TimeOff_02;ATC_FFCOffsetOffTimeEdit[1][2]=edFFC_Arm2TimeOff_03;
    ATC_FFCOffsetOffTimeEdit[1][3]=edFFC_Arm2TimeOff_04;ATC_FFCOffsetOffTimeEdit[1][4]=edFFC_Arm2TimeOff_05;ATC_FFCOffsetOffTimeEdit[1][5]=edFFC_Arm2TimeOff_06;
    ATC_FFCOffsetOffTimeEdit[1][6]=edFFC_Arm2TimeOff_07;ATC_FFCOffsetOffTimeEdit[1][7]=edFFC_Arm2TimeOff_08;ATC_FFCOffsetOffTimeEdit[1][8]=edFFC_Arm2TimeOff_09;
    ATC_FFCOffsetOffTimeEdit[1][9]=edFFC_Arm2TimeOff_10;
    //Arm1 Offset
    ATC_FFCOffsetEdit[0][0]=edFFC_Arm1Offset_01;        ATC_FFCOffsetEdit[0][1]=edFFC_Arm1Offset_02;        ATC_FFCOffsetEdit[0][2]=edFFC_Arm1Offset_03;
    ATC_FFCOffsetEdit[0][3]=edFFC_Arm1Offset_04;        ATC_FFCOffsetEdit[0][4]=edFFC_Arm1Offset_05;        ATC_FFCOffsetEdit[0][5]=edFFC_Arm1Offset_06;
    ATC_FFCOffsetEdit[0][6]=edFFC_Arm1Offset_07;        ATC_FFCOffsetEdit[0][7]=edFFC_Arm1Offset_08;        ATC_FFCOffsetEdit[0][8]=edFFC_Arm1Offset_09;
    ATC_FFCOffsetEdit[0][9]=edFFC_Arm1Offset_10;
    //Arm2 Offset
    ATC_FFCOffsetEdit[1][0]=edFFC_Arm2Offset_01;        ATC_FFCOffsetEdit[1][1]=edFFC_Arm2Offset_02;        ATC_FFCOffsetEdit[1][2]=edFFC_Arm2Offset_03;
    ATC_FFCOffsetEdit[1][3]=edFFC_Arm2Offset_04;        ATC_FFCOffsetEdit[1][4]=edFFC_Arm2Offset_05;        ATC_FFCOffsetEdit[1][5]=edFFC_Arm2Offset_06;
    ATC_FFCOffsetEdit[1][6]=edFFC_Arm2Offset_07;        ATC_FFCOffsetEdit[1][7]=edFFC_Arm2Offset_08;        ATC_FFCOffsetEdit[1][8]=edFFC_Arm2Offset_09;
    ATC_FFCOffsetEdit[1][9]=edFFC_Arm2Offset_10;
    //Arm1 Point Enable
    ATC_FFCPointUse[0][0]  =edFFC_Arm1Enable_01;        ATC_FFCPointUse[0][1]  =edFFC_Arm1Enable_02;        ATC_FFCPointUse[0][2]  =edFFC_Arm1Enable_03;
    ATC_FFCPointUse[0][3]  =edFFC_Arm1Enable_04;        ATC_FFCPointUse[0][4]  =edFFC_Arm1Enable_05;        ATC_FFCPointUse[0][5]  =edFFC_Arm1Enable_06;
    ATC_FFCPointUse[0][6]  =edFFC_Arm1Enable_07;        ATC_FFCPointUse[0][7]  =edFFC_Arm1Enable_08;        ATC_FFCPointUse[0][8]  =edFFC_Arm1Enable_09;
    ATC_FFCPointUse[0][9]  =edFFC_Arm1Enable_10;
    //Arm2 Point Enable
    ATC_FFCPointUse[1][0]  =edFFC_Arm2Enable_01;        ATC_FFCPointUse[1][1]  =edFFC_Arm2Enable_02;        ATC_FFCPointUse[1][2]  =edFFC_Arm2Enable_03;
    ATC_FFCPointUse[1][3]  =edFFC_Arm2Enable_04;        ATC_FFCPointUse[1][4]  =edFFC_Arm2Enable_05;        ATC_FFCPointUse[1][5]  =edFFC_Arm2Enable_06;
    ATC_FFCPointUse[1][6]  =edFFC_Arm2Enable_07;        ATC_FFCPointUse[1][7]  =edFFC_Arm2Enable_08;        ATC_FFCPointUse[1][8]  =edFFC_Arm2Enable_09;
    ATC_FFCPointUse[1][9]  =edFFC_Arm2Enable_10;
    //<==
    //Ifor 20190730 : add FFC Temperature Offset
    ZoneTempUse[0]=cbZoneEnable1;
    ZoneTempUse[1]=cbZoneEnable2;
    ZoneTempUse[2]=cbZoneEnable3;
    ZoneTempUse[3]=cbZoneEnable4;

    ZoneTempSetting[0]=edZoneSetting1;
    ZoneTempSetting[1]=edZoneSetting2;
    ZoneTempSetting[2]=edZoneSetting3;
    ZoneTempSetting[3]=edZoneSetting4;
    if(AirStream_Select==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        tmr_ATC_Deforst->Enabled=true;
    }

    MultiSensorOffsetUse[0]=cbMultiSensorEnable1;
    MultiSensorOffsetUse[1]=cbMultiSensorEnable2;
    MultiSensorOffsetUse[2]=cbMultiSensorEnable3;
    MultiSensorOffsetUse[3]=cbMultiSensorEnable4;

    ATC_MultiSensorOffsetEdit[0]=edATC_Arm1_S1_TC1;     ATC_MultiSensorOffsetEdit[1]=edATC_Arm1_S1_TC2;     ATC_MultiSensorOffsetEdit[2]=edATC_Arm1_S1_TC3;     ATC_MultiSensorOffsetEdit[3]=edATC_Arm1_S1_TC4;
    ATC_MultiSensorOffsetEdit[4]=edATC_Arm1_S2_TC1;     ATC_MultiSensorOffsetEdit[5]=edATC_Arm1_S2_TC2;     ATC_MultiSensorOffsetEdit[6]=edATC_Arm1_S2_TC3;     ATC_MultiSensorOffsetEdit[7]=edATC_Arm1_S2_TC4;
    ATC_MultiSensorOffsetEdit[8]=edATC_Arm1_S3_TC1;     ATC_MultiSensorOffsetEdit[9]=edATC_Arm1_S3_TC2;     ATC_MultiSensorOffsetEdit[10]=edATC_Arm1_S3_TC3;    ATC_MultiSensorOffsetEdit[11]=edATC_Arm1_S3_TC4;
    ATC_MultiSensorOffsetEdit[12]=edATC_Arm1_S4_TC1;    ATC_MultiSensorOffsetEdit[13]=edATC_Arm1_S4_TC2;    ATC_MultiSensorOffsetEdit[14]=edATC_Arm1_S4_TC3;    ATC_MultiSensorOffsetEdit[15]=edATC_Arm1_S4_TC4;

    ATC_MultiSensorOffsetEdit[16]=edATC_Arm2_S1_TC1;    ATC_MultiSensorOffsetEdit[17]=edATC_Arm2_S1_TC2;    ATC_MultiSensorOffsetEdit[18]=edATC_Arm2_S1_TC3;    ATC_MultiSensorOffsetEdit[19]=edATC_Arm2_S1_TC4;
    ATC_MultiSensorOffsetEdit[20]=edATC_Arm2_S2_TC1;    ATC_MultiSensorOffsetEdit[21]=edATC_Arm2_S2_TC2;    ATC_MultiSensorOffsetEdit[22]=edATC_Arm2_S2_TC3;    ATC_MultiSensorOffsetEdit[23]=edATC_Arm2_S2_TC4;
    ATC_MultiSensorOffsetEdit[24]=edATC_Arm2_S3_TC1;    ATC_MultiSensorOffsetEdit[25]=edATC_Arm2_S3_TC2;    ATC_MultiSensorOffsetEdit[26]=edATC_Arm2_S3_TC3;    ATC_MultiSensorOffsetEdit[27]=edATC_Arm2_S3_TC4;
    ATC_MultiSensorOffsetEdit[28]=edATC_Arm2_S4_TC1;    ATC_MultiSensorOffsetEdit[29]=edATC_Arm2_S4_TC2;    ATC_MultiSensorOffsetEdit[30]=edATC_Arm2_S4_TC3;    ATC_MultiSensorOffsetEdit[31]=edATC_Arm2_S4_TC4;
}

//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Image) -- golden :410-419. Entire
// body dereferences `Image1` (golden TImage*), which has ZERO port anywhere
// in this tree (see facade header G-Image). Whole body gated; function
// reduces to a safe no-op (matches forms/fLotInfo.h's identical treatment of
// LoadRTCFullViewImg for the same class of missing dependency).
//---------------------------------------------------------------------------
void TfTemp_Set::SetBasePointIMG(int iPoint)
{
#if 0 // GATE(G-Image) -- Image1 (TImage*) has no port anywhere in this tree
    AnsiString str;
    if(iPoint<=8)                                                               //kevin 20141006 add 尚未有圖片  //Ifor 20191015 4 => 8 新增五點校正圖檔
    {
        str.sprintf("%stmode%d.bmp", BmpPath, iPoint);
        if(FileExists(str))
            Image1->Picture->LoadFromFile(str);
    }
#endif // GATE(G-Image)
    (void)iPoint;   // write-only once GATE(G-Image) is inert
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- golden casts `Sender`
// to `TRadioGroup*` here (a GOLDEN ODDITY: rb1Point/rb2Point/.../rb6Point are
// each individually-declared `TRadioButton*` members sharing this one OnClick
// handler, not children of a real TRadioGroup -- verified against golden's
// own uTemp_Set.h __published block; the cast type is wrong-looking but
// harmless in real VCL because `Tag` lives on the common TComponent ancestor
// regardless of the static cast type used to read it). Translated literally
// (same class of preserved oddity as forms/fTemperFrom.h's :607 note) --
// gated because (a) `Tag` has no port on ANY vclcompat control here (these 5
// radio buttons are bare `TRadioButton`, not the `TfTemp_SetTagEdit`/
// `TfTemp_SetTagButton` wrappers), and (b) even if it were spellable, its
// only consumer, `SetBasePointIMG`, is ITSELF already fully gated (GATE
// G-Image) -- so this call is inert twice over.
void TfTemp_Set::rb1PointClick(TObject *Sender)
{
#if 0 // GATE(dep-Tag)
    TRadioGroup *Ptr=(TRadioGroup*) Sender;
    SetBasePointIMG(Ptr->Tag);
#endif // GATE(dep-Tag)
    UpDateEdit();
}
//---------------------------------------------------------------------------
void TfTemp_Set::FormCreate(TObject *Sender)
{
    pgcTempSetting->ActivePageIndex=0;
}
//---------------------------------------------------------------------------
void TfTemp_Set::FormShow(TObject *Sender)
{
    AnsiString S;

    rgTemperatureMode       ->Enabled=true;                                     //Steven 20120804 Start: 先在最上面Enable全部畫面
    gbBasePoint             ->Enabled=true;
    gbHotModeSetting        ->Enabled=true;
    gbAmbientModeSetting    ->Enabled=true;
    tsOther                 ->Enabled=true;                                     //kevin 20150127
    rgIndexHeatMode         ->Enabled=true;
    edLowBase               ->Enabled=true;
    edMidBase               ->Enabled=true;
    edHighBase              ->Enabled=true;
    edAbLow                 ->Enabled=true;                                     //kevin 20140918
    edAbMid                 ->Enabled=true;                                     //kevin 20140918
    btClearAll              ->Enabled=true;
    rbATCActiveOn           ->Enabled=fSecurity->Insufficient(158, false);      //JerryYang 20181214 : 新增ATC權限管控
    edSoakTime              ->Enabled=true;                                     //JerryYang 20220311 : ATP鎖定Critical parameter
    edWorkTemp              ->Enabled=true;
    edSHighBase             ->Enabled=true;                                     //Ztex 2024.07.27 Add 6 Point Temperature Offset

    bNeedChange=true;
    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08 )                                       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {
        switch(TestIF.iTestMode)
        {
            case SingleSite:
                if(ATC_SYSTEM>=eATC60 &&
                   Temperature.bATCActiveCooling==true)                         //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                {
                }
                else
                {
                    if(TestIF_File.b2CableLayoutKit)                            //JerryYang 20160826 single site兩條線版本(使用一支加熱棒)
                    {
                    }
                    else
                    {
                        bNeedChange=false;
                    }
                }
                break;
            case DualSite:                                                      //1x2
                if((ATC_SYSTEM==eNewATCSystem ||                                //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {
                    bNeedChange=false;
                }
                else if(TestIF_File.b2CableLayoutKit)                           //JerryYang 20160826 1x2 兩條線版本使用兩支加熱棒
                {
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //jou 2015-10-15  : 16溫控器 1條線版本
                {
                    bNeedChange=false;
                }
                else
                {
                    bNeedChange=false;
                }
                break;
            case TriSite1X3:
                break;
            case QualSite1X4:                                                   //1x4
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 修正 ATC 1*4 溫度顯示異常問題
                   Temperature.bATCActiveCooling==true)
                {
                }
                else if(TestIF_File.b2CableLayoutKit)                           //Steven 20150724 : 16溫控器 2條線版本
                {
                }
                else if(TestIF_File.bNS7000kit==true ||
                        TestIF_File.b1CableLayoutKit==true)
                {
                    bNeedChange=false;
                }
                else                                                            //Steven 20250821 : 1x4 三條線版本
                {
                    bNeedChange=false;
                }
                break;
            case QualSite2X2:                                                   //2x2
                if(TestIF_File.bSquare_OctalKit ||                              //Steven 20141224 : 2x2Site使用8Site Kit
                   TestIF_File.b2x2Use16SiteKit)                                //Steven 20191113 : 2x2Site使用16Site Kit
                {
                }
                else if(CUSTOMER_CODE==CC_ATEC &&
                        TestIF_File.b2CableLayoutKit &&
                        DeviceForm_File.iHeadDeviceCT==3)                       //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //JerryYang 20181207 2x2新增一條線版本
                {
                    bNeedChange=false;
                }
                else
                {
                    bNeedChange=false;
                }
                break;
        }
    }

    for(int i=tcHotPlate1; i<tcTotalCount; i++)                                 //Ifor 20191024 : Fix 權限切換後無法編輯Temp Offset問題
    {
        myTempPal[i]->edLow->Enabled             =true;
        myTempPal[i]->edMid->Enabled             =true;
        myTempPal[i]->edLowbase->Enabled         =true;
        myTempPal[i]->edBase->Enabled            =true;
        myTempPal[i]->edHighBase->Enabled        =true;
        myTempPal[i]->edSHighBase->Enabled       =true;                         //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CUSTOMER_CODE!=CC_KYEC_LEE)
        {
            #ifndef HiSilicon
            if(IniConfig.bVTESTFunction==true)                                  //jou 20231101 : 客戶要求 temp頁面 Single Limit 不使用
                myTempPal[i]->edSingleLimit->Enabled =false;
            else
                myTempPal[i]->edSingleLimit->Enabled =true;
            myTempPal[i]->edInitTempOffset->Enabled  =true;
            myTempPal[i]->edEOTTempOffset->Enabled   =true;
            #else
            myTempPal[i]->edSingleLimit->Enabled     =false;
            myTempPal[i]->edInitTempOffset->Enabled  =false;
            myTempPal[i]->edEOTTempOffset->Enabled   =false;
            edWorkTemp->Enabled   = false;                                      //kevin 20200102 add disable

            myTempPal[i]->edLowbase->Enabled         =false;                    //kevin 20200508 add HISI
            myTempPal[i]->edBase->Enabled            =false;
            myTempPal[i]->edHighBase->Enabled        =false;
            myTempPal[i]->edSHighBase->Enabled       =false;                    //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edLow->Enabled            =false;                     //kevin 20200508 add HISI
            myTempPal[i]->edMid->Enabled            =false;
            #endif
            myTempPal[i]->edOffset->Enabled          =true;                     //kevin 20200514 溫度offset 可以輸入
        }
        else                                                                    //Ifor 20200305 : fix OP權限開啟後Offset鎖死
        {
            myTempPal[i]->edOffset->Enabled             =true;
            myTempPal[i]->edSingleLimit->Enabled        =true;
            myTempPal[i]->edInitTempOffset->Enabled     =true;
            myTempPal[i]->edEOTTempOffset->Enabled      =true;
        }
    }

    if(ATC_SYSTEM==eNewATCSystem &&
       (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 ||       //JerryYang 20250729 : ATC 3.3+6.0
        ATC_MixMode==eMixATC60_ATC33))                          //Steven 20181102 : For QTI全球統一Offset
    {
        cbEnableIndividualMode->Visible=false;
        tsBoostFunc->TabVisible=true;
        tsLBCtr->TabVisible=true;
        rgATC60AirOn->Visible=true;                                             //Steven 20191025 : for ATC6.0
    }
    else
    {
        cbEnableIndividualMode->Visible=CosFunction.bUseIndividulTempSet;       //Steven 20140924 : 各個加熱區獨立有自己的設定值
        tsBoostFunc->TabVisible=false;
        tsLBCtr->TabVisible=false;
        if(LB_TEMP==true && CosFunction.bNonATCSupportLBFunction==true)         //JerryYang 20220126: non ATC也支援LB function
        {
            tsLBCtr->TabVisible=true;
        }
        else
        {
            tsLBCtr->TabVisible=false;
        }
        rgATC60AirOn->Visible=false;                                            //Steven 20191025 : for ATC6.0
    }

    tsOthers                ->TabVisible=LB_TEMP_UpDown;                        //Frank 20241231 : add
    palIndiTemp             ->Visible=(CosFunction.bUseIndividulTempSet || Tri_Temp_Machine==1);                        //Steven 20140924 : 各個加熱區獨立有自己的設定值  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    chkSoakTimeReduce       ->Visible=CosFunction.bUseInitialDelayAsSoakTime;   //Steven 20170329 (wei) : 使用initial delay當 Soak time
    chkTempErrorAction      ->Visible=CosFunction.bTempAlarmBinNeedToError;     //Steven 20251022 : Temp alarm need put to error bin
    chkUseCDAOnly           ->Visible=(INSTALL_HEAT_GUN>1);                     //Steven 20181012 : 使用熱風槍吹冷風
    palLBAir                ->Visible=(CosFunction.bNonATCSupportLBFunction);   //JerryYang 20220927 : add
    tsUnUse                 ->TabVisible=false;
    tsTriTempSet            ->TabVisible=(Tri_Temp_Machine==1 || (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));                //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    //Steven 20120804 End: 先在最上面Enable全部畫面

    // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- golden
    // `TRadioButton::SetFocus()` (a real VCL TWinControl method) has no port
    // on `vclcompat::TRadioButton` (pure keyboard-focus chrome, no business
    // state; matches this tree's "no rendering/no window-chrome" convention).
#if 0 // GATE(dep-SetFocus)
    rbTemp->SetFocus();
#endif // GATE(dep-SetFocus)
    GetLimitAuth();
    pnlUseOtherArmToTestAfterInitialDelay->Visible=CosFunction.bAfterInitialDelayUseOtherArm;                           //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail

    if(IniConfig.bI03AmbientTempControl)                                        //kevin 20140918
    {
        rgTemperatureMode->Items->Clear();
        rgTemperatureMode->Items->Add("Hot");
        rgTemperatureMode->Items->Add("Ambient");
        if(IniConfig.bOnlyRoomOrHot)
        {
            pnlCover->Visible=true;                                             //Steven 20210816 : 隱藏Temp control的部分
            rgTemperatureMode->Items->Add("");
        }
        else
        {
            rgTemperatureMode->Items->Add("Ambient/High/Temp control");
        }
        rgTemperatureMode->Items->Add("Temp control");
    }
    else if(IniConfig.bOnlyRoomOrHot)                                           //Steven 20110914
    {
        pnlCover->Visible=true;                                                 //Steven 20210816 : 隱藏Temp control的部分
        rgTemperatureMode->Items->Clear();
        rgTemperatureMode->Items->Add("Hot");
        rgTemperatureMode->Items->Add("Ambient");
    }

    ReadTempFile(true);

    S.sprintf("Temperature Parameter ''%s''  ", GetLastOpenFN());
    // AI(W906-FW3-TempSet-WA) 20260820: golden `Caption=S;` -- TfTemp_Set is
    // a plain (non-TControl/TForm) facade class here, matching every other
    // translated form facade in this tree (e.g. TfTemperFrom); it carries no
    // `Caption` of its own (a real VCL TForm::Caption is the dialog's title
    // bar text -- pure window-chrome, not business state). golden's own
    // read side of this member is nowhere in the 6,970-line file (grepped
    // `\bCaption\b` outside this one write, 20260820 -- 0 hits reading it
    // back), so dropping the write loses no observable behaviour this file
    // itself depends on.
    (void)S;

    switch(Temperature.iTempMode)
    {
        case 1:
        case 2:
        case 4:
        case 8:                                                                 //kevin 20141006
            SetBasePointIMG(Temperature.iTempMode);
            break;
        case 16:                                                                //Ztex 2024.07.27 Add 6 Point Temperature Offset
            SetBasePointIMG(Temperature.iTempMode);
            break;
        default :
            rb1Point->Checked=true;
            Temperature.iTempMode=1;
            SetBasePointIMG(Temperature.iTempMode);
    }

    //jou 2012-03-12 假如ATC模式只支援Head mode & Head + Socket mode
    if(ATC_SYSTEM<=eATC30)                                                      //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    {
        if(IniConfig.bNoHeadaddChamberOption)
        {
            btClearAll->Visible=false;
        }
    }
    else
    {
        Label44->Visible=false;
        Label45->Visible=false;
        edChamberCoolTemp->Visible=false;
    }
                                                                                //Steven 20240201 : 重新整理Index Heat Mode
    gbSocketAirCooling->Visible=(IniConfig.bL03SocketAirCoolingCT);             //Steven 20160714

    rgIndexHeatMode->Enabled=(LastSet.iTemperature==Tempture_Hot)?false:true;   //Steven 20091102
    // AI(W906-FW3-TempSet-WA) 20260820: golden `Left=10;Top=10;` (TForm
    // position) -- pure window-chrome, same class of drop as `Caption`
    // above; no Wave A method reads either back.

    if(HasICUnderMachine()==false)                                              //jou 2010-01-11 start : 機台上有IC時，不能改變加熱狀態
    {
        #ifdef HiSilicon                                                        //kevin 20200508 add HISI no change
            btClearAll->Visible         =false;
            edAbLow->Enabled            =false;
            edAbMid->Enabled            =false;
            edLowBase->Enabled          =false;
            edMidBase->Enabled          =false;
            edHighBase->Enabled         =false;
            gbBasePoint->Enabled        =false;
            rgTemperatureMode->Enabled  =false;
            edSHighBase->Enabled        =false;                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset

        #else
            btClearAll->Visible         =fSecurity->Insufficient(17, false);
            edAbLow->Enabled            =fSecurity->Insufficient(17, false);    //kevin 20140918 add
            edAbMid->Enabled            =fSecurity->Insufficient(17, false);    //kevin 20140918 add
            edLowBase->Enabled          =fSecurity->Insufficient(17, false);
            edMidBase->Enabled          =fSecurity->Insufficient(17, false);
            edHighBase->Enabled         =fSecurity->Insufficient(17, false);
            gbBasePoint->Enabled        =fSecurity->Insufficient(17, false);
            rgTemperatureMode->Enabled  =fSecurity->Insufficient(17, false);
            edSHighBase->Enabled        =fSecurity->Insufficient(17, false);    //Ztex 2024.07.27 Add 6 Point Temperature Offset

            if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)      //JerryYang 20220311 : ATP鎖定Critical parameter
            {
                if(bAuthCriticalPara[13])
                {
                    rgTemperatureMode->Enabled=false;
                }
            }
        #endif

        if(IniConfig.bKoreaFunction==false)
        {
            for(int i=0; i<tcTotalCount; i++)
            {
                myTempPal[i]->edLow->Enabled        =fSecurity->Insufficient(17, false);                                //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled        =fSecurity->Insufficient(17, false);
                myTempPal[i]->edLowbase->Enabled    =fSecurity->Insufficient(17, false);
                myTempPal[i]->edBase->Enabled       =fSecurity->Insufficient(17, false);
                myTempPal[i]->edHighBase->Enabled   =fSecurity->Insufficient(17, false);
                myTempPal[i]->edSHighBase->Enabled  =fSecurity->Insufficient(17, false);                                //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }

        lblNotice->Visible=false;
    }
    else
    {
        btClearAll->Visible =false;
        edAbLow->Enabled    =false;                                             //kevin 20140918
        edAbMid->Enabled    =false;                                             //kevin 20140918
        edLowBase->Enabled  =false;
        edMidBase->Enabled  =false;
        edHighBase->Enabled =false;
        gbBasePoint->Enabled=false;
        rgTemperatureMode->Enabled=false;
        edSHighBase->Enabled=false;                                             //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(IniConfig.bKoreaFunction==true && ATC_SYSTEM!=eATC60 &&
           (Temperature.bBoostFuncttion || Temperature.bLBTempFunction))
        {
        }
        else if(IniConfig.bKoreaFunction==false)
        {
            for(int i=0; i<tcTotalCount; i++)
            {
                myTempPal[i]->edLow->Enabled=false;                             //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=false;
                myTempPal[i]->edLowbase->Enabled=false;
                myTempPal[i]->edBase->Enabled=false;
                myTempPal[i]->edHighBase->Enabled=false;
                myTempPal[i]->edSHighBase->Enabled=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }

        lblNotice->Visible=true;
    }
    //jou 2010-01-11 end

    //jou 2011-12-21
    if(IniConfig.bKoreaFunction==true ||
       CUSTOMER_CODE==CC_UTAC_TW)                                               //Sam 20240416 : 聯測東廷要求開放
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            if(AccessLevel>iDefSupervisorLevel)                                 //jou 2014-06-19 Security Have 5 Level 2->iDefSupervisorLevel
            {
                myTempPal[i]->edLow->Enabled=true;                              //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=true;
                myTempPal[i]->edLowbase->Enabled=true;
                myTempPal[i]->edBase->Enabled=true;
                myTempPal[i]->edHighBase->Enabled=true;
                myTempPal[i]->edSHighBase->Enabled=true;                        //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
            else
            {
                myTempPal[i]->edLow->Enabled=false;                             //Steven 20200717 : 補上前兩點的權限控制
                myTempPal[i]->edMid->Enabled=false;
                myTempPal[i]->edLowbase->Enabled=false;
                myTempPal[i]->edBase->Enabled=false;
                myTempPal[i]->edHighBase->Enabled=false;
                myTempPal[i]->edSHighBase->Enabled=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }
    }

    PnlDewPointAlarm->Visible=false;                                            //(DewPoint_Hardware_Install>0);            //Steven 20191017 : 露點計

    if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_SCK)                          //Steven 20101221 //ChungHung 20130621 add SCK RMS
    {
        if(IniConfig.bEnableRms)                                                //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            gbHotModeSetting    ->Enabled=false;
            gbAmbientModeSetting->Enabled=false;
            gbBasePoint         ->Enabled=false;
            gbTempOffset        ->Enabled=(CUSTOMER_CODE==CC_SCK);          //JimmyChiu 20211008 : Add for JSCK
            rgTemperatureMode   ->Enabled=false;
            rgIndexHeatMode     ->Enabled=false;
            pnlOffsetBase       ->Enabled=false;                                //Steven 20210616 : JSCK RMS enabled, can not changed "Site map, temperature & Bin map"
        }
        else
        {
            gbHotModeSetting    ->Enabled=true;
            gbAmbientModeSetting->Enabled=true;
            gbBasePoint         ->Enabled=true;
            gbTempOffset        ->Enabled=true;
            rgTemperatureMode   ->Enabled=true;
            rgIndexHeatMode     ->Enabled=true;
            pnlOffsetBase       ->Enabled=true;                                 //Steven 20210616 : JSCK RMS enabled, can not changed "Site map, temperature & Bin map"
        }
    }

    lblTempBase ->Visible=true;
    btClearAll  ->Visible=true;
    edAbLow     ->Visible=true;                                                 //kevin 20140918
    edAbMid     ->Visible=true;                                                 //kevin 20140918
    edLowBase   ->Visible=true;
    edMidBase   ->Visible=true;
    edHighBase  ->Visible=true;
    edWorkTemp  ->Visible=true;
    edSHighBase ->Visible=true;                                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset

    grpChamberBoost->Visible=CosFunction.bUseChamberBoostMode;

    if(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)      //Steven 20140924 : 各個加熱區獨立有自己的設定值
        edWorkTemp->Visible=false;

    lblShtSoakTime      ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    edInitialStart2Time ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    lbllblShtSoakTimeSec->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time
    rgShuttleSoakTime   ->Visible=IniConfig.bIndexPickupWait;                   //jou 2012-06-29 Index Pick up need wait Soak Time

    chkAmbUsingAFan     ->Visible   =IniConfig.bAmbRunChamberFanCanStop;        //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    grpRS232            ->Visible   =(ATC_SYSTEM==eATCSiliconType);
    tsATC               ->TabVisible=(ATC_SYSTEM>=eATC30 &&                     //20141204 ChungHung add for ATC3.0
                                      ATC_SYSTEM!=eNonChamber);                 //2014-05-30    Dell    for ATC6.0

    if(CUSTOMER_CODE==CC_SCS)                                                   //jou 2015-12-10 SCS 要求把ATC PID頁面關掉不顯示
    {
        tsATC_PID->TabVisible=false;
        tsATC70->TabVisible=false;                                              //Ifor 20160711 SCS 要求把ATC 7.0頁面關掉不顯示
    }
    else
    {
        tsATC_PID           ->TabVisible=(ATC_SYSTEM==eATCHonPrecType ||        //wei 20151019 ATC PID顯示
                                         (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_20) ||
                                         (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61) ||
                                         (Tri_Temp_Machine==1));                //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        tsATC70->TabVisible =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70 //Ifor 20160412 add ATC7.0 加入NewATCIF
    }

    if(ATC_SYSTEM==eNewATCSystem && CosFunction.bUseFFCTempOffset==true)
    {
        tsATC_FFC->TabVisible=true;
    }
    else
    {
        tsATC_FFC->TabVisible=false;
    }

    tsATC               ->Enabled   =(AccessLevel>=LevelSet.AccessLevel[44]);   //Steven 20130122 : 加入權限
    gbATCAmbientTemp    ->Visible   =(ATC_SYSTEM>=eATC30 &&
                                      ATC_SYSTEM!=eNonChamber);                 //wei 20151013  by Setup File ATC Ambient Temp set

    grpTemperatureCheck ->Visible   =(ATC_SYSTEM==eATCSiliconType);             //Steven 20120611
    grbATCTempAlarm     ->Visible   =(ATC_SYSTEM==eATCSiliconType);             //Steven 20120611
    grpATC_HotAir       ->Visible   =(ATC_SYSTEM==eNewATCSystem &&
                                      (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
                                       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                                       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));
    tsHeatGun           ->TabVisible=(INSTALL_HEAT_GUN ||                       //Steven 20120525 : 熱風槍
                                      ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61 ||
                                      INSTALL_ATC_HEAT_GUN);                    //Ifor 20250926 add:ATC_HEAT_GUN 顯示
    tsArm1              ->TabVisible=(USE_16_HEATER==eht16Heater    ||
                                      USE_16_HEATER==eht16HeaterEJ1N   ||
                                      USE_16_HEATER==eht32HeaterEJ1N   ||       //Steven 20140923 : Index使用EJ1N版32組加熱器
                                      USE_16_HEATER==eht32HeaterKT4H   ||       //Steven 20150211 : Index使用KT4H版32組加熱器
                                      USE_16_HEATER==eht16HeaterDTME08 ||       //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                                      USE_16_HEATER==eht32HeaterDTME08);        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

    tsArm2              ->TabVisible=(USE_16_HEATER==eht16Heater   ||
                                      USE_16_HEATER==eht16HeaterEJ1N  ||
                                      USE_16_HEATER==eht32HeaterEJ1N  ||        //Steven 20140923 : Index使用EJ1N版32組加熱器
                                      USE_16_HEATER==eht32HeaterKT4H  ||        //Steven 20150211 : Index使用KT4H版32組加熱器
                                      USE_16_HEATER==eht16HeaterDTME08||        //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
                                      USE_16_HEATER==eht32HeaterDTME08);        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器

    tsDUT               ->TabVisible=(iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea);

    tsAmbCtr->TabVisible=IniConfig.bI03AmbientTempControl;                      //jou 2015-12-10 Ambient_control Guard Band 顯示依照 bI03AmbientTempControl Function打開關閉
    rgAmbientBrand->Visible=IniConfig.bL20AbientGuardBand;                      //kevin 20180115 (Steven) add Amient Guard Band
    tsSigmaForTemp->TabVisible=IniConfig.bL22Enable3SigmaTempMonitor;           //kevin 20200521 : ASEKH 3SIGMA  溫度統計

    SetTempPanelCaption();

    ShowLineOnTop();
    pgcTempOffset->ActivePageIndex=0;
    // AI(W906-FW3-TempSet-WA) 20260820: golden `pgcTempOffset->Repaint();`
    // dropped -- pure GDI repaint trigger, no vclcompat::TPageControl member
    // (matches this tree's "no rendering" convention, same class as
    // TfObserverGrid::Refresh()/Repaint() no-ops).
    btnSort->Visible=false;
    fShow=true;

    //Steven 20120804 Start: 在最下面決定權限能不能Enable
    rgTemperatureMode       ->Enabled=(AccessLevel<LevelSet.AccessLevel[50])?false:rgTemperatureMode     ->Enabled;
    gbBasePoint             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:gbBasePoint           ->Enabled;
    gbHotModeSetting        ->Enabled=(AccessLevel<LevelSet.AccessLevel[55])?false:gbHotModeSetting      ->Enabled;
    gbAmbientModeSetting    ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:gbAmbientModeSetting  ->Enabled;
    tsOther                 ->Enabled=(AccessLevel<LevelSet.AccessLevel[56])?false:tsOther               ->Enabled;     //kevin 20150127 add
    rgIndexHeatMode         ->Enabled=(AccessLevel<LevelSet.AccessLevel[57])?false:rgIndexHeatMode       ->Enabled;
    btClearAll              ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:btClearAll            ->Enabled;

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)
    {
        edWorkTemp          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edWorkTemp                ->Enabled;                                         //20140312 wei Kyec
    }
    else
    {
        edAbLow             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edAbLow->Enabled;                    //kevin 20140918
        edAbMid             ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edAbMid->Enabled;                    //kevin 20140918
        edLowBase           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edLowBase->Enabled;
        edMidBase           ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edMidBase->Enabled;
        edHighBase          ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edHighBase->Enabled;
        edSHighBase         ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edSHighBase->Enabled;                //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(IniConfig.bKoreaFunction)                                            //ChungHung 20140609 add Korea 溫度要卡權限 包含 Temp_Set
            edWorkTemp      ->Enabled=(AccessLevel<LevelSet.AccessLevel[52])?false:edWorkTemp                ->Enabled;
    }

    for(int i=tcHotPlate1; i<tcTotalCount; i++)
    {
        myTempPal[i]->edLow->Enabled             =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edLow->Enabled;                                    //Steven 20200717 : 補上前兩點的權限控制
        myTempPal[i]->edMid->Enabled             =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edMid->Enabled;
        myTempPal[i]->edLowbase->Enabled         =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edLowbase->Enabled;
        myTempPal[i]->edBase->Enabled            =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edBase->Enabled;
        myTempPal[i]->edHighBase->Enabled        =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edHighBase->Enabled;
        myTempPal[i]->edOffset->Enabled          =(AccessLevel<LevelSet.AccessLevel[53])?false:myTempPal[i]->edOffset->Enabled;
        myTempPal[i]->edSingleLimit->Enabled     =(AccessLevel<LevelSet.AccessLevel[54])?false:myTempPal[i]->edSingleLimit->Enabled;
        myTempPal[i]->edInitTempOffset->Enabled  =(AccessLevel<LevelSet.AccessLevel[106])?false:myTempPal[i]->edInitTempOffset->Enabled;
        myTempPal[i]->edEOTTempOffset->Enabled   =(AccessLevel<LevelSet.AccessLevel[124])?false:myTempPal[i]->edEOTTempOffset->Enabled;
        myTempPal[i]->edSHighBase->Enabled       =(AccessLevel<LevelSet.AccessLevel[51])?false:myTempPal[i]->edSHighBase->Enabled;                              //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    //Steven 20120804 End: 在最下面決定權限能不能Enable

    //jou 2012-09-17 SPIL要修改常溫關閉chamber風扇,必須使用hontech權限
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        grpATCTempOffset->Enabled=false;                                        //Ifor 20160518 客戶要求ATC 溫度 Offset 不可修改
        chkAmbUsingAFan->Enabled=false;                                         //Ifor 20160411 客戶要求反灰不可修改
    }

    if(CUSTOMER_CODE==CC_AMKOR_China ||                                         //jou 2013-04-24 客戶說常溫應該要disable
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 add QUALCOMM
    {
        bool bTempFlag=true;
        if(Temperature.iMachineTempMode==1)
            bTempFlag=false;
        else
            bTempFlag=true;

        for(int i=tcHotPlate1; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Enabled            =bTempFlag;                 //Steven 20200717 : 補上前兩點的權限控制
            myTempPal[i]->edMid->Enabled            =bTempFlag;                 //Steven 20200717 : 補上前兩點的權限控制
            myTempPal[i]->edLowbase->Enabled        =bTempFlag;
            myTempPal[i]->edBase->Enabled           =bTempFlag;
            myTempPal[i]->edHighBase->Enabled       =bTempFlag;
            myTempPal[i]->edOffset->Enabled         =bTempFlag;
            myTempPal[i]->edSingleLimit->Enabled    =bTempFlag;
            myTempPal[i]->edSHighBase->Enabled      =bTempFlag;                 //Ztex 2024.07.27 Add 6 Point Temperature Offset
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX)             //20140320 wei   KYEC 低於權限顯示不能修改
    {
        if(AccessLevel<LevelSet.AccessLevel[17])
        {
            rgTemperatureMode   ->Enabled=false;
            gbHotModeSetting    ->Enabled=false;
            gbAmbientModeSetting->Enabled=false;
            rgIndexHeatMode     ->Enabled=false;
            pnlHeatMode         ->Enabled=false;
            gbTempOffset        ->Enabled=false;
        }
        else
        {
            rgTemperatureMode   ->Enabled=true;
            gbHotModeSetting    ->Enabled=true;
            gbAmbientModeSetting->Enabled=true;
            rgIndexHeatMode     ->Enabled=true;
            pnlHeatMode         ->Enabled=true;
            gbTempOffset        ->Enabled=true;
        }
    }

    chkTesterDock->Visible=IniConfig.bA05UseAutoDocking;
    edtTestDock->Visible  =IniConfig.bA05UseAutoDocking;

    if(CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20140714 only for SCK add CUSTOMER_CODE==CC_SCK
    {
        chkTesterDock->Visible=true;
        edtTestDock  ->Visible=true;
        chkTesterDock->Enabled=false;
    }

    if(IniConfig.bTemp25degControl==true)                                       //jou 2014-06-07 Temperature 25 deg. control
    {
        if((TestIF.iTestMode==_8Site2X4 || LastSet.iTemperature==Tempture_Hot) &&
            Temperature.iIndexHeatMode==HeadOnly && Temperature.fSoakTime==0 && Temperature.fWorkTemperBase<=25.0)
        {
            if(IndexHasIC() || ShuttleHasIC() || InArmSuck.HasIC() ||
               MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())
            {
                edSoakTime->Enabled=false;
                edWorkTemp->Enabled=false;
            }
            else
            {
                edSoakTime->Enabled=true;
                edWorkTemp->Enabled=true;
            }
        }
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[0])
        {
            edWorkTemp->Enabled=false;
        }

        if(bAuthCriticalPara[1])
        {
            edSoakTime->Enabled=false;
        }
    }

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    lblRT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    lblFT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_1_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_2_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_3_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_4_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_5_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_6_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_7_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_8_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_9_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    edtInitialDelay_10_RT->Visible=(IniConfig.bI13InitStartDelayHasFTandRT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    pnlArm1Offset->Visible    =false;                                           //wei 20151230
    pnlArm1Offset->Enabled    =fSecurity->Insufficient(17,false);               //wei 20151230
    pnlArm2Offset->Visible    =false;                                           //wei 20151230
    pnlArm2Offset->Enabled    =fSecurity->Insufficient(17,false);               //wei 20151230

    gbNoFullsiteOffset->Visible =(CUSTOMER_CODE==CC_TSMC_TAINAN && IniConfig.bL18NofullsiteaddTemperatureoffset);       //wei 20160107 No FullSite Add Offset

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160101 海思專用版本 顯示不可修改溫度
    {
        rgTemperatureMode->Enabled      =false;
        tsHot->Enabled                  =false;
        tsAmbient->Enabled              =false;
        tsAmbCtr->Enabled               =false;                                 //JerryYang 20181002 (Steven) : fix頁面完全鎖定住
        tsInitOffset->Enabled           =false;
        tsBoostFunc->Enabled            =false;
        gbBasePoint->Enabled            =false;
        rgIndexHeatMode->Enabled        =false;
        cbHeaterGun->Enabled              =false;                               //kevin 20200513 HSIS 可以改OFFSET
        rbATCActiveOn->Enabled          =false;
        palATC->Enabled                 =false;
        tsATC70->Enabled                =false;
        tsATC_PID->Enabled              =false;
        pnlArm1Offset->Enabled          =false;
        pnlArm2Offset->Enabled          =false;
        edATCAmbTemp->Enabled           =false;
        edTempAlwaysSameAlarm->Enabled  =false;
        cbbATC_RecipeFile->Enabled      =false;
        edChillerTemp->Enabled          =false;
        grpATCTempOffset->Enabled       =false;
        grpInPC->Enabled                =false;
    }
    else if(bEnablePEModel==true)                                               //Ifor 20160824 避免PE工程模式無法開啟
    {
        pnlArm1Offset->Enabled          =true;
        pnlArm2Offset->Enabled          =true;
        rgTemperatureMode->Enabled      =true;
        tsHot->Enabled                  =true;
        tsAmbient->Enabled              =true;
        tsOther->Enabled                =true;                                  //Ifor 20160825 Temp_Set Other選擇頁面
        gbBasePoint->Enabled            =true;                                  //Ifor 20160825 Base Point選擇頁面
        rgIndexHeatMode->Enabled        =true;                                  //Ifor 20160825 Heat Mode 選擇頁面
        tsHeatGun->Enabled              =true;                                  //Ifor 20160825 Heat Gun 頁面控制
        rbATCActiveOn->Enabled          =true;                                  //Ifor 20160825 ATC Active ATC Cooling 選項控制
        palATC->Enabled                 =true;                                  //Ifor 20160825 ATC 下方控制
        tsATC70->Enabled                =true;                                  //Ifor 20160825 ATC 7.0 頁面控制
        grpATCTempOffset->Enabled       =true;                                  //Ifor 20160825 ATC Temp Offset
        grpInPC->Enabled                =true;                                  //Ifor 20160825 Temp offset
        tsATC_PID->Enabled              =true;                                  //Ifor 20160825 ATC_PID
        edATCAmbTemp->Enabled           =true;
        edTempAlwaysSameAlarm->Enabled  =true;
        cbbATC_RecipeFile->Enabled      =true;
        edChillerTemp->Enabled          =true;
    }

    if(CosFunction.bHiSiliconFunction==false && ATC_SYSTEM==eNewATCSystem)
    {
        if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                                     //kevin 20181219 (Steven) :  add write ATC FILE NAME
            edAtcFileName->Visible=false;
        cbbATC_RecipeFile->Visible=true;
    }

    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)                          //Ifor 20160420 ATC Temp Offset設定
    {
        grpInPC->Visible=true;
    }
    else if(ATC_SYSTEM==eNewATCSystem)
    {
        if(CosFunction.bUseOldATCTempOffset==true)                              //JerryYang 20190123 ATC Offset UI使用舊版
        {
            grpATCTempOffset->Visible=true;
        }
        else
        {
            grpATCTempOffset->Visible=CosFunction.bUseSecondATCTempOffset;      //Ifor 20241118 : 測試中變溫
        }
    }
    else if(ATC_SYSTEM==eATCHonPrecType)
    {
        grpInPC->Visible=true;
        gbChillerTemp->Enabled=false;                                           //Ifor 20161103 ATC2.0 Chiller 溫度固定20度不可修改
    }

    if((CUSTOMER_CODE==CC_KYEC_LEE && bUseATC_SelfTestFunction==true) ||        //Alick 20170117 add 京元要求一般版也要有//Ifor 20170124 : add KYEC 要求由General.ini開關使用ATC Self Test功能
        CosFunction.bHiSiliconFunction==true)                                   //Ifor 20160907 Modify ATC Slef Test 按鍵 僅顯示於海思版本
        sbSafeTestATC->Visible=true;
    else
        sbSafeTestATC->Visible=false;

    //Ifor 20170426 整合UpDateEdit資料至FormShow 處理
    //==>
    if(IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false)   //Sam 20231214 : Temp offset use ready temp range
    {
        labReadyTempRange->Visible=true;
        edTempReadyRange->Visible=true;
    }
    else
    {
        labReadyTempRange->Visible=false;
        edTempReadyRange->Visible=false;
    }

    gb_TjFunction->Visible  =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);
    gbATC70->Visible        =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70
    gbTSD->Visible          =(ATC_SYSTEM==eATCHonPrecType || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70);           //Steven 20151123 : move to tsATC70
    if(CosFunction.bHiSiliconFunction==true || TestIF_File.i2DIDFormat==eAMD || CosFunction.bATCUseHandlerRecipeName==true)                                     //Ifor 20191126 : add ATC Use Handler Recipe Name  //JerryYang 20200422 2DID format選項改用下拉選單
    {
        gbFileName->Visible=false;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //KEVIN 20181121 (Steven) : atc file name
        {
            gbFileName->Visible =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);               //Steven 20150810 : add for ATC //Ifor 20160111 Add For New Atc System
            edAtcFileName->Enabled=false;
            cbbATC_RecipeFile->Enabled=false;
        }
    }
    else
    {
        gbFileName->Visible =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eNewATCSystem);                   //Steven 20150810 : add for ATC //Ifor 20160111 Add For New Atc System
    }

    gbChillerTemp->Visible  =(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30 || ATC_SYSTEM==eATCHonPrecType || ATC_SYSTEM==eNewATCSystem);                            //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0

    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70)
        gbChillerTemp->Visible=false;

    if(CosFunction.bUseHandlerSetChillerTemp==true)                             //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    {
        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 || ATC_SYSTEM==eATCHonPrecType)
        {
            gbChillerTemp->Visible=false;
        }
        else
        {
            gbChillerTemp->Visible=true;
        }
    }
    else
    {
        gbChillerTemp->Visible=false;
    }

    RachoiceATCFile->Checked=true;                                              //kevin 20181219 (Steven) :  選ATC工作檔還是輸入
    gbATCPackageOffset->Visible=(CosFunction.bATCUsePackageOffset);             //Ifor 20190215 : add ATC 使用 Package Offset 功能
    grpTJTempRange->Visible=(CosFunction.bUseATCTJControlMode || Temperature.bEnableTJFunction);                        //Ifor 20190328 : add TJ Temp Over Range  //JerryYang 20251124 : Tj control吃不同的溫度range
    if(CosFunction.bATC32UseTJMode==true)
    {
        cbATCReferTempSensor->Caption="Display TJ Temperature";
    }
    //<==
    //Ifor 20170426 整合UpDateEdit資料至FormShow 處理
    //Ifor 20170905 (wei) add HT9045AT 不顯示Heat頁面
    //==>
     //kevin 20181016 add
    if(CosFunction.bUseOldATCTempOffset==true)                                  //JerryYang 20190123 ATC Offset UI使用舊版
    {
        if(ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16)                // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180817 : Boost Function
        {
            tsArm1      ->TabVisible=false;
            tsArm2      ->TabVisible=false;
        }
    }
    else
    {
        grpATCTempOffset->Visible=CosFunction.bUseSecondATCTempOffset;          //Ifor 20241014 Use Second ATC Temp Offset
    }
    //<==
    //Ifor 20170905 (wei) add HT9045AT 不顯示Heat頁面
    if(CUSTOMER_CODE==CC_ASE_M && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_32)
    {
        gbTSD->Visible                  =true;
        gbATC70->Visible                =false;
        gbATCInitialOffset->Visible     =false;
        gbATCConsFail->Visible          =false;
        gbATCQAModeOffset->Visible      =false;
        gbATCTestTimeOffset->Visible    =false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        PnlIndexSoakTime->Visible=false;                                        //kevin 20210511 ASE KH  JL 說關閉
    }

    gbHeatGunTempATC->Visible=(INSTALL_ATC_HEAT_GUN>0);                         //JerryYang 20220408 : add for ATC3.5
    gb_HotGunFlow->Visible=HotGunFlowEnable;                                    //KaiChen 20190729 ：Hot Gun Flow

    chkTempCalByRecipe->Visible=CosFunction.bTempCalByRecipe;                   //jou 20220725 : Temperature calibration by recipe

    cbByPassChamber->Visible=(CUSTOMER_CODE==CC_HONPREC_QC);                    //KenHsieh 20230301 : By Pass Chamber
    sgTjMap->Cells[0][0]="Define";                                              //JerryYang 20231128 : 傳送ATC site map
    sgTjMap->Cells[0][1]="TC";
    for(int i=0; i<16; i++)
    {
        S.sprintf("TJ%02d", i+1);
        sgTjMap->Cells[i+1][0]=S;
    }

    gbPowerFollowFuncAct->Visible=IniConfig.bL43EnableATCPowerFollow;           //Hmy 20240207 Add Send Pf Par to Atc

    tsATCMultiSensor->TabVisible=(ATC_SYSTEM==eNewATCSystem &&
                                 (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                                  ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36) );
    //這一行請保持在最下面!!-----------------
}
//---------------------------------------------------------------------------
// golden :1323-1974 (652 lines) -- pure Caption assignment per test-mode
// switch; no dependency gaps (myTempPal[] already ported, PT-W3). Transcribed
// verbatim.
//---------------------------------------------------------------------------
void TfTemp_Set::SetTempPanelCaption()                                          //Steven 20250716 : Rework for ATC temp offset
{
    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08 )                                       //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {                                                                           //Steven 20140620 : Jovie說畫面要有溫控器對應位置
        myTempPal[tcAa1]->SetCaption("A1");
        myTempPal[tcBa1]->SetCaption("B1");
        myTempPal[tcAb1]->SetCaption("C1");
        myTempPal[tcBb1]->SetCaption("D1");
        myTempPal[tcAc1]->SetCaption("E1");
        myTempPal[tcBc1]->SetCaption("F1");
        myTempPal[tcAd1]->SetCaption("G1");
        myTempPal[tcBd1]->SetCaption("H1");
        myTempPal[tcAe1]->SetCaption("I1");
        myTempPal[tcBe1]->SetCaption("J1");
        myTempPal[tcAf1]->SetCaption("K1");
        myTempPal[tcBf1]->SetCaption("L1");
        myTempPal[tcAg1]->SetCaption("M1");
        myTempPal[tcBg1]->SetCaption("N1");
        myTempPal[tcAh1]->SetCaption("O1");
        myTempPal[tcBh1]->SetCaption("P1");

        myTempPal[tcAa2]->SetCaption("A2");
        myTempPal[tcBa2]->SetCaption("B2");
        myTempPal[tcAb2]->SetCaption("C2");
        myTempPal[tcBb2]->SetCaption("D2");
        myTempPal[tcAc2]->SetCaption("E2");
        myTempPal[tcBc2]->SetCaption("F2");
        myTempPal[tcAd2]->SetCaption("G2");
        myTempPal[tcBd2]->SetCaption("H2");
        myTempPal[tcAe2]->SetCaption("I2");
        myTempPal[tcBe2]->SetCaption("J2");
        myTempPal[tcAf2]->SetCaption("K2");
        myTempPal[tcBf2]->SetCaption("L2");
        myTempPal[tcAg2]->SetCaption("M2");
        myTempPal[tcBg2]->SetCaption("N2");
        myTempPal[tcAh2]->SetCaption("O2");
        myTempPal[tcBh2]->SetCaption("P2");

        if(CosFunction.bTempSetDisenableNotUseHeater)
        {
            for(int i=tcAa1; i<tcBd2+1; i++)
            {
                myTempPal[i]->SetEnable(bUT150Install[i]);
            }
            for(int i=tcAe1; i<tcBh2+1; i++)
            {
                myTempPal[i]->SetEnable(bUT150Install[i]);
            }
        }

        switch(TestIF.iTestMode)
        {
            case SingleSite:
                if(ATC_SYSTEM>=eATC60 && Temperature.bATCActiveCooling==true)   //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    if(Temperature.bMultiZoneEnable==true)                      //Ifor 20240821 add:ATC多區溫控模式Offset補償
                    {
                        if(Temperature.bZoneTempEnable[1]==true)
                        {
                            myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                            myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                        }

                        if(Temperature.bZoneTempEnable[2]==true)
                        {
                            myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                            myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                        }

                        if(Temperature.bZoneTempEnable[3]==true)
                        {
                            myTempPal[tcAd1]->SetCaption("G1 (Ad1)");
                            myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                        }
                    }
                }
                else
                {
                    if(TestIF_File.b2CableLayoutKit)                            //JerryYang 20160826 single site兩條線版本(使用一支加熱棒)
                    {
                        myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                        myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    }
                    else
                    {
                        myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                        myTempPal[tcBa1]->SetCaption("B1 (Aa1)");
                        myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                        myTempPal[tcBa2]->SetCaption("B2 (Aa2)");
                    }
                }
                break;
            case DualSite:                                                      //1x2
                if(TestIF_File.bUse1x3SiteKit==true &&                          //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                   Temperature.bATCActiveCooling==true)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                }
                else if((ATC_SYSTEM==eNewATCSystem ||                           //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ab1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab2)");
                }
                else if(TestIF_File.b2CableLayoutKit)                           //JerryYang 20160826 1x2 兩條線版本使用兩支加熱棒
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //jou 2015-10-15  : 16溫控器 1條線版本
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ab1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab2)");
                }
                else if(TestIF_File.bUse1x3SiteKit==true)                       //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Aa1)");
                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Aa2)");

                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Ab1)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Ab2)");
                }
                break;
            case TriSite1X3:
                myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                myTempPal[tcAc1]->SetCaption("E1 (Ac1)");

                myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                break;
            case QualSite1X4:                                                   //1x4
            case _8Site1X4:                                                     //ChungHung 20150528 add for 海思 _8Site1x4
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 修正 ATC 1*4 溫度顯示異常問題
                   Temperature.bATCActiveCooling==true)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                }
                else if(TestIF_File.b2CableLayoutKit)                           //Steven 20150724 : 16溫控器 2條線版本
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                }
                else if(TestIF_File.bNS7000kit==true ||
                        TestIF_File.b1CableLayoutKit==true)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ab1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ac1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Ad1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Ad2)");
                }
                else                                                            //Steven 20250821 : 1x4 三條線版本
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ab1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ac1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Ad1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Ad2)");
                }
                break;
            case DualSite2x1:
                myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                myTempPal[tcBa1]->SetCaption("B1 (Aa1)");
                myTempPal[tcAa2]->SetCaption("A2 (Aa2)");                           //Ifor 20191015 : A1 => A2
                myTempPal[tcBa2]->SetCaption("B2 (Aa2)");                           //Ifor 20191015 : B1 => B2

                myTempPal[tcAb1]->SetCaption("C1 (Ba1)");
                myTempPal[tcBb1]->SetCaption("D1 (Ba1)");
                myTempPal[tcAb2]->SetCaption("C2 (Ba2)");                           //Ifor 20191015 : C1 => C2
                myTempPal[tcBb2]->SetCaption("D2 (Ba2)");                           //Ifor 20191015 : D1 => D2
                break;
            case QualSite2X2:                                                   //2x2
                if(TestIF_File.bSquare_OctalKit ||                              //Steven 20141224 : 2x2Site使用8Site Kit
                   TestIF_File.b2x2Use16SiteKit)                                //Steven 20191113 : 2x2Site使用16Site Kit
                {
                    myTempPal[tcAb1]->SetCaption("C1 (Aa1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Ba1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ab1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bb1)");

                    myTempPal[tcAa2]->SetCaption("C2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("D2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("E2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("F2 (Bb2)");
                }
                else if(CUSTOMER_CODE==CC_ATEC && TestIF_File.b2CableLayoutKit && DeviceForm_File.iHeadDeviceCT==3)     //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1, Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1, Bb1)");
                    myTempPal[tcAa2]->SetCaption("A2 (Aa2, Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2, Bb2)");
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //JerryYang 20181207 2x2新增一條線版本
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1, Ba1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ab1, Bb1)");
                    myTempPal[tcAa2]->SetCaption("A2 (Aa2, Ba2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab2, Bb2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                }
                break;
            case QualSite2X2N:                                                  //Frank 20200520 2X2NN Mode
                if(TestIF_File.bUse1x3SiteKit==true)                            //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Ba)");
                    myTempPal[tcBa1]->SetCaption("B1 (Bb)");
                    myTempPal[tcAa2]->SetCaption("A2 (Aa)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab)");
                }
                break;
            case _6Site2X3:                                                     //ChungHung 20140115 add for 2x3_6
                myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                myTempPal[tcBc1]->SetCaption("F1 (Bc1)");

                myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                myTempPal[tcBc2]->SetCaption("F2 (Bc2)");
                break;
            case _6Site2X3N:                                                    //Steven 20220425 : 2X3NN Mode
                myTempPal[tcAa1]->SetCaption("A1 (Ba)");
                myTempPal[tcAb1]->SetCaption("C1 (Bb)");
                myTempPal[tcAc1]->SetCaption("E1 (Bc)");

                myTempPal[tcAa2]->SetCaption("A2 (Aa)");
                myTempPal[tcAb2]->SetCaption("C2 (Ab)");
                myTempPal[tcAc2]->SetCaption("E2 (Ac)");
                break;
            case _8Site2X4:                                                     //2x4
                if(TestIF_File.bOctal_12Kit==true)                              //JerryYang 20230204 : 修正8 site mode使用12site SLK溫度offset異常
                {
                    myTempPal[tcAb1]->SetCaption("C1 (Aa1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Ba1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ab1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bb1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ac1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bc1)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ad1)");
                    myTempPal[tcBe1]->SetCaption("J1 (Bd1)");

                    myTempPal[tcAb2]->SetCaption("C2 (Aa2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Ba2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ab2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bb2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ac2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bc2)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ad2)");
                    myTempPal[tcBe2]->SetCaption("J2 (Bd2)");
                }
                else if(TestIF_File.bOctal_16Kit==true)
                {
                    myTempPal[tcAc1]->SetCaption("E1 (Aa1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Ba1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ab1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bb1)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ac1)");
                    myTempPal[tcBe1]->SetCaption("J1 (Bc1)");
                    myTempPal[tcAf1]->SetCaption("K1 (Ad1)");
                    myTempPal[tcBf1]->SetCaption("L1 (Bd1)");

                    myTempPal[tcAc2]->SetCaption("E2 (Aa2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Ba2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ab2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bb2)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ac2)");
                    myTempPal[tcBe2]->SetCaption("J2 (Bc2)");
                    myTempPal[tcAf2]->SetCaption("K2 (Ad2)");
                    myTempPal[tcBf2]->SetCaption("L2 (Bd2)");
                }
                else if(TestIF_File.bNS8000CS==true)                            //Steven 20250826 : fixed for NS8000cs display
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1, Ab1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1, Bb1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ac1, Ad1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bc1, Bd1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2, Ab2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2, Bb2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac2, Ad2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bc2, Bd2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bc1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bd1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bd2)");
                }
                break;
            case _8Site2X4N:                                                    //Wei 20231211 : 2X4NN Mode
                if(TestIF_File.bNS7000kit==true ||
                   TestIF_File.b2CableLayoutKit==true ||                        //Steven 20250613 : 修正顯示 for 1x4 3 cable SLK
                   TestIF_File.b1CableLayoutKit==true)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Ba)");
                    myTempPal[tcBa1]->SetCaption("B1 (Bb)");
                    myTempPal[tcAb1]->SetCaption("C1 (Bc)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bd)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ab)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac)");
                    myTempPal[tcBb2]->SetCaption("D2 (Ad)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Ba)");
                    myTempPal[tcAb1]->SetCaption("C1 (Bb)");
                    myTempPal[tcAc1]->SetCaption("E1 (Bc)");
                    myTempPal[tcAd1]->SetCaption("G1 (Bd)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad)");
                }
                break;
            case _10Site2X5:                                                    //wei 20190614 10 site
                if(TestIF_File.bUse32Heater || TestIF_File.b12SiteUse10Heater)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bc1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bd1)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ae1)");
                    myTempPal[tcBe1]->SetCaption("J1 (Be1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bd2)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ae2)");
                    myTempPal[tcBe2]->SetCaption("J2 (Be2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1, Ad1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bc1, Bd1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ae1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Be1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2, Ad2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc2, Bd2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ae2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Be2)");
                }
                break;
            case _12Site2X6:
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bc1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bd1)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ae1)");
                    myTempPal[tcBe1]->SetCaption("J1 (Be1)");
                    myTempPal[tcAf1]->SetCaption("K1 (Af1)");
                    myTempPal[tcBf1]->SetCaption("L1 (Bf1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bd2)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ae2)");
                    myTempPal[tcBe2]->SetCaption("J2 (Be2)");
                    myTempPal[tcAf2]->SetCaption("K2 (Af2)");
                    myTempPal[tcBf2]->SetCaption("L2 (Bf2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1, Ab1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1, Bb1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ac1, Ad1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bc1, Bd1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ae1, Af1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Be1, Bf1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2, Ab2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2, Bb2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac2, Ad2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bc2, Bd2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ae2, Af2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Be2, Bf2)");
                }
                break;
            case _16Site2X8:                                                    //2x8
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ab1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bb1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ac1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Bc1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ad1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bd1)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ae1)");
                    myTempPal[tcBe1]->SetCaption("J1 (Be1)");
                    myTempPal[tcAf1]->SetCaption("K1 (Af1)");
                    myTempPal[tcBf1]->SetCaption("L1 (Bf1)");
                    myTempPal[tcAg1]->SetCaption("M1 (Ag1)");
                    myTempPal[tcBg1]->SetCaption("N1 (Bg1)");
                    myTempPal[tcAh1]->SetCaption("O1 (Ah1)");
                    myTempPal[tcBh1]->SetCaption("P1 (Bh1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bd2)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ae2)");
                    myTempPal[tcBe2]->SetCaption("J2 (Be2)");
                    myTempPal[tcAf2]->SetCaption("K2 (Af2)");
                    myTempPal[tcBf2]->SetCaption("L2 (Bf2)");
                    myTempPal[tcAg2]->SetCaption("M2 (Ag2)");
                    myTempPal[tcBg2]->SetCaption("N2 (Bg2)");
                    myTempPal[tcAh2]->SetCaption("O2 (Ah2)");
                    myTempPal[tcBh2]->SetCaption("P2 (Bh2)");
                }
                else
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Aa1, Ab1)");
                    myTempPal[tcBa1]->SetCaption("B1 (Ba1, Bb1)");
                    myTempPal[tcAb1]->SetCaption("C1 (Ac1, Ad1)");
                    myTempPal[tcBb1]->SetCaption("D1 (Bc1, Bd1)");
                    myTempPal[tcAc1]->SetCaption("E1 (Ae1, Af1)");
                    myTempPal[tcBc1]->SetCaption("F1 (Be1, Bf1)");
                    myTempPal[tcAd1]->SetCaption("G1 (Ag1, Ah1)");
                    myTempPal[tcBd1]->SetCaption("H1 (Bg1, Bh1)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa2, Ab2)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba2, Bb2)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ac2, Ad2)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bc2, Bd2)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ae2, Af2)");
                    myTempPal[tcBc2]->SetCaption("F2 (Be2, Bf2)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ag2, Ah2)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bg2, Bh2)");
                }
                break;
            case _32Site4X8N:
                if(TestIF_File.bUse32Heater)
                {
                    myTempPal[tcAa1]->SetCaption("A1 (Ca)");
                    myTempPal[tcBa1]->SetCaption("B1 (Da)");
                    myTempPal[tcAb1]->SetCaption("C1 (Cb)");
                    myTempPal[tcBb1]->SetCaption("D1 (Db)");
                    myTempPal[tcAc1]->SetCaption("E1 (Cc)");
                    myTempPal[tcBc1]->SetCaption("F1 (Dc)");
                    myTempPal[tcAd1]->SetCaption("G1 (Cd)");
                    myTempPal[tcBd1]->SetCaption("H1 (Dd)");
                    myTempPal[tcAe1]->SetCaption("I1 (Ce)");
                    myTempPal[tcBe1]->SetCaption("J1 (De)");
                    myTempPal[tcAf1]->SetCaption("K1 (Cf)");
                    myTempPal[tcBf1]->SetCaption("L1 (Df)");
                    myTempPal[tcAg1]->SetCaption("M1 (Cg)");
                    myTempPal[tcBg1]->SetCaption("N1 (Dg)");
                    myTempPal[tcAh1]->SetCaption("O1 (Ch)");
                    myTempPal[tcBh1]->SetCaption("P1 (Dh)");

                    myTempPal[tcAa2]->SetCaption("A2 (Aa)");
                    myTempPal[tcBa2]->SetCaption("B2 (Ba)");
                    myTempPal[tcAb2]->SetCaption("C2 (Ab)");
                    myTempPal[tcBb2]->SetCaption("D2 (Bb)");
                    myTempPal[tcAc2]->SetCaption("E2 (Ac)");
                    myTempPal[tcBc2]->SetCaption("F2 (Bc)");
                    myTempPal[tcAd2]->SetCaption("G2 (Ad)");
                    myTempPal[tcBd2]->SetCaption("H2 (Bd)");
                    myTempPal[tcAe2]->SetCaption("I2 (Ae)");
                    myTempPal[tcBe2]->SetCaption("J2 (Be)");
                    myTempPal[tcAf2]->SetCaption("K2 (Af)");
                    myTempPal[tcBf2]->SetCaption("L2 (Bf)");
                    myTempPal[tcAg2]->SetCaption("M2 (Ag)");
                    myTempPal[tcBg2]->SetCaption("N2 (Bg)");
                    myTempPal[tcAh2]->SetCaption("O2 (Ah)");
                    myTempPal[tcBh2]->SetCaption("P2 (Bh)");
                }
                else
                {
                    if(TestIF_File.dSiteYPitch<50.0)
                    {
                        myTempPal[tcAa1]->SetCaption("A1 (Ca, Da)");
                        myTempPal[tcBa1]->SetCaption("B1 (Cb, Db)");
                        myTempPal[tcAb1]->SetCaption("C1 (Cc, Dc)");
                        myTempPal[tcBb1]->SetCaption("D1 (Cd, Dd)");
                        myTempPal[tcAc1]->SetCaption("E1 (Ce, De)");
                        myTempPal[tcBc1]->SetCaption("F1 (Cf, Df)");
                        myTempPal[tcAd1]->SetCaption("G1 (Cg, Dg)");
                        myTempPal[tcBd1]->SetCaption("H1 (Ch, Dh)");

                        myTempPal[tcAa2]->SetCaption("A2 (Aa, Ba)");
                        myTempPal[tcBa2]->SetCaption("B2 (Ab, Bb)");
                        myTempPal[tcAb2]->SetCaption("C2 (Ac, Bc)");
                        myTempPal[tcBb2]->SetCaption("D2 (Ad, Bd)");
                        myTempPal[tcAc2]->SetCaption("E2 (Ae, Be)");
                        myTempPal[tcBc2]->SetCaption("F2 (Af, Bf)");
                        myTempPal[tcAd2]->SetCaption("G2 (Ag, Bg)");
                        myTempPal[tcBd2]->SetCaption("H2 (Ah, Bh)");
                    }
                    else
                    {
                        myTempPal[tcAa1]->SetCaption("A1 (Ca, Cb)");
                        myTempPal[tcBa1]->SetCaption("B1 (Da, Db)");
                        myTempPal[tcAb1]->SetCaption("C1 (Cc, Cd)");
                        myTempPal[tcBb1]->SetCaption("D1 (Dc, Dd)");
                        myTempPal[tcAc1]->SetCaption("E1 (Ce, Cf)");
                        myTempPal[tcBc1]->SetCaption("F1 (De, Df)");
                        myTempPal[tcAd1]->SetCaption("G1 (Cg, Ch)");
                        myTempPal[tcBd1]->SetCaption("H1 (Dg, Dh)");

                        myTempPal[tcAa2]->SetCaption("A2 (Aa, Ab)");
                        myTempPal[tcBa2]->SetCaption("B2 (Ba, Bb)");
                        myTempPal[tcAb2]->SetCaption("C2 (Ac, Ad)");
                        myTempPal[tcBb2]->SetCaption("D2 (Bc, Bd)");
                        myTempPal[tcAc2]->SetCaption("E2 (Ae, Af)");
                        myTempPal[tcBc2]->SetCaption("F2 (Be, Bf)");
                        myTempPal[tcAd2]->SetCaption("G2 (Ag, Ah)");
                        myTempPal[tcBd2]->SetCaption("H2 (Bg, Bh)");
                    }
                }
                break;
            case _32Site4X8M:
                break;
        }
    }
}

//---------------------------------------------------------------------------
// golden :1976-3143 (1,168 lines). Two internal gates: (1) `fMain->
// palIndivisual` -- DEPENDENCY gate, member absent from the current fMain
// facade (grepped `palIndivisual` across forms/*.h, 20260820, 0 hits;
// fMain.h is outside this wave's 3-file write boundary); (2)
// `rgIndexHeatMode->Controls[i]->Visible` -- DEPENDENCY gate,
// vclcompat::TRadioGroup carries no Controls[] child-widget accessor. The
// `fDynamicTemp` NULL-check block is translated NORMALLY (that facade
// exists, forms/fDynamicTemp.h, this wave's sibling).
//---------------------------------------------------------------------------
void TfTemp_Set::ReadTempFile(bool bUpdateAll)
{
    int i;
    unsigned int iAtc;
    bool bReadType=false;
    AnsiString S="", str="";
    S=GetLastOpenFN();
    AnsiString szDir="", szDir2="";
    AnsiString str1, sTemp="";                                                  //kevin 20151023

    if(CosFunction.bSaveTemperatureByMachine &&
       IniConfig.bA57_2SaveTemperatureByMachine)                                //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s\\", DataPath, S);
    }
    MyForceDirectories(szDir);
    szDir+="Temperature.Data";

    double  bufferfWorkTemperBase=0.0;                                          //kevin 20160531
    double  bufferffSoakTime=0.0;                                               //kevin 20160531
    int iTemperatureDataVer=0;                                                  //Sam 20210106 : TemperatureData 新增版號來相容

    if(FileExists(szDir)==false)                                                //kevin 20160531 add
    {
        iSendChangeTempError=1;
        return;
    }

    if(bUpdateAll==true)
    {
        if(CosFunction.bUseIndividulTempSet==false ||                           //Steven 20181102 : For QTI全球統一Offset
           (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60))
            Temperature.bUseIndividualTemp=false;
        else
            Temperature.bUseIndividualTemp=ReadIniData(szDir, "Mode",    "UseIndividualTemp",         false);           //Steven 20140924 : 各個加熱區獨立有自己的設定值

        if(CosFunction.bUseInitialDelayAsSoakTime)                              //Steven 20170329 (wei) : 使用initial delay當 Soak time
            Temperature.bUseInitialDelayAsSoakTime=ReadIniData(szDir, "Mode", "bUseInitialDelayAsSoakTime", false);
        else
            Temperature.bUseInitialDelayAsSoakTime=false;

        if(CosFunction.bTempAlarmBinNeedToError)                                //Steven 20251022 : Temp alarm need put to error bin
            Temperature.bTempAlarmBinNeedToError  =ReadIniData(szDir, "Mode",       "bTempAlarmBinNeedToError",  false);
        else
            Temperature.bTempAlarmBinNeedToError  =false;

        // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- golden
        // `fMain->palIndivisual->Visible=...`/`->Visible` read. `palIndivisual`
        // (golden TPanel*) has no member on the current fMain facade (grepped
        // 20260820, 0 hits); fMain.h is outside this wave's write boundary.
#if 0 // GATE(dep-fMain-palIndivisual)
        fMain->palIndivisual->Visible=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp);
        if(LastSet.iTemperature==Tempture_Hot ||
           LastSet.iTemperature==Tempture_AmbientHot)                           //kevin 20140918 恆溫控制
            fMain->edWorkTemperBase->Visible=!fMain->palIndivisual->Visible;
#endif // GATE(dep-fMain-palIndivisual)

        if(IniConfig.bOnlyRoomOrHot)                                            //Steven 20110914
            rgTemperatureMode->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 1);
        else if(IniConfig.bI03AmbientTempControl)                               //kevin 20140918 恆溫控制
            rgTemperatureMode->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 3);
        else
            rgTemperatureMode->ItemIndex=CheckRange(ReadIniData(szDir, "Mode",    "Mode",         0), 0, 2);

        Temperature.iMachineTempMode=ReadIniData(szDir, "Mode",    "Mode",         0);
        bufferfWorkTemperBase=Temperature.fWorkTemperBase ;                     //kevin 20160531

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20160614
            Temperature.fWorkTemperBase=ReadIniData(szDir, "Mode",    "Temperature",  10.0);
        else
            Temperature.fWorkTemperBase=ReadIniData(szDir, "Mode",    "Temperature",  0.0);

        if(Temperature.iMachineTempMode==0)
        {
            LastSet.iTemperature=Tempture_Hot;
        }
        else if(Temperature.iMachineTempMode==1)
        {
            LastSet.iTemperature=Tempture_Ambient;
        }
        else if(Temperature.iMachineTempMode==3)                                //Steven 20200421 : 補上Ambient Temp
        {
            LastSet.iTemperature=Tempture_AmbientHot;
        }
        else
        {
            if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)               //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
                LastSet.iTemperature=ReadIniData(szDir, "Mode",    "WorkTempMode",  LastSet.iTemperature);
        }

        if(fDynamicTemp!=NULL)
        {
            fDynamicTemp->edUpper->Text=AnsiString(Temperature.fWorkTemperBase+1);
            fDynamicTemp->edLower->Text=AnsiString(Temperature.fWorkTemperBase-1);
            fDynamicTemp->edMax  ->Text=AnsiString(Temperature.fWorkTemperBase+5);
            fDynamicTemp->edMin  ->Text=AnsiString(Temperature.fWorkTemperBase-5);
        }

        if(IniConfig.bVTESTFunction==false)                                     //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
            Temperature.bUseAbitCHK      =CheckAndReadIniData(szDir, "Mode",    "AmbCheck",     0.0);
        Temperature.iAbitCHKStatus   = 1;
        if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                    //Steven 20230410 : 南茂鍾永生要求修改
            Temperature.fAbitTemp    =CheckRange(CheckAndReadIniData(szDir, "Ambient", "Temperature",  23.0), 23.0, 30.0);                                      //Sam 20240424 : 修改南茂鍾永生要求修改 25 > 23
        else
            Temperature.fAbitTemp    =CheckRange(CheckAndReadIniData(szDir, "Ambient", "Temperature",  25.0), 50.0, 10.0);                                      //Jimmychiu 20220901 預設常溫為25

        if(LastSet.iTemperature==Tempture_Ambient &&
           IniConfig.bL20AbientGuardBand)                                       //kevin 20180115 (Steven) add Amient Guard Band
        {
        }
        else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                               //Sam 20240424 : 修改南茂鍾永生要求修改 25 > 23
        {
            if(Temperature.fAbitTemp<23 || Temperature.fAbitTemp>40)
                Temperature.fAbitTemp=0;                                        //強制設定為0度請人員確認
        }
        else if(Temperature.fAbitTemp<25 || Temperature.fAbitTemp>40)           //Ifor 20170605 (wei) add 常溫溫度讀取卡控避免人員修改檔案造成異常
        {
            Temperature.fAbitTemp=0;                                            //強制設定為0度請人員確認
        }

        if(DewPoint_Hardware_Install>0)                                         //Steven 20191017 : 露點計
        {
            Temperature.bWaitDewPoint         =ReadIniData(szDir, "Mode",    "WaitDewPoint",              false);       //Isaac 20180828 : wait dewpoint
            Temperature.dDewPointRange        =CheckRange(ReadIniData(szDir ,"Mode" ,"DewPointRange" ,10.0), 1.0, 30.0);                                        //Isaac 20181217 : 露點計報警範圍可設定
            Temperature.iDewPointAlarmInterval=CheckRange(ReadIniData(szDir ,"Mode" ,"DewPointAlarmInterval" ,10), 1, 30);                                      //Isaac 20181217 : 露點計報警範圍可設定
        }
        else
        {
            Temperature.bWaitDewPoint=false;
        }

        if(LB_TEMP>0)                                                           //JerryYang 20220126: non ATC也支援LB function
        {
            Temperature.bLBCoolingAirOn         =ReadIniData(szDir, "Mode",    "bLBCoolingAirOn",              false);  //Isaac 20180828 : wait dewpoint
            Temperature.dLBAirOnTemp            =CheckRange(ReadIniData(szDir ,"Mode" ,"dLBAirOnTemp" ,55.0), 40.0, 100.0);                                     //Isaac 20181217 : 露點計報警範圍可設定
            Temperature.iLBTempAlmInterval      =CheckRange(ReadIniData(szDir ,"Mode" ,"iLBTempAlmInterval" ,30), 30, 300);                                     //Isaac 20181217 : 露點計報警範圍可設定
        }
        else
        {
            Temperature.bLBCoolingAirOn=false;
        }

        Temperature.fAmbientHotGuartbent = CheckRange(ReadIniData(szDir, "Ambient", "fAmbientHotGuartbent",  3.0), 1.0, 30.0);                                  //JerryYang 20181001 (Steven) : 20->30   //kevin 20180906 add 10->20 //kevin 20141219 恆溫控制
        if(IniConfig.bI03AmbientTempControl && LastSet.iTemperature==Tempture_AmbientHot)
        {
            Temperature.bShuttleNoHeatUp=ReadIniData(szDir, "Ambient",    "bShuttleNoHeatUp", false);                   //Steven 20180815 : Amb Ctr mode, shuttle no heat up
            Temperature.bSLKNoHeatUp    =ReadIniData(szDir, "Ambient",    "bSLKNoHeatUp", false);;                      //Steven 20230221 : Amb Ctr mode, SLK no heat up
        }
        else
        {
            Temperature.bShuttleNoHeatUp=false;
            Temperature.bSLKNoHeatUp    =false;                                 //Steven 20230221 : Amb Ctr mode, SLK no heat up
        }

        bufferffSoakTime=Temperature.fSoakTime;                                 //kevin 20160531
        Temperature.fSoakTime        =CheckAndReadIniData(szDir, "Time",    "Soak",         999.0);                     //kevin 20160614 0.0->999
        Temperature.fJamSoakTime     =CheckAndReadIniData(szDir, "Time",    "Jam Soak",     0.0);
        Temperature.fInitialWaitTime =CheckAndReadIniData(szDir, "Time",    "H.Initial",    0.0);
        Temperature.fAbitInitWaitTime=CheckAndReadIniData(szDir, "Time",    "A.Initial",    0.0);
        Temperature.fAbitColdTime    =CheckAndReadIniData(szDir, "Time",    "Cool Time",    0.0);
        Temperature.iIndexHeatMode   =CheckAndReadIniData(szDir, "Index",   "Heating Mode", HeadOnly);                  //kevin 20160614 檔案寫入參數名稱

        if(CUSTOMER_CODE!=CC_HONPREC_QC)                                        //JerryYang 20160810 搬到讀取完iIndexHeatMode之後,避免Chamber加熱超過175度
        {
            dTempMax=MaxTempSetting();                                          //Steven 20170427 : 回傳機台可以用的最大溫度值
            dTempMin=MinTempSetting();
            if(bSendChangeTemp==false)                                          //kevin 20160531 起動auto temp offset 成功
                Temperature.fWorkTemperBase=CheckRange(Temperature.fWorkTemperBase, dTempMax, dTempMin);                //Steven 20110412
            if(ATC_SYSTEM==eNewATCSystem && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))
            {
                IniConfig.dATCAmbientTemperature=CheckRange(IniConfig.dATCAmbientTemperature, 30.0, -5.0);
            }
            else
            {
                IniConfig.dATCAmbientTemperature=CheckRange(IniConfig.dATCAmbientTemperature, 30.0, 25.0);              //wei 20151013  by Setup File ATC Ambient Temp set
            }
        }
        IniConfig.dHeatGunTempATC=CheckRange(IniConfig.dHeatGunTempATC, 70.0, 30.0);                                    //JerryYang 20220408 : add for ATC3.5

        //Ifor 20160411 客戶要求強制開啟
        if(IniConfig.bSPILFunction==true ||                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
           IniConfig.bAmbRunChamberFanCanStop==false)                           //Steven 20250806 : add protection
            Temperature.bAmbUsingAFan=true;
        else
            Temperature.bAmbUsingAFan=CheckAndReadIniData(szDir, "Ambient", "bAmbUsingAFan", true);                     //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動

        if(IniConfig.bL20AbientGuardBand)                                       //kevin 20180115 (Steven) add Amient Guard Band
        {
            Temperature.bAmbientGuardbandCheck=CheckAndReadIniData(szDir, "Ambient", "bAmbientGuardbandCheck", true);
            Temperature.iAmbGuardband=CheckAndReadIniData(szDir, "Ambient", "iAmbGuardband", 1);                        //JerryYang 20190520 false->1, fix AmbGuardband參數讀取異常問題
        }
        else
        {
            Temperature.bAmbientGuardbandCheck=false;
            Temperature.iAmbGuardband=0;
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE ||                                        //Ifor 20160318 add 客戶要求強制打開 常溫確認
           CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //KEVIN 20190529 Telix 要求有mail
        {
            Temperature.bUseAbitCHK=true;
        }

        // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE --
        // `rgIndexHeatMode->Controls[i]->Visible`. vclcompat::TRadioGroup
        // carries only ItemIndex/Items, no Controls[] child-widget accessor
        // (verified against vclcompat/Controls.h directly this wave).
#if 0 // GATE(dep-RadioGroup-Controls)
        if(Temperature.iIndexHeatMode<rgIndexHeatMode->Items->Count)            //Steven 20240206 : 根據Temperature.iIndexHeatMode顯示狀態決定是否要調整Index Heat Mode
        {
            if(rgIndexHeatMode->Controls[Temperature.iIndexHeatMode]->Visible==false)
                Temperature.iIndexHeatMode=HeadOnly;
        }
        else
        {
            Temperature.iIndexHeatMode=HeadOnly;
        }
#endif // GATE(dep-RadioGroup-Controls)

        //jou 981030 start
        Temperature.iInitialStart1Time=ReadIniData(szDir, "Time", "iInitialStart1Time", 0.0);
        Temperature.iInitialStart2Time=ReadIniData(szDir, "Time", "iInitialStart2Time", 0.0);
        //jou 981030 end
        Temperature.iIndexSoakTime=ReadIniData(szDir, "Time", "iIndexSoakTime", 0.0);                                   //2013-11-27   Dell    需要做Index soak time
        Temperature.iOSTime=ReadIniData(szDir, "Time", "iOSTime", 0.0);         //2013-11-27   Dell    需要做Index soak time
        if(CUSTOMER_CODE == CC_ASE_KaohSiung)
        {
            Temperature.iIndexSoakTime = 0;
            Temperature.iOSTime = 0;
        }                                                                       //kevin 20210511 ASE KH  JL 說關閉
        Temperature.iShuttleSoakTimeMode=ReadIniData(szDir, "Time", "In Shuttle Soak Time Mode", 1);                    //jou 2012-06-29 Index Pick up need wait Soak Time
        Temperature.bZ2DownSocket=ReadIniData(szDir, "Time", "bZ2DownSocket",0);                                        //kevin 20131211

        if(CosFunction.bOffsetTempByRecipeMinMaxLimit)                          //StevenHong 20260119 : Add Offset By Recipe Max Limit
        {
            Temperature.iOffsetByRecipeMaxLimit = ReadWriteIni(szDir, "User OffSet", "MaxLimit", Temperature.iOffsetByRecipeMaxLimit, 60,  bReadFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
            Temperature.iOffsetByRecipeMinLimit = ReadWriteIni(szDir, "User OffSet", "MinLimit", Temperature.iOffsetByRecipeMinLimit, -60, bReadFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
        }
        for(i=0; i<tcTotalCount; i++)
        {
            S.printf("CH%d", i+1);
            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&bSendChangeTemp)               //kevin 20161011 ASE調整溫度過大 需設最大值
                dTempOffset[0][i]=ReadIniData(szDir, "User OffSet", S, 0.1);
            else
                dTempOffset[0][i]=ReadWriteIni(szDir, "User OffSet", S, dTempOffset[0][i], 0.1, bReadFile, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);  //Steven 20160906 : 溫度讀檔保護

            if(CosFunction.bTemp5PointKitOffset)                                //JerryYang 20221003 Analog要求新增Kit三點offset
            {
                S.printf("CH%d", i+1);
                Temperature.fTempOffSet[KitLowBase][i]      =ReadIniData(szDir, "Kit Low OffSet",  S, 0.0);
                Temperature.fTempOffSet[KitMidBase][i]      =ReadIniData(szDir, "Kit Mid. OffSet", S, 0.0);
                Temperature.fTempOffSet[KitHigBase][i]      =ReadIniData(szDir, "Kit High OffSet", S, 0.0);
                Temperature.fTempOffSet[KitAmbientHotLow][i]=ReadIniData(szDir, "Kit AmbientHotLowOffSet", S, 0.0);     //kevin 20141006
                Temperature.fTempOffSet[KitAmbientHotMid][i]=ReadIniData(szDir, "Kit AmbientHotMidOffSet", S, 0.0);     //kevin 20141006
            }

            if(CosFunction.bHiSiliconFunction)                                  //kevin 20191227 add
            {
               if(dTempOffset[0][i]>=30)
                    dTempOffset[0][i]=30;
            }

            if(bSendChangeTemp)                                                 //起動auto temp offset
            {
                if(abs(dTempOffset[0][i]-dTempOffset[1][i])<=10 &&              //kevin 20151204 生產中offset 溫度<=10 度 才可以改變溫度
                  (dTempOffset[0][i]<=InputLimit.iTempHigh &&
                   dTempOffset[0][i]>=InputLimit.iTempLow))                     //kevin 20161011
                {
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                }
                else
                {
                    if(dTempOffset[0][i]>=InputLimit.iTempHigh)
                    {
                        dTempOffset[1][i]= InputLimit.iTempHigh;
                        str1.printf("ASE_Tmperature_offset_OverHigh: %d", dTempOffset[0][i]);
                        RecordProcess(str1);                                    //kevin 20161011
                    }
                    else if(dTempOffset[0][i]<=InputLimit.iTempLow)             //kevin 20161011 //kevin 20151204 生產中offset 溫度<=10 度 才可以改變溫度
                    {
                        dTempOffset[1][i]=InputLimit.iTempLow;
                        str1.printf("ASE_Tmperature_offset_OverLow: %d", dTempOffset[0][i]);
                        RecordProcess(str1);                                    //kevin 20161011
                    }
                    str1="";
                    str1.printf("CH%d", i+1);
                    WriteIniData(szDir, "User OffSet", str1, dTempOffset[1][i]);
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[1][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                    bSendChangeTempError=true;
                }
            }
            else
            {
                dTempOffset[1][i]=dTempOffset[0][i];
                Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];       //kevin 20151023 紀錄讀取溫度offset 0:最新資料
            }

            if(ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                             //Steven 20181102 : For QTI全球統一Offset
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "Cal Temp", S, 0.0);
            else if(ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61)
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "IndividualTempSetting", S, 40.0);
            else
                Temperature.fIndividualTemp[i]=ReadIniData(szDir, "IndividualTempSetting", S, 30.0);                    //Steven 20140924 : 各個加熱區獨立有自己的設定值

            tATCData.dOriginalUseOffset[i]=Temperature.fTempOffSet[UserOffSet][i];                                      //2014-05-30    Dell    for ATC6.0

            if(CosFunction.bInitTempOffsetByWorkFile==true)                     //jou 2015-06-13 Initial Temperture Offset by WorkFile
                Temperature.fTempOffSet[InitTempOffset][i]  =ReadIniData(szDir, "Init Temp OffSet", S, 0.0);            //Steven 201    41117 : 起測時溫度要補Offset
        }

        if(bSendChangeTemp)                                                     //kevin 20160119 起動auto temp offset 成功
        {
            if(bSendChangeTempError)
            {                                                                   //溫度異常   恢復
                for(i=0; i<tcTotalCount; i++)
                {
                    str1.printf("CH%d", i+1);
                    WriteIniData(szDir, "User OffSet", str1, dTempOffset[1][i]);
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[1][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                }
            }
            else
            {
                for(i=0; i<tcTotalCount; i++)                                   //ok  sample 更新資料
                {
                    Temperature.fTempOffSet[UserOffSet][i]=dTempOffset[0][i];   //kevin 20151023 紀錄讀取溫度offset 0:最新資料
                    dTempOffset[1][i]=dTempOffset[0][i];                        //kevin 20160119    sample
                }
            }

            if(Temperature.fWorkTemperBase==10)                                 //溫度資料異常
            {
                iSendChangeTempError=2;
                Temperature.fWorkTemperBase=bufferfWorkTemperBase;              //kevin 20160614
                if(bSendChangeTemp==false)
                {
                    ShowErrorMessage("WAR16107", K_RETRY, MMSystem);            //20160531
                }
            }

            if(Temperature.fSoakTime==999)
            {
                Temperature.fSoakTime=bufferffSoakTime;                         //kevin 20160614
                iSendChangeTempError=3;
                if(bSendChangeTemp==false)
                {
                    ShowErrorMessage("WAR16108", K_RETRY, MMSystem);            //20160531
                }
            }
        }

        bSendChangeTemp=false;                                                  //kevin 20151023
        Temperature.bActiveHeatGun  =ReadIniData(szDir, "Mode", "Active_Heat_Gun", false);                              //Steven 20120525 : 熱風槍
        if(INSTALL_HEAT_GUN>1)
        {
            Temperature.bUseCDAOnly =ReadIniData(szDir, "Mode", "Use CDA Only", false);;                                //Steven 20181012 : 使用熱風槍吹冷風
            if(Temperature.bUseCDAOnly)
            {
                Temperature.bActiveHeatGun=false;
            }
        }
        else
        {
            Temperature.bUseCDAOnly =false;
        }
        Temperature.iHotGunFLowLimit_H  =ReadIniData(szDir, "Mode", "iHotGunFLowLimit_H", 0);                           //KaiChen 20190729 ：Hot Gun Flow
        Temperature.iHotGunFLowLimit_L  =ReadIniData(szDir, "Mode", "iHotGunFLowLimit_L", 0);                           //KaiChen 20190729 ：Hot Gun Flow

        Temperature.bUseFixTemp     =ReadIniData(szDir, "DUT Setting",  "bUseFixTemp",    false);                       //Steven 20131025 : 使4固定的DUT溫度
        Temperature.dFixedTemp      =ReadIniData(szDir, "DUT Setting",  "dFixedTemp",     40.0);                        //Steven 20131025 : 固定的DUT溫度值
        Temperature.bShowFixedTemp  =ReadIniData(szDir, "DUT Setting",  "bShowFixedTemp", false);                       //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值
        Temperature.bActiveATCHeatGun  =ReadIniData(szDir, "Mode", "Active_ATC_Heat_Gun", false);                       //JerryYang 20220408 : add for ATC3.5

        if(CosFunction.bTempCalByRecipe==true)                                  //jou 20220725 : Temperature calibration by recipe
            Temperature.bTempCalByRecipe=ReadIniData(szDir, "Mode", "bTempCalByRecipe", false);
    }

    if(Temperature.iMachineTempMode==0)
    {
        LastSet.iTemperature=Tempture_Hot;
    }
    else if(Temperature.iMachineTempMode==1)
    {
        LastSet.iTemperature=Tempture_Ambient;
    }
    else if(Temperature.iMachineTempMode==3)                                    //Steven 20200421 : 補上Ambient Temp
    {
        LastSet.iTemperature=Tempture_AmbientHot;
    }
    else
    {
        if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)                   //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
            LastSet.iTemperature=ReadIniData(szDir, "Mode",    "WorkTempMode",  LastSet.iTemperature);
    }

    if(ATC_SYSTEM==eNewATCSystem &&                                             //Steven 20180817 : Boost Function
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
    {
        Temperature.bBoostFuncttion                     =CheckAndReadIniData(szDir, "Boost Function", "bBoostFuncttion",        false);
        Temperature.bLBTempFunction                     =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempFunction",        false);
    }
    else
    {
        Temperature.bBoostFuncttion=false;
        if(LB_TEMP==true && CosFunction.bNonATCSupportLBFunction==true)         //JerryYang 20220126: non ATC也支援LB function
        {
            Temperature.bLBTempFunction                     =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempFunction",        false);
        }
        else
        {
            Temperature.bLBTempFunction=false;
        }
    }
    Temperature.bEnableBoostOffset[Temperature.eBMax]   =Temperature.bBoostFuncttion;
    Temperature.dBoostIdleTime[Temperature.eBMax]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostIdleTime[0]",      15.0);
    Temperature.dBoostOffset[Temperature.eBMax]         =CheckAndReadIniData(szDir, "Boost Function", "dBoostOffset[0]",        0.0);
    Temperature.dBoostDuration[Temperature.eBMax]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostDuration[0]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBMax]   =CheckAndReadIniData(szDir, "Boost Function", "dPostBoostDuration[0]",  0.0);
    Temperature.bEnableBoostOffset[Temperature.eBMid]   =Temperature.bBoostFuncttion;
    Temperature.bEnableBoostOffset[Temperature.eBMin]   =Temperature.bBoostFuncttion;
    Temperature.dBoostIdleTime[Temperature.eBMin]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostIdleTime[2]",      5.0);
    Temperature.dBoostOffset[Temperature.eBMin]         =CheckAndReadIniData(szDir, "Boost Function", "dBoostOffset[2]",        0.0);
    Temperature.dBoostDuration[Temperature.eBMin]       =CheckAndReadIniData(szDir, "Boost Function", "dBoostDuration[2]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBMin]   =CheckAndReadIniData(szDir, "Boost Function", "dPostBoostDuration[2]",  0.0);

    Temperature.bEnableBoostOffset[Temperature.eBLBI]   =true;
    Temperature.dBoostIdleTime[Temperature.eBLBI]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[3]",      0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostOffset[Temperature.eBLBI]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[3]",        0.0);
    Temperature.dBoostDuration[Temperature.eBLBI]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostDuration[3]",      0.0);
    Temperature.dPostBoostDuration[Temperature.eBLBI]   =CheckAndReadIniData(szDir, "LB Temp Function", "dPostBoostDuration[3]",  0.0);

    Temperature.bEnableBoostOffset[Temperature.eBLBL]   =Temperature.bLBTempFunction;
    Temperature.dBoostIdleTime[Temperature.eBLBL]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[4]",      0.0);
    Temperature.dBoostOffset[Temperature.eBLBL]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[4]",        0.0);
    Temperature.dThreshold                              =CheckAndReadIniData(szDir, "LB Temp Function", "Threshold",              0.0);                         //Steven 20190928 : L/B升溫的門檻值
    Temperature.dBoostDuration[Temperature.eBLBL]       =0.0;
    Temperature.dPostBoostDuration[Temperature.eBLBL]   =0.0;

    Temperature.bEnableBoostOffset[Temperature.eBLBB]   =CheckAndReadIniData(szDir, "LB Temp Function", "bEnableBoostOffset[5]",  false);
    Temperature.dBoostIdleTime[Temperature.eBLBB]       =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostIdleTime[5]",      0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostOffset[Temperature.eBLBB]         =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostOffset[5]",        0.0);                         //Steven 20181023 : LB溫度
    Temperature.dBoostDuration[Temperature.eBLBB]       =10.0;
    Temperature.dPostBoostDuration[Temperature.eBLBB]   =0.0;
    Temperature.dBoostTimeOut                           =CheckAndReadIniData(szDir, "LB Temp Function", "dBoostTimeOut",          1200.0);                      //Steven 20181222 : Add LB升溫的Time out

    Temperature.iBoostFunctionMode      =CheckAndReadIniData(szDir, "Boost Function", "iBoostFunctionMode",  0);

    Temperature.dLBTempHighSettingValue                          =CheckAndReadIniData(szDir, "LB Temp Function", "LB_Temp_High_Setting",            0.0);       //Frank 20241231 : add
    Temperature.dLBTempLowSettingValue                           =CheckAndReadIniData(szDir, "LB Temp Function", "LB_Temp_Low_Setting",             0.0);
    Temperature.bLBTempHighAlarm_Enable                          =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempHighAlarm_Enable",         false);     //Frank 20241231 : add
    Temperature.bLBTempLowAlarm_Enable                           =CheckAndReadIniData(szDir, "LB Temp Function", "bLBTempLowAlarm_Enable",          false);

    if(Temperature.dBoostIdleTime[Temperature.eBMin]>Temperature.dBoostIdleTime[Temperature.eBMax])
        Temperature.dBoostIdleTime[Temperature.eBMin]=Temperature.dBoostIdleTime[Temperature.eBMax];

    if(LastSet.iTemperature==Tempture_Ambient)
        Temperature.bBoostFuncttion=false;
    //<==
    //Steven 20180817 : Boost Function

    if(Temperature.bBoostFuncttion==false)                                      //Steven 20180817 : Boost Function啟用時, initial offset就關閉
        Temperature.bEnableTempOffsetForInitial         =ReadIniData(szDir, "InitialMode",       "EnableTemperatureOffsetforInitial",         false);           //Steven 20141117 : 起測時溫度要補Offset
    else
        Temperature.bEnableTempOffsetForInitial         =false;

    if(CosFunction.bUseChamberBoostMode)                                        //Steven 20191128 : Chamber Boost Function
    {
        Temperature.bEnableChamberBoost                 =ReadIniData(szDir, "ChamberBoostMode",       "bEnableChamberBoost",        false);
        Temperature.iChamberBoostTime                   =CheckRange(ReadIniData(szDir, "ChamberBoostMode",       "iChamberBoostTime",          10), 1, 30);
        Temperature.iChamberBoostOffset                 =CheckRange(ReadIniData(szDir, "ChamberBoostMode",       "iChamberBoostOffset",        10), 0, 30);
        fLotInfo->tsChamberBoost->TabVisible            =Temperature.bEnableChamberBoost;
    }
    else
    {
        Temperature.bEnableChamberBoost     =false;
        fLotInfo->tsChamberBoost->TabVisible=false;
    }

    if((LastSet.iTemperature==Tempture_Hot ||
        LastSet.iTemperature==Tempture_AmbientHot ||
        CUSTOMER_CODE==CC_KYEC_LEE) &&
       Temperature.bEnableTempOffsetForInitial)                                 //Steven 20180820 : 簡化init temp offset判斷式
    {
        bUseInitTempOffset=true;
    }
    else
    {
        bUseInitTempOffset=false;
    }

    if(LastSet.iTemperature==Tempture_Hot ||
       LastSet.iTemperature==Tempture_AmbientHot ||
       CUSTOMER_CODE==CC_KYEC_LEE)                                              //Steven 20180820 : 簡化init temp offset判斷式
    {
        bUseInitDelay=true;
    }
    else
    {
        bUseInitDelay=false;
    }

    if(CUSTOMER_CODE==CC_SIGURD_PeiXing && W906_FormShowing("fContact", fContact->fShow))                     //Sam 20240118 : contact 介面下關閉 InitTempOffset 加熱功能  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
    {
        bUseInitTempOffset=false;
    }

    Temperature.iCintactCntForTempOffsetAtInitial       =ReadIniData(szDir, "InitialMode",       "iCintactCntForTempOffsetAtInitial",         1);               //Steven 20141117 : 起測時溫度要補Offset
    Temperature.iCintactDelayCntForInitTempOffset       =ReadIniData(szDir, "InitialMode",       "iCintactDelayCntForInitTempOffset",         10);              //Steven 20141117 : 起測時溫度要補Offset

    if(IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false)   //Sam 20231214 : Temp offset use ready temp range
        Temperature.iTempReadyRange                         =CheckRange(ReadIniData(szDir, "InitialMode", "iTempReadyRange", 0), 10, 0);
    else
        Temperature.iTempReadyRange                         =0;

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20160621 海思版本PID 設定Function 強制設為false
        Temperature.bUsePIDControl=false;
    else
        Temperature.bUsePIDControl                      =ReadIniData(szDir, "InitialMode",       "UsePIDControl",                             false);           //Ifor 20150910 : ATC PID Control

    for(int i=0; i<3; i++)
    {
        if(CosFunction.bHiSiliconFunction==true)                                //wei 20160506 ID_Min ID_Max 強制為0
        {                                                                       //Ifor 20160621 海思版本不讀取設定強制為0
            Temperature.iATC_PID_Min_Offset[i]          =0;
            Temperature.iATC_PID_Max_Offset[i]          =0;
        }
        else
        {
            Temperature.iATC_PID_Min_Offset[i]          =ReadIniData(szDir, "InitialMode",       "iATC_PID_Min_Offset_"+ AnsiString(i),        1000);           //Ifor 20150910 : ATC PID Min Offset
            Temperature.iATC_PID_Max_Offset[i]          =ReadIniData(szDir, "InitialMode",       "iATC_PID_Max_Offset_"+ AnsiString(i),        2000);           //Ifor 20150910 : ATC PID Max Offset
        }
    }
    Temperature.b3SigmaTempMonitior_Enable              =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_Enable",                             false);      //kevin 20200521 add 3 Sigma 溫度記錄比對
    Temperature.i3SigmaTempMonitior_SetCoun             =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_SetCount",                             0);        //kevin 20200521 add 3 Sigma 溫度記錄比對          //kevin 20200521 add 3 Sigma Index Contract time
    Temperature.d3SigmaTempMonitior_Set3xSigmaValue     =ReadIniData(szDir, "Sigma",       "SigmaTempMonitior_Set3xSigmaValue",                      0.0);      //kevin 20200521 add 3 Sigma 溫度記錄比對  //kevin 20200521 add 3 Sigma 客戶比對誤差值

    Temperature.bEnableTJFunction                       =ReadIniData(szDir, "ATC",       "bEnableTJFunction",             false);                               //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    Temperature.iTjSiteMapType                          =ReadIniData(szDir, "ATC",       "iTjSiteMapType",                0);                                   //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    Temperature.fSetTJ_Offset                           =ReadIniData(szDir, "ATC",       "dTjOffset",                     0.0);
    Temperature.fSetTJ_Slope                            =ReadIniData(szDir, "ATC",       "dTjSlope",                      1.0);

    Temperature.fTjInputVLow                            =ReadIniData(szDir, "ATC",       "dTjInputVLow",                  0.00);
    Temperature.fTjInputVHigh                           =ReadIniData(szDir, "ATC",       "dTjInputVHigh",                 10.0);

    Temperature.dDelayAfterSOT                          =ReadIniData(szDir, "ATC",       "dDelayAfterSOT",                0.0);
    Temperature.dDelayAfterSOTContinue                  =ReadIniData(szDir, "ATC",       "dDelayAfterSOTContinue",                0.0);                         //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    Temperature.bTjWatchdog                             =ReadIniData(szDir, "ATC",       "bTjWatchdog",                   false);                               //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    sATC_CH_Tj->CommaText=ReadIniData(szDir, "ATC",       "sATC_CH_Tj",                   AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));                       //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    for(int i=0; i<16; i++)
    {
        Temperature.iATCCH_Tj[i]=atoi(AnsiString(sATC_CH_Tj->Strings[i]).c_str());
    }

    if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                            //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
    {
        if(ATC_SYSTEM==eATCSiliconType)
        {
            Temperature.bATCTemperatureSet  =ReadIniData(szDir, "ATC", "Temperature Set", false);
        }
        else if(ATC_SYSTEM==eATCHonPrecType)
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                Temperature.iATCChillerTemp=15;                                 //Ifor 20161103 ATC 2.0 Chiller 溫度寫死20度   //JerryYang 20161128 守志說矽品SPEC固定15度
            else
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", 15), 40, 5);           //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
        }
        else if(ATC_SYSTEM==eNewATCSystem)
        {
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -20);        //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }
            else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36)   //JerryYang 20220408 : add for ATC3.5
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -40);        //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }
            else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61)
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", -20), 30, -55);
            }
            else
            {
                Temperature.iATCChillerTemp=CheckRange(ReadIniData(szDir, "ATC", "Chiller Temp", 15), 40, 5);           //Ifor 20161103 (Jou)Chiller溫度 下極限5度 上極限40度
            }

            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {
            }
            else
            {
                if(CosFunction.bUseHandlerSetChillerTemp==true ||               //Ifor 20161103 (Jou)矽品要求依據工作擋修改Chiller溫度
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)
                {
                }
                else
                {
                    if(Temperature.iATCChillerTemp<15)
                    {
                        if(LastSet.iTemperature==Tempture_Hot)
                        {
                            if(Temperature.fWorkTemperBase<=25)
                                Temperature.iATCChillerTemp=15;
                            else if(Temperature.fWorkTemperBase<=80)
                                Temperature.iATCChillerTemp=20;
                            else
                                Temperature.iATCChillerTemp=25;
                        }
                        else
                        {
                            Temperature.iATCChillerTemp=15;
                        }
                    }
                }
            }
            edChillerTemp->Text=IntToStr(Temperature.iATCChillerTemp);
            WriteIniData(szDir, "ATC", "Chiller Temp", edChillerTemp->Text);
        }

        Temperature.bATCHandlerStart        =ReadIniData(szDir, "ATC", "Handler Start"  , true);
        Temperature.bATCTestStrat           =ReadIniData(szDir, "ATC", "Test Strat"     , true);
        Temperature.dATCSP2                 =CheckRange(ReadIniData(szDir, "ATC", "SP2", 130.0), 135.0, 20.0);
        Temperature.bATCActiveCooling       =ReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.dATCInPC[0]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[0]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[1]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[1]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[2]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[2]", 0.0), 30.0, -30.0);
        Temperature.dATCInPC[3]             =CheckRange(ReadIniData(szDir, "ATC", "InPC[3]", 0.0), 30.0, -30.0);

        if(CUSTOMER_CODE==CC_SCS)                                               //Ifor 20160711 避免SCS要求ATC7.0頁面關閉造成異常問題
            Temperature.bATC70Active        =false;
        else
            Temperature.bATC70Active        =ReadIniData(szDir, "ATC", "ATC7", false);                                  //Eliot 2015_0105

        Temperature.bATC7ChannelEnabled[0]  =ReadIniData(szDir, "ATC", "ATC7CH1Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[1]  =ReadIniData(szDir, "ATC", "ATC7CH2Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[2]  =ReadIniData(szDir, "ATC", "ATC7CH3Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7ChannelEnabled[3]  =ReadIniData(szDir, "ATC", "ATC7CH4Enabled", false);                        //Eliot 2015_0105
        Temperature.bATC7TSDFunction        =ReadIniData(szDir, "ATC", "TSDFunction",   false);                         //Eliot 2015_0105
        Temperature.dATC7TSDTimeOut         =ReadIniData(szDir, "ATC", "TSDTimeOut",    0.0);                           //Steven 20160604 : by site TSD
        Temperature.ATCInitialOffset[0]     =ReadIniData(szDir, "ATC", "Initial_0", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[1]     =ReadIniData(szDir, "ATC", "Initial_1", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[2]     =ReadIniData(szDir, "ATC", "Initial_2", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC
        Temperature.ATCInitialOffset[3]     =ReadIniData(szDir, "ATC", "Initial_3", 0.0);                               //Steven 20151006 : Initial Temp Offset for ATC

        Temperature.dATCConFailOffset[0]    =ReadIniData(szDir, "ATC", "ConsFail_0", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[1]    =ReadIniData(szDir, "ATC", "ConsFail_1", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[2]    =ReadIniData(szDir, "ATC", "ConsFail_2", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.dATCConFailOffset[3]    =ReadIniData(szDir, "ATC", "ConsFail_3", 0.0);                              //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.bEnableATCConFailOffset =ReadIniData(szDir, "ATC", "bEnableATCConFailOffset", false);               //Steven 20151123 : Continue Fail Temp Offset for ATC
        Temperature.iATCConFailOffsetCount  =ReadIniData(szDir, "ATC", "iATCConFailOffsetCount",  0);                   //Steven 20151123 : Continue Fail Temp Offset for ATC

        Temperature.dATCQAModeOffset[0]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset1", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[1]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset2", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[2]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset3", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.dATCQAModeOffset[3]     =ReadIniData(szDir, "ATC", "ATCQAModeOffset4", 0.0);                        //Steven 20151125 : QA Mode Temp Offset for ATC
        Temperature.bEnableATCQAModeOffset  =ReadIniData(szDir, "ATC", "bEnableATCQAModeOffset", false);                //Steven 20151125 : QA Mode Temp Offset for ATC

        Temperature.bEnableATCTestTimeOffset=ReadIniData(szDir, "ATC", "bEnableATCTestTimeOffset", false);              //Steven 20160216 : 測試時間太短也要Offset
        Temperature.iATCTestTimeOffsetTime  =ReadIniData(szDir, "ATC", "iATCTestTimeOffsetTime", 15);                   //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[0]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset1", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[1]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset2", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[2]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset3", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset
        Temperature.dATCTestTimeOffset[3]   =ReadIniData(szDir, "ATC", "dATCTestTimeOffset4", 0.0);                     //Steven 20160216 : 測試時間太短也要Offset

        Temperature.iATC_OFS_ST            =ReadIniData(szDir, "ATC", "ATC_OFS_ST",   0.0);                             //Ifor 20241118 : 測試中變溫

        for(int i=0; i<32; i++)                                                 //Ifor 20160419 add ATC 32 Site Offset
        {
            str1.sprintf("ATCTempOffset[%d]", i);
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20180215 (Steven) ADD atc
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 60.0, -60.0);
            else
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
            if(CosFunction.bUseSecondATCTempOffset)                             //Ifor 20241118 : 測試中變溫
            {
                str1.sprintf("ATCSecondTempOffset[%d]", i);
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20180215 (Steven) ADD atc
                    Temperature.dATCSecondTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 60.0, -60.0);
                else
                    Temperature.dATCSecondTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
            }
        }

        Temperature.bUseReferTempSensor=CheckAndReadIniData(szDir, "ATC", "UseReferTempSensor", (Temperature.bATCActiveCooling));                               //wei 20160506 第二點溫度預設修改為true //Steven 20251003 : 預設值根據ATC有沒有開來決定
        Temperature.bUseTC2Offset           =CheckAndReadIniData(szDir, "ATC", "UseTC2Offset",       false);            //KenHsieh 20240311 : add Tc2 Offset

        for(int i=0; i<3; i++)                                                  //Ifor 20190306 : add Package Offset 三點校正
        {
            str1.sprintf("ATCPackageOffset[%d]", i);
            Temperature.dATCPackageOffset[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 20.0, -20.0);

            str1.sprintf("ATCPackagetemp[%d]", i);
            Temperature.dATCPackageTemp[i]=CheckRange(ReadIniData(szDir, "ATC", str1, 0.0), 150.0, 20.0);
        }

        for(int i=0; i<2; i++)                                                  //Ifor 20190730 : add FFC Temperature Offset
        {
            for(int j=0; j<10; j++)
            {
                str1.sprintf("iArm%d_FFCTimeOn[%d]", i+1, j);
                Temperature.iATC_FFCTimeOn[i][j]   =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCTimeOff[%d]", i+1, j);
                Temperature.iATC_FFCTimeOff[i][j]  =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCOffset[%d]", i+1, j);
                Temperature.dATC_FFCOffset[i][j]   =ReadIniData(szDir, "ATC", str1, 0.0);

                str1.sprintf("iArm%d_FFCPointUse[%d]", i+1, j);
                Temperature.bATC_FFCPointUse[i][j] =ReadIniData(szDir, "ATC", str1, false);
            }
        }
        Temperature.bATC_FFCEnable =ReadIniData(szDir, "ATC", "bATC_FFCEnable", false);

        Temperature.bMultiZoneEnable=ReadIniData(szDir, "ATC", "Multi Zone Enable", false);                             //wei 20240617 Multi Zone
        for(int i=0; i<4; i++)
        {
            str1.sprintf("Zone Temp %d Use", i+1);
            Temperature.bZoneTempEnable[i]=ReadIniData(szDir, "ATC", str1, false);

            str1.sprintf("Zone Temp %d Setting", i+1);
            Temperature.dZoneTempSetting[i]=ReadIniData(szDir, "ATC", str1, 0.0);
        }

        fLotInfo->ShowATCTempPanel();                                           //Steven 20241112 : 調整ATC溫度顯示

        InitialHeaterDoor();

        if(ATC_SYSTEM==eATCHonPrecType)                                         //Steven 20120410 End: Hontech ATC
        {
            if(LastSet.iTemperature==Tempture_Hot ||
               LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20180709 (wei) add  Tempture_AmbientHot
            {
                for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
                {
                    ATCInterfaceForm->SetWorkTemperature(iAtc, Temperature.fWorkTemperBase);

                    if(bUseInitTempOffset &&                                    //Steven 20141117 : 起測時溫度要補Offset
                       iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)                                 //Steven 20141125 : <= --> <
                    {
                        ATCInterfaceForm->SetOffsetTemperature(iAtc, Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc]);
                    }
                    else
                    {
                        ATCInterfaceForm->SetOffsetTemperature(iAtc, Temperature.dATCInPC[iAtc]);
                    }
                }
            }
            else
            {
                for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
                {
                    ATCInterfaceForm->SetWorkTemperature(iAtc, IniConfig.dATCAmbientTemperature);                       //Steven 20131007 : 啟用ATC時,強制使用 [L11] ATC常溫的溫度
                }
            }
            ATCInterfaceForm->SetChillerTemperature(Temperature.iATCChillerTemp);
        }

        if(Temperature.bUseTC2Offset)                                           //KenHsieh 20240311 : add Tc2 Offset
        {
            IniConfig.bL11_6ATCUseTemperatureOutsideAlarm=false;
            IniConfig.bL11_7ATCUseMaxSurgeAlarm=false;
            IniConfig.bL11_8ATCUseTemperatureCompare=false;
        }

        if(ATC_SYSTEM==eNewATCSystem && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36) )
            Temperature.bATC_MultiSensorEnable=ReadIniData(szDir, "ATC", "Multi Sensor Offset Enable", false);          //Ifor 20251203 add:ATC Multi Sensor Offset
        else
            Temperature.bATC_MultiSensorEnable=false;
        for(int i=0; i<4; i++)
        {
            str1.sprintf("Multi Sensor Offset %d Use", i+1);
            Temperature.bATC_MultiSensorUse[i]=ReadIniData(szDir, "ATC", str1, false);                                  //Ifor 20251203 add:ATC Multi Sensor Offset
        }

        for(int i=0; i<32; i++)                                                 //Ifor 20190730 : add FFC Temperature Offset
        {
            str1.sprintf("Multi Sensor Offset[%d]", i+1);
            Temperature.dATC_MultiSensorOfs[i]=ReadIniData(szDir, "ATC", str1, 0.0);
        }
    }
    else
    {
        Temperature.bUseReferTempSensor =false;                                 //Steven 20251003 : add default value
    }

    Temperature.ATCTypeName             =CheckAndReadIniData(szDir, "ATC", "Type Name", AnsiString("Rogers"));          //Ifor 20230828 add:送ATC Type 資料給GPIB
    if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                //20141204 ChungHung add for ATC3.0   //2014-05-30    Dell    for ATC6.0
    {
        ATCInterfaceForm->ATC_60_SYS.SetATCRun(LastSet.iTemperature>0?true:false);
        ATCInterfaceForm->ATC_60_SYS.SetTargetTemperature(Temperature.fWorkTemperBase);
        Temperature.bATCActiveCooling   =CheckAndReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.bUseTjFunction      =CheckAndReadIniData(szDir, "ATC", "USE Tj Function", false);
        Temperature.iTjMode             =CheckAndReadIniData(szDir, "ATC", "Tj Mode", 0);
        Temperature.iTjAvgTimes         =CheckAndReadIniData(szDir, "ATC", "Tj Avg Times", 5);
        Temperature.ATCFileName         =CheckAndReadIniData(szDir, "ATC", "File Name", AnsiString("1"));               //Steven 20150810 : add for ATC

        ATCInterfaceForm->ATC_60_SYS.LoadRecipe(Temperature.ATCFileName);       //Steven 20150810 : add for ATC
    }
    else if(ATC_SYSTEM==eNewATCSystem)                                          //Ifor 20151230 :add New ATC Interface HandlerArm
    {                                                                           //Ifor 20160429強制關閉ATC7.0 舊的啟動選項
        Temperature.bATC70Active=false;
        Temperature.bATCActiveCooling   =CheckAndReadIniData(szDir, "ATC", "Active Cooling", false);
        Temperature.bUseTjFunction      =CheckAndReadIniData(szDir, "ATC", "USE Tj Function", false);
        Temperature.iTjMode             =CheckAndReadIniData(szDir, "ATC", "Tj Mode", 0);
        Temperature.iTjAvgTimes         =CheckAndReadIniData(szDir, "ATC", "Tj Avg Times", 5);
        Temperature.ATCFileName         =CheckAndReadIniData(szDir, "ATC", "File Name",AnsiString("1"));                //Steven 20150810 : add for ATC
        Temperature.iATC60Air           =CheckAndReadIniData(szDir, "ATC", "iATC60Air", 0);
        cbbATC_RecipeFile->Text         =Temperature.ATCFileName;

        ControlATC60AirFlow();

        //Ifor 20190328 : add TJ Temp Over Range
        //==>
        Temperature.dTJTempRange_High   =CheckRange(ReadIniData(szDir, "ATC", "TJTempRange_High", 0.0), 20.0, 1.0);     //JerryYang 20251201 : 放寬到20
        Temperature.dTJTempRange_Low    =CheckRange(ReadIniData(szDir, "ATC", "TJTempRange_Low", 0.0) , 30.0, 1.0);
        //<==
        //Ifor 20190328 : add TJ Temp Over Range
        if(ATC_SYSTEM==eNewATCSystem && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))                     //Ifor 20230418 add:Index 回溫功能
            Temperature.bEnableTempRise     =CheckAndReadIniData(szDir, "ATC", "Enable Temp Rise", false);
        else
            Temperature.bEnableTempRise=false;
        Temperature.dTempRiseTemp       =CheckRange(ReadIniData(szDir, "ATC", "dTempRiseTemp", 25.0), 100.0, 1.0);
        Temperature.dTempRiseDelay      =CheckRange(ReadIniData(szDir, "ATC", "dTempRiseDelay", 1.0), 1000.0, 1.0);
        if(Tri_Temp_Machine==1 ||
           (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
        {
            Temperature.EnableAirMachineSocket              = ReadIniData(szDir,"InitialMode","EnableAirMachineSocket",false);
            Temperature.dSetAirstreamTemperatureRang_Socket = ReadIniData(szDir,"InitialMode","dSetAirstreamTemperatureRang_Socket",0.0);
            Temperature.bUseOutShuttleDesoakTime            = ReadIniData(szDir,"InitialMode","UseOutShuttleDesoakTime",false);
            Temperature.iOutShuttleDesoakTime               = ReadIniData(szDir,"InitialMode","OutShuttleDesoakTime", 0);
            Temperature.bEnableTesterDryAirControl          = ReadIniData(szDir,"InitialMode","EnableTesterDryAirControl",false);
            Temperature.SetAirstreamTemperatureRang_Index   = ReadIniData(szDir,"InitialMode","SetAirstreamTemperatureRang_Index",0.0);
            Temperature.bEnableArm_1_Air                    = ReadIniData(szDir,"InitialMode","bEnableArm_1_Air",false);
            Temperature.bEnableArm_2_Air                    = ReadIniData(szDir,"InitialMode","bEnableArm_2_Air",false);
            Temperature.bEnableSocket_Air                   = ReadIniData(szDir,"InitialMode","bEnableSocket_Air",false);
            Temperature.iAirVolumeLmt                       = ReadIniData(szDir,"InitialMode","iAirVolumeLmt",0);
            Temperature.dAirStreamSocket_Offset             = ReadIniData(szDir,"InitialMode","AirStreamSocket_Offset", 0);                                     //Ztex 2023.12.19 Add Air Stream Offset
            Temperature.dAirStreamIndex_Offset              = ReadIniData(szDir,"InitialMode","AirStreamIndex_Offset", 0);                                      //Ztex 2023.12.19 Add Air Stream Offset
            Temperature.iDefrostTimeTooLower                = ReadIniData(szDir,"InitialMode","Defrost_Time_Too_Lower", 600);                                   //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
        }
    }

    Temperature.fChamberCoolTemp =CheckAndReadIniData(szDir, "Cooling", "ChamberCoolTemp", 40.0);                       //Steven 20111209 : Chamber降溫溫度
    Temperature.bUseTesterDocking=CheckAndReadIniData(szDir, "Time", "bUseTesterDocking", false);                       //jou 2014-04-25 Tester Docking Wait delay time
    Temperature.iUseTesterDocking=CheckAndReadIniData(szDir, "Time", "iUseTesterDocking", 20);                          //jou 2014-04-25 Tester Docking Wait delay time

    IniConfig.dATCAmbientTemperature=CheckRange(ReadIniData(szDir, "ATC", "ATCAmbientTemperature", 25.0), 30.0, -5.0);  //Steven 20150108 : [L11-5] For海思使用兩組感溫;     //wei 20151013  by Setup File ATC Ambient Temp set  //JerryYang 20220927 : ATC常溫range限制範圍
    IniConfig.dHeatGunTempATC=CheckRange(ReadIniData(szDir, "Index", "ATC_HeatGunTemp", 30.0), 70.0, 30.0);             //JerryYang 20220408 : add for ATC3.5
    Temperature.dATC_HotGunTemp     =CheckAndReadIniData(szDir, "ATC", "dATC_HotGunTemp", 25.0);                        //Ifor 20220124 add: ATC Hot Air
    Temperature.dATC_HotGunTime     =CheckAndReadIniData(szDir, "ATC", "dATC_HotGunTime", 0.0);                         //Ifor 20220124 add: ATC Hot Air

    Temperature.Arm1Offset  =CheckAndReadIniData(szDir, "Index", "Arm1 Offset", 0.0);
    Temperature.Arm2Offset  =CheckAndReadIniData(szDir, "Index", "Arm2 Offset", 0.0);

    Temperature.Arm1NoFullsiteOffset_1  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset1", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_2  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset2", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_3  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset3", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_4  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset4", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm1NoFullsiteOffset_5  =CheckAndReadIniData(szDir, "Index", "Arm1 No Fullsite Offset5", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_1  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset1", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_2  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset2", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_3  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset3", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_4  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset4", 0.0);          //wei 20160107 No FullSite Add Offset
    Temperature.Arm2NoFullsiteOffset_5  =CheckAndReadIniData(szDir, "Index", "Arm2 No Fullsite Offset5", 0.0);          //wei 20160107 No FullSite Add Offset

    ATCInterfaceForm->iCheckSameTempTime=CheckAndReadIniData(AuthPath+"ATC.ini", "Setup", "iCheckSameTempTime", 300);                                //Ifor 20160308 Add 開啟ATC功能時讀取ATC設定檔   //AI(W906-I01C) 20261002 (Ifor01): golden literal "D:\\HT9045\\Config\\ATC.ini" == AuthPath+"ATC.ini" in production (common.cpp:31; Windows paths ignore case) -- the AuthPath seam (W906_AUTH_PATH) keeps a ctest off the real config\ATC.ini

    Temperature.dSocketAirCoolingOnTimer=CheckAndReadIniData(szDir, "Index", "dSocketAirCoolingOnTimer", 5.0);          //jou 2016-04-28 Socket Air Cooling contact count trun on
    Temperature.dSocketAirCoolingOffTimer=CheckAndReadIniData(szDir, "Index", "dSocketAirCoolingOffTimer", 5.0);        //jou 2016-04-28 Socket Air Cooling contact count trun on

    if(CUSTOMER_CODE==CC_HONPREC_QC)                                            //KenHsieh 20230301 : By Pass Chamber
        Temperature.bByPassChamber          =CheckAndReadIniData(szDir, "Ambient",  "ByPassChamber",     false);
    else
        Temperature.bByPassChamber=false;

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        Temperature.bUseTriTempHeater_Ini[0]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterOutShuttle",           false);
        Temperature.bUseTriTempHeater_Ini[1]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterIndex",                false);
        Temperature.bUseTriTempHeater_Ini[2]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterBase",                 false);
        Temperature.bUseTriTempHeater_Ini[3]    =ReadIniData(szDir, "TriTempSet",  "UseHeaterDoor",                 false);                                     //Ztex 2023.10.23 Add Index Door Heater
    }                                                                           //Heater開關設定

    Temperature.bPowerFollower_Enable   =CheckAndReadIniData(szDir, "ATC",   "bPowerFollow_Enable", false);             //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.iPowerFollower_FullPower=CheckAndReadIniData(szDir, "ATC",   "iPowerFollow_FullPower", 1000);           //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.dPowerFollower_PFSlope  =CheckAndReadIniData(szDir, "ATC",   "dPowerFollower_PFSlope", 0.0);            //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.dPowerFollower_WGain    =CheckAndReadIniData(szDir, "ATC",   "dPowerFollower_WGain", 0.0);              //KenHsieh 20240216 : add ATC Power Follow Function
    Temperature.iPowerFollower_Many2one =CheckAndReadIniData(szDir, "ATC",   "iPowerFollower_Many2one",1);              //KenHsieh 20240216 : add ATC Power Follow Function

//------------------------我是分隔線:以下是寫到DefineTemp-----------------------
    if(bUpdateAll==true)                                                        //jou 2010-01-11 start : 效正溫度改固定一組
    {
        bReadType=(Temperature.iIndexHeatMode==HeadChamber)?true:false;
    }
    else
    {
        bReadType=(rgIndexHeatMode->ItemIndex==HeadChamber)?true:false;
    }

    szDir.sprintf("%sDefineTemp\\", DefaultPath);                               //JerryYang 20170123 避免路徑不存在出現錯誤
    MyForceDirectories(szDir);

    if(bReadType)                                                               //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
                WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                              //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常。//Sam 20210106 : TemperatureData 新增版號來相容
            }
            else                                                                //Sam 20210106 : TemperatureData 新增版號來相容
            {
                if(INDEX_PRESS_TYPE==e500KG)
                {
                    iTemperatureDataVer=ReadIniData(szDir, "Mode","Version",0);
                    if(iTemperatureDataVer<atoi(AnsiString(TemperatureDatavision).c_str()))                             //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    {
                        DeleteFile(szDir);
                        str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                        CopyFile(str.c_str(), szDir.c_str(), false);
                        WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                      //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    }
                }
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber_ATC.Data", DefaultPath);
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }
    else
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\Temperature60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
                WriteIniData(szDir, "Mode", "Version", AnsiString(TemperatureDatavision));                              //Sam 20210106 : TemperatureData 新增版號來相容
            }
            else                                                                //Sam 20210106 : TemperatureData 新增版號來相容
            {
                if(INDEX_PRESS_TYPE==e500KG)
                {
                    iTemperatureDataVer=ReadIniData(szDir, "Mode","Version",0);
                    if(iTemperatureDataVer<atoi(AnsiString(TemperatureDatavision).c_str()))                             //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    {
                        DeleteFile(szDir);
                        str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                        CopyFile(str.c_str(), szDir.c_str(), false);
                        WriteIniData(szDir, "Mode", "Version", (AnsiString)TemperatureDatavision);                      //Sam 20210816 : 修正 TempSet 大缸徑 Offset 設定檔讀取異常
                    }
                }
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                //AI(W906-I08) 20261004 (Ifor01): V912 uTemp_Set.cpp:3005-3012 kept (RULINGS_20261003 #1 / #22, TO_IFOR I-08) --
                //  Hot / AmbientHot read Temperature_ATC.Data, every other mode Temperature_ATC_Cold.Data, the same split as the
                //  web temperature page (FileRW/Temperature.gen.inc:4034-4041). golden 0618 uTemp_Set.cpp:2976 used the one _ATC
                //  file for every mode, so a cold calibration saved on the web page was never read at boot / recipe change /
                //  SetTemp. Ledger: docs/IFOR01_V912_LEDGER.md. The HeadChamber branch above (:3124) is not split in V912 either.
                if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC.Data", DefaultPath);
                }
                else
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC_Cold.Data", DefaultPath);
                }
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20151204 ase kaohsiung 林佳青只 使用一組  temp offset
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }
    else if(CosFunction.bTempCalByRecipe==true && Temperature.bTempCalByRecipe==true)                                   //jou 20220725 : Temperature calibration by recipe
    {
        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\DefineTemperature.Data", DataPath, S);
    }

    szDir2.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);

    Temperature.iTempMode           =ReadIniData(szDir, "Mode",                 "Points",   1);                         //Jimmychiu 20230926 : fixed for iTempMode initial value is 1
    Temperature.fLowBase            =ReadIniData(szDir, "Low OffSet",           "Base",     0.0);
    Temperature.fMiddBase           =ReadIniData(szDir, "Mid. OffSet",          "Base",     0.0);
    Temperature.fHighBase           =ReadIniData(szDir, "High OffSet",          "Base",     0.0);

    Temperature.fAmbientHotLowBase  =ReadIniData(szDir, "AmbientHotLowOffSet",  "Base",     0.0);                       //kevin 20141006
    Temperature.fAmbientHotMiddBase =ReadIniData(szDir, "AmbientHotMidOffSet",  "Base",     0.0);                       //kevin 20141006
    Temperature.fSHighBase          =ReadIniData(szDir, "SHigh OffSet",         "Base",     0.0);                       //Ztex 2024.07.27 Add 6 Point Temperature Offset

    for(i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);
        Temperature.fTempOffSet[LowBase][i]                     =ReadIniData(szDir, "Low OffSet",  S, 0.0);
        Temperature.fTempOffSet[MidBase][i]                     =ReadIniData(szDir, "Mid. OffSet", S, 0.0);
        Temperature.fTempOffSet[HigBase][i]                     =ReadIniData(szDir, "High OffSet", S, 0.0);

        if(CosFunction.bInitTempOffsetByWorkFile==false)                        //jou 2015-06-13 Initial Temperture Offset by WorkFile
            Temperature.fTempOffSet[InitTempOffset][i]          =ReadIniData(szDir, "Init Temp OffSet", S, 0.0);        //Steven 201    41117 : 起測時溫度要補Offset

        Temperature.fTempOffSet[AmbientHotLow][i]               =ReadIniData(szDir, "AmbientHotLowOffSet", S, 0.0);     //kevin 20141006
        Temperature.fTempOffSet[AmbientHotMid][i]               =ReadIniData(szDir, "AmbientHotMidOffSet", S, 0.0);     //kevin 20141006
        Temperature.fTempOffSet[TestOverTimeTempOffset][i]      =ReadIniData(szDir, "TestOverTime Temp OffSet", S, 0.0);                                        //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        Temperature.fTempOffSet[SHigBase][i]                    =ReadIniData(szDir, "SHigh OffSet", S, 0.0);            //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    //jou 2010-01-11 end

    if(IniConfig.bL13HotPlateAndShuttleUseOneTempOffset)                        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    {
        for(i=tcHotPlate1; i<=tcShuttle2; i++)
        {
            S.printf("CH%d", i+1);
            Temperature.fTempOffSet[LowBase][i]                 =ReadIniData(szDir2, "Low OffSet",  S, 0.0);
            Temperature.fTempOffSet[MidBase][i]                 =ReadIniData(szDir2, "Mid. OffSet", S, 0.0);
            Temperature.fTempOffSet[HigBase][i]                 =ReadIniData(szDir2, "High OffSet", S, 0.0);

            if(CosFunction.bInitTempOffsetByWorkFile==false)                    //jou 2015-06-13 Initial Temperture Offset by WorkFile
                Temperature.fTempOffSet[InitTempOffset][i]      =ReadIniData(szDir2, "Init Temp OffSet", S, 0.0);       //Steven 20141117 : 起測時溫度要補Offset

            Temperature.fTempOffSet[AmbientHotLow][i]           =ReadIniData(szDir2, "AmbientHotLowOffSet", S, 0.0);    //kevin 20141006
            Temperature.fTempOffSet[AmbientHotMid][i]           =ReadIniData(szDir2, "AmbientHotMidOffSet", S, 0.0);    //kevin 20141006
            Temperature.fTempOffSet[TestOverTimeTempOffset][i]  =ReadIniData(szDir2, "TestOverTime Temp OffSet", S, 0.0);                                       //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
            Temperature.fTempOffSet[SHigBase][i]                =ReadIniData(szDir2, "SHigh OffSet", S, 0.0);           //Ztex 2024.07.27 Add 6 Point Temperature Offset
        }
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        Temperature.bUseAbitCHK      =CheckAndReadIniData(szDir2, "Mode",    "AmbCheck",     true);

    if(TC401HeaterControl==NoHeater)                                            //Steven 20171227 (Wei) : Add for HT-9045L
    {
        Temperature.iMachineTempMode=1;
        rgTemperatureMode->ItemIndex=1;
        Temperature.bUseAbitCHK=false;
    }

    //Steven 20181203 : fTempOffSet --> fIndividualTemp
    Temperature.dIndexATCSecondTempOffset[ 0]=Temperature.fIndividualTemp[tcAa1];
    Temperature.dIndexATCSecondTempOffset[ 1]=Temperature.fIndividualTemp[tcAb1];
    Temperature.dIndexATCSecondTempOffset[ 2]=Temperature.fIndividualTemp[tcAc1];
    Temperature.dIndexATCSecondTempOffset[ 3]=Temperature.fIndividualTemp[tcAd1];
    Temperature.dIndexATCSecondTempOffset[ 4]=Temperature.fIndividualTemp[tcAe1];
    Temperature.dIndexATCSecondTempOffset[ 5]=Temperature.fIndividualTemp[tcAf1];
    Temperature.dIndexATCSecondTempOffset[ 6]=Temperature.fIndividualTemp[tcAg1];
    Temperature.dIndexATCSecondTempOffset[ 7]=Temperature.fIndividualTemp[tcAh1];
    Temperature.dIndexATCSecondTempOffset[ 8]=Temperature.fIndividualTemp[tcBa1];
    Temperature.dIndexATCSecondTempOffset[ 9]=Temperature.fIndividualTemp[tcBb1];
    Temperature.dIndexATCSecondTempOffset[10]=Temperature.fIndividualTemp[tcBc1];
    Temperature.dIndexATCSecondTempOffset[11]=Temperature.fIndividualTemp[tcBd1];
    Temperature.dIndexATCSecondTempOffset[12]=Temperature.fIndividualTemp[tcBe1];
    Temperature.dIndexATCSecondTempOffset[13]=Temperature.fIndividualTemp[tcBf1];
    Temperature.dIndexATCSecondTempOffset[14]=Temperature.fIndividualTemp[tcBg1];
    Temperature.dIndexATCSecondTempOffset[15]=Temperature.fIndividualTemp[tcBh1];
    Temperature.dIndexATCSecondTempOffset[16]=Temperature.fIndividualTemp[tcAa2];
    Temperature.dIndexATCSecondTempOffset[17]=Temperature.fIndividualTemp[tcAb2];
    Temperature.dIndexATCSecondTempOffset[18]=Temperature.fIndividualTemp[tcAc2];
    Temperature.dIndexATCSecondTempOffset[19]=Temperature.fIndividualTemp[tcAd2];
    Temperature.dIndexATCSecondTempOffset[20]=Temperature.fIndividualTemp[tcAe2];
    Temperature.dIndexATCSecondTempOffset[21]=Temperature.fIndividualTemp[tcAf2];
    Temperature.dIndexATCSecondTempOffset[22]=Temperature.fIndividualTemp[tcAg2];
    Temperature.dIndexATCSecondTempOffset[23]=Temperature.fIndividualTemp[tcAh2];
    Temperature.dIndexATCSecondTempOffset[24]=Temperature.fIndividualTemp[tcBa2];
    Temperature.dIndexATCSecondTempOffset[25]=Temperature.fIndividualTemp[tcBb2];
    Temperature.dIndexATCSecondTempOffset[26]=Temperature.fIndividualTemp[tcBc2];
    Temperature.dIndexATCSecondTempOffset[27]=Temperature.fIndividualTemp[tcBd2];
    Temperature.dIndexATCSecondTempOffset[28]=Temperature.fIndividualTemp[tcBe2];
    Temperature.dIndexATCSecondTempOffset[29]=Temperature.fIndividualTemp[tcBf2];
    Temperature.dIndexATCSecondTempOffset[30]=Temperature.fIndividualTemp[tcBg2];
    Temperature.dIndexATCSecondTempOffset[31]=Temperature.fIndividualTemp[tcBh2];

    Temperature.dIndexATCInitTempOffset[ 0]=Temperature.fTempOffSet[InitTempOffset][tcAa1];
    Temperature.dIndexATCInitTempOffset[ 1]=Temperature.fTempOffSet[InitTempOffset][tcAb1];
    Temperature.dIndexATCInitTempOffset[ 2]=Temperature.fTempOffSet[InitTempOffset][tcAc1];
    Temperature.dIndexATCInitTempOffset[ 3]=Temperature.fTempOffSet[InitTempOffset][tcAd1];
    Temperature.dIndexATCInitTempOffset[ 4]=Temperature.fTempOffSet[InitTempOffset][tcAe1];
    Temperature.dIndexATCInitTempOffset[ 5]=Temperature.fTempOffSet[InitTempOffset][tcAf1];
    Temperature.dIndexATCInitTempOffset[ 6]=Temperature.fTempOffSet[InitTempOffset][tcAg1];
    Temperature.dIndexATCInitTempOffset[ 7]=Temperature.fTempOffSet[InitTempOffset][tcAh1];
    Temperature.dIndexATCInitTempOffset[ 8]=Temperature.fTempOffSet[InitTempOffset][tcBa1];
    Temperature.dIndexATCInitTempOffset[ 9]=Temperature.fTempOffSet[InitTempOffset][tcBb1];
    Temperature.dIndexATCInitTempOffset[10]=Temperature.fTempOffSet[InitTempOffset][tcBc1];
    Temperature.dIndexATCInitTempOffset[11]=Temperature.fTempOffSet[InitTempOffset][tcBd1];
    Temperature.dIndexATCInitTempOffset[12]=Temperature.fTempOffSet[InitTempOffset][tcBe1];
    Temperature.dIndexATCInitTempOffset[13]=Temperature.fTempOffSet[InitTempOffset][tcBf1];
    Temperature.dIndexATCInitTempOffset[14]=Temperature.fTempOffSet[InitTempOffset][tcBg1];
    Temperature.dIndexATCInitTempOffset[15]=Temperature.fTempOffSet[InitTempOffset][tcBh1];
    Temperature.dIndexATCInitTempOffset[16]=Temperature.fTempOffSet[InitTempOffset][tcAa2];
    Temperature.dIndexATCInitTempOffset[17]=Temperature.fTempOffSet[InitTempOffset][tcAb2];
    Temperature.dIndexATCInitTempOffset[18]=Temperature.fTempOffSet[InitTempOffset][tcAc2];
    Temperature.dIndexATCInitTempOffset[19]=Temperature.fTempOffSet[InitTempOffset][tcAd2];
    Temperature.dIndexATCInitTempOffset[20]=Temperature.fTempOffSet[InitTempOffset][tcAe2];
    Temperature.dIndexATCInitTempOffset[21]=Temperature.fTempOffSet[InitTempOffset][tcAf2];
    Temperature.dIndexATCInitTempOffset[22]=Temperature.fTempOffSet[InitTempOffset][tcAg2];
    Temperature.dIndexATCInitTempOffset[23]=Temperature.fTempOffSet[InitTempOffset][tcAh2];
    Temperature.dIndexATCInitTempOffset[24]=Temperature.fTempOffSet[InitTempOffset][tcBa2];
    Temperature.dIndexATCInitTempOffset[25]=Temperature.fTempOffSet[InitTempOffset][tcBb2];
    Temperature.dIndexATCInitTempOffset[26]=Temperature.fTempOffSet[InitTempOffset][tcBc2];
    Temperature.dIndexATCInitTempOffset[27]=Temperature.fTempOffSet[InitTempOffset][tcBd2];
    Temperature.dIndexATCInitTempOffset[28]=Temperature.fTempOffSet[InitTempOffset][tcBe2];
    Temperature.dIndexATCInitTempOffset[29]=Temperature.fTempOffSet[InitTempOffset][tcBf2];
    Temperature.dIndexATCInitTempOffset[30]=Temperature.fTempOffSet[InitTempOffset][tcBg2];
    Temperature.dIndexATCInitTempOffset[31]=Temperature.fTempOffSet[InitTempOffset][tcBh2];
   //---------------------------我是分隔線-----------------------------------------
    DoIniDataToForm(bUpdateAll);

    //Steven 20110324 : 加熱膨脹係數 (比例)-------------------------------------
    if(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)      //Steven 20140924 : 各個加熱區獨立有自己的設定值
    {
        fHotPlateExpansionCoefficient=1;
    }
    else if(Temperature.fWorkTemperBase>30)                                     //要大於常溫才可以設定
    {
        fHotPlateExpansionCoefficient=(Temperature.fWorkTemperBase-30.0)*0.000023+1.0;
    }
    else
    {
        fHotPlateExpansionCoefficient=1;
    }
    //Steven 20110324 End------------------------------------------------------

    fLotInfo->SetATCFormVisible();                                              //Steven 20160217 : For ATC7.0
    // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- `COM2->
    // ATCInitialTask()`. golden `TCOM2Shim` (atester_shims.h:364) is scoped
    // to exactly the surface the W5 tester-comm engine derefs
    // (bCCDDummyRum/DoReleaseAndInspEnd/...); ATCInitialTask is not among
    // them (matches this shim's own documented "scoped to exactly what's
    // touched" policy, cited verbatim by forms/fDynamicTemp.h's identical
    // COM2 gap note for a different member of the same shim).
#if 0 // GATE(dep-COM2-ATCInitialTask)
    COM2->ATCInitialTask();                                                     //jou 2012-03-14 ATC
#endif // GATE(dep-COM2-ATCInitialTask)
    ATKRecipeInfo->SaveFile();                                                  //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案
}

//---------------------------------------------------------------------------
// golden :3145-3669. GATE(G-FTestIF) at the two `FTestIF->ReadTestIFFile()`
// call sites (golden TFTestIF has no facade anywhere in this port -- see
// facade header G-FTestIF).
//---------------------------------------------------------------------------
void TfTemp_Set::DoIniDataToForm(bool bUpdateAll)
{
    int iSel=0;
    AnsiString S="";
    // AI(W906-FW3-TempSet-WA) 20260820: GATE(G-FTestIF) -- see banner.
#if 0 // GATE(G-FTestIF)
    FTestIF->ReadTestIFFile();                                                  //Steven 20180815 : Initial Temp Offset
#endif // GATE(G-FTestIF)

    if(bUpdateAll==true)
    {
        cbEnableIndividualMode->Checked=Temperature.bUseIndividualTemp;         //Steven 20140924 : 各個加熱區獨立有自己的設定值
        chkSoakTimeReduce     ->Checked=Temperature.bUseInitialDelayAsSoakTime;                                         //Steven 20170329 (wei) : 使用initial delay當 Soak time
        chkTempErrorAction    ->Checked=Temperature.bTempAlarmBinNeedToError;   //Steven 20251022 : Temp alarm need put to error bin

        if(ATC_SYSTEM<=eATC60)                                                  //2014-05-30    Dell    for ATC6.0
        {
            if(rgIndexHeatMode->Items->Count==5)                                //JerryYang 20161222 (jou) 中壢日月光先不開Head + Chamber + Socket, 怕電流超過20A
                rgIndexHeatMode->Items->Add("Head + Chamber + Socket");         //kevin 20131209
            rbATCActiveOn->Checked  =Temperature.bATCActiveCooling;
            chk_UseTj->Checked      =Temperature.bUseTjFunction;
            rg_TjMode->ItemIndex    =Temperature.iTjMode;
            edt_TjAvgTimes->Text    =Temperature.iTjAvgTimes;
        }

        if(IniConfig.bOnlyRoomOrHot)                                            //Steven 20110914
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 1);
        else
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 2);

        if(IniConfig.bI03AmbientTempControl)                                    //kevin 20140918 恆溫控制
        {
            iSel=CheckRange(Temperature.iMachineTempMode, 0, 3);
        }
        rgTemperatureMode->ItemIndex=iSel;
        iTempMoldSet=iSel;                                                      //20140217  wei  Barcode Reader

        edWorkTemp->Text        =FormatFloat("0.0", Temperature.fWorkTemperBase);
        cbUseAbitCHK->Checked   =Temperature.bUseAbitCHK;
        rgAmbCheck->ItemIndex   =Temperature.iAbitCHKStatus;
        edAmbTemp->Text         =FormatFloat("0.0", Temperature.fAbitTemp);
        chkAmbUsingAFan->Checked=Temperature.bAmbUsingAFan;
        edSoakTime->Text        =FormatFloat("0.0", Temperature.fSoakTime);
        edJamSoakTime->Text     =FormatFloat("0.0", Temperature.fJamSoakTime);
        edInitialWaitTime->Text =FormatFloat("0.0", Temperature.fInitialWaitTime);
        edAbitInitWaitTime->Text=FormatFloat("0.0", Temperature.fAbitInitWaitTime);
        edAbitColdTime->Text    =FormatFloat("0.0", Temperature.fAbitColdTime);
        edATCAmbTemp->Text      =FormatFloat("0.0", IniConfig.dATCAmbientTemperature);                                  //wei 20151013  by Setup File ATC Ambient Temp set
        edtHeatGunTempATC->Text =FormatFloat("0.0", IniConfig.dHeatGunTempATC);                                         //JerryYang 20220408 : add for ATC3.5
        rgShuttleSoakTime->ItemIndex=Temperature.iShuttleSoakTimeMode;

        cbWaitDewPoint->Checked         =Temperature.bWaitDewPoint;             //Steven 20191017 : 露點計
        edDewPointRange->Text           =Temperature.dDewPointRange;
        edDewPointAlarmInterval->Text   =Temperature.iDewPointAlarmInterval;

        cbBlowLB->Checked         =Temperature.bLBCoolingAirOn;                 //JerryYang 20220923 : LB吹氣function
        edtLBAirOnTemp->Text      =Temperature.dLBAirOnTemp;
        edLBTempAlarmTime->Text   =Temperature.iLBTempAlmInterval;

        for(int i=0; i<tcTotalCount; i++)                                       // 將檔案讀到的限制值存放進各頻道的容器中
        {
            myTempPal[i]->iOffsetByRecipeMaxLimit = Temperature.iOffsetByRecipeMaxLimit;
            myTempPal[i]->iOffsetByRecipeMinLimit = Temperature.iOffsetByRecipeMinLimit;
            myTempPal[i]->edOffset->Text=FormatFloat("0.0", Temperature.fTempOffSet[UserOffSet][i]);
            myTempPal[i]->edIndiTemp->Text=FormatFloat("0.0", Temperature.fIndividualTemp[i]);                          //Steven 20140924 : 各個加熱區獨立有自己的設定值
        }

        if(ATC_SYSTEM>eATC60)                                                   //2014-05-30    Dell    for ATC6.0
        {
            if(Temperature.iIndexHeatMode!=HeadOnly)                            //Steven 20240426 : unmark
                rgIndexHeatMode->ItemIndex=HeadSocket;
            else
                rgIndexHeatMode->ItemIndex=HeadOnly;

            if(ATC_SYSTEM!=eNonChamber && ATC_SYSTEM!=eATC30)                   //20141204 ChungHung add for ATC3.0   //Steven 20140314 : For HT9045WA
            {
                if(ATC_SYSTEM!=eATCSiliconType)
                {
                    rbTemperatureSetOn->Checked=Temperature.bATCTemperatureSet;
                }
                rbHandlerStartOn->Checked=Temperature.bATCHandlerStart;
                rbTestStartOn->Checked=Temperature.bATCTestStrat;
                edtATCSP2->Text=Temperature.dATCSP2;
                rbATCActiveOn->Checked=Temperature.bATCActiveCooling;
                edtATCInPC1->Text=Temperature.dATCInPC[0];
                edtATCInPC2->Text=Temperature.dATCInPC[1];
                edtATCInPC3->Text=Temperature.dATCInPC[2];
                edtATCInPC4->Text=Temperature.dATCInPC[3];
                edATCInitialOffset1->Text=Temperature.ATCInitialOffset[0];      //Steven 20151006 : Initial Temp Offset for ATC
                edATCInitialOffset2->Text=Temperature.ATCInitialOffset[1];      //Steven 20151006 : Initial Temp Offset for ATC
                edATCInitialOffset3->Text=Temperature.ATCInitialOffset[2];      //Steven 20151006 : Initial Temp Offset for ATC
                edATCInitialOffset4->Text=Temperature.ATCInitialOffset[3];      //Steven 20151006 : Initial Temp Offset for ATC
                edATCContFailOffset1->Text=Temperature.dATCConFailOffset[0];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                edATCContFailOffset2->Text=Temperature.dATCConFailOffset[1];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                edATCContFailOffset3->Text=Temperature.dATCConFailOffset[2];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                edATCContFailOffset4->Text=Temperature.dATCConFailOffset[3];    //Steven 20151123 : Continue Fail Temp Offset for ATC
                cbEnableATCConsFailOffset->Checked=Temperature.bEnableATCConFailOffset;                                 //Steven 20151123 : Continue Fail Temp Offset for ATC
                edATCContFailOffsetCnt->Text=Temperature.iATCConFailOffsetCount;                                        //Steven 20151123 : Continue Fail Temp Offset for ATC
                edATCQAModeOffset1->Text=Temperature.dATCQAModeOffset[0];       //Steven 20151125 : QA Mode Temp Offset for ATC
                edATCQAModeOffset2->Text=Temperature.dATCQAModeOffset[1];       //Steven 20151125 : QA Mode Temp Offset for ATC
                edATCQAModeOffset3->Text=Temperature.dATCQAModeOffset[2];       //Steven 20151125 : QA Mode Temp Offset for ATC
                edATCQAModeOffset4->Text=Temperature.dATCQAModeOffset[3];       //Steven 20151125 : QA Mode Temp Offset for ATC
                cbEnableATCQAModeOffset->Checked=Temperature.bEnableATCQAModeOffset;                                    //Steven 20151125 : QA Mode Temp Offset for ATC

                cbATCTestTimeOffset->Checked=Temperature.bEnableATCTestTimeOffset;                                      //Steven 20160216 : 測試時間太短也要Offset
                edATCTestTimeOffset->Text=Temperature.iATCTestTimeOffsetTime;   //Steven 20160216 : 測試時間太短也要Offset
                edATCTestTimeOffset1->Text=Temperature.dATCTestTimeOffset[0];   //Steven 20160216 : 測試時間太短也要Offset
                edATCTestTimeOffset2->Text=Temperature.dATCTestTimeOffset[1];   //Steven 20160216 : 測試時間太短也要Offset
                edATCTestTimeOffset3->Text=Temperature.dATCTestTimeOffset[2];   //Steven 20160216 : 測試時間太短也要Offset
                edATCTestTimeOffset4->Text=Temperature.dATCTestTimeOffset[3];   //Steven 20160216 : 測試時間太短也要Offset

                edChillerTemp->Text=Temperature.iATCChillerTemp;
                edAtcFileName->Text=Temperature.ATCFileName;                    //Ifor 20160111 顯示目前設定receipe 檔案名稱
                cbbATC_RecipeFile->Text=Temperature.ATCFileName;                //Ifor 20160111 顯示目前設定receipe 檔案名稱
                rgATC60AirOn->ItemIndex =Temperature.iATC60Air;

                if(CosFunction.bUseOldATCTempOffset==true)                      //JerryYang 20190123 ATC Offset UI使用舊版
                {
                    for(int i=0; i<32; i++)
                    {
                        ATCOffsetEdit[i]->Text=Temperature.dATCTempOffset[i];
                    }
                }
                else
                {
                    if(ATC_SYSTEM==eNewATCSystem &&                             //Ifor 20190930 ATC 功能開啟才使用ATC Temp Offset檔案
                       Temperature.bATCActiveCooling==true)                     // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180814
                    {
                        for(int i=0; i<tcTotalCount; i++)
                        {
                            for(int j=0; j<32; j++)
                            {
                                if(myTempPal[i]->iIndexTag==j)
                                {
                                    myTempPal[i]->edOffset->Text=Temperature.dATCTempOffset[j];
                                    Temperature.fTempOffSet[UserOffSet][i]=Temperature.dATCTempOffset[j];               //Ifor 20200106 fix ATC offset
                                }
                            }
                        }
                    }

                    if(CosFunction.bUseSecondATCTempOffset==true)               //Ifor 20241014 Use Second ATC Temp Offset
                    {
                        for(int i=0; i<32; i++)
                        {
                            ATCOffsetEdit[i]->Text=Temperature.dATCSecondTempOffset[i];
                        }
                    }
                }

                //Ifor 20160421 第二點溫度Sensor設定改至工作檔
                cbATCReferTempSensor->Checked=Temperature.bUseReferTempSensor;
                //Ifor 20190306 : add Package Offset 三點校正
                //==>
                for(int i=0; i<3; i++)
                {
                    ATCPackageOffsetEdit[i] ->Text=Temperature.dATCPackageOffset[i];
                    ATCPackageTempEdit[i]   ->Text=Temperature.dATCPackageTemp[i];
                }
                //<==
                //Ifor 20190306 : add Package Offset 三點校正

                //Ifor 20190328 : add TJ Temp Over Range
                //==>
                 edTJTempRange_High ->Text=Temperature.dTJTempRange_High;
                 edTJTempRange_Low  ->Text=Temperature.dTJTempRange_Low;
                //<==
                //Ifor 20190328 : add TJ Temp Over Range
                cbATCTempRise       ->Checked=Temperature.bEnableTempRise;
                edt_TempRiseTemp    ->Text=Temperature.dTempRiseTemp;
                edt_TempRise_Delay  ->Text=Temperature.dTempRiseDelay;
                edtATC_Type->Text=Temperature.ATCTypeName;                      //Ifor 20230828 add:送ATC Type 資料給GPIB
                cbUseTC2Offset->Checked=Temperature.bUseTC2Offset;              //KenHsieh 20240311 : add Tc2 Offset
            }
        }
        else
        {
            rgIndexHeatMode->ItemIndex  =Temperature.iIndexHeatMode;            //Steven 20090926
        }

        edInitialStart1Time->Text=Temperature.iInitialStart1Time;
        edInitialStart2Time->Text=Temperature.iInitialStart2Time;

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            Temperature.iIndexSoakTime = 0;
            Temperature.iOSTime = 0;
        }                                                                       //kevin 20210511 ASE KH  JL 說關閉
        edIndexSoakTime->Text   =Temperature.iIndexSoakTime;                    //2013-11-27   Dell    需要做Index soak time
        edOSTime->Text          =Temperature.iOSTime;                           //2013-11-27   Dell    需要做Index soak time
        cbUseFixTemp->Checked   =Temperature.bUseFixTemp;                       //Steven 20131025 : 使4固定的DUT溫度
        edFixedTemp->Text       =Temperature.dFixedTemp;                        //Steven 20131025 : 固定的DUT溫度值
        cbShowFixTemp->Checked  =Temperature.bShowFixedTemp;                    //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值

        if(CosFunction.bTempCalByRecipe==true)                                  //jou 20220725 : Temperature calibration by recipe
            chkTempCalByRecipe->Checked=Temperature.bTempCalByRecipe;
    }

    if(Tri_Temp_Machine==1 ||
       (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
    {
        edt_SetAirstreamTemperatureRang_Socket->Text        = AnsiString(Temperature.dSetAirstreamTemperatureRang_Socket);
        edt_SetAirstreamTemperatureRang_Index->Text         = AnsiString(Temperature.SetAirstreamTemperatureRang_Index);
        cbEnableTesterDryAirControl->Checked                = Temperature.bEnableTesterDryAirControl;
        chk_EnableArm_1_Air->Checked                        = Temperature.bEnableArm_1_Air;
        chk_EnableArm_2_Air->Checked                        = Temperature.bEnableArm_2_Air;
        chk_EnableSocket_Air->Checked                       = Temperature.bEnableSocket_Air;
        edt_AirVolumeLmt_Index->Text                        = Temperature.iAirVolumeLmt;
        edt_AirVolumeLmt_Socket->Text                       = edt_AirVolumeLmt_Index->Text;
        edtSetTempature2AirMachine->Text                    = AnsiString(Temperature.dAirStreamSocket_Offset);          //Ztex 2023.12.19 Add Air Stream Offset
        edt_SetIndexAirstreamTemp->Text                     = AnsiString(Temperature.dAirStreamIndex_Offset);           //Ztex 2023.12.19 Add Air Stream Offset
        edt_Defrost_Time_Too_Lower->Text                    = AnsiString(Temperature.iDefrostTimeTooLower);             //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
    }

    //Steven 20180817 : Boost Function
    //==>
    chkBoostFunction->Checked   =Temperature.bBoostFuncttion;
    chkLBTempFunction->Checked  =Temperature.bLBTempFunction;
    edtIdleTime_Long->Text      =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBMax]);
    edtBoostOffset_Long->Text   =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBMax]);
    edtBoostDuration_Long->Text =FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBMax]);
    edtPostBoost_Long->Text     =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBMax]);

    if(iTriggerBoostFunction!=-1)
    {
        edtIdleTime_Mid->Text       =FormatFloat("0.0", iBoostEotToSotTime);
        edtBoostOffset_Mid->Text    =FormatFloat("0.0", Temperature.dBoostOffset[iTriggerBoostFunction]);
        edtBoostDuration_Mid->Text  =FormatFloat("0.0", Temperature.dBoostDuration[iTriggerBoostFunction]);
        edtPostBoost_Mid->Text      =FormatFloat("0.0", Temperature.dPostBoostDuration[iTriggerBoostFunction]);
    }
    else if(iTriggerBoostFuncBack!=-1)
    {
        edtIdleTime_Mid->Text       =FormatFloat("0.0", iBoostEotToSotTime);
        edtBoostOffset_Mid->Text    =FormatFloat("0.0", Temperature.dBoostOffset[iTriggerBoostFuncBack]);
        edtBoostDuration_Mid->Text  =FormatFloat("0.0", Temperature.dBoostDuration[iTriggerBoostFuncBack]);
        edtPostBoost_Mid->Text      =FormatFloat("0.0", Temperature.dPostBoostDuration[iTriggerBoostFuncBack]);
    }
    else
    {
        edtIdleTime_Mid->Text       =0;
        edtBoostOffset_Mid->Text    =0;
        edtBoostDuration_Mid->Text  =0;
        edtPostBoost_Mid->Text      =0;
    }
    edtIdleTime_Short->Text     =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBMin]);
    edtBoostOffset_Short->Text  =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBMin]);
    edtBoostDuration_Short->Text=FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBMin]);
    edtPostBoost_Short->Text    =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBMin]);
    rgBoostMode->ItemIndex      =Temperature.iBoostFunctionMode;

    chkLBInitOffset->Checked  =Temperature.bEnableBoostOffset[Temperature.eBLBI];
    edtIdleTime_LB->Text      =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBI]);                       //Steven 20181023 : LB溫度
    edtBoostOffset_LB->Text   =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBI]);
    edtBoostDuration_LB->Text =FormatFloat("0.0", Temperature.dBoostDuration[Temperature.eBLBI]);
    edtPostBoost_LB->Text     =FormatFloat("0.0", Temperature.dPostBoostDuration[Temperature.eBLBI]);

    chkBoostOffset->Checked   =Temperature.bEnableBoostOffset[Temperature.eBLBL];
    edtLBTempMin->Text        =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBL]);
    edtLBTempOffset->Text     =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBL]);
    edtThreshold->Text        =FormatFloat("0.0", Temperature.dThreshold);      //Steven 20190928 : L/B升溫的門檻值
    edtLBTimeOut->Text        =FormatFloat("0.0", Temperature.dBoostTimeOut);   //Steven 20181222 : Add LB升溫的Time out

    chkBoostOffset->Checked   =Temperature.bEnableBoostOffset[Temperature.eBLBB];
    edtBoostTempMin->Text     =FormatFloat("0.0", Temperature.dBoostIdleTime[Temperature.eBLBB]);                       //Steven 20181023 : LB溫度
    edtBoostOffset->Text      =FormatFloat("0.0", Temperature.dBoostOffset[Temperature.eBLBB]);
    //<==
    //Steven 20180817 : Boost Function

    edChamberCoolTemp->Text=FormatFloat("0.0", Temperature.fChamberCoolTemp);   //Steven 20111209 : Chamber降溫溫度
    chkTesterDock->Checked=Temperature.bUseTesterDocking;                       //jou 2014-04-25 Tester Docking Wait delay time
    edtTestDock->Text=Temperature.iUseTesterDocking;                            //jou 2014-04-25 Tester Docking Wait delay time    //Richard 20230322 : limit修改

    rb1Point->Checked   =Temperature.iTempMode & 0x01;
    rb2Point->Checked   =Temperature.iTempMode & 0x02;
    rb3Point->Checked   =Temperature.iTempMode & 0x04;
    rb5Point->Checked   =Temperature.iTempMode & 0x08;                          //kevin 20140918
    rb6Point->Checked   =Temperature.iTempMode & 0x10;                          //Ztex 2024.07.27 Add 6 Point Temperature Offset    //Ifor 20250704 Fix 0x16 => 0x10

    edAbLow->Text           =FormatFloat("0.0",Temperature.fAmbientHotLowBase);                                         //kevin 20140918
    edAbMid->Text           =FormatFloat("0.0",Temperature.fAmbientHotMiddBase);                                        //kevin 20140918
    edLowBase->Text         =FormatFloat("0.0",Temperature.fLowBase);
    edMidBase->Text         =FormatFloat("0.0",Temperature.fMiddBase);
    edHighBase->Text        =FormatFloat("0.0",Temperature.fHighBase);
    edSHighBase->Text       =FormatFloat("0.0",Temperature.fSHighBase);         //Ztex 2024.07.27 Add 6 Point Temperature Offset
    rgZ1Down->ItemIndex     =Temperature.bZ2DownSocket;                         //kevin 201312111 使用加熱z2在下
    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edLowbase->Text        =FormatFloat("0.0", Temperature.fTempOffSet[LowBase][i]);
        myTempPal[i]->edBase->Text           =FormatFloat("0.0", Temperature.fTempOffSet[MidBase][i]);
        myTempPal[i]->edHighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[HigBase][i]);
        myTempPal[i]->edInitTempOffset->Text =FormatFloat("0.0", Temperature.fTempOffSet[InitTempOffset][i]);           //Steven 20141117 : 起測時溫度要補Offset
        myTempPal[i]->edLow->Text  =FormatFloat("0.0", Temperature.fTempOffSet[AmbientHotLow][i]);                      //kevin 20140918 恆溫控制 低 offset
        myTempPal[i]->edMid->Text  =FormatFloat("0.0", Temperature.fTempOffSet[AmbientHotMid][i]);                      //kevin 20140918 恆溫控制 中 offset
        myTempPal[i]->edEOTTempOffset->Text  =FormatFloat("0.0", Temperature.fTempOffSet[TestOverTimeTempOffset][i]);   //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        myTempPal[i]->edSHighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[SHigBase][i]);                //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            myTempPal[i]->edKit_Lowbase->Text        =FormatFloat("0.0", Temperature.fTempOffSet[KitLowBase][i]);
            myTempPal[i]->edKit_Base->Text           =FormatFloat("0.0", Temperature.fTempOffSet[KitMidBase][i]);
            myTempPal[i]->edKit_HighBase->Text       =FormatFloat("0.0", Temperature.fTempOffSet[KitHigBase][i]);
            myTempPal[i]->edKit_Low->Text  =FormatFloat("0.0", Temperature.fTempOffSet[KitAmbientHotLow][i]);           //kevin 20140918 恆溫控制 低 offset
            myTempPal[i]->edKit_Mid->Text  =FormatFloat("0.0", Temperature.fTempOffSet[KitAmbientHotMid][i]);           //kevin 20140918 恆溫控制 中 offset
        }
    }

    cbTempOffset->Checked           =Temperature.bEnableTempOffsetForInitial;   //Steven 20141117 : 起測時溫度要補Offset
    edTempOffsetCount->Text         =Temperature.iCintactCntForTempOffsetAtInitial;                                     //Steven 20141117 : 起測時溫度要補Offset
    edTempDownContactDelay->Text    =Temperature.iCintactDelayCntForInitTempOffset;                                     //Steven 20141117 : 起測時溫度要補Offset
    edTempReadyRange->Text          =Temperature.iTempReadyRange;               //Sam 20231214 : Temp offset use ready temp range

    chkChamberBoost->Checked        =Temperature.bEnableChamberBoost;           //Steven 20191128 : Chamber Boost Function
    edtChamberBoostTime->Text       =Temperature.iChamberBoostTime;
    edtChamberBoostOffset->Text     =Temperature.iChamberBoostOffset;

    if(CUSTOMER_CODE==CC_SCS)                                                   //jou 2015-12-10 SCS 要求把ATC PID頁面關掉不顯示
    {
        cdATCUsePIDCtrl->Checked=false;
        Temperature.bUsePIDControl=false;
    }
    else
    {
        //Ifor 20150910 :ATC PID 顯示前轉換
        cdATCUsePIDCtrl->Checked  = Temperature.bUsePIDControl;
        edtATCPIDOffset_MinP->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[0]/100.0);
        edtATCPIDOffset_MinI->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[1]/100.0);
        edtATCPIDOffset_MinD->Text = FormatFloat("0.00", Temperature.iATC_PID_Min_Offset[2]/100.0);
        edtATCPIDOffset_MaxP->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[0]/100.0);
        edtATCPIDOffset_MaxI->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[1]/100.0);
        edtATCPIDOffset_MaxD->Text = FormatFloat("0.00", Temperature.iATC_PID_Max_Offset[2]/100.0);
    }

    cbHeaterGun->Checked=Temperature.bActiveHeatGun;                            //Steven 20120525 : 熱風槍
    chkUseCDAOnly->Checked=Temperature.bUseCDAOnly;                             //Steven 20181012 : 使用熱風槍吹冷風
    edtHotGunFLowLimit_H->Text     =FormatFloat("0", Temperature.iHotGunFLowLimit_H);                                   //KaiChen 20190729 ：Hot Gun Flow
    edtHotGunFLowLimit_L->Text     =FormatFloat("0", Temperature.iHotGunFLowLimit_L);                                   //KaiChen 20190729 ：Hot Gun Flow

    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edSingleLimit->Text=FormatFloat("0.0", IniConfig.dSingleTempLimit[i]);
    }
    edAmbHotGuartbent->Text     =FormatFloat("0.0", Temperature.fAmbientHotGuartbent);                                  //kevin 20140918 恆溫控制溫度
    chkShuttleNoHeatUp->Checked =Temperature.bShuttleNoHeatUp;                  //Steven 20180815 : Amb Ctr mode, shuttle no heat up
    chkSLKNoHeatUp->Checked     =Temperature.bSLKNoHeatUp;                      //Steven 20230221 : Amb Ctr mode, SLK no heat up
    cbUseAmbGuard->Checked      =Temperature.bAmbientGuardbandCheck;            //kevin 20180115 (Steven) add ambient grand bient check
    edAmbGuardband->Text        =FormatFloat("0", Temperature.iAmbGuardband);   //kevin 20180115 (Steven) add ambient grand bient check

    rbATC70ActiveOn->Checked    =Temperature.bATC70Active;                      //Eliot 2015_0105
    rbATC7TSDFunction->Checked  =Temperature.bATC7TSDFunction;                  //Eliot 2015_0105
    edTSDTimeOut->Text          =Temperature.dATC7TSDTimeOut;                   //Steven 20160604 : by site TSD

    edArm1Offset->Text= Temperature.Arm1Offset;
    edArm2Offset->Text= Temperature.Arm2Offset;

    edArm1NoFullsiteOffset_1->Text=Temperature.Arm1NoFullsiteOffset_1;          //wei 20160107 No FullSite Add Offset
    edArm1NoFullsiteOffset_2->Text=Temperature.Arm1NoFullsiteOffset_2;          //wei 20160107 No FullSite Add Offset
    edArm1NoFullsiteOffset_3->Text=Temperature.Arm1NoFullsiteOffset_3;          //wei 20160107 No FullSite Add Offset
    edArm1NoFullsiteOffset_4->Text=Temperature.Arm1NoFullsiteOffset_4;          //wei 20160107 No FullSite Add Offset
    edArm1NoFullsiteOffset_5->Text=Temperature.Arm1NoFullsiteOffset_5;          //wei 20160107 No FullSite Add Offset
    edArm2NoFullsiteOffset_1->Text=Temperature.Arm2NoFullsiteOffset_1;          //wei 20160107 No FullSite Add Offset
    edArm2NoFullsiteOffset_2->Text=Temperature.Arm2NoFullsiteOffset_2;          //wei 20160107 No FullSite Add Offset
    edArm2NoFullsiteOffset_3->Text=Temperature.Arm2NoFullsiteOffset_3;          //wei 20160107 No FullSite Add Offset
    edArm2NoFullsiteOffset_4->Text=Temperature.Arm2NoFullsiteOffset_4;          //wei 20160107 No FullSite Add Offset
    edArm2NoFullsiteOffset_5->Text=Temperature.Arm2NoFullsiteOffset_5;          //wei 20160107 No FullSite Add Offset

    edTempAlwaysSameAlarm->Text=ATCInterfaceForm->iCheckSameTempTime;           //Ifor 20160308 設定溫度相同連續時間 Alarm

    edSocketAirCoolingOn->Text =Temperature.dSocketAirCoolingOnTimer;           //jou 2016-04-28 Socket Air Cooling contact count trun on
    edSocketAirCoolingOff->Text=Temperature.dSocketAirCoolingOffTimer;          //jou 2016-04-28 Socket Air Cooling contact count trun on

    cbByPassChamber->Checked   =Temperature.bByPassChamber;                     //KenHsieh 20230301 : By Pass Chamber
    edATCOfsTime->Text=Temperature.iATC_OFS_ST;                                 //Ifor 20160308 設定溫度相同連續時間 Alarm

    cbPowerFollow_Enable->Checked  =Temperature.bPowerFollower_Enable;          //KenHsieh 20240216 : add ATC Power Follow Function
    edtPowerFollow_FullPower->Text =Temperature.iPowerFollower_FullPower;       //KenHsieh 20240216 : add ATC Power Follow Function
    edtPowerFollower_PFSlope->Text =FormatFloat("0.000", Temperature.dPowerFollower_PFSlope);                           //KenHsieh 20240216 : add ATC Power Follow Function
    edtPowerFollower_WGain->Text   =FormatFloat("0.000", Temperature.dPowerFollower_WGain);                             //KenHsieh 20240216 : add ATC Power Follow Function
    edtPowerFollower_Many2one->Text=Temperature.iPowerFollower_Many2one;        //KenHsieh 20240216 : add ATC Power Follow Function
//Steven 20180815 : Initial Temp Offset-----------------------------------------
//==>
    cbEveryFirstDeviceUseInitialDelay->Checked          =TestIF_File.bEveryFirstDeviceUseInitialDelay;
    cbUseOtherArmToTestAfterInitialDelay->Checked       =TestIF_File.bUseOtherArmToTestAfterInitialDelay;               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    cbAfterShowAlarmMessageUseInitialDelay->Checked     =TestIF_File.bAfterShowAlarmMessageUseInitialDelay;
    cbWhenHappenTestedTimeBelowUseInitialDelay->Checked =TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay;
    cbAfterAutoCleanFunctionUseInitialDelay->Checked    =TestIF_File.bAfterAutoCleanFunctionUseInitialDelay;            //ChungHung 20141017 add for SCK Add for TSMC Device
    cbAfterOpenHeatDoorUseInitialDelay->Checked         =TestIF_File.bAfterOpenHeatDoorUseInitialDelay;                 //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    edBelowSec->Text                                    =FormatFloat("0.00", TestIF_File.iEveryFirstDeviceUseInitialDelay);
    edtInitialDelay_1->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay);                //ChungHung 20141210 add for SCK want to every event have delay
    edtInitialDelay_2->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_2);              //ChungHung 20141210 add for SCK want to every event have delay
    edtInitialDelay_3->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_3);              //ChungHung 20141210 add for SCK want to every event have delay
    edtInitialDelay_4->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_4);              //ChungHung 20141210 add for SCK want to every event have delay
    edtInitialDelay_5->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_5);              //ChungHung 20141210 add for SCK want to every event have delay

    cbWhenPressStopOverUseInitialDelay->Checked         =TestIF_File.bWhenPressStopOverUseInitialDelay;                 //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    edOverSec->Text                                     =FormatFloat("0.00", TestIF_File.iWhenPressStopOver);           //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    edtInitialDelay_6->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_6);              //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    cbWhenNoFullSiteUseInitialDelay->Checked            =TestIF_File.bWhenNoFullSiteUseInitialDelay;                    //wei 20151228 No FullSite delay
    edtInitialDelay_7->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_7);              //wei 20151228 No FullSite delay

    cbTestFinishToNextTestOver->Checked                 =TestIF_File.bTestFinishToNextTestOver;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    iTestFinishToNextTestOver->Text                     =FormatFloat("0.00", TestIF_File.iTestFinishToNextTestOver);    //kevin 20160310
    edtInitialDelay_8->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_8);              //kevin 20160310

    cbOTDUnlockDelay->Checked                           =TestIF_File.bOTDUnlockDelay;                                   //Steven 20160818 : OTD打開Delay
    edtInitialDelay_9->Text                             =FormatFloat("0.00", TestIF_File.iInitialDelay_9);              //Steven 20160818 : OTD打開Delay
    cbTestStartToNextTestStart->Checked                 =TestIF_File.bTestStartToNextTestStart;                         //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    iTeststartToNextTestStart->Text                     =FormatFloat("0.00", TestIF_File.dTeststartToNextTestStart);    //kevin 20160310
    edtInitialDelay_10->Text                            =FormatFloat("0.00", TestIF_File.dInitialDelay_10);             //kevin 20181031 : SOT 超過時間Delay

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    edtInitialDelay_1_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_1_RT);
    edtInitialDelay_2_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_2_RT);
    edtInitialDelay_3_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_3_RT);
    edtInitialDelay_4_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_4_RT);
    edtInitialDelay_5_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_5_RT);
    edtInitialDelay_6_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_6_RT);
    edtInitialDelay_7_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_7_RT);
    edtInitialDelay_8_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_8_RT);
    edtInitialDelay_9_RT->Text                          =FormatFloat("0.00", TestIF_File.dInitialDelay_9_RT);
    edtInitialDelay_10_RT->Text                         =FormatFloat("0.00", TestIF_File.dInitialDelay_10_RT);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    cbEnableTJFunction->Checked=Temperature.bEnableTJFunction;                  //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    rgTjMapType->ItemIndex=Temperature.iTjSiteMapType;                          //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    if(Temperature.iTjSiteMapType==2)
    {
        sgTjMap->Visible=true;
    }
    else
    {
        sgTjMap->Visible=false;
    }

    edtSetTJ_Offset->Text=FormatFloat("0.00", Temperature.fSetTJ_Offset);
    edtSetTJ_Slope->Text=FormatFloat("0.00", Temperature.fSetTJ_Slope);

    edtInputVLow->Text=FormatFloat("0.00", Temperature.fTjInputVLow);
    edtInputVHigh->Text=FormatFloat("0.00", Temperature.fTjInputVHigh);
    edtDelayAfterSOT->Text=FormatFloat("0.00",Temperature.dDelayAfterSOT);
    edContinuousSec->Text=FormatFloat("0.00",Temperature.dDelayAfterSOTContinue);                                       //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    cbTjWatchdog->Checked= Temperature.bTjWatchdog;

    for(int i=0; i<16; i++)
    {
        sgTjMap->Cells[i+1][1]=AnsiString(sATC_CH_Tj->Strings[i]);
    }

    //Ifor 20190730 : add FFC Temperature Offset
    //==>
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<10; j++)
        {
            ATC_FFCOffsetOnTimeEdit[i][j]->Text   =Temperature.iATC_FFCTimeOn[i][j];
            ATC_FFCOffsetOffTimeEdit[i][j]->Text  =Temperature.iATC_FFCTimeOff[i][j];
            ATC_FFCOffsetEdit[i][j]->Text         =Temperature.dATC_FFCOffset[i][j];
            ATC_FFCPointUse[i][j]->Checked        =Temperature.bATC_FFCPointUse[i][j];
        }
    }
    cbFFCOffsetEnable->Checked  =Temperature.bATC_FFCEnable;

    cbMultiZoneFunction->Checked=Temperature.bMultiZoneEnable;                  //wei 20240617 Multi Zone
    for(int i=0; i<4; i++)
    {
        ZoneTempUse[i]->Checked=Temperature.bZoneTempEnable[i];
        ZoneTempSetting[i]->Text=Temperature.dZoneTempSetting[i];
    }

    edtATC_HotGunTemp->Text     =FormatFloat("0.0", Temperature.dATC_HotGunTemp);                                       //Ifor 20220124 add: ATC Hot Air
    edtATC_HotGunTime->Text     =FormatFloat("0.0", Temperature.dATC_HotGunTime);                                       //Ifor 20220124 add: ATC Hot Air

    if(IniConfig.bL22Enable3SigmaTempMonitor ==true)
    {
        gbcb3SigmaTempMonitior->Visible = true;
        cb3SigmaTempMonitior_Enable->Checked  = Temperature.b3SigmaTempMonitior_Enable;                                 //kevin 20200521 add 3 Sigma 溫度記錄比對
        if(Temperature.i3SigmaTempMonitior_SetCoun<5)
        {
            Temperature.i3SigmaTempMonitior_SetCoun = 5;
        }
        edt3SigmaTempMonitior_SetCount->Text            = IntToStr(Temperature.i3SigmaTempMonitior_SetCoun);            //kevin 20200521 add 3 Sigma Index Contract time
        edt3SigmaTempMonitior_Set3xSigmaValue->Text     = FloatToStr(Temperature.d3SigmaTempMonitior_Set3xSigmaValue);  //kevin 20200521 add 3 Sigma 客戶比對誤差值
    }
    else
    {
        gbcb3SigmaTempMonitior->Visible = false;
        Temperature.b3SigmaTempMonitior_Enable = false;
    }
    //Hmy 20200515 Copy//Hmy 20200510 Add Enable 3 Sigma Temp Monitor//Hmy 20200510 Add Enable 3 Sigma Temp Monitor <-
    //<==
    //Ifor 20190730 : add FFC Temperature Offset

    edLBTempHighSettingValue->Text         =FormatFloat("0.0", Temperature.dLBTempHighSettingValue);                    //Frank 20241231 : add
    edLBTempLowSettingValue->Text          =FormatFloat("0.0", Temperature.dLBTempLowSettingValue);
    cbLBTempHighAlarmEnable->Checked       =Temperature.bLBTempHighAlarm_Enable;
    cbLBTempLowAlarmEnable->Checked        =Temperature.bLBTempLowAlarm_Enable;

//<==
//Steven 20180815 : Initial Temp Offset-----------------------------------------

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        chkTriTempEnableOutSht->Checked=Temperature.bUseTriTempHeater_Ini[0];
        chkTriTempEnableIndex ->Checked=Temperature.bUseTriTempHeater_Ini[1];
        chkTriTempEnableBase  ->Checked=Temperature.bUseTriTempHeater_Ini[2];
    }                                                                           //Heater開關設定

    cbMultiSensorOffsetEnable->Checked  =Temperature.bATC_MultiSensorEnable;    //Ifor 20251203 add:ATC Multi Sensor Offset

    for(int i=0; i<4; i++)                                                      //Ifor 20251203 add:ATC Multi Sensor Offset
    {
        MultiSensorOffsetUse[i]->Checked=Temperature.bATC_MultiSensorUse[i];
    }

    for(int i=0; i<32; i++)
    {                                                                           //Ifor 20251203 add:ATC Multi Sensor Offset
        ATC_MultiSensorOffsetEdit[i]->Text=Temperature.dATC_MultiSensorOfs[i];
    }
}

//---------------------------------------------------------------------------
// golden :3671-4101. Internal gates: `rgBasePoint->Color=` (x2, TRadioGroup
// has no Color), `Image2->Visible=` (TImage no port), `->palTemp->Align=`
// (many sites, TPanel has no Align) -- all GATE(dep), see facade header.
//---------------------------------------------------------------------------
void TfTemp_Set::UpDateEdit()
{
    int x, y;

    int iShowNum[2][16]={{-1, -1, -1, -1, -1, -1, -1, -1,
                          -1, -1, -1, -1, -1, -1, -1, -1},
                         {-1, -1, -1, -1, -1, -1, -1, -1,
                          -1, -1, -1, -1, -1, -1, -1, -1}};

    int iAddr[2][16]={{tcAa1, tcBa1, tcAb1, tcBb1, tcAc1, tcBc1, tcAd1, tcBd1,
                       tcAe1, tcBe1, tcAf1, tcBf1, tcAg1, tcBg1, tcAh1, tcBh1},
                      {tcAa2, tcBa2, tcAb2, tcBb2, tcAc2, tcBc2, tcAd2, tcBd2,
                       tcAe2, tcBe2, tcAf2, tcBf2, tcAg2, tcBg2, tcAh2, tcBh2}};

    if(CosFunction.bTemp5PointKitOffset)                                        //JerryYang 20221003 Analog要求新增Kit三點offset
    {
        rgBasePoint->Visible=true;
        // AI(W906-FW3-TempSet-WA) 20260820: GATE(dep-RadioGroup-Color) --
        // vclcompat::TRadioGroup carries no `Color`.
#if 0 // GATE(dep-RadioGroup-Color)
        if(rgBasePoint->ItemIndex==0)
        {
            rgBasePoint->Color=(TColor)0x00C2B8A6;
        }
        else
        {
            rgBasePoint->Color=clMedGray;
        }
#endif // GATE(dep-RadioGroup-Color)
    }
    else
    {
        rgBasePoint->Visible=false;
    }

    for(int i=0; i<tcTotalCount; i++)
    {
        myTempPal[i]->edInitTempOffset->Visible=true;
        myTempPal[i]->edEOTTempOffset->Visible=true;
        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            if(rgBasePoint->ItemIndex==1)
            {
                myTempPal[i]->edKit_Low->Visible=true;
                myTempPal[i]->edKit_Mid->Visible=true;
                myTempPal[i]->edKit_Lowbase->Visible=true;
                myTempPal[i]->edKit_HighBase->Visible=true;
                myTempPal[i]->edKit_Base->Visible=true;

                myTempPal[i]->edKit_Low->Enabled=fSecurity->Insufficient(175, false);                                   //JerryYang 20230822 : 174->175
                myTempPal[i]->edKit_Mid->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_Lowbase->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_HighBase->Enabled=fSecurity->Insufficient(175, false);
                myTempPal[i]->edKit_Base->Enabled=fSecurity->Insufficient(175, false);

                myTempPal[i]->edLowbase->Visible=false;
                myTempPal[i]->edHighBase->Visible=false;
                myTempPal[i]->edBase->Visible=false;

                myTempPal[i]->edOffset->Visible=false;
                myTempPal[i]->edSingleLimit->Visible=false;
                myTempPal[i]->edIndiTemp->Visible=false;
                myTempPal[i]->edInitTempOffset->Visible=false;
                myTempPal[i]->edEOTTempOffset->Visible=false;
                myTempPal[i]->edSHighBase->Visible=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
            else
            {
                myTempPal[i]->edKit_Low->Visible=false;
                myTempPal[i]->edKit_Mid->Visible=false;
                myTempPal[i]->edKit_Lowbase->Visible=false;
                myTempPal[i]->edKit_HighBase->Visible=false;
                myTempPal[i]->edKit_Base->Visible=false;
                myTempPal[i]->edLow->Visible=true;
                myTempPal[i]->edMid->Visible=true;
                myTempPal[i]->edLowbase->Visible=true;
                myTempPal[i]->edHighBase->Visible=true;
                myTempPal[i]->edBase->Visible=true;

                myTempPal[i]->edOffset->Visible=true;
                myTempPal[i]->edSingleLimit->Visible=true;
                myTempPal[i]->edIndiTemp->Visible=true;
                myTempPal[i]->edInitTempOffset->Visible=true;
                myTempPal[i]->edEOTTempOffset->Visible=true;
                myTempPal[i]->edSHighBase->Visible=false;                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
            }
        }
        else
        {
            myTempPal[i]->edLow->Visible=true;
            myTempPal[i]->edMid->Visible=true;
            myTempPal[i]->edLowbase->Visible=true;
            myTempPal[i]->edHighBase->Visible=true;
            myTempPal[i]->edBase->Visible=true;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
            myTempPal[i]->edKit_Lowbase->Visible=false;
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Base->Visible=false;
        }

        for(int j=0; j<32; j++)                                                 //JerryYang 20190926 修正溫度Offset不見的問題
        {
            if(myTempPal[i]->iIndexTag==j)
            {
                myTempPal[i]->palTemp->Visible=true;
            }
        }
    }

    myTempPal[tcHotPlate3]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcHotPlate4]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcShuttle3 ]->palTemp->Visible=(Tri_Temp_Machine==1);
    myTempPal[tcShuttle4 ]->palTemp->Visible=(Tri_Temp_Machine==1);

    if(rb1Point->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edBase->Visible=false;
            myTempPal[i]->edHighBase->Visible=false;
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Base->Visible=false;                            //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(rb2Point->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edBase->Visible=false;
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Base->Visible=false;                            //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(rb3Point->Checked)
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Visible=false;
            myTempPal[i]->edMid->Visible=false;
            myTempPal[i]->edSHighBase->Visible=false;                           //Ztex 2024.07.27 Add 6 Point Temperature Offset

            myTempPal[i]->edKit_Low->Visible=false;                             //JerryYang 20221003 Analog要求新增Kit三點offset
            myTempPal[i]->edKit_Mid->Visible=false;
        }
    }
    else if(rb6Point->Checked)                                                  //Ztex 2024.07.27 Add 6 Point Temperature Offset ==>
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edLow->Visible=true;
            myTempPal[i]->edMid->Visible=true;
            myTempPal[i]->edLowbase->Visible=true;
            myTempPal[i]->edHighBase->Visible=true;
            myTempPal[i]->edBase->Visible=true;
            myTempPal[i]->edSHighBase->Visible=true;

            myTempPal[i]->edKit_Low->Visible=false;
            myTempPal[i]->edKit_Mid->Visible=false;
            myTempPal[i]->edKit_Lowbase->Visible=false;
            myTempPal[i]->edKit_HighBase->Visible=false;
            myTempPal[i]->edKit_Base->Visible=false;
        }
    }                                                                           //Ztex 2024.07.27 Add 6 Point Temperature Offset <==

    if(USE_16_HEATER==eht16Heater       ||                                      //Steven 20120606 : 16溫控器 8Site使用Hontech頭
       USE_16_HEATER==eht16HeaterEJ1N   ||
       USE_16_HEATER==eht32HeaterEJ1N   ||                                      //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 ||                                      //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
       USE_16_HEATER==eht32HeaterDTME08)                                        //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
    {
        for(int j=tcHead1; j<=tcHead4; j++)
        {
            myTempPal[j]->palTemp->Visible=false;
        }
    }

    if(USE_16_HEATER==eht16Heater       ||
       USE_16_HEATER==eht16HeaterEJ1N   ||                                      //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08)                                        //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
    {
        for(int j=tcAe1; j<=tcBh2; j++)
        {
            myTempPal[j]->palTemp->Visible=false;
        }
    }

    myTempPal[tcChamber]->palTemp->Visible=(myTempPal[tcChamber]->palTemp->Visible && ATC_SYSTEM<=eATC60);
    myTempPal[tcCCD]->palTemp->Visible=false;

    if(REAL_TIME_CCD==false)                                                    //kevin 20210803
    {
        myTempPal[tcCCD_2]->palTemp->Visible=false;
        myTempPal[tcCCD_2]->edIndiTemp->Visible=false;
    }

    if(REAL_TIME_CCD==true && RTC_TemperNumber==2)                              //Isaac 20201217 : RTC CCD增加第二組感溫
    {
        myTempPal[tcCCD_2]->palTemp->Visible=false;
    }
    myTempPal[tc2D]->palTemp->Visible=false;

    if(iSocketBaseTempCount==eDut2ea)                                           //Steven 20140116 : Socket數量,改用數字設定
    {
        myTempPal[tcDUT4]->palTemp->Visible=false;
        myTempPal[tcDUT3]->palTemp->Visible=false;
        myTempPal[tcSocket]->palTemp->Visible=false;
        // AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Image) -- Image2 has no port.
#if 0 // GATE(G-Image)
        Image2->Visible=false;
#endif // GATE(G-Image)
    }
    else if(iSocketBaseTempCount==eDut4ea)                                      //Steven 20141120 : Fixed 溫度設定
    {
        myTempPal[tcSocket]->palTemp->Visible=false;
    }

    if(Index_ESDAir ==false)                                                    //kevin 20200210 add index ESD temp
       myTempPal[tcIndexESD]->palTemp->Visible=false;

    if(CosFunction.bUseIndividulTempSet)
        DisplayTargetTempEdit(Temperature.bUseIndividualTemp);
    else
        DisplayTargetTempEdit(false);

    if(CosFunction.bUseOldATCTempOffset==false &&                               //JerryYang 20190123 ATC Offset UI使用舊版
       bNeedChange==true)
    {
        if(ATC_SYSTEM==eNewATCSystem &&
           Temperature.bATCActiveCooling)                                       //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
        {
            for(int i=tcAa1; i<=tcBd2; i++)
            {
                if(CosFunction.bATCUseTempAdjustment==true &&
                   bUT150Install[i]==true)                                      //Ifor 20190215 : add ATC 使用 三點校正功能
                {
                }
                else
                {
                    myTempPal[i]->edLow->Visible=false;
                    myTempPal[i]->edMid->Visible=false;
                    myTempPal[i]->edLowbase->Visible=false;
                    myTempPal[i]->edBase->Visible=false;
                    myTempPal[i]->edHighBase->Visible=false;
                    myTempPal[i]->edSHighBase->Visible=false;                   //Ztex 2024.07.27 Add 6 Point Temperature Offset
                }
            }

            for(int i=tcAe1; i<=tcBh2; i++)
            {
                if(CosFunction.bATCUseTempAdjustment==true &&
                   bUT150Install[i]==true)                                      //Ifor 20190215 : add ATC 使用 三點校正功能
                {
                }
                else
                {
                    myTempPal[i]->edLow->Visible=false;
                    myTempPal[i]->edMid->Visible=false;
                    myTempPal[i]->edLowbase->Visible=false;
                    myTempPal[i]->edBase->Visible=false;
                    myTempPal[i]->edHighBase->Visible=false;
                    myTempPal[i]->edSHighBase->Visible=false;                   //Ztex 2024.07.27 Add 6 Point Temperature Offset
                }
            }
        }

        // AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Align) -- golden
        // `->palTemp->Align=alBottom/alTop;` (this loop and the two Sort
        // branches below). vclcompat::TPanel/TControl carry no `Align`
        // (pure Z-order/paint property, see facade header G-Align). The
        // Visible-only branch of the `_32Site4X8N` sort path (which does not
        // touch Align) stays ACTIVE below.
#if 0 // GATE(G-Align)
        for(int i=0; i<tcTotalCount; i++)
        {
            for(int j=0; j<32; j++)
            {
                if(myTempPal[i]->iIndexTag==j)
                {
                    myTempPal[i]->palTemp->Align=alBottom;
                }
            }
        }
#endif // GATE(G-Align)

        if(btnSort->Tag<2)                                                      //Steven 20181119 : 新增溫度補償值可以排序
        {
#if 0 // GATE(G-Align) -- entire Tag<2 sort path only ever sets ->Align
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<16; j++)
                {
                    if(btnSort->Tag==0)
                    {
                        x=iAddr[i][j];
                        if(i==0)
                        {
                            myTempPal[x]->palTemp->Align=alTop;
                        }
                        else
                        {
                            myTempPal[x]->palTemp->Align=alTop;
                        }
                    }
                    else if(btnSort->Tag==1)
                    {
                        x=iAddr[i][j];
                        iShowNum[i][j]=j/2+j%2*8;
                    }
                }
            }

            if(btnSort->Tag==1)
            {
                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
                                myTempPal[x]->palTemp->Align=alTop;
                            }
                        }
                    }
                }
            }
#endif // GATE(G-Align)
        }
        else
        {
            if(TestIF.iTestMode==_32Site4X8N)
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<8; j++)
                    {
                        x=i+j*2;
                        if(TestIF_File.iSiteMap[i][j]>0)
                        {
                            iShowNum[0][x]=TestIF_File.iSiteMap[i][j]-1;
                        }
                        else
                        {
                            iShowNum[0][x]=-1;
                        }

                        if(TestIF_File.iSiteMap[i+2][j]>0)
                        {
                            iShowNum[1][x]=TestIF_File.iSiteMap[i+2][j]-1;
                        }
                        else
                        {
                            iShowNum[1][x]=-1;
                        }
                    }
                }

                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
#if 0 // GATE(G-Align)
                                myTempPal[x]->palTemp->Align=alTop;
#endif // GATE(G-Align)
                            }
                            else if(iShowNum[i][j]==-1)
                            {
                                myTempPal[x]->palTemp->Visible=false;
                            }
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<8; j++)
                    {
                        y=i+j*2;
                        for(int k=0; k<2; k++)
                        {
                            x=iAddr[k][y];
                            if(TestIF_File.iSiteMap[i][j]>0)
                            {
                                iShowNum[k][y]=TestIF_File.iSiteMap[i][j]-1;
                            }
                            else
                            {
                                iShowNum[k][y]=-1;
                            }
                        }
                    }
                }

                for(int k=0; k<16; k++)
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<16; j++)
                        {
                            x=iAddr[i][j];
                            if(iShowNum[i][j]==k)
                            {
#if 0 // GATE(G-Align)
                                myTempPal[x]->palTemp->Align=alTop;
#endif // GATE(G-Align)
                            }
                            else if(iShowNum[i][j]==-1)
                            {
                                myTempPal[x]->palTemp->Visible=false;
                            }
                        }
                    }
                }
            }
        }
    //<==
    //Steven 20181119 : 新增溫度補償值可以排序
    }
    //Ifor 20161220 (Steven) add 初始化 溫度設定 For Even Log
    //==>
    dOldWorkTemp    =Temperature.fWorkTemperBase;
    dOldAmbWorkTemp =Temperature.fAbitTemp;
    dOldSockTime    =Temperature.fSoakTime;
    //<==
    //Ifor 20161220 (Steven) add 初始化 溫度設定 For Even Log
}

//---------------------------------------------------------------------------
void TfTemp_Set::edSoakTimeKeyPress(TObject *Sender, char &Key)
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Barcode); G-Qwerty OPENED -- see facade
// header. Barcode_Reader/fQwertyKey have zero port anywhere in this tree.
//---------------------------------------------------------------------------
void TfTemp_Set::edSoakTimeClick(TObject *Sender)
{
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 1
//   widget(s) it wires to this handler (edSoakTime, golden uTemp_Set.dfm:439)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }
#endif // GATE(G-Barcode)
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10000, 0);   //Steven 20240719 : 5 --> 0
}
//---------------------------------------------------------------------------
void TfTemp_Set::edJamSoakTimeClick(TObject *Sender)
{
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 8
//   widget(s) it wires to this handler (edAbitInitWaitTime, golden uTemp_Set.dfm:1286)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }
#endif // GATE(G-Barcode)
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10000, 0);
}
//---------------------------------------------------------------------------
// DEVIATION（已於 FW-SIG-W18 20260826 解除）: 原本 golden 的
// `(TObject*, TMouseButton, TShiftState, int X, int Y)` 被丟成只剩 Sender，
// 理由是那兩個型別沒有 port。vclcompat/ShiftState.h（commit f184093）
// 補上之後，本波把簽章回填為 golden 原文。
//---------------------------------------------------------------------------
void TfTemp_Set::edLHP1MouseDown(TObject *Sender)
{
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) OPENED. Tag is real since this
//   wave (vclcompat::TControl) and Barcode_Reader since FW-BARCODE1 (e7b4bf8).
//   Tag==0 is FAITHFUL here, which is why this one may open while its six
//   siblings may not: golden wires NO widget at all to this handler (measured over uTemp_Set.dfm
//   20260825), so the guard is unreachable in golden too.
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }

    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);                                   //Steven 20141120 : Modify
}
//---------------------------------------------------------------------------
void TfTemp_Set::edLHeatGun1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iHeaterGunH, (double)InputLimit.iHeaterGunL);                              //Steven 20141120 : Modify
}
//---------------------------------------------------------------------------
// DEVIATION: golden `FormClose(TObject *Sender, TCloseAction &Action)` --
// TCloseAction dropped (no port, established tree-wide idiom).
//---------------------------------------------------------------------------
void TfTemp_Set::FormClose(TObject *Sender)
{
    fShow=false;

    rgTemperatureMode->Enabled=true;                                            //jou 2012-05-04 沒有把Enabled打開,會出現無法force的錯誤
    //這一行請保持在最下面!!-----------------
}
//---------------------------------------------------------------------------
void TfTemp_Set::edLHP1KeyUp(TObject *Sender,
      WORD &Key, TShiftState Shift)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Key; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);                                   //Steven 20141120 : Modify
}
//---------------------------------------------------------------------------
void TfTemp_Set::edLowBaseMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 13
//   widget(s) it wires to this handler (edWorkTemp, golden uTemp_Set.dfm:10212)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }
#endif // GATE(G-Barcode)

    dTempMax=MaxTempSetting();                                                  //Steven 20170427 : 回傳機台可以用的最大溫度值
    dTempMin=MinTempSetting();
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, dTempMax, dTempMin);                                  //Steven 20141120 : Modify
}
//---------------------------------------------------------------------------
void TfTemp_Set::rgIndexHeatModeClick(TObject *Sender)
{
    if(fShow==true)
        ReadTempFile(false);
}

//---------------------------------------------------------------------------
// golden :4201-4528. SAFETY GATE (S2): the [A01_2] operator-lockout guard and
// the temperature-offset-limit / FFC time-ordering validation checks stay
// ACTIVE (pure computation + ShowMyMessage, no I/O); the mutating tail (the
// SaveSetupFile(...) call onward: DefineTemp path selection, CopyFile/
// WriteIniData/SaveLastSetIni/fMain->BackupSetupFile) is gated as one block.
//---------------------------------------------------------------------------
void TfTemp_Set::spbSaveClick(TObject *Sender)
{
    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        ShowMyMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        Close();
    }

    AnsiString S="";
    int iSum=0;
    S=GetLastOpenFN();
    AnsiString szDir="", str="", szDir2="";
    if(CosFunction.bSaveTemperatureByMachine && IniConfig.bA57_2SaveTemperatureByMachine)                               //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir=DataPath+S;
    }
    //JerryYang 20170213 (Steven) 矽品騰清要求修改Run check觸發條件
    //==>
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        if(CheckTempSettingChange()==true)
        {
            bHasSaveSet=true;
        }
        EventReport(SECS_EVENT.SaveRecipe);
    }
    //<==
    //JerryYang 20170213 (Steven) 矽品騰清要求修改Run check觸發條件

    //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20
    //==>
    double dbSetTemp=0;
    bool bHasErrorSet=false;
    AnsiString asErrorAddr="";
    {
        if(LastSet.iTemperature==Tempture_Ambient && Temperature.bATCActiveCooling)
        {
            // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE --
            // `fMain->edATCAmbientTemper` has no member on the current fMain
            // facade (grepped forms/fMain.h, 20260820, 0 hits); fMain.h is
            // outside this wave's 3-file write boundary. `dbSetTemp` keeps
            // its `=0` NSDMI default on this branch while gated.
//#if 0 // GATE(dep-fMain-edATCAmbientTemper)   //AI(W906-S09-B1) 20260929: gate retired -- TfMain::edATCAmbientTemper is forms/fMain.h:1151; dbSetTemp is read only inside GATE(dep-FormHS) below, inert today; golden 906_0625_Steven uTemp_Set.cpp:4244
            dbSetTemp=atof(fMain->edATCAmbientTemper->Text.c_str());
//#endif // GATE(dep-fMain-edATCAmbientTemper)   //AI(W906-S09-B1) 20260929: see :4539
        }
        else if(LastSet.iTemperature==Tempture_Ambient &&
                Temperature.bATCActiveCooling==false)                           //Sam 20220527 : 常溫下不需要檢查溫度極限
        {
            dbSetTemp=25;
        }
        else
        {
            dbSetTemp=atof(fMain->edWorkTemperBase->Text.c_str());
        }

        // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- `FormHS` is
        // golden `HS_Function.h`/`.cpp` (NOT forms/fHandlerSys.h's
        // `THandlerSystem` -- a same-sounding but UNRELATED golden class,
        // "Handler System Setup" vs golden HS_Function's own "Handler
        // Self-test" helper; confirmed by grepping golden itself, 20260820:
        // `FormHS`/`CheckTempOffset` both resolve only to HS_Function.h/.cpp
        // there). Neither HS_Function.h nor a `FormHS` global exists
        // anywhere in this port tree (grepped, 0 hits outside this file and
        // golden). Whole validation block gated; `bHasErrorSet` keeps its
        // `=false` default while gated (cannot validate what cannot be read).
//AI(W906-I01C) 20261002 (Ifor01): GATE(dep-FormHS) OPENED -- TFormHS::CheckTempOffset has a body (forms/fHS.cpp:804); FormHS -> UTS_FormHS() (top of file)
        for(int i=0; i<tcTotalCount; i++)
        {
            if(bUT150Install[i]==true)
            {
                if(UTS_FormHS()->CheckTempOffset(i, atof(myTempPal[i]->edLowbase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(UTS_FormHS()->CheckTempOffset(i, atof(myTempPal[i]->edBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(UTS_FormHS()->CheckTempOffset(i, atof(myTempPal[i]->edHighBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))
                {
                    bHasErrorSet=true;
                }
                else if(UTS_FormHS()->CheckTempOffset(i, atof(myTempPal[i]->edSHighBase->Text.c_str())+atof(myTempPal[i]->edOffset->Text.c_str())+dbSetTemp))         //Ztex 2024.07.27 Add 6 Point Temperature Offset
                {
                    bHasErrorSet=true;
                }
            }
        }

        if(bHasErrorSet==true)
        {
            UTS_FormHS()->CheckTempOffset(0, 0, true);
            return;
        }
//AI(W906-I01C) 20261002: (the old #endif of GATE(dep-FormHS))
    }
    //<==
    //Ifor 20180809 (Steven) : add Check the temperature Offset limit Hisi_V02.20

    if(ATC_SYSTEM==eNewATCSystem && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))                         //Ifor 20230504 add:回溫中不可關閉回溫功能
    {
        if(Temperature.bEnableTempRise==true && cbATCTempRise->Checked==false && bDoATCTempRise==true)
        {
            ShowMyMessage("During the warm up process , the temperature warm up function cannot be disabled.", "執行回溫中不可關閉回溫功能");
            cbATCTempRise->Checked=true;
            return;
        }
    }
    //Ifor 20190730 : add FFC Temperature Offset
    //==>
    int iPrevious=0;
    bHasErrorSet=false;
    asErrorAddr="";
    str="";
    for(int i=0; i<2; i++)
    {
        iPrevious=0;
        for(int j=0; j<10; j++)
        {
            if(ATC_FFCPointUse[i][j]->Checked==true)
            {
                if(j>0)
                {
                    if(atoi(ATC_FFCOffsetOnTimeEdit[i][j]->Text.c_str())<=atoi(ATC_FFCOffsetOffTimeEdit[i][iPrevious]->Text.c_str()))                           //下一個Time On 須大於上一個 Time Off時間點
                    {
                        bHasErrorSet=true;
                        str.sprintf("Arm%d_P%d,", i+1, j+1);
                        asErrorAddr=asErrorAddr+str;
                    }
                }

                if(atoi(ATC_FFCOffsetOffTimeEdit[i][j]->Text.c_str())<=atoi(ATC_FFCOffsetOnTimeEdit[i][j]->Text.c_str()))                                       //下一個Time off 須大於 Time On時間點
                {
                    bHasErrorSet=true;
                    str.sprintf("Arm%d_P%d,", i+1, j+1);
                    asErrorAddr=asErrorAddr+str;
                }
                iPrevious=j;
            }
        }
    }

    if(bHasErrorSet==true)
    {
        str.sprintf("FFC Offset Setting error!!");
        ShowMyMessage(str, asErrorAddr);
        return;
    }
    //<==
    //Ifor 20190730 : add FFC Temperature Offset

    // AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S2) -- everything past
    // this point is the real "commit to disk" tail (SaveSetupFile + the
    // DefineTemp path CopyFile/WriteIniData + SaveLastSetIni +
    // fMain->BackupSetupFile). Gated whole-block per the task brief.
//AI(W906-I01C) 20261002 (Ifor01): SAFETY GATE (S2) OPENED (RULINGS_20261001 #0; TO_IFOR §4 1002 08:5x ①, W-05 = (a)) -- the commit-to-disk tail runs as golden
    SaveSetupFile(szDir, S);                                                    //kevin 20150105
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(S,"JOBFILE");
    #endif

    if(     rb1Point->Checked)  iSum+=1;
    else if(rb2Point->Checked)  iSum+=2;
    else if(rb3Point->Checked)  iSum+=4;
    else if(rb5Point->Checked)  iSum+=8;                                        //kevin 20141006
    else if(rb6Point->Checked)  iSum+=16;                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
    //-----------------------------------------------------------

    szDir.sprintf("%sDefineTemp\\", DefaultPath);                               //JerryYang 20170123 避免路徑不存在出現錯誤
    MyForceDirectories(szDir);
    //jou 2010-01-11 start : 效正溫度改固定一組
    if(rgIndexHeatMode->ItemIndex==2)                                           //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
    }
    else
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }

    bool bReadType=(rgIndexHeatMode->ItemIndex==HeadChamber)?true:false;        //Steven 20111102

    if(bReadType)                                                               //jou 2010-06-25 start : 增加 head + chamber mode 獨立一組 K溫值
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                szDir.sprintf("%sDefineTemp\\TemperatureHeadChamber_ATC.Data", DefaultPath);
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\TemperatureHeadChamber.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }
    else
    {
        if((INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e500KG) &&                        //Sam 20210106 : TemperatureData 新增版號來相容
           (TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4   ||                               //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF_File.iTestMode==_8Site2X4   ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
            TestIF_File.dSiteXPitch>50.0       &&
            TestIF_File.bNS7000kit==false      &&
            TestIF_File.bNS7000CS==false)                                       //Steven 20111003 : 240KG, X Pitch>50要另外K
        {
            szDir.sprintf("%sDefineTemp\\Temperature60mm.Data", DefaultPath);
            if(FileExists(szDir)==false)
            {
                str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                CopyFile(str.c_str(), szDir.c_str(), false);
            }
        }
        else
        {
            szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
        }

        if(CosFunction.bATCUseTempAdjustment==true)                             //Ifor 20190215 : add ATC 使用 三點校正功能
        {
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling)      //Steven 20181227 : Fixed ATC沒開要顯示K溫溫度
            {
                //AI(W906-I08) 20261004 (Ifor01): V912 uTemp_Set.cpp:4467-4474 kept (RULINGS_20261003 #1 / #22, TO_IFOR I-08) --
                //  the save side of the same split (golden 0618 uTemp_Set.cpp:4428 wrote the one _ATC file); web save path
                //  FileRW/Temperature.gen.inc:5532-5539. Ledger: docs/IFOR01_V912_LEDGER.md.
                if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC.Data", DefaultPath);
                }
                else
                {
                    szDir.sprintf("%sDefineTemp\\Temperature_ATC_Cold.Data", DefaultPath);
                }
                if(FileExists(szDir)==false)
                {
                    str.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
                    CopyFile(str.c_str(), szDir.c_str(), false);
                }
            }
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20161103 ase kaohsiung 林佳青只 使用一組  temp offset
    {
        szDir.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);
    }
    else if(CosFunction.bTempCalByRecipe==true &&
            chkTempCalByRecipe->Checked==true)                                  //jou 20220725 : Temperature calibration by recipe
    {
        S=GetLastOpenFN();
        szDir.sprintf("%s%s\\DefineTemperature.Data", DataPath, S);
    }

    szDir2.sprintf("%sDefineTemp\\Temperature.Data", DefaultPath);

    WriteIniData(szDir, "Mode",       "Points",      iSum);
    WriteIniData(szDir, "Low OffSet",  "Base",       edLowBase->Text);
    WriteIniData(szDir, "High OffSet", "Base",       edHighBase->Text);
    WriteIniData(szDir, "Mid. OffSet", "Base",       edMidBase->Text);
    WriteIniData(szDir, "SHigh OffSet","Base",       edSHighBase->Text);        //Ztex 2024.07.27 Add 6 Point Temperature Offset

    WriteIniData(szDir, "AmbientHotLowOffSet", "Base",       edAbLow->Text);    //kevin 20140918 恆溫控制
    WriteIniData(szDir, "AmbientHotMidOffSet", "Base",       edAbMid->Text);    //kevin 20140918 恆溫控制

    for(int i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);
        WriteIniData(szDir, "Low OffSet",           S, myTempPal[i]->edLowbase->Text);
        WriteIniData(szDir, "Mid. OffSet",          S, myTempPal[i]->edBase->Text);
        WriteIniData(szDir, "High OffSet",          S, myTempPal[i]->edHighBase->Text);
        WriteIniData(szDir, "SHigh OffSet",         S, myTempPal[i]->edSHighBase->Text);                                //Ztex 2024.07.27 Add 6 Point Temperature Offset

        if(CosFunction.bInitTempOffsetByWorkFile==false)                        //jou 2015-06-13 Initial Temperture Offset by WorkFile
            WriteIniData(szDir, "Init Temp OffSet",     S, myTempPal[i]->edInitTempOffset->Text);                       //Steven 20141117 : 起測時溫度要補Offset

        WriteIniData(szDir, "AmbientHotLowOffSet",  S, myTempPal[i]->edLow->Text);                                      //kevin 20140918 恆溫控制
        WriteIniData(szDir, "AmbientHotMidOffSet",  S, myTempPal[i]->edMid->Text);                                      //kevin 20140918 恆溫控制
        WriteIniData(szDir, "TestOverTime Temp OffSet",  S, myTempPal[i]->edEOTTempOffset->Text);                       //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
    }
    //jou 2010-01-11 end

    if(IniConfig.bL13HotPlateAndShuttleUseOneTempOffset)                        //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    {
        for(int i=tcHotPlate1; i<=tcShuttle2; i++)
        {
            S.printf("CH%d", i+1);
            WriteIniData(szDir2, "Low OffSet",          S, myTempPal[i]->edLowbase->Text);
            WriteIniData(szDir2, "Mid. OffSet",         S, myTempPal[i]->edBase->Text);
            WriteIniData(szDir2, "High OffSet",         S, myTempPal[i]->edHighBase->Text);
            WriteIniData(szDir2, "SHigh OffSet",        S, myTempPal[i]->edSHighBase->Text);                            //Ztex 2024.07.27 Add 6 Point Temperature Offset

            if(CosFunction.bInitTempOffsetByWorkFile==false)                    //jou 2015-06-13 Initial Temperture Offset by WorkFile
                WriteIniData(szDir2, "Init Temp OffSet",    S, myTempPal[i]->edInitTempOffset->Text);                   //Steven 20141117 : 起測時溫度要補Offset

            WriteIniData(szDir2, "AmbientHotLowOffSet", S, myTempPal[i]->edLow->Text);                                  //kevin 20140918 恆溫控制
            WriteIniData(szDir2, "AmbientHotMidOffSet", S, myTempPal[i]->edMid->Text);                                  //kevin 20140918 恆溫控制
            WriteIniData(szDir2, "TestOverTime Temp OffSet",  S, myTempPal[i]->edEOTTempOffset->Text);                  //kevin 20160312 : 距離上一次測試超過時間溫度要補Offset
        }
    }

    if(IniConfig.bVTESTFunction==true)                                          //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        WriteIniData(szDir2, "Mode",       "AmbCheck",    cbUseAbitCHK->Checked);

    //Steven 20120525 Start: 取消TempMonitor, 改到Temp_Set
    for(int i=0; i<tcTotalCount; i++)
    {
        IniConfig.dSingleTempLimit[i]=atof(myTempPal[i]->edSingleLimit->Text.c_str());
    }

    SaveLastSetIni();                                                           //20111212  Dell 修正single temp limit range未存檔
    //Steven 20120525 End
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        if(LastSet.iTemperature==Tempture_Ambient)
        {
            ;   // GATE(W906-I01C) missing-dependency: golden SetTempShiftOffsetPeriod(25,fMain->cbSetupFileName->Text); -- no body in the port (golden cOffSet.cpp:3621; CC_ASE_CL only; the C route gates it too, FileRW/Temperature.gen.inc ELTodo)
        }
        else
        {
            ;   // GATE(W906-I01C) missing-dependency: golden SetTempShiftOffsetPeriod(Temperature.fWorkTemperBase, fMain->cbSetupFileName->Text); -- same as above
        }
    }
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    bNeedInitialTestDelay=true;                                                 //Ztex 2024.02.21 Add Save Temp Offset And Send Offset Value

  //---------------------------我是分隔線-----------------------------------------
    AddSpace(szDir);
    spbSave->Down=false;
    ReadTempFile(true);
    DoIniDataToForm(true);
//AI(W906-I01C) 20261002: (the old #endif of SAFETY GATE (S2))
    (void)iSum; (void)szDir2;   // write-only once SAFETY GATE (S2) is inert
}
//------------------------------------------------------------
//kevin 20140105 儲存 檔案另存 jobfile
//------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S1) -- ENTIRE body is
// WriteIniData calls (~150 of them). Gated whole-body; SaveSetupFile becomes
// a safe no-op. Full golden body preserved verbatim inside the gate.
//---------------------------------------------------------------------------
void TfTemp_Set::SaveSetupFile(AnsiString szDir, AnsiString S)
{
//AI(W906-I01C) 20261002 (Ifor01): SAFETY GATE (S1) OPENED (same ruling) -- SaveSetupFile writes the recipe Temperature.Data as golden
    int iSum=0;
    int data=0;
    int i=0, j=0;
    AnsiString szDir3, Str;
    AnsiString S1="";

    if(CosFunction.bSaveTemperatureByMachine && IniConfig.bA57_2SaveTemperatureByMachine)                               //JimmyChiu 20220618 : save by machine
    {
        szDir3=sSaveByMachine;
    }
    else
    {
        S1=GetLastOpenFN();
        szDir3.sprintf("%s%s\\", DataPath, S1);
    }
    szDir3+="Tester.Data";
    MyForceDirectories(szDir);
    if(     rb1Point->Checked)  iSum+=1;
    else if(rb2Point->Checked)  iSum+=2;
    else if(rb3Point->Checked)  iSum+=4;
    else if(rb5Point->Checked)  iSum+=8;                                        //kevin 20141006
    else if(rb6Point->Checked)  iSum+=16;                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
    if(cbUseAbitCHK->Checked)
        data=1;

    szDir+="\\Temperature.Data";
    WriteIniData(szDir, "Mode",       "Mode",        rgTemperatureMode->ItemIndex);
    WriteIniData(szDir, "Mode",       "Temperature", FormatFloat("0.0", edWorkTemp->Text.ToDouble()));                  //wei 20150422 存檔為小數點後一位，因為SPIL比對資料

    if(IniConfig.bVTESTFunction==false)                                         //jou 20250107 : VTEST 要求 Ambient check 跟著機台設定
        WriteIniData(szDir, "Mode",       "AmbCheck",    data);

    WriteIniData(szDir, "Mode",       "UseIndividualTemp",         cbEnableIndividualMode->Checked);                    //Steven 20140924 : 各個加熱區獨立有自己的設定值
    WriteIniData(szDir, "Mode",       "bUseInitialDelayAsSoakTime",chkSoakTimeReduce->Checked);                         //Steven 20170329 (wei) : 使用initial delay當 Soak time
    WriteIniData(szDir, "Mode",       "bTempAlarmBinNeedToError",  chkTempErrorAction->Checked);                        //Steven 20251022 : Temp alarm need put to error bin

    //Ifor 20180212 : add FTP DownLoad Temp Mode By SetupFile
    if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)
    {
        WriteIniData(szDir, "Mode",       "WorkTempMode", LastSet.iTemperature);
    }

    if(CosFunction.bTempCalByRecipe==true)                                      //jou 20220725 : Temperature calibration by recipe
        WriteIniData(szDir, "Mode", "bTempCalByRecipe", chkTempCalByRecipe->Checked);

    WriteIniData(szDir, "Ambient",    "Check",       1);

    {
        WriteIniData(szDir, "Ambient",    "Temperature", edAmbTemp->Text);
    }

    if(DewPoint_Hardware_Install>0)                                             //Steven 20191017 : 露點計
    {
        WriteIniData(szDir, "Mode", "WaitDewPoint",          cbWaitDewPoint->Checked);                                  //Isaac 20180828 : wait dewpoint
        WriteIniData(szDir, "Mode", "DewPointRange",         edDewPointRange->Text);                                    //Isaac 20181217 : 露點計報警範圍可設定
        WriteIniData(szDir, "Mode", "DewPointAlarmInterval", edDewPointAlarmInterval->Text);                            //Isaac 20181217 : 露點計報警範圍可設定
    }

    if(LB_TEMP==true)                                                           //JerryYang 20220923 : LB吹氣function
    {
        WriteIniData(szDir, "Mode", "bLBCoolingAirOn",          cbBlowLB->Checked);                                     //Isaac 20180828 : wait dewpoint
        WriteIniData(szDir, "Mode", "dLBAirOnTemp",         edtLBAirOnTemp->Text);                                      //Isaac 20181217 : 露點計報警範圍可設定
        WriteIniData(szDir, "Mode", "iLBTempAlmInterval", edLBTempAlarmTime->Text);                                     //Isaac 20181217 : 露點計報警範圍可設定
    }

    WriteIniData(szDir, "Ambient",    "bAmbUsingAFan", chkAmbUsingAFan->Checked);                                       //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    WriteIniData(szDir, "Ambient",    "fAmbientHotGuartbent", edAmbHotGuartbent->Text);                                 //kevin 20140918 恆溫控制
    WriteIniData(szDir, "Ambient",    "bShuttleNoHeatUp", chkShuttleNoHeatUp->Checked);                                 //Steven 20180815 : Amb Ctr mode, shuttle no heat up
    WriteIniData(szDir, "Ambient",    "bSLKNoHeatUp", chkSLKNoHeatUp->Checked);                                         //Steven 20230221 : Amb Ctr mode, SLK no heat up
    WriteIniData(szDir, "Ambient",    "bAmbientGuardbandCheck", cbUseAmbGuard->Checked);                                //kevin 20180115 (Steven) add ambient grand bient check
    WriteIniData(szDir, "Ambient",    "iAmbGuardband", edAmbGuardband->Text);   //kevin 20180115 (Steven) add ambient grand bient check

    WriteIniData(szDir, "Time",       "Soak",        FormatFloat("0.0", edSoakTime->Text.ToDouble()));                  //wei 20150422 存檔為小數點後一位，因為SPIL比對資料
    WriteIniData(szDir, "Time",       "Jam Soak",    edJamSoakTime->Text);
    WriteIniData(szDir, "Time",       "H.Initial",   edInitialWaitTime->Text);
    WriteIniData(szDir, "Time",       "A.Initial",   edAbitInitWaitTime->Text);
    WriteIniData(szDir, "Time",       "Cool Time",   edAbitColdTime->Text);
    WriteIniData(szDir, "Time",       "iInitialStart1Time",   edInitialStart1Time->Text);
    WriteIniData(szDir, "Time",       "iInitialStart2Time",   edInitialStart2Time->Text);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        edIndexSoakTime->Text = "0";
        edOSTime->Text = "0";
    }                                                                           //kevin 20210511 ASE KH  JL 說關閉
    WriteIniData(szDir, "Time",       "iIndexSoakTime",   edIndexSoakTime->Text);                                       //2013-11-27   Dell    增加Index從Shuttle吸起IC時的Soak Time
    WriteIniData(szDir, "Time",       "iOSTime",          edOSTime->Text);      //2013-11-27   Dell    當發生o/s 也是要做Index soak time
    WriteIniData(szDir, "Time",       "bZ2DownSocket",    rgZ1Down->ItemIndex);                                         //kevin 20131211

    WriteIniData(szDir, "Mode",       "Active_Heat_Gun",  cbHeaterGun->Checked);                                        //Steven 20120525 : 熱風槍
    WriteIniData(szDir, "Mode",       "Use CDA Only",     chkUseCDAOnly->Checked);                                      //Steven 20181012 : 使用熱風槍吹冷風
    WriteIniData(szDir, "Mode",       "iHotGunFLowLimit_H",         edtHotGunFLowLimit_H->Text);                        //KaiChen 20190729 ：Hot Gun Flow
    WriteIniData(szDir, "Mode",       "iHotGunFLowLimit_L",         edtHotGunFLowLimit_L->Text);                        //KaiChen 20190729 ：Hot Gun Flow
    WriteIniData(szDir, "DUT Setting",  "bUseFixTemp",    cbUseFixTemp->Checked);                                       //Steven 20131025 : 使4固定的DUT溫度
    WriteIniData(szDir, "DUT Setting",  "dFixedTemp",     edFixedTemp->Text);   //Steven 20131025 : 固定的DUT溫度值
    WriteIniData(szDir, "DUT Setting",  "bShowFixedTemp", cbShowFixTemp->Checked);                                      //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值

    WriteIniData(szDir, "InitialMode",  "EnableTemperatureOffsetforInitial",         cbTempOffset->Checked);            //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iCintactCntForTempOffsetAtInitial",         edTempOffsetCount->Text);          //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iCintactDelayCntForInitTempOffset",         edTempDownContactDelay->Text);     //Steven 20141117 : 起測時溫度要補Offset
    WriteIniData(szDir, "InitialMode",  "iTempReadyRange",                          edTempReadyRange->Text);            //Sam 20231214 : Temp offset use ready temp range
    WriteIniData(szDir, "ChamberBoostMode",       "bEnableChamberBoost",        chkChamberBoost->Checked);              //Steven 20191128 : Chamber Boost Function
    WriteIniData(szDir, "ChamberBoostMode",       "iChamberBoostTime",          edtChamberBoostTime->Text);
    WriteIniData(szDir, "ChamberBoostMode",       "iChamberBoostOffset",        edtChamberBoostOffset->Text);

    if(CosFunction.bHiSiliconFunction==false)                                   //Ifor 20160622 海思版本不存檔，避免蓋掉設定
    {
        //Ifor 20150910 :ATC PID 存檔前轉換
        Temperature.bUsePIDControl = cdATCUsePIDCtrl->Checked;
        Temperature.iATC_PID_Min_Offset[0] =  atof(edtATCPIDOffset_MinP->Text.c_str())*100;
        Temperature.iATC_PID_Min_Offset[1] =  atof(edtATCPIDOffset_MinI->Text.c_str())*100;
        Temperature.iATC_PID_Min_Offset[2] =  atof(edtATCPIDOffset_MinD->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[0] =  atof(edtATCPIDOffset_MaxP->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[1] =  atof(edtATCPIDOffset_MaxI->Text.c_str())*100;
        Temperature.iATC_PID_Max_Offset[2] =  atof(edtATCPIDOffset_MaxD->Text.c_str())*100;

        //Ifor 20150910 :ATC PID Control
        WriteIniData(szDir, "InitialMode",  "UsePIDControl",         Temperature.bUsePIDControl);
        //Ifor 20150910 : ATC PID Offset
        for(int i=0; i<3; i++)
        {
            WriteIniData(szDir, "InitialMode", "iATC_PID_Min_Offset_"+ AnsiString(i)     , Temperature.iATC_PID_Min_Offset[i]);
            WriteIniData(szDir, "InitialMode", "iATC_PID_Max_Offset_"+ AnsiString(i)     , Temperature.iATC_PID_Max_Offset[i]);
        }
    }

    WriteIniData(szDir, "Time", "In Shuttle Soak Time Mode", rgShuttleSoakTime->ItemIndex);                             //jou 2012-06-29 Index Pick up need wait Soak Time
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_Enable"   , cb3SigmaTempMonitior_Enable->Checked);                  //kevin 20200521 add 3 Sigma 溫度記錄比對
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_SetCount" , edt3SigmaTempMonitior_SetCount->Text);                  //kevin 20200521 add 3 Sigma Index Contract time
    WriteIniData(szDir, "Sigma", "SigmaTempMonitior_Set3xSigmaValue"  , edt3SigmaTempMonitior_Set3xSigmaValue->Text);   //kevin 20200521 add 3 Sigma 客戶比對誤差值

    WriteIniData(szDir, "ATC", "bEnableTJFunction"  , cbEnableTJFunction->Checked);                                     //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    WriteIniData(szDir, "ATC", "iTjSiteMapType"  ,    rgTjMapType->ItemIndex);  //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    WriteIniData(szDir, "ATC", "dTjInputVLow"  , atof(edtInputVLow->Text.c_str()));
    WriteIniData(szDir, "ATC", "dTjInputVHigh"  ,  atof(edtInputVHigh->Text.c_str()));

    WriteIniData(szDir, "ATC", "dDelayAfterSOT"  , atof(edtDelayAfterSOT->Text.c_str()));
    WriteIniData(szDir, "ATC", "dDelayAfterSOTContinue"  , atof(edContinuousSec->Text.c_str()));                        //JerryYang 20251201 : ATC連動watchdog新增continue欄位

    WriteIniData(szDir, "ATC", "bTjWatchdog"  , cbTjWatchdog->Checked);         //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)

    WriteIniData(szDir, "ATC", "dTjOffset"  , atof(edtSetTJ_Offset->Text.c_str()));
    WriteIniData(szDir, "ATC", "dTjSlope"  ,  atof(edtSetTJ_Slope->Text.c_str()));

    WriteIniData(szDir, "ATC", "sATC_CH_Tj", sATC_CH_Tj->CommaText);

    if(ATC_SYSTEM>eATC60)                                                       //ChungHung 20141124 add
    {
        if(rgIndexHeatMode->ItemIndex!=0)
            WriteIniData(szDir, "Index",       "Heating Mode",  HeadSocket);    //Steven `20090923
        else
            WriteIniData(szDir, "Index",       "Heating Mode",  HeadOnly);      //Steven `20090923

        if(ATC_SYSTEM==eATCSiliconType)
        {
            WriteIniData(szDir, "ATC", "Temperature Set", rbTemperatureSetOn->Checked);
        }
        else if(ATC_SYSTEM==eATCHonPrecType)                                    //Steven 20120410 : Hontech ATC 2.0
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                WriteIniData(szDir, "ATC", "Chiller Temp", IntToStr(15));       //Ifor 20161103 ATC 2.0 Chiller 溫度寫死20度   //JerryYang 20161128 (Jou)守志說矽品SPEC固定15度
            else
                WriteIniData(szDir, "ATC", "Chiller Temp", edChillerTemp->Text);
        }
        else if(ATC_SYSTEM==eNewATCSystem)                                      //Ifor 20161101 Fix New ATC Chiller Temp Not Save
        {
            WriteIniData(szDir, "ATC", "Chiller Temp", edChillerTemp->Text);
        }

        if(ATC_SYSTEM!=eNonChamber)                                             //Steven 20140314 : For HT9045WA
        {
            WriteIniData(szDir, "ATC", "Active Cooling" , rbATCActiveOn->Checked);
            WriteIniData(szDir, "ATC", "Handler Start"  , rbHandlerStartOn->Checked);
            WriteIniData(szDir, "ATC", "Test Strat"     , rbTestStartOn->Checked);
            WriteIniData(szDir, "ATC", "SP2"            , edtATCSP2->Text);
            WriteIniData(szDir, "ATC", "InPC[0]"        , edtATCInPC1->Text);
            WriteIniData(szDir, "ATC", "InPC[1]"        , edtATCInPC2->Text);
            WriteIniData(szDir, "ATC", "InPC[2]"        , edtATCInPC3->Text);
            WriteIniData(szDir, "ATC", "InPC[3]"        , edtATCInPC4->Text);
            WriteIniData(szDir, "ATC", "Initial_0", edATCInitialOffset1->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_1", edATCInitialOffset2->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_2", edATCInitialOffset3->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC
            WriteIniData(szDir, "ATC", "Initial_3", edATCInitialOffset4->Text);                                         //Steven 20151006 : Initial Temp Offset for ATC

            WriteIniData(szDir, "ATC", "ConsFail_0", edATCContFailOffset1->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_1", edATCContFailOffset2->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_2", edATCContFailOffset3->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ConsFail_3", edATCContFailOffset4->Text);                                       //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "bEnableATCConFailOffset", cbEnableATCConsFailOffset->Checked);                  //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "iATCConFailOffsetCount",  edATCContFailOffsetCnt->Text);                        //Steven 20151123 : Continue Fail Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset1", edATCQAModeOffset1->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset2", edATCQAModeOffset2->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset3", edATCQAModeOffset3->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "ATCQAModeOffset4", edATCQAModeOffset4->Text);                                   //Steven 20151125 : QA Mode Temp Offset for ATC
            WriteIniData(szDir, "ATC", "bEnableATCQAModeOffset", cbEnableATCQAModeOffset->Checked);                     //Steven 20151125 : QA Mode Temp Offset for ATC

            WriteIniData(szDir, "ATC", "bEnableATCTestTimeOffset", cbATCTestTimeOffset->Checked);                       //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "iATCTestTimeOffsetTime", edATCTestTimeOffset->Text);                            //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset1", edATCTestTimeOffset1->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset2", edATCTestTimeOffset2->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset3", edATCTestTimeOffset3->Text);                              //Steven 20160216 : 測試時間太短也要Offset
            WriteIniData(szDir, "ATC", "dATCTestTimeOffset4", edATCTestTimeOffset4->Text);                              //Steven 20160216 : 測試時間太短也要Offset

            WriteIniData(szDir, "ATC", "USE Tj Function" , chk_UseTj->Checked);                                         //2014-05-30    Dell    for ATC6.0
            WriteIniData(szDir, "ATC", "Tj Mode" , rg_TjMode->ItemIndex);       //2014-05-30    Dell    for ATC6.0
            WriteIniData(szDir, "ATC", " Tj Avg Times" , edt_TjAvgTimes->Text);                                         //2014-05-30    Dell    for ATC6.0

            WriteIniData(szDir, "ATC", "ATC7"           , rbATC70ActiveOn->Checked);                                    //Eliot 2015_0105
            WriteIniData(szDir, "ATC", "TSDFunction"    , rbATC7TSDFunction->Checked);                                  //Eliot 2015_0105
            WriteIniData(szDir, "ATC", "TSDTimeOut"     , edTSDTimeOut->Text);  //Steven 20160604 : by site TSD
            WriteIniData(szDir, "ATC", "iATC60Air"      , rgATC60AirOn->ItemIndex);
            WriteIniData(szDir, "ATC", "ATC_OFS_ST"    , edATCOfsTime->Text);   //Ifor 20241118 : 測試中變溫

            if(CosFunction.bUseOldATCTempOffset==true)                          //JerryYang 20190123 ATC Offset UI使用舊版
            {
                for(i=0; i<32; i++)
                {
                    Str.sprintf("ATCTempOffset[%d]", i);                        //JerryYang 20190910 fix ATC offset存檔問題
                    WriteIniData(szDir, "ATC", Str, ATCOffsetEdit[i]->Text);
                }
            }
            else
            {
                //Ifor 20190930 ATC 功能開啟才使用ATC Temp Offset檔案
                if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)                                    // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180817 : Boost Function
                {
                    for(i=0; i<tcTotalCount; i++)
                    {
                        for(j=0; j<32; j++)
                        {
                            if(myTempPal[i]->iIndexTag==j)
                            {
                                Str.sprintf("ATCTempOffset[%d]", j);            //JerryYang 20190910 fix ATC offset存檔問題
                                WriteIniData(szDir, "ATC", Str, myTempPal[i]->edOffset->Text);
                            }
                        }
                    }
                }

                if(CosFunction.bUseSecondATCTempOffset==true)                   //Ifor 20241014 Use Second ATC Temp Offset
                {
                    for(i=0; i<32; i++)
                    {
                        Str.sprintf("ATCSecondTempOffset[%d]", i);
                        WriteIniData(szDir, "ATC", Str, ATCOffsetEdit[i]->Text);
                    }
                }
            }
            //Ifor 20160421 第二點溫度Sensor設定改至工作檔
            WriteIniData(szDir, "ATC", "UseReferTempSensor" , cbATCReferTempSensor->Checked);

            //Ifor 20190306 : add Package Offset 三點校正
            //==>
            for(int i=0; i<3; i++)
            {
                Str.sprintf("ATCPackageOffset[%d]", i);
                WriteIniData(szDir, "ATC", Str, ATCPackageOffsetEdit[i]->Text);

                Str.sprintf("ATCPackagetemp[%d]", i);
                WriteIniData(szDir, "ATC", Str, ATCPackageTempEdit[i]->Text);
            }
            //<==
            //Ifor 20190306 : add Package Offset 三點校正

            //Ifor 20190328 : add TJ Temp Over Range
            //==>
             WriteIniData(szDir, "ATC", "TJTempRange_High"  , edTJTempRange_High->Text);
             WriteIniData(szDir, "ATC", "TJTempRange_Low"   , edTJTempRange_Low->Text);
            //<==
            //Ifor 20190328 : add TJ Temp Over Range

            for(int i=0; i<2; i++)                                              //Ifor 20190730 : add FFC Temperature Offset
            {
                for(int j=0; j<10; j++)
                {
                    Str.sprintf("iArm%d_FFCTimeOn[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetOnTimeEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCTimeOff[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetOffTimeEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCOffset[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCOffsetEdit[i][j]->Text);

                    Str.sprintf("iArm%d_FFCPointUse[%d]", i+1, j);
                    WriteIniData(szDir, "ATC", Str , ATC_FFCPointUse[i][j]->Checked);
                }
            }
            WriteIniData(szDir, "ATC", "bATC_FFCEnable" , cbFFCOffsetEnable->Checked);
            WriteIniData(szDir, "ATC", "UseTC2Offset",    cbUseTC2Offset->Checked);                                     //KenHsieh 20240311 : add Tc2 Offset

            WriteIniData(szDir, "ATC", "Multi Zone Enable", cbMultiZoneFunction->Checked);                              //wei 20240617 Multi Zone
            for(int i=0; i<4; i++)
            {
                Str.sprintf("Zone Temp %d Use", i+1);
                WriteIniData(szDir, "ATC", Str , ZoneTempUse[i]->Checked);

                Str.sprintf("Zone Temp %d Setting", i+1);
                WriteIniData(szDir, "ATC", Str , ZoneTempSetting[i]->Text);
            }
            WriteIniData(szDir, "ATC", "Enable Temp Rise" , cbATCTempRise->Checked);                                    //Ifor 20230418 add:Index 回溫功能
            WriteIniData(szDir, "ATC", "dTempRiseTemp", edt_TempRiseTemp->Text);
            WriteIniData(szDir, "ATC", "dTempRiseDelay", edt_TempRise_Delay->Text);

            if(ATC_SYSTEM==eNewATCSystem)
            {
                WriteIniData(szDir, "ATC", "Multi Sensor Offset Enable", cbMultiSensorOffsetEnable->Checked);           //Ifor 20251203 add:ATC Multi Sensor Offset
                for(int i=0; i<4; i++)                                          //Ifor 20251203 add:ATC Multi Sensor Offset
                {
                    Str.sprintf("Multi Sensor Offset %d Use", i+1);
                    WriteIniData(szDir, "ATC", Str , MultiSensorOffsetUse[i]->Checked);
                }
                for(int i=0; i<32; i++)                                         //Ifor 20251203 add:ATC Multi Sensor Offset
                {
                    Str.sprintf("Multi Sensor Offset[%d]", i+1);
                    WriteIniData(szDir, "ATC", Str , ATC_MultiSensorOffsetEdit[i]->Text);
                }
            }
        }
    }
    else
    {
        WriteIniData(szDir, "Index",       "Heating Mode",  rgIndexHeatMode->ItemIndex);                                //Steven `20090923
    }

    if(Tri_Temp_Machine==1 ||
       (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
    {
        WriteIniData(szDir, "InitialMode", "dSetAirstreamTemperatureRang_Socket" , edt_SetAirstreamTemperatureRang_Socket->Text);
        WriteIniData(szDir, "InitialMode", "EnableTesterDryAirControl"           , cbEnableTesterDryAirControl->Checked);
        WriteIniData(szDir, "InitialMode", "SetAirstreamTemperatureRang_Index"   , edt_SetAirstreamTemperatureRang_Index->Text);
        WriteIniData(szDir, "InitialMode", "bEnableArm_1_Air"                    , chk_EnableArm_1_Air->Checked);
        WriteIniData(szDir, "InitialMode", "bEnableArm_2_Air"                    , chk_EnableArm_2_Air->Checked);
        WriteIniData(szDir, "InitialMode", "bEnableSocket_Air"                   , chk_EnableSocket_Air->Checked);
        WriteIniData(szDir, "InitialMode", "iAirVolumeLmt"                       , edt_AirVolumeLmt_Index->Text);
        WriteIniData(szDir, "InitialMode", "AirStreamSocket_Offset"             , edtSetTempature2AirMachine->Text);    //Ztex 2023.12.19 Add Air Stream Offset
        WriteIniData(szDir, "InitialMode", "AirStreamIndex_Offset"              , edt_SetIndexAirstreamTemp->Text);     //Ztex 2023.12.19 Add Air Stream Offset
        WriteIniData(szDir, "InitialMode", "Defrost_Time_Too_Lower"             , edt_Defrost_Time_Too_Lower->Text);    //Ztex 2024.03.25 Add Defrost Time Too Lower Show Alarm
    }

    AnsiString sATCFILENAME="";
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20181219 (Steven) :  add write ATC FILE NAME
    {
         if(RachoiceATCFile->Checked)
            sATCFILENAME = cbbATC_RecipeFile->Text;
         else
            sATCFILENAME = edAtcFileName->Text;
         WriteIniData(szDir, "ATC", "File Name", sATCFILENAME);                 //Steven 20150810 : add for ATC
    }
    else
    {
        if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)
        {
            WriteIniData(szDir, "ATC", "File Name", edAtcFileName->Text);       //Steven 20150810 : add for ATC
        }
        else if(ATC_SYSTEM==eNewATCSystem)                                      //Ifor 20160111 add for New ATC System
        {
            WriteIniData(szDir, "ATC", "File Name", cbbATC_RecipeFile->Text);   //Ifor 20160111 : add for New ATC System
        }
    }

    if(CosFunction.bOffsetTempByRecipeMinMaxLimit)                              //StevenHong 20260119 : Add Offset By Recipe Max Limit
    {
        Temperature.iOffsetByRecipeMaxLimit = ReadWriteIni(szDir, "User OffSet", "MaxLimit", Temperature.iOffsetByRecipeMaxLimit, 60,  bWriteFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
        Temperature.iOffsetByRecipeMinLimit = ReadWriteIni(szDir, "User OffSet", "MinLimit", Temperature.iOffsetByRecipeMinLimit, -60, bWriteFile, true, -80, 80);  //StevenHong 20260119 : Add Offset By Recipe Max Limit
    }
    for(int i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);

        if(((i>=tcAa1 && i<=tcBd2) || (i>=tcAe1 && i<=tcBh2)) &&
           Temperature.bATCActiveCooling==true)                                 //Ifor 20191111 : Fix ATC 開啟時Offset無法修改問題
        {
        }
        else
        {
            WriteIniData(szDir, "User OffSet", S, myTempPal[i]->edOffset->Text);
        }

        if(CosFunction.bTemp5PointKitOffset)                                    //JerryYang 20221003 Analog要求新增Kit三點offset
        {
            WriteIniData(szDir, "Kit Low OffSet",           S, myTempPal[i]->edKit_Lowbase->Text);
            WriteIniData(szDir, "Kit Mid. OffSet",          S, myTempPal[i]->edKit_Base->Text);
            WriteIniData(szDir, "Kit High OffSet",          S, myTempPal[i]->edKit_HighBase->Text);
            WriteIniData(szDir, "Kit AmbientHotLowOffSet",  S, myTempPal[i]->edKit_Low->Text);
            WriteIniData(szDir, "Kit AmbientHotMidOffSet",  S, myTempPal[i]->edKit_Mid->Text);
        }

        if(ATC_SYSTEM==eNewATCSystem &&
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                      //Steven 20181102 : For QTI全球統一Offset
            WriteIniData(szDir, "Cal Temp", S, myTempPal[i]->edIndiTemp->Text);
        else
            WriteIniData(szDir, "IndividualTempSetting", S, myTempPal[i]->edIndiTemp->Text);                            //Steven 20140924 : 各個加熱區獨立有自己的設定值

        if(CosFunction.bInitTempOffsetByWorkFile==true)                         //jou 2015-06-13 Initial Temperture Offset by WorkFile
            WriteIniData(szDir, "Init Temp OffSet",     S, myTempPal[i]->edInitTempOffset->Text);                       //Steven 20141117 : 起測時溫度要補Offset
    }

    WriteIniData(szDir, "Cooling", "ChamberCoolTemp",   edChamberCoolTemp->Text);                                       //Steven 20111209 : Chamber降溫溫度
    WriteIniData(szDir, "Time",    "bUseTesterDocking", chkTesterDock->Checked);                                        //jou 2014-04-25 Tester Docking Wait delay time
    WriteIniData(szDir, "Time",    "iUseTesterDocking", atoi(edtTestDock->Text.c_str()));                               //jou 2014-04-25 Tester Docking Wait delay time
    WriteIniData(szDir, "ATC", "ATCAmbientTemperature", edATCAmbTemp->Text);    //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "ATC_HeatGunTemp", edtHeatGunTempATC->Text);   //JerryYang 20220408 : add for ATC3.5
    WriteIniData(szDir, "ATC", "dATC_HotGunTemp", edtATC_HotGunTemp->Text);     //Ifor 20220124 add: ATC Hot Air
    WriteIniData(szDir, "ATC", "dATC_HotGunTime", edtATC_HotGunTime->Text);     //Ifor 20220124 add: ATC Hot Air

    WriteIniData(szDir, "Index", "Arm1 Offset",   edArm1Offset->Text);          //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "Arm2 Offset",   edArm2Offset->Text);          //wei 20151013  by Setup File ATC Ambient Temp set mark
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset1", edArm1NoFullsiteOffset_1->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset2", edArm1NoFullsiteOffset_2->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset3", edArm1NoFullsiteOffset_3->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset4", edArm1NoFullsiteOffset_4->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm1 No Fullsite Offset5", edArm1NoFullsiteOffset_5->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset1", edArm2NoFullsiteOffset_1->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset2", edArm2NoFullsiteOffset_2->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset3", edArm2NoFullsiteOffset_3->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset4", edArm2NoFullsiteOffset_4->Text);                           //wei 20160107 No FullSite Add Offset
    WriteIniData(szDir, "Index", "Arm2 No Fullsite Offset5", edArm2NoFullsiteOffset_5->Text);                           //wei 20160107 No FullSite Add Offset

    //Ifor 20160308 add Temp
    double dAlarmCount=StrToFloat(edTempAlwaysSameAlarm->Text);
    if(dAlarmCount<20)
    {
        edTempAlwaysSameAlarm->Text = "20";
    }

    WriteIniData(AuthPath+"ATC.ini", "Setup", "iCheckSameTempTime", edTempAlwaysSameAlarm->Text);   //AI(W906-I01C) 20261002 (Ifor01): golden literal "D:\\HT9045\\Config\\ATC.ini" == AuthPath+"ATC.ini" in production (common.cpp:31; Windows paths ignore case) -- the AuthPath seam (W906_AUTH_PATH) keeps a ctest off the real config\ATC.ini

    WriteIniData(szDir, "Index", "dSocketAirCoolingOnTimer", edSocketAirCoolingOn->Text);                               //jou 2016-04-28 Socket Air Cooling contact count trun on
    WriteIniData(szDir, "Index", "dSocketAirCoolingOffTimer", edSocketAirCoolingOff->Text);                             //jou 2016-04-28 Socket Air Cooling contact count trun on

    //Steven 20180817 : Boost Function
    //==>
    WriteIniData(szDir, "Boost Function", "bBoostFuncttion",            chkBoostFunction->Checked);
    WriteIniData(szDir, "LB Temp Function", "bLBTempFunction",          chkLBTempFunction->Checked);
    WriteIniData(szDir, "Boost Function", "dBoostIdleTime[0]",          atof(edtIdleTime_Long->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostOffset[0]",            atof(edtBoostOffset_Long->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostDuration[0]",          atof(edtBoostDuration_Long->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dPostBoostDuration[0]",      atof(edtPostBoost_Long->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostIdleTime[2]",          atof(edtIdleTime_Short->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostOffset[2]",            atof(edtBoostOffset_Short->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dBoostDuration[2]",          atof(edtBoostDuration_Short->Text.c_str()));
    WriteIniData(szDir, "Boost Function", "dPostBoostDuration[2]",      atof(edtPostBoost_Short->Text.c_str()));

    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[3]",        atof(edtIdleTime_LB->Text.c_str()));            //Steven 20181023 : LB溫度
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[3]",          atof(edtBoostOffset_LB->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostDuration[3]",        atof(edtBoostDuration_LB->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dPostBoostDuration[3]",    atof(edtPostBoost_LB->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[4]",        atof(edtLBTempMin->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[4]",          atof(edtLBTempOffset->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "Threshold",                atof(edtThreshold->Text.c_str()));              //Steven 20190928 : L/B升溫的門檻值
    WriteIniData(szDir, "LB Temp Function", "bEnableBoostOffset[5]",    chkBoostOffset->Checked);
    WriteIniData(szDir, "LB Temp Function", "dBoostIdleTime[5]",        atof(edtBoostTempMin->Text.c_str()));           //Steven 20181023 : LB溫度
    WriteIniData(szDir, "LB Temp Function", "dBoostOffset[5]",          atof(edtBoostOffset->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "dBoostTimeOut",            atof(edtLBTimeOut->Text.c_str()));              //Steven 20181222 : Add LB升溫的Time out
    WriteIniData(szDir, "Boost Function",   "iBoostFunctionMode",       rgBoostMode->ItemIndex);
    //<==
    //Steven 20180817 : Boost Function

    //KenHsieh 20240216 : add ATC Power Follow Function
    //==>
    WriteIniData(szDir, "ATC",     "bPowerFollow_Enable",     cbPowerFollow_Enable->Checked);
    WriteIniData(szDir, "ATC",     "iPowerFollow_FullPower",  edtPowerFollow_FullPower->Text);
    WriteIniData(szDir, "ATC",     "dPowerFollower_PFSlope",  atof(edtPowerFollower_PFSlope->Text.c_str()));
    WriteIniData(szDir, "ATC",     "dPowerFollower_WGain",    atof(edtPowerFollower_WGain->Text.c_str()));
    WriteIniData(szDir, "ATC",     "iPowerFollower_Many2one", edtPowerFollower_Many2one->Text);
    //<==
    //KenHsieh 20240216 : add ATC Power Follow Function

    //Steven 20180815 : Initial Temp Offset-------------------------------------
    //==>
    WriteIniData(szDir3, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",            cbEveryFirstDeviceUseInitialDelay->Checked);

    if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        WriteIniData(szDir3, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         cbUseOtherArmToTestAfterInitialDelay->Checked);
    else
        WriteIniData(szDir3, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",         false);

    WriteIniData(szDir3, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",       cbAfterShowAlarmMessageUseInitialDelay->Checked);
    WriteIniData(szDir3, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay",   cbWhenHappenTestedTimeBelowUseInitialDelay->Checked);
    WriteIniData(szDir3, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",            edBelowSec->Text);
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay",                               edtInitialDelay_1->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_2",                             edtInitialDelay_2->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_3",                             edtInitialDelay_3->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_4",                             edtInitialDelay_4->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_5",                             edtInitialDelay_5->Text);  //ChungHung 20141210 add for SCK want to every event have delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_6",                             edtInitialDelay_6->Text);  //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir3, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           cbWhenPressStopOverUseInitialDelay->Checked);                      //ChungHung 20150526 add for ATK want to even stop over will use initial delay
    WriteIniData(szDir3, "InitialMode",       "iWhenPressStopOver",    edOverSec->Text);                                //ChungHung 20150526 add for ATK want to even stop over will use initial delay

    WriteIniData(szDir3, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              cbWhenNoFullSiteUseInitialDelay->Checked);                         //wei 20151228 No FullSite delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_7",                             edtInitialDelay_7->Text);  //wei 20151228 No FullSite delay

    //ChungHung 20141017 add for SCK Add for TSMC Device
    WriteIniData(szDir3, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      cbAfterAutoCleanFunctionUseInitialDelay->Checked);
    //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time
    WriteIniData(szDir3, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           cbAfterOpenHeatDoorUseInitialDelay->Checked);

    WriteIniData(szDir3, "InitialMode",       "bTestFinishToNextTestOver",                   cbTestFinishToNextTestOver->Checked);                              //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir3, "InitialMode",       "iTestFinishToNextTestOver",                   iTestFinishToNextTestOver->Text);                                  //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_8",                             edtInitialDelay_8->Text);  //kevin 20160310 測試完成到下一次完成超過所設時間須啟動

    WriteIniData(szDir3, "InitialMode",       "bOTDUnlockDelay",                             cbOTDUnlockDelay->Checked);                                        //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir3, "InitialMode",       "iInitialDelay_9",                             edtInitialDelay_9->Text);  //Steven 20160818 : OTD打開Delay
    WriteIniData(szDir3, "InitialMode",       "bTestStartToNextTestStart",                   cbTestStartToNextTestStart->Checked);                              //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir3, "InitialMode",       "dTeststartToNextTestStart",                   iTeststartToNextTestStart->Text);                                  //kevin 20181031 : SOT 超過時間Delay
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_10",                            edtInitialDelay_10->Text);                                         //kevin 20181031 : SOT 超過時間Delay

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    //==>
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_1_RT",             edtInitialDelay_1_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_2_RT",             edtInitialDelay_2_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_3_RT",             edtInitialDelay_3_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_4_RT",             edtInitialDelay_4_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_5_RT",             edtInitialDelay_5_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_6_RT",             edtInitialDelay_6_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_7_RT",             edtInitialDelay_7_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_8_RT",             edtInitialDelay_8_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_9_RT",             edtInitialDelay_9_RT->Text);
    WriteIniData(szDir3, "InitialMode",       "dInitialDelay_10_RT",            edtInitialDelay_10_RT->Text);
    //<==
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT

    WriteIniData(szDir, "Ambient",            "ByPassChamber",                  cbByPassChamber->Checked);              //KenHsieh 20230301 : By Pass Chamber

    WriteIniData(szDir, "LB Temp Function", "LB_Temp_High_Setting",      atof(edLBTempHighSettingValue->Text.c_str()));                                         //Frank 20241231 : add
    WriteIniData(szDir, "LB Temp Function", "LB_Temp_Low_Setting",       atof(edLBTempLowSettingValue->Text.c_str()));
    WriteIniData(szDir, "LB Temp Function", "bLBTempHighAlarm_Enable",   cbLBTempHighAlarmEnable->Checked);
    WriteIniData(szDir, "LB Temp Function", "bLBTempLowAlarm_Enable",    cbLBTempLowAlarmEnable->Checked);
    UTS_ReadTestIFFile_InitialMode();                                            //AI(W906-I01C) 20261002: golden FTestIF->ReadTestIFFile(); -- only its [InitialMode] block (GATE G-FTestIF, top of file)
    //<==
    //Steven 20180815 : Initial Temp Offset-------------------------------------

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        WriteIniData(szDir, "TriTempSet",   "UseHeaterOutShuttle",              chkTriTempEnableOutSht->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterIndex",                   chkTriTempEnableIndex->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterBase",                    chkTriTempEnableBase->Checked);
        WriteIniData(szDir, "TriTempSet",   "UseHeaterDoor",                    chk_IndexHeaterDoor->Checked);          //Ztex 2023.10.23 Add Index Door Heater
    }                                                                           //Heater開關設定
//AI(W906-I01C) 20261002: (the old #endif of SAFETY GATE (S1))
    (void)szDir; (void)S;   // write-only once SAFETY GATE (S1) is inert
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: DEVIATION -- see facade header. Golden
// TForm::Close() has no window here; permanent no-op.
//---------------------------------------------------------------------------
void TfTemp_Set::Close()
{
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: not on the safety-gate list by name, but
// touches real ATC hardware dispatch (golden's own BCB6 delegate-as-function-
// call idiom, `->btOnLine->Click()`/`->btOffLine->Click()`, same class as
// G-Delegate). Caption/Color bookkeeping and flags stay ACTIVE; the two real
// ATC online/offline dispatch calls are gated.
//---------------------------------------------------------------------------
void TfTemp_Set::sbtExitClick(TObject *Sender)
{
    sbtExit->Down=false;
    if(rbATCActiveOn->Checked==true)                                            //Ifor 20170615 (wei) add 開啟/關閉Active ATC cooling 時自動連線/斷線
    {
        bStartATCRun=true;
        fLotInfo->pl_ATC_Online->Caption="ATC On Line";
        fLotInfo->pl_ATC_Online->Color=clLime;
        if(ATC_SYSTEM==eATCHonPrecType)
#if 0 // GATE(dep-ATC-btOnLine-Click)
            ATCInterfaceForm->btOnLine->Click();
#endif // GATE(dep-ATC-btOnLine-Click)
        ;
    }
    else
    {
        bStartATCRun=false;
        fLotInfo->pl_ATC_Online->Caption="ATC Off Line";
        fLotInfo->pl_ATC_Online->Color=clRed;
        if(ATC_SYSTEM==eATCHonPrecType)
#if 0 // GATE(dep-ATC-btOffLine-Click)
            ATCInterfaceForm->btOffLine->Click();
#endif // GATE(dep-ATC-btOffLine-Click)
        ;
    }
    iATCForHSMode=0;
    bATCTempAdjustmentOffset=false;
    bSetTempChange=true;                                                        //Ifor 20160425 工作檔切換需重新設定ATC參數
    Close();
}
//---------------------------------------------------------------------------
void TfTemp_Set::btClearAllClick(TObject *Sender)
{
    edLowBase->Text="";
    edMidBase->Text="";
    edHighBase->Text="";
    edWorkTemp->Text="";
    edAbLow->Text="";                                                           //kevin 20140918
    edAbMid->Text="";                                                           //kevin 20140918
    edSHighBase->Text="";                                                       //Ztex 2024.07.27 Add 6 Point Temperature Offset
}
//---------------------------------------------------------------------------
void TfTemp_Set::DisableOffset()
{
    if(CUSTOMER_CODE==CC_AMKOR_China ||
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 add QUALCOMM
    {
        for(int i=0; i<tcTotalCount; i++)
        {
            myTempPal[i]->edOffset->Visible=false;
            myTempPal[i]->edOffset->Text="0";
        }
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE, WHOLE BODY -- every
// branch here compares against one of `TTabSheet::TabIndex`/`PageIndex` or
// `TPageControl::ActivePage`, none of which exist on vclcompat::TTabSheet/
// TPageControl (that type carries only Caption/TabVisible and
// ActivePageIndex respectively, verified against vclcompat/Controls.h
// directly this wave). Golden's own tab ORDER (which index `tsATC`/`tsArm1`/
// `tsArm2` occupy inside `pgcTempOffset`) is .dfm-derived and not otherwise
// reconstructable from this file alone, so there is no safe substitute
// expression to write in place of the missing members -- whole body gated.
void TfTemp_Set::pgcTempOffsetChange(TObject *Sender)
{
#if 0 // GATE(dep-TabSheet-PageIndex)
    if(pgcTempOffset->ActivePageIndex==tsATC->TabIndex)
    {
        lblTempBase ->Visible=false;
        btClearAll  ->Visible=false;
        edLowBase   ->Visible=false;
        edMidBase   ->Visible=false;
        edHighBase  ->Visible=false;
        edWorkTemp  ->Visible=false;
        edAbLow     ->Visible=false;                                            //kevin 20140918
        edAbMid     ->Visible=false;                                            //kevin 20140918
        edSHighBase ->Visible=false;                                            //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }
    else
    {
        lblTempBase ->Visible=true;
        btClearAll  ->Visible=true;
        edLowBase   ->Visible=true;
        edMidBase   ->Visible=true;
        edHighBase  ->Visible=true;
        edWorkTemp  ->Visible=true;
        edAbLow     ->Visible=true;                                             //kevin 20140918
        edAbMid     ->Visible=true;                                             //kevin 20140918
        edSHighBase ->Visible=true;                                             //Ztex 2024.07.27 Add 6 Point Temperature Offset
    }

    btnSort->Visible=(CosFunction.bUseOldATCTempOffset==false &&
                     (pgcTempOffset->ActivePage==tsArm1 ||                      //Steven 20181119 : 新增溫度補償值可以排序
                      pgcTempOffset->ActivePage==tsArm2) &&                     //JerryYang 20190123 ATC Offset UI使用舊版
                      bNeedChange==true);

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //wei 20151230
    {
        pnlArm1Offset->Visible          =(pgcTempOffset->ActivePageIndex==tsArm1->PageIndex);
        pnlArm2Offset->Visible          =(pgcTempOffset->ActivePageIndex==tsArm2->PageIndex);
        gbNoFullsiteOffset->Visible     =(IniConfig.bL18NofullsiteaddTemperatureoffset);                                //wei 20160107 No FullSite Add Offset
    }
#endif // GATE(dep-TabSheet-PageIndex)
}
//---------------------------------------------------------------------------
// AI(W906-FW-BARCODE2) 20260825: G-Barcode OPENED at this site -- the ONLY
// G-Barcode guard in this file with no Buffer->Tag co-blocker. Barcode_Reader
// real since FW-BARCODE1 (e7b4bf8); non-KYEC config returns 2, so the guard
// never early-returns headless (behaviour identical to the gated shape).
//---------------------------------------------------------------------------
void TfTemp_Set::rgTemperatureModeClick(TObject *Sender)
{
    if(rgTemperatureMode->ItemIndex!=iTempMoldSet &&
       Barcode_Reader(bcTemperature)==0)                                        // 20140103 wei KYEC Barcode Reader
    {
        rgTemperatureMode->ItemIndex=iTempMoldSet;
        return;
    }

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20151118 海思專用版本 工作模式不可修改
    {
        return;
    }

    if(CUSTOMER_CODE==CC_AMKOR_China ||                                         //jou 2013-04-24 客戶說常溫應該要disable
       CUSTOMER_CODE==CC_QUALCOMM)                                              //JerryYang 20170412 add QUALCOMM
    {
        bool bTempFlag=true;
        if(rgTemperatureMode->ItemIndex==1)
            bTempFlag=false;
        else
            bTempFlag=true;

        for(int j=tcHotPlate1; j<tcTotalCount; j++)
        {
            myTempPal[j]->edLowbase->Enabled         =bTempFlag;
            myTempPal[j]->edBase->Enabled            =bTempFlag;
            myTempPal[j]->edHighBase->Enabled        =bTempFlag;
            myTempPal[j]->edOffset->Enabled          =bTempFlag;
            myTempPal[j]->edSingleLimit->Enabled     =bTempFlag;
            myTempPal[j]->edSHighBase->Enabled       =bTempFlag;                //Ztex 2024.07.27 Add 6 Point Temperature Offset
        }
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTargetHP1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    dTempMax=MaxTempSetting();                                                  //Steven 20170427 : 回傳機台可以用的最大溫度值
    dTempMin=MinTempSetting();
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, dTempMax, dTempMin);
}
//---------------------------------------------------------------------------
void TfTemp_Set::DisplayTargetTempEdit(bool Visible)
{
    if(ATC_SYSTEM==eNewATCSystem &&
       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)                          //Steven 20181102 : For QTI全球統一Offset
    {
        palIndiTemp->Visible=true;
        palIndiTemp->Text   ="Cal Offset";
        for(int j=0; j<tcTotalCount; j++)
        {
            if((j>=tcAa1 && j<=tcBd2) || (j>=tcAe1 && j<=tcBh2))
                myTempPal[j]->edIndiTemp->Visible=true;
            else
                myTempPal[j]->edIndiTemp->Visible=false;
        }
    }
    else
    {
        for(int j=0; j<tcTotalCount; j++)
        {
            myTempPal[j]->edIndiTemp->Visible=Visible;
        }

        if(USE_16_HEATER==eht16Heater       ||                                  //Steven 20120606 : 16溫控器 8Site使用Hontech頭
           USE_16_HEATER==eht16HeaterEJ1N   ||
           USE_16_HEATER==eht32HeaterEJ1N   ||                                  //Steven 20140923 : Index使用EJ1N版32組加熱器
           USE_16_HEATER==eht32HeaterKT4H   ||                                  //Steven 20150211 : Index使用KT4H版32組加熱器
           USE_16_HEATER==eht16HeaterDTME08 ||                                  //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
           USE_16_HEATER==eht32HeaterDTME08)                                    //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {
            for(int j=tcHead1; j<=tcHead4; j++)
            {
                myTempPal[j]->edIndiTemp->Visible=false;
            }
        }

        if(iSocketBaseTempCount==eDut2ea)                                       //Steven 20140116 : Socket數量,改用數字設定
        {
            myTempPal[tcDUT4]->edIndiTemp->Visible=false;
            myTempPal[tcDUT3]->edIndiTemp->Visible=false;
            myTempPal[tcSocket]->edIndiTemp->Visible=false;
        }
        else if(iSocketBaseTempCount==eDut4ea)                                  //Steven 20141120 : Fixed 溫度設定
        {
            myTempPal[tcSocket]->edIndiTemp->Visible=false;
        }

        if(ATC_SYSTEM==eATCUninstall || ATC_SYSTEM==eNonChamber)                //20141204 ChungHung ????
            myTempPal[tcChamber]->edIndiTemp->Visible=false;
        myTempPal[tcCCD]->edIndiTemp    ->Visible=false;

        if(REAL_TIME_CCD==true && RTC_TemperNumber==2)                          //Isaac 20201217 : RTC CCD增加第二組感溫
        {
            myTempPal[tcCCD_2]->edIndiTemp    ->Visible=false;
        }
        myTempPal[tc2D]->edIndiTemp     ->Visible=false;                        //wei 20160524 2D溫度

        edWorkTemp->Visible=!Visible;

        myTempPal[tcOutSht1]->edIndiTemp->Visible=(Tri_Temp_Machine==1);
        myTempPal[tcOutSht2]->edIndiTemp->Visible=(Tri_Temp_Machine==1);
        if(Tri_Temp_Machine==1)
        {
            for(int i=0; i<4; i++)
                myTempPal[tcDUT1+i]->edIndiTemp->Visible=true;
        }

        for(int i=0; i<6; i++)
            myTempPal[tcBase1+i]->edIndiTemp->Visible=(Tri_Temp_Machine==1);

        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            palIndiTemp->Text="Set Temp";

        if(INDEXDOORHEATER==1)                                                  //Ztex 2023.10.23 Add Index Door Heater
        {
            myTempPal[tcDoor1]->edIndiTemp->Visible=true;
            myTempPal[tcDoor2]->edIndiTemp->Visible=true;
        }
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::cbEnableIndividualModeClick(TObject *Sender)
{
    DisplayTargetTempEdit(cbEnableIndividualMode->Checked);                     //Steven 20140924 : 各個加熱區獨立有自己的設定值
}
//---------------------------------------------------------------------------
void TfTemp_Set::edAmbHotGuartbentClick(TObject *Sender)
{
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 2
//   widget(s) it wires to this handler (edATCContFailOffsetCnt, golden uTemp_Set.dfm:7275)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }
#endif // GATE(G-Barcode)

    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 1, true, 1.0, 30.0);  //JerryYang 20181001 (Steven) : 20->30
}
//---------------------------------------------------------------------------
void TfTemp_Set::rbATC70ActiveOnClick(TObject *Sender)
{
    if( rbATC70ActiveOn->Checked ==true)
    {
        rbATCActiveOn->Checked = false;
        rbATCActiveOn->Enabled = false;
        edChillerTemp->Enabled = false;
        edtATCInPC1->Enabled = false;
        edtATCInPC2->Enabled = false;
        edtATCInPC3->Enabled = false;
        edtATCInPC4->Enabled = false;
    }
    else
    {
        rbATCActiveOn->Enabled = true;
        edChillerTemp->Enabled = true;
        edtATCInPC1->Enabled = true;
        edtATCInPC2->Enabled = true;
        edtATCInPC3->Enabled = true;
        edtATCInPC4->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::rbATCActiveOnClick(TObject *Sender)
{
    //Ifor 20160412 New ATC System 整合ATC7.0
    if(ATC_SYSTEM==eNewATCSystem)
    {
        rbATC70ActiveOn->Visible    =false;
        rbATC70ActiveOn->Checked    = false;
    }
    else if(rbATCActiveOn->Checked ==true)
    {
        rbATC70ActiveOn->Checked    = false;
        rbATC70ActiveOn->Enabled    = false;
        rbATC7TSDFunction->Enabled  = false;
        edTSDTimeOut->Enabled       = false;
    }
    else
    {
        rbATC70ActiveOn->Enabled    = true;
        rbATC7TSDFunction->Enabled  = true;
        edTSDTimeOut->Enabled       = true;
    }
}

//---------------------------------------------------------------------------
void TfTemp_Set::edtATCInPC1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) OPENED. Tag is real since this
//   wave (vclcompat::TControl) and Barcode_Reader since FW-BARCODE1 (e7b4bf8).
//   Tag==0 is FAITHFUL here, which is why this one may open while its six
//   siblings may not: golden wires 101 widgets to this handler and NOT ONE carries a dfm Tag
//   line, so golden also always takes the Tag!=1 path.
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20180215 (Steven) ADD atc
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, (double)60.0, (double)-60.0);
    else
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, (double)20.0, (double)-20.0);                     //wei 20150427  SPIL   ATC Temperature Offset 20 ~ -20
}
//---------------------------------------------------------------------------
void TfTemp_Set::edAtcFileNameMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL);                    //Steven 20150810 : add for ATC
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCAmbTempMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 1
//   widget(s) it wires to this handler (edATCAmbTemp, golden uTemp_Set.dfm:5426)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }
#endif // GATE(G-Barcode)

    if(ATC_SYSTEM==eNewATCSystem &&
       (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
        ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
        ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, -5.00);
    }
    else
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, 25.00);                                    //Steven 20141120 : Modify
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edArm1OffsetMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    if(AccessLevel<LevelSet.AccessLevel[17])
        return;

    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, -2.00, 2.00);                                         //Steven 20141120 : Modify
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCTestTimeOffsetMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 1000);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCTempAlwaysSameAlarmMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 20, 600);    //Ifor 20170907 (wei) 60000 -> 600
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCChillerTempMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33)                          //Ifor 20240301 add: Chiller 設定小鍵盤
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, -20, 30);
    }
    else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36)           //JerryYang 20220408 : add for ATC3.5
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, -40, 30);
    }
    else if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61)
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, -55, 30);
    }
    else
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 5, 40);  //Ifor 20161103 add Chiller 溫度 最低5度 最高 40度
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCInitialOffset1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) OPENED. Tag is real since this
//   wave (vclcompat::TControl) and Barcode_Reader since FW-BARCODE1 (e7b4bf8).
//   Tag==0 is FAITHFUL here, which is why this one may open while its six
//   siblings may not: golden wires 16 widgets to this handler and none carries a dfm Tag line,
//   so golden also always takes the Tag!=1 path.
    if(Buffer->Tag==1)                                                          //20140320 wei
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }

    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, (double)40.0, 0.0);                                   //Steven 20160227 : for ATC 7.0
}
//---------------------------------------------------------------------------
void TfTemp_Set::edArm1NoFullsiteOffset_1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 20.0, -20.00);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTSDTimeOutMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, (double)100.0, 0.0);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S5) -- whole body: its
// entire purpose is ATC command dispatch (ATCInterfaceForm/ATC_InterfaceForm
// ...SelfTest* calls + FormHS->Show()). Decision logic kept inside the gate
// too (no "safe half" -- the branch selection exists solely to pick WHICH
// command to send).
//---------------------------------------------------------------------------
void TfTemp_Set::SendATCSelfTest(int Source)                                    //Ifor 20160823 add 送出ATC Self Test 1:Auto  2:Manually
{
#if 0 // SAFETY GATE (S5)
    bool bSendOut=false;

    if(Source>=1 && Source<=2)
    {
        if(ATC_SYSTEM==eATCHonPrecType && ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect->Value==true)
        {
            if(Source==1)                                                       //Ifor 20160823 add Auto Sent ATC Self Test Command
                ATCInterfaceForm->SetATCSelfTest();
            else                                                                //Ifor 20160823 add Manually Sent ATC Self Test Command
                ATCInterfaceForm->SetATCManualSelfTest();
            ATCInterfaceForm->ATC_SYS_PAL[0]->iATCSelfTestResult=0;             //Ifor 20160823 add 送出 ATC Self Test Command 清除上次結果
            bSendOut=true;
        }
        else if(ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->IsConnect()==true)
        {
            if(Source==1)                                                       //Ifor 20160823 add Auto Sent ATC Self Test Command
                ATC_InterfaceForm->SetRunSelfTest();
            else                                                                //Ifor 20160823 add Manually Sent ATC Self Test Command
                ATC_InterfaceForm->SetManualSelfTest();
            ATC_InterfaceForm->flagATCSelfTestResult=0;                         //Ifor 20160823 add 送出 ATC Self Test Command 清除上次結果
            bSendOut=true;
        }

        if(bSendOut==true)
        {
            bNeedSendATCRunSelfTest = false;                                    //Ifor 20160726 add 避免多次送出ATC Run Slef Test 指令
            bNeedWaitATCRunSelfTestFinish=true;                                 //Ifor 20160721 等待ATC Self Test回覆
            FormHS->Show();
            if(Source==1)                                                       //Ifor 20160910 add 儲存手動 Or 自動啟動 ATC Self Test
                RecordProcess("Automatic Start ATC Self Test");
            else
                RecordProcess("Manually Start ATC Self Test");
        }
        else
        {
            ShowErrorMessage("WAR15311", 0, MMATC_Handler, false, "SendATCSelfTest");                                   //ATC Not Running!! Please confirm whether to open ATC
        }
    }
    else
    {
        ShowMyMessage("ATC Self Test Start Error,Please try again");
        RecordProcess(IntToStr(Source)+"ATC Self-Test Source Error");           //Ifor 20160823 add 紀錄非預設的條件觸發
    }
#endif // SAFETY GATE (S5)
    (void)Source;   // parameter write-only once SAFETY GATE (S5) is inert
}
//---------------------------------------------------------------------------
void TfTemp_Set::sbSafeTestATCClick(TObject *Sender)
{
    SendATCSelfTest(2);
}
//---------------------------------------------------------------------------
bool TfTemp_Set::CheckTempSettingChange()                                       //JerryYang 20170213 (Steven) Run check觸發條件判斷
{
    //==============Temperature=======================
    if(Temperature.iMachineTempMode!=rgTemperatureMode->ItemIndex)
        return true;
    //==============High Mode Setting=================
    else if(edSoakTime->Text!=FormatFloat("0.0", Temperature.fSoakTime))
        return true;
    else if(edJamSoakTime->Text!=FormatFloat("0.0", Temperature.fJamSoakTime))
        return true;
    else if(edInitialWaitTime->Text!=FormatFloat("0.0", Temperature.fInitialWaitTime))
        return true;
    else if(atoi(edInitialStart1Time->Text.c_str())!=Temperature.iInitialStart1Time)
        return true;
    else if(atoi(edInitialStart2Time->Text.c_str())!=Temperature.iInitialStart2Time)
        return true;
    else if(edChamberCoolTemp->Text!=FormatFloat("0.0", Temperature.fChamberCoolTemp))
        return true;
    else if(atoi(edIndexSoakTime->Text.c_str())!=Temperature.iIndexSoakTime)
        return true;
    else if(atoi(edOSTime->Text.c_str())!=Temperature.iOSTime)
        return true;
    else if(rgZ1Down->ItemIndex!=Temperature.bZ2DownSocket)
        return true;
    else if(rgShuttleSoakTime->ItemIndex!=Temperature.iShuttleSoakTimeMode)
        return true;
    else if(cbEnableIndividualMode->Checked!=Temperature.bUseIndividualTemp)
        return true;
    //==============Ambient Mode Setting==============
    else if(cbUseAbitCHK->Checked!=Temperature.bUseAbitCHK)
        return true;
    else if(edAmbTemp->Text!=FormatFloat("0.0", Temperature.fAbitTemp))
        return true;
    else if(chkAmbUsingAFan->Checked!=Temperature.bAmbUsingAFan)
        return true;
    //==============Temperature offset function=======
    else if(cbTempOffset->Checked!=Temperature.bEnableTempOffsetForInitial)
        return true;
    else if(atoi(edTempOffsetCount->Text.c_str())!=Temperature.iCintactCntForTempOffsetAtInitial)
        return true;
    else if(atoi(edTempDownContactDelay->Text.c_str())!=Temperature.iCintactDelayCntForInitTempOffset)
        return true;
    //==============Base-point========================
    else if((Temperature.iTempMode==1)!=rb1Point->Checked)
        return true;
    else if((Temperature.iTempMode==2)!=rb2Point->Checked)
        return true;
    else if((Temperature.iTempMode==4)!=rb3Point->Checked)
        return true;
    else if((Temperature.iTempMode==8)!=rb5Point->Checked)
        return true;
    else if((Temperature.iTempMode==16)!=rb6Point->Checked)                     //Ztex 2024.07.27 Add 6 Point Temperature Offset
        return true;
    //==============Index Heating Mode================
    else if(rgIndexHeatMode->ItemIndex!=Temperature.iIndexHeatMode)
        return true;
    else if(edWorkTemp->Text!=FormatFloat("0.0", Temperature.fWorkTemperBase))
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
//V3.27B.521 Steven 20170427 : 回傳機台可以用的最大溫度值
//---------------------------------------------------------------------------
double TfTemp_Set::MaxTempSetting()
{
    dTempMax=300.0;
    if(CUSTOMER_CODE!=CC_HONPREC_QC)
    {
        if(LastSet.iTemperature==Tempture_Ambient &&
           Temperature.bATCActiveCooling)                                       //wei 20151013  by Setup File ATC Ambient Temp set
        {
            dTempMax=30.0;
        }
        else if(Tri_Temp_Machine==1)                                            //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            dTempMax=Tri_Temperature_MaxDegree;                                 //設定TriTemp Max、Min溫度
        }
        else if(iTempLimitation==tTemp175)                                      //Frank 20160705 add 改機最高溫150度         //Steven 20211118 : 175度項目移到最上面
        {
            if(Temperature.iIndexHeatMode==HeadOnly ||                          //JerryYang 20160810 改成rgIndexHeatMode->ItemIndex,因為還沒存檔,要根據選擇的加熱模式來設定溫度上限
               Temperature.iIndexHeatMode==HeadSocket)                          //Sam 20231129 : 修正溫度上限模式
                dTempMax=175.0;
            else
                dTempMax=150.0;
        }
        else if(iTempLimitation==tTemp155)                                      //Sam 20240118 新增 155度 模式
        {
            if(Temperature.iIndexHeatMode==HeadOnly ||
               Temperature.iIndexHeatMode==HeadSocket)                          //Sam 20231129 : 修正溫度上限模式
                dTempMax=155.0;
            else
                dTempMax=150.0;
        }
        else if(iTempLimitation==tTemp200)                                      //Sam 20240118 新增 155度 模式
        {
            if(Temperature.iIndexHeatMode==HeadOnly ||
               Temperature.iIndexHeatMode==HeadSocket)                          //Sam 20231129 : 修正溫度上限模式
                dTempMax=200.0;
            else
                dTempMax=150.0;
        }
        else if(ATC_SYSTEM==eNonChamber)                                        //Steven 20140312 : For HT9046HA no chamber
        {
            dTempMax=150.0;
        }
        else if(CosFunction.bTemperatureMax130==true)                           //Steven 20140911 : Xilinx最高只要130度
        {
            dTempMax=130.0;
        }
        else if(iTempLimitation==tTemp150 ||                                    //2014-02-12 Dell HT9046LS 最高溫度150c
                CUSTOMER_CODE==CC_SCC ||
                MachineTypeChoice==Type_HT9046_LS)                              //jou 2014-05-22 SCC 的所有機型要開到150度
        {
            dTempMax=150.0;
        }
        else
        {
            dTempMax=135.0;
        }
    }
    else
    {
        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            dTempMax=Tri_Temperature_MaxDegree;                                 //設定TriTemp Max、Min溫度
        }
    }

    return dTempMax;
}
//---------------------------------------------------------------------------
double TfTemp_Set::MinTempSetting()                                             //Steven 20230712 : 統一溫度設定的最小值判斷
{
    dTempMin=20.0;
    if(CUSTOMER_CODE!=CC_HONPREC_QC)
    {
        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            dTempMin=Tri_Temperature_MinDegree;                                 //設定TriTemp Max、Min溫度
        }
        else if(LastSet.iTemperature==Tempture_Ambient &&
                Temperature.bATCActiveCooling)                                  //wei 20151013  by Setup File ATC Ambient Temp set
        {
            if(ATC_SYSTEM==eNewATCSystem &&
               (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
                ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
                ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
                dTempMin=-5.0;
            else
                dTempMin=25.0;
        }
        else
        {
            dTempMin=20.0;
        }
    }
    else
    {
        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            dTempMin=Tri_Temperature_MinDegree;                                 //設定TriTemp Max、Min溫度
        }
    }

    return dTempMin;
}
//---------------------------------------------------------------------------
void TfTemp_Set::edAmbTempClick(TObject *Sender)
{
    if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                        //Steven 20230410 : 南茂鍾永生要求修改
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 23.0, 30.0);                                      //Sam 20240424 : 修改南茂鍾永生要求修改 25 > 23
    else
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 50.0, 10.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtIdleTime_LongClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 1200.0);
    double d1, d2;
    d1=atof(edtLBTempMin->Text.c_str());
    d2=atof(edtBoostTempMin->Text.c_str());

    if(d1<=d2)
    {
        edtBoostTempMin->Text=AnsiString(d1+2.0);
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtBoostOffset_LongClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, -30.0, 30.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtBoostOffsetClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, -5.0, 5.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtInitialDelay_1Click(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 3000.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::btnSortClick(TObject *Sender)
{                                                                               //Steven 20181119 : 新增溫度補償值可以排序
    btnSort->Tag++;
    if(btnSort->Tag>=3)
        btnSort->Tag=0;

    if(bNeedChange==true)
    {
        if(btnSort->Tag==0)
            btnSort->Caption="Sort by Col";
        else if(btnSort->Tag==1)
            btnSort->Caption="Sort by Row";
        else
            btnSort->Caption="Sort by Site";
    }

    UpDateEdit();
}
//---------------------------------------------------------------------------
void TfTemp_Set::edLimitHP1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 10.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtLBTimeOutClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 300.0, 1200.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::FormDestroy(TObject *Sender)
{
    try
    {
        listNormal->Clear();
        listArm1->Clear();
        listArm2->Clear();
        delete listNormal;
        delete listArm1;
        delete listArm2;

        for(int i=0; i<tcTotalCount; i++)
        {
            if(myTempPal[i]!=NULL)
            {
                delete myTempPal[i];
            }
            myTempPal[i]=NULL;
        }
        sATC_CH_Tj->Clear();                                                    //JerryYang 20231128 : 傳送ATC site map
        delete sATC_CH_Tj;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfTemp_Set::FormDestroy");
    }
    LogSoftwareOffTime("TfTemp_Set, FormDestroy");                              //Steven 20210526 : 紀錄軟體執行時間
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Align) -- whole body, every line
// is `->BringToFront()` (pure Z-order, no vclcompat::TPanel member).
//---------------------------------------------------------------------------
void TfTemp_Set::ShowLineOnTop()
{
#if 0 // GATE(G-Align)
    Panel13->BringToFront();
    Panel2->BringToFront();
    Panel21->BringToFront();
    Panel4->BringToFront();
    Panel36->BringToFront();

    Panel12->BringToFront();
    Panel3->BringToFront();
    Panel23->BringToFront();
    Panel9->BringToFront();
    Panel37->BringToFront();

    Panel5->BringToFront();
    Panel10->BringToFront();
    Panel25->BringToFront();
    Panel29->BringToFront();
    Panel34->BringToFront();

    Panel11->BringToFront();
    Panel6->BringToFront();
    Panel27->BringToFront();
    Panel31->BringToFront();
    Panel41->BringToFront();

    Panel16->BringToFront();
    Panel15->BringToFront();
    Panel28->BringToFront();
    Panel32->BringToFront();
    Panel42->BringToFront();
#endif // GATE(G-Align)
}

//---------------------------------------------------------------------------
// AI(W906-W9) 20260927: SAFETY GATE (S3) OPENED -- Steven W9=A: a remote offset (GPIB SETTESTOFFSET_ / DEVICETEMP,
// SECS TEMP_OFFSET) is written into the recipe's Temperature.Data as golden 906 :5917 / :5954 do
// (gated 20260820 by AI(W906-FW3-TempSet-WA) pending that sign-off).
//---------------------------------------------------------------------------
int TfTemp_Set::SaveRemoteTempOffset(int iArm, int iSite, double dOffset)       //JerryYang 20190812 For Qualcomm 溫度offset funciton
{
    AnsiString S, S1, sLog;

    S1=GetLastOpenFN();
    AnsiString szDir="", str="", szDir2="";
    szDir=DataPath+S1;
    double dNowTemp=0.0;

    int iThermalHead=-1;
    int iResult=-1;
    MyForceDirectories(szDir);
    szDir+="\\Temperature.Data";

    iThermalHead=fMain->RefreshTempData(true, iArm, iSite);                     //輸入Arm,site, 取得溫控器number

    if(iThermalHead<0)                                                          //Steven 20251020 : Add protection.
    {
        RecordProcess("Remote Control Temp Offset is Fail. The channel is -1.");
        return iResult;
    }

    if(ATC_SYSTEM>eATC60 && ATC_SYSTEM!=eNonChamber)                            //Sam 20220406 : 溫度自動補償功能 By FTP     //ChungHung 20141124 add
    {
        if(ATC_SYSTEM!=eNonChamber)                                             //Steven 20140314 : For HT9045WA
        {
            if(CosFunction.bUseOldATCTempOffset==true)                          //JerryYang 20190123 ATC Offset UI使用舊版
            {
                iResult=-1;
                RecordProcess("Remote Control Temp Offset is Fail. OldATCTempOffset is not support.");                  //JerryYang 20200312 add log
            }
            else
            {
                double dMin=InputLimit.iTempLow;                                //JerryYang 20260203 : remove負號     //Jimmychiu 20241203 : add get site on off for SIGURD_PeiXing
                double dMax=InputLimit.iTempHigh;
                if(IniConfig.iN31_UseAutoTempOfsByFTP!=0)
                {
                    dMin=IniConfig.dN31_MinOffset;
                    dMax=IniConfig.dN31_MaxOffset;
                }

                if(dMax<dMin)
                {
                    double dtemp=dMax;
                    dMax=dMin;
                    dMin=dtemp;
                }

                for(int i=0; i<32; i++)
                {
                    if(i==myTempPal[iThermalHead]->iIndexTag)
                    {
                        S.sprintf("ATCTempOffset[%d]", i);
                        if(ATC_SYSTEM==eNewATCSystem)                           // && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) //Steven 20180817 : Boost Function
                        {
                            if((iThermalHead>=tcAa1 && iThermalHead<=tcBd2) ||
                               (iThermalHead>=tcAe1 && iThermalHead<=tcBh2))    //JerryYang 20220815 : 修正ATC remote溫度offset失效
                            {
                                dNowTemp=ReadIniData(szDir, "ATC", S, 0.0);
                                if(dNowTemp+dOffset>dMin && dNowTemp+dOffset<dMax)                                      //Jimmychiu 20241203 : add get site on off for SIGURD_PeiXing
                                {
// AI(W906-W9) 20260927: SAFETY GATE (S3) opened (Steven W9=A) -- golden 906_0625_Steven uTemp_Set.cpp:5917
                                    WriteIniData(szDir, "ATC", S, dNowTemp+dOffset);
// (end of the former SAFETY GATE (S3))
                                    sLog.sprintf("Remote Control Temp Offset %.3f to %.3f", dNowTemp, dNowTemp+dOffset);
                                    RecordProcess(sLog);
                                    iResult=0;
                                }
                                else
                                {
                                    iResult=-1;
                                    S=AnsiString().sprintf("Remote Control Temp Offset is Fail. Adjusted device offset value exceeds [%d, %d]",dMin,dMax);      //Jimmychiu 20241203 : add get site on off for SIGURD_PeiXing
                                    RecordProcess(S);                           //JerryYang 20200312 add log
                                }
                            }
                        }
                        else
                        {
                            iResult=-1;
                            RecordProcess("Remote Control Temp Offset is Fail. Invalid ARM_ID or SITE_NUM");            //JerryYang 20200312 add log
                        }
                    }
                }
            }
        }
        else
        {
            iResult=-1;
            RecordProcess("Remote Control Temp Offset is Fail. Non Chamber type is not support.");                      //JerryYang 20200312 add log
        }
    }
    else
    {
        if((iThermalHead>=tcAa1 && iThermalHead<=tcBd2) ||
           (iThermalHead>=tcAe1 && iThermalHead<=tcBh2))
        {
            S.printf("CH%d", iThermalHead+1);
            dNowTemp=ReadIniData(szDir, "User OffSet", S, 0.0);
            if(dNowTemp+dOffset>InputLimit.iTempLow && dNowTemp+dOffset<InputLimit.iTempHigh)
            {
// AI(W906-W9) 20260927: SAFETY GATE (S3) opened (Steven W9=A) -- golden 906_0625_Steven uTemp_Set.cpp:5954
                WriteIniData(szDir, "User OffSet", S, dNowTemp+dOffset);        //Steven 20141117 : 起測時溫度要補Offset
// (end of the former SAFETY GATE (S3))
                sLog.sprintf("Remote Control Temp. Offset %.3f to %.3f", dNowTemp, dNowTemp+dOffset);
                RecordProcess(sLog);
                iResult=0;
            }
            else
            {
                iResult=-1;
                S.printf("Remote Control Temp Offset is Fail. Adjusted device offset value exceeds [%d, %d]", InputLimit.iTempLow, InputLimit.iTempHigh);
                RecordProcess(S);                                               //JerryYang 20200312 : add log
            }
        }
        else
        {
            iResult=-1;
            RecordProcess("Remote Control Temp Offset is Fail. Invalid ARM_ID or SITE_NUM");                            //JerryYang 20200312 add log
        }
    }
    return iResult;
}
//------------------------------------------------------------------------------
// AI(W906-W9) 20260927: (S4) delegates to (S3), now open; + 912 uTemp_Set.cpp:6142-6163 (RogerYang
// 20260823): reload the offsets after any successful write, as the SECS and FTP paths already do.
//------------------------------------------------------------------------------
void TfTemp_Set::SaveRemoteTempOffsetFromGPIB(AnsiString Msg)                   //Steven 20241113 : for MSG_CMD_SetTJ / DEVICETEMP / SETTESTOFFSET_
{
    int iArm;
    AnsiString Str=StringReplace(Msg, "_", ",", TReplaceFlags()<<rfReplaceAll);
    TStringList *List=new TStringList();
    List->CommaText=Str;

    if(IndexStatus==Z1Down_Z2Up || iContactMode==CONTACT_TEST && iIndexArm==0)
        iArm=1;
    else if(IndexStatus==Z1Up_Z2Down || iContactMode==CONTACT_TEST && iIndexArm==1)
        iArm=2;
    else
        iArm=0;

    if(List->Count>1)
    {
        {   bool bNeedUpdateATC=false;                                      //AI(W906-W9) 20260927: 912 uTemp_Set.cpp:6142 (RogerYang 20260823) GPIB下的溫度offset需回送ATC
            for(int i=1; i<List->Count; i++)                                    //QQQ For NN mode
            {
                if(i>32)                                                        //最大吃到 32Site
                    break;
                double dTempureOfs=atof(AnsiString(List->Strings[i]).c_str());
                if((iArm==0 || iArm==1) && dTempureOfs!=0)                      //Arm1
                    { if(SaveRemoteTempOffset(1, i, dTempureOfs)==0) bNeedUpdateATC=true; }     //AI(W906-W9) 20260927: 912 :6149-6152 有寫入成功才需更新

                if((iArm==0 || iArm==2) && dTempureOfs!=0)                      //Arm2
                    { if(SaveRemoteTempOffset(2, i, dTempureOfs)==0) bNeedUpdateATC=true; }     //AI(W906-W9) 20260927: 912 :6155-6158
            }   if(bNeedUpdateATC==true) ReadRemoteTempOffset();                //AI(W906-W9) 20260927: 912 :6161-6162 比照SECS TEMP_OFFSET, 重載offset並送ATC (the ATC push itself stays GATE(dep-fLotInfo-SetATCOffset))
        }
    }
    List->Clear();
    delete List;
}
//------------------------------------------------------------------------------
// Reads the recipe into Temperature.* (plus MyForceDirectories, as golden :6068); no SAFETY GATE.
//------------------------------------------------------------------------------
void TfTemp_Set::ReadRemoteTempOffset()                                         //JerryYang 20190812 For Qualcomm 溫度offset funciton
{
    AnsiString S, S1, szDir3;

    S1=GetLastOpenFN();
    AnsiString szDir="", str="", szDir2="";
    szDir=DataPath+S1;
    MyForceDirectories(szDir);
    szDir+="\\Temperature.Data";

    for(int i=0; i<tcTotalCount; i++)
    {
        S.printf("CH%d", i+1);

        if((i>=tcAa1 && i<=tcBd2) || (i>=tcAe1 && i<=tcBh2))                    //JerryYang 20190812 add
            Temperature.fTempOffSet[UserOffSet][i]=ReadIniData(szDir, "User OffSet", S, 0.0);
    }

    if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                            //JerryYang 20200623 fix remote temp. command ATC offset異常問題
    {
        for(int i=0; i<32; i++)
        {
            S.sprintf("ATCTempOffset[%d]", i);
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20180215 (Steven) ADD atc
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", S, 0.0), 60.0, -60.0);
            else
                Temperature.dATCTempOffset[i]=CheckRange(ReadIniData(szDir, "ATC", S, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
        }

        for(int i=0; i<tcTotalCount; i++)
        {
            for(int j=0; j<32; j++)
            {
                if(myTempPal[i]->iIndexTag==j)
                {
                    Temperature.fTempOffSet[UserOffSet][i]=Temperature.dATCTempOffset[j];
                }
            }
        }
    }

    if(ATC_SYSTEM>eATC60)                                                       //ChungHung 20141124 add
    {
        if(ATC_SYSTEM!=eNonChamber)                                             //Steven 20140314 : For HT9045WA
        {
            if(CosFunction.bUseOldATCTempOffset==true)                          //JerryYang 20190123 ATC Offset UI使用舊版
            {
            }
            else
            {
                // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE --
                // `fLotInfo->SetATCOffset` has no member on the current
                // fLotInfo facade (grepped forms/fLotInfo.h, 20260820, 0
                // hits -- only the unrelated `fTempATCOffset[]` data array
                // exists); fLotInfo.h is outside this wave's write boundary.
#if 0 // GATE(dep-fLotInfo-SetATCOffset)
                fLotInfo->SetATCOffset(true);                                   //ReadRemoteTempOffset
#endif // GATE(dep-fLotInfo-SetATCOffset)
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTJTempRange_HighClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 20.0, 1.0);   //JerryYang 20251201 : 放寬到20
}
//---------------------------------------------------------------------------
void TfTemp_Set::edAmbGuardbandClick(TObject *Sender)                           //Richard 20230322 : limit修改
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 10, 1);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTJTempRange_LowClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.0, 1.0);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S13) -- decision logic
// (iATC60Air/fWorkTemperBase branch selection) stays ACTIVE; the 3
// `ATC_InterfaceForm->SetAirValve(...)` calls are gated individually.
//---------------------------------------------------------------------------
void TfTemp_Set::ControlATC60AirFlow(int iStatus)
{
    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
    {
        if(Temperature.iATC60Air==0)
        {
#if 0 // SAFETY GATE (S13)
            ATC_InterfaceForm->SetAirValve(true, true);
#endif // SAFETY GATE (S13)
        }
        else if(Temperature.iATC60Air==1)
        {
#if 0 // SAFETY GATE (S13)
            ATC_InterfaceForm->SetAirValve(true, false);
#endif // SAFETY GATE (S13)
        }
        else
        {
            if(Temperature.fWorkTemperBase<50.0)
            {
#if 0 // SAFETY GATE (S13)
                ATC_InterfaceForm->SetAirValve(true, true);
#endif // SAFETY GATE (S13)
            }
            else if(iStatus==0)
            {
#if 0 // SAFETY GATE (S13)
                ATC_InterfaceForm->SetAirValve(true, false);
#endif // SAFETY GATE (S13)
            }
            else if(iStatus==1)
            {
#if 0 // SAFETY GATE (S13)
                ATC_InterfaceForm->SetAirValve(true, true);
#endif // SAFETY GATE (S13)
            }
        }
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtThresholdClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 30.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edDewPointRangeMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 1.0, 30.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edDewPointAlarmIntervalMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 1, 30);
}
//---------------------------------------------------------------------------
void TfTemp_Set::btnSameAsArm1Click(TObject *Sender)
{
    for(int j=0; j<10; j++)
    {
        ATC_FFCOffsetOnTimeEdit[1][j]->Text     =ATC_FFCOffsetOnTimeEdit[0][j]->Text;
        ATC_FFCOffsetOffTimeEdit[1][j]->Text    =ATC_FFCOffsetOffTimeEdit[0][j]->Text;
        ATC_FFCOffsetEdit[1][j]->Text           =ATC_FFCOffsetEdit[0][j]->Text;
        ATC_FFCPointUse[1][j]->Checked          =ATC_FFCPointUse[0][j]->Checked;
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtChamberBoostTimeClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 1, 30);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtChamberBoostOffsetClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 0, 30);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTempOffsetCountClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 0, 30);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTempDownContactDelayClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 1, 300);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtHeatGunTempATCMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
// AI(W906-FW-TAG1) 20260825: GATE(G-Barcode) STAYS SHUT -- premise re-stated,
//   because the old one ("no Tag member") expired this wave and a compilable
//   block is not a faithful one. Golden sets Tag=1 on exactly ONE of the 2
//   widget(s) it wires to this handler (edtHeatGunTempATC, golden uTemp_Set.dfm:5640)
//   and it does so through the .dfm DESIGNER. This port has no .dfm->C++ Tag
//   loader (dfm2rc keeps Tag in the stage-1 IR and web/forms/*.layout.json;
//   the .rc/layout/uimap emitters drop it), so Tag would read 0 here and the
//   KYEC operator-ID barcode interlock would be silently skipped for that
//   widget. Re-open when the values are supplied, not when it compiles.
#if 0 // GATE(G-Barcode)
    if(Buffer->Tag==1)
    {
        if(Barcode_Reader(bcTemperature)==0)
        {
            return;
        }
    }
#endif // GATE(G-Barcode)

    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 70.00, 30.00);
}
//---------------------------------------------------------------------------
//jou 20220725 : Temperature calibration by recipe
void TfTemp_Set::chkTempCalByRecipeClick(TObject *Sender)
{
    if(CosFunction.bTempCalByRecipe==true && fShow==true)                       //jou 20220725 : Temperature calibration by recipe
    {
        Temperature.bTempCalByRecipe=chkTempCalByRecipe->Checked;
        ReadTempFile(false);
        DoIniDataToForm(false);
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: FormShortCut(TWMKey&,bool&) OMITTED --
// see facade header OMITTED note (TWMKey has zero port; Msg.CharCode is
// genuinely read, not a droppable-unused-parameter case).
//---------------------------------------------------------------------------
void TfTemp_Set::edLBTempAlarmTimeMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 2, true, 30, 300);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtLBAirOnTempMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 40.0, 100.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtATCPIDOffset_MinPClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 99999, 0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edFFC_Arm1TimeOn_01Click(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10000, 0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtATC_HotGunTimeClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.0, 0.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtLBTempMinClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, 0.0, 100.0);  //Frank 20241231 : add
}
//---------------------------------------------------------------------------
void TfTemp_Set::rgBasePointClick(TObject *Sender)
{
    UpDateEdit();
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtSetTJ_OffsetClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)99999.0, (double)-99999.0);   //RogerYang 20260318 : ATK P260303-ATK-H9-01
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtSetTJ_SlopeChange(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)99999.0, (double)-99999.0);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Grid) -- whole body. Its only
// payload, `sgTjMap->MouseToCell(...)`, has no port (vclcompat::TStringGrid
// explicitly scopes out mouse/paint surface, see facade header G-Grid).
//---------------------------------------------------------------------------
void TfTemp_Set::sgTjMapMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
#if 0 // GATE(G-Grid)
    int Column, Row;
    sgTjMap->MouseToCell(X, Y, Column, Row);                                    //JerryYang 20170221 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm,改成可複選bin
    if(Row==1)
    {
        sgTjMap->Cells[Column][Row]=IntToStr(StrToInt(sgTjMap->Cells[Column][Row])+1);
        if(StrToInt(sgTjMap->Cells[Column][Row])>16)
        {
            sgTjMap->Cells[Column][Row]=0;
        }

        for(int i=0; i<16; i++)
        {
            sATC_CH_Tj->Strings[i]=sgTjMap->Cells[i+1][Row];
        }
    }
#endif // GATE(G-Grid)
    (void)Sender; (void)X; (void)Y;   // params write-only once GATE(G-Grid) is inert
}
//---------------------------------------------------------------------------
void TfTemp_Set::rgTjMapTypeClick(TObject *Sender)
{
    if(rgTjMapType->ItemIndex==2)
    {
        sgTjMap->Visible=true;
    }
    else
    {
        sgTjMap->Visible=false;
    }
}
//---------------------------------------------------------------------------
void TfTemp_Set::edTempReadyRangeMouseDown(TObject *Sender)                     //Sam 20231214 : Temp offset use ready temp range, DEVIATION
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 10);
}

//---------------------------------------------------------------------------
// golden :6329-6662 (334 lines). Pure ATC-channel<->site data-matrix
// calculation -- no widget/portless dependency, translated verbatim, no
// gate (per task brief's explicit no-gate list).
//---------------------------------------------------------------------------
int TfTemp_Set::InitialAddrToATC()                                              //Steven 20241105 : 更新轉換矩陣
{
    int iResult=1;
    int iHalfATCHeadCnt=ATC_HEAD_COUNT/2;

    for(int i=0; i<tcTotalCount; i++)
        iAddrToATC[i]=-1;

    for(int i=0; i<ATC_HEAD_COUNT; i++)
    {
        iATCToAddr[i]=-1;
        iATCToSiteArm[i]=-1;
        iATCToSiteRow[i]=-1;
        iATCToSiteCol[i]=-1;
    }

    for(int i=0; i<MAX_Index_Row; i++)
    {
        for(int j=0; j<MAX_Index_Col; j++)
        {
            iSiteToATC[0][i][j]=-1;
            iSiteToATC[1][i][j]=-1;

            iSiteToOfs[0][i][j]=j+(i*8);                                        //Steven 20250710 : ATC的頻道轉為Site的Offset index
            iSiteToOfs[1][i][j]=(j+16)+(i*8);
        }
    }

    if(USE_16_HEATER==eht4Heater)
    {
        for(int i=0; i<4; i++)
            iATCToSiteArm[i]=(i<2)?0:1;

        iAddrToATC[tcHead1]=0;  iAddrToATC[tcHead2]=1;  iAddrToATC[tcHead3]=2;  iAddrToATC[tcHead4]=3;
        iATCToAddr[0]=tcHead1;  iATCToAddr[1]=tcHead2;  iATCToAddr[2]=tcHead3;  iATCToAddr[3]=tcHead4;

        if(FTestSuck.iShtRow==1)
        {
            iSiteToATC[0][0][0]=0; iSiteToATC[0][0][1]=1; iSiteToATC[1][0][0]=2; iSiteToATC[1][0][1]=3;
            iATCToSiteRow[0]=0;    iATCToSiteRow[1]=0;    iATCToSiteRow[2]=0;    iATCToSiteRow[3]=0;
            iATCToSiteCol[0]=0;    iATCToSiteCol[1]=1;    iATCToSiteCol[2]=0;    iATCToSiteCol[3]=1;
        }
        else
        {
            iSiteToATC[0][0][0]=0; iSiteToATC[0][1][0]=1; iSiteToATC[1][0][0]=2; iSiteToATC[1][1][0]=3;
            iATCToSiteRow[0]=0;    iATCToSiteRow[1]=1;    iATCToSiteRow[2]=0;    iATCToSiteRow[3]=1;
            iATCToSiteCol[0]=0;    iATCToSiteCol[1]=0;    iATCToSiteCol[2]=0;    iATCToSiteCol[3]=0;
        }
    }
    else if(iATC_Use_Heat_Count==2)
    {
        iAddrToATC[tcAa1]=0;    iAddrToATC[tcAa2]=1;
        iATCToAddr[0]=tcAa1;    iATCToAddr[1]=tcAa2;
        iSiteToATC[0][0][0]=0;  iSiteToATC[1][0][0]=1;
        iATCToSiteArm[0]=0;     iATCToSiteArm[1]=1;
        iATCToSiteRow[0]=0;     iATCToSiteRow[1]=0;
        iATCToSiteCol[0]=0;     iATCToSiteCol[1]=0;
    }
    else
    {
        for(int i=0; i<iATC_Use_Heat_Count; i++)
            iATCToSiteArm[i]=(i<iHalfATCHeadCnt)?0:1;

        if(FTestSuck.iShtRow==1)
        {
            if(iATC_Use_Heat_Count==32)
            {
                for(int i=0; i<iATC_Use_Heat_Count; i++)
                {
                    if(i>=8 && i<16)
                        iATCToSiteRow[i]=1;
                    else if(i>=24 && i<32)
                        iATCToSiteRow[i]=1;
                    else
                        iATCToSiteRow[i]=0;

                    if(i>=8 && i<16)
                        iATCToSiteCol[i]=i-8;
                    else if(i>=16 && i<24)
                        iATCToSiteCol[i]=i-16;
                    else if(i>=24 && i<32)
                        iATCToSiteCol[i]=i-24;
                    else
                        iATCToSiteCol[i]=i;
                }
            }
            else
            {
                for(int i=0; i<iATC_Use_Heat_Count; i++)
                {
                    iATCToSiteRow[i]=0;

                    if(i<iHalfATCHeadCnt)
                        iATCToSiteCol[i]=i;
                    else
                        iATCToSiteCol[i]=i-iHalfATCHeadCnt;
                }
            }
        }
        else
        {
            for(int i=0; i<iATC_Use_Heat_Count; i++)
            {
                iATCToSiteRow[i]=i%2;
            }
        }

        if(iATC_Use_Heat_Count==4)
        {
            if(FTestSuck.iShtRow==1)                                            //1x1, 1x2
            {
                //Aa1                           //Ab1
                iAddrToATC[tcAa1]   =0;         iAddrToATC[tcAb1]   =1;
                iATCToAddr[0]       =tcAa1;     iATCToAddr[1]       =tcAb1;
                iSiteToATC[0][0][0] =0;         iSiteToATC[0][0][1] =1;
                iATCToSiteCol[0]    =0;         iATCToSiteCol[1]    =1;
                iSiteToOfs[0][0][0] =0;         iSiteToOfs[0][0][1] =8;         //Steven 20241105 : ATC的頻道轉為Site的Offset index

                //Aa2                           //Ab2
                iAddrToATC[tcAa2]   =2;         iAddrToATC[tcAb2]   =3;
                iATCToAddr[2]       =tcAa2;     iATCToAddr[3]       =tcAb2;
                iSiteToATC[1][0][0] =2;         iSiteToATC[1][0][1] =3;
                iATCToSiteCol[2]    =0;         iATCToSiteCol[3]    =1;
                //AI(Jimmy) 20260417: fix typo - 第二個 subscript 應為 [1][0][1] 而非重複 [1][0][0]，
                //AI(Jimmy) 20260417: 造成 Arm2 col0 的 offset index 被 24 覆蓋成 Ba2，SingleSite/1x2 時 Arm2 改 Aa2 無效、改 Ba2 才生效
                iSiteToOfs[1][0][0] =16;        iSiteToOfs[1][0][1] =24;
            }
            else                                                                //2x1
            {
                //Aa1
                iAddrToATC[tcAa1]   =0;
                iATCToAddr[0]       =tcAa1;
                iSiteToATC[0][0][0] =0;
                iATCToSiteCol[0]    =0;
                iSiteToOfs[0][0][0] =0;

                //Ba1
                iAddrToATC[tcBa1]   =1;
                iATCToAddr[1]       =tcBa1;
                iSiteToATC[0][1][0] =1;
                iATCToSiteCol[1]    =0;
                iSiteToOfs[0][1][0] =8;

                //Aa2
                iAddrToATC[tcAa2]   =2;
                iATCToAddr[2]       =tcAa2;
                iSiteToATC[1][0][0] =2;
                iATCToSiteCol[2]    =0;
                iSiteToOfs[1][0][0] =16;

                //Ba2
                iAddrToATC[tcBa2]   =3;
                iATCToAddr[3]       =tcBa2;
                iSiteToATC[1][1][0] =3;
                iATCToSiteCol[3]    =0;
                iSiteToOfs[1][0][1] =24;
            }
        }
        else if(iATC_Use_Heat_Count==8)
        {
            if(FTestSuck.iShtRow==1)                                            //1x1, 1x2, 1x3, 1x4
            {
                //Aa1                       //Ab1                       //Ac1                       //Ad1
                iAddrToATC[tcAa1]   =0;     iAddrToATC[tcAb1]   =1;     iAddrToATC[tcAc1]   =2;     iAddrToATC[tcAd1]   =3;
                iATCToAddr[0]       =tcAa1; iATCToAddr[1]       =tcAb1; iATCToAddr[2]       =tcAc1; iATCToAddr[3]       =tcAd1;
                iSiteToATC[0][0][0] =0;     iSiteToATC[0][0][1] =1;     iSiteToATC[0][0][2] =2;     iSiteToATC[0][0][3] =3;
                iSiteToOfs[0][0][0] =0;     iSiteToOfs[0][0][1] =8;     iSiteToOfs[0][0][2] =1;     iSiteToOfs[0][0][3] =9;                                     //Steven 20241105 : ATC的頻道轉為Site的Offset index

                //Aa2                       //Ab2                       //Ac2                       //Ad2
                iAddrToATC[tcAa2]   =4;     iAddrToATC[tcAb2]   =5;     iAddrToATC[tcAc2]   =6;     iAddrToATC[tcAd2]   =7;
                iATCToAddr[4]       =tcAa2; iATCToAddr[5]       =tcAb2; iATCToAddr[6]       =tcAc2; iATCToAddr[7]       =tcAd2;
                iSiteToATC[1][0][0] =4;     iSiteToATC[1][0][1] =5;     iSiteToATC[1][0][2] =6;     iSiteToATC[1][0][3] =7;
                iSiteToOfs[1][0][0] =16;    iSiteToOfs[1][0][1] =24;    iSiteToOfs[1][0][2] =17;    iSiteToOfs[1][0][3] =25;
            }
            else
            {
                //Aa1                       //Ab1
                iAddrToATC[tcAa1]   =0;     iAddrToATC[tcAb1]   =2;
                iATCToAddr[0]       =tcAa1; iATCToAddr[2]       =tcAb1;
                iSiteToATC[0][0][0] =0;     iSiteToATC[0][0][1] =2;
                iATCToSiteCol[0]    =0;     iATCToSiteCol[2]    =1;
                iSiteToOfs[0][0][0] =0;     iSiteToOfs[0][0][1] =1;

                //Ba1                       //Bb1
                iAddrToATC[tcBa1]   =1;     iAddrToATC[tcBb1]   =3;
                iATCToAddr[1]       =tcBa1; iATCToAddr[3]       =tcBb1;
                iSiteToATC[0][1][0] =1;     iSiteToATC[0][1][1] =3;
                iATCToSiteCol[1]    =0;     iATCToSiteCol[3]    =1;
                iSiteToOfs[0][1][0] =8;     iSiteToOfs[0][1][1] =9;

                //Aa2                       //Ab2
                iAddrToATC[tcBa2]   =5;     iAddrToATC[tcBb2]   =7;
                iATCToAddr[4]       =tcAa2; iATCToAddr[6]       =tcAb2;
                iSiteToATC[1][0][0] =4;     iSiteToATC[1][0][1] =6;
                iATCToSiteCol[4]    =0;     iATCToSiteCol[6]    =1;
                iSiteToOfs[1][0][0] =16;    iSiteToOfs[1][0][1] =17;

                //Ba2                       //Bb2
                iAddrToATC[tcAa2]   =4;     iAddrToATC[tcAb2]   =6;
                iATCToAddr[5]       =tcBa2; iATCToAddr[7]       =tcBb2;
                iSiteToATC[1][1][0] =5;     iSiteToATC[1][1][1] =7;
                iATCToSiteCol[5]    =0;     iATCToSiteCol[7]    =1;
                iSiteToOfs[1][1][0] =24;    iSiteToOfs[1][1][1] =25;
            }
        }
        else if(iATC_Use_Heat_Count==16)
        {
            if(FTestSuck.iShtRow==1)
            {
                iAddrToATC[tcAa1]=0;    iAddrToATC[tcAb1]=1;    iAddrToATC[tcAc1]=2;    iAddrToATC[tcAd1]=3;
                iAddrToATC[tcAe1]=4;    iAddrToATC[tcAf1]=5;    iAddrToATC[tcAg1]=6;    iAddrToATC[tcAh1]=7;
                iAddrToATC[tcAa2]=8;    iAddrToATC[tcAb2]=9;    iAddrToATC[tcAc2]=10;   iAddrToATC[tcAd2]=11;
                iAddrToATC[tcAe2]=12;   iAddrToATC[tcAf2]=13;   iAddrToATC[tcAg2]=14;   iAddrToATC[tcAh2]=15;

                iATCToAddr[ 0]=tcAa1;   iATCToAddr[ 1]=tcAb1;   iATCToAddr[ 2]=tcAc1;   iATCToAddr[ 3]=tcAd1;
                iATCToAddr[ 4]=tcAe1;   iATCToAddr[ 5]=tcAf1;   iATCToAddr[ 6]=tcAg1;   iATCToAddr[ 7]=tcAh1;
                iATCToAddr[ 8]=tcAa2;   iATCToAddr[ 9]=tcAb2;   iATCToAddr[10]=tcAc2;   iATCToAddr[11]=tcAd2;
                iATCToAddr[12]=tcAe2;   iATCToAddr[13]=tcAf2;   iATCToAddr[14]=tcAg2;   iATCToAddr[15]=tcAh2;

                iSiteToATC[0][0][0]=0;  iSiteToATC[0][0][1]=1;  iSiteToATC[0][0][2]=2;  iSiteToATC[0][0][3]=3;
                iSiteToATC[0][0][4]=4;  iSiteToATC[0][0][5]=5;  iSiteToATC[0][0][6]=6;  iSiteToATC[0][0][7]=7;
                iSiteToATC[1][0][0]=8;  iSiteToATC[1][0][1]=9;  iSiteToATC[1][0][2]=10; iSiteToATC[1][0][3]=11;
                iSiteToATC[1][0][4]=12; iSiteToATC[1][0][5]=13; iSiteToATC[1][0][6]=14; iSiteToATC[1][0][7]=15;
            }
            else
            {
                iAddrToATC[tcAa1]= 0;   iAddrToATC[tcBa1]= 1;   iAddrToATC[tcAb1]= 2;   iAddrToATC[tcBb1]= 3;
                iAddrToATC[tcAc1]= 4;   iAddrToATC[tcBc1]= 5;   iAddrToATC[tcAd1]= 6;   iAddrToATC[tcBd1]= 7;
                iAddrToATC[tcAa2]= 8;   iAddrToATC[tcBa2]= 9;   iAddrToATC[tcAb2]=10;   iAddrToATC[tcBb2]=11;
                iAddrToATC[tcAc2]=12;   iAddrToATC[tcBc2]=13;   iAddrToATC[tcAd2]=14;   iAddrToATC[tcBd2]=15;

                iATCToAddr[ 0]=tcAa1;   iATCToAddr[ 1]=tcBa1;   iATCToAddr[ 2]=tcAb1;   iATCToAddr[ 3]=tcBb1;
                iATCToAddr[ 4]=tcAc1;   iATCToAddr[ 5]=tcBc1;   iATCToAddr[ 6]=tcAd1;   iATCToAddr[ 7]=tcBd1;
                iATCToAddr[ 8]=tcAa2;   iATCToAddr[ 9]=tcBa2;   iATCToAddr[10]=tcAb2;   iATCToAddr[11]=tcBb2;
                iATCToAddr[12]=tcAc2;   iATCToAddr[13]=tcBc2;   iATCToAddr[14]=tcAd2;   iATCToAddr[15]=tcBd2;

                iSiteToATC[0][0][0]=0;  iSiteToATC[0][1][0]=1;  iSiteToATC[0][0][1]=2;  iSiteToATC[0][1][1]=3;
                iSiteToATC[0][0][2]=4;  iSiteToATC[0][1][2]=5;  iSiteToATC[0][0][3]=6;  iSiteToATC[0][1][3]=7;
                iSiteToATC[1][0][0]=8;  iSiteToATC[1][1][0]=9;  iSiteToATC[1][0][1]=10; iSiteToATC[1][1][1]=11;
                iSiteToATC[1][0][2]=12; iSiteToATC[1][1][2]=13; iSiteToATC[1][0][3]=14; iSiteToATC[1][1][3]=15;

                iATCToSiteCol[ 0]=0;    iATCToSiteCol[ 1]=0;    iATCToSiteCol[ 2]=1;    iATCToSiteCol[ 3]=1;
                iATCToSiteCol[ 4]=2;    iATCToSiteCol[ 5]=2;    iATCToSiteCol[ 6]=3;    iATCToSiteCol[ 7]=3;
                iATCToSiteCol[ 8]=0;    iATCToSiteCol[ 9]=0;    iATCToSiteCol[10]=1;    iATCToSiteCol[11]=1;
                iATCToSiteCol[12]=2;    iATCToSiteCol[13]=2;    iATCToSiteCol[14]=3;    iATCToSiteCol[15]=3;
            }
        }
        else if(iATC_Use_Heat_Count==32)                                        //Steven 20241126 : 修正ATC溫度顯示
        {
            {
                iAddrToATC[tcAa1]= 0;   iAddrToATC[tcBa1]= 1;   iAddrToATC[tcAb1]= 2;   iAddrToATC[tcBb1]= 3;
                iAddrToATC[tcAc1]= 4;   iAddrToATC[tcBc1]= 5;   iAddrToATC[tcAd1]= 6;   iAddrToATC[tcBd1]= 7;
                iAddrToATC[tcAe1]= 8;   iAddrToATC[tcBe1]= 9;   iAddrToATC[tcAf1]=10;   iAddrToATC[tcBf1]=11;
                iAddrToATC[tcAg1]=12;   iAddrToATC[tcBg1]=13;   iAddrToATC[tcAh1]=14;   iAddrToATC[tcBh1]=15;
                iAddrToATC[tcAa2]=16;   iAddrToATC[tcBa2]=17;   iAddrToATC[tcAb2]=18;   iAddrToATC[tcBb2]=19;
                iAddrToATC[tcAc2]=20;   iAddrToATC[tcBc2]=21;   iAddrToATC[tcAd2]=22;   iAddrToATC[tcBd2]=23;
                iAddrToATC[tcAe2]=24;   iAddrToATC[tcBe2]=25;   iAddrToATC[tcAf2]=26;   iAddrToATC[tcBf2]=27;
                iAddrToATC[tcAg2]=28;   iAddrToATC[tcBg2]=29;   iAddrToATC[tcAh2]=30;   iAddrToATC[tcBh2]=31;

                iATCToAddr[ 0]=tcAa1;   iATCToAddr[ 1]=tcBa1;   iATCToAddr[ 2]=tcAb1;   iATCToAddr[ 3]=tcBb1;
                iATCToAddr[ 4]=tcAc1;   iATCToAddr[ 5]=tcBc1;   iATCToAddr[ 6]=tcAd1;   iATCToAddr[ 7]=tcBd1;
                iATCToAddr[ 8]=tcAe1;   iATCToAddr[ 9]=tcBe1;   iATCToAddr[10]=tcAf1;   iATCToAddr[11]=tcBf1;
                iATCToAddr[12]=tcAg1;   iATCToAddr[13]=tcBg1;   iATCToAddr[14]=tcAh1;   iATCToAddr[15]=tcBh1;
                iATCToAddr[16]=tcAa2;   iATCToAddr[17]=tcBa2;   iATCToAddr[18]=tcAb2;   iATCToAddr[19]=tcBb2;
                iATCToAddr[20]=tcAc2;   iATCToAddr[21]=tcBc2;   iATCToAddr[22]=tcAd2;   iATCToAddr[23]=tcBd2;
                iATCToAddr[24]=tcAe2;   iATCToAddr[25]=tcBe2;   iATCToAddr[26]=tcAf2;   iATCToAddr[27]=tcBf2;
                iATCToAddr[28]=tcAg2;   iATCToAddr[29]=tcBg2;   iATCToAddr[30]=tcAh2;   iATCToAddr[31]=tcBh2;

                iSiteToATC[0][0][0]=0;  iSiteToATC[0][1][0]=1;  iSiteToATC[0][0][1]=2;  iSiteToATC[0][1][1]=3;
                iSiteToATC[0][0][2]=4;  iSiteToATC[0][1][2]=5;  iSiteToATC[0][0][3]=6;  iSiteToATC[0][1][3]=7;
                iSiteToATC[0][0][4]=8;  iSiteToATC[0][1][4]=9;  iSiteToATC[0][0][5]=10; iSiteToATC[0][1][5]=11;
                iSiteToATC[0][0][6]=12; iSiteToATC[0][1][6]=13; iSiteToATC[0][0][7]=14; iSiteToATC[0][1][7]=15;
                iSiteToATC[1][0][0]=16; iSiteToATC[1][1][0]=17; iSiteToATC[1][0][1]=18; iSiteToATC[1][1][1]=19;
                iSiteToATC[1][0][2]=20; iSiteToATC[1][1][2]=21; iSiteToATC[1][0][3]=22; iSiteToATC[1][1][3]=23;
                iSiteToATC[1][0][4]=24; iSiteToATC[1][1][4]=25; iSiteToATC[1][0][5]=26; iSiteToATC[1][1][5]=27;
                iSiteToATC[1][0][6]=28; iSiteToATC[1][1][6]=29; iSiteToATC[1][0][7]=30; iSiteToATC[1][1][7]=31;

                iATCToSiteCol[ 0]=0;    iATCToSiteCol[ 1]=0;    iATCToSiteCol[ 2]=1;    iATCToSiteCol[ 3]=1;
                iATCToSiteCol[ 4]=2;    iATCToSiteCol[ 5]=2;    iATCToSiteCol[ 6]=3;    iATCToSiteCol[ 7]=3;
                iATCToSiteCol[ 8]=4;    iATCToSiteCol[ 9]=4;    iATCToSiteCol[10]=5;    iATCToSiteCol[11]=5;
                iATCToSiteCol[12]=6;    iATCToSiteCol[13]=6;    iATCToSiteCol[14]=7;    iATCToSiteCol[15]=7;
                iATCToSiteCol[16]=0;    iATCToSiteCol[17]=0;    iATCToSiteCol[18]=1;    iATCToSiteCol[19]=1;
                iATCToSiteCol[20]=2;    iATCToSiteCol[21]=2;    iATCToSiteCol[22]=3;    iATCToSiteCol[23]=3;
                iATCToSiteCol[24]=4;    iATCToSiteCol[25]=4;    iATCToSiteCol[26]=5;    iATCToSiteCol[27]=5;
                iATCToSiteCol[28]=6;    iATCToSiteCol[29]=6;    iATCToSiteCol[30]=7;    iATCToSiteCol[31]=7;
            }
        }
    }

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        iAddrToATC[tcHotPlate1]=bUT150HasUse[tcHotPlate1];
        iAddrToATC[tcHotPlate2]=bUT150HasUse[tcHotPlate2];
        iAddrToATC[tcHotPlate3]=bUT150HasUse[tcHotPlate3];
        iAddrToATC[tcHotPlate4]=bUT150HasUse[tcHotPlate4];

        iAddrToATC[tcShuttle1] =bUT150HasUse[tcShuttle1];
        iAddrToATC[tcShuttle2] =bUT150HasUse[tcShuttle2];
        iAddrToATC[tcShuttle3] =bUT150HasUse[tcShuttle3];
        iAddrToATC[tcShuttle4] =bUT150HasUse[tcShuttle4];
    }

    return iResult;
}
//---------------------------------------------------------------------------
void TfTemp_Set::edATCOfsTimeMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W18): golden 這一支也沒讀它們
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 60000);
}
//---------------------------------------------------------------------------
//Ztex 2023.04.19 Add HT-1032 TriTemp Function
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S7) -- whole body past the
// early guard (the rest is 3 ATC_InterfaceForm calls -- get status, get
// mode, then send the actual switch command).
//---------------------------------------------------------------------------
void TfTemp_Set::SwitchOnOffRefrigeratorNo(int iRefrigeratorNo, int iOnOff)     //Hmy20180730 Add ATC Refrigerator Status
{
    if(iRefrigeratorNo<=0)                                                      //ATC 壓縮機相關命令
        return;
#if 0 // SAFETY GATE (S7)
    int iUseRefrigeratorNo=iRefrigeratorNo-1;

    ATC_InterfaceForm->Get_ATCRefrigeratorAllStatus(true);
    ATC_InterfaceForm->Get_ATCRefrigeratorAtcSetMode(Total_Compressor, RefrigeratorUserModeState);

    if(RefrigeratorUserModeState[iUseRefrigeratorNo]!=iOnOff)
        ATC_InterfaceForm->SendSwitchRefrigerator(iUseRefrigeratorNo, iOnOff);
#endif // SAFETY GATE (S7)
    (void)iOnOff;   // parameter write-only once SAFETY GATE (S7) is inert
}
//------------------------------------------------------------------------------
// NOT gated -- golden's own body is `iUseCloseSiteTempCtrl[iChannel]=bUse;
// //No Use`, a plain array write golden itself marks dead. No hardware reach.
//------------------------------------------------------------------------------
void TfTemp_Set::UseCloseSiteTemperatureCtrl(int iChannel, bool bUse)
{
    iUseCloseSiteTempCtrl[iChannel]=bUse;                                       //No Use
}
//------------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S9), WHOLE BODY -- revised
// from the originally-planned "decision stays active, only SetSingleTemp
// gated" split: the `bUse` lookup itself needs `TriTemp_Ch` (golden `extern
// int TriTemp_Ch[ATC_MAX_SITE]`, ATC/ATC_Handler_Side.h:497), which does NOT
// exist anywhere in this port tree yet (MainCalcCore.h's own 20260728 recon
// note says so explicitly: "Neither exists in this ported tree yet"). With
// no `bUse` to compute, there is no "safe half" of this function left
// un-gated; both the DEPENDENCY gap and the SAFETY gap collapse into one
// whole-body gate.
//------------------------------------------------------------------------------
void TfTemp_Set::SetSingleWorkTemperature(int iChannel, double fTemp, bool bDefrost)
{
#if 0 // SAFETY GATE (S9) + GATE(dep-TriTemp_Ch)
    bool bUse;
    bUse=bUT150HasUse[TriTemp_Ch[iChannel]];
    if(bDefrost==true)                                                          //ATC 設定單點溫度
    {
        ATC_InterfaceForm->SetSingleTemp(iChannel, fTemp);
    }
    else if(bUse==false)
    {
        ATC_InterfaceForm->SetSingleTemp(iChannel, 25.0);
    }
#endif // SAFETY GATE (S9) + GATE(dep-TriTemp_Ch)
    (void)iChannel; (void)fTemp; (void)bDefrost;   // params write-only once gated
}
//------------------------------------------------------------------------------
// NOT gated -- golden's own body is a bounds check plus a plain
// `fCloseSiteTemp=fTemp;`, marked "No Use" by golden itself.
//------------------------------------------------------------------------------
void TfTemp_Set::SetCloseSiteTemperature(double fTemp)
{
    if(fTemp<10 || fTemp>150)                                                   //No Use
        return;

    fCloseSiteTemp=fTemp;
}
//------------------------------------------------------------------------------
//Ztex 2023.04.19 Add HT-1032 TriTemp Function ==>
void TfTemp_Set::cbSelectDeviceChange(TObject *Sender)
{
    AnsiString strSelectDefrostDevice = cbSelectDevice->Text;
    for(int i=0; i<7; i++)
    {
        if(DefrostFunction.strDefrostDevice[i]==strSelectDefrostDevice &&
           DefrostFunction.bDefrostReq[i]==true)
        {                                                                       //除霜相關功能
            btnDefrostStart->Enabled=false;
            btnDefrostEnd->Enabled  =true;
            break;
        }
        else
        {
            btnDefrostStart->Enabled=true;
            btnDefrostEnd->Enabled  =false;
        }
    }
}
//------------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S11) -- the full guard chain
// (SystemStart/iATC_On/AccessLevel/iWorkTemp checks, all ShowErrorMessage+
// return, no I/O) stays ACTIVE; the terminal defrost-start action is gated.
//------------------------------------------------------------------------------
void TfTemp_Set::btnDefrostStartClick(TObject *Sender)
{
    int iWorkTemp=fMain->edWorkTemperBase->Text.ToIntDef(25);
    int iATC_On=0;
    #ifdef SOFT_SIMULTE
        iATC_On=1;
    #else
        iATC_On=(iATCOnLine==true)?1:0;                                         //除霜相關功能
    #endif
    int iIndex=cbSelectDevice->ItemIndex;
    if(SystemStart==true)                                                       //Add Check Auo Run Not Manual Defrost
    {
        ShowErrorMessage("WAR31004", K_RETRY, MMSystem);
        return;
    }

    if(iATC_On!=1)                                                              //Add check ATC Must On-Line
    {
        ShowErrorMessage("WAR31005", K_RETRY, MMSystem);
        return;
    }

    if(AccessLevel<1)
    {
        ShowErrorMessage("WAR31006", K_RETRY, MMSystem);
        return;
    }

    if(iWorkTemp>=26)                                                           //Modify 25 ->26
    {
        ShowErrorMessage("WAR31007", K_RETRY, MMSystem);
        return;
    }
    //Modify Move Pos
    if(iIndex==-1)
        iIndex=0;

#if 0 // SAFETY GATE (S11)
    fTriMachineSelectDeviceDefrost_Start(iIndex);                               //將以下Code移至TriTemp.cpp內
    bManualDefrost_Start=true;                                                  //Add Defrost Abnormal
#endif // SAFETY GATE (S11)
    (void)iIndex;   // write-only once SAFETY GATE (S11) is inert
}
//------------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S12) -- whole tail (only the
// Defrost_End call).
//------------------------------------------------------------------------------
void TfTemp_Set::btnDefrostEndClick(TObject *Sender)
{
    int iIndex=cbSelectDevice->ItemIndex;
    if(iIndex==-1)
        iIndex=0;
#if 0 // SAFETY GATE (S12)
    fTriMachineSelectDeviceDefrost_End(iIndex);
#endif // SAFETY GATE (S12)
    (void)iIndex;   // write-only once SAFETY GATE (S12) is inert
}
//---------------------------------------------------------------------------
// On the task brief's explicit no-SAFETY-gate list (pure version-string
// parsing; `bDefrostNeedSendCommandToAtc=true;` is a plain flag write, not
// itself a hardware call) -- but a DEPENDENCY gate is needed regardless
// (orthogonal category): `ATC_InterfaceForm->asATC_SW_Ver` has no member on
// the current acarry_shims.h TATC_InterfaceFormShim (scoped to exactly
// `iATC_MODE_TYPE`). Whole body gated; returns golden's own "no version
// string" default (`iReturnStatus=1`, golden :6789-6792) instead of
// inventing a different fallback.
//---------------------------------------------------------------------------
int TfTemp_Set::CheckVerSupportFunction()                                       //Add chcke Atc Ver Support special Function
{
    //目前ATC版本 1.5.10以上支援冷媒機狀態回傳
    //目前ATC版本 1.6.7以上除霜需要額外送出除霜命令
    int iReturnStatus=1;
#if 0 // GATE(dep-ATC-asATC_SW_Ver)
    iReturnStatus=0;
    AnsiString asRecVer="";                                                     //No Use
    AnsiString asGetVer="";
    AnsiString AsVer[3]={"","",""};
    AnsiString asTranVer[2];
    int iVer[2]={0,0};
    if(ATC_InterfaceForm->asATC_SW_Ver=="")
    {
        iReturnStatus = 1;
        return iReturnStatus;
    }
    asRecVer=ATC_InterfaceForm->asATC_SW_Ver.Trim();
    AsVer[0]=asRecVer.SubString(1, asRecVer.Pos(".")-1);
    asGetVer=asRecVer.SubString(asRecVer.Pos(".")+1, asRecVer.Length());
    AsVer[1]=asGetVer.SubString(1, asGetVer.Pos(".")-1);
    asGetVer=asGetVer.SubString(asGetVer.Pos(".")+1, asGetVer.Length());
    AsVer[2]=asGetVer;
    asTranVer[0]=AsVer[0]+ AsVer[1];
    asTranVer[1]=AsVer[2];
    iVer[0]= atoi(asTranVer[0].c_str());
    iVer[1]= atoi(asTranVer[1].c_str());
    if(iVer[0]>15 || iVer[0]>=15 && iVer[1]>=10)
    {
        iReturnStatus=3;
    }
    else
    {
        iReturnStatus=2;
    }

    bDefrostNeedSendCommandToAtc = true;
#endif // GATE(dep-ATC-asATC_SW_Ver)
    return iReturnStatus;
}
//------------------------------------------------------------------------------
// NOT gated directly (see facade header S17 note): no direct hardware call
// here -- only guards and delegation to Tri_Temp_Low_Temperature_Use_Heater/
// Tri_Temp_High_Temperature_UseNo_Heater, both listed next.
//------------------------------------------------------------------------------
void TfTemp_Set::tmr_ATC_DeforstTimer(TObject *Sender)                          //Add chcke Atc Ver Support special Function
{
    if(InitialOK==false || Tri_Temp_Machine!=1)                                 //除霜相關功能
        return;
    // AI(W906-FW3-TempSet-WA) 20260820: DEPENDENCY GATE -- `ATC_InterfaceForm
    // ->IsConnect()`. golden `TATC_InterfaceFormShim` (acarry_shims.h) is
    // scoped to exactly `iATC_MODE_TYPE`; IsConnect is not among its members.
#if 0 // GATE(dep-ATC-IsConnect)
    if(bUT150Install[tcATCHotAir1]==true || bUT150Install[tcATCHotAir2]==true)
    {
        if(ATC_InterfaceForm->IsConnect()==false)
        {
            UN150Read[tcATCHotAir1]=Temperature.fWorkTemperBase;
            UN150Read[tcATCHotAir2]=Temperature.fWorkTemperBase;
        }
    }
#endif // GATE(dep-ATC-IsConnect)

    if(bALLTempFirstReady==true && Temperature.fWorkTemperBase>=25)
    {
        Tri_Temp_High_Temperature_UseNo_Heater();
    }
    else if(Temperature.fWorkTemperBase<=25)
    {
        Tri_Temp_Low_Temperature_Use_Heater();
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: GATE(G-Delegate) -- see facade header.
// golden invokes a VCL TNotifyEvent delegate directly as a function call
// (`->pl_ATC_Online->OnClick(fLotInfo)`); no vclcompat stock-widget stand-in
// carries an assignable/invokable OnClick slot. DEPENDENCY gap, not a safety
// concern -- this function is on the task brief's explicit no-gate list.
//---------------------------------------------------------------------------
void TfTemp_Set::ATC_Online_Or_offline()
{
    if(IniConfig.bL35_1OverSetTempOpenFan==true)                                //ATC 連線/斷線
    {
        bcheckSuperHotTemperatureKitTiming[1] = true;
        bcheckSuperHotTemperatureKitTiming[0] = true;
    }
#if 0 // GATE(G-Delegate)
    fLotInfo->pl_ATC_Online->OnClick(fLotInfo);
#endif // GATE(G-Delegate)
}
//---------------------------------------------------------------------------
// NOT gated -- see facade header S18 note: `spbSave->Click()` resolves to
// vclcompat::TControl::Click(), a permanent no-op (no OnClick delegate wiring
// exists anywhere in this tree, same class as G-Delegate above); ReadTempFile
// is a read, not a write.
//---------------------------------------------------------------------------
void TfTemp_Set::Tri_Temp_Low_Temperature_Use_Heater()
{
    bool bCheck=true;                                                           //檢查低溫需使用Heater

    if(chkTriTempEnableOutSht->Checked==false || Temperature.bUseTriTempHeater_Ini[0]==false)
        bCheck=false;

    if(chkTriTempEnableIndex->Checked==false || Temperature.bUseTriTempHeater_Ini[1]==false)
        bCheck=false;

    if(chkTriTempEnableBase->Checked==false || Temperature.bUseTriTempHeater_Ini[2]==false)
        bCheck=false;

    if(bCheck==false)
    {
        ReadTempFile(true);
        chkTriTempEnableOutSht->Checked=true;
        chkTriTempEnableIndex->Checked=true;
        chkTriTempEnableBase->Checked=true;
        spbSave->Click();                                                       // permanent no-op, see banner note above
    }
}
//---------------------------------------------------------------------------
// NOT gated -- see Tri_Temp_Low_Temperature_Use_Heater's own note (identical
// spbSave->Click() no-op reasoning).
//---------------------------------------------------------------------------
void TfTemp_Set::Tri_Temp_High_Temperature_UseNo_Heater()
{
    if(Temperature.bUseTriTempHeater_Ini[0]==false &&
       Temperature.bUseTriTempHeater_Ini[1]==false &&
       Temperature.bUseTriTempHeater_Ini[2]==false)
    {
        return;
    }
    Temperature.bUseTriTempHeater_Ini[0]=false;
    Temperature.bUseTriTempHeater_Ini[1]=false;
    Temperature.bUseTriTempHeater_Ini[2]=false;
    ReadTempFile(true);
    chkTriTempEnableOutSht->Checked=false;
    chkTriTempEnableIndex->Checked =false;
    chkTriTempEnableBase->Checked  =false;
    spbSave->Click();                                                           // permanent no-op, see Tri_Temp_Low_Temperature_Use_Heater's note
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S14) -- whole body (only the
// Send_ATC_DewPoint call).
//---------------------------------------------------------------------------
void TfTemp_Set::Handler_Send_To_ATC_DewPoint(double dDewPoint_Index, double dDewPoint_InSht, double dDewPointOutSht)
{
#if 0 // SAFETY GATE (S14)
    ATC_InterfaceForm->Send_ATC_DewPoint(dDewPoint_Index, dDewPoint_InSht, dDewPointOutSht);
#endif // SAFETY GATE (S14)
    (void)dDewPoint_Index; (void)dDewPoint_InSht; (void)dDewPointOutSht;   // params write-only once SAFETY GATE (S14) is inert
}
//Ztex 2023.04.19 Add HT-1032 TriTemp Function <==
//---------------------------------------------------------------------------
void TfTemp_Set::edt_SetAirstreamTemperatureRang_SocketClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 10.0, 0.0);
}
//---------------------------------------------------------------------------
void TfTemp_Set::edtSetTempature2AirMachineClick(TObject *Sender)
{
    int Tag;                                                                    //Ztex 2023.12.19 Add Air Stream Offset ==>
    TfTemp_SetTagEdit *Ptr;                                                     // golden `TEdit*` -- retyped to the Tag-carrying wrapper (see facade header DESIGN NOTE)
    Ptr=(TfTemp_SetTagEdit *)Sender;
    Tag=Ptr->Tag;
    TEdit *edt[2]={edt_SetIndexAirstreamTemp,edtSetTempature2AirMachine};
    double dsum=0.0;
    double dbSetTemp=atof(fMain->edWorkTemperBase->Text.c_str());

    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 20.0, -20.0);

    dsum=dbSetTemp+atof(edt[Tag]->Text.c_str());
    if(dsum<-70)
        edt[Tag]->Text=AnsiString(-70-dbSetTemp);
    if(Tag==0)
    {
        if(dsum>35)
            edt[Tag]->Text="0";
    }
    else
    {
        if(dsum>230)
            edt[Tag]->Text=AnsiString(230-dbSetTemp);
    }                                                                           //Ztex 2023.12.19 Add Air Stream Offset <==
}
//---------------------------------------------------------------------------
void TfTemp_Set::edt_AirVolumeLmt_IndexClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 1000.00, 350.00);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S15) -- whole body (pure
// action, no guard logic in golden either).
//---------------------------------------------------------------------------
void TfTemp_Set::btn_DefrostAllUseStartClick(TObject *Sender)
{
#if 0 // SAFETY GATE (S15)
    fTriMachineSelectDeviceDefrost_Start(0);
    fTriMachineSelectDeviceDefrost_Start(1);
    fTriMachineSelectDeviceDefrost_Start(2);
    bManualDefrost_Start=true;
#endif // SAFETY GATE (S15)
}
//---------------------------------------------------------------------------
void TfTemp_Set::edt_Defrost_Time_Too_LowerClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 1200.00, 600.00);
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: SAFETY GATE (S16) -- whole body.
//---------------------------------------------------------------------------
void TfTemp_Set::btn_DefrostAllUseEndClick(TObject *Sender)
{
#if 0 // SAFETY GATE (S16)
    fTriMachineSelectDeviceDefrost_End(0);
    fTriMachineSelectDeviceDefrost_End(1);
    fTriMachineSelectDeviceDefrost_End(2);
#endif // SAFETY GATE (S16)
}
//---------------------------------------------------------------------------
void TfTemp_Set::cbATCReferTempSensorClick(TObject *Sender)
{
    if(cbATCReferTempSensor->Checked)
    {
        cbUseTC2Offset->Visible=true;
    }
    else
    {
        cbUseTC2Offset->Visible=false;
        cbUseTC2Offset->Checked=false;
    }
}
//---------------------------------------------------------------------------
// AI(W906-FW3-TempSet-WA) 20260820: ATC_Power()/CheckAirMachineStatus() --
// declared in the facade header (matching golden's own header) but have NO
// body here, matching golden's own "declared but never defined, never
// called" state (see facade header OMITTED note).

//------------------------------------------------------------------------------
// AI(W906-I01C) 20261002 (Ifor01): golden TFTestIF::ReadTestIFFile (906 cTesterIF.cpp:562) -- only the path (:565-569)
//   and the [InitialMode] block (:689-758) that SaveSetupFile writes (GATE G-FTestIF, top of file: TFTestIF has no
//   facade; the rest of that reader is other pages' data). Copied from the cp950-decoded golden by line number (generator,
//   not retyped). Same subset as the C route's TS_ReadTestIFFile_InitialMode (FileRW/Temperature.gen.inc, translated
//   from V912 cTesterIF.cpp:713-779 -- the same text). Called from SaveSetupFile where golden calls FTestIF->ReadTestIFFile().
//------------------------------------------------------------------------------
static void UTS_ReadTestIFFile_InitialMode()
{
    AnsiString S="";   // golden cTesterIF.cpp:565
    S=GetLastOpenFN();   // golden cTesterIF.cpp:566
    AnsiString szDir="";//, szDir2;   // golden cTesterIF.cpp:567
    szDir=DataPath+S;   // golden cTesterIF.cpp:568
    szDir+="\\Tester.Data";   // golden cTesterIF.cpp:569
    //ChungHung 20140425 add for TSMC Device   // golden cTesterIF.cpp:689
    TestIF_File.bEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "bEveryFirstDeviceUseInitialDelay",          false);   // golden cTesterIF.cpp:690

    if(CosFunction.bAfterInitialDelayUseOtherArm)                               //Jerryyang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail   // golden cTesterIF.cpp:692
        TestIF_File.bUseOtherArmToTestAfterInitialDelay     =ReadIniData(szDir, "InitialMode",       "bUseOtherArmToTestAfterInitialDelay",       false);   // golden cTesterIF.cpp:693
    else   // golden cTesterIF.cpp:694
        TestIF_File.bUseOtherArmToTestAfterInitialDelay     =false;   // golden cTesterIF.cpp:695

    TestIF_File.bAfterShowAlarmMessageUseInitialDelay       =ReadIniData(szDir, "InitialMode",       "bAfterShowAlarmMessageUseInitialDelay",     false);   // golden cTesterIF.cpp:697
    TestIF_File.bWhenHappenTestedTimeBelowUseInitialDelay   =ReadIniData(szDir, "InitialMode",       "bWhenHappenTestedTimeBelowUseInitialDelay", false);   // golden cTesterIF.cpp:698
    TestIF_File.iEveryFirstDeviceUseInitialDelay            =ReadIniData(szDir, "InitialMode",       "iEveryFirstDeviceUseInitialDelay",          1.00);   // golden cTesterIF.cpp:699

    #ifdef SOFT_SIMULTE                                                         //Steven 20241108 : 軟體模擬有做五秒就好   // golden cTesterIF.cpp:701
    TestIF_File.iInitialDelay                               =5;   // golden cTesterIF.cpp:702
    TestIF_File.iInitialDelay_2                             =5;   // golden cTesterIF.cpp:703
    TestIF_File.iInitialDelay_3                             =5;   // golden cTesterIF.cpp:704
    TestIF_File.iInitialDelay_4                             =5;   // golden cTesterIF.cpp:705
    TestIF_File.iInitialDelay_5                             =5;   // golden cTesterIF.cpp:706
    TestIF_File.iInitialDelay_6                             =5;   // golden cTesterIF.cpp:707
    TestIF_File.iInitialDelay_7                             =5;   // golden cTesterIF.cpp:708
    TestIF_File.iInitialDelay_8                             =5;   // golden cTesterIF.cpp:709
    TestIF_File.iInitialDelay_9                             =5;   // golden cTesterIF.cpp:710
    TestIF_File.dInitialDelay_10                            =5;   // golden cTesterIF.cpp:711
    #else   // golden cTesterIF.cpp:712
    TestIF_File.iInitialDelay                               =ReadIniData(szDir, "InitialMode",       "iInitialDelay",            1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:713
    TestIF_File.iInitialDelay_2                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_2",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:714
    TestIF_File.iInitialDelay_3                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_3",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:715
    TestIF_File.iInitialDelay_4                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_4",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:716
    TestIF_File.iInitialDelay_5                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_5",          1.00);     //ChungHung 20141210 add for SCK want to every event have delay   // golden cTesterIF.cpp:717
    TestIF_File.iInitialDelay_6                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_6",          1.00);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:718
    TestIF_File.iInitialDelay_7                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_7",          1.00);     //wei 20151228 No FullSite delay   // golden cTesterIF.cpp:719
    TestIF_File.iInitialDelay_8                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_8",          1.00);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:720
    TestIF_File.iInitialDelay_9                             =ReadIniData(szDir, "InitialMode",       "iInitialDelay_9",          1.00);     //Steven 20160818 : OTD打開Delay   // golden cTesterIF.cpp:721
    TestIF_File.dInitialDelay_10                            =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10",         0.0);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41 wait time   // golden cTesterIF.cpp:722
    #endif   // golden cTesterIF.cpp:723

    if(CosFunction.bHiSiliconFunction && CUSTOMER_CODE==CC_ASE_KaohSiung)       //kevin 20191227 add  Initial Start Delay Time 需卡下限, 最小30秒.   // golden cTesterIF.cpp:725
    {   // golden cTesterIF.cpp:726
        if(TestIF_File.dInitStartDelayTime<30)                                  //kevin 20200110   // golden cTesterIF.cpp:727
        {   // golden cTesterIF.cpp:728
            TestIF_File.dInitStartDelayTime=30;   // golden cTesterIF.cpp:729
            TestIF_File.iInitialDelay=30;   // golden cTesterIF.cpp:730
        }   // golden cTesterIF.cpp:731
    }   // golden cTesterIF.cpp:732

    TestIF_File.bOTDUnlockDelay                             =ReadIniData(szDir, "InitialMode",       "bOTDUnlockDelay",                             false);     //Steven 20160818 : OTD打開Delay   // golden cTesterIF.cpp:734
    TestIF_File.bWhenPressStopOverUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bWhenPressStopOverUseInitialDelay",           false);     //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:735
    TestIF_File.iWhenPressStopOver                          =ReadIniData(szDir, "InitialMode",       "iWhenPressStopOver",                          1.00);      //ChungHung 20150526 add for ATK want to even stop over will use initial delay   // golden cTesterIF.cpp:736
    TestIF_File.bWhenNoFullSiteUseInitialDelay              =ReadIniData(szDir, "InitialMode",       "bWhenNoFullSiteUseInitialDelay",              false);     //wei 20151228 No FullSite delay   // golden cTesterIF.cpp:737
    TestIF_File.bAfterAutoCleanFunctionUseInitialDelay      =ReadIniData(szDir, "InitialMode",       "bAfterAutoCleanFunctionUseInitialDelay",      false);   // golden cTesterIF.cpp:738
    TestIF_File.bTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "bTestFinishToNextTestOver",                   false);     //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:739
    TestIF_File.iTestFinishToNextTestOver                   =ReadIniData(szDir, "InitialMode",       "iTestFinishToNextTestOver",                   1.00);      //kevin 20160310 測試完成到下一次完成超過所設時間須啟動   // golden cTesterIF.cpp:740
    TestIF_File.bAfterOpenHeatDoorUseInitialDelay           =ReadIniData(szDir, "InitialMode",       "bAfterOpenHeatDoorUseInitialDelay",           false);     //ChungHung 20141210 add for SCK want to after Open HeatDoor have delay time   // golden cTesterIF.cpp:741
    TestIF_File.bTestStartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "bTestStartToNextTestStart",                   false);     //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41   // golden cTesterIF.cpp:742
    TestIF_File.dTeststartToNextTestStart                   =ReadIniData(szDir, "InitialMode",       "dTeststartToNextTestStart",                   1.00);      //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41  over time   // golden cTesterIF.cpp:743

    //Steven 20190313 : Initial Start Delay use different setting in FT and RT   // golden cTesterIF.cpp:745
    //==>   // golden cTesterIF.cpp:746
    TestIF_File.dInitialDelay_1_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_1_RT",              0.0);   // golden cTesterIF.cpp:747
    TestIF_File.dInitialDelay_2_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_2_RT",              0.0);   // golden cTesterIF.cpp:748
    TestIF_File.dInitialDelay_3_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_3_RT",              0.0);   // golden cTesterIF.cpp:749
    TestIF_File.dInitialDelay_4_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_4_RT",              0.0);   // golden cTesterIF.cpp:750
    TestIF_File.dInitialDelay_5_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_5_RT",              0.0);   // golden cTesterIF.cpp:751
    TestIF_File.dInitialDelay_6_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_6_RT",              0.0);   // golden cTesterIF.cpp:752
    TestIF_File.dInitialDelay_7_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_7_RT",              0.0);   // golden cTesterIF.cpp:753
    TestIF_File.dInitialDelay_8_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_8_RT",              0.0);   // golden cTesterIF.cpp:754
    TestIF_File.dInitialDelay_9_RT                          =ReadIniData(szDir, "InitialMode",       "dInitialDelay_9_RT",              0.0);   // golden cTesterIF.cpp:755
    TestIF_File.dInitialDelay_10_RT                         =ReadIniData(szDir, "InitialMode",       "dInitialDelay_10_RT",             0.0);   // golden cTesterIF.cpp:756
    //<==   // golden cTesterIF.cpp:757
    //Steven 20190313 : Initial Start Delay use different setting in FT and RT   // golden cTesterIF.cpp:758
}
