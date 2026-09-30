// ===========================================================================
//  tests/test_main_ctlbuttons_stubs.cpp
//
//  AI(W906-FLOW-4) 20260930: link-only stand-ins for tests/test_main_ctlbuttons.cpp.
//  FileRW/MainClick.cpp is compiled into that test whole (its act.main.ctlButton queue is what the test drives), and its
//  OTHER entry points name functions that live in wb_serve's own sources: the FileRW window-edge bodies
//  (FileRW/WindowEdgeTails.h, one per FileRW/<struct>.cpp), three FileRW helpers, and the form-bridge lock
//  (JsonBridge/FormJson.cpp).  The set below is exactly the linker's undefined-reference list of the first build
//  (20260930), nothing more.
//  The test never calls any of the FileRW ones: each ABORTS if reached, so a future change that routes the ctlButton path
//  through one of them fails loudly instead of passing on a stand-in.  FormLock / FormUnlock ARE on the tested path
//  (St01's queue takes the lock around every push / pop, the consumer around every body): a single-threaded test needs no
//  lock, wb_serve's real one is a CRITICAL_SECTION (JsonBridge/FormJson.cpp:39).
// ===========================================================================
#include <cstdio>
#include <cstdlib>

namespace ht9045 {
namespace formjson {
void FormLock() {}
void FormUnlock() {}
}  // namespace formjson
}  // namespace ht9045

static void W906StubReached(const char* name)
{
    std::printf("FAIL: link-only stand-in %s was CALLED (tests/test_main_ctlbuttons_stubs.cpp)\n", name);
    std::fflush(stdout);
    std::abort();
}

#define W906_EDGE_STUB(fn) const char* fn(bool) { W906StubReached(#fn); return nullptr; }
W906_EDGE_STUB(FileRW_YieldMonitoring_WindowEdge)
W906_EDGE_STUB(FileRW_Setup_WindowEdge)
W906_EDGE_STUB(FileRW_Offset_WindowEdge)
W906_EDGE_STUB(FileRW_Contact_WindowEdge)
W906_EDGE_STUB(FileRW_IniConfig_WindowEdge)
W906_EDGE_STUB(FileRW_Speed_WindowEdge)
W906_EDGE_STUB(FileRW_LdUld_WindowEdge)
W906_EDGE_STUB(FileRW_TrayForm_WindowEdge)
W906_EDGE_STUB(FileRW_TrayAssignment_WindowEdge)
W906_EDGE_STUB(FileRW_Temperature_WindowEdge)
W906_EDGE_STUB(FileRW_TTLCfg_WindowEdge)
W906_EDGE_STUB(FileRW_QAMode_WindowEdge)
W906_EDGE_STUB(FileRW_BarCode_WindowEdge)
W906_EDGE_STUB(FileRW_VacuumUnit_WindowEdge)
W906_EDGE_STUB(FileRW_BinSelect_WindowEdge)
W906_EDGE_STUB(FileRW_HSys_WindowEdge)
W906_EDGE_STUB(FileRW_CounterSel_WindowEdge)
W906_EDGE_STUB(FileRW_StartCondition_WindowEdge)
W906_EDGE_STUB(FileRW_Cleaning_WindowEdge)
#undef W906_EDGE_STUB

void FileRW_Setup_ContactReadFile()      { W906StubReached("FileRW_Setup_ContactReadFile"); }       // FileRW/TestIF_File_SetUp.cpp
void FileRW_SetDummyVacuumToSucks()      { W906StubReached("FileRW_SetDummyVacuumToSucks"); }       // FileRW/_KitSuck.cpp
void FileRW_Cleaning_LoadAutoCleanData() { W906StubReached("FileRW_Cleaning_LoadAutoCleanData"); }  // FileRW/TestIF_File_Cleaning.cpp

// FileRW/IniConfig.cpp:222.  NOT an abort stub: cprod.cpp (ht9045_globals) calls it, and ht9045_globals already carries a no-op
// fallback for it (FileRW/_fallback.cpp).  That fallback member also defines FileRW_ProxyChecked, which FileRW/_EditList.cpp (compiled
// into this test for MainClick.cpp's filerw:: calls) defines for real -- pulling the member would be a duplicate definition.  So this
// test defines the one symbol the member was pulled for, with the fallback's own body (FileRW/_fallback.cpp:23: do nothing).
void FileRW_IniConfig_ChangeCBListProperty() {}
