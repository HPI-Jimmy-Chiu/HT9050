// =============================================================================
//  tests/test_automation.cpp -- Automation/automation.cpp (TfAutomation)
//  "network wiring + small helpers" translation wave (W906-Automation).
//
//  AI(W906-Automation) 20260716: new file.
//
//  No external test framework: a tiny PASS/FAIL harness (same style as
//  tests/test_auto9045.cpp / tests/test_serversocket.cpp) prints per-case
//  results and a final summary, returning non-zero on any failure.
//
//  ORACLE STRATEGY (per this project's testing convention -- prefer a real,
//  independently-derivable oracle over invented numbers):
//    * crc16(): the exact golden algorithm (init 65535, XOR, /2, XOR 40961 if
//      odd, 8x per byte) was independently re-implemented in Python and run
//      against several inputs to get ground-truth values (not hand-guessed):
//        crc16("A")   -> 28799
//        crc16("ABC") -> 34128
//        crc16("123") -> 31349
//        crc16("")    -> 65535
//    * SplitDataBySoh/GetCRC/AddBlock/IsStartWord/charcmp: pure string/byte
//      logic, hand-traced against the golden source line-by-line (see each
//      case's comment).
//    * Wire-format bytes (CommandProcess/SendClient/SendServer): hand-built
//      from golden's own HEAD/head construction formula (STX+CMD+SOH+
//      V_TOTAL+SOH+Data[0]+SOH+Data[1]+...+ETX), byte-for-byte.
//
//  SCOPE: exercises the in-scope translated surface (see automation.h's own
//  file-head scope note). AI(W906-AutoPB) 20260720: ProcessBuffer/
//  ProcessBuffer1/SendReportRequest now dispatch for REAL (see
//  DESIGN_automation_ProcessBuffer.md) -- Parts 13-25 below exercise the full
//  OLP command-dispatch ladder, the PP_DL_REQUEST file-receive sibling, and
//  the host EVENT/REPORT sender, including the golden bug/quirk oracles
//  (P1-P18) and the 3 PORT-ONLY UB guards (D1-D3).
//
//  SIDE EFFECTS (documented, same posture as test_auto9045.cpp's own
//  disclosed-side-effects section): WriteTCPDataToTextFile/SaveRecord write
//  real files under D:\<HandlerType>_Log\... on the machine running this
//  test (debug/comm log paths, never a production recipe file). SaveRecord's
//  own file write is only reached from inside ShowMSG once Memo/MemoChar/
//  MemoCode->Lines->Count exceeds 500 -- never true in this short run, so it
//  is never actually reached even where IniConfig.bN08_1SaveOLPLog is
//  temporarily toggled true to open ShowRecord's logging guard.
//  WriteTCPDataToTextFile IS exercised once (Part 8) and does write one line
//  to D:\HT9045_Log\TCP_Data\<today>.tcp -- flagged here, not hidden.
//
//  AI(W906-AutoPB) 20260720: additional test-hygiene rules for Parts 13-25
//  (design doc §0-8 / §6):
//    * Part 22 was NARROWED during construction (disclosed deviation, see the
//      Part 22 block comment): it never enters the PP_DL_REQUEST branch, so
//      no file path -- production or scratch -- is written; aDataPath is left
//      untouched. (An earlier draft of this rule promised an aDataPath
//      redirect; that described the pre-narrowing plan and is superseded.)
//    * INITIATE_REQUEST is only ever exercised with a GUARANTEED-INVALID
//      SYSTEMTIME payload (all-zero date/time digits) -- Win32 SetLocalTime()
//      rejects invalid field values and returns without touching the real
//      system clock; Part 23 asserts the wall-clock year is unchanged as a
//      belt-and-suspenders check.
//    * CUSTOMER_CODE / CosFunction.bOEEFunction / IniConfig.bC11UseMonitorView
//      / LastSet.OLPSetBinErr[] / bLockByServer / SoftStop / SystemStart /
//      fMain->palMainStatus->Caption are all saved and restored by whichever
//      Part temporarily changes them.
//    * Part 22 (ProcessBuffer1) deliberately does NOT drive a live
//      PP_DL_REQUEST frame through ProcessBuffer/ProcessBuffer1's real
//      DoDLRequest call -- see that Part's own comment: DoDLRequest (already
//      translated in auto9045.cpp, out of this wave's scope) performs
//      unguarded real filesystem mutations against the PRODUCTION
//      `DataPath`/`OffsetPath` globals (common.cpp:87-88, literally
//      `D:\HT9045\IniData\Data\`/`D:\HT9045\IniData\Offset\` -- inside this
//      very repo, NOT shielded by redirecting `aDataPath`), plus a
//      CopyFile("C:\Program Files\7-Zip\7z.exe", ...) and a process-wide
//      SetCurrentDirectory("D://") call. auto9045.cpp's own translator
//      already flagged DoDLRequest with "test harness does NOT call this
//      function" for the exact same reason. This is reported (not silently
//      routed around) per this wave's task brief Q4 ("dispatch 深處若炸：
//      回報，不自行修 auto9045") -- treating an unguarded real filesystem
//      mutation against this repo's own production paths as equivalent in
//      severity to a crash for test-hygiene purposes. Part 22 instead
//      exercises ProcessBuffer1's entry parsing + unconditional tail
//      TCPstr-clear via a non-PP_DL_REQUEST global TCPstr value, which never
//      reaches the DoDLRequest call; the PP_DL hex-decode-writes-all-NUL-
//      bytes quirk (P11) and the SendServer routing quirk (P13) are verified
//      by hand-tracing the golden-faithful translated body instead (see
//      ProcessBuffer1's own AI(W906-AutoPB) comments in automation.cpp).
// =============================================================================
#include "Automation/automation.h"
#include "Automation/auto9045.h"        // oracle accessors (GetSoftwareVersion/GetPitch/GetDropHeight/
                                         // GetActTemp/SetCategory/SetLotInfo/SetLotInfoGreatekOEE/...)
                                         // -- self-referential-oracle test pattern, design doc §6.

#include "MachineDefine.h"
#include "MachineType.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include "canary_support.h"
#include "FormsFacade.h"
#include "Public/cJSON.h"

#include <cstdio>
#include <cstring>
#include <vector>

// TU-local extern redeclarations: automation.cpp defines these as plain
// file-scope globals with external linkage (matching golden) but does not
// expose them via automation.h's public surface (see that header's own
// "File-scope globals" comment -- only HandlerType/aDataPath are exposed
// there, since nothing else in the translated tree needs the rest yet).
extern AnsiString STX;
extern AnsiString SOH;
extern AnsiString ETX;
extern AnsiString ReceiveString;
extern AnsiString TCPstr;      // AI(W906-AutoPB) 20260720: needed by Part 22 (ProcessBuffer1 reads this global)
extern bool bLockByServer;     // AI(W906-AutoPB) 20260720: needed by Part 18 (PAUSE/RESUME_REQUEST)

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static AnsiString VecToAnsi(const std::vector<char> &v)
{
    AnsiString s;
    for (size_t i = 0; i < v.size(); ++i)
        s += v[i];
    return s;
}

