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

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a CT-3b —— golden TfContact 開頁／關頁碰 TMyKitSuck 與 MOT 的幾行
//    （FileRW/DeviceForm_File.gen.inc 不能 include aHotPlateSubstrate.h，理由見本檔檔頭；設定 tools/editlist/DeviceForm_File.py _FC_REPLACE／FormShow）。
//    golden V912 cContact.cpp（cp950）：
//      FormShow  :1388-1404  iFTestBackItem／iBTestBackItem ← FTestSuck／BTestSuck.Item（備份；golden 陣列 cContact.h:527-528 int [4][8]）
//      FormClose :1866-1879  iContactMode!=CONTACT_NORMAL ⇒ fLtcSensor->GetLtcSensor(1)、(0)（清 Latch）＋FTestSuck／BTestSuck.SetItemData 還原備份
//                            （「解決未完成 auto high 中途退出會一直出現 need one cycle error」）
//      FormClose :1884-1886  MOT[MMTrayY].HasIC() ⇒ ClearTray(__FUNC__)
//      FormClose :1904-1905  bSetHasIC ⇒ FLCarryKit／BLCarryKit.SetAll(NULL_IC)
//    ⚠ [W906] 陣列邊界：golden 迴圈上限是 FTestSuck.iMaxRow／iMaxCol（備份清 0）與 iShtRow／iShtCol，陣列是 [4][8] —— 這裡另外夾在 [4][8] 之內
//      （golden 超出時是越界寫，不照翻那一半；ht9045-array-audit 的規則）。
//    TfLtcSensor（acarry_shims.h）在移植樹是離線 shim（GetLtcSensor 回 0，不碰硬體）；照 golden 呼叫。
// ===========================================================================
#include "acarry_shims.h"          // AI(W906-EVB10A) 20260929 [W906]: fLtcSensor（golden LtcSensor.h）
#include "Motor/mymotor.h"         // AI(W906-EVB10A) 20260929 [W906]: MOT[]（TTrayMotor HasIC／ClearTray）

namespace {
int g_iFTestBackItem[4][8];        // golden cContact.h:527 TfContact::iFTestBackItem
int g_iBTestBackItem[4][8];        // golden cContact.h:528 TfContact::iBTestBackItem
inline bool InBack(int i, int j) { return i >= 0 && i < 4 && j >= 0 && j < 8; }
}  // namespace

// golden FormShow :1388-1404
void FileRW_Contact_TestSuckBackup()
{
    for(int i=0; i<FTestSuck.iMaxRow; i++)                                      //jou 2011-07-01 start : 解決未完成auto high中途退出會一直出現need one cycle error
    {
        for(int j=0; j<FTestSuck.iMaxCol; j++)
        {
            if(!InBack(i, j)) continue;                                         // [W906] 陣列邊界（見上）
            g_iFTestBackItem[i][j]=NULL_IC;
            g_iBTestBackItem[i][j]=NULL_IC;
        }
    }

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(!InBack(i, j)) continue;                                         // [W906] 陣列邊界（見上）
            g_iFTestBackItem[i][j]=FTestSuck.Item[i][j];
            g_iBTestBackItem[i][j]=BTestSuck.Item[i][j];
        }
    }
}

// golden FormClose :1866-1879 的本體（呼叫端照 golden 先判 iContactMode!=CONTACT_NORMAL）
void FileRW_Contact_TestSuckRestore()
{
    fLtcSensor->GetLtcSensor(1);                                                //Steven 20230118 : 將Latch清空，確保沒有問題
    fLtcSensor->GetLtcSensor(0);

    for(int i=0; i<FTestSuck.iShtRow; i++)                                      //jou 2011-07-01 start : 解決未完成auto high中途退出會一直出現need one cycle error
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(!InBack(i, j)) continue;                                         // [W906] 陣列邊界（見上）
            FTestSuck.SetItemData(i, j, g_iFTestBackItem[i][j]);
            BTestSuck.SetItemData(i, j, g_iBTestBackItem[i][j]);
        }
    }
}

// golden FormClose :1884-1886
bool FileRW_MotTrayHasIC(int iMot) { return MOT[iMot].HasIC(); }
void FileRW_MotClearTray(int iMot, const char* func) { MOT[iMot].ClearTray(func); }

// golden FormClose :1904-1905
void FileRW_CarryKitsSetAllNull()
{
    FLCarryKit.SetAll(NULL_IC);
    BLCarryKit.SetAll(NULL_IC);
}

// ===========================================================================
//  AI(W906-D012) 20260929 [W906] Q44 B：golden TfMain::FormClose（V912 main.cpp:12046-12048）
//      //jou 2011-12-26 關閉index kit吸嘴偵測 真空/破壞
//      CheckKitSuck.Suck[0][0].Normal();
//      CheckKitSuck.Suck[0][1].Normal();
//    呼叫端 FileRW/MainClose.cpp ShutdownSequence（關站段，照 golden 的位置；只編進 wb_serve）。放在本檔的理由同檔頭：
//    CheckKitSuck（mykitsuck.h:464，定義 mykitsuck.cpp:217）經 aHotPlateSubstrate.h:97 → mykitsuck.h 才看得到，MainClose.cpp 不 include 它。
//    （MainClose.cpp 舊註解說「兩個 TMyKitSuck 佈局」—— A4-6 起舊的精簡佈局是 #if 0，只剩 golden 的那一個。）
//    TMySucker::Normal()（mykitsuck.cpp:2189）＝OffSuck（DoOnIO(false) 真空關）＋OffDestroy（DoOffIO(false) 破壞關）＋bSuckOK／bDestroyOK=true；
//    輸出只有 OnEnable／OffEnable 的點才寫（IO_Table 的 CheckKitSuck_1／_2 On／Off 列，cinitial.cpp:514-515）。
//    execute=false：只數點、不寫。回傳啟用的 IO 點數（0～4）；*n1203＝其中 ISABase==ePCI1203 的點數（回報用）。
// ===========================================================================
int FileRW_CheckKitSuckNormal(bool execute, int* n1203)
{
    int nIo = 0, n3 = 0;
    for (int i = 0; i < 2; i++)
    {
        const TMySucker& s = CheckKitSuck.Suck[0][i];
        if (s.OnEnable)  { ++nIo; if (s.OnISABase  == ePCI1203) ++n3; }
        if (s.OffEnable) { ++nIo; if (s.OffISABase == ePCI1203) ++n3; }
    }
    if (execute)
    {
        CheckKitSuck.Suck[0][0].Normal();                                       // golden :12047
        CheckKitSuck.Suck[0][1].Normal();                                       // golden :12048
    }
    if (n1203) *n1203 = n3;
    return nIo;
}

//AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b'（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」）——
//   golden TfMain::BtnStartClick（906 main.cpp:6261-6323（V912 :6529-6593））SOFT_SIMULTE 的 AAL 段 906 :6286（V912 :6556） `if(InArmSuck.iShtRow==1)`（AI(W906-E030-CITE) 20261003）。
//   呼叫端 FileRW/DeviceForm_File.cpp 檔尾（Contact 頁 btnStartClick → BtnStartClick 的翻譯；那個 TU 帶 HTEditList.h，不能 include aHotPlateSubstrate.h）。
int FileRW_InArmSuckShtRow()
{
    return InArmSuck.iShtRow;
}
