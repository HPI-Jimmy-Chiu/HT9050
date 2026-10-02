// =============================================================================
//  MyVacuumPanel.cpp  --  TMyVacuumPanel definitions (see MyVacuumPanel.h for
//                          the full ROLE / ZERO CONSUMERS / GATE REGISTER
//                          writeup -- re-cited briefly at each call site here).
//
//  Faithful translation of golden VacuumUnit/MyVacuumPanel.cpp (581 lines,
//  BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  First file to land.
//
//  golden_fns = 21: ctor(1) dtor(2) ShowCurectVal(3) ShowThreshold(4)
//  SetEvent(5) SetBG(6) edSVClick(7) btnSVClick(8) btnVaccumOnOffOnClick(9)
//  RefreshCurrectVal(10) RefreshThresholdVal(11) RefreshVaccumIO(12)
//  RefreshEvent(13) RefreshDOIO(14) ChangeBtnPanelLane(15)
//  ReadVaccumCurrect(16) ReadVaccumThreshold(17) ReadVaccumIO(18)
//  WriteVaccumThreshold(19) InitialThresholdMode(20) Reset(21).
//  ported_fns = 20: every one of the 21 gets a real definition here EXCEPT
//  ChangeBtnPanelLane, which is OMITTED per
//  MyVacuumPanel.h's GATE (2) (its sole parameter type `TBtnPanelLane*` cannot
//  be spelled anywhere in this port -- the identical class of omission
//  EJ1N/MyOmronPanel.cpp already applied to GroupBox1MouseUp/Down/Move).  The
//  other 20 are ACTIVE, VERBATIM-shaped bodies (several with an internal
//  golden-verbatim `#if 0` span per MyVacuumPanel.h's GATE REGISTER).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "MyVacuumPanel.h"
//   #include "myQwertyKeyBoard.h"   -- NOT included, see MyVacuumPanel.h GATE (3)
//   #include "cmydef.h"
//   #include "VacuumUnit.h"
//   #include "AdvMotDrv.h"
//   #include "AdvMotApi.h"
//   #include "MyKitSuck.h"          -- NOT included: nothing in the translated
//                                      bodies below reaches a TMySucker member.
//   #include "MyLaneIo.h"
//   #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella)
#include "VacuumUnit/MyVacuumPanel.h"     // this unit's own contract
#include "cmydef.h"                 // uiDevhand (EtherCAT PCI1203 device handle)
#include "VacuumUnit/VacuumUnit.h"        // TfVacuumUnit (this wave's sibling file #2) + _fVacuumUnit's type
#include "MyLaneIo.h"                // MyLaneIO (GetIOValue/GetIOValueThread/SetIOValueThread --
                                    //   these already internally gate their own Acm_* calls behind
                                    //   #if HAVE_PCI1203, see MyLaneIo.cpp:711-786)
#include "MachineType.h"             // ePCI1203
#include "VacuumUnit/Vc8Route.h"     //AI(W906-VACUNIT-1203) 20260930: the ECAT-VC8 route the three direct-Acm_* #else arms below ask (POD; no route = golden's failure arm, as before). On the old blank line, so no line below moves
//AI(W906-PT-W3) 20260807: golden RefreshDOIO/ReadVaccumIO/InitialThresholdMode
//  call Acm_DaqDoGetBitEx/Acm_DaqDiGetBitEx/Acm_DevWriteSDOData DIRECTLY
//  (unlike ReadVaccumCurrect/ReadVaccumThreshold/WriteVaccumThreshold, which
//  reach the same vendor calls indirectly through MyLaneIO and inherit ITS
//  gating for free). This tree's OWN established, REUSED convention for every
//  other direct Acm_* caller (EtherCAT/MyEtherCAT.cpp, EtherCAT/MyNUEC1.cpp,
//  MyLaneIo.cpp itself) is: include the vendor header AND make every call
//  conditional on `#if HAVE_PCI1203` (undefined/0 by DEFAULT in this tree
//  today -- confirmed via EtherCAT/MyEtherCAT.cpp's own "DEFAULT (HAVE_PCI1203
//  undefined)" citations), with a same-shape safe-default `#else` arm. Applied
//  identically here, at each of the 3 call sites below, rather than gating the
//  whole functions -- this is NOT one of MyVacuumPanel.h's 3 GATE-REGISTER
//  entries (this is real, reachable substrate whose OWN header is
//  build-flag-conditional, not a missing-type/missing-member gap).
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
#if HAVE_PCI1203
#include "EtherCAT/AdvMotCompat.h"            // AI(W906-1203HAL-1) 20260820: ADVCMNAPI shim -> vendor AdvMotApi.h (pulls in AdvMotDev.h/AdvMotDrv.h/
                                    //   AdvMotPropID.h/AdvMotErr.h itself) -- Acm_DaqDoSetBitEx/
                                    //   Acm_DaqDoGetBitEx/Acm_DaqDiGetBitEx/Acm_DevWriteSDOData/
                                    //   ECAT_TYPE_I16, same single-include idiom EtherCAT/
                                    //   MyEtherCAT.cpp:114 and MyLaneIo.cpp:113 already use
