// ===========================================================================
//  TesterComm/Handler/HandlerGpibAux.cpp -- see HandlerGpibAux.h.  AI(W906-GB-P6) 20260926.
// ===========================================================================
#include "TesterComm/Handler/HandlerGpibAux.h"
#include "TesterComm/HandlerSettings.h"
#include "TesterComm/Rs232SetupCodes.h"
#include "common.h"        // ReadIniData / WriteIniData / DataPath / GetLastOpenFN
#include "cmydef.h"        // GPIB_MODE, CUSTOMER_CODE, InitialOK, SystemStart, TestIF / TestIF_File (cprod.h)
#include "forms/fMain.h"   // fMain->BackupSetupFile()

// golden cMyDB.h:62 -- declared locally, as in HandlerGpibMsg.cpp:184-188 (the header's default-argument collision).
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");
// golden FTestIF->DoIniDataToForm() (TestIF_File -> the TesterIF page's widgets).  FileRW is compiled into wb_serve only,
// so FileRW/TestIF_File_TesterIF.cpp installs FileRW_TesterIF_DoIniDataToForm here at static init; NULL elsewhere.
void (*W906_GpibAuxRefreshTesterIfPage)() = 0;

namespace {

bool g_enabled = false;   // W906_GpibAuxEnable (wb_serve's W906_TesterCommInit)
bool g_applies = false;   // the last bridge start: GPIB-mode recipe, GPIB engine, extra port on, Handler initialised
bool g_deferred = false;  // the last start came before InitialOK on a GPIB-mode recipe: decide on the first good tick
bool g_asked = false;     // a restart was already asked for since the last start
int g_startPack = -1;     // W906_GpibAuxPackRecipe(TestIF_File.Rs232_Data) at the last start (g_applies only)

// golden GPIB RS232.cpp LoadSetupData defaults (V906 TesterComm/Gpib/GpibAux.cpp): BaudRate 9600, ByteSize _7,
// StopBits _1, Parity Even.
const int kSetupDefBaud = 9600, kSetupDefByteSize = 2, kSetupDefStopBits = 0, kSetupDefParity = 2;

void ReadSetupIni(const AnsiString& setupIni, int* baud, int* bs, int* sb, int* pa)
{
    *baud = kSetupDefBaud;
    *bs = kSetupDefByteSize;
    *sb = kSetupDefStopBits;
    *pa = kSetupDefParity;
    if (!FileExists(setupIni))
        return;
    *baud = ReadIniData(setupIni, "COMPort", "BaudRate", kSetupDefBaud);
    *bs = ReadIniData(setupIni, "COMPort", "ByteSize", kSetupDefByteSize);
    *sb = ReadIniData(setupIni, "COMPort", "StopBits", kSetupDefStopBits);
    *pa = ReadIniData(setupIni, "COMPort", "Parity", kSetupDefParity);
}

// The framing the engine opened with when nothing was published (golden: Setup.ini as read in LoadSetupData).
int SetupIniPack()
{
    int baud, bs, sb, pa;
    ReadSetupIni(W906_GPIBAUX_SETUP_INI, &baud, &bs, &sb, &pa);
    return testercomm::HsPackAuxFraming(baud, bs, sb, pa);
}

// Port check + seeding for the current recipe; returns the value to publish (-1 = golden Setup.ini).
int Prepare()
{
    g_applies = false;
    g_startPack = -1;
    if (TestIF.iTestType != GPIB_MODE || !W906_GpibAuxPortOn(W906_GPIBAUX_GENERAL_INI, CUSTOMER_CODE))
        return -1;
    g_applies = true;

    const AnsiString recipe = GetLastOpenFN();
    const AnsiString td = DataPath + recipe + "\\Tester.Data";
    RS232_DATA rd = TestIF_File.Rs232_Data;
    AnsiString detail;
    const int r = W906_GpibAuxSeedRecipeFile(td, W906_GPIBAUX_SETUP_INI, &rd, &detail);
    if (r == 1)
    {
        // what golden spbSaveClick does after SaveSetupFile: the in-memory copy (ReadTestIFFile), the page, the recipe
        // backup.  SECS SaveRecipe is not reported: no operator saved (see the ledger, P6 Q2).
        TestIF_File.Rs232_Data = rd;
        if (W906_GpibAuxRefreshTesterIfPage)
            W906_GpibAuxRefreshTesterIfPage();   // else a later TesterIF page save would write the old widget values back
        if (fMain)
            fMain->BackupSetupFile();                                           //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
        NewRecordProcess("", "TesterIF", "P6 Q2(a): recipe " + recipe + " [RS-232C] seeded from " +
                                             AnsiString(W906_GPIBAUX_SETUP_INI) + " [COMPort] (GPIB extra RS232 port): " +
                                             detail);
    }
    else if (r < 0)
    {
        NewRecordProcess("", "TesterIF", "P6 Q2(a): recipe " + recipe + " not seeded (" + td +
                                             " missing or not writable) -- the GPIB extra RS232 port keeps " +
                                             AnsiString(W906_GPIBAUX_SETUP_INI));
    }
    g_startPack = W906_GpibAuxPackRecipe(TestIF_File.Rs232_Data);
    if (r < 0)
        return -1;
    if (g_startPack < 0)
    {
        AnsiString s;
        s.sprintf("P6 2A: recipe %s [RS-232C] BaudRate=%d Bit Length=%d Stop Bit=%d Parity=%d is not a usable framing -- "
                  "the GPIB extra RS232 port keeps %s", recipe.c_str(), TestIF_File.Rs232_Data.Baud_Rate,
                  TestIF_File.Rs232_Data.Bit_Length, TestIF_File.Rs232_Data.Stop_Bit, TestIF_File.Rs232_Data.Parity,
                  W906_GPIBAUX_SETUP_INI);
        NewRecordProcess("", "TesterIF", s);
    }
    return g_startPack;
}

}  // namespace

