// =============================================================================
//  cTemperFrom_E023.cpp -- todo E-023 TP-2: the hidden Handler System entry on the temperature strip (golden TfTemperFrom)
//
//  //AI(W906-E023-TP2) 20261002 [W906] (St01) new file, ht9045_sm (CMakeLists.txt:2498, same line).  todo
//    D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-023; St02 inventory
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.14; Jimmy RULINGS_20261001 #0 and #40 (the passwords in
//    code / config are test values: translate as golden, never print them).  Kept out of jimmychiu's cTemperFrom.cpp (forms/fTemperFrom.h:82-85
//    lists Panel71/72/73MouseDown as excluded from his wave; nothing of his is changed).
//  golden (V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cTemperFrom.cpp, cp950):
//    :36 bool bGreen=false,bYellow=false;
//    :1702-1718 Panel73MouseDown, :1720-1742 Panel72MouseDown, :1744-1779 Panel71MouseDown
//    dfm palLed :21 (the 22-pixel strip at the right edge): Panel73 :29 (clYellow, OnMouseDown :43), Panel72 :45 (Color 2468626 = green,
//    :59), Panel71 :61 (Color 8388863, :75).  golden shows fTemperFrom at boot and keeps it shown (main.cpp:9156-9157 DoShowUserDefFrom).
//  What golden does: left-click 73 (yellow) -> bYellow; left-click 72 (green) -> bGreen; then a NON-left click on 71 -> HonPrec level and
//    not running -> "Reset the hardware apparatus information?" YES / NO -> (not SOFT_SIMULTE, not CC_HONPREC_QC) the password on the QWERTY
//    keyboard -> HandlerSystem->ShowModal().  Any other click order resets both flags.  Panel73 also calls fAutoTeach->DoAutoTeachProcess().
//
//  WS route: act.temperFrom.mouseDown through the act.* catch-all (tools/wb_serve.cpp:4833 -> JsonBridge/ChanAction.cpp:346 same line) ->
//    ht9045::sjson::W906_TemperFromAct below.  value {"panel":71|72|73,"button":"left"|"right"|"middle","answer"?:"yes"|"no","password"?:s}.
//    Token + WebCmdGuard as every act.*; no FormLock (none needed: memory flags only).
//  The YES / NO box (golden :1762 Application->MessageBox(..., MB_YESNO|MB_TOPMOST)): the browser asks (two-step, as E-022).  The first
//    Panel71 request runs golden up to the box and answers needConfirm (+ needPassword, the golden #ifndef SOFT_SIMULTE / HONPREC_QC rule);
//    the browser asks YES / NO, then (needPassword) the password on its QWERTY keyboard (golden fQwertyKey N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD),
//    and sends both in one second request.
//    [W906] port-only: the second request is accepted only while golden's box is "open" (g_tp2Asked, set when the first request reached
//      :1762); any new mouse-down drops an unanswered box (the web box is not modal).  golden's guards :1747-1752 run again on the answer
//      (stricter: the browser is not trusted).
//  Password: compared HERE, in C++, as golden :1771 (the literal is golden's test value, RULINGS_20261001 #40); it is not on the page
//    (Status.TemperFrom.html used to compare it in JS), not in any reply, not printed.  golden's commented-out :1768 (it also holds the
//    literal) is not copied.  A wrong password = golden's silent return.
//  After YES (+ password): the reply says open "handlersys"; the page posts {open:'handlersys'} (background.html opens HW.HandlerSys.html,
//    whose editlist.get re-checks golden's :1747-1752 in C++, FileRW/_EditPage.cpp:899 GHandlerSys).  [W906] limit: that open gate does not
//    ask for this sequence or the password (it never did); opening HW.HandlerSys.html some other way still passes on level + running alone
//    -- listed for Steven (binding it needs an unlock token and touches the HSys tests).
//  golden oddity, kept: Panel71 resets bGreen / bYellow BEFORE the box (:1760-1761), so a NO or a wrong password needs the whole sequence
//    again.  golden returns silently at every guard; the port replies executed:true with "opened":false (the page stays silent, as golden).
//  SAFETY-GATE (Panel73 :1717): fAutoTeach is not ported -- see the #if 0 below.
//  [W906] one console line per request: "[E023-TP2] ...", without the password.
//  Tests: ctest E023_StatusEvents (tests/test_e023_statusev.cpp) and E023_StatusPages (tools/webprobe/e023_status_selftest.cjs).
// =============================================================================
#include "MachineType.h"          // CC_* customer codes; SOFT_SIMULTE (W906_NO_SOFT_SIMULTE picks the SHIP configuration)
#include "cmydef.h"               // SystemStart, AccessLevel, iDefHonPrecLevel, CUSTOMER_CODE
#include "vclcompat/ShiftState.h" // TMouseButton / mbLeft / mbRight / mbMiddle (golden Controls.hpp)
#include "Public/cJSON.h"

