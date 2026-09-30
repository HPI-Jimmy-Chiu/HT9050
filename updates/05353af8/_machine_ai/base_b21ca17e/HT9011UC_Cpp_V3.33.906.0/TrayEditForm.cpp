// ===========================================================================
//  TrayEditForm.cpp -- golden TTrayEditForm (906_0625_Steven uTrayEditForm.cpp), line by line.  See TrayEditForm.h.
//
//  AI(W906-S10) 20260929 (St02-E).  Golden line numbers are 906_0625_Steven uTrayEditForm.cpp unless marked cSortCT.
//  The VCL widget mtLoaderBuffer (TTMyTray) is kept as plain state (cell colour index / number text, the colour map) and
//  sent to the page (WebTrayEdit.cpp); golden's other controls likewise (CheckBox1, cbBinCount, edtXPos / edtYPos,
//  spbUpdate->Visible, Caption).  Deviations (V906 only, each marked where it happens):
//    * ShowModal cannot block the wb_serve tick thread: the page table opens the window (W906_FormProgramShow) and the
//      form stays open until Update / Cancel / fill / Close.  Only the operator entry (cSortCT right-click) is wired.
//    * SaveJPG: wb_serve has no screen to capture -> the same folder / file name as a text map of the tray (Steven, one
//      line; this is the default until answered).  MySleep(100) before it (FormClose :238) is dropped (it only let the
//      capture finish).
//    * Left / Top centring (FormShow :74-75): the page table places the window.
//    * ShowMyMessage is queued and shown after act.trayEdit releases FormLock (QueueMyMessage below; St02-E2 note 1).
//    * The operator closing the window = golden Cancel (WindowClosed below, from the page table; St02-E2 note 2).
// ===========================================================================
#include "TrayEditForm.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"            // e6TrayName, eocrUninstal, CC_*, rsmFIFOMode ...
#include "cmydef.h"                 // MM* motor indexes, HAS_* codes, iMMAuto / iMMgzTray, iLDTrayNeedManualRemoveTray ...
#include "cprod.h"                  // TestIF, TestIF_File, TrayForm
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "LastSet.h"                // LastSet (SendCT, iRunStartMode, iTester)
#include "common.h"                 // asTrayLogPath, WriteIniData, CheckAndReadIniData, MyForceDirectories
#include "canary_support.h"         // ShowMyMessage
#include "Motor/mymotor.h"          // MOT[], SetTraySingleData
#include "forms/fSecurity.h"        // fSecurity->Insufficient
#include "forms/fSortCT.h"          // fSortCT->pnlLoad (bShowHPICCount)
#include "W906FormShowing.h"        // W906_FormProgramShow
#include "acatchtray_shims.h"       // TrayEditForm (the laptop's facade: fShow); also declares NewRecordProcess (cMyDB.cpp body) --
                                    //   not cMyDB.h: both give the same default arguments, so the two cannot be in one TU

extern void (*W906_TrayEditCloseHook)();   // acatchtray_shims.cpp:99 (S-10 claim 3.6): TTrayEditForm_Facade::Close calls it
namespace ht9045 { extern void (*W906_TrayEditWindowClosedHook)(); }   // WebPageTable.cpp:630 (S-10 claim 3.8), called at :463 (claim 3.7)
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:39-40

