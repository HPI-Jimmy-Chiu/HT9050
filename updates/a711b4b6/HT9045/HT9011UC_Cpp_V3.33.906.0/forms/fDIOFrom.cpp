// =============================================================================
//  forms/fDIOFrom.cpp  --  definitions for the fDIOFrom facade
//
//  AI(W906-FW3-BTQ1) 20260828: new file, FW wave FW3-BTQ1 (4 of 5 facades).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/DIOInterFaceCFG.cpp
//  (268 lines) + DIOInterFaceCFG.h (70 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 14 golden `TfDIOFrom::` member bodies, 236 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 14-member denominator:
//     8 ACTIVE           66 of 236 golden span lines (28.0%)
//     6 GATED-WITH-BODY  170 golden span lines carried as `#if 0` transcript
//
//  See forms/fDIOFrom.h for the full GATE REGISTER (I-1)..(I-6), the ACTIVE
//  evidence, DEVIATIONS D-1..D-7, the FIELD LIST, the nine RESERVED NAMES this
//  facade honours for forms/fTesterIF.cpp, and -- most importantly -- the
//  EMPTY-ITEMS warning about the six pre-committed `->Items->Strings[i]` reads.
//
//  ⚠ TWO GATES HERE ARE POLICY, NOT REACH, and both are cheap to lift once the
//  user approves a write direction:
//    (I-2) GetDIOFileName copies a file into the recipe folder (golden :61);
//    (I-3) LoadData sets the global run-state flag `SystemStart=false`
//          (golden :81) from a UI path -- a MODE SWITCH.
//  Every symbol either of them needs is already reachable from ht9045_forms.
//  Recorded so a future wave re-asks the question instead of inheriting the
//  answer (docs/KNOWLEDGE.md "前提死掉不代表答案就是退役").
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED.  The text is golden's
//  own (the only edits are dropping `__fastcall` and the unused FormClose
//  parameters), so it is a faithful TRANSCRIPT -- not verified code.  Several
//  of the identifiers it names (OpenDialog1, SaveDialog1, spbSave, spbLoad,
//  spbDelete, Label55/Label4/Label2, JCET_FOR_EVAN, TTL_MODE, _5BitPE,
//  _5BitPO, _10BitPE, _10BitPO, FTestIF->cbDIOType, ...) are NOT members of
//  this facade and/or were not confirmed to have a port; un-gating requires
//  supplying them first.
//
//  BACKSLASH-COMMENT SCAN (the -Wcomment line-splice trap): DIOInterFaceCFG.cpp
//  was scanned 20260828 for a `//` comment whose line ends in a backslash.
//  ZERO hits -- so no comment delimiter was changed anywhere in this file.
// =============================================================================
#include "forms/fDIOFrom.h"
#include "forms/fQwertyKey.h"    // fQwertyKey->ShowQwertyKey (forms/fQwertyKey.cpp, ht9045_forms)
#include "cprod.h"               // TTLCfg (cprod.h:32, TTL_DATA at :18-31, cprod.cpp, ht9045_globals)
#include "cmydef.h"              // N_INTEGER (extern const int, cmydef.cpp, ht9045_globals)
#include "CosFunction.h"         // CosFunction.bTTLUseUSec (CosFunction.cpp, ht9045_globals)
#include "common.h"              // OnlyNumberInPut / OnlyNumberAndDotInPut (common.cpp, ht9045_core)

// AI(W906-FW3-BTQ1) 20260828: TfDIOFrom/fDIOFrom were FREE tree-wide -- same
// idiom as forms/fCounterSel.cpp:38 / forms/fLd_ULd.cpp:43.  Golden's ctor
// body (:19-22) is EMPTY, so this static-init `new` touches nothing at all --
// no SIOF risk whatsoever (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL
// 全域"; the fLaserSensor incident that rule comes from turned 88 of 134 ctest
// binaries into SEGFAULTs).  The eleven widget field initialisers construct
// leaf vclcompat controls and call nothing translated.
TfDIOFrom *fDIOFrom = new TfDIOFrom();

TfDIOFrom::TfDIOFrom()   // golden :19-22 -- golden's body is empty
{
}

