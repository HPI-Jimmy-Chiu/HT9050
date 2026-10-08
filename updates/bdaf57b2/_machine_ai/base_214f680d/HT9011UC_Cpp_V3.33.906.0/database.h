// ===========================================================================
//  database.h  -- PARTIAL translation (W3-cont2: config-table loaders batch)
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\database.h
//
//  WHAT IS TRANSLATED (cumulative through GA-1-B6):
//    TIODATA        struct + ctor         (database.h:17-38)
//    TIOTABLENO     struct + methods      (database.h:40-61)
//    TMOTDATA       struct + ctor         (database.h:63-98)
//    TMOTNO         struct + methods      (database.h:100-135)
//    SYSTEM_MODULAR class -- FULL member surface now active (com-ports, CCD/
//      RFID/ATC/Laser/Barcode/GroundMan arrays, dNumberPanelDelay,
//      bUseSocketTemp, ReadGeneralIni()) as of GA-1-B6, since ReadGeneralIni
//      assigns every one of them.  (database.h:193-292)
//    extern SYSTEM_MODULAR HSys           (database.h:293)
//
//  WHAT IS STILL GATED (#if 0 // TODO(wave)) elsewhere in this pair:
//    TDataModule1 class + extern DataModule1  (database.cpp)
//      -> BDE TTable / TDataModule / VCL form; needs DBTables.hpp.
//    SYSTEM_MODULAR explicit ctor/dtor + InstallColorBinDisplay (database.cpp)
//      -> ctor calls ReadGeneralIni+SystemModularInitial; dtor deletes
//         BinDisCtrl/ATKRecipeInfo; InstallColorBinDisplay needs TMyBinDispHT9046.
//    ATKRecipeInfo pointer + ATK_RECIPE_INFO include
//
//  AI(W906-SysModWire) 20260720: HTGem *MyGem member of SYSTEM_MODULAR is NO
//  LONGER gated -- wired for real by SystemModularInitial (database.cpp),
//  see that member's own comment below.
//
//  AI(ht9045-v899/W906-GA1-B6) 20260804: un-gated the SYSTEM_MODULAR member
//  surface (private dNumberPanelDelay/bUseSocketTemp; public com-port/CCD/
//  RFID/ATC/Laser/Barcode/GroundMan scalars+arrays; ReadGeneralIni() decl) to
//  land the real ReadGeneralIni translation in database.cpp.  These are all
//  members of THIS class (not the shared cmydef.h/cpublic.h/cprod.h/Config.h/
//  CosFunction.h/canary_support.h headers), so un-gating them is in-scope for
//  the GA-1-B6 "database.cpp/database.h, existing files, this batch exclusive"
//  write authorization -- it is not "adding a field to a shared header".
//  Added #include "MachineType.h" (for the iTotalFunction enum used by the
//  asCCDTrayIP/asCCDTrayPort array sizes) since database.cpp includes this
//  header before MachineType.h -- database.h must be self-contained.
//
//  ENCODING NOTE: original is Big5; this translation is UTF-8.  Chinese
//  comments in the golden are replaced with their semantic equivalents in ASCII
//  (matches this file's own established convention -- see database.cpp, which
//  is 100% ASCII-comment despite AGENTS.md's general cp950->UTF-8 preference
//  for the wider ported tree).
// ===========================================================================
#ifndef DATABASEH
#define DATABASEH

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList, FileExists, ...
#include <map>
#include <vector>
#include "myTimer.h"                // TQPF_Timer (database.h:12)
#include "MachineType.h"            // iTotalFunction (eFunction enum) -- GA-1-B6: needed by
                                    // asCCDTrayIP[iTotalFunction]/asCCDTrayPort[iTotalFunction]
                                    // members below; database.h must be self-contained since
                                    // database.cpp includes this header BEFORE MachineType.h.
using namespace std;