#ifndef SUCCESS
#define SUCCESS 0UL
#endif
#endif
//---------------------------------------------------------------------------
TfVacuumUnit *_fVacuumUnit;
//---------------------------------------------------------------------------
TMyVacuumPanel::TMyVacuumPanel(TComponent* Owner, int _Kind ,int _iCol,int _iRow) : TComponent(Owner)
{
    AnsiString str="",str1="",str2="",str3="";

    AnsiString sIndexName_16[2][8]={{"    Aa    ","    Ab    ","    Ac    ","    Ad    ","    Ae    ","    Af    ","    Ag    ","    Ah    "},
                                    {"    Ba    ","    Bb    ","    Bc    ","    Bd    ","    Be    ","    Bf    ","    Bg    ","    Bh    "}};

    AnsiString sInOutName_8[2][4]={{"     A    ","     C    ","     E    ","     G    "},
                                   {"     B    ","     D    ","     F    ","     H    "}};

    //HT9046LS  IndexArm1 2 排序不同，所以分開兩個

    int iIndexVCSort1_16[2][8]={{0,2,4,6,0,2,4,6},
                                {1,3,5,7,1,3,5,7}};

    int iIndexVCSort2_16[2][8]={{3,1,7,5,3,1,7,5},
                                {2,0,6,4,2,0,6,4}};

    int iInOutVCSort[2][4]={{0,1,2,3},
                            {4,5,6,7}};

    iKind=_Kind;      //0:IndexArm1 ; 1:IndexArm2 ; 2:InArm ; 3:OutArm
    iCol=_iCol;
    iRow=_iRow;

    //Ring
    OnRing=1;
    OffRing=1;
    SenRing=1;
    //Type
    OnType=1;
    OffType=1;
    SenType=1;
    //ISABase
    OnISABase=ePCI1203;
    OffISABase=ePCI1203;
    SenISABase=ePCI1203;

    if(iKind==0)         //IndexArm1 HT9046LS
    {
        iVacuOnOffSwap=1;
        str.sprintf("%s", sIndexName_16[iRow][iCol]);
        //IP
        if(iCol<=3)
        {
            OnIP=0x40;
            OffIP=0x40;
            SenIP=0x40;
        }
        else
        {
            OnIP=0x41;
            OffIP=0x41;
            SenIP=0x41;
        }
        //VCNo
        OnVCNo=iIndexVCSort1_16[iRow][iCol];
        OffVCNo=iIndexVCSort1_16[iRow][iCol];
        SenVCNo=iIndexVCSort1_16[iRow][iCol];
    }
    else if(iKind==1)    //IndexArm2 HT9046LS
    {
        iVacuOnOffSwap=0;
        str.sprintf("%s", sIndexName_16[iRow][iCol]);
        //IP
        if(iCol<=3)
        {
            OnIP=0x50;
            OffIP=0x50;
            SenIP=0x50;
        }
        else
        {
            OnIP=0x51;
            OffIP=0x51;
            SenIP=0x51;
        }
        //VCNo
        OnVCNo=iIndexVCSort2_16[iRow][iCol];
        OffVCNo=iIndexVCSort2_16[iRow][iCol];
        SenVCNo=iIndexVCSort2_16[iRow][iCol];
    }
    else if(iKind==2)    //InArm
    {
        iVacuOnOffSwap=1;
        str.sprintf("%s", sInOutName_8[iRow][iCol]);
        //IP
        OnIP=0x20;
        OffIP=0x20;
        SenIP=0x20;
        //VCNo
        OnVCNo=iInOutVCSort[iRow][iCol];
        OffVCNo=iInOutVCSort[iRow][iCol];
        SenVCNo=iInOutVCSort[iRow][iCol];
    }
    else if(iKind==3)    //OutArm
    {
        iVacuOnOffSwap=1;
        str.sprintf("%s", sInOutName_8[iRow][iCol]);
        //IP
        OnIP=0x30;
        OffIP=0x30;
        SenIP=0x30;
        //VCNo
        OnVCNo=iInOutVCSort[iRow][iCol];
        OffVCNo=iInOutVCSort[iRow][iCol];
        SenVCNo=iInOutVCSort[iRow][iCol];
    }
    else if(iKind==4)    //IndexArm1 HT9045
    {
        iVacuOnOffSwap=1;
        str.sprintf("%s", sIndexName_16[iRow][iCol]);
        //IP
        OnIP=0x40;
        OffIP=0x40;
        SenIP=0x40;
        //VCNo
        OnVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
        OffVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
        SenVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
    }
    else if(iKind==5)    //IndexArm2 HT9045
    {
        iVacuOnOffSwap=1;
        str.sprintf("%s", sIndexName_16[iRow][iCol]);
        //IP
        OnIP=0x50;
        OffIP=0x50;
        SenIP=0x50;
        //VCNo
        OnVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
        OffVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
        SenVCNo=iIndexVCSort2_16[iRow][iCol]; //HT9045 的排序原要是 HT9046LS Arm2 排序，只是換個名
    }
    _fVacuumUnit=(TfVacuumUnit *)Owner;
    GroupBox        =new TGroupBox();
    edSV            =new TEdit();
    btnSV           =new TSpeedButton();

    //AI(W906-PT-W3) 20260807: GATE (1)+(2) -- golden's TImage allocation, every
    //  widget geometry/style/Font/BorderStyle/Parent/OnClick/ShowHint/Hint
    //  property golden's ctor sets on ImgVacuumPanel/edSV/btnSV, and the
    //  TMyLed/TBtnPanelLane allocation+geometry+wiring entirely (golden
    //  :150-171,178-182,184-187,191-205,209,216-230,233,235-249,252).  None of
    //  Left/Top/Width/Height/Parent/Font/BorderStyle/OnClick/ShowHint/Hint
    //  exist on vclcompat::TControl by design (GATE 1); TMyLed/TBtnPanelLane
    //  have no widget-level port at all (GATE 2). Preserved verbatim:
#if 0
    ImgVacuumPanel  =new TImage(this);
    myld1           =new TMyLed(this);
    bplOn           =new TBtnPanelLane(this);
    bplOff          =new TBtnPanelLane(this);

    GroupBox->Width     =VACUUM_UNIT_WIDTH;
    GroupBox->Height    =VACUUM_UNIT_HEIGHT;
    GroupBox->Color     =clBtnFace;

    //背景
    ImgVacuumPanel->Parent=GroupBox;
    ImgVacuumPanel->Top=20;
    ImgVacuumPanel->Left=4;
    ImgVacuumPanel->Width=VACUUM_UNIT_WIDTH-8;
    ImgVacuumPanel->Height=VACUUM_UNIT_HEIGHT-25;
    ImgVacuumPanel->Canvas->Font->Name ="Arial";
#endif
    GroupBox->Visible   =false;
    GroupBox->Caption=str;

    SetBG        (clGray, _fVacuumUnit->pnlCurectVal->Color);                   //設定背景
    //AI(W906-PT-W3) 20260807: GATE (1) -- golden's 2nd arg is
    //  `_fVacuumUnit->pnlCurectVal->Font->Color`/`->pnlThreshold->Font->Color`
    //  (golden :173-174). vclcompat::TPanel carries Caption+Color, NOT a Font
    //  member (see Controls.h) -- the chain cannot be evaluated. Substituted
    //  with the same-signature literal TColor(0): inert either way, because
    //  ShowCurectVal/ShowThreshold's Color parameter feeds ONLY the (gated,
    //  GATE 1) Canvas paint, never any other state. See MyVacuumPanel.h GATE (1).
    ShowCurectVal ("0.0", TColor(0));             //顯示目前壓力值 (golden: _fVacuumUnit->pnlCurectVal->Font->Color)
    ShowThreshold ("0.0", TColor(0));             //顯示目前壓力值下限值 (golden: _fVacuumUnit->pnlThreshold->Font->Color)
    SetEvent     ("Event",       clBlack);                                      //警報事件

    //數值輸入
#if 0
    edSV->Parent        =GroupBox;
    edSV->Top           =_fVacuumUnit->edSV->Top;
    edSV->Left          =_fVacuumUnit->edSV->Left;
    edSV->Width         =_fVacuumUnit->edSV->Width;
    edSV->Height        =_fVacuumUnit->edSV->Height;
#endif
    edSV->Text          =_fVacuumUnit->edSV->Text;
#if 0
    edSV->Font->Name    =_fVacuumUnit->edSV->Font->Name;
    edSV->Font->Size    =_fVacuumUnit->edSV->Font->Size;
    edSV->Font->Charset =_fVacuumUnit->edSV->Font->Charset;
    edSV->BorderStyle   =_fVacuumUnit->edSV->BorderStyle;
    edSV->OnClick       =edSVClick;
#endif

    //數值設定
#if 0
    btnSV->Parent   =GroupBox;
    btnSV->Top      =_fVacuumUnit->btnSV->Top;
    btnSV->Left     =_fVacuumUnit->btnSV->Left;
    btnSV->Width    =_fVacuumUnit->btnSV->Width;
    btnSV->Height   =_fVacuumUnit->btnSV->Height;
#endif
    btnSV->Caption  =_fVacuumUnit->btnSV->Caption;
#if 0
    btnSV->OnClick  =btnSVClick;
#endif

    //數值 On/Off
#if 0
    myld1->Parent   =GroupBox;
    myld1->Top      =_fVacuumUnit->myld1->Top;
    myld1->Left     =_fVacuumUnit->myld1->Left;
    myld1->Width    =_fVacuumUnit->myld1->Width;
    myld1->Height   =_fVacuumUnit->myld1->Height;
    myld1->ShowHint =true;
#endif
    SenPort=128+SenVCNo;
#if 0
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) VCNo%d", SenRing,SenIP,SenPort,SenVCNo,SenVCNo);
    myld1->Hint     =str;