namespace {

// ---- golden file statics (:24-31) ----
bool bMouseDown = false;
int  iStartX = 0, iStartY = 0, iEndX = 0, iEndY = 0;
int  BackTray[100][100];
int  iEditMotorIndex = -1;
int  ICType = 0;
bool bEnterSave = true;

// ---- golden form members / controls kept as state ----
int         iSendCT = 0;                // TTrayEditForm::iSendCT
int         iHasMap = 0;                // TTrayEditForm::iHasMap
std::string gMotorName;                 // EditTray's asString (for the page)
bool        gCheckBox1 = false;         // CheckBox1->Checked
std::vector<std::string> gBinItems;     // cbBinCount->Items
std::string gBinText = "1";             // cbBinCount->Text (uTrayEditForm.dfm: Text = '1'; the box is never hidden)
bool        gUpdateVisible = true;      // spbUpdate->Visible
std::string gCaption;                   // Caption

// AI(W906-S10) 20260929 (St02-E, St02-E2 review note 1): golden's ShowMyMessage in this form is always the last
//   thing its handler does (:269-276, :530, :626, :634-641).  Here it would wait in the web message box
//   (tools/wb_serve.cpp W906MbShowMyMessage -> MbWait) while act.trayEdit holds FormLock, so it is queued and
//   w906trayedit::ShowQueuedMessages() shows it once the lock is released: same order, nothing of golden's runs in
//   between.  WAR1676 (fSecurity->Insufficient -> ShowErrorMessage kcode 0) stays where golden has it: a note that
//   does not wait (tools/wb_serve.cpp:496-510).
std::vector<std::pair<std::string, std::string> > gQueuedMsg;
void QueueMyMessage(const char* S1, const char* S2 = "") { gQueuedMsg.push_back(std::make_pair(std::string(S1), std::string(S2))); }

// mtLoaderBuffer (TTMyTray): geometry, per-cell colour index and number text, colour map overrides
struct Grid {
    int xItem, yItem, xbItem, ybItem, xbWidth, ybWidth;
    int color[100][100];
    std::string number[100][100];
    std::vector<std::pair<int, std::string> > colorMap;
    Grid() : xItem(0), yItem(0), xbItem(0), ybItem(0), xbWidth(0), ybWidth(0) {}
    void SetCellColorIndex(int x, int y, int c) { if (x >= 0 && x < 100 && y >= 0 && y < 100) color[x][y] = c; }
    void SetCellNumber(int x, int y, const std::string& s) { if (x >= 0 && x < 100 && y >= 0 && y < 100) number[x][y] = s; }
    void SetCellNumber(int x, int y, int n) { char b[16]; std::snprintf(b, sizeof(b), "%d", n); SetCellNumber(x, y, std::string(b)); }
    void SetColorMap(int idx, const char* rgb)
    {
        for (std::size_t i = 0; i < colorMap.size(); ++i) if (colorMap[i].first == idx) { colorMap[i].second = rgb; return; }
        colorMap.push_back(std::make_pair(idx, std::string(rgb)));
    }
    void Clear()
    {
        for (int x = 0; x < 100; ++x) for (int y = 0; y < 100; ++y) { color[x][y] = 0; number[x][y].clear(); }
        colorMap.clear();
    }
};
Grid mtLoaderBuffer;

bool ValidMotor() { return iEditMotorIndex >= 0 && iEditMotorIndex < MAX_TRAY_MOTOR; }
int  XItem() { return ValidMotor() ? MOT[iEditMotorIndex].Tray.XItem : 0; }
int  YItem() { return ValidMotor() ? MOT[iEditMotorIndex].Tray.YItem : 0; }

bool OcrOn() { return INSTALL_OCR != eocrUninstal && TestIF.bOcrFunction; }   // golden's repeated OCR condition

void FormShow();
void FormClose();
void SaveJPG(const std::string& S);
bool CheckCanEdit_SG(int MotorIndexIndex);

void DoClose()
{
    if (!TrayEditForm->fShow) return;   // already closed (golden Close on a hidden form does nothing)   //AI(W906-S10-FSHOW) 20260930 (St02-E): DIRECT ON PURPOSE -- TrayEditForm's own modal state, maintained by St02's facade as golden (set :120 = golden :76, cleared :221 = golden :239, including the HMI close: WindowClosed :420 -> DoClose -> FormClose); W906_FormShowing would OR in the page-table window state, so a stale "trayedit" window (e.g. left open across a wb_serve restart) would let these guards run on a form EditTray never set up. Baselined (FShow_Audit).
    FormClose();
    W906_FormProgramShow("TrayEditForm", false, "uTrayEditForm.cpp FormClose (Close)");
}

// ---- FormShow :70-231 ----
void FormShow()
{
    gCheckBox1 = false;                                                          // :72 避免異常無法跳出
    iSendCT = LastSet.SendCT[0];                                                 // :73
    TrayEditForm->fShow = true;                                                  // :76
    if (!ValidMotor()) return;
    TMyTray& T = MOT[iEditMotorIndex].Tray;
    mtLoaderBuffer.Clear();
    mtLoaderBuffer.xbItem = T.XBItem;                                            // :78-85
    mtLoaderBuffer.ybItem = T.YBItem;
    mtLoaderBuffer.xbWidth = T.XBWidth;
    mtLoaderBuffer.ybWidth = T.YBWidth;
    mtLoaderBuffer.xItem = T.XItem;
    mtLoaderBuffer.yItem = T.YItem;
    int i, j;
    ICType = HAS_IC;                                                             // :88

    if (OcrOn()) mtLoaderBuffer.SetColorMap(HAS_OCR_OK, "#FF0000");              // :90-93 clRed

    if (iHasMap == 2) {                                                          // :95-100
        gBinItems.clear();                                                       // TComboBox::Clear empties the items AND the text
        gBinText.clear();
        for (i = 0; i <= 6; i++) { char b[8]; std::snprintf(b, sizeof(b), "%d", i); gBinItems.push_back(b); }
    }

    if (iHasMap == 2) {                                                          // :102-120 TSMC 手動整盤
        for (i = 0; i < T.XItem; i++)
            for (j = 0; j < T.YItem; j++) {
                if (T.Data[i][j] == NULL_IC) {
                    BackTray[i][j] = 0;
                    mtLoaderBuffer.SetCellColorIndex(i, j, 0);
                } else {
                    BackTray[i][j] = T.iTarget[i][j];
                    mtLoaderBuffer.SetCellColorIndex(i, j, BackTray[i][j]);
                }
            }
    } else if (iHasMap == 3) {                                                   // :121-138 退 Tray 顯示 Error Bin
        for (i = 0; i < T.XItem; i++)
            for (j = 0; j < T.YItem; j++) {
                if ((T.Data[i][j] == HAS_IC && T.iBinCode[i][j] == TEST_PASS + iTestBinCount) || T.Data[i][j] == HAS_BARCODEERROR_IC)
                    mtLoaderBuffer.SetCellColorIndex(i, j, 4);
                else if (T.Data[i][j] == HAS_IC)
                    mtLoaderBuffer.SetCellColorIndex(i, j, 1);
            }
    } else if (iHasMap == 4) {                                                   // :139-155 DamageTrayMapping
        for (i = 0; i < T.XItem; i++)
            for (j = 0; j < T.YItem; j++) {
                if (T.Data[i][j] == HAS_IC && T.iAOIResult[i][j] == 4)
                    mtLoaderBuffer.SetCellColorIndex(i, j, 4);
                else if (T.Data[i][j] == HAS_IC)
                    mtLoaderBuffer.SetCellColorIndex(i, j, 1);
            }
    } else {                                                                     // :156-227
        for (i = 0; i < T.XItem; i++) {
            for (j = 0; j < T.YItem; j++) {
                if (T.Data[i][j] != NULL_IC) {
                    if (OcrOn() && T.Data[i][j] == HAS_OCR_OK) {                    // :164-169 (golden writes the tray here)
                        MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_OCR_OK);
                        mtLoaderBuffer.SetCellColorIndex(i, j, HAS_OCR_OK);
                        ICType = T.Data[i][j];
                    } else if (OcrOn() && T.Data[i][j] == HAS_OCR_Err) {            // :170-175
                        MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_OCR_Err);
                        mtLoaderBuffer.SetCellColorIndex(i, j, HAS_OCR_Err);
                        ICType = T.Data[i][j];
                    } else if (T.Data[i][j] == HAS_NULL_IC) {                     // :176-181
                        MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_NULL_IC);
                        mtLoaderBuffer.SetCellColorIndex(i, j, HAS_NULL_IC);
                        ICType = T.Data[i][j];
                    } else {                                                      // :182-187 (golden normalises to HAS_IC)
                        MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_IC);
                        mtLoaderBuffer.SetCellColorIndex(i, j, 1);
                        ICType = T.Data[i][j];
                    }
                } else {
                    mtLoaderBuffer.SetCellColorIndex(i, j, 0);                   // :189-192
                }

