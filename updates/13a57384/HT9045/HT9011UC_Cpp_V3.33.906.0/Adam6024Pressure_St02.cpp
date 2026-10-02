// ===========================================================================
//  Adam6024Pressure_St02.cpp  --  AI(W906-ST02-ADAM) 20261002 (St02-E helper H2)
//
//  golden 912 adam6024.cpp (D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy, cp950), word for word:
//      IsMultiEPPressureRouteActive          :96-105     (10 lines)
//      IsIndependentEPPressureRouteActive    :108-120    (13)
//      AdamOutputToPA                        :528-541    (14)
//      ADAM_Rang                             :543-546    (4)
//      iAdamOutValue                         :548        (the global between them)
//      ADAM_Alarm                            :549-605    (57)
//      ADAM_Alarm_Kg                         :607-668    (62)
//      ADAM_DualAlarm                        :670-712    (43)
//      KpaTransferKG                         :714-972    (259)
//      MultiTransferKG                       :974-1041   (68, with golden's banner)  [912]
//      ADAM_ReturnValueCheck                 :2675-2759  (85)
//  plus the file-scope names only this code uses: iADAMRange :48, dADAMRange_Kg :49 and the MNetLog
//  prototype :56 (ownership: Adam6024_St02.h:48-67, H1's header).
//
//  906 -> 912 inside these ranges (difflib over 906_0625_Steven and 912, 20261002): ONE change.
//  MultiTransferKG's argument is `double fInputMPA` in 912 (:978, Eastsun 20260710 "修正沒有小數點"),
//  `int fInputMPA` in 906 (:973).  Marked [912] at the line.  Every other line equals 906 (906 numbers are
//  912 minus 4 up to :1041 and minus 2 at ReturnValueCheck: 906 :545 / :710 / :973 / :2673).
//
//  WHERE THE GOLDEN NAMES LIVE IN THE PORT
//  ---------------------------------------------------------------------------
//  * fContactForce->SLKClass / DieForceSLKClass  ->  cft = ContactForceTables() (ContactForce.h:326-353),
//    the same mechanical mapping as the laptop's TransformFuntion (adam6024.cpp:17-36):
//        fContactForce->SLKClass[i]->dLoadRate   ->  cft.SLKClass.items[i].dLoadRate
//        fContactForce->SLKClass.size()          ->  cft.SLKClass.size()
//    Fields read here: dDiameter, dLoadRate, dLoadRate_NS, dHotOffset (all SlkForceData, ContactForce.h:40-52).
//    No Count (slSLKType...->Count) is read by these two functions, so the Tokens-vs-size() trap
//    (ContactForce.h:334-348) does not arise here.
//    !! CALLER MUST HAVE LOADED THE TABLES (ContactForce.h:317-324, the "silent-30.0 defect" warning).  golden
//    has no check (its TfContactForce ctor fills them); neither has this file, exactly like TransformFuntion
//    (adam6024.cpp:38-46).  The port's guarantee is wb_serve's bring-up LoadContactForceTables()
//    (tools/wb_serve.cpp:4095).  On an EMPTY table golden's `items[0]` fallback is undefined behaviour.
//  * golden `fContact` (TfContact, cContact.h:667).  The port has two objects for that name:
//    `TfContactShim *fContact` (atester_shims.h:154-251: fShow and nine other members, no widgets) and
//    `TfContact *fContactForm` (forms/fContact.h:1634, forms/fContact.cpp:90: the facade that carries golden's
//    widget members).  The members golden touches here -- edSetKg, edDoubleForce (Text read), lblReadEP,
//    lblReadEP2, lblDieForceEP (Caption write), fShow -- exist only on fContactForm, so fContact->X becomes
//    fContactForm->X (precedent: cStateRecord.cpp:1578-1579, csystem.cpp:3133 / :20039 "golden fContact =
//    fContactForm").  This TU cannot include atester_shims.h anyway (Adam6024_St02.h:36-46).
//    golden `fContact->fShow` -> W906_FormShowing("fContact", fContactForm->fShow), St01's page table
//    (W906FormShowing.h:10-16), the tree-wide idiom (ainarm9045.cpp:6431, AutoClean.cpp:1386, ...).  Both
//    member flags only ever hold false (forms/fContact.cpp:492, atester_shims.cpp:292); under wb_serve the
//    page table gives the real "Contact page open" answer.
//    !! fContactForm->edSetKg->Text and ->edDoubleForce->Text are EMPTY in the port.  golden fills
//    edDoubleForce in TfContact::DoIniDataToForm (cContact.cpp:951, run at every device load: main.cpp:9384,
//    csystem.cpp:23550, cBuilder.cpp:515) and edSetKg only in TransformFuntion (:1803).  In the port both
//    writers are gated: forms/fContact.h:1497 DoIniDataToForm GATE (W-02), adam6024.cpp:862 GATE
//    (W906-P2B-CONTACTDISPLAY).  The web Contact page has its own copy (FileRW EL proxy
//    "TfContact"/"edDoubleForce", FileRW/DeviceForm_File.gen.inc:2249), linkable only inside wb_serve.
//    So ADAM_Alarm_Kg and ADAM_DualAlarm, which have NO caller in the port today, would throw
//    std::runtime_error from AnsiString::ToDouble("") (vclcompat/AnsiString.cpp:185-194; BCB6 raises
//    EConvertError the same way) the first time they ran.  Kept as golden reads the widget; whoever wires a
//    caller (golden HS_Function.cpp:936-960 / :1117-1127) must first give fContactForm those two texts.
//  * ADAM_ReadPA / fCheckConnectStatus_ADAM6024 / bConnectStatus / iWritePA: H1 (Adam6024Comm_St02.cpp,
//    declared in Adam6024_St02.h).  MNetLog: Motor/myMN200motor.cpp:2516 (its golden body is gated there, it
//    returns true and records nothing).  ShowErrorMessage / ShowMyMessage: canary_support.h:66 / :80.
//
//  [W906] DEVIATIONS (each also at its line)
//  ---------------------------------------------------------------------------
//  D1 S25 (RULINGS_20260925 S25 "customer-specific conditions: skip for now, annotate"): the customer `if`
//     head and body are inside `#if 0 // GATE (S25)`, golden's generic else stays live (the MainTimer3.cpp:42-43
//     pattern).  Every other customer runs exactly golden; the named customer's machine runs the generic arm.
//       (gated lines = the customer `if`, its body and the `else` keyword; live = golden's generic arm)
//       ADAM_Alarm            gated :568-592   CC_JCET / CC_KYEC_LEE fixed-or-percentage range   live :593-601
//       KpaTransferKG         gated :731-736   CC_ASE_KaohSiung: NS switch only with bNSKitPress  live :737-739
//       KpaTransferKG         gated :844-854   CC_KYEC_LEE 2.8 -> 3.0 / 5.8 -> 6.0 diameter fix   (no else)
//       KpaTransferKG         gated :866-938   CC_KYEC_LEE NS kit by test mode                     live :939-941
//       MultiTransferKG       gated :1011-1021 CC_KYEC_LEE 2.8 / 5.8 diameter fix                  (no else)
//       ADAM_ReturnValueCheck :2729      `&& CUSTOMER_CODE!=CC_GIGAS` (voltage check off for GIGAS) -> the
//                                        check runs on a GIGAS machine too (more alarms, never fewer)
//     NOTE the asymmetry with the laptop's TransformFuntion (adam6024.cpp:100, :502-512, :534-619, written
//     20260919, before S25), which keeps CC_ASE_KaohSiung / CC_KYEC_LEE / CC_ASE_SG live: on a KYEC or
//     ASE-Kaohsiung machine KpaTransferKG may pick another SLK row than the force writer did.  No other
//     machine is affected.  The CosFunction flag branches (bEPUseNSSLK = KYEC NS head, CosFunction.cpp:938;
//     bUseLoadCellOffsetByHeater = TSMC, CosFunction.cpp:284) stay LIVE here as they do in TransformFuntion:
//     they are not CUSTOMER_CODE tests and are false on every other machine (CosFunction.cpp:4244 / :4313).
//  D2 A3 tolerance (RULINGS A3 20260924 + the 20260917 standing ruling; docs/FP_ORACLE_FINDINGS.md section
//     8.4-8.5): `double d=...dDiameter/10.0; if(d==fDiameter)` becomes `(d - fDiameter < 1e-6 &&
//     d - fDiameter > -1e-6)` at KpaTransferKG :827 / :856 and MultiTransferKG :993 / :1023, the same rewrite
//     the laptop made at adam6024.cpp:478 / :514 (g++ -O3 keeps the computed d in an 80-bit register, BCB6
//     -Od -r- stores it to a 64-bit double first).  Same line, golden text in the comment.  The KYEC
//     `d==2.8` / `d==5.8` lines are inside S25 gates and kept verbatim (apply A3 like adam6024.cpp:504/:508
//     if they are ever lifted).  `fDiameter==6.0 / 4.0 / 5.6 / 0 / 40.2` compare a value loaded from memory
//     (DeviceForm_File) -> BCB6 and both g++ levels agree (section 8.3-8.4) -> golden `==` kept.
//  D3 fContact -> fContactForm / W906_FormShowing (above).
//  D4 TEST SEAMS.  ADAM_Alarm, ADAM_Alarm_Kg, ADAM_DualAlarm and ADAM_ReturnValueCheck are compiled inside
//     `namespace w906_adampress`, where three same-named callees -- ADAM_ReadPA, fCheckConnectStatus_ADAM6024,
//     MNetLog -- hide the global ones and forward through function pointers whose DEFAULTS ARE THE GLOBAL
//     FUNCTIONS (H3's pattern, Adam6024Apax_St02.cpp:71-77).  Golden lines read word for word; production
//     behaviour is unchanged.  The global symbols are one-line forwarders with golden's signatures at the end.
//     Setters (0 restores golden's callee): W906_AdamPressSetReadPA / W906_AdamPressSetCheckConnect /
//     W906_AdamPressSetNetLog (tests/test_adam6024_pressure.cpp).
//  D5 `(void)iArm;` / `(void)bHome;` in the SOFT_SIMULTE arms (golden compiles the parameter out there).
//  D6 H4's ONE live switch (Adam6024Integrate_St02.cpp:23-44, requirement R2 for H1-H3): while
//     W906_AdamEpLive() is false -- the build default, `W906_ADAM_EP_LIVE` not defined -- ADAM_Alarm,
//     ADAM_Alarm_Kg and ADAM_DualAlarm answer like the retired stand-ins (false, atester_shims.cpp:328-329;
//     the last two had none) and ADAM_ReturnValueCheck does nothing (the retired csystem.cpp:6369 seam):
//     no ADAM read at all.  Switched on, the golden bodies run.  Without it, a SHIP build with the EP writers
//     still off would read the ADAM in IndexEveryTimeCheckEP (atester.cpp:9662-9667) and raise WAR1605.
//     One `if` in each global forwarder at the end of this file.  If St02-E drops the switch, delete those four
//     `if` lines and the W906_AdamEpLive declaration below; nothing else depends on it.
//
//  GOLDEN QUIRKS, KEPT (golden's behaviour, not port bugs)
//  ---------------------------------------------------------------------------
//  Q1 ADAM_Alarm compares the read-back kPa with AdamOutputToPA(iWritePA): the kPa of the LAST DA code
//     ADAM_DirectWriteData / ADAM_WriteVoltage sent (H1, golden :1875 / :1912 / :1993 / :2008).
//  Q2 AdamOutputToPA: (EP_MAXKPA-EP_MINMPA*1000)/(fMaxUnit+1)*code -- 4096 steps over the span, EP_MINMPA not
//     added back, stored in `int iPA` (truncated) and returned as double.  EP_Install 0 / 4 -> fMaxUnit 0 ->
//     the span times the code.
//  Q3 ADAM_DualAlarm: iType 0 hands the Die Force KG text (ToDouble, truncated to int) to AdamOutputToPA as if
//     it were a DA code; iType 1 truncates the read kg to int (`PA=dDualTempKg;`); any other iType leaves
//     dOutValue uninitialised (callers pass 0 / 1).  Its two log sprintf pass the int PA to %f (undefined
//     varargs in golden too); MNetLog keeps nothing in the port, so nothing is printed.
//  Q4 KpaTransferKG: the `bDualForce && DOUBLE_EP_INDIVIAL` arm keeps fDiameter = DeviceForm_File.dKitDiameter
//     (not the Die Force diameter); the table walk has no break (last match wins); `iTag>(int)size()` is `>`
//     not `>=` (harmless: iTag is -1 or an index).  "無安裝dual force" is shown when bDualForce is asked
//     with INSTALL_DOUBLE_EP==0, then the Die Force table is read anyway.
//  Q5 MultiTransferKG: after a 40.2 match fDiameter becomes 4.0 and the walk goes on comparing later rows
//     with 4.0 (last match wins).
//  Q6 ADAM_ReturnValueCheck: iEPVoltageErrorCount[] is a function static that a GOOD read never clears --
//     the alarm comes at the 101st bad read in total, not the 101st in a row (golden's "100 sec" comment).
//     bHome raises at the first bad read and clears fAllMotorHome (the homing fails); the counter is then
//     not reset.  EP_Install==3 reads channel 2 (Die Force EP) only when INSTALL_DOUBLE_EP==1.
//  Q7 the keep-alive read in ADAM_ReturnValueCheck's else (every 61st call) discards its value.
// ===========================================================================
#define _USE_MATH_DEFINES           // M_PI under -std=c++17 (__STRICT_ANSI__), as adam6024.cpp:48

