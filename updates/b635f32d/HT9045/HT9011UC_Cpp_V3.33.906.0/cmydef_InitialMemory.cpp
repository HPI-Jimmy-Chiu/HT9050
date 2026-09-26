//------------------------------------------------------------------------------
// AI(W906-INITMEM) 20260927: golden cmydef.cpp:5786-5872 InitialMemory(), line for line (cp950-decoded), with ONE
//   deviation (asGPIBTempShow, see that line).  The golden copy in cmydef.cpp:6017 stays inside its TODO(W6) gate as
//   the reference text; THIS is the live definition (ht9045_globals, CMakeLists.txt next to cmydef.cpp).
//
//   WHY IT MATTERS: golden runs it first thing in the HSys ctor (database.cpp:47 -- SYSTEM_MODULAR HSys is a file-scope
//   object), before InitCommonString / OpenGeneralIniFile / ReadGeneralIni.  The port gated that ctor as a whole
//   (database.cpp:133-156) and nothing called InitialMemory.  SiteData[] -- the X x Y of every test mode -- was being
//   filled by St01's SeedSiteData() (FileRW/TestIF_File_SetUp.cpp:75, reached at boot via wb_serve.cpp:4158 ->
//   W906_DoReadLastData -> FileRW_Setup_Boot, and only when the recipe path is ready); now it is set earlier and also
//   without a usable recipe path, and SeedSiteData's Cnt!=0 guard makes it a no-op.  The 48 ZeroMemory() targets all
//   have live, zero-initialised definitions in cmydef.cpp, so they are no-ops at boot; lHandlerStopTime is latched at boot as in golden.
//
//   CALLER: tools/wb_serve.cpp, on the line just before LoadMachineConfig() (the port's ReadGeneralIni), i.e. the
//   same order as golden.  It must NOT move after LoadMachineConfig(): ReadGeneralIni fills LOAD_Z_USE_MOTOR[]
//   (database.cpp:880-886) and this would zero it again.
//------------------------------------------------------------------------------
#include "cmydef.h"
#include "myTimer.h"   // TQPF_Timer::LatchCycleTime (lHandlerStopTime, cmydef.h:3618)
#include <windows.h>   // ZeroMemory

