// ===========================================================================
//  TesterComm/Gpib/GpibUiSnapshot.cpp -- see GpibUiSnapshot.h.  AI(W906-GB-P7) 20260926.
//
//  Snapshot JSON (keys are the golden widget names so the page maps 1:1 to Main.dfm):
//    { "up":bool, "driver":"...", "version":"...", "caption":"...", "closeReason":"...", "notice":"...",
//      "leds":[{"label":"...","on":bool} x13], "status":["..." x8],
//      "labels":{"lblGPIBWnd":"...", "lblTestTime":"...", "labStatus":"...", ...},
//      "checks":{"chkUpperCase":{"checked":bool,"visible":bool}, ...},
//      "combos":{"cbbGPIBTimo":{"index":n,"text":"...","items":[...]}, ...},
//      "panels":{"palSCKART":bool, "pnlAMDCmd":bool, "pnlHanaART":bool, "tsRS232":bool},
//      "sites":[{"caption":"Site 01","site":"01","color":n,"bin":n,"binText":"...","on":bool,"ocr":"..."} x32],
//      "logs":{"Memo1":[...], "lstRecord":[...], "mmoBINON":[...], "memoAlarmCode":[...]} }
//
//  Commands (one line of text, space separated; `text` = the rest of the line):
//    click <button>                  btnManualStart btnSendTemp btnRunMode btnUpdate btnSaveLog btnDiagZip
//                                    spbAutoRetest btnHANA_SendCB btnSend_ED btnHANA_SendCBTotester
//                                    btnSend_EDToTester
//    check <box> 0|1                 chkUpperCase chkStrLengthCheck cbBinonEcho cbFullSiteTimeOut
//                                    cbGPIBWriteWithout_r_n chkTempReady chkCMDLog
//    combo <combo> <index>           cbbGPIBTimo cbbFullsiteTimeOut cbbRunMode cbMode cbCmdHANAART cbCmdHANAARTtoTester
//    radio rgController <index>
//    edit <edit> <text>              edCmdHANAART edCmdHANAARTtoTester
//    site <0..31> on 0|1             MY_DUT_PAL[i]->cbSiteOn->Checked
//    site <0..31> bin <index>        MY_DUT_PAL[i]->cbBin->ItemIndex (+Text), the simulate-bin choice
//  The OnClick / OnChange wiring is golden Main.dfm's (cbbFullsiteTimeOut's OnChange is cbbGPIBTimoChange there).
// ===========================================================================
#include "TesterComm/Gpib/GpibUiSnapshot.h"
#include "TesterComm/Gpib/GpibBridge.h"
#include "TesterComm/UiChannel.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace gpibbridge {

namespace {

using testercomm::JsonEscape;

const int kLogTail = 200;

std::string S(const AnsiString& a) { return std::string(a.c_str()); }
std::string Q(const AnsiString& a) { return JsonEscape(S(a)); }
const char* B(bool b) { return b ? "true" : "false"; }

std::string Num(long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);
    return b;
}

std::string Lines(const TStringList* l, int tail)
{
    std::string o = "[";
    if (l)
    {
        const int n = l->GetCount();
        const int from = n > tail ? n - tail : 0;
        for (int i = from; i < n; ++i)
        {
            if (i > from)
                o += ',';
            o += Q(l->GetString(i));
        }
    }
    return o + "]";
}

std::string Combo(const TComboBox* c)
{
    if (!c)
        return "null";
    return "{\"index\":" + Num(c->ItemIndex) + ",\"text\":" + Q(c->Text) + ",\"visible\":" + B(c->Visible) +
           ",\"enabled\":" + B(c->Enabled) + ",\"items\":" + Lines(c->Items, 300) + "}";
}

std::string Check(const TCheckBox* c)
{
    if (!c)
        return "null";
    return std::string("{\"checked\":") + B(c->Checked) + ",\"visible\":" + B(c->Visible) + ",\"enabled\":" +
           B(c->Enabled) + ",\"caption\":" + Q(c->Caption) + "}";
}