                if (T.Data[i][j] == HAS_HOT_IC || T.Data[i][j] == HAS_IC)         // :194-213
                    BackTray[i][j] = 1;
                else if (OcrOn() && T.Data[i][j] == HAS_OCR_OK)
                    BackTray[i][j] = HAS_OCR_OK;
                else if (OcrOn() && T.Data[i][j] == HAS_OCR_Err)
                    BackTray[i][j] = HAS_OCR_Err;
                else if (T.Data[i][j] == HAS_NULL_IC)
                    BackTray[i][j] = HAS_NULL_IC;
                else
                    BackTray[i][j] = 0;

                if (iHasMap == 0) {                                               // :215-224 Auto Contact Test
                    if (i == 0 || j == 0) {
                        if (i == 0) mtLoaderBuffer.SetCellNumber(i, j, j + 1);
                        else        mtLoaderBuffer.SetCellNumber(i, j, i + 1);
                    }
                }
            }
        }
    }
    bEnterSave = true;                                                           // :228
    gUpdateVisible = !(iEditMotorIndex == MMPlate1 || iEditMotorIndex == MMPlate2);   // :230 確安
}

// ---- FormClose :233-255 ----
void FormClose()
{
    SaveJPG("Leave");                                                            // :237 (MySleep(100) :238 dropped: see file head)
    TrayEditForm->fShow = false;                                                 // :239
    if (CosFunction.bUseEditLDTrayNeedManualRemoveTray == true) {                // :241-249
        iLDTrayNeedManualRemoveTray = 0;
    } else {
        iLDTrayNeedManualRemoveTray = 0;
        bNeedManualRemoveTray = false;
    }
    if (CosFunction.bShowHPICCount && fSortCT && fSortCT->pnlLoad)               // :251-254 計算加熱盤IC數量
        fSortCT->pnlLoad->Caption = IntToStr(MOT[MMTrayY].Tray.HowManyIC());
}

// ---- mtLoaderBuffer MouseDown :257-314 (X / Y already converted to cells, ConvertIndexCells) ----
w906trayedit::Result MouseDown(int X, int Y)
{
    w906trayedit::Result r;
    if (iEditMotorIndex == MMPlate1 || iEditMotorIndex == MMPlate2) {            // :260-261
        r.executed = false; r.guard = "plate"; r.detail = "Hot Plate: view only (golden :260)"; return r;
    }
    if (CUSTOMER_CODE == CC_SIGURD_HUKOU) {                                      // :263-278
        if (CheckCanEdit_SG(iEditMotorIndex) == false) {
            if (iEditMotorIndex == MMTrayY) QueueMyMessage("Tray Data Not Edit! (Loader)");
            else                            QueueMyMessage("Tray Data Not Edit! (Unloader)");
            r.executed = false; r.guard = "not-editable"; r.detail = "Gerneral.ini [TrayEdit] (golden CheckCanEdit_SG)"; return r;
        }
    }
    if (CosFunction.bUnloaderEditTrayLevelSet) {                                 // :280-292
        if (iEditMotorIndex == MMTrayY) {
            if (fSecurity->Insufficient(104) == false) { r.executed = false; r.guard = "not-authorized"; r.detail = "Security 104"; return r; }
        } else {
            if (fSecurity->Insufficient(160) == false) { r.executed = false; r.guard = "not-authorized"; r.detail = "Security 160"; return r; }
        }
    } else {
        if (fSecurity->Insufficient(104) == false) {                             // :293-299 SPIL蘇州要求Tray編輯要有權限
            r.executed = false; r.guard = "not-authorized"; r.detail = "Security 104"; return r;
        }
    }
    if (X < 0 || X >= mtLoaderBuffer.xItem || Y < 0 || Y >= mtLoaderBuffer.yItem) {   // :302-303
        r.executed = false; r.guard = "out-of-grid"; return r;
    }
    bMouseDown = true;                                                           // :304-308
    iStartX = X; iStartY = Y; iEndX = X; iEndY = Y;
    if (CosFunction.bUseEditLDTrayNeedManualRemoveTray == true && iLDTrayNeedManualRemoveTray == 1)   // :310-313
        iLDTrayNeedManualRemoveTray = 2;
    return r;
}