// ---------------------------------------------------------------------------
//  golden :133-146.  ACTIVE.  The READ direction of this form: eleven
//  TTLCfg -> widget assignments and nothing else.
//  ⚠ golden :137 assigns `TTLCfg.iSTPluseWidth` (an `unsigned int`,
//  cprod.h:23) straight into an AnsiString `Text`.  Kept verbatim -- vclcompat
//  AnsiString provides the numeric assignment, the same construct
//  forms/fLd_ULd.cpp:96 already relies on for a double.
// ---------------------------------------------------------------------------
void TfDIOFrom::DoIniDataToForm()
{
    rgStartLogic->ItemIndex     =TTLCfg.iSTLogicMode;
    rgStartChannel->ItemIndex   =TTLCfg.iStartType;
    edPulseWidth->Text          =TTLCfg.iSTPluseWidth;
    cbOneSTChannel->ItemIndex   =TTLCfg.iOneSTChannel;
    cbSignalType->ItemIndex     =TTLCfg.iDutType;
    edSignalBeforeOn->Text      =TTLCfg.iDutBfOnTime;
    edSignalAfterOff->Text      =TTLCfg.iDutAfOffTime;
    rgBinLogic->ItemIndex       =TTLCfg.iCateLogicMode;
    rgBinBitLength->ItemIndex   =TTLCfg.iCateBitLength;
    rgBinDataType->ItemIndex    =TTLCfg.iCateDataType;
    edTTLModeName->Text         =TTLCfg.cModeName;
}

// ---------------------------------------------------------------------------
//  golden :148-164.  ACTIVE.  Blanks the widgets and this facade's own
//  DIOFileName.  Needs no external symbol whatsoever.
// ---------------------------------------------------------------------------
void TfDIOFrom::InitData()
{
    rgStartLogic->ItemIndex=0;
    rgStartChannel->ItemIndex=0;
    edPulseWidth->Text="";

    cbSignalType->ItemIndex=0;
    edSignalBeforeOn->Text="";
    edSignalAfterOff->Text="";

    rgBinLogic->ItemIndex=0;
    rgBinBitLength->ItemIndex=0;
    rgBinDataType->ItemIndex=0;

    edTTLModeName->Text="";
    DIOFileName="";
}

// ---------------------------------------------------------------------------
void TfDIOFrom::Edit3KeyPress(TObject *Sender, char &Key)   // golden :166-170
{
    (void)Sender;
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=0;   // golden `Key=NULL;` on a char -- faithful as 0, DEVIATION D-7
}

// ---------------------------------------------------------------------------
void TfDIOFrom::edSignalBeforeOnKeyPress(TObject *Sender, char &Key)   // golden :172-176
{
    (void)Sender;
    if(OnlyNumberInPut(Key)==false)
        Key=0;   // golden `Key=NULL;` on a char -- faithful as 0, DEVIATION D-7
}

// ---------------------------------------------------------------------------
//  golden :178-189.  ACTIVE.  DEVIATION D-5: golden's unused TMouseButton /
//  TShiftState / int X / int Y parameters are dropped.
// ---------------------------------------------------------------------------
void TfDIOFrom::edPulseWidthMouseDown(TObject *Sender)
{
    if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 1, 500000);     //Ifor 20171229 : UNISEM TTL SOT訊號寬度統一修正為1ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
    }
    else
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10, 500);    //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
    }
}

// ---------------------------------------------------------------------------
void TfDIOFrom::spbExitClick(TObject *Sender)   // golden :258-261
{
    (void)Sender;
    Close();   // DEVIATION D-3 -- port-only no-op
}