// ---------------------------------------------------------------------------
//  Forward declarations for opaque pointer members (deferred waves)
// ---------------------------------------------------------------------------
class HTGem;             // SECS wave -- uHGemClass.h (database.h:15)
class TMyBinDispCtrl;    // UI wave   -- MyBinDisp.h  (database.h:5)

// ===========================================================================
//  TIODATA  (database.h:17-38)
//  One row of IO_Table.csv after CSV parse + name-based column resolution.
// ===========================================================================
typedef struct TIODATA
{
    int         Tag;            // index == HSys.IOTable.size() at ctor time
    AnsiString  _CommaText;     // raw CSV row string (or "" if Count<14)
    AnsiString  Type;           // IO type string  (col eioType)
    AnsiString  Alias;          // IO alias string (col eioAlias)
    int         iLane;          // -1 if empty; sets bHasNullData when eMotionNet
    int         iModuleType;    // -1 if empty (no null-flag)
    int         iIP;            // -1 if empty; sets bHasNullData when eMotionNet
    int         iPort;          // -1 if empty (+bHasNullData); HEX parse when ISABase in {1,2,4}
    int         iBit;           // -1 if empty (+bHasNullData)
    int         iInType;        // 0 if empty (NOT -1; contrast other fields)
    int         iISABase;       // eMotionNet(0) if empty; else atoi
    int         iEnable;        // 0 if empty OR bHasNullData
    int         iOnAlarmTime;   // -1 if empty
    int         iOffAlarmTime;  // -1 if empty
    int         iOnDelayTime;   // -1 if empty
    int         iOffDelayTime;  // -1 if empty
    bool        bDelete;        // always false at construction

    TIODATA(AnsiString Str=AnsiString(""));
}TIODATA;

// ===========================================================================
//  TIOTABLENO  (database.h:40-61)
//  Holds name-resolved column indices for IO_Table.csv.
//  Default ctor sets positional fallbacks; SetIOTableNo() overwrites via
//  name-scan (AnsiPos substring, last-match-wins).
// ===========================================================================
typedef struct TIOTABLENO
{
    int eioType;         // col index for "IOType"       (fallback 0)
    int eioAlias;        // col index for "Alias"        (fallback 1)
    int eioLane;         // col index for "Lane"         (fallback 2)
    int eioModuleType;   // col index for "ModuleType"   (fallback 3)
    int eioIP;           // col index for "IP"           (fallback 4)
    int eioPort;         // col index for "Port"         (fallback 5)
    int eioBit;          // col index for "Bit"          (fallback 6)
    int eioInType;       // col index for "InType"       (fallback 7)
    int eioISABase;      // col index for "ISABase"      (fallback 8)
    int eioEnable;       // col index for "Enable"       (fallback 9)
    int eioOnAlarmTime;  // col index for "OnAlarmTime"  (fallback 10)
    int eioOffAlarmTime; // col index for "OffAlarmTime" (fallback 11)
    int eioOnDelayTime;  // col index for "OnDelayTime"  (fallback 12)
    int eioOffDelayTime; // col index for "OffDelayTime" (fallback 13)
    int eioNote;         // col index for "Note"         (fallback 14)
    int eioTotal;        // expected column count == 15

    int SetIOTableNo(AnsiString Str);   // name-scan header row; returns eioTotal on success
    TIOTABLENO();
}TIOTABLENO;

