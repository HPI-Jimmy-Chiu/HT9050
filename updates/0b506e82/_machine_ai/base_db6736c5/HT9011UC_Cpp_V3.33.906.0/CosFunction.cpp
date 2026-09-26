// =============================================================================
//  CosFunction.cpp  --  customer-code feature-flag dispatch table
//
//  Faithful translation of golden CosFunction.cpp (4456 lines, BCB6, Big5/cp950).
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\CosFunction.cpp
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: PT-W3 ("cosfunction" group).
//
//  ROLE: this is the customer-code (CUSTOMER_CODE / CC_*) feature-flag
//  dispatch table.  DoCustomerFunction() (golden :3703-3876) switches on
//  CUSTOMER_CODE and calls exactly one FUNC_CC_<name>() per machine; each
//  FUNC_CC_<name>() sets a subset of the ~470 CosFunction.b*/i*/d* fields
//  (HT9045_COUSTOMER_FUNCTION, CosFunction.h) and IniConfig.* fields
//  (Config.h) to the values that customer's machine ships with.
//  InitialCosFunction() (golden :3880-end) runs FIRST and unconditionally
//  stamps every CosFunction/IniConfig field to its factory default, then
//  calls DoCustomerFunction() at the very end so the per-customer FUNC_CC_*
//  overrides land last and win.  VTEST_Funtion() / SPILFunction() /
//  KoreaFunction() / SIGURDFunction() / MaximFunction() / SingaporeFunction()
//  are shared "customer-family" bodies -- this file only DEFINES them; the
//  CALL SITES are in OTHER already-ported files (confirmed by whole-golden-
//  tree grep, not guessed): SingaporeFunction() is called from golden
//  cprod.cpp:3679 ("if(IniConfig.bSingaporeFunction) SingaporeFunction();").
//  The other five are declared extern in this unit's own CosFunction.h and
//  follow golden's own established pattern of one CC_* case in
//  DoCustomerFunction() delegating to a second, broader helper (e.g.
//  FUNC_CC_SPIL_SHINCHU sets IniConfig.bSPILFunction=true, and a SEPARATE
//  already-ported caller reads that flag and invokes SPILFunction() for the
//  whole SPIL family; ditto bKoreaFunction/KoreaFunction,
//  bVTESTFunction/VTEST_Funtion, and the Maxim/SIGURD equivalents) -- none of
//  those callers are in THIS file or THIS wave's scope.
//
//  WHY THIS UNIT MATTERS MORE THAN ITS LINE COUNT SUGGESTS: before this wave
//  CosFunction.cpp had NO compiled body anywhere in the port tree (task
//  brief's own framing: "CosFunction is 488 bytes of all-zero in the port
//  today because nothing defines it").  Every one of the ~470
//  HT9045_COUSTOMER_FUNCTION fields is a plain global struct member with a
//  zero-initialized default (BSS), so without this translation EVERY
//  CosFunction.b* flag silently read as false/0 regardless of CUSTOMER_CODE
//  -- a silent, config-layer-wide faithfulness gap across all ~165 customer
//  codes, not a crash.  This single file is why "the whole config layer
//  currently reads zero" per the task brief.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim, whole file, all 176 golden
//  function definitions -- golden line number cited after each name; the
//  body text below this banner is golden's OWN lines :12-4456 copied
//  unchanged except for the Big5->UTF-8 re-encode of comments):
//
//  FUNC_CC_HONPREC_QC:15  FUNC_CC_BROADCOM_US:85  FUNC_CC_AVAGO_Korea:89  FUNC_CC_TSMC_TAINAN:93
//  FUNC_CC_ChenYuanXiang_CHINA:122  FUNC_CC_HTKJXA_CHINA:127  FUNC_CC_STK:131  FUNC_CC_RIGGER_MICRO:135
//  FUNC_CC_SPIL_SHINCHU:141  FUNC_CC_SPIL_TAICHUNG_LOGIC:149  FUNC_CC_SPIL_CHINA_SUZHOU:154  FUNC_CC_SPIL_CS:159
//  FUNC_CC_XINYUN:164  FUNC_CC_SCX:171  FUNC_CC_HUAWEI:175  FUNC_CC_KYEC_CHEN:221
//  FUNC_CC_KYEC_LEE:265  FUNC_CC_KYEC_STM:354  FUNC_CC_KYEC_JCTHIU:433  FUNC_CC_DL_TEK:465
//  FUNC_CC_KYEC_XILINX:469  FUNC_CC_ASE_KaohSiung_K12:531  FUNC_CC_ASE_SG:567  FUNC_CC_ASE_JP:582
//  FUNC_CC_ASE_Korea:588  FUNC_CC_ASE_CL:600  FUNC_CC_ASE_SH:671  FUNC_CC_ASE_N:694
//  FUNC_CC_ASE_KaohSiung:707  FUNC_CC_ASE_M:808  FUNC_CC_ASE_KaohSiung_K3:868  FUNC_CC_ASE_KaohSiung_K11:874
//  FUNC_CC_UTAC:878  FUNC_CC_SIGURD_HUKOU:886  FUNC_CC_RFMD_BEIJING:922  FUNC_CC_JSCC_OS:926
//  FUNC_CC_CAPCON:1030  FUNC_CC_SCC:1035  FUNC_CC_SCS:1146  FUNC_CC_SIGURD_ChungXing:1180
//  FUNC_CC_SIGURD_PeiXing:1202  FUNC_CC_SCK:1257  FUNC_CC_RFMD_USA:1299  FUNC_CC_ASE_KS:1303
//  FUNC_CC_APTOS:1326  FUNC_CC_WINBOND:1330  FUNC_CC_G_Link:1334  FUNC_CC_AMKOR:1338
//  FUNC_CC_GIGA:1386  FUNC_CC_LINGSEN:1390  FUNC_CC_GONGJIN_SHANGHAI:1396  FUNC_CC_GONGJIN_SUZHOU:1401
//  FUNC_CC_Greatek:1406  FUNC_CC_PTI:1469  FUNC_CC_THAILIN:1510  FUNC_CC_SJ_Semiconductor:1514
//  FUNC_CC_SJ_Semiconductor_OS:1606  FUNC_CC_JCET:1634  FUNC_CC_JSI_HAOXING:1730  FUNC_CC_OSE:1737
//  FUNC_CC_MTI:1742  FUNC_CC_NUVOTON:1751  FUNC_CC_UPRTEK:1755  FUNC_CC_Eutrend:1759
//  FUNC_CC_TICP:1763  FUNC_CC_THEIL:1768  FUNC_CC_TERAPOWER:1773  FUNC_CC_TeraProbe:1821
//  FUNC_CC_GIGAS:1868  FUNC_CC_DoosanTesna:1914  FUNC_CC_AMKOR_Korea:1920  FUNC_CC_AMKOR_China:1976
//  FUNC_CC_AMKOR_Japan:2048  FUNC_CC_AMKOR_Philippines:2067  FUNC_CC_CARSEM_M:2158  FUNC_CC_UNISEM_M:2162
//  FUNC_CC_MAXIM_THAILAND:2175  FUNC_CC_Renesas:2183  FUNC_CC_MAXIM:2191  FUNC_CC_MARVELL:2199
//  FUNC_CC_ATMEL:2203  FUNC_CC_USI:2243  FUNC_CC_DYNACARD:2247  FUNC_CC_CYPRESS:2251
//  FUNC_CC_GERADTECH_CHINA:2268  FUNC_CC_GM_TEST:2272  FUNC_CC_I_TECH:2280  FUNC_CC_WIN_PAC:2285
//  FUNC_CC_VTEST:2291  FUNC_CC_VTEST_Shanghai:2296  VTEST_Funtion:2301  FUNC_CC_SILICON_LABS_SG:2377
//  FUNC_CC_SILICON_LABS_SZ:2387  FUNC_CC_Altera_USA:2391  FUNC_CC_WINSTEK:2403  FUNC_CC_HDSC:2407
//  FUNC_CC_SANECHIPS:2411  FUNC_CC_RF360:2415  FUNC_CC_CSAMQ:2484  FUNC_CC_ETRENDTECH:2491
//  FUNC_CC_QUALCOMM:2518  FUNC_CC_Microchip_Thailand:2595  FUNC_CC_Microchip_Philippines:2626  FUNC_CC_Microchip_China:2663
//  FUNC_CC_ANST:2667  KoreaFunction:2671  SingaporeFunction:2744  FUNC_CC_RICHTEK:2770
//  FUNC_CC_TSI:2789  FUNC_CC_ARDENTEC:2808  FUNC_CC_FULCAP:2812  FUNC_CC_AOSL:2816
//  FUNC_CC_Spreadtrum:2820  FUNC_CC_UMC:2824  FUNC_CC_ITestInc:2828  FUNC_CC_EMemory:2832
//  FUNC_CC_CYUEAN:2836  FUNC_CC_PANTHER:2863  FUNC_CC_ATEC:2870  FUNC_CC_SIGURD_SUZHOU:2882
//  FUNC_CC_VATE:2889  FUNC_CC_Murata:2894  FUNC_CC_ChipMos_ZHUBEI:2913  FUNC_CC_GT:2929
//  FUNC_CC_AMD_M:2942  FUNC_CC_HANA_MICRON:2970  SPILFunction:2989  MaximFunction:3131
//  FUNC_CC_TFME_CHINA:3172  FUNC_CC_CETC:3201  FUNC_CC_LEADYO:3209  FUNC_CC_NEXPERIA_Guangdong:3221
//  FUNC_CC_JSSI_Semiconductor:3229  FUNC_CC_Goertek:3247  FUNC_CC_BOJIAN:3251  FUNC_CC_ONSEMI_CA:3255
//  FUNC_CC_ONSEMI_M:3260  FUNC_CC_ITS:3266  FUNC_CC_DJI_SZ:3270  FUNC_CC_NXP_TJ:3274
//  FUNC_CC_SANAN:3278  FUNC_CC_HABANA:3292  FUNC_CC_XDXCT:3296  FUNC_CC_THINE:3301
//  FUNC_CC_CENTER:3305  FUNC_CC_SINOICTECH:3309  FUNC_CC_HYGEIA_SUZHOU:3320  FUNC_CC_UTAC_TW:3324
//  FUNC_CC_FOREHOPE_NINGBO:3361  FUNC_CC_FMSH:3388  FUNC_CC_ASIAOPTICAL:3395  FUNC_CC_HXYSEMI:3399
//  FUNC_CC_INTEL_IL:3403  FUNC_CC_IBM_CANADA:3407  FUNC_CC_ISE_US:3411  FUNC_CC_ISE_SH:3415
//  FUNC_CC_BARUN:3422  FUNC_CC_IFXTH_Thai:3427  FUNC_CC_Ramos:3433  FUNC_CC_Carsem_Thai:3438
//  FUNC_CC_GOOGLE_Israel:3443  FUNC_CC_Mellanox_Israel:3447  SIGURDFunction:3452  FUNC_CC_AnalogDevice_Phil:3488
//  FUNC_CC_Advantest_GE:3530  FUNC_CC_XINITECH:3535  FUNC_CC_ChipOn:3541  FUNC_CC_Allegro_Philippines:3553
//  FUNC_CC_UTAC_Indonesia:3615  FUNC_CC_Elmos_Germany:3619  FUNC_CC_Realtek:3681  FUNC_CC_HTML_Monitor:3689
//  FUNC_CC_HuaTian_NNAJING:3694  FUNC_CC_SFASemicon:3698  DoCustomerFunction:3703  InitialCosFunction:3880
//
//  Every one of the 176 functions above is copied FIELD-FOR-FIELD from
//  golden: identifiers, magic numbers (iTempeAlarmSecond_Over=1/2/4/5,
//  iFixedDropSpeed=30, iLimitMaxSpeed=100, dLimitMinDropOffset=0.0, ...),
//  #ifdef/#else ladders (HiSilicon / DEBUG_HT1032 / FOR_QLE / SOFT_SIMULTE --
//  all four are commented-out #define's in this tree's MachineType.h, so
//  every one of these ladders takes the SAME branch here that it takes on a
//  real release build of golden), local bool declarations
//  (bUseSecsGem/bUseNSKitKey), if/else branches on CUSTOMER_CODE /
//  ATC_SYSTEM / REAL_TIME_CCD / fSCKART / fAGV, commented-out ("//") dead
//  lines, and every //-comment (English and Chinese) -- nothing reordered,
//  nothing collapsed, nothing "improved".  A whole-file grep (PT-W3 wave
//  notes) found exactly 5 distinct real function/method calls in the entire
//  4456-line body outside the FUNC_CC_*/dispatch scaffolding itself:
//  CheckAndReadIniData (int/bool/AnsiString overloads, golden
//  :764/767/2017/2454/2557/3186), CheckAndReadIniDataGeneral (AnsiString
//  overload, golden :2260), CheckIniData (golden :751), TfSCKART::iTesterType
//  member reads (golden :2035/2054/3090/... -- a data member, not a call),
//  and TfAGV::IsATK_AMR() (golden :4450).  All five resolve to REAL,
//  already-compiled bodies elsewhere in this tree (see SATISFIED-BY-SUBSTRATE
//  below) -- so this file needs ZERO gates.
//
//  GATE REGISTER -- EMPTY.  No #if 0/#else pair anywhere in this file.  Every
//  external symbol the golden body touches already has a real, compiled
//  definition somewhere in the port tree (verified by grep over the WHOLE
//  port tree, excluding build*/, before writing a single line):
//    CheckAndReadIniData / CheckAndReadIniDataGeneral / CheckIniData / AuthPath
//      -- declared common.h:217-231/71, DEFINED common.cpp:102/283 (AuthPath)
//         and common.cpp:505/592/613/636/667/1404/1432/1448 (the 8 overloads).
//    fSCKART (TfSCKART*) / TfSCKART::iTesterType
//      -- declared forms/fSCKART.h:118/140, DEFINED forms/fSCKART.cpp.
//    fAGV (TfAGV*) / TfAGV::IsATK_AMR()
//      -- declared forms/fAGV.h:33/42, DEFINED forms/fAGV.cpp.
//    CUSTOMER_CODE, ATC_SYSTEM, REAL_TIME_CCD, AUTO3_IS_MAGAZINE,
//    SPIL_FOR_QLE -- plain extern globals, declared+defined in cmydef.h/
//      cmydef.cpp (already-ported substrate).
//    eATCUninstall, eulEnglish/eulChinese/eulKorea/eulSingapore, all 164
//    distinct CC_* customer-code constants used by this file -- #define /
//      enum constants in MachineType.h (already-ported substrate; a
//      textual diff confirmed all 164 CC_* tokens referenced by this file
//      exist in the ported MachineType.h -- zero misses).
//    IniConfig (HT9045_CONFIG) / CosFunction (HT9045_COUSTOMER_FUNCTION)
//      -- extern globals declared Config.h / CosFunction.h.  CosFunction.h
//      pre-exists in this tree BYTE-IDENTICAL to golden's own header (a
//      prior wave, W0-TAIL, already translated it) and per Hard Rule 7 is
//      NOT touched by this wave -- only CosFunction.cpp is new here.
//
//  SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed):
//    Config.h (IniConfig, 1501 golden lines / 1501 ported lines -- 1:1
//      mirror), cprod.h (3326/3326), cmydef.h (5934 golden / 5946 ported,
//      the extra lines are AI-authored forward-decl/typedef shims for
//      TMyLog/TMyStringList/uPoint2D noted in cmydef.h's own W0-TAIL banner),
//      MachineType.h (all CC_*/eATCType/eUserLanguage constants).
//
//  INCLUDE-BLOCK DEVIATION FROM GOLDEN, AND WHY (Hard Rule 3 exception,
//  reasoned exactly like aRotateKIT.cpp's "NOT re-included here" precedent):
//  golden's own block is
//    #include "MachineDefine.h"  ; #pragma hdrstop
//    #include "CosFunction.h" / "Config.h" / "cprod.h" / "cmydef.h" /
//      "SCK_ART.h" / "common.h" / "AGV.h"
//  "SCK_ART.h" and "AGV.h" resolve (via BCB6's project-wide search path) to
//  Automation/SCK_ART.h and Automation/AGV.h -- the ART STATE-MACHINE and AGV
//  E84 headers.  Nothing in this file's 176 bodies calls into either engine;
//  the only two symbols golden reaches are the FORM POINTERS fSCKART and
//  fAGV (TfSCKART*/TfAGV*, golden SCK_ART.h:249's owning form / AGV.h:210),
//  which in THIS tree live in forms/fSCKART.h and forms/fAGV.h, NOT in
//  Automation/SCK_ART.h or Automation/AGV.h (that split, and the resulting
//  form-global bridging idiom, is documented at Automation/SCK_ART.h:26-32
//  and FormsFacade.h's own file-head).  This wave's syntax-check include path
//  has no -IAutomation, so golden's literal "SCK_ART.h"/"AGV.h" would not
//  even resolve here, and would pull in the wrong (ART-engine) header even if
//  it did.  This file therefore does NOT forward golden's SCK_ART.h/AGV.h
//  includes and instead adds "FormsFacade.h" -- the tree's own established
//  bridging idiom for exactly this situation (see acatchtray.cpp:98,
//  "#include "FormsFacade.h"  // fMain / fAGV / fSortCT / fLotInfo / fSCKART
//  offline stand-ins").  This is an include-path substitution only; every
//  symbol golden's body actually uses (fSCKART->iTesterType,
//  fAGV->IsATK_AMR()) is reached identically either way.
//
//  VCL/Borland conversions: #pragma hdrstop dropped (golden :2).  No
//  __fastcall / __property / __published in this file's OWN code (the ini-
//  helper declarations it calls carry __fastcall in common.h, which this
//  toolchain treats as a real calling-convention keyword -- see
//  vclcompat/vcl_compat.h's own note -- so it is not neutralized, only kept
//  in lockstep between common.h's declarations and common.cpp's
//  definitions, neither of which this file touches).  AnsiString usage
//  (golden :1978/2253/2260/2417/2520/3174, 6 occurrences) is the tree's own
//  vclcompat AnsiString (vclcompat/AnsiString.h), not std::string; its
//  operator+ / operator== overloads used here (AnsiString+const char*,
//  AnsiString==const char*) are already provided there.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "CosFunction.h"            // this unit's own contract: HT9045_COUSTOMER_FUNCTION + CosFunction + the 6 extern function decls
#include "Config.h"                 // IniConfig (HT9045_CONFIG) -- the OTHER struct every FUNC_CC_* writes into
#include "cprod.h"                  // Prod/TestIF-family globals reached transitively (cprod.h itself pulls MachineType.h+Config.h+CosFunction.h)
#include "cmydef.h"                 // CUSTOMER_CODE, ATC_SYSTEM, REAL_TIME_CCD, AUTO3_IS_MAGAZINE, SPIL_FOR_QLE, all CC_* constants (via MachineType.h)
#include "common.h"                 // AuthPath, CheckAndReadIniData*(3 overloads used), CheckAndReadIniDataGeneral, CheckIniData
#include "FormsFacade.h"            // fSCKART (TfSCKART*, ::iTesterType) + fAGV (TfAGV*, ::IsATK_AMR()) -- see INCLUDE-BLOCK DEVIATION note above; golden's own "SCK_ART.h"/"AGV.h" are NOT forwarded
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  客戶功能選擇區
//------------------------------------------------------------------------------
void FUNC_CC_HONPREC_QC()
{
//    IniConfig.bIndexJamInArmAway                                              =true;  //Index掉料時,In Arm要先移開才Alarm
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bHaveRotateShuttle                                                =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
    IniConfig.bQAMode                                                           =true;
    IniConfig.bCanByPassIonFan                                                  =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bStopMustTestTimeOut                                            =true;  //Steven 20111003 : 暫停時也要計算Tester Time Out
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bResetCanServoOff                                                 =true;  //Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off
//    CosFunction.bEnable_SECS_GEM                                              =true;  //jou 2012-03-12 Enable SECS_GEM
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    CosFunction.bAutoKTemp                                                      =true;  //Steven 20120719 : 自動K溫
    IniConfig.bEnableKT4HAlarm1                                                 =true;  //Steven 20120809 : KT4H使用Alarm1作加熱保護
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    CosFunction.bEnable2x1Site                                                  =true;  //Steven 20120814: 支援2x1Site模式
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNewResetFunction                                                 =true;  //Steven 20130625 : 新的Reset方式
//    IniConfig.bUseTrayBlockMode                                               =true;  //2014-03-04    Dell    for SPIL WLP Add Tray Block //打開需驗證Fix分Tray模式
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bInOutArmCanPushHome                                              =true;
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150128 : KK說要開啟使用
//    IniConfig.bASE_Report                                                     =true;
    CosFunction.bUseGPIB_V108Command                                            =true;  //Steven 20150901 : For GPIB V1.08 Command
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bTrayOCR                                                        =true;
    CosFunction.bESDAutoDecayTeachFunction                                      =true;  //Ifor 20151222 :新增 ESD Auto Decay Teach Function
    CosFunction.bUseTrayUpDownSet                                               =true;  //wei 20160224 TSMC FIX UPDOWN  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20160303 : FIFO Mode
    CosFunction.bUseMonitorView                                                 =true;  //Ifor 20160930 add USE MonitorView 錄影監視功能顯示
    CosFunction.bUseFix3FullTray                                                =true;  //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
    CosFunction.bTTLCanUse8Site                                                 =true;  //Alick 20161011 (Steven) : TTL支援8Site
//    CosFunction.bAutoCleanUse2RowBy46LS                                       =true;  //Alick 20170223 (wei) add 9046LS可開啟雙排吸嘴做AutoClean
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 (wei) : For SCK 93K ART
    if(fSCKART!=NULL && fSCKART->iTesterType==1)                                        //Steven 20161201 (wei) : For SCK 93K ART
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;

    CosFunction.bInShuttleDetectByLatch                                         =true;  //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    CosFunction.bRecipeParameterDefault                                         =true;  //Isaac 20170527 (Steven) defalut值比較功能
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =true;  //Ifor 20160829 : Loader Try 最後一盤入料Alarm不停機
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bColorSensorUseCCLink                                           =true;  //wei 20180409 color sensor
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bNonCenterModeCanUseShtOffset                                   =true;
    CosFunction.b1x4OnlyAaUse1x1Mode                                            =true;
    CosFunction.bEnableMulti2D                                                  =true;  //Steven 20200810 : 一個IC使用多個2DID
    CosFunction.bIOTriggerIonFanAutoClean                                       =true;  //Isaac 20210609 : IO觸發IonFan清針
    CosFunction.bIndexCheckCanTurnOff                                           =true;  //Isaac 20211019 : 可選擇做index check的時機
    IniConfig.bAutoTrayLink                                                     =true;
    CosFunction.bTrayDeviceCheckFromLoader                                      =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bTrayMapFromLoader                                              =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    #ifdef DEBUG_HT1032
        CosFunction.bATCUseHandlerRecipeName                                    =true;  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    #endif
    CosFunction.bNeedAlarmAfterUnloaderFull                                     =true;  //Jimmychiu 20240902 : Need Alarm After Unloader Full
    CosFunction.bCheckTempDuringIndexArmTesting                                 =true;  //Jimmychiu 20240916 : Check the temperature during index arm testing
    CosFunction.bReadClipCodeFromUnloader                                       =true;  //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
}
//------------------------------------------------------------------------------
void FUNC_CC_BROADCOM_US()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_AVAGO_Korea()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_TSMC_TAINAN()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bHeadChamberSocketMode                                            =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
    IniConfig.bShowFunctionWindow                                               =true;  //2014-04-15    Dell
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bUseLoadCellOffsetByHeater                                      =true;  //2014-06-26    Dell    高溫Load cell offset
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    //IniConfig.bShowLotInfo                                                    =true;  //ChungHung 20150408 add for TSMC add    //jou 2013-01-18 Show Lot Info
    CosFunction.bBuilderImportSingleFolder                                      =true;  //ChungHung 20150414 add for TSMC
    CosFunction.bEnableSoftWareControlButton                                    =true;  //ChungHung 20150609 add only for TSMC
    CosFunction.bManualProcess                                                  =true;  //ChungHung 20150609 add for TSMC //ChungHung 20150526 add for QualComm US
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
    CosFunction.bManualSortMode                                                 =true;  //Steven 20150915 : For TSMC 手動整盤功能
    CosFunction.bTrayOCR                                                        =true;  //wei 20150926 TSMC Tray Arm OCR
    IniConfig.bRTCbySystem                                                      =true;  //ChungHung 20120716 RTC by System
    CosFunction.bUseTrayUpDownSet                                               =true;  //wei 20160224 TSMC FIX UPDOWN  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bLastSetInSetUpFile                                             =false; //wei 20160621 FTPDOWNLOAD錯誤   先關閉此功能
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    CosFunction.bGPIBLotEnd                                                     =true;
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
}
//------------------------------------------------------------------------------
void FUNC_CC_ChenYuanXiang_CHINA()
{
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
}
//------------------------------------------------------------------------------
void FUNC_CC_HTKJXA_CHINA()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_STK()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_RIGGER_MICRO()
{
    CosFunction.bUseLotIDWithoutSECS                                            =true;
    IniConfig.bOneCycleCanTrayFeed                                              =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SPIL_SHINCHU()
{
    IniConfig.bSPILFunction                                                     =true;  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    #ifdef FOR_QLE
        SPIL_FOR_QLE                                                            =1;     //Steven 20230110 : For渠梁
    #endif
}
//------------------------------------------------------------------------------
void FUNC_CC_SPIL_TAICHUNG_LOGIC()
{
    IniConfig.bSPILFunction                                                     =true;  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
}
//------------------------------------------------------------------------------
void FUNC_CC_SPIL_CHINA_SUZHOU()
{
    IniConfig.bSPILFunction                                                     =true;  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
}
//------------------------------------------------------------------------------
void FUNC_CC_SPIL_CS()
{
    IniConfig.bSPILFunction                                                     =true;  //KevinCheng 20251002  矽品中山-客戶碼統一用SPILFunction
}
//------------------------------------------------------------------------------
void FUNC_CC_XINYUN()                                                                   //杭州芯云
{
    IniConfig.bSPILFunction                                                     =true;
    IniConfig.bShowFormByInitPos                                                =false;
}

//------------------------------------------------------------------------------
void FUNC_CC_SCX()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_HUAWEI()
{
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    IniConfig.bOpenDoorNotStopFan                                               =true;
    IniConfig.bUseFix3                                                          =true;  //Steven 20140115 : LS有FIX3
    IniConfig.bSiteMappingFastSetDisable                                        =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    CosFunction.bOneCycleCanChangeArm                                           =true;  //Hung 20111108 Amb/Hot Mode OneCycle Can Change Arm
    IniConfig.iTempeAlarmSecond_Over                                            =2;     //Steven 20111027 : 溫度過高的Alarm時間
    IniConfig.bShuttleMode50                                                    =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bEventLogAutoSaveFunction                                         =true;
//    IniConfig.bShowFormByInitPos                                              =true;  //jou 2013-12-02
    IniConfig.bFTPJamCodeUpload                                                 =true;  //ChungHung 20140108 add FTP unload jam code
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    CosFunction.bHaveIndexContactDelay                                          =true;  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //ChungHung 20140715 SPIL 又想改回 可以 Retry and Skip 所以直接開放可以選
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bArm2ForFingerPrintTest                                         =true;  //Steven 20140922 : Arm2當作指紋測試
    CosFunction.bUseIndividulTempSet                                            =true;  //Steven 20140924 : 各個加熱區獨立有自己的設定值
    CosFunction.bRCMDStart                                                      =true;  //Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bStopMustTestTimeOut                                            =true;
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //Steven 20141105 : Index內的所有異常都只能用Skip
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150128 : KK說要開啟使用
    CosFunction.bRTCFullViewErrorOnlyRetry                                      =false; //Steven 20150304 : bRTCFullViewErrorOnlyRetry預設改為True
    CosFunction.bEnableOctal_12Kit                                              =true;  //ChungHung 20140508 add 使用12 layout kit Octal Shutle kit
    CosFunction.b12SiteCloseSiteRun2x4                                          =true;  //Steven 20150721 : 12Site關Site跑2x4 Mode
    CosFunction.bAutoCleanAutoSelIndexArm                                       =true;  //jou 2015-08-12 Auto Clean Auto Select Index Arm
}
//------------------------------------------------------------------------------
void FUNC_CC_KYEC_CHEN()                                                                //京元 記憶體.銅鑼廠
{                                                                                       //以前記憶體部門，目前沒有再用了，改成銅鑼廠使用
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;
    //IniConfig.bIndexDropOnlyReset                                             =true;  //ChungHung 20150618 delete   //jou 2013-12-02 Index Drop Only Reset
//    IniConfig.bIndexJamInArmAway                                              =true;  //Index掉料時,In Arm要先移開才Alarm
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式  //20140327 wei 新增
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    //CosFunction.bAfterRTChangeToInitialStart                                  =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
//    IniConfig.bUseTrayBlockMode                                               =true;  //2014-03-04    Dell    for SPIL WLP Add Tray Block //打開需驗證Fix分Tray模式
    IniConfig.bShuttleModeAccseeLevel                                           =true;
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    CosFunction.bEPUseNSSLK                                                   =true;  //wei 20150303   京元NS浮動頭  //Ifor 20200424 Mark
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣   //wei 20150420 Open
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //jou 2013-09-25 Testing Need Stop All Motor
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bLastSetInSetUpFile                                             =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bCheckInOutArmZHomeSensor                                       =true;  //Frank 20160612 add Check In/Out Arm Home Sensor Function
//    CosFunction.bRotateDetectErrNeedShake                                     =true;  //JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm
    CosFunction.bUsePEModelFunction                                             =true;  //Ifor 20160822 Use PE Model Function (PE 工程模式)
    CosFunction.bUseFixTryCheckRemainingAmount                                  =true;  //Ifor 20160829 add Use Fix Try Check Remaining Amount
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =true;  //Ifor 20160829 add Use Loader Try Feed Continue Run
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20160909 避免LotInf 被關閉
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;  //Steven 20180627 (wei) : Time out預設只顯示SKIP
    CosFunction.bFTPDownloadAlwaysCover                                         =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bUseATCFileTransfer                                             =true;  //Eastsun 20260522 整合: ATC Recipe 傳送與接收
}
//------------------------------------------------------------------------------
void FUNC_CC_KYEC_LEE()
{
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;
    IniConfig.bIndexDropOnlyReset                                               =true;  //jou 2013-12-02 Index Drop Only Reset
//    IniConfig.bIndexJamInArmAway                                              =true;  //Index掉料時,In Arm要先移開才Alarm
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式  //20140327 wei 新增
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    IniConfig.bUseTrayBlockMode                                                 =true;
    IniConfig.bShuttleModeAccseeLevel                                           =true;
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    CosFunction.bEPUseNSSLK                                                   =true;  //wei 20150303   京元NS浮動頭  //Ifor 20200424 Mark
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣   //wei 20150420 Open
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //jou 2013-09-25 Testing Need Stop All Motor
    //CosFunction.bRCMDStart                                                    =true;  //ChungHung 20150511 modify
    CosFunction.bDisableRTBinSet                                                =true;  //wei 20150622 不顯示設定RT Bin set
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bRCMDStart                                                      =true;  //Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.bSECS_GEM_OneCycle                                              =true;  //wei 20150824 Secs_Gem 斷線Onecycle
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bLowYeildByTotal                                                =true;  //wei 20151116 Low Yeild By Total
    IniConfig.iTempeAlarmSecond_Over                                            =5;     //Steven 20111027 : 溫度過高的Alarm時間
//    IniConfig.bJAM0301NeedOpenChamberDoor                                     =true;  //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20160304 //Ifor 20160304 :Log Up to FTP Function
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bContactTestWaitSoakTime                                        =true;  //wei 20160329 Contact Test Wait SoakTime
//    CosFunction.bSecurityHave5Level                                           =true;  //jou 2014-06-19 Security Have 5 Level
//    CosFunction.bLastSetInSetUpFile                                           =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File //Ifor 20161220 add
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bUsePEModelFunction                                             =true;  //Ifor 20160822 Use PE Model Function (PE 工程模式)
    CosFunction.bUseFixTryCheckRemainingAmount                                  =true;  //Ifor 20160829 add Use Fix Try Check Remaining Amount
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =true;  //Ifor 20160829 add Use Loader Try Feed Continue Run
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //jou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bTrayOCR                                                        =true;  //wei 20150926 TSMC Tray Arm OCR
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20160909 避免LotInf 被關閉
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =true;
//    CosFunction.bShowLastContactHigh                                          =true;  //Steven 20140409 : 顯示之前的Contact高度 //Ifor 20170418 KYEC 喬治說不要顯示Org height
    CosFunction.bUseARTSortCount                                                =true;  //Ifor 20170315 add 新增使用ART Sort Count 計數功能   //20170417 功能延至下ㄧ版本
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bUseMRTMode                                                     =true;  //Ifor 20170316 (wei) add KYEC MRT Mode
    CosFunction.bCanChangeAutoCleanCount                                        =true;  //Ifor 20171024 (Steven) : add 可更改Auto Clean 計數
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20171123 (Steven) : add FTP DownLoad Site By SetupFile
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion //Frank 20171121 add
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //KaiChen 20180123 (Steven) ：Yield Alarm 補上清空計數
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //Ifor 20180322 : add Use One By One Index Check
    CosFunction.bYieldControlUseEACount                                         =true;  //wei 20180606 Yield控制使用EA Count
    CosFunction.bHaveFIFOMode                                                   =true;  //Ifor 20181001: Add KYEC 使用FIFO功能
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關

//   if(bEnable_KLT_Function==true)                                                     //Ifor 20180802 : add KLT bBarCodeRules -> bEnable_KLT_Function 0:KYEC 1: KLT
//    {                                                                                 //Ifor 20180802 : add 京隆要求開啟Auto SKIP 功能
        IniConfig.bRecordSkipPosition                                           =true;  //JerryYang 20190710 KYEC開放Auto Speed
//    }
    CosFunction.bFTPDownloadAlwaysCover                                         =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bUseHeadContactCount                                            =true;  //Ifor 20160516 京元要求銦片 Life Time 功能
    CosFunction.bUseChangeLogByLot                                              =true;  //Ifor 20191002 : add Change Log By Lot
    CosFunction.bUseN07_5                                                       =true;  //Steven 20200309 : [N07-5]改成by客戶開啟
    CosFunction.bUseHandlerSetChillerTemp                                       =false; //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bKeepOnly1SetupFile                                             =true;  //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bUseTrayThickAdjustZHeight                                      =true;  //Ifor 20221214 add: 使用Tray 厚度自動校正吸嘴高度
    CosFunction.bATCUseHandlerRecipeName                                        =true;
    CosFunction.bUploadTestArmTorqueLog                                         =true;  //Steven 20250609 : 統一整理
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bCylinderOnOffTimeLog                                           =true;  //Eastsun 20260522 整合: Pre Alrm Cylinder
    CosFunction.bUseATCFileTransfer                                             =true;  //Eastsun 20260522 整合: ATC Recipe 傳送與接收

    CosFunction.b2DUsePinInspection                                             =true;  //Eastsun 20260526 整合://Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
    CosFunction.bUseBarcodeAutoAdjustLight                                      =true;  //Eastsun 20260527 整合
}
//------------------------------------------------------------------------------
void FUNC_CC_KYEC_STM()
{
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;
    IniConfig.bIndexDropOnlyReset                                               =true;  //jou 2013-12-02 Index Drop Only Reset
//    IniConfig.bIndexJamInArmAway                                              =true;  //Index掉料時,In Arm要先移開才Alarm
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式  //20140327 wei 新增
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    IniConfig.bUseTrayBlockMode                                                 =true;
    IniConfig.bShuttleModeAccseeLevel                                           =true;
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    CosFunction.bEPUseNSSLK                                                   =true;  //wei 20150303   京元NS浮動頭  //Ifor 20200424 Mark
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣   //wei 20150420 Open
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //jou 2013-09-25 Testing Need Stop All Motor
    //CosFunction.bRCMDStart                                                    =true;  //ChungHung 20150511 modify
    CosFunction.bDisableRTBinSet                                                =true;  //wei 20150622 不顯示設定RT Bin set
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bRCMDStart                                                      =true;  //Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.bSECS_GEM_OneCycle                                              =true;  //wei 20150824 Secs_Gem 斷線Onecycle
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bLowYeildByTotal                                                =true;  //wei 20151116 Low Yeild By Total
    IniConfig.iTempeAlarmSecond_Over                                            =5;     //Steven 20111027 : 溫度過高的Alarm時間
//    IniConfig.bJAM0301NeedOpenChamberDoor                                     =true;  //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20160304 //Ifor 20160304 :Log Up to FTP Function
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bContactTestWaitSoakTime                                        =true;  //wei 20160329 Contact Test Wait SoakTime
//    CosFunction.bSecurityHave5Level                                           =true;  //jou 2014-06-19 Security Have 5 Level
//    CosFunction.bLastSetInSetUpFile                                           =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File //Ifor 20161220 add
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bUsePEModelFunction                                             =true;  //Ifor 20160822 Use PE Model Function (PE 工程模式)
    CosFunction.bUseFixTryCheckRemainingAmount                                  =true;  //Ifor 20160829 add Use Fix Try Check Remaining Amount
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =true;  //Ifor 20160829 add Use Loader Try Feed Continue Run
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //jou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bTrayOCR                                                        =true;  //wei 20150926 TSMC Tray Arm OCR
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20160909 避免LotInf 被關閉
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =true;
//    CosFunction.bShowLastContactHigh                                          =true;  //Steven 20140409 : 顯示之前的Contact高度 //Ifor 20170418 KYEC 喬治說不要顯示Org height
    CosFunction.bUseARTSortCount                                                =true;  //Ifor 20170315 add 新增使用ART Sort Count 計數功能   //20170417 功能延至下ㄧ版本
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bUseMRTMode                                                     =true;  //Ifor 20170316 (wei) add KYEC MRT Mode
    CosFunction.bCanChangeAutoCleanCount                                        =true;  //Ifor 20171024 (Steven) : add 可更改Auto Clean 計數
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20171123 (Steven) : add FTP DownLoad Site By SetupFile
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion //Frank 20171121 add
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //KaiChen 20180123 (Steven) ：Yield Alarm 補上清空計數
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //Ifor 20180322 : add Use One By One Index Check
    CosFunction.bYieldControlUseEACount                                         =true;  //wei 20180606 Yield控制使用EA Count
    CosFunction.bHaveFIFOMode                                                   =true;  //Ifor 20181001: Add KYEC 使用FIFO功能
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關

//   if(bEnable_KLT_Function==true)                                                     //Ifor 20180802 : add KLT bBarCodeRules -> bEnable_KLT_Function 0:KYEC 1: KLT
//    {                                                                                 //Ifor 20180802 : add 京隆要求開啟Auto SKIP 功能
        IniConfig.bRecordSkipPosition                                           =true;  //JerryYang 20190710 KYEC開放Auto Speed
//    }
//    CosFunction.bFTPDownloadAlwaysCover                                       =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bUseHeadContactCount                                            =true;  //Ifor 20160516 京元要求銦片 Life Time 功能
    CosFunction.bUseChangeLogByLot                                              =true;  //Ifor 20191002 : add Change Log By Lot
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bUseATCFileTransfer                                             =true;  //Eastsun 20260522 整合: ATC Recipe 傳送與接收
}
//------------------------------------------------------------------------------
void FUNC_CC_KYEC_JCTHIU()
{
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;
    IniConfig.bIndexDropOnlyReset                                               =true;  //jou 2013-12-02 Index Drop Only Reset
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
//    IniConfig.bUseTrayBlockMode                                               =true;  //2014-03-04    Dell    for SPIL WLP Add Tray Block //打開需驗證Fix分Tray模式
    IniConfig.bShuttleModeAccseeLevel                                           =true;
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bLastSetInSetUpFile                                             =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bUsePEModelFunction                                             =true;  //Ifor 20160822 Use PE Model Function (PE 工程模式)
    CosFunction.bUseFixTryCheckRemainingAmount                                  =true;  //Ifor 20160829 add Use Fix Try Check Remaining Amount
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =true;  //Ifor 20160829 add Use Loader Try Feed Continue Run
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20160909 避免LotInf 被關閉
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20170214 (wei) add Auto Clean Function
    CosFunction.bYieldControlUseEACount                                         =true;  //wei 20180606 Yield控制使用EA Count
    CosFunction.bFTPDownloadAlwaysCover                                         =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bUseATCFileTransfer                                             =true;  //Eastsun 20260522 整合: ATC Recipe 傳送與接收
}
//------------------------------------------------------------------------------
void FUNC_CC_DL_TEK()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_KYEC_XILINX()
{
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;
    IniConfig.bIndexDropOnlyReset                                               =true;  //jou 2013-12-02 Index Drop Only Reset
//    IniConfig.bIndexJamInArmAway                                              =true;  //Index掉料時,In Arm要先移開才Alarm
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式  //20140327 wei 新增
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
//    IniConfig.bUseTrayBlockMode                                               =true;  //2014-03-04    Dell    for SPIL WLP Add Tray Block //打開需驗證Fix分Tray模式
    IniConfig.bShuttleModeAccseeLevel                                           =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bTemperatureMax130                                              =true;  //Steven 20140911 : Xilinx最高只要130度;
    CosFunction.bConAlarmNeedKeyInPassword                                      =true;  //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password    //wei 20150420 Open
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bForceSetAuto3Fix3SideHasNullIC                                 =true;  //JerryYang 20151120 add for 京元Xilinx, IC 大於 40x40mm時會敲到,Auto3和Fix3最右邊要設為 HasNullIC
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bLowYeildByTotal                                                =true;  //wei 20151116 Low Yield By Total
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.bAutoRetestGPIBmode                                             =true;  //jou 2015-10-02 Auto Retest GPIB mode
    CosFunction.bDisableRTBinSet                                                =true;  //wei 20150622 不顯示設定RT Bin set
    CosFunction.bLastSetInSetUpFile                                             =false; //JerryYang 20160513 京元先關閉此功能
    IniConfig.bIndexPickupWait                                                  =true;
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20160909 避免LotInf 被關閉
//    CosFunction.bAutoCleanUse2RowBy46LS                                       =true;  //Alick 20170223 (wei) add 9046LS可開啟雙排吸嘴做AutoClean
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20171123 : add FTP DownLoad Site By SetupFile
    if(DeviceForm_File.bEnableUseUniversalShuttle==true)                                //Frank 20170720 (Steven) Xilinx 需要有開關
    {
        IniConfig.bRemeberAutoHeight                                            =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
        IniConfig.bChangeKitNoHardStop                                          =true;  //jou 2015-12-08 Xilinx 驗證用
    }
    else
    {
        IniConfig.bRemeberAutoHeight                                            =false; //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
        IniConfig.bChangeKitNoHardStop                                          =false; //jou 2015-12-08 Xilinx 驗證用
    }
    CosFunction.bUniversalKit                                                   =true;  //JerryYang 20171102 (wei) UniversalKit改成客戶功能
//    IniConfig.bRemeberAutoHeight                                              =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
//    IniConfig.bChangeKitNoHardStop                                            =true;  //jou 2015-12-08 Xilinx 驗證用
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 : Use FTP Download Data Check
    CosFunction.bYieldControlUseEACount                                         =true;  //wei 20180606 Yield控制使用EA Count
    CosFunction.bFTPDownloadAlwaysCover                                         =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bKeepOnly1SetupFile                                             =true;  //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bCylinderOnOffTimeLog                                           =true;  //Eastsun 20260522 整合: Pre Alrm Cylinder
    CosFunction.bUseATCFileTransfer                                             =true;  //Eastsun 20260522 整合: ATC Recipe 傳送與接收
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_KaohSiung_K12()
{
    IniConfig.iTempeAlarmSecond_Over                                            =1;     //Steven 20111027 : 溫度過高的Alarm時間
    if(USE_ROTATE_KIT)                                                                  //kevin rotate motor
        IniConfig.bHaveRotateShuttle                                            =false;
    else
        IniConfig.bHaveRotateShuttle                                            =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    if(REAL_TIME_CCD==false)
        IniConfig.bEnableCCDUSETCPIP                                            =true;
    IniConfig.bAlarmNeedServoOff                                                =false;
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;  //kevin 20120217 Autoclean
//    IniConfig.bL07UseSingleTenmpertureLimit                                   =false;
//    IniConfig.bCheckFixTray                                                   =true;  //kevin 20130705 K15說CC強致使用
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
    CosFunction.bOnlyUseContactModeDiffSpeed                                    =true;  //Steven 20131101 : 強制使用兩段速Contact
    CosFunction.bFixedDropSpeed                                                 =true;  //Steven 20131101 : 使用固定的Drop速度
    CosFunction.iFixedDropSpeed                                                 =20;    //Steven 20131101 : 使用固定的Drop速度
    CosFunction.bLimitMaxSpeed                                                  =true;  //Steven 20131101 : 限制最高速度
    CosFunction.iLimitMaxSpeed                                                  =70;    //Steven 20131101 : 限制最高速度
    CosFunction.dLimitMinDropOffset                                             =5.0;   //kevin 20131115  Tom      //Steven 20131101 : 限制最小距離
    CosFunction.bPiggyBackForASE                                                =true;  //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    CosFunction.bPiggyBackShowMainForm                                          =true;  //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
//    CosFunction.bAutoCleanOnHotPlate                                          =true;  //Steven 20131101 : 將Clean Pad放在HotPlate上,也就是高雄版
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關
    CosFunction.bCanUseSearch2DIDByLot                                          =true;  //Steven 20190508

    CosFunction.bBarcodeTrayRecFile                                             =true;  //jou 20190930 : Barcode Tray record file
    CosFunction.bBarcodeErrNoTestAndShowH                                       =true;  //jou 20191007 : Barcode Error No Test & Show "H"
    CosFunction.bBarcodeDuplicateFileByOutArm                                   =true;  //jou 20191008 : Barcode duplicate file by out arm
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //jou 20191008 : (Steven) add SCC使用Lot ID
    CosFunction.bTrayOCR                                                        =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_SG()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20200103 : 開啟Auto Clean功能
    IniConfig.bSingaporeFunction                                                =true;  //Steven 20120910 : 新加坡代理商的需求
    CosFunction.bUseFFCTempOffset                                               =true;  //Ifor 20190730 : add FFC Temperature Offset
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ifor 20190215 : add ATC 使用 三點校正功能
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;  //Ifor 20200107 : add FTP DownLoad Temp Mode By SetupFile
    CosFunction.bUseHandlerSetChillerTemp                                       =true;  //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bATCUseHandlerRecipeName                                        =true;  //Ifor 20191126 : add ATC Use Handler Recipe Name
    CosFunction.b2DUseAnyCharFunction                                           =true;  //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    CosFunction.bAutoCleanUseHPSetByRecipe                                      =true;  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    CosFunction.bATCModeNoUseCheckHeaterFunction                                =true;  //Ifor 20260113 add:ATC Mode No Use CheckHeater Function
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_JP()
{
    CosFunction.bTTLCanUse8Site                                                 =true;  //Frank 20220408 Add TTL ASE_JP Mode
    CosFunction.bGPIBLotEnd                                                     =true;  //kevin 20190613 addSam 20181030 : 透過 GPIB 送 LotEnd 給測試機。
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_Korea()
{
    IniConfig.bKoreaFunction                                                    =true;
    IniConfig.bTrayAssignUseGraphic                                             =true;  //Steven 20111121 : 使用圖片去顯示Tray Assign
    IniConfig.bOnlyRoomOrHot                                                    =true;
    IniConfig.bSocketCommunication                                              =true;  //ChungHung 20130112 add for ASE_KR Socket Tester
    IniConfig.bEnableAutoCleanFunction                                          =true;  //ChungHung 20140513 add for ASE_Korea FIX3
    CosFunction.bUseTrayUpDownSet                                               =false; //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    IniConfig.bHeadSocketMode                                                   =true;
    IniConfig.bHeadChamberSocketMode                                            =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_CL()
{
    IniConfig.bInOutArmCanPushHome                                              =true;
    IniConfig.bHaveRotateShuttle                                                =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.b1x2Use4Suck                                                      =true;  //Hung 20110812 : 1x2使用4吸嘴
    IniConfig.b1x4Use8Suck                                                      =true;  //Hung 20110812 : 1x4使用8吸嘴
    IniConfig.b2x2Use8Suck                                                      =true;  //Hung 20110812 : 2x2使用8吸嘴
//    IniConfig.bHotPlateMove1CM                                                  =true;  //Hung 20110812 : HotPlate使用1CM轉板
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bRotateShNeedSensorCheck                                          =true;  //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bDualSiteSupply4CH                                                =true;  //jou 2012-11-20 Dual Site supply 4's Channel
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    CosFunction.bLockF06ByFile                                                  =true;  //Steven 20140627 : Add for ASE-CL
    CosFunction.bLockD41ByFile                                                  =true;  //Steven 20140627 : Add for ASE-CL
//    CosFunction.bOLPFunction                                                    =true;  //Steven 20141229 : OLP功能
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bEnableCCDUSETCPIP                                                =true;  //Ifor 20150720 : 開啟CCD功能
    CosFunction.bEnable_SECS_GEM                                                =true;  //JerryYang 20170306 (wei) Enable SECS_GEM
    CosFunction.bRecipeParameterDefault                                         =true;  //Isaac 20170527 (Steven) defalut值比較功能
    CosFunction.bPiggybackFunctionByHandler                                     =true;  //Isaac 20170712 (wei) :Piggyback function By Handlder(save file to config.ini)
    IniConfig.bShowFunctionWindow                                               =true;  //Isaac 20170720 (wei) 客戶要求顯示piggyback是否開啟
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;  //Steven 20180627 : Time out預設只顯示SKIP   //Steven 20160222 : KH Chiou發信說只要SKIP
    CosFunction.bIncludeMTBA                                                    =true;  //JerryYang 20180619 : 新增可自定義Jam code是否列入MTBA計算
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient  //Isaac 20171215 : ASE_CL要求開啟"Low Yield Auto Site Off for Ambient"功能
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm                  //Isaac 20171215 : ASE_CL要求開啟"Low Yield Auto Site Off for Ambient"功能
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //JerryYang 20180726 尚智要求 TEST IF 相關異常只能skip
    IniConfig.bFTBin2RTBin                                                      =true;  //JerryYang 20190109 尚智要求開放此功能
    IniConfig.bShowLotInfo                                                      =true;  //JerryYang 20190704 add
    CosFunction.bSECS_GEM_OneCycle                                              =true;
    CosFunction.bBarcodeErrNoTestAndShowH                                       =true;
    CosFunction.bUse2DIDAllSiteFailSetToErrBin                                  =true;  //Steven 20200825 : 2DID all site fail
    CosFunction.bCanUse2x2Bias                                                  =true;  //JerryYang 20220215 : 開啟 2X2 偏心
    IniConfig.bUseAutoSiteMapping                                               =true;  //JerryYang 20220215 : 啟用Auto site mapping
    CosFunction.bCylinderOnOffTimeLog                                           =true;  //JerryYang 20220215 : cylinder作動次數計數
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;      //Ifor 20190308 add Open/Close Site Mapping Function at Any Time
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;      //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bLoginASECL                                                     =true;      //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bBarcodeErrNoTestAndShowH                                       =true;      //KaiChen 20200527 ：Add
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;      //KaiHuang 20201021 : 16Site關Site跑2x4 Mode
    CosFunction.bLoaderAutoRetry                                                =true;      //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
    CosFunction.bHotModeUseDiffScale                                            =true;      //JerryYang 20221024 : add
    CosFunction.bUseOneByOneIndexCheck                                          =true;
    CosFunction.bUseInvisibleOffset                                             =true;
    CosFunction.bSetOffsetLimitToAll                                            =true;      //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
    CosFunction.bUseConfigSaveButton                                            =true;      //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    CosFunction.bTrayMapFromLoader                                              =true;      //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    CosFunction.bTrayDeviceCheckFromLoader                                      =true;      //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    CosFunction.bEnableDual_1x4Kit                                              =true;
    CosFunction.bD44Once4Suck                                                   =true;
    CosFunction.bBySiteByBinPercentCompare                                      =true;      //JerryYang 20230725 : song
    CosFunction.bByBinAlarmFromYieldForm                                        =true;
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;
    CosFunction.bPickupErrorAtLoaderNeedOpenDoor                                =true;
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bRCMDStart                                                      =true;
    CosFunction.RunCheckWhenRecPause                                            =true;
    IniConfig.bInitialStartDelayCount                                           =true;
    CosFunction.bInitialStartDelayCount_Init                                    =true;
    CosFunction.b2DUseAnyCharFunction                                           =true;
//    CosFunction.bConfigAddObjectName                                           =true;
    #ifdef FOR_ASECL_L8
    CosFunction.bZHomingAfterPickErr                                             =false;
    #endif
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_SH()
{
    IniConfig.bInOutArmCanPushHome                                              =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bDualSiteSupply4CH                                                =true;  //jou 2012-11-20 Dual Site supply 4's Channel
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    CosFunction.bAmbientNoShowTemp                                              =true;  //wei 20150609 常溫不顯示
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bOffLineBin                                                     =true;
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_N()                                                                    //JerryYang 20170726 (Steven) add ASE_N客戶功能
{
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bTechComUseComboBox                                             =true;
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    CosFunction.bEnableBarcodeReader                                            =true;  //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔   //Steven 20201126 : add
//    CosFunction.bSLTReportFunction                                            =true;  //Steven 20180420 : SLT報表功能
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_KaohSiung()
{
    bool bUseSecsGem                                                            =false; //kevin 20180511 機台SECSGEM
    IniConfig.iTempeAlarmSecond_Over                                            =1;     //Steven 20111027 : 溫度過高的Alarm時間
    if(REAL_TIME_CCD==false)
        IniConfig.bEnableCCDUSETCPIP                                            =true;
    IniConfig.bAlarmNeedServoOff                                                =false;
    //IniConfig.bLastLoaderNoInSide                                             =true;  //kevin 20221007 mark
    IniConfig.bEnableAutoCleanFunction                                          =true;  //kevin 20120217 Autoclean
//    IniConfig.bL07UseSingleTenmpertureLimit                                   =false;
//    IniConfig.bCheckFixTray                                                   =true;  //kevin 20130705 K15說CC強致使用
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
    CosFunction.bPiggyBackForASE                                                =true;  //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    CosFunction.bPiggyBackShowMainForm                                          =true;  //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
//    if(bUse_NewAutoCleanForm==false)
//        CosFunction.bAutoCleanOnHotPlate                                      =true;  //Steven 20131101 : 將Clean Pad放在HotPlate上,也就是高雄版
//    else
        CosFunction.bAutoCleanOnHotPlate                                        =false; //kevin 20150507 使用Clean 模組

    //IniConfig.bStartProductOnLine                                             =true;  //kevin 20140407 生產前OP OFF_LINE 強制 On line
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    CosFunction.bEnable_SECS_GEM                                                =false; //jou 2012-03-12 Enable SECS_GEM //Steven 20140922
    IniConfig.bUseAutoSiteMapping                                               =true;  //kevin 20140930
//    IniConfig.bAmbientTempControl                                             =true;  //kevin 20140918 常溫恆溫控制
//    IniConfig.bTempOverCannotRun                                              =false; //kevin 20140918 溫度過高停機
    IniConfig.bASE_Report                                                       =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File //Steven 20141125 Add
    IniConfig.bDutOnOffNeedASM                                                  =true;  //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
//    CosFunction.bEnable12Site                                                 =true;  //kevin 20150112 Steven 20120813 : 支援12Site模式
    CosFunction.bContactShowOffset                                              =true;  //kevin 20150519 Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    //CosFunction.bEnableOctal_12Kit                                            =true;  //kevin 20150603 add //ChungHung 20140508 add 使用12 layout kit Octal Shutle kit
    //IniConfig.bIndexPickErrOnlySKIP                                           =true;  //kevin 20180725 pickerror set    //kevin 20151223 //jou 2012-02-13 index pick-up error only skip
    CosFunction.bESDAutoDecayTeachFunction                                      =true;  //kevin 20160623 //Ifor 20151222 :新增 ESD Auto Decay Teach Function
    IniConfig.bInitialStartDelayCount                                           =true;  //kevin 20160720 add jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bCancelErrorBin                                                 =true;  //kevin 20160724 取消 ERROR BIN設定
    IniConfig.bO10UseEventLogSaver                                              =false;
    CosFunction.bHWBinBox                                                       =true;  //kevin 20160819 第7個 HARD WARE BIN support Bin Box
    IniConfig.bRecordSkipPosition                                               =true;  //kevin 20170209 add //jou 2013-05-30 Record Skip position
    IniConfig.bCleanOutCanTrayEnd                                               =IniConfig.bP34CleanOutChangeInitialMode; //kevin 20170417 (wei) add cleanout 切換 Initial
    IniConfig.bDualSiteSupply4CH                                                =true;  //kevin 20170513 (wei) add jou 2012-11-20 Dual Site supply 4's Channel
    IniConfig.bOpenDoorNotStopFan                                               =true;  //kevin 20170522 (wei) add
    IniConfig.bAmbRunChamberFanCanStop                                          =true;  //kevin 20170522 (wei) add jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    IniConfig.bBinBox                                                           =true;  //kevin 20170607 add jou 2012-12-11 support Bin Box
    bUseNSKitKey=CheckIniData("D:\\HT9045\\system\\NSKit.txt","System" , "NSKit");      //kevin 20170814 (Steven) 低FORCE 需軟體鎖住

    if(bUseNSKitKey)                                                                    //kevin 20170814 (Steven) 低FORCE 需軟體鎖住
    {
        CosFunction.bEPUseNSSLK                                                 =true;  //kevin 20170803
    }
    else
    {
        CosFunction.bEPUseNSSLK                                                 =false; //kevin 20210813 add 自定義Kit直徑
    }
    CosFunction.bAllSiteSameFailBinShowAlarm                                    =true;  //kevin 0170828 add JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    IniConfig.bOurArmDropICSkip                                                 =true;  //kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門
    CosFunction.bEnable_SECS_GEM                                                =false; //kevin 20180206 addSteven 20141006 : SECS GEM使用Remote Start功能
    bUseSecsGem=CheckAndReadIniData("C:\\Windows\\System32\\NSKit.txt","System" , "X1",0); //kevin bye 機台設定SECS GEM 功能
    if(bUseSecsGem)                                                                     //kevin 20180511 機台SECSGEM
       CosFunction.bEnable_SECS_GEM =true;                                              //kevin 20180511 bye 機台設定功能
    bUseSecsGem=CheckAndReadIniData("C:\\Windows\\System32\\NSKit.txt","System" , "X1",0); //kevin bye 機台設定SECS GEM 功能
    if(bUseSecsGem)                                                                     //kevin 20180511 機台SECSGEM
       CosFunction.bEnable_SECS_GEM =true;                                              //kevin 20180511 bye 機台設定功能

    IniConfig.bCleanOutCanTrayEnd                                               =true;  //kevin 20180711 addSteven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bRCMDStart                                                      =true;  //kevin 20180803 add Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bUsePEModelFunction                                             =true;  //kevin 20180810 add Ifor 20160822 Use PE Model Function (PE 工程模式)
//    CosFunction.bAfterInitialDelayUseOtherArm                                 =true;  //JerryYang 20181129 取消此功能  //kevin 20180815 add Jerryyang 20180607 : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    CosFunction.bCCLinkValueSaveFile                                            =true;  //kevin 20180828 cc - link save data
    CosFunction.bHaveFIFOMode                                                   =true;  //kevin 20180830 Steven 20160303 : FIFO Mode
    CosFunction.bSiteCmpYield                                                   =true;  //Steven 20170308 add to JCET
    CosFunction.bLowYeildByTotal                                                =true;  //Steven 20170308 add to JCET
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //kevin 20181011 Steven 20141105 : Index內的所有異常都只能用Skip
    //IniConfig.bQAMode                                                         =true;  //kevin 20181019 客戶要求disable QA mode
    CosFunction.bEnable_SECS_GEM                                                =true;  //kevin 20181101 JerryYang 20170306 (wei) Enable SECS_GEM
    //CosFunction.bUseOneByOneIndexCheck                                        =true;  //kevin 20221004  //kevin 20220922  //kevin 20190529 add Ifor 20180322 : add Use One By One Index Check
    CosFunction.bGPIBLotEnd                                                     =true;  //kevin 20190613 addSam 20181030 : 透過 GPIB 送 LotEnd 給測試機。
    CosFunction.bCanUse2x2Bias                                                  =true;  //Steven 20190724 : 2x2偏心開關
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;
    CosFunction.bPopAutoClean                                                   =true;  //JerryYang 20190918 Pop auto clean
    CosFunction.bEnableOctal_12Kit                                              =true;
    CosFunction.bEnableAutoSpeed                                                =true;  //kevin 20201030
    CosFunction.bUseHandlerSetChillerTemp                                       =false; //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //kevin 20201215 addjou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bInOutArmZCalibration                                           =true;  //kevin 20210205 add Frank 20171213 (Steven) : In/OutArmZCalibration
    IniConfig.bEnableUnloadTrayFree                                             =true;  //kevin 20210219
    CosFunction.bRecordVacuumOnOffTime                                          =true;  //Sam 20210204 : Auto Teach Loader Z Record Vacuum On off time
    CosFunction.bIOTriggerIonFanAutoClean                                       =true;  //Isaac 20210609 : IO觸發IonFan清針
    CosFunction.bYPitchNotUseSearchLastMode                                     =true;  //kevin 20210617 add JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
    CosFunction.bEnable2x1Site                                                  =true;
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //kevin 20211222 add Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    CosFunction.bBarcodeTrayRecFile                                             =true;  //jou 20190930 : Barcode Tray record file
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;  //kevin 20220630
    IniConfig.bQAMode                                                           =true;  //kevin 20220729 add QA mode
    CosFunction.bAutoSkipNoDropError                                            =true;  //kevin 20220930 add Drop
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_M()
{
//    IniConfig.bSingaporeFunction                                              =true;  //Steven 20120910 : 新加坡代理商的需求
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
//    IniConfig.bIndexJamInArmAway                                              =true;
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    IniConfig.bQAMode                                                           =true;
    IniConfig.b1x2Use4Suck                                                      =true;  //Hung 20110812 : 1x2使用4吸嘴
    CosFunction.bUseFix3FullTray                                                =true;  //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
//    IniConfig.bFix3PutAllFullIC                                               =true;  //Ifor 20161205 ASE_M 嘗試Fix3放滿功能開啟 (會有衝突不可使用，後續修改若有開啟bUseFix3FullTray強制關閉)
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bShowLastContactHigh                                            =true;  //Steven 20140409 : 顯示之前的Contact高度
    IniConfig.bResetCanServoOff                                                 =true;  //Steven 20140807
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bIndexDropErrCanMove                                              =true;  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bShowFunctionWindow                                               =true;  //Ifor 20170626 (wei) ASEM Jackson 要求不顯示畫面 //Ifor 20171204 ASEM Jackson要求加回去
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =true;  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bEnableCCDUSETCPIP                                                =true;
//    IniConfig.bDisableSelectSearchLast                                        =true;  //jou 2012-01-10 取消Setup，Search Last Mode功能。
    IniConfig.bShowOffYieldBlink                                                =false; //Steven 20120609 : 功能關畫面要不要閃爍
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    CosFunction.bAutoKTemp                                                      =true;  //Steven 20120719 : 自動K溫
//    IniConfig.bLastLoaderNoInSide                                             =true;  //Steven 20110922 : 最後一盤不入料
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    CosFunction.bUseAlarmUnlockPassWord                                         =true;  //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
    CosFunction.bTechComUseComboBox                                             =true;  //Ifor 20170426 (wei) add ASEM 使用密碼本時,用下拉選單選使用者
    CosFunction.bUseAutoBackUpSetupFile                                         =true;  //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    CosFunction.bUseARMSFunction                                                =true;  //Ifor 20170621 (wei) add ARMS Function
    IniConfig.bShowFTandRTButton                                                =true;  //Ifor 20170704 (wei) add ASEM Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Ifor 20170704 (wei) add ASEM FT & RT Buttion 可以切換
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20170706 (wei) add ASEM ShowLotInfo
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;
    CosFunction.bATC32UseTJMode                                                 =true;  //Ifor 20190121 : add ATC32 Use TJ Mode
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;  //Ifor 20190308 add Open/Close Site Mapping Function at Any Time
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //Ifor 20181227 add bSetupFileName Control By Level
    CosFunction.bUseDynamicKitDiameter                                          =true;  //kevin 20200722 Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.bUseShuttlePickShiftDetect                                      =true;  //Ifor 20221220 add: Shuttle 吸料後 先上升設定高度再檢查有無IC
    CosFunction.bUseFTPDownloadDataCheck                                        =false; //Ztex 2025.03.28 Add HT-1132
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ztex 2025.03.28 Add HT-1132
    CosFunction.bCanRemoteStart                                                 =true;  //Ifor 20250816 add : ASEM 要求遠端可以START
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_KaohSiung_K3()
{
    CosFunction.bEnable_SECS_GEM                                                =true;  //KenHsieh 20220912 : K1 SECS功能開啟
    CosFunction.bRCMDStart                                                      =true;  //KenHsieh 20220912 : add Steven 20141006 : SECS GEM使用Remote Start功能
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_KaohSiung_K11()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_UTAC()
{
    CosFunction.b2x4SupportCenterPitch                                          =true;  //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bUseShuttlePickShiftDetect                                      =true;  //Ifor 20221220 add: Shuttle 吸料後 先上升設定高度再檢查有無IC
}
//------------------------------------------------------------------------------
void FUNC_CC_SIGURD_HUKOU()
{
    IniConfig.bSIGURDFunction                                                   =true;  //KaiChen 20200506 ：矽格統一軟體功能
    IniConfig.bCanDisableTempMonitor                                            =true;  //能夠關閉溫控器監視。
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bShowMainDebugRecord                                              =true;  //jou 2013-02-25 Show Main form Debug Record
    CosFunction.bLockD41ByFile                                                  =true;  //Steven 20140627 : Add for ASE-CL        //wei 20141203 add
    IniConfig.bFix3PutAllFullIC                                                 =true;
    CosFunction.bNotClearAllHotBuffer                                           =true;  //JerryYang 20151226 For 矽格 由GPIB設定完溫度後，會馬上再問一次溫度。所以不清除暫存溫度
    IniConfig.bHeadSocketMode                                                   =true;  //Alick 20160628 矽格湖口修改開啟
    IniConfig.bHeadChamberSocketMode                                            =true;  //Alick 20160628 矽格湖口修改開啟
    CosFunction.ShowLotCategoryCount                                            =true;  //KaiChen 20181002 ：Category Count by Lot
    CosFunction.ShowLotCount                                                    =true;  //KaiChen 20181002 ：改成CosFunction   //wei 20171130 Count By Lot
    CosFunction.bYieldControlBinSelectUseContactCount                           =true;  //KaiChen 20181115 ：矽格-湖口 要求 BinSelect 使用 Contact Count
    CosFunction.bPickupErrorAtLoaderNeedOpenDoor                                =true;  //KaiChen 20181126 ：矽格-湖口 Add
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //KaiChen 20190918 ：矽格-湖口 Add 鍾卓劭, Index內的所有異常都只能用Skip
    CosFunction.bOutShtLoseICInArmAway                                          =true;  //KaiChen 20200722 ：矽格-湖口，Add
    CosFunction.bTempSetDisenableNotUseHeater                                   =true;
    CosFunction.bLockF26ByFile                                                  =true;  //Sam 20220527 : for 矽格湖口 -- F26 Enable
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Sam 20230220 : 移除客戶碼
    CosFunction.bLockF06ByFile                                                  =true;
    CosFunction.bLockF11ByFile                                                  =true;  //Sam 20240202 : 新增 F11 Lock by file 功能
    IniConfig.bRTCbySystem                                                      =true;
    CosFunction.bLockRTCByFile                                                  =true;  //Sam 20240311 : 新增 RTC Lock by file 功能
    CosFunction.bAfterInitialDelayUseOtherArm                                   =true;  //Sam 20240624 : 湖口廠也要開啟 Arm2 預熱功能。
    CosFunction.bGPIBUseSECSGENData                                             =true;  //Sam 20240826 : GPIB 通訊資料使用 SECSGEM Data
    IniConfig.bUseAutoSiteMapping                                               =true;  //Sam 20241225 :  客戶付費開啟
    CosFunction.bUSEJCETSiteMapMode                                             =true;
    IniConfig.bDutOnOffNeedASM                                                  =true;  //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bAutoSiteMappingSetOpenBIN                                      =true;
    CosFunction.bAutoSiteMappingUseFailBinSetting                               =true;
    CosFunction.bDownloadUpdateAutomatically                                    =true;  //Sam 20250328 : 湖口廠也要自動安裝功能
    CosFunction.bCheckOutSuckICFallDownOnlySkip                                 =true;  //Jimmychiu 20240813 : Only Skip When Checking Out Suck IC Fall Down
}
//------------------------------------------------------------------------------
void FUNC_CC_RFMD_BEIJING()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_JSCC_OS()                                                          //長電微電子 (JSCC OS部門)
{
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bI21EnableASMByRecipe                                           =true;
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bIndexDropErrCanMove                                              =true;  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    IniConfig.bIndexPickupErrStop                                               =true;  //jou 2012-02-29 index pick up error,index arm move to center & alarm
    IniConfig.bIndexJamInArmAway                                                =true;
    if(REAL_TIME_CCD==true)
        IniConfig.bShuttleMode50                                                =true;  //暫時先打開，等RTC關Arm功能改好
    else
        IniConfig.bShuttleMode50                                                =false; //暫時先打開，等RTC關Arm功能改好
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    IniConfig.bRTCbySystem                                                    =true;  //ChungHung 20120716 RTC by System  //Ifor 20181222 : Mark SCC 要求改By Setup File
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bControlTorque                                                    =true;  //jou 2013-11-05 Index Control Torque
    IniConfig.bOutShLoseNeedOpenChamber                                         =true;  //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20140715 Add
    CosFunction.bFuncStateStopFirtDelay                                         =true;  //jou 2014-09-03 Function State Stop Firt Initital Delay Time
    CosFunction.bConAlarmNeedKeyInPassword                                      =true;  //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password
    CosFunction.bShowHandlerStopTime                                            =true;  //jou 2014-09-21 Show Handler Stop Time
    CosFunction.bTesterLowYieldOneCycle                                         =true;  //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
    CosFunction.bConFailFoolProofing                                            =true;  //jou 2014-09-28 continue fail 防呆提醒,總開關開啟為On時,BIN別未勾選任何continue fail alarm
    CosFunction.bTempLess30degShowLight                                         =true;  //jou 2014-12-19 Temperature Less 30 deg. Show Light
    CosFunction.bTempHeaterOkShowLight                                          =true;  //jou 20180529 : Temperature Heater Ok Show Light
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bOutShuttleSensorCanNotDisable                                  =true;  //Steven 20151202 : Out Shuttle Sensor不能關閉檢查
//    CosFunction.bOutShuttleLoseICNeedHome                                     =true;  ///JerryYang 20160225
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //Alick 20160727 add for SCC
//    CosFunction.bFTRTDifferentDutOnOff                                        =true;  //JerryYang 20170816 (Steven) Mark,SCC要求取消此功能  //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
    CosFunction.bIndexPickErrSkipNeedCheckVac                                   =true;  //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
    CosFunction.bOutShtLoseICSetErrUntilOneCycle                                =true;  //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin
//    CosFunction.bTechComUseComboBox                                           =true;  //Alick 20160727 add for SCC
    CosFunction.bUse32ChanelSiteMap                                             =true;  //Steven 20170530 (wei) : Use 32CH site map
//    CosFunction.b2x4SupportCenterPitch                                        =true;  //Pending //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20170911 (Steven) add SCC 新增海思全流程 Log
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Ifor 20170913 (Steven) add SCC使用Lot ID
    CosFunction.bHisiLogUploadNetwork                                           =true;  //Ifor 20170913 (Steven) add 海思 Log 上傳至網路磁碟
    CosFunction.bUseAuto1OnlyBin1                                               =true;  //Ifor 20171018 (wei) : Only Bin 1 is allowed to set to Auto 1
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
//    CosFunction.bResetFirstTestToErr                                          =true;  //jou 20180103 : Reset第一個測試完的IC要丟到Error bin
//    IniConfig.bResetPutUntestToErrorBin                                       =true;  //Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin
    CosFunction.bFTPFunction                                                    =true;  //Ifor 20181023 SCC Add FTP Function
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
//    CosFunction.bUseDefineAutoCleanOffset                                     =true;  //Ifor 20181106 add 新增Auto Clean Offset By 本機
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;  //Ifor 20181222 add 新增Auto Clean 12 site取放位置固定
    CosFunction.bUseAutoCleanCloseSiteAlsoDo                                    =true;  //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
//    CosFunction.bIndexPickErrNeedPiggyBack                                    =true;  //jou 20171211 (Steven) : After index pick up error, device go to Error bin ,follow by Index check or piggy back check
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bSiteCmpYield                                                   =true;
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bInitialStartDelayCount                                           =true;
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bByBinAlarmFromYieldForm                                        =true;  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bYieldAlarmUseDouble                                            =true;
    #ifdef HiSilicon
        CosFunction.bCanUseBias                                                 =false; //Steven 20190801 : SCC李國旗說要取消BIAS選項
    #else
        CosFunction.bCanUseBias                                                 =true;  //Steven 20190801 : SCC李國旗說要取消BIAS選項
    #endif
    CosFunction.bUseHeadContactCount                                            =true;  //Ifor 20160516 京元要求銦片 Life Time 功能
//    CosFunction.bHeadContactCountByRecipe                                       =true;  //Steven 20241030 : 銦片 Life Time 功能by工作檔  //RogerYang 20260515 : dead flag, 未被任何邏輯讀取
    CosFunction.bCanChangeAutoCleanCount                                        =true;
    IniConfig.bShowFTandRTButtonCanClick                                        =true;
    CosFunction.bUseN07_5                                                       =true;  //Steven 20200309 : [N07-5]改成by客戶開啟
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
//    CosFunction.bAutoCleanUse2RowBy46LS                                       =true;
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;
    CosFunction.bEnableOctal_16Kit                                              =true;
//    CosFunction.bKeepOnly1SetupFile                                             =true;  //RogerYang 20260127 : 田揚志需求移除 //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bContactHeightSaveToContactIni                                  =true;  //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
    IniConfig.bSiteMappingFastSetDisable                                        =true;  //JerryYang 20200916 SCC楊建軍要求取消site map快捷
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bOurArmDropICSkip                                                 =true;  //kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門
//    CosFunction.bUSEJCETSiteMapMode                                           =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bS5F1UseJamRate                                                 =true;  //Steven 20220331 : S5F1的ALTX裡面加上JAM Rate判斷
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bDownloadUpdateAutomatically                                    =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.PassworDownloadByFTP                                            =true;
    CosFunction.bNeedAlarmAfterUnloaderFull                                     =true;  //Jimmychiu 20240902 : Need Alarm After Unloader Full
    CosFunction.bUseSocketContactCount                                          =true;
    CosFunction.bFTPUseBarcodeReader                                            =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_CAPCON()                                                           //北京華封
{
    CosFunction.bEnable_SECS_GEM                                                =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SCC()
{
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bI21EnableASMByRecipe                                           =true;
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bIndexDropErrCanMove                                              =true;  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    IniConfig.bIndexPickupErrStop                                               =true;  //jou 2012-02-29 index pick up error,index arm move to center & alarm
    IniConfig.bIndexJamInArmAway                                                =true;
    if(REAL_TIME_CCD==true)
        IniConfig.bShuttleMode50                                                =true;  //暫時先打開，等RTC關Arm功能改好
    else
        IniConfig.bShuttleMode50                                                =false; //暫時先打開，等RTC關Arm功能改好
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    IniConfig.bRTCbySystem                                                    =true;  //ChungHung 20120716 RTC by System  //Ifor 20181222 : Mark SCC 要求改By Setup File
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bControlTorque                                                    =true;  //jou 2013-11-05 Index Control Torque
    IniConfig.bOutShLoseNeedOpenChamber                                         =true;  //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20140715 Add
    CosFunction.bFuncStateStopFirtDelay                                         =true;  //jou 2014-09-03 Function State Stop Firt Initital Delay Time
    CosFunction.bConAlarmNeedKeyInPassword                                      =true;  //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password
    CosFunction.bShowHandlerStopTime                                            =true;  //jou 2014-09-21 Show Handler Stop Time
    CosFunction.bTesterLowYieldOneCycle                                         =true;  //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
    CosFunction.bConFailFoolProofing                                            =true;  //jou 2014-09-28 continue fail 防呆提醒,總開關開啟為On時,BIN別未勾選任何continue fail alarm
    CosFunction.bTempLess30degShowLight                                         =true;  //jou 2014-12-19 Temperature Less 30 deg. Show Light
    CosFunction.bTempHeaterOkShowLight                                          =true;  //jou 20180529 : Temperature Heater Ok Show Light
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bOutShuttleSensorCanNotDisable                                  =true;  //Steven 20151202 : Out Shuttle Sensor不能關閉檢查
//    CosFunction.bOutShuttleLoseICNeedHome                                     =true;  ///JerryYang 20160225
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //Alick 20160727 add for SCC
//    CosFunction.bFTRTDifferentDutOnOff                                        =true;  //JerryYang 20170816 (Steven) Mark,SCC要求取消此功能  //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
    CosFunction.bIndexPickErrSkipNeedCheckVac                                   =true;  //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
    CosFunction.bOutShtLoseICSetErrUntilOneCycle                                =true;  //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin
//    CosFunction.bTechComUseComboBox                                           =true;  //Alick 20160727 add for SCC
    CosFunction.bUse32ChanelSiteMap                                             =true;  //Steven 20170530 (wei) : Use 32CH site map
//    CosFunction.b2x4SupportCenterPitch                                        =true;  //Pending //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20170911 (Steven) add SCC 新增海思全流程 Log
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Ifor 20170913 (Steven) add SCC使用Lot ID
    CosFunction.bHisiLogUploadNetwork                                           =true;  //Ifor 20170913 (Steven) add 海思 Log 上傳至網路磁碟
    CosFunction.bUseAuto1OnlyBin1                                               =true;  //Ifor 20171018 (wei) : Only Bin 1 is allowed to set to Auto 1
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
//    CosFunction.bResetFirstTestToErr                                          =true;  //jou 20180103 : Reset第一個測試完的IC要丟到Error bin
//    IniConfig.bResetPutUntestToErrorBin                                       =true;  //Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin
    CosFunction.bFTPFunction                                                    =true;  //Ifor 20181023 SCC Add FTP Function
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
//    CosFunction.bUseDefineAutoCleanOffset                                     =true;  //Ifor 20181106 add 新增Auto Clean Offset By 本機
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;  //Ifor 20181222 add 新增Auto Clean 12 site取放位置固定
    CosFunction.bUseAutoCleanCloseSiteAlsoDo                                    =true;  //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
//    CosFunction.bIndexPickErrNeedPiggyBack                                    =true;  //jou 20171211 (Steven) : After index pick up error, device go to Error bin ,follow by Index check or piggy back check
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bSiteCmpYield                                                   =true;
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bInitialStartDelayCount                                           =true;
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bByBinAlarmFromYieldForm                                        =true;  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bYieldAlarmUseDouble                                            =true;
    #ifdef HiSilicon
        CosFunction.bCanUseBias                                                 =false; //Steven 20190801 : SCC李國旗說要取消BIAS選項
    #else
        CosFunction.bCanUseBias                                                 =true;  //Steven 20190801 : SCC李國旗說要取消BIAS選項
    #endif
    CosFunction.bUseHeadContactCount                                            =true;  //Ifor 20160516 京元要求銦片 Life Time 功能
//    CosFunction.bHeadContactCountByRecipe                                       =true;  //Steven 20241030 : 銦片 Life Time 功能by工作檔  //RogerYang 20260515 : dead flag, 未被任何邏輯讀取
    CosFunction.bCanChangeAutoCleanCount                                        =true;
    IniConfig.bShowFTandRTButtonCanClick                                        =true;
    CosFunction.bUseN07_5                                                       =true;  //Steven 20200309 : [N07-5]改成by客戶開啟
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
//    CosFunction.bAutoCleanUse2RowBy46LS                                       =true;
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;
    CosFunction.bEnableOctal_16Kit                                              =true;
    CosFunction.bKeepOnly1SetupFile                                             =true;  //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bContactHeightSaveToContactIni                                  =true;  //Steven 20200616 : JSCC要求把Contact Height放到別的檔案

    CosFunction.bEnable_SECS_GEM                                                =true;
    IniConfig.bSiteMappingFastSetDisable                                        =true;  //JerryYang 20200916 SCC楊建軍要求取消site map快捷
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bOurArmDropICSkip                                                 =true;  //kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門
//    CosFunction.bUSEJCETSiteMapMode                                           =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bSECS_GEM_OneCycle                                              =true;
    CosFunction.bS5F1UseJamRate                                                 =true;  //Steven 20220331 : S5F1的ALTX裡面加上JAM Rate判斷
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bDownloadUpdateAutomatically                                    =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.PassworDownloadByFTP                                            =true;
    CosFunction.bNeedAlarmAfterUnloaderFull                                     =true;  //Jimmychiu 20240902 : Need Alarm After Unloader Full
    CosFunction.bUseSocketContactCount                                          =true;
    CosFunction.bRunModeFollowLotInfo                                           =true;  //Steven 20250603 : 根據Lot Info的Run mode進行切換
    CosFunction.bLowYeildByTotal                                                =true;  //wei 20151116 Low Yeild By Total
    IniConfig.bLowYieldAlarmSameNS                                              =true;  //KaiChen 20180612 ：矽格-北興 add
    CosFunction.bYieldAlarmNoWait1Min                                           =true;
    CosFunction.bPickerLifeAlmNeedOneCycle                                      =true;  //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
}
//------------------------------------------------------------------------------
void FUNC_CC_SCS()
{
    IniConfig.bSingaporeFunction                                                =true;  //Steven 20120910 : 新加坡代理商的需求
    IniConfig.bUseAutoSiteMapping                                               =true;
    IniConfig.bQAMode                                                           =true;
    IniConfig.bDutOnOffNeedASM                                                  =true;  //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
    IniConfig.bResetPutUntestToErrorBin                                         =true;  //Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bNewResetFunction                                                 =true;  //Steven 20130625 : 新的Reset方式
    CosFunction.bStopMustTestTimeOut                                            =true;  //Steven 20130703 : 避免測試Hang Up
    IniConfig.bEnableStepShuttle                                                =true;  //jou 2013-07-16 Step Shuttle check Index -> Input
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    IniConfig.bOpenDoorNotStopFan                                               =true;
    IniConfig.bIndexDropOnlySKIP                                                =true;
    CosFunction.bAutoCleanAutoSelIndexArm                                       =true;  //jou 2015-08-12 Auto Clean Auto Select Index Arm

//    CosFunction.bAutoCleanShuttleDisable                                      =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect  //jou 2016-05-04 SCS 要求顯示Auto Clean shuttle sensor選項
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    CosFunction.bLockD41ByFile                                                  =true;  //Steven 20140627 : Add for ASE-CL
    CosFunction.bInOutArmUseBackRowSuck                                         =true;  //jou 20161122 (Steven) In & Out arm use back row suck
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;  //Steven 20180627 (wei) : Time out預設只顯示SKIP
    CosFunction.bTempCalByRecipe                                                =true;  //jou 20220725 : Temperature calibration by recipe
    CosFunction.bPassBinNoRotate                                                =true;  //jou 20231020 : Pass bin no rotate
    CosFunction.bRotateUseRTmode                                                =true;  //jou 20231122 : Rotate Use RT mode
    CosFunction.bUseFTPDownloadDataCheck                                        =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SIGURD_ChungXing()
{
    IniConfig.bSIGURDFunction                                                   =true;  //KaiChen 20200506 ：矽格統一軟體功能
    IniConfig.bFix3PutAllFullIC                                                 =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Ifor 20170505 (wei) add 矽格中興廠使用Lot ID
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bFTPDataTrayFeedAutoUpdata                                      =true;  //Ifor 20181214 (Steven) : add Tray Feed 後自動上傳海思檔案至FTP
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //KaiChen 20200203 ：矽格-中興 add
    IniConfig.bUseTrayBlockMode                                                 =true;  //KaiChen 20200529 ：矽格-中興，加入
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;
    CosFunction.bSortingBinTraywhenCleanOut                                     =true;  //JerryYang 20151006 整盤功能
    CosFunction.bRecordGroundESDByTestIC                                        =true;  //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    //CosFunction.b2DCodeCheckByCoustomerLot                                    =true;  //Sam 20220223 : 2D Code Check by Coustomer Lot
    CosFunction.bOutShtLoseICInArmAway                                          =true;
    CosFunction.bUnloadCylinerUpRelief                                          =true;  //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
    CosFunction.bD44Once4Suck                                                   =true;  //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
    CosFunction.bInitTempOffsetByWorkFile                                       =true;
    CosFunction.bAfterInitialDelayUseOtherArm                                   =true;
    CosFunction.bRCMDStart                                                      =true;  //Sam 20250331 : 矽格中興開啟 Remote Start
}
//------------------------------------------------------------------------------
void FUNC_CC_SIGURD_PeiXing()
{
    IniConfig.bSIGURDFunction                                                   =true;  //KaiChen 20200506 ：矽格統一軟體功能
    CosFunction.bSortingBinTraywhenCleanOut                                     =true;  //JerryYang 20151006 整盤功能
    CosFunction.bTesterLowYieldOneCycle                                         =true;  //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
    CosFunction.bEnableSoftWareControlButton                                    =true;  //JerryYang 20160302 建榮要求軟體控制面板功能
    CosFunction.bContactTestVacOffByCloseSite                                   =true;  //JerryYang 20160328 Contact test及auto height時,關site的部分不吸取IC
    CosFunction.bG09NeedPasswordWhenEditSiteMap                                 =true;  //JerryYang 20160425 修改Site map需要密碼
    CosFunction.bLastSetInSetUpFile                                             =true;  //JerryYang 20160422 把LastSet選項存在Set Up File
    CosFunction.bLockP24ByFile                                                  =true;  //JerryYang 20160425 lock P24
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //Alick 20161122 add 北興要求在未CleanOut狀態下可以修改ContinuesFailBin
    CosFunction.bEnable12Site                                                   =true;  //Steven 20120813 : 支援12Site模式  //Ifor 20170420 (wei) 新增SIGURD PeiXing 支援12Site模式
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
    IniConfig.bEventLogAutoSaveFunction                                         =true;  //Ifor 20171018 (wei) add 矽格北興廠使用自動存EventLog
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Ifor 20171018 (wei) add 矽格北興廠使用Lot ID
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bFTPDataTrayFeedAutoUpdata                                      =true;  //Ifor 20181214 (Steven) : add Tray Feed 後自動上傳海思檔案至FTP
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //KaiChen 20180612 ：矽格-北興 add
    IniConfig.bLowYieldAlarmSameNS                                              =true;  //KaiChen 20180612 ：矽格-北興 add
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //KaiChen 20181023 ：矽格-北興 add
    CosFunction.bSaveEventLogByLotID                                            =true;  //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
    CosFunction.bLockF06ByFile                                                  =true;  //KaiChen 20181027 ：矽格-北興 add
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //KaiChen 20190527 ：矽格-北興 add
    CosFunction.bUseDynamicKitDiameter                                          =true;  //KaiChen 20190628 ：矽格-北興 add
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //KaiChen 20191017 ：矽格-北興 add
    CosFunction.bRotateUseHT7000HPKit                                           =true;  //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
    IniConfig.b1x2Use4Suck                                                      =true;
    CosFunction.bEnableAfterTestedDelay                                         =true;
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bNeedToRestartGroundWhenInitialStart                            =true;  //Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Sam 20220117 : Add SIGURD_PeiXing
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bHeadChamberSocketMode                                            =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
    CosFunction.bAutoTmpeOfsByFTP                                               =true;  //Sam 20220406 : 溫度自動補償功能 By FTP
    CosFunction.bLockI06ByFile                                                  =true;  //Sam 20220527 : for 矽格北興
    CosFunction.bManuallyRemoveForceInColor                                     =true;  //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    CosFunction.bOutShtLoseICInArmAway                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bDownloadUpdateAutomatically                                    =true;  //Sam 20220824 : FTP 自動下載安裝更新包
    CosFunction.bAutoSideFixerCheckAlarmAutoRetry                               =true;  //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
    CosFunction.bAutoOpenConfigA01                                              =true;  //Sam 20220929 : Config A01 自動開啟。
    CosFunction.bEnable6Site                                                    =true;  //Sam 20221003 : 北興開啟
    CosFunction.bSpecialP24                                                     =true;  //Sam 20230320 : 整合 P24 北興版為獨立功能碼
    CosFunction.bAfterInitialDelayUseOtherArm                                   =true;
    CosFunction.bInitTempOffsetByWorkFile                                       =true;
    IniConfig.bQAMode                                                           =true;  //Sam 20231117 : 整合到 QA 模式
    CosFunction.bVerifyMode                                                     =true;  //Sam 20231117 : 整合到 QA 模式
    CosFunction.bDisableOpenAllSiteWhenChangeShtMod                             =true;  //Jimmychiu 20241203 : Disable opening all sites when changing shuttle mode.
    CosFunction.bContactAlmNeedOneCycle                                         =true;  //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Sam 20250415 : 開啟 RTC CheckSiteMap
    IniConfig.b2x2Use8Suck                                                      =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SCK()
{
    IniConfig.bKoreaFunction                                                    =true;
    CosFunction.bStopMustTestTimeOut                                            =true;
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bRetryNoNeedRestartGpib                                           =true;  //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bShuttleMode50                                                    =true;  //Steven 20120417 : RTC開單Arm Hang Up
    IniConfig.bRTCbySystem                                                      =true;  //ChungHung 20120716 RTC by System
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =false; //ChungHung 20120911 add 與OneCycle can desable site 衝突
    CosFunction.bFullTestBeforeContactHeight                                    =true;  //ChungHung 20130122 add for SCK Contact Height前做FullView
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
//    CosFunction.bSendJamCodeByTcpIp                                           =true;  //ChungHung 20150518 add for SCK Send JamCode By TcpIp
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 : For SCK 93K ART
    CosFunction.bUseInitialDelayAsSoakTime                                      =true;  //Steven 20170329 (Wei) : 使用initial delay當 Soak time
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    CosFunction.bCanChangeAutoCleanCount                                        =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bShowLastContactHigh                                            =true;
//    CosFunction.bNextTrayNeedManualRemoveTray                                 =true;  //Steven 20220517 : Mark
    CosFunction.bAutoCloseSiteWhenRT                                            =true;  //Steven 20200225 : 切到RT的時候,要關閉Socket
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    CosFunction.bCleanOutCanTrayEndBySetupFile                                  =true;  //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
//    CosFunction.bInOutArmUseBackRowSuck                                       =true;

    if(fSCKART!=NULL && fSCKART->iTesterType==1)                                        //Steven 20161201 (wei) : For SCK 93K ART
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    CosFunction.bOpenAutoCloseSiteInNormal                                      =true;  //JimmyChiu 20211015 : only user disabled site keep disabled and other sites change to enabled
    CosFunction.bRTC_ROICount                                                   =true;
    CosFunction.bHaveFIFOMode                                                   =true;
//    CosFunction.bCheckTempDuringIndexArmTesting                                 =true;  //Jimmychiu 20240916 : Check the temperature during index arm testing
    CosFunction.bRMSNoNeedToDownloadEveryTime                                   =true;  //Steven 20240926 : RMS不要每次下載包成功能
    CosFunction.bIndexCheckCanTurnOff                                           =true;
    CosFunction.bAutoTmpeOfsByFTP                                               =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
}
//------------------------------------------------------------------------------
void FUNC_CC_RFMD_USA()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ASE_KS()
{
    IniConfig.bInOutArmCanPushHome                                              =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bDualSiteSupply4CH                                                =true;  //jou 2012-11-20 Dual Site supply 4's Channel
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    CosFunction.bAmbientNoShowTemp                                              =true;  //wei 20150609 常溫不顯示
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bOffLineBin                                                     =true;
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_APTOS()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_WINBOND()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_G_Link()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_AMKOR()
{
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //ChungHung 2011/12/08
    IniConfig.bContactAlwaysIncludeShuttle                                      =true;  //ChungHung 2011/12/08
    IniConfig.bNoHeadaddChamberOption                                           =true;  //ChungHung 2011/12/08
    IniConfig.bDisibleResetButton                                               =true;  //ChungHung 2011/12/08
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bDisableSelectSearchLast                                          =true;  //jou 2012-01-10 取消Setup，Search Last Mode功能。
    IniConfig.bIndexDropErrCanMove                                              =true;  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    IniConfig.bIndexPickupErrStop                                               =true;  //jou 2012-02-29 index pick up error,index arm move to center & alarm
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =false; //ChungHung 20120911 add 與OneCycle can desable site 衝突
    CosFunction.bOneCycleCanChangeArm                                           =true;  //Hung 20111108 Amb/Hot Mode OneCycle Can Change Arm
    IniConfig.bOnlyRoomOrHot                                                    =true;
    IniConfig.bQAMode                                                           =true;
    IniConfig.bTrayAssignUseGraphic                                             =true;  //Steven 20111121 : 使用圖片去顯示Tray Assign
    IniConfig.bUseAutoSiteMapping                                               =true;  //Steven 20111208 : 客戶需求
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bRetryNoNeedRestartGpib                                           =true;  //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bFTTrayAss2RTTrayAss                                              =true;  //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    IniConfig.bRemeberAutoHeight                                                =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bResetCanServoOff                                                 =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bLastLoaderAutoCleanOut                                           =false; //Steven 20111028 : 搭配"最後一盤不入料"功能,要不要自動CleanOut
    IniConfig.iTempeAlarmSecond_Over                                            =4;     //Steven 20111027 : 溫度過高的Alarm時間  //Steven 20111215 2-->4
    IniConfig.iTempeAlarmSecond_Below                                           =2;     //Steven 20111027 : 溫度過低的Alarm時間
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =false; //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    IniConfig.bSiteMappingFastSetDisable                                        =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    CosFunction.bStopMustTestTimeOut                                            =true;
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bEnableBarcodeReader                                            =true;  //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
    CosFunction.bAmkorFunction                                                  =true;  //Steven 20171017 (wei) : Functions for Amkor Group
}
//------------------------------------------------------------------------------
void FUNC_CC_GIGA()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_LINGSEN()
{
    IniConfig.bTesterTimeUpErrorNeedPassword                                    =true;  //jou 2012-08-28 Tester Time up Error Need Password
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;  //Steven 20180627 (wei) : Time out預設只顯示SKIP   //菱生測試機當掉只能使用skip
}
//------------------------------------------------------------------------------
void FUNC_CC_GONGJIN_SHANGHAI()
{
    IniConfig.bBinBox                                                           =true;  //jou 2012-12-11 support Bin Box
}
//------------------------------------------------------------------------------
void FUNC_CC_GONGJIN_SUZHOU()
{
    IniConfig.bBinBox                                                           =true;  //jou 2012-12-11 support Bin Box
}
//------------------------------------------------------------------------------
void FUNC_CC_Greatek()
{
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bOffLineBin                                                     =false; //Hung 20111110 新增Offline Bin
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    CosFunction.bUsePMAlarmFunction                                             =true;  //wei 20160225 PMAlarmFunction
    CosFunction.bYieldAlarm4                                                    =true;  //wei 20160406 Yield Alarm4
    CosFunction.bUseMonitorView                                                 =true;  //Ifor 20160930 add USE MonitorView 錄影監視功能顯示
    IniConfig.bShowLotInfo                                                      =true;  //Sam 20170719 (Steven) 開啟 Lot 功能
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Sam 20170719 (Steven) 開啟 Lot 功能
    CosFunction.bFTPFunction                                                    =true;  //Sam 20170815 (Steven) 超豐開啟 FTP 功能
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bOEEFunction                                                    =true;  //Steven 20180417 (Jou) : OEE功能
    CosFunction.bRemoteLotStart                                                 =true;  //AI(ht9045-v899) 20260331: Greatek TCP Remote Start Lot (HTSET 720)
    CosFunction.bIndexArmVacOffErrNeedPiggyBack                                 =true;
    CosFunction.bNextTrayNeedManualRemoveTray                                   =true;  //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
    CosFunction.bKeepOnly1SetupFile                                             =true;  //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bAllSiteSameFailBinShowAlarm                                    =true;
//    CosFunction.bLoadPickUpErrorMES0101Record                                   =true;  //Sam 20200922 : Pick up Error Production_Log 也要記錄
    CosFunction.bRecipeParameterDefault                                         =true;  //Sam 20201209 : Default Recipe ChangeLog
    CosFunction.bRecipeParameterDefaultLog                                      =true;  //Sam 20201209 : Default Recipe ChangeLog
    CosFunction.bProgramStartOnLine                                             =true;  //Sam 20210423 : 程式開啟時切換為 OnLine/Real
    CosFunction.bUseOneByOneIndexCheck                                          =true;
    CosFunction.bManualSteplAutoTeach                                           =true;  //JimmyChiu 20211020 : Auto alignment mode
    CosFunction.bSetupTeach                                                     =true;  //JerryYang 20180921 Setup Teach功能 JimmyChiu 20211102 Add
    CosFunction.bConfigStandard                                                 =true;  //Sam 20210324 : 程式開啟，直接讀取 config_Standard.ini
    CosFunction.bOffsetContactForce                                             =true;  //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
    CosFunction.bUseFTPDownloadDataCheck                                        =false; //JimmyChiu 20220117 : 超豐於Start lot將會下載FTP資料
    CosFunction.bTrayDeviceCheckFromLoader                                      =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bTrayMapFromLoader                                              =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bUseMDB                                                         =false; //Steven 20210526 : 部分客戶取消使用MDB  //JimmyChiu 20220302 : 關閉
    CosFunction.bIndexCheckCanTurnOff                                           =true;  //JimmyChiu 20220516 : do onecycle after index alarm
    CosFunction.bSaveArmSpeedByMachine                                          =true;  //JimmyChiu 20220618 : save by machine
    CosFunction.bSaveTemperatureByMachine                                       =true;  //JimmyChiu 20220618 : save by machine
    CosFunction.bSaveOffsetByMachine                                            =true;  //JimmyChiu 20220618 : save by machine
    CosFunction.bLowYieldDoOneCycle                                             =true;  //JimmyChiu 20230323 : Add run onecycle after low yield
    IniConfig.bIndexJamInArmAway                                                =true;  //JimmyChiu 20230323 : Add by 政賢要求
    CosFunction.bTemp5PointKitOffset                                            =true;  //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bDownloadUpdateAutomatically                                    =true;  //Sam 20220824 : FTP 自動下載安裝更新包
    CosFunction.bMaualRemoveTray3Alarm                                          =true;  //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
    IniConfig.bShowFTandRTButton                                                =true;  //Jimmychiu 20231214 : 政賢要求加入顯示FTRT
    CosFunction.bUseSCKART                                                      =true;  //Sam 20191113 : TCP ART
    CosFunction.bAutoRetestGPIBmode                                             =true;  //Sam 20191113 : TCP ART
    CosFunction.iAutoRetestTCPmode                                              =1;     //Sam 20191113 : TCP ART
    CosFunction.bEnableHandlerResultServer                                      =true;  //Jimmychiu 20240202 : Add for ART command
    CosFunction.bInspectSuckICWhenSoftStart                                     =false; //Jimmychiu 20240217 : 汶彥要求移除檢查
    CosFunction.bAutoCleanUseHPSetByRecipe                                      =true;  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    CosFunction.bCheckOutSuckICFallDownOnlySkip                                 =true;  //Jimmychiu 20240813 : Only Skip When Checking Out Suck IC Fall Down
    CosFunction.bAutoTeachOutShuttleLantch                                      =true;  //Jimmychiu 20240923 : Auto teach in/out shuttle site kit position
}
//------------------------------------------------------------------------------
void FUNC_CC_PTI()
{
    IniConfig.bShowFTandRTButton                                                =true;  //Steven 20131224 : Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    CosFunction.bRTCAlarmSetIndexToErrBin                                       =true;  //JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin
    IniConfig.bHeadChamberSocketMode                                            =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bShowLotInfo                                                      =true;  //RogerYang 20170327 add
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //RogerYang 20170327 add
    CosFunction.bFTPFunction                                                    =true;  //RogerYang 20170406 add
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Sam 20181018 : Add Auto Clean
    CosFunction.bOLPFunction                                                    =true;  //Sam 20190429 : Add CC_PTI_NEWWORK
    CosFunction.bAutoHeightSHTReleaseByFile                                     =true;  //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
    CosFunction.bYieldAlarmClearAllCount                                        =true;
    CosFunction.bSpecailLowYeild                                                =true;  //Sam 20210505 : PTI 要求的兩段 Low Yeild
    CosFunction.IntervalYieldCount                                              =true;  //Sam 20211221 : PTI 開啟 IntervalYieldCount
    CosFunction.bUseMDB                                                         =true;  //Steven 20210526 : 部分客戶取消使用MDB
    IniConfig.bRecordSkipPosition                                               =true;  //Sam 20220819 : PTI 開啟 AutoSkip
    CosFunction.bLowYieldUseContactCounts                                       =true;  //Sam 20221020 : LowYield 改使用 ContactCounts 的資料來計算
    CosFunction.bAfterOneCycleShowTrayQty                                       =true;  //Sam 20221101 : OneCycle 完成後顯示所有 Tray 各別的IC數量
    CosFunction.bUseFTPDownloadDataCheck                                        =false;
    CosFunction.bUseSocketContactCount                                          =true;
    #ifndef SOFT_SIMULTE
    CosFunction.bUseBarCoderChangeSetupFile                                     =true;  //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
    CosFunction.bUseBarCoderAutoLogin                                           =true;  //Sam 20221101 : 使用 BarCoder 自動登錄
    #endif
    CosFunction.bSpecialP24                                                     =true;  //Sam 20231129 : 力成也要矽格北興P24功能
    CosFunction.bCategoryInfoByContactCT                                        =true;  //Sam 20240131 : 新增一組 CategoryInfo 紀錄資料使用 ContactCT
    //CosFunction.bRTCAutoTuning                                                =true;
    CosFunction.bYieldAlarm4                                                    =true;
    CosFunction.bUseSCKART                                                      =true;  //Sam 20240809 : PTI ART 模式
    CosFunction.bAutoRetestGPIBmode                                             =true;
    CosFunction.bUseARTSortCount                                                =true;
    CosFunction.bSmartAutoClean                                                 =true;  //Sam 20230111 : Smart Auto Clean
    CosFunction.bAdaptiveYield                                                  =true;  //Sam 20230914 : 自適應性良率監控
    CosFunction.bShowYieldMonitor                                               =true;
    CosFunction.bFirstTrayCheckOnUnloader                                       =true;  //Jimmychiu 20251205 : First Tray Check On Unloader
    CosFunction.bEndLotAfterTrayFeed                                            =true;  //Jimmychiu 20250115 : Auto End Lot After Tray Feed
}
//------------------------------------------------------------------------------
void FUNC_CC_THAILIN()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_SJ_Semiconductor()                                                         //盛合晶微
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    IniConfig.bShowFTandRTButton                                                =true;  //Steven 20131224 : Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按

    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM       //wei 20170209 add
    CosFunction.bTTLCanUse8Site                                                 =true;  //Alick 20161011 (Steven) : TTL支援8Site
    CosFunction.bFTPFunction                                                    =true;  //Frank 20170216 新增
    IniConfig.bFTBin2RTBin                                                      =true;  //Frank 20170216 新增
    CosFunction.bFTBin2RTBinNoYield                                             =true;  //jou 20170925 (wei) : FT/RT Bin設定不變,yield control可設定.
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Frank 20170228 新增
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20170214 (wei) add Auto Clean Function
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20170302 (wei) : FIFO MODE
    CosFunction.bSiteCmpYield                                                   =true;  //Steven 20170308 add to JCET
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min    //Steven 20170308 add to JCET
    CosFunction.bLowYieldDoOneCycle                                             =true;  //Steven 20170308 (wei) : 良率相關的都做One Cycle
//    CosFunction.bUseLowYieldAlarmByBin                                        =true;  //Steven 20140828 : By Bin Yield Monitor      //JerryYang (Steven) 20170421 凌中心要求取消

    CosFunction.bOPCanPressStepAndTStart                                        =true;  //JerryYang 20170417 (Steven) JCET吳如春要求OP權限也能按Step跟T.Start
    CosFunction.bYieldAlarmUseDouble                                            =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bAutoFullDoNotStopImmediately                                   =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bUseDynamicKitDiameter                                          =true;  //Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bByBinAlarmFromYieldForm                                        =true;  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20170818 (Steven) add JCET 新增海思全流程 Log
    if(CosFunction.bHiSiliconFunction)
    {
        CosFunction.bUseLotIDWithoutSECS                                        =true;  //Ifor 20170915 (Steven) add JCET 使用CosFunction.bUseLotIDWithoutSECS
        CosFunction.bFTPDataTrayFeedAutoUpdata                                  =true;  //Ifor 20181214 : add Tray Feed 後自動上傳海思檔案至FTP
        CosFunction.bUseFTPDownloadDataCheck                                    =true;  //JerryYang 20180329 吳如春要求 Hisi版再做check   //Ifor 20180125 : Use FTP Download Data Check
    }
    else
    {
        CosFunction.bUseFTPDownloadDataCheck                                    =false; //Isaac 20211008 : 本來是全客戶開啟，但吳如春指定不要開啟這檢查
    }
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;  //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20170915 (Steven) 避免LotInf 被關閉
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bLowYeildByTotal                                                =true;  //Ifor 20171117 : add 長電 Low Yeild By Total
    IniConfig.b2x2Use8Suck                                                      =true;  //Ifor 20171117 : add 長電 2x2使用8吸嘴
    IniConfig.b1x2Use4Suck                                                      =true;  //Hung 20110812 : 1x2使用4吸嘴
    IniConfig.b1x4Use8Suck                                                      =true;  //Hung 20110812 : 1x4使用8吸嘴
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20171123 : add FTP DownLoad Site By SetupFile
//    CosFunction.bAutoSiteMappingUseFailBinSetting                             =true;  //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
    CosFunction.bUsePassBinOnlyCanSetOneBin                                     =true;  //Ifor 20171213 (Steven) : Auto Tray 設定Pass時僅可設定一個Bin
    IniConfig.bLowYieldAlarmSameNS                                              =true;
//    CosFunction.bHisiLogUploadNetwork                                         =true;
//    CosFunction.bSLTReportFunction                                            =true;  //Steven 20180420 : SLT報表功能
    CosFunction.bUseYieldControlFunction                                        =true;  //Ifor 20180731 (wei) : add Use Yield Control Function
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
//    CosFunction.bEnableTimeOutCanSkip                                         =true;
    CosFunction.bUseLocalRecipeOffset                                           =true;  //Ifor 20181211 : add Use Local Recipe Offset
    IniConfig.bInitialStartDelayCount                                           =true;
    CosFunction.bSECS_GEM_OneCycle                                              =true;
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =true;  //JerryYang 20190919 :　黃剛要求index arm pick up error強制開門
    CosFunction.bAfterAutoCleanNoIndexCheck                                     =true;  //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check'
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;  //Steven 20191224 : Unloader編輯Tray改用另外一組權限
    CosFunction.bForecePerPinKGf                                                =true;  //Isaac 20210514 : 嚴海軍要求只留N，gf隱藏true->false       //Steven 20210726 : 凌中心說要Mark
    CosFunction.bStopMustTestTimeOut                                            =true;
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20200507 : 吳如春說要用下拉選帳號
    IniConfig.bHeadChamberSocketMode                                            =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bECChangeEventReportCanOnOff                                    =true;
    CosFunction.bAutoTrayFullAlarmCanNotSkip                                    =true;  //Steven 20210219 : Unloader滿盤alarm不能Skip
    CosFunction.bI21EnableASMByRecipe                                           =true;  //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bUseSocketContactCount                                          =true;
    CosFunction.bUseHeadContactCount                                            =true;
    CosFunction.bRCMDStart                                                      =true;
    CosFunction.bKeepOnly1SetupFile                                             =true;
    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bFTPUseBarcodeReader                                            =true;  //Ifor 20210413 add:FTP Use Barcode Reader
    CosFunction.bSortingBy2DList                                                =true;
    CosFunction.bUploadLogByTypeFolder                                          =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bRunModeFollowLotInfo                                           =true;  //Steven 20250603 : 根據Lot Info的Run mode進行切換
    CosFunction.PassworDownloadByFTP                                            =true;
    CosFunction.bSaveEventLogByLotID                                            =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SJ_Semiconductor_OS()                                                      //盛合晶微Open Short
{
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM       //wei 20170209 add
    CosFunction.bOPCanPressStepAndTStart                                        =true;  //JerryYang 20170417 (Steven) JCET吳如春要求OP權限也能按Step跟T.Start
    CosFunction.bYieldAlarmUseDouble                                            =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bAutoFullDoNotStopImmediately                                   =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bStopMustTestTimeOut                                            =true;
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;
    CosFunction.bI21EnableASMByRecipe                                           =true;  //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    CosFunction.bFTPFunction                                                    =true;  //Frank 20170216 新增
    IniConfig.bFTBin2RTBin                                                      =true;  //Frank 20170216 新增
    CosFunction.bFTBin2RTBinNoYield                                             =true;  //jou 20170925 (wei) : FT/RT Bin設定不變,yield control可設定.
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bUseDynamicKitDiameter                                          =true;  //Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bUseSocketContactCount                                          =true;
    CosFunction.bRCMDStart                                                      =true;
//    CosFunction.bLoginShowUserName                                            =true;  //Steven 20230317 : 登入時顯示帳號名稱
    CosFunction.bKeepOnly1SetupFile                                             =true;
    CosFunction.bOffLineBin                                                     =true;
    CosFunction.bFTPUseBarcodeReader                                            =true;  //Ifor 20210413 add:FTP Use Barcode Reader
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_JCET()
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    IniConfig.bShowFTandRTButton                                                =true;  //Steven 20131224 : Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bLoseDeviceOutShuttleServoOff                                   =true;  //ChungHung 20140522 add OutShuttle lose devices can servo off  //先不開放有問題
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM       //wei 20170209 add
    CosFunction.bTTLCanUse8Site                                                 =true;  //Alick 20161011 (Steven) : TTL支援8Site
    CosFunction.bFTPFunction                                                    =true;  //Frank 20170216 新增
    IniConfig.bFTBin2RTBin                                                      =true;  //Frank 20170216 新增
    CosFunction.bFTBin2RTBinNoYield                                             =true;  //jou 20170925 (wei) : FT/RT Bin設定不變,yield control可設定.
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Frank 20170228 新增
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20170214 (wei) add Auto Clean Function
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20170302 (wei) : FIFO MODE
    CosFunction.bSiteCmpYield                                                   =true;  //Steven 20170308 add to JCET
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min    //Steven 20170308 add to JCET
    CosFunction.bLowYieldDoOneCycle                                             =true;  //Steven 20170308 (wei) : 良率相關的都做One Cycle
//    CosFunction.bUseLowYieldAlarmByBin                                        =true;  //Steven 20140828 : By Bin Yield Monitor      //JerryYang (Steven) 20170421 凌中心要求取消
    CosFunction.bOPCanPressStepAndTStart                                        =true;  //JerryYang 20170417 (Steven) JCET吳如春要求OP權限也能按Step跟T.Start
    CosFunction.bYieldAlarmUseDouble                                            =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bAutoFullDoNotStopImmediately                                   =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
    CosFunction.bUseDynamicKitDiameter                                          =true;  //Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bByBinAlarmFromYieldForm                                        =true;  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20170818 (Steven) add JCET 新增海思全流程 Log
    if(CosFunction.bHiSiliconFunction)
    {
        CosFunction.bUseLotIDWithoutSECS                                        =true;  //Ifor 20170915 (Steven) add JCET 使用CosFunction.bUseLotIDWithoutSECS
        CosFunction.bFTPDataTrayFeedAutoUpdata                                  =true;  //Ifor 20181214 : add Tray Feed 後自動上傳海思檔案至FTP
        CosFunction.bUseFTPDownloadDataCheck                                    =true;  //JerryYang 20180329 吳如春要求 Hisi版再做check   //Ifor 20180125 : Use FTP Download Data Check
    }
    else
    {
        CosFunction.bUseFTPDownloadDataCheck                                    =false; //Isaac 20211008 : 本來是全客戶開啟，但吳如春指定不要開啟這檢查
    }
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;  //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
    IniConfig.bShowLotInfo                                                      =true;  //Ifor 20170915 (Steven) 避免LotInf 被關閉
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bLowYeildByTotal                                                =true;  //Ifor 20171117 : add 長電 Low Yeild By Total
    IniConfig.b2x2Use8Suck                                                      =true;  //Ifor 20171117 : add 長電 2x2使用8吸嘴
    IniConfig.b1x2Use4Suck                                                      =true;  //Hung 20110812 : 1x2使用4吸嘴
    IniConfig.b1x4Use8Suck                                                      =true;  //Hung 20110812 : 1x4使用8吸嘴
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20171123 : add FTP DownLoad Site By SetupFile
//    CosFunction.bAutoSiteMappingUseFailBinSetting                             =true;  //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
    CosFunction.bUsePassBinOnlyCanSetOneBin                                     =true;  //Ifor 20171213 (Steven) : Auto Tray 設定Pass時僅可設定一個Bin
    IniConfig.bLowYieldAlarmSameNS                                              =true;
//    CosFunction.bHisiLogUploadNetwork                                         =true;
//    CosFunction.bSLTReportFunction                                            =true;  //Steven 20180420 : SLT報表功能
    CosFunction.bUseYieldControlFunction                                        =true;  //Ifor 20180731 (wei) : add Use Yield Control Function
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
//    CosFunction.bEnableTimeOutCanSkip                                         =true;
    CosFunction.bUseLocalRecipeOffset                                           =true;  //Ifor 20181211 : add Use Local Recipe Offset
    IniConfig.bInitialStartDelayCount                                           =true;
    CosFunction.bSECS_GEM_OneCycle                                              =true;
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =true;  //JerryYang 20190919 :　黃剛要求index arm pick up error強制開門
    CosFunction.bAfterAutoCleanNoIndexCheck                                     =true;  //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check'
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;  //Steven 20191224 : Unloader編輯Tray改用另外一組權限
    CosFunction.bForecePerPinKGf                                                =true;  //Isaac 20210514 : 嚴海軍要求只留N，gf隱藏true->false       //Steven 20210726 : 凌中心說要Mark
    CosFunction.bStopMustTestTimeOut                                            =true;
    #ifdef FOR_EVAN                                                                     //Steven 20210304 : 長電給如春的版本
    JCET_FOR_EVAN           =1;
    #endif
    if(JCET_FOR_EVAN==1)
    {
        CosFunction.PassworDownloadByFTP                                        =true;  //Steven 20221216 : 吳如春要改成從網路抓密碼本
    }
    CosFunction.bUseLoginDatToSetLevel                                          =true;  //Steven 20200507 : 凌中心說要把帳號密碼加密
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20200507 : 吳如春說要用下拉選帳號
    IniConfig.bHeadChamberSocketMode                                            =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bECChangeEventReportCanOnOff                                    =true;
    CosFunction.bAutoTrayFullAlarmCanNotSkip                                    =true;  //Steven 20210219 : Unloader滿盤alarm不能Skip
    CosFunction.bI21EnableASMByRecipe                                           =true;  //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bF18F22InshuttleDetectSaveByRecipe                              =true;  //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
    IniConfig.bPowerSaveFunction                                                =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;  //Richard 20230424 : add jcet cbDisableSiteMappingCheck->Visible
    CosFunction.bKeepOnly1SetupFile                                             =true;
//    CosFunction.bUse32ChanelSiteMap                                           =true;
//    CosFunction.bRead2DIDFromServer                                             =true; //RogerYang 20251211 JCET沒用到 //Jimmychiu 20230925 : read 2did in json file
//    CosFunction.bFixNameOfForcePerPinG                                        =true;  //Steven 20240821 : 凌中心說要修正存檔名稱   //Steven 20240822 : 凌中心又說不要了
    CosFunction.bUploadLogByTypeFolder                                          =true;  //Steven 20250217 : 根據Log內容作資料夾分類
    CosFunction.bRunModeFollowLotInfo                                           =true;  //Steven 20250603 : 根據Lot Info的Run mode進行切換
    CosFunction.bMakeWhite2DIDList                                              =true;  //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    CosFunction.bEnableDual_1x4Kit                                              =true;  //RogerYang 20251211 : add
}
//------------------------------------------------------------------------------
void FUNC_CC_JSI_HAOXING()
{
    FUNC_CC_JCET();
    CosFunction.bUseSocketContactCount                                          =true;  //Sam 20220720 : 新增一組 Socket Count
    CosFunction.bContactCountSavedByRecipe                                      =true;  //Jimmychiu 20240826 : Contact count saved by recipe
}
//------------------------------------------------------------------------------
void FUNC_CC_OSE()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //kevin 20180525 20180309
}
//------------------------------------------------------------------------------
void FUNC_CC_MTI()
{
    IniConfig.bShowFTandRTButton                                                =true;  //Steven 20131224 : Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
    CosFunction.bOLPFunction                                                    =true;  //Steven 20141229 : OLP功能
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
    IniConfig.bShowLotInfo                                                      =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_NUVOTON()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_UPRTEK()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_Eutrend()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_TICP()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20170214 (wei) add Auto Clean Function
}
//------------------------------------------------------------------------------
void FUNC_CC_THEIL()
{
    IniConfig.bIndexArm2SupplyLight                                             =true;  //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
}
//------------------------------------------------------------------------------
void FUNC_CC_TERAPOWER()
{
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    IniConfig.bEnableCCDUSETCPIP                                                =true;  //wei 20170310 (steven) 開啟CCD功能
    CosFunction.bColorSensorUseCCLink                                           =true;  //wei 20180409 color sensor
    CosFunction.bEnableHandlerResultServer                                      =true;  //Isaac 20170613 (wei) TCP/IP
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //RogerYang 20170327 (Steven) 開放LotID但又不使用SECS/GEM  //Isaac 20170613 (wei) TCP/IP
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按 //Isaac 20170613 (wei) TCP/IP
    CosFunction.bRotateNoUseFix1                                                =true;  //Frank 20170531 (Steven) add Open Rotate不能選Fix1
    CosFunction.bYieldAlarm4                                                    =true;  //Sam 20181025 開啟 Alarm4
    CosFunction.bGPIBLotEnd                                                     =true;  //Sam 20181030 : 透過 GPIB 送 LotEnd 給測試機。
    CosFunction.bIncludeMTBA                                                    =true;  //Sam 20181031 : 開啟
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Sam 20181109 : Add cleanout 切換 Initial
    IniConfig.bShowLotInfo                                                      =true;  //Sam 20181112 : 避免LotInf 被關閉
    CosFunction.bSiteStopVisblieSpecialColor                                    =true;  //Sam 20181112 : 開啟 Site 又將 Site 停止時以特別顏色顯示。
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bUseSCKART                                                      =true;  //Sam 20191113 : TCP ART
    CosFunction.bAutoRetestGPIBmode                                             =true;  //Sam 20191113 : TCP ART
    CosFunction.iAutoRetestTCPmode                                              =1;     //Sam 20191113 : TCP ART
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Sam 20200215 : 開啟 AutoClean
    CosFunction.bIndexCycleTimeRecord                                           =true;  //Sam 20200916 : Add Index Cycle Time Record
    CosFunction.bUseAfterHomeShtChkLoseICNeedSlow                               =true;  //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
    CosFunction.LoaderFloating                                                  =true;
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;
    CosFunction.bFullTrayAlarmAfterUnloadEnd                                    =true;
    CosFunction.PassworDownloadByFTP                                            =true;  //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
    IniConfig.bHeadChamberSocketMode                                            =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bShowYieldMonitor                                               =true;  //Sam 20210916 : 新增 Yiled Monitor 到畫面上
    CosFunction.bUseDefineAutoCleanOffset                                       =true;  //Sam 20220317 : TPW Auto Clean Ofs 也要By本機。
    CosFunction.bUse2X4_4AutoCleanFixedPosition                                 =true;  //Sam 20220423 : 新增 Auto Clean 2X4_4 模式取放位置固定
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //Ifor 20180322 : add Use One By One Index Check
    CosFunction.bAutoCleanOffsetUseSingleSetting                                =true;  //Sam 20220720 : AutoClean Offset 僅使用 Clean 設定 Offset 來補償
    CosFunction.bUseSocketContactCount                                          =true;  //Sam 20220720 : 新增一組 Socket Count
    //CosFunction.RecordIndexAirOnTime                                          =true;  //Sam 20220419 : 先 Mark 有需要再打開  //Sam 20220329 : Record Index Air On Time
    CosFunction.bUseMDB                                                         =true;  //Steven 20210526 : 部分客戶取消使用MDB
    CosFunction.bLowYieldUseContactCounts                                       =true;  //Sam 20221020 : LowYield 改使用 ContactCounts 的資料來計算
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //Sam 20221020 : 晶兆成建勳也要 OneCycle 後可以編輯
    CosFunction.bSmartAutoClean                                                 =true;  //Sam 20230111 : Smart Auto Clean
    CosFunction.bAdaptiveYield                                                  =true;  //Sam 20230914 : 自適應性良率監控
    CosFunction.bLimitSensorOnNeedManualMove                                    =true;  //Sam 20230522 : InOutArm Server Off 後，重新 Server On 時檢查 CCW CW 若有 On 時需要手動排除異常。
    CosFunction.bART_RT_NoRotate                                                =true;  //Sam 20240809 : ART RT No Rotate
    CosFunction.bResetModeIncludeCleanOut                                       =true;
    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bCylinderOnOffTimeLog                                           =true;  //Eastsun 20260522 整合: Pre Alrm Cylinder
}
//------------------------------------------------------------------------------
void FUNC_CC_TeraProbe()                                                        //RogerYang 20260223 Add for TeraProbe (copy from TERAPOWER)
{
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bLastSetInSetUpFile                                             =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    CosFunction.bColorSensorUseCCLink                                           =true;
    CosFunction.bEnableHandlerResultServer                                      =true;
    CosFunction.bUseLotIDWithoutSECS                                            =true;
    IniConfig.bShowFTandRTButtonCanClick                                        =true;
    CosFunction.bRotateNoUseFix1                                                =true;
    CosFunction.bYieldAlarm4                                                    =true;
    CosFunction.bGPIBLotEnd                                                     =true;
    CosFunction.bIncludeMTBA                                                    =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bSiteStopVisblieSpecialColor                                    =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;
    CosFunction.bUseSCKART                                                      =true;
    CosFunction.bAutoRetestGPIBmode                                             =true;
    CosFunction.iAutoRetestTCPmode                                              =1;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bIndexCycleTimeRecord                                           =true;
    CosFunction.bUseAfterHomeShtChkLoseICNeedSlow                               =true;
    CosFunction.LoaderFloating                                                  =true;
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;
    CosFunction.bFullTrayAlarmAfterUnloadEnd                                    =true;
    CosFunction.PassworDownloadByFTP                                            =true;
    IniConfig.bHeadChamberSocketMode                                            =true;
    IniConfig.bHeadSocketMode                                                   =true;
    CosFunction.bShowYieldMonitor                                               =true;
    CosFunction.bUseDefineAutoCleanOffset                                       =true;
    CosFunction.bUse2X4_4AutoCleanFixedPosition                                 =true;
    CosFunction.bUseOneByOneIndexCheck                                          =true;
    CosFunction.bAutoCleanOffsetUseSingleSetting                                =true;
    CosFunction.bUseSocketContactCount                                          =true;
    //CosFunction.RecordIndexAirOnTime                                          =true;
    CosFunction.bUseMDB                                                         =true;
    CosFunction.bLowYieldUseContactCounts                                       =true;
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;
    CosFunction.bSmartAutoClean                                                 =true;
    CosFunction.bAdaptiveYield                                                  =true;
    CosFunction.bLimitSensorOnNeedManualMove                                    =true;
    CosFunction.bART_RT_NoRotate                                                =true;
    CosFunction.bResetModeIncludeCleanOut                                       =true;
    IniConfig.bRecordSkipPosition                                               =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_GIGAS()                                                                    //Richard 20220905 Add FUNC_GIGAS
{
    //yield
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    //Auto clean
    IniConfig.bEnableAutoCleanFunction                                          =true;
//    CosFunction.bAutoCleanShuttleDisable                                        =true;  //jou 2013-02-27 Auto Clean disable shuttle sensor detect     //Jimmychiu 20250114 : 工程師Ben要求重啟功能

    //FTP
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bShowLotInfo                                                      =true;  //Sam 20181112 : 避免LotInf 被關閉   //Isaac 20210129 : FTP頁面，關閉功能要不顯示畫面，但視窗不要關掉
    CosFunction.bFTPDownloadAlwaysCover                                         =true;  //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主  //Isaac 20200720 : 下載後覆蓋當前的資料

    //Bin
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin

    //Others
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //jou 2013-09-25 Testing Need Stop All Motor
    IniConfig.iTempeAlarmSecond_Over                                            =5;     //Steven 20111027 : 溫度過高的Alarm時間
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false;
    CosFunction.bUseARTSortCount                                                =false; //Ifor 20170315 add 新增使用ART Sort Count 計數功能   //20170417 功能延至下ㄧ版本
    CosFunction.bHaveFIFOMode                                                   =true;  //Ifor 20181001: Add KYEC 使用FIFO功能
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;  //Steven 20180627 (wei) : Time out預設只顯示SKIP
    CosFunction.bEnableDual_1x4Kit                                              =true;  //Isaac 20200311 : 1x2Site使用1x4Site Kit
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bTrayAssignUseGraphic                                             =true;  //Steven 20111121 : 使用圖片去顯示Tray Assign
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //Isaac 20200602 : 客戶要求
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start //Isaac 20200727 : add
    CosFunction.bUse32ChanelSiteMap                                             =true;  //Steven 20170530 (wei) : Use 32CH site map //Isaac 20200803 : 客戶要求開放
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護 //Isaac : 開啟保護
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣   //Isaac 20220120 : Open function
    CosFunction.bTempCalByRecipe                                                =true;  //jou 20220725 : Temperature calibration by recipe
    CosFunction.bFillTheTrayAfterOutArmPlace                                    =true;  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    CosFunction.bUseFTPDownloadDataCheck                                        =false; //Jimmychiu 20250701 : Ben requested that the FTP data check be closed.
    CosFunction.bSocketSensorCheckICAtArmDown                                   =true;  //Jimmychiu 20250826 : 每次下壓確認有IC在socket
    CosFunction.bOffsetTempByRecipeMinMaxLimit                                  =true;  //StevenHong 20260119 : Add Offset By Recipe Max Limit
    CosFunction.bCleanCountAlarmByMin                                           =true;  //Jimmychiu 20260212 : Gigas Clear alarms based on minimum usage count
    //AI(ht9045-v899) 20260421: 全智反饋 Auto Contact Height Calibration / Contact Test 過程若 IC drop 無偵測會壓壞 IC 與配件，啟用 Index Z 取料/下壓掉料保護
    CosFunction.bContactTestICDropGuard                                         =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_DoosanTesna()
{
    IniConfig.bKoreaFunction                                                    =true;
    IniConfig.bShowFunctionWindow                                               =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_AMKOR_Korea()
{
    IniConfig.bKoreaFunction                                                    =true;
    IniConfig.bOnlyRoomOrHot                                                    =true;
    IniConfig.bQAMode                                                           =true;
    IniConfig.bTrayAssignUseGraphic                                             =true;  //Steven 20111121 : 使用圖片去顯示Tray Assign
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bRetryNoNeedRestartGpib                                           =true;  //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bShuttleMode50                                                    =true;  //Steven 20120417 : RTC開單Arm Hang Up
    IniConfig.bFTTrayAss2RTTrayAss                                              =true;  //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    IniConfig.bRemeberAutoHeight                                                =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =false; //ChungHung 20120911 add 與OneCycle can desable site 衝突
    CosFunction.bOneCycleCanChangeArm                                           =true;  //Hung 20111108 Amb/Hot Mode OneCycle Can Change Arm
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bShowTrayAndDeviceDir                                             =true;  //jou 2013-03-25 show Tray & Device Direction
    CosFunction.bShowLastContactHigh                                            =true;  //Steven 20140409 : 顯示之前的Contact高度
    CosFunction.bShowLastShuttleHigh                                            =true;  //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    CosFunction.bEnableAfterTestedDelay                                         =true;  //ChungHung 20140730 add for ATK function after tested delay time
    CosFunction.bEnable2x1Site                                                  =true;  //Steven 20120814: 支援2x1Site模式
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150415 : ALTERA在ATK要求要開啟該功能

    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    CosFunction.bAutoSetContFailAfterInitialStart                               =true;  //ChungHung 20150519 add Auto Set ContFail After InitialStart.
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    CosFunction.bUseERMS                                                        =true;  //Steven 20160711 : 使用進階版RMS
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 (wei) : For SCK 93K ART
    CosFunction.bFTPFunction                                                    =true;

    if(fSCKART!=NULL && fSCKART->iTesterType==1)                                        //Steven 20170309 (wei) : Fixed for ART
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    CosFunction.bAmkorFunction                                                  =true;  //Steven 20171017 (wei) : Functions for Amkor Group
    CosFunction.bBySiteByBinPercentCompare                                      =true;
    CosFunction.bUseChamberBoostMode                                            =true;  //Steven 20191128 : Chamber Boost Function
    CosFunction.bShakeShuttleEveryTime                                          =true;  //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
    IniConfig.bAutoTrayLink                                                     =true;  //jou 2012-06-14 Auto Tray Link
    CosFunction.bAfterAutoCleanNoIndexCheck                                     =true;
    CosFunction.bUse32ChanelSiteMap                                             =true;
    CosFunction.bQAModeFinishOfflineShowOnLinePicture                           =true;  //Sam 20200804 : QA 模式完成後切回 OffLine 時，強制顯示 OnLine 圖片。
    CosFunction.bEnableMulti2D                                                  =true;  //Steven 20200810 : 一個IC使用多個2DID
    CosFunction.bTraySortCntFunc                                                =true;  //Steven 20210608 : Tray指定數量退盤功能

    CosFunction.bSortingBy2DList                                                =true;  //Frank 20221122 : 2DID sorting for ATK
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bLotIDVerify                                                    =true;  //Steven 20240704 : Lot Verification function for ATK
    CosFunction.bUseTSVFunction                                                 =true;  //Steven 20240904 : for ATK的TSV功能
    CosFunction.bAutoTmpeOfsByFTP                                               =true;
    CosFunction.bRead2DIDFromServer                                             =true;  //Jimmychiu 20230925 : read 2did in json file
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
}
//------------------------------------------------------------------------------
void FUNC_CC_AMKOR_China()
{
    AnsiString sPath=AuthPath+"config.ini";

    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.bOneCycleCanTrayFeed                                              =true;
    IniConfig.bHighModeCanOffTemp                                               =true;  //Steven 20110524
    IniConfig.bOneCycleDoQuickCleanOut                                          =true;  //Steven 20110524 : OneCycle後,只做快速CleanOut
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526   //Alick 20161019改True
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;  //skip in/out arm drop error
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
//    IniConfig.bLowYieldAlarmSameNS                                            =true;  //與 bLowYieldAlarmByBin 衝突
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bFix3PutAllFullIC                                                 =true;  //ChungHung 20111215
    IniConfig.bShuttleModeAccseeLevel                                           =true;  //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bDownLoadAutoCountClear                                           =true;  //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    if(REAL_TIME_CCD==true)
        IniConfig.bShuttleMode50                                                =true;  //暫時先打開，等RTC關Arm功能改好
    else
        IniConfig.bShuttleMode50                                                =false; //暫時先打開，等RTC關Arm功能改好
//    IniConfig.bEventLogAutoSaveFunction                                       =true;  //jou 20170214 (Steven) 改用evenlogsave小程式
//#ifndef SOFT_SIMULTE
//    IniConfig.bResetPortCOM1                                                    =true;  //jou 2012-10-24 Reset Port COM1
//#endif
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPasswordSecret                                                   =true;  //jou 2013-01-04 Password Txt 加密
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bShowTrayAndDeviceDir                                             =true;  //jou 2013-03-25 show Tray & Device Direction
    IniConfig.bOnlyRoomOrHot                                                    =true;
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20140715 Add
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bQAMode                                                           =CheckAndReadIniData(sPath, "QA Mode", "Enable QA Mode", false);     //jou 2015-09-11 客戶要求disable QA mode
    CosFunction.bQAmodeSupplyTrayDir                                            =true;  //jou 2014-10-24 QA mode 增加支援吸Tray方向
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //jou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    CosFunction.bRecipeCheck                                                    =true;  //JerryYang 20151028 add for Amkor 如果download的工作檔不一樣,要求做Height Calibration後才能start
    CosFunction.bDownloadRecipeLevelMode                                        =true;  //jou 2016-01-06 download recipe 增加權限模式選擇
    CosFunction.bLockF19AlwaysOn                                                =true;  //JerryYang 20160224 永遠啟用PiggyBack且不能改
    CosFunction.bLastSetInSetUpFile                                             =false; //JerryYang 20160513 京元先關閉此功能

    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bRTCalarmUnload                                                 =true;  //jou 20170210 (Steven) : RTC alarm image unload
//    IniConfig.bFTBin2RTBin                                                    =true;  //JerryYang 20170413 (Steven) 與Amkor孔維瓏確認，關閉此功能避免造成server工作檔必須重新上傳    //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣 //jou 20170213 (Steven) 會議討論增加RT BIN設定 for Sharkls

    CosFunction.bUseSCKART                                                      =true;  //Steven 20170512 (Wei) : Amkor China add ART function
    if(fSCKART!=NULL && fSCKART->iTesterType==1)
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    CosFunction.bAmkorFunction                                                  =true;  //Steven 20171017 (wei) : Functions for Amkor Group
    IniConfig.bPowerSaveFunction                                                =true;  //JerryYang 20180212 (Steven) add power save function
    CosFunction.bYieldAlarmUseDouble                                            =true;
//    CosFunction.bYieldAlmNeedOneCycle                                         =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bTraySortCntFunc                                                =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bUseTSVFunction                                                 =true;  //Steven 20240904 : for ATK的TSV功能
}
//------------------------------------------------------------------------------
void FUNC_CC_AMKOR_Japan()
{
    IniConfig.iUserLanguage                                                     =eulEnglish;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAmkorFunction                                                  =true;  //Steven 20171017 (wei) : Functions for Amkor Group
    CosFunction.bUseSCKART                                                      =true;  //Steven 20171017 : Amkor Phi add ART function
    if(fSCKART!=NULL && fSCKART->iTesterType==1)
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20200103 : 開啟Auto Clean功能
    CosFunction.bART_RT_NoRotate                                                =true;  //Sam 20240809 : ART RT No Rotate
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Sam 20250326 : 開啟 AutoSiteOff
    CosFunction.bSiteCmpYield                                                   =true;  //Sam 20250401 : 開啟 SiteCmpYield
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Sam 20250401 : 開啟 SiteCmpYield
    CosFunction.bUseRenesasServer                                               =true;  //RogerYang 20250812 : 瑞薩FT-CT
    CosFunction.iAutoRetestTCPmode                                              =2;     //RogerYang 20250812 : 瑞薩FT-CT
}
//------------------------------------------------------------------------------
void FUNC_CC_AMKOR_Philippines()
{
    IniConfig.iUserLanguage                                                     =eulEnglish;
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bOnlyRoomOrHot                                                    =true;
    IniConfig.bTrayAssignUseGraphic                                             =true;  //Steven 20111121 : 使用圖片去顯示Tray Assign
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bRetryNoNeedRestartGpib                                           =true;  //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bShuttleMode50                                                    =true;  //Steven 20120417 : RTC開單Arm Hang Up
    IniConfig.bFTTrayAss2RTTrayAss                                              =true;  //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    IniConfig.bRemeberAutoHeight                                                =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =false; //ChungHung 20120911 add 與OneCycle can desable site 衝突
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bShowTrayAndDeviceDir                                             =true;  //jou 2013-03-25 show Tray & Device Direction
    CosFunction.bShowLastContactHigh                                            =true;  //Steven 20140409 : 顯示之前的Contact高度
    IniConfig.bResetCanServoOff                                                 =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.iTempeAlarmSecond_Over                                            =4;     //Steven 20111027 : 溫度過高的Alarm時間  //Steven 20111215 2-->4
    IniConfig.iTempeAlarmSecond_Below                                           =2;     //Steven 20111027 : 溫度過低的Alarm時間
    IniConfig.bShowFunctionWindow                                               =true;
//    IniConfig.bCleanOutCanTrayEnd                                             =true;  //JerryYang 20161207 Garry說不要顯示Tray end
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =true;  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!  //Isaac 20170607 (wei) ATP要求開放UpDownTray功能
    IniConfig.bSiteMappingFastSetDisable                                        =true;
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    CosFunction.bStopMustTestTimeOut                                            =true;
    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bChamberModeEvenBlowNeedWaitTime                                =true;  //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
    IniConfig.bTemp25degControl                                                 =true;  //jou 2014-06-07 Temperature 25 deg. control
    IniConfig.bQAMode                                                           =true;
//    CosFunction.bSecurityHave5Level                                           =true;  //jou 2014-06-19 Security Have 5 Level
    CosFunction.bEnableOctal_16Kit                                              =true;  //Steven 20140312 : 8Site使用16Site Kit
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //Steven 20141105 : Index內的所有異常都只能用Skip  //JerryYang 20161007 Erick要求不能重送測式訊號
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bLockD41ByFile                                                  =true;
    CosFunction.bLockD42ByFile                                                  =true;  //Jerryyang 20160220 Lock D42 by file
    CosFunction.bLockD44ByFile                                                  =true;  //Jerryyang 20160220 Lock D44 by file
    IniConfig.bRTCbySystem                                                      =true;  //JerryYang 20160220 RTC by System
    CosFunction.bLockRTC                                                        =true;  //JerryYang 20160220 避免人員關閉RTC功能
    CosFunction.bPickupErrorAtLoaderNeedOpenDoor                                =true;  //JerryYang 20160220 add for Amkor-Philippine,In ARM至Loader吸取異常要開門確認後才能繼續
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //JerryYang 20160224
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //JerryYang 20160224
    IniConfig.bAlarmMustRedColor                                                =true;  //JerryYang 20160224 : 特殊Alarm需要改紅底
    CosFunction.bLockF19AlwaysOn                                                =true;  ///JerryYang 20160224 永遠啟用PiggyBack且不能改
    CosFunction.bOutShuttleLoseICNeedHome                                       =true;  ///JerryYang 20160225
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bPurgeBeforePickShuttle                                         =true;  //JerryYang 20160906 PurgeBeforePickShuttle
    CosFunction.bShowLastShuttleHigh                                            =true;  //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    CosFunction.bEnable2x1Site                                                  =true;
    CosFunction.bInOutArmUseBackRowSuck                                         =true;

    CosFunction.bUseSCKART                                                      =true;  //Steven 20171017 : Amkor Phi add ART function
    if(fSCKART!=NULL && fSCKART->iTesterType==1)
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    CosFunction.bAmkorFunction                                                  =true;  //Steven 20171017 (wei) : Functions for Amkor Group
    CosFunction.bRTC_ROICount                                                   =true;  //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bStatisticsJamCount                                             =true;  //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
//    CosFunction.bIndexPickErrNeedPiggyBack                                    =true;  //jou 20171211 (Steven) : After index pick up error, device go to Error bin ,follow by Index check or piggy back check
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bIndexPickErrSkipNeedCheckVac                                   =true;  //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
    CosFunction.bIndexPickErrSkipBlowAirTime                                    =true;  //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
//    CosFunction.bOutShtLoseICSetErrUntilOneCycle                              =true;  //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin

    CosFunction.bInOutArmZCalibration                                           =true;  //Frank 20171213 (Steven) : In/OutArmZCalibration
    CosFunction.bFTPFunction                                                    =true;  //JerryYang 20181011 ATP啟用FTP
    CosFunction.bDLLCommands                                                    =true;  //JerryYang 20181126 (Steven) : support Epson DLL function
    CosFunction.bYPitchNotUseSearchLastMode                                     =true;  //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
    CosFunction.bAutoTrayFullAlarmCanNotSkip                                    =true;  //Steven 20210219 : Unloader滿盤alarm不能Skip
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bRecipeParameterDefault                                         =true;  //Isaac 20170527 (Steven) defalut值比較功能
    IniConfig.bAutoTrayLink                                                     =true;  //jou 2012-06-14 Auto Tray Link
    CosFunction.bLotStartLockCriticalPara                                       =true;  //JerryYang 20220311 : ATP鎖定Critical parameter
    CosFunction.bUseLowYieldAlarmByBin                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_CARSEM_M()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_UNISEM_M()
{
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    CosFunction.bUseAuto1OnlyBin1                                               =true;  //Ifor 20171018 (wei) : Only Bin 1 is allowed to set to Auto 1
    IniConfig.bHeadSocketMode                                                   =true;  //Ifor 20171018 (wei) : UNISEM Add Head + Socket Mode
    IniConfig.bHeadChamberSocketMode                                            =true;  //Ifor 20171018 (wei) : UNISEM Add Chamber + Head +Socket
    CosFunction.bOffLineBin                                                     =true;  //Ifor 20171212 : 新增Offline Bin
    IniConfig.bIndexJamInArmAway                                                =true;
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Ifor 20171212 : add JAM0303 & JAM0403需要開啟Chamber門10秒
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =true;  //Ifor 20180110 : add JAM0301 & JAM0402需要開啟Chamber門10秒
    CosFunction.bTTLUseUSec                                                     =true;  //Steven 20180808 (wei) : TTL的時間單位改成microsecond
}
//------------------------------------------------------------------------------
void FUNC_CC_MAXIM_THAILAND()
{
    IniConfig.bMaximFunction                                                    =true;  //JerryYang 20190522 Maxim統一軟體功能
    CosFunction.bLoaderTrayToAuto1                                              =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_Renesas()
{
    IniConfig.iUserLanguage                                                     =eulJapan;  //Steven 20120203 : 使用者的語系
    CosFunction.bInShuttleDetectByLatch                                         =true;  //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
}
//------------------------------------------------------------------------------
void FUNC_CC_MAXIM()
{
    IniConfig.bMaximFunction                                                    =true;  //JerryYang 20190522 Maxim統一軟體功能
    CosFunction.bLoaderTrayToAuto1                                              =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_MARVELL()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ATMEL()
{
    IniConfig.iUserLanguage                                                     =eulEnglish;
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bOnlyRoomOrHot                                                    =true;
    IniConfig.bRecordPiggyBackStartEnd                                          =true;
    IniConfig.bRetryNoNeedRestartGpib                                           =true;  //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bShuttleMode50                                                    =true;  //Steven 20120417 : RTC開單Arm Hang Up
    IniConfig.bFTTrayAss2RTTrayAss                                              =true;  //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    IniConfig.bRemeberAutoHeight                                                =true;  //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =false; //ChungHung 20120911 add 與OneCycle can desable site 衝突
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPowerSaveFunction                                                =true;
    CosFunction.bShowLastContactHigh                                            =true;  //Steven 20140409 : 顯示之前的Contact高度
    IniConfig.bResetCanServoOff                                                 =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.iTempeAlarmSecond_Over                                            =4;     //Steven 20111027 : 溫度過高的Alarm時間  //Steven 20111215 2-->4
    IniConfig.iTempeAlarmSecond_Below                                           =2;     //Steven 20111027 : 溫度過低的Alarm時間
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =false; //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    CosFunction.bStopMustTestTimeOut                                            =true;
    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
    CosFunction.bChamberModeEvenBlowNeedWaitTime                                =true;  //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
    IniConfig.bTemp25degControl                                                 =true;  //jou 2014-06-07 Temperature 25 deg. control
    CosFunction.bEnableOctal_16Kit                                              =true;  //Steven 20140312 : 8Site使用16Site Kit
}
//------------------------------------------------------------------------------
void FUNC_CC_USI()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_DYNACARD()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_CYPRESS()
{
    AnsiString s="";
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;  //JerryYang 20201119 Cypress啟用auto clean
    CosFunction.bUseLoginDatToSetLevel                                          =true;  //JerryYang 20230721 : add
    CosFunction.bOffLineBin                                                     =true;  //JerryYang 20230721 : add
    CosFunction.bEnable_SECS_GEM                                                =true;  //JerryYang 20230721 : add
    s=CheckAndReadIniDataGeneral("Version", "Serial No", AnsiString("29818"));
    if(s=="PHLL993" || s=="PHLL173" || s=="PJLL1899" || s=="PKLL1128" ||
       s=="PKLL1127" || s=="PLLW1985" || s=="PLLW1986")
    {
        CosFunction.bEnable_SECS_GEM                                            =false; //JerryYang 20230721 : add
    }
}
//------------------------------------------------------------------------------
void FUNC_CC_GERADTECH_CHINA()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_GM_TEST()
{
    IniConfig.bKoreaFunction                                                    =true;
    IniConfig.bQAMode                                                           =false; //Steven 20220523 : esytQAMode的顯示畫面拿來當作Site map check 旗標
    IniConfig.bShowFunctionWindow                                               =true;
    CosFunction.bHaveFIFOMode                                                   =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_I_TECH()
{
    IniConfig.bKoreaFunction                                                    =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_WIN_PAC()
{
    IniConfig.bKoreaFunction                                                    =true;
    CosFunction.bIndexCheckCanTurnOff                                           =true;  //Isaac 20211019 : 可選擇做index check的時機
}
//------------------------------------------------------------------------------
void FUNC_CC_VTEST()
{
    IniConfig.bVTESTFunction                                                    =true;  //jou 20210811 : VTEST function
}
//------------------------------------------------------------------------------
void FUNC_CC_VTEST_Shanghai()
{
    IniConfig.bVTESTFunction                                                    =true; //jou 20210811 : VTEST function
}
//------------------------------------------------------------------------------
void VTEST_Funtion()
{
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bForecePerPinKGf                                                =true;
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //jou 20191008 : (Steven) add SCC使用Lot ID
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bDownLoadAutoCountClear                                           =true;  //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bAutoSiteMappingSetOpenBIN                                      =true;  //jou 20200928 : Auto Site Mapping Set Open BIN
    IniConfig.bDutOnOffNeedASM                                                  =true;  //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
    CosFunction.bUseARTSortCount                                                =true;  //Ifor 20170315 add 新增使用ART Sort Count 計數功能   //20170417 功能延至下ㄧ版本
//    IniConfig.bLowYieldAlarmSameNS                                            =true;
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level

    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bHeadChamberSocketMode                                            =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket

    CosFunction.bUnloaderEditTrayLevelSet                                       =true;  //Steven 20191224 : Unloader編輯Tray改用另外一組權限
#ifdef HiSilicon
    CosFunction.bUseDefineAutoCleanOffset                                       =true;  //Ifor 20181106 add 新增Auto Clean Offset By 本機
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bByBinAlarmFromYieldForm                                        =true;  //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =true;  //Ifor 20180110 : add JAM0301 & JAM0402需要開啟Chamber門10秒
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //RogerYang 20250711 改位置 //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
#else
    CosFunction.bContactHaveOffset                                              =true;
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
#endif

    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bAutoTrayLink                                                     =true;  //jou 2012-06-14 Auto Tray Link
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bShow0Xbin                                                      =true;  //jou 20220719 : show 0X bin
    CosFunction.bLowYieldAlarmIntervalTimeBySetting                             =true;  //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
    CosFunction.bF18F22InshuttleDetectSaveByRecipe                              =true;  //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用

    CosFunction.bAutoSiteMappingSetPassBIN                                      =true;  //jou 20230221 : Auto Site Mapping Set Pass BIN
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =true;  //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray

    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    CosFunction.bShowHandlerStopTime                                            =true;  //jou 2014-09-21 Show Handler Stop Time

    CosFunction.bAutoFullDoNotStopImmediately                                   =true;  //JerryYang 20170421 (Steven) JCET凌中心要求開啟
//    CosFunction.bUseLoginDatToSetLevel                                        =true;  //Steven 20170301 (wei) : 使用Login.dat當密碼本
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    CosFunction.bCreateManualEOCAP                                              =true;  //jou 20221104 : VTest CreateManualEOCAP function;
    CosFunction.bKeepOnly1SetupFile                                             =true;

    CosFunction.b2DUseAnyCharFunction                                           =true;  //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷

//    CosFunction.bI21EnableASMByRecipe                                         =true;  //Steven 20210518 : 吳如春希望Auto site map從工作檔開關

#ifdef SOFT_SIMULTE
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //Alick 20160727 add for SCC
#endif

    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bAutoCleanTimeCT                                                =true;  //jou 20250102 : auto clean triger time count
    CosFunction.bUploadTestArmTorqueLog                                         =true;  //Steven 20250609 : 統一整理

    CosFunction.bEnable_SECS_GEM                                                =true;  //RogerYang 20250818: 偉測AddSECS
    IniConfig.b1x4Use8Suck                                                      =true;  //RogerYang 20260327 : add

    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    CosFunction.bUseHeadContactCount                                            =true;  //AI(ht9045-config) 20260508 (RogerYang) : VTEST銦片LifeTime功能
    CosFunction.bUse32ChanelSiteMap                                             =true;  //偉測趙坤鵬說開放到最多16site
    CosFunction.bCanRemoteStart                                                 =true;  //RogerYang 20260608 : Add for VTEST SECS 趙坤鵬已mail同意
}
//------------------------------------------------------------------------------
void FUNC_CC_SILICON_LABS_SG()
{
    IniConfig.bSingaporeFunction                                                =true;  //Steven 20120910 : 新加坡代理商的需求
    IniConfig.bLastLoaderNoInSide                                               =true;  //Steven 20110922 : 最後一盤不入料
    IniConfig.bResetClearArmIC                                                  =true;  //Steven 20120830 : Reset按下時,把吸嘴上的IC清空
    CosFunction.bUseLoginDatToSetLevel                                          =true;  //Steven 20170301 (wei) : 使用Login.dat當密碼本
    IniConfig.bHeadSocketMode                                                   =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SILICON_LABS_SZ()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_Altera_USA()
{
    IniConfig.bCleanOutCanTrayEnd                                               =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =false; //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    IniConfig.bTestIcCheckInContact                                             =false; //Eliot 2015-02-25
    CosFunction.bIndexZDownToAboveSocket                                        =true;  //Eliot 2015-02-25 Initial Check時手臂只下降到Above Socket Offset位置,避免壓壞Socket pogoPin
}
//------------------------------------------------------------------------------
void FUNC_CC_WINSTEK()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_HDSC()                                                                     //華大半導體
{
}
//------------------------------------------------------------------------------
void FUNC_CC_SANECHIPS()                                                                //中興微電子 深圳
{
}
//------------------------------------------------------------------------------
void FUNC_CC_RF360()                                                                    //JerryYang 20170412 (Steven) Amkor上海廠區裡的QTS
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.bOneCycleCanTrayFeed                                              =true;
    IniConfig.bHighModeCanOffTemp                                               =true;  //Steven 20110524
    IniConfig.bOneCycleDoQuickCleanOut                                          =true;  //Steven 20110524 : OneCycle後,只做快速CleanOut
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526   //Alick 20161019改True
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;  //skip in/out arm drop error
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
//    IniConfig.bLowYieldAlarmSameNS                                            =true;  //與 bLowYieldAlarmByBin 衝突
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bFix3PutAllFullIC                                                 =true;  //ChungHung 20111215
    IniConfig.bShuttleModeAccseeLevel                                           =true;  //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bDownLoadAutoCountClear                                           =true;  //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    if(REAL_TIME_CCD==true)
        IniConfig.bShuttleMode50                                                =true;  //暫時先打開，等RTC關Arm功能改好
    else
        IniConfig.bShuttleMode50                                                =false; //暫時先打開，等RTC關Arm功能改好
//    IniConfig.bEventLogAutoSaveFunction                                       =true;  //jou 20170214 (Steven) 改用evenlogsave小程式
//#ifndef SOFT_SIMULTE
//    IniConfig.bResetPortCOM1                                                    =true;  //jou 2012-10-24 Reset Port COM1
//#endif
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPasswordSecret                                                   =true;  //jou 2013-01-04 Password Txt 加密
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bShowTrayAndDeviceDir                                             =true;  //jou 2013-03-25 show Tray & Device Direction
    IniConfig.bOnlyRoomOrHot                                                    =true;
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20140715 Add
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bQAMode                                                           =CheckAndReadIniData(sPath, "QA Mode", "Enable QA Mode", false);     //jou 2015-09-11 客戶要求disable QA mode
    CosFunction.bQAmodeSupplyTrayDir                                            =true;  //jou 2014-10-24 QA mode 增加支援吸Tray方向
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //jou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    CosFunction.bRecipeCheck                                                    =true;  //JerryYang 20151028 add for Amkor 如果download的工作檔不一樣,要求做Height Calibration後才能start
    CosFunction.bDownloadRecipeLevelMode                                        =true;  //jou 2016-01-06 download recipe 增加權限模式選擇
    CosFunction.bLockF19AlwaysOn                                                =true;  //JerryYang 20160224 永遠啟用PiggyBack且不能改
//    CosFunction.bLastSetInSetUpFile                                           =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bLastSetInSetUpFile                                             =true;

    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bRTCalarmUnload                                                 =true;  //jou 20170210 (Steven) : RTC alarm image unload
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣 //jou 20170213 (Steven) 會議討論增加RT BIN設定 for Sharkls
    IniConfig.bPowerSaveFunction                                                =true;  //JerryYang 20180212 (Steven) add power save function
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bDeviceMapTest                                                  =true;  //Steven 20190910 : Qualcomm功能
    CosFunction.bHaveFIFOMode                                                   =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    CosFunction.bSortingBy2DList                                                =true;
    CosFunction.bUseSCKART                                                      =true;  //Steven 20171017 : Amkor Phi add ART function
    CosFunction.bAutoRetestGPIBmode                                             =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_CSAMQ()
{
    CosFunction.bUseBarCoderChangeSetupFile                                     =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //KenHsieh 20230406 (wei) : add 廣東利揚使用Lot ID
}
//------------------------------------------------------------------------------
void FUNC_CC_ETRENDTECH()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;    //KaiHuang 20201214 Add
    IniConfig.bFix3PutAllFullIC                                                 =true;    //KaiHuang 20201214 Add
    CosFunction.bOffLineBin                                                     =true;      //Hung 20111110 新增Offline Bin
    CosFunction.bSiteCmpYield                                                   =true;      //jou 2014-08-14 Site Compare Low Yield alarm

    CosFunction.bYieldAlarmNoWait1Min                                           =true;      //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.IntervalYieldCount                                              =true;      //wei 20180606 Interval Low Yield By Site
    CosFunction.bUseLowYieldAlarmByBin                                          =true;      //Steven 20140828 : By Bin Yield Monitor
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bIndexPickErrOnlySKIP                                             =true;      //jou 2012-02-13 index pick-up error only skip
    IniConfig.bIndexDropOnlySKIP                                                =true;      //jou 2012-02-13 index drop error only skip
//    CosFunction.bYieldControlUseEACount                                       =true;      //wei 20180606 Yield控制使用EA Count
    CosFunction.bTechComUseComboBox                                             =true;      //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    CosFunction.bRTC_ROICount                                                   =true;      //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bIndexProtect                                                   =true;      //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
    CosFunction.bUseAuto1OnlyBin1                                               =true;
    CosFunction.bConAlarmInTimeLevelUp                                          =true;      //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    IniConfig.bUseAutoSiteMapping                                               =true;      //Auto site mapping Steven 20140106
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;      //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bEnableDual_1x4Kit                                              =true;      //Isaac 20200311 : 1x2Site使用1x4Site Kit
}
//------------------------------------------------------------------------------
void FUNC_CC_QUALCOMM()
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bOneCycleNeedPowerOff                                             =true;
    IniConfig.bOneCycleCanTrayFeed                                              =true;
    IniConfig.bHighModeCanOffTemp                                               =true;  //Steven 20110524
    IniConfig.bOneCycleDoQuickCleanOut                                          =true;  //Steven 20110524 : OneCycle後,只做快速CleanOut
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526   //Alick 20161019改True
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;  //skip in/out arm drop error
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
//    IniConfig.bLowYieldAlarmSameNS                                            =true;  //與 bLowYieldAlarmByBin 衝突
    IniConfig.bLastLoaderNoInSide                                               =true;
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bFix3PutAllFullIC                                                 =true;  //ChungHung 20111215
    IniConfig.bShuttleModeAccseeLevel                                           =true;  //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bDownLoadAutoCountClear                                           =true;  //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    if(REAL_TIME_CCD==true)
        IniConfig.bShuttleMode50                                                =true;  //暫時先打開，等RTC關Arm功能改好
    else
        IniConfig.bShuttleMode50                                                =false; //暫時先打開，等RTC關Arm功能改好
//    IniConfig.bEventLogAutoSaveFunction                                       =true;  //jou 20170214 (Steven) 改用evenlogsave小程式
//#ifndef SOFT_SIMULTE
//    IniConfig.bResetPortCOM1                                                    =true;  //jou 2012-10-24 Reset Port COM1
//#endif
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bPasswordSecret                                                   =true;  //jou 2013-01-04 Password Txt 加密
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bShowTrayAndDeviceDir                                             =true;  //jou 2013-03-25 show Tray & Device Direction
    IniConfig.bOnlyRoomOrHot                                                    =true;
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20140715 Add
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bQAMode                                                           =CheckAndReadIniData(sPath, "QA Mode", "Enable QA Mode",false);     //jou 2015-09-11 客戶要求disable QA mode
    CosFunction.bQAmodeSupplyTrayDir                                            =true;  //jou 2014-10-24 QA mode 增加支援吸Tray方向
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bInitTempOffsetByWorkFile                                       =true;  //jou 2015-06-13 Initial Temperture Offset by WorkFile
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    CosFunction.bRecipeCheck                                                    =true;  //JerryYang 20151028 add for Amkor 如果download的工作檔不一樣,要求做Height Calibration後才能start
    CosFunction.bDownloadRecipeLevelMode                                        =true;  //jou 2016-01-06 download recipe 增加權限模式選擇
    CosFunction.bLockF19AlwaysOn                                                =true;  //JerryYang 20160224 永遠啟用PiggyBack且不能改
//    CosFunction.bLastSetInSetUpFile                                           =false; //JerryYang 20160513 京元先關閉此功能
    CosFunction.bLastSetInSetUpFile                                             =true;

    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bRTCalarmUnload                                                 =true;  //jou 20170210 (Steven) : RTC alarm image unload
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣 //jou 20170213 (Steven) 會議討論增加RT BIN設定 for Sharkls
    IniConfig.bPowerSaveFunction                                                =true;  //JerryYang 20180212 (Steven) add power save function
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bDeviceMapTest                                                  =true;  //Steven 20190910 : Qualcomm功能
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bEnableOctal_16Kit                                              =true;
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;
    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bATC_SlopeSaveOnHandler                                         =true;  //JerryYang 20231016 : Add handler端設定TJ參數(Slope、Offset)
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    CosFunction.bSortingBy2DList                                                =true;
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 : For SCK 93K ART
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bAutoRetestGPIBmode                                             =true;
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ifor 20190215 : add ATC 使用 三點校正功能
}
//------------------------------------------------------------------------------
void FUNC_CC_Microchip_Thailand()
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bRecordSkipPosition                                               =true;  //jou 2013-05-30 Record Skip position
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    IniConfig.bQAMode                                                           =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //Steven 20150224
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    CosFunction.bBin1CanNotInFix                                                =true;  //Steven 20150427 : Bin1不能放到Fix盤
    CosFunction.bResetModeIncludeCleanOut                                       =true;  //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
    CosFunction.bInitTestHeadByTestSiteUse                                      =true;  //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
}
//------------------------------------------------------------------------------
void FUNC_CC_Microchip_Philippines()
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bRecordSkipPosition                                               =true;  //jou 2013-05-30 Record Skip position
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    IniConfig.bQAMode                                                           =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //Steven 20150224
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    CosFunction.bBin1CanNotInFix                                                =true;  //Steven 20150427 : Bin1不能放到Fix盤
    CosFunction.bResetModeIncludeCleanOut                                       =true;  //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
    CosFunction.bInitTestHeadByTestSiteUse                                      =true;  //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
    CosFunction.bLockRTC                                                        =true;  //JerryYang 20160223 add for Philippine,切換為Online時強制開啟RTC
    CosFunction.bHaveFIFOMode                                                   =true;  //JerryYang 20181116 (Steven) : add FIFO mode
    CosFunction.bUseOldATCTempOffset                                            =true;  //JerryYang 20190123 ATC Offset UI使用舊版
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //JerryYang 20190521 add
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bRTC_ROICount                                                   =true;  //JerryYang 20220728 : add
}
//------------------------------------------------------------------------------
void FUNC_CC_Microchip_China()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ANST()
{
}
//------------------------------------------------------------------------------
void KoreaFunction()
{
    CosFunction.bShowLastContactHigh                                            =true;
    IniConfig.bResetCanServoOff                                                 =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
//    IniConfig.bLastLoaderAutoCleanOut                                         =false; //Steven 20111028 : 搭配"最後一盤不入料"功能,要不要自動CleanOut   //Steven 20170110 : Mark for ART
    IniConfig.iTempeAlarmSecond_Over                                            =4;     //Steven 20111027 : 溫度過高的Alarm時間  //Steven 20111215 2-->4
    IniConfig.iTempeAlarmSecond_Below                                           =2;     //Steven 20111027 : 溫度過低的Alarm時間
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.iUserLanguage                                                     =eulKorea;  //Steven 20120203 : 使用者的語系
    IniConfig.bCleanOutCanTrayEnd                                               =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    IniConfig.bSiteMappingFastSetDisable                                        =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
//        IniConfig.bShuttleMode50                                              =true;  //Steven 20120417 : RTC開單Arm Hang Up
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
//    IniConfig.bDualSiteCloseAbCanFullHotplate                                 =true;  //ChungHung 20120911 add 與OneCycle can desable site 衝突
    IniConfig.bAnyLevelCanGetStateRecode                                        =true;  //ChungHung 20120922 add
    CosFunction.bStopMustTestTimeOut                                            =true;
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bShowFTandRTButton                                                =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    CosFunction.bEnableOctal_12Kit                                              =true;  //ChungHung 20140508 add 使用12 layout kit Octal Shutle kit
    CosFunction.bEnableOctal_16Kit                                              =true;
    CosFunction.bChamberModeEvenBlowNeedWaitTime                                =true;  //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
    //CosFunction.bLoseDeviceOutShuttleServoOff                                 =true;  //ChungHung 20140522 add OutShuttle lose devices can servo off  //先不開放有問題
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
//    IniConfig.bEnableAutoCleanFunction                                        =true;  //ChungHung 20140723 沒開會跑到高雄跨排模式 Hangup 先不動高雄的code
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
    //IniConfig.bASE_Report                                                     =true;  //ChungHung 20150203 add
    CosFunction.bRTCFullViewErrorOnlyRetry                                      =false; //Steven 20150304 : bRTCFullViewErrorOnlyRetry預設改為True
    CosFunction.b12SiteCloseSiteRun2x4                                          =true;  //Steven 20150721 : 12Site關Site跑2x4 Mode
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20161011 : 開給韓國用
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm   //Steven 20161011 : 開給韓國用
    CosFunction.bLowYeildByTotal                                                =true;  //wei 20151116 Low Yield By Total      //Steven 20161011 : 開給韓國用
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bUseYieldControlFunction                                        =true;  //Ifor 20180731 (wei) : add Use Yield Control Function
//    IniConfig.bAmbientTempControl                                             =true;  //kevin 20140918
    CosFunction.bCanUseSearch2DIDByLot                                          =true;  //Steven 20190508
//    CosFunction.bAutoCleanUse2RowBy46LS                                       =true;
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bNonCenterModeCanUseShtOffset                                   =true;
    CosFunction.b1x4OnlyAaUse1x1Mode                                            =true;
    CosFunction.bThickTrayNoNeedCover                                           =true;  //Steven 20200723 : 太厚的Tray不能蓋蓋子
    CosFunction.b32SiteYOffsetMode                                              =true;
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
    CosFunction.bConAlarmInTimeLevelUp                                          =true;  //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    CosFunction.bDeviceMapTest                                                  =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20160303 : FIFO Mode
    CosFunction.bUseAutoTrayPreAlarm                                            =true;
    CosFunction.b2DUseAnyCharFunction                                           =true;
    CosFunction.bEnable12SiteUse16SLK                                           =false; //Steven 20240807 : 12Site使用16Site Kit
    CosFunction.bUseSCKART                                                      =true;  //Steven 20260402 : Add

    CosFunction.bPurgeBeforePickShuttle                                         =true;
}
//------------------------------------------------------------------------------
void SingaporeFunction()
{
    IniConfig.bResetCanServoOff                                                 =true;  //Steven 20140807
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bIndexDropErrCanMove                                              =true;  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
//    IniConfig.bIndexJamInArmAway                                              =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    CosFunction.bUseTrayUpDownSet                                               =false; //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.iUserLanguage                                                     =eulSingapore;  //Steven 20120203 : 使用者的語系
    IniConfig.bDisableSelectSearchLast                                          =true;  //jou 2012-01-10 取消Setup，Search Last Mode功能。
    IniConfig.bShowOffYieldBlink                                                =false; //Steven 20120609 : 功能關畫面要不要閃爍
    IniConfig.bIOFormCanControlHeaterFan                                        =true;
    CosFunction.bAutoKTemp                                                      =true;  //Steven 20120719 : 自動K溫
//        IniConfig.bLastLoaderNoInSide                                         =true;  //Steven 20110922 : 最後一盤不入料
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;  //Steven 20120202
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
}
//------------------------------------------------------------------------------
void FUNC_CC_RICHTEK()                                                                  //Alick 20161206 add 立錡預設開啟功能
{
    IniConfig.bCanDisableTempMonitor                                            =true;  //能夠關閉溫控器監視。
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;  //自動存EventLog
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    IniConfig.bShowMainDebugRecord                                              =true;  //jou 2013-02-25 Show Main form Debug Record
    IniConfig.bShowFormByInitPos                                                =true;  //jou 2013-12-02
    CosFunction.bLockD41ByFile                                                  =true;  //Steven 20140627 : Add for ASE-CL        //wei 20141203 add
    IniConfig.bFix3PutAllFullIC                                                 =true;
    CosFunction.bNotClearAllHotBuffer                                           =true;  //JerryYang 20151226 For 矽格 由GPIB設定完溫度後，會馬上再問一次溫度。所以不清除暫存溫度
    CosFunction.bYieldAlarmUseDouble                                            =true;  //JerryYang 20160615 Yield相關limit可以設定到小數點
    IniConfig.bHeadSocketMode                                                   =true;  //Alick 20160628 矽格湖口修改開啟
    IniConfig.bHeadChamberSocketMode                                            =true;  //Alick 20160628 矽格湖口修改開啟
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bDeviceMapTest                                                  =true;  //Steven 20190910 : Qualcomm功能
    CosFunction.bDeviceMapTestPandP                                             =true;  //Jimmychiu 20251222 : Device Map Test By P&P
}
//------------------------------------------------------------------------------
void FUNC_CC_TSI()
{
    CosFunction.bYieldAlarmNoWait1Min                                           =true;  //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.IntervalYieldCount                                              =true;  //wei 20180606 Interval Low Yield By Site
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
//    CosFunction.bYieldControlUseEACount                                       =true;  //wei 20180606 Yield控制使用EA Count
    CosFunction.bTechComUseComboBox                                             =true;  //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    CosFunction.bRTC_ROICount                                                   =true;  //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
    IniConfig.bHeadSocketMode                                                   =true;  //Alick 20160628 矽格湖口修改開啟
    IniConfig.bHeadChamberSocketMode                                            =true;  //Alick 20160628 矽格湖口修改開啟
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bAutoCleanUseHPSetByRecipe                                      =true;  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
}
//------------------------------------------------------------------------------
void FUNC_CC_ARDENTEC()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_FULCAP()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_AOSL()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_Spreadtrum()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_UMC()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ITestInc()                                                                 //iTest,Inc. 美國
{
}
//------------------------------------------------------------------------------
void FUNC_CC_EMemory()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_CYUEAN()
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.bShowHPICCount                                                  =true;  //Steven 20221228 : 計算加熱盤IC數量
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bShakeShuttleEveryTime                                          =true;
    CosFunction.bAutoSiteMappingSetOpenBIN                                      =true;  //Steven 20230213 : [I21-9]的OS Bin跟著工作檔
    CosFunction.bFTPUseBarcodeReader                                            =true;  //Ifor 20210413 add:FTP Use Barcode Reader
    CosFunction.bKeepOnly1SetupFile                                             =true;  //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bUseLowYieldAlarmByBin                                          =true;
    CosFunction.bContactHeightSaveToContactIni                                  =true;
    CosFunction.bConAlarmNeedKeyInPassword                                      =true;
    CosFunction.bLastSetInSetUpFile                                             =true;
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;
    IniConfig.bShowFTandRTButtonCanClick                                        =true;
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    CosFunction.bRunModeFollowLotInfo                                           =true;  //Steven 20250603 : 根據Lot Info的Run mode進行切換
//    CosFunction.bUnloadTrayModeByRecipe                                         =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_PANTHER()
{
    CosFunction.bEndLotAfterTrayFeed                                            =true;  //Jimmychiu 20250115 : Auto End Lot After Tray Feed
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bShowLotInfo                                                      =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ATEC()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.bSiteCmpYield                                                   =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_SIGURD_SUZHOU()                                                            //KaiChen 20200506 ：新增客戶碼，矽格蘇州(SIGURD_SUZHOU)
{
    IniConfig.bSIGURDFunction                                                   =true;  //KaiChen 20200506 ：矽格統一軟體功能
    IniConfig.bHeadChamberSocketMode                                            =true;  //Sam 20230628 : Add
    IniConfig.bHeadSocketMode                                                   =true;  //Sam 20230628 : Add
}
//------------------------------------------------------------------------------
void FUNC_CC_VATE()                                                                     //立衛科技
{
    IniConfig.bFTBin2RTBin                                                      =true;  //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣   //wei 20150420 OpenS
}
//------------------------------------------------------------------------------
void FUNC_CC_Murata()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;
    CosFunction.bShowLastContactHigh                                            =true;
    IniConfig.bHeadSocketMode                                                   =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bEnableUnloadTrayFree                                             =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    CosFunction.bStopMustTestTimeOut                                            =true;
    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bUseAutoTrayPreAlarm                                            =true;
    CosFunction.bSaveProductionLogByUnloaderTray                                =true;  //Steven 20200330 : production log by unloader tray存檔
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bRunModeFollowLotInfo                                           =true;  //Steven 20250603 : 根據Lot Info的Run mode進行切換
}
//------------------------------------------------------------------------------
void FUNC_CC_ChipMos_ZHUBEI()
{
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnableAutoSpeed                                                =true;
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bGPIBUseSECSGENData                                             =true;  //Sam 20240826 : GPIB 通訊資料使用 SECSGEM Data
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bRMSNoNeedToDownloadEveryTime                                   =true;  //Steven 20240926 : RMS不要每次下載包成功能
    CosFunction.bFTPUseBarcodeReader                                            =true;  //Ifor 20210413 add:FTP Use Barcode Reader
    CosFunction.bLowYieldAutoSiteOff                                            =true;  //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bOpenDoorCheckLoaderAfterTrayEnd                                =true;  //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
}
//------------------------------------------------------------------------------
void FUNC_CC_GT()
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //wei 20180202 : 開啟Auto Clean功能
    IniConfig.bShowLotInfo                                                      =true;  //wei 20180202 : 開啟Auto Clean功能
    CosFunction.bEnableBarcodeReader                                            =true;  //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
    IniConfig.bIndexDropOnlySKIP                                                =true;
    CosFunction.bTestTimeOutOnlyShowSkip                                        =true;
    IniConfig.bOurArmDropICSkip                                                 =true;  //kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bGPIB_Command_DOPAUSE                                           =true;  //Jimmychiu 20231102 : GPIB Command SETHANDLERDOPAUSE
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
}
//------------------------------------------------------------------------------
void FUNC_CC_AMD_M()
{
    CosFunction.bATC32UseTJMode                                                 =true;  //Ifor 20190121 : add ATC32 Use TJ Mode
    CosFunction.bUseAuto1OnlyBin1                                               =true;  //Ifor 20190123 : add
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //Ifor 20190131 : add
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ifor 20190215 : add ATC 使用 三點校正功能
    CosFunction.bATCUsePackageOffset                                            =true;  //Ifor 20190215 : add ATC 使用 Package Offset 功能
    CosFunction.bUseHandShakeCommunication                                      =true;  //Ifor 20190225 :add Bar Code Use HandShake Communication
    CosFunction.bUseATCTJControlMode                                            =true;  //Ifor 20190328 : add TJ Temp Over Range
    CosFunction.bAMDFunction                                                    =true;  //Ifor 20190927 : add AMD 強制開啟AMD Function
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bRecordSkipPosition                                               =true;  //kevin 20170209 add //jou 2013-05-30 Record Skip position
    CosFunction.bUseFTPDownLoadATCRecipe                                        =true;  //Ifor 20191115 : add FTP DownLoad ATC Recipe
    CosFunction.bATCUseHandlerRecipeName                                        =true;  //Ifor 20191126 : add ATC Use Handler Recipe Name
    CosFunction.bUseHandlerSetChillerTemp                                       =true;  //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    CosFunction.bOffLineBin                                                     =true;  //Ifor 20200121 : add AMD Offline Bin
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =true;  //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
    CosFunction.b2DUseSubJobFunction                                            =true;  //Ifor 20200807 add:In House 2D Use Sub Job Function
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bFTPUseBarcodeReader                                            =true;  //Ifor 20210413 add:FTP Use Barcode Reader
    CosFunction.b2DUseAnyCharFunction                                           =true;  //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =true;  //Ifor 20210923 : add FTP DownLoad Site By SetupFile
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
}
//------------------------------------------------------------------------------
void FUNC_CC_HANA_MICRON()
{
    CosFunction.bEnableOctal_16Kit                                              =true;
    IniConfig.bKoreaFunction                                                    =true;
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bUnloaderEditTrayLevelSet                                       =true;  //Steven 20191224 : Unloader編輯Tray改用另外一組權限
    CosFunction.bAutoCloseSiteWhenRT                                            =true;  //Steven 20200225 : 切到RT的時候,要關閉Socket
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bPowerSaveFunction                                                =true;
//    CosFunction.bAutoCleanUseHPSetByRecipe                                    =true;  //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    CosFunction.bYieldAlarmClearAllCount                                        =true;
    CosFunction.bRMSNoNeedToDownloadEveryTime                                   =true;  //Steven 20240926 : RMS不要每次下載包成功能
    CosFunction.bCheckTempDuringIndexArmTesting                                 =true;  //Jimmychiu 20240916 : Check the temperature during index arm testing
    CosFunction.bUseSCKART                                                      =true;
    CosFunction.bAutoRetestGPIBmode                                             =true;
    CosFunction.bTempAlarmBinNeedToError                                        =true;  //Steven 20251022 : Temp alarm need put to error bin
}
//------------------------------------------------------------------------------
void SPILFunction()                                                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
{
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bDoorOpenShuttleContinueHeat                                      =true;
    IniConfig.bOpenDoorNotStopFan                                               =true;
    IniConfig.bUseFix3                                                          =true;  //Steven 20140115 : LS有FIX3
    IniConfig.bSiteMappingFastSetDisable                                        =true;
//    IniConfig.bIndexJamInArmAway                                              =true;
    CosFunction.bOneCycleCanChangeArm                                           =true;  //Hung 20111108 Amb/Hot Mode OneCycle Can Change Arm
    IniConfig.iTempeAlarmSecond_Over                                            =2;     //Steven 20111027 : 溫度過高的Alarm時間
    IniConfig.bShuttleMode50                                                    =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bInitialStartDelayCount                                           =true;  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    IniConfig.bShowLotInfo                                                      =true;  //jou 2013-01-18 Show Lot Info
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
//    CosFunction.bEnable12Site                                                 =true;  //Steven 20120813 : 支援12Site模式
    IniConfig.bEventLogAutoSaveFunction                                         =true;

//    if(SPIL_FOR_QLE==true)
//    {
//        IniConfig.bShowFormByInitPos                                          =false;
//    }
//    else
//    {
//        IniConfig.bShowFormByInitPos                                          =true;  //jou 2013-12-02
//    }
//    IniConfig.bIndexDropOnlyReset                                             =true;  //jou 2013-12-02 Index Drop Only Reset
    IniConfig.bFTPJamCodeUpload                                                 =true;  //ChungHung 20140108 add FTP unload jam code
//    IniConfig.bUseTrayBlockMode                                                                           =true;  //2014-03-04    Dell    for SPIL WLP Add Tray Block //打開需驗證Fix分Tray模式
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    CosFunction.bHaveIndexContactDelay                                          =true;  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
//    CosFunction.bEnableTimeOutCanSkip                                         =true;  //ChungHung 20140715 SPIL 又想改回 可以 Retry and Skip 所以直接開放可以選
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bArm2ForFingerPrintTest                                         =true;  //Steven 20140922 : Arm2當作指紋測試
    CosFunction.bUseIndividulTempSet                                            =true;  //Steven 20140924 : 各個加熱區獨立有自己的設定值
    CosFunction.bRCMDStart                                                      =true;  //Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bStopMustTestTimeOut                                            =true;
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =true;  //Steven 20141105 : Index內的所有異常都只能用Skip
    CosFunction.bYieldControlUseContactCount                                    =true;  //Steven 20141212 : Yield控制使用Contact Count
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150128 : KK說要開啟使用
    CosFunction.bRTCFullViewErrorOnlyRetry                                      =false; //Steven 20150304 : bRTCFullViewErrorOnlyRetry預設改為True
    CosFunction.bEnableOctal_12Kit                                              =true;  //ChungHung 20140508 add 使用12 layout kit Octal Shutle kit
    CosFunction.b12SiteCloseSiteRun2x4                                          =true;  //Steven 20150721 : 12Site關Site跑2x4 Mode
//    CosFunction.bAutoCleanAutoSelIndexArm                                     =true;  //JerryYang 20230517 : 國龍要求要可以選擇用哪一支ARM做auto clean  //jou 2015-08-12 Auto Clean Auto Select Index Arm
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bESDAutoDecayTeachFunction                                      =true;  //Ifor 20151222 :新增 ESD Auto Decay Teach Function
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20160223 : 入出料FIFO Mode
//    CosFunction.bKnockerSetBySetupFile                                        =true;  //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理  //JerryYang 20220330 : SPIL佶融要求把所有振動器改回Config
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20160407 :Log Up to FTP Function
    CosFunction.bYieldAlarmUseDouble                                            =true;  //JerryYang 20160615 Yield相關limit可以設定到小數點    //Ifor 20160712 新竹矽品-世明 要求新增
    CosFunction.bUse1_16KgEPForm                                                =true;  //Ifor 20160712 add Use 1~16 Kg EP對照表功能
    CosFunction.bShowAlarmIfCCLinkConnectFail                                   =true;  //JerryYang 20160729 for 矽品, CC-link連線異常要Show alarm
    CosFunction.bAutoFullDoNotStopImmediately                                   =false;  //JerryYang 20160810 當Auto滿tray時,要先show message鳴叫且繼續生產,不要馬上停機show alrm
    CosFunction.bIndexArmVacOffErrNeedPiggyBack                                 =true;  //JerryYang 20160815 for 矽品, Index arm 發生Vaccum off error要做piggy back
    CosFunction.bAllSiteSameFailBinShowAlarm                                    =true;  //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
//    IniConfig.bLastLoaderNoInSide                                             =true;  //JerryYang 20160201 最後一盤不入料  //JerryYang 20170410 (wei) Mark,最後一盤不入料統一調sensor
    IniConfig.b1x2Use4Suck                                                      =true;  //Hung 20110812 : 1x2使用4吸嘴
    CosFunction.bBeforeAutoCleanOnlyUseRTC                                      =true;  //JerryYang 20161216 (jou) auto clean的前後只靠RTC來檢查socket,不做index下壓至socket吸真空
    CosFunction.bFullTestBeforeAutoClean                                        =true;  //JerryYang 20160331 SPIL中科要求Auto clean前做 full view check
    CosFunction.bFullTestBeforeContactHeight                                    =true;  //JerryYang 20160516 Contact Height前做FullView
    CosFunction.bSuckDevicesDuringTest                                          =true;  //JerryYang 20170522 drop contact改為邊吸邊測
//    CosFunction.bSortingBinTraywhenCleanOut                                   =true;  //RogerYang 20170703 (Steven) Auto Sorting BinTray by Out Arm when Clean Out
    CosFunction.bSortingBinTrayWhenTrayFeed                                     =true;  //JerryYang 20170911 (Steven) 整盤功能,執行時機由clean out改至tray feed前
    CosFunction.bNewAutoTrayDetect                                              =true;  //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //JerryYang 20170922 (Steven) add Check EP function  //jou 20171121 : 修正D24關閉時會被強制開啟     //JerryYang 20181102 重新啟用
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
    CosFunction.bHisiLogUploadNetwork                                           =true;  //Ifor 20170913 (Steven) add 海思 Log 上傳至網路磁碟
//    CosFunction.bSaveAutoCleanCnt                                             =true;  //JerryYang 20171102 (wei) auto clean 清潔次數by 工作檔儲存
    CosFunction.bCanChangeAutoCleanCount                                        =true;  //Ifor 20171024 : add 可更改Auto Clean 計數  //JerryYang 20171128 auto clean 清潔次數可修改
//    CosFunction.bUseLoginDatToSetLevel                                        =true;
    CosFunction.bUseDynamicKitDiameter                                          =true;  //Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
//    CosFunction.bTestTimeOutOnlyShowSkip                                      =true;  //Steven 20180627 : Time out預設只顯示SKIP
    CosFunction.bTestTimeOutShowSkipAndHome                                     =true;  //JerryYang 20231208
    CosFunction.bAfterInitialDelayUseOtherArm                                   =true;  //JerryYang 20181129 取消此功能  //Jerryyang 20180607 : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
//    IniConfig.bAmbientTempControl                                             =true;  //kevin 20140918
    CosFunction.bRotateNoUseFix1                                                =true;
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;  //JerryYang 20180614 (wei) : SPIL新竹要求12 site auto clean取放位置要固定
    CosFunction.bATC32UseTJMode                                                 =true;  //Ifor 20190121 : add ATC32 Use TJ Mode
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ifor 20190215 : add ATC 使用 三點校正功能
    CosFunction.bATCUsePackageOffset                                            =true;  //Ifor 20190215 : add ATC 使用 Package Offset 功能
    CosFunction.bUseATCTJControlMode                                            =true;  //Ifor 20190328 : add TJ Temp Over Range
    CosFunction.bAMDFunction                                                    =true;
    IniConfig.bQAMode                                                           =true;  //JerryYang 20190704 add
    CosFunction.bForecePerPinKGf                                                =true;  //JerryYang 20180515 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    IniConfig.bUseAutoOffsetFunction                                            =true;
//    CosFunction.bSetupTeach                                                   =true;  //JerryYang 20190313 : MARK掉
    CosFunction.bAutoSkipNoDropError                                            =true;
    CosFunction.bUseSCKART                                                      =true;  //Steven 20171017 : Amkor Phi add ART function
    CosFunction.bCanDisableQAMode                                               =true;  //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
    if(fSCKART!=NULL && fSCKART->iTesterType==1)
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;
    CosFunction.bYPitchNotUseSearchLastMode                                     =true;  //JerryYang 20200324 中科志強要求開啟
    CosFunction.bECChangeEventReportCanOnOff                                    =true;  //JerryYang 20200520 舊電腦做DoReportECDataChangeCheck函式會影響UPH,改成功能選項
    CosFunction.bOutShtLoseICInArmAway                                          =true;  //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
    CosFunction.bAutoTrayFullAlarmCanNotSkip                                    =true;  //Steven 20210219 : Unloader滿盤alarm不能Skip
    CosFunction.bUseMDB                                                         =true;  //Steven 20210526 : 部分客戶取消使用MDB
    CosFunction.bART_SECSGEM_93K                                                =true;
    CosFunction.b2DUseSubJobFunction                                            =true;  //Ifor 20200807 add:In House 2D Use Sub Job Function
    CosFunction.bOutArmPickUpErrBySetup                                         =true;
    CosFunction.bNonATCSupportLBFunction                                        =true;  //JerryYang 20220126: non ATC也支援LB function
    CosFunction.bOutShtLoseICSetErrUntilOneCycle                                =true;
    CosFunction.bIndexDropICSetErrUntilOneCycle                                 =true;
    CosFunction.bFTRTDiffInitStartDelayTime                                     =true;
    CosFunction.bATCUseHandlerRecipeName                                        =true;  //Ifor 20191126 : add ATC Use Handler Recipe Name
    CosFunction.bEnableOctal_16Kit                                              =true;
    CosFunction.bTesterSidePushFunction                                         =true;  //Richard 20230107 : 渠梁Side Push
    CosFunction.bUserDefineIndexZSafePos                                        =true;  //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
    CosFunction.bSortingBy2DList                                                =true;  //JerryYang 20230322 : add
    CosFunction.bBySiteByBinPercentCompare                                      =true;
    CosFunction.bThickTrayUseDiffHeight                                         =true;
    CosFunction.bTrayOCR                                                        =true;
//    CosFunction.bUseLoginDatToSetLevel                                        =true;
    CosFunction.bShowCloseSiteAlarmWhenStart                                    =true;  //Jimmychiu 20230925 : Show Close Sites Alarm When Start
    CosFunction.b2DUseAnyCharFunction                                           =true;
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;
    CosFunction.bATC_SlopeSaveOnHandler                                         =true;
    CosFunction.bFullTrayAlarmAfterUnloadEnd                                    =true;
    CosFunction.bRMSNoNeedToDownloadEveryTime                                   =true;  //Steven 20240926 : RMS不要每次下載包成功能
    CosFunction.bUse32ChanelSiteMap                                             =true;  //JerryYang 20240916 : Nvidia Enzo要求開啟
    CosFunction.bUseSecondATCTempOffset                                         =true;  //Ifor 20241014 Use Second ATC Temp Offset
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bEnable_1x3Kit                                                  =true;  //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    CosFunction.bCanUse2x2NNMode                                                =true;  //KevinCheng 20260109 : 2x2NN
    CosFunction.bUseSocketContactCount                                          =true;
    CosFunction.bLockI02ByFile                                                  =true;  //JerryYang 20260504 : for SPIL -- I02/A03 Lock by File
    CosFunction.bLockA03ByFile                                                  =true;  //JerryYang 20260504 : for SPIL -- I02/A03 Lock by File
}
//------------------------------------------------------------------------------
void MaximFunction()                                                                    //JerryYang 20190522 Maxim統一軟體功能
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
    IniConfig.bRecordSkipPosition                                               =true;  //jou 2013-05-30 Record Skip position
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    IniConfig.bQAMode                                                           =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //Steven 20150224
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    CosFunction.bBin1CanNotInFix                                                =true;  //Steven 20150427 : Bin1不能放到Fix盤
    CosFunction.bResetModeIncludeCleanOut                                       =true;  //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
    IniConfig.bShowLotInfo                                                      =true;  //JerryYang 20160422 : Show Lot Info
    CosFunction.bInitTestHeadByTestSiteUse                                      =true;  //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bRTC_ROICount                                                   =true;  //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bLockRTC                                                        =true;  //JerryYang 20160223 add for Philippine,切換為Online時強制開啟RTC
    CosFunction.bColorSensorUseCCLink                                           =true;
    CosFunction.bRTCHalfViewAutoVerify                                          =true;  //JerryYang 20220215 : RTC Auto Verify half view check
//    CosFunction.bQAModeUseUnloadCnt                                             =true;  //JerryYang 20221005 : QA mode count 用BIN1數量計數
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bUseMDB                                                         =true;      //Steven 20210526 : 部分客戶取消使用MDB
//    CosFunction.bSECSGEM_UseKeyPro                                              =true;
    CosFunction.bInspectSuckICWhenSoftStart                                     =false;
}
//------------------------------------------------------------------------------
void FUNC_CC_TFME_CHINA()
{
    AnsiString sPath=AuthPath+"config.ini";

    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bRCMDStart                                                      =true;
    CosFunction.bSiteCmpYield                                                   =true;
    CosFunction.b12SiteCloseSiteRun2x4                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bShowLotInfo                                                      =true;
//    IniConfig.bQAMode                                                           =true;
    IniConfig.bQAMode                                                           =CheckAndReadIniData(sPath, "QA Mode", "Enable QA Mode", false);     //jou 2015-09-11 客戶要求disable QA mode
    CosFunction.bATCUseHandlerRecipeName                                        =true;
    CosFunction.bYieldControlUseEACount                                         =true;
    IniConfig.bLowYieldAlarmSameNS                                              =true;
    CosFunction.bYieldAlarm4                                                    =true;  //Sam 20181025 開啟 Alarm4
    IniConfig.bPowerSaveFunction                                                =true;
    IniConfig.bFTBin2RTBin                                                      =true;
    CosFunction.bYieldAlmNeedOneCycle                                           =true;
    CosFunction.bContinueFailNeedAlarmDirectly                                  =true;  //Steven 20230831 : 當使用bYieldAlmNeedOneCycle, 連續Fail要直接Alarm
    IniConfig.bOutShLoseNeedOpenChamber                                         =true;
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =true;
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;
//    CosFunction.bOffLineBin                                                   =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_CETC()
{
    CosFunction.bTTLCanUse8Site                                                 =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Steven 20140513 : [D35]
    IniConfig.bSiteMappingFastSetDisable                                        =true;  //JerryYang 20200916 SCC楊建軍要求取消site map快捷
}
//------------------------------------------------------------------------------
void FUNC_CC_LEADYO()                                                                   //廣東利揚
{
    CosFunction.bTrayOCR                                                        =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //KenHsieh 20230406 (wei) : add 廣東利揚使用Lot ID
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;  //Steven 20140521 : RT後自動切回FT
}
//------------------------------------------------------------------------------
void FUNC_CC_NEXPERIA_Guangdong()                                                       //廣州安世
{
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bQAMode                                                           =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_JSSI_Semiconductor()                                                       //江蘇芯德半導體.
{
    IniConfig.bShowLotInfo                                                      =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    IniConfig.bFTBin2RTBin                                                      =true;
    CosFunction.bYieldControlUseEACount                                         =true;
    CosFunction.bReadClipCodeFromUnloader                                       =true;  //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
    CosFunction.bMonitorLBonAmbient                                             =true;  //Jimmychiu 20251030 : Monitor the LB temperature under ambient temperature
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bCanRemoteStart                                                 =true;  //Jimmychiu 20251231 : JSSI要求遠端可以START
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
}
//------------------------------------------------------------------------------
void FUNC_CC_Goertek()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_BOJIAN()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ONSEMI_CA()                                                                // ONSEMI_Canada
{
    CosFunction.bTrayOCR                                                        =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ONSEMI_M()
{
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM   //20140124 wei
    CosFunction.bRCMDStart                                                      =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ITS()                                                                      //創量科技
{
}
//------------------------------------------------------------------------------
void FUNC_CC_DJI_SZ()                                                                   //大疆創新 深圳
{
}
//------------------------------------------------------------------------------
void FUNC_CC_NXP_TJ()                                                                   //恩智浦 天津
{
}
//------------------------------------------------------------------------------
void FUNC_CC_SANAN()                                                                    //三安光電
{
    CosFunction.bUseMDB                                                         =false; //Steven 20210526 : 部分客戶取消使用MDB
    CosFunction.bFTPFunction                                                    =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bRCMDStart                                                      =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.bIncludeMTBA                                                    =true;  //Sam 20181031 : 開啟
}
//------------------------------------------------------------------------------
void FUNC_CC_HABANA()                                                                   //HABANA
{
}
//------------------------------------------------------------------------------
void FUNC_CC_XDXCT()                                                                    //象帝先
{
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_THINE()                                                                    //Thine 日本
{
}
//------------------------------------------------------------------------------
void FUNC_CC_CENTER()                                                                   //中芯國際
{
}
//------------------------------------------------------------------------------
void FUNC_CC_SINOICTECH()                                                               //上海華嶺
{
    CosFunction.bOutShuttleSensorCanNotDisable                                  =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_HYGEIA_SUZHOU()                                                            //海京 蘇州
{
}
//------------------------------------------------------------------------------
void FUNC_CC_UTAC_TW()                                                                  //聯測科技 台灣
{
    IniConfig.bSIGURDFunction                                                   =true;  //KaiChen 20200506 ：矽格統一軟體功能
    CosFunction.bSortingBinTraywhenCleanOut                                     =true;  //JerryYang 20151006 整盤功能
    CosFunction.bTesterLowYieldOneCycle                                         =true;  //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
    CosFunction.bEnableSoftWareControlButton                                    =true;  //JerryYang 20160302 建榮要求軟體控制面板功能
    CosFunction.bContactTestVacOffByCloseSite                                   =true;  //JerryYang 20160328 Contact test及auto height時,關site的部分不吸取IC
    CosFunction.bG09NeedPasswordWhenEditSiteMap                                 =true;  //JerryYang 20160425 修改Site map需要密碼
    CosFunction.bLastSetInSetUpFile                                             =true;  //JerryYang 20160422 把LastSet選項存在Set Up File
    CosFunction.bLockP24ByFile                                                  =true;  //JerryYang 20160425 lock P24
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //Alick 20161122 add 北興要求在未CleanOut狀態下可以修改ContinuesFailBin
    CosFunction.bEnable12Site                                                   =true;  //Steven 20120813 : 支援12Site模式  //Ifor 20170420 (wei) 新增SIGURD PeiXing 支援12Site模式
    CosFunction.bSetupFileNameControlByLevel                                    =true;  //jou 20171011 (wei) : bSetupFileName Control By Level
    IniConfig.bEventLogAutoSaveFunction                                         =true;  //Ifor 20171018 (wei) add 矽格北興廠使用自動存EventLog
    CosFunction.bUseLotIDWithoutSECS                                            =true;  //Ifor 20171018 (wei) add 矽格北興廠使用Lot ID
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 (Steven) : Use FTP Download Data Check
    CosFunction.bFTPDataTrayFeedAutoUpdata                                      =true;  //Ifor 20181214 (Steven) : add Tray Feed 後自動上傳海思檔案至FTP
    CosFunction.bUseLowYieldAlarmByBin                                          =true;  //KaiChen 20180612 ：矽格-北興 add
    IniConfig.bLowYieldAlarmSameNS                                              =true;  //KaiChen 20180612 ：矽格-北興 add
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //KaiChen 20181023 ：矽格-北興 add
    CosFunction.bSaveEventLogByLotID                                            =true;  //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
    CosFunction.bLockF06ByFile                                                  =true;  //KaiChen 20181027 ：矽格-北興 add
    IniConfig.bEnableTestingNeedStopAllMotor                                    =true;  //KaiChen 20190527 ：矽格-北興 add
    CosFunction.bUseDynamicKitDiameter                                          =true;  //KaiChen 20190628 ：矽格-北興 add
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //KaiChen 20191017 ：矽格-北興 add
    CosFunction.bRotateUseHT7000HPKit                                           =true;  //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
    IniConfig.b1x2Use4Suck                                                      =true;
    CosFunction.bEnableAfterTestedDelay                                         =true;
    CosFunction.bHaveFIFOMode                                                   =true;
    CosFunction.bContactShowOffset                                              =true;  //Sam 20210104 : 聯測也要 Contact Offset
    CosFunction.bAutoTmpeOfsByFTP                                               =true;  //Sam 20220406 : 溫度自動補償功能 By FTP
    CosFunction.bUseMRTMode                                                     =true;  //Sam 20230328 : 聯測開啟第二組 RT 設定
    CosFunction.bAfterInitialDelayUseOtherArm                                   =true;
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    IniConfig.bHeadChamberSocketMode                                            =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
}
//------------------------------------------------------------------------------
void FUNC_CC_FOREHOPE_NINGBO()                                                          //甬矽
{
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bUseFTPDownloadDataCheck                                        =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;  //Steven 20140426 : 客戶要求Clean Out後要跳Initial Start
    CosFunction.bUnloadTrayModeByRecipe                                         =true;  //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    IniConfig.bFTBin2RTBin                                                      =true;  //Frank 20170216 新增
    CosFunction.bCanChangeAutoCleanCount                                        =true;
    CosFunction.bAfterRTChangeToInitialStart                                    =true;
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =true;
    CosFunction.bUseOneByOneIndexCheck                                          =true;  //Ifor 20180322 : add Use One By One Index Check
    CosFunction.bSiteCmpYield                                                   =true;  //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =true;  //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    CosFunction.bUseFix3FullTray                                                =true;
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bShowLastContactHigh                                            =true;
    CosFunction.bSetContactForceToAutoClean                                     =true;  //Steven 20250915 : 複製Contact Force給Auto Clean用
    CosFunction.bOneByOneWhenPickErrAtLoader                                    =true;  //Jimmychiu 20250826 : Suck one by one when a pickup error occurs at the loader.
    CosFunction.bUseDefineAutoCleanOffset                                       =true;  //Ifor 20181106 add 新增Auto Clean Offset By 本機
}
//------------------------------------------------------------------------------
void FUNC_CC_FMSH()                                                                     //上海復旦微電子
{
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Ifor 20200103 : 開啟Auto Clean功能
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bEnable_SECS_GEM                                                =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_ASIAOPTICAL()                                                              //亞洲光學
{
}
//------------------------------------------------------------------------------
void FUNC_CC_HXYSEMI()                                                                  //浙江航源芯
{
}
//------------------------------------------------------------------------------
void FUNC_CC_INTEL_IL()                                                                 //INTEL 以色列
{
}
//------------------------------------------------------------------------------
void FUNC_CC_IBM_CANADA()                                                               // IBM 加拿大
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ISE_US()                                                                   //ISE 美國
{
}
//------------------------------------------------------------------------------
void FUNC_CC_ISE_SH()                                                                   //上海月芯半導體科技
{
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bSECSGEM_UseKeyPro                                              =true;  //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    CosFunction.bCanRemoteStart                                                 =true;  //Steven 20241122 : ISE要求遠端可以START
}
//------------------------------------------------------------------------------
void FUNC_CC_BARUN()                                                                    //百齡
{
    IniConfig.bKoreaFunction                                                    =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_IFXTH_Thai()                                                               //IFXTH泰國
{
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_Ramos()                                                                    //Ramos
{
    IniConfig.bKoreaFunction                                                    =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_Carsem_Thai()                                                              //Carsem 泰國
{
    CosFunction.bEnable_SECS_GEM                                                =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_GOOGLE_Israel()                                                            //GOOGLE 以色列
{
}
//------------------------------------------------------------------------------
void FUNC_CC_Mellanox_Israel()                                                          //Mellanox 以色列
{
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void SIGURDFunction()                                                                   //KaiChen 20200506 ：矽格統一軟體功能
{
    CosFunction.bYieldAlarmClearAllCount                                        =true;
    CosFunction.bYieldAlarmNoWait1Min                                           =true;
    CosFunction.bUseJamRawData                                                  =true;
    CosFunction.bUse1CableLayoutKitByConfig                                     =true;  //Sam 20210715 : Use 1CableLayoutKit By Config

    IniConfig.bRecordSkipPosition                                               =true;
    CosFunction.bUseLogUploadToFTPFunction                                      =true;
    IniConfig.bShowLotInfo                                                      =true;
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bFTPFunction                                                    =true;
    CosFunction.bYieldControlUseContactCount                                    =true;
    CosFunction.bLowYeildByTotal                                                =true;
    CosFunction.bSiteCmpYield                                                   =true;
    IniConfig.bShowFTandRTButtonCanClick                                        =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bYieldAlarmUseDouble                                            =true;
    CosFunction.bEnableAutoSpeed                                                =true;
    CosFunction.bUseAlarmLogXml                                                 =true;
    CosFunction.bLowYieldAutoSiteOff                                            =true;
    CosFunction.bHandlerStateChangeUploadServer                                 =true;  //Sam 20230511 : 機台資料變更後須上傳 FTP
    CosFunction.bGPIB_Command_DOPAUSE                                           =true;  //Jimmychiu 20231102 : GPIB Command SETHANDLERDOPAUSE
    IniConfig.b1x4Use8Suck                                                      =true;  //Hung 20110812 : 1x4使用8吸嘴
    CosFunction.bRecordGroundESDByTestIC                                        =true;
    CosFunction.bAutoTmpeOfsByFTP                                               =true;
    CosFunction.bRTCFullViewErrorOnlyRetry                                      =false;
    CosFunction.bRTCAutoTuning                                                  =true;
    IniConfig.bPowerSaveFunction                                                =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //Sam 20250325 : 矽格開啟 SECS/GEM 功能
    #ifndef SOFT_SIMULTE
    CosFunction.bSECSGEM_UseKeyPro                                              =true;
    #endif
    IniConfig.bHaveRTCCheckSiteMap                                              =true;  //Sam 20241230 : 矽格開啟 RTC Site 檢查
}
//------------------------------------------------------------------------------
void FUNC_CC_AnalogDevice_Phil()                                                        //JerryYang 20230721 : Analog
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping Steven 20140106
    IniConfig.bHeadSocketMode                                                   =true;  //Steven 20131023 : For HT9046AH
    IniConfig.bRecordSkipPosition                                               =true;  //jou 2013-05-30 Record Skip position
    CosFunction.bEnableAutoSpeed                                                =true;  //Steven 20110526 : 開啟Auto Speed功能
    IniConfig.bUseAutoOffsetFunction                                            =true;  //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bLastLoaderNoInSide                                               =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNoTrayAutoCleanOut                                               =true;  //jou 2013-08-01 Loader No Tray Auto Clean
    IniConfig.bAlarmMustRedColor                                                =true;  //Steven 20111116 : 特殊Alarm需要改紅底
    IniConfig.bEventLogAutoSaveFunction                                         =true;
    CosFunction.bOffLineBin                                                     =true;  //Hung 20111110 新增Offline Bin
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bCleanOutCanTrayEnd                                               =true;
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bContactHaveOffset                                              =false; //ChungHung 20140327 add by Customer
    IniConfig.bTestIcCheckInContact                                             =false; //ChungHung 20140327 add by Customer
    IniConfig.bQAMode                                                           =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    CosFunction.bEnable_SECS_GEM                                                =true;  //Steven 20150224
    IniConfig.bIndexDropOnlySKIP                                                =true;  //jou 2012-02-13 index drop error only skip
    IniConfig.bIndexPickErrOnlySKIP                                             =true;  //jou 2012-02-13 index pick-up error only skip
    CosFunction.bBin1CanNotInFix                                                =true;  //Steven 20150427 : Bin1不能放到Fix盤
    CosFunction.bResetModeIncludeCleanOut                                       =true;  //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
    IniConfig.bShowLotInfo                                                      =true;  //JerryYang 20160422 : Show Lot Info
    CosFunction.bInitTestHeadByTestSiteUse                                      =true;  //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
    CosFunction.bRTCAutoModelVerify                                             =true;  //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bUseLoginDatToSetLevel                                          =true;
    CosFunction.bRTC_ROICount                                                   =true;  //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bLockRTC                                                        =true;  //JerryYang 20160223 add for Philippine,切換為Online時強制開啟RTC
    CosFunction.bColorSensorUseCCLink                                           =true;
    CosFunction.bRTCHalfViewAutoVerify                                          =true;  //JerryYang 20220215 : RTC Auto Verify half view check
    CosFunction.bQAModeUseUnloadCnt                                             =true;  //JerryYang 20221005 : QA mode count 用BIN1數量計數
    CosFunction.bTemp5PointKitOffset                                            =true;  //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
    CosFunction.bYieldAlmNeedOneCycle                                           =true;  //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bTowerLightUseLD                                                =true;  //JerryYang 20230605 : 沒裝ART的時候tower light可使用LD/ULD
    CosFunction.bUseMDB                                                         =true;  //Steven 20210526 : 部分客戶取消使用MDB
    CosFunction.bInspectSuckICWhenSoftStart                                     =false;
}
//------------------------------------------------------------------------------
void FUNC_CC_Advantest_GE()                                                             //926 Advantest-GE 德國
{
    IniConfig.bQAMode                                                           =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_XINITECH()                                                                 //北京芯力
{
    CosFunction.bEnable_SECS_GEM                                                =true;
    CosFunction.bCanRemoteStart                                                 =true;  //Steven 20250707 : Add //Steven 20250712 : 有通過風險告知可以遠端start的客戶
}
//------------------------------------------------------------------------------
void FUNC_CC_ChipOn()                                                                   //上海芯旺
{
    IniConfig.bUseAutoSiteMapping                                               =true;  //Auto site mapping
    CosFunction.bAutoSiteMappingUseHotPlate                                     =true;  //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUSEJCETSiteMapMode                                             =true;  //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bEnableRMS                                                      =true;  //Jimmychiu 20241213 : add coustomer func EnableRMS
    IniConfig.bEnableAutoCleanFunction                                          =true;  //Steven 20110528 : 開啟Auto Clean功能
    CosFunction.bScanBarcodeAndDownloadFileInRMS                                =true;  //Jimmychiu 20241226 : Scan Barcode And Download File In RMS
    IniConfig.bShowFTandRTButton                                                =true;  //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =true;  //Steven 20131224 : FT & RT Buttion 可以按
}
//------------------------------------------------------------------------------
void FUNC_CC_Allegro_Philippines()
{
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bHaveRotateShuttle                                                =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
    IniConfig.bQAMode                                                           =true;
    IniConfig.bCanByPassIonFan                                                  =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bStopMustTestTimeOut                                            =true;  //Steven 20111003 : 暫停時也要計算Tester Time Out
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bResetCanServoOff                                                 =true;  //Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    CosFunction.bAutoKTemp                                                      =true;  //Steven 20120719 : 自動K溫
    IniConfig.bEnableKT4HAlarm1                                                 =true;  //Steven 20120809 : KT4H使用Alarm1作加熱保護
    CosFunction.bEnable2x1Site                                                  =true;  //Steven 20120814: 支援2x1Site模式
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNewResetFunction                                                 =true;  //Steven 20130625 : 新的Reset方式
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bInOutArmCanPushHome                                              =true;
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150128 : KK說要開啟使用
    CosFunction.bUseGPIB_V108Command                                            =true;  //Steven 20150901 : For GPIB V1.08 Command
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bTrayOCR                                                        =true;
    CosFunction.bESDAutoDecayTeachFunction                                      =true;  //Ifor 20151222 :新增 ESD Auto Decay Teach Function
    CosFunction.bUseTrayUpDownSet                                               =true;  //wei 20160224 TSMC FIX UPDOWN  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20160303 : FIFO Mode
    CosFunction.bUseMonitorView                                                 =true;  //Ifor 20160930 add USE MonitorView 錄影監視功能顯示
    CosFunction.bUseFix3FullTray                                                =true;  //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
    CosFunction.bTTLCanUse8Site                                                 =true;  //Alick 20161011 (Steven) : TTL支援8Site
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 (wei) : For SCK 93K ART
    if(fSCKART!=NULL && fSCKART->iTesterType==1)                                        //Steven 20161201 (wei) : For SCK 93K ART
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;

    CosFunction.bInShuttleDetectByLatch                                         =true;  //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    CosFunction.bRecipeParameterDefault                                         =true;  //Isaac 20170527 (Steven) defalut值比較功能
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bColorSensorUseCCLink                                           =true;  //wei 20180409 color sensor
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bNonCenterModeCanUseShtOffset                                   =true;
    CosFunction.b1x4OnlyAaUse1x1Mode                                            =true;
    CosFunction.bEnableMulti2D                                                  =true;  //Steven 20200810 : 一個IC使用多個2DID
    CosFunction.bIOTriggerIonFanAutoClean                                       =true;  //Isaac 20210609 : IO觸發IonFan清針
    CosFunction.bIndexCheckCanTurnOff                                           =true;  //Isaac 20211019 : 可選擇做index check的時機
    IniConfig.bAutoTrayLink                                                     =true;
    CosFunction.bTrayDeviceCheckFromLoader                                      =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bTrayMapFromLoader                                              =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bATCUseHandlerRecipeName                                        =true;  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    CosFunction.bTemp5PointKitOffset                                            =true;  //Ztex 2024.02.21 Add For QC Mode
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ztex 2024.02.21 Add For QC Mode
    CosFunction.bEnableAutoSpeed                                                =true;
    IniConfig.bInitialStartDelayCount                                           =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_UTAC_Indonesia()                                                   //聯測科技 印尼
{
}
//------------------------------------------------------------------------------
void FUNC_CC_Elmos_Germany()
{
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bHaveRotateShuttle                                                =true;
    IniConfig.bEnableCCDUSETCPIP                                                =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
    IniConfig.bIndexEveryTimeCheckEP                                            =true;  //Index每一次都確認EP是否有充飽氣。
    IniConfig.bQAMode                                                           =true;
    IniConfig.bCanByPassIonFan                                                  =true;
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File
    CosFunction.bStopMustTestTimeOut                                            =true;  //Steven 20111003 : 暫停時也要計算Tester Time Out
    IniConfig.bIndexDropNeedPwdByIni                                            =true;
    IniConfig.bResetCanServoOff                                                 =true;  //Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode
    CosFunction.bAutoKTemp                                                      =true;  //Steven 20120719 : 自動K溫
    IniConfig.bEnableKT4HAlarm1                                                 =true;  //Steven 20120809 : KT4H使用Alarm1作加熱保護
    CosFunction.bEnable2x1Site                                                  =true;  //Steven 20120814: 支援2x1Site模式
    CosFunction.bMES0101CanCleanOut                                             =true;  //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bNewResetFunction                                                 =true;  //Steven 20130625 : 新的Reset方式
    CosFunction.bContactShowOffset                                              =true;  //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    IniConfig.bInOutArmCanPushHome                                              =true;
    CosFunction.bEnable6Site                                                    =true;  //ChungHung 20140115 add for 2x3_6
    IniConfig.bRecordSkipPosition                                               =true;  //Steven 20150128 : KK說要開啟使用
    CosFunction.bUseGPIB_V108Command                                            =true;  //Steven 20150901 : For GPIB V1.08 Command
    CosFunction.bStartESDAutoDecayFunction                                      =true;  //Ifor 20150924 :Start ESD Auto Decay Test Function
    CosFunction.bTrayOCR                                                        =true;
    CosFunction.bESDAutoDecayTeachFunction                                      =true;  //Ifor 20151222 :新增 ESD Auto Decay Teach Function
    CosFunction.bUseTrayUpDownSet                                               =true;  //wei 20160224 TSMC FIX UPDOWN  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    CosFunction.bHaveFIFOMode                                                   =true;  //Steven 20160303 : FIFO Mode
    CosFunction.bUseMonitorView                                                 =true;  //Ifor 20160930 add USE MonitorView 錄影監視功能顯示
    CosFunction.bUseFix3FullTray                                                =true;  //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
    CosFunction.bTTLCanUse8Site                                                 =true;  //Alick 20161011 (Steven) : TTL支援8Site
    CosFunction.bUseSCKART                                                      =true;  //Steven 20161201 (wei) : For SCK 93K ART
    if(fSCKART!=NULL && fSCKART->iTesterType==1)                                        //Steven 20161201 (wei) : For SCK 93K ART
        CosFunction.bAutoRetestGPIBmode                                         =true;
    else
        CosFunction.bAutoRetestGPIBmode                                         =false;

    CosFunction.bInShuttleDetectByLatch                                         =true;  //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    CosFunction.bRecipeParameterDefault                                         =true;  //Isaac 20170527 (Steven) defalut值比較功能
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =true;  //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =true;  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bColorSensorUseCCLink                                           =true;  //wei 20180409 color sensor
    CosFunction.b16SiteCloseSiteRun2x4                                          =true;  //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bCanUse2x2NNMode                                                =true;  //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =true;  //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =true;  //Wei 20231211 : 2X4NN Mode
    CosFunction.bNonCenterModeCanUseShtOffset                                   =true;
    CosFunction.b1x4OnlyAaUse1x1Mode                                            =true;
    CosFunction.bEnableMulti2D                                                  =true;  //Steven 20200810 : 一個IC使用多個2DID
    CosFunction.bIOTriggerIonFanAutoClean                                       =true;  //Isaac 20210609 : IO觸發IonFan清針
    CosFunction.bIndexCheckCanTurnOff                                           =true;  //Isaac 20211019 : 可選擇做index check的時機
    IniConfig.bAutoTrayLink                                                     =true;
    CosFunction.bTrayDeviceCheckFromLoader                                      =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bTrayMapFromLoader                                              =true;  //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bATCUseHandlerRecipeName                                        =true;  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    CosFunction.bTemp5PointKitOffset                                            =true;  //Ztex 2024.02.21 Add For QC Mode
    CosFunction.bATCUseTempAdjustment                                           =true;  //Ztex 2024.02.21 Add For QC Mode
    CosFunction.bEnableAutoSpeed                                                =true;
    IniConfig.bInitialStartDelayCount                                           =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_Realtek()                                                          //瑞昱半導體
{
    IniConfig.bShowFunctionWindow                                               =true;
    IniConfig.bResetCanServoOff                                                 =true;
    IniConfig.bUseAutoSiteMapping                                               =true;
    IniConfig.bEnableAutoCleanFunction                                          =true;
}
//------------------------------------------------------------------------------
void FUNC_CC_HTML_Monitor()
{
    CosFunction.bOLPFunction                                                    =true;  //Steven 20141229 : OLP功能
}
//------------------------------------------------------------------------------
void FUNC_CC_HuaTian_NNAJING()
{
}
//------------------------------------------------------------------------------
void FUNC_CC_SFASemicon()                                                       //SFA Semicon
{
    IniConfig.bKoreaFunction                                                    =true;  //Steven 20110831 : 韓國代理商的需求
}
//------------------------------------------------------------------------------
void DoCustomerFunction()
{
    switch(CUSTOMER_CODE)
    {
        case CC_HONPREC_QC:                 FUNC_CC_HONPREC_QC();               break;
        case CC_SPIL_SHINCHU:               FUNC_CC_SPIL_SHINCHU();             break;
        case CC_SPIL_CHINA_SUZHOU:          FUNC_CC_SPIL_CHINA_SUZHOU();        break;
        case CC_SPIL_TAICHUNG_LOGIC:        FUNC_CC_SPIL_TAICHUNG_LOGIC();      break;
        case CC_SPIL_CS:                    FUNC_CC_SPIL_CS();                  break;//KevinCheng 20251002 add 矽品中山
        case CC_KYEC_JCTHIU:                FUNC_CC_KYEC_JCTHIU();              break;
        case CC_KYEC_CHEN:                  FUNC_CC_KYEC_CHEN();                break;
        case CC_KYEC_LEE:                   FUNC_CC_KYEC_LEE();                 break;
        case CC_DL_TEK:                     FUNC_CC_DL_TEK();                   break;
        case CC_KYEC_XILINX:                FUNC_CC_KYEC_XILINX();              break;
        case CC_KYEC_STM:                   FUNC_CC_KYEC_STM();                 break;
        case CC_ASE_SG:                     FUNC_CC_ASE_SG();                   break;
        case CC_ASE_JP:                     FUNC_CC_ASE_JP();                   break;
        case CC_ASE_Korea:                  FUNC_CC_ASE_Korea();                break;
        case CC_ASE_CL:                     FUNC_CC_ASE_CL();                   break;
        case CC_ASE_SH:                     FUNC_CC_ASE_SH();                   break;
        case CC_ASE_N:                      FUNC_CC_ASE_N();                    break;
        case CC_ASE_M:                      FUNC_CC_ASE_M();                    break;
        case CC_ASE_KaohSiung_K3:           FUNC_CC_ASE_KaohSiung_K3();         break;
        case CC_ASE_KaohSiung_K11:          FUNC_CC_ASE_KaohSiung_K11();        break;
        case CC_ASE_KaohSiung_K12:          FUNC_CC_ASE_KaohSiung_K12();        break;
        case CC_ASE_KaohSiung:              FUNC_CC_ASE_KaohSiung();            break;
        case CC_ASE_KS:                     FUNC_CC_ASE_KS();                   break;
        case CC_UTAC:                       FUNC_CC_UTAC();                     break;
        case CC_SIGURD_ChungXing:           FUNC_CC_SIGURD_ChungXing();         break;
        case CC_SIGURD_PeiXing:             FUNC_CC_SIGURD_PeiXing();           break;
        case CC_SIGURD_HUKOU:               FUNC_CC_SIGURD_HUKOU();             break;
        case CC_SCS:                        FUNC_CC_SCS();                      break;
        case CC_SCC:                        FUNC_CC_SCC();                      break;
        case CC_JSCC_OS:                    FUNC_CC_JSCC_OS();                  break;  //長電微電子 (JSCC OS部門)
        case CC_SCK:                        FUNC_CC_SCK();                      break;
        case CC_APTOS:                      FUNC_CC_APTOS();                    break;
        case CC_WINBOND:                    FUNC_CC_WINBOND();                  break;
        case CC_G_Link:                     FUNC_CC_G_Link();                   break;
        case CC_DoosanTesna:                FUNC_CC_DoosanTesna();              break;
        case CC_AMKOR:                      FUNC_CC_AMKOR();                    break;
        case CC_AMKOR_China:                FUNC_CC_AMKOR_China();              break;
        case CC_AMKOR_Japan:                FUNC_CC_AMKOR_Japan();              break;
        case CC_AMKOR_Korea:                FUNC_CC_AMKOR_Korea();              break;
        case CC_AMKOR_Philippines:          FUNC_CC_AMKOR_Philippines();        break;
        case CC_ATMEL:                      FUNC_CC_ATMEL();                    break;
        case CC_RFMD_BEIJING:               FUNC_CC_RFMD_BEIJING();             break;
        case CC_RFMD_USA:                   FUNC_CC_RFMD_USA();                 break;
        case CC_GIGA:                       FUNC_CC_GIGA();                     break;
        case CC_LINGSEN:                    FUNC_CC_LINGSEN();                  break;
        case CC_Greatek:                    FUNC_CC_Greatek();                  break;
        case CC_THAILIN:                    FUNC_CC_THAILIN();                  break;
        case CC_JCET:                       FUNC_CC_JCET();                     break;
        case CC_OSE:                        FUNC_CC_OSE();                      break;
        case CC_PTI:                        FUNC_CC_PTI();                      break;
        case CC_MTI:                        FUNC_CC_MTI();                      break;
        case CC_TERAPOWER:                  FUNC_CC_TERAPOWER();                break;
        case CC_TeraProbe:                  FUNC_CC_TeraProbe();                break;       //RogerYang 20260223 Add for TeraProbe
        case CC_GIGAS:                      FUNC_CC_GIGAS();                    break;
        case CC_CARSEM_M:                   FUNC_CC_CARSEM_M();                 break;
        case CC_UNISEM_M:                   FUNC_CC_UNISEM_M();                 break;
        case CC_CYPRESS:                    FUNC_CC_CYPRESS();                  break;
        case CC_GERADTECH_CHINA:            FUNC_CC_GERADTECH_CHINA();          break;
        case CC_GM_TEST:                    FUNC_CC_GM_TEST();                  break;
        case CC_I_TECH:                     FUNC_CC_I_TECH();                   break;
        case CC_WIN_PAC:                    FUNC_CC_WIN_PAC();                  break;
        case CC_RF360:                      FUNC_CC_RF360();                    break;
        case CC_QUALCOMM:                   FUNC_CC_QUALCOMM();                 break;
        case CC_BROADCOM_US:                FUNC_CC_BROADCOM_US();              break;
        case CC_SILICON_LABS_SG:            FUNC_CC_SILICON_LABS_SG();          break;
        case CC_SILICON_LABS_SZ:            FUNC_CC_SILICON_LABS_SZ();          break;
        case CC_TICP:                       FUNC_CC_TICP();                     break;
        case CC_THEIL:                      FUNC_CC_THEIL();                    break;
        case CC_Altera_USA:                 FUNC_CC_Altera_USA();               break;
        case CC_TSMC_TAINAN:                FUNC_CC_TSMC_TAINAN();              break;
        case CC_MAXIM:                      FUNC_CC_MAXIM();                    break;
        case CC_MAXIM_THAILAND:             FUNC_CC_MAXIM_THAILAND();           break;
        case CC_MARVELL:                    FUNC_CC_MARVELL();                  break;
        case CC_USI:                        FUNC_CC_USI();                      break;
        case CC_DYNACARD:                   FUNC_CC_DYNACARD();                 break;
        case CC_STK:                        FUNC_CC_STK();                      break;
        case CC_Renesas:                    FUNC_CC_Renesas();                  break;
        case CC_HUAWEI:                     FUNC_CC_HUAWEI();                   break;
        case CC_Microchip_Thai:             FUNC_CC_Microchip_Thailand();       break;
        case CC_Microchip_Phil:             FUNC_CC_Microchip_Philippines();    break;
        case CC_Microchip_China:            FUNC_CC_Microchip_China();          break;
        case CC_ChipMos_ZHUBEI:             FUNC_CC_ChipMos_ZHUBEI();           break;
        case CC_ANST:                       FUNC_CC_ANST();                     break;
        case CC_RICHTEK:                    FUNC_CC_RICHTEK();                  break;  //Alick 20161206 add 立錡
        case CC_TSI:                        FUNC_CC_TSI();                      break;  //誠遠科技
        case CC_ARDENTEC:                   FUNC_CC_ARDENTEC();                 break;  //欣詮
        case CC_FULCAP:                     FUNC_CC_FULCAP();                   break;  //詮容
        case CC_AOSL:                       FUNC_CC_AOSL();                     break;  //萬有半導體
        case CC_Spreadtrum:                 FUNC_CC_Spreadtrum();               break;  //展訊 上海
        case CC_UMC:                        FUNC_CC_UMC();                      break;  //聯電
        case CC_GT:                         FUNC_CC_GT();                       break;  //寰邦科技
        case CC_AMD_M:                      FUNC_CC_AMD_M();                    break;  //AMD
        case CC_VTEST:                      FUNC_CC_VTEST();                    break;  //無錫偉測半導體
        case CC_VTEST_Shanghai:             FUNC_CC_VTEST_Shanghai();           break;  //上海偉測半導體
        case CC_TFME_CHINA:                 FUNC_CC_TFME_CHINA();               break;
        case CC_ATEC:                       FUNC_CC_ATEC();                     break;  //艾科
        case CC_Murata:                     FUNC_CC_Murata();                   break;  //村田製作所
        case CC_HANA_MICRON:                FUNC_CC_HANA_MICRON();              break;
        case CC_CETC:                       FUNC_CC_CETC();                     break;  //中國電子-第58研究所
        case CC_ONSEMI_M:                   FUNC_CC_ONSEMI_M();                 break;  //ON SEMI Malaysia
        case CC_ONSEMI_CA:                  FUNC_CC_ONSEMI_CA();                break;  // ONSEMI_Canada
        case CC_Goertek:                    FUNC_CC_Goertek();                  break;  //歌爾微電子
        case CC_BOJIAN:                     FUNC_CC_BOJIAN();                   break;  //蘇州 博劍
        case CC_ETRENDTECH:                 FUNC_CC_ETRENDTECH();               break;  //逸昌科技
        case CC_DJI_SZ:                     FUNC_CC_DJI_SZ();                   break;  //大疆創新 深圳
        case CC_NXP_TJ:                     FUNC_CC_NXP_TJ();                   break;  //恩智浦 天津
        case CC_FMSH:                       FUNC_CC_FMSH();                     break;  //上海復旦微電子
        case CC_FOREHOPE_NINGBO:            FUNC_CC_FOREHOPE_NINGBO();          break;  //甬矽
        case CC_WINSTEK:                    FUNC_CC_WINSTEK();                  break;  //台星科
        case CC_HDSC:                       FUNC_CC_HDSC();                     break;  //華大半導體
        case CC_SANECHIPS:                  FUNC_CC_SANECHIPS();                break;  //中興微電子 深圳
        case CC_ITS:                        FUNC_CC_ITS();                      break;  //創量科技
        case CC_ITestInc:                   FUNC_CC_ITestInc();                 break;  //iTest,Inc. 美國
        case CC_EMemory:                    FUNC_CC_EMemory();                  break;  //EMemory 力旺電子
        case CC_CYUEAN:                     FUNC_CC_CYUEAN();                   break;  //CYUEAN 确安科技
        case CC_PANTHER:                    FUNC_CC_PANTHER();                  break;  //PANTHER 鴻谷科技
        case CC_SANAN:                      FUNC_CC_SANAN();                    break;  //三安光電
        case CC_HABANA:                     FUNC_CC_HABANA();                   break;  //HABANA
        case CC_UTAC_TW:                    FUNC_CC_UTAC_TW();                  break;  //聯測科技 台灣
        case CC_BARUN:                      FUNC_CC_BARUN();                    break;  //百齡
        case CC_SJ_Semiconductor:           FUNC_CC_SJ_Semiconductor();         break;  //盛合晶微
        case CC_SJ_Semiconductor_OS:        FUNC_CC_SJ_Semiconductor_OS();      break;  //盛合晶微Open Short
        case CC_SIGURD_SUZHOU:              FUNC_CC_SIGURD_SUZHOU();            break;  //KaiChen 20200506 ：新增客戶碼，矽格蘇州(SIGURD_SUZHOU)
        case CC_VATE:                       FUNC_CC_VATE();                     break;  //立衛科技
        case CC_ASIAOPTICAL:                FUNC_CC_ASIAOPTICAL();              break;  //亞洲光學
        case CC_HXYSEMI:                    FUNC_CC_HXYSEMI();                  break;  //浙江航源芯
        case CC_INTEL_IL:                   FUNC_CC_INTEL_IL();                 break;  //INTEL 以色列
        case CC_IBM_CANADA:                 FUNC_CC_IBM_CANADA();               break;  //IBM 加拿大
        case CC_XDXCT:                      FUNC_CC_XDXCT();                    break;  //象帝先
        case CC_XINYUN:                     FUNC_CC_XINYUN();                   break;  //杭州芯云
        case CC_THINE:                      FUNC_CC_THINE();                    break;  //Thine 日本
        case CC_CENTER:                     FUNC_CC_CENTER();                   break;  //中芯國際
        case CC_ISE_US:                     FUNC_CC_ISE_US();                   break;  //ISE 美國
        case CC_ISE_SH:                     FUNC_CC_ISE_SH();                   break;  //上海月芯半導體科技
        case CC_SINOICTECH:                 FUNC_CC_SINOICTECH();               break;  //上海華嶺
        case CC_HYGEIA_SUZHOU:              FUNC_CC_HYGEIA_SUZHOU();            break;  //海京 蘇州
        case CC_JSI_HAOXING:                FUNC_CC_JSI_HAOXING();              break;  //紹興長電
        case CC_LEADYO:                     FUNC_CC_LEADYO();                   break;  //廣東利揚
        case CC_NEXPERIA_Guangdong:         FUNC_CC_NEXPERIA_Guangdong();       break;  //廣州安世
        case CC_JSSI_Semiconductor:         FUNC_CC_JSSI_Semiconductor();       break;  //江蘇芯德半導體.
        case CC_AnalogDevice_Phil:          FUNC_CC_AnalogDevice_Phil();        break;  //ANALOG DEVICE
        case CC_Advantest_GE:               FUNC_CC_Advantest_GE();             break;  //926 Advantest-GE 德國
        case CC_CSAMQ:                      FUNC_CC_CSAMQ();                    break;  //csamq 長沙安牧泉
        case CC_RIGGER_MICRO:               FUNC_CC_RIGGER_MICRO();             break;  //鄭州銳傑微
        case CC_GONGJIN_SHANGHAI:           FUNC_CC_GONGJIN_SHANGHAI();         break;  //上海共進
        case CC_GONGJIN_SUZHOU:             FUNC_CC_GONGJIN_SUZHOU();           break;  //蘇州共進
        case CC_XINITECH:                   FUNC_CC_XINITECH();                 break;  //北京芯力
        case CC_ChipOn:                     FUNC_CC_ChipOn();                   break;  //上海芯旺
        case CC_Allegro_Philippines:        FUNC_CC_Allegro_Philippines();      break;  //Allegro菲律賓
        case CC_UTAC_Indonesia:             FUNC_CC_UTAC_Indonesia();           break;  //聯測科技 印尼
        case CC_Elmos_Germany:              FUNC_CC_Elmos_Germany();            break;  //Elmos Germany
        case CC_Realtek:                    FUNC_CC_Realtek();                  break;  //瑞昱半導體
        case CC_HTML_Monitor:               FUNC_CC_HTML_Monitor();             break;  //HTML_Monitor
        case CC_Carsem_Thai:                FUNC_CC_Carsem_Thai();              break;  //Carsem 泰國
        case CC_IFXTH_Thai:                 FUNC_CC_IFXTH_Thai();               break;  //IFXTH泰國
        case CC_CAPCON:                     FUNC_CC_CAPCON();                   break;  //北京華封
        case CC_Mellanox_Israel:            FUNC_CC_Mellanox_Israel();          break;  //Mellanox 以色列
        case CC_GOOGLE_Israel:              FUNC_CC_GOOGLE_Israel();            break;  //GOOGLE 以色列
        case CC_HuaTian_NNAJING:            FUNC_CC_HuaTian_NNAJING();          break;  //華天南京
        case CC_ChenYuanXiang_CHINA:        FUNC_CC_ChenYuanXiang_CHINA();      break;  //西安 晨元翔
        case CC_Ramos:                      FUNC_CC_Ramos();                    break;  //Ramos
        case CC_SFASemicon:                 FUNC_CC_SFASemicon();               break;  //SFA Semicon
        case CC_SCX:                             FUNC_CC_SCX();                   break;  //深測芯
    }

    if(CosFunction.bHiSiliconFunction)                                          //Steven 20151029 : 海思專用功能
    {
        CosFunction.bUploadTestArmTorqueLog                                     =true;  //Steven 20250609 : 統一整理
    }

    // AI(W906-SJSON-NOMDB) 20260923: ⛔ **SQLite 在這棵樹一律不使用。**
    //   使用者 20260923 裁決，原話兩句：
    //     「SQLite 資料庫已經不再使用, 只使用單純的 csv 存檔方式」
    //     「目前機台就是 CosFunction.bUseMDB=false, 沒有其他選項」
    //     「C++ 實作的時候, 也是強制為 false」
    //
    //   ⚠ **這是刻意偏離 golden，不是翻譯漏抄。**golden 是逐客戶決定的
    //     （`//Steven 20210526 : 部分客戶取消使用MDB`）—— 本檔的 switch 裡
    //     6 個客戶設 true、3 個設 false。移植樹把它**壓成單一值**。
    //
    //   ⚠ 位置是**整個 switch 跑完之後**，而且必須留在這裡：寫在 switch 之前，
    //     或寫進某個 FUNC_CC_* 裡，都會被後面的客戶碼函式覆蓋回 true，
    //     而且症狀是「換一台客戶機就又連回 sqlite」—— 換機才會出現的分岔
    //     是最難查的那一種。
    //
    //   ⓘ 後果（查證於 cMyDB.cpp，20260923）：golden 兩種守衛形狀
    //         if(CosFunction.bUseMDB==false) return 0;   // 早退（:175、:241-242）
    //         if(CosFunction.bUseMDB){ ... }             // 包起來（golden :614）
    //     在本樹共 13 處（golden 6 處 —— 移植樹的守衛**比 golden 多**，
    //     翻譯時沒有掉守衛）。旗標固定為 false 之後，19 個
    //     `sqlite3_get_table` / `sqlite3_exec` 呼叫點**一個都到不了**。
    //     ⇒ 留痕一律走 CSV。新寫的 DB 相關程式碼照上面兩種形狀之一加守衛。
    //
    //   ⚠ **不要**因此把 cMyDB.cpp 或 third_party/sqlite3/ 從建置移除 ——
    //     使用者指定的做法就是「參照 if(CosFunction.bUseMDB==false) 的方式」，
    //     程式碼留著、旗標關掉。移除是另一個決定，沒有人下過。
    CosFunction.bUseMDB                                                         =false;
}
//------------------------------------------------------------------------------
// funtion說明區
//------------------------------------------------------------------------------
void InitialCosFunction()
{
    #ifdef HiSilicon                                                                    //Ifor 20171116 海思功能define移動至最上面
        CosFunction.bHiSiliconFunction                                          =true;  //Steven 20151029 : 海思專用功能
    #else
        CosFunction.bHiSiliconFunction                                          =false; //Steven 20151029 : 海思專用功能
    #endif

    IniConfig.bInOutArmCanPushHome                                              =false; //In & Out Arm 吸取異常時，能按Home單獨歸In & out arm步進馬達
    IniConfig.bUseAutoSiteMapping                                               =false; //Auto site mapping
    IniConfig.bDutOnOffNeedASM                                                  =false; //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
    IniConfig.bIndexEveryTimeCheckEP                                            =false; //Index每一次都確認EP是否有充飽氣。
    IniConfig.bPowerSaveFunction                                                =false; //省電模式
    CosFunction.bFTPFunction                                                    =false; //FTP功能
    IniConfig.bEventLogAutoSaveFunction                                         =true;  //自動存EventLog
    IniConfig.bShowFunctionWindow                                               =false; //顯示在溫度值下面的功能開關畫面
    IniConfig.bHaveRotateShuttle                                                =false; //轉轉蝦頭
    IniConfig.bCanDisableTempMonitor                                            =false; //能夠關閉溫控器監視。
    IniConfig.bIndexJamInArmAway                                                =true;  //Index掉料時,In Arm要先移開才Alarm     //強制True, 不用的才關掉
    IniConfig.bOneCycleNeedPowerOff                                             =false; //OneCycle->Skip時要斷電
    IniConfig.bOneCycleCanTrayFeed                                              =false; //OneCycle時,要可以選擇Tray Feed
    IniConfig.bCleanOutCanTrayEnd                                               =false; //CleanOut後,可以選擇Tray End
    CosFunction.bCleanOutCanTrayEndBySetupFile                                  =false; //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    IniConfig.bEnableInOutArmPlaceSkipSuckDetect                                =true;  //skip in/out arm drop error     //Steven 20210914 : 開放成全部可選
    IniConfig.bEnableUnloadTrayFree                                             =false; //unload tray汽缸開門時也要可以打開
    IniConfig.bHighModeCanOffTemp                                               =false; //高溫模式可以關加熱
    IniConfig.bOneCycleDoQuickCleanOut                                          =false; //Steven 20110524 : OneCycle後,只做快速CleanOut
    CosFunction.bEnableAutoSpeed                                                =false; //Steven 20110526 : 開啟Auto Speed功能
    CosFunction.bEnableBarcodeReader                                            =false; //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
    IniConfig.bEnableCCDUSETCPIP                                                =false; //Steven 20110528 : 開啟CCD功能
    IniConfig.bEnableAutoCleanFunction                                          =false; //Steven 20110528 : 開啟Auto Clean功能
//    IniConfig.bTempOverCannotRun                                              =false; //Steven 20110607 : 溫度過高時,機器不能動
    CosFunction.bUseTrayUpDownSet                                               =true;  //這個變數改變會影響分BIN, 切換時要提醒客戶注意!!!
    IniConfig.bIndexDropNeedPwdByIni                                            =false; //Steven 20110627 : Index掉料時,需要輸入密碼,密碼放在D:\HT9045\system\SpecialErrNote.ini
    IniConfig.bChangeTempAutoSetDown                                            =true;  //Steven 20120910 : 強制開啟
    IniConfig.bDoorOpenShuttleContinueHeat                                      =false; //Steven 20110709 : Chamber門打開時不關閉加熱電源，只將Temp=0，Hotplate & shuttle除外。
    IniConfig.bLowYieldAlarmSameNS                                              =false; //jou    2011-07-16 : Low Yield Alarm模式與NS機台相同,skip會清除單獨Site.
    IniConfig.bOpenDoorNotStopFan                                               =false; //Steven 20110725 : Chamber門打開時不關閉風扇。
    IniConfig.bAlarmNeedServoOff                                                =true;  //Steven 20110802 : Alarm時,相關位置要可以Servo Off //Steven 20110829 全面開放
    IniConfig.b1x2Use4Suck                                                      =false; //Hung 20110812 : 1x2使用4吸嘴
    IniConfig.b1x4Use8Suck                                                      =false; //Hung 20110812 : 1x4使用8吸嘴
    IniConfig.b2x2Use8Suck                                                      =false; //Hung 20110812 : 2x2使用8吸嘴
    IniConfig.bHotPlateMove1CM                                                  =false; //Hung 20110812 : HotPlate使用1CM轉板
//  IniConfig.bEnableHotplateVibration                                          =false; //jou 2011-08-09 : Hotplate也要敲敲敲
    IniConfig.bKoreaFunction                                                    =false; //Steven 20110831 : 韓國代理商的需求
    IniConfig.bFinishSuckAfterPause                                             =false; //Hung 20110901 : finish suck and destroy 後才暫停
    IniConfig.bOnlyRoomOrHot                                                    =false; //Steven 20110814 : 只要兩種,不要Ambient/Hot模式
    IniConfig.bLastLoaderNoInSide                                               =false; //Steven 20110922 : 最後一盤不入料
    IniConfig.bRotateShNeedSensorCheck                                          =false; //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
    IniConfig.bTModeMotorFree                                                   =false; //Hung 20110923 : 開啟index T Mode時，開啟馬達煞車(測試驗證使用)
    IniConfig.bUseFix3                                                          =true;  //kevin 20110916 暫時性ic超過 15mm就不使用Fix3 tray
    IniConfig.bQAMode                                                           =false; //Steven 20111005 : QA模式
    IniConfig.bCanByPassIonFan                                                  =false; //Steven 20111013 : 可以不檢查離子風扇
    CosFunction.bLastSetInSetUpFile                                             =true;  //Steven 20111019 : 把LastSet選項存在Set Up File //JerryYang 20160426 強制開啟把LastSet選項存在Set Up File
    IniConfig.iTempeAlarmSecond_Over                                            =30;    //Steven 20111027 : 溫度過高的Alarm時間
    IniConfig.iTempeAlarmSecond_Below                                           =40;    //Steven 20111027 : 溫度過低的Alarm時間
    IniConfig.bLastLoaderAutoCleanOut                                           =true;  //Steven 20111028 : 搭配"最後一盤不入料"功能,要不要自動CleanOut
    IniConfig.bSiteMappingDisable                                               =false; //jou 2011-10-04 Site Mapping Disable，"Security_new.def", "Setup", "Machine Setup=1 || 4"
    IniConfig.bSiteMappingFastSetDisable                                        =false; //jou 2011-10-11 Site Mapping 快速設定Disable
    CosFunction.bOneCycleCanChangeArm                                           =false; //Hung 20111108 Amb/Hot Mode OneCycle Can Change Arm
    IniConfig.bAlarmMustRedColor                                                =false; //Steven 20111116 : 特殊Alarm需要改紅底
    CosFunction.bOffLineBin                                                     =false; //Hung 20111110 新增Offline Bin
    IniConfig.bTrayAssignUseGraphic                                             =false; //Steven 20111121 : 使用圖片去顯示Tray Assign
    IniConfig.bContactAlwaysIncludeShuttle                                      =false; //Steven 20090926 for KYEC 29818 : Never Enable and Always True
    IniConfig.bNoHeadaddChamberOption                                           =false; //Steven :Head + Chamber disable
    IniConfig.bDisibleResetButton                                               =false; //ChungHung 20111208 Disable Reset Button
    IniConfig.bShuttleMode50                                                    =false; //Dell 20111212   : 使用關Arm模式後,將被關的Arm測試高度設在-50mm
    IniConfig.bAbnormalStartCheck                                               =true;  //Steven 20111216 : 不正常關程式偵測
//    CosFunction.bEnableTimeOutCanSkip                                         =false; //Steven 20111220 : 測試TimeOut可以Skip
    IniConfig.bRetryNoNeedRestartGpib                                           =false; //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    IniConfig.bResetCanServoOff                                                 =false; //Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off
    IniConfig.bFix3PutAllFullIC                                                 =false; //ChungHung 20111215 Fix3盡量擺滿
    IniConfig.bDisableSelectSearchLast                                          =false; //jou 2012-01-10 取消Setup，Search Last Mode功能。
    IniConfig.bShuttleModeAccseeLevel                                           =false; //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    IniConfig.bAmbRunChamberFanCanStop                                          =false; //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    IniConfig.bIndexDropOnlySKIP                                                =false; //jou 2012-02-13 index drop error only skip
    IniConfig.bIndexPickErrOnlySKIP                                             =false; //jou 2012-02-13 index pick-up error only skip
    IniConfig.iUserLanguage                                                     =eulChinese;//Steven 20120203 : 使用者的語系
//    IniConfig.bOnlyUse30mmHead                                                =false; //Steven 20120203 : 只使用30mm的浮動頭
//    IniConfig.bIndexYAxisServoOff                                             =false; //jou 981214 Contect & OneCycle & CleanOut & Home 完成時，index Y axis servo off
    IniConfig.bIndexDropErrCanMove                                              =false; //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    IniConfig.bIndexPickupErrStop                                               =false; //jou 2012-02-29 index pick up error,index arm move to center & alarm
//    IniConfig.bP18FailAutoTrayManual                                          =false; //jou 2012-03-16 Fail Auto Tray手動補Tray
    IniConfig.bFTBin2RTBin                                                      =false; //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    IniConfig.bIOFormCanControlHeaterFan                                        =false; //Steven 20120607 : 當chamber門開啟時,需要能開關風扇
    IniConfig.bShowOffYieldBlink                                                =true;  //Steven 20120609 : 功能關畫面要不要閃爍
    IniConfig.bRTCbySystem                                                      =false; //ChungHung 20120716 RTC by System
    IniConfig.bDoorOpenHeadContinueHeat                                         =false; //ChungHung 20120719 add DoorOpenHeadContinueHeat
    IniConfig.bRemeberAutoHeight                                                =false; //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
//    IniConfig.DisableIndexCheck                                               =true;  //ChungHung 20120606 Disable IndexCheck     //Steven 20140529 : Mark
    IniConfig.bDownLoadAutoCountClear                                           =false; //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    IniConfig.bHeadSocketMode                                                   =true;  //jou 2012-05-30 增加 Head + Socket Mode    //Steven 20240201 : 機台標配, 全開
    IniConfig.bAutoTrayLink                                                     =false; //jou 2012-06-14 Auto Tray Link
    IniConfig.bIndexPickupWait                                                  =true;  //jou 2012-06-29 Index Pick up need wait Soak Time  //Steven 20160912 : 全面開放bIndexPickupWait
    CosFunction.bAutoKTemp                                                      =false; //Steven 20120719 : 自動K溫
    IniConfig.bEnableKT4HAlarm1                                                 =false; //Steven 20120809 : KT4H使用Alarm1作加熱保護
    IniConfig.bTesterTimeUpErrorNeedPassword                                    =false; //jou 2012-08-28 Tester Time up Error Need Password
    CosFunction.bMES0101CanCleanOut                                             =false; //Steven 20120830 : Loader吸取異常可以按CleanOut
    IniConfig.bResetClearArmIC                                                  =false; //Steven 20120830 : Reset按下時,把吸嘴上的IC清空
    IniConfig.bSingaporeFunction                                                =false; //Steven 20120910 : 新加坡代理商的需求
    IniConfig.bDualSiteCloseAbCanFullHotplate                                   =true;  //ChungHung 20120911 add 與OneCycle can desable site 衝突  //Steven 20161213 : 改用判斷Site Ba是不是等於 Site 0 方式
    IniConfig.bResetPutUntestToErrorBin                                         =false; //Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin
    IniConfig.bIndexArm2SupplyLight                                             =false; //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
//    IniConfig.bResetPortCOM1                                                    =false; //jou 2012-10-24 Reset Port COM1
    IniConfig.bDualSiteSupply4CH                                                =false; //jou 2012-11-20 Dual Site supply 4's Channel
    IniConfig.bInitialStartDelayCount                                           =false; //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    IniConfig.bSocketCommunication                                              =false; //ChungHung 20130112 add for ASE_KR Socket Tester
    IniConfig.bAnyLevelCanGetStateRecode                                        =false; //ChungHung 20120922 add
    IniConfig.bBinBox                                                           =false; //jou 2012-12-11 support Bin Box
    IniConfig.bPasswordSecret                                                   =false; //jou 2013-01-04 Password Txt 加密
//    IniConfig.bShowLotInfo                                                    =false; //jou 2013-01-18 Show Lot Info
    IniConfig.bShowMainDebugRecord                                              =false; //jou 2013-02-25 Show Main form Debug Record
    IniConfig.bShowTrayAndDeviceDir                                             =false; //jou 2013-03-25 show Tray & Device Direction
    IniConfig.bShowFTandRTButton                                                =false; //jou 2013-04-27 Show FT & RT Buttion
    IniConfig.bShowFTandRTButtonCanClick                                        =false; //Steven 20131224 : FT & RT Buttion 可以按
//    IniConfig.bRecordSkipPosition                                             =false; //jou 2013-05-30 Record Skip position
    IniConfig.bNewResetFunction                                                 =false; //Steven 20130625 : 新的Reset方式
    IniConfig.bEnableStepShuttle                                                =false; //jou 2013-07-16 Step Shuttle check Index -> Input
    IniConfig.bNoTrayAutoCleanOut                                               =false; //jou 2013-08-01 Loader No Tray Auto Clean
    if(ATC_SYSTEM==eATCUninstall)                                                       //Steven 20240206 : 有Chember才可以開Head+Chamber+Socket
        IniConfig.bHeadChamberSocketMode                                        =true;  //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
    else
        IniConfig.bHeadChamberSocketMode                                        =false;
    IniConfig.bUseAutoOffsetFunction                                            =false; //jou 2013-08-29 Use Auto Offset Funtion
    IniConfig.bEnableTestingNeedStopAllMotor                                    =false; //jou 2013-09-25 Testing Need Stop All Motor
    IniConfig.bControlTorque                                                    =false; //jou 2013-11-05 Index Control Torque
    IniConfig.bShowFormByInitPos                                                =false; //jou 2013-12-02
    IniConfig.bIndexDropOnlyReset                                               =false; //jou 2013-12-02 Index Drop Only Reset
    IniConfig.bOutShLoseNeedOpenChamber                                         =false; //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
    IniConfig.bFTPJamCodeUpload                                                 =false; //ChungHung 20140108 add FTP unload jam code
    IniConfig.bUseTrayBlockMode                                                 =false; //2014-03-04    Dell    for SPIL WLP Add Tray Block
    IniConfig.bTestIcCheckInContact                                             =true;  //ChungHung 20140327 add by Customer
    CosFunction.bContactHaveOffset                                              =true;  //ChungHung 20140327 add by Customer
//    IniConfig.bStartProductOnLine                                             =false; //kevin 20140407 生產前OP OFF_LINE 強制 On line     //kevin 20180517 mark
    CosFunction.bContactShowOffset                                              =false; //Steven 20140409 : 矽品要求Contact畫面顯示Offset
    CosFunction.bShowLastContactHigh                                            =false; //Steven 20140409 : 顯示之前的Contact高度
    IniConfig.bHaveRTCCheckSiteMap                                              =false; //Steven 20140513 : [D35]
    CosFunction.bHaveIndexContactDelay                                          =false; //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
    IniConfig.bASE_Report                                                       =false; //kevin 20140918 高雄日月光IC履歷記錄
//    IniConfig.bAmbientTempControl                                             =false; //kevin 20140918 常溫恆溫控制
    if(CUSTOMER_CODE!=CC_ASE_CL)
    {
        IniConfig.bRecordSkipPosition                                           =false;
    }
    IniConfig.bTemp25degControl                                                 =false; //jou 2014-06-07 Temperature 25 deg. control
    CosFunction.bJAM0303NeedOpenChamberDoor                                     =true;  //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒    //Steven 20171204 (Wei) : must be true

    CosFunction.bUseLoadCellOffsetByHeater                                      =false; //2014-06-26    Dell    高溫Load cell offset

    CosFunction.bIndexZDownToAboveSocket                                        =false; //Eliot 2015-02-25 Initial Check時手臂只下降到Above Socket Offset位置,避免壓壞Socket pogoPin

    CosFunction.bEnable6Site                                                    =false; //ChungHung 20140115 add for 2x3_6
    CosFunction.bEnableOctal_12Kit                                              =false; //ChungHung 20140508 add 使用12 layout kit Octal Shutle kit
    CosFunction.bShowLastShuttleHigh                                            =false; //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
    CosFunction.bChamberModeEvenBlowNeedWaitTime                                =false; //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
    CosFunction.bLoseDeviceOutShuttleServoOff                                   =false; //ChungHung 20140522 add OutShuttle lose devices can servo off
    CosFunction.bEnableAfterTestedDelay                                         =false; //ChungHung 20140730 add for ATK function after tested delay time
//    CosFunction.bContinueAutoSkipAutoTrayEnd                                  =false; //ChungHung 20141014 add for KYEC AutoRetest
    CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen                        =true;  //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion   //Steven 20190717 : 強制開啟
    CosFunction.bBuilderImportSingleFolder                                      =false; //ChungHung 20150414 add for TSMC
//    CosFunction.bSendJamCodeByTcpIp                                           =false; //ChungHung 20150518 add for SCK Send JamCode By TcpIp        //Steven 20161201 : Mark of SCK no use TCP IP
    CosFunction.bOneCycleCanChangeContinuesFailBin                              =false; //ChungHung 20150519 add Continues Failure BIN can select after "OneCycle".
    CosFunction.bAutoSetContFailAfterInitialStart                               =false; //ChungHung 20150519 add Auto Set ContFail After InitialStart.
    CosFunction.bManualProcess                                                  =false; //ChungHung 20150526 add for QualComm US
    CosFunction.bTopPogoPinMode                                                 =false; //ChungHung 20150528 add for 海思 _8Site1x4
    CosFunction.bEnableSoftWareControlButton                                    =false; //ChungHung 20150609 add only for TSMC

    CosFunction.bStopMustTestTimeOut                                            =true;  //Steven 20111003 : 暫停時也要計算Tester Time Out   //Steven 20211213 : bStopMustTestTimeOut改成全面開啟
    CosFunction.bEnable12Site                                                   =true;  //Steven 20120813 : 支援12Site模式
    CosFunction.bEnable2x1Site                                                  =false; //Steven 20120814 : 支援2x1Site模式
    CosFunction.bOnlyUseContactModeDiffSpeed                                    =false; //Steven 20131101 : 強制使用兩段速Contact
    CosFunction.bFixedDropSpeed                                                 =false; //Steven 20131101 : 使用固定的Drop速度
    CosFunction.iFixedDropSpeed                                                 =30;    //Steven 20131101 : 使用固定的Drop速度
    CosFunction.bLimitMaxSpeed                                                  =false; //Steven 20131101 : 限制最高速度
    CosFunction.dLimitMinDropOffset                                             =0.0;   //Steven 20131101 : 限制最小距離
    CosFunction.iLimitMaxSpeed                                                  =100;   //Steven 20131101 : 限制最高速度
    CosFunction.bPiggyBackForASE                                                =false; //Steven 20131101 : 高雄ASE不要Continual Pass Bin(Total )跟 Continual Loader兩種
    CosFunction.bPiggyBackShowMainForm                                          =false; //Steven 20131101 : PiggyBack數量到達時,顯示在Main Form上面
    CosFunction.bAutoCleanOnHotPlate                                            =false; //Steven 20131101 : 將Clean Pad放在HotPlate上,也就是高雄版
    CosFunction.bEnableOctal_16Kit                                              =false; //Steven 20140312 : 8Site使用16Site Kit
    CosFunction.bAfterRTChangeToInitialStart                                    =false; //Steven 20140521 : RT後自動切回FT
    CosFunction.bTechComUseComboBox                                             =false; //Steven 20140530 : 使用密碼本時,用下拉選單選使用者
    CosFunction.bLockF06ByFile                                                  =false; //Steven 20140627 : Add for ASE-CL
    CosFunction.bLockD41ByFile                                                  =false; //Steven 20140627 : Add for ASE-CL
    CosFunction.bUseNew12Site                                                   =false; //Steven 20140716
    CosFunction.bUseLowYieldAlarmByBin                                          =false; //Steven 20140828 : By Bin Yield Monitor
    CosFunction.bYieldAlarmClearAllCount                                        =true;  //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
    CosFunction.bTemperatureMax130                                              =false; //Steven 20140911 : Xilinx最高只要130度
    CosFunction.bArm2ForFingerPrintTest                                         =false; //Steven 20140922 : Arm2當作指紋測試
    CosFunction.bUseIndividulTempSet                                            =false; //Steven 20140924 : 各個加熱區獨立有自己的設定值
    CosFunction.bRCMDStart                                                      =false; //Steven 20141006 : SECS GEM使用Remote Start功能
    CosFunction.bIndexAreaOnlyCanUseSkip                                        =false; //Steven 20141105 : Index內的所有異常都只能用Skip
    CosFunction.bYieldControlUseContactCount                                    =false; //Steven 20141212 : Yield控制使用Contact Count
    CosFunction.bOLPFunction                                                    =false; //Steven 20141229 : OLP功能
    CosFunction.bRTCFullViewErrorOnlyRetry                                      =true;  //Steven 20150304 : bRTCFullViewErrorOnlyRetry預設改為True
    CosFunction.bBin1CanNotInFix                                                =false; //Steven 20150427 : Bin1不能放到Fix盤
    CosFunction.bAmbientNoShowTemp                                              =false; //Steven 20150518 : 蘇州矽品要求常溫不顯示溫度
    CosFunction.b12SiteCloseSiteRun2x4                                          =false; //Steven 20150721 : 12Site關Site跑2x4 Mode
    CosFunction.b16SiteCloseSiteRun2x4                                          =false; //Steven 20191124 : 16Site關Site跑2x4 Mode
    CosFunction.bManualSortMode                                                 =false; //Steven 20150915 : For TSMC 手動整盤功能
    CosFunction.bOutShuttleSensorCanNotDisable                                  =false; //Steven 20151202 : Out Shuttle Sensor不能關閉檢查
    CosFunction.bHaveFIFOMode                                                   =false; //Steven 20160303 : FIFO Mode

    CosFunction.bEnable_SECS_GEM                                                =false; //jou 2012-03-12 Enable SECS_GEM
    CosFunction.bSecurityHave5Level                                             =false; //jou 2014-06-19 Security Have 5 Level
    CosFunction.bRTCAutoModelVerify                                             =false; //jou 2014-06-24 RTC 自動進行Model驗證
    CosFunction.bSiteCmpYield                                                   =false; //jou 2014-08-14 Site Compare Low Yield alarm
    CosFunction.bFuncStateStopFirtDelay                                         =false; //jou 2014-09-03 Function State Stop Firt Initital Delay Time
    CosFunction.bConAlarmNeedKeyInPassword                                      =false; //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password
    CosFunction.bConAlarmInTimeLevelUp                                          =false; //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    CosFunction.bShowHandlerStopTime                                            =false; //jou 2014-09-21 Show Handler Stop Time
    CosFunction.bTesterLowYieldOneCycle                                         =false; //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
    CosFunction.bConFailFoolProofing                                            =false; //jou 2014-09-28 continue fail 防呆提醒,總開關開啟為On時,BIN別未勾選任何continue fail alarm
    CosFunction.bQAmodeSupplyTrayDir                                            =false; //jou 2014-10-24 QA mode 增加支援吸Tray方向
    CosFunction.bInitTempOffsetByWorkFile                                       =false; //jou 2015-06-13 Initial Temperture Offset by WorkFile;

    CosFunction.bEPUseNSSLK                                                     =false; //wei 20150303   京元NS浮動頭
    CosFunction.bDisableRTBinSet                                                =false; //wei 20150622 不顯示設定RT Bin set

    CosFunction.bYieldAlarmNoWait1Min                                           =false; //wei 20150820  Yield Alarm No Wait 1Min
    CosFunction.bSECS_GEM_OneCycle                                              =false; //wei 20150824 Secs_Gem 斷線Onecycle
    CosFunction.bAutoCleanAutoSelIndexArm                                       =false; //jou 2015-08-12 Auto Clean Auto Select Index Arm
    CosFunction.bAutoCleanShuttleDisable                                        =false; //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    CosFunction.bResetModeIncludeCleanOut                                       =false; //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
    CosFunction.bInitTestHeadByTestSiteUse                                      =false; //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
    CosFunction.bTrayOCR                                                        =false; //wei 20150926 TSMC Tray Arm OCR
    CosFunction.bRecipeCheck                                                    =false; //JerryYang 20151028 add for Amkor 如果download的工作檔不一樣,要求做Height Calibration後才能start
    CosFunction.bLowYeildByTotal                                                =false; //wei 20151116 Low Yield By Total
    CosFunction.bForceSetAuto3Fix3SideHasNullIC                                 =false; //JerryYang 20151120 add for 京元Xilinx, IC 大於 40x40mm時會敲到,Auto3和Fix3最右邊要設為 HasNullIC
    CosFunction.bSortingBinTraywhenCleanOut                                     =false; //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
    CosFunction.bNotClearAllHotBuffer                                           =false; //JerryYang 20151226 For 矽格 由GPIB設定完溫度後，會馬上再問一次溫度。所以不清除暫存溫度
    CosFunction.bDownloadRecipeLevelMode                                        =false; //jou 2016-01-06 download recipe 增加權限模式選擇
    CosFunction.bLockD42ByFile                                                  =false; //JerryYang 20160220 add for Amkor-Philippine
    CosFunction.bLockD43ByFile                                                  =false; //JerryYang 20160220 add for Amkor-Philippine
    CosFunction.bLockD44ByFile                                                  =false; //JerryYang 20160220 add for Amkor-Philippine
    CosFunction.bLockRTC                                                        =false; //JerryYang 20160220 add for Amkor-Philippine
    CosFunction.bPickupErrorAtLoaderNeedOpenDoor                                =false; //JerryYang 20160220 add for Amkor-Philippine,In ARM至Loader吸取異常要開門確認後才能繼續
    CosFunction.bLockF19AlwaysOn                                                =false; //JerryYang 20160225 add for Amkor-Philippine
    CosFunction.bOutShuttleLoseICNeedHome                                       =false; //JerryYang 20160225 add for Amkor-Philippine
//    CosFunction.bUseTrayUpDownSet                                             =false; //wei 20160224 TSMC FIX UPDOWN  //Mark
    CosFunction.bUsePMAlarmFunction                                             =false; //wei 20160225 PMAlarmFunction
    CosFunction.bJAM0301NeedOpenChamberDoor                                     =false; //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
    CosFunction.bAutoRetestGPIBmode                                             =false; //jou 2015-10-02 Auto Retest GPIB mode
    CosFunction.iAutoRetestTCPmode                                              =0;     //Sam 20191113 : TCP ART
    CosFunction.bKnockerSetBySetupFile                                          =false; //Steven 20160329 : 敲擊汽缸參數調整可搭配工作檔處理
    CosFunction.bContactTestVacOffByCloseSite                                   =false; //JerryYang 20160328 Contact test及auto height時,關site的部分不吸取IC
    //AI(ht9045-v899) 20260421: 全智(GIGAS)專用，預設關閉，由 FUNC_CC_GIGAS 開啟；Contact/Auto Height 加強掉料偵測
    CosFunction.bContactTestICDropGuard                                         =false;
    CosFunction.bContactTestWaitSoakTime                                        =false; //wei 20160329 Contact Test Wait SoakTime
    CosFunction.bFullTestBeforeAutoClean                                        =true;  //Sam 20230517 : 全部開啟 //JerryYang 20160331 Auto Clean之前要先做Full view check
    CosFunction.bYieldAlmNeedOneCycle                                           =false; //JerryYang 20160407 Yield相關Alarm要先做完one cycle才show alarm
    CosFunction.bContinueFailNeedAlarmDirectly                                  =false; //Steven 20230831 : 當使用bYieldAlmNeedOneCycle, 連續Fail要直接Alarm
    CosFunction.bG09NeedPasswordWhenEditSiteMap                                 =false; //JerryYang 20160425 修改Site map需要密碼
    CosFunction.bLockP24ByFile                                                  =false; //JerryYang 20160425 lock P24
    CosFunction.bYieldAlarmUseDouble                                            =false; //JerryYang 20160615 Yield相關limit可以設定到小數點
    CosFunction.bFTRTDifferentDutOnOff                                          =false; //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
    CosFunction.bCheckInOutArmZHomeSensor                                       =false; //Frank 20160612 add Check In/Out Arm Home Sensor Function
    CosFunction.bUseERMS                                                        =false; //Steven 20160711 : 使用進階版RMS
    CosFunction.bCancelErrorBin                                                 =false; //kevin 20160724 取消 ERROR BIN設定
    CosFunction.bRTCAlarmSetIndexToErrBin                                       =false; //JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin
    CosFunction.bUse1_16KgEPForm                                                =true;  //Ifor 20160712 add Use 1~16 Kg EP對照表功能    //JerryYang 20160824 全部開放Low contact force 1~16kg功能
    CosFunction.bShowAlarmIfCCLinkConnectFail                                   =false; //JerryYang 20160729 for 矽品, CC-link連線異常要Show alarm
    CosFunction.bAutoFullDoNotStopImmediately                                   =false; //JerryYang 20160810 當Auto滿tray時,要先show message鳴叫且繼續生產,不要馬上停機show alrm
    CosFunction.bIndexArmVacOffErrNeedPiggyBack                                 =false; //JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back
    CosFunction.bHWBinBox                                                       =false; //kevin 20160819 第7個 HARD WARE BIN support Bin Box
//    CosFunction.bRotateDetectErrNeedShake                                     =false; //JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm
    CosFunction.bUsePEModelFunction                                             =false; //Ifor 20160822 Use PE Model Function (PE 工程模式)
    CosFunction.bUseFixTryCheckRemainingAmount                                  =false; //Ifor 20160829 add Use Fix Try Check Remaining Amount
//    CosFunction.bUseLoaderTryFeedContinueRun                                  =false; //Ifor 20160829 add Use Loader Try Feed Continue Run
    CosFunction.bAllSiteSameFailBinShowAlarm                                    =false; //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    CosFunction.bPurgeBeforePickShuttle                                         =false; //JerryYang 20160906 PurgeBeforePickShuttle
    CosFunction.bUseMonitorView                                                 =false; //Ifor 20160930 add USE MonitorView 錄影監視功能顯示
    CosFunction.bUseFix3FullTray                                                =true;  //Ifor 20161121 add Use Fix3 Full Tray By CosFunction
    CosFunction.bUseSCKART                                                      =false; //Steven 20161201 : For SCK 93K ART
    CosFunction.bBeforeAutoCleanOnlyUseRTC                                      =false; //JerryYang 20161216 (jou) auto clean的前後只靠RTC來檢查socket,不做index下壓至socket吸真空
    CosFunction.bAfterAutoCleanNoIndexCheck                                     =false; //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
    CosFunction.bIndexCheckCanTurnOff                                           =false; //Isaac 20211019 : 可選擇做index check的時機
    CosFunction.bFullTestBeforeContactHeight                                    =false; //Steven 20170109 : add initial value
    CosFunction.bUseAlarmUnlockPassWord                                         =false; //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
    CosFunction.bLowYieldDoOneCycle                                             =false; //Steven 20170308 (wei) : 良率相關的都做One Cycle
    CosFunction.bUseLoginDatToSetLevel                                          =false; //Steven 20170301 (wei) : 使用Login.dat當密碼本
    CosFunction.bTTLCanUse8Site                                                 =false; //Frank 20170329 (wei) add
    CosFunction.bUseInitialDelayAsSoakTime                                      =false; //Steven 20170329 (Wei) : 使用initial delay當 Soak time
    CosFunction.bUseARTSortCount                                                =false; //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    CosFunction.bUseEmptyColorTrayPreAlarm                                      =false; //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    CosFunction.bUseAutoTrayPreAlarm                                            =false; //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    CosFunction.bUseMRTMode                                                     =false; //Ifor 20170316 (wei) add KYEC MRT Mode
    CosFunction.bUseLotIDWithoutSECS                                            =false; //RogerYang 20170327 (Steven) 開放LotID但又不使用SECS/GEM
    CosFunction.bInShuttleDetectByLatch                                         =false; //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
    CosFunction.bInOutArmUseBackRowSuck                                         =false; //jou 20161122 (Steven) In & Out arm use back row suck
    CosFunction.bSuckDevicesDuringTest                                          =false; //JerryYang 20170522 drop contact改為邊吸邊測
    CosFunction.bUse32ChanelSiteMap                                             =false; //Steven 20170530 (wei) : Use 32CH site map
    CosFunction.bRecipeParameterDefault                                         =false; //Isaac 20170527 (Steven) defalut值比較功能
    CosFunction.bRecipeParameterDefaultLog                                      =false; //Sam 20201209 : Default Recipe ChangeLog
    CosFunction.bProgramStartOnLine                                             =false; //Sam 20210423 : 程式開啟時切換為 OnLine/Real
    CosFunction.bIndexPickErrSkipNeedCheckVac                                   =false; //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
    CosFunction.bOutShtLoseICSetErrUntilOneCycle                                =false; //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin
    CosFunction.bUseDynamicKitDiameter                                          =true;  //JerryYang 20181220 全面啟用新版contact force校正方式 //Steven 20170605 (wei) : 可以自定義Kit直徑
    CosFunction.b2x4SupportCenterPitch                                          =false; //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC
    CosFunction.bPiggybackFunctionByHandler                                     =false; //Isaac 20170712 (wei) :Piggyback function By Handlder(save file to config.ini)
    CosFunction.bUseAutoBackUpSetupFile                                         =false; //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    CosFunction.bUseARMSFunction                                                =false; //Ifor 20170621 (wei) add ARMS Function
    CosFunction.bEnableHandlerResultServer                                      =false; //Isaac 20170613 (wei) TCP/IP
    CosFunction.bBySiteByBinPercentCompare                                      =true;  //JerryYang 20170712 (Steven) by site by bin compare percent
    CosFunction.bNewAutoTrayDetect                                              =true;  //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
    CosFunction.bYieldAlarm4                                                    =false; //wei 20160406 Yield Alarm4
    CosFunction.bYieldAlarm5                                                    =false; //Sam 20171213 (Steven) : 超豐良率監控
    CosFunction.bLowYieldAutoSiteOff                                            =false; //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
    CosFunction.bAmkorFunction                                                  =false; //Steven 20171017 (wei) : Functions for Amkor Group
    CosFunction.bSortingBinTrayWhenTrayFeed                                     =false; //JerryYang 20170911 (Steven) 整盤功能,執行時機由clean out改至tray feed前
    CosFunction.bSetupFileNameControlByLevel                                    =false; //jou 20171011  (wei): bSetupFileName Control By Level
    CosFunction.bUseLogUploadToFTPFunction                                      =true;  //Ifor 20160304 (Steven) :新增Log Up to FTP Function    //Steven 20231120 : 全面開放
    CosFunction.bHisiLogUploadNetwork                                           =false; //Ifor 20170913 (Steven) : add 海思 Log 上傳至網路磁碟
    CosFunction.bAutoSiteMappingUseHotPlate                                     =false; //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    CosFunction.bUseAuto1OnlyBin1                                               =false; //Ifor 20171018 (wei) : Only Bin 1 is allowed to set to Auto 1
    CosFunction.bCanChangeAutoCleanCount                                        =false; //Ifor 20171024 (wei) : add 可更改Auto Clean 計數
//    CosFunction.bSaveAutoCleanCnt                                             =false; //JerryYang 20171031 (wei) 儲存auto clean清潔次數
    CosFunction.bUniversalKit                                                   =false; //JerryYang 20171102 (wei) UniversalKit改成客戶功能
    CosFunction.bFTPDownLoadSiteBySetupFile                                     =false; //Ifor 20171123 : add FTP DownLoad Site By SetupFile
    CosFunction.bAutoSiteMappingUseFailBinSetting                               =true;  //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting       //Steven 20230213 : [I21-9]全部開放
    CosFunction.bUsePassBinOnlyCanSetOneBin                                     =false; //Ifor 20171213 (Steven) : Auto Tray 設定Pass時僅可設定一個Bin
    CosFunction.bByBinAlarmFromYieldForm                                        =false; //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    CosFunction.bUseFTPDownloadDataCheck                                        =true;  //Ifor 20180125 : Use FTP Download Data Check    //Ifor 20200828 Fix:FTP Download Data Check強制開啟
    CosFunction.bFTPDownLoadTempModeBySetupFile                                 =false; //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
    CosFunction.bFTPDataTrayFeedAutoUpdata                                      =false; //Ifor 20181214 : add Tray Feed 後自動上傳海思檔案至FTP
    CosFunction.bUseOneByOneIndexCheck                                          =false; //Ifor 20180322 : add Use One By One Index Check
    CosFunction.bOEEFunction                                                    =false; //Steven 20180417 (Jou) : OEE功能
    CosFunction.bRemoteLotStart                                                 =false; //AI(ht9045-v899) 20260331: Greatek TCP Remote Start Lot (HTSET 720)
    CosFunction.bIndexProtect                                                   =true;  //JerryYang 20180410 (jou) : 加入index arm移動前位置保護
//    CosFunction.bSLTReportFunction                                            =false; //Steven 20180420 : SLT報表功能
    CosFunction.bRTC_ROICount                                                   =false; //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
    CosFunction.bStatisticsJamCount                                             =false; //jou 20171201 (Steven) : 新增統計jam code alarm次數,達到設定數量後提高一階權限才能解開alarm
//    CosFunction.bIndexPickErrNeedPiggyBack                                    =false; //jou 20171211 (Steven) : After index pick up error, device go to Error bin ,follow by Index check or piggy back check
    CosFunction.bIndexPickErrSkipBlowAirTime                                    =false;
    CosFunction.bInOutArmZCalibration                                           =false; //Frank 20171213 (Steven) : In/OutArmZCalibration
    CosFunction.bColorSensorUseCCLink                                           =false; //wei 20180409 color sensor
    CosFunction.bRotateNoUseFix1                                                =false; //Frank 20170531 (Steven) add Open Rotate不能選Fix1
    CosFunction.bYieldControlUseEACount                                         =false; //wei 20180606 Yield控制使用EA Count
    CosFunction.bUseYieldControlFunction                                        =false; //Ifor 20180731 : add Use Yield Control Function
    CosFunction.bStartESDAutoDecayFunction                                      =false; //Ifor 20180731 : 補上 Auto Decay Function 初始值
    CosFunction.bTestTimeOutOnlyShowSkip                                        =false; //Steven 20180627 : Time out預設只顯示SKIP
    CosFunction.bIncludeMTBA                                                    =true;  //JerryYang 20180619 : 新增可自定義Jam code是否列入MTBA計算
    CosFunction.bAfterInitialDelayUseOtherArm                                   =false; //Jerryyang 20180607 : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
    CosFunction.bTTLUseUSec                                                     =false; //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    CosFunction.bCCLinkValueSaveFile                                            =false; //kevin 20180827 add wei 20180622  CCLink Value Save File
    CosFunction.bYPitchNotUseSearchLastMode                                     =false; //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
    CosFunction.bDLLCommands                                                    =false; //JerryYang 20181126 (Steven) : support Epson DLL function
    CosFunction.bTempLess30degShowLight                                         =false; //jou 2014-12-19 Temperature Less 30 deg. Show Light
    CosFunction.bTempHeaterOkShowLight                                          =false; //jou 20180529 : Temperature Heater Ok Show Light
    CosFunction.bUseDefineAutoCleanOffset                                       =false; //Ifor 20181106 add 新增Auto Clean Offset By 本機
    CosFunction.bUse12SiteAutoCleanFixedPosition                                =true;  //Ifor 20181222 add 新增Auto Clean 12 site取放位置固定 //Steven 20200507 : 改成強制開啟
    CosFunction.bUseAutoCleanCloseSiteAlsoDo                                    =false; //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
    CosFunction.bGPIBLotEnd                                                     =false; //Sam 20181030 : 透過 GPIB 送 LotEnd 給測試機。
    CosFunction.bSiteStopVisblieSpecialColor                                    =false; //Sam 20181112 : 開啟 Site 又將 Site 停止時以特別顏色顯示。
    CosFunction.bUseLocalRecipeOffset                                           =false; //Ifor 20181211 : add Use Local Recipe Offset
    CosFunction.bATC32UseTJMode                                                 =false; //Ifor 20190121 : add ATC32 Use TJ Mode
    CosFunction.bATCUseTempAdjustment                                           =false; //Ifor 20190215 : add ATC 使用 三點校正功能
    CosFunction.bATCUsePackageOffset                                            =false; //Ifor 20190215 : add ATC 使用 Package Offset 功能
    CosFunction.bUseHandShakeCommunication                                      =false; //Ifor 20190225 : add Bar Code Use HandShake Communication
    CosFunction.bUseATCTJControlMode                                            =true;  //Ifor 20190328 : add TJ Temp Over Range
    CosFunction.bAMDFunction                                                    =false;
    CosFunction.bCanUseSearch2DIDByLot                                          =false; //Steven 20190508
    CosFunction.bFTPDownloadAlwaysCover                                         =false; //JerryYang 20190523 KYEC download工作檔因為是by機台上傳, 所有參數always以server的為主
    CosFunction.bCanUse2x2Bias                                                  =false; //Steven 20190724 : 2x2偏心開關
    CosFunction.bForecePerPinKGf                                                =false; //JerryYang 20180515 : 輸入每個pin的力量,單位以kgf為主,避免轉換成牛頓又換回KGf會有小數點兩位的差異
    CosFunction.bCanUseBias                                                     =true;  //Steven 20190801 : SCC李國旗說要取消BIAS選項
    CosFunction.bNextTrayNeedManualRemoveTray                                   =false; //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
    CosFunction.bUseHeadContactCount                                            =false; //Ifor 20160516 京元要求銦片 Life Time 功能
//    CosFunction.bHeadContactCountByRecipe                                       =false; //Steven 20241030 : 銦片 Life Time 功能by工作檔  //RogerYang 20260515 : dead flag, 未被任何邏輯讀取
    CosFunction.bPopAutoClean                                                   =false; //JerryYang 20190918 Pop auto clean
    CosFunction.bBarcodeTrayRecFile                                             =false; //jou 20190930 : Barcode Tray record file
    CosFunction.bBarcodeErrNoTestAndShowH                                       =false; //jou 20191007 : Barcode Error No Test & Show "H"
    CosFunction.bBarcodeDuplicateFileByOutArm                                   =false; //jou 20191008 : Barcode duplicate file by out arm
    CosFunction.bDeviceMapTest                                                  =false; //Steven 20190910 : Qualcomm功能
    CosFunction.bSetupTeach                                                     =false; //JerryYang 20180921 Setup Teach功能
    CosFunction.bUseChangeLogByLot                                              =false; //Ifor 20191002 : add Change Log By Lot
    CosFunction.bUseFFCTempOffset                                               =false; //Ifor 20190730 : add FFC Temperature Offset
    CosFunction.bUseFTPDownLoadATCRecipe                                        =false; //Ifor 20191115 : add FTP DownLoad ATC Recipe
    CosFunction.bUseOpenCloseSiteMapAtAnyTime                                   =false; //Ifor 20190308 add Open/Close Site Mapping Function at Any Time
    CosFunction.bATCUseHandlerRecipeName                                        =false; //Ifor 20191126 : add ATC Use Handler Recipe Name
    CosFunction.bUseChamberBoostMode                                            =false; //Steven 20191128 : Chamber Boost Function
    CosFunction.bUnloaderEditTrayLevelSet                                       =false; //Steven 20191224 : Unloader編輯Tray改用另外一組權限
    CosFunction.bAutoCloseSiteWhenRT                                            =false; //Steven 20200225 : 切到RT的時候,要關閉Socket
    CosFunction.bAutoSkipNoDropError                                            =false;
    CosFunction.bUseN07_5                                                       =false; //Steven 20200309 : [N07-5]改成by客戶開啟
    CosFunction.bAutoHeightSHTReleaseByFile                                     =false; //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
    CosFunction.bCanDisableQAMode                                               =false; //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
    CosFunction.bEnableDual_1x4Kit                                              =false; //Isaac 20200311 : 1x2Site使用1x4Site Kit
    CosFunction.bEnable_1x3Kit                                                  =false; //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
    CosFunction.bSaveProductionLogByUnloaderTray                                =false; //Steven 20200330 : production log by unloader tray存檔
    CosFunction.bUseHandlerSetChillerTemp                                       =true;  //Ifor 20191218 : add ATC Chiller 溫度由Handler設定
    CosFunction.bUseEditLDTrayNeedManualRemoveTray                              =false; //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
    CosFunction.bKeepOnly1SetupFile                                             =false; //Steven 20200511 : 改成客戶功能 //wei 20131115 FTP下載後保留下載檔案，其餘Data刪除
    CosFunction.bContactHeightSaveToContactIni                                  =false; //Steven 20200616 : JSCC要求把Contact Height放到別的檔案
    CosFunction.bShakeShuttleEveryTime                                          =false; //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
    CosFunction.bCanDisableQAMode                                               =false;
    CosFunction.bECChangeEventReportCanOnOff                                    =false; //JerryYang 20200520 舊電腦做DoReportECDataChangeCheck函式會影響UPH,改成功能選項
//    CosFunction.bOutShtLoseICInArmAway                                        =false; //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
    CosFunction.bNonCenterModeCanUseShtOffset                                   =false; //Steven 20200715 : for Tinton
    CosFunction.bCanUse2x2NNMode                                                =false; //Steven 20200722 : 2x2 NN mode
    CosFunction.bCanUse2x3NNMode                                                =false; //Steven 20220425 : 2x3 NN mode
    CosFunction.bCanUse2x4NNMode                                                =false; //Wei 20231211 : 2X4NN Mode
    CosFunction.bThickTrayNoNeedCover                                           =false; //Steven 20200723 : 太厚的Tray不能蓋蓋子
    CosFunction.b1x4OnlyAaUse1x1Mode                                            =false; //Steven 20200720 : 1x4只開site Ac, 跑1x1模式
    CosFunction.bQAModeFinishOfflineShowOnLinePicture                           =false; //Sam 20200804 : QA 模式完成後切回 OffLine 時，強制顯示 OnLine 圖片。
    CosFunction.bEnableMulti2D                                                  =false; //Steven 20200810 : 一個IC使用多個2DID
    CosFunction.b32SiteYOffsetMode                                              =false;
//    CosFunction.bLoadPickUpErrorMES0101Record                                   =false; //Sam 20200922 : Pick up Error Production_Log 也要記錄
    CosFunction.bAutoTrayFullAlarmCanNotSkip                                    =false; //Steven 20210219 : Unloader滿盤alarm不能Skip
    CosFunction.bIndexCycleTimeRecord                                           =false; //Sam 20200916 : Add Index Cycle Time Record
    CosFunction.bUseAfterHomeShtChkLoseICNeedSlow                               =false; //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
    CosFunction.LoaderFloating                                                  =false;
    CosFunction.bRecordVacuumOnOffTime                                          =false; //Sam 20210204 : Auto Teach Loader Z Record Vacuum On off time
    CosFunction.bSpecailLowYeild                                                =false; //Sam 20210505 : PTI 要求的兩段 Low Yeild
    CosFunction.bI21EnableASMByRecipe                                           =false; //Steven 20210518 : 吳如春希望Auto site map從工作檔開關
    CosFunction.bUseMDB                                                         =false; //Steven 20210526 : 部分客戶取消使用MDB
    CosFunction.bTraySortCntFunc                                                =false; //Steven 20210608 : Tray指定數量退盤功能
    CosFunction.bIOTriggerIonFanAutoClean                                       =false; //Isaac 20210609 : IO觸發IonFan清針
    //==> Eastsun 20260527 整合#028.AAL.P15b default false :KYEC
    CosFunction.bUseBarcodeAutoAdjustLight                                      =false;
    //<== Eastsun 20260527 #028.AAL.P15b
    CosFunction.bUSEJCETSiteMapMode                                             =false; //Ifor 20210611 : add USE JCET Auto Site Mapping Mode
    CosFunction.bAutoCleanUseHPSetByRecipe                                      =false; //Steven 20210825 : Auto Clean使用加熱盤要改成在工作檔設定
    CosFunction.b2DUseSubJobFunction                                            =false; //Ifor 20200807 add:In House 2D Use Sub Job Function
    CosFunction.bFTPUseBarcodeReader                                            =false; //Ifor 20210413 add:FTP Use Barcode Reader
    CosFunction.b2DUseAnyCharFunction                                           =false; //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    //==> Eastsun 20260526 #026-4.PinN.P-N2 Pin1 default init :KYEC
    CosFunction.b2DUsePinInspection                                                =false; //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
    //<== Eastsun 20260526 #026-4.PinN.P-N2
    CosFunction.bOpenAutoCloseSiteInNormal                                      =false; //JimmyChiu 20211015 : only user disabled site keep disabled and other sites change to enabled
    CosFunction.bManualSteplAutoTeach                                           =false; //JimmyChiu 20211020 : Auto alignment mode
    CosFunction.ShowLotCategoryCount                                            =false; //KaiChen 20181002 ：Category Count by Lot
    CosFunction.ShowLotCount                                                    =false; //KaiChen 20181002 ：改成CosFunction   //wei 20171130 Count By Lot
    CosFunction.bYieldControlBinSelectUseContactCount                           =false; //KaiChen 20181115 : BinSelect裡面 Yield控制使用 Contact Count
    CosFunction.bSaveEventLogByLotID                                            =false; //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
    CosFunction.bOutShtLoseICInArmAway                                          =true;  //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm   //Steven 20220624 : 全面啟用
    CosFunction.bConfigStandard                                                 =false; //Sam 20210324 : 程式開啟，直接讀取 config_Standard.ini
    CosFunction.bRotateUseHT7000HPKit                                           =false; //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
    CosFunction.bUseJamRawData                                                  =false; //Sam 20210224 : Auto Upload FTP JAMRawData 功能
    CosFunction.bUseAlarmLogXml                                                 =false; //Sam 20210621 : 新增 Event Log xml  Updata
    CosFunction.bUse1CableLayoutKitByConfig                                     =false; //Sam 20210715 : Use 1CableLayoutKit By Config
    CosFunction.bTempSetDisenableNotUseHeater                                   =false; //Sam 20210805 : TempSet 加熱區地方沒用到的就停用
    CosFunction.bOffsetContactForce                                             =false; //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
    CosFunction.bNeedToRestartGroundWhenInitialStart                            =false; //Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan
    CosFunction.bRTCHalfViewAutoVerify                                          =false; //JerryYang 20220215 : RTC half view auto verify
    CosFunction.bCylinderOnOffTimeLog                                           =false; //JerryYang 20220215 : cylinder作動次數計數
    CosFunction.bUseATCFileTransfer                                             =false; //Eastsun 20260522 整合: ATC Recipe 傳送與接收 default
    CosFunction.bS5F1UseJamRate                                                 =false; //Steven 20220331 : S5F1的ALTX裡面加上JAM Rate判斷
    IniConfig.bVTESTFunction                                                    =false; //jou 20210811 : VTEST function
    CosFunction.bShow0Xbin                                                      =false; //jou 20220719 : show 0X bin
    CosFunction.bTempCalByRecipe                                                =false; //jou 20220725 : Temperature calibration by recipe
    CosFunction.bF18F22InshuttleDetectSaveByRecipe                              =false; //JimmyChiu 20220601 : F18/F22 in shuttle A7 sensor進出檢測功能跟隨recipe是否啟用
    CosFunction.bLowYieldAlarmIntervalTimeBySetting                             =false; //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
    CosFunction.bAlarmForIonFanCleaning                                         =false; //JimmyChiu 20220606 : 增加一個離子風扇清潔報警，時間間隔可設定。需報警提示，需輸入密碼才可消除
    CosFunction.bRecordGroundESDByTestIC                                        =false; //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    CosFunction.b2DCodeCheckByCoustomerLot                                      =false; //Sam 20220223 : 2D Code Check by Coustomer Lot
    CosFunction.bAutoTmpeOfsByFTP                                               =false; //Sam 20220406 : 溫度自動補償功能 By FTP
    CosFunction.bLockF26ByFile                                                  =false; //Sam 20220527 : for 矽格湖口 -- F26 Enable
    CosFunction.bLockI06ByFile                                                  =false; //Sam 20220527 : for 矽格北興
    CosFunction.bManuallyRemoveForceInColor                                     =false; //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    CosFunction.bUnloadTrayModeByRecipe                                         =false; //Steven 20220710 : 甬矽要求Unload Tray Mode by機台設置
    CosFunction.bFullTrayAlarmAfterUnloadEnd                                    =false; //Sam 20210602 : Unload 做完後再 Alarm
    CosFunction.PassworDownloadByFTP                                            =false; //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
    CosFunction.bShowYieldMonitor                                               =false; //Sam 20210916 : 新增 Yiled Monitor 到畫面上
    CosFunction.RecordIndexAirOnTime                                            =false; //Sam 20220329 : Record Index Air On Time
    CosFunction.bUse2X4_4AutoCleanFixedPosition                                 =false; //Sam 20220423 : 新增 Auto Clean 2X4_4 模式取放位置固定
    CosFunction.bAutoCleanOffsetUseSingleSetting                                =false; //Sam 20220720 : AutoClean Offset 僅使用 Clean 設定 Offset 來補償
    CosFunction.bUseSocketContactCount                                          =false; //Sam 20220720 : 新增一組 Socket Count
    CosFunction.bDownloadUpdateAutomatically                                    =false; //Sam 20220824 : FTP 自動下載安裝更新包
    CosFunction.bTrayDeviceCheckFromLoader                                      =false; //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bTrayMapFromLoader                                              =false; //JimmyChiu 20220219 : 殘料檢查 From Loader
    CosFunction.bSaveArmSpeedByMachine                                          =false; //JimmyChiu 20220618 : save by machine
    CosFunction.bSaveTemperatureByMachine                                       =false; //JimmyChiu 20220618 : save by machine
    CosFunction.bSaveOffsetByMachine                                            =false; //JimmyChiu 20220618 : save by machine
    CosFunction.bART_SECSGEM_93K                                                =false;
    CosFunction.bOutArmPickUpErrBySetup                                         =false;
    CosFunction.bNonATCSupportLBFunction                                        =false; //JerryYang 20220126: non ATC也支援LB function
    CosFunction.bIndexDropICSetErrUntilOneCycle                                 =false;
    CosFunction.bFTRTDiffInitStartDelayTime                                     =false;
    CosFunction.bSortingBy2DList                                                =false; //Frank 20221122 : 2DID sorting for ATK
    CosFunction.bShowHPICCount                                                  =false; //Steven 20221228 : 計算加熱盤IC數量
    CosFunction.bUseTrayThickAdjustZHeight                                      =false; //Ifor 20221214 add: 使用Tray 厚度自動校正吸嘴高度
    CosFunction.bUseShuttlePickShiftDetect                                      =false; //Ifor 20221220 add: Shuttle 吸料後 先上升設定高度再檢查有無IC
    CosFunction.bLowYieldUseContactCounts                                       =false; //Sam 20221020 : LowYield 改使用 ContactCounts 的資料來計算
    CosFunction.bD44Once4Suck                                                   =false; //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
    CosFunction.bAutoSideFixerCheckAlarmAutoRetry                               =false; //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
    CosFunction.bAutoOpenConfigA01                                              =false; //Sam 20220929 : Config A01 自動開啟。
    CosFunction.bVerifyMode                                                     =false; //Sam 20221012 : 新增 VerifyMode 功能
    CosFunction.bUnloadCylinerUpRelief                                          =false; //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
    CosFunction.IntervalYieldCount                                              =false; //wei 20180606 Interval Low Yield By Site
    CosFunction.bLoginShowUserName                                              =true;  //Steven 20230317 : 登入時顯示帳號名稱
    CosFunction.bSmartAutoClean                                                 =false; //Sam 20230111 : Smart Auto Clean
    CosFunction.bAdaptiveYield                                                  =false; //Sam 20230914 : 自適應性良率監控
    CosFunction.bAfterOneCycleShowTrayQty                                       =false; //Sam 20221101 : OneCycle 完成後顯示所有 Tray 各別的IC數量
    CosFunction.bUseBarCoderAutoLogin                                           =false; //Sam 20221101 : 使用 BarCoder 自動登錄
    CosFunction.bUseBarCoderChangeSetupFile                                     =false; //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
    CosFunction.bSpecialP24                                                     =false; //Sam 20230320 : 整合 P24 北興版為獨立功能碼
    CosFunction.bHandlerStateChangeUploadServer                                 =false; //Sam 20230511 : 機台資料變更後須上傳 FTP
    CosFunction.bLimitSensorOnNeedManualMove                                    =false; //Sam 20230522 : InOutArm Server Off 後，重新 Server On 時檢查 CCW CW 若有 On 時需要手動排除異常。
    CosFunction.bQAModeUseUnloadCnt                                             =false; //JerryYang 20221005 : QA mode count 用BIN1數量計數
    CosFunction.bTemp5PointKitOffset                                            =false; //JerryYang 20221005 : 新增溫度5點offset by recipe儲存
    CosFunction.bTowerLightUseLD                                                =false; //JerryYang 20230721 : LD/ULD區分不同的異常音樂
    CosFunction.bRead2DIDFromServer                                             =false; //Jimmychiu 20230925 : read 2did in json file
    CosFunction.bMakeWhite2DIDList                                              =false; //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    CosFunction.bPassBinNoRotate                                                =false; //jou 20231020 : Pass bin no rotate
    CosFunction.bGPIB_Command_DOPAUSE                                           =false; //Jimmychiu 20231102 : GPIB Command SETHANDLERDOPAUSE
    CosFunction.bRotateUseRTmode                                                =false; //jou 20231122 : Rotate Use RT mode
    CosFunction.bMaualRemoveTray3Alarm                                          =false; //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
    CosFunction.bSECSGEM_UseKeyPro                                              =false; //Ifor 20230721 : add SECS GEM 使用 KeyPro 控管
    CosFunction.bLockF11ByFile                                                  =false; //Sam 20240202 : 新增 F11 Lock by file 功能
    CosFunction.bLockRTCByFile                                                  =false; //Sam 20240311 : 新增 RTC Lock by file 功能
    CosFunction.bRTCAutoTuning                                                  =true;  //Sam 20230419 : 新增 RTC Auto Tuning 功能
    CosFunction.bCreateManualEOCAP                                              =false; //jou 20221104 : VTest CreateManualEOCAP function;
    CosFunction.bAutoSiteMappingSetOpenBIN                                      =false; //jou 20200928 : Auto Site Mapping Set Open BIN
    CosFunction.bAutoSiteMappingSetPassBIN                                      =false; //jou 20230221 : Auto Site Mapping Set Pass BIN
    CosFunction.bLotIDVerify                                                    =false; //Steven 20240704 : Lot Verification function for ATK
    CosFunction.bCategoryInfoByContactCT                                        =false; //Sam 20240131 : 新增一組 CategoryInfo 紀錄資料使用 ContactCT
    CosFunction.bEnable12SiteUse16SLK                                           =false; //Steven 20240807 : 12Site使用16Site Kit
    CosFunction.bFixNameOfForcePerPinG                                          =false; //Steven 20240821 : 凌中心說要修正存檔名稱
    CosFunction.bART_RT_NoRotate                                                =false; //Sam 20240809 : ART RT No Rotate
    CosFunction.bUseTSVFunction                                                 =false; //Steven 20240904 : for ATK的TSV功能
    CosFunction.bCheckTempDuringIndexArmTesting                                 =false; //Jimmychiu 20240916 : Check the temperature during index arm testing
    CosFunction.bNeedAlarmAfterUnloaderFull                                     =false; //Jimmychiu 20240902 : Need Alarm After Unloader Full
    CosFunction.bRMSNoNeedToDownloadEveryTime                                   =false; //Steven 20240926 : RMS不要每次下載包成功能
    CosFunction.bGPIBUseSECSGENData                                             =false; //Sam 20240826 : GPIB 通訊資料使用 SECSGEM Data
    CosFunction.bFillTheTrayAfterOutArmPlace                                    =false; //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    CosFunction.bDisableOpenAllSiteWhenChangeShtMod                             =false; //Jimmychiu 20241203 : Disable opening all sites when changing shuttle mode.
    CosFunction.bContactAlmNeedOneCycle                                         =false; //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    CosFunction.bPickerLifeAlmNeedOneCycle                                      =false; //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
    CosFunction.bEnableRMS                                                      =false; //Jimmychiu 20241213 : add coustomer func EnableRMS
    CosFunction.bContactCountSavedByRecipe                                      =false; //Jimmychiu 20240826 : Contact count saved by recipe
    CosFunction.bScanBarcodeAndDownloadFileInRMS                                =false; //Jimmychiu 20241226 : Scan Barcode And Download File In RMS
    CosFunction.bAutoCleanTimeCT                                                =false; //jou 20250102 : auto clean triger time count
    CosFunction.bUploadLogByTypeFolder                                          =false; //Steven 20250217 : 根據Log內容作資料夾分類
    CosFunction.bRunModeFollowLotInfo                                           =false; //Steven 20250603 : 根據Lot Info的Run mode進行切換
    CosFunction.bUploadTestArmTorqueLog                                         =false; //Steven 20250609 : 統一整理
    CosFunction.bCanRemoteStart                                                 =false; //Steven 20250712 : 有通過風險告知可以遠端start的客戶
    CosFunction.bEndLotAfterTrayFeed                                            =false; //Jimmychiu 20250115 : Auto End Lot After Tray Feed
    CosFunction.bReadClipCodeFromUnloader                                       =false; //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
    CosFunction.bSocketSensorCheckICAtArmDown                                   =false; //Jimmychiu 20250826 : 每次下壓確認有IC在socket
    CosFunction.bOpenDoorCheckLoaderAfterTrayEnd                                =false; //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
    CosFunction.bSetContactForceToAutoClean                                     =false; //Steven 20250915 : 複製Contact Force給Auto Clean用
    CosFunction.bOneByOneWhenPickErrAtLoader                                    =false; //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
    CosFunction.bLoaderTrayToAuto1                                              =false;
    CosFunction.bTempAlarmBinNeedToError                                        =false; //Steven 20251022 : Temp alarm need put to error bin
    CosFunction.bInspectSuckICWhenSoftStart                                     =true;  //Jimmychiu 20240217 : 汶彥要求移除檢查
    CosFunction.bCheckOutSuckICFallDownOnlySkip                                 =false; //Jimmychiu 20240813 : Only Skip When Checking Out Suck IC Fall Down
    CosFunction.bAutoTeachOutShuttleLantch                                      =false; //Jimmychiu 20240923 : Auto teach in/out shuttle site kit position
    CosFunction.bMonitorLBonAmbient                                             =false; //Jimmychiu 20251030 : Monitor the LB temperature under ambient temperature
    CosFunction.bFirstTrayCheckOnUnloader                                       =false; //Jimmychiu 20251205 : First Tray Check On Unloader
    CosFunction.bDeviceMapTestPandP                                             =false; //Jimmychiu 20251222 : Device Map Test By P&P
    CosFunction.bOffsetTempByRecipeMinMaxLimit                                  =false; //StevenHong 20260119 : Add Offset By Recipe Max Limit
    CosFunction.bCleanCountAlarmByMin                                           =false; //Jimmychiu 20260212 : Gigas Clear alarms based on minimum usage count
    CosFunction.bLoginASECL                                                     =false;     //JerryYang 20211221 : ASECL密碼本(tech.com)
    CosFunction.bLoaderAutoRetry                                                =false;     //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
    CosFunction.bHotModeUseDiffScale                                            =false;     //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
    CosFunction.bSetOffsetLimitToAll                                            =false;     //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
    CosFunction.bUseConfigSaveButton                                            =false;     //JerryYang 20230215 : ASE-CL 尚智要求follow 7000軟體新增config存檔按鈕
    CosFunction.bConfigAddObjectName                                            =false;
    CosFunction.bZHomingAfterPickErr                                            =true;
    //----------------------
    DoCustomerFunction();                                                               //Steven 20240927 : 整合並保持在最下面

    #ifdef SOFT_SIMULTE
    CosFunction.bUseOneByOneIndexCheck                                          =false;
    #endif

    if(AUTO3_IS_MAGAZINE==1)                                                    //Ifor 20231122 add Magazine Link
        IniConfig.bAutoTrayLink                                                 =true; //jou 2012-06-14 Auto Tray Link

    if(fAGV!=NULL && fAGV->IsATK_AMR())                                         //Steven 20251210 : ATK AMR
    {
        IniConfig.bLastLoaderNoInSide=false;
    }
}
//------------------------------------------------------------------------------