// ---------------------------------------------------------------------------
//  golden :263-267.  ACTIVE -- a FULL translation, both statements live.
//  Unlike forms/fQAMode.cpp's FormClose (whose DoIniDataToForm is gated on a
//  missing fLotInfo member), this form's DoIniDataToForm is itself ACTIVE.
//  DEVIATION D-4: golden's (TObject*, TCloseAction&) parameters dropped.
// ---------------------------------------------------------------------------
void TfDIOFrom::FormClose()
{
    fShow=false;
    DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
//  See forms/fDIOFrom.h for the per-entry reasoning.
// ===========================================================================

#if 0 // GATE (I-1) FormShow -- golden :24-45.  NO DIALOG PORT: golden :26
      // `OpenDialog1->InitialDir=DIOCFGPath;`, and neither TOpenDialog nor
      // TSaveDialog exists in vclcompat (0 hits, 20260828).  Label55/Label4/
      // Label2 and JCET_FOR_EVAN are not declared here either (DEVIATION D-6).
      // NOT SPLIT even though :27 InitData() is ACTIVE and :44 fShow=true
      // would work: priming the file dialog is what golden's FormShow is FOR.
void TfDIOFrom::FormShow(TObject *Sender)
{
    OpenDialog1->InitialDir=DIOCFGPath;
    InitData();
    Top=30;
    Left=200;

    if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    {
        Label55->Caption="μs";
        Label4 ->Caption="μs";
        Label2 ->Caption="μs";
    }

    if(JCET_FOR_EVAN==1)        //Steven 20220506 : 吳如春要求只用10 bit bit
    {
        rgBinBitLength->Enabled=false;
        rgBinDataType ->Enabled=false;
    }

    fShow=true;
}
#endif // GATE (I-1)

#if 0 // GATE (I-2) GetDIOFileName -- golden :47-70.  RESERVED NAME
      // (forms/fTesterIF.cpp:1539).  WRITES DISK: golden :61
      // `CopyFile(szDir.c_str(), S1.c_str(), false)` copies the master DIO
      // .ini from DIOCFGPath INTO the recipe folder whenever the recipe copy
      // is missing.  PURE SAFETY GATE -- everything it needs is reachable:
      // DIOCFGPath (common.h:80, ht9045_core), GetLastOpenFN, FileExists,
      // DataPath, IniConfig, and FTestIF->cbDIOType (forms/fTesterIF.h, same
      // ht9045_forms target).
AnsiString TfDIOFrom::GetDIOFileName()          //Steven 20180626 (wei) : TTL設定存到工作檔裡面
{
    AnsiString S, S1;
    AnsiString szDir="";
    if(IniConfig.bI16TTLSaveInSetupFile)
    {
        S=GetLastOpenFN();

        S1.sprintf("%s%s\\%s.ini", DataPath, S, FTestIF->cbDIOType->Text);
        if(FileExists(S1)==false)        //檔案不存在的話,就去複製一份過來
        {
            szDir.sprintf("%s%s.ini", DIOCFGPath, FTestIF->cbDIOType->Text);
            if(FileExists(szDir))
            {
                CopyFile(szDir.c_str(), S1.c_str(), false);
            }
        }
    }
    else
    {
        S1.sprintf("%s%s.ini", DIOCFGPath, FTestIF->cbDIOType->Text);
    }
    return S1;
}
#endif // GATE (I-2)

#if 0 // GATE (I-3) LoadData -- golden :72-131.  RESERVED NAME
      // (forms/fTesterIF.cpp:1540).  MODE SWITCH: golden :81 sets the GLOBAL
      // RUN-STATE FLAG `SystemStart=false;` from a UI path when the DIO file
      // is missing -- the "模式切換" class the project rules call
      // safety-critical, which this wave's read-only direction does not take.
      // ⚠ POLICY GATE, NOT A REACHABILITY GATE: SystemStart is cmydef.h:221 ->
      // cmydef.cpp (ht9045_globals, reachable), ReadIniData / CheckRange /
      // ShowMyMessage are all reachable, and DoIniDataToForm() at :130 is
      // ACTIVE.  Cheap to lift with the user's approval.
      // Secondary, NOT the deciding fact: its natural producer
      // GetDIOFileName is GATE (I-2), so with (I-2) shut this would be handed
      // a path nothing produced.
void TfDIOFrom::LoadData(AnsiString S)
{
    AnsiString asString="";

    if(FileExists(S)==false)    //Steven 20180620 (Jou) : 加上DIO檔案遺失的保護判斷
    {
        if(TestIF_File.iTestType==TTL_MODE) //Steven 20210723 : 修正不是TTL模式會Alarm
        {
            ShowMyMessage("Current DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");
            SystemStart=false;
        }
        return;
    }

    TTLCfg.iSTLogicMode         =ReadIniData(S, "Start Signal",  "Logic", 0);
    TTLCfg.iStartType           =ReadIniData(S, "Start Signal",  "Channel", 0);
    TTLCfg.iOneSTChannel        =ReadIniData(S, "Start Signal",  "SelSignalCH", 0);
    TTLCfg.iSTPluseWidth        =ReadIniData(S, "Start Signal",  "Pluse Width", 0);
    TTLCfg.iDutType             =ReadIniData(S, "DUT Signal",    "Type", 0);
    if(TTLCfg.iDutType<0)   //Steven 20110105
        TTLCfg.iDutType=0;

    TTLCfg.iDutBfOnTime         =ReadIniData(S, "DUT Signal",    "Before On", 0);
    TTLCfg.iDutAfOffTime        =ReadIniData(S, "DUT Signal",    "After Off", 0);

    TTLCfg.iCateLogicMode       =ReadIniData(S, "Cate Signal",   "Logic", 0);
    if(JCET_FOR_EVAN==1)        //Steven 20220506 : 吳如春要求只用10 bit bit
    {
        TTLCfg.iCateBitLength       =3;
        TTLCfg.iCateDataType        =0;
    }
    else
    {
        TTLCfg.iCateBitLength       =ReadIniData(S, "Cate Signal",   "Channel status", 0);
        TTLCfg.iCateDataType        =ReadIniData(S, "Cate Signal",   "Data Type", 0);
    }

    asString=ReadIniData(S, "Name",          "Data Type", AnsiString("AAA"));
    strncpy(TTLCfg.cModeName, asString.c_str(), sizeof(TTLCfg.cModeName));
    if(TTLCfg.iCateBitLength==_5BitPE || TTLCfg.iCateBitLength==_10BitPE)
        TTLCfg.iCateParity=2;//even
    else if(TTLCfg.iCateBitLength==_5BitPO || TTLCfg.iCateBitLength==_10BitPO)
        TTLCfg.iCateParity=1;//odd
    else
        TTLCfg.iCateParity=0;//not use
    if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
    {
        TTLCfg.iSTPluseWidth    =CheckRange((int)TTLCfg.iSTPluseWidth, 1, 500000);    //Ifor 20171229 : UNISEM TTL SOT訊號寬度統一修正為1ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutBfOnTime     =CheckRange(TTLCfg.iDutBfOnTime, 1, 500000);
        TTLCfg.iDutAfOffTime    =CheckRange(TTLCfg.iDutAfOffTime, 1, 500000);
    }
    else
    {
        TTLCfg.iSTPluseWidth    =CheckRange((int)TTLCfg.iSTPluseWidth, 10, 500);   //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutBfOnTime     =CheckRange(TTLCfg.iDutBfOnTime, 10, 500);         //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        TTLCfg.iDutAfOffTime    =CheckRange(TTLCfg.iDutAfOffTime, 10, 500);        //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
    }

    DoIniDataToForm();
}
#endif // GATE (I-3)

#if 0 // GATE (I-4) spbSaveClick -- golden :191-235.  WRITES DISK: eleven
      // WriteIniData(DIOFileName, ...) calls at :222-232.  Also needs
      // SaveDialog1->Execute() -- no TSaveDialog port (see I-1).  Both a
      // safety gate and a missing-type gate.
void TfDIOFrom::spbSaveClick(TObject *Sender)
{
    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        ShowMyMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        Close();
    }

    if(DIOFileName=="")
        return;

    SaveDialog1->InitialDir=DIOCFGPath;
    SaveDialog1->FileName=DIOFileName;

    if(SaveDialog1->Execute())
    {
        if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        {
            TTLCfg.iSTPluseWidth        =CheckRange(atoi(edPulseWidth->Text.c_str()), 1, 500000);      //Ifor 20171229 : UNISEM TTL SOT訊號寬度統一修正為1ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutBfOnTime         =CheckRange(atoi(edSignalBeforeOn->Text.c_str()), 1, 500000);
            TTLCfg.iDutAfOffTime        =CheckRange(atoi(edSignalAfterOff->Text.c_str()), 1, 500000);
        }
        else
        {
            TTLCfg.iSTPluseWidth        =CheckRange(atoi(edPulseWidth->Text.c_str()), 10, 500);        //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutBfOnTime         =CheckRange(atoi(edSignalBeforeOn->Text.c_str()), 10, 500);    //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
            TTLCfg.iDutAfOffTime        =CheckRange(atoi(edSignalAfterOff->Text.c_str()), 10, 500);    //Steven 20161221 (jou) : TTL SOT訊號寬度統一修正為10ms ~ 500ms , 主要是保護在短秒數測試時SOT過長會造成多測問題
        }

        DIOFileName=SaveDialog1->FileName;
        WriteIniData(DIOFileName, "Start Signal",  "Logic",           rgStartLogic->ItemIndex);
        WriteIniData(DIOFileName, "Start Signal",  "Channel",         rgStartChannel->ItemIndex);
        WriteIniData(DIOFileName, "Start Signal",  "Pluse Width",     (int)TTLCfg.iSTPluseWidth);
        WriteIniData(DIOFileName, "Start Signal",  "SelSignalCH",     cbOneSTChannel->ItemIndex);
        WriteIniData(DIOFileName, "DUT Signal",    "Type",            cbSignalType->ItemIndex);
        WriteIniData(DIOFileName, "DUT Signal",    "Before On",       TTLCfg.iDutBfOnTime);
        WriteIniData(DIOFileName, "DUT Signal",    "After Off",       TTLCfg.iDutAfOffTime);
        WriteIniData(DIOFileName, "Cate Signal",   "Logic",           rgBinLogic->ItemIndex);
        WriteIniData(DIOFileName, "Cate Signal",   "Channel status",  rgBinBitLength->ItemIndex);
        WriteIniData(DIOFileName, "Cate Signal",   "Data Type",       rgBinDataType->ItemIndex);
        WriteIniData(DIOFileName, "Name",          "Data Type",       edTTLModeName->Text);
    }
    spbSave->Down=false;
}
#endif // GATE (I-4)

