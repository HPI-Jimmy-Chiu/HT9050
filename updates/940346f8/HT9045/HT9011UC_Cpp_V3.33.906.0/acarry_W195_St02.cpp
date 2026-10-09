//---------------------------------------------------------------------------
//  acarry_W195_St02.cpp -- golden 913 acarry.cpp pieces that are new since 912, kept out of the shared acarry.cpp so its line
//  numbers stay put (acarry.cpp only gets same-line calls).  AI(W906-W195) 20261009 (St02-E), laptop card W-205 (W-195 (2) C items).
//
//  L05 -- the Shuttle Move Timeout interlock snapshot (golden 913 acarry.cpp:3458-3460 externs + :3484-3515, RogerYang 20260915
//         "ht9045-shuttle-flow", SCK HT9046LS 20260907 case).  Called from DoShtMoveTimeoutHandle (acarry.cpp:3577) after the latch
//         dump and BEFORE the operator message, as golden ("必須在 ShowMyMessage 之前, 按 OK 後旗標就被清掉了").  Read-only: one
//         EventLog row; no motion, no IO write, no alarm.  No customer or machine flag; HT9050 (Type_HT9050) runs Do_Auto_InSH /
//         Do_Auto_OutSH (csystem.cpp:1916), never Do_Auto_SHT1/2, so it never reaches DoShtMoveTimeoutHandle.
//         [W906] IndexZCanMove is {false,false} in V906 (ainarm9045_w7_shims.cpp:57, GATE W4G-1), so the row shows IdxZCanMove=0/0
//         where golden shows 1/1 -- the V906 value, accepted (W-205).
//  L10 -- [C25] floodgate (golden 913 acarry.cpp:8800-8810, JerryYang 20261006): DoFloodGateCloseAtShuttleLeft closes the Out-Shuttle
//         floodgate of a shuttle parked at Left, under the same rule as the 8 in-line close points (DoFloodGateClose, acarry.cpp:8644,
//         913 rule "C25 on and ATC active cooling").  Called at HOME done / Tray Feed done (csystem.cpp:7450 / :7743, golden 913
//         csystem.cpp:10986 / :11326).  Only SHUTTLE_FLOODGATE==1 machines; HT9050 has 0 -> returns at once.
//---------------------------------------------------------------------------
#include "MachineDefine.h"
#include "acarry.h"
#include "acarry_shims.h"          // the 2-arg MyDBIProcess adapter acarry.cpp binds to (do not include cMyDB.h)
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cmydef.h"
#include "atester.h"               // iTestYTask (and iTestTask / iTestHeadMotorTask)
#include "canary_support.h"
#include "mycylin.h"               // L10: Cylinder[] (C_OutShuttle1Floodgate / 2)

extern bool IndexZCanMove[2];                                                   //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : Shuttle Move Timeout 互鎖診斷用 (ainarm2.h)   golden 913 acarry.cpp:3458; V906 ainarm9045_w7_shims.cpp:57
extern int  iTestTask;                                                          //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : 同上   golden 913 :3459 (also atester.h)
extern int  iTestHeadMotorTask;                                                 //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : 同上   golden 913 :3460 (also atester.h)
extern int  iPickFromShuttle1Task, iPickFromShuttle2Task;                       // [W906] golden reaches them through aArmHeader.h -> aoutarm.h:109-110 (V906 aoutarm.cpp:609-610)