// ===========================================================================
//  PART 1 -- the 6 zero-dependency free functions
// ===========================================================================
static void Part1_FreeFunctions()
{
    printf("\n[1] Free functions (SplitDataBySoh/GetCRC/AddBlock/IsStartWord/crc16/charcmp)\n");

    // SplitDataBySoh: golden :212-223 -- extracts everything before the
    // first SOH (0x01) [or ETX (0x03) if no SOH present], consuming the
    // delimiter too.
    {
        AnsiString soh1; soh1.sprintf("%c", 0x01);
        AnsiString etx1; etx1.sprintf("%c", 0x03);
        SOH = soh1;   // SplitDataBySoh reads the file-scope SOH/ETX globals
        ETX = etx1;

        AnsiString buf = AnsiString("CMD") + soh1 + AnsiString("REST");
        AnsiString got = SplitDataBySoh(buf);
        CHECK(got == "CMD", "SplitDataBySoh extracts up to the first SOH");
        CHECK(buf == "REST", "SplitDataBySoh consumes the extracted part + delimiter");

        AnsiString buf2 = AnsiString("ONLYETX") + etx1;
        AnsiString got2 = SplitDataBySoh(buf2);
        CHECK(got2 == "ONLYETX", "SplitDataBySoh falls back to ETX when no SOH present (golden :216-218)");
    }

    // GetCRC: golden :225-231 -- takes the first 5 chars, but (preserved
    // quirk) only deletes 1 char from Buffer, not 5.
    {
        AnsiString buf = "ABCDEFGH";
        AnsiString crc = GetCRC(buf);
        CHECK(crc == "ABCDE", "GetCRC extracts exactly the first 5 chars");
        CHECK(buf == "BCDEFGH", "GetCRC's preserved quirk: only 1 char removed from Buffer, not 5");
    }

    // AddBlock: golden :233-238 -- wraps in [ ].
    CHECK(AddBlock("0001") == "[0001]", "AddBlock wraps its argument in [ ]");
    CHECK(AddBlock("") == "[]", "AddBlock on an empty string still wraps (2-char result)");

    // IsStartWord: golden :240-248 -- true + consumes 1 char iff w is found
    // at position 1.
    {
        AnsiString stxG; stxG.sprintf("%c", 0x02);
        AnsiString buf = stxG + "REST";
        bool r = IsStartWord(buf, stxG);
        CHECK(r == true, "IsStartWord returns true when w is at position 1");
        CHECK(buf == "REST", "IsStartWord consumes exactly 1 char on match");

        AnsiString buf2 = "XREST";
        bool r2 = IsStartWord(buf2, stxG);
        CHECK(r2 == false, "IsStartWord returns false when w is not at position 1");
        CHECK(buf2 == "XREST", "IsStartWord leaves Buffer untouched on no-match");
    }

    // crc16: golden :250-275 -- independently re-derived oracle (see file
    // header). n excludes any trailing NUL (raw char* + explicit length).
    {
        char a[] = "A";
        CHECK(crc16(a, 1) == "28799", "crc16(\"A\") == 28799 (independently re-derived oracle)");
        char abc[] = "ABC";
        CHECK(crc16(abc, 3) == "34128", "crc16(\"ABC\") == 34128");
        char n123[] = "123";
        CHECK(crc16(n123, 3) == "31349", "crc16(\"123\") == 31349");
        char empty[] = "";
        CHECK(crc16(empty, 0) == "65535", "crc16(\"\") == 65535 (identity -- no bytes ever flip the init value)");
    }

    // charcmp: golden :2279-2288 (ChungHung 20120413 add).
    {
        char b1[4] = {1, 2, 3, 4};
        char b2[4] = {1, 2, 3, 4};
        char b3[4] = {1, 2, 3, 9};
        CHECK(charcmp(b1, b2, 4) == true, "charcmp: identical buffers -> true");
        CHECK(charcmp(b1, b3, 4) == false, "charcmp: differing buffers -> false");
    }
}

// ===========================================================================
//  PART 2 -- event API (GetEventNum/SetEventResult/GetEventResult/
//  CompareEvent/ClearEvent/GetEventStrResult)
// ===========================================================================
static void Part2_EventApi(TfAutomation *fa)
{
    printf("\n[2] Event API (GetEventNum/SetEventResult/GetEventResult/CompareEvent/ClearEvent/GetEventStrResult)\n");

    int num = -99;
    bool got = fa->GetEventNum(num, "PRODUCTION_REPLY", "0003");
    CHECK(got == true, "GetEventNum finds a free slot (fresh instance -> always succeeds)");
    CHECK(num >= 0, "GetEventNum returns a valid slot index");
    CHECK(fa->GetEventResult(num) == false, "freshly-registered event starts un-fired (def=false)");

    // CompareEvent: golden :189-203 -- matches when str contains BOTH the
    // CommandString AND the NumString (or NumString=="").
    fa->CompareEvent("...PRODUCTION_REPLY...0003...");
    CHECK(fa->GetEventResult(num) == true, "CompareEvent fires the event when both substrings are present");

    fa->ClearEvent(num);
    CHECK(fa->GetEventResult(num) == false, "ClearEvent resets def=false");

    // GetEventStrResult: golden :2396-2419 -- 3 special-cased (cmdstr,numstr)
    // pairs map to 1/2/3, everything else (including a cleared slot) -> 4.
    CHECK(fa->GetEventStrResult(num) == 4, "GetEventStrResult on a cleared slot -> 4 (no match)");

    int num2 = -1;
    fa->SetEventResult(0, false, "PRODUCTION_REQUEST", "0007");
    num2 = 0;
    CHECK(fa->GetEventStrResult(num2) == 2, "GetEventStrResult(\"PRODUCTION_REQUEST\",\"0007\") -> 2 (golden :2408-2411)");

    // out-of-range num is rejected by every accessor (golden's own bounds
    // guard `num>=0 && num<iMaxEvent`).
    CHECK(fa->GetEventResult(-1) == false, "GetEventResult(-1) -> false (out-of-range guard)");
    CHECK(fa->GetEventResult(999) == false, "GetEventResult(999) -> false (out-of-range guard)");
    CHECK(fa->SetEventResult(999, true, "X", "Y") == false, "SetEventResult(999,...) -> false (out-of-range guard)");
}

// ===========================================================================
//  PART 3 -- GetTimeInfo() format sanity (golden :277-286: "%04d%02d%02d%02d%02d%02d")
// ===========================================================================
static void Part3_GetTimeInfo(TfAutomation *fa)
{
    printf("\n[3] GetTimeInfo() format\n");

    AnsiString t = fa->GetTimeInfo();
    CHECK(t.Length() == 14, "GetTimeInfo() returns exactly 14 chars (YYYYMMDDHHMMSS)");
    bool allDigits = true;
    for (int i = 1; i <= t.Length(); ++i)
        if (t[i] < '0' || t[i] > '9') allDigits = false;
    CHECK(allDigits, "GetTimeInfo() is all-digit");
    AnsiString yearPart = t.SubString(1, 4);
    CHECK(yearPart.ToInt() >= 2024 && yearPart.ToInt() <= 2100, "GetTimeInfo()'s year prefix is plausible");
}

// ===========================================================================
//  PART 4 -- ctor customer-code branching (golden :57-147)
// ===========================================================================
static void Part4_CtorBranching()
{
    printf("\n[4] Ctor customer-code branching\n");

    int savedCC = CUSTOMER_CODE;

    CUSTOMER_CODE = CC_ASE_CL;
    TfAutomation *fa1 = new TfAutomation(NULL);
    CHECK(fa1->OLPServer->Port == 5001, "CC_ASE_CL: OLPServer->Port stays at the .dfm default (5001, never assigned this branch)");
    CHECK(fa1->OLPServer->IsActiveNow() == true, "CC_ASE_CL: OLPServer->Open() called");
    CHECK(fa1->gbAutomation->Visible == true, "CC_ASE_CL: gbAutomation->Visible == true");

    CUSTOMER_CODE = CC_PTI;
    TfAutomation *fa2 = new TfAutomation(NULL);
    CHECK(fa2->OLPServer->Port == 6671, "CC_PTI: OLPServer->Port forced to 6671 (Sam 20210812)");
    CHECK(fa2->OLPServer->IsActiveNow() == true, "CC_PTI: OLPServer->Open() called");
    CHECK(fa2->gbAutomation->Visible == true, "CC_PTI: gbAutomation->Visible == true");

    CUSTOMER_CODE = 0;   // no matching branch -> the else path
    TfAutomation *fa3 = new TfAutomation(NULL);
    CHECK(fa3->OLPServer->IsActiveNow() == false, "default (unmatched CUSTOMER_CODE): OLPServer->Close() called");
    CHECK(fa3->gbAutomation->Visible == false, "default: gbAutomation->Visible == false");
    CHECK(fa3->OLPClient->Address == AnsiString("10.33.10.70"), "default: OLPClient->Address stays at the .dfm default (never assigned this branch)");
    CHECK(fa3->OLPClient->Port == 5002, "default: OLPClient->Port stays at the .dfm default (5002)");
    CHECK(fa3->cbbOLPCommand->Text == "0001", "cbbOLPCommand->Text starts at the .dfm default");
    CHECK(fa3->edinputIP->Text == "192.168.120.143", "edinputIP->Text starts at the .dfm default");
    CHECK(fa3->edinputport->Text == "6670", "edinputport->Text starts at the .dfm default");
    CHECK(fa3->TestMode == 0, "TestMode starts at 0");
    CHECK(fa3->bReceive == false && fa3->bReceive2 == false, "bReceive/bReceive2 start false");
    CHECK(fa3->CommandBuffer->Count == 0, "CommandBuffer starts empty");
    CHECK(fa3->CommandClientReplyBuffer->Count == 0, "CommandClientReplyBuffer starts empty");

    CUSTOMER_CODE = savedCC;
}

