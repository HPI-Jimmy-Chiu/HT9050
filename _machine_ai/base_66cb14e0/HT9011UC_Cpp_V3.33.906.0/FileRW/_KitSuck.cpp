// ===========================================================================
//  FileRW/_KitSuck.cpp -- C 路產生檔要用、但不能直接 include 的移植樹狀態（Steven 20260924）。
//
//  aHotPlateSubstrate.h／mykitsuck.h 與 Public/HTEditList.h 各自定義了 TList、uPlateInfo（同名不同物），
//  同一個 TU 不能兩個都 include。gen_editlist.py 產生的 FileRW/*.gen.inc 一定帶 HTEditList.h，所以 golden
//  讀 InArmSuck／OutArmSuck 的敘述經這裡的函式轉接（本檔不 include HTEditList.h）。
// ===========================================================================
#include "aHotPlateSubstrate.h"   // TMyKitSuck InArmSuck／OutArmSuck（golden MyKitSuck.h:357）

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
