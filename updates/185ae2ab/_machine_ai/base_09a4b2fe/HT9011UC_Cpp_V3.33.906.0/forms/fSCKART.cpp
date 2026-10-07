// =============================================================================
//  forms/fSCKART.cpp  --  definitions for the fSCKART facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Bodies moved VERBATIM.
// =============================================================================
#include "forms/fSCKART.h"
// AI(W906-W7-L1-Wave0) 20260801: cmydef.h pulls in CUSTOMER_CODE (cmydef.h:3181,
// `extern int`) and MachineType.h's CC_SCK, both needed by the golden-faithful
// iTesterType initialisation below.  Same cross-library shape forms/fMain.cpp
// already uses: ht9045_forms PUBLIC-links ht9045_globals (root CMakeLists.txt),
// which is where CUSTOMER_CODE is defined (cmydef.cpp:3412).
#include "cmydef.h"
// AI(W906-PT-W3-integrate) 20260808: three more real-global reaches for the
// ClearLotInfo faithful subset below -- LastSet (LAST_GENERAL_SET,
// ht9045_globals), LotSummary (TLotSummary, cSocket.cpp), TestIF_File
// (SYSTEM_TEST_IF, cprod.cpp).  Same cross-library shape as cmydef.h above.
#include "LastSet.h"
#include "cSocket.h"
#include "cprod.h"