#endif

    int iSwap1=0,iSwap2=1;
    if(iVacuOnOffSwap==1)
    {
        iSwap1=1;
        iSwap2=0;
    }
    //吸嘴開
#if 0
    bplOn->Parent           =GroupBox;
    bplOn->Left             =_fVacuumUnit->bplOn->Left;
    bplOn->Top              =_fVacuumUnit->bplOn->Top;
    bplOn->Width            =_fVacuumUnit->bplOn->Width;
    bplOn->Height           =_fVacuumUnit->bplOn->Height;
    bplOn->Caption          =_fVacuumUnit->bplOn->Caption;
    bplOn->Style            =_fVacuumUnit->bplOn->Style;
    bplOn->Color            =_fVacuumUnit->bplOn->Color;
    bplOn->TrueColor        =_fVacuumUnit->bplOn->TrueColor;
    bplOn->TrueFontColor    =_fVacuumUnit->bplOn->TrueFontColor;
    bplOn->FalseColor       =_fVacuumUnit->bplOn->FalseColor;
    bplOn->FalseFontColor   =_fVacuumUnit->bplOn->FalseFontColor;
    bplOn->OnClick          =btnVaccumOnOffOnClick;
    bplOn->ShowHint         =true;
#endif
    OnPort=16+OnVCNo*2+iSwap1;
#if 0
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) VCNo%d", OnRing,OnIP,OnPort,OnVCNo,OnVCNo);
    bplOn->Hint             =str;
#endif

    //吸嘴關
#if 0
    bplOff->Parent          =GroupBox;
    bplOff->Left            =_fVacuumUnit->bplOff->Left;
    bplOff->Top             =_fVacuumUnit->bplOff->Top;
    bplOff->Width           =_fVacuumUnit->bplOff->Width;
    bplOff->Height          =_fVacuumUnit->bplOff->Height;
    bplOff->Caption         =_fVacuumUnit->bplOff->Caption;
    bplOff->Style           =_fVacuumUnit->bplOff->Style;
    bplOff->Color           =_fVacuumUnit->bplOff->Color;
    bplOff->TrueColor       =_fVacuumUnit->bplOff->TrueColor;
    bplOff->TrueFontColor   =_fVacuumUnit->bplOff->TrueFontColor;
    bplOff->FalseColor      =_fVacuumUnit->bplOff->FalseColor;
    bplOff->FalseFontColor  =_fVacuumUnit->bplOff->FalseFontColor;
    bplOff->OnClick         =btnVaccumOnOffOnClick;
    bplOff->ShowHint        =true;
#endif
    OffPort                =16+OffVCNo*2+iSwap2;
#if 0
    str.sprintf("(Lane:%d,IP:%d,Port:%d,Bit:%d) VCNo%d", OffRing,OffIP,OffPort,OffVCNo,OffVCNo);
    bplOff->Hint             =str;
