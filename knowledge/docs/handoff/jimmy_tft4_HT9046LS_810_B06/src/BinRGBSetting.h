//---------------------------------------------------------------------------
// //Eastsun 20260825 : Mag Bin RGB
#ifndef BinRGBSettingH
#define BinRGBSettingH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfBinRGBSetting : public TForm
{
__published:
    TPanel       *pnlGrid;
    TStringGrid  *sgBinRGB;
    TGroupBox    *gbActions;
    TButton      *btnPick;
    TButton      *btnClearRow;
    TButton      *btnReload;
    TButton      *btnSave;
    TPanel       *pnlBottom;
    TButton      *btnClose;
    TColorDialog *dlgColor;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall sgBinRGBDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall sgBinRGBDblClick(TObject *Sender);
    void __fastcall sgBinRGBSetEditText(TObject *Sender, int ACol, int ARow, const AnsiString Value);
    void __fastcall btnPickClick(TObject *Sender);
    void __fastcall btnClearRowClick(TObject *Sender);
    void __fastcall btnReloadClick(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall btnCloseClick(TObject *Sender);
private:
    void RefreshGrid();
public:
    __fastcall TfBinRGBSetting(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfBinRGBSetting *fBinRGBSetting;
//---------------------------------------------------------------------------
#endif
