// =============================================================================
//  forms/fNote_JamCount.cpp  --  golden TfNote::FormClose 的 Jam 計數
//
//  AI(W906-J2) 20260926: new file. 從 906 golden 逐行搬（cp950 解碼成 UTF-8，零 U+FFFD，依行號機械複製）：
//    HT9011UC_Code_V3.33.906.0_20260618/note.cpp
//      CheckRecordJamType            note.cpp:344-389（檔案範圍自由函式）
//      FormClose 的呼叫              note.cpp:2528-2529
//
//  St01 AUDIT_PROD_20260926 J2：移植樹 0 命中 ⇒ LastSet.iJamCount[0..2]／iDayJamCount／iRecordJamRateByTime_JamCount
//  從來不加 —— 網頁 prod 的 Jam 數（JsonBridge/ChanProduction.cpp:112、ChanAction.cpp:526）、SECS SV 1036 "Jam Count"、
//  cObserver 的 MTBA／Jam rate 字串、cMyDB 生產紀錄的 Jam 數永遠是 0。讀者全是統計／顯示，沒有會讓機台動作的。
//
//  呼叫點：tools/wb_serve.cpp ForwardShowErrorMessage 的兩個回答出口（網頁、面板鍵），在 W906_AlarmAnswerStartLikeGolden 之前。
//    golden 的順序是 TfNote::Start → fMain->Start（note.cpp:3669）→ Close()（:3678）→ FormClose → 計數；
//    golden 框開著時的新警報在 ShowErrorMessage :814-818 直接 return 0，所以 FormClose 看到的永遠是被答掉的那一則。
//    移植樹重走啟動檢查時可以再開一個框（巢狀的 W906ModalWaitScope 會蓋掉 fNote->edErrorCode／AlarmType），
//    所以在答案確定、重走之前計 —— 計數結果與 golden 相同（fMain->Start 不讀這些計數）。
//  條件照 :2528：AlarmType==1 && iRealDummy==REALLY && !bLampTrayEnd && iDuplicateError!=1
//    * bLampTrayEnd：:2523-2524 先做 *bPtr[i]=Select[i]，bPtr[3]=&bLampTrayEnd（:2505）⇒ 等於「選的是 TRAY END」＝ k==K_TRAY_END
//    * iDuplicateError!=1 ⇔ 最近一次 ShowErrorMessage 的 bDuplicateErr 是 false（:802-803，1 的優先權最高；canary_support.cpp 記）
//
//  已知差異（刻意）：
//    * kcode==0 的通知框不進等待（使用者 20260923 裁決，AI(W906-Q30-KZERO)）⇒ 沒有關框 ⇒ 不計；golden 會在操作員關框時計。
//    * CosFunction.bIncludeMTBA 那一支閘著（G1，理由在那一行）。
//    * :2553 bCheckOnly=true 那一次（SECS ReportAlarm 的 bIsJam）不在這裡 —— 關框的 HGem->ReportAlarm 另案（INBOX 64 EventReport）。
//
//  ctest：tests/test_note_jamcount.cpp（NoteJamCount）。
// =============================================================================
#include "forms/fNote.h"             // fNote->AlarmType / edErrorCode
#include "forms/fMesSystem.h"        // fMesSystem->iJamRateTotalForAlways
#include "canary_support.h"          // W906_ShowErrorMessage_LastDuplicate（golden iDuplicateError==1）
#include "vclcompat/vcl_compat.h"
#include "LastSet.h"                 // LastSet.iJamCount / iDayJamCount / iRealDummy / iTester
#include "CosFunction.h"             // CosFunction.bIncludeMTBA
#include "Config.h"                  // IniConfig.bVTESTFunction / bCheckFile
#include "cprod.h"                   // RunInfo.bLotStart
#include "cmydef.h"                  // REALLY / ON_LINE / K_TRAY_END / iRecordJamRateByTime_JamCount / CUSTOMER_CODE
#include "MachineType.h"             // CC_AnalogDevice_Phil

#include <cstdlib>

bool W906_CheckRecordJamType(AnsiString asJamCode, bool bCheckOnly=false);
void W906_NoteJamCountOnClose(int k);

//------------------------------------------------------------------------------
bool W906_CheckRecordJamType(AnsiString asJamCode, bool bCheckOnly)             // golden note.cpp:344 CheckRecordJamType (default bCheckOnly=false is on the declaration above)
{
    int iUnitNo;
    bool bResult=false, bAddCount=false;
    AnsiString CodeBuffer;

    CodeBuffer=asJamCode.UpperCase();
    iUnitNo=atoi(AnsiString(asJamCode.SubString(4, 2)).c_str());

    bool bIncludAllUnit=(CUSTOMER_CODE==CC_AnalogDevice_Phil);                  //JerryYang 20230721 : Analog要求修改

    if(CosFunction.bIncludeMTBA)
    {
#if 0 // GATE(W906-J2) G1: fNote has no sJamArea / sJamCode (golden :872-877 builds them from MyDBIEvent's UnitNo + JamArea[]; the port's MyDBIEvent segment is still #if 0, cMyDB.cpp:878) -- MTBA customers keep today's behaviour (no count) -- golden note.cpp:357-358
        if(LastSet.iRealDummy==REALLY)
            bAddCount=fSecurity->GetJemIncludeMTBA(fNote->sJamArea, fNote->sJamCode);
#endif
    }
    else
    {
        if(CodeBuffer.Pos("JAM")>0 &&
           (iUnitNo<9 || bIncludAllUnit==true) &&                               //Steven 20101102 : 改用UnitNo來判別要不要記入Jam Rate
           LastSet.iRealDummy==REALLY)                                          //JerryYang 20230721 : Analog要求修改
        {
            if(bCheckOnly==false)                                               //Steven 20140528 : Secs Gem
            {
                bAddCount=true;
            }
        }
    }

    if(bAddCount)
    {
        for(int i=0; i<3; i++)
            LastSet.iJamCount[i]++;

        LastSet.iDayJamCount++;                                                 //jou 20210108 : 上海偉測要求新增每日jam rate統計
        iRecordJamRateByTime_JamCount++;                                        // 2015.11.11 , Joye , Add Jam Rate Record

        if(IniConfig.bVTESTFunction==true && LastSet.iTester==ON_LINE &&        //marvin 20200424 (Kirin) Added always record report by time.
           RunInfo.bLotStart==true && IniConfig.bCheckFile==true)
        {
            if (fMesSystem != 0) fMesSystem->iJamRateTotalForAlways++;          // AI(W906-J2): the null guard is the port's
        }
        bResult=true;
    }
    return bResult;
}
//------------------------------------------------------------------------------
void W906_NoteJamCountOnClose(int k)                                            // golden note.cpp:2528-2529（呼叫點見檔頭）
{
    if (fNote == 0) return;
    const bool bTrayEndSelected = (k == K_TRAY_END);                            // :2523-2524 *bPtr[3]=Select[3] ⇒ bLampTrayEnd
    const int  iDuplicateError  = W906_ShowErrorMessage_LastDuplicate ? 1 : 0;  // :802-813（只有 1 會擋）
    if(fNote->AlarmType==1 && LastSet.iRealDummy==REALLY && !bTrayEndSelected && iDuplicateError!=1)
        W906_CheckRecordJamType(fNote->edErrorCode->Text);
}
