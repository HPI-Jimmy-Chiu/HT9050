// =============================================================================
//  ainarm_SearchPickPlate.h  --  shared InArm HotPlate-PICK geometry prototypes
//
//  Mirrored VERBATIM from: HT9011UC_Code_V3.33.906.0_20260618/ainarm_SearchPickPlate.h
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP-pick geometry leaf)
//  Translator: AI(W6.2-INARM) 20260626
//
//  This is the shared HP-pick sub-task machine (DoInArmPickFromHotPlate_9045 +
//  its geometry leaves) that the ~30 in-arm site-variants call.  Header mirrored
//  verbatim; the iInArmType dispatch stays in the (later) ainarm core.
//  cmydef.h pulls X_PITCH_COUNT (MachineType.h) + the global type universe.
//  Big5: Chinese comments preserved as UTF-8 (cp950).  NO U+FFFD.
// =============================================================================
//---------------------------------------------------------------------------
#ifndef ainarm_SearchPickPlateH
#define ainarm_SearchPickPlateH

#include "cmydef.h"
//---------------------------------------------------------------------------
extern bool ArmRow1AllHasIC();
extern int  GetInArmToHPYPitch_9045();
extern bool DoInArmPickFromHotPlate_9045();

extern bool bPickFormHotplatePartOK;                                            //ChungHung 2012012 add
extern bool bInArmToPickHotPlatePos;                                            //ChungHung 20110323
extern bool bPickFormHotplateRetry;                                             //Ifor 20160616 add Hot plate Pickup error Retry
extern bool bInArmTryPickFromHotPlateFinish;                                    //ChungHung 20120206 Hotplate check

extern int  GetHeaterWaitTime();
extern bool HasHotReadyIC_9045();
extern bool InArmNeedCheckHotPlateOffset(bool bPlace);                          //Steven 20230531 : 簡化判斷式

extern bool MoveInArmXYPickHotPlate_9045(bool bNeedDown);

extern void TryHotClearRecBufMMPlate();                                         //ChungHung 20120206 Hotplate check
extern void BackupHotplatelocation();                                           //Ifor 20171006 (Steven) : add 備份Hotplate取料位置
extern void HotplateDataConversion(int iSuckR, int iSuckC, int iP, int iPlateR, int iPlateC, bool *flage1);             //Ifor 20180518 : add 整合Hot plate 資料轉換
extern bool IsFLCarrKitAllHasIC();                                              //ChungHung 20111230 add
extern bool IsBLCarrKitAllHasIC();                                              //ChungHung 20111230 add
#endif
