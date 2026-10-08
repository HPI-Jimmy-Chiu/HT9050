//AI(W906-IFORGPT-IG-3) 20261008: Real heater payload tests; no source-text assertions.
// Golden 0618: uHeaterThread.cpp:56-66; csystem.cpp:1109-1410,16530-16604.
// No IO table, controller, live socket or hardware output is installed here.
// Sensor fixtures use the real reader's unknown-bus fall-through (ret=false).
// The ATC error is seeded through its real public socket-error event, not a stub.
// Does not start a worker thread. The live beat remains FastClockJobs / HeaterSimTick.
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "csystem.h"
#include "LastSet.h"
#include "Config.h"
#include "uHeaterThread.h"
#include "forms/fMain.h"
#include "mysensor.h"
#include "myswitch.h"
#include "ATC/ATCInterface.h"
#include "canary_support.h"
#include "st02_test_containment.h"

#include <cstdio>
#include <string>

namespace ht9045
{
void W906_FastClockHeaterBeat();
}

extern TQPF_Timer DoHeaterOnDelay;
static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok)
    {
        ++g_fail;
        std::printf("FAIL line %d: %s\n", line, what);
    }
    else
    {
        std::printf("PASS: %s\n", what);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

static const int kDoors[] = { SnHeaterDoor, SnHeaterDoor2, SnHeaterDoor3, SnHeaterDoor4 };
static const int kEmg[] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG, SnAllEMG };

static void setSensor(int sensor, bool on, bool enabled = true)
{
    Sen[sensor].Enable = enabled;
    Sen[sensor].ISABase = -1; // No driver branch is taken by TMySensor::IsOn/IsOff.
    Sen[sensor].Type = on ? 0 : 1;
}

static void baseFixture()
{
    InitialOK = true;
    SystemInitialOK = false;
    SystemStart = false;
    ATC_SYSTEM = eATCUninstall;
    CUSTOMER_CODE = CC_PTI;
    TC401HeaterControl = NoHeater; // DoThermo's golden first-line return.
    iControlPanelMode = 0;
    REAL_TIME_CCD = false;
    Enable_PLCSafety_IO = false;
    LastSet.iTemperature = Tempture_Ambient;
    IniConfig.bAmbRunChamberFanCanStop = false;
    IniConfig.bOpenDoorNotStopFan = false;
    IniConfig.bKoreaFunction = false;
    IniConfig.bL21PowerOffTemperature = false;
    IniConfig.bTemp25degControl = false;
    IniConfig.bL20AbientGuardBand = false;
    Temperature.bAmbientGuardbandCheck = false;
    Temperature.bAmbUsingAFan = false;
    Temperature.bATCActiveCooling = false;
    Temperature.bActiveHeatGun = false;
    Temperature.iIndexHeatMode = HeadOnly;
    Prod.bAfterOpenHeatDoorUseInitialDelay = true;
    fMain->chkHeaterOk->Checked = true;
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
    {
        Sen[i].Enable = false;
    }
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i)
    {
        SW[i].Enable = false;
    }
    for (int i = 0; i < tcTotalCount; ++i)
    {
        bUT150Install[i] = false;
        AMBIENT_TEMP_CHECK[i] = false;
        UN150Read[i] = 25.0;
    }
    for (int i = 0; i < 4; ++i)
    {
        setSensor(kDoors[i], true);
        bHeaterDoorIsOpen[i] = false;
    }
    for (int i = 0; i < 5; ++i)
    {
        setSensor(kEmg[i], true);
    }
    DoHeaterOnDelay.SetSecAndOn(0);
    DoCloseHeadterDelay.SetSecAndOn(0);
    W906_ShowMyMessage_Reset();
}

