// ===========================================================================
//  FileRW/TempSet_Ts9.cpp -- Setup.Temp_Set (golden TfTemp_Set) Q41 TS-9: the editlist.get "extra" for the air-stream clamp.
//
//  AI(W906-Q41-TS9) 20260928 (St02-E helper). St02's file, hand-written, NOT golden: golden runs the whole handler on the
//    form; the port splits it into this server half (the one value the handler reads from ANOTHER form) and the page half
//    (web/page/ht9045_temp_set_c.js, TS-9 section), which runs the clamp when the on-screen keypad closes.
//
//  golden (906_0625_Steven uTemp_Set.cpp:6903-6929; 912 uTemp_Set.cpp:7018-7044, same body):
//    TfTemp_Set::edtSetTempature2AirMachineClick = OnClick of four TEdits (906_0625_Steven uTemp_Set.dfm):
//      edt_SetIndexAirstreamTemp :4903, edt_SetAirstreamTemperatureRang_Index :4914,
//      edtSetTempature2AirMachine :5033, edt_SetAirstreamTemperatureRang_Socket :5044 -- none has a Tag line, so Tag = 0.
//    :6912  dbSetTemp = atof(fMain->edWorkTemperBase->Text.c_str());      <- the value this file sends
//    :6914  fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 20.0, -20.0);
//    :6910  edt[2] = {edt_SetIndexAirstreamTemp, edtSetTempature2AirMachine};
//    :6916  dsum = dbSetTemp + atof(edt[Tag]->Text.c_str());
//    :6917  dsum < -70            -> edt[Tag]->Text = AnsiString(-70 - dbSetTemp);
//    :6921  Tag == 0: dsum > 35   -> edt[Tag]->Text = "0";
//    :6926  else:     dsum > 230  -> edt[Tag]->Text = AnsiString(230 - dbSetTemp);
//
//  Wiring (St01 20260927 20:20, FROM_STEVEN section 4: "TS-9 = B", St02 edits the kPage extraJson cell itself):
//    FileRW/Temperature.cpp (St01's file) kPage extraJson cell (:245, same line; St01's &BeforeApply kept) and the declaration
//    on :40 (a blank line before this change); CMakeLists.txt wb_serve source list (the ChanMvTrays line, same line).
//
//  "extra" = {"fMain":{"edWorkTemperBase":{"text":"<Text>","golden":"..."}}}
//    text = the TEdit's Text exactly as golden reads it (the page does the atof, as golden does); null only when fMain or its
//    edWorkTemperBase is not built -- the page then does not clamp (same as before this change).
//  Read-only: nothing is written. The value is the one at editlist.get (page open / reload after save); golden reads it at
//    each click -- the difference is described in the page file's TS-9 header.
// ===========================================================================
#include <string>

#include "vclcompat/vcl_compat.h"
#include "forms/fMain.h"           // fMain->edWorkTemperBase (golden main.h:732)
#include "WebBridge/JsonWriter.h"

std::string W906_TempSetTs9ExtraJson();   // declared again in FileRW/Temperature.cpp :40 (St01's file; no shared header)

std::string W906_TempSetTs9ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("fMain").BeginObject();
    w.Key("edWorkTemperBase").BeginObject();
    if (fMain != NULL && fMain->edWorkTemperBase != NULL)
        w.Key("text").String(fMain->edWorkTemperBase->Text.c_str());
    else
        w.Key("text").Null();
    w.Key("golden").String("906_0625_Steven uTemp_Set.cpp:6912 atof(fMain->edWorkTemperBase->Text.c_str()) in edtSetTempature2AirMachineClick");
    w.EndObject();
    w.EndObject();
    w.EndObject();
    return w.Str();
}
