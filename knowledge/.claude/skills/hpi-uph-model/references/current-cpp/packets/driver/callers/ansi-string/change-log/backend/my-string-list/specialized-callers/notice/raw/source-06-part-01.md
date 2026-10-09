# 原文 06／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `W906_NoteAuthNoticeGate`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `b0f951f05d97e21f06e1125957652e9f9ea6723baac020c88f6b4a2d8958e58b`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
bool W906_NoteAuthNoticeGate(const std::string& requestId, std::string* why)
{
    using namespace noteauth;
    Lock lock;
    Note* n = Find(requestId);
    if(!n || !n->shown || !n->notice) return true;                              // not an armed notice: NotifyAckHandle answers it
    if(SystemStart==false && SoftStart==false && SpecialLocked())               // AI(W906-D034) 20261002: golden BtnPauseClick :3830（V912 :3870） (before ASE_SG :3833（V912 :3873）); a PAUSE press only, as below
    {
        if(why) *why = "auth-required: golden TfNote::BtnPauseClick returns while the SpecialPanel password (SpecialErrNote.ini) is not entered "
                       "(906 note.cpp:3830, V912 :3870) -- send dialog.auth first (PanSpecialNoteClick :5442 (V912 :5489))";
        return false;
    }
    if(W906_NoteNoticeAckRefusal(requestId.c_str())) return true;               // golden BtnPauseClick returns before DoPassword
    if(SystemStart==true || SoftStart==true) return true;                       // not a PAUSE press (pause 2): no DoPassword
    if(n->passValid)
    {
        const bool match = n->passK == 0 && n->passPressed.empty();         // a notice's pass (W906_NoteAuthVerify); any connection
        n->passValid = false;
        if(match) return true;
    }
    Out o;
    if(Press(*n, -1, nullptr, &o)) return true;
    if(why) *why = "auth-required: golden TfNote::BtnPauseClick (KeyCode==0, note.cpp:4092) -> DoPassword asks for a login (level " +
                   reauth::Num(o.required) + ") -- send dialog.auth first";
    return false;
}

<!-- preserved-content:end -->
```
