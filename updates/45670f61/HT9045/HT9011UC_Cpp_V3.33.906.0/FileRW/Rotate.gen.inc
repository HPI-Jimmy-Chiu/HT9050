// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Rotate 的讀寫檔（HTEditList： → <DataPath>\<配方>\Rotate.Data [SETTING]（讀 fRotate_ReadFile：只有 ReadIniData；寫 spbSaveClick：WriteIniData））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\RotateKit/fRotate.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden RotateKit/fRotate.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 18 個（header 宣告 41 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "forms/fRotate.h"
#include "forms/fMain.h"
#include "cinitial.h"
#include "RotateKit/aRotateKIT.h"

void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket）
// golden TMyKitSuck 的四個格數欄位的轉接物件：TestSocket 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）
struct RT_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };
static inline RT_Kit RT_KitOf(int w) { RT_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }

// golden RotateKit/fRotate.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
#define bShowRotateBySite (FrmRotate->bShowRotateBySite)   // golden fRotate.h:142 → forms/fRotate.h（main.cpp:32717、aRotateKIT_In/Out 讀）
#define bIsAngleZero (FrmRotate->bIsAngleZero)             // golden fRotate.h:136 → forms/fRotate.h（csystem.cpp:18865 讀）
#define iRotateDutDate (FrmRotate->iRotateDutDate)         // golden fRotate.h:138 → forms/fRotate.h（aRotateDegreeClass.cpp:100 寫）
#define iFromTrayAngle (FrmRotate->iFromTrayAngle)         // golden fRotate.h:139 → forms/fRotate.h（ainarm9045.cpp:2712 讀）
#define TestSocket RT_KitOf(0)                             // golden TestSocket.iShtRow／iShtCol → FileRW/_KitSuck.cpp
static TEdit *Rotate_Offset[2];                                   // golden RotateKit/fRotate.cpp:25 //Ifor 20251210 add:Rotate Offset
static TEdit *Pre_Rotate[2];                                      // golden RotateKit/fRotate.cpp:26 //Ifor 20260901 add:Pre Rotate degree before pick
static bool IsRotateFlowNotFinish()                                             //KevinCheng 20260318 : Rotate流程未完，不可關閉。
{
    if(USE_ROTATE_KIT!=1)
        return false;

    if(iInArmRotateKit!=1 ||                                                    // rotate task不在初始狀態，表示流程仍在執行中
       iOutArmRotateKit!=1)
        return true;

    if(bWaitRotateFinish)
        return true;

    return false;
}   // golden RotateKit/fRotate.cpp:28-41（逐字）

using filerw::EL;

// 前置宣告
static void RT_TFrmRotate();
static void RT_fRotate_ReadFile();
static void RT_DoIniDataToForm();
static void RT_spbSaveClick();

// golden RotateKit/fRotate.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void RT_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("TFrmRotate", "rgDutNum")->Items;
      s->Add("2x2");
      s->Add("2x4 ");
      s->Add("1x1 ");
      s->Add("1x2 ");
    }
    { TStringList* s = EL<TRadioGroup>("TFrmRotate", "rgSelect")->Items;
      s->Add("All");
      s->Add("Single");
    }
}