// VCL csDropDownList: setting ItemIndex also shows Items[ItemIndex] (vclcompat keeps the two apart).
void SetComboIndex(TComboBox* c, int idx)
{
    if (!c || !c->Items)
        return;
    if (idx < -1 || idx >= c->Items->GetCount())
        return;
    c->ItemIndex = idx;
    c->Text = idx >= 0 ? c->Items->GetString(idx) : AnsiString("");
}

}  // namespace

std::string BuildUiSnapshot(bool up, const char* driverName)
{
    TSerialPoll* sp = SerialPoll;
    if (sp == 0)
        return std::string();
    std::string o;
    o.reserve(16384);
    o += "{\"up\":";
    o += B(up);
    o += ",\"driver\":" + JsonEscape(driverName ? driverName : "");
    o += ",\"version\":" + Q(GPIBVersion);
    o += ",\"caption\":" + Q(sp->Caption);
    o += ",\"closeReason\":" + Q(sp->closeReason);
    o += ",\"notice\":" + Q(sp->lastNotice);

    TALed* leds[13] = { sp->ALed1, sp->ALed2, sp->ALed3, sp->ALed4, sp->ALed5, sp->ALed6, sp->ALed7,
                        sp->ALed8, sp->ALed9, sp->ALed10, sp->ALed11, sp->ALed12, sp->ALed13 };
    TLabel* ledLabels[13] = { sp->Label11, sp->Label12, sp->Label13, sp->Label14, sp->Label15, sp->Label16,
                              sp->Label17, sp->Label18, sp->Label19, sp->Label20, sp->Label21, sp->Label22,
                              sp->Label23 };
    o += ",\"leds\":[";
    for (int i = 0; i < 13; ++i)
    {
        if (i)
            o += ',';
        o += "{\"label\":" + (ledLabels[i] ? Q(ledLabels[i]->Caption) : std::string("\"\"")) + ",\"on\":" +
             B(leds[i] && leds[i]->Value) + "}";
    }
    o += "]";

    o += ",\"status\":[";
    for (int i = 0; i < TStatusPanels::kCount; ++i)
    {
        if (i)
            o += ',';
        o += (sp->StatusBar1 && sp->StatusBar1->Panels) ? Q(sp->StatusBar1->Panels->Items[i]->Text)
                                                         : std::string("\"\"");
    }
    o += "]";

    struct NamedLabel { const char* name; TLabel* l; };
    NamedLabel labels[] = {
        { "lblGPIBWnd", sp->lblGPIBWnd }, { "lblTestTime", sp->lblTestTime }, { "labDebugMode", sp->labDebugMode },
        { "labStatus", sp->labStatus }, { "labLotID", sp->labLotID }, { "labLotCount", sp->labLotCount },
        { "labTestCount", sp->labTestCount }, { "labPackageType", sp->labPackageType }, { "labOcr", sp->labOcr } };
    o += ",\"labels\":{";
    for (size_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(labels[i].name) + ":{\"caption\":" +
             (labels[i].l ? Q(labels[i].l->Caption) : std::string("\"\"")) + ",\"visible\":" +
             B(labels[i].l && labels[i].l->Visible) + "}";
    }
    o += "}";

    struct NamedCheck { const char* name; TCheckBox* c; };
    NamedCheck checks[] = {
        { "chkUpperCase", sp->chkUpperCase }, { "chkStrLengthCheck", sp->chkStrLengthCheck },
        { "cbBinonEcho", sp->cbBinonEcho }, { "cbFullSiteTimeOut", sp->cbFullSiteTimeOut },
        { "cbGPIBWriteWithout_r_n", sp->cbGPIBWriteWithout_r_n }, { "chkTempReady", sp->chkTempReady },
        { "chkCMDLog", sp->chkCMDLog } };
    o += ",\"checks\":{";
    for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(checks[i].name) + ":" + Check(checks[i].c);
    }
    o += "}";

    struct NamedCombo { const char* name; TComboBox* c; };
    NamedCombo combos[] = {
        { "cbbGPIBTimo", sp->cbbGPIBTimo }, { "cbbFullsiteTimeOut", sp->cbbFullsiteTimeOut },
        { "cbbRunMode", sp->cbbRunMode }, { "cbMode", sp->cbMode }, { "cbCmdHANAART", sp->cbCmdHANAART },
        { "cbCmdHANAARTtoTester", sp->cbCmdHANAARTtoTester } };
    o += ",\"combos\":{";
    for (size_t i = 0; i < sizeof(combos) / sizeof(combos[0]); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(combos[i].name) + ":" + Combo(combos[i].c);
    }
    o += "}";
    o += ",\"rgController\":" + Num(sp->rgController ? sp->rgController->ItemIndex : -1);
    o += ",\"spbAutoRetest\":" + std::string(B(sp->spbAutoRetest && sp->spbAutoRetest->Down));

    o += ",\"panels\":{\"palSCKART\":" + std::string(B(sp->palSCKART && sp->palSCKART->Visible)) +
         ",\"pnlAMDCmd\":" + B(sp->pnlAMDCmd && sp->pnlAMDCmd->Visible) +
         ",\"pnlHanaART\":" + B(sp->pnlHanaART && sp->pnlHanaART->Visible) +
         ",\"tsRS232\":" + B(sp->tsRS232 && sp->tsRS232->TabVisible) + "}";

    o += ",\"sites\":[";
    for (size_t i = 0; i < sp->MY_DUT_PAL.size(); ++i)
    {
        TMyDutPanel* p = sp->MY_DUT_PAL[i];
        if (i)
            o += ',';
        if (!p)
        {
            o += "null";
            continue;
        }
        o += "{\"caption\":" + (p->gpSite ? Q(p->gpSite->Caption) : std::string("\"\"")) +
             ",\"site\":" + (p->plSite ? Q(p->plSite->Caption) : std::string("\"\"")) +
             ",\"color\":" + Num(p->plSite ? p->plSite->Color : 0) +
             ",\"bin\":" + Num(p->cbBin ? p->cbBin->ItemIndex : -1) +
             ",\"binText\":" + (p->cbBin ? Q(p->cbBin->Text) : std::string("\"\"")) +
             ",\"on\":" + B(p->cbSiteOn && p->cbSiteOn->Checked) +
             ",\"ocr\":" + (p->labOcr ? Q(p->labOcr->Caption) : std::string("\"\"")) + "}";
    }
    o += "]";

    o += ",\"logs\":{\"Memo1\":" + Lines(sp->Memo1 ? sp->Memo1->Lines : 0, kLogTail) +
         ",\"lstRecord\":" + Lines(sp->lstRecord ? sp->lstRecord->Items : 0, kLogTail) +
         ",\"mmoBINON\":" + Lines(sp->mmoBINON ? sp->mmoBINON->Lines : 0, 100) +
         ",\"memoAlarmCode\":" + Lines(sp->memoAlarmCode ? sp->memoAlarmCode->Lines : 0, 100) + "}";
    o += "}";
    return o;
}

