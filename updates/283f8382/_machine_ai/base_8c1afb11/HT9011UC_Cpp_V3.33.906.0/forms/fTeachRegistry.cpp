// ===========================================================================
//  forms/fTeachRegistry.cpp  --  TfTeach 的 teach 參數登錄表
//
//  翻譯波次：AI(W906-TEACH-W1) 20260919
//  golden：`uteach.cpp:342-995`（`TfTeach::TfTeach` ctor 的登錄表區段）
//
//  ⚠⚠ 這個檔的函式本體是**產生的，不要手改**。
//      重新產生：`python tools/gen_teach_registry.py forms/fTeachRegistry.body`
//      （那支工具的檔頭寫了 T1-T4 四條變換各自是什麼、為什麼）。
//      手改會在下一次重新產生時被蓋掉，而且會讓「這是 golden 的可驗證變換」
//      這個性質失效 —— 那個性質才是這個檔存在的理由。
//
//  ---------------------------------------------------------------------------
//  為什麼是產生的
//  ---------------------------------------------------------------------------
//  登錄表是 440 個 `push_back` 加一個 2x8 的填值迴圈。手打一次就是 440 次
//  出錯機會，而且沒有人有辦法逐行覆核。產生的版本是 golden 的**可重跑變換**：
//  golden 動了就重跑，`git diff` 會說話。
//
//  產生時做的四條變換（統計值由工具在產生當下印出，20260919 實測）：
//    T1  widget 建構子引數 -> `/*原識別字*/0`      1,129 處
//    T2  純 widget 敘述（`X->Caption=` …）整行註解      8 處
//    T3  控制流 / 註解 / 空白 / golden 的死碼 -> 原樣保留
//    T4  `X.SetEdit[..] = <widget>;` 成員賦值 -> 賦 0    4 處
//  ⇒ 引數位置、順序、欄寬全部保持，所以這個檔可以跟 golden **逐行機械比對**。
//
//  偏離的完整說明在 `forms/fTeachPara.h` 的檔頭，不在這裡重複。
//  一句話：移植樹的 `TfTeach` 沒有 widget 實例（661 個識別字，
//  `forms/fTeach.h:330-343` 明文 DEFERRED），而讀取路徑
//  （`Parameter` / `MotorSelect` / `Key`）完全不經過 widget。
//
//  ---------------------------------------------------------------------------
//  結構上的一處偏離：golden 在 ctor 裡做，這裡拆成一個方法
//  ---------------------------------------------------------------------------
//  golden 把這 654 行直接寫在 `TfTeach::TfTeach()` 裡（:342-995）。
//  移植樹把它拆成 `TfTeach::BuildTechRegistry()`，由 ctor 在**最後**呼叫一次。
//  * 語意零差異：同一個物件、同一個順序、同一個時機（ctor 結束前）。
//  * 為什麼拆：`forms/fTeach.cpp` 的既有行號被別處引用（實測 11 筆），
//    往 ctor 中間塞 654 行會把它們全部推掉。memory:
//    never-mechanically-shift-line-citations。
//  * `TECH_MAX_ITEM` / `TechTwoItem` 的賦值（golden :993-994）也在這支尾端，
//    位置與 golden 相同。
// ===========================================================================
#include "forms/fTeachPara.h"
#include "forms/fTeach.h"

#include "cmydef.h"                // 馬達代號（MInArmZA / MTestZ1 / ...）、USE_OUT_SORT_ARM、
                                   // eartUninstall、IndexSuckName、SubMachineType、Type_*
#include "cprod.h"                 // Teach（Teach_Pos）
#include "LastSet.h"               // Tech（TECH）
#include "cpublic.h"               // IniConfig / CosFunction / TestIF 等組態旗標
#include "Motor/mymotor.h"         // MOT[]、InArmZIndex / OutArmZIndex / SortArmZIndex

#include "vclcompat/vcl_compat.h"  // AnsiString

// ---------------------------------------------------------------------------
//  golden uteach.cpp:342-995 —— 產生的，不要手改。
// ---------------------------------------------------------------------------
void TfTeach::BuildTechRegistry()
{
    AnsiString asString;

    // golden :342
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            asString.sprintf("Picker%s", IndexSuckName[i][j]);
            TechSuckPara[0].Key[i][j]           =asString;
            TechSuckPara[0].SetEdit[i][j]       =/*teInArm[i][j]*/0;
            TechSuckPara[0].MotorSelect[i][j]   =InArmZIndex[i][j];

            TechSuckPara[1].Key[i][j]           =asString;
            TechSuckPara[1].SetEdit[i][j]       =/*teOutArm[i][j]*/0;
            TechSuckPara[1].MotorSelect[i][j]   =OutArmZIndex[i][j];

            if(j<4)                                                             //Steven 20240703 : i --> j
            {
                TechSuckPara[0].Parameter[i][j] =&Tech.iInArmZHeightSub[i][j];
                TechSuckPara[1].Parameter[i][j] =&Tech.iOutArmZHeightSub[i][j];
            }
            else
            {
                TechSuckPara[0].Parameter[i][j] =&Tech.iInArmZHeightSub_16[i][j-4];
                TechSuckPara[1].Parameter[i][j] =&Tech.iOutArmZHeightSub_16[i][j-4];
            }
        }
    }
    // golden :367
    TechSuckPara[0].iTag=0;
    TechSuckPara[0].Group="InArmZSub";
    TechSuckPara[1].iTag=1;
    TechSuckPara[1].Group="OutArmZSub";

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250416 for HT9046AU add
    {
        asString.sprintf("Picker%s", IndexSuckName[0][0]);                      //Aa
        TechSuckPara[2].Key[0][0]           =asString;
        TechSuckPara[2].SetEdit[0][0]       =/*teSortArm[0]*/0;
        TechSuckPara[2].MotorSelect[0][0]   =SortArmZIndex[0];
        TechSuckPara[2].Parameter[0][0]     =&Tech.iSortArmZHeightSub[0];

        asString.sprintf("Picker%s", IndexSuckName[0][1]);                      //Ab
        TechSuckPara[2].Key[0][1]           =asString;
        TechSuckPara[2].SetEdit[0][1]       =/*teSortArm[1]*/0;
        TechSuckPara[2].MotorSelect[0][1]   =SortArmZIndex[1];
        TechSuckPara[2].Parameter[0][1]     =&Tech.iSortArmZHeightSub[1];

        TechSuckPara[2].iTag=2;
        TechSuckPara[2].Group="SortArmZSub";
    }

    TechPara.push_back(new TECH_PARA(&Tech.iInArmSafeZ1                     ,   MInArmZA        , /*setEditInZSafeHeight*/0      , "setEditInZSafeHeight"        , /*SetButton030*/0          , /*GoButton030*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTestZ1ShutlePick                ,   MTestZ1         , /*setEditIndex1ToSht1Z*/0      , "setEditIndex1ToSht1Z"        , /*SetButton066*/0          , /*GoButton066*/0));
    // golden :392
    TechPara.push_back(new TECH_PARA(&Tech.iTestZ2ShutlePick                ,   MTestZ2         , /*setEditIndex2ToSht2Z*/0      , "setEditIndex2ToSht2Z"        , /*SetButton067*/0          , /*GoButton067*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTestZDown                       ,   MTestZ1         , /*setEditWaitTestZDown*/0      , "setEditWaitTestZDown"        , /*SetButton068*/0          , /*GoButton068*/0));  //index to socket hight
    TechPara.push_back(new TECH_PARA(&Tech.iTestZ1ShutleWait                ,   MTestZ1         , /*setEditTestZSafePos*/0       , "setEditTestZSafePos"         , /*SetButton069*/0          , /*GoButton069*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTestZ2ShutleWait                ,   MTestZ2         , /*setEditTestZSafePos*/0       , "setEditTestZSafePos"         , /*SetButton069*/0          , /*GoButton069*/0));  //Steven 20091113
    TechPara.push_back(new TECH_PARA(&Teach.iLoadCellY1                     ,   MTestY1         , /*setEdLoadCellY1*/0           , "setEdLoadCellY1"             , /*SetButton068*/0          , /*GoButton068*/0));  //kevin 20190907 add loadcell pos
    TechPara.push_back(new TECH_PARA(&Teach.iLoadCellY2                     ,   MTestY2         , /*setEdLoadCellY2*/0           , "setEdLoadCellY2"             , /*SetButton068*/0          , /*GoButton068*/0));  //kevin 20190907 add loadcell pos
    TechPara.push_back(new TECH_PARA(&Teach.iLoadCellZ1Down                 ,   MTestZ1         , /*setEdLoadCellZ1*/0           , "setEdLoadCellZ1"             , /*SetButton068*/0          , /*GoButton068*/0));  //kevin 20190907 add loadcell pos
    TechPara.push_back(new TECH_PARA(&Teach.iLoadCellZ2Down                 ,   MTestZ2         , /*setEdLoadCellZ2*/0           , "setEdLoadCellZ2"             , /*SetButton068*/0          , /*GoButton068*/0));  //kevin 20190907 add loadcell pos

    TechPara.push_back(new TECH_PARA(&Tech.iOutArmSafeZ1                    ,   MOutArmZA       , /*setEditOutZSafeHeight*/0     , "setEditOutZSafeHeight"       , /*SetButton106*/0          , /*GoButton106*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXLoader                     ,   MTrayX          , /*setEditTrayLoaderX*/0        , "setEditTrayLoaderX"          , /*SetButton140*/0          , /*GoButton140*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXEmpty                      ,   MTrayX          , /*setEditTrayEmptyX*/0         , "setEditTrayEmptyX"           , /*SetButton141*/0          , /*GoButton141*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXColor                      ,   MTrayX          , /*setEditTrayColorX*/0         , "setEditTrayColorX"           , /*SetButton142*/0          , /*GoButton142*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto1                      ,   MTrayX          , /*setEditTrayAuto1X*/0         , "setEditTrayAuto1X"           , /*SetButton143*/0          , /*GoButton143*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto2                      ,   MTrayX          , /*setEditTrayAuto2X*/0         , "setEditTrayAuto2X"           , /*SetButton144*/0          , /*GoButton144*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto3                      ,   MTrayX          , /*setEditTrayAuto3X*/0         , "setEditTrayAuto3X"           , /*SetButton145*/0          , /*GoButton145*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto4                      ,   MTrayX          , /*setEdtAuto4*/0               , "setEdtAuto4"                 , /*btnAuto4*/0              , /*GoBtnAuto4*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto5                      ,   MTrayX          , /*setEdtAuto5*/0               , "setEdtAuto5"                 , /*btnAuto5*/0              , /*GoBtnAuto5*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXAuto6                      ,   MTrayX          , /*setEdtAuto6*/0               , "setEdtAuto6"                 , /*btnAuto6*/0              , /*GoBtnAuto6*/0));

    if(USE_PICKER_COUNT==ep16Picker)
    {
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch               ,   MInArmPitch     , /*setEditInXPitch40*/0         , "setEditInXPitch40"           , /*SetButtonInX140*/0       , /*GoButton003*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch              ,   MInArmPitch     , /*setEditInXPitch120*/0        , "setEditInXPitch120"          , /*SetButtonInX1120*/0      , /*GoButton005*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch2              ,   MInArmPitchX2   , /*setEditInX240*/0             , "setEditInX240"               , /*SetButtonInX240*/0       , /*GoButtonInX240*/0));
    // golden :417
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch2             ,   MInArmPitchX2   , /*setEditInX2120*/0            , "setEditInX2120"              , /*SetButtonInX2120*/0      , /*GoButtonInX2120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmY15Pitch               ,   MInArmPitchY    , /*setEditInY15*/0              , "setEditInY15"                , /*SetButtonInY15*/0        , /*GoButtonInY15*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmY60Pitch               ,   MInArmPitchY    , /*setEditInY60*/0              , "setEditInY60"                , /*SetButtonInY60*/0        , /*GoButtonInY60*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch              ,   MOutArmPitch    , /*setEditOutXPitch40*/0        , "setEditOutXPitch40"          , /*SetButtonOutX140*/0      , /*GoButton004*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch             ,   MOutArmPitch    , /*setEditOutXPitch120*/0       , "setEditOutXPitch120"         , /*SetButtonOutX1120*/0     , /*GoButton006*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch2             ,   MOutArmPitchX2  , /*setEditOutX240*/0            , "setEditOutX240"              , /*SetButtonOutX240*/0      , /*GoButtonOutX240*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch2            ,   MOutArmPitchX2  , /*setEditOutX2120*/0           , "setEditOutX2120"             , /*SetButtonOutX2120*/0     , /*GoButtonOutX2120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmY15Pitch              ,   MOutArmPitchY   , /*setEditOutY15*/0             , "setEditOutY15"               , /*SetButtonOutY15*/0       , /*GoButtonOutY15*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmY60Pitch              ,   MOutArmPitchY   , /*setEditOutY60*/0             , "setEditOutY60"               , /*SetButtonOutY60*/0       , /*GoButtonOutY60*/0));
    }
    else
    {
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch               ,   MInArmPitch     , /*setEditInXPitch40*/0         , "setEditInXPitch40"           , /*SetButtonInX140*/0       , /*GoButton003*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch              ,   MInArmPitch     , /*setEditInXPitch120*/0        , "setEditInXPitch120"          , /*SetButtonInX1120*/0      , /*GoButton005*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch2              ,   MInArmPitchX2   , /*setEditInX240*/0             , "setEditInX240"               , /*SetButtonInX240*/0       , /*GoButtonInX240*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch2             ,   MInArmPitchX2   , /*setEditInX2120*/0            , "setEditInX2120"              , /*SetButtonInX2120*/0      , /*GoButtonInX2120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch3              ,   MInArmPitchX3   , /*setEditInX340*/0             , "setEditInX340"               , /*SetButtonInX340*/0       , /*GoButtonInX340*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch3             ,   MInArmPitchX3   , /*setEditInX3120*/0            , "setEditInX3120"              , /*SetButtonInX3120*/0      , /*GoButtonInX3120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX40Pitch4              ,   MInArmPitchX4   , /*setEditInX440*/0             , "setEditInX440"               , /*SetButtonInX440*/0       , /*GoButtonInX440*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmX120Pitch4             ,   MInArmPitchX4   , /*setEditInX4120*/0            , "setEditInX4120"              , /*SetButtonInX4120*/0      , /*GoButtonInX4120*/0));

        TechPara.push_back(new TECH_PARA(&Tech.iInArmY15Pitch               ,   MInArmPitchY    , /*setEditInY15*/0              , "setEditInY15"                , /*SetButtonInY15*/0        , /*GoButtonInY15*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmY60Pitch               ,   MInArmPitchY    , /*setEditInY60*/0              , "setEditInY60"                , /*SetButtonInY60*/0        , /*GoButtonInY60*/0));

        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch              ,   MOutArmPitch    , /*setEditOutXPitch40*/0        , "setEditOutXPitch40"          , /*SetButtonOutX140*/0      , /*GoButton004*/0));
    // golden :442
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch             ,   MOutArmPitch    , /*setEditOutXPitch120*/0       , "setEditOutXPitch120"         , /*SetButtonOutX1120*/0     , /*GoButton006*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch2             ,   MOutArmPitchX2  , /*setEditOutX240*/0            , "setEditOutX240"              , /*SetButtonOutX240*/0      , /*GoButtonOutX240*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch2            ,   MOutArmPitchX2  , /*setEditOutX2120*/0           , "setEditOutX2120"             , /*SetButtonOutX2120*/0     , /*GoButtonOutX2120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch3             ,   MOutArmPitchX3  , /*setEditOutX340*/0            , "setEditOutX340"              , /*SetButtonOutX340*/0      , /*GoButtonOutX340*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch3            ,   MOutArmPitchX3  , /*setEditOutX3120*/0           , "setEditOutX3120"             , /*SetButtonOutX3120*/0     , /*GoButtonOutX3120*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX40Pitch4             ,   MOutArmPitchX4  , /*setEditOutX440*/0            , "setEditOutX440"              , /*SetButtonOutX440*/0      , /*GoButtonOutX440*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmX120Pitch4            ,   MOutArmPitchX4  , /*setEditOutX4120*/0           , "setEditOutX4120"             , /*SetButtonOutX4120*/0     , /*GoButtonOutX4120*/0));

        TechPara.push_back(new TECH_PARA(&Tech.iOutArmY15Pitch              ,   MOutArmPitchY   , /*setEditOutY15*/0             , "setEditOutY15"               , /*SetButtonOutY15*/0       , /*GoButtonOutY15*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmY60Pitch              ,   MOutArmPitchY   , /*setEditOutY60*/0             , "setEditOutY60"               , /*SetButtonOutY60*/0       , /*GoButtonOutY60*/0));
    }
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXClean                      ,   MTrayX          , /*setEditTrayCleanX*/0         , "setEditTrayCleanX"           , /*SetButton146*/0          , /*GoButton146*/0));  //wei 20150826 拍拍Tray X軸 Teach
    TechPara.push_back(new TECH_PARA(&Tech.iTrayXOCR                        ,   MTrayX          , /*setEditTrayOCRX*/0           , "setEditTrayOCRX"             , /*SetButton147*/0          , /*GoButton147*/0));  //wei 20151001 OCRTray X軸 Teach

    TechPara.push_back(new TECH_PARA(&Tech.OutSH1ZDetectPos                 ,   MInShuttle1     , /*setEditOutSht1KitPos*/0      , "setEditOutSht1KitPos"        , /*SetButton200*/0          , /*GoButton200*/0));
    TechPara.push_back(new TECH_PARA(&Tech.OutSH2ZDetectPos                 ,   MInShuttle2     , /*setEditOutSht2KitPos*/0      , "setEditOutSht2KitPos"        , /*SetButton201*/0          , /*GoButton201*/0));

    TechPara.push_back(new TECH_PARA(&Tech.OutSH1ZOneRowDetectPos           ,   MInShuttle1     , /*setEditOutSht1OneRowKit*/0   , "setEditOutSht1OneRowKit"     , /*SetButton206*/0          , /*GoButton206*/0));
    TechPara.push_back(new TECH_PARA(&Tech.OutSH2ZOneRowDetectPos           ,   MInShuttle2     , /*setEditOutSht2OneRowKit*/0   , "setEditOutSht2OneRowKit"     , /*SetButton207*/0          , /*GoButton207*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iInShuttle1Left                  ,   MInShuttle1     , /*setEditInSht1Left*/0         , "setEditInSht1Left"           , /*SetButton060*/0          , /*GoButton060*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInShuttle1Right                 ,   MInShuttle1     , /*setEditInSht1Right*/0        , "setEditInSht1Right"          , /*SetButton061*/0          , /*GoButton061*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInShuttle2Left                  ,   MInShuttle2     , /*setEditInSht2Left*/0         , "setEditInSht2Left"           , /*SetButton062*/0          , /*GoButton062*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInShuttle2Right                 ,   MInShuttle2     , /*setEditInSht2Right*/0        , "setEditInSht2Right"          , /*SetButton063*/0          , /*GoButton063*/0));

    // golden :467
    TechPara.push_back(new TECH_PARA(&Tech.iInSH1Sen7DetectPos              ,   MInShuttle1     , /*edtEditInSht1OctSiteKit*/0   , "edtEditInSht1OctSiteKit"     , /*btnSetButton204*/0       , /*btnGoButton204*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInSH2Sen7DetectPos              ,   MInShuttle2     , /*edtEditInSht2OctSiteKit*/0   , "edtEditInSht2OctSiteKit"     , /*btnSetButton205*/0       , /*btnGoButton205*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInSH1BarCodePos                 ,   MInShuttle1     , /*edtSetEditIS1BarCode*/0      , "edtSetEditIS1BarCode"        , /*btnSetInSht1BarCode*/0   , /*btnGoInSht1BarCode*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInSH2BarCodePos                 ,   MInShuttle2     , /*edtSetEditIS2BarCode*/0      , "edtSetEditIS2BarCode"        , /*btnSetInSht2BarCode*/0   , /*btnGoInSht2BarCode*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutSH1BarCodePos                ,   MInShuttle1     , /*edtSetEditOS1BarCode*/0      , "edtSetEditOS1BarCode"        , /*btnSetOutSht1BarCode*/0  , /*btnGoOutSht1BarCode*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutSH2BarCodePos                ,   MInShuttle2     , /*edtSetEditOS2BarCode*/0      , "edtSetEditOS2BarCode"        , /*btnSetOutSht2BarCode*/0  , /*btnGoOutSht2BarCode*/0));

