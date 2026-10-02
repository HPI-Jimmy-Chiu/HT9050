// =============================================================================
//  tests/test_home_monitor.cpp -- AI(W906-HOMEMON) 20261001: ctest HomeMonitor
//
//  The web Home Monitor's C++ half (JsonBridge/ChanHome.cpp):
//    A  the tag JSON -- home.rows from golden's runtime THomeClass (Visible rows only, vector order, labName->Caption /
//       Left / Top, edPos->Text, ShowLed's lamp), home.log from ListBox1 (newest first, at most kHomeLogMax), escaping.
//    B  the stage gate -- nothing staged while the window is closed everywhere and the home sequence does not show it;
//       7 tags when a browser has it open (streamWanted) or when fHome->Show() ran (golden FormShow); home.resetOk follows
//       Panel2->Visible (FormShow hides it, case 1100 shows it).
//    C  act.home.abort -- refused (nothing called, nothing changed) while fHome->fShow is false; the happy path runs golden
//       TfHome::sbAbortHomeClick (GaliMotorServoOff + fAbort + Close): the fake 1203 stop hook is called once with golden's
//       reason, SystemStart / fAllMotorHome / motor power go false, the relay output goes off, the form closes; a second
//       press is refused again (the form is closed).
//  Memory only: FAKE 1203 hook (the real one is installed by wb_serve), no card, no file of the machine.  RecordProcess
//  (GaliMotorServoOff's last line) writes only under ctest's redirect roots -- refused (exit 2) outside them.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                 // SystemStart / fAllMotorHome / bMotorPowerState
#include "csystem.h"                // W906_Stop1203AllHook
#include "common.h"                 // as9045LogPath / asSaveEventLogPath (runtime paths for the guard)
#include "Motor/mymotor.h"          // MOT[] / MAX_TRAY_MOTOR
#include "myswitch.h"               // SW[] / SwMotorRelay
#include "forms/fHome.h"
#include "vclcompat/Controls.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"

#include "JsonBridge/ChanHome.h"
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "WebBridge/JsonWriter.h"   // IsValidUtf8
#include "Public/cJSON.h"