// golden RotateKit/fRotate.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kRT_ParentOf[][2] = {
    {"GroupBox1", "Panel1"},
    {"Label_Err", "Panel1"},
    {"XPitch", "Panel1"},
    {"YPitch", "Panel1"},
    {"cbActiveRotate", "Panel1"},
    {"cbOutRotateDifferentAngle", "Panel1"},
    {"chkART_RTNoRotate", "Panel1"},
    {"chkPassBinNoRotate", "Panel1"},
    {"chkRotateUseRTmode", "Panel1"},
    {"edInPreRotate_0", "grpPreRotate"},
    {"edInRotate_0", "grpRotateOffset"},
    {"edOutPreRotate_0", "grpPreRotate"},
    {"edOutRotate_0", "grpRotateOffset"},
    {"grpPreRotate", "Panel1"},
    {"grpRotateOffset", "Panel1"},
    {"pgcRotate", "Panel1"},
    {"rgDutNum", "Panel1"},
    {"sgFTA", "GroupBox1"},
    {"sgRotateIn", "tsM1RotateIn"},
    {"sgRotateInRT", "tsM1RotateInRT"},
    {"sgRotateOut", "tsM1RotateOut"},
    {"sgRotateOutRT", "tsM1RotateOutRT"},
    {"tsM1RotateIn", "pgcRotate"},
    {"tsM1RotateInRT", "pgcRotate"},
    {"tsM1RotateOut", "pgcRotate"},
    {"tsM1RotateOutRT", "pgcRotate"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void RT_CreateContainerProxies()
{
    EL<TGroupBox>("TFrmRotate", "GroupBox1");
    EL<TPanel>("TFrmRotate", "Panel1");
    EL<TGroupBox>("TFrmRotate", "grpPreRotate");
    EL<TGroupBox>("TFrmRotate", "grpRotateOffset");
    EL<TPageControl>("TFrmRotate", "pgcRotate");
    EL<TTabSheet>("TFrmRotate", "tsM1RotateIn");
    EL<TTabSheet>("TFrmRotate", "tsM1RotateInRT");
    EL<TTabSheet>("TFrmRotate", "tsM1RotateOut");
    EL<TTabSheet>("TFrmRotate", "tsM1RotateOutRT");
    filerw::ELSetParents("TFrmRotate", kRT_ParentOf, (int)(sizeof(kRT_ParentOf) / sizeof(kRT_ParentOf[0])));
}

// golden RotateKit/fRotate.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void RT_DfmState()
{
    EL<TLabel>("TFrmRotate", "Label_Err")->Visible = false;
    EL<TEdit>("TFrmRotate", "edInPreRotate_0")->Text = "0";
    EL<TEdit>("TFrmRotate", "edInRotate_0")->Text = "0";
    EL<TEdit>("TFrmRotate", "edOutPreRotate_0")->Text = "0";
    EL<TEdit>("TFrmRotate", "edOutRotate_0")->Text = "0";
    EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kRT_EnableAll[] = {
    "GroupBox1", "Label_Err", "Panel1", "cbActiveRotate", "cbOutRotateDifferentAngle", "chkART_RTNoRotate",
    "chkPassBinNoRotate", "chkRotateUseRTmode", "grpPreRotate", "grpRotateOffset", "pgcRotate", "rgDutNum",
    "tsM1RotateIn", "tsM1RotateInRT", "tsM1RotateOut", "tsM1RotateOutRT",
};

// 存檔流程（spbSaveClick）讀的替身 8 個 —— FileRW/Rotate.cpp 用它決定頁面必須送哪些值
static const char* const kRT_SaveReads[] = {
    "XPitch", "YPitch", "cbActiveRotate", "cbOutRotateDifferentAngle", "chkART_RTNoRotate", "chkPassBinNoRotate",
    "chkRotateUseRTmode", "rgDutNum",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void RT_CreateSaveProxies()
{
    EL<TLabeledEdit>("TFrmRotate", "XPitch");
    EL<TLabeledEdit>("TFrmRotate", "YPitch");
    EL<TCheckBox>("TFrmRotate", "cbActiveRotate");
    EL<TCheckBox>("TFrmRotate", "cbOutRotateDifferentAngle");
    EL<TCheckBox>("TFrmRotate", "chkART_RTNoRotate");
    EL<TCheckBox>("TFrmRotate", "chkPassBinNoRotate");
    EL<TCheckBox>("TFrmRotate", "chkRotateUseRTmode");
    EL<TRadioGroup>("TFrmRotate", "rgDutNum");
}

// golden RotateKit/fRotate.cpp:43  TFrmRotate::TFrmRotate(TComponent* Owner)
static void RT_TFrmRotate()
{

    if(USE_ROTATE_KIT==1 &&
       (iRotate_Type==e4MotRotate ||
        iRotate_Type==e8MotRotate ||
        iRotate_Type==e2MotRotate2Dut ||
        iRotate_Type==e1MotRotate1Dut))
    {
        bShowRotateBySite=true;
    }
    else
    {
    ;   // golden RotateKit/fRotate.cpp:56-69 取代：純畫面：非 by-site 時把 sgRotateIn／sgRotateOut（與 RT 兩張）的格子清成 0。具名替身 filerw::ELStringGrid 沒有 ColCount／RowCount（FileRW/_EditList.h:107-111 只有 Cells），而且格子內容 golden 開頁 SetGridDraw（:643，FormShow :134）會整張重畫；檔案與 tRotate 都不受影響
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:56-69 -- 純畫面：非 by-site 時把 sgRotateIn／sgRotateOut（與 RT 兩張）的格子清成 0。具名替身 filerw::ELStringGrid 沒有 ColCount／RowCount（FileRW/_EditList.h:107-111 只有 Cells），而且格子內容 golden 開頁 SetGridDraw（:643，FormShow :134）會整張重畫；檔案與 tRotate 都不受影響
        for(int i=0; i<EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->ColCount; i++)
        {
            for(int j=0; j<EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->RowCount; j++)
            {
                EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->Cells[i][j]=0;
                EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateOut")->Cells[i][j]=0;                                     //Ifor 20170407 add Rotate Out 獨立旋轉角度

                if(CosFunction.bRotateUseRTmode==true)                          //jou 20231122 : Rotate Use RT mode
                {
                    EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateInRT")->Cells[i][j]=0;
                    EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateOutRT")->Cells[i][j]=0;                               //Ifor 20170407 add Rotate Out 獨立旋轉角度
                }
            }
        }
#endif // GATE (S12-C save)
        bShowRotateBySite=false;
    }

    EL<filerw::ELStringGrid>("TFrmRotate", "sgFTA")->Cells[0][0]=0;

    Rotate_Offset[0]=EL<TEdit>("TFrmRotate", "edInRotate_0");
    Rotate_Offset[1]=EL<TEdit>("TFrmRotate", "edOutRotate_0");

    Pre_Rotate[0]=EL<TEdit>("TFrmRotate", "edInPreRotate_0");                                              //Ifor 20260901 add:Pre Rotate degree before pick
    Pre_Rotate[1]=EL<TEdit>("TFrmRotate", "edOutPreRotate_0");

    ZeroMemory(&iRotateDutDate, sizeof(iRotateDutDate));
    iFromTrayAngle=0;

}

// golden RotateKit/fRotate.cpp:241  TFrmRotate::fRotate_ReadFile()
static void RT_fRotate_ReadFile()
{

    AnsiString tmpA ,tmpB;
    AnsiString S="", str;
    S=GetLastOpenFN();
    AnsiString szDir="";
//    int rt =-1;
    AnsiString tmpS ="";
    szDir.sprintf("%s%s\\Rotate.Data", DataPath, S);
    TStringList *slistIn  = new TStringList;
    TStringList *slistOut = new TStringList;                                    //Ifor 20170407 (Steven) add Rotate Out 獨立旋轉角度
    TStringList *slistInRT  = new TStringList;
    TStringList *slistOutRT = new TStringList;
    int tmpAngleIn, tmpAngleOut;                                                //Ifor 20170412 (Steven) add In/Out Rotate 旋轉角度暫存
    bool bInAngleNotExist=true;                                                 //Ifor 20170412 (Steven) add In/Out Rotate 旋轉角度不存在
    bool bOutAngleNotExist=true;                                                //Ifor 20170412 (Steven) add In/Out Rotate 旋轉角度不存在
    bool bInAngleNotExistRT=true;
    bool bOutAngleNotExistRT=true;

    tRotate.ActiveRotate        = ReadIniData(szDir, "SETTING", "ActiveRotate", false);
    tRotate.bUseDifferentAngle  = ReadIniData(szDir, "SETTING", "OutRotateDifferentAngle", false);  //Ifor 20170407 add Rotate Out 獨立旋轉角度
    if(CosFunction.bPassBinNoRotate==true)                                      //jou 20231020 : Pass bin no rotate
        tRotate.bPassBinNoRotate= ReadIniData(szDir, "SETTING", "bPassBinNoRotate", false);

    if(USE_ROTATE_KIT==1)
    {
        if(iRotate_Type==e4MotRotate)
        {
            tRotate.DutNum          = tDutType_4;
        }
        else if(iRotate_Type==e1MotRotate1Dut)
        {
            tRotate.DutNum          = tDutType_1;
        }
        else if(iRotate_Type==e2MotRotate2Dut)
        {
            tRotate.DutNum          = tDutType_2;
        }
        else
        {
            tRotate.DutNum          = ReadIniData(szDir, "SETTING", "Dut Num", 0);
        }
    }
    else
    {
        tRotate.DutNum          = ReadIniData(szDir, "SETTING", "Dut Num", 0);
    }
    tRotate.FromTrayAngle       = ReadIniData(szDir, "SETTING", "FromTrayAngle", 0.0);
    slistIn->CommaText          = ReadIniData(szDir, "SETTING", "DutAngle",    AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
    slistOut->CommaText         = ReadIniData(szDir, "SETTING", "DutAngleOut", AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20170407 add Rotate Out 獨立旋轉角度

    if(CosFunction.bRotateUseRTmode==true)                                      //jou 20231122 : Rotate Use RT mode
    {
        tRotate.bRotateUseRTmode = ReadIniData(szDir, "SETTING", "bRotateUseRTmode", false);
        slistInRT->CommaText     = ReadIniData(szDir, "SETTING", "DutAngleRT",    AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
        slistOutRT->CommaText    = ReadIniData(szDir, "SETTING", "DutAngleOutRT", AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));
    }

    if(CosFunction.bART_RT_NoRotate)                                            //Sam 20240809 : ART RT No Rotate
        tRotate.bART_RT_NoRotate= ReadIniData(szDir, "SETTING", "bART_RT_NoRotate", false);
    strInRotateDutAngle         = slistIn->CommaText;                           //wei 20180316
    strOutRotateDutAngle        = slistOut->CommaText;                          //wei 20180316

    if(USE_ROTATE_KIT==1 && bShowRotateBySite==true)
    {
        tRotate.RowCount        =TestSocket.iShtRow;
        tRotate.ColCount        =TestSocket.iShtCol;
    }
    else
    {
        tRotate.ColCount        = ReadIniData(szDir, "SETTING", "ColCount", 0);
        tRotate.RowCount        = ReadIniData(szDir, "SETTING", "RowCount", 0);
    }

    if(tRotate.DutNum==tDutType_8)                                              //20140320 wei   固定 4 Rotate Xpitch= 80mm; 8 Rotate Xpitch= 40mm
    {
        tRotate.RotateKit_PitchX=40;
        tRotate.RotateKit_PitchY=ReadIniData(szDir, "SETTING", "KitPitchY", 60.0);   //jou 2015-09-03 修正Rotate pitch無法存檔的問題
    }
    else if(tRotate.DutNum==tDutType_1)
    {
        tRotate.RotateKit_PitchX=0;
        tRotate.RotateKit_PitchY=0;
    }
    else if(tRotate.DutNum==tDutType_2)
    {
        tRotate.RotateKit_PitchX=80;
        tRotate.RotateKit_PitchY=0;
    }
    else
    {
        tRotate.RotateKit_PitchX=ReadIniData(szDir, "SETTING", "KitPitchX", 80.0);   //jou 2015-09-03 修正Rotate pitch無法存檔的問題
        tRotate.RotateKit_PitchY=ReadIniData(szDir, "SETTING", "KitPitchY", 60.0);   //jou 2015-09-03 修正Rotate pitch無法存檔的問題
    }
    ZeroMemory(&tRotate.RotationCount, sizeof(tRotate.RotationCount));
    ZeroMemory(&tRotate.OutRotationCount, sizeof(tRotate.OutRotationCount));    //kevin 20131003  清為0

    ZeroMemory(&tRotate.RotationCountRT, sizeof(tRotate.RotationCountRT));
    ZeroMemory(&tRotate.OutRotationCountRT, sizeof(tRotate.OutRotationCountRT));

    if(slistIn->Count==8)                                                       //Ifor 20170411 (Steven) add Rotate 舊資料僅八筆資料會發生異常，補插入八筆資料
    {
        for(int i=4; i<=12; i=i+8)
        {
            for(int j=0; j<4; j++)
            {
                slistIn->Insert(i, 0);
            }
        }
    }

    tRotate.RotationTimeIn =0;                                                  //清除 Count 數
    tRotate.RotationTimeOut=0;

    tRotate.RotationTimeInRT =0;
    tRotate.RotationTimeOutRT=0;

    if(slistIn->Count!=32)                                                      //Ifor 20170411 (Steven) : Rotate資料補成32site
    {
        do
        {
            slistIn->Add(0);
        }
        while(slistIn->Count<32);
    }

    if(slistOut->Count!=32)                                                     //Ifor 20170424 (Steven) : Out Rotate資料補成32site
    {
        do
        {
            slistOut->Add(0);
        }
        while(slistOut->Count<32);
    }

    if(slistInRT->Count!=32)                                                    //Ifor 20170411 (Steven) : Rotate資料補成32site
    {
        do
        {
            slistInRT->Add(0);
        }
        while(slistInRT->Count<32);
    }

    if(slistOutRT->Count!=32)                                                   //Ifor 20170424 (Steven) : Out Rotate資料補成32site
    {
        do
        {
            slistOutRT->Add(0);
        }
        while(slistOutRT->Count<32);
    }

    bIsAngleZero=false;                                                          //Eastsun 20260515 F023: E6 detect rotate 0 degree
    if(slistIn->Count>0)                                                        //Ifor 20170626 (wei) add 迴圈改用row col 避免tRotate.RotationTime 多1次
    {
        for(int i=0; i<2; i++)                                                  //Ifor 20171005 (Steven) : 避免1x2模式Rotate無法存檔問題 iRow -> 2
        {
            for(int j=0; j<8; j++)                                              //Steven 20201029 : iCol --> 8
            {
                //Ifor 20170407 (Steven) add Rotate Out 獨立旋轉角度
                //==>
                tRotate.RotateDutDate[0][i][j]=atoi(slistIn->Strings [i*8+j].c_str());
                tRotate.RotateDutDate[1][i][j]=atoi(slistOut->Strings[i*8+j].c_str());

                tRotate.RotateDutDate[2][i][j]=atoi(slistInRT->Strings [i*8+j].c_str());
                tRotate.RotateDutDate[3][i][j]=atoi(slistOutRT->Strings[i*8+j].c_str());

                //AI(ht9045-outarm-flow) 20260903 (RogerYang) : 正規化到 [0,360), 修復既有被累減成負值的工作檔
                //  C 的 % 對負數會回負值, 下游 DrawState()/SetDegree() 只認 0/90/180/270 與 0..3
                for(int k=0; k<4; k++)
                    tRotate.RotateDutDate[k][i][j]=((tRotate.RotateDutDate[k][i][j]%360)+360)%360;

                if(USE_ROTATE_KIT==1 && bShowRotateBySite==true)
                {
                    tRotate.RotationTimeIn =1;
                    tRotate.RotationTimeOut=1;
                }
                else
                {
                    //====================計算 Rotate In 旋轉角度====================
                    tmpAngleIn=tRotate.RotateDutDate[0][i][j];

                    if(tmpAngleIn==0 &&                                         //Eastsun 20260515 F023: E6 detect rotate 0 degree ; AI(ht9045-flow-architect) 20260805 (RogerYang) : 加真實格位判斷,原固定2x8會掃到不存在欄位(恆0)誤報擋機
                        i<tRotate.RowCount &&
                        j<tRotate.ColCount)
                    {
                        bIsAngleZero=true;                                      //AI(ht9045-outarm-flow) 20260904 (RogerYang) : 零度偵測移出 dedup 區塊--下方 Ifor 20240919 的 ||條件會讓 0 不進 if(bInAngleNotExist), 偵測會被壓掉
                    }

                    for(int k=0; k<tRotate.RotationTimeIn; k++)                 //Ifor 20170412 (Steven) 判斷In Rotate 旋轉角度是否已存在
                    {
                        if(tRotate.RotationCount[k]==tmpAngleIn || tmpAngleIn==0)   //Ifor 20240919 add:避免Count 多一次導致流程異常
                            bInAngleNotExist=false;
                    }

                    if(bInAngleNotExist==true)
                    {
                        tRotate.RotationCount[tRotate.RotationTimeIn]=tmpAngleIn;
                        tRotate.RotationTimeIn++;
                    }
                    bInAngleNotExist=true;                                      //避免有一次相同角度後無法再次更新

                    //====================計算 Rotate Out 旋轉角度====================
                    if(tRotate.bUseDifferentAngle==true)                        //Out Arm 使用不同角度
                    {
                        tmpAngleOut=360-tRotate.RotateDutDate[0][i][j]+tRotate.RotateDutDate[1][i][j]-360;
                    }
                    else
                    {
                        tmpAngleOut=0-tmpAngleIn;
                    }

                    if(tmpAngleOut==0 &&                                        //Eastsun 20260515 F023: E6 detect rotate 0 degree ; AI(ht9045-flow-architect) 20260805 (RogerYang) : 加真實格位判斷,原固定2x8會掃到不存在欄位(恆0)誤報擋機
                        i<tRotate.RowCount &&
                        j<tRotate.ColCount)
                    {
                        bIsAngleZero=true;                                      //AI(ht9045-outarm-flow) 20260904 (RogerYang) : 零度偵測移出 dedup 區塊--下方 Ifor 20240919 的 ||條件會讓 0 不進 if(bInAngleNotExist), 偵測會被壓掉
                    }

                    for(int k=0; k<tRotate.RotationTimeOut; k++)                //Ifor 20170412 (Steven) 判斷Out Rotate 旋轉角度是否已存在
                    {
                        if(tRotate.OutRotationCount[k]==tmpAngleOut || tmpAngleOut==0)   //Ifor 20240919 add:避免Count 多一次導致流程異常
                            bOutAngleNotExist=false;
                    }

                    if(bOutAngleNotExist==true)
                    {
                        tRotate.OutRotationCount[tRotate.RotationTimeOut]=tmpAngleOut;
                        tRotate.RotationTimeOut++;
                    }
                    bOutAngleNotExist=true;                                     //避免有一次相同角度後無法再次更新
                    //<==
                    //Ifor 20170407 add Rotate Out 獨立旋轉角度

                    if(CosFunction.bRotateUseRTmode==true)                      //jou 20231122 : Rotate Use RT mode
                    {
                        tmpAngleIn=tRotate.RotateDutDate[2][i][j];
                        for(int k=0; k<tRotate.RotationTimeInRT; k++)           //Ifor 20170412 (Steven) 判斷In Rotate 旋轉角度是否已存在
                        {
                            if(tRotate.RotationCountRT[k]==tmpAngleIn)
                                bInAngleNotExistRT=false;
                        }

                        if(bInAngleNotExistRT==true)
                        {
                            tRotate.RotationCountRT[tRotate.RotationTimeInRT]=tmpAngleIn;
                            tRotate.RotationTimeInRT++;
                        }
                        bInAngleNotExistRT=true;                                //避免有一次相同角度後無法再次更新

                        //====================計算 Rotate Out 旋轉角度====================
                        if(tRotate.bUseDifferentAngle==true)                    //Out Arm 使用不同角度
                        {
                            tmpAngleOut=360-tRotate.RotateDutDate[2][i][j]+tRotate.RotateDutDate[3][i][j]-360;
                        }
                        else
                        {
                            tmpAngleOut=0-tmpAngleIn;
                        }

                        for(int k=0; k<tRotate.RotationTimeOutRT; k++)          //Ifor 20170412 (Steven) 判斷Out Rotate 旋轉角度是否已存在
                        {
                            if(tRotate.OutRotationCountRT[k]==tmpAngleOut)
                                bOutAngleNotExistRT=false;
                        }

                        if(bOutAngleNotExistRT==true)
                        {
                            tRotate.OutRotationCountRT[tRotate.RotationTimeOutRT]=tmpAngleOut;
                            tRotate.RotationTimeOutRT++;
                        }
                        bOutAngleNotExistRT=true;                               //避免有一次相同角度後無法再次更新
                    }
                }
            }
        }
    }
    else
    {
        ZeroMemory(&tRotate.RotateDutDate ,sizeof(tRotate.RotateDutDate));
    }

    if(bShowRotateBySite==true)                                                 //Steven 20170329 (wei) : Add individual rotate motor
    {
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<8; j++)
            {
    ;   // golden RotateKit/fRotate.cpp:530-530 取代：純畫面：FrmRotDegree[2][4][8] 是 golden FormCreate（:85）建的 64 個 TfrmRot 動態表單（RotateKit/aRotateDegreeClass.cpp，移植樹沒有）；SetDegree（aRotateDegreeClass.cpp:49-58）只設 rgRotateDegree->ItemIndex 與 ReloadPicture。下一行 tRotate.RotateDutDate[1] 的就地改寫（資料）照 golden 保留
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:530-530 -- 純畫面：FrmRotDegree[2][4][8] 是 golden FormCreate（:85）建的 64 個 TfrmRot 動態表單（RotateKit/aRotateDegreeClass.cpp，移植樹沒有）；SetDegree（aRotateDegreeClass.cpp:49-58）只設 rgRotateDegree->ItemIndex 與 ReloadPicture。下一行 tRotate.RotateDutDate[1] 的就地改寫（資料）照 golden 保留
                FrmRotDegree[0][i][j]->SetDegree(tRotate.RotateDutDate[0][i][j]/90);
#endif // GATE (S12-C save)

                //AI(ht9045-outarm-flow) 20260903 (RogerYang) : 讀檔不可就地換算 Out 角度 (偉測 2-site Out Rotate 存不上根因)
                //  原式 RotateDutDate[1]=360-RotateDutDate[0]+RotateDutDate[1]-360; 化簡就是 Out-=In,
                //  而 fRotate_ReadFile() 每次都會跑: 開工作檔(main.cpp)/開畫面(L127)/按 Save 之後(L1122),
                //  所以每讀一次 Out 就少一個 In 角度, 且無 %360 正規化 -> 累積成 -630 這種非法值。
                //  症狀: 改了存不上(存完 re-read 又減一次, 關畫面 memcpy 固化)、Out Aa/Ab 黃橢圓不見、點位不對。
                //  Out 使用獨立角度時, 檔案值本身就是絕對角度, 直接沿用即可;
                //  需要「相對旋轉量」的地方本來就自己算在區域變數 (見 DoIniDataToForm() 的 tmpAngleOut)。
                if(tRotate.bUseDifferentAngle==true)
                {
                    //AI : 沿用檔案內的絕對角度, 不做任何就地修改
                }
                else
                {
                    tRotate.RotateDutDate[1][i][j]=0-tRotate.RotateDutDate[0][i][j];
                }
    ;   // golden RotateKit/fRotate.cpp:547-547 取代：同上（Out 那一組 TfrmRot 的顯示）
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:547-547 -- 同上（Out 那一組 TfrmRot 的顯示）
                FrmRotDegree[1][i][j]->SetDegree(tRotate.RotateDutDate[1][i][j]/90);
#endif // GATE (S12-C save)
            }
        }
    }

    if(iRotate_Type==eInOutArm1Motor)                                           //Ifor 20251210 add:Rotate Offset
    {
        for(int i=0; i<2; i++)
        {
            if(i==0)
                str.sprintf("In Rotate Offset");
            else
                str.sprintf("Out Rotate Offset");
            tRotate.iRotateOffset[i]=ReadIniData(szDir, "SETTING", str, 0);
        }
    }

    if(iRotate_Type==eInOutArm1Motor && USE_PICKER_COUNT==ep1Picker)            //Ifor 20260901 add:Pre Rotate degree before pick
    {
        for(int i=0; i<2; i++)
        {
            if(i==0)
                str.sprintf("In Pre Rotate");
            else
                str.sprintf("Out Pre Rotate");
            tRotate.iPreRotate[i]=ReadIniData(szDir, "SETTING", str, 0);
        }
    }

    slistIn->Clear();                                                           //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    slistOut->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    slistInRT->Clear();
    slistOutRT->Clear();
    delete slistIn;
    delete slistOut;                                                            //Ifor 20170407 add Rotate Out 獨立旋轉角度
    delete slistInRT;
    delete slistOutRT;

}

// golden RotateKit/fRotate.cpp:586  TFrmRotate::DoIniDataToForm()
static void RT_DoIniDataToForm()
{

    EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked             =tRotate.ActiveRotate;
    EL<TCheckBox>("TFrmRotate", "cbOutRotateDifferentAngle")->Checked  =tRotate.bUseDifferentAngle;
    if(CosFunction.bPassBinNoRotate==true)                                      //jou 20231020 : Pass bin no rotate
        EL<TCheckBox>("TFrmRotate", "chkPassBinNoRotate")->Checked     =tRotate.bPassBinNoRotate;
    if(CosFunction.bRotateUseRTmode==true)                                      //jou 20231122 : Rotate Use RT mode
        EL<TCheckBox>("TFrmRotate", "chkRotateUseRTmode")->Checked     =tRotate.bRotateUseRTmode;
    if(CosFunction.bART_RT_NoRotate==true)                                      //Sam 20240809 : ART RT No Rotate
        EL<TCheckBox>("TFrmRotate", "chkART_RTNoRotate")->Checked      =tRotate.bART_RT_NoRotate;
    EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex                 =tRotate.DutNum;
    iFromTrayAngle                      =tRotate.FromTrayAngle;
    memcpy(&iRotateDutDate, &tRotate.RotateDutDate, sizeof(tRotate.RotateDutDate));

    if(EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex==tDutType_8)                                         //20140320 wei   固定 4 Rotate Xpitch= 80mm; 8 Rotate Xpitch= 40mm
    {
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Text = tRotate.RotateKit_PitchX;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Text = tRotate.RotateKit_PitchY;
    }
    else if(EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex==tDutType_1)                                    //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Text=0;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Text=0;
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Visible=false;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Visible=false;
    }
    else if(EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex==tDutType_2)                                    //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Text=80;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Text=0;
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Enabled=false;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Visible=false;
    }
    else //if(rgDutNum->ItemIndex==tDutType_4)
    {
        EL<TLabeledEdit>("TFrmRotate", "XPitch")->Text = tRotate.RotateKit_PitchX;
        EL<TLabeledEdit>("TFrmRotate", "YPitch")->Text = tRotate.RotateKit_PitchY;
    }

    if(iRotate_Type==eInOutArm1Motor)                                           //Ifor 20251210 add:Rotate Offset
    {
        for(int i=0; i<2; i++)
        {
            Rotate_Offset[i]->Text=tRotate.iRotateOffset[i];
        }
    }

    if(iRotate_Type==eInOutArm1Motor && USE_PICKER_COUNT==ep1Picker)            //Ifor 20260901 add:Pre Rotate degree before pick
    {
        for(int i=0; i<2; i++)
        {
            Pre_Rotate[i]->Text=tRotate.iPreRotate[i];
        }
    }
    SetWorkParameter();

}

// golden RotateKit/fRotate.cpp:1036  TFrmRotate::spbSaveClick(TObject *Sender)
static void RT_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden RotateKit/fRotate.cpp:1042-1042 取代：Close()：golden A02（Operator 權限）時關表單（下一行 return，golden AI 20260907 註解）；網頁端記 closed，沒寫檔
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:1042-1042 -- Close()：golden A02（Operator 權限）時關表單（下一行 return，golden AI 20260907 註解）；網頁端記 closed，沒寫檔
        Close();
#endif // GATE (S12-C save)
    return;   // golden RotateKit/fRotate.cpp:1043-1043 取代：AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 fRotate.cpp:982-987：A02 分支只有 Close()（:986），後面沒有 return，存檔照跑；主選單 main.cpp:29653 用 Show 開（非對話框），Close() 當場跑 FormClose（:1141-1150），其中 DoIniDataToForm()（:1146，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。V912 fRotate.cpp:1042-1043：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）；這句 return 是 V912 AI(ht9045-a012-save-guard) 20260907 加的（原註解在下面 #if 0 裡）。Kept: #20 exception (Steven 1003 Q78, keep V912)
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:1043-1043 -- AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。906 fRotate.cpp:982-987：A02 分支只有 Close()（:986），後面沒有 return，存檔照跑；主選單 main.cpp:29653 用 Show 開（非對話框），Close() 當場跑 FormClose（:1141-1150），其中 DoIniDataToForm()（:1146，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。V912 fRotate.cpp:1042-1043：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）；這句 return 是 V912 AI(ht9045-a012-save-guard) 20260907 加的（原註解在下面 #if 0 裡）。Kept: #20 exception (Steven 1003 Q78, keep V912)
        return;                                                                //AI(ht9045-a012-save-guard) 20260907 : Close() does not abort this method, so without this return the whole save below still ran and the A01_2 "operator may not change settings" guard did not actually block anything.
#endif // GATE (S12-C save)
    }

    AnsiString tmpA="", tmpB="", tmpC="", tmpD="",tmpB_RT="",tmpD_RT="";
    AnsiString S="", str;
    bool bSettingErr = true;

    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\Rotate.Data", DataPath, S);
    EL<TLabel>("TFrmRotate", "Label_Err")->Visible =false;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<8; j++)
        {
            tmpA.sprintf("%0d,", iRotateDutDate[0][i][j]);
            tmpB+=tmpA;

            tmpC.sprintf("%0d,", iRotateDutDate[1][i][j]);                      //Ifor 20170407 add Rotate Out 獨立旋轉角度
            tmpD+=tmpC;

            if(iFromTrayAngle!=iRotateDutDate[0][i][j] ||
               iFromTrayAngle!=iRotateDutDate[1][i][j])
                bSettingErr=false;

            if(CosFunction.bRotateUseRTmode==true)                              //jou 20231122 : Rotate Use RT mode
            {
                tmpA.sprintf("%0d,", iRotateDutDate[2][i][j]);
                tmpB_RT+=tmpA;

                tmpC.sprintf("%0d,", iRotateDutDate[3][i][j]);                  //Ifor 20170407 add Rotate Out 獨立旋轉角度
                tmpD_RT+=tmpC;

                if(iFromTrayAngle!=iRotateDutDate[2][i][j] ||
                   iFromTrayAngle!=iRotateDutDate[3][i][j])
                    bSettingErr=false;
            }
        }
    }

    if(bSettingErr)
    {
        EL<TLabel>("TFrmRotate", "Label_Err")->Visible =true;
        EL<TLabel>("TFrmRotate", "Label_Err")->Caption ="*與入料角度相同不存檔!!!";
        return;
    }

    if(tRotate.ActiveRotate==true &&                                            //KevinCheng 20260318 : Rotate流程未完，不可關閉。
       EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked==false &&
       IsRotateFlowNotFinish())
    {
        EL<TLabel>("TFrmRotate", "Label_Err")->Visible =true;
        EL<TLabel>("TFrmRotate", "Label_Err")->Caption ="*Rotate流程未完成，不可關閉Active Rotate!!!";
        EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked=true;
        return;
    }

    if(CosFunction.bART_RT_NoRotate)                                            //Sam 20240809 : ART RT No Rotate
    {
    filerw::ELTodo("golden RotateKit/fRotate.cpp:1103-1110 ART RT No Rotate 的「每格角度都要相同」檢查掃 sgRotateIn->RowCount×ColCount：替身沒有格子大小（golden SetGridDraw :650-682 在開頁時依 rgDutNum 設成 4x2／1x1／2x1／2x2），而且 golden 的 iR 直接當 iRotateDutDate 第二維（上限 4）用，格子大小不對會越界 —— 頁面接上（FormShow＋SetGridDraw 加進 METHODS）之前不跑，回報 todo");
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:1103-1110 -- ART RT No Rotate 的「每格角度都要相同」檢查掃 sgRotateIn->RowCount×ColCount：替身沒有格子大小（golden SetGridDraw :650-682 在開頁時依 rgDutNum 設成 4x2／1x1／2x1／2x2），而且 golden 的 iR 直接當 iRotateDutDate 第二維（上限 4）用，格子大小不對會越界 —— 頁面接上（FormShow＋SetGridDraw 加進 METHODS）之前不跑，回報 todo
        for(int iR=0; iR<EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->RowCount; iR++)
        {
            for(int iC=0; iC<EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->ColCount; iC++)
            {
                if(iRotateDutDate[0][0][0]!=iRotateDutDate[0][iR][iC])
                    bSettingErr=true;
            }
        }
#endif // GATE (S12-C save)

        if(EL<TCheckBox>("TFrmRotate", "cbOutRotateDifferentAngle")->Checked)
            bSettingErr=true;

        if(bSettingErr)
        {
            if(bCanRunSCKART && EL<TCheckBox>("TFrmRotate", "chkART_RTNoRotate")->Checked)
            {
                EL<TLabel>("TFrmRotate", "Label_Err")->Visible =true;
                EL<TLabel>("TFrmRotate", "Label_Err")->Caption ="*ART RT No Rotate* function only support the same rotate angle!!!";
                return;
            }
            else
            {
                EL<TCheckBox>("TFrmRotate", "chkART_RTNoRotate")->Checked=false;
            }
        }
    }

    filerw::ELMark("RT_WriteIniData"); WriteIniData(szDir, "SETTING", "ActiveRotate",  EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked);   // golden RotateKit/fRotate.cpp:1130-1130 取代：存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02、「與入料角度相同不存檔」、Active Rotate 流程保護、ART RT 檢查都在它之前 return（沒寫檔）
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:1130-1130 -- 存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02、「與入料角度相同不存檔」、Active Rotate 流程保護、ART RT 檢查都在它之前 return（沒寫檔）
    WriteIniData(szDir, "SETTING", "ActiveRotate",  EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked);
