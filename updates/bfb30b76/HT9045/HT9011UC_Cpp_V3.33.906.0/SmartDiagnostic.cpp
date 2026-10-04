// ============================================================================
//  SmartDiagnostic.cpp -- Steven 20260925 (Data.SmartDiagnostic web page)
//
//  golden: HT9011UC_Code_V3.33.912.0_20260908_Jimmy\SmartDiagnostic.cpp (cp950),
//  TfSmartDiagnostic::sb_SmarDiagnostic_CreateCyliderNameClick, :594-623.
//  Transcribed VERBATIM apart from the two marked lines.
//
//  WHY THIS BODY IS NOT IN forms/fSmartDiagnostic.cpp.
//  It reads `Cylinder[i].CylinderName`. Cylinder[] is defined in mycylin.cpp,
//  library ht9045_io; forms/fSmartDiagnostic.cpp is in ht9045_forms, which links
//  only vclcompat + ht9045_globals + ht9045_core (CMakeLists.txt target_link_
//  libraries(ht9045_forms ...)), and CMakeLists.txt's own note on that source line
//  records exactly this as the reason the handler was excluded. ht9045_sm links
//  ht9045_io PUBLIC, so the member is defined here and the facade keeps its
//  declaration -- the split forms/fOffSet.h + cOffSet.cpp and forms/fSetup.h +
//  cSetUp.cpp already use.
//
//  ⇒ INTEGRATOR: add this file to add_library(ht9045_sm ...). Its only caller is
//    WebSmartDiag.cpp (wb_serve); no other binary extracts it.
//
//  WHAT IT DOES: in MEMORY only -- no file is written here. It sizes the two
//  grids to MaxCylinderItem+1 rows and fills one row per named cylinder (count 0,
//  start/end time = now), and appends the names to the combo (golden does not
//  Clear the combo first, so pressing it twice lists every name twice -- kept).
//  The operator's Save (FormButtonClick Tag==9) is what writes the record files.
// ============================================================================
#include "forms/fSmartDiagnostic.h"
#include "mycylin.h"        // Cylinder[MaxCylinderItem] (mycylin.cpp, ht9045_io) / TMyCylinder::CylinderName

static const int mrNo = 7;  // VCL Controls.hpp mrNo (same TU-local constant as forms/fSmartDiagnostic.cpp)

//==============================================================================
void __fastcall TfSmartDiagnostic::sb_SmarDiagnostic_CreateCyliderNameClick(
      TObject *Sender)
{
    int ret=W906_MessageDlg("Sure To Create New Report?");         // golden :597 MessageDlg(..., mtConfirmation, mbYes|mbNo, 0) -- asked by the browser, see forms/fSmartDiagnostic.cpp W906_MessageDlg
    TDateTime TimeNow=Now();

    if(ret==mrNo)
        return;

    sg_SmartDiagnostic_Summary->RowCount=W906_CylinderRowsShown()+1;           //AI(W906-F9050-BD) 20261004: was MaxCylinderItem+1 (golden :603); 295 rows unless Type_HT9050, so this grid and SmartDiagnosticRecord.txt do not grow with MaxCylinderItem 295 -> 321 while 9050GPIB decodes as Type_HT9046_LS (mycylin.h W906_CylinderRowsShown)
    sg_SmartDiagnostic_CyliderManagement->RowCount=W906_CylinderRowsShown()+1; //AI(W906-F9050-BD) 20261004: was MaxCylinderItem+1 (golden :604), same reason

    for(int i=0; i<W906_CylinderRowsShown(); i++)                              //AI(W906-F9050-BD) 20261004: was i<MaxCylinderItem (golden :606), same reason
    {
        sg_SmartDiagnostic_Summary->Cells[0][i+1]       = i+1;
        if(Cylinder[i].CylinderName!="")
        {
            sg_SmartDiagnostic_Summary->Cells[1][i+1]   = Cylinder[i].CylinderName;
            sg_SmartDiagnostic_Summary->Cells[2][i+1]   = "0";
            sg_SmartDiagnostic_Summary->Cells[3][i+1]   = "0";
            sg_SmartDiagnostic_Summary->Cells[4][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);   // golden TimeNow.FormatString(...) -- same substitution as forms/fSmartDiagnostic.cpp FormShow
            sg_SmartDiagnostic_Summary->Cells[5][i+1]   = FormatDateTime("YYYY-MM-DD-HH", TimeNow);
            iRecordCyliderOnCount[i]                    = 0;
            iRecordCyliderOffCount[i]                   = 0;
            cob_SmartDiagnostic_CyliderName->Items->Add(Cylinder[i].CylinderName);
            sg_SmartDiagnostic_CyliderManagement->Cells[0][i+1]   =Cylinder[i].CylinderName;
            sg_SmartDiagnostic_CyliderManagement->Cells[1][i+1]   ="0";
        }
    }
}
//==============================================================================
