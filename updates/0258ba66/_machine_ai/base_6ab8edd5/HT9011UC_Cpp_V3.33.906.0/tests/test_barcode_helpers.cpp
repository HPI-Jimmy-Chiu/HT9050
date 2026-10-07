// tests/test_barcode_helpers.cpp
// Verification harness for the BarCode/BarCode_Helpers.{h,cpp} translation
// (smallest W5-BarCode unit: TfHIKROBOTcmd, uCCDUnloaderClip,
// BarCode_IsSHT2DIDScanFinish).
//
// Golden reference: HT9011UC_Code_V3.33.906.0_20260618/BarCode/BarCode.h/.cpp
//   TfHIKROBOTcmd::GetStartCmd/GetAnalysisResult   :11568-11588
//   uCCDUnloaderClip                                :11591-11719
//   TfBarCode::IsSHT2DIDScanFinish                  :9097-9152
//
// LIMITATION (stated explicitly, same as test_ContactForce.cpp): no Borland
// binary exists in this environment. Verification here is therefore:
//   (1) the translation compiles clean under MinGW g++, AND
//   (2) its outputs equal values hand-derived from the BCB6 source (AnsiString
//       1-based Pos/SubString semantics) / match the documented gated-socket
//       conservative defaults / reproduce the documented missing-semicolon
//       quirk in SendCommandStr.
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure.

#include "BarCode/BarCode_Helpers.h"
#include "aHotPlateSubstrate.h"   // FLCarryKit / BLCarryKit (TMyKitSuck) for part 3
#include "cmydef.h"               // HAS_IC / HAS_HOT_IC
#include <cstdio>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void check_s(const char* name, const std::string& got, const std::string& expected)
{
    if (got == expected) { printf("PASS  %-46s got=\"%s\"\n", name, got.c_str()); ++g_pass; }
    else { printf("FAIL  %-46s got=\"%s\" exp=\"%s\"\n", name, got.c_str(), expected.c_str()); ++g_fail; }
}

static void check_b(const char* name, bool got, bool expected)
{
    if (got == expected) { printf("PASS  %-46s got=%d\n", name, (int)got); ++g_pass; }
    else { printf("FAIL  %-46s got=%d exp=%d\n", name, (int)got, (int)expected); ++g_fail; }
}

static void check_i(const char* name, int got, int expected)
{
    if (got == expected) { printf("PASS  %-46s got=%d\n", name, got); ++g_pass; }
    else { printf("FAIL  %-46s got=%d exp=%d\n", name, got, expected); ++g_fail; }
}

// ---------------------------------------------------------------------------
//  Part 3 helper: FLCarryKit/BLCarryKit are process-wide globals (defined in
//  aHotPlateSubstrate.cpp) -- reset to a known-empty baseline before each
//  check so PARTs stay independent of execution order (same convention as
//  test_amr.cpp's Reset* helpers).
// ---------------------------------------------------------------------------
static void ResetKit(TMyKitSuck &kit, int iShtRow, int iShtCol)
{
    kit.ClearAll();                                   // Item[][] = NULL_IC (golden :312)
    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; i++)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; j++)
            kit.cDeviceInf[i][j] = "";
    kit.iShtRow = iShtRow;
    kit.iShtCol = iShtCol;
}