//    TechPara.push_back(new TECH_PARA(&Tech.iShuttle1BusyHalfOffset          ,   MInShuttle1     , /*setEditSht1BusyHalf*/0       , "setEditSht1BusyHalf"         , /*SetButton202*/0          , /*GoButton202*/0));
//    TechPara.push_back(new TECH_PARA(&Tech.iShuttle2BusyHalfOffset          ,   MInShuttle2     , /*setEditSht2BusyHalf*/0       , "setEditSht2BusyHalf"         , /*SetButton203*/0          , /*GoButton203*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmLoadStagePickZ2            ,   MInArmZE        , /*SetEditPickLoader*/0         , "SetEditPickLoader"           , /*SetButtonPick1*/0        , /*GoButtonPick1*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmPlatePickZ2                ,   MInArmZE        , /*SetEditHP*/0                 , "SetEditHP"                   , /*SetButtonPick2*/0        , /*GoButtonPick2*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmShuttlePickZ2             ,   MOutArmZE       , /*SetEditPickOutSht*/0         , "SetEditPickOutSht"           , /*SetButtonPick4*/0        , /*GoButtonPick4*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmPlaceFixZ1                ,   MOutArmZE       , /*SetEditPlaceFix*/0           , "SetEditPlaceFix"             , /*SetButtonPlace1*/0       , /*GoButtonPlace1*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmPlaceZ2                   ,   MOutArmZE       , /*SetEditPlaceAuto*/0          , "SetEditPlaceAuto"            , /*SetButtonPlace2*/0       , /*GoButtonPlace2*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmShuttlePlaceZ              ,   MInArmZE        , /*SetEditPlaceInShuttle*/0     , "SetEditPlaceInShuttle"       , /*SetButtonPlace3*/0       , /*GoButtonPlace3*/0));

    TechPara.push_back(new TECH_PARA(&Tech.Sht1XGabage                      ,   MInArmX         , /*setEditSht1XGabage*/0        , "setEditSht1XGabage"          , /*btnSetSht1XGabage*/0     , /*btnGoSht1XGabage*/0));  //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.Sht1YGabage                      ,   MInArmY         , /*setEditSht1YGabage*/0        , "setEditSht1YGabage"          , /*btnSetSht1YGabage*/0     , /*btnGoSht1YGabage*/0));  //kevin 20220819 add place shuttle ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.Sht2XGabage                      ,   MInArmX         , /*setEditSht2XGabage*/0        , "setEditSht2XGabage"          , /*btnSetSht2XGabage*/0     , /*btnGoSht2XGabage*/0));  //kevin 20220819 add place ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.Sht2YGabage                      ,   MInArmY         , /*setEditSht2YGabage*/0        , "setEditSht2YGabage"          , /*btnSetSht2YGabage*/0     , /*btnGoSht2YGabage*/0));  //kevin 20220819 add place ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.LoadXGabage                      ,   MInArmX         , /*setEditLoadXGabage*/0        , "setEditLoadXGabage"          , /*btnSetLoadXGabage*/0     , /*btnGoLoadXGabage*/0));  //kevin 20220819 add place ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.LoadYGabage                      ,   MInArmY         , /*setEditLoadYGabage*/0        , "setEditLoadYGabage"          , /*btnSetLoadYGabage*/0     , /*btnGoLoadYGabage*/0));  //kevin 20220819 add place ic error  要吸起IC放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.iGabageX                         ,   MInArmX         , /*setEdGabageX*/0              , "setEdGabageX"                , /*btnSetXGabage*/0         , /*btnGoXGabage*/0));  //kevin 20220819 add 放置一個地方集中
    TechPara.push_back(new TECH_PARA(&Tech.iGabageY                         ,   MInArmY         , /*setEdGabageY*/0              , "setEdGabageY"                , /*btnSetYGabage*/0         , /*btnGoYGabage*/0));  //kevin 20220819 add 放置一個地方集中

    // golden :492
    TechPara.push_back(new TECH_PARA(&Teach.iAutoCleanPick                  ,   MInArmZE        , /*SetEditAutoClean*/0          , "SetEditAutoClean"            , /*SetBtnAutoCleanPick2*/0  , /*GoBtnAutoCleanPick2*/0));  //kevin 20190305 add AutoCleacl pos

    TechPara.push_back(new TECH_PARA(&Tech.iInArmPreciserPlaceZ             ,   MInArmZE        , /*SetEditPlacePreciserZ*/0     , "SetEditPlacePreciserZ"       , /*SetBtnPlacePreciser*/0   , /*GoBtnPlacePreciser*/0));  //Frank 20180410 (Steven) : InArm Preciser Station
    TechPara.push_back(new TECH_PARA(&Tech.iPreciserOpenPitch               ,   MPreciser       , /*setEditPreciserPitchOpen*/0  , "setEditPreciserPitchOpen"    , /*SetBtnPreciserOpen*/0    , /*GoBtnPreciserOpen*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iPreciserClosePitch              ,   MPreciser       , /*setEditPreciserPitchClose*/0 , "setEditPreciserPitchClose"   , /*SetBtnPreciserClose*/0   , /*GoBtnPreciserClose*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[0]                  ,   MLoaderZ        , /*setLoaderZ*/0                , "setLoaderZ"                  , /*setBtnLoaderZ*/0         , /*GoBtnLoaderZ*/0));  //Steven 20120822 : 加入Tray Z軸馬達
    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[1]                  ,   MEmptyZ         , /*setEmptyZ*/0                 , "setEmptyZ"                   , /*setBtnEmptyZ*/0          , /*GoBtnEmptyZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[2]                  ,   MColorZ         , /*setColorZ*/0                 , "setColorZ"                   , /*setBtnColorZ*/0          , /*GoBtnColorZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[3]                  ,   MAuto1Z         , /*setAuto1Z*/0                 , "setAuto1Z"                   , /*setBtnAuto1Z*/0          , /*GoBtnAuto1Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[4]                  ,   MAuto2Z         , /*setAuto2Z*/0                 , "setAuto2Z"                   , /*setBtnAuto2Z*/0          , /*GoBtnAuto2Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayLoaderZ[5]                  ,   MAuto3Z         , /*setAuto3Z*/0                 , "setAuto3Z"                   , /*setBtnAuto3Z*/0          , /*GoBtnAuto3Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayAuto4Z[0]                   ,   MAuto4Z         , /*setAuto4Z*/0                 , "setAuto4Z"                   , /*setBtnAuto4Z*/0          , /*GoBtnAuto4Z*/0));  //Steven 20230907 : For HT-9011UC
    TechPara.push_back(new TECH_PARA(&Tech.iTrayAuto4Z[1]                   ,   MAuto5Z         , /*setAuto5Z*/0                 , "setAuto5Z"                   , /*setBtnAuto5Z*/0          , /*GoBtnAuto5Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iTrayAuto4Z[2]                   ,   MAuto6Z         , /*setAuto6Z*/0                 , "setAuto6Z"                   , /*setBtnAuto6Z*/0          , /*GoBtnAuto6Z*/0));

    if(USE_PICKER_COUNT==ep16Picker ||                                          //Ztex 2023.12.06 Add HT-1032
        SubMachineType==Type_HT9046AU)                                          //RogerYang 20250415 Add for HT9046AU
    {
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[0]              ,   MTrayZ          , /*setLoaderZUp*/0              , "setLoaderZUp"                , /*SetBtnLoaderZUp*/0       , /*GoBtnLoaderZUp*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[1]              ,   MTrayZ          , /*setEmptyZUp*/0               , "setEmptyZUp"                 , /*SetBtnEmptyZUp*/0        , /*GoBtnEmptyZUp*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[2]              ,   MTrayZ          , /*setColorZUp*/0               , "setColorZUp"                 , /*SetBtnColorZUp*/0        , /*GoBtnColorZUp*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[3]              ,   MTrayZ          , /*setAuto1ZUp*/0               , "setAuto1ZUp"                 , /*SetBtnAuto1ZUp*/0        , /*GoBtnAuto1ZUp*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[4]              ,   MTrayZ          , /*setAuto2ZUp*/0               , "setAuto2ZUp"                 , /*SetBtnAuto2ZUp*/0        , /*GoBtnAuto2ZUp*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[5]              ,   MTrayZ          , /*setAuto3ZUp*/0               , "setAuto3ZUp"                 , /*SetBtnAuto3ZUp*/0        , /*GoBtnAuto3ZUp*/0));
    // golden :517
        if(AUTO_EMPTY_COLOR>=3)
        {
            TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[6]          ,   MTrayZ          , /*setAuto4ZUp*/0               , "setAuto4ZUp"                 , /*SetBtnAuto4ZUp*/0        , /*GoBtnAuto4ZUp*/0));
            TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[7]          ,   MTrayZ          , /*setAuto5ZUp*/0               , "setAuto5ZUp"                 , /*SetBtnAuto5ZUp*/0        , /*GoBtnAuto5ZUp*/0));
        }

        if(AUTO_EMPTY_COLOR>=4)
        {
            TechPara.push_back(new TECH_PARA(&Tech.iTrayArmZPnP[8]          ,   MTrayZ          , /*setAuto6ZUp*/0               , "setAuto6ZUp"                 , /*SetBtnAuto6ZUp*/0        , /*GoBtnAuto6ZUp*/0));
        }
    }

    TechPara.push_back(new TECH_PARA(&Tech.M_In_iRotateA                    ,   MInRotateKit    , /*setEditRotateA*/0            , "setEditRotateA"              , /*SetButtonRotateA*/0      , /*GoButtonRotateA*/0));  //2013-04-12    Dell :旋轉站;馬達版
    TechPara.push_back(new TECH_PARA(&Tech.M_Out_iRotateA                   ,   MOutRotateKit   , /*setEditRotateOutA*/0         , "setEditRotateOutA"           , /*SetButtonRotateOutA*/0   , /*GoButtonRotateOutA*/0));
    TechPara.push_back(new TECH_PARA(&Tech.M_In_iRotatePick                 ,   MInArmZE        , /*SetEditPickInRotate*/0       , "SetEditPickInRotate"         , /*SetButtonPick3*/0        , /*GoButtonPick3*/0));
    TechPara.push_back(new TECH_PARA(&Tech.M_In_iRotatePlace                ,   MInArmZE        , /*SetEditPlaceInRotate*/0      , "SetEditPlaceInRotate"        , /*SetButtonPlace4*/0       , /*GoButtonPlace4*/0));
    TechPara.push_back(new TECH_PARA(&Tech.M_Out_iRotatePick                ,   MOutArmZE       , /*SetEditPickOutRotate*/0      , "SetEditPickOutRotate"        , /*SetButtonPick5*/0        , /*GoButtonPick5*/0));
    TechPara.push_back(new TECH_PARA(&Tech.M_Out_iRotatePlace               ,   MOutArmZE       , /*SetEditPlaceOutRotate*/0     , "SetEditPlaceOutRotate"       , /*SetButtonPlace5*/0       , /*GoButtonPlace5*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[0][0]                  ,   MInRotateKit    , /*setEditInRA*/0               , "setEditInRA"                 , /*SetButtonInRA*/0         , /*GoButtonInRA*/0));  //Steven 20170329 (Wei) : Add individual rotate motor
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[1][0]                  ,   MInRotateB      , /*setEditInRB*/0               , "setEditInRB"                 , /*SetButtonInRB*/0         , /*GoButtonInRB*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[0][1]                  ,   MInRotateC      , /*setEditInRC*/0               , "setEditInRC"                 , /*SetButtonInRC*/0         , /*GoButtonInRC*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[1][1]                  ,   MInRotateD      , /*setEditInRD*/0               , "setEditInRD"                 , /*SetButtonInRD*/0         , /*GoButtonInRD*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[0][2]                  ,   MInRotateE      , /*setEditInRE*/0               , "setEditInRE"                 , /*SetButtonInRE*/0         , /*GoButtonInRE*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[1][2]                  ,   MInRotateF      , /*setEditInRF*/0               , "setEditInRF"                 , /*SetButtonInRF*/0         , /*GoButtonInRF*/0));
    // golden :542
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[0][3]                  ,   MInRotateG      , /*setEditInRG*/0               , "setEditInRG"                 , /*SetButtonInRG*/0         , /*GoButtonInRG*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInRotate[1][3]                  ,   MInRotateH      , /*setEditInRH*/0               , "setEditInRH"                 , /*SetButtonInRH*/0         , /*GoButtonInRH*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[0][0]                 ,   MOutRotateKit   , /*setEditOutRA*/0              , "setEditOutRA"                , /*SetButtonOutRA*/0        , /*GoButtonOutRA*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[1][0]                 ,   MOutRotateB     , /*setEditOutRB*/0              , "setEditOutRB"                , /*SetButtonOutRB*/0        , /*GoButtonOutRB*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[0][1]                 ,   MOutRotateC     , /*setEditOutRC*/0              , "setEditOutRC"                , /*SetButtonOutRC*/0        , /*GoButtonOutRC*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[1][1]                 ,   MOutRotateD     , /*setEditOutRD*/0              , "setEditOutRD"                , /*SetButtonOutRD*/0        , /*GoButtonOutRD*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[0][2]                 ,   MOutRotateE     , /*setEditOutRE*/0              , "setEditOutRE"                , /*SetButtonOutRE*/0        , /*GoButtonOutRE*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[1][2]                 ,   MOutRotateF     , /*setEditOutRF*/0              , "setEditOutRF"                , /*SetButtonOutRF*/0        , /*GoButtonOutRF*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[0][3]                 ,   MOutRotateG     , /*setEditOutRG*/0              , "setEditOutRG"                , /*SetButtonOutRG*/0        , /*GoButtonOutRG*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutRotate[1][3]                 ,   MOutRotateH     , /*setEditOutRH*/0              , "setEditOutRH"                , /*SetButtonOutRH*/0        , /*GoButtonOutRH*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iSH1Laser                        ,   MInShuttle1     , /*edtSetSht1Laser*/0           , "edtSetSht1Laser"             , /*btnSetSht1Laser*/0       , /*btnGoSht1Laser*/0));  //Steven 20140228 : 雷射測距功能
    TechPara.push_back(new TECH_PARA(&Tech.iSH2Laser                        ,   MInShuttle2     , /*edtSetSht2Laser*/0           , "edtSetSht2Laser"             , /*btnSetSht2Laser*/0       , /*btnGoSht2Laser*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iOutArmPlaceFix2Z1               ,   MOutArmZE       , /*SetEditPlaceFix2*/0          , "SetEditPlaceFix2"            , /*SetButtonPlace6*/0       , /*GoButtonPlace6*/0));  //ChungHung 20140722 add for HT9046LA

    TechPara.push_back(new TECH_PARA(&Tech.M_iTopView_Pick                  ,   MOutArmZE       , /*setEditTopView_Pick*/0       , "setEditTopView_Pick"         , /*SetBtnTopView_Pick*/0    , /*GoBtnTopView_Pick*/0, false));  //wei 20160617 Vitrox   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    TechPara.push_back(new TECH_PARA(&Tech.M_iTopView_Place                 ,   MOutArmZE       , /*setEditTopView_Place*/0      , "setEditTopView_Place"        , /*SetBtnTopView_Place*/0   , /*GoBtnTopView_Place*/0, false));  //wei 20160617 Vitrox
    TechPara.push_back(new TECH_PARA(&Tech.M_iPADView_Z                     ,   MOutArmZE       , /*setEditPADViewZ*/0           , "setEditPADViewZ"             , /*SetBtnPADView_Z*/0       , /*GoBtnPADView_Z*/0));  //wei 20160617 Vitrox
    TechPara.push_back(new TECH_PARA(&Tech.M_iBGAView_Z                     ,   MOutArmZE       , /*setEditBGAViewZ*/0           , "setEditBGAViewZ"             , /*SetBtnBGAView_Z*/0       , /*GoBtnBGAView_Z*/0));  //wei 20160617 Vitrox
    TechPara.push_back(new TECH_PARA(&Tech.M_iTopViewKit_Zup                ,   MAOIKit         , /*setEditTopViewKitZup*/0      , "setEditTopViewKitZup"        , /*SetBtnTopViewKit_Zup*/0  , /*GoBtnTopViewKit_Zup*/0, false));  //wei 20160617 Vitrox
    TechPara.push_back(new TECH_PARA(&Tech.M_iTopView_KitZ                  ,   MAOIKit         , /*setEditTopViewKitZ*/0        , "setEditTopViewKitZ"          , /*SetBtnTopViewKit_Z*/0    , /*GoBtnTopViewKit_Z*/0, false));  //wei 20160617 Vitrox

    TechPara.push_back(new TECH_PARA(&Tech.iShuttle1120Pitch                ,   MShuttle1Pitch  , /*setEditSht1Pitch120*/0       , "setEditSht1Pitch120"         , /*SetButton153*/0          , /*GoButton153*/0));  //wei 20160914 Auto Shuttle Sensor
    TechPara.push_back(new TECH_PARA(&Tech.iShuttle1180Pitch                ,   MShuttle1Pitch  , /*setEditSht1Pitch180*/0       , "setEditSht1Pitch180"         , /*SetButton154*/0          , /*GoButton154*/0));  //wei 20160914 Auto Shuttle Sensor
    // golden :567
    TechPara.push_back(new TECH_PARA(&Tech.iShuttle2120Pitch                ,   MShuttle2Pitch  , /*setEditSht2Pitch120*/0       , "setEditSht2Pitch120"         , /*SetButton155*/0          , /*GoButton155*/0));  //wei 20160914 Auto Shuttle Sensor
    TechPara.push_back(new TECH_PARA(&Tech.iShuttle2180Pitch                ,   MShuttle2Pitch  , /*setEditSht2Pitch180*/0       , "setEditSht2Pitch180"         , /*SetButton156*/0          , /*GoButton156*/0));  //wei 20160914 Auto Shuttle Sensor

    TechPara.push_back(new TECH_PARA(&Tech.iTrayMapping                     ,   MTrayX          , /*setEditTrayMapX*/0           , "setEditTrayMapX"             , /*SetButton160*/0          , /*GoButton160*/0));  //wei 20161219 Tray Mapping
    TechPara.push_back(new TECH_PARA(&Tech.iTrayID                          ,   MTrayX          , /*setEditTrayIDX*/0            , "setEditTrayIDX"              , /*SetButton161*/0          , /*GoButton161*/0));  //wei 20161219 Tray Mapping

    TechPara.push_back(new TECH_PARA(&Tech.iLoadHingeR[0]                   ,   MLoadHingeR     , /*setEditHingeRotateLoader*/0  , "setEditHingeRotateLoader"    , /*SetButton162*/0          , /*GoButton162*/0));  //wei 20170405
    TechPara.push_back(new TECH_PARA(&Tech.iLoadHingeR[1]                   ,   MLoadHingeR     , /*setEditHingeRotateEmpty*/0   , "setEditHingeRotateEmpty"     , /*SetButton163*/0          , /*GoButton163*/0));  //wei 20170405

    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[0]                 ,   MCasArmX        , /*setEditBuffer1X*/0           , "setEditBuffer1X"             , /*SetBtnBuffer1X*/0        , /*GoBtnBuffer1X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[1]                 ,   MCasArmX        , /*setEditBuffer2X*/0           , "setEditBuffer2X"             , /*SetBtnBuffer2X*/0        , /*GoBtnBuffer2X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[2]                 ,   MCasArmX        , /*setEditBuffer3X*/0           , "setEditBuffer3X"             , /*SetBtnBuffer3X*/0        , /*GoBtnBuffer3X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[3]                 ,   MCasArmX        , /*setEditBuffer4X*/0           , "setEditBuffer4X"             , /*SetBtnBuffer4X*/0        , /*GoBtnBuffer4X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[4]                 ,   MCasArmX        , /*setEditBuffer5X*/0           , "setEditBuffer5X"             , /*SetBtnBuffer5X*/0        , /*GoBtnBuffer5X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[5]                 ,   MCasArmX        , /*setEditBuffer6X*/0           , "setEditBuffer6X"             , /*SetBtnBuffer6X*/0        , /*GoBtnBuffer6X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[6]                 ,   MCasArmX        , /*setEditBuffer7X*/0           , "setEditBuffer7X"             , /*SetBtnBuffer7X*/0        , /*GoBtnBuffer7X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[7]                 ,   MCasArmX        , /*setEditBuffer8X*/0           , "setEditBuffer8X"             , /*SetBtnBuffer8X*/0        , /*GoBtnBuffer8X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[8]                 ,   MCasArmX        , /*setEditBuffer9X*/0           , "setEditBuffer9X"             , /*SetBtnBuffer9X*/0        , /*GoBtnBuffer9X*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmX[9]                 ,   MCasArmX        , /*setEditBuffer10X*/0          , "setEditBuffer10X"            , /*SetBtnBuffer10X*/0       , /*GoBtnBuffer10X*/0));  //wei 20180702 MR

    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[0]                 ,   MCasArmZ        , /*setEditBuffer1Z*/0           , "setEditBuffer1Z"             , /*SetBtnBuffer1Z*/0        , /*GoBtnBuffer1Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[1]                 ,   MCasArmZ        , /*setEditBuffer2Z*/0           , "setEditBuffer2Z"             , /*SetBtnBuffer2Z*/0        , /*GoBtnBuffer2Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[2]                 ,   MCasArmZ        , /*setEditBuffer3Z*/0           , "setEditBuffer3Z"             , /*SetBtnBuffer3Z*/0        , /*GoBtnBuffer3Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[3]                 ,   MCasArmZ        , /*setEditBuffer4Z*/0           , "setEditBuffer4Z"             , /*SetBtnBuffer4Z*/0        , /*GoBtnBuffer4Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[4]                 ,   MCasArmZ        , /*setEditBuffer5Z*/0           , "setEditBuffer5Z"             , /*SetBtnBuffer5Z*/0        , /*GoBtnBuffer5Z*/0));  //wei 20180702 MR
    // golden :592
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[5]                 ,   MCasArmZ        , /*setEditBuffer6Z*/0           , "setEditBuffer6Z"             , /*SetBtnBuffer6Z*/0        , /*GoBtnBuffer6Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[6]                 ,   MCasArmZ        , /*setEditBuffer7Z*/0           , "setEditBuffer7Z"             , /*SetBtnBuffer7Z*/0        , /*GoBtnBuffer7Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[7]                 ,   MCasArmZ        , /*setEditBuffer8Z*/0           , "setEditBuffer8Z"             , /*SetBtnBuffer8Z*/0        , /*GoBtnBuffer8Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[8]                 ,   MCasArmZ        , /*setEditBuffer9Z*/0           , "setEditBuffer9Z"             , /*SetBtnBuffer9Z*/0        , /*GoBtnBuffer9Z*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iCassetteArmZ[9]                 ,   MCasArmZ        , /*setEditBuffer10Z*/0          , "setEditBuffer10Z"            , /*SetBtnBuffer10Z*/0       , /*GoBtnBuffer10Z*/0));  //wei 20180702 MR

    TechPara.push_back(new TECH_PARA(&Tech.iLoadPortZ[0]                    ,   MCaselevatorZ   , /*setEditLoadPortZ*/0          , "setEditLoadPortZ"            , /*SetBtnLoadPortZ*/0       , /*GoBtnLoadPortZ*/0));  //wei 20180702 MR
    TechPara.push_back(new TECH_PARA(&Tech.iLoadPortZ[1]                    ,   MCaselevatorZ   , /*setEditLoadSafeZ*/0          , "setEditLoadSafeZ"            , /*SetBtnLoadSafeZ*/0       , /*GoBtnLoadSafeZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iLoadPortZ[2]                    ,   MCaselevatorZ   , /*setEditLoadTemporaryZ*/0     , "setEditLoadTemporaryZ"       , /*SetBtnLoadTemporaryZ*/0  , /*GoBtnLoadTemporaryZ*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[0]                 ,   MStackedTrayX   , /*setEditStackedLoaderX*/0     , "setEditStackedLoaderX"       , /*SetBtnStackedLoaderX*/0  , /*GoBtnStackedLoaderX*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[1]                 ,   MStackedTrayX   , /*setEditStackedEmptyX*/0      , "setEditStackedEmptyX"        , /*SetBtnStackedEmptyX*/0   , /*GoBtnStackedEmptyX*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[2]                 ,   MStackedTrayX   , /*setEditStackedConversionX*/0 , "setEditStackedConversionX"   , /*SetBtnStackedConversionX*/0, /*GoBtnStackedConversionX*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[3]                 ,   MStackedTrayX   , /*setEditStackedAuto1X*/0      , "setEditStackedAuto1X"        , /*SetBtnStackedAuto1X*/0   , /*GoBtnStackedAuto1X*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[4]                 ,   MStackedTrayX   , /*setEditStackedAuto2X*/0      , "setEditStackedAuto2X"        , /*SetBtnStackedAuto2X*/0   , /*GoBtnStackedAuto2X*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayX[5]                 ,   MStackedTrayX   , /*setEditStackedAuto3X*/0      , "setEditStackedAuto3X"        , /*SetBtnStackedAuto3X*/0   , /*GoBtnStackedAuto3X*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[0]                 ,   MStackedTrayZ   , /*setEditStackedLoaderZ*/0     , "setEditStackedLoaderZ"       , /*SetBtnStackedLoaderZ*/0  , /*GoBtnStackedLoaderZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[1]                 ,   MStackedTrayZ   , /*setEditStackedEmptyZ*/0      , "setEditStackedEmptyZ"        , /*SetBtnStackedEmptyZ*/0   , /*GoBtnStackedEmptyZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[2]                 ,   MStackedTrayZ   , /*setEditStackedConversionZ*/0 , "setEditStackedConversionZ"   , /*SetBtnStackedConversionZ*/0, /*GoBtnStackedConversionZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[3]                 ,   MStackedTrayZ   , /*setEditStackedAuto1Z*/0      , "setEditStackedAuto1Z"        , /*SetBtnStackedAuto1Z*/0   , /*GoBtnStackedAuto1Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[4]                 ,   MStackedTrayZ   , /*setEditStackedAuto2Z*/0      , "setEditStackedAuto2Z"        , /*SetBtnStackedAuto2Z*/0   , /*GoBtnStackedAuto2Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iStackedTrayZ[5]                 ,   MStackedTrayZ   , /*setEditStackedAuto3Z*/0      , "setEditStackedAuto3Z"        , /*SetBtnStackedAuto3Z*/0   , /*GoBtnStackedAuto3Z*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iTrayBracketZ[0]                 ,   MTrayBracketZ   , /*setEditTrayBracketSaftZ*/0   , "setEditTrayBracketSaftZ"     , /*SetBtnTrayBracketSaftZ*/0, /*GoBtnTrayBracketSaftZ*/0));
    // golden :617
    TechPara.push_back(new TECH_PARA(&Tech.iTrayBracketZ[1]                 ,   MTrayBracketZ   , /*setEditTrayBracketConversionZ*/0, "setEditTrayBracketConversionZ", /*SetBtnTrayBracketConversionZ*/0, /*GoBtnTrayBracketConversionZ*/0));

