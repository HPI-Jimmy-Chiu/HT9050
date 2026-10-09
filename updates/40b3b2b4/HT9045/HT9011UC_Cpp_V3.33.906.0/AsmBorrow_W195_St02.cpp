//---------------------------------------------------------------------------
//  AsmBorrow_W195_St02.cpp -- golden 913 r913 log "L03 ASM 熱盤借料帳錯亂 (WAR0150)" (RogerYang 20260914, VTEST HHT-83), the pieces in
//  mykitsuck.cpp and uhome.cpp, kept out of those shared files so their line numbers stay put (each host only gets same-line calls).
//  AI(W906-W195) 20261009 (St02-E), laptop card W-205 (W-195 (2) C items).
//
//  L03a -- golden 913 mykitsuck.cpp:1638-1647: TMyKitSuck::CopyToTray (mykitsuck.cpp:1665) reads the Shuttle / Kit / hot-count backup
//          of the nozzle that BORROWED the hot-plate cell (InArmSiteMapData: UpdateData at the borrow, ainarm_SearchPickPlate.cpp;
//          ClearData when the write-back is done, ainarm_SearchPlacePlate.cpp DoPlaceToHPBackupData), not of the global cursor
//          iAutoSiteMapInArmRow/Col that ReStartAutoSiteMapping() resets to (0,0) -- and only when the cell matches.
//          [W906] golden reaches InArmSiteMapData through mykitsuck.cpp:12 #include "ainarm2.h"; its V906 home aHotPlateSubstrate.h
//          cannot be included in mykitsuck.cpp (its global TList shim vs mykitsuck.cpp:200 using vclcompat::TList), so the read lives here.
//  L03b -- golden 913 uhome.cpp:1410-1433: HOME (ProcessMotorHome case 1, uhome.cpp:1117) ends an ASM borrow it interrupted -- frees
//          the HAS_NULL_IC placeholder of the borrowed cell, clears the borrow flags and InArmSiteMapData.  Outside #ifndef SOFT_SIMULTE
//          as golden.  [W906] golden's comment on iResetSiteMappingStep=0 names PurgePausedBookings (913 main.cpp:29258-29275,
//          Public/HTEditList.cpp:2827, RogerYang 20260831f), which V906 does not have yet; here the 0 means "no write-back pending" for
//          DoPlaceToHPBackupData (ainarm_SearchPlacePlate.cpp:4811) and ainarm2.cpp:7064, as in golden.
//  Memory only (no IO, motion, file or alarm).  Customer ASM only: L03a under bRunAutoSiteMapping (CosFunction.bAutoSiteMappingUseHotPlate,
//  ainarm9045.cpp:993-1000), L03b under CosFunction.bUSEJCETSiteMapMode && IniConfig.bI21EnableASM.  HT9050 (CUSTOMER_CODE 957 = CC_PTI,
//  FUNC_CC_PTI CosFunction.cpp:1652) sets neither -> never reached.
//---------------------------------------------------------------------------
#include "MachineDefine.h"
#include "aHotPlateSubstrate.h"        // strAUTOSITEMAP + extern InArmSiteMapData (golden ainarm2.h:10 / :34); _MAX_SUCK_ROW_ITEM / _COL_ITEM
#include "Motor/mymotor.h"             // MOT[] (TTrayMotor: Tray.Data / Tray.SiteMapData / SetTraySingleData)
#include "cmydef.h"                    // NULL_IC / HAS_NULL_IC, MMPlate1, bAutoSiteMapHotplateSave / bAutoSiteMapHasPickHP / iResetSiteMappingStep
#include "Config.h"                    // IniConfig.bI21EnableASM
#include "CosFunction.h"               // CosFunction.bUSEJCETSiteMapMode
#include "canary_support.h"            // RecordProcess(AnsiString, AnsiString="") as uhome.cpp uses it (do not include cMyDB.h)