// ===========================================================================
//  PART 5 -- DoCommandBuffer + CommandBuffer bookkeeping (golden :937-947)
// ===========================================================================
static void Part5_DoCommandBuffer(TfAutomation *fa)
{
    printf("\n[5] DoCommandBuffer + CommandBuffer bookkeeping\n");

    int before = fa->CommandBuffer->Count;
    fa->DoCommandBuffer("ALARM_REQUEST", "SUB", "Some Alarm Message", 7, "JAM0109");
    CHECK(fa->CommandBuffer->Count == before + 1, "DoCommandBuffer appends exactly one entry");

    MyProcessBuffer *p = (MyProcessBuffer *)fa->CommandBuffer->Items[fa->CommandBuffer->Count - 1];
    CHECK(p->Command == "ALARM_REQUEST", "the pushed entry's Command round-trips");
    CHECK(p->SubCommand == "SUB", "the pushed entry's SubCommand round-trips");
    CHECK(p->AlarmMessage == "Some Alarm Message", "the pushed entry's AlarmMessage round-trips");
    CHECK(p->ErrType == 7, "the pushed entry's ErrType round-trips");
    CHECK(p->AlarmID == "JAM0109", "the pushed entry's AlarmID round-trips");
    CHECK(p->TimeString.Length() == 14, "the pushed entry's TimeString is stamped via GetTimeInfo()");
}

// ===========================================================================
//  PART 6 -- CommandProcess -> SendClient wire format, NO connection yet
//  (falls through to CommandClientReplyBuffer, golden :855-862)
// ===========================================================================
static void Part6_CommandProcessNoConnection(TfAutomation *fa)
{
    printf("\n[6] CommandProcess -> SendClient wire format (no connection)\n");

    AnsiString Data[4];
    Data[0] = "AAA";
    Data[1] = "BBB";
    int before = fa->CommandClientReplyBuffer->Count;
    fa->CommandProcess("TEST_CMD", 2, Data, true, 0);

    CHECK(fa->CommandClientReplyBuffer->Count == before + 1, "SendClient's no-connection path buffers the reply for later retry");

    AnsiString expected = STX + AnsiString("TEST_CMD") + SOH + AnsiString("2") + SOH + AnsiString("AAA") + SOH + AnsiString("BBB") + ETX;
    AnsiString actual = fa->CommandClientReplyBuffer->Strings[fa->CommandClientReplyBuffer->Count - 1];
    CHECK(actual == expected, "buffered reply matches golden's HEAD wire-format construction exactly");
}

// ===========================================================================
//  PART 7 -- CommandProcess -> SendClient wire format, WITH a Sim-accepted
//  OLPServer connection (bytes actually go out via SendBuf)
// ===========================================================================
static void Part7_CommandProcessWithConnection(TfAutomation *fa)
{
    printf("\n[7] CommandProcess -> SendClient wire format (Sim connection accepted)\n");

    fa->OLPServer->Open();   // realistic precondition: a peer cannot connect to a closed listener
    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 12345);
    CHECK(conn != 0, "OLPServer->SimAcceptConnection returns a connection");
    CHECK(fa->OnLine->Enabled == true, "OLPServerClientConnect (wired) sets OnLine->Enabled=true on a single connection");

    fa->iSocketHandle = conn->SocketHandle;   // as OLPServerClientRead would have set it from a real inbound read

    AnsiString Data[2];
    Data[0] = "0";
    fa->CommandProcess("REPORT_GRANT", 1, Data, true, conn->SocketHandle);

    AnsiString expected = STX + AnsiString("REPORT_GRANT") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    AnsiString onWire = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWire == expected, "bytes actually sent via Connections[0]->SendBuf match the golden wire format");
}

// ===========================================================================
//  PART 8 -- WriteTCPDataToTextFile (real file write -- see file-head
//  SIDE EFFECTS note) + SendServer via OLPClient
// ===========================================================================
static void Part8_WriteAndSendServer(TfAutomation *fa)
{
    printf("\n[8] WriteTCPDataToTextFile (real log write) + SendServer via OLPClient\n");

    fa->DoCommandBuffer("MESSAGE_REQUEST", "", "hello from test_automation", 0, "");
    int before = fa->CommandBuffer->Count;
    CHECK(before >= 1, "a pending entry exists before WriteTCPDataToTextFile drains it");
    fa->WriteTCPDataToTextFile();
    CHECK(fa->CommandBuffer->Count == 0, "WriteTCPDataToTextFile drains CommandBuffer via ->Clear() (golden :932)");

    fa->OnLine2->Enabled = true;   // as OLPClientConnect would have set it
    AnsiString S = STX + AnsiString("PING") + ETX;
    AnsiString S2 = "[STX][PING][ETX]";
    fa->SendServer(S, S2);
    AnsiString onWire = VecToAnsi(fa->OLPClient->Socket->SimTxBuffer());
    CHECK(onWire == S, "SendServer sends the exact bytes via OLPClient->Socket->SendBuf when OnLine2->Enabled");
}

// ===========================================================================
//  PART 9 -- ShowMSG / ShowCharHex / ShowRecord (real TStringList-backed
//  Memo/MemoChar/MemoCode, chkViewComm->Checked toggled per golden's own guard)
// ===========================================================================
static void Part9_MemoLogging(TfAutomation *fa)
{
    printf("\n[9] ShowMSG / ShowCharHex / ShowRecord\n");

    // ShowMSG itself is private (golden automation.h private: section) --
    // exercised indirectly via the public ShowRecord(), which calls it after
    // building its "[Receive]/[Send]" prefix. golden ShowMSG: Lines->Add(S)
    // then an UNCONDITIONAL trailing Lines->Add("") -- 2 new lines per call
    // (golden :2435-2449).
    IniConfig.bN08_1SaveOLPLog = true;   // open ShowRecord's guard for this one call
    fa->chkViewComm->Checked = false;
    int before = fa->Memo->Lines->Count;
    fa->ShowRecord(true, "hello", 42);
    CHECK(fa->Memo->Lines->Count == before + 2, "ShowRecord->ShowMSG appends the message line + a trailing blank line");
    AnsiString msgLine = fa->Memo->Lines->GetString(fa->Memo->Lines->Count - 2);
    CHECK(msgLine.Pos("[Receive]") != 0 && msgLine.Pos("hello") != 0, "the [Receive]-prefixed message line round-trips (golden :773-786)");
    IniConfig.bN08_1SaveOLPLog = false;

    fa->chkViewComm->Checked = false;
    int beforeChar = fa->MemoChar->Lines->Count;
    fa->ShowCharHex("X");
    CHECK(fa->MemoChar->Lines->Count == beforeChar, "ShowCharHex is a no-op while chkViewComm->Checked==false (golden :723-724)");

    fa->chkViewComm->Checked = true;
    fa->ShowCharHex("A");
    CHECK(fa->MemoChar->Lines->Count == beforeChar + 2, "ShowCharHex appends 2 lines to MemoChar once enabled (char row + blank)");
    CHECK(fa->MemoCode->Lines->Count >= 2, "ShowCharHex also appends to MemoCode (hex row + blank)");
    // 'A' == 0x41 -> hex row should contain "41 "
    AnsiString hexRow = fa->MemoCode->Lines->GetString(fa->MemoCode->Lines->Count - 2);
    CHECK(hexRow.Pos("41") != 0, "ShowCharHex's hex dump of 'A' contains \"41\"");

    IniConfig.bN08_1SaveOLPLog = false;   // keep ShowRecord's own guard closed -> no disk write
    fa->chkViewComm->Checked = false;
    int beforeRec = fa->Memo->Lines->Count;
    fa->ShowRecord(true, "unused-while-guard-closed", 0);
    CHECK(fa->Memo->Lines->Count == beforeRec, "ShowRecord is a no-op while both its guards are closed (golden :775-776)");
}