/*
    TechPara.push_back(new TECH_PARA(&Tech.iLoadRobotZ[0]                   ,   MLoadRobotZ     , setEditLoadPortBufferZ    , "setEditLoadPortBufferZ"      , SetBtnLoadPortBufferZ , GoBtnLoadPortBufferZ));  //Sam 20190112 LM
    TechPara.push_back(new TECH_PARA(&Tech.iLoadRobotZ[1]                   ,   MLoadRobotZ     , setEditLoadPort1Z         , "setEditLoadPort1Z"           , SetBtnLoadPort1Z      , GoBtnLoadPort1Z));
    TechPara.push_back(new TECH_PARA(&Tech.iLoadRobotZ[2]                   ,   MLoadRobotZ     , setEditLoadPort2Z         , "setEditLoadPort2Z"           , SetBtnLoadPort2Z      , GoBtnLoadPort2Z));
    TechPara.push_back(new TECH_PARA(&Tech.iLoadRobotZ[3]                   ,   MLoadRobotZ     , setEditLoadPort3Z         , "setEditLoadPort3Z"           , SetBtnLoadPort3Z      , GoBtnLoadPort3Z));
    TechPara.push_back(new TECH_PARA(&Tech.iLoadRobotZ[4]                   ,   MLoadRobotZ     , setEditLoadPort4Z         , "setEditLoadPort4Z"           , SetBtnLoadPort4Z      , GoBtnLoadPort4Z));
*/
    TechPara.push_back(new TECH_PARA(&Tech.iLDCassetteFront                 ,   MLoaderY        , /*SetEditLDFront*/0             , "SetEditLDFront"              , /*SetLoaderCassetteFront*/0     , /*GoLoaderCassetteFront*/0));  //Frank 20251217 add
    TechPara.push_back(new TECH_PARA(&Tech.iLDCassetteFrontBack             ,   MLoaderY        , /*SetEditLDFrontBack*/0         , "SetEditLDFrontBack"          , /*SetLoaderCassetteFrontBack*/0 , /*GoLoaderCassetteFrontBack*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iLDCassetteRear                  ,   MLoaderY_CCW    , /*SetEditLDRear*/0              , "SetEditLDRear"               , /*SetLoaderCassetteRear*/0      , /*GoLoaderCassetteRear*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iLDCassetteRearBack              ,   MLoaderY_CCW    , /*SetEditLDRearBack*/0          , "SetEditLDRearBack"           , /*SetLoaderCassetteRearBack*/0  , /*GoLoaderCassetteRearBack*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iLDCassetteZStart                ,   MTrayZ          , /*SetEditLDCassetteZStart*/0    , "SetEditLDCassetteZStart"     , /*SetLoaderCassetteZStart*/0    , /*GoLoaderCassetteZStart*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteFront[0]            ,   MAuto1Y         , /*SetEditAuto1Front*/0          , "SetEditAuto1Front"           , /*SetAuto1CassetteFront*/0      , /*GoAuto1CassetteFront*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteFrontBack[0]        ,   MAuto1Y         , /*SetEditAuto1FrontBack*/0      , "SetEditAuto1FrontBack"       , /*SetAuto1CassetteFrontBack*/0  , /*GoAuto1CassetteFrontBack*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteRear[0]             ,   MAuto1Y_CCW     , /*SetEditAuto1Rear*/0           , "SetEditAuto1Rear"            , /*SetAuto1CassetteRear*/0       , /*GoAuto1CassetteRear*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteRearBack[0]         ,   MAuto1Y_CCW     , /*SetEditAuto1RearBack*/0       , "SetEditAuto1RearBack"        , /*SetAuto1CassetteRearBack*/0   , /*GoAuto1CassetteRearBack*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteZStart[0]           ,   MAuto1Z         , /*SetEditAuto1CassetteZStart*/0 , "SetEditAuto1CassetteZStart"  , /*SetAuto1CassetteZStart*/0     , /*GoAuto1CassetteZStart*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteFront[1]            ,   MAuto2Y         , /*SetEditAuto2Front*/0          , "SetEditAuto2Front"           , /*SetAuto2CassetteFront*/0      , /*GoAuto2CassetteFront*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteFrontBack[1]        ,   MAuto2Y         , /*SetEditAuto2FrontBack*/0      , "SetEditAuto2FrontBack"       , /*SetAuto2CassetteFrontBack*/0  , /*GoAuto2CassetteFrontBack*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteRear[1]             ,   MAuto2Y_CCW     , /*SetEditAuto2Rear*/0           , "SetEditAuto2Rear"            , /*SetAuto2CassetteRear*/0       , /*GoAuto2CassetteRear*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteRearBack[1]         ,   MAuto2Y_CCW     , /*SetEditAuto2RearBack*/0       , "SetEditAuto2RearBack"        , /*SetAuto2CassetteRearBack*/0   , /*GoAuto2CassetteRearBack*/0));
    // golden :642
    TechPara.push_back(new TECH_PARA(&Tech.iAutoCassetteZStart[1]           ,   MAuto2Z         , /*SetEditAuto2CassetteZStart*/0 , "SetEditAuto2CassetteZStart"  , /*SetAuto2CassetteZStart*/0     , /*GoAuto2CassetteZStart*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iUnloadRobotZ[0]                 ,   MUnloadRobotZ   , /*setEditUnloadPortBufferZ*/0  , "setEditUnloadPortBufferZ"    , /*SetBtnUnloadPortBufferZ*/0, /*GoBtnUnloadPortBufferZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iUnloadRobotZ[1]                 ,   MUnloadRobotZ   , /*setEditUnloadPort1Z*/0       , "setEditUnloadPort1Z"         , /*SetBtnUnloadPort1Z*/0    , /*GoBtnUnloadPort1Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iUnloadRobotZ[2]                 ,   MUnloadRobotZ   , /*setEditUnloadPort2Z*/0       , "setEditUnloadPort2Z"         , /*SetBtnUnloadPort2Z*/0    , /*GoBtnUnloadPort2Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iUnloadRobotZ[3]                 ,   MUnloadRobotZ   , /*setEditUnloadPort3Z*/0       , "setEditUnloadPort3Z"         , /*SetBtnUnloadPort3Z*/0    , /*GoBtnUnloadPort3Z*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iUnloadRobotZ[4]                 ,   MUnloadRobotZ   , /*setEditUnloadPort4Z*/0       , "setEditUnloadPort4Z"         , /*SetBtnUnloadPort4Z*/0    , /*GoBtnUnloadPort4Z*/0));

    TechPara.push_back(new TECH_PARA(&Teach.iContactZ1Relative              ,   MTestZ1         ,/*seteditContactZ1Relative*/0   , "seteditContactZ1Relative"    , /*SetButton077*/0          ,/*GoButton077*/0));
    TechPara.push_back(new TECH_PARA(&Teach.iContactZ2Relative              ,   MTestZ2         ,/*seteditContactZ2Relative*/0   , "seteditContactZ2Relative"    , /*SetButton078*/0          ,/*GoButton078*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iOutArmBinBoxZ                   ,   MOutArmZE       , /*edtBinBoxZ*/0                , "edtBinBoxZ"                  , /*btnSetBinBoxZ*/0         , /*btnGoBinBoxZ*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iFix3PosL                        ,   MFix3Full       , /*setFix3L*/0                  , "setFix3L"                    , /*setBtnFix3L*/0           , /*GoBtnFix3L*/0));  //JimmyChiu 20220927 : Stepper Motor Control in Fix3
    TechPara.push_back(new TECH_PARA(&Tech.iFix3PosR                        ,   MFix3Full       , /*setFix3R*/0                  , "setFix3R"                    , /*setBtnFix3R*/0           , /*GoBtnFix3R*/0));

    TechPara.push_back(new TECH_PARA(&Tech.M_ScannerAOI_Z                   ,   MOutArmZE       , /*setEditScannerAOIZ*/0        , "setEditScannerAOIZ"          , /*SetBtnScannerAOI_Z*/0    , /*GoBtnScannerAOI_Z*/0));  //Ifor 20191211 : add Scanner AOI
    TechPara.push_back(new TECH_PARA(&Tech.iMagazineTray1Pos                ,   MMagazine       , /*edtEditMagZTray1*/0          , "edtEditMagZTray1"            , /*setBtnMagZTray1*/0       , /*GoBtnMagZTray1*/0));  //JerryYang 20220909 : add magazine
    TechPara.push_back(new TECH_PARA(&Tech.iCatchMazTray_Front              ,   MCatchMgzTray   , /*setEditCatchMagFront*/0      , "setEditCatchMagFront"        , /*sbCatchMagFront*/0       , /*sbGoCatchMagFront*/0));  //JerryYang 20220909 : add magazine
    TechPara.push_back(new TECH_PARA(&Tech.iCatchMazTray_Rear               ,   MCatchMgzTray   , /*setEditCatchMagRear*/0       , "setEditCatchMagRear"         , /*sbCatchMagRear*/0        , /*sbGoCatchMagRear*/0));  //JerryYang 20220909 : add magazine

    TechPara.push_back(new TECH_PARA(&Tech.iInArmNGBinBoxPlaceZ             ,   MInArmZE        , /*SetEditPlaceNGBinBoxZ*/0     , "SetEditPlaceNGBinBoxZ"       , /*SetButtonPlaceNGBinBox*/0, /*GoButtonPlaceNGBinBox*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][0], MInArmZA        , /*setEditAlignInZAa*/0         , "setEditAlignInZAa"           , /*sbAlignInZAa*/0          , /*sbGoAlignInZAa*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][1], MInArmZC        , /*setEditAlignInZAb*/0         , "setEditAlignInZAb"           , /*sbAlignInZAb*/0          , /*sbGoAlignInZAb*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][2], MInArmZE        , /*setEditAlignInZAc*/0         , "setEditAlignInZAc"           , /*sbAlignInZAc*/0          , /*sbGoAlignInZAc*/0));
    // golden :667
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][3], MInArmZG        , /*setEditAlignInZAd*/0         , "setEditAlignInZAd"           , /*sbAlignInZAd*/0          , /*sbGoAlignInZAd*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][0], MInArmZB        , /*setEditAlignInZBa*/0         , "setEditAlignInZBa"           , /*sbAlignInZBa*/0          , /*sbGoAlignInZBa*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][1], MInArmZD        , /*setEditAlignInZBb*/0         , "setEditAlignInZBb"           , /*sbAlignInZBb*/0          , /*sbGoAlignInZBb*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][2], MInArmZF        , /*setEditAlignInZBc*/0         , "setEditAlignInZBc"           , /*sbAlignInZBc*/0          , /*sbGoAlignInZBc*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][3], MInArmZH        , /*setEditAlignInZBd*/0         , "setEditAlignInZBd"           , /*sbAlignInZBd*/0          , /*sbGoAlignInZBd*/0));

    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][0], MOutArmZA      , /*setEditAlignOutZAa*/0        , "setEditAlignOutZAa"          , /*sbAlignOutZAa*/0         , /*sbGoAlignOutZAa*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][1], MOutArmZC      , /*setEditAlignOutZAb*/0        , "setEditAlignOutZAb"          , /*sbAlignOutZAb*/0         , /*sbGoAlignOutZAb*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][2], MOutArmZE      , /*setEditAlignOutZAc*/0        , "setEditAlignOutZAc"          , /*sbAlignOutZAc*/0         , /*sbGoAlignOutZAc*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][3], MOutArmZG      , /*setEditAlignOutZAd*/0        , "setEditAlignOutZAd"          , /*sbAlignOutZAd*/0         , /*sbGoAlignOutZAd*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][0], MOutArmZB      , /*setEditAlignOutZBa*/0        , "setEditAlignOutZBa"          , /*sbAlignOutZBa*/0         , /*sbGoAlignOutZBa*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][1], MOutArmZD      , /*setEditAlignOutZBb*/0        , "setEditAlignOutZBb"          , /*sbAlignOutZBb*/0         , /*sbGoAlignOutZBb*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][2], MOutArmZF      , /*setEditAlignOutZBc*/0        , "setEditAlignOutZBc"          , /*sbAlignOutZBc*/0         , /*sbGoAlignOutZBc*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][3], MOutArmZH      , /*setEditAlignOutZBd*/0        , "setEditAlignOutZBd"          , /*sbAlignOutZBd*/0         , /*sbGoAlignOutZBd*/0));

    if(USE_PICKER_COUNT==ep16Picker)                                            //Ztex 2023.12.06 Add HT-1032
    {
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][4],  MInArmZA   , /*setEditAlignInZAe*/0         , "setEditAlignInZAe"           , /*sbAlignInZAe*/0          , /*sbGoAlignInZAe*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][5],  MInArmZC   , /*setEditAlignInZAf*/0         , "setEditAlignInZAf"           , /*sbAlignInZAf*/0          , /*sbGoAlignInZAf*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][6],  MInArmZE   , /*setEditAlignInZAg*/0         , "setEditAlignInZAg"           , /*sbAlignInZAg*/0          , /*sbGoAlignInZAg*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[0][7],  MInArmZG   , /*setEditAlignInZAh*/0         , "setEditAlignInZAh"           , /*sbAlignInZAh*/0          , /*sbGoAlignInZAh*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][4],  MInArmZB   , /*setEditAlignInZBe*/0         , "setEditAlignInZBe"           , /*sbAlignInZBe*/0          , /*sbGoAlignInZBe*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][5],  MInArmZD   , /*setEditAlignInZBf*/0         , "setEditAlignInZBf"           , /*sbAlignInZBf*/0          , /*sbGoAlignInZBf*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][6],  MInArmZF   , /*setEditAlignInZBg*/0         , "setEditAlignInZBg"           , /*sbAlignInZBg*/0          , /*sbGoAlignInZBg*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iInArmZBasePickerAlignmentPos[1][7],  MInArmZH   , /*setEditAlignInZBh*/0         , "setEditAlignInZBh"           , /*sbAlignInZBh*/0          , /*sbGoAlignInZBh*/0));
    // golden :692

        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][4], MOutArmZA  , /*setEditAlignOutZAe*/0        , "setEditAlignOutZAe"          , /*sbAlignOutZAe*/0         , /*sbGoAlignOutZAe*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][5], MOutArmZC  , /*setEditAlignOutZAf*/0        , "setEditAlignOutZAf"          , /*sbAlignOutZAf*/0         , /*sbGoAlignOutZAf*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][6], MOutArmZE  , /*setEditAlignOutZAg*/0        , "setEditAlignOutZAg"          , /*sbAlignOutZAg*/0         , /*sbGoAlignOutZAg*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[0][7], MOutArmZG  , /*setEditAlignOutZAh*/0        , "setEditAlignOutZAh"          , /*sbAlignOutZAh*/0         , /*sbGoAlignOutZAh*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][4], MOutArmZB  , /*setEditAlignOutZBe*/0        , "setEditAlignOutZBe"          , /*sbAlignOutZBe*/0         , /*sbGoAlignOutZBe*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][5], MOutArmZD  , /*setEditAlignOutZBf*/0        , "setEditAlignOutZBf"          , /*sbAlignOutZBf*/0         , /*sbGoAlignOutZBf*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][6], MOutArmZF  , /*setEditAlignOutZBg*/0        , "setEditAlignOutZBg"          , /*sbAlignOutZBg*/0         , /*sbGoAlignOutZBg*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmZBasePickerAlignmentPos[1][7], MOutArmZH  , /*setEditAlignOutZBh*/0        , "setEditAlignOutZBh"          , /*sbAlignOutZBh*/0         , /*sbGoAlignOutZBh*/0));
    }

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250416 for HT9046AU add
    {
        TechPara.push_back(new TECH_PARA(&Tech.iSortShuttleLeft      ,   MOutSortSht     , /*edtOutSortSht_L*/0              , "edtOutSortSht_L"           , /*btnSetSortShtL*/0              , /*btnGoSortShtL*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortShuttleRight     ,   MOutSortSht     , /*edtOutSortSht_R*/0              , "edtOutSortSht_R"           , /*btnSetSortShtR*/0              , /*btnGoSortShtR*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortArmXPitchMin     ,   MOutSortPitchX  , /*edtsetSortXPitchMin*/0          , "edtsetSortXPitchMin"       , /*btnSetSortArmXMin*/0           , /*btnGoSortArmXMin*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortArmXPitchMax     ,   MOutSortPitchX  , /*edtsetSortXPitchMax*/0          , "edtsetSortXPitchMax"       , /*btnSetSortArmXMax*/0           , /*btnGoSortArmXMax*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortArmShuttlePickZ  ,   MOutSortAb      , /*edtSetPickSortSHT*/0            , "edtSetPickSortSHT"         , /*btnSetSortShtPick*/0           , /*btnGoSortShtPick*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortArmAutoPlaceZ    ,   MOutSortAb      , /*edtSetPlaceSortAuto*/0          , "edtSetPlaceSortAuto"       , /*btnSetAutoPlace*/0             , /*btnGoAutoPlace*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iSortArmSafeZ1        ,   MOutSortAb      , /*edtSetSortZSafeHeight*/0        , "edtSetSortZSafeHeight"     , /*btnSetSortZSafeHeight*/0       , /*btnGoSortZSafeHeight*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iOutArmToSortShtPlaceZ,   MOutArmZE       , /*edtSetOutArmToSortShtPlace*/0   , "edtSetOutArmToSortShtPlace", /*btnSetOutArmToSortShtPlace*/0  , /*btnGoOutArmToSortShtPlace*/0));
    }

    TechPara.push_back(new TECH_PARA(&Tech.iMagazineStandbyPos,  MMagazine,     /*edtEditMagZStandby*/0, "edtEditMagZStandby",  /*setBtnMagZTrayStandby*/0,  /*GoBtnMagZTrayStandby*/0));  //Ifor 20240102 add: Magazine Tray Standby Pos

    // golden :717
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmX              , /*MotorInArmX*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmY              , /*MotorInArmY*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitch          , /*MotorInArmPitchX*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitch          , /*btnInXPitch1*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZA             , /*MotorInArmZA*/0));
    if(InOutArmPickerUseMotor!=eptUseMotCyn)
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZB         , /*MotorInArmZB*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZC         , /*MotorInArmZC*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZD         , /*MotorInArmZD*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZE         , /*MotorInArmZE*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZF         , /*MotorInArmZF*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZG         , /*MotorInArmZG*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZH         , /*MotorInArmZH*/0));
    }

    if(USE_PICKER_COUNT==ep16Picker &&                                          //Ztex 2023.12.06 Add HT-1032
       InOutArmPickerUseMotor!=eptUseMotCyn)
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZAe        , /*MotorInArmZAe*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZBe        , /*MotorInArmZAf*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZAf        , /*MotorInArmZAg*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZBf        , /*MotorInArmZAh*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZAg        , /*MotorInArmZBe*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZBg        , /*MotorInArmZBf*/0));
    // golden :742
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZAh        , /*MotorInArmZBg*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmZBh        , /*MotorInArmZBh*/0));
    }

    TechMotorAxle.push_back(new TECH_MotorAxle(MInShuttle1          , /*MotorInSh1*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInShuttle2          , /*MotorInSh2*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MTestY1              , /*MotorIndexArm1Y*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MTestZ1              , /*MotorIndexArm1Z*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MTestZ2              , /*MotorIndexArm2Z*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MTestY2              , /*MotorIndexArm2Y*/0));