#include <cstdio>
#include <cstring>
#include <string>

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_home_monitor.cpp:%d]  %s\n", line, what); }
    else     {           std::printf("  ok    %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static int         g_stopCalls = 0;
static std::string g_stopWhy;
static void FakeStop1203(const char* why) { ++g_stopCalls; g_stopWhy = why ? why : ""; }

// one publish of the home.* family; returns what W906_StageHomeMonitor said it staged, and the published map
static std::size_t Publish(webbridge::TagSnapshot& snap, bool wanted, webbridge::TagMap& out)
{
    snap.beginPublish();
    const std::size_t n = W906_StageHomeMonitor(snap, wanted);
    snap.commitPublish();
    out = snap.read().tags;
    return n;
}
static const webbridge::TagValue* Tag(const webbridge::TagMap& m, const char* k)
{
    webbridge::TagMap::const_iterator it = m.find(k);
    return it == m.end() ? 0 : &it->second;
}
static int HomeKeys(const webbridge::TagMap& m)
{
    int n = 0;
    for (webbridge::TagMap::const_iterator it = m.begin(); it != m.end(); ++it)
        if (it->first.compare(0, 5, "home.") == 0) ++n;
    return n;
}
static int Int(const cJSON* o, const char* k)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k);
    return cJSON_IsNumber(v) ? v->valueint : -9999;
}
static std::string Str(const cJSON* o, const char* k)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k);
    return (cJSON_IsString(v) && v->valuestring) ? std::string(v->valuestring) : std::string("<not a string>");
}
static bool Bool(const cJSON* o, const char* k, bool want)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k);
    return want ? cJSON_IsTrue(v) != 0 : cJSON_IsFalse(v) != 0;
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("HomeMonitor", rt))
        return 2;
    std::printf("=== test_home_monitor (AI(W906-HOMEMON)) ===\n");
    W906_TestEnsureSimMotors();
    W906_Stop1203AllHook = 0;
    CHECK(fHome != 0 && fHome->Panel2 != 0 && fHome->ListBox1 != 0);
    if (fHome == 0) return 1;
    fHome->InitialHomeClass();

    int nVis = 0, firstVis = -1, secondVis = -1;
    for (std::size_t i = 0; i < fHome->HomeClass.size(); ++i)
        if (fHome->HomeClass[i] && fHome->HomeClass[i]->Visible) {
            if (firstVis < 0) firstVis = (int)i; else if (secondVis < 0) secondVis = (int)i;
            ++nVis;
        }
    std::printf("  HomeClass %u, Visible %d (first slot %d, second %d)\n", (unsigned)fHome->HomeClass.size(), nVis, firstVis, secondVis);
    CHECK(nVis >= 2 && firstVis >= 0 && secondVis > firstVis);
    if (nVis < 2) return 1;

    // -----------------------------------------------------------------------
    //  PART A -- the JSON
    // -----------------------------------------------------------------------
    std::printf("\n-- PART A: home.rows / home.log JSON --\n");
    fHome->ResetAllMotorLed();
    fHome->ShowLed(secondVis, 2);                                              // golden ShowLed: 2 = clRed
    fHome->HomeClass[firstVis]->edPos->Text = AnsiString("1234");             // as ShowMotorHomePos writes it (golden :694/:696)
    {
        const std::string rows = ht9045::homemon::RowsJson(*fHome, W906_HomeLedState, MAX_TRAY_MOTOR);
        cJSON* a = cJSON_Parse(rows.c_str());
        CHECK(cJSON_IsArray(a));
        CHECK(a != 0 && cJSON_GetArraySize(a) == nVis);                         // Visible rows only
        const cJSON* r0 = a ? cJSON_GetArrayItem(a, 0) : 0;
        const cJSON* r1 = a ? cJSON_GetArrayItem(a, 1) : 0;
        CHECK(r0 != 0 && Int(r0, "slot") == firstVis && Int(r0, "motor") == fHome->HomeClass[firstVis]->index);
        CHECK(r0 != 0 && Str(r0, "name") == std::string(MOT[firstVis].NumberAlias.c_str()));   // golden :82
        CHECK(r0 != 0 && Int(r0, "x") == 5 && Int(r0, "y") == 8);               // golden layout loop: iLabelL 5, iLabelT 8
        CHECK(r1 != 0 && Int(r1, "slot") == secondVis && Int(r1, "y") == 38);   // next row: + iTPitch 30
        CHECK(r0 != 0 && Str(r0, "pos") == "1234");
        CHECK(r1 != 0 && Str(r1, "pos") == "0");                                // golden :90 edPos->Text=0
        CHECK(r0 != 0 && Int(r0, "lamp") == 0);
        CHECK(r1 != 0 && Int(r1, "lamp") == 2);                                 // ShowLed(secondVis, 2)
        cJSON_Delete(a);
        std::printf("  rows: %u bytes\n", (unsigned)rows.size());
    }
    {
        // a caption the page must still parse: quote, backslash, a control byte, a lone Big5 byte pair
        THomeClass* c = fHome->HomeClass[firstVis];
        const AnsiString save = c->labName->Caption;
        c->labName->Caption = AnsiString("[M00] a\"b\\c\td \xB4\xFA");
        const std::string rows = ht9045::homemon::RowsJson(*fHome, W906_HomeLedState, MAX_TRAY_MOTOR);
        cJSON* a = cJSON_Parse(rows.c_str());
        CHECK(cJSON_IsArray(a) && cJSON_GetArraySize(a) == nVis);
        const std::string nm = a ? Str(cJSON_GetArrayItem(a, 0), "name") : std::string();
        CHECK(nm.compare(0, 13, "[M00] a\"b\\c\td") == 0);                      // escaped by JsonWriter, decoded back by cJSON
        CHECK(webbridge::IsValidUtf8(rows));                                    // the Big5 pair was transcoded, not passed raw
        cJSON_Delete(a);
        c->labName->Caption = save;
    }
    {
        fHome->ListBox1->Clear();
        fHome->ListBox1->Items->Insert(0, AnsiString("M01 home finish."));
        fHome->ListBox1->Items->Insert(0, AnsiString("M02 home finish."));
        cJSON* a = cJSON_Parse(ht9045::homemon::LogJson(*fHome, ht9045::homemon::kHomeLogMax).c_str());
        CHECK(cJSON_IsArray(a) && cJSON_GetArraySize(a) == 2);
        CHECK(a != 0 && std::string(cJSON_GetArrayItem(a, 0)->valuestring) == "M02 home finish.");   // newest first
        CHECK(a != 0 && std::string(cJSON_GetArrayItem(a, 1)->valuestring) == "M01 home finish.");
        cJSON_Delete(a);
        for (int i = 0; i < 150; ++i) { char b[32]; std::snprintf(b, sizeof(b), "line %03d", i); fHome->ListBox1->Items->Insert(0, AnsiString(b)); }
        a = cJSON_Parse(ht9045::homemon::LogJson(*fHome, ht9045::homemon::kHomeLogMax).c_str());
        CHECK(cJSON_IsArray(a) && cJSON_GetArraySize(a) == ht9045::homemon::kHomeLogMax);   // capped
        CHECK(a != 0 && std::string(cJSON_GetArrayItem(a, 0)->valuestring) == "line 149");
        cJSON_Delete(a);
    }

    // -----------------------------------------------------------------------
    //  PART B -- the stage gate
    // -----------------------------------------------------------------------
    std::printf("\n-- PART B: staged only while the window is open --\n");
    webbridge::TagSnapshot snap;
    webbridge::TagMap m;
    fHome->Close();                                                            // golden FormClose: fShow=false
    CHECK(fHome->fShow == false);
    CHECK(Publish(snap, false, m) == 0 && HomeKeys(m) == 0);                    // closed everywhere: nothing
    CHECK(Publish(snap, true, m) == 7 && HomeKeys(m) == 7);                     // a browser has the window open
    {
        const webbridge::TagValue* v = Tag(m, "home.fShow");
        CHECK(v && v->isBool() && v->asBool(true) == false);
        v = Tag(m, "home.step");
        CHECK(v && v->isInt() && v->asInt(-1) == fHome->iHomeStep);
        v = Tag(m, "home.log.count");
        CHECK(v && v->isInt() && v->asInt(-1) == 152);                          // every line, also those beyond the cap
        v = Tag(m, "home.rows");
        cJSON* a = v && v->isString() ? cJSON_Parse(v->asString().c_str()) : 0;
        CHECK(cJSON_IsArray(a) && cJSON_GetArraySize(a) == nVis);
        cJSON_Delete(a);
        v = Tag(m, "home.log");
        a = v && v->isString() ? cJSON_Parse(v->asString().c_str()) : 0;
        CHECK(cJSON_IsArray(a) && cJSON_GetArraySize(a) == ht9045::homemon::kHomeLogMax);
        cJSON_Delete(a);
    }
    fHome->Panel2->Visible = true;
    fHome->Show();                                                             // golden FormShow: Panel2 hidden, fShow, fAbort=false
    CHECK(fHome->fShow == true && fHome->Panel2->Visible == false && fHome->fAbort == false);
    CHECK(Publish(snap, false, m) == 7 && HomeKeys(m) == 7);                    // the home sequence shows it: staged without a browser
    {
        const webbridge::TagValue* v = Tag(m, "home.fShow");
        CHECK(v && v->asBool(false) == true);
        v = Tag(m, "home.resetOk");
        CHECK(v && v->isBool() && v->asBool(true) == false);
    }
    fHome->Panel2->Visible = true;                                             // golden ProcessMotorHome case 1100 (uhome.cpp:3516)
    Publish(snap, false, m);
    {
        const webbridge::TagValue* v = Tag(m, "home.resetOk");
        CHECK(v && v->asBool(false) == true);
    }
    fHome->ListBox1->Items->Insert(0, AnsiString("Reset OK"));
    Publish(snap, false, m);
    {
        const webbridge::TagValue* v = Tag(m, "home.log");
        cJSON* a = v && v->isString() ? cJSON_Parse(v->asString().c_str()) : 0;
        CHECK(a != 0 && std::string(cJSON_GetArrayItem(a, 0)->valuestring) == "Reset OK");   // a new newest line is picked up
        cJSON_Delete(a);
    }

    // -----------------------------------------------------------------------
    //  PART C -- act.home.abort
    // -----------------------------------------------------------------------
    std::printf("\n-- PART C: act.home.abort --\n");
    W906_Stop1203AllHook = &FakeStop1203;
    std::string ack;

    // C1 not open: refused, nothing called
    fHome->Close(); fHome->fAbort = false;
    SystemStart = true; fAllMotorHome = true; bMotorPowerState = true; SW[SwMotorRelay].On();
    g_stopCalls = 0;
    CHECK(W906_HomeAbortWire("{\"source\":\"HW.home\",\"button\":\"sbAbortHome\"}", ack) == false);
    CHECK(ack.compare(0, 9, "not-open:") == 0);
    CHECK(g_stopCalls == 0);
    CHECK(SystemStart == true && fAllMotorHome == true && bMotorPowerState == true && SW[SwMotorRelay].OutValue == true);
    CHECK(fHome->fAbort == false && fHome->fShow == false);

    // C2 happy path: the home sequence shows the form -> golden sbAbortHomeClick
    fHome->Show();
    fHome->iHomeStep = 600;                                                    // somewhere inside ProcessMotorHome
    SystemStart = true; fAllMotorHome = true; bMotorPowerState = true; SW[SwMotorRelay].On(); SW[SwServerON].On();
    g_stopCalls = 0; g_stopWhy.clear(); ack.clear();
    CHECK(W906_HomeAbortWire("{\"source\":\"HW.home\",\"button\":\"sbAbortHome\"}", ack) == true);
    CHECK(g_stopCalls == 1 && g_stopWhy == "GaliMotorServoOff - sbAbortHomeClick");   // GaliMotorServoOff("sbAbortHomeClick")
    CHECK(fHome->fShow == false && fHome->fAbort == true);                     // fAbort=true; Close()
    CHECK(SystemStart == false && fAllMotorHome == false && bMotorPowerState == false);
    CHECK(SW[SwMotorRelay].OutValue == false && SW[SwServerON].OutValue == false);
    CHECK(fHome->iHomeStep == 600);                                            // golden leaves the cursor; ProcessMotorHome's fAbort arm resets it
    {
        cJSON* o = cJSON_Parse(ack.c_str());
        CHECK(cJSON_IsObject(o));
        CHECK(o != 0 && Bool(o, "accepted", true) && Bool(o, "fShow", false) && Bool(o, "fAbort", true));
        CHECK(o != 0 && Bool(o, "systemStart", false) && Bool(o, "fAllMotorHome", false) && Bool(o, "motorPower", false));
        CHECK(o != 0 && Int(o, "homeStep") == 600);
        cJSON_Delete(o);
        std::printf("  ack: %s\n", ack.c_str());
    }
    CHECK(Publish(snap, true, m) == 7);
    {
        const webbridge::TagValue* v = Tag(m, "home.fShow");
        CHECK(v && v->asBool(true) == false);
        v = Tag(m, "home.fAbort");
        CHECK(v && v->asBool(false) == true);
    }

    // C3 a second press: the form is closed -> refused, nothing called again
    SystemStart = true;
    CHECK(W906_HomeAbortWire("{}", ack) == false && ack.compare(0, 9, "not-open:") == 0);
    CHECK(g_stopCalls == 1 && SystemStart == true);

    W906_Stop1203AllHook = 0;
    SystemStart = false; fHome->fAbort = false; fHome->iHomeStep = 1;
    fHome->ListBox1->Clear();
    std::printf("\n=== %d checks, %d FAIL ===\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