#endif

    IO_Index=0x6100;
    IO_SubIndex=0x01;

    CurValue_Index=0x6000;   //電流量測
    CurValue_SubIndex=0x01;

    Threshold_Index=0x8000;
    Threshold_SubIndex=0x13;
    ThresholdMode_SubIndex=0x02;

    DataSize=128;
    bInitialThresholdModeOK=false;
    bNeedReadVaccumThreshold=true;
    dOldCurrectVal=0.0;
    dCurrectVal=0.0;
    dThresholdVal=0.0;
    sEvent="";
    sOldEvent="Init";  sW906Cur="0.0"; sW906Thr="0.0"; sW906Event="Event"; ulW906EventColor=(unsigned long)clBlack; iW906Led=-1; iW906OnDown=-1; iW906OffDown=-1;   //AI(W906-VACUNIT-1203) 20260930: the web page's copy of what the ctor's gated paint shows (golden :173-175 "0.0" / "0.0" / "Event"), nothing read yet. Same line
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: golden body (golden :274-283) is ALREADY fully
//  commented out -- 7 `//delete Xxx;` lines, no live statement at all. This is
//  golden's OWN deliberate no-op (a create-once, process-lifetime UI panel
//  the original author chose not to free), NOT a case of relying on VCL's
//  owned-children auto-free the way EJ1N/MyOmronPanel's dtor was (that one had
//  a documented substrate-limitation reason to diverge into a real free; this
//  one has none -- preserved exactly empty).
TMyVacuumPanel::~TMyVacuumPanel()
{
    //delete GroupBox;
    //delete ImgVacuumPanel;
    //delete edSV;
    //delete btnSV;
    //delete myld1;
    //delete bplOn;
    //delete bplOff;
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::ShowCurectVal(AnsiString CurectVal , TColor Color)  //顯示目前壓力值
{
    if(GroupBox->Visible==false)
        return;
    //AI(W906-PT-W3) 20260807: GATE (1) -- golden's Canvas paint (golden
    //  :289-311) has no substrate (no TImage/TCanvas type in this port, see
    //  MyVacuumPanel.h GATE (1)). The `if(GroupBox->Visible==false) return;`
    //  guard above IS real (vclcompat::TControl::Visible) and is the only
    //  state-carrying effect this function has -- preserved ACTIVE. Verbatim:
#if 0
    int iTextWidth=0, iTextHeight=0,iTextOutX=0,iTextOutY=0;
    int iLeft=0,iTop=0,iHeight=0,iWidth=0;
    AnsiString sEmpty="          ";  //顯示前先顯示空白字元避免殘影
    ImgVacuumPanel->Canvas->Font->Color=Color;
    ImgVacuumPanel->Canvas->Font->Size=_fVacuumUnit->pnlCurectVal->Font->Size;
    ImgVacuumPanel->Canvas->Font->Style =_fVacuumUnit->pnlCurectVal->Font->Style;

    iLeft       =_fVacuumUnit->pnlCurectVal->Left;
    iTop        =_fVacuumUnit->pnlCurectVal->Top-10;
    iHeight     =_fVacuumUnit->pnlCurectVal->Height;
    iWidth      =_fVacuumUnit->pnlCurectVal->Width;

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(sEmpty);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(sEmpty);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, sEmpty);

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(CurectVal);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(CurectVal);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, CurectVal);
#endif
    sW906Cur=CurectVal; (void)Color;   //AI(W906-VACUNIT-1203) 20260930: the text golden paints, kept for HW.VacuumUnit (the colour is always pnlCurectVal's font colour)
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::ShowThreshold(AnsiString ThresholdVal, TColor Color)  //顯示目前壓力值下限值
{
    if(GroupBox->Visible==false)
        return;

    //AI(W906-PT-W3) 20260807: GATE (1) -- 100% Canvas paint, no other state
    //  beyond the guard above (see ShowCurectVal's identical note). Verbatim:
#if 0
    int iTextWidth=0, iTextHeight=0,iTextOutX=0,iTextOutY=0;
    int iLeft=0,iTop=0,iHeight=0,iWidth=0;
    AnsiString sEmpty="          ";  //顯示前先顯示空白字元避免殘影
    ImgVacuumPanel->Canvas->Font->Color=Color;
    ImgVacuumPanel->Canvas->Font->Size=_fVacuumUnit->pnlThreshold->Font->Size;
    ImgVacuumPanel->Canvas->Font->Charset=_fVacuumUnit->pnlThreshold->Font->Charset;

    iLeft       =_fVacuumUnit->pnlThreshold->Left;
    iTop        =_fVacuumUnit->pnlThreshold->Top-20;
    iHeight     =_fVacuumUnit->pnlThreshold->Height;
    iWidth      =_fVacuumUnit->pnlThreshold->Width;

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(sEmpty);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(sEmpty);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, sEmpty);

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(ThresholdVal);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(ThresholdVal);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, ThresholdVal);
#endif
    sW906Thr=ThresholdVal; (void)Color;   //AI(W906-VACUNIT-1203) 20260930: the text golden paints, kept for HW.VacuumUnit
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::SetEvent(AnsiString Event, TColor Color)        //警報事件
{
    if(GroupBox->Visible==false)
        return;
    //AI(W906-PT-W3) 20260807: GATE (1) -- 100% Canvas paint, no other state
    //  beyond the guard above. Verbatim:
#if 0
    int iTextWidth=0, iTextHeight=0,iTextOutX=0,iTextOutY=0;
    int iLeft=0,iTop=0,iHeight=0,iWidth=0;
    AnsiString sEmpty="          ";  //顯示前先顯示空白字元避免殘影
    ImgVacuumPanel->Canvas->Font->Color=Color;
    ImgVacuumPanel->Canvas->Font->Size=_fVacuumUnit->pnlEvent->Font->Size;
    ImgVacuumPanel->Canvas->Font->Charset=_fVacuumUnit->pnlEvent->Font->Charset;

    iLeft       =_fVacuumUnit->pnlEvent->Left;
    iTop        =_fVacuumUnit->pnlEvent->Top-10;
    iHeight     =_fVacuumUnit->pnlEvent->Height;
    iWidth      =_fVacuumUnit->pnlEvent->Width;

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(sEmpty);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(sEmpty);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, sEmpty);

    iTextWidth  =ImgVacuumPanel->Canvas->TextWidth(Event);
    iTextHeight =ImgVacuumPanel->Canvas->TextHeight(Event);
    iTextOutX   =iLeft+(iWidth-iTextWidth)/2;
    iTextOutY   =iTop-(iHeight-iTextHeight)/2;
    ImgVacuumPanel->Canvas->TextOutA(iTextOutX, iTextOutY, Event);