void W906_GpibAuxEnable(bool on)
{
    g_enabled = on;
    g_applies = g_deferred = g_asked = false;
    g_startPack = -1;
    testercomm::HsGpibAuxFraming().store(-1);
}

bool W906_GpibAuxPortOn(const AnsiString& gpibGeneralIni, int customerCode)
{
    // golden (V906 GpibAux.cpp OpenTesterComm)                                //Jou 20240828 : 改成可以開關RS232功能
    const bool bDefault = customerCode == CC_HONPREC_QC || customerCode == CC_MAXIM_THAILAND ||
                          customerCode == CC_MAXIM || customerCode == CC_Microchip_Thai ||
                          customerCode == CC_Microchip_Phil || customerCode == CC_Microchip_China ||
                          customerCode == CC_SCC;
    if (!FileExists(gpibGeneralIni))
        return bDefault;
    return ReadIniData(gpibGeneralIni, "OpenTesterComm", "RS232", bDefault);
}

int W906_GpibAuxPackRecipe(const RS232_DATA& rd)
{
    // Out-of-range indices fall into golden CheckRs232StandardIni's else branch, as the mapping functions document.
    return testercomm::HsPackAuxFraming(rd.Baud_Rate, W906_Rs232ByteSizeCode(rd.Bit_Length),
                                        W906_Rs232StopBitsCode(rd.Stop_Bit), W906_Rs232ParityCode(rd.Parity));
}

int W906_GpibAuxSeedRecipeFile(const AnsiString& testerData, const AnsiString& setupIni, RS232_DATA* rd,
                               AnsiString* detail)
{
    if (!FileExists(testerData))
        return -1;
    if (ReadIniData(testerData, "RS-232C", W906_GPIBAUX_MARKER, 0) == 1)
        return 0;

    int baud, bs, sb, pa;
    ReadSetupIni(setupIni, &baud, &bs, &sb, &pa);
    RS232_DATA nd = *rd;
    if (baud > 0)
        nd.Baud_Rate = baud;
    const int bl = W906_Rs232BitLengthFromCode(bs), sbi = W906_Rs232StopBitFromCode(sb), pai = W906_Rs232ParityFromCode(pa);
    if (bl >= 0)
        nd.Bit_Length = bl;
    if (sbi >= 0)
        nd.Stop_Bit = sbi;
    if (pai >= 0)
        nd.Parity = pai;

    // the keys and value forms golden SaveSetupFile writes (cTesterIF.cpp; V906 TestIF_File_TesterIF.gen.inc :932-935)
    WriteIniData(testerData, "RS-232C", "BaudRate", nd.Baud_Rate);
    WriteIniData(testerData, "RS-232C", "Bit Length", nd.Bit_Length);
    WriteIniData(testerData, "RS-232C", "Stop Bit", nd.Stop_Bit);
    WriteIniData(testerData, "RS-232C", "Parity", nd.Parity);
    WriteIniData(testerData, "RS-232C", W906_GPIBAUX_MARKER, 1);
    if (ReadIniData(testerData, "RS-232C", W906_GPIBAUX_MARKER, 0) != 1 ||
        ReadIniData(testerData, "RS-232C", "BaudRate", -1) != nd.Baud_Rate)
        return -1;

    if (detail)
    {
        detail->sprintf("BaudRate %d->%d, Bit Length %d->%d, Stop Bit %d->%d, Parity %d->%d (Setup.ini%s ByteSize=%d "
                        "StopBits=%d Parity=%d)", rd->Baud_Rate, nd.Baud_Rate, rd->Bit_Length, nd.Bit_Length,
                        rd->Stop_Bit, nd.Stop_Bit, rd->Parity, nd.Parity,
                        FileExists(setupIni) ? "" : " missing, engine defaults", bs, sb, pa);
    }
    *rd = nd;
    return 1;
}

void W906_GpibAuxBeforeBridgeStart(int effectiveTestType)
{
    g_applies = g_deferred = g_asked = false;
    g_startPack = -1;
    int publish = -1;
    if (g_enabled && effectiveTestType == GPIB_MODE && TestIF.iTestType == GPIB_MODE)
    {
        if (InitialOK)
            publish = Prepare();
        else
            g_deferred = true;   // the recipe may not be loaded yet: W906_GpibAuxNeedsRestart decides later
    }
    testercomm::HsGpibAuxFraming().store(publish);
}

bool W906_GpibAuxNeedsRestart(bool bridgeUp)
{
    if (!g_enabled || !bridgeUp || g_asked || !InitialOK || SystemStart)
        return false;
    if (g_deferred)
    {
        // The engine took Setup.ini; restart only when the recipe says otherwise.
        g_deferred = false;
        const int publish = Prepare();
        testercomm::HsGpibAuxFraming().store(publish);
        if (publish < 0 || publish == SetupIniPack())
            return false;
        g_asked = true;
        return true;
    }
    if (!g_applies || W906_GpibAuxPackRecipe(TestIF_File.Rs232_Data) == g_startPack)
        return false;
    g_asked = true;
    return true;
}
