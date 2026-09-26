// ===========================================================================
//  TesterComm/Handler/HandlerGpibAux.h -- the GPIB program's extra RS232 port follows the recipe (tester-comm P6).
//  AI(W906-GB-P6) 20260926.  User ruling 2A + Q2 = (a) (github-59 relayed, steven-handoff FROM_STEVEN §1 P6).
//
//  Golden: H9046_32GPIB.exe opens a second COM port (RS232.cpp TfRS232Main::CommTester) for customer codes 0 / 860-863 /
//  943 / 985 or when its general.ini [OpenTesterComm] RS232=1, with the framing from D:\RS232Standard\System\Setup.ini
//  [COMPort].  The Handler writes that file only for RS232-mode recipes (cTesterIF.cpp CheckRs232StandardIni), so a
//  GPIB-mode recipe's [RS-232C] values were never used and are often defaults.
//  Ruling 2A: the port follows the Handler recipe (TestIF_File.Rs232_Data).  Ruling Q2 (a): on the first run, each
//  GPIB-mode recipe is seeded ONCE from the current Setup.ini [COMPort] values (marker [RS-232C] W906_SeededFromSetupIni=1
//  in its Tester.Data); from then on the recipe is authoritative.
//
//  Only a GPIB-mode recipe (TestIF.iTestType==GPIB_MODE, bridge on the GPIB engine) with the extra port on is affected.
//  Every other case publishes -1 and the engine keeps golden's Setup.ini values.  Nothing runs until
//  W906_GpibAuxEnable(true) (TesterCommWiring.cpp W906_TesterCommInit, i.e. wb_serve): a ctest that drives
//  THandlerTesterSide directly never reads or writes the production recipe / Setup.ini files.
// ===========================================================================
#ifndef HT9045_TESTERCOMM_HANDLER_HANDLERGPIBAUX_H
#define HT9045_TESTERCOMM_HANDLER_HANDLERGPIBAUX_H

#include "vclcompat/vcl_compat.h"   // AnsiString
#include "cprod.h"                // RS232_DATA

// golden paths (golden GPIB RS232.cpp OpenTesterComm / LoadSetupData)
#define W906_GPIBAUX_GENERAL_INI  "D:\\GPIB9045\\system\\general.ini"
#define W906_GPIBAUX_SETUP_INI    "D:\\RS232Standard\\System\\Setup.ini"
#define W906_GPIBAUX_MARKER       "W906_SeededFromSetupIni"   // Tester.Data [RS-232C]

// Composition root switch (TesterCommWiring.cpp).  Off = everything below is inert and -1 is published.
void W906_GpibAuxEnable(bool on);

// Installed by FileRW/TestIF_File_TesterIF.cpp (wb_serve only): refreshes the TesterIF page's widgets after seeding.
extern void (*W906_GpibAuxRefreshTesterIfPage)();

// golden GPIB RS232.cpp OpenTesterComm (V906 TesterComm/Gpib/GpibAux.cpp): is the extra port opened?  Read-only here
// (the engine's CheckAndReadIniData writes the default back; the Handler does not).
bool W906_GpibAuxPortOn(const AnsiString& gpibGeneralIni, int customerCode);

// The recipe's framing as the engine's packed value (TesterComm/HandlerSettings.h), -1 when a field is invalid.
int W906_GpibAuxPackRecipe(const RS232_DATA& rd);

// Seed one recipe file from Setup.ini [COMPort] -- file work only (no fMain, no page, no log), for the ctest.
//   1 = seeded now (rd updated to the written values, *detail says old -> new), 0 = marker already there (nothing
//   written), -1 = no recipe file or the write did not stick.  A missing Setup.ini seeds the engine's own defaults
//   (9600 / 7 / 1 / Even, GpibAux.cpp LoadSetupData) -- what the port used; a value that is not an SPComm ordinal keeps
//   the recipe's value for that field.
int W906_GpibAuxSeedRecipeFile(const AnsiString& testerData, const AnsiString& setupIni, RS232_DATA* rd,
                               AnsiString* detail);

// Handler thread, THandlerTesterSide::StartBridgeProgram right before the engine (re)starts.  Seeds the current recipe
// if needed (normal save tail: fMain->BackupSetupFile(), page widgets refreshed, NewRecordProcess) and publishes
// HsGpibAuxFraming for the engine's LoadSetupData.
void W906_GpibAuxBeforeBridgeStart(int effectiveTestType);

// Handler thread, every tick (TesterCommWiring.cpp): true = the port no longer matches the recipe (the recipe's RS232
// values changed, or the first start ran before the recipe was loaded) -> the caller closes the bridge, which then
// restarts through WakeupGPIB like golden's CheckRs232StandardIni -> CloseGpibProgram.  Only asks once per start.
bool W906_GpibAuxNeedsRestart(bool bridgeUp);

#endif
