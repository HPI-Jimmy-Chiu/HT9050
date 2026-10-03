// ===========================================================================
//  sjson_SYSTEM_TRAY_FORM.gen.cpp -- GENERATED, DO NOT EDIT BY HAND.
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//
//  產生器：tools/gen_sjson.py
//  來源　：cprod.h:1301-1358（結構 SYSTEM_TRAY_FORM，44 個頂層成員）
//  規格　：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
//
//  重跑：python tools\gen_sjson.py
//
//  ⚠ offset 與 elemSize 都是 offsetof/sizeof —— **編譯器**算的，不是產生器。
//    產生器只知道名字。
// ===========================================================================
#include "JsonBridge/FieldDesc.h"

#include "vclcompat/vcl_compat.h"
#include "cprod.h"

namespace ht9045 {
namespace sjson {

namespace {

const FieldDesc kFields_SYSTEM_TRAY_FORM[] = {
    { "LodareType", offsetof(SYSTEM_TRAY_FORM, LodareType), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->LodareType), (std::size_t)(0), (std::size_t)(0) },
    { "Auto", offsetof(SYSTEM_TRAY_FORM, Auto), kFieldUnsupported, sizeof(((SYSTEM_TRAY_FORM*)0)->Auto[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->Auto) / sizeof(((SYSTEM_TRAY_FORM*)0)->Auto[0])), (std::size_t)(0) },
    { "Loader", offsetof(SYSTEM_TRAY_FORM, Loader), kFieldUnsupported, sizeof(((SYSTEM_TRAY_FORM*)0)->Loader), (std::size_t)(0), (std::size_t)(0) },
    { "Empty", offsetof(SYSTEM_TRAY_FORM, Empty), kFieldUnsupported, sizeof(((SYSTEM_TRAY_FORM*)0)->Empty), (std::size_t)(0), (std::size_t)(0) },
    { "Color", offsetof(SYSTEM_TRAY_FORM, Color), kFieldUnsupported, sizeof(((SYSTEM_TRAY_FORM*)0)->Color), (std::size_t)(0), (std::size_t)(0) },
    { "LoaderToEmptyColor", offsetof(SYSTEM_TRAY_FORM, LoaderToEmptyColor), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->LoaderToEmptyColor[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->LoaderToEmptyColor) / sizeof(((SYSTEM_TRAY_FORM*)0)->LoaderToEmptyColor[0])), (std::size_t)(0) },
    { "AutoFromEmptyColor", offsetof(SYSTEM_TRAY_FORM, AutoFromEmptyColor), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->AutoFromEmptyColor[0][0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->AutoFromEmptyColor) / sizeof(((SYSTEM_TRAY_FORM*)0)->AutoFromEmptyColor[0])), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->AutoFromEmptyColor[0]) / sizeof(((SYSTEM_TRAY_FORM*)0)->AutoFromEmptyColor[0][0])) },
    { "iRotateKIT_InputType", offsetof(SYSTEM_TRAY_FORM, iRotateKIT_InputType), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iRotateKIT_InputType), (std::size_t)(0), (std::size_t)(0) },
    { "iRotateKIT_OutputType", offsetof(SYSTEM_TRAY_FORM, iRotateKIT_OutputType), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iRotateKIT_OutputType), (std::size_t)(0), (std::size_t)(0) },
    { "iFixTrayMode", offsetof(SYSTEM_TRAY_FORM, iFixTrayMode), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iFixTrayMode), (std::size_t)(0), (std::size_t)(0) },
    { "iUsePickUnitCount", offsetof(SYSTEM_TRAY_FORM, iUsePickUnitCount), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iUsePickUnitCount), (std::size_t)(0), (std::size_t)(0) },
    { "bAutoFeed", offsetof(SYSTEM_TRAY_FORM, bAutoFeed), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bAutoFeed), (std::size_t)(0), (std::size_t)(0) },
    { "bColorTray", offsetof(SYSTEM_TRAY_FORM, bColorTray), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bColorTray), (std::size_t)(0), (std::size_t)(0) },
    { "bChkLoadDirection", offsetof(SYSTEM_TRAY_FORM, bChkLoadDirection), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bChkLoadDirection), (std::size_t)(0), (std::size_t)(0) },
    { "AutoCoverInitial", offsetof(SYSTEM_TRAY_FORM, AutoCoverInitial), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->AutoCoverInitial), (std::size_t)(0), (std::size_t)(0) },
    { "AutoCoverRetest", offsetof(SYSTEM_TRAY_FORM, AutoCoverRetest), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->AutoCoverRetest), (std::size_t)(0), (std::size_t)(0) },
    { "iTrayTransportMode", offsetof(SYSTEM_TRAY_FORM, iTrayTransportMode), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iTrayTransportMode), (std::size_t)(0), (std::size_t)(0) },
    { "iManualRemoveLoader", offsetof(SYSTEM_TRAY_FORM, iManualRemoveLoader), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iManualRemoveLoader), (std::size_t)(0), (std::size_t)(0) },
    { "bFailAutoTrayManual_FT", offsetof(SYSTEM_TRAY_FORM, bFailAutoTrayManual_FT), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bFailAutoTrayManual_FT), (std::size_t)(0), (std::size_t)(0) },
    { "bFailAutoTrayManual_RT", offsetof(SYSTEM_TRAY_FORM, bFailAutoTrayManual_RT), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bFailAutoTrayManual_RT), (std::size_t)(0), (std::size_t)(0) },
    { "iMagTraySource", offsetof(SYSTEM_TRAY_FORM, iMagTraySource), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iMagTraySource), (std::size_t)(0), (std::size_t)(0) },
    { "iMagFixTrayType", offsetof(SYSTEM_TRAY_FORM, iMagFixTrayType), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iMagFixTrayType), (std::size_t)(0), (std::size_t)(0) },
    { "iMagDisplayOrder", offsetof(SYSTEM_TRAY_FORM, iMagDisplayOrder), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iMagDisplayOrder), (std::size_t)(0), (std::size_t)(0) },
    { "bMoveAfterTrayGoOut", offsetof(SYSTEM_TRAY_FORM, bMoveAfterTrayGoOut), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bMoveAfterTrayGoOut), (std::size_t)(0), (std::size_t)(0) },
    { "bSpecTrayCnt", offsetof(SYSTEM_TRAY_FORM, bSpecTrayCnt), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bSpecTrayCnt), (std::size_t)(0), (std::size_t)(0) },
    { "iFullTrayCount", offsetof(SYSTEM_TRAY_FORM, iFullTrayCount), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iFullTrayCount), (std::size_t)(0), (std::size_t)(0) },
    { "iInputTrayCount", offsetof(SYSTEM_TRAY_FORM, iInputTrayCount), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iInputTrayCount), (std::size_t)(0), (std::size_t)(0) },
    { "bVTestNoRTBin", offsetof(SYSTEM_TRAY_FORM, bVTestNoRTBin), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bVTestNoRTBin), (std::size_t)(0), (std::size_t)(0) },
    { "asNoRTBinFix", offsetof(SYSTEM_TRAY_FORM, asNoRTBinFix), kFieldAnsiString, sizeof(((SYSTEM_TRAY_FORM*)0)->asNoRTBinFix[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->asNoRTBinFix) / sizeof(((SYSTEM_TRAY_FORM*)0)->asNoRTBinFix[0])), (std::size_t)(0) },
    { "asGEM_LoaderTo_FT", offsetof(SYSTEM_TRAY_FORM, asGEM_LoaderTo_FT), kFieldAnsiString, sizeof(((SYSTEM_TRAY_FORM*)0)->asGEM_LoaderTo_FT), (std::size_t)(0), (std::size_t)(0) },
    { "asGEM_LoaderTo_RT", offsetof(SYSTEM_TRAY_FORM, asGEM_LoaderTo_RT), kFieldAnsiString, sizeof(((SYSTEM_TRAY_FORM*)0)->asGEM_LoaderTo_RT), (std::size_t)(0), (std::size_t)(0) },
    { "asGEM_TrayType", offsetof(SYSTEM_TRAY_FORM, asGEM_TrayType), kFieldAnsiString, sizeof(((SYSTEM_TRAY_FORM*)0)->asGEM_TrayType), (std::size_t)(0), (std::size_t)(0) },
    { "bFixTrayLink", offsetof(SYSTEM_TRAY_FORM, bFixTrayLink), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bFixTrayLink[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->bFixTrayLink) / sizeof(((SYSTEM_TRAY_FORM*)0)->bFixTrayLink[0])), (std::size_t)(0) },
    { "bTrayUpDownSet", offsetof(SYSTEM_TRAY_FORM, bTrayUpDownSet), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bTrayUpDownSet[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->bTrayUpDownSet) / sizeof(((SYSTEM_TRAY_FORM*)0)->bTrayUpDownSet[0])), (std::size_t)(0) },
    { "bTraySortCntFunc", offsetof(SYSTEM_TRAY_FORM, bTraySortCntFunc), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bTraySortCntFunc[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->bTraySortCntFunc) / sizeof(((SYSTEM_TRAY_FORM*)0)->bTraySortCntFunc[0])), (std::size_t)(0) },
    { "iTraySortCntFunc", offsetof(SYSTEM_TRAY_FORM, iTraySortCntFunc), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iTraySortCntFunc[0][0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->iTraySortCntFunc) / sizeof(((SYSTEM_TRAY_FORM*)0)->iTraySortCntFunc[0])), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->iTraySortCntFunc[0]) / sizeof(((SYSTEM_TRAY_FORM*)0)->iTraySortCntFunc[0][0])) },
    { "iTrayOrder", offsetof(SYSTEM_TRAY_FORM, iTrayOrder), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iTrayOrder[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->iTrayOrder) / sizeof(((SYSTEM_TRAY_FORM*)0)->iTrayOrder[0])), (std::size_t)(0) },
    { "bIDTrayOrder", offsetof(SYSTEM_TRAY_FORM, bIDTrayOrder), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bIDTrayOrder[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->bIDTrayOrder) / sizeof(((SYSTEM_TRAY_FORM*)0)->bIDTrayOrder[0])), (std::size_t)(0) },
    { "bEnableAMR", offsetof(SYSTEM_TRAY_FORM, bEnableAMR), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bEnableAMR), (std::size_t)(0), (std::size_t)(0) },
    { "bEnableAMRLoader", offsetof(SYSTEM_TRAY_FORM, bEnableAMRLoader), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bEnableAMRLoader), (std::size_t)(0), (std::size_t)(0) },
    { "iReaderPos", offsetof(SYSTEM_TRAY_FORM, iReaderPos), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iReaderPos), (std::size_t)(0), (std::size_t)(0) },
    { "iUnloadTrayCount", offsetof(SYSTEM_TRAY_FORM, iUnloadTrayCount), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->iUnloadTrayCount), (std::size_t)(0), (std::size_t)(0) },
    { "bAutoTrayGoOutNeedDelay", offsetof(SYSTEM_TRAY_FORM, bAutoTrayGoOutNeedDelay), kFieldBool, sizeof(((SYSTEM_TRAY_FORM*)0)->bAutoTrayGoOutNeedDelay[0]), (std::size_t)(sizeof(((SYSTEM_TRAY_FORM*)0)->bAutoTrayGoOutNeedDelay) / sizeof(((SYSTEM_TRAY_FORM*)0)->bAutoTrayGoOutNeedDelay[0])), (std::size_t)(0) },
    { "ibAutoTrayGoOutDelayTime", offsetof(SYSTEM_TRAY_FORM, ibAutoTrayGoOutDelayTime), kFieldInt, sizeof(((SYSTEM_TRAY_FORM*)0)->ibAutoTrayGoOutDelayTime), (std::size_t)(0), (std::size_t)(0) },
};

