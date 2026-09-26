// ===========================================================================
//  TesterComm/Rs232/Rs232UiSnapshot.cpp -- see Rs232UiSnapshot.h.  AI(W906-GB-P7) 20260926.
//
//  Snapshot JSON (golden MainForm.dfm names):
//    { "up", "version", "caption", "closeReason", "notice", "mode" (iUseRS232Mode), "ttlCardType",
//      "comm":[bCommConnect x3], "status":[8], "labels":{labDebugMode, labOcr},
//      "checks":{cbShowLog, cbSaveLog, cbShowLog_TTL, cbSaveLog_TTL},
//      "combos":{cbBaudRate, cbByteSize, cbStopBit, cbParity, cbDevice, cbBaudRate_TTL, cbByteSize_TTL,
//                cbStopBit_TTL, cbParity_TTL, cbDevice_TTL, cbDevice_TTL_2},
//      "edits":{edReadIntervalTimeout, edReadIntervalTimeout_TTL},
//      "tabs":{ts_STD, tsTTL} (TabVisible), "sites":[32 x {caption, site, color, bin, binText, on, ocr}],
//      "logs":{MemoLog, MemoBinData, MemoBinData_TTL, MemoVer} }
//  Commands:
//    click <button>        btnUpdate btClear btManualTest btnAllUse btnAllNoUse btnUpdate_TTL btClear_TTL
//                          btnTTL_Manual btnClearSot Button10 Button11 btnEnableSOTCSOT   (MainForm.dfm OnClick)
//    check <box> 0|1       cbShowLog cbSaveLog cbShowLog_TTL cbSaveLog_TTL   (no OnClick in the .dfm: golden reads them)
//    combo <combo> <index> the COM setting combos above (applied by the next btnUpdate / btnUpdate_TTL click, as golden)
//    edit <edit> <text>    edReadIntervalTimeout edReadIntervalTimeout_TTL
//    site <0..31> on 0|1 / site <0..31> bin <index>
// ===========================================================================
#include "TesterComm/Rs232/Rs232UiSnapshot.h"
#include "TesterComm/Rs232/Rs232Bridge.h"
#include "TesterComm/UiChannel.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

namespace rs232std {

namespace {

using testercomm::JsonEscape;

std::string Q(const AnsiString& a) { return JsonEscape(std::string(a.c_str())); }
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
void SetComboIndex(TComboBox* c, int idx)
{
    if (!c || !c->Items || idx < -1 || idx >= c->Items->GetCount())
        return;
    c->ItemIndex = idx;
    c->Text = idx >= 0 ? c->Items->GetString(idx) : AnsiString("");
}

}  // namespace

std::string BuildUiSnapshot(bool up)
{
    TfRS232Main* f = fRS232Main;
    if (f == 0)
        return std::string();
    std::string o;
    o.reserve(16384);
    o += "{\"up\":";
    o += B(up);
    o += ",\"version\":" + Q(RS232Version);
    o += ",\"caption\":" + Q(f->Caption);
    o += ",\"closeReason\":" + Q(f->closeReason);
    o += ",\"notice\":" + Q(f->lastNotice);
    o += ",\"mode\":" + Num(iUseRS232Mode);
    o += ",\"ttlCardType\":" + Num(TTL_CARD_TYPE);
    o += ",\"comm\":[" + std::string(B(f->bCommConnect[0])) + "," + B(f->bCommConnect[1]) + "," +
         B(f->bCommConnect[2]) + "]";
    o += ",\"status\":[";
    for (int i = 0; i < TStatusPanels::kCount; ++i)
    {
        if (i)
            o += ',';
        o += (f->StatusBar1 && f->StatusBar1->Panels) ? Q(f->StatusBar1->Panels->Items[i]->Text) : std::string("\"\"");
    }
    o += "]";
    o += ",\"labels\":{\"labDebugMode\":{\"caption\":" + (f->labDebugMode ? Q(f->labDebugMode->Caption) : std::string("\"\"")) +
         ",\"visible\":" + B(f->labDebugMode && f->labDebugMode->Visible) + "},\"labOcr\":{\"caption\":" +
         (f->labOcr ? Q(f->labOcr->Caption) : std::string("\"\"")) + ",\"visible\":" + B(f->labOcr && f->labOcr->Visible) + "}}";

    struct NC { const char* n; TCheckBox* c; };
    NC checks[] = { { "cbShowLog", f->cbShowLog }, { "cbSaveLog", f->cbSaveLog },
                    { "cbShowLog_TTL", f->cbShowLog_TTL }, { "cbSaveLog_TTL", f->cbSaveLog_TTL } };
    o += ",\"checks\":{";
    for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(checks[i].n) + ":" +
             (checks[i].c ? std::string("{\"checked\":") + B(checks[i].c->Checked) + ",\"enabled\":" + B(checks[i].c->Enabled) + "}"
                          : std::string("null"));
    }
    o += "}";