#include <cstdio>
#include <cstring>
#include <string>

bool bGreen=false,bYellow=false;                                                // golden cTemperFrom.cpp:36 (the port's cTemperFrom.cpp:101-105: not declared there)

namespace {

bool g_tp2Asked = false;   // [W906] golden's MB_YESNO box (:1762) is open: the first Panel71 request reached it

std::string E023T_Q(const std::string& s)
{
    std::string o = "\"";
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b; }
        else o += (char)c;
    }
    return o + "\"";
}

std::string E023T_Refuse(const char* guard, const char* goldenLine, const std::string& detail)
{
    std::printf("[E023-TP2] refused: %s (%s) -- %s\n", guard, goldenLine, detail.c_str());
    return "{\"executed\":false,\"guard\":" + E023T_Q(guard) + ",\"goldenLine\":" + E023T_Q(goldenLine) + ",\"detail\":" + E023T_Q(detail) + "}";
}

// golden returned (silently) at goldenLine -- the reply says where, the page says nothing (as golden)
std::string E023T_Returned(int panel, const char* goldenLine)
{
    std::printf("[E023-TP2] Panel%d: golden returned at %s\n", panel, goldenLine);
    return std::string("{\"executed\":true,\"opened\":false,\"returned\":") + E023T_Q(goldenLine) + "}";
}

// golden :1765-1776: does this build / customer ask for the password after YES?
bool E023T_NeedPassword()
{
    #ifndef SOFT_SIMULTE
    if(CUSTOMER_CODE!=CC_HONPREC_QC)                                            //jou 2010-07-27 start : 隱藏畫面加上password防護
        return true;
    #endif
    return false;
}

// golden :1771 (the literal is golden's test value, RULINGS_20261001 #40; never printed)
bool E023T_PasswordRefused(const std::string& edPasswordText)
{
    return edPasswordText!="27025312";
}

}  // namespace

// golden cTemperFrom.cpp:1702-1718 TfTemperFrom::Panel73MouseDown
void W906_E023_Panel73MouseDown(TMouseButton Button)
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              // :1705
        return;
    if(Button==mbLeft)                                                          // :1707
    {
        if(bGreen==false)                                                       // :1709
            bYellow=true;
    }
    else
    {
        bGreen=false;                                                           // :1714
        bYellow=false;
    }
    // SAFETY-GATE(W906-E023-TP2-AUTOTEACH) golden :1717 -- missing dependency: fAutoTeach (golden TfAutoTeach, AutoTeach/AutoTeach.cpp) is not
    //   ported; the same gate as jimmychiu's csystem.cpp:30722-30729 SAFETY-GATE(W906-T6-AUTOTEACH).  golden DoAutoTeachProcess
    //   (AutoTeach.cpp:323-418) runs one step of the manual-step Auto Teach ("Auto alignment mode") when CosFunction.bManualSteplAutoTeach &&
    //   IniConfig.bA56EnableAutoTeachFunciton and the saved step LastSet.iAutoTeachStep is not eATClose (IsRun :217-240) -- that step can move
    //   motors.  Consequence: on such a machine a Panel73 click does not advance the Auto Teach in the port.  Everywhere else it is a no-op
    //   in golden too (IsRun() returns false).  UN-GATE together with csystem.cpp's gate once TfAutoTeach is ported (Jimmy).
