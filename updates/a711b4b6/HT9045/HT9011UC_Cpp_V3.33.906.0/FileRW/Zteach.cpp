// ===========================================================================
//  FileRW/Zteach.cpp -- golden TZteach（AutoTeach\InOutArmZteach.cpp，V912，5171 行，cp950）的「寫檔段」：
//  <OffsetPath>\<配方>\Position Offset.Data（LastSet.iTemperature==Tempture_Hot 時 Position Offset Hot.Data）。
//  沒有頁面、沒有讀檔、目前沒有呼叫者（見下）。手寫翻譯（不是 gen_editlist 產生）：這兩支沒有 HTEditList、也沒有讀任何元件，
//  是「記憶體裡的 offset 物件 → WriteIniData」（write-inventory.md 標 B 形狀），逐行照 golden 抄，golden 註解原樣保留。
//
//  //AI(W906-FRW-S68) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S68）。一律照 V912（RULINGS 第 37 條）。
//
//  ---- golden 的讀寫面（V912 全檔 grep ReadIniData／WriteIniData／.ini／.Data／.dat，20260926）--------------------------
//    讀：**TZteach 自己不讀任何檔**。它寫出去的值來自 InArmOffSet[]／OutArmOffSet_File[]（cprod.h:233-236），
//        那兩組由 TfOffSet::ReadFile（移植樹 cOffSet.cpp:367，開機與換配方 W906_DoReadLastData 已接）讀同一個 Position Offset.Data；
//        AutoTeachOffset（cprod.h:69-77，cprod.cpp:51）只有 TZteach 的量測流程寫（:4577-4615 dXPos／dYPos）與本檔的 dPick。
//    寫：SaveFile（:4653）→ SaveSetupFile（:4674）。本檔就是這兩支。
//        另一個寫點 :3663 fTeach->SaveFile(true)（tech.dat，在自動教導流程裡）屬 S75（TfTeach::SaveFile，Jimmy），不在本檔。
//
//  ---- 呼叫者 ------------------------------------------------------------------------------------------------------
//    golden 的 SaveFile 呼叫者只有 TZteach 自己的自動教導 Z 流程（InOutArmZteach.cpp:1100／1150／1196／1247／1301／1365／1412，
//    全部是 SaveFile(n,1,false)，n＝0..6，SpecialMode＝1＝iSaveStander），外部是 AutoAlignment.cpp／SmartSetup.cpp 呼叫
//    Zteach->AutoTeachZ(...)／DoZHome()／InOutArmZHome() —— 全部是**機台動作**（馬達、真空、編碼器）。
//    RULINGS：會讓機台動作的（自動教導 Z）不接，列給 Jimmy。⇒ 本檔的 FileRW_Zteach_SaveFile() 目前**沒有呼叫者**。
//    移植樹的 Zteach 只有 aHotPlateSubstrate.h:1170 的 TfInOutArmZteach_Facade（fShow，恆 false），沒有 TZteach 本體。
//
//  ---- 給之後翻 TZteach 本體的人（Jimmy）--------------------------------------------------------------------------
//    本檔只依賴 ht9045_globals／ht9045_sm 的東西（fOffSet＝cOffSet.cpp、InArmOffSet／OutArmOffSet_File／AutoTeachOffset／
//    InputLimit＝cprod.cpp、LastSet、Tempture_Hot＝cmydef、WriteIniData／MyForceDirectories／AddSpace＝common），
//    沒有 include FileRW/_EditList.h。翻 TZteach 本體（放 ht9045_sm）時，這兩支可以原封搬進那個 TU 當成員函式，
//    本檔跟著退役（FileRW 只編進 wb_serve，ht9045_sm 的程式呼叫不到這裡）。
//
//  ---- golden 看起來錯、照翻的地方（沒有「修好」，要改行為請 Steven／Jimmy 決定）-----------------------------------------
//    (1) SaveSetupFile :4741／:4743（入料臂分支，iSelPartData 0..4）：先把 CheckRange 後的值存進 dPick[sel][j][i]，
//        寫檔卻寫 dPick[sel][j][j]（對角線那一格）。只有 i==j 的兩格（A、D 的 PickUp，A、D 的 Place）寫對；其他 12 格寫到
//        的是上一輪留在 [sel][0][0]／[sel][1][1] 的值（例：PickUp C 寫成剛存進 [sel][0][0] 的 Place A）。出料 shuttle 分支
//        :4733／:4736 用 [j][i]。//AI(W906-FRW-S68) 20260927: 原本這裡寫「看起來是筆誤」，更正：不一定是筆誤 ——
//        可能因 MTray 元件 [Col][Row] 的慣例（跟一般 [Row][Col] 相反）而刻意如此（Steven 20260927 Q22＝A），照翻，待 Jimmy 確認原意。
//    (2) SaveFile :4655 `iSelPartData>OfsTotal`（應該是 >=）：CapStr 只有 7 個有字、InArmOffSet 只有 InOfsTotal 個、
//        AutoTeachOffset.dPick 第一維只有 10 —— 呼叫者只傳 0..6，所以實際不會越界。
//    (3) 入料臂分支寫 InArmOffSet[]（執行期那份），出料 shuttle 分支寫 OutArmOffSet_File[]（檔案那份）—— 兩邊不對稱，照 golden。
//    (4) SaveSetupFile 的 bReset 沒有用到（golden 也沒有）；不是 iSaveStander 時只算出路徑、呼叫 AddSpace（golden 與移植樹都是空函式）。
// ===========================================================================
#include "forms/fOffSet.h"   // fOffSet->GetOffsetPath()（cOffSet.cpp:188，golden cOffSet.cpp:1426-1462）
#include "cprod.h"           // InArmOffSet／OutArmOffSet_File（ARM_OFFSET）、AutoTeachOffset、InputLimit
#include "LastSet.h"         // LastSet.iTemperature
#include "cmydef.h"          // Tempture_Hot
#include "common.h"          // WriteIniData、MyForceDirectories、AddSpace
#include "MachineType.h"     // OfsTotal、CheckRange