// ===========================================================================
//  PART 10 -- GetHTMLJSONDatas / GetMachineStatus / GetWorkOrder / GetMainTemp
//  (via the FormsFacade fMain facade)
// ===========================================================================
static void Part10_HtmlJsonAndFacade(TfAutomation *fa)
{
    printf("\n[10] GetHTMLJSONDatas / GetMachineStatus / GetWorkOrder / GetMainTemp\n");

    fMain->palMainStatus->Caption = "RUNNING";
    fMain->cbSetupFileName->Text = "RECIPE_A";
    fMain->edWorkTemperBase->Text = "25.0";

    CHECK(fa->GetMachineStatus() == "RUNNING", "GetMachineStatus() passes through fMain->palMainStatus->Caption");
    CHECK(fa->GetWorkOrder() == "RECIPE_A", "GetWorkOrder() passes through fMain->cbSetupFileName->Text");
    CHECK(fa->GetMainTemp() == "25.0", "GetMainTemp() passes through fMain->edWorkTemperBase->Text");

    double temps[10];
    fa->GetTemperatures(temps);
    bool identity = true;
    for (int i = 0; i < 10; ++i)
        if (temps[i] != (double)i) identity = false;
    CHECK(identity, "GetTemperatures() fills temps[i]=i (golden :2489-2495 verbatim placeholder body)");

    AnsiString json = fa->GetHTMLJSONDatas();
    cJSON *root = cJSON_Parse(json.c_str());
    CHECK(root != 0, "GetHTMLJSONDatas() produces parseable JSON");
    if (root != 0)
    {
        cJSON *ms = cJSON_GetObjectItem(root, "machineStatus");
        CHECK(ms != 0 && strcmp(ms->valuestring, "RUNNING") == 0, "JSON machineStatus field round-trips");
        cJSON *wo = cJSON_GetObjectItem(root, "workOrder");
        CHECK(wo != 0 && strcmp(wo->valuestring, "RECIPE_A") == 0, "JSON workOrder field round-trips");
        cJSON *mt = cJSON_GetObjectItem(root, "MainTemp");
        CHECK(mt != 0 && strcmp(mt->valuestring, "25.0") == 0, "JSON MainTemp field round-trips");
        cJSON *temps_j = cJSON_GetObjectItem(root, "temperatures");
        CHECK(temps_j != 0, "JSON temperatures object present");
        if (temps_j != 0)
        {
            cJSON *a1 = cJSON_GetObjectItem(temps_j, "A1");
            CHECK(a1 != 0 && strcmp(a1->valuestring, "0.0") == 0, "JSON temperatures.A1 == \"0.0\" (temps[0]=0, sensor index i+1)");
            cJSON *a10 = cJSON_GetObjectItem(temps_j, "A10");
            CHECK(a10 != 0 && strcmp(a10->valuestring, "9.0") == 0, "JSON temperatures.A10 == \"9.0\" (temps[9]=9)");
        }
        cJSON_Delete(root);
    }
}

// ===========================================================================
//  PART 11 -- OLPServerClientRead via SimPushReceive (frame parsing +
//  bReceive/ReceiveString bookkeeping). AI(W906-AutoPB) 20260720:
//  ProcessBuffer now dispatches for real; this frame ("ECHO") deliberately
//  matches no branch (Command.Pos("INQUIRE")==0 and no "ECHO"-named
//  else-if exists) -- assertions below are UNCHANGED, still verifying only
//  the bReceive/ReceiveString bookkeeping around the call.
// ===========================================================================
static void Part11_OLPServerClientRead(TfAutomation *fa)
{
    printf("\n[11] OLPServerClientRead via SimPushReceive\n");

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 55000);
    ReceiveString = "";   // reset the shared parse buffer between test parts
    fa->bReceive = false;
    fa->bReceiveHasData = false;

    AnsiString frame = STX + AnsiString("ECHO") + SOH + AnsiString("0") + ETX;
    conn->SimPushReceive(frame.c_str(), frame.Length());

    CHECK(fa->bReceive == false, "bReceive is reset to false again at the end of OLPServerClientRead (ProcessBuffer runs, then resets it)");
    CHECK(ReceiveString.Length() == 0, "the fully-framed message is consumed out of ReceiveString");
    CHECK(fa->iSocketHandle == conn->SocketHandle, "iSocketHandle is updated from the event's own Socket param (golden :370)");
}

// ===========================================================================
//  PART 12 -- tmrOLPTimer smoke test (first-call bfirst branch only; static
//  locals inside tmrOLPTimer are process-wide, so this is called exactly
//  once across the whole binary -- matches golden's own singleton-timer
//  design, see automation.h's own EVENT WIRING note)
// ===========================================================================
static void Part12_TmrOLPTimerSmoke(TfAutomation *fa)
{
    printf("\n[12] tmrOLPTimer smoke test\n");

    bool savedOLP = CosFunction.bOLPFunction;
    bool savedInit = InitialOK;
    CosFunction.bOLPFunction = true;
    InitialOK = true;
    IniConfig.sN08OlpIP = "10.1.1.1";
    IniConfig.sN08OlpPort = "";   // exercise the "" -> "6670" fallback (golden :456-458)

    fa->tmrOLPTimer(NULL);

    CHECK(fa->edinputIP->Text == "10.1.1.1", "tmrOLPTimer's first-call branch copies IniConfig.sN08OlpIP into edinputIP->Text");
    CHECK(fa->edinputport->Text == "6670", "tmrOLPTimer's first-call branch falls back to \"6670\" when sN08OlpPort is empty");

    CosFunction.bOLPFunction = savedOLP;
    InitialOK = savedInit;
}

// ===========================================================================
//  AI(W906-AutoPB) 20260720: Parts 13-25 -- ProcessBuffer/ProcessBuffer1/
//  SendReportRequest real-dispatch coverage (design doc
//  DESIGN_automation_ProcessBuffer.md §6). Frame-building helper mirrors the
//  golden inbound wire format: STX + CMD + SOH + V_TOTAL + (SOH + field)*n + ETX.
// ===========================================================================
static AnsiString MakeInFrame(const char *cmd, int n, AnsiString f0 = "", AnsiString f1 = "", AnsiString f2 = "")
{
    AnsiString fields[3] = { f0, f1, f2 };
    AnsiString s = STX + AnsiString(cmd) + SOH + AnsiString(n);
    for (int i = 0; i < n; ++i)
        s += SOH + fields[i];
    s += ETX;
    return s;
}