    struct NB { const char* n; TComboBox* c; };
    NB combos[] = { { "cbBaudRate", f->cbBaudRate }, { "cbByteSize", f->cbByteSize }, { "cbStopBit", f->cbStopBit },
                    { "cbParity", f->cbParity }, { "cbDevice", f->cbDevice }, { "cbBaudRate_TTL", f->cbBaudRate_TTL },
                    { "cbByteSize_TTL", f->cbByteSize_TTL }, { "cbStopBit_TTL", f->cbStopBit_TTL },
                    { "cbParity_TTL", f->cbParity_TTL }, { "cbDevice_TTL", f->cbDevice_TTL },
                    { "cbDevice_TTL_2", f->cbDevice_TTL_2 } };
    o += ",\"combos\":{";
    for (size_t i = 0; i < sizeof(combos) / sizeof(combos[0]); ++i)
    {
        if (i)
            o += ',';
        o += JsonEscape(combos[i].n) + ":" + Combo(combos[i].c);
    }
    o += "}";
    o += ",\"edits\":{\"edReadIntervalTimeout\":" + (f->edReadIntervalTimeout ? Q(f->edReadIntervalTimeout->Text) : std::string("\"\"")) +
         ",\"edReadIntervalTimeout_TTL\":" + (f->edReadIntervalTimeout_TTL ? Q(f->edReadIntervalTimeout_TTL->Text) : std::string("\"\"")) + "}";
    o += ",\"tabs\":{\"ts_STD\":" + std::string(B(f->ts_STD && f->ts_STD->TabVisible)) + ",\"tsTTL\":" +
         B(f->tsTTL && f->tsTTL->TabVisible) + "}";

    o += ",\"sites\":[";
    for (size_t i = 0; i < f->MY_DUT_PAL.size(); ++i)
    {
        TMyDutPanel* p = f->MY_DUT_PAL[i];
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
    o += ",\"logs\":{\"MemoLog\":" + Lines(f->MemoLog ? f->MemoLog->Lines : 0, 200) +
         ",\"MemoBinData\":" + Lines(f->MemoBinData ? f->MemoBinData->Lines : 0, 100) +
         ",\"MemoBinData_TTL\":" + Lines(f->MemoBinData_TTL ? f->MemoBinData_TTL->Lines : 0, 100) +
         ",\"MemoVer\":" + Lines(f->MemoVer ? f->MemoVer->Lines : 0, 60) + "}";
    o += "}";
    return o;
}

bool ApplyUiCommand(const std::string& command)
{
    TfRS232Main* f = fRS232Main;
    if (f == 0)
        return false;
    std::istringstream in(command);
    std::string verb, name;
    in >> verb >> name;
    if (verb.empty() || name.empty())
        return false;

    if (verb == "click")
    {
        if (name == "btnUpdate")              f->btnUpdateClick(f->btnUpdate);
        else if (name == "btClear")           f->btClearClick(f->btClear);
        else if (name == "btManualTest")      f->btManualTestClick(f->btManualTest);
        else if (name == "btnAllUse")         f->btnAllUseClick(f->btnAllUse);
        else if (name == "btnAllNoUse")       f->btnAllNoUseClick(f->btnAllNoUse);
        else if (name == "btnUpdate_TTL")     f->btnUpdate_TTLClick(f->btnUpdate_TTL);
        else if (name == "btClear_TTL")       f->btClear_TTLClick(f->btClear_TTL);
        else if (name == "btnTTL_Manual")     f->btnTTL_ManualClick(f->btnTTL_Manual);
        else if (name == "btnClearSot")       f->btnClearSotClick(f->btnClearSot);
        else if (name == "Button10")          f->Button10Click(f->Button10);
        else if (name == "Button11")          f->Button11Click(f->Button11);
        else if (name == "btnEnableSOTCSOT")  f->btnEnableSOTCSOTClick(f->btnEnableSOTCSOT);
        else return false;
        return true;
    }
    if (verb == "check")
    {
        int v = -1;
        in >> v;
        TCheckBox* c = name == "cbShowLog" ? f->cbShowLog : name == "cbSaveLog" ? f->cbSaveLog :
                       name == "cbShowLog_TTL" ? f->cbShowLog_TTL : name == "cbSaveLog_TTL" ? f->cbSaveLog_TTL : 0;
        if (!c || (v != 0 && v != 1) || !c->Enabled)
            return false;
        c->Checked = (v == 1);
        return true;
    }
    if (verb == "combo")
    {
        int idx = -2;
        in >> idx;
        struct NB { const char* n; TComboBox* c; };
        NB combos[] = { { "cbBaudRate", f->cbBaudRate }, { "cbByteSize", f->cbByteSize }, { "cbStopBit", f->cbStopBit },
                        { "cbParity", f->cbParity }, { "cbDevice", f->cbDevice }, { "cbBaudRate_TTL", f->cbBaudRate_TTL },
                        { "cbByteSize_TTL", f->cbByteSize_TTL }, { "cbStopBit_TTL", f->cbStopBit_TTL },
                        { "cbParity_TTL", f->cbParity_TTL }, { "cbDevice_TTL", f->cbDevice_TTL },
                        { "cbDevice_TTL_2", f->cbDevice_TTL_2 } };
        for (size_t i = 0; i < sizeof(combos) / sizeof(combos[0]); ++i)
        {
            if (name == combos[i].n && combos[i].c && combos[i].c->Enabled)
            {
                SetComboIndex(combos[i].c, idx);
                return true;
            }
        }
        return false;
    }
    if (verb == "edit")
    {
        std::string text;
        std::getline(in, text);
        if (!text.empty() && text[0] == ' ')
            text.erase(0, 1);
        if (name == "edReadIntervalTimeout" && f->edReadIntervalTimeout)
            f->edReadIntervalTimeout->Text = text.c_str();
        else if (name == "edReadIntervalTimeout_TTL" && f->edReadIntervalTimeout_TTL)
            f->edReadIntervalTimeout_TTL->Text = text.c_str();
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
        if (site < 0 || site >= (int)f->MY_DUT_PAL.size() || f->MY_DUT_PAL[site] == 0)
            return false;
        TMyDutPanel* p = f->MY_DUT_PAL[site];
        if (what == "on" && (v == 0 || v == 1) && p->cbSiteOn)
        {
            p->cbSiteOn->Checked = (v == 1);
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

}  // namespace rs232std