// ===========================================================================
//  TMOTDATA  (database.h:63-98)
//  One row of Mot_Table.csv after CSV parse + name-based column resolution.
// ===========================================================================
typedef struct TMOTDATA
{
    AnsiString  _CommaText;     // raw CSV row
    AnsiString  No;             // Motorname  (col emotNo)
    AnsiString  Alias;          // Alias      (col emotAlias)
    int         iEnable;        // 0 if empty (or -1 in Count<29 path)
    int         iBoardID;       // -1 if empty; 0 if MC88X1
    int         iPort;          // -1 if empty; HexStrToInt(AnsiString) for MC88X1
    int         iIP;            // -1 unless CardModel=="SYNTEK"
    double      dGearRatio;     // atof; default 1.0
    int         iSoftLimitN;    // default -999999
    int         iSoftLimitP;    // default  999999
    AnsiString  CardModel;      // e.g. "SMC", "MC88X1", "SYNTEK"

    int         iDirection;     // default 0
    int         iHomeDirectior; // note: misspelling preserved from BCB6 original
    double      dAcc;           // atof; 1.0 for MC88X1 / INDEX_MOTION_CARD==0 index motors
    double      dDec;           // atof; 1.0 for MC88X1 / INDEX_MOTION_CARD==0 index motors
    int         iInitSpeed;     // default 100
    int         iHomeHighSpeed; // default 100
    int         iHomeLowSpeed;  // default 100
    int         iJogHighSpeed;  // default 100
    int         iJogLowSpeed;   // default 100
    int         iRange;         // 10 for MC88X1; default 1
    int         iRate;          // default 1
    int         iServoAlarmOn;  // default 0
    int         i1P2P;          // default 0
    int         iSensorType;    // default 0
    int         iEncodeType;    // 0 for MC88X1; default 0
    int         iPickLimit;     // only for MTestZ1/Z2; default 0
    int         iLimitLogic;    // 0 for MC88X1 / INDEX_MOTION_CARD==0 index motors
    int         iIn1Logic;      // 0 for MC88X1 / INDEX_MOTION_CARD==0 index motors
    int         iSimulateSpeed; // 1000 for MC88X1; default 10000

    TMOTDATA(AnsiString Str=AnsiString(""));
}TMOTDATA;

// ===========================================================================
//  TMOTNO  (database.h:100-135)
//  Holds name-resolved column indices for Mot_Table.csv.
// ===========================================================================
typedef struct TMOTNO
{
    int emotNo;             // col index for "Motorname"      (fallback 0)
    int emotAlias;          // col index for "Alias"          (fallback 1)
    int emotEnable;         // col index for "Enable"         (fallback 2)
    int emotBoardID;        // col index for "BoardID"        (fallback 3)
    int emotPort;           // col index for "Port"           (fallback 4)
    int emotIP;             // col index for "IP"             (fallback 5)
    int emotGearRatio;      // col index for "GearRatio"      (fallback 6)
    int emotSoftLimitN;     // col index for "SoftLimitN"     (fallback 7)
    int emotSoftLimitP;     // col index for "SoftLimitP"     (fallback 8)
    int emotCardModel;      // col index for "CardModel"      (fallback 9)
    int emotDirection;      // col index for "Direction"      (fallback 10)
    int emotHomeDirectior;  // col index for "HomeDirectior"  (fallback 11) -- sic, misspelling
    int emotAcc;            // col index for "Acc"            (fallback 12)
    int emotDec;            // col index for "Dec"            (fallback 13)
    int emotInitSpeed;      // col index for "InitSpeed"      (fallback 14)
    int emotHomeHighSpeed;  // col index for "HomeHighSpeed"  (fallback 15)
    int emotHomeLowSpeed;   // col index for "HomeLowSpeed"   (fallback 16)
    int emotJogHighSpeed;   // col index for "JogHighSpeed"   (fallback 17)
    int emotJogLowSpeed;    // col index for "JogLowSpeed"    (fallback 18)
    int emotRange;          // col index for "Range"          (fallback 19)
    int emotRate;           // col index for "Rate"           (fallback 20)
    int emotServoAlarmOn;   // col index for "ServoAlarmOn"   (fallback 21)
    int emot1P2P;           // col index for "1P2P"           (fallback 22)
    int emotSensorType;     // col index for "SensorType"     (fallback 23)
    int emotEncodeType;     // col index for "EncodeType"     (fallback 24)
    int emotPickLimit;      // col index for "PickLimit"      (fallback 25)
    int emotLimitLogic;     // col index for "LimitLogic"     (fallback 26)
    int emotIn1Logic;       // col index for "In1Logic"       (fallback 27)
    int emotSimulateSpeed;  // col index for "SimulateSpeed"  (fallback 28)
    int emotTotal;          // expected column count == 29

    TMOTNO();
    int SetMOTTableNo(AnsiString Str);  // name-scan header row; returns emotTotal on full success
}TMOTNO;

