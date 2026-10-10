//AI(W906-IFORGPT-IG8) 20261009: run the real PAUSE caller and FileRW picker-count writer.
// golden 913 e9908638 main.cpp:6637-6641. Recipe files live in a per-process CTest scratch.
// O20 ON: all 48 keys, re-read, repeated PAUSE, recipe change; OFF: existing file unchanged,
// missing file stays missing (also D05_1=true). D05_1=true with O20 ON is deliberately not run:
// the generated body has a hardcoded real SocketCount.ini. No generated source is modified.
// Real StopAllMotor runs over one recorder derived from the existing Sim HAL. The SCKART
// recorder delegates to the real AccessFile after checking SoftStop -> DecStop -> SCKART.
#include "WebStart.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "common.h"
#include "MachineType.h"
#include "forms/fSCKART.h"
#include "JsonBridge/FormBridge.h"
#include "w906_ctest_guard.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Motor/mySimMotor.h"
#include "Motor/mymotor.h"
#pragma GCC diagnostic pop
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

extern void FileRW_StartCondition_Boot();
extern void FileRW_StartCondition_ReadWriteStartCondition(bool);

//AI(W906-IFORGPT-IG8) 20261009: no HTTP form pages in this runtime fixture.
// The real FormJson lock and FormBridge code are linked; only their page registry is empty.
namespace ht9045 {
namespace formbridge {
extern const BridgeDesc* const kBridges[] = { 0 };
extern const std::size_t kBridgeCount = 0;
}
}

//AI(W906-IFORGPT-IG8) 20261009: unrelated Configuration-page linkage; fail if executed.
// Resolve this symbol locally so the globals archive does not extract _fallback.cpp,
// which would collide with the real _EditList proxy functions linked by this test.
void FileRW_IniConfig_ChangeCBListProperty()
{
    std::fprintf(stderr, "unexpected Configuration-page helper in PAUSE test\n");
    std::abort();
}

namespace {
int checks = 0, failures = 0, stops = 0, sckCalls = 0;
AnsiString conditionFile;
std::string beforePause;
bool observeOrder = false;

void check(bool ok, const char* message)
{
    ++checks;
    if (!ok)
    {
        ++failures;
        std::printf("FAIL %s\n", message);
    }
}

std::string readBytes(const AnsiString& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

void writeBytes(const AnsiString& path, const char* contents)
{
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
    file << contents;
    if (!file)
    {
        std::printf("FAIL fixture write: %s\n", path.c_str());
        std::exit(2);
    }
}

class PauseMotor : public TMySimMotor
{
public:
    virtual void DecStop()
    {
        ++stops;
        if (observeOrder)
        {
            check(SoftStop, "SoftStop is already true at the real motor stop");
            check(readBytes(conditionFile) == beforePause, "picker file unchanged at motor stop");
        }
        TMySimMotor::DecStop();
    }
};

class PauseSck : public TfSCKART
{
public:
    virtual void AccessFile(bool bRead, int iAccess = -1)
    {
        ++sckCalls;
        check(!bRead && iAccess == -1, "PAUSE preserves SCKART save arguments");
        if (observeOrder)
        {
            check(SoftStop && stops > 0, "real motor stop precedes SCKART save");
            check(readBytes(conditionFile) == beforePause, "picker file unchanged at SCKART entry");
        }
        TfSCKART::AccessFile(bRead, iAccess);
    }
};

void setCounts(int base)
{
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            TestIF_File.InArmPickerLifeCnt[row][col] = base + row * 100 + col;
            TestIF_File.OutArmPickerLifeCnt[row][col] = base + 1000 + row * 100 + col;
        }
        for (int col = 0; col < 8; ++col)
        {
            TestIF_File.Arm1PickerLifeCnt[row][col] = base + 2000 + row * 100 + col;
            TestIF_File.Arm2PickerLifeCnt[row][col] = base + 3000 + row * 100 + col;
        }
    }
}

void checkKey(const char* prefix, int row, int col, int value)
{
    AnsiString key;
    key.sprintf("%s%d_%d", prefix, row, col);
    AnsiString what;
    what.sprintf("persisted %s expected %d", key.c_str(), value);
    check(ReadIniData(conditionFile, "O_Count", key, -98765) == value, what.c_str());
}

void checkFile(int base)
{
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            checkKey("InArmSuckCnt", row, col, base + row * 100 + col);
            checkKey("OutArmSuckCnt", row, col, base + 1000 + row * 100 + col);
        }
        for (int col = 0; col < 8; ++col)
        {
            checkKey("Arm1SuckCnt", row, col, base + 2000 + row * 100 + col);
            checkKey("Arm2SuckCnt", row, col, base + 3000 + row * 100 + col);
        }
    }
}

void checkMemory(int base)
{
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            check(TestIF_File.InArmPickerLifeCnt[row][col] == base + row * 100 + col, "re-read InArm count");
            check(TestIF_File.OutArmPickerLifeCnt[row][col] == base + 1000 + row * 100 + col, "re-read OutArm count");
        }
        for (int col = 0; col < 8; ++col)
        {
            check(TestIF_File.Arm1PickerLifeCnt[row][col] == base + 2000 + row * 100 + col, "re-read Arm1 count");
            check(TestIF_File.Arm2PickerLifeCnt[row][col] == base + 3000 + row * 100 + col, "re-read Arm2 count");
        }
    }
}

