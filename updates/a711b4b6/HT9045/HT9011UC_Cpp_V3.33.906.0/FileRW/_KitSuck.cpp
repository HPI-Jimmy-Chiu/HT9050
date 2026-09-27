// ===========================================================================
//  FileRW/_KitSuck.cpp -- C 路產生檔要用、但不能直接 include 的移植樹狀態（Steven 20260924）。
//
//  aHotPlateSubstrate.h／mykitsuck.h 與 Public/HTEditList.h 各自定義了 TList、uPlateInfo（同名不同物），
//  同一個 TU 不能兩個都 include。gen_editlist.py 產生的 FileRW/*.gen.inc 一定帶 HTEditList.h，所以 golden
//  讀 InArmSuck／OutArmSuck 的敘述經這裡的函式轉接（本檔不 include HTEditList.h）。
// ===========================================================================
#include "aHotPlateSubstrate.h"   // TMyKitSuck InArmSuck／OutArmSuck（golden MyKitSuck.h:357）
#include "cprod.h"                // AI(W906-FRW-S100) 20260926: DummyVacuum（FileRW_SetDummyVacuumToSucks）
#include "LastSet.h"              // AI(W906-FRW-S100) 20260926: LastSet.i*VacuumDummy*Time（同上）
#include "MachineType.h"          // AI(W906-FRW-S100) 20260926: MAX_Index_Row（NEW_MAX_Index_Col 在 cmydef.h，經 aHotPlateSubstrate.h）

// golden：InArmSuck.HasIC() || OutArmSuck.HasIC()
bool FileRW_InOutArmSuckHasIC()
{
    return InArmSuck.HasIC() || OutArmSuck.HasIC();
}

// golden：OutArmSuck.HasIC()（TfBinSel::ShowChangeBinMessage cBinSel.cpp:1646、FormShow :1739）
// Steven 20260925（S12-C BinSelect）：FileRW/BinSelect.gen.inc 用
bool FileRW_OutArmSuckHasIC()
{
    return OutArmSuck.HasIC();
}

// golden：InArmSuck／OutArmSuck 的 iMotRow／iMotCol（TfOffSet ShowOneByOneOffSet :904-906、DoIniDataToForm :2364-2417、
// SaveSetupFile :1534-1536／:1732-1734）。which：0＝InArmSuck，1＝OutArmSuck。
// Steven 團隊 20260925（S12-C Offset_File）：FileRW/Offset_File.gen.inc 用（巨集 InArmSuck／OutArmSuck → OS_SuckOf）
void FileRW_ArmSuckDims(int which, int* row, int* col)
{
    const TMyKitSuck& s = which == 0 ? InArmSuck : OutArmSuck;
    *row = s.iMotRow;
    *col = s.iMotCol;
}

// golden：InArmSuck.HasIC()／OutArmSuck.HasIC()（TfOffSet FormShow :752，CC_ASE_CL）
bool FileRW_ArmSuckHasIC(int which)
{
    return which == 0 ? InArmSuck.HasIC() : OutArmSuck.HasIC();
}

// golden：TestSocket／FTestSuck 的 iShtRow／iShtCol／iMaxRow／iMaxCol（TfStartCondition FormShow :496-512／:595-605、
// FormClose :633-639、ReadWriteStartCondition（經 BuildHeadRowMap :56-66）、sbHeadCondition1SaveClick :1106-1140、
// DoIniDataToForm :1181-1196、sbHeadCondition1ClearClick :807-838）。which：0＝TestSocket，1＝FTestSuck。
// Steven 團隊 20260925（S12-C StartCondition）：FileRW/StartCondition.gen.inc 用（巨集 TestSocket／FTestSuck → SC_KitOf）
void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol)
{
    const TMyKitSuck& s = which == 0 ? TestSocket : FTestSuck;
    *shtRow = s.iShtRow;
    *shtCol = s.iShtCol;
    *maxRow = s.iMaxRow;
    *maxCol = s.iMaxCol;
}