// ---------------------------------------------------------------------------
// #if 0 // TODO(wave): TDataModule1 (BDE / VCL TDataModule)
//   class TDataModule1 : public TDataModule  { ... TTable *MotorTable; ... }
//   database.h:139-187; needs <Db.hpp> / <DBTables.hpp>; deferred to BDE wave.
//   extern PACKAGE TDataModule1 *DataModule1;  (database.h:187)
// #endif
// ---------------------------------------------------------------------------

// ===========================================================================
//  SYSTEM_MODULAR  (database.h:193-287)
//  Global system parameter aggregate.  PARTIAL: only the loader-batch members
//  are active.  All other members (com-ports, CCD, RFID, SECS, BinDisp, etc.)
//  are gated #if 0 // TODO(wave).
// ===========================================================================
class SYSTEM_MODULAR
{
    private:
        // (database.h:197-198) -- ACTIVE as of GA-1-B6 (ReadGeneralIni assigns both:
        // golden database.cpp:214/127 -- dNumberPanelDelay from [NUMBER_PANEL]
        // NUMBER_PANEL_DELAY, bUseSocketTemp from [TempCtrl] Socket).
        double dNumberPanelDelay;   // Sam 20240604: display polling period as double
        bool   bUseSocketTemp;      // 9th-axis (Socket) heater enable

    protected:
        // AI(W906-FW-BINDISP1) 20260824: declaration un-gated -- body lands in
        // database.cpp this wave (golden database.cpp:1684-1729) with the
        // user-ruled TMyBinDispOffline substitution; see the body's DEVIATION
        // note. The old blocker (TMyBinDispHT9046 untranslated) is bypassed,
        // not solved: the protocol subclass is still a future wave.
        void InstallColorBinDisplay(int iType);

    public:
        // Ctor/dtor are #if 0 gated in database.cpp (ctor calls ReadGeneralIni
        // + SystemModularInitial; dtor deletes BinDisCtrl/ATKRecipeInfo).
        // The default compiler-generated ctor/dtor are used for the loader batch.
        // (database.h:205-206)
#if 0 // TODO(wave): explicit BCB6 ctor/dtor -- call sites: database.cpp:43, 1731
        _fastcall SYSTEM_MODULAR::SYSTEM_MODULAR();
        _fastcall SYSTEM_MODULAR::~SYSTEM_MODULAR();
#endif

        TQPF_Timer SysTimer;    // (database.h:207)

        // -----------------------------------------------------------------------
        //  Com-port / CCD / RFID / ATC / Laser / Barcode / GroundMan members
        //  (golden database.h:209-249) -- ACTIVE as of GA-1-B6 (ReadGeneralIni
        //  assigns every one of these).  Order/types match golden exactly.
        // -----------------------------------------------------------------------
        AnsiString sNumberPanelComPort;   // Steven 20120217: Com Port made configurable
        AnsiString sNumberPanelComPort2;
        AnsiString sTempComPort;
        AnsiString sTempOmronComPort;
        AnsiString sTempDynamicComPort;
        AnsiString sRTCComPort;
        AnsiString sTorqueComPort;
        AnsiString asATC1ComPort;
        AnsiString asATC2ComPort;
        AnsiString asATC3ComPort;
        AnsiString asATC4ComPort;
        AnsiString TrayStepMotor_ComPort;
        AnsiString sFinePitchComPort;
        AnsiString sFinePitchAdjustmentComPort;
        AnsiString asVisionLightPort;     // RogerYang 20180901: Fix AI CCD demo