#endif
    sW906Event=Event; ulW906EventColor=(unsigned long)Color;   //AI(W906-VACUNIT-1203) 20260930: the text and colour golden paints (OK clGreen / error clRed / Reset clYellow), kept for HW.VacuumUnit
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::SetBG(TColor OutLine, TColor BackGround)        //設定背景
{
    //AI(W906-PT-W3) 20260807: GATE (1) -- 100% Canvas paint, no other golden
    //  state at all (unlike ShowCurectVal/ShowThreshold/SetEvent, golden's
    //  SetBG has no Visible guard to keep -- its whole body is the paint).
    //  Verbatim:
#if 0
    ImgVacuumPanel->Canvas->Brush->Color=BackGround;
    ImgVacuumPanel->Canvas->Pen->Color  =OutLine;
    ImgVacuumPanel->Canvas->Rectangle(TRect(0, 0,ImgVacuumPanel->Width, ImgVacuumPanel->Height));
#endif
    (void)OutLine; (void)BackGround;
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::edSVClick(TObject *Sender)                      //輸入下限值
{
    //AI(W906-PT-W3) 20260807: GATE (3) -- golden `fQwertyKey->ShowQwertyKey(
    //  (TEdit*)Sender, N_DOUBLE, 0, true, -116.0, 148.0);` (golden :382).
    //  fQwertyKey real since FW-QWKEY1 (fc08e09) -- GATE (3) OPENED 20260824
    //  (FW-QWKEY2); call live below, headless no-op until HTEdit GATE (6)
    //  wires the runtime keyboard instance (see MyVacuumPanel.h GATE (3)).
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 0, true, -116.0, 148.0); //小鍵盤
    (void)Sender;
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::btnSVClick(TObject *Sender)                     //設定下限值
{
    (void)Sender;
    dWriteThresholdVal=atof(edSV->Text.c_str());
    WriteVaccumThreshold(dWriteThresholdVal);
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::btnVaccumOnOffOnClick(TObject *Sender)                     //吸嘴開開關
{
    //AI(W906-PT-W3) 20260807: GATE (3) -- golden's ENTIRE body (golden
    //  :391-416) casts `Sender` to `TBtnPanelLane*`, GATE (2)'s type, which
    //  cannot be spelled anywhere in this port (see MyVacuumPanel.h GATE 2/3).
    //  Golden is wired ONLY as bplOn/bplOff's OnClick, and neither member
    //  exists here, so this handler is unreachable in this build either way.
    //  ACTIVE arm: no-op. Kept DECLARED (not omitted, unlike ChangeBtnPanelLane)
    //  because its own signature `(TObject*)` is fully spellable and a future
    //  wave landing BtnPanelCore's widget half may re-wire it. Verbatim:
#if 0
    TBtnPanelLane *Ptr;
    Ptr=(TBtnPanelLane *) Sender;
    int ret=0;
    Ptr->Down=!Ptr->Down;
    if(Ptr->Down)
    {
        if(Ptr->Caption=="^")   //吸嘴開開
            ret=Acm_DaqDoSetBitEx(uiDevhand, OnRing, OnIP, OnPort, (OnType==1)?1:0);
        else                    //開嘴開開
            ret=Acm_DaqDoSetBitEx(uiDevhand, OffRing, OffIP, OffPort, (OffType==1)?1:0);
    }
    else
    {
        if(Ptr->Caption=="^")   //吸嘴開關
            ret=Acm_DaqDoSetBitEx(uiDevhand, OnRing, OnIP, OnPort, (OnType==1)?0:1);
        else                    //開嘴開關
            ret=Acm_DaqDoSetBitEx(uiDevhand, OffRing, OffIP, OffPort, (OffType==1)?0:1);
    }
    ChangeBtnPanelLane(Ptr);

    if(ret!=0)
    {
    }
#endif
    (void)Sender;
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::RefreshCurrectVal()
{
    if(sEvent!="")
        return;

    AnsiString s="";
    dCurrectVal=ReadVaccumCurrect();
    if(dOldCurrectVal!=dCurrectVal)
    {
        dOldCurrectVal=dCurrectVal;
        s.sprintf("%0.1f",dCurrectVal);
        //AI(W906-PT-W3) 20260807: GATE (1) -- golden's 2nd arg
        //  `_fVacuumUnit->pnlCurectVal->Font->Color` (golden :429) has no
        //  substrate (TPanel carries no Font member). See ctor's identical
        //  substitution note above.
        ShowCurectVal(s,TColor(0));
    }
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::RefreshThresholdVal()
{
    if(sEvent!="")
        return;

    AnsiString s="";
    if(bNeedReadVaccumThreshold)
    {
        dThresholdVal=ReadVaccumThreshold();
        s.sprintf("%0.1f",dThresholdVal);
        //AI(W906-PT-W3) 20260807: GATE (1) -- golden's 2nd arg
        //  `_fVacuumUnit->pnlThreshold->Font->Color` (golden :443) has no
        //  substrate (TPanel carries no Font member). Same substitution as
        //  RefreshCurrectVal above.
        ShowThreshold(s,TColor(0));
    }
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::RefreshVaccumIO()
{
    if(sEvent!="")
        return;

    //AI(W906-PT-W3) 20260807: GATE (2) -- golden `myld1->Value=ReadVaccumIO();`
    //  (golden :452). `myld1` has no port (see MyVacuumPanel.h GATE 2). The
    //  real HW read + its real side effect (ReadVaccumIO sets sEvent="Error3"
    //  on an Acm_DaqDiGetBitEx failure) is preserved ACTIVE by calling it and
    //  discarding only the on-screen LED value it would have displayed:
    iW906Led = ReadVaccumIO() ? 1 : 0;   //AI(W906-VACUNIT-1203) 20260930: ...no longer discarded: golden's myld1->Value, kept for HW.VacuumUnit
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::RefreshEvent()
{
    if(sOldEvent!=sEvent)
    {
        sOldEvent=sEvent;
        if(sEvent=="")
            SetEvent("OK",clGreen);
        else
            SetEvent(sEvent,clRed);
    }
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::RefreshDOIO()
{
    int Result1=0,Result2=0;
    unsigned char Value1=0, Value2=0;

    //ECAT-VC8 DO0-DO15 16-35
    //AI(W906-PT-W3) 20260807: golden calls Acm_DaqDoGetBitEx directly -- gated
    //  behind #if HAVE_PCI1203 (undefined/0 by default in this tree today),
    //  matching this file's own include-block note and MyLaneIo.cpp's
    //  established idiom for the identical vendor call.
#if HAVE_PCI1203
    Result1=(int)Acm_DaqDoGetBitEx(uiDevhand, OnRing, OnIP, OnPort,  &Value1);
    Result2=(int)Acm_DaqDoGetBitEx(uiDevhand, OffRing, OffIP, OffPort, &Value2);
#else
    Result1=W906_Vc8ReadDo(OnRing, OnIP, OnPort,  &Value1); Result2=W906_Vc8ReadDo(OffRing, OffIP, OffPort, &Value2);   //AI(W906-VACUNIT-1203) 20260930: the two Acm_DaqDoGetBitEx, asked of the ECAT-VC8 route (the monitor's DO read-back of an identity-checked VC8); no route -> non-zero, as the old -1. Same line
#endif

    //AI(W906-PT-W3) 20260807: GATE (2) -- golden's `bplOn->Down=...`/
    //  `bplOff->Down=...` writes (golden :479-489) and the two
    //  ChangeBtnPanelLane(...) calls (golden :491-492) have no substrate
    //  (bplOn/bplOff/ChangeBtnPanelLane all gated, see MyVacuumPanel.h
    //  GATE 2). The real Acm_DaqDoGetBitEx HW reads above are preserved
    //  ACTIVE (Result1/Result2/Value1/Value2 computed for real); only the
    //  on-screen switch-latch display is dropped. Verbatim:
#if 0
    if(Result1==SUCCESS)
    {
        if(Value1>0)
            bplOn->Down=(OnType==1)?true:false;
        else
            bplOn->Down=(OnType==1)?false:true;
    }

    if(Result2==SUCCESS)
    {
        if(Value2>0)
            bplOff->Down=(OffType==1)?true:false;
        else
            bplOff->Down=(OffType==1)?false:true;
    }
    ChangeBtnPanelLane(bplOn);
    ChangeBtnPanelLane(bplOff);
#endif
    //AI(W906-VACUNIT-1203) 20260930: the Down golden gives bplOn / bplOff above, same operands, kept for
    //  HW.VacuumUnit. A failed read leaves golden's button unchanged; here it leaves -1 (unknown) so the page
    //  cannot draw a coil it did not read as off.
    iW906OnDown  = (Result1==0) ? ((Value1>0) ? ((OnType==1)?1:0)  : ((OnType==1)?0:1))  : -1;
    iW906OffDown = (Result2==0) ? ((Value2>0) ? ((OffType==1)?1:0) : ((OffType==1)?0:1)) : -1;
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: GATE (2) -- golden `void TMyVacuumPanel::
//  ChangeBtnPanelLane(TBtnPanelLane *Ptr)` (golden .cpp :495-519) OMITTED
//  entirely: its sole parameter type `TBtnPanelLane*` cannot be spelled
//  anywhere in this port (see MyVacuumPanel.h GATE 2). Golden body kept here
//  verbatim, in comment form, for provenance:
//
//  void TMyVacuumPanel::ChangeBtnPanelLane(TBtnPanelLane *Ptr)
//  {
//      if(Ptr->Down)
//      {
//          Ptr->Color          =Ptr->TrueColor;
//          Ptr->Font->Color    =Ptr->TrueFontColor;
//          Ptr->BevelInner     =bvLowered;
//          Ptr->BevelOuter     =bvLowered;
//      }
//      else
//      {
//          Ptr->Color          =Ptr->FalseColor;
//          Ptr->Font->Color    =Ptr->FalseFontColor;
//          if(Ptr->Style==tsFlatButtons)
//          {
//              Ptr->BevelInner =bvNone;
//              Ptr->BevelOuter =bvNone;
//          }
//          else
//          {
//              Ptr->BevelInner =bvRaised;
//              Ptr->BevelOuter =bvRaised;
//          }
//      }
//  }
//---------------------------------------------------------------------------
double TMyVacuumPanel::ReadVaccumCurrect()
{
    return MyLaneIO.GetIOValue(SenRing,SenIP,SenVCNo,0,SenISABase);
}
//---------------------------------------------------------------------------
double TMyVacuumPanel::ReadVaccumThreshold()
{
    bNeedReadVaccumThreshold=false;
    return MyLaneIO.GetIOValueThread(SenRing,SenIP,SenVCNo,0,SenISABase);
}
//---------------------------------------------------------------------------
bool TMyVacuumPanel::ReadVaccumIO()
{
    //ECAT-VC8 DI0-DI7 128-135
    //AI(W906-PT-W3) 20260807: golden calls Acm_DaqDiGetBitEx directly --
    //  gated behind #if HAVE_PCI1203 (undefined/0 by default), see this
    //  file's include-block note. #else preserves golden's own failure arm
    //  (sEvent="Error3"; return false;) as the safe default -- the real
    //  vendor call cannot report SUCCESS when the card is not built in, so
    //  golden's own else-branch IS the faithful offline outcome.
#if HAVE_PCI1203
    int Result=0;
    unsigned char Value=0;
    Result=(int)Acm_DaqDiGetBitEx(uiDevhand, SenRing, SenIP, SenPort, &Value);
    if(Result==SUCCESS)
    {
        if(Value>0)
            return true;
        else
            return false;
    }
    else
    {
        sEvent="Error3";
        return false;
    }
#else
    //AI(W906-VACUNIT-1203) 20260930: the Acm_DaqDiGetBitEx above, asked of the ECAT-VC8 route (the monitor's
    //  DI sample of an identity-checked VC8, channel 128+VC); golden's own two arms after it, verbatim. No route
    //  (or a refusal: not a VC8, not in the map) -> non-zero -> golden's Error3, as before.
    int Result=0;
    unsigned char Value=0;
    Result=W906_Vc8ReadDi(SenRing, SenIP, SenPort, &Value);
    if(Result==0)
    {
        if(Value>0)
            return true;
        else
            return false;
    }
    else
    {
        sEvent="Error3";
        return false;
    }
#endif
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::WriteVaccumThreshold(double dKpa)
{
    if(MyLaneIO.SetIOValueThread(dKpa,SenRing,SenIP,SenVCNo,0,SenISABase))
        bNeedReadVaccumThreshold=true;
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::InitialThresholdMode()
{
    short iValue=1;   //0:不設定 ; 1:低於設定下限值 ; 2:高於設定下限值 ; 3:等於設定下限值
    if(bInitialThresholdModeOK==false)
    {
        //AI(W906-PT-W3) 20260807: golden calls Acm_DevWriteSDOData directly --
        //  gated behind #if HAVE_PCI1203 (undefined/0 by default), see this
        //  file's include-block note (same vendor call MyLaneIo.cpp's
        //  GetIOValueThread/SetIOValueThread already gate identically).
        //  #else preserves golden's own failure arm (sEvent="Error5") as the
        //  safe default, matching ReadVaccumIO's identical reasoning.
#if HAVE_PCI1203
        unsigned long Result;
        Result=Acm_DevWriteSDOData(uiDevhand, SenRing, SenIP, Threshold_Index+SenVCNo*0x0010, ThresholdMode_SubIndex,  ECAT_TYPE_I16, DataSize, &iValue);
        if(Result==SUCCESS)
            bInitialThresholdModeOK=true;
        else
            sEvent="Error5";
#else
        //AI(W906-VACUNIT-1203) 20260930: the Acm_DevWriteSDOData above, asked of the ECAT-VC8 route (fail closed:
        //  VacuUnitType 1, not running, an identity-checked VC8 in OP, then Pci1203Control::Execute; a DRY RUN
        //  writes nothing and fails). Same object 8000h+VC*10h:02h = 1. DEVIATION (R5): DataSize 2, not 128.
        //  No route -> non-zero -> golden's Error5, as before.
        unsigned long Result;
        Result=(unsigned long)W906_Vc8WriteSdoI16(SenRing, SenIP, Threshold_Index+SenVCNo*0x0010, ThresholdMode_SubIndex, iValue);
        if(Result==0)
            bInitialThresholdModeOK=true;
        else
            sEvent="Error5";
#endif
    }
}
//---------------------------------------------------------------------------
void TMyVacuumPanel::Reset()
{
    sEvent="";
    sOldEvent="Reset";
    SetEvent("Reset",clYellow);
    dOldCurrectVal=0.0;
    bInitialThresholdModeOK=false;
    bNeedReadVaccumThreshold=true;
}
//---------------------------------------------------------------------------

// =============================================================================
//  APPEND BLOCK -- BANNER EXTENSION
//  AI(W906-PT-W8) 20260811 -- golden `ChangeBtnPanelLane`, which this file's
//  original banner listed as "GATE 3, whole method omitted", is landed here as
//  a real, compilable, callable, and -- unlike the sibling mouse handlers this
//  same wave landed in MyTempPanel.cpp and EJ1N/MyOmronPanel.cpp -- a
//  100% ACTIVE, FULLY FAITHFUL body.  ZERO GATES.  Nothing above this line is
//  edited or reordered (append-only).
//
//  WHY THE ORIGINAL OMISSION IS SUPERSEDED -- THIS IS A CORRECTED PREMISE,
//  NOT A RELAXED STANDARD
//  -------------------------------------------------------------------------
//  MyVacuumPanel.h's GATE (2) omitted this method because its parameter type
//  `TBtnPanelLane*` "cannot even be spelled", the tree having "ONLY the
//  framework-free STATE cores ... never the widget classes themselves".
//  The second half of that sentence is TRUE.  The conclusion drawn from it is
//  WRONG FOR THIS PARTICULAR METHOD, and the reason is worth stating exactly,
//  because it is the difference between a gate that is unavoidable and a gate
//  that was assumed:
//
//    Every single property golden's ChangeBtnPanelLane touches --
//    ->Down, ->TrueColor, ->FalseColor, ->Font->Color, ->TrueFontColor,
//    ->FalseFontColor, ->BevelInner, ->BevelOuter, ->Style -- IS PRESENT on
//    vclcompat::BtnPanelLaneCore / BtnPanelCoreBase (vclcompat/BtnPanelCore.h
//    :72-131, :194-224).  That is not a coincidence: BtnPanelCore.h's own
//    banner states that this control family has ZERO drawing code and that
//    "the 'rendering' IS just those 4 TPanel property writes, exposed here as
//    plain public state for a real TPanel/CStatic shell to copy across".
//    ChangeBtnPanelLane IS those 4 property writes.  It is precisely the
//    function that core was built to make portable.
//
//  So the widget half being absent does not gate this method; the STATE half
//  is the whole method.  The parameter type becomes
//  `vclcompat::BtnPanelLaneCore*`.
//
//  WHICH DEFINITION THE PARAMETER TYPE NAMES (the two-headers hazard)
//  -------------------------------------------------------------------------
//  Checked explicitly rather than assumed, because this tree has already been
//  bitten twice by two headers declaring one class name (TMyKitSuck:
//  mykitsuck.h:274 vs aHotPlateSubstrate.h:365, DIFFERENT LAYOUTS;
//  TInLaserCheck: aHotPlateSubstrate.h:836 vs OmronLaser/LaserSensorInArm.h:34):
//    cmd:  grep -rn --include=*.h --include=*.cpp -E
//          '^[[:space:]]*(class|struct)[[:space:]]+(TBtnPanelLane|BtnPanelLaneCore)\b' .
//    run:  2026-08-11 11:23:06
//    ->    EXACTLY ONE hit, vclcompat/BtnPanelCore.h:194, and ZERO hits for
//          `TBtnPanelLane`.  There is no second definition to pick wrongly,
//          and no layout ambiguity.
//
//  NO NEW LINK EDGE IS CREATED (checked, because MyVacuumPanel.cpp lives in
//  ht9045_sm while vclcompat/BtnPanelCore.cpp is compiled ONLY into the UI
//  targets -- CMakeLists.txt's HT9045_UI_CTRL_SOURCES, inside `if(HT9045_UI)`,
//  and its own comment at CMakeLists.txt:318-320 says BtnPanelCore.cpp is
//  deliberately NOT in the vclcompat library).  The body below uses ONLY:
//    - GetDown() / GetTrueColor() / GetFalseColor() / GetTrueFontColor() /
//      GetFalseFontColor() / GetStyle()  -- all defined INLINE in the header
//      (vclcompat/BtnPanelCore.h :85, :91-94, :98)
//    - the plain public data members Color / FontColor / BevelInner /
//      BevelOuter (:78-81)
//  It constructs nothing, destroys nothing, and calls no out-of-line member,
//  so it emits NO undefined reference into BtnPanelCore.cpp and does not
//  require it to be added to any archive.  Verified by -fsyntax-only plus
//  reading which members are inline; stated here so the integrating loop does
//  not "helpfully" add BtnPanelCore.cpp to ht9045_sm on this file's account.
//
//  SHAPE: a file-scope free function, not a member, because PT-W3 removed the
//  declaration from VacuumUnit/MyVacuumPanel.h and that header is outside this
//  wave's write boundary.  golden's body touches NO member of TMyVacuumPanel
//  at all (only its parameter), so it does not even need a `Self`.  See the
//  HAND-OFF block at the end of this file.  Until the loop acts on it, this
//  function has EXTERNAL LINKAGE AND NO CALLER anywhere in the tree.
//
//  WAVE SCOPE -- ONE LINE PER GOLDEN FUNCTION
//   ChangeBtnPanelLane  golden VacuumUnit/MyVacuumPanel.cpp:495-519 -- ACTIVE,
//                       100%, zero gates.
//
//  GATE REGISTER: EMPTY.  This block introduces no `#if 0` and no gated call
//  site.  (The two PRE-EXISTING gated call sites above -- golden :491-492
//  inside RefreshDOIO, and golden :513 inside btnVaccumOnOffOnClick -- are NOT
//  touched by this wave: they are gated on `bplOn`/`bplOff`/`Sender`, i.e. on
//  the missing WIDGET members, which this block does not change.  Retiring
//  those two is a separate, header-touching job; see the HAND-OFF block.)
//
//  VCL/Borland name mapping applied to the body below, one line each, so the
//  correspondence with golden is checkable at a glance:
//    golden `Ptr->Down`            -> Ptr->GetDown()            (:85, inline)
//    golden `Ptr->TrueColor`       -> Ptr->GetTrueColor()       (:91, inline)
//    golden `Ptr->FalseColor`      -> Ptr->GetFalseColor()      (:92, inline)
//    golden `Ptr->Font->Color`     -> Ptr->FontColor            (:79, the
//                                     header's OWN documented stand-in:
//                                     "stand-in for golden's `Font->Color`")
//    golden `Ptr->TrueFontColor`   -> Ptr->GetTrueFontColor()   (:93, inline)
//    golden `Ptr->FalseFontColor`  -> Ptr->GetFalseFontColor()  (:94, inline)
//    golden `Ptr->BevelInner/Outer`-> Ptr->BevelInner/BevelOuter (:81, real)
//    golden `Ptr->Style`           -> Ptr->GetStyle()           (:98, inline)
//    golden bvLowered/bvNone/bvRaised -> vclcompat::TBevelShim  (:63)
//    golden tsFlatButtons          -> vclcompat::TTabStyleShim  (:56)
//  Enum spellings are FULLY QUALIFIED below on purpose: `bvNone`/`bvRaised`/
//  `tsFlatButtons` are short, generic identifiers and this TU pulls in the
//  whole MachineDefine.h umbrella.
//
//  Big5: no Chinese text occurs in golden :495-519.  ZERO U+FFFD.
// =============================================================================
#include "vclcompat/BtnPanelCore.h"   // BtnPanelLaneCore -- the tree's ONLY
                                      //   definition (:194); header-only for
                                      //   every member this file uses
//---------------------------------------------------------------------------
//  golden VacuumUnit/MyVacuumPanel.cpp:495-519  --
//  TMyVacuumPanel::ChangeBtnPanelLane(TBtnPanelLane *Ptr)
//  Ported parameter type is vclcompat::BtnPanelLaneCore* (see BANNER
//  EXTENSION).  Body is otherwise line-for-line golden.
void TMyVacuumPanel_ChangeBtnPanelLane(vclcompat::BtnPanelLaneCore *Ptr)
{
    if(Ptr->GetDown())
    {
        Ptr->Color          =Ptr->GetTrueColor();
        Ptr->FontColor      =Ptr->GetTrueFontColor();
        Ptr->BevelInner     =vclcompat::bvLowered;
        Ptr->BevelOuter     =vclcompat::bvLowered;
    }
    else
    {
        Ptr->Color          =Ptr->GetFalseColor();
        Ptr->FontColor      =Ptr->GetFalseFontColor();
        if(Ptr->GetStyle()==vclcompat::tsFlatButtons)
        {
            Ptr->BevelInner =vclcompat::bvNone;
            Ptr->BevelOuter =vclcompat::bvNone;
        }
        else
        {
            Ptr->BevelInner =vclcompat::bvRaised;
            Ptr->BevelOuter =vclcompat::bvRaised;
        }
    }
}
//---------------------------------------------------------------------------
//  HAND-OFF TO THE INTEGRATING LOOP -- DESCRIBED, DELIBERATELY NOT DONE HERE
//  (VacuumUnit/MyVacuumPanel.h is outside this wave's write boundary.)
//
//  1. To collapse the function above back into a class member, add to
//     VacuumUnit/MyVacuumPanel.h's `private:` block, replacing the
//     commented-out golden signature PT-W3 left at MyVacuumPanel.h:243:
//
//         void ChangeBtnPanelLane(vclcompat::BtnPanelLaneCore *Ptr);
//
//     then rename the definition above to
//     `TMyVacuumPanel::ChangeBtnPanelLane`.  No other edit is needed: the body
//     reads no member of the class.  MyVacuumPanel.h already includes
//     vclcompat/LedCore.h, which defines the same guarded
//     HT9045_W7C1_TCOLOR_SHIM block BtnPanelCore.h carries, so adding
//     `#include "vclcompat/BtnPanelCore.h"` to that header cannot re-define
//     TColor or the clXxx constants.
//
//  2. NOT DONE, and NOT recommended as part of (1): retiring
//     MyVacuumPanel.h's GATE (2) wholesale.  Only the ChangeBtnPanelLane HALF
//     of that gate is now dead.  The `TMyLed *myld1` / `TBtnPanelLane *bplOn`
//     / `*bplOff` MEMBERS are a genuinely different question -- golden's ctor
//     `new`s them and sets ->Parent/->Top/->Left/->Width/->Height/->Hint/
//     ->ShowHint/->Caption on them, and NONE of those geometry properties
//     exist on BtnPanelLaneCore (it is a STATE core, deliberately: see
//     BtnPanelCore.h's banner).  Re-pointing those three members at the cores
//     would make the two gated call sites at golden :479-492 and :391-416
//     un-gateable in part, but it changes this class's construction and its
//     dtor, needs the same decision for MyLedCore, and lands squarely in
//     ht9045_sm -- measure it as its own wave, not as a rider on this one.
//     Also note it would create the FIRST ht9045_sm reference to
//     BtnPanelLaneCore's OUT-OF-LINE members (the ctor, SetDown, SetTrueColor,
//     ...), which -- unlike this block -- WOULD require
//     vclcompat/BtnPanelCore.cpp to be reachable from ht9045_sm.  Today it is
//     compiled only into the UI targets (CMakeLists.txt HT9045_UI_CTRL_SOURCES),
//     so that wave must move it, exactly the way PT-W3-integrate moved
//     vclcompat/TrayCore.cpp into the vclcompat library for the same reason
//     (see CMakeLists.txt:300-330).
//
//  RE-RUN LOG -- the absence/uniqueness claims quoted above were re-run from
//  the tree root at 2026-08-11 11:45, AFTER this block was written and after
//  all sibling files in this wave had landed, with the same results.
//---------------------------------------------------------------------------

// =============================================================================
//  APPEND BLOCK -- AI(W906-VACUNIT-1203) 20260930
//  golden TMyVacuumPanel::btnVaccumOnOffOnClick (golden VacuumUnit/MyVacuumPanel.cpp:391-416), made callable.
//  EastSun 20260930: 「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」.
//
//  The gated handler above (GATE (3)) stays exactly as it is: its parameter is the TBtnPanelLane that was
//  clicked, and golden reads two things from it -- which button (`Ptr->Caption=="^"` : bplOn "^" = 吸真空 /
//  bplOff "v" = 破真空) and its Down AFTER golden's own `Ptr->Down=!Ptr->Down`. Both become parameters; the
//  body below is golden's, line for line, with golden's Acm_DaqDoSetBitEx in the HAVE_PCI1203 arm and the
//  ECAT-VC8 route (VacuumUnit/Vc8Route.h) in the other -- the same arm split every other call in this file has.
//
//  DEVIATIONS (recorded):
//    * golden repaints the button from its own toggled Down (ChangeBtnPanelLane(Ptr)) even when the write
//      failed ("if(ret!=0){}" does nothing). Here nothing is painted: the caller (FileRW/TestIF_File_VacuumUnit.cpp)
//      re-reads the DO with RefreshDOIO() and the page draws the CARD's read-back -- a refused or DRY write
//      must not look applied (same rule as HW.IoSetView, JsonBridge/IoBtnPanelClick.cpp DEVIATION d).
//    * the route refuses switching a channel ON while the other channel of the same VC reads ON (vacuum and
//      blow together; EtherCAT/Pci1203Vc8Route.inc). golden allows it.
// =============================================================================
int TMyVacuumPanel::W906_VaccumOnOff(bool bOnButton, bool bDown)
{
    int ret=0;
    if(bDown)
    {
        if(bOnButton)   //吸真空開
#if HAVE_PCI1203
            ret=(int)Acm_DaqDoSetBitEx(uiDevhand, OnRing, OnIP, OnPort, (OnType==1)?1:0);
#else
            ret=W906_Vc8WriteDo(OnRing, OnIP, OnPort, (unsigned char)((OnType==1)?1:0));
#endif
        else            //破真空開
#if HAVE_PCI1203
            ret=(int)Acm_DaqDoSetBitEx(uiDevhand, OffRing, OffIP, OffPort, (OffType==1)?1:0);
#else
            ret=W906_Vc8WriteDo(OffRing, OffIP, OffPort, (unsigned char)((OffType==1)?1:0));
#endif
    }
    else
    {
        if(bOnButton)   //吸真空關
#if HAVE_PCI1203
            ret=(int)Acm_DaqDoSetBitEx(uiDevhand, OnRing, OnIP, OnPort, (OnType==1)?0:1);
#else
            ret=W906_Vc8WriteDo(OnRing, OnIP, OnPort, (unsigned char)((OnType==1)?0:1));
#endif
        else            //破真空關
#if HAVE_PCI1203
            ret=(int)Acm_DaqDoSetBitEx(uiDevhand, OffRing, OffIP, OffPort, (OffType==1)?0:1);
#else
            ret=W906_Vc8WriteDo(OffRing, OffIP, OffPort, (unsigned char)((OffType==1)?0:1));
#endif
    }
    //  golden: ChangeBtnPanelLane(Ptr); if(ret!=0){} -- see DEVIATIONS above.
    return ret;
}
//---------------------------------------------------------------------------
