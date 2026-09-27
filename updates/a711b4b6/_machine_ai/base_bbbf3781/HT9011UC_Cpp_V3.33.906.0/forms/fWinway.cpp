// =============================================================================
//  forms/fWinway.cpp  --  TfWinway member bodies
//
//  AI(W906-FW3-OWM1) 20260828: new file. See forms/fWinway.h for the full
//  wave banner, GATE register (W-1/W-2) and the WHY-THIS-FILE-IS-THIN
//  explanation (11 of 17 golden bodies excluded entirely -- every one reaches
//  arrATC_Site[n], an ATC_WinWay* this wave leaves permanently NULL because
//  ATC_WinWay's own .cpp lives in ht9045_comms, unreachable from
//  ht9045_forms).
// =============================================================================
#include "forms/fWinway.h"
#include "forms/fQwertyKey.h"   // fQwertyKey / ShowQwertyKey -- edtSetTempClick
#include "cmydef.h"             // N_DOUBLE, dTempMax -- edtSetTempClick

// AI(W906-FW3-OWM1) 20260828: golden ATC/WinWaySetting.h/.cpp both free
// tree-wide (class name + global name), so this facade declares the global --
// same idiom as forms/fCleaning.cpp:56 / forms/fMesSystem.cpp:597 /
// forms/fSmartDiagnostic.cpp:37. Safe at static-init: the ctor below (PORT
// SPLIT) touches only its own just-`new`'d fields and explicit scalar
// literals, no other global.
TfWinway *fWinway = new TfWinway();

// -----------------------------------------------------------------------
//  ctor -- PORT SPLIT (task-mandated, static-init-ctor safety rule:
//  "只准塞欄位與 new 自己的 widget stand-in，絕對不准 deref 任何其他全域").
//  golden :12-32. Live: the 5 field-only lines (:15-19). GATED: the 4
//  `new ATC_WinWay(WinWayATCCommN)` constructions (:22-25) and the priming
//  Load/SaveCommData loop (:27-31) -- ATC_WinWay's ctor lives in
//  ATC/ATC_WinWay.cpp (ht9045_comms), not reachable from ht9045_forms, and
//  LoadCommData/SaveCommData are independently excluded (write-path +
//  hardware setpoint, see header banner). `arrATC_Site[i]` is explicitly
//  zeroed (not left uninitialized) so the NULL state is a documented choice,
//  not an accident -- every EXCLUDED member above that reads arrATC_Site[n]
//  cites this exact line as why it cannot be safely translated even
//  field-only.
// -----------------------------------------------------------------------
TfWinway::TfWinway()
{
    edtSetTemp->Text = "0";                 // golden :15
    Width = 330;                            // golden :16
    Height = 518;                           // golden :17
    iWinwayATCIndex = 0;                    // golden :18
    bShow = false;                          // golden :20

    for (int i = 0; i < SiteNum; i++)
    {
        arrATC_Site[i] = 0;   // PORT-ONLY: golden :22-25 `new ATC_WinWay(...)` -- GATED below
    }

#if 0
    // golden :22-31, VERBATIM -- GATED, see header banner "WHY THIS FILE IS THIN".
    arrATC_Site[0]=new ATC_WinWay(WinWayATCComm1);
    arrATC_Site[1]=new ATC_WinWay(WinWayATCComm2);
    arrATC_Site[2]=new ATC_WinWay(WinWayATCComm3);
    arrATC_Site[3]=new ATC_WinWay(WinWayATCComm4);
    for(int i=0; i<SiteNum; i++)
    {
        LoadCommData(arrATC_Site[i]->WinwayCOM , i);
        SaveCommData(arrATC_Site[i]->WinwayCOM , i);
    }
#endif
}

// golden :34-41. GATE W-1: `ShowCommData(iWinwayATCIndex);` (:39) -- rest ACTIVE.
void TfWinway::FormShow()
{
    Left = (1024 - Width) / 2;
    Top  = (768 - Height) / 2;
    cbbWinwayATCIndex->ItemIndex = iWinwayATCIndex;

#if 0
    // golden :39, VERBATIM -- GATE W-1. ShowCommData is EXCLUDED (header
    // banner): every line in it reads through arrATC_Site[iIndex], which
    // this wave's ctor leaves permanently NULL.
    ShowCommData(iWinwayATCIndex);
#endif

    bShow = true;
}

// golden :150-154. GATE W-1: `ShowCommData(iWinwayATCIndex);` (:153) -- rest
// ACTIVE. Unread `TObject *Sender` dropped (golden's own body never reads it).
void TfWinway::cbbWinwayATCIndexChange()
{
    iWinwayATCIndex = cbbWinwayATCIndex->ItemIndex;

#if 0
    // golden :153, VERBATIM -- GATE W-1, same reason as FormShow above.
    ShowCommData(iWinwayATCIndex);
#endif
}

// golden :164-168. GATE W-2 -- predetermined name (forms/fLotInfo.cpp:4256,
// `fWinway->OpenCommPort();`, zero-arg -- verified this wave). Entire body
// gated: it only ever calls the int-arg OpenCommPort(_idx) overload, which is
// independently EXCLUDED (SAFETY: opens a real COM port; also
// arrATC_Site[_idx] null-deref/cross-archive -- header banner).
void TfWinway::OpenCommPort()
{
#if 0
    // golden :166-167, VERBATIM -- GATE W-2.
    for(int i=0; i<SiteNum; i++)
        OpenCommPort( i );
#endif
}

// golden :195-201. GATE W-2 -- predetermined name (forms/fLotInfo.cpp:4272,
// `fWinway->SetTempratureAll(dbSetATCTemp);`). Entire body gated: SAFETY
// (hardware setpoint broadcast to all 4 sites via ATC_WinWay::SetST) AND
// arrATC_Site[i] null-deref/cross-archive (header banner).
void TfWinway::SetTempratureAll(double _value)
{
#if 0
    // golden :197-200, VERBATIM -- GATE W-2.
    for(int i=0; i<SiteNum; i++)
    {
        arrATC_Site[i]->SetST(_value);
    }
#endif
    (void)_value;   // suppress -Wunused-parameter while the body above is gated
}

// golden :235-238, ACTIVE verbatim. Delegates to the on-screen numeric
// keyboard facade (forms/fQwertyKey.h) -- UI input assistance, not a
// machine/comm action; fQwertyKey stays a NULL global until a future wiring
// wave constructs it (same exposure golden itself has pre-CreateForm,
// fQwertyKey.h D-8/GATE precedent -- this wave adds no new caller of the
// handler itself, per "本體可翻，但不准去別的檔加呼叫").
void TfWinway::edtSetTempClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 1, true, dTempMax, 0.0);
}