// ===========================================================================
//  PART 13 -- ProcessBuffer entry-guard quirks (BCB6 NULL/0 fidelity, D4 oracle)
// ===========================================================================
static void Part13_ProcessBufferEntryGuard(TfAutomation *fa)
{
    printf("\n[13] ProcessBuffer entry-guard quirks (BCB6 NULL/0 fidelity, D4 oracle)\n");

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 13000);
    fa->iSocketHandle = conn->SocketHandle;
    size_t before;

    // (a) Buffer == the STRING "0" (AnsiString(0)'s int->string conversion) --
    // BCB6-fidelity entry guard (design definition B / D4) fires and
    // ProcessBuffer returns immediately, before even the STX check -- zero
    // reply bytes of any kind.
    before = conn->SimTxBuffer().size();
    fa->ProcessBuffer(AnsiString("0"), conn->SocketHandle);
    CHECK(conn->SimTxBuffer().size() == before, "ProcessBuffer(\"0\",...) hits the BCB6 NULL/0 entry guard -- zero bytes sent (D4 oracle)");

    // (b) Buffer == "" does NOT equal AnsiString(0) ("0" != "") -- the entry
    // guard does NOT fire; IsStartWord(Buffer,STX) fails instead (no STX in
    // an empty string) and ProcessBuffer returns via its SECOND early return.
    // Different code path, same observable result: no reply.
    before = conn->SimTxBuffer().size();
    fa->ProcessBuffer(AnsiString(""), conn->SocketHandle);
    CHECK(conn->SimTxBuffer().size() == before, "ProcessBuffer(\"\",...) falls through IsStartWord's no-STX path -- zero bytes sent");

    // (c) Non-empty noise with no leading STX -- same IsStartWord-fails path.
    before = conn->SimTxBuffer().size();
    fa->ProcessBuffer(AnsiString("RANDOM_NOISE"), conn->SocketHandle);
    CHECK(conn->SimTxBuffer().size() == before, "ProcessBuffer(noise-without-STX,...) -- zero bytes sent, no crash");
}

// ===========================================================================
//  PART 14 -- generic INQUIRE happy path + CMD-suffix transform oracle
// ===========================================================================
static void Part14_GenericInquireHappyPath(TfAutomation *fa)
{
    printf("\n[14] generic INQUIRE happy path + CMD variant oracle\n");

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 14000);
    fa->iSocketHandle = conn->SocketHandle;

    AnsiString versionFrame = MakeInFrame("VERSION_INQUIRE", 0);
    fa->ProcessBuffer(versionFrame, conn->SocketHandle);
    AnsiString expected1 = STX + AnsiString("VERSION_GRANT") + SOH + AnsiString("2") + SOH + GetSoftwareVersion() + SOH + AnsiString("0") + ETX;
    AnsiString onWire1 = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWire1 == expected1, "VERSION_INQUIRE -> VERSION_GRANT wire format matches golden's HEAD construction exactly");

    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 14001);
    fa->iSocketHandle = conn2->SocketHandle;
    AnsiString setupFrame = MakeInFrame("SETUP_FILE_NAME_INQUIRE", 0);
    fa->ProcessBuffer(setupFrame, conn2->SocketHandle);
    AnsiString expected2 = STX + AnsiString("SETUP_FILE_NAME_GRANT") + SOH + AnsiString("2") + SOH + GetSetUpFileName() + SOH + AnsiString("0") + ETX;
    AnsiString onWire2 = VecToAnsi(conn2->SimTxBuffer());
    CHECK(onWire2 == expected2, "SETUP_FILE_NAME_INQUIRE -> SETUP_FILE_NAME_GRANT (CMD.Delete(Length-6,7)+\"GRANT\" transform verified)");
}

// ===========================================================================
//  PART 15 -- INQUIRE quirk trio (P4 GetPitch overwrite / P2 Data[0]
//  double-write leak / P3+D2 trailing-space-unreachable-branch)
// ===========================================================================
static void Part15_InquireQuirkTrio(TfAutomation *fa)
{
    printf("\n[15] INQUIRE quirk trio (GetPitch overwrite / Data[0] leak / unreachable ArmTotalForce)\n");

    // (a) GetPitch_INQUIRE: golden fills Data[0]=GetPitch(0)/Data[1]=GetPitch(1)
    // then iInquire=2 -> the trailing Data[iInquire-1]="0" append (Data[1]="0")
    // OVERWRITES GetPitch(1)'s result (P4 oracle).
    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 15000);
    fa->iSocketHandle = conn->SocketHandle;
    AnsiString pitchFrame = MakeInFrame("GetPitch_INQUIRE", 0);
    fa->ProcessBuffer(pitchFrame, conn->SocketHandle);
    AnsiString expectedPitch = STX + AnsiString("GetPitch_GRANT") + SOH + AnsiString("2") + SOH + GetPitch(0) + SOH + AnsiString("0") + ETX;
    AnsiString onWirePitch = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWirePitch == expectedPitch, "GetPitch_INQUIRE: 2nd reply field is \"0\", NOT GetPitch(1) -- overwritten by the trailing append (P4 oracle)");

    // (b) SetDropHeight_INQUIRE: golden writes Data[0]=GetDropHeight(0) then
    // Data[0]=GetDropHeight(1) (a literal typo -- both hit index 0, never
    // touching Data[1]) -- so an inbound Data[1] sentinel is never
    // overwritten and gets ECHOED BACK in the reply's 2nd field verbatim (a
    // genuine inbound-data-leak bug, P2 oracle). Send 2 inbound fields with a
    // sentinel in the 2nd to demonstrate the leak.
    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 15001);
    fa->iSocketHandle = conn2->SocketHandle;
    AnsiString dropFrame = MakeInFrame("SetDropHeight_INQUIRE", 2, "IGNORED_FIELD0", "LEAK_SENTINEL");
    fa->ProcessBuffer(dropFrame, conn2->SocketHandle);
    AnsiString expectedDrop = STX + AnsiString("SetDropHeight_GRANT") + SOH + AnsiString("3") + SOH + GetDropHeight(1) + SOH + AnsiString("LEAK_SENTINEL") + SOH + AnsiString("0") + ETX;
    AnsiString onWireDrop = VecToAnsi(conn2->SimTxBuffer());
    CHECK(onWireDrop == expectedDrop, "SetDropHeight_INQUIRE: inbound 2nd field \"LEAK_SENTINEL\" leaks straight into the reply's 2nd field (P2 oracle)");

    // (c) ArmTotalForce_INQUIRE (WITHOUT the golden branch's literal trailing
    // space) matches no branch -- falls to the unknown-INQUIRE path with
    // iInquire staying 0. The CMD.Delete(Length-6,7)+"GRANT" transform still
    // runs unconditionally on the RAW Command though, so CMD is still
    // correctly "ArmTotalForce_GRANT" -- and the D2 guard (iInquire>=1) skips
    // the Data[iInquire-1]="0" append, so CommandProcess is called with
    // iInquire==0 -> V_TOTAL="0", zero data fields (P3+D2 oracle).
    TCustomWinSocket *conn3 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 15002);
    fa->iSocketHandle = conn3->SocketHandle;
    AnsiString armFrame = MakeInFrame("ArmTotalForce_INQUIRE", 0);   // no trailing space -- matches golden's UNREACHABLE branch's condition
    fa->ProcessBuffer(armFrame, conn3->SocketHandle);
    AnsiString expectedArm = STX + AnsiString("ArmTotalForce_GRANT") + SOH + AnsiString("0") + ETX;
    AnsiString onWireArm = VecToAnsi(conn3->SimTxBuffer());
    CHECK(onWireArm == expectedArm, "ArmTotalForce_INQUIRE (no trailing space) -> empty GRANT, no crash (P3+D2 oracle)");
}