#endif // GATE (S12-C save)
    WriteIniData(szDir, "SETTING", "Dut Num",       EL<TRadioGroup>("TFrmRotate", "rgDutNum")->ItemIndex);
    WriteIniData(szDir, "SETTING", "FromTrayAngle", iFromTrayAngle);

    tmpB.Delete(tmpB.Length() ,1);
    WriteIniData(szDir, "SETTING", "DutAngle",          tmpB);

    tmpD.Delete(tmpD.Length() ,1);                                              //Ifor 20170407 add Rotate Out 獨立旋轉角度
    WriteIniData(szDir, "SETTING", "DutAngleOut",       tmpD);

    if(CosFunction.bRotateUseRTmode==true)                                      //jou 20231122 : Rotate Use RT mode
    {
        tmpB_RT.Delete(tmpB_RT.Length() ,1);
        WriteIniData(szDir, "SETTING", "DutAngleRT",          tmpB_RT);

        tmpD_RT.Delete(tmpD_RT.Length() ,1);                                    //Ifor 20170407 add Rotate Out 獨立旋轉角度
        WriteIniData(szDir, "SETTING", "DutAngleOutRT",       tmpD_RT);
    }

    WriteIniData(szDir, "SETTING", "OutRotateDifferentAngle",  EL<TCheckBox>("TFrmRotate", "cbOutRotateDifferentAngle")->Checked);
    if(CosFunction.bPassBinNoRotate==true)                                      //jou 20231020 : Pass bin no rotate
        WriteIniData(szDir, "SETTING", "bPassBinNoRotate",         EL<TCheckBox>("TFrmRotate", "chkPassBinNoRotate")->Checked);
    if(CosFunction.bRotateUseRTmode==true)                                      //jou 20231122 : Rotate Use RT mode
        WriteIniData(szDir, "SETTING", "bRotateUseRTmode",         EL<TCheckBox>("TFrmRotate", "chkRotateUseRTmode")->Checked);
    if(CosFunction.bART_RT_NoRotate==true)                                      //Sam 20240809 : ART RT No Rotate
        WriteIniData(szDir, "SETTING", "bART_RT_NoRotate",         EL<TCheckBox>("TFrmRotate", "chkART_RTNoRotate")->Checked);
    if(USE_ROTATE_KIT==1 && bShowRotateBySite==true)
    {
//        WriteIniData(szDir, "SETTING", "ColCount",      tRotate.ColCount);
//        WriteIniData(szDir, "SETTING", "RowCount",      tRotate.RowCount);
    }
    else
    {
    filerw::ELTodo("golden RotateKit/fRotate.cpp:1163-1164 ColCount／RowCount 寫的是 sgRotateIn 的格子大小（golden SetGridDraw 開頁時設）：替身沒有，照寫會把錯的值寫進 Rotate.Data —— 頁面接上之前不寫，回報 todo（這兩鍵保持檔案原值）");
#if 0 // GATE (S12-C save) golden RotateKit/fRotate.cpp:1163-1164 -- ColCount／RowCount 寫的是 sgRotateIn 的格子大小（golden SetGridDraw 開頁時設）：替身沒有，照寫會把錯的值寫進 Rotate.Data —— 頁面接上之前不寫，回報 todo（這兩鍵保持檔案原值）
        WriteIniData(szDir, "SETTING", "ColCount",      EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->ColCount);
        WriteIniData(szDir, "SETTING", "RowCount",      EL<filerw::ELStringGrid>("TFrmRotate", "sgRotateIn")->RowCount);
#endif // GATE (S12-C save)
    }
    WriteIniData(szDir, "SETTING", "KitPitchX",     atof(EL<TLabeledEdit>("TFrmRotate", "XPitch")->Text.c_str()));
    WriteIniData(szDir, "SETTING", "KitPitchY",     atof(EL<TLabeledEdit>("TFrmRotate", "YPitch")->Text.c_str()));

    //AI(ht9045-prerotate-save-order) 20260907 : these two blocks used to sit AFTER fRotate_ReadFile()
    //  below. Save therefore re-loaded the OLD ini values into tRotate and only afterwards wrote the new
    //  UI values to the ini file, so tRotate.iPreRotate / tRotate.iRotateOffset kept the old value and
    //  the feature did nothing until the program was restarted (Pre Rotate looked completely dead).
    //  Moved above the re-read so Save takes effect at once, like every other setting in this function
    //  (see the OutRotateDifferentAngle write further up).
    if(iRotate_Type==eInOutArm1Motor)                                           //Ifor 20251210 add:Rotate Offset
    {
        for(int i=0; i<2; i++)
        {
            if(i==0)
                str.sprintf("In Rotate Offset");
            else
                str.sprintf("Out Rotate Offset");

            WriteIniData(szDir, "SETTING", str, Rotate_Offset[i]->Text);
        }
    }

    if(iRotate_Type==eInOutArm1Motor && USE_PICKER_COUNT==ep1Picker)            //Ifor 20260901 add:Pre Rotate degree before pick
    {
        for(int i=0; i<2; i++)
        {
            if(i==0)
                str.sprintf("In Pre Rotate");
            else
                str.sprintf("Out Pre Rotate");

            WriteIniData(szDir, "SETTING", str, Pre_Rotate[i]->Text);
        }
    }

    RT_fRotate_ReadFile();
    if(IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;                                                       //Ifor 20151204 新增判斷機台有無修改設定檔
    }

    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data

}