// ---- ShowTray :376-456 (the drag preview; run for the move that ends the drag) ----
void ShowTray()
{
    int iSX, iSY, iEX, iEY, i, j;
    if (iStartX > iEndX) { iSX = iEndX; iEX = iStartX; } else { iSX = iStartX; iEX = iEndX; }
    if (iStartY > iEndY) { iSY = iEndY; iEY = iStartY; } else { iSY = iStartY; iEY = iEndY; }
    for (i = 0; i < XItem(); i++) {
        for (j = 0; j < YItem(); j++) {
            const bool inRect = (i >= iSX && i <= iEX && j >= iSY && j <= iEY);
            if (inRect) { mtLoaderBuffer.SetCellColorIndex(i, j, 2); continue; }
            if (iHasMap == 2) {                                                  // TSMC 手動整盤
                if (BackTray[i][j] == 0)      mtLoaderBuffer.SetCellColorIndex(i, j, 0);
                else if (BackTray[i][j] == 2) mtLoaderBuffer.SetCellColorIndex(i, j, BackTray[i][j]);
            } else {
                if (BackTray[i][j] == 1)                mtLoaderBuffer.SetCellColorIndex(i, j, 1);
                else if (BackTray[i][j] == 2)           mtLoaderBuffer.SetCellColorIndex(i, j, 3);
                else if (BackTray[i][j] == HAS_NULL_IC) mtLoaderBuffer.SetCellColorIndex(i, j, HAS_NULL_IC);
                else                                    mtLoaderBuffer.SetCellColorIndex(i, j, 0);
            }
        }
    }
}

// ---- MouseMove :316-331 ----
void MouseMove(int X, int Y)
{
    if (iEditMotorIndex == MMPlate1 || iEditMotorIndex == MMPlate2) return;      // :319-320
    if (bMouseDown) {
        if (X < 0 || X >= mtLoaderBuffer.xItem || Y < 0 || Y >= mtLoaderBuffer.yItem) return;   // :325-326
        iEndX = X;
        iEndY = Y;
        ShowTray();                                                              // :329
    }
}

// ---- Change :346-374 ----
void Change()
{
    int iSX, iSY, iEX, iEY;
    if (iStartX > iEndX) { iSX = iEndX; iEX = iStartX; } else { iSX = iStartX; iEX = iEndX; }
    if (iStartY > iEndY) { iSY = iEndY; iEY = iStartY; } else { iSY = iStartY; iEY = iEndY; }
    iStartX = iSX; iStartY = iSY; iEndX = iEX; iEndY = iEY;
}

// ---- SetTray :458-497 ----
void SetTray()
{
    int i, j;
    Change();
    for (i = iStartX; i <= iEndX; i++) {
        for (j = iStartY; j <= iEndY; j++) {
            if (iHasMap == 1) {                                                  // Auto Contact Test
                BackTray[i][j]++;
                if (BackTray[i][j] >= 3) BackTray[i][j] = 0;
            } else if (iHasMap == 2) {                                           // TSMC 手動整盤
                BackTray[i][j] = std::atoi(gBinText.c_str());
            } else {
                if (BackTray[i][j] == 1) BackTray[i][j] = 0;
                else                     BackTray[i][j] = 1;
            }
            if (iHasMap == 1 || iHasMap == 2) {
                if (BackTray[i][j] == 0) mtLoaderBuffer.SetCellNumber(i, j, std::string());
                else                     mtLoaderBuffer.SetCellNumber(i, j, BackTray[i][j]);
            }
            mtLoaderBuffer.SetCellColorIndex(i, j, BackTray[i][j]);
        }
    }
}

// ---- MouseUp :333-344 ----
void MouseUp()
{
    if (iEditMotorIndex == MMPlate1 || iEditMotorIndex == MMPlate2) return;      // :336-337
    if (bMouseDown) {
        bMouseDown = false;
        SetTray();
    }
}

// ---- SaveJPG :683-736 (V906: a text map, see the file head) ----
void SaveJPG(const std::string& S)
{
    if (gCheckBox1 == true) return;                                              // :685
    try {
        Word Year, Month, Day, Hour, Min, Sec, MSec;                             // :690-693
        TDateTime dtPresent = Now();
        DecodeDate(dtPresent, Year, Month, Day);
        DecodeTime(dtPresent, Hour, Min, Sec, MSec);
        if (!DirectoryExists(asTrayLogPath)) {                                   // :694-698
            if (!MyForceDirectories(asTrayLogPath)) return;
        }
        AnsiString subDir = IntToStr(Year) + IntToStr(Month) + IntToStr(Day);    // :699-704
        if (!DirectoryExists(asTrayLogPath + "\\" + subDir)) {
            if (!MyForceDirectories(asTrayLogPath + "\\" + subDir)) return;
        }
        AnsiString filename = IntToStr(Year) + IntToStr(Month) + IntToStr(Day) + IntToStr(Hour) + IntToStr(Min) + IntToStr(Sec) +
                              AnsiString(S.c_str());                             // :705
        AnsiString varb = asTrayLogPath + "\\" + subDir + "\\" + filename;       // :707
        // V906: instead of the screen capture (:709-730) the tray as text, with the same path and name + ".txt"
        std::string txt = "Tray Edit " + S + " -- " + gMotorName + " (motor " + std::to_string(iEditMotorIndex) +
                          ", iHasMap " + std::to_string(iHasMap) + ")\r\n";
        if (ValidMotor()) {
            const TMyTray& T = MOT[iEditMotorIndex].Tray;
            txt += "Tray.Data (row y, column x):\r\n";
            for (int y = 0; y < T.YItem; ++y) {
                for (int x = 0; x < T.XItem; ++x) { txt += std::to_string(T.Data[x][y]); txt += (x + 1 < T.XItem) ? "\t" : ""; }
                txt += "\r\n";
            }
            txt += "Edit map (BackTray):\r\n";
            for (int y = 0; y < T.YItem && y < 100; ++y) {
                for (int x = 0; x < T.XItem && x < 100; ++x) { txt += std::to_string(BackTray[x][y]); txt += (x + 1 < T.XItem) ? "\t" : ""; }
                txt += "\r\n";
            }
        }
        std::FILE* fp = std::fopen((std::string(varb.c_str()) + ".txt").c_str(), "wb");
        if (fp) { std::fwrite(txt.data(), 1, txt.size(), fp); std::fclose(fp); }
    } catch (...) {                                                              // :732-735 allow the form to go on
    }
}