// ===========================================================================
//  PART 16 -- empty INQUIRE branch (D2 guard regression) + ACT_TEMP_INQUIRE
// ===========================================================================
static void Part16_EmptyInquireAndActTemp(TfAutomation *fa)
{
    printf("\n[16] empty INQUIRE branch (D2 guard regression) + ACT_TEMP_INQUIRE\n");

    // AlarmMode_INQUIRE: one of the 8 golden empty *_INQUIRE branches (no
    // body at all) -- iInquire stays 0, hits the SAME Data[-1] UB territory
    // as the unreachable-branch case above; the D2 guard must make this a
    // clean empty GRANT, not a crash.
    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 16000);
    fa->iSocketHandle = conn->SocketHandle;
    AnsiString alarmFrame = MakeInFrame("AlarmMode_INQUIRE", 0);
    fa->ProcessBuffer(alarmFrame, conn->SocketHandle);
    AnsiString expectedAlarm = STX + AnsiString("AlarmMode_GRANT") + SOH + AnsiString("0") + ETX;
    AnsiString onWireAlarm = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWireAlarm == expectedAlarm, "AlarmMode_INQUIRE (one of the 8 empty golden branches) -> empty GRANT, no crash (D2 guard regression)");

    // ACT_TEMP_INQUIRE: fills Data[0..9]=GetActTemp(i) then Data[10]="0"
    // explicitly, iInquire=11 -- the trailing append (Data[10]="0" again) is
    // redundant here (already "0"), not an additional leak.
    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 16001);
    fa->iSocketHandle = conn2->SocketHandle;
    AnsiString actTempFrame = MakeInFrame("ACT_TEMP_INQUIRE", 0);
    fa->ProcessBuffer(actTempFrame, conn2->SocketHandle);
    AnsiString expectedActTemp = STX + AnsiString("ACT_TEMP_GRANT") + SOH + AnsiString("11");
    for (int i = 0; i < 10; ++i)
        expectedActTemp += SOH + GetActTemp(i);
    expectedActTemp += SOH + AnsiString("0") + ETX;
    AnsiString onWireActTemp = VecToAnsi(conn2->SimTxBuffer());
    CHECK(onWireActTemp == expectedActTemp, "ACT_TEMP_INQUIRE -> 11 fields (10 GetActTemp() readings + trailing \"0\")");
}

// ===========================================================================
//  PART 17 -- REPORT_INQUIRE -> dual-frame reply (REPORT_GRANT then
//  SendReportRequest's own REPORT_REQUEST push)
// ===========================================================================
static void Part17_ReportInquireDualFrame(TfAutomation *fa)
{
    printf("\n[17] REPORT_INQUIRE -> dual-frame reply\n");

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 17000);
    fa->iSocketHandle = conn->SocketHandle;

    AnsiString frame = MakeInFrame("REPORT_INQUIRE", 3, "D0", "D1", "0001");
    fa->ProcessBuffer(frame, conn->SocketHandle);

    AnsiString expectedGrant = STX + AnsiString("REPORT_GRANT") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    AnsiString onWire = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWire.SubString(1, expectedGrant.Length()) == expectedGrant,
          "REPORT_INQUIRE's own reply (frame 1) is REPORT_GRANT[1][0] (Data[0]=0 assignment, P15 oracle)");

    AnsiString rest = onWire.SubString(expectedGrant.Length() + 1, onWire.Length());
    CHECK(rest.Pos(AnsiString("REPORT_REQUEST")) != 0, "frame 2 (from SendReportRequest(\"0001\")) is a REPORT_REQUEST");
    CHECK(rest.Pos(SOH + AnsiString("0001") + SOH) != 0, "frame 2 echoes back ReportID \"0001\" (from inbound Data[2], P15 oracle)");
    CHECK(rest.Pos(AnsiString("RUN")) != 0, "frame 2's status field is \"RUN\" (GetMainStatus() offline default)");
    CHECK(rest[rest.Length()] == ETX[1], "frame 2 is properly ETX-terminated");
}

// ===========================================================================
//  PART 18 -- control REQUEST family (PAUSE / RESUME / START)
// ===========================================================================
static void Part18_ControlRequestFamily(TfAutomation *fa)
{
    printf("\n[18] control REQUEST family (PAUSE/RESUME/START)\n");

    bool savedLock = bLockByServer;
    bool savedSoftStop = SoftStop;
    bool savedSystemStart = SystemStart;
    AnsiString savedCaption = fMain->palMainStatus->Caption;

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 18000);
    fa->iSocketHandle = conn->SocketHandle;

    // (a) PAUSE_REQUEST
    bLockByServer = false;
    SoftStop = false;
    AnsiString pauseFrame = MakeInFrame("PAUSE_REQUEST", 0);
    fa->ProcessBuffer(pauseFrame, conn->SocketHandle);
    CHECK(bLockByServer == true, "PAUSE_REQUEST sets bLockByServer=true");
    CHECK(SoftStop == true, "PAUSE_REQUEST sets SoftStop=true");
    AnsiString expectedPause = STX + AnsiString("PAUSE_REPLY") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expectedPause, "PAUSE_REQUEST -> PAUSE_REPLY[1][0]");

    // (b) RESUME_REQUEST: clears bLockByServer but -- preserved golden quirk
    // (P8) -- does NOT clear SoftStop.
    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 18001);
    fa->iSocketHandle = conn2->SocketHandle;
    AnsiString resumeFrame = MakeInFrame("RESUME_REQUEST", 0);
    fa->ProcessBuffer(resumeFrame, conn2->SocketHandle);
    CHECK(bLockByServer == false, "RESUME_REQUEST clears bLockByServer");
    CHECK(SoftStop == true, "RESUME_REQUEST does NOT clear SoftStop (P8 quirk oracle)");
    AnsiString expectedResume = STX + AnsiString("RESUME_REPLY") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn2->SimTxBuffer()) == expectedResume, "RESUME_REQUEST -> RESUME_REPLY[1][0]");

    // (c) START_REQUEST: the reply is ALWAYS "START_REPLY" regardless of
    // whether the internal success/fail branch was taken (golden :1578-1579)
    // -- exercise both the idle-and-startable case and a not-startable case;
    // both produce the identical wire reply.
    TCustomWinSocket *conn3 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 18002);
    fa->iSocketHandle = conn3->SocketHandle;
    fMain->palMainStatus->Caption = "HALT";
    SystemStart = false;
    AnsiString startFrame = MakeInFrame("START_REQUEST", 0);
    fa->ProcessBuffer(startFrame, conn3->SocketHandle);
    AnsiString expectedStart = STX + AnsiString("START_REPLY") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn3->SimTxBuffer()) == expectedStart, "START_REQUEST (HALT+idle, success path taken internally) -> START_REPLY[1][0]");

    TCustomWinSocket *conn4 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 18003);
    fa->iSocketHandle = conn4->SocketHandle;
    fMain->palMainStatus->Caption = "RUNNING";
    AnsiString startFrame2 = MakeInFrame("START_REQUEST", 0);
    fa->ProcessBuffer(startFrame2, conn4->SocketHandle);
    CHECK(VecToAnsi(conn4->SimTxBuffer()) == expectedStart, "START_REQUEST (RUNNING, fail path taken internally) -> STILL START_REPLY[1][0] (same observable reply either way)");

    bLockByServer = savedLock;
    SoftStop = savedSoftStop;
    SystemStart = savedSystemStart;
    fMain->palMainStatus->Caption = savedCaption;
}

// ===========================================================================
//  PART 19 -- CATEGORY_REQUEST + LastSet.OLPSetBinErr[0] latch
// ===========================================================================
static void Part19_CategoryLatch(TfAutomation *fa)
{
    printf("\n[19] CATEGORY_REQUEST + LastSet.OLPSetBinErr[0] latch\n");

    int savedErr = LastSet.OLPSetBinErr[0];

    // Self-referential oracle (design §6 P19 pattern): call SetCategory(...)
    // directly with the SAME input first to get the expected return value --
    // verifies the DISPATCH WIRING is correct, not SetCategory's own internal
    // logic (already covered by test_auto9045.cpp).
    AnsiString oracleData[40];
    oracleData[0] = "0";   // Auto1
    AnsiString expectedReturn = AnsiString(SetCategory(oracleData));

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 19000);
    fa->iSocketHandle = conn->SocketHandle;
    AnsiString frame = MakeInFrame("CATEGORY_REQUEST", 1, "0");
    fa->ProcessBuffer(frame, conn->SocketHandle);

    AnsiString expectedReply = STX + AnsiString("CATEGORY_REPLY") + SOH + AnsiString("1") + SOH + expectedReturn + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expectedReply, "CATEGORY_REQUEST's reply matches a direct SetCategory(...) call with the same input (dispatch-wiring oracle)");

    int expectedLatch = (atoi(expectedReturn.c_str()) >= 3) ? atoi(expectedReturn.c_str()) : 0;
    CHECK(LastSet.OLPSetBinErr[0] == expectedLatch, "LastSet.OLPSetBinErr[0] latch rule: >=3 stores the value, else 0 (golden :1644-1647)");

    LastSet.OLPSetBinErr[0] = savedErr;
}