#if 0 // GATE (I-5) spbLoadClick -- golden :237-246.  NO DIALOG PORT
      // (`OpenDialog1->Execute()`); also [T] on GATE (I-3) at :243.
void TfDIOFrom::spbLoadClick(TObject *Sender)
{
    OpenDialog1->Title="Open DIO File";
    if(OpenDialog1->Execute())
    {
        DIOFileName=OpenDialog1->FileName;
        LoadData(DIOFileName);
    }
    spbLoad->Down=false;
}
#endif // GATE (I-5)

#if 0 // GATE (I-6) spbDeleteClick -- golden :248-256.  DELETES A FILE:
      // `DeleteFile(OpenDialog1->FileName)` at :253 -- and vclcompat DOES
      // provide DeleteFile (vclcompat/SysUtils.h), so an un-gated version
      // would really unlink whatever the dialog returned.  Also no dialog
      // port.  Two independent gates.
void TfDIOFrom::spbDeleteClick(TObject *Sender)
{
    OpenDialog1->Title="Select file to delete";
    if(OpenDialog1->Execute())
    {
        DeleteFile(OpenDialog1->FileName);
    }
    spbDelete->Down=false;
}
#endif // GATE (I-6)
//AI(W906-FRW-S101) 20260926: 這道閘的兩個理由 —— (1)「寫入／刪檔一律閘」是 20260819 的政策，已被 S12／S52（讀寫檔照 golden 移植）取代；
//  (2)「沒有對話框」仍成立 —— 所以本門面的 spbDeleteClick 維持關閉、不解。網頁路徑另走 C 路：golden 本體逐行在
//  FileRW/TTLCfg.gen.inc DI_spbDeleteClick（tools/editlist/TTLCfg.py），對話框換成 FileRW/TTLCfg.cpp FileRW_TTLCfg_DeleteOp
//  （只收 DIOCFGPath 底下的 *.ini）。RULINGS_20260926 S101。
