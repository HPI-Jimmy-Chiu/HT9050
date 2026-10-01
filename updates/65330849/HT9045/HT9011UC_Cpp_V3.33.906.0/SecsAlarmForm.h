//------------------------------------------------------------------------------
//  SecsAlarmForm.h -- TSecsAlarmForm / fSecsAlarm, golden 912 mymessbox.h:63-83 (RogerYang 20260724: the S10F3 SECS alarm window)
//
//  AI(W906-H013) 20261001 (St02-E): ruling 11 = B (docs/TESTERCOMM_PORT_LEDGER.md:194; todo H-013 item 2) -- the LINK layer
//    only, so golden 912's three Command.cpp terms `(fSecsAlarm && fSecsAlarm->Visible)` compile verbatim:
//    GetHandlerStatusByDll (golden 912 :9606-9608), RemoteControl (:9688), TCPCommandServerClientRead HTGR,801 (:14074-14075).
//    Same shape as mymessbox_shim.h's TMyMessageBoxShim.  Definition of the pointer: SecsAlarmForm.cpp (ht9045_globals).
//  golden 912's class (mymessbox.h:63-79) is a TForm built in code (no .dfm; body mymessbox.cpp:1405-1625):
//    private  btnOK, btnAlarmReset, btnOKClick(), btnAlarmResetClick();
//    public   moSecs (TMemo, both scroll bars), lblChinese (TLabel), palWaitEAP (TPanel), bMBoxNeedPassword,
//             bDisableAlarmBuzzer, the TForm(Owner, 0) constructor, DoReleaseAndHide();
//    plus the free functions ShowSecsAlarmMessage(S1, S2="") and CloseSecsAlarmForm() (mymessbox.h:82-83).
//  WHY ONLY Visible: nothing in the port creates the window.  golden 912's only `new TSecsAlarmForm` is in
//    ShowSecsAlarmMessage (mymessbox.cpp:1579), whose only caller is TFSECS::TimerSecsAlarmTimer
//    (SECSGEM/UsecegemMainFrom.cpp:1100) -- TFSECS is not translated (SECSGEM/uHGemEquipment.h:791).  So fSecsAlarm stays
//    NULL, the three terms stay false and behaviour is unchanged.  The window body, ShowSecsAlarmMessage,
//    CloseSecsAlarmForm and golden 912's other ~30 readers are not in ruling 11.
//  NOTE: fSecsAlarm is not a row of the page table (WebPageTable.cpp kRows), so the three reads are not wrapped in
//    W906_FormShowing and tools/fshow_audit.py does not count them.  Whoever adds "fSecsAlarm" to kRows wraps them then.
//------------------------------------------------------------------------------
#ifndef SecsAlarmFormH
#define SecsAlarmFormH

class TSecsAlarmForm                                                            //RogerYang 20260724 : Add for new S10F3 SECS alarm
{
public:
    bool Visible;     // golden TForm::Visible -- the S10F3 window is showing (no window in the port: false)
    TSecsAlarmForm() : Visible(false) {}
};

extern TSecsAlarmForm *fSecsAlarm;                 // 單例指標   (golden 912 mymessbox.h:81)

#endif