// ===========================================================================
//  PART 20 -- CheckNeedCleanOut()-gated REQUEST family (TEMPERATURE_REQUEST
//  representative)
// ===========================================================================
static void Part20_CheckNeedCleanOutGatedFamily(TfAutomation *fa)
{
    printf("\n[20] CheckNeedCleanOut()-gated REQUEST family (TEMPERATURE_REQUEST)\n");

    // Offline, CheckCanChangeRealDummy()==true and HasICUnderMachine()==false
    // (empty Sim-HAL grid) -> CheckNeedCleanOut()==false -> the REAL SetTemperature
    // path runs (not the "1"-error fallback). golden then unconditionally
    // overwrites Data[0]="0" AFTER calling SetTemperature(Data), so the reply
    // is deterministic regardless of SetTemperature's own internal effect.
    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 20000);
    fa->iSocketHandle = conn->SocketHandle;
    AnsiString frame = MakeInFrame("TEMPERATURE_REQUEST", 1, "25.0");
    fa->ProcessBuffer(frame, conn->SocketHandle);
    AnsiString expected = STX + AnsiString("TEMPERATURE_REPLY") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expected, "TEMPERATURE_REQUEST (CheckNeedCleanOut()==false offline) -> real SetTemperature path -> TEMPERATURE_REPLY[1][0]");
}

// ===========================================================================
//  PART 21 -- LotInfo_REQUEST dual branch (default SetLotInfo vs Greatek+OEE
//  SetLotInfoGreatekOEE)
// ===========================================================================
static void Part21_LotInfoDualBranch(TfAutomation *fa)
{
    printf("\n[21] LotInfo_REQUEST dual branch (SetLotInfo vs SetLotInfoGreatekOEE)\n");

    int savedCC = CUSTOMER_CODE;
    bool savedOEE = CosFunction.bOEEFunction;
    bool savedMonitor = IniConfig.bC11UseMonitorView;

    // (a) default (non-Greatek-OEE) path -> SetLotInfo(Data), 1-field reply.
    CUSTOMER_CODE = 0;
    CosFunction.bOEEFunction = false;
    AnsiString oracleData[40];
    AnsiString expectedA = AnsiString(SetLotInfo(oracleData));

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 21000);
    fa->iSocketHandle = conn->SocketHandle;
    AnsiString frameA = MakeInFrame("LotInfo_REQUEST", 0);
    fa->ProcessBuffer(frameA, conn->SocketHandle);
    AnsiString expectedReplyA = STX + AnsiString("LotInfo_REPLY") + SOH + AnsiString("1") + SOH + expectedA + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expectedReplyA, "LotInfo_REQUEST (default customer) -> SetLotInfo path -> LotInfo_REPLY[1][<SetLotInfo's own return>]");

    // (b) Greatek + OEE-enabled path -> SetLotInfoGreatekOEE(Data,&errMsg).
    // Force bC11UseMonitorView=false so the offline OEE_StartLot()==false
    // path is reached deterministically (design's own analysis of the
    // offline gate chain).
    CUSTOMER_CODE = CC_Greatek;
    CosFunction.bOEEFunction = true;
    IniConfig.bC11UseMonitorView = false;
    AnsiString oracleErrMsg = "";
    AnsiString oracleData2[40];
    int expectedRetB = SetLotInfoGreatekOEE(oracleData2, oracleErrMsg);
    AnsiString expectedB = AnsiString(expectedRetB);

    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 21001);
    fa->iSocketHandle = conn2->SocketHandle;
    AnsiString frameB = MakeInFrame("LotInfo_REQUEST", 0);
    fa->ProcessBuffer(frameB, conn2->SocketHandle);
    AnsiString onWireB = VecToAnsi(conn2->SimTxBuffer());
    if (expectedB == "0")
    {
        AnsiString expectedReplyB = STX + AnsiString("LotInfo_REPLY") + SOH + AnsiString("1") + SOH + expectedB + ETX;
        CHECK(onWireB == expectedReplyB, "LotInfo_REQUEST (Greatek+OEE, success) -> LotInfo_REPLY[1][0]");
    }
    else
    {
        AnsiString expectedReplyB = STX + AnsiString("LotInfo_REPLY") + SOH + AnsiString("2") + SOH + expectedB + SOH + oracleErrMsg + ETX;
        CHECK(onWireB == expectedReplyB, "LotInfo_REQUEST (Greatek+OEE, offline-fail path) -> LotInfo_REPLY[2][<code>][<errMsg>]");
    }

    CUSTOMER_CODE = savedCC;
    CosFunction.bOEEFunction = savedOEE;
    IniConfig.bC11UseMonitorView = savedMonitor;
}

// ===========================================================================
//  PART 22 -- ProcessBuffer1 (safe partial coverage -- see this file's own
//  file-head note for why the live PP_DL_REQUEST+DoDLRequest path is
//  deliberately NOT driven here)
// ===========================================================================
static void Part22_ProcessBuffer1SafePartial(TfAutomation *fa)
{
    printf("\n[22] ProcessBuffer1 entry parsing + tail TCPstr-clear (non-PP_DL_REQUEST path)\n");

    AnsiString savedTCPstr = TCPstr;

    // A Command other than "PP_DL_REQUEST" exercises ProcessBuffer1's entry
    // parsing (IsStartWord + Command extraction) and the unconditional tail
    // `TCPstr="";` WITHOUT ever reaching the `if(Command=="PP_DL_REQUEST")`
    // block that calls DoDLRequest -- see file-head note for why that call is
    // out of this test's envelope.
    TCPstr = STX + AnsiString("SOME_OTHER_COMMAND") + SOH + AnsiString("0") + ETX;
    fa->ProcessBuffer1();
    CHECK(TCPstr == "", "ProcessBuffer1 clears the global TCPstr via its unconditional tail statement even on a non-PP_DL_REQUEST Command");

    // No leading STX -> the FIRST early return (before the tail TCPstr="" is
    // ever reached) -- TCPstr is left untouched.
    TCPstr = AnsiString("NO_STX_HERE");
    fa->ProcessBuffer1();
    CHECK(TCPstr == "NO_STX_HERE", "ProcessBuffer1 leaves TCPstr untouched when IsStartWord fails (returns before the tail clear)");

    TCPstr = savedTCPstr;
}