        AnsiString asATCSYSTEMIP;
        int asATCSYSTEMPORT;
        int asATCSYSTEMUSEHEAT;           // Ifor 20160506: new ATC interface use-heat count

        AnsiString asLASER_COM[4];        // Eastsun 20260525: +OutArm COM (was [3])
        AnsiString asBarCodeComPort[4];
        int BarcodeBaudRate;
        int InBarcodeBaudRate;
        int BarcodeByteSize;
        int BarcodeStopBit;
        AnsiString BarcodeParity;

        AnsiString asRFIDCom;
        int  iRFIDBaudRate;               // Steven 20220713: RFID reader for SJSEMI
        int  iRFIDByteSize;
        int  iRFIDStopBit;
        AnsiString sRFIDParity;

        AnsiString asOCRComPort;
        AnsiString sTempOmronCom4Port;    // kevin 20130520: 4-Dut temp ctrl (unused by
                                          // ReadGeneralIni; kept for class-layout parity)
        AnsiString asOCRwithTesterComPort;
        AnsiString asAirConPort;
        AnsiString asGroundManComPort;    // Steven 20190828: communication-based Ground Man
        int iGroundManScanPoint;          // KaiChen 20191005
        int iGroundManAlarmOhm;           // KaiChen 20191005

        // AI(W906-SysModWire) 20260720: wired for real -- SystemModularInitial
        // (database.cpp) now assigns this (golden database.h:250). NULL via
        // static zero-init of the global HSys ONLY -- do not stack-allocate
        // SYSTEM_MODULAR (its implicit ctor leaves this pointer
        // uninitialized; only the global HSys's static storage guarantees a
        // zero start).
        HTGem *MyGem;

        // BinDisp member -- forward-declared opaque (database.h:204)
        TMyBinDispCtrl *BinDisCtrl;  // opaque; NULL until UI wave wires InstallColorBinDisplay

        // ATC alarm map (database.h:255-256) -- populated in ctor; gated pending ctor wave
        map<AnsiString, AnsiString>              mapATCAlarmCode;
        map<AnsiString, AnsiString>::iterator    mapATCAlarmCodeIter;

        // -----------------------------------------------------------------------
        //  Config-table loader members (ACTIVE this batch)
        //  (database.h:257-266)
        // -----------------------------------------------------------------------
        void LoadIoData();      // parses IO_Table.csv  -> IOTable / mapIOTable
        void LoadMotData();     // parses Mot_Table.csv -> MotTable / mapMotTable

        TIOTABLENO IoNo;        // name-resolved IO column indices
        TMOTNO     MotNo;       // name-resolved Motor column indices

        vector <TIODATA *>  IOTable;        // one entry per IO_Table.csv data row
        map<AnsiString, AnsiString> mapIOTable;
        map<AnsiString, AnsiString>::iterator mapIOTableIter;

        vector <TMOTDATA *> MotTable;       // one entry per Mot_Table.csv data row
        map<AnsiString, AnsiString> mapMotTable;
        map<AnsiString, AnsiString>::iterator mapMotTableIter;

        // -----------------------------------------------------------------------
        //  ReadGeneralIni (golden database.h:268, database.cpp:301-1537)
        //  ACTIVE as of GA-1-B6 -- see database.cpp for the full translation and
        //  its own per-field range-clause notes (gated sub-blocks for the handful
        //  of fields not yet present anywhere in the ported tree).
        // -----------------------------------------------------------------------
        void ReadGeneralIni();
        void SystemModularInitial();    // real as of W906-SysModWire (wires MyGem) -- see database.cpp