#define ADAM6024_ST02_INTERNAL      // Adam6024_St02.h:162-185: bConnectStatus, iWritePA, iADAMRange, dADAMRange_Kg, AdamOutputToPA
#include "Adam6024_St02.h"          // H1: golden 912 adam6024.h declarations (ADAM_ReadPA, fCheckConnectStatus_ADAM6024, ...)
#include "MachineType.h"            // SOFT_SIMULTE (:48 unless W906_NO_SOFT_SIMULTE), CC_*, eATC30, eht4Heater, test modes
#include "cmydef.h"                 // EP_Install, EP_MAXKPA, EP_MINMPA, INSTALL_DOUBLE_EP, DOUBLE_EP_*, SwMultiEp, iReadAdamEP,
                                    // fAllMotorHome, K_RETRY, MMSystem, ATC_SYSTEM, USE_16_HEATER, Tempture_Hot
#include "cprod.h"                  // TestIF, TestIF_File, DeviceForm_File, Temperature
#include "CosFunction.h"            // CosFunction
#include "Config.h"                 // IniConfig
#include "LastSet.h"                // LastSet.dIndexLoadRate / iTemperature
#include "ContactForce.h"           // SlkForceTables / ContactForceTables()
#include "myswitch.h"               // SW[] (TMySwitch)
#include "canary_support.h"         // ShowErrorMessage (:66), ShowMyMessage (:80)
#include "cpublic.h"                // GetFloatFormatString (:17)
#include "W906FormShowing.h"        // W906_FormShowing (body csystem.cpp, ht9045_sm)
#include "forms/fContact.h"         // fContactForm (golden fContact, see banner)