bool ApplyUiCommand(const std::string& command)
{
    TSerialPoll* sp = SerialPoll;
    if (sp == 0)
        return false;
    std::istringstream in(command);
    std::string verb, name;
    in >> verb >> name;
    if (verb.empty() || name.empty())
        return false;

    if (verb == "click")
    {
        if (name == "btnManualStart")              sp->btnManualStartClick(sp->btnManualStart);
        else if (name == "btnSendTemp")            sp->btnSendTempClick(sp->btnSendTemp);
        else if (name == "btnRunMode")             sp->btnRunModeClick(sp->btnRunMode);
        else if (name == "btnUpdate")              sp->btnUpdateClick(sp->btnUpdate);
        else if (name == "btnSaveLog")             sp->btnSaveLogClick(sp->btnSaveLog);
        else if (name == "btnDiagZip")             sp->btnDiagZipClick(sp->btnDiagZip);
        else if (name == "spbAutoRetest")          sp->spbAutoRetestClick(sp->spbAutoRetest);
        else if (name == "btnHANA_SendCB")         sp->btnHANA_SendCBClick(sp->btnHANA_SendCB);
        else if (name == "btnSend_ED")             sp->btnSend_EDClick(sp->btnSend_ED);
        else if (name == "btnHANA_SendCBTotester") sp->btnHANA_SendCBTotesterClick(sp->btnHANA_SendCBTotester);
        else if (name == "btnSend_EDToTester")     sp->btnSend_EDToTesterClick(sp->btnSend_EDToTester);
        else return false;
        return true;
    }
    if (verb == "check")
    {
        int v = -1;
        in >> v;
        if (v != 0 && v != 1)
            return false;
        struct Entry { const char* n; TCheckBox* c; void (TSerialPoll::*h)(TObject*); };
        Entry table[] = {
            { "chkUpperCase", sp->chkUpperCase, &TSerialPoll::chkUpperCaseClick },
            { "chkStrLengthCheck", sp->chkStrLengthCheck, &TSerialPoll::chkStrLengthCheckClick },
            { "cbBinonEcho", sp->cbBinonEcho, &TSerialPoll::cbBinonEchoClick },
            { "cbFullSiteTimeOut", sp->cbFullSiteTimeOut, &TSerialPoll::cbFullSiteTimeOutClick },
            { "cbGPIBWriteWithout_r_n", sp->cbGPIBWriteWithout_r_n, &TSerialPoll::cbGPIBWriteWithout_r_nClick },
            { "chkTempReady", sp->chkTempReady, 0 },   // no OnClick in Main.dfm: read by the AMD code
            { "chkCMDLog", sp->chkCMDLog, 0 } };
        for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
        {
            if (name != table[i].n || table[i].c == 0)
                continue;
            if (!table[i].c->Enabled)
                return false;   // a disabled VCL check box cannot be clicked
            table[i].c->Checked = (v == 1);
            if (table[i].h)
                (sp->*table[i].h)(table[i].c);   // VCL: toggling Checked fires OnClick
            return true;
        }
        return false;
    }
    if (verb == "combo")
    {
        int idx = -2;
        in >> idx;
        struct Entry { const char* n; TComboBox* c; void (TSerialPoll::*h)(TObject*); };
        Entry table[] = {
            { "cbbGPIBTimo", sp->cbbGPIBTimo, &TSerialPoll::cbbGPIBTimoChange },
            { "cbbFullsiteTimeOut", sp->cbbFullsiteTimeOut, &TSerialPoll::cbbGPIBTimoChange },   // Main.dfm quirk
            { "cbbRunMode", sp->cbbRunMode, 0 },
            { "cbMode", sp->cbMode, 0 },
            { "cbCmdHANAART", sp->cbCmdHANAART, 0 },
            { "cbCmdHANAARTtoTester", sp->cbCmdHANAARTtoTester, 0 } };
        for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
        {
            if (name != table[i].n || table[i].c == 0)
                continue;
            SetComboIndex(table[i].c, idx);
            if (table[i].h)
                (sp->*table[i].h)(table[i].c);   // VCL: a user pick fires OnChange
            return true;
        }
        return false;
    }
    if (verb == "radio" && name == "rgController")
    {
        int idx = -2;
        in >> idx;
        if (!sp->rgController || !sp->rgController->Items || idx < 0 || idx >= sp->rgController->Items->GetCount())
            return false;
        sp->rgController->ItemIndex = idx;
        return true;
    }
    if (verb == "edit")
    {
        std::string text;
        std::getline(in, text);
        if (!text.empty() && text[0] == ' ')
            text.erase(0, 1);
        if (name == "edCmdHANAART" && sp->edCmdHANAART)
            sp->edCmdHANAART->Text = text.c_str();
        else if (name == "edCmdHANAARTtoTester" && sp->edCmdHANAARTtoTester)
            sp->edCmdHANAARTtoTester->Text = text.c_str();
        else
            return false;
        return true;
    }
    if (verb == "site")
    {
        const int site = std::atoi(name.c_str());
        std::string what;
        int v = -2;
        in >> what >> v;
        if (site < 0 || site >= (int)sp->MY_DUT_PAL.size() || sp->MY_DUT_PAL[site] == 0)
            return false;
        TMyDutPanel* p = sp->MY_DUT_PAL[site];
        if (what == "on" && (v == 0 || v == 1) && p->cbSiteOn)
        {
            p->cbSiteOn->Checked = (v == 1);   // golden TMyDutPanel has no OnClick: bridge code reads it
            return true;
        }
        if (what == "bin" && p->cbBin)
        {
            SetComboIndex(p->cbBin, v);
            return true;
        }
        return false;
    }
    return false;
}

}  // namespace gpibbridge