// ---- CheckCanEdit_SG :739-763 ----
bool CheckCanEdit_SG(int MotorIndexIndex)
{
    bool bLoaderNotEdit = false, bUnloaderNotEdit = false;
    AnsiString sPathName = "";
    sPathName.sprintf("D:\\HT9045\\system");
    MyForceDirectories(sPathName, "TTrayEditForm::CheckCanEdit_SG");
    sPathName.sprintf("D:\\HT9045\\system\\Gerneral.ini");
    if (FileExists(sPathName) == false) {
        WriteIniData(sPathName, "TrayEdit", "LoaderTrayCanNotEdit", false);
        WriteIniData(sPathName, "TrayEdit", "UnloaderTrayCanNotEdit", true);
    }
    bLoaderNotEdit   = CheckAndReadIniData(sPathName, "TrayEdit", "LoaderTrayCanNotEdit", false);
    bUnloaderNotEdit = CheckAndReadIniData(sPathName, "TrayEdit", "UnloaderTrayCanNotEdit", true);
    if (MotorIndexIndex == MMTrayY) return !bLoaderNotEdit;
    return !bUnloaderNotEdit;
}

bool FifoLocked()   // spbUpdateClick :526-531 / Button1Click :622-627
{
    return (CUSTOMER_CODE == CC_KYEC_LEE || CUSTOMER_CODE == CC_UTAC_TW) && LastSet.iRunStartMode == rsmFIFOMode;
}

void HookClose() { DoClose(); }

// AI(W906-S10) 20260929 (St02-E, St02-E2 review note 2): the operator closed the HW.TrayEdit window without Cancel.
//   Golden's form is modal and cannot vanish; with the window gone the golden way out is SpeedButton2Click (Cancel:
//   Close -> FormClose, the tray is not written).  The page table calls this on its operator-close edge
//   (WebPageTable.cpp:463), on the wb_serve tick thread; FormLock as act.trayEdit takes it.
struct WindowLock {
    WindowLock()  { ht9045::formjson::FormLock(); }
    ~WindowLock() { ht9045::formjson::FormUnlock(); }
};
void WindowClosed()
{
    WindowLock lock;
    if (!TrayEditForm->fShow) return;   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    std::printf("[S-10] Tray Edit window closed on the HMI without Cancel -> golden SpeedButton2Click (Close)\n");
    std::fflush(stdout);
    DoClose();                                                                   // SpeedButton2Click :616 Close()
}

struct InstallHook {
    InstallHook()
    {
        W906_TrayEditCloseHook = &HookClose;                                     // facade Close -> this form
        ht9045::W906_TrayEditWindowClosedHook = &WindowClosed;                   // page table operator-close edge -> Cancel
    }
} g_installHook;

// ---- golden cSortCT right-click handlers ----
enum Handler { kNone = 0, kLoader, kAuto1Yield, kHP1, kHP2 };