        // -----------------------------------------------------------------------
        //  Array members dependent on iTotalFunction / CCD / RFID
        //  (golden database.h:270-286) -- ACTIVE as of GA-1-B6 (ReadGeneralIni
        //  assigns every one of these).
        // -----------------------------------------------------------------------
        AnsiString asCCDTrayIP[iTotalFunction];      // wei 20161219: Tray Mapping
        AnsiString asCCDTrayPort[iTotalFunction];    // wei 20161219: Tray Mapping
        AnsiString asFix2BGAAICCDIP[2];              // RogerYang 20180901: Fix AI CCD demo
        AnsiString asFix2BGAAICCDPort[2];
        AnsiString asRFIDComPort[2];                 // wei 20180726: RFID
        int RFIDBaudRate;
        int RFIDByteSize;
        int RFIDStopBit;
        AnsiString RFIDParity;
        AnsiString asCCDAlignIP[4];                  // Sam 20181201: Auto Alignment
        AnsiString asCCDAlignPort[4];
        AnsiString asCCDBarCodeIP[4];                // Ifor 20151224
        AnsiString asCCDBarCodePort[4];
};

// Global SYSTEM_MODULAR instance (database.cpp:27 / database.h:288)
extern SYSTEM_MODULAR HSys;

// ---------------------------------------------------------------------------
//  LoadMachineConfig  --  AI(W906-WebBridge) 20260806.  NOT in golden.
//
//  The one call the application should make to bring the machine data layer up.
//  It exists because getting this sequence wrong does not misbehave, it
//  SEGFAULTS, and the trap is completely invisible at the call site:
//
//    CheckAndReadIniDataGeneral() dereferences the global INIFileGeneral with no
//    NULL check (common.cpp, faithful BCB6 behaviour, documented there).
//    INIFileGeneral is set ONLY by OpenGeneralIniFile().  So every caller of
//    ReadGeneralIni() must have opened the ini first -- and in golden the thing
//    that did was the SYSTEM_MODULAR constructor, which is #if 0 in this port.
//
//  Golden performs this work from that constructor, i.e. during static
//  initialisation.  This port deliberately does NOT: `asGeneralPath` is a global
//  in a different translation unit (common.cpp), and standard C++ gives no
//  ordering guarantee between translation units.  BCB6 got away with it via
//  `#pragma package(smart_init)`; reproducing that here would be a latent
//  order-of-initialisation fault, which is a bad trade for matching a shape.
//  So the sequence is explicit and callable, and the caller decides when.
//
//  Call it once, from the application's own init path, BEFORE anything reads
//  IniConfig / CosFunction / LastSet / HSys.
//
//  SIDE EFFECTS, because they are not optional:
//    * ReadGeneralIni SEEDS missing keys, i.e. it WRITES to asGeneralPath.
//      Tests must point asGeneralPath at a scratch copy first -- see
//      tests/test_ga1_readgeneralini.cpp and tests/test_wb_datalayer.cpp.
//    * the ungated ReadLastSetIni() chain creates directories
//      (MyForceDirectories(GetRecipePath())).
//
//  Returns false only if the ini could not be opened.  A missing KEY is not a
//  failure -- that is what the seeding is for.
// ---------------------------------------------------------------------------
bool LoadMachineConfig();

// AI(W906-HT9050-ID) 20260924: machine.typeChoice／typeName／gpibModel 三個 tag 的來源（WebBridgeTags.cpp 發布；本體在 database.cpp 檔尾）。
extern AnsiString W906_GpibModel;                         // ReadGeneralIni() 在 database.cpp:318 讀到的 [Version] Model
const char* W906_MachineTypeName(int machineTypeChoice);  // eMachineType 的列舉名稱；未知值回 ""
AnsiString  W906_MainCaption();                           // AI(W906-CAPTION) 20260930: golden fMain->Caption（main.cpp:9216-9253 / :9590-9607）+ HT9050；本體在 WebBridgeTags.cpp 檔尾，tag machine.caption

#endif // DATABASEH
