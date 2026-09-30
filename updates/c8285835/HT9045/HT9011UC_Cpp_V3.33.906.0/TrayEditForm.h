// ===========================================================================
//  TrayEditForm.h -- golden TTrayEditForm (906_0625_Steven uTrayEditForm.cpp / .h / .dfm) for the web page HW.TrayEdit.
//
//  AI(W906-S10) 20260929 (St02-E): S-10 (TO_STEVEN d04e82d1, user 14:1x).  The laptop's T1 replay stops at PAUSE
//  because the port had no Tray Edit.  Golden, line by line, in TrayEditForm.cpp:
//    EditTray :33-59, FormShow :70-231, FormClose :233-255, mtLoaderBuffer MouseDown / Move / Up :257-344, Change :346,
//    ShowTray :376, SetTray :458, Timer1Timer :499, spbUpdateClick :522-612, SpeedButton2Click :614,
//    Button1Click :619-662, Button2Click :677, SaveJPG :683-736 (see the V906 note there), CheckCanEdit_SG :739-763;
//    and golden cSortCT.cpp's right-click entry points (:436-583, :1776-1885, the runtime rewiring :126-135).
//  The form object golden's other files read is the laptop's facade `TrayEditForm` (acatchtray_shims.h:362, member
//  fShow); this module keeps the rest of the form's state and sets TrayEditForm->fShow exactly where golden does.
//  ShowModal -> the page table opens the web window (W906_FormProgramShow, WebPageTable row "TrayEditForm", kPgBoth);
//  Close -> golden FormClose + the window closes.
//  The page never decides anything: every guard is here.  Thread: the wb_serve tick thread (the command drain).
//  Closing the window on the HMI = golden Cancel (the page table's operator-close edge, WebPageTable.cpp:463).
// ===========================================================================
#ifndef W906_TRAYEDITFORM_H
#define W906_TRAYEDITFORM_H

#include <string>
#include <vector>

// golden uTrayEditForm.h:60 -- also the entry of golden's other callers (alarm recovery: asendic / cContact / note / AOI),
// which are NOT wired to this yet (they block in golden ShowModal; follow-up claim).
void EditTray(int MotorIndexIndex, int iHasMap = 0);

namespace w906trayedit {

// what the page draws (golden mtLoaderBuffer + the form's controls)
struct View {
    bool                     open;              // golden fShow
    int                      motor;             // iEditMotorIndex
    std::string              motorName;         // EditTray's asName (the MES21108 record text)
    int                      hasMap;            // iHasMap
    int                      xItem, yItem, xbItem, ybItem, xbWidth, ybWidth;
    std::vector<int>         color;             // [y * xItem + x] = mtLoaderBuffer SetCellColorIndex
    std::vector<std::string> number;            // [y * xItem + x] = SetCellNumber text ("" = none)
    std::vector<std::pair<int, std::string> > colorMap;   // SetColorMap overrides (index, "#RRGGBB")
    int                      x0, y0, x1, y1;    // iStartX / iStartY / iEndX / iEndY
    std::string              caption;           // Timer1Timer's "(%d,%d)...(%d,%d)"
    bool                     binVisible;        // cbBinCount (always shown, as golden; its items only in iHasMap 2)
    std::vector<std::string> binItems;
    std::string              binValue;
    bool                     updateVisible;     // spbUpdate->Visible
    bool                     noSave;            // CheckBox1->Checked
    View() : open(false), motor(-1), hasMap(0), xItem(0), yItem(0), xbItem(0), ybItem(0), xbWidth(0), ybWidth(0),
             x0(0), y0(0), x1(0), y1(0), binVisible(false), updateVisible(true), noSave(false) {}
};

// one operator action; guard = "" when golden went through, else why golden returned (the page shows it)
struct Result {
    bool        executed;
    std::string guard, detail;
    Result() : executed(true) {}
};

Result RightClick(const std::string& panel);               // golden cSortCT OnMouseDown(mbRight) of that panel
Result Select(int x0, int y0, int x1, int y1, bool moved);   // mtLoaderBuffer MouseDown(x0,y0) [MouseMove(x1,y1) if moved] MouseUp
Result BinCount(const std::string& value);                // cbBinCount->Text
Result NoSave(bool value);                                 // CheckBox1->Checked
Result Update();                                           // spbUpdateClick
Result Cancel();                                           // SpeedButton2Click
Result Fill(const std::string& x, const std::string& y);  // edtXPos / edtYPos + Button1Click
Result Snapshot();                                         // Button2Click
void   Timer1();                                           // Timer1Timer (the page's state poll runs it)
View   GetView();
void   Close();                                            // golden TForm::Close on this form (FormClose + fShow false)
void   ShowQueuedMessages();                               // golden ShowMyMessage of the last action; the caller runs it
                                                           //   after releasing FormLock (it can wait for the operator)

}  // namespace w906trayedit

#endif  // W906_TRAYEDITFORM_H
