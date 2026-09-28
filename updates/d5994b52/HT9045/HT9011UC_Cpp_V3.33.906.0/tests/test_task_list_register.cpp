// ===========================================================================
//  test_task_list_register.cpp -- AI(W906-TASKLIST) 20260927
//
//  W906_BootRegisterTaskList() (cStateRecord.cpp EOF) = golden TfMain::FormShow main.cpp:9717-10036 + :10049-10053:
//  the QueueTaskList[1..281] registrations that State Record's Task_ListWithTime.csv is made of.  Before it the port's
//  ring was never registered, so every row of that file was empty (0927 flow-comparison run 1: 152,307 bytes of commas).
//   [1] nothing registered before the call; 265 rows registered after (281 golden rows - 4 golden-commented - 12 gated; AI(W906-FLOW-2) 20260928: TestZTask row 198 un-gated);
//       golden's first / last rows; the port-side mappings point at the variables the port's code actually drives:
//       &fContact->X -> fContactForm (the global fContact is the TfContactShim stand-in), &fBarCode->X -> the port globals;
//       a gated row (SetStepMotorTask, dmTrayMotor not in the port) stays empty; StringGrid2 column 0 = the aliases.
//   [2] the golden recorder (TMyQueue10::CheckTaskChange, called per tick by csystem.cpp:30700) now records a change,
//       and TfMain::SaveTaskList writes the row in the reference StateRecord shape:
//       `Alias, hh:mm:ss.mmm, newest, hh:mm:ss.mmm, older, ...`.
//  Writes only %TEMP%\ht9045_tasklist_test\ (SaveTaskList's own two files), removed at the end.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                          // QueueTaskList, qTaskCount
#include "cpublic.h"                         // TMyQueue10
#include "forms/fMain.h"
#include "forms/fHome.h"
#include "forms/fContact.h"
#include "acarry.h"                          // AutoSHT1Task
#include "BarCode/BarCode_Shuttle1_Scan.h"   // iInitialBarcodeInShuttle1Task

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

extern void W906_BootRegisterTaskList();

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) { ++g_pass; std::printf("  PASS: %s\n", #c); } \
                      else { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", #c, __LINE__); } } while (0)

static int Registered()
{
    int n = 0;
    for (int i = 1; i < qTaskCount; ++i)
        if (QueueTaskList[i].iTask != NULL) ++n;
    return n;
}

static std::string ReadAll(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string("<none>");
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

int main()
{
    std::printf("[1] registration (golden main.cpp:9717-10036)\n");
    CHECK(Registered() == 0);
    W906_BootRegisterTaskList();
    std::printf("  registered = %d\n", Registered());
    CHECK(Registered() == 265);   CHECK(fHome != NULL && QueueTaskList[198].Alias == "TestZTask" && QueueTaskList[198].iTask == &fHome->TestZTask);   //AI(W906-FLOW-2) 20260928: golden main.cpp:9936 un-gated (was 264)
    CHECK(QueueTaskList[1].Alias == "AutoSHT1Task" && QueueTaskList[1].iTask == &AutoSHT1Task);          // golden :9720
    CHECK(QueueTaskList[281].Alias == "ReceiveAutoTrayTask[5]");                                         // golden :10027
    CHECK(fHome != NULL && QueueTaskList[267].Alias == "HomeStep" && QueueTaskList[267].iTask == &fHome->iHomeStep);   // :10012
    CHECK(fContactForm != NULL && QueueTaskList[200].Alias == "CarlibrationTask" &&
          QueueTaskList[200].iTask == &fContactForm->CarlibrationTask);                                  // :9939 -> fContactForm
    CHECK(QueueTaskList[33].Alias == "BarcodeInShuttle1Task" && QueueTaskList[33].iTask == &iInitialBarcodeInShuttle1Task);   // :9755 -> global
    CHECK(QueueTaskList[112].Alias == "" && QueueTaskList[112].iTask == NULL);                           // :9842 SetStepMotorTask, gated
    CHECK(QueueTaskList[141].Alias == "" && QueueTaskList[141].iTask == NULL);                           // :9874 commented out in golden
    CHECK(fMain != NULL && fMain->StringGrid2->RowCount == qTaskCount);                                  // :10034
    CHECK(fMain->StringGrid2->Cells[0][0] == "Task" && fMain->StringGrid2->Cells[0][1] == "AutoSHT1Task"); // :10035 / :10052

    std::printf("[2] recorder + SaveTaskList\n");
    QueueTaskList[1].ClearData();
    AutoSHT1Task = 10;
    CHECK(QueueTaskList[1].CheckTaskChange() == true);
    // golden quirk, kept: ClearData leaves iCount=-1 (golden cpublic.cpp:909, port cpublic.cpp:43), so CheckTaskChange's
    // `iCount<=0 -> add` fires twice and the first value is recorded twice.
    CHECK(QueueTaskList[1].CheckTaskChange() == true);
    CHECK(QueueTaskList[1].CheckTaskChange() == false);                                                  // unchanged -> nothing
    AutoSHT1Task = 100;
    CHECK(QueueTaskList[1].CheckTaskChange() == true);
    char tmp[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "ht9045_tasklist_test";
    CreateDirectoryA(dir.c_str(), NULL);
    fMain->SaveTaskList(dir.c_str());
    const std::string csv = ReadAll(dir + "\\Task_ListWithTime.csv");
    const std::string::size_type at = csv.find("AutoSHT1Task,");
    CHECK(at == 0);                                                                                      // row 1 comes first
    const std::string row = csv.substr(at, csv.find('\n', at) - at);
    std::printf("  row: %.80s\n", row.c_str());
    // newest first: ", hh:mm:ss.mmm, 100, hh:mm:ss.mmm, 10,"
    const std::string::size_type p100 = row.find(", 100,");
    const std::string::size_type p10 = row.find(", 10,");
    CHECK(p100 != std::string::npos && p10 != std::string::npos && p100 < p10);
    CHECK(row.size() > 26 && row[12] == ',' && row[13] == ' ' && row[16] == ':' && row[19] == ':' && row[22] == '.');   // "AutoSHT1Task, hh:mm:ss.mmm"
    CHECK(csv.find("HomeStep,") != std::string::npos && csv.find("CarlibrationTask,") != std::string::npos);
    DeleteFileA((dir + "\\Task_ListWithTime.csv").c_str());
    DeleteFileA((dir + "\\Task_ListWithTime2.csv").c_str());
    RemoveDirectoryA(dir.c_str());

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
