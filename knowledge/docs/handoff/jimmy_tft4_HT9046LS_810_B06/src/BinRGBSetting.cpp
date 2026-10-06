//---------------------------------------------------------------------------
// //Eastsun 20260825 : Mag Bin RGB
#include "MachineDefine.h"
#pragma hdrstop

#include "BinRGBSetting.h"
#include "cmydef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfBinRGBSetting *fBinRGBSetting;
//---------------------------------------------------------------------------
// Eastsun 20260903 : KYEC customers - BIN idx 0..7 locked (fixed colors)
static bool IsKYECFixedBin(int idx)
{
    // Eastsun 20260903 : BIN 1..8 (grid idx 1..8) are locked for KYEC
    if(idx < 1 || idx > 8) return false;
    return (CUSTOMER_CODE == CC_KYEC_LEE   ||
            CUSTOMER_CODE == CC_KYEC_JCTHIU ||
            CUSTOMER_CODE == CC_KYEC_CHEN  ||
            CUSTOMER_CODE == CC_KYEC_XILINX ||
            CUSTOMER_CODE == CC_KYEC_STM);   // Eastsun 20260904 : add STM
}
//---------------------------------------------------------------------------
__fastcall TfBinRGBSetting::TfBinRGBSetting(TComponent* Owner)
    : TForm(Owner)
{
    LoadBinRGBTable();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::FormShow(TObject *Sender)
{
    int rows = iTestBinCount;
    if(rows < 1) rows = 1;
    if(rows > TEST_MAX_BIN) rows = TEST_MAX_BIN;
    sgBinRGB->ColCount = 6; // Eastsun 20260827: +Barcode column
    sgBinRGB->RowCount = rows + 1;
    sgBinRGB->FixedRows = 1;
    sgBinRGB->ColWidths[0] = 40;
    sgBinRGB->ColWidths[1] = 50;
    sgBinRGB->ColWidths[2] = 50;
    sgBinRGB->ColWidths[3] = 50;
    sgBinRGB->ColWidths[4] = 60;
    sgBinRGB->ColWidths[5] = 150;
    sgBinRGB->Cells[0][0] = "Bin";
    sgBinRGB->Cells[1][0] = "R";
    sgBinRGB->Cells[2][0] = "G";
    sgBinRGB->Cells[3][0] = "B";
    sgBinRGB->Cells[4][0] = "Preview";
    sgBinRGB->Cells[5][0] = "Barcode";
    RefreshGrid();
}
//---------------------------------------------------------------------------
void TfBinRGBSetting::RefreshGrid()
{
    int rows = sgBinRGB->RowCount - 1;
    for(int i=0; i<rows && i<TEST_MAX_BIN; i++)
    {
        int row = i + 1;
        sgBinRGB->Cells[0][row] = AnsiString(i);
        if(BinRGBTable[i].R>=0 && BinRGBTable[i].R<=255 &&
           BinRGBTable[i].G>=0 && BinRGBTable[i].G<=255 &&
           BinRGBTable[i].B>=0 && BinRGBTable[i].B<=255)
        {
            sgBinRGB->Cells[1][row] = AnsiString(BinRGBTable[i].R);
            sgBinRGB->Cells[2][row] = AnsiString(BinRGBTable[i].G);
            sgBinRGB->Cells[3][row] = AnsiString(BinRGBTable[i].B);
        }
        else
        {
            sgBinRGB->Cells[1][row] = "";
            sgBinRGB->Cells[2][row] = "";
            sgBinRGB->Cells[3][row] = "";
        }
        sgBinRGB->Cells[4][row] = "";
        sgBinRGB->Cells[5][row] = BinRGBTable[i].Barcode; // Eastsun 20260827
    }
    sgBinRGB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::sgBinRGBDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
    int idxRow = ARow - 1;
    // Eastsun 20260903 : KYEC locked bins - silver background on R/G/B cells only
    // (skip Preview col 4 and Barcode col 5 - both stay normal)
    if(ACol != 4 && ACol != 5 && ARow >= 1 && IsKYECFixedBin(idxRow))
    {
        sgBinRGB->Canvas->Brush->Color = clSilver;
        sgBinRGB->Canvas->FillRect(Rect);
        sgBinRGB->Canvas->Font->Color = clBlack;
        AnsiString txt = sgBinRGB->Cells[ACol][ARow];
        sgBinRGB->Canvas->TextOut(Rect.Left + 2, Rect.Top + 2, txt);
        return;
    }
    if(ACol == 4 && ARow >= 1)
    {
        int idx = ARow - 1;
        if(idx >= 0 && idx < TEST_MAX_BIN &&
           BinRGBTable[idx].R>=0 && BinRGBTable[idx].R<=255 &&
           BinRGBTable[idx].G>=0 && BinRGBTable[idx].G<=255 &&
           BinRGBTable[idx].B>=0 && BinRGBTable[idx].B<=255)
        {
            TColor c = (TColor)((BinRGBTable[idx].B << 16) | (BinRGBTable[idx].G << 8) | BinRGBTable[idx].R);
            sgBinRGB->Canvas->Brush->Color = c;
            sgBinRGB->Canvas->FillRect(Rect);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::sgBinRGBDblClick(TObject *Sender)
{
    btnPickClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::sgBinRGBSetEditText(TObject *Sender, int ACol, int ARow, const AnsiString Value)
{
    if(ARow < 1 || ARow >= sgBinRGB->RowCount) return;
    int idx = ARow - 1;
    if(idx < 0 || idx >= TEST_MAX_BIN) return;
    // Eastsun 20260903 : KYEC BIN 1..8 lock R/G/B but allow barcode edit
    if(IsKYECFixedBin(idx) && ACol != 5) { RefreshGrid(); return; }
    int v = -1;
    if(!Value.IsEmpty()) v = StrToIntDef(Value, -1);
    if(ACol == 1) BinRGBTable[idx].R = v;
    else if(ACol == 2) BinRGBTable[idx].G = v;
    else if(ACol == 3) BinRGBTable[idx].B = v;
    else if(ACol == 5) BinRGBTable[idx].Barcode = Value; // Eastsun 20260827
    sgBinRGB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::btnPickClick(TObject *Sender)
{
    int row = sgBinRGB->Row;
    if(row < 1 || row >= sgBinRGB->RowCount) return;
    int idx = row - 1;
    if(idx < 0 || idx >= TEST_MAX_BIN) return;
    if(IsKYECFixedBin(idx)) return; // Eastsun 20260903 : KYEC locked
    if(BinRGBTable[idx].R>=0 && BinRGBTable[idx].R<=255 &&
       BinRGBTable[idx].G>=0 && BinRGBTable[idx].G<=255 &&
       BinRGBTable[idx].B>=0 && BinRGBTable[idx].B<=255)
        dlgColor->Color = (TColor)((BinRGBTable[idx].B << 16) | (BinRGBTable[idx].G << 8) | BinRGBTable[idx].R);
    else
        dlgColor->Color = clWhite;
    if(dlgColor->Execute())
    {
        int c = (int)dlgColor->Color;
        BinRGBTable[idx].R = c & 0xFF;
        BinRGBTable[idx].G = (c >> 8) & 0xFF;
        BinRGBTable[idx].B = (c >> 16) & 0xFF;
        RefreshGrid();
    }
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::btnClearRowClick(TObject *Sender)
{
    int row = sgBinRGB->Row;
    if(row < 1 || row >= sgBinRGB->RowCount) return;
    int idx = row - 1;
    if(idx < 0 || idx >= TEST_MAX_BIN) return;
    if(IsKYECFixedBin(idx)) return; // Eastsun 20260903 : KYEC locked
    BinRGBTable[idx].R = -1;
    BinRGBTable[idx].G = -1;
    BinRGBTable[idx].B = -1;
    BinRGBTable[idx].Barcode = ""; // Eastsun 20260827
    RefreshGrid();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::btnReloadClick(TObject *Sender)
{
    // Reload also refresh RowCount in case iTestBinCount changed since FormShow      //Eastsun 20260825 : Mag Bin RGB
    LoadBinRGBTable();
    int rows = iTestBinCount;                                                         //Eastsun 20260825 : Mag Bin RGB
    if(rows < 1) rows = 1;                                                            //Eastsun 20260825 : Mag Bin RGB
    if(rows > TEST_MAX_BIN) rows = TEST_MAX_BIN;                                      //Eastsun 20260825 : Mag Bin RGB
    sgBinRGB->RowCount = rows + 1;                                                    //Eastsun 20260825 : Mag Bin RGB
    RefreshGrid();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::btnSaveClick(TObject *Sender)
{
    SaveBinRGBTable();
    LoadBinRGBTable();
}
//---------------------------------------------------------------------------
void __fastcall TfBinRGBSetting::btnCloseClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