#if 0 // SAFETY-GATE(W906-E023-TP2-AUTOTEACH)
    fAutoTeach->DoAutoTeachProcess();                                           //JimmyChiu 20211020 : Auto alignment mode
#endif
}

// golden cTemperFrom.cpp:1720-1742 TfTemperFrom::Panel72MouseDown
void W906_E023_Panel72MouseDown(TMouseButton Button)
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              // :1723
        return;
    if(Button==mbLeft)                                                          // :1725
    {
        if(bGreen==false && bYellow)                                            // :1727
        {
            bGreen=true;
        }
        else
        {
            bGreen=false;                                                       // :1733
            bYellow=false;
        }
    }
    else
    {
        bGreen=false;                                                           // :1739
        bYellow=false;
    }
}

// golden cTemperFrom.cpp:1744-1763 TfTemperFrom::Panel71MouseDown up to the YES / NO box.  0 = returned at a guard (*line), 1 = reached the box.
int W906_E023_Panel71MouseDown(TMouseButton Button, const char** line)
{
    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
       CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                     //Steven 20131101 : Add ASE-K12
    { *line = "V912 cTemperFrom.cpp:1747-1749"; return 0; }

    if(SystemStart || AccessLevel<iDefHonPrecLevel)                             //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
    { *line = "V912 cTemperFrom.cpp:1751-1752"; return 0; }

    if(bGreen==false || bYellow==false || Button==mbLeft)                       // :1754
    {
        bGreen=false;
        bYellow=false;
        *line = "V912 cTemperFrom.cpp:1754-1759";
        return 0;
    }
    bGreen=false;                                                               // :1760
    bYellow=false;
    *line = "V912 cTemperFrom.cpp:1762";
    return 1;                                                                   // :1762 Application->MessageBox("Reset the hardware apparatus information?", NULL, MB_YESNO | MB_TOPMOST)
}

namespace {

bool E023T_Button(const cJSON* root, TMouseButton* b)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(root, "button");
    if (!v || !cJSON_IsString(v) || !v->valuestring) return false;
    if (std::strcmp(v->valuestring, "left") == 0)   { *b = mbLeft;   return true; }
    if (std::strcmp(v->valuestring, "right") == 0)  { *b = mbRight;  return true; }
    if (std::strcmp(v->valuestring, "middle") == 0) { *b = mbMiddle; return true; }
    return false;
}

// the second request: the answer to golden's box (+ the password)
std::string E023T_Answer(bool yes, const cJSON* pw)
{
    if (!g_tp2Asked)
        return E023T_Refuse("not-asked", "[W906] port-only", "no Handler System question is open (the box was not reached, or a new click dropped it)");
    g_tp2Asked = false;
    // [W906] golden :1747-1752 once more (the browser box is not modal; stricter than golden)
    if (CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_KaohSiung_K12) return E023T_Returned(71, "V912 cTemperFrom.cpp:1747-1749");
    if (SystemStart || AccessLevel<iDefHonPrecLevel) return E023T_Returned(71, "V912 cTemperFrom.cpp:1751-1752");
    if (!yes)                                                                   // :1762 != IDYES
        return E023T_Returned(71, "V912 cTemperFrom.cpp:1762-1763 (NO)");
    if (E023T_NeedPassword()) {                                                 // :1765-1776
        const std::string text = (pw && cJSON_IsString(pw) && pw->valuestring) ? pw->valuestring : "";   // :1769 Clear() + :1770 the keyboard
        if (E023T_PasswordRefused(text)) {                                      // :1771
            std::printf("[E023-TP2] Panel71: the password does not match (golden :1771) -> nothing opened\n");
            return std::string("{\"executed\":true,\"opened\":false,\"returned\":") + E023T_Q("V912 cTemperFrom.cpp:1771-1774 (password)") + "}";
        }
    }
    std::printf("[E023-TP2] Panel71: YES%s -> golden HandlerSystem->ShowModal() (V912 cTemperFrom.cpp:1778): the page opens handlersys\n",
                E023T_NeedPassword() ? " + password" : " (no password: SOFT_SIMULTE or CC_HONPREC_QC, golden :1765-1766)");
    return std::string("{\"executed\":true,\"opened\":true,\"open\":\"handlersys\",\"goldenLine\":") + E023T_Q("V912 cTemperFrom.cpp:1778") + "}";
}

}  // namespace