// golden cOffSet.cpp:172 `const int iSaveStander=1;`（cOffSet.h:504 extern，InOutArmZteach.cpp 經 cOffSet.h 看到它）。
// 移植樹沒有全域那一份（FileRW/Offset_File.gen.inc:36 是它自己的 static），同樣照抄成本 TU 的常數。
static const int iSaveStander=1;

static void ZT_SaveSetupFile(AnsiString szDir, AnsiString szFilePath, int iSelPartData, int SpecialMode, bool bReset);

// golden AutoTeach/InOutArmZteach.cpp:4653  void __fastcall TZteach::SaveFile(int iSelPartData, int SpecialMode, bool bReset)
// 目前沒有呼叫者（見檔頭「呼叫者」）。會寫 Position Offset.Data（或 Hot），SpecialMode 要含 iSaveStander(1) 才真的寫。
void FileRW_Zteach_SaveFile(int iSelPartData, int SpecialMode, bool bReset)
{
    if(iSelPartData<0 || iSelPartData>OfsTotal)
        return;
    AnsiString szDir="", szFilePath="";

    szDir=fOffSet->GetOffsetPath();
    szFilePath=szDir;

    //kevin 20150105  Start 另存 jobfile
    ZT_SaveSetupFile(szDir, szFilePath, iSelPartData, SpecialMode, bReset);     //kevin 20150105
    /*
    #ifdef ASE_KaohSiung
        fBuilder->bSaveAsJobFile(fOffSet->LastFileName, "JOBFILE");
    #endif
    */
    //kevin 20150105 end
}
//------------------------------------------------------------
//kevin 20210311 oFFSET  儲存 檔案另存 jobfile
//------------------------------------------------------------
// golden AutoTeach/InOutArmZteach.cpp:4674  void __fastcall TZteach::SaveSetupFile(AnsiString szDir, AnsiString szFilePath,
//   int iSelPartData, int SpecialMode, bool bReset)
// 寫的鍵（區段＝CapStr[iSelPartData]）：出料 shuttle（5／6）Hand X、Hand Y、PickUp、PickUp A..H、Place A..H（19 鍵）；
//   其他（0..4）Hand X、Hand Y、PickUp、Place、PickUp A..H、Place A..H（20 鍵）。
static void ZT_SaveSetupFile(AnsiString szDir, AnsiString szFilePath, int iSelPartData, int SpecialMode, bool /*bReset*/)
{
//    bool FilePathErr=false;
    int i, j;                                                                   //, iNum;
    AnsiString str;
    int iOffsetUnit=0;                                                          //kevin 20210720 add log

    AnsiString CapStr[OfsTotal]=
    {
        "Loader",                                                               //0
        "Hot Plate1",                                                           //1
        "Hot Plate2",                                                           //2
        "Input Shuttle1",                                                       //3
        "Input Shuttle2",                                                       //4
        "Output Shuttle1",                                                      //5
        "Output Shuttle2",                                                      //6
    };

    MyForceDirectories(szDir);
    szDir+="\\Position Offset.Data";

//    if(!FileExists(szDir))
//        FilePathErr=true;

    if(SpecialMode & iSaveStander)
    {
        szDir=szFilePath;                                                       //kevin 20150105
        szDir=szFilePath;                                                       //kevin 20150105
        if(LastSet.iTemperature==Tempture_Hot)                                  //kevin 20210720
            szDir+="\\Position Offset Hot.Data";
        else
            szDir+="\\Position Offset.Data";

         if(iSelPartData==5)                                                    //kevin 20210720 add ootSHT
             iOffsetUnit=0;
         else if(iSelPartData==6)
             iOffsetUnit=1;

        if(iSelPartData==5 ||iSelPartData==6)                                   // OUT SHUTTLE 1 2
        {
            WriteIniData(szDir, CapStr[iSelPartData], "Hand X",   OutArmOffSet_File[iOffsetUnit]->GetX());  //kevin 20210720 add ootSHT
            WriteIniData(szDir, CapStr[iSelPartData], "Hand Y",   OutArmOffSet_File[iOffsetUnit]->GetY());  //kevin 20210720 add ootSHT
            WriteIniData(szDir, CapStr[iSelPartData], "PickUp",   OutArmOffSet_File[iOffsetUnit]->GetPickUp());  //kevin 20210720 add ootSHT
        }
        else
        {
            WriteIniData(szDir, CapStr[iSelPartData], "Hand X",   InArmOffSet[iSelPartData]->GetX());  //kevin 20210629 add
            WriteIniData(szDir, CapStr[iSelPartData], "Hand Y",   InArmOffSet[iSelPartData]->GetY());  //kevin 20210629 add
            WriteIniData(szDir, CapStr[iSelPartData], "PickUp",   InArmOffSet[iSelPartData]->GetPickUp());
            WriteIniData(szDir, CapStr[iSelPartData], "Place",   InArmOffSet[iSelPartData]->GetPlace());
        }

        for(i=0; i<4; i++)                                                      //8=In/out arm suction
        {
            for(j=0; j<2; j++)                                                  //8=In/out arm suction
            {
                if(iSelPartData==5 ||iSelPartData==6)
                {
                    AutoTeachOffset.dPick[iSelPartData][j][i] = CheckRange(double(OutArmOffSet_File[iOffsetUnit]->GetPickUp(j,i)),double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));  //JerryYang 20190328 Offset上下限保護
                    WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("PickUp %c", 'A'+(i*2)+j), AutoTeachOffset.dPick[iSelPartData][j][i]);  //kevin 20210720 add ootSHT

                    AutoTeachOffset.dPick[iSelPartData][j][i] = CheckRange(double(OutArmOffSet_File[iOffsetUnit]->GetPlace(j,i)),double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));  //JerryYang 20190328 Offset上下限保護
                    WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Place %c", 'A'+(i*2)+j), AutoTeachOffset.dPick[iSelPartData][j][i]);
                }
                else
                {
                    //AI(W906-FRW-S68) 20260926: 下面兩個 WriteIniData 寫 dPick[..][j][j]（不是剛算好的 [j][i]）—— golden 原樣（檔頭 (1)），照翻不修
                    //AI(W906-FRW-S68) 20260927: 可能因 MTray 元件 [Col][Row] 的慣例而刻意如此（Steven 20260927 Q22＝A），待 Jimmy 確認
                    AutoTeachOffset.dPick[iSelPartData][j][i] = CheckRange(double(InArmOffSet[iSelPartData]->GetPickUp(j,i)),double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));  //JerryYang 20190328 Offset上下限保護
                    WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("PickUp %c", 'A'+(i*2)+j), AutoTeachOffset.dPick[iSelPartData][j][j]);
                    AutoTeachOffset.dPick[iSelPartData][j][i] = CheckRange(double(InArmOffSet[iSelPartData]->GetPlace(j,i)),double(InputLimit.iOffsetZHigh), double(InputLimit.iOffsetZLow));  //JerryYang 20190328 Offset上下限保護
                    WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Place %c", 'A'+(i*2)+j), AutoTeachOffset.dPick[iSelPartData][j][j]);
                }
            }
        }
    }

    AddSpace(szDir);
}
