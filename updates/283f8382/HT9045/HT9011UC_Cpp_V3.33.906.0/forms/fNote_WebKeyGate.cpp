// ===========================================================================
//  forms/fNote_WebKeyGate.cpp -- AI(W906-NOTE-KEYGATE) 20261002: the alarm note's key lock for a key chosen ON THE SCREEN.
//
//  The every-component check (EastSun 20261001「不是只有檢查按鈕喔 我說的是所有元件」, helper motorB) found that the web
//  answer path (tools/wb_serve.cpp, modal.answer / dialog.response) accepted SKIP / RETRY / TRAY FEED / TRAY END / CLEAN OUT /
//  HOME / TRAIN / ONE CYCLE whatever the machine state, while golden ignores the press:
//    golden V906 note.cpp:2843-2870 TfNote::BtnSkipClick (the OnClick of those 8 keys, note.dfm), non-SIM:
//      IsTestSitICFallDown()                                   -> return   (jou 2011-12-20: an IC fell into the test site)
//      bContactCTOverCHK || bAutoCleanCheckOpenDoor || bChangeCleanPad -> return
//      bP59UnloaderICFloattingAlarmAfterExit && iICFloattingCheckStep!=0 -> return   (iICFloattingCheckStep: TfNote member
//                                                                         not ported -> 0, the clause is false; as the panel path)
//    then UpdateButtonStatus :2764-2873:
//      IsSafeLockCheck()                                       -> return
//      IsTestSitICFallDown() (non-SIM)                         -> return   (「有掉料的話，要開Chamber門和按Z1才能接收按鍵」)
//      CUSTOMER_CODE!=CC_SCK && bIndexJamInArmAway && bD40IndexICFallDownMustPressFMotorDown &&
//        bOpenChamberDoor==false && bIsTestSitICFallDown==true -> return
//      bAutoRetestJam && bOpenAllDoor==false                   -> return
//      BinBox (bOpenSixDoor && bBinError[]): bOpenSixDoor is only set by golden ShowErrorMessage (not ported) -> always false,
//        not checked -- same as the physical-key path.
//  The physical panel keys already apply the same lock (tools/wb_serve.cpp W906_AlarmIoAnswer, golden :2793-2811); this is the
//  screen side, so the screen can no longer RETRY + START with an IC still in the socket.
//  Golden gives no message (the press is just ignored); the web gets the reason as the refusal text so the operator knows why.
//  RESET is not gated here: its OnClick is BtnResetClick (runs the main Reset first), whose main half is not wired yet.
//  bOpenChamberDoor is the TfNote member W906_AlarmIoAnswer keeps (golden Timer1Timer :3187-3205); the caller passes it.
// ===========================================================================
#include <string>

#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"
#include "csystem.h"
#include "forms/fNote.h"

bool W906_NoteWebKeyGate(bool bOpenChamberDoor, std::string* why)
{
    std::string w;
#ifdef SOFT_SIMULTE
    if (IsSafeLockCheck())                                                      // golden :2847-2849 (SIM arm of BtnSkipClick)
        w = "Safe Lock（golden IsSafeLockCheck）";
#else
    if (fNote && fNote->IsTestSitICFallDown())                                  // golden :2851-2852
        w = "測試座有掉料：要先開 Index 門並按 Z1 才能選鍵（golden IsTestSitICFallDown）";
    else if (bContactCTOverCHK || bAutoCleanCheckOpenDoor || bChangeCleanPad)   // golden :2853-2858
        w = bContactCTOverCHK ? "接觸次數超過（golden bContactCTOverCHK）"
          : bAutoCleanCheckOpenDoor ? "Auto Clean 警報要先開後門（golden bAutoCleanCheckOpenDoor）"
          : "要先換清潔墊（golden bChangeCleanPad）";
#endif
    if (w.empty() && IsSafeLockCheck())                                         // UpdateButtonStatus :2773
        w = "Safe Lock（golden IsSafeLockCheck）";
    if (w.empty() && CUSTOMER_CODE != CC_SCK && IniConfig.bIndexJamInArmAway == true &&
        IniConfig.bD40IndexICFallDownMustPressFMotorDown == true &&
        bOpenChamberDoor == false && bIsTestSitICFallDown == true)              // UpdateButtonStatus :2781-2789
        w = "Index 掉料：要先開 Chamber 門（golden bOpenChamberDoor）";
    if (w.empty() && bAutoRetestJam && bOpenAllDoor == false)                   // UpdateButtonStatus :2791-2792
        w = "Auto Retest Jam：要先開門確認（golden bOpenAllDoor）";
    if (w.empty()) return true;
    if (why) *why = "note-key-locked: golden 不接受這個按鍵 —— " + w;
    return false;
}

// AI(W906-NOTE-SCREENSTART) 20261002: EastSun「Bcb上怎做你就怎做」 -- golden V906 note.cpp:3806-3824 TfNote::BtnStartClick (the ON-SCREEN START):
//   non-SIM: only CUSTOMER_CODE==CC_SIGURD_PeiXing calls Start() (+ NewRecordProcess MES2110); every other customer does nothing
//   (the `bErrPan_err && Pwd!=""` return is a no-op either way); SIM: Start().  The PANEL START key is TfNote::Timer1Timer -> Start()
//   (tools/wb_serve.cpp W906_AlarmIoAnswer), not this, and is not affected.  Not ported: the first line
//   `CosFunction.bEnableHandlerResultServer && bNeedTCPAlarm` return (TfNote has no bNeedTCPAlarm in the port; it only adds a refusal).
bool W906_NoteScreenStartActs()
{
#ifdef SOFT_SIMULTE
    return true;                                                                // golden :3821-3822
#else
    return CUSTOMER_CODE == CC_SIGURD_PeiXing;                                  // golden :3811-3815
#endif
}