//---------------------------------------------------------------------------
// L03a: golden 913 mykitsuck.cpp:1640-1647 (inside CopyToTray); iBkR / iBkC come in as golden :1638-1639 (= iSuckR / iSuckC)
void W906_AsmBorrowSuckRC_St02(int TrayR, int TrayC, int &iBkR, int &iBkC)
{
    if(InArmSiteMapData.iSuckR>=0 && InArmSiteMapData.iSuckR<_MAX_SUCK_ROW_ITEM &&   //AI : ReStartAutoSiteMapping() 會把游標無條件歸零成 (0,0) --
       InArmSiteMapData.iSuckC>=0 && InArmSiteMapData.iSuckC<_MAX_SUCK_COL_ITEM &&   //AI : 借料存在[D]、補回取在[A] -> Count/Sht/Kit 全讀到 0 (偉測 HHT-83 三顆孤島)
       InArmSiteMapData.iPlateR==TrayR &&                                   //AI : InArmSiteMapData 在 ainarm_SearchPickPlate 借料時 UpdateData,
       InArmSiteMapData.iPlateC==TrayC)                                     //AI : 在 DoPlaceToHPBackupData 補回完成時 ClearData -- 座標對得上才採用
    {
        iBkR=InArmSiteMapData.iSuckR;
        iBkC=InArmSiteMapData.iSuckC;
    }
}

//---------------------------------------------------------------------------
// L03b: golden 913 uhome.cpp:1410-1433 (ProcessMotorHome case 1, after the ASM_Home log block), verbatim
void W906_AsmHomeReleaseBorrow_St02()
{
    AnsiString Str;
            //AI(ht9045-inarm-flow) 20260914 (RogerYang) : Home 中止 ASM 借還交易的收尾 --
            //AI : “必須放在 #ifndef SOFT_SIMULTE 外面” : 20260914 模擬實證, 放在裡面會被模擬版編掉,
            //AI : 導致 bAutoSiteMapHotplateSave 殘留 -> Home 後仍走 JCET write-back -> 跟沒修一樣。
            //AI : 不收尾的後果 : 該格永久留 HAS_NULL_IC 占位(吃掉一個 2x4 落點) + 旗標殘留做錯的補回。
            if(CosFunction.bUSEJCETSiteMapMode &&
               IniConfig.bI21EnableASM==true &&
               InArmSiteMapData.iP!=-1)
            {
                if(InArmSiteMapData.iP>=0 && InArmSiteMapData.iP<2 &&
                   InArmSiteMapData.iPlateR>=0 && InArmSiteMapData.iPlateC>=0)
                {
                    if(MOT[MMPlate1+InArmSiteMapData.iP].Tray.Data[InArmSiteMapData.iPlateC][InArmSiteMapData.iPlateR]==HAS_NULL_IC)
                    {
                        MOT[MMPlate1+InArmSiteMapData.iP].SetTraySingleData(InArmSiteMapData.iPlateC, InArmSiteMapData.iPlateR, NULL_IC);
                        MOT[MMPlate1+InArmSiteMapData.iP].Tray.SiteMapData[InArmSiteMapData.iPlateC][InArmSiteMapData.iPlateR]=0;
                        Str.sprintf("ASM Home : release borrowed cell P%d R%d C%d", InArmSiteMapData.iP, InArmSiteMapData.iPlateR, InArmSiteMapData.iPlateC);
                        RecordProcess(Str, "ProcessMotorHome");                 //AI : 留痕 -- 有這一行就代表這次 Home 打斷了 ASM 借料
                    }
                }
                bAutoSiteMapHotplateSave=false;                                 //AI : 交易中止, 不可再補回
                bAutoSiteMapHasPickHP  =false;                                  //AI : 否則下一次放料仍走 JCET write-back 分支
                iResetSiteMappingStep  =0;                                      //AI : 補回已取消, 放行後面 ReStartAutoSiteMapping 內的 PurgePausedBookings 去清這筆暫停帳
                InArmSiteMapData.ClearData();                                   //AI : 重入保護(case 1 若中途 return false 會重跑) + 不再重複寫 log
            }
}
//---------------------------------------------------------------------------