void InitialMemory()                                                            //Steven 20160319 : 初始化數值
{
    ZeroMemory(bCleanKitSuckDuplicateErr, sizeof(bCleanKitSuckDuplicateErr));
    for(int i=0; i<tcTotalCount; i++) asGPIBTempShow[i]="";                     //AI(W906-INITMEM) 20260927: golden `ZeroMemory(asGPIBTempShow, sizeof(asGPIBTempShow));` -- a BCB6 AnsiString is one pointer, so zeroing it IS the empty string; vclcompat::AnsiString is a class, so ZeroMemory would corrupt it. Same result, by assignment.
    ZeroMemory(ContinuousFailSKTCount, sizeof(ContinuousFailSKTCount));
    ZeroMemory(ContinuousFailARMCount, sizeof(ContinuousFailARMCount));
    ZeroMemory(ContinuousFailSKTCount_AutoClean, sizeof(ContinuousFailSKTCount_AutoClean));
    ZeroMemory(ContinuousFailARMCount_AutoClean, sizeof(ContinuousFailARMCount_AutoClean));
    ZeroMemory(SpecialBinContinuousFailSKTCount, sizeof(SpecialBinContinuousFailSKTCount));
    ZeroMemory(SpecialBinContinuousFailARMCount, sizeof(SpecialBinContinuousFailARMCount));
    ZeroMemory(iAutoCleanByBinCount, sizeof(iAutoCleanByBinCount));
    ZeroMemory(iAutoCleanBySiteCount, sizeof(iAutoCleanBySiteCount));
    ZeroMemory(iLoadPersentCT, sizeof(iLoadPersentCT));
    ZeroMemory(iLoadCountCT, sizeof(iLoadCountCT));
    ZeroMemory(fTrayYield, sizeof(fTrayYield));
    ZeroMemory(fIntervalYield_YieldHistory, sizeof(fIntervalYield_YieldHistory));
    ZeroMemory(iNeedBarcodeCount, sizeof(iNeedBarcodeCount));
    ZeroMemory(iBarcodeDuplicate, sizeof(iBarcodeDuplicate));
    ZeroMemory(iBarcodeErrorCount, sizeof(iBarcodeErrorCount));
    ZeroMemory(iBarcodePassCount, sizeof(iBarcodePassCount));
    ZeroMemory(iBarcodeAutoRetry, sizeof(iBarcodeAutoRetry));
    ZeroMemory(bBarcodeFirstAutoRetry, sizeof(bBarcodeFirstAutoRetry));
    ZeroMemory(bLowYieldCloseSite, sizeof(bLowYieldCloseSite));
    //ZeroMemory(iBinTray, sizeof(iBinTray));                                   //kevin 20170223 (wei) 不使用
    ZeroMemory(iATC_TempIndex, sizeof(iATC_TempIndex));
    ZeroMemory(bSiteHasTurnOn, sizeof(bSiteHasTurnOn));                         //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
    ZeroMemory(iInArmPutIcToSH, sizeof(iInArmPutIcToSH));                       //Ifor 20171121 : Test 查看異常資料
    ZeroMemory(iMagneticScalePos, sizeof(iMagneticScalePos));                   //Ifor 20180227 : 初始值歸零
    ZeroMemory(dATCTempAdjustmentOffset, sizeof(dATCTempAdjustmentOffset));     //Ifor 20190215 : add ATC 使用 三點校正功能
    ZeroMemory(LOAD_Z_USE_MOTOR, sizeof(LOAD_Z_USE_MOTOR));                     //Steven 20190813 : 入Tray改用步進馬達
    ZeroMemory(LOADUNLOAD_USE_CASSETTE, sizeof(LOADUNLOAD_USE_CASSETTE));

    ZeroMemory(iByBinTotal, sizeof(iByBinTotal));
    ZeroMemory(bUnloadHasBin, sizeof(bUnloadHasBin));
    ZeroMemory(iTrayLastBin, sizeof(iTrayLastBin));
    ZeroMemory(bPickLoaderDuplicateErr, sizeof(bPickLoaderDuplicateErr));
    ZeroMemory(bPickHPDuplicateErr, sizeof(bPickHPDuplicateErr));
    ZeroMemory(bTryPickHPDuplicateErr, sizeof(bTryPickHPDuplicateErr));
    ZeroMemory(dTorqueArray, sizeof(dTorqueArray));

    ZeroMemory(iSLT_HeadContactCount, sizeof(iSLT_HeadContactCount));           //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
//    ZeroMemory(iTrayXAutoSitemapping, sizeof(iTrayXAutoSitemapping));         //Ifor 20210524 add:mykitsuck移至cmydef
//    ZeroMemory(iTrayYAutoSitemapping, sizeof(iTrayYAutoSitemapping));         //Ifor 20210524 add:mykitsuck移至cmydef
    ZeroMemory(dIndexZOffset, sizeof(dIndexZOffset));                           //Ifor 20210114 add: Index Z Offset
    ZeroMemory(bFTestSuckError, sizeof(bFTestSuckError));                       //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    ZeroMemory(bBTestSuckError, sizeof(bBTestSuckError));                       //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    ZeroMemory(bATC_EnablesChannel, sizeof(bATC_EnablesChannel));
//    ZeroMemory(bFIFOStep, sizeof(bFIFOStep));                                 //Steven 20180305 : 一次跑一顆的FIFO版本

//    ZeroMemory(iBufferDataType, sizeof(iBufferDataType));
//    ZeroMemory(asBufferCassetteID, sizeof(asBufferCassetteID));
//    ZeroMemory(asBufferLotID, sizeof(asBufferLotID));
//    ZeroMemory(asViewMessage, sizeof(asViewMessage));

    //JerryYang 20181011 (Steven) : SiteData改成全域變數
    SiteData[SingleSite].SetData(1, 1);
    SiteData[DualSite].SetData(2, 1);
//    SiteData[DualSiteBS].SetData(2, 1);
    SiteData[TriSite1X3].SetData(3, 1);
    SiteData[QualSite1X4].SetData(4, 1);
    SiteData[DualSite2x1].SetData(1, 2);
    SiteData[QualSite2X2].SetData(2, 2);
    SiteData[QualSite2X2N].SetData(2, 2);                                       //Frank 20200520 2X2NN Mode
//    SiteData[QualSite2X2BS].SetData(2, 2);
    SiteData[_6Site2X3].SetData(3, 2);
    SiteData[_6Site2X3N].SetData(3, 2);                                         //Steven 20220425 : 2X3NN Mode
    SiteData[_8Site2X4].SetData(4, 2);
    SiteData[_10Site2X5].SetData(5, 2);
    SiteData[_12Site2X6].SetData(6, 2);
    SiteData[_16Site2X8].SetData(8, 2);
    SiteData[_16Site4X4].SetData(4, 4);                                         //Sam 20190226 : 16Site4X4
    SiteData[_32Site4X8N].SetData(8, 4);
    SiteData[_32Site4X8M].SetData(8, 4);
    SiteData[_8Site1X4].SetData(4, 1);
    SiteData[_8Site2X4N].SetData(4, 2);                                         //Wei 20231211 : 2X4NN Mode
    lHandlerStopTime.LatchCycleTime(true);
    ZeroMemory(iRTC_CCD_NG, sizeof(iRTC_CCD_NG));                               //Ifor 20210203 : add CCD NG Result
    ZeroMemory(bBarcodeNeedAutoAdjust, sizeof(bBarcodeNeedAutoAdjust));         //Ifor 20210531 add: Barcode Auto Adjust Light
    ZeroMemory(iAutoHasHod, sizeof(iAutoHasHod));                               //Ifor 20200803 add:改陣列處理

    ZeroMemory(bIdleNeedCheckSafeDoor, sizeof(bIdleNeedCheckSafeDoor));         //Steven 20230704 : add bypass idle check safe door
    ZeroMemory(iAutoHasHod, sizeof(iAutoHasHod));                               //Ifor 20200803 add:改陣列處理
    ZeroMemory(bNowUseArmSuck, sizeof(bNowUseArmSuck));                         //Ifor 20200803 add:改陣列處理
    ZeroMemory(bATC_EnableSiteMap, sizeof(bATC_EnableSiteMap));                 //Ifor 20241105 add:ATC Enable Site Map
    ZeroMemory(RefrigeratorUserModeState, sizeof(RefrigeratorUserModeState));   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    ZeroMemory(bAlignmentChangeUnloadTray, sizeof(bAlignmentChangeUnloadTray)); //Steven 20240428 : Add for HT9011 AOA
}