/*  #ifdef Carry4
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutShuttle1         , MotorOutSh1));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutShuttle2         , MotorOutSh2));
  #else*/
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutShuttle1         , /*MotorOutSh1*/0           , false));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutShuttle2         , /*MotorOutSh2*/0           , false));
//  #endif
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmX             , /*MotorOutArmX*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmY             , /*MotorOutArmY*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitch         , /*MotorOutArmPitchX*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitch         , /*btnOutXPitch1*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZA            , /*MotorOutArmZA*/0));

    if(USE_PICKER_COUNT==ep16Picker)                                            //Steven 20240502 : For HT-1032
    {
    // [W906-TEACH-W1 T2 widget-only] golden :767
    //        MotorInArmZA->Caption="Z Aa";   MotorOutArmZA->Caption="Z Aa";
    // [W906-TEACH-W1 T2 widget-only] golden :768
    //        MotorInArmZC->Caption="Z Ab";   MotorOutArmZC->Caption="Z Ab";
    // [W906-TEACH-W1 T2 widget-only] golden :769
    //        MotorInArmZE->Caption="Z Ac";   MotorOutArmZE->Caption="Z Ac";
    // [W906-TEACH-W1 T2 widget-only] golden :770
    //        MotorInArmZG->Caption="Z Ad";   MotorOutArmZG->Caption="Z Ad";
    // [W906-TEACH-W1 T2 widget-only] golden :771
    //        MotorInArmZB->Caption="Z Ba";   MotorOutArmZB->Caption="Z Ba";
    // [W906-TEACH-W1 T2 widget-only] golden :772
    //        MotorInArmZD->Caption="Z Bb";   MotorOutArmZD->Caption="Z Bb";
    // [W906-TEACH-W1 T2 widget-only] golden :773
    //        MotorInArmZF->Caption="Z Bc";   MotorOutArmZF->Caption="Z Bc";
    // [W906-TEACH-W1 T2 widget-only] golden :774
    //        MotorInArmZH->Caption="Z Bd";   MotorOutArmZH->Caption="Z Bd";
    }

    if(InOutArmPickerUseMotor!=eptUseMotCyn)
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZB        , /*MotorOutArmZB*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZC        , /*MotorOutArmZC*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZD        , /*MotorOutArmZD*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZE        , /*MotorOutArmZE*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZF        , /*MotorOutArmZF*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZG        , /*MotorOutArmZG*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZH        , /*MotorOutArmZH*/0));
    }

    if(USE_PICKER_COUNT==ep16Picker &&                                          //Ztex 2023.12.06 Add HT-1032
       InOutArmPickerUseMotor!=eptUseMotCyn)
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZAe       , /*MotorOutArmZAe*/0));
    // golden :792
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZBe       , /*MotorOutArmZAf*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZAf       , /*MotorOutArmZAg*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZBf       , /*MotorOutArmZAh*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZAg       , /*MotorOutArmZBe*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZBg       , /*MotorOutArmZBf*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZAh       , /*MotorOutArmZBg*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmZBh       , /*MotorOutArmZBh*/0));
    }

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250416 for HT9046AU add
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortX            , /*MotorSortArmX*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortY            , /*MotorSortArmY*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortPitchX       , /*MotorSortArmPitchX*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortPitchX       , /*btnSortXPitch*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortAa           , /*MotorSortArmZA*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortAb           , /*MotorSortArmZB*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutSortSht          , /*MotorSortSHT*/0));
    }

    TechMotorAxle.push_back(new TECH_MotorAxle(MTrayX               , /*MotorTrayX*/0));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchY         , /*MotorInArmPitchY*/0      , USE_IN_Y_IS_AUTO_PITCH));                                         //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchY         , /*btnInYPitch*/0           , USE_IN_Y_IS_AUTO_PITCH));                                         //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX2        , /*MotorInArmPitchX2*/0     , USE_IN_Y_IS_AUTO_PITCH));                                         //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX2        , /*btnInXPitch2*/0          , USE_IN_Y_IS_AUTO_PITCH));                                         //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    // golden :817
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchY        , /*MotorOutArmPitchY*/0     , USE_OUT_Y_IS_AUTO_PITCH));                                        //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchY        , /*btnOutYPitch*/0          , USE_OUT_Y_IS_AUTO_PITCH));                                        //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX2       , /*MotorOutArmPitchX2*/0    , USE_OUT_Y_IS_AUTO_PITCH));                                        //Steven 20120706 : 加入OCR與Y變距  //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX2       , /*btnOutXPitch2*/0         , USE_OUT_Y_IS_AUTO_PITCH));                                        //Steven 20120706 : 加入OCR與Y變距  //Ztex 2023.12.06 Add HT-1032 //Ztex 2024.02.24 Add HT-1132
    TechMotorAxle.push_back(new TECH_MotorAxle(MLoaderZ             , /*MotorLoaderZ*/0          , LOAD_Z_USE_MOTOR[0]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MEmptyZ              , /*MotorEmptyZ*/0           , LOAD_Z_USE_MOTOR[1]));    //Steven 20120901 : 加入Tray Z
    TechMotorAxle.push_back(new TECH_MotorAxle(MColorZ              , /*MotorColorZ*/0           , LOAD_Z_USE_MOTOR[2]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto1Z              , /*MotorAuto1Z*/0           , LOAD_Z_USE_MOTOR[3]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto2Z              , /*MotorAuto2Z*/0           , LOAD_Z_USE_MOTOR[4]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto3Z              , /*MotorAuto3Z*/0           , LOAD_Z_USE_MOTOR[5]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto4Z              , /*MotorAuto4Z*/0           , LOAD_Z_USE_MOTOR[6]));    //Steven 20230907 : For HT-9011UC
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto5Z              , /*MotorAuto5Z*/0           , LOAD_Z_USE_MOTOR[7]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto6Z              , /*MotorAuto6Z*/0           , LOAD_Z_USE_MOTOR[8]));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateKit         , /*MotorInRotateKit*/0      , (USE_ROTATE_KIT && iRotate_Type!=eCynRotate)));                   //2013-04-12    Dell :旋轉站;馬達版
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateKit        , /*MotorOutRotateKit*/0     , (USE_ROTATE_KIT && iRotate_Type!=eCynRotate)));

    if(USE_PICKER_COUNT==ep16Picker)                                            //Ztex 2023.12.06 Add HT-1032
    {
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX3    , /*MotorInArmPitchX3*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX4    , /*MotorInArmPitchX4*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX3   , /*MotorOutArmPitchX3*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX4   , /*MotorOutArmPitchX4*/0));

        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX3    , /*btnInXPitch3*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MInArmPitchX4    , /*btnInXPitch4*/0));
    // golden :842
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX3   , /*btnOutXPitch3*/0));
        TechMotorAxle.push_back(new TECH_MotorAxle(MOutArmPitchX4   , /*btnOutXPitch4*/0));
    }

    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateKit         , /*MotorInRA*/0             , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));  //Steven 20170329 (Wei) : Add individual rotate motor
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateB           , /*MotorInRB*/0             , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateC           , /*MotorInRC*/0             , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateD           , /*MotorInRD*/0             , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateE           , /*MotorInRE*/0             , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateF           , /*MotorInRF*/0             , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateG           , /*MotorInRG*/0             , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInRotateH           , /*MotorInRH*/0             , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateKit        , /*MotorOutRA*/0            , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateB          , /*MotorOutRB*/0            , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateC          , /*MotorOutRC*/0            , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateD          , /*MotorOutRD*/0            , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateE          , /*MotorOutRE*/0            , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateF          , /*MotorOutRF*/0            , (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateG          , /*MotorOutRG*/0            , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    TechMotorAxle.push_back(new TECH_MotorAxle(MOutRotateH          , /*MotorOutRH*/0            , (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));

    TechMotorAxle.push_back(new TECH_MotorAxle(MTrayZ               , /*MotorTrayZ*/0            , TRAY_ARM_MODE==eUnderCoveyor));
    TechMotorAxle.push_back(new TECH_MotorAxle(MTopAOIArmX      ,/*MotorTopAOIArmX*/0    , USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));     //Jimmychiu 20240322 : Top & Bottom Inspect
    TechMotorAxle.push_back(new TECH_MotorAxle(MTopAOIArmY      ,/*MotorTopAOIArmY*/0    , USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));     //Jimmychiu 20240322 : Top & Bottom Inspect
    TechMotorAxle.push_back(new TECH_MotorAxle(MTopAOIArmR      ,/*MotorTopAOIArmR*/0    , USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));     //Jimmychiu 20240322 : Top & Bottom Inspect
    // golden :867
    TechMotorAxle.push_back(new TECH_MotorAxle(MTopAOICCDZ      ,/*MotorTopAOICCDZ*/0    , USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall));     //Jimmychiu 20240322 : Top & Bottom Inspect
    TechMotorAxle.push_back(new TECH_MotorAxle(MFix3Full            , /*MotorFix3*/0             , FIX3_FULL_PLACE==Fix3K_UseStepperMotor));                         //JimmyChiu 20220927 : Stepper Motor Control in Fix3

    TechMotorAxle.push_back(new TECH_MotorAxle(MInSh1LtcSenZ1       , /*MotorInSh1LtcZ1*/0       , In_Shuttle_Auto_Latch));  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    TechMotorAxle.push_back(new TECH_MotorAxle(MInSh1LtcSenZ2       , /*MotorInSh1LtcZ2*/0       , In_Shuttle_Auto_Latch));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInSh2LtcSenZ1       , /*MotorInSh2LtcZ1*/0       , In_Shuttle_Auto_Latch));
    TechMotorAxle.push_back(new TECH_MotorAxle(MInSh2LtcSenZ2       , /*MotorInSh2LtcZ2*/0       , In_Shuttle_Auto_Latch));

    TechMotorAxle.push_back(new TECH_MotorAxle(MTrayZ               , /*MotorCassLDZ*/0          , USE_LdUldCassetteMode));  //Ifor 20251216 add:Boat Carrier
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto1Z              , /*MotorCassAuto1Z*/0       , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto2Z              , /*MotorCassAuto2Z*/0       , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MLoaderY             , /*btnLoaderY*/0            , INSTALL_OCR_YMot==eocrYMotInstal ||
                                                                                              (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR) ||    //KenHsieh 20260514 : 補上OCR+皮帶
                                                                                              USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MLoaderY_CCW         , /*MotorLoaderYCCW*/0       , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto1Y              , /*MotorAuto1YCW*/0         , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto1Y_CCW          , /*MotorAuto1YCCW*/0        , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto2Y              , /*MotorAuto2YCW*/0         , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MAuto2Y_CCW          , /*MotorAuto2YCCW*/0        , USE_LdUldCassetteMode));
    TechMotorAxle.push_back(new TECH_MotorAxle(MLdCarRotArm         , /*btnLoaderRotZ*/0         , USE_LD_Rot_Arm));

    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmPreciserX            ,   &Tech.iInArmPreciserY           ,   MInArmX         , MInArmY       , /*setEditPreciserX*/0      , /*setEditPreciserY*/0      , "setEditPreciserX"        , "setEditPreciserY"        , /*SetButtonPreciser*/0 , /*GoButtonPreciser*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmLoadStageX           ,   &Tech.iInArmLoadStageY          ,   MInArmX         , MInArmY       , /*setEditLoaderX*/0        , /*setEditLoaderY*/0        , "setEditLoaderX"          , "setEditLoaderY"          , /*SetButton020*/0      , /*GoButton020*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmPlate1X              ,   &Tech.iInArmPlate1Y             ,   MInArmX         , MInArmY       , /*setEditHP1X*/0           , /*setEditHP1Y*/0           , "setEditHP1X"             , "setEditHP1Y"             , /*SetButton024*/0      , /*GoButton024*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmPlate2X              ,   &Tech.iInArmPlate2Y             ,   MInArmX         , MInArmY       , /*setEditHP2X*/0           , /*setEditHP2Y*/0           , "setEditHP2X"             , "setEditHP2Y"             , /*SetButton026*/0      , /*GoButton026*/0));
    // golden :892
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmShuttle1X            ,   &Tech.iInArmShuttle1Y           ,   MInArmX         , MInArmY       , /*setEditInSht1X*/0        , /*setEditInSht1Y*/0        , "setEditInSht1X"          , "setEditInSht1Y"          , /*SetButton040*/0      , /*GoButton040*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmShuttle2X            ,   &Tech.iInArmShuttle2Y           ,   MInArmX         , MInArmY       , /*setEditInSht2X*/0        , /*setEditInSht2Y*/0        , "setEditInSht2X"          , "setEditInSht2Y"          , /*SetButton042*/0      , /*GoButton042*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmShuttle1X           ,   &Tech.iOutArmShuttle1Y          ,   MOutArmX        , MOutArmY      , /*setEditOutSht1X*/0       , /*setEditOutSht1Y*/0       , "setEditOutSht1X"         , "setEditOutSht1Y"         , /*SetButton080*/0      , /*GoButton080*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmShuttle2X           ,   &Tech.iOutArmShuttle2Y          ,   MOutArmX        , MOutArmY      , /*setEditOutSht2X*/0       , /*setEditOutSht2Y*/0       , "setEditOutSht2X"         , "setEditOutSht2Y"         , /*SetButton082*/0      , /*GoButton082*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto1X              ,   &Tech.iOutArmAuto1Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto1X*/0         , /*setEditAuto1Y*/0         , "setEditAuto1X"           , "setEditAuto1Y"           , /*SetButton100*/0      , /*GoButton100*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto2X              ,   &Tech.iOutArmAuto2Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto2X*/0         , /*setEditAuto2Y*/0         , "setEditAuto2X"           , "setEditAuto2Y"           , /*SetButton102*/0      , /*GoButton102*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto3X              ,   &Tech.iOutArmAuto3Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto3X*/0         , /*setEditAuto3Y*/0         , "setEditAuto3X"           , "setEditAuto3Y"           , /*SetButton104*/0      , /*GoButton104*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto4X              ,   &Tech.iOutArmAuto4Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto4X*/0         , /*setEditAuto4Y*/0         , "setEditAuto4X"           , "setEditAuto4Y"           , /*btnSetBtnAuto4*/0    , /*btnGoBtnAuto4*/0));  //Steven 20230907 : For HT-9011UC
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto5X              ,   &Tech.iOutArmAuto5Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto5X*/0         , /*setEditAuto5Y*/0         , "setEditAuto5X"           , "setEditAuto5Y"           , /*btnSetBtnAuto5*/0    , /*btnGoBtnAuto5*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmAuto6X              ,   &Tech.iOutArmAuto6Y             ,   MOutArmX        , MOutArmY      , /*setEditAuto6X*/0         , /*setEditAuto6Y*/0         , "setEditAuto6X"           , "setEditAuto6Y"           , /*btnSetBtnAuto6*/0    , /*btnGoBtnAuto6*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix1X               ,   &Tech.iOutArmFix1Y              ,   MOutArmX        , MOutArmY      , /*setEditFix1X*/0          , /*setEditFix1Y*/0          , "setEditFix1X"            , "setEditFix1Y"            , /*SetButton120*/0      , /*GoButton120*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix2X               ,   &Tech.iOutArmFix2Y              ,   MOutArmX        , MOutArmY      , /*setEditFix2X*/0          , /*setEditFix2Y*/0          , "setEditFix2X"            , "setEditFix2Y"            , /*SetButton122*/0      , /*GoButton122*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix3X               ,   &Tech.iOutArmFix3Y              ,   MOutArmX        , MOutArmY      , /*setEditFix3X*/0          , /*setEditFix3Y*/0          , "setEditFix3X"            , "setEditFix3Y"            , /*SetButton124*/0      , /*GoButton124*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix4X               ,   &Tech.iOutArmFix4Y              ,   MOutArmX        , MOutArmY      , /*setEditFix4X*/0          , /*setEditFix4Y*/0          , "setEditFix4X"            , "setEditFix4Y"            , /*btnSetBtnFix4*/0     , /*btnGoBtnFix4*/0));  //Steven 20230907 : For HT-9011UC
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix5X               ,   &Tech.iOutArmFix5Y              ,   MOutArmX        , MOutArmY      , /*setEditFix5X*/0          , /*setEditFix5Y*/0          , "setEditFix5X"            , "setEditFix5Y"            , /*btnSetBtnFix5*/0     , /*btnGoBtnFix5*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmFix6X               ,   &Tech.iOutArmFix6Y              ,   MOutArmX        , MOutArmY      , /*setEditFix6X*/0          , /*setEditFix6Y*/0          , "setEditFix6X"            , "setEditFix6Y"            , /*btnSetBtnFix6*/0     , /*btnGoBtnFix6*/0));

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHT9040TestY1_Middle   ,   &Tech.iHT9040TestY1_Middle      ,   MTestY1         , MTestY1       , /*setEditIndex1ToSocketY*/0, /*setEditIndex1ToSocketY*/0, "setEditIndex1ToSocketY"  , "setEditIndex1ToSocketY"  , /*SetButton064*/0      , /*GoButton064*/0));
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHT9040TestY1_Front    ,   &Tech.iHT9040TestY1_Front       ,   MTestY1         , MTestY1       , /*setEditIndex1ToSht1Y*/0  , /*setEditIndex1ToSht1Y*/0  , "setEditIndex1ToSht1Y"    , "setEditIndex1ToSht1Y"    , /*SetButton065*/0      , /*GoButton065*/0));
    }
    else
    {
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHT9040TestY1_Middle   ,   &Tech.iHT9040TestY2_Rear        ,   MTestY1         , MTestY2       , /*setEditIndex1ToSocketY*/0, /*setEditIndex2ToSht2Y*/0  , "setEditIndex1ToSocketY"  , "setEditIndex2ToSht2Y"    , /*SetButton064*/0      , /*GoButton064*/0));
    // golden :917
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHT9040TestY2_Middle   ,   &Tech.iHT9040TestY1_Front       ,   MTestY2         , MTestY1       , /*setEditIndex2ToSocketY*/0, /*setEditIndex1ToSht1Y*/0  , "setEditIndex2ToSocketY"  , "setEditIndex1ToSht1Y"    , /*SetButton065*/0      , /*GoButton065*/0));
    }

    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmPickX                ,   &Tech.iInArmPickY               ,   MInArmX         , MInArmY       , /*setInPickX*/0            , /*setInPickY*/0            , "setInPickX"              , "setInPickY"              , /*SetBtnInPick*/0      , /*GoBtnInPick*/0   ,   false));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmPickX               ,   &Tech.iOutArmPickY              ,   MOutArmX        , MOutArmY      , /*setOutPickX*/0           , /*setOutPickY*/0           , "setOutPickX"             , "setOutPickY"             , /*SetBtnOutPick*/0     , /*GoBtnOutPick*/0  ,   false));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmAutoCleanX           ,   &Tech.iInArmAutoCleanY          ,   MInArmX         , MInArmY       , /*setEditAutoCleanX*/0     , /*setEditAutoCleanY*/0     , "setEditAutoCleanX"       , "setEditAutoCleanY"       , /*SetButton014*/0      , /*GoButton014*/0));  //Steven 20120706 : 加入OCR與Y變距
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_In_iRotateX              ,   &Tech.M_In_iRotateY             ,   MInArmX         , MInArmY       , /*setEditRotateX*/0        , /*setEditRotateY*/0        , "setEditRotateX"          , "setEditRotateY"          , /*SetButtonRotate*/0   , /*GoButtonRotate*/0));  //2013-04-12    Dell :旋轉站;馬達版
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_Out_iRotateX             ,   &Tech.M_Out_iRotateY            ,   MOutArmX        , MOutArmY      , /*setEditRotateOutX*/0     , /*setEditRotateOutY*/0     , "setEditRotateOutX"       , "setEditRotateOutY"       , /*SetButtonRotateOut*/0, /*GoButtonRotateOut*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHP1LaserX                 ,   &Tech.iHP1LaserY                ,   MInArmX         , MInArmY       , /*setEdtHP1LaserX*/0       , /*setEdtHP1LaserY*/0       , "setEdtHP1LaserX"         , "setEdtHP1LaserY"         , /*SetBtnHP1Laser*/0    , /*GoBtnHP1Laser*/0));  //Steven 20140228 : 雷射測距功能
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iHP2LaserX                 ,   &Tech.iHP2LaserY                ,   MInArmX         , MInArmY       , /*setEdtHP2LaserX*/0       , /*setEdtHP2LaserY*/0       , "setEdtHP2LaserX"         , "setEdtHP2LaserY"         , /*SetBtnHP2Laser*/0    , /*GoBtnHP2Laser*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_iTopView_X               ,   &Tech.M_iTopView_Y              ,   MOutArmX        , MOutArmY      , /*setEditTopViewX*/0       , /*setEditTopViewY*/0       , "setEditTopViewX"         , "setEditTopViewY"         , /*SetBtnTopView*/0     , /*GoBtnTopView*/0  ,   false));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_iPADView_X               ,   &Tech.M_iPADView_Y              ,   MOutArmX        , MOutArmY      , /*setEditPADViewX*/0       , /*setEditPADViewY*/0       , "setEditPADViewX"         , "setEditPADViewY"         , /*SetBtnPADView*/0     , /*GoBtnPADView*/0));  //wei 20160617 Vitrox
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_iBGAView_X               ,   &Tech.M_iBGAView_Y              ,   MOutArmX        , MOutArmY      , /*setEditBGAViewX*/0       , /*setEditBGAViewY*/0       , "setEditBGAViewX"         , "setEditBGAViewY"         , /*SetBtnBGAView*/0     , /*GoBtnBGAView*/0));  //wei 20160617 Vitrox
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_iTopViewSafePos_X        ,   &Tech.M_iTopViewSafePos_Y       ,   MOutArmX        , MOutArmY      , /*setEditSafePosX*/0       , /*setEditSafePosY*/0       , "setEditSafePosX"         , "setEditSafePosY"         , /*SetBtnSafePos*/0     , /*GoBtnSafePos*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmDecay_X              ,   &Tech.iInArmDecay_Y             ,   MInArmX         , MInArmY       , /*setEdtINDecayX*/0        , /*setEdtINDecayY*/0        , "setEdtINDecayX"          , "setEdtINDecayY"          , /*SetDecayInButton*/0  , /*GoDecayInButton*/0));  //Ifor 20151209 :新增 Auto Decay In Arm Teach 點位
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmDecay_X             ,   &Tech.iOutArmDecay_Y            ,   MOutArmX        , MOutArmY      , /*setEdtOUTDecayX*/0       , /*setEdtOUTDecayY*/0       , "setEdtOUTDecayX"         , "setEdtOUTDecayY"         , /*SetDecayOutButton*/0 , /*GoDecayOutButton*/0));  //Ifor 20151209 :新增 Auto Decay Out Arm Teach 點位
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmBinBoxX             ,   &Tech.iOutArmBinBoxY            ,   MOutArmX        , MOutArmY      , /*edtBinBoxX*/0            , /*edtBinBoxY*/0            , "edtBinBoxX"              , "edtBinBoxY"              , /*btnSetBtnBinBox*/0   , /*btnGoBtnBinBox*/0));  //kevin 20160822 bulk box
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iBottom2DIDInX             ,   &Tech.iBottom2DIDInY            ,   MInArmX         , MInArmY       , /*edtBt2DX*/0              , /*edtBt2DY*/0              , "edtBt2DX"                , "edtBt2DY"                , /*btnBottom2DSet*/0        , /*btnBottom2DGo*/0));  //Steven 20190308 : Bottom 2D
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmNGBinBoxX            ,   &Tech.iInArmNGBinBoxY           ,   MInArmX         , MInArmY       , /*setEditNGBinBoxX*/0      , /*setEditNGBinBoxY*/0      , "setEditNGBinBoxX"        , "setEditNGBinBoxY"        , /*SetButtonNGBinBox*/0     , /*GoButtonNGBinBox*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.M_ScannerAOI_X             ,   &Tech.M_ScannerAOI_Y            ,   MOutArmX        , MOutArmY      , /*setEditScannerAOIX*/0    , /*setEditScannerAOIY*/0    , "setEditScannerAOIX"      , "setEditScannerAOIY"      , /*SetBtnScannerAOI*/0      ,   /*GoBtnScannerAOI*/0));  //Ifor 20191211 : add Scanner AOI
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmXCCDAlignmentPos     ,   &Tech.iInArmYCCDAlignmentPos    ,   MInArmX         , MInArmY       , /*edtInArmXCCDPos*/0       , /*edtInArmYCCDPos*/0       , "edtInArmXCCDPos"         , "edtInArmYCCDPos"         , /*sbInArmCCDPos*/0         , /*sbGoInArmCCDPos*/0));  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInArmXBasePickerAlignmentPos, &Tech.iInArmYBasePickerAlignmentPos,MInArmX         , MInArmY       , /*edtInArmXBasePickerPos*/0, /*edtInArmYBasePickerPos*/0, "edtInArmXBasePickerPos"  , "edtInArmYBasePickerPos"  , /*sbInArmBasePickerPos*/0  , /*sbGoInArmBasePickerPos*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmXCCDAlignmentPos    ,   &Tech.iOutArmYCCDAlignmentPos   ,   MOutArmX        , MOutArmY      , /*edtOutArmXCCDPos*/0      , /*edtOutArmYCCDPos*/0      , "edtOutArmXCCDPos"        , "edtOutArmYCCDPos"        , /*sbOutArmCCDPos*/0        , /*sbGoOutArmCCDPos*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmXBasePickerAlignmentPos,&Tech.iOutArmYBasePickerAlignmentPos,MOutArmX       , MOutArmY      , /*edtOutArmXBasePickerPos*/0,/*edtOutArmYBasePickerPos*/0,"edtOutArmXBasePickerPos" ,"edtOutArmYBasePickerPos"  , /*sbOutArmBasePickerPos*/0 , /*sbGoOutArmBasePickerPos*/0));
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInPlacementX              ,   &Tech.iInPlacementY             ,   MInPlacementX   , MInPlacementY , /*setEditInarmPlacementX*/0, /*setEditInarmPlacementY*/0, "setEditInarmPlacementX"  , "setEditInarmPlacementY"  , /*SetInarmPlacementXY*/0   ,   /*GoInarmPlacementXY*/0));  //JimmyChiu 20220908 add Pickup Error Placement
    // golden :942
    TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iInPlacementOffsetByBasicX ,   &Tech.iInPlacementOffsetByBasicY,   MInArmX         , MInArmY       , /*setEditInarmPlacementXOffsetByBasicSuck*/0       , /*setEditInarmPlacementYOffsetByBasicSuck*/0, "setEditInarmPlacementXOffsetByBasicSuck", "setEditInarmPlacementYOffsetByBasicSuck", /*SetComputeInSh2*/0, /*btnStop*/0));

    if(INSTALL_OCR_YMot==eocrYMotInstal)                                        //Frank 20250214 add
    {
        TechPara.push_back(new TECH_PARA(&Tech.iMLoaderYCarPos      ,MLoaderY  , /*setYCarPos*/0     , "setYCarPos"              , /*sbLoaderYCarPos*/0               , /*GoBtnYCarPos*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iMLoaderYOCRPos      ,MLoaderY  , /*setYOCRPos*/0     , "setYOCRPos"              , /*sbLoaderYOCRPos*/0               , /*GoBtnYOCRPos*/0));
        TechPara.push_back(new TECH_PARA(&Tech.iMLoaderYSurePos     ,MLoaderY  , /*setYSurePos*/0    , "setYSurePos"             , /*sbLoaderYSurePos*/0              , /*GoBtnYSurePos*/0));
    }

    TechPara.push_back(new TECH_PARA(&Tech.iInSH1SenICDetectPos     ,MInShuttle1       ,/*edtSetSH1_16SiteKit*/0  ,"edtSetSH1_16SiteKit"   ,/*btnSetSH1_16SiteKit*/0       ,/*btnGoSH1_16SiteKit*/0));  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    TechPara.push_back(new TECH_PARA(&Tech.iInSH2SenICDetectPos     ,MInShuttle2       ,/*edtSetSH2_16SiteKit*/0  ,"edtSetSH2_16SiteKit"   ,/*btnSetSH2_16SiteKit*/0       ,/*btnGoSH2_16SiteKit*/0));  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料

    TechPara.push_back(new TECH_PARA(&Tech.iInSH1SenICDetectZ1      ,MInSh1LtcSenZ1    ,/*setEditInSh1LtcSenZ1*/0 ,"setEditInSh1LtcSenZ1"  ,/*SetButtonInSh1LtcSenZ1*/0    ,/*GoButtonInSh1LtcSenZ1*/0));  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    TechPara.push_back(new TECH_PARA(&Tech.iInSH1SenICDetectZ2      ,MInSh1LtcSenZ2    ,/*setEditInSh1LtcSenZ2*/0 ,"setEditInSh1LtcSenZ2"  ,/*SetButtonInSh1LtcSenZ2*/0    ,/*GoButtonInSh1LtcSenZ2*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInSH2SenICDetectZ1      ,MInSh2LtcSenZ1    ,/*setEditInSh2LtcSenZ1*/0 ,"setEditInSh2LtcSenZ1"  ,/*SetButtonInSh2LtcSenZ1*/0    ,/*GoButtonInSh2LtcSenZ1*/0));
    TechPara.push_back(new TECH_PARA(&Tech.iInSH2SenICDetectZ2      ,MInSh2LtcSenZ2    ,/*setEditInSh2LtcSenZ2*/0 ,"setEditInSh2LtcSenZ2"  ,/*SetButtonInSh2LtcSenZ2*/0    ,/*GoButtonInSh2LtcSenZ2*/0));

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //Steven 20141029 : XY-Pitch
    {
    // [W906-TEACH-W1 T2 widget-only] golden :961
    //        MotorInArmZE->Flat      =false;
    // [W906-TEACH-W1 T2 widget-only] golden :962
    //        MotorInArmZF->Flat      =true;
        if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                               //StevenHong 20260325 :  //RogerYang 20251222 : IN/OUT ARM支援不同模組
        {
    // [W906-TEACH-W1 T2 widget-only] golden :965
    //            MotorOutArmZE->Flat     =false;
    // [W906-TEACH-W1 T2 widget-only] golden :966
    //            MotorOutArmZD->Flat     =true;
    // golden :967
        }
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
    // [W906-TEACH-W1 T2 widget-only] golden :971
    //        MotorInArmZE->Flat      =false;
    // [W906-TEACH-W1 T2 widget-only] golden :972
    //        MotorInArmZH->Flat      =true;
    // [W906-TEACH-W1 T2 widget-only] golden :973
    //        MotorOutArmZE->Flat     =false;
    // [W906-TEACH-W1 T2 widget-only] golden :974
    //        MotorOutArmZBe->Flat    =true;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                       //Ztex 2024.02.24 Add HT-1132
    {
    // [W906-TEACH-W1 T2 widget-only] golden :978
    //        MotorInArmZE->Flat      =false;
    // [W906-TEACH-W1 T2 widget-only] golden :979
    //        MotorInArmZD->Flat      =true;
    // [W906-TEACH-W1 T2 widget-only] golden :980
    //        MotorOutArmZE->Flat     =false;
    // [W906-TEACH-W1 T2 widget-only] golden :981
    //        MotorOutArmZF->Flat     =true;
    }

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250416 for HT9046AU add
    {
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iSortArmShtStageX      ,   &Tech.iSortArmShtStageY         ,   MOutSortX       , MOutSortY       , /*edtOutSortArmShtL_X*/0 , /*edtOutSortArmShtL_Y*/0   , "edtOutSortArmShtL_X"     , "edtOutSortArmShtL_Y"     , /*btnSetSortArmToSHT*/0    , /*btnGoSortArmToSHT*/0));
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iSortArmAuto4X         ,   &Tech.iSortArmAuto4Y            ,   MOutSortX       , MOutSortY       , /*edtOutSortArmAuto4_X*/0, /*edtOutSortArmAuto4_Y*/0  , "edtOutSortArmAuto4_X"    , "edtOutSortArmAuto4_Y"    , /*btnSetSortArmToAuto4*/0  , /*btnGoSortArmToAuto4*/0));
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iSortArmAuto5X         ,   &Tech.iSortArmAuto5Y            ,   MOutSortX       , MOutSortY       , /*edtOutSortArmAuto5_X*/0, /*edtOutSortArmAuto5_Y*/0  , "edtOutSortArmAuto5_X"    , "edtOutSortArmAuto5_Y"    , /*btnSetSortArmToAuto5*/0  , /*btnGoSortArmToAuto5*/0));
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iSortArmAuto6X         ,   &Tech.iSortArmAuto6Y            ,   MOutSortX       , MOutSortY       , /*edtOutSortArmAuto6_X*/0, /*edtOutSortArmAuto6_Y*/0  , "edtOutSortArmAuto6_X"    , "edtOutSortArmAuto6_Y"    , /*btnSetSortArmToAuto6*/0  , /*btnGoSortArmToAuto6*/0));
        TechTwoPara.push_back(new TECH_TWOPARA(&Tech.iOutArmSortShtX        ,   &Tech.iOutArmSortShtY           ,   MOutArmX        , MOutArmY        , /*edtOuttArmShtR_X*/0    , /*edtOuttArmShtR_Y*/0      , "edtOuttArmShtR_X"        , "edtOuttArmShtR_Y"        , /*btnSetOutArmToSHT*/0     , /*btnGoOutArmToSHT*/0));
    }
    // golden :992

    TECH_MAX_ITEM=TechPara.size();
    TechTwoItem=TechTwoPara.size();
}