void selectRecipe(const char* name)
{
    writeBytes(LastDataPath, name);
    check(MyForceDirectories(GetRecipePath()), "create isolated recipe directory");
    conditionFile = GetRecipeFileName("HandlerCondition.Data");
    check(W906CtestGuardInScratch(conditionFile.c_str()), "real recipe resolver stays in scratch");
}

void pause(TfMainWeb& main, const char* source, bool running)
{
    SystemStart = running;
    SoftStop = false;
    bStartMoveSpeed = true;
    beforePause = readBytes(conditionFile);
    stops = 0;
    sckCalls = 0;
    observeOrder = true;
    check(main.PauseFromWeb(source), "real PauseFromWeb returns true");
    observeOrder = false;
    check(SoftStop && !bStartMoveSpeed, "PAUSE retains SoftStop and clears start speed");
    check(stops == 1 && sckCalls == 1, "one real motor stop then one SCKART save");
}
}

int main()
{
    if (!W906TestRequireCtestRedirects("IforGPT_StartConditionPauseRuntime"))
    {
        return 2;
    }
    AnsiString root;
    root.sprintf("%s/ig8_pause_%lu_%lu/", std::getenv("W906_INIDATA_ROOT"),
                 (unsigned long)GetCurrentProcessId(), (unsigned long)GetTickCount());
    check(MyForceDirectories(root), "create per-process fixture directory");
    DataPath = root + "recipes/";
    LastDataPath = root + "setup.inf";
    TfMain* oldMain = fMain;
    TfSCKART* oldSck = fSCKART;
    TfMainWeb* main = new TfMainWeb();
    PauseSck* sck = new PauseSck();
    fMain = main;
    fSCKART = sck;
    PauseMotor* motor = new PauseMotor();
    for (int i = 0; i < TOTAL_MOTOR; ++i)
    {
        MOT[i].Motor = 0;
    }
    MOT[MInArmX].Motor = motor;
    MOT[MInArmX].Mot_Name = MInArmX;
    motor->Enable = true;
    motor->PServoAlarmOn = 1;
    IniConfig.bEnable_SECS_GEM = false;
    IniConfig.bG14UseStartSoundAlarm = false;
    IniConfig.bUseAutoSiteMapping = false;
    IniConfig.bI21EnableASM = false;
    IniConfig.bN29_ParameterCheckForGMTest = false;
    IniConfig.bD05_1SaveSocketCntByHandler = false;
    IniConfig.bO20InOutArmPickerLifeTimeCount = true;
    CosFunction.bUseSCKART = true;
    CosFunction.bTrayOCR = false;
    CUSTOMER_CODE = CC_PTI;
    selectRecipe("recipeA\n");
    writeBytes(conditionFile, "[untouched]\nmarker=keep\n[O_Count]\nO_20InOutArmLifeCntSet=2468\n");
    const std::string unbootedBefore = readBytes(conditionFile);
    pause(*main, "before FileRW boot", false);
    check(readBytes(conditionFile) == unbootedBefore, "unbooted wrapper preserves file");
    FileRW_StartCondition_Boot();
    setCounts(101);
    pause(*main, "Auto PAUSE", true);
    checkFile(101);
    check(ReadIniData(conditionFile, "untouched", "marker", AnsiString("missing")) == "keep", "unrelated section preserved");
    check(ReadIniData(conditionFile, "O_Count", "O_20InOutArmLifeCntSet", -1) == 2468, "PAUSE does not rewrite alarm setpoint");
    check(FileExists(GetRecipeFileName("Tester.Data")), "real SCKART AccessFile writes sandbox Tester.Data");
    setCounts(9999);
    FileRW_StartCondition_ReadWriteStartCondition(true);
    checkMemory(101);
    setCounts(0);
    pause(*main, "Manual PAUSE", false);
    checkFile(0);
    const AnsiString recipeA = conditionFile;
    const std::string recipeABefore = readBytes(recipeA);
    selectRecipe("recipeB\n");
    setCounts(9001);
    pause(*main, "Alarm Recovery PAUSE", false);
    checkFile(9001);
    check(readBytes(recipeA) == recipeABefore, "recipe switch leaves old recipe unchanged");
    IniConfig.bO20InOutArmPickerLifeTimeCount = false;
    beforePause = readBytes(conditionFile);
    const std::string disabledBefore = beforePause;
    setCounts(88888);
    pause(*main, "O20 OFF", true);
    check(readBytes(conditionFile) == disabledBefore, "O20 OFF preserves existing recipe file byte for byte");
    selectRecipe("recipeOff\n");
    pause(*main, "O20 OFF missing file", false);
    check(!FileExists(conditionFile), "O20 OFF does not create recipe HandlerCondition.Data");
    IniConfig.bD05_1SaveSocketCntByHandler = true;
    pause(*main, "O20 OFF handler count selected", false);
    check(!FileExists(conditionFile), "O20 guard returns before hardcoded handler path selection");
    fMain = oldMain;
    fSCKART = oldSck;
    MOT[MInArmX].Motor = 0;
    delete motor;
    delete sck;
    delete main;
    std::printf("IG8 PAUSE: %d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