namespace ht9045 {
namespace sjson {
// JsonBridge/ChanAction.cpp:346 declares this at block scope (same namespace) and calls it for every act.temperFrom.*.
std::string W906_TemperFromAct(const std::string& cmd, const std::string& payloadJson)
{
    if (cmd != "act.temperFrom.mouseDown")
        return E023T_Refuse("unknown-action", "", "act.temperFrom.* has mouseDown; got " + cmd);
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return E023T_Refuse("bad-payload", "", "value must be a JSON object string");
    }
    std::string out;
    int panel = 0;
    TMouseButton button = mbLeft;
    const cJSON* p = cJSON_GetObjectItemCaseSensitive(root, "panel");
    const cJSON* a = cJSON_GetObjectItemCaseSensitive(root, "answer");
    if (p && cJSON_IsNumber(p)) panel = (int)p->valuedouble;
    if (panel != 71 && panel != 72 && panel != 73)
        out = E023T_Refuse("bad-payload", "", "panel must be 71, 72 or 73 (golden palLed Panel71..73)");
    else if (!E023T_Button(root, &button))
        out = E023T_Refuse("bad-payload", "", "button must be \"left\", \"right\" or \"middle\" (golden TMouseButton)");
    else if (a && !cJSON_IsNull(a)) {
        const bool yes = cJSON_IsString(a) && a->valuestring && std::strcmp(a->valuestring, "yes") == 0;
        const bool no = cJSON_IsString(a) && a->valuestring && std::strcmp(a->valuestring, "no") == 0;
        if (panel != 71 || (!yes && !no))
            out = E023T_Refuse("bad-payload", "", "an answer belongs to Panel71 and is \"yes\" or \"no\" (golden MB_YESNO)");
        else
            out = E023T_Answer(yes, cJSON_GetObjectItemCaseSensitive(root, "password"));
    } else {
        g_tp2Asked = false;                                                     // [W906] a new mouse-down drops an unanswered box
        if (panel == 73) {
            W906_E023_Panel73MouseDown(button);
            std::printf("[E023-TP2] Panel73 mouse-down (golden :1702-1718)\n");
            out = "{\"executed\":true,\"panel\":73}";
        } else if (panel == 72) {
            W906_E023_Panel72MouseDown(button);
            std::printf("[E023-TP2] Panel72 mouse-down (golden :1720-1742)\n");
            out = "{\"executed\":true,\"panel\":72}";
        } else {
            const char* line = "";
            if (W906_E023_Panel71MouseDown(button, &line) == 0) {
                out = E023T_Returned(71, line);
            } else {
                g_tp2Asked = true;
                std::printf("[E023-TP2] Panel71: golden reached the YES/NO box (V912 cTemperFrom.cpp:1762) -> needConfirm\n");
                out = std::string("{\"executed\":false,\"needConfirm\":true,\"prompt\":[\"Reset the hardware apparatus information?\"],\"needPassword\":") +
                      (E023T_NeedPassword() ? "true" : "false") + ",\"goldenLine\":" + E023T_Q("V912 cTemperFrom.cpp:1762") + "}";
            }
        }
    }
    cJSON_Delete(root);
    return out;
}
}  // namespace sjson
}  // namespace ht9045
