// =============================================================================
//  WebBinDispStatus_St02.cpp -- ST02-C14 part 3: the web "Bin Display Status" pane's data (golden tsUnloadMap).
//  AI(W906-ST02-C14) 20261002 (St02-E helper).  NOT in golden as a file.  Declarations: WebBinDispStatus_St02.h.
//
//  READ ONLY.  W906_StageBinDispStatus_St02 stages the binsel.disp.* tags from what C++ already holds: the facade
//  TfShowBinSelect (forms/fShowBinSelect.h) as golden ChangeBinDispStatus / ShowBinSel painted it, and the bin display
//  controller HSys.BinDisCtrl (the TMyBinDispCtrl getters).  It calls no golden function, writes no global, opens no file
//  and no port.  The page that reads the tags: web/page/ht9045_bindisp_status.js (Status.ShowBinSelect.html, tab bindisp).
//
//  Caller: tools/wb_serve.cpp PublishExtraTags (the tag publish on wb_serve's main-loop thread, the same thread that runs
//    MainTimer3.cpp's once-a-second ChangeBinDispStatus and the St02 dispatcher's Timer1Timer), only while some browser has
//    the BinSelect window (background.html id binselect) open or minimized -- RULINGS_20260930 #12, the motionview rule.
//    With no such window the family leaves the snapshot (the page is not running then).
//  Why wb_serve's source list and not ht9045_sm: it needs ht9045_webbridge (TagSnapshot), which ht9045_sm does not link --
//    the same reason as WebShowBinSelect.cpp.  Not in WebBridge/ either: that directory is ht9045_webbridge, the library
//    wb_gateway links, and it must hold no machine code.
//
//  Golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven (cp950; RULINGS_20261002 #20: this card is 906 only):
//    tsUnloadMap          cShowBinSelect.dfm:1361-2790 (Caption 'Bin Display Status', PageControl1 index 3)
//    ChangeBinDispStatus  cShowBinSelect.cpp:208-386: UnLoadPanel[] :210-216 (the order of the 36 below), ColorMap :218
//                         (5 entries: clGray, clRed, clGreen, 0x000080FF, clBlack), the paint :294-338 (L / E / C / X /
//                         the number), the error flash :325-331, sbRunStatus :376-385, the jump :269-273
//    ShowBinSel           :388-756: UnLoadLabel[i]->Caption (:621 / :625 / :692 / :711) and ->Font->Color (:639 / :647 /
//                         :697-702 / :717-722), the bin list under each cell, e6TrayName order
//    SetAutoVisible       :1436-1477: pnlMag123 / pnlFix789 / pnlAuto456 ->Visible (:1439-1441, :1463-1467); each station's
//                         group box is grpBinDisp[i] (the ctor array :110-130 IS gbAuto1..gbMag14), which bin.visible carries
//    FormShow             :758-764: the tab only for NUMBER_PANEL_TYPE 3 / 4
//
//  TAGS (15; null = unknown, never a made-up 0):
//    binsel.disp.panelType    int   NUMBER_PANEL_TYPE (Gerneral.ini [System])
//    binsel.disp.tabVisible   bool  NUMBER_PANEL_TYPE==3 || ==4 (FormShow :760-764)
//    binsel.disp.ctrl         bool  HSys.BinDisCtrl exists (InstallColorBinDisplay ran)
//    binsel.disp.caption      str   36 pnlX->Caption, TAB-separated, golden :210-216 order        live: fShowBinSelect
//    binsel.disp.color        str   36 pnlX->Color (TColor as unsigned decimal), comma-separated  live: fShowBinSelect
//    binsel.disp.inst         str   36 '1'/'0' UnitHasInstall(i)                                  live: BinDisCtrl
//    binsel.disp.err          str   36 '1'/'0' GerErrNow(i)                                       live: BinDisCtrl
//    binsel.disp.colorNow     str   36 GetColorNow(i) (the ColorMap index), comma-separated       live: BinDisCtrl
//    binsel.disp.binNow       str   36 GetBinNow(i), comma-separated                              live: BinDisCtrl
//    binsel.disp.lbl          str   33 UnLoadLabel[i]->Caption, TAB-separated, e6TrayName order  live: fShowBinSelect
//    binsel.disp.lblColor     str   33 UnLoadLabel[i]->Font->Color, comma-separated               live: fShowBinSelect
//    binsel.disp.groups       str   3 '1'/'0': pnlMag123, pnlFix789, pnlAuto456 ->Visible         live: fShowBinSelect
//    binsel.disp.status       str   sbRunStatus->Panels->Items[0]->Text (:376-385)                live: fShowBinSelect
//    binsel.disp.statusColor  int   sbRunStatus->Color (clRed, or clBtnFace 0x8000000F)           live: fShowBinSelect
//    binsel.disp.jumpSeq      int   W906_BinDispJumpSeq_St02() (golden :272 jumps seen so far)
//  The panel colours already carry golden's black / red flash (bChangeColor flips once per call, and MainTimer3.cpp calls
//    once a second), so the page paints what it gets.  That the panels are painted at all while the web shows the tab is
//    MainTimer3.cpp's W906_BinDispShownScope_St02 (BinDisplay/BinDispBringUp_St02.h banner, [W906]).
//  A TAB / CR / LF inside a caption becomes a space, so the separators stay unambiguous (golden captions never have one).
// =============================================================================
#include "WebBinDispStatus_St02.h"