int main()
{
    printf("=== BarCode_Helpers translation verification ===\n");
    printf("(values hand-derived from BCB6 BarCode.cpp; original binary NOT run)\n\n");

    // -------------------------------------------------------------------
    // Part 1: TfHIKROBOTcmd -- pure string builders (golden :11568-11588).
    // -------------------------------------------------------------------
    printf("-- TfHIKROBOTcmd --\n");
    {
        TfHIKROBOTcmd cmd;
        check_s("GetStartCmd()", cmd.GetStartCmd().c_str(), "start");
    }
    {
        // "12LF34@56": lfPos=Pos("LF")=3 (1-based) -> temp=SubString(5,9)="34@56"
        // -> atPos=temp.Pos("@")=3 -> result=temp.SubString(1,2)="34".
        TfHIKROBOTcmd cmd;
        AnsiString r = cmd.GetAnalysisResult("12LF34@56");
        check_s("GetAnalysisResult normal", r.c_str(), "34");
    }
    {
        // No "LF" anywhere -> lfPos=0 -> if(lfPos>0) false -> result stays "".
        TfHIKROBOTcmd cmd;
        AnsiString r = cmd.GetAnalysisResult("hello@world");
        check_s("GetAnalysisResult no-LF", r.c_str(), "");
    }
    {
        // "LF" present but no "@" after it -> temp="yyyy", atPos=0 -> result "".
        TfHIKROBOTcmd cmd;
        AnsiString r = cmd.GetAnalysisResult("xxLFyyyy");
        check_s("GetAnalysisResult LF-no-at", r.c_str(), "");
    }

    // -------------------------------------------------------------------
    // Part 2: uCCDUnloaderClip -- gated-socket helper (golden :11591-11719).
    // Every case below exercises the OFFLINE (no live EJ1N socket, no
    // SOFT_SIMULTE) conservative-default path -- see BarCode_Helpers.cpp's
    // gate comments for the exact golden line each default replaces.
    // -------------------------------------------------------------------
    printf("-- uCCDUnloaderClip --\n");
    {
        uCCDUnloaderClip clip;
        check_b("ctor InitialDatas bGetResult", clip.bGetResult, false);
        check_i("ctor InitialDatas iRetryConn", clip.iRetryConn, 0);
        check_s("ctor InitialDatas sIP", clip.sIP.c_str(), "");
        check_s("ctor InitialDatas sPort", clip.sPort.c_str(), "");
        check_s("ctor InitialDatas asReceiveMsg", clip.asReceiveMsg.c_str(), "");
    }
    {
        // SetCommParameter is a fully-gated socket call -> conservative
        // default: accept offline (golden BarCode.cpp:11651 always forwarded
        // to socketClient->SetCommParameter; we cannot reach the real socket).
        uCCDUnloaderClip clip;
        check_b("SetCommParameter offline default", clip.SetCommParameter("1.2.3.4", "6000"), true);
    }
    {
        // DoCommuncation(): bSimulationCommand==false (no SOFT_SIMULTE defined
        // in this build) -> falls through to the gated socketClient->
        // DoOpenCommuncation() call -> conservative default false (golden
        // BarCode.cpp:11658, "no live socket offline").
        uCCDUnloaderClip clip;
        check_b("DoCommuncation offline default (no sim)", clip.DoCommuncation(), false);
    }
    {
        // FAITHFUL missing-semicolon quirk (golden BarCode.cpp:11642-11647,
        // see BarCode_Helpers.cpp comment): with bSimulationCommand==false
        // (this build), SendCommandStr's whole body collapses to nothing --
        // SendCommandSocket is NEVER called, so asReceiveMsg (which
        // SendCommandSocket would reset to "") must stay untouched.
        uCCDUnloaderClip clip;
        clip.asReceiveMsg = "marker";
        clip.SendCommandStr("hello");
        check_s("SendCommandStr quirk: no-op when not simulating", clip.asReceiveMsg.c_str(), "marker");
    }
    {
        // SendCommandPhoto() -> bSimulationCommand==false -> calls
        // SendCommandSocket(GetStartCmd()="start"). Inside: socketClient->
        // SendCommand is gated -> bflag=false (conservative default) -> the
        // RecMsgCCDUnloaderClip closure's ELSE branch fires, reporting
        // "[Send Error]"+cmd+"[IP]"+sIP+"[Port]"+sPort (sIP/sPort substituted
        // for the gated socketClient->GetSocketAddress()/GetSocketPort() --
        // see BarCode_Helpers.cpp's comment on why sIP/sPort are a faithful
        // stand-in). asReceiveMsg is reset to "" by SendCommandSocket first.
        uCCDUnloaderClip clip;
        clip.sIP   = "1.2.3.4";
        clip.sPort = "6000";
        clip.asReceiveMsg = "stale";
        std::string captured;
        clip.RecMsgCCDUnloaderClip = [&captured](AnsiString msg){ captured = msg.c_str(); };
        clip.SendCommandPhoto();
        check_s("SendCommandPhoto -> Send Error callback", captured, "[Send Error]start[IP]1.2.3.4[Port]6000");
        check_s("SendCommandPhoto resets asReceiveMsg", clip.asReceiveMsg.c_str(), "");
    }
    {
        // DoGetPhotoCmd(bReset=true, ...) -- pure cursor/flag reset, no gate.
        uCCDUnloaderClip clip;
        clip.iRetryConn = 7;
        clip.bGetResult = true;
        AnsiString sErr = "stale";
        AnsiString r = clip.DoGetPhotoCmd(true, sErr);
        check_s("DoGetPhotoCmd(reset) return", r.c_str(), "");
        check_s("DoGetPhotoCmd(reset) sErr", sErr.c_str(), "");
        check_i("DoGetPhotoCmd(reset) iRetryConn", clip.iRetryConn, 0);
        check_b("DoGetPhotoCmd(reset) bGetResult", clip.bGetResult, false);
    }
    {
        // DoGetPhotoCmd task 1, empty sIP/sPort (default) -> DoCommuncation()
        // offline-false -> sErr formatted from the (empty) sIP/sPort.
        uCCDUnloaderClip clip;
        AnsiString sErr;
        AnsiString r = clip.DoGetPhotoCmd(false, sErr);
        check_s("DoGetPhotoCmd task1 default sErr", sErr.c_str(), "Socket connection failed.IP: Port:");
        check_s("DoGetPhotoCmd task1 default return", r.c_str(), "");
    }
    {
        // Same, with sIP/sPort configured -- proves the %s substitution.
        uCCDUnloaderClip clip;
        clip.sIP   = "10.0.0.5";
        clip.sPort = "7000";
        AnsiString sErr;
        clip.DoGetPhotoCmd(false, sErr);
        check_s("DoGetPhotoCmd task1 custom sErr", sErr.c_str(), "Socket connection failed.IP:10.0.0.5 Port:7000");
    }
    {
        // DoSocketClose() -- smoke test only (bSimulationCommand==false ->
        // the socketClient->Close() call is gated to a pure no-op; must not
        // crash).
        uCCDUnloaderClip clip;
        clip.DoSocketClose();
        check_b("DoSocketClose does not crash", true, true);
    }

    // -------------------------------------------------------------------
    // Part 3: BarCode_IsSHT2DIDScanFinish (golden :9097-9152).
    // -------------------------------------------------------------------
    printf("-- BarCode_IsSHT2DIDScanFinish --\n");
    {
        // SHT=0, FLCarryKit has no real IC anywhere -> HasRealIC()==false ->
        // trivially finished (golden JerryYang 20230620 comment).
        ResetKit(FLCarryKit, 1, 1);
        ResetKit(BLCarryKit, 1, 1);
        check_b("SHT0 no-real-ic -> finished", BarCode_IsSHT2DIDScanFinish(0), true);
    }
    {
        // SHT=0, site [0][0] has HAS_IC but cDeviceInf=="" (not yet scanned)
        // -> not finished.
        ResetKit(FLCarryKit, 1, 1);
        FLCarryKit.Item[0][0] = HAS_IC;
        check_b("SHT0 HAS_IC + empty devinf -> NOT finished", BarCode_IsSHT2DIDScanFinish(0), false);
    }
    {
        // Same site, now with a decoded device-info string -> finished.
        ResetKit(FLCarryKit, 1, 1);
        FLCarryKit.Item[0][0] = HAS_IC;
        FLCarryKit.cDeviceInf[0][0] = "ABC123";
        check_b("SHT0 HAS_IC + real devinf -> finished", BarCode_IsSHT2DIDScanFinish(0), true);
    }
    {
        // HAS_HOT_IC site with cDeviceInf=="0" (placeholder, NOT a real scan)
        // -> still NOT finished (the =="0" half of the golden OR condition).
        ResetKit(FLCarryKit, 1, 1);
        FLCarryKit.Item[0][0] = HAS_HOT_IC;
        FLCarryKit.cDeviceInf[0][0] = "0";
        check_b("SHT0 HAS_HOT_IC + \"0\" devinf -> NOT finished", BarCode_IsSHT2DIDScanFinish(0), false);
    }
    {
        // SHT=1 reads BLCarryKit, NOT FLCarryKit -- prove the two kits are
        // not aliased: FLCarryKit left in a "not finished" state, BLCarryKit
        // has no real IC (trivially finished) -> SHT=1 must report finished.
        ResetKit(FLCarryKit, 1, 1);
        FLCarryKit.Item[0][0] = HAS_IC;               // FLCarryKit: NOT finished (if it were read)
        ResetKit(BLCarryKit, 1, 1);                   // BLCarryKit: no real IC -> finished
        check_b("SHT1 reads BLCarryKit not FLCarryKit", BarCode_IsSHT2DIDScanFinish(1), true);
    }
    {
        // SHT=1, BLCarryKit itself has an unscanned real IC -> NOT finished.
        ResetKit(BLCarryKit, 1, 1);
        BLCarryKit.Item[0][0] = HAS_IC;
        check_b("SHT1 BLCarryKit HAS_IC + empty devinf -> NOT finished", BarCode_IsSHT2DIDScanFinish(1), false);
    }
    {
        // FAITHFUL golden gap: SHT is neither 0 nor 1 -- no trailing `else`,
        // so bResult keeps its initial `true` regardless of grid contents.
        ResetKit(FLCarryKit, 1, 1);
        FLCarryKit.Item[0][0] = HAS_IC;               // would be "NOT finished" under SHT==0
        check_b("SHT=2 (unhandled) -> golden-gap default true", BarCode_IsSHT2DIDScanFinish(2), true);
    }

    // Leave the shared globals in a clean state for any test run after this
    // one in the same process (ctest may share nothing across executables,
    // but stay tidy regardless).
    ResetKit(FLCarryKit, 2, 1);
    ResetKit(BLCarryKit, 2, 1);

    // -------------------------------------------------------------------
    // Summary
    // -------------------------------------------------------------------
    printf("\n=== SUMMARY: %d passed, %d failed (total %d) ===\n",
           g_pass, g_fail, g_pass + g_fail);
    if (g_fail != 0)
    {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: ALL PASS\n");
    return 0;
}