//---------------------------------------------------------------------------
// golden 913 acarry.cpp:3484-3515 (inside DoShtMoveTimeoutHandle), verbatim; iMot as golden :3470
void W906_ShtMoveTimeoutDiag_St02(int iSelSHT)
{
    int    iMot     = (iSelSHT==0) ? MInShuttle1 : MInShuttle2;                 //達索引   (golden 913 :3470)

    //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : SCK HT9046LS 20260907 19:36 Shuttle2 Move Timeout 互鎖快照
    //   Enc 停在右位完全沒動 + 兩次 HOME 都清不掉 fRearNeedSuck, 需要 fCanMove 系列與 Index 旗標才能釘根因
    //   唯讀 dump, 只在 timeout 觸發時跑一次 (頻率極低); 必須在 ShowMyMessage 之前, 按 OK 後旗標就被清掉了
    AnsiString sDiag;
    int iY1C=0, iY1E=0, iY2C=0, iY2E=0, iZ1C=0, iZ1E=0, iZ2C=0, iZ2E=0;
    if(MOT[MTestY1].Motor!=NULL && MOT[MTestZ1].Motor!=NULL && MOT[MTestZ2].Motor!=NULL)   //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : Gali_ReadEncoderPos 無 NULL 保護
    {
        iY1C=MOT[MTestY1].Gali_ReadPos();
        iY1E=MOT[MTestY1].Gali_ReadEncoderPos();
        iZ1C=MOT[MTestZ1].Gali_ReadPos();
        iZ1E=MOT[MTestZ1].Gali_ReadEncoderPos();
        iZ2C=MOT[MTestZ2].Gali_ReadPos();
        iZ2E=MOT[MTestZ2].Gali_ReadEncoderPos();
        if(USE_INDEX_ARM_AXES!=IndexArm_3_Axis && MOT[MTestY2].Motor!=NULL)
        {
            iY2C=MOT[MTestY2].Gali_ReadPos();
            iY2E=MOT[MTestY2].Gali_ReadEncoderPos();
        }
    }
    sDiag.sprintf("ShtMoveTimeout Diag SHT%d : CanMove=%d L=%d R=%d M=%d | IdxZCanMove=%d/%d | "
                  "FrontNeedTest=%d FrontNeedSuck=%d FrontNeedSuckIC=%d RearNeedSuck=%d RearNeedSuckIC=%d RearNeedDestroy=%d | "
                  "TestTask=%d HeadMotTask=%d TestYTask=%d SHT1Task=%d SHT2Task=%d PickSht1=%d PickSht2=%d | "
                  "Y1 %d/%d Y2 %d/%d Z1 %d/%d Z2 %d/%d (cmd/enc)",
                  iSelSHT+1,
                  (int)MOT[iMot].fCanMove, (int)MOT[iMot].fCanMoveL, (int)MOT[iMot].fCanMoveR, (int)MOT[iMot].fCanMoveM,
                  (int)IndexZCanMove[0], (int)IndexZCanMove[1],
                  (int)fFrontNeedTest, (int)fFrontNeedSuck, (int)fFrontNeedSuckIC,
                  (int)fRearNeedSuck,  (int)fRearNeedSuckIC, (int)fRearNeedDestroy,
                  iTestTask, iTestHeadMotorTask, iTestYTask,
                  AutoSHT1Task, AutoSHT2Task, iPickFromShuttle1Task, iPickFromShuttle2Task,
                  iY1C, iY1E, iY2C, iY2E, iZ1C, iZ1E, iZ2C, iZ2E);
    MyDBIProcess("Message", sDiag);                                             //AI(ht9045-shuttle-flow) 20260915 (RogerYang) : 互鎖快照寫入 EventLog
}
//---------------------------------------------------------------------------
// L10: golden 913 acarry.cpp:8800-8810 (after DoFloodGateClose), verbatim
void DoFloodGateCloseAtShuttleLeft()                                            //JerryYang 20261006 : add close Out Shuttle Floodgate while shuttle parked at Left
{
    if(DoFloodGateClose()==false)                                               //Same policy as the in-line Floodgate close points
        return;

    if(MOT[MInShuttle1].CompareCommandPos(Prod.InSHT[0].iLeft, 2)==1)           //JerryYang 20261006 : Shuttle1 stays at Left
        Cylinder[C_OutShuttle1Floodgate].On();

    if(MOT[MInShuttle2].CompareCommandPos(Prod.InSHT[1].iLeft, 2)==1)           //JerryYang 20261006 : Shuttle2 stays at Left
        Cylinder[C_OutShuttle2Floodgate].On();
}
//---------------------------------------------------------------------------