#include <cmath>

//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:48-49 / :56 (owners: Adam6024_St02.h:179-180 = H2; MNetLog body Motor/myMN200motor.cpp:2516)
int iADAMRange=0;                                                               //20111217 ChungHung
double dADAMRange_Kg=0;                                                         //JerryYang 20171023 (wei) add ADAM Range
extern bool MNetLog(AnsiString Message);

//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:95-105
//AI(ht9045-v899) 20260526: Multi EP pressure output is valid only when the physical SwMultiEp valve is actually ON.
bool IsMultiEPPressureRouteActive()
{
    if(INSTALL_DOUBLE_EP!=DOUBLE_EP_MULTI || TestIF_File.bIndEPSLK!=true)
        return false;

    if(SW[SwMultiEp].Enable==false)
        return false;

    return (SW[SwMultiEp].Status()==true);
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:107-120
//AI(ht9045-v899) 20260526: keep mode 2 independent EP behavior, but let mode 3 fall back to normal EP when SwMultiEp is OFF.
bool IsIndependentEPPressureRouteActive()
{
    if(TestIF_File.bIndEPSLK!=true)
        return false;

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL)
        return true;

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
        return IsMultiEPPressureRouteActive();

    return false;
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:528-541 (Q2)
double AdamOutputToPA(int iAdamOutput)                                          //JerryYang 20171030 (wei) adam output轉成PA
{
    double fMaxUnit=0.0;
    if(EP_Install==1 || EP_Install==3 || EP_Install==5)
    {
        fMaxUnit=4095.0;
    }
    else if(EP_Install==2)
    {
        fMaxUnit=1022.0;
    }
    int iPA=((EP_MAXKPA-EP_MINMPA*1000.0)/(fMaxUnit+1))*iAdamOutput;
    return iPA;
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:543-546
void ADAM_Rang(int v)
{
    iADAMRange=v;
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:548 (extern in golden adam6024.h:28 = Adam6024_St02.h:101; H1's ADAM_WriteVoltage writes it too, :1913)
double iAdamOutValue=0.0;
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:714-972
double KpaTransferKG(int fInputMPA, bool bDualForce)                            //Ifor 20150826 :新增 Kpa 轉 公斤 Function  //JerryYang 20171023 (wei) int -> double
{
    SlkForceTables& cft = ContactForceTables();                                 // [W906] golden fContactForce-> (banner; caller must have loaded the tables)
    double  fTestvalue,
            fTeskKG,
            fLoadRate,
            fDiameter;

    double  dIndex60mmLoadRate ,
            dIndex40mmLoadRate ,
            dIndex30mmLoadRate ,
            dIndex56mmLoadRate ;                                                //wei 20151005 add 56mm

    bool bNSKit=false, bNSKitSwitch=false;
    int iTag=-1;
    if(CosFunction.bEPUseNSSLK==true)                                           //kevin 20170804 (Steven) EP表頭另一種TYPE
    {
        bNSKit=true;                                                            //使用NS KIT
#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_ASE_KaohSiung only (D1) -- golden :731-736 VERBATIM, golden's else below stays live
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(TestIF_File.bNSKitPress)
                bNSKitSwitch=true;                                              //使用NS KIT
        }
        else
#endif  // GATE (S25)
        {
            bNSKitSwitch=true;
        }
    }

    if(CosFunction.bUseLoadCellOffsetByHeater &&
       LastSet.iTemperature==Tempture_Hot)                                      //2014-06-26    Dell    for TSMC 高溫Load cell offset
    {
        dIndex60mmLoadRate=LastSet.dIndexLoadRate[0][0]+LastSet.dIndexLoadRate[2][0];
        dIndex56mmLoadRate=LastSet.dIndexLoadRate[0][1]+LastSet.dIndexLoadRate[2][1];   //wei 20151005 add 56mm
        dIndex40mmLoadRate=LastSet.dIndexLoadRate[0][2]+LastSet.dIndexLoadRate[2][2];
        dIndex30mmLoadRate=LastSet.dIndexLoadRate[0][3]+LastSet.dIndexLoadRate[2][3];
    }
    else if(bNSKit && bNSKitSwitch &&                                                                               //kevin 20170804 (Steven) 使用另一種EP 壓力表
            (TestIF_File.bNSKitPress ||
             TestIF_File.bNS7000kit  ||
             TestIF_File.bNS7000CS   ||
             TestIF_File.bNS8000CS))                                            //wei 20150303   京元NS浮動頭
    {
        dIndex60mmLoadRate = LastSet.dIndexLoadRate[1][0];
        dIndex56mmLoadRate = LastSet.dIndexLoadRate[1][1];                      //wei 20151005 add 56mm
        dIndex40mmLoadRate = LastSet.dIndexLoadRate[1][2];
        dIndex30mmLoadRate = LastSet.dIndexLoadRate[1][3];
    }
    else
    {
        dIndex60mmLoadRate = LastSet.dIndexLoadRate[0][0];
        dIndex56mmLoadRate = LastSet.dIndexLoadRate[0][1];                      //wei 20151005 add 56mm
        dIndex40mmLoadRate = LastSet.dIndexLoadRate[0][2];
        dIndex30mmLoadRate = LastSet.dIndexLoadRate[0][3];
    }

    fDiameter=DeviceForm_File.dKitDiameter;

    if(bDualForce==true && INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL)                                //Ifor 20191003 : add Die Force 可以自定義Kit直徑
    {
        if(fDiameter==6.0)
        {
            fLoadRate=dIndex60mmLoadRate;                                       //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex60mmLoadRate
        }
        else if(fDiameter==4.0)
        {
            fLoadRate=dIndex40mmLoadRate;                                       //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex40mmLoadRate
        }
        else if(fDiameter==5.6)                                                 //wei 20151005 add 56mm
        {
            fLoadRate=dIndex56mmLoadRate;
        }
        else
        {
            if(fDiameter==0)                                                    //Steven 20140627 : 避免分母為0
                fDiameter=3.0;
            fLoadRate=dIndex30mmLoadRate;
        }
    }
    else
    {
        if(CosFunction.bUseDynamicKitDiameter==false)                           //Steven 20170605 (wei) : 可以自定義Kit直徑
        {                                                                       //Wei 20220217 : Add for EP回授數值計算
            if(fDiameter==6.0)
            {
                fLoadRate=dIndex60mmLoadRate;                                   //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex60mmLoadRate
            }
            else if(fDiameter==4.0)
            {
                fLoadRate=dIndex40mmLoadRate;                                   //wei 20150303     IniConfig.dIndex40mmLoadRate-->dIndex40mmLoadRate
            }
            else if(fDiameter==5.6)                                             //wei 20151005 add 56mm
            {
                fLoadRate=dIndex56mmLoadRate;
            }
            else
            {
                if(fDiameter==0)                                                //Steven 20140627 : 避免分母為0
                    fDiameter=3.0;
                fLoadRate=dIndex30mmLoadRate;
            }
        }
        else
        {
            if(bDualForce==true)                                                //Ifor 20191003 : add Die Force 可以自定義Kit直徑
            {
                if(INSTALL_DOUBLE_EP==0)                                        //JerryYang 20210119 : 增加dual EP防呆
                {
                   ShowMyMessage("無安裝dual force, 請確認硬體選項");
                }
                fDiameter=DeviceForm_File.dDieForceKitDiameter;
                for(unsigned int i=0; i<cft.DieForceSLKClass.size(); i++)
                {
                    double d=cft.DieForceSLKClass.items[i].dDiameter/10.0;
                    if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)           //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [W906] D2 A3 tolerance (FP_ORACLE_FINDINGS.md 8.5, as adam6024.cpp:478); golden :827 `if(d==fDiameter)`
                    {
                        iTag=i;
                    }
                }

                if(iTag==-1 || iTag>(int)cft.DieForceSLKClass.size())
                    iTag=0;

                fLoadRate=cft.DieForceSLKClass.items[iTag].dLoadRate;
            }
            else
            {
                for(unsigned int i=0; i<cft.SLKClass.size(); i++)
                {
                    double d=cft.SLKClass.items[i].dDiameter/10.0;

#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_KYEC_LEE only (D1) -- golden :844-854 VERBATIM (if lifted, A3 like adam6024.cpp:504/:508)
                    if(CUSTOMER_CODE==CC_KYEC_LEE)                              //Ifor 20200407 : Fix KYEC 特殊缸徑造成資料異常
                    {
                        if(d==2.8)
                        {
                            d=3.0;
                        }
                        else if(d==5.8)
                        {
                            d=6.0;
                        }
                    }
#endif  // GATE (S25)

                    if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)           //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [W906] D2 A3 tolerance (FP_ORACLE_FINDINGS.md 8.5, as adam6024.cpp:514); golden :856 `if(d==fDiameter)`
                    {
                        iTag=i;
                    }
                }

                if(iTag==-1 || iTag>(int)cft.SLKClass.size())
                    iTag=0;

                bool bUseNSKit=false;
#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_KYEC_LEE only (D1) -- golden :866-938 VERBATIM, golden's else below stays live
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    switch(TestIF.iTestMode)
                    {
                        case SingleSite:
                        case DualSite:
                            if(CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case QualSite1X4:
                            if((CosFunction.bCanUseBias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case QualSite2X2:
                            if((CosFunction.bCanUse2x2Bias==true && TestIF_File.bNS7000kit==true) || TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case DualSite2x1:
                            if(TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case TriSite1X3:
                        case _8Site1X4:
                            if(ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true)
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        case _8Site2X4:
                            if((ATC_SYSTEM>eATC30 && TestIF_File.bNS7000CS==true) ||
                               (USE_16_HEATER!=eht4Heater && TestIF_File.bNS8000CS==true))
                            {
                                bUseNSKit=true;
                            }
                            else
                            {
                                bUseNSKit=false;
                            }
                            break;
                        default :
                            bUseNSKit=false;
                            break;
                    }
                }
                else
#endif  // GATE (S25)
                {
                    bUseNSKit=false;
                }

                if(bNSKit && bNSKitSwitch &&                                    //kevin 20170804 (Steven) 使用另一種EP 壓力表
                   (TestIF_File.bNSKitPress ||
                    TestIF_File.bNS7000kit ||
                    TestIF_File.bNS7000CS ||
                    TestIF_File.bNS8000CS))                                     //wei 20150303   京元NS浮動頭
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate_NS;
                }
                else if(bUseNSKit==true)
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate_NS;
                }
                else
                {
                    fLoadRate=cft.SLKClass.items[iTag].dLoadRate;
                }

                if(CosFunction.bUseLoadCellOffsetByHeater &&
                   LastSet.iTemperature==Tempture_Hot)                          //2014-06-26    Dell    for TSMC 高溫Load cell offset
                {
                    fLoadRate+=cft.SLKClass.items[iTag].dHotOffset;
                }
            }
        }
    }
    fTestvalue  =fInputMPA*10.197;
    fTeskKG     =fTestvalue*(fDiameter*fDiameter*M_PI/4.0*fLoadRate);
    fTeskKG     =(fTeskKG/1000.0);
    return  fTeskKG;
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:974-1041 (Q5)
//AI(ht9045-v899) 20260504: port from V896 (Ifor 20250416). Multi EP per-site Kpa->Kg.
//   Simplified KpaTransferKG: only handles dKitDiameter / dDieForceKitDiameter,
//   includes KYEC special diameter (28->30, 58->60, 40.2->4.0) per V896 logic.
//double MultiTransferKG(int fInputMPA, bool bDualForce)  //Eastsun 20260710 Merge
double MultiTransferKG(double fInputMPA, bool bDualForce)  //Eastsun 20260710 Merge //Eastsun 修正沒有小數點   [912] golden 912 :978; 906 :973 `double MultiTransferKG(int fInputMPA, bool bDualForce)`
{
    SlkForceTables& cft = ContactForceTables();                                 // [W906] golden fContactForce-> (banner; caller must have loaded the tables)
    double fTestvalue, fTeskKG, fLoadRate, fDiameter;
    int iTag=-1;

    if(bDualForce==true)
    {
        if(INSTALL_DOUBLE_EP==DOUBLE_EP_NONE)
        {
            ShowMyMessage("No dual force installed, please check hardware option");
        }
        fDiameter=DeviceForm_File.dDieForceKitDiameter;
        for(unsigned int i=0; i<cft.DieForceSLKClass.size(); i++)
        {
            double d=cft.DieForceSLKClass.items[i].dDiameter/10.0;
            if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)                   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [W906] D2 A3 tolerance; golden :993 `if(d==fDiameter)`
            {
                iTag=i;
            }
        }

        if(iTag==-1 || iTag>(int)cft.DieForceSLKClass.size())
            iTag=0;

        fLoadRate=cft.DieForceSLKClass.items[iTag].dLoadRate;
    }
    else
    {
        fDiameter=DeviceForm_File.dKitDiameter;
        for(unsigned int i=0; i<cft.SLKClass.size(); i++)
        {
            double d=cft.SLKClass.items[i].dDiameter/10.0;

#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_KYEC_LEE only (D1) -- golden :1011-1021 VERBATIM (if lifted, A3 like adam6024.cpp:504/:508)
            if(CUSTOMER_CODE==CC_KYEC_LEE)
            {
                if(d==2.8)
                {
                    d=3.0;
                }
                else if(d==5.8)
                {
                    d=6.0;
                }
            }
#endif  // GATE (S25)

            if(d - fDiameter < 1e-6 && d - fDiameter > -1e-6)                   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): [W906] D2 A3 tolerance; golden :1023 `if(d==fDiameter)`
            {
                iTag=i;
                if(fDiameter==40.2)
                    fDiameter=4.0;
            }
        }

        if(iTag==-1 || iTag>(int)cft.SLKClass.size())
            iTag=0;

        fLoadRate=cft.SLKClass.items[iTag].dLoadRate;
    }

    fTestvalue = fInputMPA*10.197;
    fTeskKG = fTestvalue*(fDiameter*fDiameter*M_PI/4.0*fLoadRate);
    fTeskKG = (fTeskKG/1000.0);
    return fTeskKG;
}

//===========================================================================
// [W906] D4 test seams.  Defaults = golden's own (global) callees, so production behaviour is golden's.
//===========================================================================
void W906_AdamPressSetReadPA(int (*fn)(double*, int));
void W906_AdamPressSetCheckConnect(bool (*fn)(int));
void W906_AdamPressSetNetLog(bool (*fn)(AnsiString));

namespace {

int AdamPressDefaultReadPA(double *dValue, int iCH)
{
    return ::ADAM_ReadPA(dValue, iCH);
}
bool AdamPressDefaultCheckConnect(int Num)
{
    return ::fCheckConnectStatus_ADAM6024(Num);
}
bool AdamPressDefaultNetLog(AnsiString Message)
{
    return ::MNetLog(Message);
}

int  (*g_pAdamPressReadPA)(double*, int)   = AdamPressDefaultReadPA;
bool (*g_pAdamPressCheckConnect)(int)      = AdamPressDefaultCheckConnect;
bool (*g_pAdamPressNetLog)(AnsiString)     = AdamPressDefaultNetLog;

} // namespace

namespace w906_adampress {

// The three hidden callees (ordinary name hiding inside this namespace; same defaults as golden adam6024.h:8).
int ADAM_ReadPA(double *dValue, int iCH=5)
{
    return g_pAdamPressReadPA(dValue, iCH);
}
bool fCheckConnectStatus_ADAM6024(int Num)
{
    return g_pAdamPressCheckConnect(Num);
}
bool MNetLog(AnsiString Message)
{
    return g_pAdamPressNetLog(Message);
}

//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:549-605 (Q1)
bool ADAM_Alarm(int iArm)
{
    #ifdef SOFT_SIMULTE
        (void)iArm;                                                             // [W906] D5
        return false;
    #else
        AnsiString AlarmMsg;
        double dValue=0.0;
        if(!bConnectStatus[0])                                                  //Hmy 20170120 add check Adam6024 Connect Status
            bConnectStatus[0] = fCheckConnectStatus_ADAM6024(0);                //Nickliu 20230330 add check statsu

        int PA;                                                                 //wei 20220309 Add EP Return Voltage

        if(EP_Install==5)
            PA=ADAM_ReadPA(&dValue, iArm);
        else
            PA=ADAM_ReadPA(&dValue);

        iReadAdamEP=PA;                                                         //jou 20170413 (Steven) : Read Adam EP 提升UPH
        iAdamOutValue=AdamOutputToPA(iWritePA);
#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_JCET / CC_KYEC_LEE only (D1) -- golden :568-592 VERBATIM, golden's else below stays live (a JCET / KYEC machine uses the fixed +-iADAMRange)
        if(CUSTOMER_CODE==CC_JCET ||                                            //Richard 20230426 : JECT ADD 固定值改為百分比 //Ifor add KYEC_LEE
           CUSTOMER_CODE==CC_KYEC_LEE)                                          //Eastsun 20260511 F008 整合
        {
            if(IniConfig.iD26_3FixValueOrPercentage==1)                         //Richard 20230428 : EP固定值或百分比
            {
                if(PA>iAdamOutValue*(1+iADAMRange/100.0) ||
                   PA<iAdamOutValue*(1-iADAMRange/100.0))
                {
                    AlarmMsg.sprintf("AdamOutValue=%f, ReadAdamValue=%d, Range=%d", iAdamOutValue, PA, iADAMRange);    //Steven 20240520 : EP alarm log
                    MNetLog(AlarmMsg);
                    return true;
                }
            }
            else if(IniConfig.iD26_3FixValueOrPercentage==0)
            {
                if(PA>iAdamOutValue+iADAMRange ||
                   PA<iAdamOutValue-iADAMRange)
                {
                    AlarmMsg.sprintf("AdamOutValue=%f, ReadAdamValue=%d, Range=%d", iAdamOutValue, PA, iADAMRange);    //Steven 20240520 : EP alarm log
                    MNetLog(AlarmMsg);
                    return true;
                }
            }
        }
        else
#endif  // GATE (S25)
        {
            if(PA>iAdamOutValue+iADAMRange ||
               PA<iAdamOutValue-iADAMRange)                                     //JerryYang 20171030 (wei) fix ADAM alarm
            {
                AlarmMsg.sprintf("AdamOutValue=%f, ReadAdamValue=%d, Range=%d", iAdamOutValue, PA, iADAMRange);    //Steven 20240520 : EP alarm log
                MNetLog(AlarmMsg);
                return true;
            }
        }
        AlarmMsg="";
        return false;
    #endif
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:607-668 -- golden has NO caller (912 tree grep 20261002); edSetKg / edDoubleForce: banner
bool ADAM_Alarm_Kg(int iAdd)                                                    //JerryYang 20171024 (wei) add 單顆浮動頭誤差範圍,依照Mars定義給海思的資料
{
    double dValue=0.0;
    int PA =0;                                                                  //wei 20220309 Add EP Return Voltage
    double dKg=0.0;
    double dSetKg=0.0;
    AnsiString AlarmMsg_KG;

    if(iAdd==0)
    {
        PA = ADAM_ReadPA(&dValue);                                              //wei 20220309 Add EP Return Voltage
        dKg=KpaTransferKG(PA);
        dSetKg=fContactForm->edSetKg->Text.ToDouble();                          // [W906] D3 golden fContact->edSetKg
    iReadAdamEP=PA;                                                             //jou 20170413 (Steven) : Read Adam EP 提升UPH
    }
    else if(iAdd==1)
    {
        PA = ADAM_ReadPA(&dValue,2);                                            //Ifor 20221215 add:讀取Dual EP
        dKg=KpaTransferKG(PA, true);                                            //Ifor 20221215 add:轉換Dual EP 公斤數
        dSetKg=fContactForm->edDoubleForce->Text.ToDouble();                    // [W906] D3 golden fContact->edDoubleForce
    }

    if(Temperature.bATCActiveCooling)                                           //ATC layout kit
    {
        if(dSetKg>60)                                                           //單顆浮動頭61~120Kg
        {
            dADAMRange_Kg=2.0;
        }
        else                                                                    //單顆浮動頭8~60Kg
        {
            dADAMRange_Kg=1.0;
        }
    }
    else
    {
        if(dSetKg>60)                                                           //單顆浮動頭61~120Kg
        {
            dADAMRange_Kg=2.0;
        }
        else if(dSetKg>10)                                                      //單顆浮動頭11~60Kg
        {
            dADAMRange_Kg=1.0;
        }
        else if(dSetKg>5)                                                       //單顆浮動頭6~10Kg
        {
            dADAMRange_Kg=0.5;
        }
        else                                                                    //單顆浮動頭1~5Kg
        {
            dADAMRange_Kg=0.25;
        }
    }

    if((dKg>dSetKg+dADAMRange_Kg) ||
       (dKg<dSetKg-dADAMRange_Kg))
    {
        AlarmMsg_KG.sprintf("AdamOutValue=%f, ReadAdamValue=%f, Range=%f", dSetKg, dKg, dADAMRange_Kg);  //Steven 20240520 : EP alarm log
        MNetLog(AlarmMsg_KG);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// golden 912 adam6024.cpp:670-712 (Q3) -- golden callers HS_Function.cpp:960 / :1127 (not in the port); edDoubleForce: banner
bool ADAM_DualAlarm(int iType)                                                  //Ifor 20221228 add:Dual EP Check
{
    double dValue=0.0;
    double dDualTempKg=0.0, dOutValue;
    int PA=0, iDualTempPA=0;
    AnsiString AlarmMsg_Dual="";

    iDualTempPA=ADAM_ReadPA(&dValue,2);                                         //Ifor 20221215 add:讀取Dual EP
    dDualTempKg=KpaTransferKG(iDualTempPA, true);                               //Ifor 20221215 add:轉換Dual EP 公斤數

    if(iType==0)
    {
        PA=iDualTempPA;
        dOutValue=AdamOutputToPA(fContactForm->edDoubleForce->Text.ToDouble()); // [W906] D3 golden fContact->edDoubleForce
    }
    else if(iType==1)
    {
        PA=dDualTempKg;
        dOutValue=fContactForm->edDoubleForce->Text.ToDouble();                 // [W906] D3 golden fContact->edDoubleForce
    }

    if(IniConfig.iD26_3FixValueOrPercentage==1)                                 //Richard 20230428 : EP固定值或百分比
    {
        if(PA>dOutValue*(1+IniConfig.iD26_3DualEPEncoderRange/100.0) ||
           PA<dOutValue*(1-IniConfig.iD26_3DualEPEncoderRange/100.0))
        {
            AlarmMsg_Dual.sprintf("AdamOutValue=%f, ReadAdamValue=%f, Range=%d%%", dOutValue, PA, IniConfig.iD26_3DualEPEncoderRange);//Steven 20240520 : EP alarm log
            MNetLog(AlarmMsg_Dual);
            return true;
        }
    }
    else
    {
        if(PA>dOutValue+IniConfig.iD26_3DualEPEncoderRange ||
           PA<dOutValue-IniConfig.iD26_3DualEPEncoderRange)
        {
            AlarmMsg_Dual.sprintf("AdamOutValue=%f, ReadAdamValue=%f, Range=%d", dOutValue, PA, IniConfig.iD26_3DualEPEncoderRange);//Steven 20240520 : EP alarm log
            MNetLog(AlarmMsg_Dual);
            return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:2675-2759 (Q6, Q7) -- golden callers csystem.cpp:10999 (HOME, true) and main.cpp:22355 (Timer2)
void ADAM_ReturnValueCheck(bool bHome)                                          //wei 20220309 Add EP Voltage Error Alarm
{
    #ifndef SOFT_SIMULTE
    double dReadVoltage=0.0;
    int iReadPA=0;
    static int iCount=0;
    AnsiString Str;

    if(EP_Install==3 || EP_Install==5)                                          //JerryYang20220701:舊板本EP不支援
    {
        if(IniConfig.bD26EnableEncodeShow ||
           IniConfig.bD24EnableEPCheckFuntion ||
           IniConfig.bD26EnableEPEncoderRange ||
           W906_FormShowing("fContact", fContactForm->fShow) ||                // [W906] D3 golden fContact->fShow
           bHome)
        {
            static int iEPVoltageErrorCount[2]={0, 0};                          //yunghsin 20220303 Add EP Voltage Error Alarm  ==>
            for(int i=0; i<2; i++)
            {
                if(EP_Install==5)
                {
                    if(i==0)
                    {
                        iReadPA=ADAM_ReadPA(&dReadVoltage, 0);
                        if(W906_FormShowing("fContact", fContactForm->fShow))  // [W906] D3 golden fContact->fShow
                            fContactForm->lblReadEP->Caption=AnsiString("Read=")+AnsiString(GetFloatFormatString(iReadPA, 3, 2));  //20111217 ChungHung
                    }
                    else
                    {
                        iReadPA=ADAM_ReadPA(&dReadVoltage, 1);
                        if(W906_FormShowing("fContact", fContactForm->fShow))  // [W906] D3 golden fContact->fShow
                            fContactForm->lblReadEP2->Caption=AnsiString("Read=")+AnsiString(GetFloatFormatString(iReadPA, 3, 2));  //20111217 ChungHung
                    }
                }
                else
                {
                    if(i==0)
                    {
                        iReadPA=ADAM_ReadPA(&dReadVoltage);
                        if(W906_FormShowing("fContact", fContactForm->fShow))  // [W906] D3 golden fContact->fShow
                            fContactForm->lblReadEP->Caption=AnsiString("Read=")+AnsiString(GetFloatFormatString(iReadPA, 3, 2));  //20111217 ChungHung
                    }
                    else
                    {
                        if(INSTALL_DOUBLE_EP!=1)                                //Steven 20220324 : ==0 --> !=1
                            continue;

                        iReadPA=ADAM_ReadPA(&dReadVoltage, 2);                  //20111217 ChungHung
                        if(W906_FormShowing("fContact", fContactForm->fShow))  // [W906] D3 golden fContact->fShow
                            fContactForm->lblDieForceEP->Caption=AnsiString("Read=")+AnsiString(GetFloatFormatString(iReadPA, 3, 2));  //20111217 ChungHung
                    }
                }

                if((dReadVoltage<0.8 || dReadVoltage>5.2)                       //EP Controller Normal Voltage DC 1~5V
#if 0   // GATE (S25) AI(W906-ST02-ADAM) 20261002 (St02-E helper H2): CC_GIGAS only (D1) -- golden :2729 VERBATIM; without it a GIGAS machine gets the generic check
                   && CUSTOMER_CODE!=CC_GIGAS                                   //Jimmychiu 20241222 : 工程師Ben要求關閉EP數值異常檢測
#endif  // GATE (S25)
                  )
                {
                    iEPVoltageErrorCount[i]++;
                    if(iEPVoltageErrorCount[i]>100 || bHome)                    //100 sec
                    {
                        Str.sprintf("dReadVoltage=%f", dReadVoltage);
                        if(i==0)
                            ShowErrorMessage("WAR16322", K_RETRY, MMSystem, false, Str);
                        else
                            ShowErrorMessage("WAR16323", K_RETRY, MMSystem, false, Str);

                        if(bHome)
                            fAllMotorHome=false;
                        else
                            iEPVoltageErrorCount[i]=0;
                    }
                }
            }
        }
        else
        {
            iCount++;
            if(iCount>60)                                                       //Sam 20220311 : 60s 讀一次避免 Adam EP 睡著
            {
                iCount=0;
                iReadPA=ADAM_ReadPA(&dReadVoltage);
            }
        }
    }
    #else
    (void)bHome;                                                                // [W906] D5
    #endif
}

} // namespace w906_adampress

//===========================================================================
// The global symbols: golden 912 signatures (Adam6024_St02.h:84-86 / :107), forwarders ([W906] D4) behind H4's
// live switch ([W906] D6; W906_AdamEpLive is defined in Adam6024Integrate_St02.cpp:103 -- St02-E moves the
// declaration into Adam6024_St02.h, as Adam6024Integrate_St02.cpp:82 asks).
//===========================================================================
// bool W906_AdamEpLive(); -- declared in Adam6024_St02.h now (H4 integration pass 20261002)

bool ADAM_Alarm(int iArm)
{
    if(!W906_AdamEpLive())                                                      // [W906] D6 OFF: the stand-in's answer, atester_shims.cpp:328-329
        return false;
    return w906_adampress::ADAM_Alarm(iArm);
}
bool ADAM_Alarm_Kg(int iAdd)
{
    if(!W906_AdamEpLive())                                                      // [W906] D6 OFF: no ADAM read, no alarm
        return false;
    return w906_adampress::ADAM_Alarm_Kg(iAdd);
}
bool ADAM_DualAlarm(int iType)
{
    if(!W906_AdamEpLive())                                                      // [W906] D6 OFF: no ADAM read, no alarm
        return false;
    return w906_adampress::ADAM_DualAlarm(iType);
}
void ADAM_ReturnValueCheck(bool bHome)
{
    if(!W906_AdamEpLive())                                                      // [W906] D6 OFF: the retired seam's no-op, csystem.cpp:6369
        return;
    w906_adampress::ADAM_ReturnValueCheck(bHome);
}

// The seam setters (tests/test_adam6024_pressure.cpp).  0 restores golden's callee.
void W906_AdamPressSetReadPA(int (*fn)(double*, int))
{
    g_pAdamPressReadPA = fn ? fn : AdamPressDefaultReadPA;
}
void W906_AdamPressSetCheckConnect(bool (*fn)(int))
{
    g_pAdamPressCheckConnect = fn ? fn : AdamPressDefaultCheckConnect;
}
void W906_AdamPressSetNetLog(bool (*fn)(AnsiString))
{
    g_pAdamPressNetLog = fn ? fn : AdamPressDefaultNetLog;
}