// AI(W906-JSONBRIDGE-S6): 欄位 -> ini 鍵。寫方向要用；讀方向用不到。
// multi=true 表示 golden 有不只一條路徑讀這個欄位，產生器不替你挑。
const IniKey kIniKeys_SYSTEM_TRAY_FORM[] = {
    { "LodareType", "szDir", "Flag", "Loader Type", "0", false },
    { "LoaderToEmptyColor", "szDir", "Loader", "ToBuffer", "0", true },
    { "AutoFromEmptyColor", "szDir", "<s6TrayName[i]>", "FromBuffer", "0", true },
    { "iFixTrayMode", "szDir", "Flag", "Fix Tray Mode", "0", false },
    { "bAutoFeed", "szDir", "Flag", "Auto Feed", "false", false },
    { "bColorTray", "szDir", "Flag", "Color Tray Sensor", "false", false },
    { "bChkLoadDirection", "szDir", "Flag", "Check loader tray direction", "false", false },
    { "AutoCoverInitial", "szDir", "Flag", "Auto Cover Initial", "false", false },
    { "AutoCoverRetest", "szDir", "Flag", "Auto Cover Retest", "false", false },
    { "iManualRemoveLoader", "szDir", "Flag", "Skip Manual Remove Tray", "0", true },
    { "bFailAutoTrayManual_FT", "szDir", "Flag", "bFailAutoTrayManual_FT", "false", false },
    { "bFailAutoTrayManual_RT", "szDir", "Flag", "bFailAutoTrayManual_RT", "false", false },
    { "bMoveAfterTrayGoOut", "szDir", "Flag", "MoveAfterTrayGoOut", "false", false },
    { "bSpecTrayCnt", "szDir", "Flag", "bSpecTrayCnt", "false", false },
    { "iFullTrayCount", "szDir", "AMR", "Full Tray Count", "12", false },
    { "iInputTrayCount", "szDir", "AMR", "Input Tray Count", "12", false },
    { "bVTestNoRTBin", "szDir", "VTest", "NoRTBin", "false", false },
    { "asNoRTBinFix", "szDir", "VTest", "NoRTBinFix1", "AnsiString(\"\")", true },
    { "bTrayUpDownSet", "szDir", "Flag", "Use Fix1 Tray", "0", true },
    { "bTraySortCntFunc", "szDir", "<s6TrayName[i]>", "bTraySortCntFunc", "false", false },
    { "iTraySortCntFunc", "szDir", "<s6TrayName[i]>", "<Str2>", "0", false },
    { "iTrayOrder", "szDir", "AMR", "1st Tray Type", "0", true },
    { "bIDTrayOrder", "szDir", "AMR", "1st ID Use", "false", true },
    { "bEnableAMR", "szDir", "AMR", "Enahle AMR", "false", false },
    { "bEnableAMRLoader", "szDir", "AMR", "Enahle AMR Loader", "false", false },
    { "iReaderPos", "szDir", "AMR", "Reader Pos", "0", false },
    { "iUnloadTrayCount", "szDir", "Other", "Unload Tray Count", "10", false },
};

}  // namespace

// ⚠ 一定要 extern。C++ 裡命名空間範圍的 `const` 物件預設是**內部連結**，
//   少了 extern 這個定義在別的 TU（Bindings.cpp）看不到，會是
//   「undefined reference to kType_...」。20260923 踩過一次。
extern const TypeDesc kType_SYSTEM_TRAY_FORM;
extern const TypeDesc kType_SYSTEM_TRAY_FORM = {
    "SYSTEM_TRAY_FORM",
    kFields_SYSTEM_TRAY_FORM,
    sizeof(kFields_SYSTEM_TRAY_FORM) / sizeof(kFields_SYSTEM_TRAY_FORM[0]),
    sizeof(SYSTEM_TRAY_FORM),
    kIniKeys_SYSTEM_TRAY_FORM,
    sizeof(kIniKeys_SYSTEM_TRAY_FORM) / sizeof(kIniKeys_SYSTEM_TRAY_FORM[0])
};

}  // namespace sjson
}  // namespace ht9045
