// =============================================================================
//  test_st02_w191_aoirecv.cpp -- W-191 MR-2: TAOISocket::ReceiveData reassembles a reply split across TCP segments
//  (golden 913 TfAOILaserScan.cpp:32-43, Ifor 20260827; golden 0618 :30-37 overwrote the message with each segment).
//
//  AI(W906-W191) 20261009 (St02-E).  Suite name (add_test): St02_W191AoiRecv.  No socket is opened: ReceiveData is called the way
//  uSocketClient's receive callback calls it (TfAOILaserScan.cpp:217).
//    [1] "RECIPE_LIST,a,b" then "c,d@" -> asReceiveMsg == "RECIPE_LIST,a,bc,d@" (0618 kept only "c,d@");
//    [2] after a complete reply (has '@') the next segment starts a new reply;
//    [3] each segment is cut to iLen before it is joined;
//    [4] the log callback sees each segment, not the joined reply.
// =============================================================================
#include "MachineDefine.h"
#include "TfAOILaserScan.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

static void Feed(TAOISocket& s, const char* text, int len)
{
    std::vector<char> buf(text, text + std::strlen(text) + 1);
    s.ReceiveData(buf.data(), len);
}

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W191AoiRecv -- TAOISocket::ReceiveData (golden 913 TfAOILaserScan.cpp:32-43)\n");
    std::vector<std::string> logged;
    TAOISocket s;
    s.SetRecordMsg([&logged](AnsiString m) { logged.push_back(m.c_str()); });

    std::printf("[1] a reply split in two TCP segments\n");
    s.asReceiveMsg = "";
    Feed(s, "RECIPE_LIST,a,b", 15);
    Check(std::string(s.asReceiveMsg.c_str()) == "RECIPE_LIST,a,b", "[1] first segment kept (no '@' yet)");
    Feed(s, "c,d@", 4);
    Check(std::string(s.asReceiveMsg.c_str()) == "RECIPE_LIST,a,bc,d@",
          "[1] second segment appended -> the whole reply (" + std::string(s.asReceiveMsg.c_str()) + ")");

    std::printf("[2] the next reply after a complete one\n");
    Feed(s, "OK@", 3);
    Check(std::string(s.asReceiveMsg.c_str()) == "OK@", "[2] a complete reply ('@' seen) is replaced, not appended (" + std::string(s.asReceiveMsg.c_str()) + ")");

    std::printf("[3] iLen cuts each segment before joining\n");
    s.asReceiveMsg = "";
    Feed(s, "ABCDEFxyz", 6);
    Feed(s, "GH@zzz", 3);
    Check(std::string(s.asReceiveMsg.c_str()) == "ABCDEFGH@", "[3] \"ABCDEF\" + \"GH@\" (" + std::string(s.asReceiveMsg.c_str()) + ")");

    std::printf("[4] the log sees each segment\n");
    const bool seg = logged.size() >= 2 && logged[0] == "[Receive]RECIPE_LIST,a,b" && logged[1] == "[Receive]c,d@";
    Check(seg, "[4] log lines are the segments: '" + (logged.size() > 1 ? logged[1] : std::string("?")) + "'");

    std::printf("\nSt02_W191AoiRecv: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