struct PanelDef { const char* name; Handler h; int tag; };
// golden cSortCT.cpp :92-124 (SetObject: pnlCount / pnlYield get Tag = the e6TrayName index) and :126-135 (every
// non-BulkBox pnlCount / pnlYield -> pnlAuto1YieldMouseDown, overriding the dfm's pnlFix*MouseDown), and the dfm's
// own OnMouseDown (cSortCT.dfm): pnlLoader / pnlLoad / pnlLoadingART -> pnlLoaderMouseDown, pnlHP1 / pnlHP2 ->
// pnlHP*MouseDown, pnlLoadTrayCt / pnlLoadCID / pnlCoverTrayD / pnlAuto1..6TrayCt -> pnlAuto1YieldMouseDown with the
// dfm Tag (none set = 0).  Golden :109 gives eFix12 the panel pnlFix11Yield, so pnlFix11Yield ends tagged eFix12.
const PanelDef kPanels[] = {
    {"pnlLoader", kLoader, 0}, {"pnlLoad", kLoader, 0}, {"pnlLoadingART", kLoader, 0},
    {"pnlHP1", kHP1, 0}, {"pnlHP2", kHP2, 0},
    {"pnlLoadTrayCt", kAuto1Yield, 0}, {"pnlLoadCID", kAuto1Yield, 0}, {"pnlCoverTrayD", kAuto1Yield, 0},
    {"pnlAuto1TrayCt", kAuto1Yield, 0}, {"pnlAuto2TrayCt", kAuto1Yield, 0}, {"pnlAuto3TrayCt", kAuto1Yield, 0},
    {"pnlAuto4TrayCt", kAuto1Yield, 0}, {"pnlAuto5TrayCt", kAuto1Yield, 0}, {"pnlAuto6TrayCt", kAuto1Yield, 0},
    {"pnlAuto1", kAuto1Yield, eAuto1}, {"pnlAuto1Yield", kAuto1Yield, eAuto1},
    {"pnlAuto2", kAuto1Yield, eAuto2}, {"pnlAuto2Yield", kAuto1Yield, eAuto2},
    {"pnlAuto3", kAuto1Yield, eAuto3}, {"pnlAuto3Yield", kAuto1Yield, eAuto3},
    {"pnlAuto4", kAuto1Yield, eAuto4}, {"pnlAuto4Yield", kAuto1Yield, eAuto4},
    {"pnlAuto5", kAuto1Yield, eAuto5}, {"pnlAuto5Yield", kAuto1Yield, eAuto5},
    {"pnlAuto6", kAuto1Yield, eAuto6}, {"pnlAuto6Yield", kAuto1Yield, eAuto6},
    {"pnlFix1", kAuto1Yield, eFix1}, {"pnlFix1Yield", kAuto1Yield, eFix1},
    {"pnlFix2", kAuto1Yield, eFix2}, {"pnlFix2Yield", kAuto1Yield, eFix2},
    {"pnlFix3", kAuto1Yield, eFix3}, {"pnlFix3Yield", kAuto1Yield, eFix3},
    {"pnlFix4", kAuto1Yield, eFix4}, {"pnlFix4Yield", kAuto1Yield, eFix4},
    {"pnlFix5", kAuto1Yield, eFix5}, {"pnlFix5Yield", kAuto1Yield, eFix5},
    {"pnlFix6", kAuto1Yield, eFix6}, {"pnlFix6Yield", kAuto1Yield, eFix6},
    {"pnlFix7", kAuto1Yield, eFix7}, {"pnlFix7Yield", kAuto1Yield, eFix7},
    {"pnlFix8", kAuto1Yield, eFix8}, {"pnlFix8Yield", kAuto1Yield, eFix8},
    {"pnlFix9", kAuto1Yield, eFix9}, {"pnlFix9Yield", kAuto1Yield, eFix9},
    {"pnlFix10", kAuto1Yield, eFix10}, {"pnlFix10Yield", kAuto1Yield, eFix10},
    {"pnlFix11", kAuto1Yield, eFix11}, {"pnlFix11Yield", kAuto1Yield, eFix12},   // golden :108-109: :109 re-tags pnlFix11Yield
    {"pnlFix12", kAuto1Yield, eFix12},
    {"pnlMag1", kAuto1Yield, eMag1}, {"pnlMag1Yield", kAuto1Yield, eMag1},
    {"pnlMag2", kAuto1Yield, eMag2}, {"pnlMag2Yield", kAuto1Yield, eMag2},
    {"pnlMag3", kAuto1Yield, eMag3}, {"pnlMag3Yield", kAuto1Yield, eMag3},
    {"pnlMag4", kAuto1Yield, eMag4}, {"pnlMag4Yield", kAuto1Yield, eMag4},
    {"pnlMag5", kAuto1Yield, eMag5}, {"pnlMag5Yield", kAuto1Yield, eMag5},
    {"pnlMag6", kAuto1Yield, eMag6}, {"pnlMag6Yield", kAuto1Yield, eMag6},
    {"pnlMag7", kAuto1Yield, eMag7}, {"pnlMag7Yield", kAuto1Yield, eMag7},
    {"pnlMag8", kAuto1Yield, eMag8}, {"pnlMag8Yield", kAuto1Yield, eMag8},
    {"pnlMag9", kAuto1Yield, eMag9}, {"pnlMag9Yield", kAuto1Yield, eMag9},
    {"pnlMag10", kAuto1Yield, eMag10}, {"pnlMag10Yield", kAuto1Yield, eMag10},
    {"pnlMag11", kAuto1Yield, eMag11}, {"pnlMag11Yield", kAuto1Yield, eMag11},
    {"pnlMag12", kAuto1Yield, eMag12}, {"pnlMag12Yield", kAuto1Yield, eMag12},
    {"pnlMag13", kAuto1Yield, eMag13}, {"pnlMag13Yield", kAuto1Yield, eMag13},
    {"pnlMag14", kAuto1Yield, eMag14}, {"pnlMag14Yield", kAuto1Yield, eMag14},
};

w906trayedit::Result Refused(const char* guard, const char* detail)
{
    w906trayedit::Result r;
    r.executed = false;
    r.guard = guard;
    r.detail = detail;
    return r;
}

// golden cSortCT.cpp :436-466 pnlLoaderMouseDown (Button==mbRight)
w906trayedit::Result PnlLoaderMouseDown()
{
    if (SystemStart) return Refused("system-running", "golden cSortCT :439");
#ifndef SOFT_SIMULTE
    if (CUSTOMER_CODE == CC_PTI) return Refused("customer", "CC_PTI: Loader 禁止編輯 (golden cSortCT :443)");
#endif
    if (MOT[MMTrayY].fHasTray) {
        if (IniConfig.bI27_ManualSortMode && bRunManualSortMode == true) {     // TSMC 手動整盤功能
            EditTray(MMTrayY, 2);
        } else {
            if (CosFunction.bUseEditLDTrayNeedManualRemoveTray == true) iLDTrayNeedManualRemoveTray = 1;   // :455-458
            EditTray(MMTrayY);
        }
    } else if (MOT[MMOCR].fHasTray) {
        EditTray(MMOCR);
    } else {
        return Refused("no-tray", "MOT[MMTrayY] / MOT[MMOCR].fHasTray false (golden cSortCT :447 / :462)");
    }
    return w906trayedit::Result();
}

