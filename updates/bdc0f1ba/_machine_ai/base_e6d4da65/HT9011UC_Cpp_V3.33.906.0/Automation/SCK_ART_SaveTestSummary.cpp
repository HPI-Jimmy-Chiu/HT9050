// AI(W906-PROD-G030) 20260926 (Steven 團隊)：golden TfSCKART::SaveTestSummary(int iSaveData)（V912 Automation/SCK_ART.cpp:1620-1646）
//   的共用轉接 —— 說明見 SCK_ART_SaveTestSummary.h。原本寫在 forms/fLotInfo.cpp 的 SetLotEnd（G-030，b40b140f），
//   Jimmy 20260926 23:4x（TO_STEVEN §4 ④）要抽成共用，給 csystem.cpp:2701／:3913 兩個空樁接。
//   放在 ht9045_sm（跟 Automation/SCK_ART_Remainder.cpp 同一個 library，CMakeLists.txt 同一行）。
#include "Automation/SCK_ART_SaveTestSummary.h"

#include <cstdio>
#include <cstring>              // std::memcpy

#include "cmydef.h"             // _2D_SORT（:87）、TCP_IP_MODE（:64）
#include "cprod.h"              // TestIF_File
#include "CosFunction.h"        // CosFunction
#include "LastSet.h"            // LastSet.iTester
#include "cSocket.h"            // LotSummary（真的那一份，cSocket.cpp:228）
#include "forms/fSCKART.h"   // fSCKART（真的那一份，forms/fSCKART.h:187）
#include "Automation/SCK_ART_Remainder.h"   // SckArtRemainderState／SckArtRem_SaveTestSummary／W5SckArtRem_LotSummary

static void W906_SckArtSkip(TStrings* skipped, const char* why)
{
    if (skipped != 0) skipped->Add(why);
    else std::printf("SaveTestSummary: %s\n", why);
}

void W906_SckArt_SaveTestSummary(int iSaveData, TStrings* skipped)
{
    // golden SCK_ART.cpp:1622-1631 的前兩支：客戶專屬、要的欄位真的 fSCKART 沒有 ⇒ 不跑（見 .h）
    const bool bSort2D  = (CosFunction.bSortingBy2DList==true && LastSet.iTester==_2D_SORT && TestIF_File.bSortingBy2DIDList==true);   // golden :1622-1624
    const bool bSECS93K = (CosFunction.bART_SECSGEM_93K==true);                                                                        // golden :1628
    if (bSort2D)
    {
        W906_SckArtSkip(skipped, "golden fSCKART->SaveTestSummary -> Save2DSortingSummary (golden SCK_ART.cpp:1622-1627; 2D sort customer branch needs SckArtRemainderState sInfo_* the port's fSCKART does not carry; S80/S25, gated)");
        return;
    }
    if (bSECS93K)
    {
        W906_SckArtSkip(skipped, "golden fSCKART->SaveTestSummary -> SaveTestSummarySECS (golden SCK_ART.cpp:1628-1631; 93K SECS ART customer branch needs SckArtRemainderState sInfo_*/sLotEndTime the port's fSCKART does not carry; S80/S25, gated)");
        return;
    }

    SckArtRemainderState st;                                                    // (1) 轉接：golden 的 fSCKART 就是這一個
    st.sLotID       = fSCKART->sLotID;
    st.sProcessCode = fSCKART->sProcessCode;
    st.iFTRTCount   = fSCKART->iFTRTCount;
    st.iNeedRT      = fSCKART->iNeedRT;
    st.iLotCount    = fSCKART->iLotCount;
    static_assert(sizeof(W5SckArtRem_LotSummary.iCountCategory) == sizeof(LotSummary.iCountCategory), "LotSummary iCountCategory shape");
    static_assert(sizeof(W5SckArtRem_LotSummary.iTotalCategory) == sizeof(LotSummary.iTotalCategory), "LotSummary iTotalCategory shape");
    static_assert(sizeof(W5SckArtRem_LotSummary.bIsRTBin)       == sizeof(LotSummary.bIsRTBin),       "LotSummary bIsRTBin shape");
    std::memcpy(W5SckArtRem_LotSummary.iCountCategory, LotSummary.iCountCategory, sizeof(LotSummary.iCountCategory));   // (2) 轉接
    std::memcpy(W5SckArtRem_LotSummary.iTotalCategory, LotSummary.iTotalCategory, sizeof(LotSummary.iTotalCategory));
    std::memcpy(W5SckArtRem_LotSummary.bIsRTBin,       LotSummary.bIsRTBin,       sizeof(LotSummary.bIsRTBin));
    W5SckArtRem_LotSummary.iLoadTotal = LotSummary.iLoadTotal;
    SckArtRem_SaveTestSummary(st, iSaveData);                                   // golden SCK_ART.cpp:1620-1646（TCP/IP：[ProcessOSPrint]＋SaveTestSummaryTSV；一般機台：iSaveData==1 時 SaveSummaryTrayFeed＋SaveTestSummaryTSV）
    fSCKART->sProcessCode = st.sProcessCode;                                    // golden :2835-2836 在 fSCKART 上補的 "FT1"
    LotSummary.ClearAllData();                                                  // golden SCK_ART.cpp:3127（SaveTestSummaryTSV 最後一行；真的那一份）
    if (TestIF_File.iTestType==TCP_IP_MODE && iSaveData)                        // golden :1632-1635 的 fTesterTCP->ProcessOSPrint() 是替身（SCK_ART_Remainder gate #6）
        W906_SckArtSkip(skipped, "golden SCK_ART.cpp:1634-1635 fTesterTCP->ProcessOSPrint() (Open/Short test report, TCP_IP_MODE tester) is a no-op stand-in in the port (SCK_ART_Remainder.cpp gate #6) -- not written; the TSV summary itself was written");
    if (CosFunction.bUseTSVFunction)
        W906_SckArtSkip(skipped, "golden SCK_ART.cpp:3043-3079/:3118-3126 ATK TSV: summary file written, but the TSV server reply wait (TimerTSV, gate #12) and FTP_Upload (gate #13) are stand-ins -- nothing is sent (S80 customer-specific)");
}