#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "MachineType.h"                    // eBinDispTotal, e3TrayCount
#include "cmydef.h"                         // NUMBER_PANEL_TYPE
#include "database.h"                       // HSys.BinDisCtrl
#include "BinDisplay/MyBinDisp.h"           // TMyBinDispCtrl getters
#include "BinDisplay/BinDispBringUp_St02.h" // W906_BinDispJumpSeq_St02
#include "forms/fShowBinSelect.h"           // fShowBinSelect (pnlLoader..pnlFix12, UnLoadLabel[], sbRunStatus, pnlMag123..)

#include <cstdio>
#include <string>

namespace {

using webbridge::TagSnapshot;
using webbridge::TagValue;

const std::size_t kBinDispTags = 15;

void StageStr(TagSnapshot& s, const char* tag, bool live, const std::string& v)
{
    s.stage(tag, live ? TagValue::makeString(v) : TagValue::makeNull());
}

void StageInt(TagSnapshot& s, const char* tag, bool live, long long v)
{
    s.stage(tag, live ? TagValue::makeInt(static_cast<std::int64_t>(v)) : TagValue::makeNull());
}

void StageBool(TagSnapshot& s, const char* tag, bool live, bool v)
{
    s.stage(tag, live ? TagValue::makeBool(v) : TagValue::makeNull());
}

std::string Safe(const AnsiString& a)
{
    std::string s(a.c_str());
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] == '\t' || s[i] == '\r' || s[i] == '\n')
            s[i] = ' ';
    return s;
}

void AppendInt(std::string& s, int i, long v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);
    if (i > 0)
        s += ',';
    s += b;
}

void AppendColor(std::string& s, int i, int c)                                  // TColor as unsigned: clBtnFace 0x8000000F stays positive
{
    char b[24];
    std::snprintf(b, sizeof(b), "%lu", (unsigned long)(unsigned int)c);
    if (i > 0)
        s += ',';
    s += b;
}

void AppendText(std::string& s, int i, const AnsiString& a)
{
    if (i > 0)
        s += '\t';
    s += Safe(a);
}

}  // namespace