// golden cSortCT.cpp :468-495 pnlAuto1YieldMouseDown (Button==mbRight), Tag = iTag
w906trayedit::Result PnlAuto1YieldMouseDown(int iTag)
{
    if (SystemStart) return Refused("system-running", "golden cSortCT :476");
    if (IniConfig.bP41UnloadTrayDisableEdit == true) return Refused("customer", "P41 Unload Tray Disable Edit (golden cSortCT :479)");
    if (LastSet.iTester == ON_LINE && USE_TRAY_MAPPING == 1 && USE_KEYENCE_EMPTY == 3 && TestIF_File.bEnableTrayID2 == true)
        return Refused("customer", "tray mapping + TrayID2 on line (golden cSortCT :482-488)");
    if (CUSTOMER_CODE == CC_PTI) return Refused("customer", "CC_PTI: Unloader 禁止編輯 (golden cSortCT :490)");
    if (iTag < 0 || iTag >= eTrayCount) return Refused("no-handler", "tag out of range");
    const int mi = iMMAuto[iTag];
    if (mi < 0 || mi >= MAX_TRAY_MOTOR || !MOT[mi].fHasTray) return Refused("no-tray", "MOT[iMMAuto[Tag]].fHasTray false (golden cSortCT :493)");
    EditTray(mi);
    return w906trayedit::Result();
}

}  // namespace

// ---- EditTray :33-59 ----
void EditTray(int MotorIndexIndex, int iHasMapIn)
{
    const int iTrayNum[16] = {MMTrayY, MMAuto1, MMAuto2, MMAuto3, MManualTray1, MManualTray2, MManualTray3, MMScanAOI,
                              MMPlate1, MMPlate2, MMAuto4, MMAuto5, MMAuto6, MManualTray4, MManualTray5, MManualTray6};
    const char* asName[16] = {"Loader", "Auto 1", "Auto 2", "Auto 3", "Fix 1", "Fix 2", "Fix 3", "AOI", "Hot Plate 1",
                              "Hot Plate 2", "Auto 4", "Auto 5", "Auto 6", "Fix 4", "Fix 5", "Fix 6"};
    AnsiString asString;
    for (int i = 0; i < 16; i++)
        if (MotorIndexIndex == iTrayNum[i]) asString = asName[i];
    NewRecordProcess("MES21108", "Enter Tray Edit Form", asString);            // :54
    gMotorName = asString.c_str();
    iEditMotorIndex = MotorIndexIndex;                                           // :56
    iHasMap = iHasMapIn;                                                         // :57
    FormShow();                                                                  // :58 ShowModal -> FormShow ...
    W906_FormProgramShow("TrayEditForm", true, "uTrayEditForm.cpp:58 ShowModal");   // ... and the page table opens the window
}

namespace w906trayedit {

Result RightClick(const std::string& panel)
{
    if (TrayEditForm->fShow) return Refused("already-open", "the Tray Edit form is open (golden ShowModal: the main screen can't be clicked)");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    for (std::size_t i = 0; i < sizeof(kPanels) / sizeof(kPanels[0]); ++i) {
        if (panel != kPanels[i].name) continue;
        switch (kPanels[i].h) {
        case kLoader:     return PnlLoaderMouseDown();
        case kAuto1Yield: return PnlAuto1YieldMouseDown(kPanels[i].tag);
        case kHP1:                                                               // golden cSortCT :1776-1783 (no SystemStart guard)
            if (!MOT[MMPlate1].fHasTray) return Refused("no-tray", "MOT[MMPlate1].fHasTray false");
            EditTray(MMPlate1);
            return Result();
        case kHP2:                                                               // :1785-1792
            if (!MOT[MMPlate2].fHasTray) return Refused("no-tray", "MOT[MMPlate2].fHasTray false");
            EditTray(MMPlate2);
            return Result();
        default: break;
        }
    }
    return Refused("no-handler", "golden has no right-click handler on this panel");
}

Result Select(int x0, int y0, int x1, int y1, bool moved)
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    Result r = MouseDown(x0, y0);
    if (!r.executed) return r;
    if (moved) MouseMove(x1, y1);                                                // golden MouseMove ran (ShowTray) if the pointer moved on a cell, even back to x0,y0
    MouseUp();
    return r;
}

Result BinCount(const std::string& value)
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    for (std::size_t i = 0; i < gBinItems.size(); ++i)
        if (gBinItems[i] == value) { gBinText = value; return Result(); }        // an item of the list ...
    gBinText = value;                                                            // ... or typed text: the dfm style is the default csDropDown
    return Result();
}

Result NoSave(bool value)
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    gCheckBox1 = value;
    return Result();
}

// ---- spbUpdateClick :522-612 ----
Result Update()
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    if (!gUpdateVisible) return Refused("plate", "spbUpdate is hidden for the Hot Plate (golden :230)");
    int i, j;
    if (FifoLocked()) {                                                          // :526-531
        QueueMyMessage("Not Support FIFO Mode!", "不支援FIFO Mode。");
        return Refused("fifo", "Not Support FIFO Mode! (golden :530)");
    }
    for (i = 0; i < XItem(); i++) {                                              // :534-572
        for (j = 0; j < YItem(); j++) {
            if (iHasMap == 1) {                                                  // Auto Contact Test
                if (BackTray[i][j] == 1)      MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_IC);
                else if (BackTray[i][j] == 2) MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_HOT_IC);   // 這個要放Shuttle 2
                else                          MOT[iEditMotorIndex].SetTraySingleData(i, j, NULL_IC);
            } else if (iHasMap == 2) {                                           // TSMC 手動整盤
                if (BackTray[i][j] == 0) MOT[iEditMotorIndex].SetTraySingleData(i, j, NULL_IC);
                else                     MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_IC, BackTray[i][j]);
            } else {
                if (BackTray[i][j] == 1)                               MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_IC);
                else if (OcrOn() && BackTray[i][j] == HAS_OCR_OK)      MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_OCR_OK);
                else if (OcrOn() && BackTray[i][j] == HAS_OCR_Err)     MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_OCR_Err);
                else if (BackTray[i][j] == HAS_NULL_IC)                MOT[iEditMotorIndex].SetTraySingleData(i, j, HAS_NULL_IC);
                else                                                   MOT[iEditMotorIndex].SetTraySingleData(i, j, NULL_IC);
            }
        }
    }
    if (CosFunction.bUseEditLDTrayNeedManualRemoveTray == true) {                // :574-609
        if (iLDTrayNeedManualRemoveTray == 2) {
            if (IniConfig.bSIGURDFunction) {
                if (IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray || IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT) {
                    if (CosFunction.bSpecialP24) {
                        MOT[MMTrayY].Tray.iNeedManualRemoved = 1;                // to color alarm
                    } else {
                        if (TrayForm.LoaderToEmptyColor[iRunStartMode] == 0) MOT[MMTrayY].Tray.iNeedManualRemoved = 0;   // to empty alarm
                        else                                                  MOT[MMTrayY].Tray.iNeedManualRemoved = 1;   // to color alarm
                    }
                    iManualRemoveTrayCnt = 2;
                } else if (IniConfig.bP39LoaderHasSkipPlaceToEmpty) {
                    MOT[MMTrayY].Tray.bMustToEmpty = true;
                }
            } else {
                if (Tri_Temp_Machine != 1) bNeedManualRemoveTray = true;
            }
        }
    }
    DoClose();                                                                   // :610 Close()
    return Result();
}