// ===========================================================================
//  PART 23 -- INITIATE_REQUEST with a guaranteed-invalid SYSTEMTIME payload
//  (system-clock-untouched proof)
// ===========================================================================
static void Part23_InitiateRequestSafePayload(TfAutomation *fa)
{
    printf("\n[23] INITIATE_REQUEST (guaranteed-invalid payload -- system clock must be untouched)\n");

    int savedCC = CUSTOMER_CODE;
    CUSTOMER_CODE = 0;   // non-MTI/PTI -> skip the OLPServer reconnect dance entirely

    AnsiString yearBefore = fa->GetTimeInfo().SubString(1, 4);

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 23000);
    fa->iSocketHandle = conn->SocketHandle;
    // all-zero date/time digits (year=0000,month=00,day=00,...) -- Win32
    // SetLocalTime() rejects invalid field values (wMonth=0 is out of the
    // valid 1-12 range) and returns FALSE WITHOUT changing the real system
    // clock; golden itself never checks the return value either way.
    AnsiString frame = MakeInFrame("INITIATE_REQUEST", 1, "00000000000000");
    fa->ProcessBuffer(frame, conn->SocketHandle);

    AnsiString yearAfter = fa->GetTimeInfo().SubString(1, 4);
    CHECK(yearBefore == yearAfter, "system wall-clock year is unchanged after INITIATE_REQUEST with an all-zero (invalid) SYSTEMTIME payload");

    AnsiString expected = STX + AnsiString("INITIATE_REPLY") + SOH + AnsiString("1") + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expected, "INITIATE_REQUEST -> INITIATE_REPLY[1][0]");

    CUSTOMER_CODE = savedCC;
}

// ===========================================================================
//  PART 24 -- e2e: SimPushReceive -> OLPServerClientRead -> ProcessBuffer ->
//  reply on the SAME connection (proves the whole socket->dispatch->reply
//  chain is alive post-un-gate)
// ===========================================================================
static void Part24_EndToEndSimPushReceive(TfAutomation *fa)
{
    printf("\n[24] e2e: SimPushReceive -> dispatch -> reply on the same connection\n");

    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 24000);
    ReceiveString = "";
    fa->bReceive = false;
    fa->bReceiveHasData = false;

    AnsiString frame = MakeInFrame("VERSION_INQUIRE", 0);
    conn->SimPushReceive(frame.c_str(), frame.Length());

    AnsiString expected = STX + AnsiString("VERSION_GRANT") + SOH + AnsiString("2") + SOH + GetSoftwareVersion() + SOH + AnsiString("0") + ETX;
    CHECK(VecToAnsi(conn->SimTxBuffer()) == expected, "SimPushReceive -> OLPServerClientRead -> ProcessBuffer -> reply lands on the SAME connection's SimTxBuffer");
    CHECK(ReceiveString.Length() == 0, "ReceiveString is drained after the round trip");
    CHECK(fa->bReceive == false, "bReceive is reset false after the round trip");
}

// ===========================================================================
//  PART 25 -- SendReportRequest direct-call supplement (ReportID branches not
//  reached via Part 17's REPORT_INQUIRE path; unknown-ID D3 oracle; Standard=
//  false OLPClient routing)
// ===========================================================================
static void Part25_SendReportRequestSupplement(TfAutomation *fa)
{
    printf("\n[25] SendReportRequest direct-call supplement\n");

    int savedCC = CUSTOMER_CODE;

    // (a) "0002": default customer -> iTotal=4; CC_PTI -> iTotal=7.
    CUSTOMER_CODE = 0;
    TCustomWinSocket *conn = fa->OLPServer->SimAcceptConnection("127.0.0.1", 25000);
    fa->iSocketHandle = conn->SocketHandle;
    fa->SendReportRequest("0002", true);
    AnsiString onWire1 = VecToAnsi(conn->SimTxBuffer());
    CHECK(onWire1.Pos(AnsiString("REPORT_REQUEST")) != 0, "SendReportRequest(\"0002\") sends a REPORT_REQUEST frame");
    // SV_TOTAL is the field right after ReportID "0002" in the HEAD -- locate
    // it via the SOH-delimiter-bounded "...SOH 0002 SOH <digit> SOH..." pattern
    // (safer than a bare substring search, which could coincidentally match
    // inside R_TIME's own 14 timestamp digits).
    AnsiString marker = SOH + AnsiString("0002") + SOH;
    int p1 = onWire1.Pos(marker);
    AnsiString svTotalDefault = onWire1.SubString(p1 + marker.Length(), 1);
    CHECK(svTotalDefault == "4", "SendReportRequest(\"0002\") default customer -> SV_TOTAL=\"4\"");

    CUSTOMER_CODE = CC_PTI;
    TCustomWinSocket *conn1b = fa->OLPServer->SimAcceptConnection("127.0.0.1", 25001);
    fa->iSocketHandle = conn1b->SocketHandle;
    fa->SendReportRequest("0002", true);
    AnsiString onWire1b = VecToAnsi(conn1b->SimTxBuffer());
    int p1b = onWire1b.Pos(marker);
    AnsiString svTotalPti = onWire1b.SubString(p1b + marker.Length(), 1);
    CHECK(svTotalPti == "7", "SendReportRequest(\"0002\") CUSTOMER_CODE==CC_PTI -> SV_TOTAL=\"7\"");
    CUSTOMER_CODE = savedCC;

    // (b) "0007": fixed {"100","101"} data fields.
    TCustomWinSocket *conn2 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 25002);
    fa->iSocketHandle = conn2->SocketHandle;
    fa->SendReportRequest("0007", true);
    AnsiString onWire2 = VecToAnsi(conn2->SimTxBuffer());
    CHECK(onWire2.Pos(AnsiString("0007")) != 0, "SendReportRequest(\"0007\") echoes ReportID \"0007\"");
    CHECK(onWire2.Pos(SOH + AnsiString("100") + SOH + AnsiString("101") + ETX) != 0, "SendReportRequest(\"0007\") -> fixed data fields {\"100\",\"101\"}");

    // (c) unknown ReportID "9999" -- D3 oracle: iTotal initialized to 0 (not
    // golden's UB-uninitialized read) -> SV_TOTAL="0", zero data fields.
    TCustomWinSocket *conn3 = fa->OLPServer->SimAcceptConnection("127.0.0.1", 25003);
    fa->iSocketHandle = conn3->SocketHandle;
    fa->SendReportRequest("9999", true);
    AnsiString onWire3 = VecToAnsi(conn3->SimTxBuffer());
    CHECK(onWire3.Pos(SOH + AnsiString("9999") + SOH + AnsiString("0") + ETX) != 0, "SendReportRequest(\"9999\") (unknown ReportID) -> SV_TOTAL=\"0\", zero data fields (D3 oracle, no crash)");

    // (d) Standard=false -> routes through SendServer/OLPClient instead of
    // SendClient/OLPServer (same routing family as ProcessBuffer1's
    // PP_DL_REPLY, P13).
    fa->OnLine2->Enabled = true;
    fa->SendReportRequest("0007", false);
    AnsiString onWireClient = VecToAnsi(fa->OLPClient->Socket->SimTxBuffer());
    CHECK(onWireClient.Pos(AnsiString("REPORT_REQUEST")) != 0, "SendReportRequest(...,false) routes through SendServer -> bytes appear on OLPClient->Socket, not OLPServer");
}

int main()
{
    Part1_FreeFunctions();

    TfAutomation *fa = new TfAutomation(NULL);

    Part2_EventApi(fa);
    Part3_GetTimeInfo(fa);
    Part4_CtorBranching();
    Part5_DoCommandBuffer(fa);
    Part6_CommandProcessNoConnection(fa);
    Part7_CommandProcessWithConnection(fa);
    Part8_WriteAndSendServer(fa);
    Part9_MemoLogging(fa);
    Part10_HtmlJsonAndFacade(fa);
    Part11_OLPServerClientRead(fa);
    Part12_TmrOLPTimerSmoke(fa);
    Part13_ProcessBufferEntryGuard(fa);
    Part14_GenericInquireHappyPath(fa);
    Part15_InquireQuirkTrio(fa);
    Part16_EmptyInquireAndActTemp(fa);
    Part17_ReportInquireDualFrame(fa);
    Part18_ControlRequestFamily(fa);
    Part19_CategoryLatch(fa);
    Part20_CheckNeedCleanOutGatedFamily(fa);
    Part21_LotInfoDualBranch(fa);
    Part22_ProcessBuffer1SafePartial(fa);
    Part23_InitiateRequestSafePayload(fa);
    Part24_EndToEndSimPushReceive(fa);
    Part25_SendReportRequestSupplement(fa);

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
