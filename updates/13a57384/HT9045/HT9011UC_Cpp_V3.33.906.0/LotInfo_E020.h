// =============================================================================
//  LotInfo_E020.h -- todo E-020 LI-6 / LI-11 / LI-13 (St01): golden TfLotInfo members the port did not have
//
//  //AI(W906-E020-LI6) 20261002 [W906] (St01): new file. golden = V912
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp (Big5):
//      LI-6  enum eWaitRtcDeleteTask :5357-5363, TQPF_Timer WaitRtcDeleteDelay :5364, TfLotInfo::Timer1Timer :5366-5453,
//            TfLotInfo::RTCChangeFile :5455-5475; uLotInfo.h Timer1 :30 (dfm :14621-14626 Enabled=False Interval=100),
//            bNeedToDeleteFile, iWaitRtcDeleteTask, bRTCChangeFileFinish, RTCChangeFile(bool bNeedDelete=true).
//      LI-11 TfLotInfo::btChangeFileClick :10394-10418 (dfm btChangeFile :4641-4648; Visible = FormShow :572).
//      LI-13 TfLotInfo::palSecsGemMouseDown :10532-10606 (dfm palSecsGem :314-323) -- the body is jimmychiu's facade copy
//            forms/fLotInfo.cpp:4794 (golden as is, uncalled until now); this file only routes the web click to it.
//  The member declarations (RTCChangeFile, Timer1Timer, Timer1) are on forms/fLotInfo.h:2333 (same line); their bodies and the
//  web actions are in LotInfo_E020.cpp (ht9045_sm, CMakeLists.txt:2499 same line).
//  Web: act.lotInfo.changeFile / act.lotInfo.palSecsGemMouseDown via JsonBridge/ChanAction.cpp:348 (same line) ->
//    ht9045::sjson::W906_LotInfoE020Act; page D:\HT9045\web\page\ht9045_lotinfo_e020.js.
//  ctest: E020_LotInfoRTC, E020_LotInfoActs (tests/test_e020_lotinfo_e020.cpp), E020_LotInfoPage (node).
// =============================================================================
#ifndef LotInfo_E020H
#define LotInfo_E020H

#include <string>
#include <vector>

// ---- LI-6 ---------------------------------------------------------------------------------------------------------------
// golden TfLotInfo::Timer1 is a TTimer (Enabled=False, Interval=100 ms; dfm :14621-14626) whose OnTimer is Timer1Timer.
//   Nothing in the port ticks TfLotInfo timers yet (todo X-1 / E-025 timer table). A future timer card calls this every 100 ms:
//   it does what VCL does for an enabled TTimer -- runs Timer1Timer only while fLotInfo->Timer1->Enabled.
//   true = Timer1Timer ran.
bool W906_TfLotInfo_Timer1Timer();

// Every COM2 RTC-vision statement in Timer1Timer is GATEd (TCOM2Shim, atester_shims.h:373-462, has no RTC vision half:
//   sRealTimeCom_Send / SendCommToVision / bRealTimeCom_ReceiveOK / rt* channels). Each skip is printed and kept here (last 200).
struct W906E020RtcSkip
{
    std::string gate;     // "W906-E020-LI6-n"
    std::string golden;   // golden line + statement
    std::string value;    // what golden would have sent / stored (the formatted string), "" if none
};
const std::vector<W906E020RtcSkip>& W906_E020_RtcSkips();
long W906_E020_RtcSkipTotal();          // all skips since start (the list keeps the last 200)
void W906_E020_RtcSkipsClear();         // tests only

// ---- LI-11 --------------------------------------------------------------------------------------------------------------
// golden btChangeFileClick's branch choice (:10396-10417) as golden. 0 = golden takes no branch (bEnableBarCode off, or none of
//   the three BAR_CODE_INSTALL cases) -- nothing happens, as golden. 1 / 2 / 3 = the branch golden takes; none is ported
//   (fBarCode has no change-file chain), so nothing is done and *whyNot = "GATE (W906-E020-LI11-n): ...". bBarcodeConnect is
//   NOT set alone (branches 2 / 3 set it only together with the scan init and the timer).
int W906_E020_btChangeFileClick(std::string* whyNot);

// ---- web ------------------------------------------------------------------------------------------------------------------
namespace ht9045 {
namespace sjson {
// JsonBridge/ChanAction.cpp:348 declares this at block scope (same namespace) and calls it for every act.lotInfo.*.
std::string W906_LotInfoE020Act(const std::string& cmd, const std::string& payloadJson);
}  // namespace sjson
}  // namespace ht9045

#endif  // LotInfo_E020H