std::size_t W906_StageBinDispStatus_St02(TagSnapshot& snap)
{
    TfShowBinSelect* const f  = fShowBinSelect;
    TMyBinDispCtrl*  const bd = HSys.BinDisCtrl;
    const bool fLive = (f != NULL);
    const bool bLive = (bd != NULL);

    StageInt (snap, "binsel.disp.panelType",  true, NUMBER_PANEL_TYPE);
    StageBool(snap, "binsel.disp.tabVisible", true, NUMBER_PANEL_TYPE == 3 || NUMBER_PANEL_TYPE == 4);   // golden FormShow :760-764
    StageBool(snap, "binsel.disp.ctrl",       true, bLive);

    // the facade: what golden ChangeBinDispStatus / ShowBinSel put on tsUnloadMap
    std::string caption, color, lbl, lblColor, groups, status;
    long long statusColor = 0;
    if (fLive)
    {
        TPanel* const pnl[] = { f->pnlLoader, f->pnlEmpty, f->pnlColor,                                   // golden :210-216
                                f->pnlAuto1,  f->pnlAuto2, f->pnlAuto3,
                                f->pnlFix1,   f->pnlFix2,  f->pnlFix3,  f->pnlFix4,  f->pnlFix5,  f->pnlFix6,  f->pnlBinBox,
                                f->pnlMag1,   f->pnlMag2,  f->pnlMag3,  f->pnlMag4,  f->pnlMag5,  f->pnlMag6,  f->pnlMag7,
                                f->pnlMag8,   f->pnlMag9,  f->pnlMag10, f->pnlMag11, f->pnlMag12, f->pnlMag13, f->pnlMag14,
                                f->pnlAuto4,  f->pnlAuto5, f->pnlAuto6,
                                f->pnlFix7,   f->pnlFix8,  f->pnlFix9,  f->pnlFix10, f->pnlFix11, f->pnlFix12 };
        static_assert(sizeof(pnl) / sizeof(pnl[0]) == eBinDispTotal, "golden 906 cShowBinSelect.cpp:210-216: one panel per eBinDispName unit");
        for (int i = 0; i < eBinDispTotal; ++i)
        {
            AppendText (caption, i, pnl[i]->Caption);
            AppendColor(color,   i, pnl[i]->Color);
        }
        for (int i = 0; i < e3TrayCount; ++i)                                   // golden ShowBinSel: UnLoadLabel[i] (lblAuto1..lblMag14)
        {
            AppendText (lbl,      i, f->UnLoadLabel[i]->Caption);
            AppendColor(lblColor, i, f->UnLoadLabel[i]->Font->Color);
        }
        groups += f->pnlMag123->Visible  ? '1' : '0';                           // golden SetAutoVisible :1439 / :1440 + :1463-1467 / :1441
        groups += f->pnlFix789->Visible  ? '1' : '0';
        groups += f->pnlAuto456->Visible ? '1' : '0';
        status      = Safe(f->sbRunStatus->Panels->Items[0]->Text);             // golden :376-385
        statusColor = (long long)(unsigned int)f->sbRunStatus->Color;          // clBtnFace 0x8000000F as 2147483663, like the colours above
    }
    StageStr(snap, "binsel.disp.caption",     fLive, caption);
    StageStr(snap, "binsel.disp.color",       fLive, color);
    StageStr(snap, "binsel.disp.lbl",         fLive, lbl);
    StageStr(snap, "binsel.disp.lblColor",    fLive, lblColor);
    StageStr(snap, "binsel.disp.groups",      fLive, groups);
    StageStr(snap, "binsel.disp.status",      fLive, status);
    StageInt(snap, "binsel.disp.statusColor", fLive, statusColor);

    // the controller: the raw unit state ChangeBinDispStatus reads (TMyBinDispCtrl getters, golden :230-240 / :296-299)
    std::string inst, err, colorNow, binNow;
    if (bLive)
    {
        for (int i = 0; i < eBinDispTotal; ++i)
        {
            inst += bd->UnitHasInstall(i) ? '1' : '0';
            err  += bd->GerErrNow(i)      ? '1' : '0';
            AppendInt(colorNow, i, (long)bd->GetColorNow(i));
            AppendInt(binNow,   i, (long)bd->GetBinNow(i));
        }
    }
    StageStr(snap, "binsel.disp.inst",     bLive, inst);
    StageStr(snap, "binsel.disp.err",      bLive, err);
    StageStr(snap, "binsel.disp.colorNow", bLive, colorNow);
    StageStr(snap, "binsel.disp.binNow",   bLive, binNow);

    StageInt(snap, "binsel.disp.jumpSeq", true, (long long)W906_BinDispJumpSeq_St02());
    return kBinDispTags;
}

std::size_t W906_BinDispStatusTagCount_St02()
{
    return kBinDispTags;
}