// --- W6.2: TfSCKART --------------------------------------------------------
TfSCKART::TfSCKART() : iInputJamCnt(0), iFTRTCount(0), iInputCount(0), iCurrent93KARTStep(0)
{
    // -- W5-Automation ADD --
    palLotNumber   = new TfSortCTPanel();
    palTestCnt     = new TfSortCTPanel();
    palRTTryCnt    = new TfSortCTPanel();
    pnlProcessCode = new TfSortCTPanel();
    edlRTTryCnt    = new TfLotInfoEdit();
    // -- AI(W906-W7-L1-Wave0) 20260801 ADD: the 7 W7-L1 golden fields + 2 seams --
    iBundleInCnt   = 0;                 // golden SCK_ART.h:343 (golden TForm int member, zero-init)
    iBundleOutCnt  = 0;                 // golden SCK_ART.h:344 (ditto)
    sBundleList    = "";                // golden SCK_ART.h:342 (golden AnsiString member, empty)
    bFirstFullSkip = false;             // golden SCK_ART.h:351 (golden TForm bool member, zero-init)
    iNeedRT        = 0;                 // golden SCK_ART.h:285 (golden TForm int member, zero-init)
    iLOTSTATUS_W   = 1;                 // golden SCK_ART.cpp:44 -- explicit `iLOTSTATUS_W=1;`
    // iTesterType: golden's value is produced by TWO ctor regions, not one --
    // SCK_ART.cpp:42 `iTesterType=0;` and then :102-114
    // `if(CUSTOMER_CODE==CC_SCK){...widgets...} else { iTesterType=1; }`.
    // Reproduced verbatim as the conditional below so the value is right for
    // whatever CUSTOMER_CODE is in force when a TfSCKART is constructed.
    //
    // STATIC-INITIALISATION CAVEAT, stated so nobody over-reads the expression:
    // the GLOBAL `fSCKART` below is constructed during dynamic initialisation,
    // and CUSTOMER_CODE (cmydef.cpp:3412 `int CUSTOMER_CODE=0;`) is
    // constant-initialised, so it is guaranteed to read 0 -- never CC_SCK (947) --
    // at that moment.  The global therefore always ends up with 1, which IS
    // golden's constructed value for every customer except CC_SCK.  The
    // conditional still earns its place: a test that sets CUSTOMER_CODE and then
    // constructs its own TfSCKART gets golden's real behaviour on both arms.
    iTesterType    = (CUSTOMER_CODE==CC_SCK) ? 0 : 1;   // golden SCK_ART.cpp:42 + :102-114
    W906_SetLotStatus_LastArg = -1;     // PORT-ONLY SEAM: -1 == never called
    W906_SetLotStatus_Count   = 0;      // PORT-ONLY SEAM
    // -- AI(W906-PT-W3-integrate) 20260808 ADD: the 8 uRENESAS_Server fields --
    iLotCount           = 0;            // golden SCK_ART.h:246 (VCL zero-init; no golden ctor assignment)
    iManualStart        = 0;            // golden SCK_ART.h:349 (ditto)
    sLotStartTime       = "";           // golden SCK_ART.cpp:41 -- explicit `sLotStartTime="";`
    iCurrentFlexARTStep = 0;            // golden SCK_ART.h:264 (VCL zero-init)
    iWaitGPIBLotR       = 0;            // golden SCK_ART.cpp:53 -- explicit `iWaitGPIBLotR=0;`
    iOutputJamCnt       = 0;            // golden SCK_ART.cpp:55 -- explicit `iOutputJamCnt=0;`
    bLdCntExdInputCnt   = false;        // golden SCK_ART.h:350 (VCL zero-init)
    iLOTSTATUS_NONE     = 0;            // golden SCK_ART.cpp:43 -- explicit `iLOTSTATUS_NONE=0;`
}
int  TfSCKART::CheckLoadingCount() { return 0; }              // W7: offline -> 0 (no ART loading mismatch)
void TfSCKART::AddOutputJamCnt(int /*row*/, int /*col*/, int /*ret*/, int /*iBinOnCarryKit*/) {}  // W6.5: offline no-op
// -- W5-Automation ADD: AMR.cpp + HANA_ART.cpp method sinks (all offline no-op) --
void TfSCKART::DoARTLotStart(AnsiString /*_sLotID*/, AnsiString /*_sProcessCode*/, int /*_iLotCount*/) {}
// TfSCKART::AccessFile: real since AI(W906-W1) 20260927, body at the end of this file (kept there so no line above moves)
// -- AI(W906-W7-L1-Wave0) 20260801 ADD: SetLotStatus (golden SCK_ART.h:269, body
//    SCK_ART.cpp:639-667).  Offline this records into the two seams instead of
//    being an unobservable sink (facade contract rule 4).  golden's sLOTSTATUS /
//    iCurrentStatus / cbLotStatus->ItemIndex writes have no facade home and no
//    reader in this tree -- see forms/fSCKART.h for that documented gap.
void TfSCKART::SetLotStatus(int iStatus)
{
    W906_SetLotStatus_LastArg = iStatus;
    W906_SetLotStatus_Count++;
}
// -- AI(W906-PT-W3-integrate) 20260808 ADD: ClearLotInfo (golden SCK_ART.cpp:
//    837-923), FAITHFUL SUBSET per the header note: every golden assignment
//    whose target exists on this facade or on the real LastSet/LotSummary
//    globals is performed IN GOLDEN ORDER; each skipped golden statement is
//    recorded inline at the exact spot it would run.
void TfSCKART::ClearLotInfo()
{
    iNeedRT         =1;                                     // golden :839
    iFTRTCount      =0;                                     // golden :840
    // SKIPPED golden :841 `iManualRejectCnt=0;` -- no facade home (SckArtState-only field)
    if(TestIF_File.bRENESAS_EnableFTCT==true)               // golden :843 (RogerYang 20250911 FT-CT)
    {
    }
    else
    {
        iLotCount       =0;                                 // golden :848
        iInputCount     =0;                                 // golden :849
        sLotID          ="";                                // golden :850
        sProcessCode    ="";                                // golden :851
        sLotStartTime   ="";                                // golden :852
        fSCKART->SetLotStatus(iLOTSTATUS_NONE);             // golden :853 (records into the W906_ seams)
    }
    LastSet.iSCKART_RTUnitCount    =0;                      // golden :855
    iInputJamCnt    =0;                                     // golden :856
    iOutputJamCnt   =0;                                     // golden :857
    for(int i=0; i<10; i++)
    {
        LastSet.lSCKARTBinCT[i]=0;                          // golden :861
    }
    LastSet.iSCKARTInputCT=0;                               // golden :863
    LastSet.lShuttleCount=0;                                // golden :864
    LastSet.iHdPass=0;                                      // golden :865-873 (RogerYang 20250923 FT-CT block)
    LastSet.iHdFail=0;
    LastSet.iHdRejectATPass=0;
    LastSet.iHdRejectATFail=0;
    LastSet.iHdRejectBT=0;
    LastSet.iTesterMatch=0;
    LastSet.iTesterUnmatch=0;
    LastSet.iTestIgnore=0;
    LastSet.iHdPickUp=0;
    LotSummary.ClearAllData();                              // golden :875
    // SKIPPED golden :877 `DoAutoSocketOff(true);` -- method has no facade home
    // SKIPPED golden :879-896 sInfo_* clears -- the sInfo_ family has no facade home
    sBundleList="";                                         // golden :898
    for(int i=0; i<5; i++)
    {
        // SKIPPED golden :902-917 sInfoArr_* clears -- no facade home
        LotSummary.iByLotLoadCount[i]=0;                    // golden :918
    }
    fSCKART->AccessFile(false);                             // golden :921 (facade AccessFile is a recorded no-op)
    // SKIPPED golden :922 `bWaitTSV=false;` -- no facade home
}
// -- AI(W906-PT-W3-integrate) 20260808 ADD: CheckNeedRT (golden SCK_ART.cpp:
//    1072-1230) -- offline no-op; iNeedRT deliberately left UNCHANGED (header
//    note explains why the RT-decision engine is not ported piecemeal here).
void TfSCKART::CheckNeedRT() {}
TfSCKART *fSCKART = new TfSCKART();
// AI(W906-W1) 20260927 (St02): Steven W1 = (b). golden TfSCKART::AccessFile (906_0625_Steven Automation/SCK_ART.cpp:190-209)
//   plus SetSetupFilePath (:183-188). Only the lot-status / setting group (iAccess -1/0/10, :200), and in it only the key this
//   facade holds: [AutoRetest] iTesterType in the active recipe's Tester.Data, plus CosFunction.bAutoRetestGPIBmode (:205-208).
//   Not ported: LOTSTATUS / iCurrentStatus (:202-203) have no facade member, and writing golden's defaults would erase the
//   recipe's values; the count group (iAccess 1, :211-) neither.  Both stay as before.
//   DEVIATION (Steven W1): a missing key reads 1 = 93K; golden reads 0 = Flex (:204).
//   RUNTIME WRITES, stated plainly: golden's read writes the default back (CheckAndReadIniData), so the boot / recipe-change
//   read (tools/wb_serve.cpp W906_DoReadLastData -> W906_RC_SCKARTAccessFileRead; RunStartMode.cpp:884) WRITES iTesterType=1
//   into a recipe's Tester.Data that lacks the key, and TESTEND / LOTSTATUS / SRQMASK (TesterComm/Handler/HandlerGpibMsg.cpp
//   :369 / :420 / :433, and :325) write the learned brand back.  Nothing happens while CosFunction.bUseSCKART is false (:195).
//   Paths: GetRecipePath / GetRecipeFileName on DataPath (common.cpp; ctest redirects DataPath with W906_INIDATA_ROOT, 7df047cf).
//   ctest: tests/test_w1_art_brand.cpp (TesterComm_W1ArtBrand).
#include "common.h"
#include "CosFunction.h"
void TfSCKART::AccessFile(bool bRead, int iAccess)
{
    if(CosFunction.bUseSCKART==false)                                           // golden :195
        return;
    AnsiString sSetupFilePath=GetRecipePath();                                  // golden :185-187 SetSetupFilePath()
    MyForceDirectories(sSetupFilePath);
    sSetupFilePath=GetRecipeFileName("Tester.Data");
    if(iAccess==-1 || iAccess==0 || iAccess==10)                                // golden :200
    {
        iTesterType=ReadWriteIni(sSetupFilePath, "AutoRetest", "iTesterType", iTesterType, 1, bRead);  // golden :204 (default 0 -> 1, Steven W1)
        if(iTesterType==1)                                                      // golden :205-208
            CosFunction.bAutoRetestGPIBmode=true;
        else
            CosFunction.bAutoRetestGPIBmode=false;
    }
}