// AI(W906-FRW-S100) 20260926: golden TfMain::SetToDefineValue1Click（V912 main.cpp:26672-26750）的 :26680-26749 ——
//   GetDummyVacuum() 之後，把 DummyVacuum 的四個時間設進各吸嘴、再抄進 LastSet（逐行照 golden；golden 是 TfMain 成員，這裡是自由函式）。
//   呼叫端 FileRW/MainClick.cpp W906_Main_SetToDefineValueOp（守衛、確認框、GetDummyVacuum 在那裡）。放在本檔的理由：
//   InArmSuck 等 TMyKitSuck 在 aHotPlateSubstrate.h，MainClick.cpp 帶 cMyDB.h，兩個同 TU 會讓 MyDBIProcess 兩參數呼叫模稜兩可。
//   ⚠ 只改記憶體（golden 本段不寫檔）。迴圈上限照 golden：InArmSuck.iMaxRow／iMaxCol、MAX_Index_Row／NEW_MAX_Index_Col ——
//     LastSet 的陣列是 [4][8]，golden 也是這個上限（mykitsuck.h _MAX_SUCK_ROW_ITEM 4／_MAX_SUCK_COL_ITEM 8），沒有另外檢查，照翻。
void FileRW_SetDummyVacuumToSucks()
{
    int iTime=0;
    for(int i=0; i<2; i++)
    {
        iTime=DummyVacuum.iArmVacuumOn;
        InArmSuck.Suck[0+i][0].VacuumOnTime=iTime;
        InArmSuck.Suck[0+i][1].VacuumOnTime=iTime;
        InArmSuck.Suck[0+i][2].VacuumOnTime=iTime;
        InArmSuck.Suck[0+i][3].VacuumOnTime=iTime;

        OutArmSuck.Suck[0+i][0].VacuumOnTime=iTime;
        OutArmSuck.Suck[0+i][1].VacuumOnTime=iTime;
        OutArmSuck.Suck[0+i][2].VacuumOnTime=iTime;
        OutArmSuck.Suck[0+i][3].VacuumOnTime=iTime;

        iTime=DummyVacuum.iArmVacuumOff;
        InArmSuck.Suck[0+i][0].VacuumOffTime=iTime;
        InArmSuck.Suck[0+i][1].VacuumOffTime=iTime;
        InArmSuck.Suck[0+i][2].VacuumOffTime=iTime;
        InArmSuck.Suck[0+i][3].VacuumOffTime=iTime;

        OutArmSuck.Suck[0+i][0].VacuumOffTime=iTime;
        OutArmSuck.Suck[0+i][1].VacuumOffTime=iTime;
        OutArmSuck.Suck[0+i][2].VacuumOffTime=iTime;
        OutArmSuck.Suck[0+i][3].VacuumOffTime=iTime;

        iTime=DummyVacuum.iIndexVacuumOn;
        FTestSuck.Suck[0+i][0].VacuumOnTime=iTime;
        FTestSuck.Suck[0+i][1].VacuumOnTime=iTime;
        FTestSuck.Suck[0+i][2].VacuumOnTime=iTime;
        FTestSuck.Suck[0+i][3].VacuumOnTime=iTime;
        BTestSuck.Suck[0+i][0].VacuumOnTime=iTime;
        BTestSuck.Suck[0+i][1].VacuumOnTime=iTime;
        BTestSuck.Suck[0+i][2].VacuumOnTime=iTime;
        BTestSuck.Suck[0+i][3].VacuumOnTime=iTime;

        iTime=DummyVacuum.iIndexVacuumOff;
        FTestSuck.Suck[0+i][0].VacuumOffTime=iTime;
        FTestSuck.Suck[0+i][1].VacuumOffTime=iTime;
        FTestSuck.Suck[0+i][2].VacuumOffTime=iTime;
        FTestSuck.Suck[0+i][3].VacuumOffTime=iTime;
        BTestSuck.Suck[0+i][0].VacuumOffTime=iTime;
        BTestSuck.Suck[0+i][1].VacuumOffTime=iTime;
        BTestSuck.Suck[0+i][2].VacuumOffTime=iTime;
        BTestSuck.Suck[0+i][3].VacuumOffTime=iTime;
    }
    CatchTraySuck.Suck[0][0].VacuumOnTime=650;
    CatchTraySuck.Suck[0][0].VacuumOffTime=31;

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            LastSet.iInArmVacuumDummyOnTime[i][j]=InArmSuck.Suck[i][j].VacuumOnTime;
            LastSet.iOutArmVacuumDummyOnTime[i][j]=OutArmSuck.Suck[i][j].VacuumOnTime;
            LastSet.iInArmVacuumDummyOffTime[i][j]=InArmSuck.Suck[i][j].VacuumOffTime;
            LastSet.iOutArmVacuumDummyOffTime[i][j]=OutArmSuck.Suck[i][j].VacuumOffTime;
        }
    }
    for(int i=0; i<MAX_Index_Row; i++)
    {
        for(int j=0; j<NEW_MAX_Index_Col; j++)
        {
            LastSet.iFTestArmVacuumDummyOnTime[i][j]=FTestSuck.Suck[i][j].VacuumOnTime;
            LastSet.iBTestArmVacuumDummyOnTime[i][j]=BTestSuck.Suck[i][j].VacuumOnTime;
            LastSet.iFTestArmVacuumDummyOffTime[i][j]=FTestSuck.Suck[i][j].VacuumOffTime;
            LastSet.iBTestArmVacuumDummyOffTime[i][j]=BTestSuck.Suck[i][j].VacuumOffTime;
        }
    }

    LastSet.iCatchArmVacuumDummyOnTime  =CatchTraySuck.Suck[0][0].VacuumOnTime;
    LastSet.iCatchArmVacuumDummyOffTime =CatchTraySuck.Suck[0][0].VacuumOffTime;
}

// golden：InArmSuck.iShtCol（TfShuttleMove::ShowShuttleSensorPosition ShuttleMove.cpp:2001-2087 的格子欄數與迴圈上限）。
// Steven 團隊 20260925（S12-C ShuttleMove）：FileRW/ShuttleMove.gen.inc 用
int FileRW_InArmSuckShtCol()
{
    return InArmSuck.iShtCol;
}
