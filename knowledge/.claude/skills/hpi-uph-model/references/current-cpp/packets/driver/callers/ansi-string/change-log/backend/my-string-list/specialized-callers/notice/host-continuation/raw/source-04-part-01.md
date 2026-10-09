# 原文 04／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `W906_NoteAuthVerify`；種類 `complete_cpp_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `48042f4f5cf660d12c66a9d6e8dbfdc6ef84212a18ba10b2279f7dce646d4b48`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
bool W906_NoteAuthVerify(const std::string& authId, const std::string& valueJson,
                         const std::string& currentId, bool blocking, std::string* reply)
{
    using namespace noteauth;
    Lock lock;
    if(currentId.empty()) { *reply = "no-pending-dialog: no alarm note is waiting for an answer"; return false; }
    cJSON* root = cJSON_Parse(valueJson.c_str());
    struct Free { cJSON* r; ~Free() { if (r) { reauth::WipeNode(r->child); cJSON_Delete(r); } } } freeRoot = { root };   // the typed strings are zeroed before the free
    if(!root || !cJSON_IsObject(root)) { *reply = "bad-payload: dialog.auth value must be the Dialog-auth-verify JSON object"; return false; }
    const cJSON* jt  = cJSON_GetObjectItemCaseSensitive(root, "target");
    const cJSON* jr  = (jt && cJSON_IsObject(jt)) ? cJSON_GetObjectItemCaseSensitive(jt, "requestId") : nullptr;
    const cJSON* ja  = cJSON_GetObjectItemCaseSensitive(root, "pendingAction");
    const cJSON* jan = (ja && cJSON_IsObject(ja)) ? cJSON_GetObjectItemCaseSensitive(ja, "name") : nullptr;
    const cJSON* jap = (ja && cJSON_IsObject(ja)) ? cJSON_GetObjectItemCaseSensitive(ja, "pressedButton") : nullptr;
    const cJSON* jc  = cJSON_GetObjectItemCaseSensitive(root, "credentials");
    const cJSON* ju  = (jc && cJSON_IsObject(jc)) ? cJSON_GetObjectItemCaseSensitive(jc, "userId") : nullptr;
    const cJSON* jpw = (jc && cJSON_IsObject(jc)) ? cJSON_GetObjectItemCaseSensitive(jc, "password") : nullptr;
    const cJSON* jx  = cJSON_GetObjectItemCaseSensitive(root, "cancelled");
    if(!jr || !cJSON_IsString(jr) || !jr->valuestring) { *reply = "bad-payload: target.requestId must be a string"; return false; }
    const std::string rid = jr->valuestring;
    if(rid != currentId) { *reply = "not-the-current-dialog: current=" + currentId; return false; }
    if(!jan || !cJSON_IsString(jan) || !jan->valuestring) { *reply = "bad-payload: pendingAction.name must be a string"; return false; }
    if(jap && !cJSON_IsNull(jap) && !(cJSON_IsString(jap) && jap->valuestring)) { *reply = "bad-payload: pendingAction.pressedButton must be a string or null"; return false; }
    if(jx && !cJSON_IsBool(jx)) { *reply = "bad-payload: cancelled must be true or false"; return false; }
    const bool cancelled = cJSON_IsTrue(jx) != 0;
    if(!cancelled && ju && !cJSON_IsNull(ju) && !(cJSON_IsString(ju) && ju->valuestring)) { *reply = "bad-payload: credentials.userId must be a string or null"; return false; }
    if(!cancelled && !(jpw && cJSON_IsString(jpw) && jpw->valuestring)) { *reply = "bad-payload: credentials.password must be a string (or send cancelled:true)"; return false; }
    const std::string action = jan->valuestring;
    const std::string pressed = (jap && cJSON_IsString(jap) && jap->valuestring) ? jap->valuestring : "";
    if(!pressed.empty() && pressed != "BtnStart" && pressed != "BtnPause") { *reply = "bad-payload: pressedButton must be BtnStart or BtnPause"; return false; }
    int sel = -1, k = 0;
    if(blocking)
    {
        sel = SelOfName(action);
        k = KeyOf(sel);
        Note* pn = Find(rid);
        const int kcode = pn ? pn->kcode : 0;
        if(sel < 0 || (k & kcode) == 0) { *reply = "not-an-offered-option: " + action; return false; }
    }
    else if(action != "ACKNOWLEDGE")
    {
        *reply = "not-an-offered-option: a kCode==0 notice is only acknowledged (" + action + ")";
        return false;
    }
    Cred cred;
    cred.cancelled = cancelled;
    if(!cancelled)
    {
        if(ju && cJSON_IsString(ju) && ju->valuestring) cred.userId = ju->valuestring;
        cred.password = jpw->valuestring;
    }
    struct WipeCred { Cred* c; ~WipeCred() { reauth::Wipe(&c->password); reauth::Wipe(&c->userId); } } wipeCred = { &cred };

    Note* n = Find(rid);
    Out o;
    bool accepted = true;
    const char* golden = blocking ? (pressed == "BtnPause" ? "V912 note.cpp:3906-3929 TfNote::BtnPauseClick Select[] loop -> DoUnlockPassword :5431 / DoPassword :5277"
                                                           : "V912 note.cpp:3584-3607 TfNote::Start Select[] loop -> DoUnlockPassword :5431 / DoPassword :5277")
                                  : "V912 note.cpp:4084-4095 TfNote::BtnPauseClick KeyCode==0 arm -> DoPassword :5277";
    const int levelBefore = AccessLevel;
    if(!n || !n->shown)
    {
        o.reason = "golden shows no note for this request (no FormShow): no password";
    }
    else if((blocking || (SystemStart==false && SoftStart==false)) && SpecialLocked())
    {
        // AI(W906-D034) 20261002: golden returns at the top of every press while the SpecialPanel password is not entered (906 note.cpp:2845（V912 :2867） /
        //   :3818（V912 :3858） / :3830（V912 :3870） / :5221（V912 :5261）); the only box the operator can open is the panel's (PanSpecialNoteClick :5442（V912 :5489）).  No DoPassword runs.
        golden = "906 note.cpp:5442-5460 (V912 :5489-5507) TfNote::PanSpecialNoteClick (SpecialPanel password, SpecialErrNote.ini; armed by FormShow 906 :1564-1606 / V912 :1574-1616)";
        n->passValid = false;
        if(PanSpecialNoteClick(cred, &o)==false)
        {
            accepted = false;
            o.reason = cancelled ? "special note password box cancelled = blank = no match (906 note.cpp:5447-5449, V912 :5494-5496); the alarm stays locked"
                                 : "the special note password did not match (906 note.cpp:5449, V912 :5496); the alarm stays locked";
        }
        else
        {
            Note probe = *n;
            Out po;
            if(Press(probe, sel, nullptr, &po)==false && (po.unlockAsks || po.asks))
            {
                accepted = false;
                o.specialNext = po.unlockAsks ? "unlock" : "login";
                o.mode = po.mode;
                o.required = po.required;
                o.reason = std::string("special note password accepted, the alarm is unlocked (906 note.cpp:5449-5451, V912 :5496-5498); golden now asks the ") +
                           (po.unlockAsks ? "alarm unlock password (DoUnlockPassword)" : "login (DoPassword)") + " for this press";
            }
            else
            {
                o.reason = "special note password accepted, the alarm is unlocked (906 note.cpp:5449-5451, V912 :5496-5498); this press needs no other password";
            }
        }
    }
    else if(!blocking && W906_NoteNoticeAckRefusal(rid.c_str()))
    {
        o.reason = std::string("golden BtnPauseClick returns before DoPassword: ") + W906_NoteNoticeAckRefusal(rid.c_str());
    }
    else if(!blocking && (SystemStart==true || SoftStart==true))
    {
        o.reason = "the machine is running again: the acknowledgement is not a PAUSE press (forms/fNote_ShowError.cpp pause 2), no DoPassword";
    }
    else
    {
        accepted = Press(*n, sel, &cred, &o);
        if(accepted)
        {
            n->passValid = true;
            n->passK = blocking ? k : 0;
            n->passPressed = blocking ? pressed : std::string();               // a notice: the page sends BtnPause, the ack carries no button
        }
        else
        {
            n->passValid = false;
        }
    }
    const bool asked = o.asks || o.unlockAsks || o.specialAsks;
    std::printf("dialog.auth qid=%s %s%s%s -> %s%s (%s)\n", rid.c_str(), action.c_str(), pressed.empty() ? "" : ":", pressed.c_str(),
                accepted ? "accepted" : "refused", cancelled ? " [cancelled]" : "", asked ? "golden asked" : "golden does not ask");
    std::fflush(stdout);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("1.0.0");
    w.Key("channel").String("dialog-auth");
    w.Key("authId").String(authId);
    w.Key("state").String("completed");
    w.Key("accepted").Bool(accepted);
    if(o.asks) w.Key("accessLevel").Number((wb_int64)o.accessLevel); else w.Key("accessLevel").Null();
    w.Key("userId").Null();                                                     // not echoed
    if(accepted) w.Key("message").Null();
    else if(o.specialAsks && !o.specialPassed) w.Key("message").String("Wrong special note password!!");   // AI(W906-D034): golden has no caption (the panel stays)
    else if(o.specialPassed) w.Key("message").String(o.specialNext == "unlock" ? "Special note password OK -- now the alarm unlock password"
                                                                                : "Special note password OK -- now log in");
    else if(o.loginNext) w.Key("message").String("Unlock password OK -- now log in");
    else if(o.unlockAsks && !o.unlockPassed) w.Key("message").String("Wrong alarm unlock password!!");
    else w.Key("message").String("Wrong ID or password or Insufficient privileges!!");   // golden fPassword Label3 caption
    w.Key("verifiedAt").Null();
    w.Key("error").Null();
    w.Key("requestId").String(rid);
    w.Key("action").String(action);
    if(pressed.empty()) w.Key("pressedButton").Null(); else w.Key("pressedButton").String(pressed);
    w.Key("cancelled").Bool(cancelled);
    w.Key("asked").Bool(asked);
    w.Key("unlockAsked").Bool(o.unlockAsks);
    w.Key("specialAsked").Bool(o.specialAsks);                                 // AI(W906-D034)
    w.Key("stage").String((o.specialAsks && !o.specialPassed) ? std::string("special") : !o.specialNext.empty() ? o.specialNext
                          : std::string(o.loginNext ? "login" : "done"));
    w.Key("mode").String(o.mode);
    w.Key("required").Number((wb_int64)o.required);
    w.Key("levelBefore").Number((wb_int64)levelBefore);
    w.Key("level").Number((wb_int64)AccessLevel);
    w.Key("loggedOut").Bool(o.loggedOut);
    if(!o.alarm.empty()) w.Key("alarm").String(o.alarm);
    w.Key("reason").String(o.reason);
    w.Key("golden").String(golden);
    w.Key("login").RawValue(WebLogin_StateJson());
    w.EndObject();
    *reply = w.Ok() ? w.Str() : std::string("{\"accepted\":false,\"message\":\"reply could not be built\"}");
    return true;
}

<!-- preserved-content:end -->
```