static void seedAtcError()
{
    int error = 10061;
    ATC60System& atc = ATCInterfaceForm->ATC_60_SYS;
    atc.ClientSocket1Error(atc.clientsocket, atc.clientsocket->Socket, eeConnect, error);
    CHECK(error == 0);
    // IsConnected sees this real socket object's in-memory state; Open is never called.
    atc.clientsocket->Socket->Connected = true;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    if (!W906TestInsideCtestRoots("IforGPT_HeaterRuntime"))
    {
        return 2;
    }
    CHECK(fMain && fMain->chkHeaterOk && ATCInterfaceForm);
    if (!fMain || !fMain->chkHeaterOk || !ATCInterfaceForm)
    {
        return 1;
    }
    THeaterThread thread(true);

    std::puts("T1 InitialOK=false preserves every poller's state");
    baseFixture();
    seedAtcError();
    ATC_SYSTEM = eATC60;
    SystemInitialOK = true;
    InitialOK = false;
    SW[SwHeaterRelay].OutValue = true;
    SW[SwHeaterFan].OutValue = false;
    for (int i = 0; i < 4; ++i)
    {
        bHeaterDoorIsOpen[i] = true;
    }
    thread.HeaterThreadProcess();
    CHECK(SW[SwHeaterRelay].OutValue && !SW[SwHeaterFan].OutValue);
    CHECK(bHeaterDoorIsOpen[0] && bHeaterDoorIsOpen[1] && bHeaterDoorIsOpen[2] && bHeaterDoorIsOpen[3]);
    CHECK(W906_ShowMyMessage_Count == 0);
    AnsiString pending;
    CHECK(ATCInterfaceForm->ATC_60_SYS.GetErrorMessage(pending));

    std::puts("T2 GATE 1: actual ATC socket errors reach ShowMyMessage once; guards keep them queued");
    baseFixture();
    seedAtcError();
    thread.HeaterThreadProcess();
    CHECK(W906_ShowMyMessage_Count == 0);
    CHECK(ATCInterfaceForm->ATC_60_SYS.GetErrorMessage(pending));
    const int atcTypes[] = { eATC60, eATC30 };
    for (int i = 0; i < 2; ++i)
    {
        baseFixture();
        seedAtcError();
        ATC_SYSTEM = atcTypes[i];
        thread.HeaterThreadProcess();
        CHECK(W906_ShowMyMessage_Count == 0); // SystemInitialOK=false.
        SystemInitialOK = true;
        thread.HeaterThreadProcess();
        CHECK(W906_ShowMyMessage_Count == 1);
        CHECK(std::string(W906_ShowMyMessage_LastS1.c_str()).find("10061") != std::string::npos);
        CHECK(!ATCInterfaceForm->ATC_60_SYS.GetErrorMessage(pending));
        thread.HeaterThreadProcess();
        CHECK(W906_ShowMyMessage_Count == 1);
    }

    std::puts("T3 GATE 3/4: fresh door state precedes heater decision; ambient relay off, fan on");
    baseFixture();
    for (int i = 0; i < 4; ++i)
    {
        bHeaterDoorIsOpen[i] = true; // Stale open flags must be refreshed first.
    }
    SW[SwHeaterRelay].OutValue = true;
    SW[SwHeaterFan].OutValue = false;
    thread.HeaterThreadProcess();
    CHECK(!bHeaterDoorIsOpen[0] && !bHeaterDoorIsOpen[1] && !bHeaterDoorIsOpen[2] && !bHeaterDoorIsOpen[3]);
    CHECK(!SW[SwHeaterRelay].OutValue && SW[SwHeaterFan].OutValue);

    std::puts("T4 GATE 4: hot permit turns relay on; EMG always cuts it");
    baseFixture();
    LastSet.iTemperature = Tempture_Hot;
    bHeatOverTenErrorOK = true;
    SW[SwHeaterRelay].OutValue = false;
    thread.HeaterThreadProcess();
    CHECK(SW[SwHeaterRelay].OutValue);
    setSensor(SnFrontLeftEMG, false);
    thread.HeaterThreadProcess();
    CHECK(!SW[SwHeaterRelay].OutValue && !fHeaterOK);

    std::puts("T5 GATE 3: each door input, optional door Enable, contact delay and SIM split");
    for (int open = 0; open < 4; ++open)
    {
        baseFixture();
        setSensor(kDoors[open], false);
        iInitContactCount = 9;
        bDoAfterOpenHeatDoorUseInitialDelay = false;
        SW[SwHeaterFan].OutValue = true;
        thread.HeaterThreadProcess();
        for (int i = 0; i < 4; ++i)
        {
#ifdef SOFT_SIMULTE
            CHECK(!bHeaterDoorIsOpen[i]);
#else
            CHECK(bHeaterDoorIsOpen[i] == (i == open));
#endif
        }
#ifdef SOFT_SIMULTE
        CHECK(iInitContactCount == 9 && !bDoAfterOpenHeatDoorUseInitialDelay);
        CHECK(SW[SwHeaterFan].OutValue);
#else
        CHECK(iInitContactCount == (open < 2 ? 0 : 9));
        CHECK(bDoAfterOpenHeatDoorUseInitialDelay == (open < 2));
        CHECK(!fHeaterOK && !SW[SwHeaterFan].OutValue);
#endif
    }
    for (int optional = 2; optional < 4; ++optional)
    {
        baseFixture();
        setSensor(kDoors[optional], false, false);
        bHeaterDoorIsOpen[optional] = true;
        thread.HeaterThreadProcess();
        CHECK(!bHeaterDoorIsOpen[optional]);
    }

    std::puts("T6 SIM checkbox remains the existing CheckHeater guard input");
#ifdef SOFT_SIMULTE
    baseFixture();
    fMain->chkHeaterOk->Checked = false;
    thread.HeaterThreadProcess();
    CHECK(!fHeaterOK && !fHeaterStableOK && iHeaterWait == 0 && iATCOnLine);
    fMain->chkHeaterOk->Checked = true;
    thread.HeaterThreadProcess();
    CHECK(fHeaterOK && fHeaterStableOK && iHeaterWait == 1 && iATCOnLine);
#endif
    std::puts("T7 live fast-clock beat preserves the existing ATC/door/relay/EMG behavior");
    baseFixture();
    seedAtcError();
    ATC_SYSTEM = eATC60;
    SystemInitialOK = true;
    for (int i = 0; i < 4; ++i)
    {
        bHeaterDoorIsOpen[i] = true;
    }
    SW[SwHeaterRelay].OutValue = true;
    SW[SwHeaterFan].OutValue = false;
    ht9045::W906_FastClockHeaterBeat();
    CHECK(W906_ShowMyMessage_Count == 1);
    CHECK(!bHeaterDoorIsOpen[0] && !bHeaterDoorIsOpen[1] &&
          !bHeaterDoorIsOpen[2] && !bHeaterDoorIsOpen[3]);
    CHECK(!SW[SwHeaterRelay].OutValue && SW[SwHeaterFan].OutValue);
    baseFixture();
    LastSet.iTemperature = Tempture_Hot;
    bHeatOverTenErrorOK = true;
    SW[SwHeaterRelay].OutValue = false;
    ht9045::W906_FastClockHeaterBeat();
    CHECK(SW[SwHeaterRelay].OutValue);
    setSensor(SnFrontLeftEMG, false);
    ht9045::W906_FastClockHeaterBeat();
    CHECK(!SW[SwHeaterRelay].OutValue && !fHeaterOK);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