Result Cancel()                                                                  // SpeedButton2Click :614-617
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    DoClose();
    return Result();
}

// ---- Button1Click :619-662 ----
Result Fill(const std::string& x, const std::string& y)
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    int iFromY, iFromX;
    if (FifoLocked()) {
        QueueMyMessage("Not Support FIFO Mode!", "不支援FIFO Mode。");
        return Refused("fifo", "Not Support FIFO Mode! (golden :626)");
    }
    iFromX = std::atoi(x.c_str());                                               // :630-631
    iFromY = std::atoi(y.c_str());
    if (iFromX < 0 || iFromX > XItem()) { QueueMyMessage("X parameter error", ""); return Refused("bad-value", "X parameter error"); }
    if (iFromY < 0 || iFromY > YItem()) { QueueMyMessage("X parameter error", ""); return Refused("bad-value", "X parameter error (golden :640 says X for Y too)"); }
    for (int j = 0; j < YItem(); j++) {
        for (int i = 0; i < XItem(); i++) {
            if (((j + 1) < iFromY) || ((j + 1) == iFromY && (i + 1) < iFromX)) {
                BackTray[i][j] = 0;
                MOT[iEditMotorIndex].SetTraySingleData(i, j, NULL_IC);
            } else {
                BackTray[i][j] = 1;
                MOT[iEditMotorIndex].SetTraySingleData(i, j, ICType);
            }
        }
    }
    DoClose();                                                                   // :660
    return Result();
}

Result Snapshot()                                                                // Button2Click :677-680
{
    if (!TrayEditForm->fShow) return Refused("not-open", "");   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    SaveJPG("Enter");
    return Result();
}

// ---- Timer1Timer :499-520 ----
void Timer1()
{
    char str[256];
    if (TrayEditForm->fShow == false) return;   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    if (SystemStart == true && iSendCT != LastSet.SendCT[0]) {
        FormShow();
        bEnterSave = false;                                                      // :507
    }
    std::snprintf(str, sizeof(str), "(%d,%d)...(%d,%d)", iStartX, iStartY, iEndX, iEndY);
    gCaption = str;
    if (bEnterSave == true) {
        bEnterSave = false;
        SaveJPG("Enter");
    }
}

View GetView()
{
    View v;
    v.open = TrayEditForm->fShow;   //AI(W906-S10-FSHOW) 20260930: direct on purpose -- TrayEditForm's own modal state, maintained by St02's facade as golden, not the page table (see :110)
    v.motor = iEditMotorIndex;
    v.motorName = gMotorName;
    v.hasMap = iHasMap;
    v.xItem = mtLoaderBuffer.xItem; v.yItem = mtLoaderBuffer.yItem;
    v.xbItem = mtLoaderBuffer.xbItem; v.ybItem = mtLoaderBuffer.ybItem;
    v.xbWidth = mtLoaderBuffer.xbWidth; v.ybWidth = mtLoaderBuffer.ybWidth;
    const int nx = v.xItem < 100 ? v.xItem : 100, ny = v.yItem < 100 ? v.yItem : 100;
    for (int y = 0; y < ny; ++y)
        for (int x = 0; x < nx; ++x) {
            v.color.push_back(mtLoaderBuffer.color[x][y]);
            v.number.push_back(mtLoaderBuffer.number[x][y]);
        }
    v.colorMap = mtLoaderBuffer.colorMap;
    v.x0 = iStartX; v.y0 = iStartY; v.x1 = iEndX; v.y1 = iEndY;
    v.caption = gCaption;
    v.binVisible = true;                                                         // golden never hides cbBinCount (only its items come from :95-100)
    v.binItems = gBinItems;
    v.binValue = gBinText;
    v.updateVisible = gUpdateVisible;
    v.noSave = gCheckBox1;
    return v;
}

void Close() { DoClose(); }

void ShowQueuedMessages()   // AI(W906-S10) 20260929 (St02-E, St02-E2 review note 1)
{
    std::vector<std::pair<std::string, std::string> > q;
    q.swap(gQueuedMsg);   // an act.trayEdit handled inside the message box's wait queues into a fresh list
    for (std::size_t i = 0; i < q.size(); ++i) ShowMyMessage(q[i].first.c_str(), q[i].second.c_str());
}

}  // namespace w906trayedit
