//---------------------------------------------------------------------------
//  Automation/AtkAmr_St02.cpp -- the ATK (Amkor Korea) AMR CEID 288 / CEID 8 pieces of golden 913 (= 912; RogerYang
//  AI(ht9045-atk-amr-flow) 20260820-0825, 0618 -> 912).  AI(W906-W195) 20261009 (St02-E), laptop card W-195 (3) "ATK"
//  (MR-ATK-1 + 1b; census C:\AI_TempFile\st02e-scratch\w195\atk\atk_census.md).  依 W-195 卡，S25 不適用（Steven 1009 13:2x）.
//
//  0618 (= V906 before this card): at Final Lot End TfMain::SetLotState(10) sent ONE CEID 8 (DoLotEnd) for an ATK AMR lot and set
//  fAGV->bATK_AMR_DoLotEndSent.  912/913: SetLotState(10) sends CEID 288 (MaximumOutputPortReport) once per Auto1..Auto3 so the
//  host can call the AMR early ("客戶變更-提前通知改發CEID288呼叫AMR取貨"), and the single CEID 8 moves to the last point of the
//  lot -- csystem.cpp DoTrayFeed (a lot without sorting) or the MainProc sorting restore -- marked "Final Track-Out" (SV 38316);
//  TfLotInfo::SetLotEnd skips it when already sent and resets the flag (golden uLotInfo.cpp:2077-2083).
//  Reached only by ATK (fAGV->IsATK_AMR(): CC_AMKOR_Korea + USE_COVER_TRAYID==tCID_NFC + [A65] BundleIDList) with SECS on;
//  HT9050 (957, USE_COVER_TRAYID=0) never.  No file, alarm or motion; host-visible SECS only (EventReport is still the sim
//  counter SECSGEM/SecsEventReport.cpp in V906, the SV / EC registry is live).
//
//  [W906] deviations: the two bodies are free functions called from the golden places (forms/fMain_SetLotState.cpp:126,
//  csystem.cpp:12077 / :32160), so the laptop's files change on the same lines only; W906_AtkCeid288Hook is a ctest seam.
//---------------------------------------------------------------------------
#include "Automation/AtkAmr_St02.h"

#include "Automation/AGV_PortScan.h"   // sOutputBinCode (golden AGV.h:241)
#include "forms/fAGV.h"                // fAGV->bATK_AMR_DoLotEndSent
#include "cmydef.h"                    // iThisPortNo / sUnloadBundleID / asBundleTrayID / sSVBinAssign
#include "cprod.h"                     // Prod.iTrayType / RunInfo
#include "LastSet.h"                   // LastSet.iUnloaderTrayCount_ART / BinCT / iLoaderTotalTray
#include "MachineType.h"               // eAuto1..eAuto3 / ePortAuto1 / e3Auto1 / tNotUse
#include "SECSGEM/SecsEventReport.h"   // EventReport
#include "SECSGEM/SecsEventType.h"     // SECS_EVENT

void (*W906_AtkCeid288Hook)(int iPortNo, const char* sBundleID, const char* sBinCode, int iTrays, int iUnits) = 0;

//---------------------------------------------------------------------------
// golden 913 main.cpp:15925-15953 (golden 912 :15781-15809), verbatim but for the [W906] hook line
void W906_AtkFinalLotEndCeid288()
{
    for(int i=eAuto1; i<=eAuto3; i++)                                    //AI(ht9045-atk-amr-flow) 20260821 (RogerYang) : 288為單軌結構, Auto1~3連發三筆; 空軌照發空值(客戶指示不在機台端擋)
    {
        iThisPortNo    =i-eAuto1+1;                                      //AI(ht9045-atk-amr-flow) 20260824 (RogerYang) : 38300 客戶定案CEID288用出料埠編號(Auto1~3=1~3, Fix1~3=4~6); 勿改用eAGVPort那套(CEID284專用)

        if(Prod.iTrayType[i]==tNotUse)                                   //AI(ht9045-atk-amr-flow) 20260823c (RogerYang) : 未使用軌四欄一起發空值--TrayID與兩個計數存在LastSet, 換工作檔不會歸零
        {
            sUnloadBundleID  ="";
            sOutputBinCode   ="";
            iATKPortTrayCount=0;
            iATKPortUnitCount=0;
        }
        else
        {
            sUnloadBundleID=asBundleTrayID[i+ePortAuto1];                //38217 與CEID8的38205~38207同源, 保證兩事件TrayID一致

            int iLen=sSVBinAssign[i].Length();                           //38314(ATK挪用) 當下軌Bin設定, 去尾逗號: 比照asendic_Auto.cpp滿盤路徑
            if(iLen>0 && sSVBinAssign[i].SubString(iLen, 1)==",")
                sOutputBinCode=sSVBinAssign[i].SubString(1, iLen-1);
            else
                sOutputBinCode=sSVBinAssign[i];

            iATKPortTrayCount=LastSet.iUnloaderTrayCount_ART[i];         //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : 37007 該軌總Tray數, 與CEID8的37003~37005同源
            iATKPortUnitCount=LastSet.BinCT[0][e3Auto1+i];               //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : 1102 該軌總Unit數, 與CEID8的1103~1105同源
        }

        if(W906_AtkCeid288Hook)                                          // [W906] ctest seam
            W906_AtkCeid288Hook(iThisPortNo, sUnloadBundleID.c_str(), sOutputBinCode.c_str(), iATKPortTrayCount, iATKPortUnitCount);
        EventReport(SECS_EVENT.MaximumOutputPortReport);                 //CEID 288
    }
    iATKPortTrayCount=LastSet.iLoaderTotalTray;                          //AI(ht9045-atk-amr-flow) 20260823b (RogerYang) : 288送完還原成原本語義, 否則緊接的CEID8/S1F3會拿到最後一軌的殘值
    iATKPortUnitCount=RunInfo.iUnloadCount;
}

//---------------------------------------------------------------------------
// golden 913 csystem.cpp:8133-8135 (DoTrayFeed) = :19315-19317 (MainProc), the same three statements
void W906_AtkFinalTrackOutCeid8()
{
    sTrackOutType_ATK="Final Track-Out";                                 //AI(ht9045-atk-amr-flow) 20260820 (RogerYang) : CEID8 SV38316 final標記
    EventReport(SECS_EVENT.DoLotEnd);                                    //Send CEID 8 with correct data
    fAGV->bATK_AMR_DoLotEndSent=true;                                    //AI(ht9045-atk-amr-flow) 20260821 (RogerYang) : final CEID8已送, 壓掉uLotInfo重複送
}
