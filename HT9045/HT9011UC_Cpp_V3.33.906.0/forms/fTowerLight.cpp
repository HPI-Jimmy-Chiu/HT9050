// =============================================================================
//  forms/fTowerLight.cpp  --  definitions for the fTowerLight facade
//
//  AI(W906-FW3-DTL1) 20260828: new file, FW wave FW3-DTL1.
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cTowerLight.cpp
//  (159 lines) + cTowerLight.h (88 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 8 golden `TfTowerLight::` member bodies, 133 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 8-member denominator:
//     4 ACTIVE           ctor / UpdateTowerLed / Timer1Timer / spbExitClick
//     2 ACTIVE-PARTIAL   FormShow / FormClose -- every line except each
//                        body's trailing `myLog.Do_Log(...)`, GATE (T-2)
//     2 GATED-WITH-BODY  RGB00Click (26L) / rgMusicTestClick (6L), 32 golden
//                        span lines carried as `#if 0` transcript below
//  BY LINES: 101 of 133 golden span lines are ACTIVE(-PARTIAL) (75.9%).
//
//  See forms/fTowerLight.h for the full GATE REGISTER (T-1)/(T-2)/(T-3), the
//  ACTIVE evidence, DEVIATIONS D-1..D-7, the FIELD LIST and the
//  VALUE-PROVENANCE warning about the LEDs' `Tag`.
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall` and renaming `TALed` ->
//  `TfTowerLightLed` per DEVIATION D-5), so it is a faithful TRANSCRIPT --
//  not verified code.
// =============================================================================
#include "forms/fTowerLight.h"
#include "LastSet.h"            // LastSet.MessageLight[][] / LastSet.MusicSelect[] (LastSet.cpp, ht9045_globals)
#include "cmydef.h"             // CUSTOMER_CODE, AccessLevel, SwMusic1 (cmydef.cpp, ht9045_globals)
#include "MachineType.h"        // CC_KYEC_XILINX (:295)
#include "Config.h"             // IniConfig (Config.cpp, ht9045_globals)
#include "CosFunction.h"        // CosFunction (CosFunction.cpp, ht9045_globals)
#include "cprod.h"              // WriteLastDataFile (cprod.cpp, ht9045_globals) -- RGB00Click golden V912 :79   //AI(W906-TOWERLIGHT) 20260925 (Steven 團隊)
#include "canary_support.h"     // ShowMyMessage -- body canary_support.cpp (ht9045_sm), one of the four sanctioned forms->sm exceptions (same as forms/fBuilder.cpp:55)   //AI(W906-TOWERLIGHT) 20260925

// AI(W906-FW3-DTL1) 20260828: TfTowerLight/fTowerLight were FREE tree-wide --
// same idiom as forms/fPrecaution.cpp:14 / forms/fHotPlate.cpp:24 /
// forms/fTesterTCP.cpp:49.  Golden's ctor body (:19-22) is EMPTY past
// `: TForm(Owner)`, so this static-init `new` touches nothing beyond its own
// already-`new`'d widget stand-ins -- no SIOF risk (docs/KNOWLEDGE.md
// "static-init ctor 不可碰 NULL 全域"; the fLaserSensor incident that rule
// comes from turned 88 of 134 ctest binaries into SEGFAULTs).
TfTowerLight *fTowerLight = new TfTowerLight();

TfTowerLight::TfTowerLight()
{
    // golden :19-22 -- empty ctor body past `: TForm(Owner)`.  Widget fields
    // are NSDMI-`new`'d in the header.
    //AI(W906-TOWERLIGHT) 20260925 (Steven 團隊): dfm 設計期的 `Tag`（golden V912 cTowerLight.dfm
    //  RGB00 :85（Tag 省略＝0）、RGB01 :96 Tag=1、RGB02 :228 Tag=2、RGB10 :75 Tag=3 … RGB72 :593 Tag=23，
    //  規則恰為 Tag = i*3+j）。VCL 載入 dfm 時設定、golden 程式碼從不寫 —— 這裡代替 dfm 載入。
    //  RGB00Click 用 Tag/3、Tag%3 分派，沒有這一段整個格子都會落在 [0][0]（header DISCLOSURE 講的就是這件事）。
    //  只寫自己的元件欄位（NSDMI 已 new 好），不碰任何全域 —— static-init 安全。
    TfTowerLightLed *const Led[8][3]=
    {
        {RGB00, RGB01, RGB02},
        {RGB10, RGB11, RGB12},
        {RGB20, RGB21, RGB22},
        {RGB30, RGB31, RGB32},
        {RGB40, RGB41, RGB42},
        {RGB50, RGB51, RGB52},
        {RGB60, RGB61, RGB62},
        {RGB70, RGB71, RGB72},
    };
    for(int i=0; i<8; i++)
        for(int j=0; j<3; j++)
            Led[i][j]->Tag=i*3+j;
    //AI(W906-TOWERLIGHT) 20260925: 同理補 VCL 的預設值 —— vclcompat::TControl 預設 Visible=false／Enabled=false
    //  （vclcompat/Controls.h 的 TControl()），VCL 預設兩者皆 true，dfm 沒有覆寫（golden V912 cTowerLight.dfm 全檔無 Visible／Enabled）。
    //  golden FormShow 只在 :108-120 設 Panel11->Visible、:122-123 設 cbOffLine->Enabled=false（SPIL），從不設回 true。
    for(int i=0; i<8; i++)
        for(int j=0; j<3; j++)
            Led[i][j]->Visible=Led[i][j]->Enabled=true;
    TComboBox *const Cb[8]={cbRunning, cbJam, cbPause, cbMessage, cbHeating, cbHome, cbOffLine, cbART};
    for(int i=0; i<8; i++)
        Cb[i]->Visible=Cb[i]->Enabled=true;
    rgMusicTest->Visible=rgMusicTest->Enabled=true;
    rgMusicTest->ItemIndex=0;                                                   // dfm :541 ItemIndex = 0
    Panel11->Visible=Panel11->Enabled=true;
    palART->Visible=palART->Enabled=true;
    palART->Caption="Auto Retest";                                              // dfm :650
}

// ---------------------------------------------------------------------------
//  golden :24-53.  ACTIVE.  Private in golden too (cTowerLight.h:79).
//
//  NOTE, golden's own shape kept verbatim: `LedPtr` is a FUNCTION-LOCAL
//  `static` initialised from THIS object's members, so the very first call
//  binds the table to whichever instance called it and every later call --
//  from any instance -- reuses that binding.  Golden has exactly one
//  instance so it never shows; recorded here because the port could grow a
//  second one.  Not "fixed" (改行為要留給使用者決定).
// ---------------------------------------------------------------------------
void TfTowerLight::UpdateTowerLed(int Attr)
{
    static int ct=0;
    static bool flag=false;
    if(ct++<5 && Attr==0)
        return;
    ct=0;
    flag=!flag;                                                                 //MessageLight最多就8組, 不能再加!!
    static TfTowerLightLed *LedPtr[8][3]=                                       //Steven 20140409 : Auto Retest
    {
        {RGB00, RGB01, RGB02},
        {RGB10, RGB11, RGB12},
        {RGB20, RGB21, RGB22},
        {RGB30, RGB31, RGB32},
        {RGB40, RGB41, RGB42},
        {RGB50, RGB51, RGB52},
        {RGB60, RGB61, RGB62},
        {RGB70, RGB71, RGB72},
    };

    for(int i=0; i<8; i++)
    {
        for(int j=0; j<3; j++)
        {
            if(     LastSet.MessageLight[i][j]==0)  LedPtr[i][j]->Value=false;
            else if(LastSet.MessageLight[i][j]==1)  LedPtr[i][j]->Value=true;
            else if(LastSet.MessageLight[i][j]==2)  LedPtr[i][j]->Value=flag;
        }
    }
}

// ---------------------------------------------------------------------------
//  golden :82-126.  ACTIVE-PARTIAL: every line except golden :125
//  (`myLog.Do_Log(Sender, asUser, asLogPath);`) -- GATE (T-2), see the header.
// ---------------------------------------------------------------------------
void TfTowerLight::FormShow(TObject *Sender)
{
    (void)Sender;   // golden passes it only to the (T-2)-gated Do_Log line
    TComboBox *Ptr[8]={cbRunning, cbJam, cbPause, cbMessage, cbHeating,
                       cbHome, cbOffLine, cbART};

    for(int i=0; i<8; i++)                                                      //Steven 20140409 : Auto Retest
    {
        if(LastSet.MusicSelect[i]<0)
            LastSet.MusicSelect[i]=0;
        if(LastSet.MusicSelect[i]>4)
            LastSet.MusicSelect[i]=4;

        Ptr[i]->ItemIndex=LastSet.MusicSelect[i];
    }
    rgMusicTest->ItemIndex=0;
    fShow=true;
    Left=75;                                                                    //Steven 20091103
    Top=10;                                                                     //Steven 20091103
    Timer1->Enabled=true;

    if((CUSTOMER_CODE==CC_KYEC_XILINX &&
        CosFunction.bAutoRetestGPIBmode==true) ||                               //Frank 20160728 add
       CosFunction.bTowerLightUseLD==true)                                      //JerryYang 20230721 : LD/ULD區分不同的異常音樂
    {
        Panel11->Visible=true;
        if(CosFunction.bTowerLightUseLD==true)
        {
            palART->Caption="LD/ULD";
        }
        else
        {
            palART->Caption="Auto Retest";
        }
    }
    else
    {
        Panel11->Visible=false;
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        cbOffLine->Enabled=false;                                               //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow

    //這一行請保持在最下面!!-----------------
    // GATE (T-2) -- golden :125 `myLog.Do_Log(Sender, asUser, asLogPath);`
    //               //Steven 20100629.  NOT translated: handlerlog.cpp is
    //               ht9045_sm (forms cannot link it), Do_Log writes disk, and
    //               its `dynamic_cast<TWinControl*>` gate could never pass a
    //               base-class-less facade anyway.  See forms/fTowerLight.h.
}

// ---------------------------------------------------------------------------
//  golden :128-139.  ACTIVE-PARTIAL: every line except golden :138 -- GATE
//  (T-2).  DEVIATION D-3: golden's `(TObject*, TCloseAction&)` parameters are
//  dropped (TCloseAction has no port; neither parameter is read outside the
//  gated Do_Log line).
// ---------------------------------------------------------------------------
void TfTowerLight::FormClose()
{
    TComboBox *Ptr[8]={cbRunning, cbJam, cbPause, cbMessage, cbHeating,
                       cbHome, cbOffLine, cbART};
    for(int i=0; i<8; i++)                                                      //Steven 20140409 : Auto Retest
        LastSet.MusicSelect[i]=Ptr[i]->ItemIndex;
    fShow=false;
    Timer1->Enabled=false;

    //這一行請保持在最下面!!-----------------
    // GATE (T-2) -- golden :138 `myLog.Do_Log(Sender, asUser, asLogPath);`
    //               //Steven 20100629.  See FormShow above / the header.
}

// ---------------------------------------------------------------------------
void TfTowerLight::Timer1Timer(TObject *Sender)   // golden :148-153
{
    (void)Sender;
    if(fShow==false)
        return;
    UpdateTowerLed(0);
}

// ---------------------------------------------------------------------------
void TfTowerLight::spbExitClick(TObject *Sender)   // golden :155-158
{
    (void)Sender;
    Close();   // DEVIATION D-4 -- port-only no-op
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  See forms/fTowerLight.h for the per-entry reasoning.
// ===========================================================================

// AI(W906-TOWERLIGHT) 20260925 (Steven 團隊): GATE (T-1) 解閘 —— 此段原本是 `#if 0 // GATE (T-1) RGB00Click`。
//   依據：golden V912 cTowerLight.cpp:55-81（行號一律 V912）。與原 transcript（BCB 906 樹）的差別只有一處：
//   V912 :62 在 `Close();` 後面多了 `return;`（A01_2 權限不足時不再往下改燈號）—— 照 V912 補上。
//   解閘前提（header GATE REGISTER (T-1) 列的兩件事）：
//     (1) Tag：建構子已補上 dfm 的 Tag（見上）。
//     (2) 寫磁碟：WriteLastDataFile() 整塊寫 D:\HT9045\system\lastdata.dat／lastdata_backup.dat（寫死路徑）。
//         LAST_GENERAL_SET 佈局已逐欄比對 golden V912 LastSet.h:10-515：20260926 依 Steven RULINGS S45 補上 V912 檔尾的
//         iBinBaseRT[256]／bO25_RTBaselined 之後，389 欄位 offset／size 全同、sizeof 179928＝V912。
//         舊的 178896 bytes 檔：ReadLastDataFile 照 golden 只讀得到前段（尾端維持 0），第一次寫檔以 OPEN_EXISTING 延長到 179928。
//   呼叫者：WebTowerLight.cpp（wb_serve 的 WS towerlight.op）。
void TfTowerLight::RGB00Click(TObject *Sender)
{
    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        ShowMyMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        Close();
        return;
    }

    TfTowerLightLed *Ptr;                                                       // golden `TALed *Ptr;` -- DEVIATION D-5
    Ptr=(TfTowerLightLed *) Sender;
    int i=Ptr->Tag/3;
    int j=Ptr->Tag%3;

    int iOfflineRun=7;                                                          //JerryYang 20230721 : LD/ULD區分不同的異常音樂
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        iOfflineRun=5;

    if(i>iOfflineRun || i<0 || j>2 || j<0)
        return;
    LastSet.MessageLight[i][j]++;
    if(LastSet.MessageLight[i][j]>2)
        LastSet.MessageLight[i][j]=0;
    WriteLastDataFile();
    UpdateTowerLed(1);
}

#if 0 // GATE (T-3) rgMusicTestClick -- golden :141-146.  TWO INDEPENDENT
      // GATES: (a) LINK BOUNDARY -- `SW[]` (myswitch.h:43) is defined in
      // myswitch.cpp, add_library(ht9045_io); ht9045_forms links only
      // vclcompat + ht9045_globals + ht9045_core.  (b) MACHINE OUTPUT --
      // these are the four physical buzzer/music relays.
void TfTowerLight::rgMusicTestClick(TObject *Sender)
{
    (void)Sender;
    for(int i=0; i<4; i++)  SW[SwMusic1+i].Off();
    if(rgMusicTest->ItemIndex>=1 && rgMusicTest->ItemIndex<=4)
        SW[SwMusic1+rgMusicTest->ItemIndex-1].On();
}
#endif // GATE (T-3)
