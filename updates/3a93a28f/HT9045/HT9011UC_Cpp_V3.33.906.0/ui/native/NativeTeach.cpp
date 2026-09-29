// ===========================================================================
//  ui/native/NativeTeach.cpp
//
//  AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach native window, v1 display only.  See NativeTeach.h.
//  Shape follows NativeMotorTest.cpp: NativeGrid for every table (no ListView -- no manifest, comctl32 v5 flickers),
//  a double-buffered LabelCreate summary, static texts built once (rebuilt only when the registry changes), 20 ms
//  updates that only mark changed rows, so an update where nothing changed repaints 0 cells (ctest
//  NativeTeach_Headless checks it).  The lamp decode is NativeMotorView's MotorLedOn (one decode for all windows).
//  One axis moving marks only the rows that use that motor (the per-motor row lists are built with the static texts).
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // as the other ui/native files
#endif
#include "ui/native/NativeTeach.h"
#include "ui/native/NativeMotorView.h"   // MotorLedOn / MotorLed (the golden ALed1..10 decode)
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace w906native {

namespace {

// ---------------------------------------------------------------------------
//  Key -> golden tab / group / Set-button caption.  GENERATED 20260929 (St02-E helper), do not hand-edit:
//    inputs  forms/fTeachRegistry.cpp (LIVE TechPara / TechTwoPara statements only -- comments stripped; the five
//            TechPara lines at :353-357 are inside the /* */ of :352-358, golden 906_0625_Steven uteach.cpp:619-625
//            the same) and
//            tools/dfm2rc/ir_out/uteach.dfm.ir.json (golden uteach.dfm; control names equal V912's .dfm, St02-E2).
//    rule    the row's edit (the /*ident*/ of the registry's SetEdit argument; equal to Key on every live row) ->
//            its enclosing TTabSheet captions (outer > inner), TGroupBox captions, and the caption of its Set button
//            (funButton) when no other row shares that button (16 buttons are shared, e.g. SetButton068 by 5 rows).
//    result  347 distinct keys from 260 TECH_PARA + 52 x 2 TECH_TWOPARA key uses (17 repeat); every live row has a tab.
//    script  St02-E helper scratch teach_table.py (read-only; not in the tree -- a tools/ file is outside this change).
//  Sorted by strcmp (ASCII keys) for the binary search in TeachFindKey.
// ---------------------------------------------------------------------------
const TeachKeyInfo kKeyInfo[] = {
    {"SetEditAuto1CassetteZStart", "Output Arm > Auto 1 Cassette", "", "Auto1 Cass Z Start", false},
    {"SetEditAuto1Front", "Output Arm > Auto 1 Cassette", "", "Auto1 Front", false},
    {"SetEditAuto1FrontBack", "Output Arm > Auto 1 Cassette", "", "Auto1 Front Back", false},
    {"SetEditAuto1Rear", "Output Arm > Auto 1 Cassette", "", "Auto1 Rear", false},
    {"SetEditAuto1RearBack", "Output Arm > Auto 1 Cassette", "", "Auto1 Rear Back", false},
    {"SetEditAuto2CassetteZStart", "Output Arm > Auto 2 Cassette", "", "Auto2 Cass Z Start", false},
    {"SetEditAuto2Front", "Output Arm > Auto 2 Cassette", "", "Auto2 Front", false},
    {"SetEditAuto2FrontBack", "Output Arm > Auto 2 Cassette", "", "Auto2 Front Back", false},
    {"SetEditAuto2Rear", "Output Arm > Auto 2 Cassette", "", "Auto2 Rear", false},
    {"SetEditAuto2RearBack", "Output Arm > Auto 2 Cassette", "", "Auto2 Rear Back", false},
    {"SetEditAutoClean", "Input Arm", "Pick Up", "AutoClean", false},
    {"SetEditHP", "Input Arm", "Pick Up", "Hot Plate", false},
    {"SetEditLDCassetteZStart", "Input Arm", "Loader Cassette", "Cassette Z Start", false},
    {"SetEditLDFront", "Input Arm", "Loader Cassette", "LD Front", false},
    {"SetEditLDFrontBack", "Input Arm", "Loader Cassette", "LD Front Back", false},
    {"SetEditLDRear", "Input Arm", "Loader Cassette", "LD Rear", false},
    {"SetEditLDRearBack", "Input Arm", "Loader Cassette", "LD Rear Back", false},
    {"SetEditPickInRotate", "Input Arm", "Pick Up", "Rotate", false},
    {"SetEditPickLoader", "Input Arm", "Pick Up", "Loader", false},
    {"SetEditPickOutRotate", "Output Arm", "Pick Up", "Rotate", false},
    {"SetEditPickOutSht", "Output Arm", "Pick Up", "Shuttle", false},
    {"SetEditPlaceAuto", "Output Arm", "Place", "Auto", false},
    {"SetEditPlaceFix", "Output Arm", "Place", "Fix", false},
    {"SetEditPlaceFix2", "Output Arm", "Place", "Fix2", false},
    {"SetEditPlaceInRotate", "Input Arm", "Place", "Rotate", false},
    {"SetEditPlaceInShuttle", "Input Arm", "Place", "Shuttle", false},
    {"SetEditPlaceNGBinBoxZ", "Input Arm", "Place", "NG Bin Box", false},
    {"SetEditPlaceOutRotate", "Output Arm", "Place", "Rotate", false},
    {"SetEditPlacePreciserZ", "Input Arm", "Place", "Preciser", false},
    {"edtBinBoxX", "Output Arm", "", "Bin Box", false},
    {"edtBinBoxY", "Output Arm", "", "Bin Box", false},
    {"edtBinBoxZ", "Output Arm", "Place", "Bin Box", false},
    {"edtBt2DX", "Input Arm", "", "Bottom 2DID", false},
    {"edtBt2DY", "Input Arm", "", "Bottom 2DID", false},
    {"edtEditInSht1OctSiteKit", "Kit", "", "In Shuttle1 8 site kit pos1", false},
    {"edtEditInSht2OctSiteKit", "Kit", "", "In Shuttle2 8 site kit pos1", false},
    {"edtEditMagZStandby", "Magazine", "Magazine Z Motor", "Standby Pos", false},
    {"edtEditMagZTray1", "Magazine", "Magazine Z Motor", "Magazine 1", false},
    {"edtInArmXBasePickerPos", "AOA > InArm", "In Arm CCD Alignment", "Base Pick", false},
    {"edtInArmXCCDPos", "AOA > InArm", "In Arm CCD Alignment", "CCD", false},
    {"edtInArmYBasePickerPos", "AOA > InArm", "In Arm CCD Alignment", "Base Pick", false},
    {"edtInArmYCCDPos", "AOA > InArm", "In Arm CCD Alignment", "CCD", false},
    {"edtOutArmXBasePickerPos", "AOA > OutArm", "Out Arm CCD Alignment", "Base Pick", false},
    {"edtOutArmXCCDPos", "AOA > OutArm", "Out Arm CCD Alignment", "CCD", false},
    {"edtOutArmYBasePickerPos", "AOA > OutArm", "Out Arm CCD Alignment", "Base Pick", false},
    {"edtOutArmYCCDPos", "AOA > OutArm", "Out Arm CCD Alignment", "CCD", false},
    {"edtOutSortArmAuto4_X", "Out Sort", "Sort Set / Out Sort Arm", "Auto 4", false},
    {"edtOutSortArmAuto4_Y", "Out Sort", "Sort Set / Out Sort Arm", "Auto 4", false},
    {"edtOutSortArmAuto5_X", "Out Sort", "Sort Set / Out Sort Arm", "Auto 5", false},
    {"edtOutSortArmAuto5_Y", "Out Sort", "Sort Set / Out Sort Arm", "Auto 5", false},
    {"edtOutSortArmAuto6_X", "Out Sort", "Sort Set / Out Sort Arm", "Auto 6", false},
    {"edtOutSortArmAuto6_Y", "Out Sort", "Sort Set / Out Sort Arm", "Auto 6", false},
    {"edtOutSortArmShtL_X", "Out Sort", "Sort Set / Out Sort Arm", "Sort SHT", false},
    {"edtOutSortArmShtL_Y", "Out Sort", "Sort Set / Out Sort Arm", "Sort SHT", false},
    {"edtOutSortSht_L", "Out Sort", "Sort Set / Out Sort Shuttle", "Sort Shuttle L", false},
    {"edtOutSortSht_R", "Out Sort", "Sort Set / Out Sort Shuttle", "Sort Shuttle  R", false},
    {"edtOuttArmShtR_X", "Out Sort", "Sort Set / Out Arm to Sort Shuttle", "Sort SHT", false},
    {"edtOuttArmShtR_Y", "Out Sort", "Sort Set / Out Arm to Sort Shuttle", "Sort SHT", false},
    {"edtSetEditIS1BarCode", "Kit", "", "In Shuttle 1 Bar Code Pos", false},
    {"edtSetEditIS2BarCode", "Kit", "", "In Shuttle 2 Bar Code Pos", false},
    {"edtSetEditOS1BarCode", "Kit", "", "Out Shuttle 1 Bar Code Pos", false},
    {"edtSetEditOS2BarCode", "Kit", "", "Out Shuttle 2 Bar Code Pos", false},
    {"edtSetOutArmToSortShtPlace", "Out Sort", "Sort Set / Out Arm to Sort Shuttle / Place", "Auto", false},
    {"edtSetPickSortSHT", "Out Sort", "Sort Set / Out Sort Arm / Pick Up", "Sort Shuttle", false},
    {"edtSetPlaceSortAuto", "Out Sort", "Sort Set / Out Sort Arm / Place", "Auto", false},
    {"edtSetSH1_16SiteKit", "Kit", "", "In Shuttle 1 16 site kit pos", false},
    {"edtSetSH2_16SiteKit", "Kit", "", "In Shuttle 2 16 site kit pos", false},
    {"edtSetSht1Laser", "Kit", "", "In Shuttle 1 Laser Pos", false},
    {"edtSetSht2Laser", "Kit", "", "In Shuttle 2 Laser Pos", false},
    {"edtSetSortZSafeHeight", "Out Sort", "Sort Set / Out Sort Arm", "Z Safe Height", false},
    {"edtsetSortXPitchMax", "Out Sort", "Sort Set / Out Sort Arm", "40mm", false},
    {"edtsetSortXPitchMin", "Out Sort", "Sort Set / Out Sort Arm", "13.33mm", false},
    {"setAuto1Z", "Tray Arm", "Tray Z Motor", "Auto 1", false},
    {"setAuto1ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 1", false},
    {"setAuto2Z", "Tray Arm", "Tray Z Motor", "Auto 2", false},
    {"setAuto2ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 2", false},
    {"setAuto3Z", "Tray Arm", "Tray Z Motor", "Auto 3", false},
    {"setAuto3ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 3", false},
    {"setAuto4Z", "Tray Arm", "Tray Z Motor", "Auto 4", false},
    {"setAuto4ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 4", false},
    {"setAuto5Z", "Tray Arm", "Tray Z Motor", "Auto 5", false},
    {"setAuto5ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 5", false},
    {"setAuto6Z", "Tray Arm", "Tray Z Motor", "Auto 6", false},
    {"setAuto6ZUp", "Tray Arm", "Tray Arm Z Motor", "Auto 6", false},
    {"setColorZ", "Tray Arm", "Tray Z Motor", "Color", false},
    {"setColorZUp", "Tray Arm", "Tray Arm Z Motor", "Color", false},
    {"setEdGabageX", "Shuttle > ShuttleGarbage", "Gabage Base", "In Arm X", false},
    {"setEdGabageY", "Shuttle > ShuttleGarbage", "Gabage Base", "In Arm Y", false},
    {"setEdLoadCellY1", "Index", "LoadCell Y", "", false},
    {"setEdLoadCellY2", "Index", "LoadCell Y", "", false},
    {"setEdLoadCellZ1", "Index", "LoadCell Y", "", false},
    {"setEdLoadCellZ2", "Index", "LoadCell Y", "", false},
    {"setEditAlignInZAa", "AOA > InArm", "InArm Z Base Pick Pos", "Aa", false},
    {"setEditAlignInZAb", "AOA > InArm", "InArm Z Base Pick Pos", "Ab", false},
    {"setEditAlignInZAc", "AOA > InArm", "InArm Z Base Pick Pos", "Ac", false},
    {"setEditAlignInZAd", "AOA > InArm", "InArm Z Base Pick Pos", "Ad", false},
    {"setEditAlignInZAe", "AOA > InArm", "InArm Z Base Pick Pos", "Ae", false},
    {"setEditAlignInZAf", "AOA > InArm", "InArm Z Base Pick Pos", "Af", false},
    {"setEditAlignInZAg", "AOA > InArm", "InArm Z Base Pick Pos", "Ag", false},
    {"setEditAlignInZAh", "AOA > InArm", "InArm Z Base Pick Pos", "Ah", false},
    {"setEditAlignInZBa", "AOA > InArm", "InArm Z Base Pick Pos", "Ba", false},
    {"setEditAlignInZBb", "AOA > InArm", "InArm Z Base Pick Pos", "Bb", false},
    {"setEditAlignInZBc", "AOA > InArm", "InArm Z Base Pick Pos", "Bc", false},
    {"setEditAlignInZBd", "AOA > InArm", "InArm Z Base Pick Pos", "Bd", false},
    {"setEditAlignInZBe", "AOA > InArm", "InArm Z Base Pick Pos", "Be", false},
    {"setEditAlignInZBf", "AOA > InArm", "InArm Z Base Pick Pos", "Bf", false},
    {"setEditAlignInZBg", "AOA > InArm", "InArm Z Base Pick Pos", "Bg", false},
    {"setEditAlignInZBh", "AOA > InArm", "InArm Z Base Pick Pos", "Bh", false},
    {"setEditAlignOutZAa", "AOA > OutArm", "OutArm Z Base Pick Pos", "A", false},
    {"setEditAlignOutZAb", "AOA > OutArm", "OutArm Z Base Pick Pos", "C", false},
    {"setEditAlignOutZAc", "AOA > OutArm", "OutArm Z Base Pick Pos", "E", false},
    {"setEditAlignOutZAd", "AOA > OutArm", "OutArm Z Base Pick Pos", "G", false},
    {"setEditAlignOutZAe", "AOA > OutArm", "OutArm Z Base Pick Pos", "Ae", false},
    {"setEditAlignOutZAf", "AOA > OutArm", "OutArm Z Base Pick Pos", "Af", false},
    {"setEditAlignOutZAg", "AOA > OutArm", "OutArm Z Base Pick Pos", "Ag", false},
    {"setEditAlignOutZAh", "AOA > OutArm", "OutArm Z Base Pick Pos", "Ah", false},
    {"setEditAlignOutZBa", "AOA > OutArm", "OutArm Z Base Pick Pos", "B", false},
    {"setEditAlignOutZBb", "AOA > OutArm", "OutArm Z Base Pick Pos", "D", false},
    {"setEditAlignOutZBc", "AOA > OutArm", "OutArm Z Base Pick Pos", "F", false},
    {"setEditAlignOutZBd", "AOA > OutArm", "OutArm Z Base Pick Pos", "H", false},
    {"setEditAlignOutZBe", "AOA > OutArm", "OutArm Z Base Pick Pos", "Be", false},
    {"setEditAlignOutZBf", "AOA > OutArm", "OutArm Z Base Pick Pos", "Bf", false},
    {"setEditAlignOutZBg", "AOA > OutArm", "OutArm Z Base Pick Pos", "Bg", false},
    {"setEditAlignOutZBh", "AOA > OutArm", "OutArm Z Base Pick Pos", "Bh", false},
    {"setEditAuto1X", "Output Arm", "", "Auto 1", false},
    {"setEditAuto1Y", "Output Arm", "", "Auto 1", false},
    {"setEditAuto2X", "Output Arm", "", "Auto 2", false},
    {"setEditAuto2Y", "Output Arm", "", "Auto 2", false},
    {"setEditAuto3X", "Output Arm", "", "Auto 3", false},
    {"setEditAuto3Y", "Output Arm", "", "Auto 3", false},
    {"setEditAuto4X", "Output Arm", "", "Auto 4", false},
    {"setEditAuto4Y", "Output Arm", "", "Auto 4", false},
    {"setEditAuto5X", "Output Arm", "", "Auto 5", false},
    {"setEditAuto5Y", "Output Arm", "", "Auto 5", false},
    {"setEditAuto6X", "Output Arm", "", "Auto 6", false},
    {"setEditAuto6Y", "Output Arm", "", "Auto 6", false},
    {"setEditAutoCleanX", "Input Arm", "", "Auto Clean", false},
    {"setEditAutoCleanY", "Input Arm", "", "Auto Clean", false},
    {"setEditBGAViewX", "AOI", "Output Arm X Y", "BGA View", false},
    {"setEditBGAViewY", "AOI", "Output Arm X Y", "BGA View", false},
    {"setEditBGAViewZ", "AOI", "Output Arm Z", "BGA View Z", false},
    {"setEditBuffer10X", "MR", "Cassette Arm", "Buffer10 X", false},
    {"setEditBuffer10Z", "MR", "Cassette Arm", "Buffer10 Z", false},
    {"setEditBuffer1X", "MR", "Cassette Arm", "Buffer1 X", false},
    {"setEditBuffer1Z", "MR", "Cassette Arm", "Buffer1 Z", false},
    {"setEditBuffer2X", "MR", "Cassette Arm", "Buffer2 X", false},
    {"setEditBuffer2Z", "MR", "Cassette Arm", "Buffer2 Z", false},
    {"setEditBuffer3X", "MR", "Cassette Arm", "Buffer3 X", false},
    {"setEditBuffer3Z", "MR", "Cassette Arm", "Buffer3 Z", false},
    {"setEditBuffer4X", "MR", "Cassette Arm", "Buffer4 X", false},
    {"setEditBuffer4Z", "MR", "Cassette Arm", "Buffer4 Z", false},
    {"setEditBuffer5X", "MR", "Cassette Arm", "Buffer5 X", false},
    {"setEditBuffer5Z", "MR", "Cassette Arm", "Buffer5 Z", false},
    {"setEditBuffer6X", "MR", "Cassette Arm", "Buffer6 X", false},
    {"setEditBuffer6Z", "MR", "Cassette Arm", "Buffer6 Z", false},
    {"setEditBuffer7X", "MR", "Cassette Arm", "Buffer7 X", false},
    {"setEditBuffer7Z", "MR", "Cassette Arm", "Buffer7 Z", false},
    {"setEditBuffer8X", "MR", "Cassette Arm", "Buffer8 X", false},
    {"setEditBuffer8Z", "MR", "Cassette Arm", "Buffer8 Z", false},
    {"setEditBuffer9X", "MR", "Cassette Arm", "Buffer9 X", false},
    {"setEditBuffer9Z", "MR", "Cassette Arm", "Buffer9 Z", false},
    {"setEditCatchMagFront", "Magazine", "MotCatchMagTray", "Catch Mag Front", false},
    {"setEditCatchMagRear", "Magazine", "MotCatchMagTray", "Catch Mag Rear", false},
    {"setEditFix1X", "Output Arm", "", "Fix 1", false},
    {"setEditFix1Y", "Output Arm", "", "Fix 1", false},
    {"setEditFix2X", "Output Arm", "", "Fix 2", false},
    {"setEditFix2Y", "Output Arm", "", "Fix 2", false},
    {"setEditFix3X", "Output Arm", "", "Fix 3", false},
    {"setEditFix3Y", "Output Arm", "", "Fix 3", false},
    {"setEditFix4X", "Output Arm", "", "Fix 4", false},
    {"setEditFix4Y", "Output Arm", "", "Fix 4", false},
    {"setEditFix5X", "Output Arm", "", "Fix 5", false},
    {"setEditFix5Y", "Output Arm", "", "Fix 5", false},
    {"setEditFix6X", "Output Arm", "", "Fix 6", false},
    {"setEditFix6Y", "Output Arm", "", "Fix 6", false},
    {"setEditHP1X", "Input Arm", "", "Hot Plate 1", false},
    {"setEditHP1Y", "Input Arm", "", "Hot Plate 1", false},
    {"setEditHP2X", "Input Arm", "", "Hot Plate 2", false},
    {"setEditHP2Y", "Input Arm", "", "Hot Plate 2", false},
    {"setEditHingeRotateEmpty", "Hinge", "R rotate Empty", "Rotate", false},
    {"setEditHingeRotateLoader", "Hinge", "R rotate Loader", "Rotate", false},
    {"setEditInRA", "Rotate", "In Rotate", "RA", false},
    {"setEditInRB", "Rotate", "In Rotate", "RB", false},
    {"setEditInRC", "Rotate", "In Rotate", "RC", false},
    {"setEditInRD", "Rotate", "In Rotate", "RD", false},
    {"setEditInRE", "Rotate", "In Rotate", "RE", false},
    {"setEditInRF", "Rotate", "In Rotate", "RF", false},
    {"setEditInRG", "Rotate", "In Rotate", "RG", false},
    {"setEditInRH", "Rotate", "In Rotate", "RH", false},
    {"setEditInSh1LtcSenZ1", "Kit", "In Shuttle Sensor Detect", "Z1", false},
    {"setEditInSh1LtcSenZ2", "Kit", "In Shuttle Sensor Detect", "Z2", false},
    {"setEditInSh2LtcSenZ1", "Kit", "In Shuttle Sensor Detect", "Z1", false},
    {"setEditInSh2LtcSenZ2", "Kit", "In Shuttle Sensor Detect", "Z2", false},
    {"setEditInSht1Left", "Shuttle > Shuttle Pos", "", "Shuttle1 L", false},
    {"setEditInSht1Right", "Shuttle > Shuttle Pos", "", "Shuttle1 R", false},
    {"setEditInSht1X", "Input Arm", "", "Shuttle 1", false},
    {"setEditInSht1Y", "Input Arm", "", "Shuttle 1", false},
    {"setEditInSht2Left", "Shuttle > Shuttle Pos", "", "Shuttle2 L", false},
    {"setEditInSht2Right", "Shuttle > Shuttle Pos", "", "Shuttle2 R", false},
    {"setEditInSht2X", "Input Arm", "", "Shuttle 2", false},
    {"setEditInSht2Y", "Input Arm", "", "Shuttle 2", false},
    {"setEditInX2120", "Hand Pitch", "", "", false},
    {"setEditInX240", "Hand Pitch", "", "", false},
    {"setEditInX3120", "Hand Pitch", "", "120mm", false},
    {"setEditInX340", "Hand Pitch", "", "40mm", false},
    {"setEditInX4120", "Hand Pitch", "", "120mm", false},
    {"setEditInX440", "Hand Pitch", "", "40mm", false},
    {"setEditInXPitch120", "Hand Pitch", "", "", false},
    {"setEditInXPitch40", "Hand Pitch", "", "", false},
    {"setEditInY15", "Hand Pitch", "", "", false},
    {"setEditInY60", "Hand Pitch", "", "", false},
    {"setEditInZSafeHeight", "Input Arm", "", "Z Safe Height", false},
    {"setEditInarmPlacementX", "Placement", "", "Placement", false},
    {"setEditInarmPlacementXOffsetByBasicSuck", "Placement", "InPlaceSuck Offset By Basic Suck", "ComputeInSh2", false},
    {"setEditInarmPlacementY", "Placement", "", "Placement", false},
    {"setEditInarmPlacementYOffsetByBasicSuck", "Placement", "InPlaceSuck Offset By Basic Suck", "ComputeInSh2", false},
    {"setEditIndex1ToSht1Y", "Index", "", "", false},
    {"setEditIndex1ToSht1Z", "Index", "", "Arm1 Z", false},
    {"setEditIndex1ToSocketY", "Index", "", "", false},
    {"setEditIndex2ToSht2Y", "Index", "", "", false},
    {"setEditIndex2ToSht2Z", "Index", "", "Arm2 Z", false},
    {"setEditIndex2ToSocketY", "Index", "", "", false},
    {"setEditLoadPortZ", "MR", "Load Port", "Load Port", false},
    {"setEditLoadSafeZ", "MR", "Load Port", "Safe", false},
    {"setEditLoadTemporaryZ", "MR", "Load Port", "Temporary", false},
    {"setEditLoadXGabage", "Shuttle > ShuttleGarbage", "Load Base", "In Arm X", false},
    {"setEditLoadYGabage", "Shuttle > ShuttleGarbage", "Load Base", "In Arm Y", false},
    {"setEditLoaderX", "Input Arm", "", "Loader", false},
    {"setEditLoaderY", "Input Arm", "", "Loader", false},
    {"setEditNGBinBoxX", "Input Arm", "", "NG Bin Box", false},
    {"setEditNGBinBoxY", "Input Arm", "", "NG Bin Box", false},
    {"setEditOutRA", "Rotate", "Out Rotate", "RA", false},
    {"setEditOutRB", "Rotate", "Out Rotate", "RB", false},
    {"setEditOutRC", "Rotate", "Out Rotate", "RC", false},
    {"setEditOutRD", "Rotate", "Out Rotate", "RD", false},
    {"setEditOutRE", "Rotate", "Out Rotate", "RE", false},
    {"setEditOutRF", "Rotate", "Out Rotate", "RF", false},
    {"setEditOutRG", "Rotate", "Out Rotate", "RG", false},
    {"setEditOutRH", "Rotate", "Out Rotate", "RH", false},
    {"setEditOutSht1KitPos", "Kit", "", "OutShuttle1 8 site kit pos1", false},
    {"setEditOutSht1OneRowKit", "Kit", "", "OutShuttle1 One Row kit pos1", false},
    {"setEditOutSht1X", "Output Arm", "", "Shuttle 1", false},
    {"setEditOutSht1Y", "Output Arm", "", "Shuttle 1", false},
    {"setEditOutSht2KitPos", "Kit", "", "OutShuttle2 8 site kit pos1", false},
    {"setEditOutSht2OneRowKit", "Kit", "", "OutShuttle2 One Row kit pos1", false},
    {"setEditOutSht2X", "Output Arm", "", "Shuttle 2", false},
    {"setEditOutSht2Y", "Output Arm", "", "Shuttle 2", false},
    {"setEditOutX2120", "Hand Pitch", "", "", false},
    {"setEditOutX240", "Hand Pitch", "", "", false},
    {"setEditOutX3120", "Hand Pitch", "", "120mm", false},
    {"setEditOutX340", "Hand Pitch", "", "40mm", false},
    {"setEditOutX4120", "Hand Pitch", "", "120mm", false},
    {"setEditOutX440", "Hand Pitch", "", "40mm", false},
    {"setEditOutXPitch120", "Hand Pitch", "", "", false},
    {"setEditOutXPitch40", "Hand Pitch", "", "", false},
    {"setEditOutY15", "Hand Pitch", "", "", false},
    {"setEditOutY60", "Hand Pitch", "", "", false},
    {"setEditOutZSafeHeight", "Output Arm", "", "Z Safe Height", false},
    {"setEditPADViewX", "AOI", "Output Arm X Y", "Pad View", false},
    {"setEditPADViewY", "AOI", "Output Arm X Y", "Pad View", false},
    {"setEditPADViewZ", "AOI", "Output Arm Z", "Pad View Z", false},
    {"setEditPreciserPitchClose", "Kit", "Preciser  Pitch  2.44mm", "Preciser", false},
    {"setEditPreciserPitchOpen", "Kit", "Preciser  Pitch  2.44mm", "Preciser", false},
    {"setEditPreciserX", "Input Arm", "", "Preciser", false},
    {"setEditPreciserY", "Input Arm", "", "Preciser", false},
    {"setEditRotateA", "Kit", "Rotate Kit / In Rotate", "In Rotate", false},
    {"setEditRotateOutA", "Kit", "Rotate Kit / Out Rotate", "Out Rotate", false},
    {"setEditRotateOutX", "Output Arm", "", "Rotate Kit", false},
    {"setEditRotateOutY", "Output Arm", "", "Rotate Kit", false},
    {"setEditRotateX", "Input Arm", "", "Rotate Kit", false},
    {"setEditRotateY", "Input Arm", "", "Rotate Kit", false},
    {"setEditSafePosX", "AOI", "Output Arm X Y", "Safe Pos", false},
    {"setEditSafePosY", "AOI", "Output Arm X Y", "Safe Pos", false},
    {"setEditScannerAOIX", "AOI", "Output Arm X Y", "Scanner AOI", false},
    {"setEditScannerAOIY", "AOI", "Output Arm X Y", "Scanner AOI", false},
    {"setEditScannerAOIZ", "AOI", "Output Arm Z", "Scanner AOI Z", false},
    {"setEditSht1Pitch120", "Shuttle > Shuttle Sensor", "Shuttle1 Pitch 120mm", "120mm", false},
    {"setEditSht1Pitch180", "Shuttle > Shuttle Sensor", "Shuttle1 Pitch 180mm", "180mm", false},
    {"setEditSht1XGabage", "Shuttle > ShuttleGarbage", "Shuttle 1 Base", "In Arm X", false},
    {"setEditSht1YGabage", "Shuttle > ShuttleGarbage", "Shuttle 1 Base", "In Arm Y", false},
    {"setEditSht2Pitch120", "Shuttle > Shuttle Sensor", "Shuttle2 Pitch 120mm", "120mm", false},
    {"setEditSht2Pitch180", "Shuttle > Shuttle Sensor", "Shuttle2 Pitch 180mm", "180mm", false},
    {"setEditSht2XGabage", "Shuttle > ShuttleGarbage", "Shuttle 2 Base", "In Arm X", false},
    {"setEditSht2YGabage", "Shuttle > ShuttleGarbage", "Shuttle 2 Base", "In Arm Y", false},
    {"setEditStackedAuto1X", "MR", "Stacked Tray", "Auto1 X", false},
    {"setEditStackedAuto1Z", "MR", "Stacked Tray", "Auto1 Z", false},
    {"setEditStackedAuto2X", "MR", "Stacked Tray", "Auto2 X", false},
    {"setEditStackedAuto2Z", "MR", "Stacked Tray", "Auto2 Z", false},
    {"setEditStackedAuto3X", "MR", "Stacked Tray", "Auto3 X", false},
    {"setEditStackedAuto3Z", "MR", "Stacked Tray", "Auto3 Z", false},
    {"setEditStackedConversionX", "MR", "Stacked Tray", "Conversion X", false},
    {"setEditStackedConversionZ", "MR", "Stacked Tray", "Conversion Z", false},
    {"setEditStackedEmptyX", "MR", "Stacked Tray", "Empty X", false},
    {"setEditStackedEmptyZ", "MR", "Stacked Tray", "Empty Z", false},
    {"setEditStackedLoaderX", "MR", "Stacked Tray", "Loader X", false},
    {"setEditStackedLoaderZ", "MR", "Stacked Tray", "Loader Z", false},
    {"setEditTestZSafePos", "Index", "", "", false},
    {"setEditTopViewKitZ", "AOI", "Output Arm Z", "AOI WD (kit Down)", true},
    {"setEditTopViewKitZup", "AOI", "Output Arm Z Up", "Kit UP", true},
    {"setEditTopViewX", "AOI", "Output Arm X Y", "Top View", true},
    {"setEditTopViewY", "AOI", "Output Arm X Y", "Top View", true},
    {"setEditTopView_Pick", "AOI", "Output Arm Z Up", "Top View Pick", true},
    {"setEditTopView_Place", "AOI", "Output Arm Z", "Top View Place", true},
    {"setEditTrayAuto1X", "Tray Arm", "", "Auto 1", false},
    {"setEditTrayAuto2X", "Tray Arm", "", "Auto 2", false},
    {"setEditTrayAuto3X", "Tray Arm", "", "Auto 3", false},
    {"setEditTrayBracketConversionZ", "MR", "Tray Bracket", "Conversion", false},
    {"setEditTrayBracketSaftZ", "MR", "Tray Bracket", "Saft", false},
    {"setEditTrayCleanX", "Tray Arm", "", "Clean", false},
    {"setEditTrayColorX", "Tray Arm", "", "Color", false},
    {"setEditTrayEmptyX", "Tray Arm", "", "Empty", false},
    {"setEditTrayIDX", "Tray Arm", "Tray Mapping Function", "ID", false},
    {"setEditTrayLoaderX", "Tray Arm", "", "Loader", false},
    {"setEditTrayMapX", "Tray Arm", "Tray Mapping Function", "Mapping", false},
    {"setEditTrayOCRX", "Tray Arm", "", "OCR", false},
    {"setEditUnloadPort1Z", "MR", "Unload Robot", "Port1 Z", false},
    {"setEditUnloadPort2Z", "MR", "Unload Robot", "Port2 Z", false},
    {"setEditUnloadPort3Z", "MR", "Unload Robot", "Port3 Z", false},
    {"setEditUnloadPort4Z", "MR", "Unload Robot", "Port4 Z", false},
    {"setEditUnloadPortBufferZ", "MR", "Unload Robot", "Buffer10 Z", false},
    {"setEditWaitTestZDown", "Index", "", "", false},
    {"setEdtAuto4", "Tray Arm", "", "Auto 4", false},
    {"setEdtAuto5", "Tray Arm", "", "Auto 5", false},
    {"setEdtAuto6", "Tray Arm", "", "Auto 6", false},
    {"setEdtHP1LaserX", "Input Arm", "", "HP 1 Laser", false},
    {"setEdtHP1LaserY", "Input Arm", "", "HP 1 Laser", false},
    {"setEdtHP2LaserX", "Input Arm", "", "HP 2 Laser", false},
    {"setEdtHP2LaserY", "Input Arm", "", "HP 2 Laser", false},
    {"setEdtINDecayX", "Input Arm", "", "Decay", false},
    {"setEdtINDecayY", "Input Arm", "", "Decay", false},
    {"setEdtOUTDecayX", "Output Arm", "", "Decay", false},
    {"setEdtOUTDecayY", "Output Arm", "", "Decay", false},
    {"setEmptyZ", "Tray Arm", "Tray Z Motor", "Empty", false},
    {"setEmptyZUp", "Tray Arm", "Tray Arm Z Motor", "Empty", false},
    {"setFix3L", "Output Arm", "Fix 3 X", "Left", false},
    {"setFix3R", "Output Arm", "Fix 3 X", "Right", false},
    {"setInPickX", "Input Arm", "", "Picker Teach", true},
    {"setInPickY", "Input Arm", "", "Picker Teach", true},
    {"setLoaderZ", "Tray Arm", "Tray Z Motor", "Loader", false},
    {"setLoaderZUp", "Tray Arm", "Tray Arm Z Motor", "Loader", false},
    {"setOutPickX", "Output Arm", "", "Picker Teach", true},
    {"setOutPickY", "Output Arm", "", "Picker Teach", true},
    {"setYCarPos", "Tray Arm", "OCR Y Use Step Motor", "Car", false},
    {"setYOCRPos", "Tray Arm", "OCR Y Use Step Motor", "OCR", false},
    {"setYSurePos", "Tray Arm", "OCR Y Use Step Motor", "Sure", false},
    {"seteditContactZ1Relative", "Index", "Contact Z relatively", "Arm1 Z", false},
    {"seteditContactZ2Relative", "Index", "Contact Z relatively", "Arm2 Z", false},
};
const int kKeyInfoCount = (int)(sizeof(kKeyInfo) / sizeof(kKeyInfo[0]));

// golden PageControl2 tab order (uteach.dfm; dfm2rc IR fTeach.PageControl2 child_paths) -- the filter combo's order
const char* const kTabOrder[] = {"Index", "Hand Pitch", "Shuttle", "Kit", "Input Arm", "Output Arm", "Out Sort", "Tray Arm",
                                 "Axle Control", "AOI", "Rotate", "Hinge", "MR", "TTLTest", "AOA", "Placement", "Magazine"};
const int kTabOrderCount = (int)(sizeof(kTabOrder) / sizeof(kTabOrder[0]));

const wchar_t kClassName[] = L"W906NativeTeach";
const int kIdcTitle = 5001, kIdcNote = 5002, kIdcSummary = 5003, kIdcList = 5004, kIdcLamps = 5005, kIdcDetail = 5006,
          kIdcExit = 5007, kIdcFilterLabel = 5008, kIdcFilter = 5009, kIdcCmdLabel = 5010, kIdcCmd0 = 5100;

// golden pnlMotion labels over ALed1..10 (uteach.dfm Label9 / 32 / 33 / 35..40 / 54), in MotorLed order
const wchar_t* const kLampNames[kLedCount] = {L"CW", L"HOME", L"CCW", L"EMG Stop", L"Alarm", L"Soft CW", L"Soft CCW",
                                             L"Servo Alarm", L"InPos", L"Servo On"};
const int kLampCols = 5;

// golden's motion-panel / Panel4 commands (uteach.dfm pnlMotion + Panel4 captions; inventory §2 "Commands"):
// shown, WS_DISABLED, and WndProc has no branch for them
const wchar_t* const kCmdNames[] = {
    L"Move +", L"Move -", L"HOME", L"SET TO（Now）", L"SET TO（Offset）", L"Move（Move To）", L"JOG N", L"JOG P",
    L"Servo", L"STOP", L"Motor Tools", L"IO Check", L"SAVE"};
const int kCmdCount = (int)(sizeof(kCmdNames) / sizeof(kCmdNames[0]));
const int kCmdPerRow = 7;

const wchar_t* const kItemNames[kTdRowCount] = {
    L"教導點（選的列）", L"頁籤", L"群組｜Set 鈕", L"教導值（*Parameter）", L"teach.ini（golden ReadFromFile 讀的）",
    L"Active Motor（Panel2）", L"軸（MOT[] Alias）", L"Now Position（edtNowPosition）", L"Encoder（pnlEncoderPos）",
    L"MotorType", L"Reset Offset（edtSetToOffset）", L"Servo", L"Alarm", L"Busy", L"InPos", L"HomeFlag",
    L"第二軸 Now Position（T 列）", L"Speed Adjust（edtSpeed）", L"Move To（edtMoveTo）", L"Lock（labLock）",
    L"網頁／C++ 選的軸（ActiveMotorIndex）", L"品質", L"來源", L"說明"};

const COLORREF kGray = RGB(128, 128, 128);
const COLORREF kRed = RGB(210, 0, 0);

// per point: the wide texts, built once per registry
struct PointText {
    std::wstring no, tab, group, label, key[2], alias[2], ini;
    std::string  top;            // top-level tab (UTF-8), "" = none
};

struct AxisText {
    std::wstring alias, numberAlias;
};

struct ViewState {
    HWND hwnd, title, note, summary, list, lamps, detail, exitBtn, filterLabel, filter, cmdLabel;
    std::vector<HWND>              cmd;
    std::vector<TeachPoint>        points;
    std::vector<TeachAxis>         axes;
    std::vector<PointText>         text;
    std::vector<AxisText>          axisText;
    std::vector<int>               pointAxis;    // 2 per point: index into axes, -1 = none
    std::vector<int>               axisOfMot;    // MOT[] index -> index into axes, -1 = none
    std::vector<std::vector<int> > axisRefs;     // per axis: the points that use it (either slot)
    std::vector<int>               disp;         // display row -> point
    std::vector<int>               dispOf;       // point -> display row, -1 = filtered out
    std::vector<std::string>       filterTabs;   // combo item k (k >= 1) -> filterTabs[k - 1] ("" = the rows with no tab)
    int                            filterItem;   // 0 = all
    int                            nNoTab;
    TeachPage                      page;
    TeachSummary                   sum;
    int                            selPoint;     // index into points; -1 = none (sticky across a filter change)
    int                            lastGridSel;  // the grid's own selection as last seen
    int                            panelMot;     // the axis the motion panel shows (MOT[] index); -1 = none
    double                         lastUpdateMs, lastT0, avgGapMs;
    ViewState() : hwnd(0), title(0), note(0), summary(0), list(0), lamps(0), detail(0), exitBtn(0), filterLabel(0), filter(0),
                  cmdLabel(0), filterItem(0), nNoTab(0), selPoint(-1), lastGridSel(-1), panelMot(-1), lastUpdateMs(0),
                  lastT0(0), avgGapMs(0) {}
};

ViewState g;

double NowMs()
{
    static LARGE_INTEGER freq;
    static bool haveFreq = false;
    if (!haveFreq) { ::QueryPerformanceFrequency(&freq); haveFreq = true; }
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return freq.QuadPart ? (double)c.QuadPart * 1000.0 / (double)freq.QuadPart : 0.0;
}

void SetNum(std::wstring& out, bool has, int v)
{
    if (!has) out = L"—";
    else GridSetInt(out, v);
}

void SetTri(std::wstring& out, int v, const wchar_t* on, const wchar_t* off)
{
    out = v < 0 ? L"—" : (v ? on : off);
}

std::wstring Ascii(const char* s)
{
    std::wstring w;
    GridSetAscii(w, s ? std::string(s) : std::string());
    return w;
}

const TeachAxis* AxisOfMot(int mi)
{
    if (mi < 0 || mi >= (int)g.axisOfMot.size()) return 0;
    const int a = g.axisOfMot[(std::size_t)mi];
    return a >= 0 ? &g.axes[(std::size_t)a] : 0;
}

const AxisText* AxisTextOfMot(int mi)
{
    if (mi < 0 || mi >= (int)g.axisOfMot.size()) return 0;
    const int a = g.axisOfMot[(std::size_t)mi];
    return a >= 0 ? &g.axisText[(std::size_t)a] : 0;
}

const TeachPoint* SelPoint()
{
    return (g.selPoint >= 0 && g.selPoint < (int)g.points.size()) ? &g.points[(std::size_t)g.selPoint] : 0;
}

// the panel's axis: the selected row's (first) axis; before any click, C++'s ActiveMotorIndex (the web page's pick)
int PanelMotor()
{
    const TeachPoint* p = SelPoint();
    if (p) return p->motIndex[0];
    if (g.page.known && g.page.activeMotor >= 0) return g.page.activeMotor;
    return -1;
}

int PanelMotor2()
{
    const TeachPoint* p = SelPoint();
    return (p && p->two) ? p->motIndex[1] : -1;
}

MotorRow LampRow(const TeachAxis& a)
{
    MotorRow r;
    r.ledKnown = a.ledKnown;
    r.motionIO = a.motionIO;
    r.state = a.state;
    return r;
}

// the teach.ini [section] Key that golden ReadFromFile reads (forms/fTeachPara.cpp:96-138 TECH_PARA,
// :250-256 TECH_TWOPARA).  v1 does not open teach.ini: for the four shuttle aliases it names both spellings.
std::wstring IniOf(const std::string& alias, const std::string& key, bool twoPara)
{
    if (alias.empty()) return twoPara ? L"（Alias 空：golden 不讀這一軸）" : L"（Alias 空：golden 不讀，值設 0）";
    const std::wstring k = Widen(key);
    if (!twoPara) {
        // forms/fTeachPara.cpp:98-125 = golden uteach.cpp:77-116 (Steven 20250926): the alias's own spelling if teach.ini has that section, else the other one
        static const char* const kPair[4][2] = {{"MInShutte1", "MInShuttle1"}, {"MInShuttle1", "MInShutte1"},
                                                {"MInShutte2", "MInShuttle2"}, {"MInShuttle2", "MInShutte2"}};
        for (int i = 0; i < 4; ++i)
            if (alias == kPair[i][0])
                return L"[" + Ascii(kPair[i][0]) + L"，沒有就 " + Ascii(kPair[i][1]) + L"] " + k;
    }
    return L"[" + Widen(alias) + L"] " + k;
}

// ---- grid callbacks (asked only for rows marked changed, or on a full render) ----
void PosText(std::wstring& out, int p, int k)
{
    const int a = g.pointAxis[(std::size_t)(2 * p + k)];
    if (a < 0) { out = L"—"; return; }
    const TeachAxis& ax = g.axes[(std::size_t)a];
    SetNum(out, ax.hasNow, ax.now);
}

void ListCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= (int)g.disp.size()) return;
    const int p = g.disp[(std::size_t)row];
    const TeachPoint& t = g.points[(std::size_t)p];
    const PointText& s = g.text[(std::size_t)p];
    if (t.goldenHidden) out.color = kGray;   // AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 2): golden hides this row at construction
    if (p == g.selPoint) out.color = kRed;   // the row the panel follows (kept when a filter change clears the grid's own highlight)
    switch (col) {
    case kTcColNo:     out.text = s.no; break;
    case kTcColTab:    if (t.tab.empty()) { out.text = L"（沒有對到頁籤）"; out.color = kGray; } else out.text = s.tab; break;
    case kTcColGroup:  out.text = s.group; break;
    case kTcColLabel:  out.text = s.label; break;
    case kTcColKey:    out.text = s.key[0]; break;
    case kTcColMotor:  out.text = s.alias[0]; break;
    case kTcColValue:  SetNum(out.text, t.hasValue[0], t.value[0]); break;
    case kTcColPos:    PosText(out.text, p, 0); break;
    case kTcColSet:    out.kind = kGridButtonOff; out.text = L"Set"; break;
    case kTcColGo:     out.kind = kGridButtonOff; out.text = L"Go"; break;
    case kTcColKey2:   if (t.two) out.text = s.key[1]; break;
    case kTcColMotor2: if (t.two) out.text = s.alias[1]; break;
    case kTcColValue2: if (t.two) SetNum(out.text, t.hasValue[1], t.value[1]); break;
    case kTcColPos2:   if (t.two) PosText(out.text, p, 1); break;
    case kTcColIni:    out.text = s.ini; break;
    default: break;
    }
}

void LampCell(void*, int row, int col, GridCell& out)
{
    const int led = row * kLampCols + col;
    if (row < 0 || col < 0 || col >= kLampCols || led >= kLedCount) return;
    const TeachAxis* a = AxisOfMot(g.panelMot);
    out.kind = kGridLedText;
    out.ledEdge = RGB(40, 40, 40);
    out.text = kLampNames[led];
    if (!a || !a->ledKnown) { out.ledFill = RGB(255, 255, 255); out.ledEdge = RGB(150, 150, 150); out.color = kGray; }   // null: white, not "off"
    else if (MotorLedOn(LampRow(*a), led)) out.ledFill = (led == kLedAlarm || led == kLedEmg) ? RGB(255, 60, 40) : RGB(0, 210, 60);
    else out.ledFill = RGB(60, 70, 60);
}

void DetailValue(int row, std::wstring& out)
{
    const TeachPoint* P = SelPoint();
    const PointText* T = P ? &g.text[(std::size_t)g.selPoint] : 0;
    const int pm = g.panelMot;
    const TeachAxis* a = AxisOfMot(pm);
    const AxisText* at = AxisTextOfMot(pm);
    switch (row) {
    case kTdRowPoint:
        if (!P) out = (g.page.known && g.page.activeMotor >= 0) ? L"（沒有點選列：面板跟著 C++ 的 ActiveMotorIndex）" : L"（沒有選教導點）";
        else out = T->no + L"  " + T->key[0] + (P->two ? L" ／ " + T->key[1] : std::wstring());
        break;
    case kTdRowTab:   out = !P ? L"—" : (P->tab.empty() ? L"（沒有對到頁籤）" : T->tab); break;
    case kTdRowLabel:
        if (!P) out = L"—";
        else {
            out = T->group;
            if (!T->group.empty() && !T->label.empty()) out += L"｜";
            out += T->label;
            if (out.empty()) out = L"（無）";
        }
        break;
    case kTdRowValue:
        if (!P) { out = L"—"; break; }
        SetNum(out, P->hasValue[0], P->value[0]);
        if (P->two) {
            std::wstring b;
            SetNum(b, P->hasValue[1], P->value[1]);
            out += L" ／ " + b;
        }
        break;
    case kTdRowIni:   out = P ? T->ini : L"—"; break;
    case kTdRowPanel2:
        // golden V912 UpdateMotorTeachMonitor :1295 Panel2 = NumberAlias; a two-axis row: UpdateMotorTeachTwoMonitor :1313 "X Y"
        if (pm < 0) { out = L"—"; break; }
        out = at ? at->numberAlias : L"—";
        if (P && P->two) {
            const AxisText* b = AxisTextOfMot(P->motIndex[1]);
            out += L" " + (b ? b->numberAlias : std::wstring(L"—"));
        }
        break;
    case kTdRowAxis:
        if (pm < 0) { out = L"—"; break; }
        GridSetInt(out, pm);
        out = L"MOT[" + out + L"] " + (at ? at->alias : std::wstring(L"（沒有教導點用這一軸：v1 沒有取值）"));
        break;
    case kTdRowNow:        if (!a) out = L"—"; else SetNum(out, a->hasNow, a->now); break;
    case kTdRowEnc:        if (!a) out = L"—"; else SetNum(out, a->hasEnc, a->enc); break;
    case kTdRowMotorType:
        if (!a || a->motorType < 0) { out = L"—"; break; }
        GridSetInt(out, a->motorType);
        out += (a->motorType == 1 || a->motorType == 3) ? L"（golden 編碼器欄讀 ReadEncoderPos）" : L"（golden 編碼器欄讀 ReadPos）";
        break;
    case kTdRowSetToOffset:   // golden edtSetToOffset: empty until a home from teach fills it (AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 3)
        if (!g.page.known) out = L"—";
        else if (g.page.setToOffset.empty()) out = L"（空白：golden 只有 teach 歸零完成後才填）";
        else out = Widen(g.page.setToOffset);
        break;
    case kTdRowServo:      if (!a) out = L"—"; else SetTri(out, a->servoOn, L"ON", L"OFF"); break;
    case kTdRowAlarm:      if (!a) out = L"—"; else SetTri(out, a->alarm, L"警報", L"正常"); break;
    case kTdRowBusy:       if (!a) out = L"—"; else SetTri(out, a->busy, L"忙", L"閒"); break;
    case kTdRowInPos:      if (!a) out = L"—"; else SetTri(out, a->inPos, L"到位", L"未到"); break;
    case kTdRowHomeFlag:
        if (!a || a->homeFlag < 0) { out = L"—"; break; }
        if (a->homeFlag == 0) out = L"0 未歸零";
        else if (a->homeFlag == 1) out = L"1 完成";
        else if (a->homeFlag == 2) out = L"2 失敗";
        else GridSetInt(out, a->homeFlag);
        break;
    case kTdRowNow2: {
        if (!P) { out = L"—"; break; }
        if (!P->two) { out = L"（單軸教導點）"; break; }
        const TeachAxis* b = AxisOfMot(P->motIndex[1]);
        if (!b) out = L"—"; else SetNum(out, b->hasNow, b->now);
        break;
    }
    case kTdRowSpeed:  out = L"—（v1 不調速：golden 選軸時的 SetSpeed(1) 不做）"; break;
    case kTdRowMoveTo: out = L"—（v1 不移動）"; break;
    case kTdRowLock:   out = L"—（頁面狀態沒有取得函式）"; break;
    case kTdRowActiveWeb:
        if (!g.page.known) out = L"—";
        else if (g.page.activeMotor < 0) out = L"（沒有）";
        else {
            GridSetInt(out, g.page.activeMotor);
            const AxisText* w = AxisTextOfMot(g.page.activeMotor);
            out = L"MOT[" + out + L"]" + (w ? L" " + w->alias : std::wstring());
        }
        break;
    case kTdRowQuality: out = a ? Widen(a->quality) : L"—"; break;
    case kTdRowSource:  out = a ? Widen(a->source) : L"—"; break;
    case kTdRowWhy:
        out = a ? Widen(a->errText) : std::wstring();
        if (!g.sum.registryWhy.empty()) out += (out.empty() ? L"" : L"；") + Widen(g.sum.registryWhy);
        break;
    default: break;
    }
}

void DetailCell(void*, int row, int col, GridCell& out)
{
    if (row < 0 || row >= kTdRowCount) return;
    if (col == 0) { out.text = kItemNames[row]; out.color = RGB(60, 60, 60); return; }
    if (col == 1) DetailValue(row, out.text);
}

HWND MakeChild(const wchar_t* cls, const wchar_t* text, DWORD style, int id, DWORD exStyle = 0)
{
    HWND h = ::CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
                               g.hwnd, (HMENU)(INT_PTR)id, ::GetModuleHandleW(0), 0);
    if (h) ::SendMessageW(h, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    return h;
}

// ---- the static half: texts, per-motor row lists, the filter ----
std::string TopTab(const std::string& tab)
{
    const std::size_t k = tab.find(" > ");
    return k == std::string::npos ? tab : tab.substr(0, k);
}

void BuildStatic()
{
    const std::size_t n = g.points.size();
    g.text.assign(n, PointText());
    g.pointAxis.assign(2 * n, -1);
    int maxMot = -1;
    for (std::size_t a = 0; a < g.axes.size(); ++a)
        if (g.axes[a].motIndex > maxMot) maxMot = g.axes[a].motIndex;
    g.axisOfMot.assign((std::size_t)(maxMot + 1), -1);
    g.axisText.assign(g.axes.size(), AxisText());
    for (std::size_t a = 0; a < g.axes.size(); ++a) {
        const int mi = g.axes[a].motIndex;
        if (mi >= 0 && g.axisOfMot[(std::size_t)mi] < 0) g.axisOfMot[(std::size_t)mi] = (int)a;
        g.axisText[a].alias = Widen(g.axes[a].alias);
        g.axisText[a].numberAlias = Widen(g.axes[a].numberAlias);
    }
    g.axisRefs.assign(g.axes.size(), std::vector<int>());
    g.nNoTab = 0;
    for (std::size_t p = 0; p < n; ++p) {
        const TeachPoint& t = g.points[p];
        PointText& s = g.text[p];
        char no[24];
        std::snprintf(no, sizeof(no), "%c%d", t.two ? 'T' : 'P', t.index);
        GridSetAscii(s.no, no);
        s.tab = Widen(t.tab);
        s.group = Widen(t.group);
        s.label = Widen(t.label);
        s.top = TopTab(t.tab);
        if (t.tab.empty()) ++g.nNoTab;
        const int slots = t.two ? 2 : 1;
        for (int k = 0; k < slots; ++k) {
            s.key[k] = Widen(t.key[k]);
            s.alias[k] = t.alias[k].empty() ? std::wstring(L"（空）") : Widen(t.alias[k]);
            const int mi = t.motIndex[k];
            const int a = (mi >= 0 && mi < (int)g.axisOfMot.size()) ? g.axisOfMot[(std::size_t)mi] : -1;
            g.pointAxis[2 * p + (std::size_t)k] = a;
            if (a >= 0 && (k == 0 || g.pointAxis[2 * p] != a)) g.axisRefs[(std::size_t)a].push_back((int)p);
        }
        s.ini = IniOf(t.alias[0], t.key[0], t.two);
        if (t.two) s.ini += L" ／ " + IniOf(t.alias[1], t.key[1], true);
    }
}

int CountTop(const std::string& top)
{
    int c = 0;
    for (std::size_t p = 0; p < g.text.size(); ++p)
        if (g.text[p].top == top) ++c;
    return c;
}

std::wstring ItemText(const std::wstring& name, int count)
{
    std::wstring c;
    GridSetInt(c, count);
    return name + L"（" + c + L"）";
}

// combo items: "all", then the tabs that have rows in golden PageControl2 order, other tab names, "(no tab)"
void BuildFilterCombo()
{
    std::string keep;
    const bool keepAll = g.filterItem <= 0 || g.filterItem > (int)g.filterTabs.size();
    if (!keepAll) keep = g.filterTabs[(std::size_t)(g.filterItem - 1)];
    g.filterTabs.clear();
    for (int i = 0; i < kTabOrderCount; ++i)
        if (CountTop(kTabOrder[i]) > 0) g.filterTabs.push_back(kTabOrder[i]);
    for (std::size_t p = 0; p < g.text.size(); ++p) {
        const std::string& top = g.text[p].top;
        if (top.empty()) continue;
        bool have = false;
        for (std::size_t k = 0; k < g.filterTabs.size() && !have; ++k) have = g.filterTabs[k] == top;
        if (!have) g.filterTabs.push_back(top);
    }
    if (g.nNoTab > 0) g.filterTabs.push_back(std::string());
    g.filterItem = 0;
    for (std::size_t k = 0; !keepAll && k < g.filterTabs.size(); ++k)
        if (g.filterTabs[k] == keep) g.filterItem = (int)k + 1;
    if (!g.filter) return;
    ::SendMessageW(g.filter, CB_RESETCONTENT, 0, 0);
    std::wstring all = ItemText(L"全部", (int)g.points.size());
    ::SendMessageW(g.filter, CB_ADDSTRING, 0, (LPARAM)all.c_str());
    for (std::size_t k = 0; k < g.filterTabs.size(); ++k) {
        const std::wstring t = g.filterTabs[k].empty() ? ItemText(L"（沒有對到頁籤）", g.nNoTab)
                                                       : ItemText(Widen(g.filterTabs[k]), CountTop(g.filterTabs[k]));
        ::SendMessageW(g.filter, CB_ADDSTRING, 0, (LPARAM)t.c_str());
    }
    ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)g.filterItem, 0);
}

// the rows the filter keeps; the grid is re-rendered once and its own highlight cleared (the panel keeps its row)
void ApplyFilter()
{
    const bool all = g.filterItem <= 0 || g.filterItem > (int)g.filterTabs.size();
    const std::string want = all ? std::string() : g.filterTabs[(std::size_t)(g.filterItem - 1)];
    g.disp.clear();
    g.dispOf.assign(g.points.size(), -1);
    for (std::size_t p = 0; p < g.points.size(); ++p) {
        if (!all && g.text[p].top != want) continue;
        g.dispOf[p] = (int)g.disp.size();
        g.disp.push_back((int)p);
    }
    if (g.list) {
        GridSetRowCount(g.list, 0);   // drops the grid's selection (a display row that now means another point)
        GridSetRowCount(g.list, (int)g.disp.size());
    }
    g.lastGridSel = -1;
}

void OnFilterItem(int item)
{
    if (item < 0 || item > (int)g.filterTabs.size()) return;
    g.filterItem = item;
    if (g.filter) ::SendMessageW(g.filter, CB_SETCURSEL, (WPARAM)item, 0);
    ApplyFilter();
}

void CreateChildren()
{
    g.title = MakeChild(L"STATIC", L"HW.teach — 原生 Win32 視窗（v1 只顯示，St02 20260929）", SS_LEFT | SS_NOPREFIX, kIdcTitle);
    if (g.title) ::SendMessageW(g.title, WM_SETFONT, (WPARAM)HostFont(true), FALSE);
    g.note = MakeChild(L"STATIC",
                       L"golden TfTeach 的顯示部分：每一列是一個教導點（P＝TECH_PARA，T＝TECH_TWOPARA 兩軸），右邊是選到那一列的軸（pnlMotion）。"
                       L"點選只換畫面、不寫卡（golden 選軸的 SetSpeed 不做）；Set／Go 是畫出來的停用鈕，運動鈕全部停用，程式沒有接任何運動、IO 或存檔。\n"
                       L"v1 不套 golden FormShow 的頁籤／面板顯示規則（約 660 行，forms/fTeach.h:432 [DEP]）：所有列都列出，實機上 golden 會藏起來的列這裡也看得到。"
                       L"上下 AOI（gbTopBtmAOI）的 9 格 golden 沒有登記（InitialFormOncetime 沒人呼叫），照 golden 不列。",
                       SS_LEFT | SS_NOPREFIX, kIdcNote);
    g.summary = LabelCreate(g.hwnd, kIdcSummary, L"（等待第一批資料）");
    if (g.summary) ::SendMessageW(g.summary, WM_SETFONT, (WPARAM)HostFont(false), FALSE);

    g.filterLabel = MakeChild(L"STATIC", L"頁籤：", SS_LEFT | SS_NOPREFIX, kIdcFilterLabel);
    g.filter = MakeChild(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, kIdcFilter);

    g.list = GridCreate(g.hwnd, kIdcList, &ListCell, 0);
    if (g.list) ::SendMessageW(g.list, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> lc((std::size_t)kTcColCount);
    lc[kTcColNo]     = GridColumn(L"#", 56, kGridLeft);
    lc[kTcColTab]    = GridColumn(L"頁籤", 150, kGridLeft);
    lc[kTcColGroup]  = GridColumn(L"群組", 130, kGridLeft);
    lc[kTcColLabel]  = GridColumn(L"Set 鈕", 140, kGridLeft);
    lc[kTcColKey]    = GridColumn(L"Key（輸入框）", 190, kGridLeft);
    lc[kTcColMotor]  = GridColumn(L"馬達", 120, kGridLeft);
    lc[kTcColValue]  = GridColumn(L"教導值", 76, kGridRight);
    lc[kTcColPos]    = GridColumn(L"目前位置", 80, kGridRight);
    lc[kTcColSet]    = GridColumn(L"Set", 62, kGridCenter);
    lc[kTcColGo]     = GridColumn(L"Go", 62, kGridCenter);
    lc[kTcColKey2]   = GridColumn(L"第二軸 Key", 180, kGridLeft);
    lc[kTcColMotor2] = GridColumn(L"第二軸馬達", 120, kGridLeft);
    lc[kTcColValue2] = GridColumn(L"教導值", 76, kGridRight);
    lc[kTcColPos2]   = GridColumn(L"目前位置", 80, kGridRight);
    lc[kTcColIni]    = GridColumn(L"teach.ini", 300, kGridLeft);
    GridSetColumns(g.list, lc, std::vector<int>());

    g.lamps = GridCreate(g.hwnd, kIdcLamps, &LampCell, 0);
    if (g.lamps) ::SendMessageW(g.lamps, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> mc((std::size_t)kLampCols);
    for (int k = 0; k < kLampCols; ++k) mc[(std::size_t)k] = GridColumn(L"", 92, kGridLeft);
    GridSetColumns(g.lamps, mc, std::vector<int>());
    GridSetRowCount(g.lamps, (kLedCount + kLampCols - 1) / kLampCols);

    g.detail = GridCreate(g.hwnd, kIdcDetail, &DetailCell, 0);
    if (g.detail) ::SendMessageW(g.detail, WM_SETFONT, (WPARAM)HostFont(false), FALSE);
    std::vector<GridColumn> dc(2);
    dc[0] = GridColumn(L"項目", 230, kGridLeft);
    dc[1] = GridColumn(L"值", 250, kGridLeft);
    GridSetColumns(g.detail, dc, std::vector<int>());
    GridSetRowCount(g.detail, kTdRowCount);

    g.cmdLabel = MakeChild(L"STATIC", L"golden 運動面板指令（v1 停用）：", SS_LEFT | SS_NOPREFIX, kIdcCmdLabel);
    g.cmd.clear();
    for (int i = 0; i < kCmdCount; ++i)
        g.cmd.push_back(MakeChild(L"BUTTON", kCmdNames[i], BS_PUSHBUTTON | WS_DISABLED, kIdcCmd0 + i));
    g.exitBtn = MakeChild(L"BUTTON", L"Exit（只關這個視窗）", BS_PUSHBUTTON | WS_TABSTOP, kIdcExit);

    BuildFilterCombo();   // "all (0)" until the first update
    ApplyFilter();
}

void Layout()
{
    if (!g.hwnd) return;
    RECT rc;
    ::GetClientRect(g.hwnd, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top, m = 8;
    ::MoveWindow(g.title,   m, 6, w - 2 * m, 22, TRUE);
    ::MoveWindow(g.note,    m, 30, w - 2 * m, 56, TRUE);
    ::MoveWindow(g.summary, m, 88, w - 2 * m, 62, TRUE);
    ::MoveWindow(g.filterLabel, m, 158, 60, 20, TRUE);
    ::MoveWindow(g.filter, m + 64, 154, 340, 420, TRUE);   // the height includes the drop-down list
    const int top = 184, rightW = 500, lampH = 110;
    const int btnRows = (kCmdCount + kCmdPerRow - 1) / kCmdPerRow, cmdH = 22 + btnRows * 30 + 8;
    const int bodyH = (h - top - m - cmdH) > 160 ? (h - top - m - cmdH) : 160;
    const int listW = (w - rightW - 3 * m) > 300 ? (w - rightW - 3 * m) : 300;
    ::MoveWindow(g.list, m, top, listW, bodyH, TRUE);
    const int rx = m + listW + m;
    ::MoveWindow(g.lamps, rx, top, rightW, lampH, TRUE);   // a header + two lamp rows + the (disabled) scroll bar
    ::MoveWindow(g.detail, rx, top + lampH + 6, rightW, bodyH - lampH - 6 > 80 ? bodyH - lampH - 6 : 80, TRUE);
    const int cy = top + bodyH + 6;
    ::MoveWindow(g.cmdLabel, m, cy, 360, 20, TRUE);
    for (int i = 0; i < kCmdCount; ++i)
        ::MoveWindow(g.cmd[(std::size_t)i], m + (i % kCmdPerRow) * 136, cy + 22 + (i / kCmdPerRow) * 30, 130, 26, TRUE);
    ::MoveWindow(g.exitBtn, w - m - 200, cy + 22 + (btnRows - 1) * 30, 200, 26, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (HostWindowMessage(hwnd, msg, wp, lp)) return 0;   // keepalive timer (NativeHost.h)
    switch (msg) {
    case WM_CREATE:
        g.hwnd = hwnd;
        HostWindowCreated(hwnd);
        CreateChildren();
        Layout();
        return 0;
    case WM_SIZE:
        Layout();
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* mm = reinterpret_cast<MINMAXINFO*>(lp);
        mm->ptMinTrackSize.x = 1100;
        mm->ptMinTrackSize.y = 640;
        return 0;
    }
    case WM_COMMAND:
        // Exit, and the tab filter (screen state only).  The golden command buttons are WS_DISABLED and deliberately
        // have no branch here; Set / Go are drawn by the grid (no HWND, no WM_COMMAND at all).
        if (LOWORD(wp) == kIdcExit && HIWORD(wp) == BN_CLICKED) ::DestroyWindow(hwnd);   // NOT golden pnlExitClick / FormClose
        else if (LOWORD(wp) == kIdcFilter && HIWORD(wp) == CBN_SELCHANGE && g.filter)
            OnFilterItem((int)::SendMessageW(g.filter, CB_GETCURSEL, 0, 0));
        return 0;
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HostWindowDestroyed(hwnd);
        break;
    case WM_NCDESTROY:
        g.hwnd = g.title = g.note = g.summary = g.list = g.lamps = g.detail = g.exitBtn = g.filterLabel = g.filter = 0;
        g.cmdLabel = 0;
        g.cmd.clear();
        // forget the data: a reopened window's first update is then a full (structural) build
        g.points.clear();
        g.axes.clear();
        g.text.clear();
        g.disp.clear();
        g.dispOf.clear();
        g.selPoint = g.lastGridSel = g.panelMot = -1;
        break;   // no PostQuitMessage: this thread is the wb_serve main loop
    default:
        break;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

bool RegisterOnce()
{
    static bool done = false;
    if (done) return true;
    WNDCLASSEXW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = ::GetModuleHandleW(0);
    wc.hCursor = ::LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    wc.hIcon = ::LoadIconW(0, (LPCWSTR)IDI_APPLICATION);
    if (!::RegisterClassExW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
    done = true;
    return true;
}

struct ButtonCount { int all, enabled; };
BOOL CALLBACK CountButtons(HWND h, LPARAM lp)
{
    ButtonCount* c = reinterpret_cast<ButtonCount*>(lp);
    wchar_t cls[32];
    if (::GetClassNameW(h, cls, 32) > 0 && ::lstrcmpiW(cls, L"Button") == 0) {
        ++c->all;
        if (::IsWindowEnabled(h)) ++c->enabled;
    }
    return TRUE;
}

bool PointStaticChanged(const TeachPoint& a, const TeachPoint& b)
{
    return a.index != b.index || a.two != b.two || a.motIndex[0] != b.motIndex[0] || a.motIndex[1] != b.motIndex[1] ||
           a.key[0] != b.key[0] || a.key[1] != b.key[1] || a.alias[0] != b.alias[0] || a.alias[1] != b.alias[1] ||
           a.tab != b.tab || a.group != b.group || a.label != b.label;
}

bool PointValueChanged(const TeachPoint& a, const TeachPoint& b)
{
    return a.hasValue[0] != b.hasValue[0] || a.value[0] != b.value[0] || a.hasValue[1] != b.hasValue[1] || a.value[1] != b.value[1];
}

bool AxisStaticChanged(const TeachAxis& a, const TeachAxis& b)
{
    return a.motIndex != b.motIndex || a.alias != b.alias || a.numberAlias != b.numberAlias;
}

bool AxisNowChanged(const TeachAxis& a, const TeachAxis& b)
{
    return a.hasNow != b.hasNow || a.now != b.now;
}

bool AxisPanelChanged(const TeachAxis& a, const TeachAxis& b)
{
    return AxisNowChanged(a, b) || a.hasEnc != b.hasEnc || a.enc != b.enc || a.motorType != b.motorType ||
           a.hasHomeOffset != b.hasHomeOffset || a.homeOffset != b.homeOffset || a.servoOn != b.servoOn ||
           a.alarm != b.alarm || a.busy != b.busy || a.inPos != b.inPos || a.homeFlag != b.homeFlag ||
           a.quality != b.quality || a.source != b.source || a.errText != b.errText;
}

bool AxisLampChanged(const TeachAxis& a, const TeachAxis& b)
{
    return a.ledKnown != b.ledKnown || a.motionIO != b.motionIO || a.state != b.state;
}

void AssignIfDiffers(std::string& dst, const std::string& src)
{
    if (dst != src) dst = src;
}

// the static fields are equal: copy the values only (strings only when they differ -- no allocation per tick)
void CopyAxisValues(TeachAxis& dst, const TeachAxis& src)
{
    dst.motorType = src.motorType;
    dst.hasNow = src.hasNow; dst.now = src.now;
    dst.hasEnc = src.hasEnc; dst.enc = src.enc;
    dst.hasHomeOffset = src.hasHomeOffset; dst.homeOffset = src.homeOffset;
    dst.servoOn = src.servoOn; dst.alarm = src.alarm; dst.busy = src.busy; dst.inPos = src.inPos; dst.homeFlag = src.homeFlag;
    dst.ledKnown = src.ledKnown; dst.motionIO = src.motionIO; dst.state = src.state;
    AssignIfDiffers(dst.quality, src.quality);
    AssignIfDiffers(dst.source, src.source);
    AssignIfDiffers(dst.errText, src.errText);
}

void MarkPoint(int p)
{
    if (!g.list || p < 0 || p >= (int)g.dispOf.size()) return;
    const int r = g.dispOf[(std::size_t)p];
    if (r >= 0) GridMarkRow(g.list, r);
}

std::wstring FilterName()
{
    if (g.filterItem <= 0 || g.filterItem > (int)g.filterTabs.size()) return L"全部";
    const std::string& t = g.filterTabs[(std::size_t)(g.filterItem - 1)];
    return t.empty() ? std::wstring(L"（沒有對到頁籤）") : Widen(t);
}

std::wstring BuildSummary()
{
    int nP = 0, nT = 0, nNow = 0, nAlarm = 0;
    for (std::size_t p = 0; p < g.points.size(); ++p) {
        if (g.points[p].two) ++nT; else ++nP;
    }
    for (std::size_t a = 0; a < g.axes.size(); ++a) {
        if (g.axes[a].hasNow) ++nNow;
        if (g.axes[a].alarm == 1) ++nAlarm;
    }
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    const GridStats a = GridGetStats(g.list), b = GridGetStats(g.detail), c = GridGetStats(g.lamps);
    const std::string reg = g.sum.registryWhy.empty() ? std::string() : "｜" + g.sum.registryWhy;
    const std::string why = g.sum.why.empty() ? std::string() : "｜" + g.sum.why;
    char buf[1800];
    std::snprintf(buf, sizeof(buf),
                  "[%s] 教導點 %u 列（P＝TECH_PARA %d＋T＝TECH_TWOPARA %d；登錄表讀到 %d＋%d）｜顯示 %u 列｜沒對到頁籤 %d 列%s\n"
                  "馬達 %u 軸（有位置 %d，警報 %d）｜覆蓋掛鉤上一拍 %lu 次（每軸一次，不是每列一次）｜1203 監看器：%s（%d 軸，poll #%lu）｜"
                  "拖曳保活 %lu 次｜更新 %02u:%02u:%02u.%03u%s\n"
                  "畫面效能：上次更新 %.2f ms｜實際更新間隔約 %.0f ms｜累計重畫 %lu 格（表 %lu／面板 %lu／燈 %lu）",
                  g.sum.buildConfig.empty() ? "?" : g.sum.buildConfig.c_str(), (unsigned)g.points.size(), nP, nT,
                  g.sum.techParaCount, g.sum.twoParaCount, (unsigned)g.disp.size(), g.nNoTab, reg.c_str(),
                  (unsigned)g.axes.size(), nNow, nAlarm, g.sum.overlayCalls, g.sum.monitorOpen ? "已開卡" : "沒有開卡",
                  g.sum.monitorAxes, g.sum.pollCount, g.sum.keepaliveCalls, (unsigned)st.wHour, (unsigned)st.wMinute,
                  (unsigned)st.wSecond, (unsigned)st.wMilliseconds, why.c_str(), g.lastUpdateMs, g.avgGapMs,
                  a.cellsPainted + b.cellsPainted + c.cellsPainted, a.cellsPainted, b.cellsPainted, c.cellsPainted);
    return Widen(buf) + L"｜篩選：" + FilterName();
}

}  // namespace

const TeachKeyInfo* TeachFindKey(const std::string& key)
{
    int lo = 0, hi = kKeyInfoCount - 1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        const int c = std::strcmp(key.c_str(), kKeyInfo[mid].key);
        if (c == 0) return &kKeyInfo[mid];
        if (c < 0) hi = mid - 1; else lo = mid + 1;
    }
    return 0;
}

int TeachKeyTableSize() { return kKeyInfoCount; }

const TeachKeyInfo* TeachKeyAt(int i) { return (i >= 0 && i < kKeyInfoCount) ? &kKeyInfo[i] : 0; }

bool TeachOpen(bool show)
{
    if (g.hwnd) {
        if (show) {
            ::ShowWindow(g.hwnd, ::IsIconic(g.hwnd) ? SW_RESTORE : SW_SHOW);
            ::SetForegroundWindow(g.hwnd);
        }
        return true;
    }
    if (!RegisterOnce()) return false;
    HWND h = ::CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"HT9045 V906 — HW.teach（原生，v1 只顯示）",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 960,
                               0, 0, ::GetModuleHandleW(0), 0);
    if (!h) return false;
    if (show) {
        ::ShowWindow(h, SW_SHOWNORMAL);
        ::UpdateWindow(h);
    }
    return true;
}

void TeachClose()
{
    if (g.hwnd) ::DestroyWindow(g.hwnd);
}

bool TeachIsOpen()
{
    return g.hwnd != 0 && ::IsWindow(g.hwnd);
}

void TeachUpdate(const std::vector<TeachPoint>& points, const std::vector<TeachAxis>& axes, const TeachPage& page,
                 const TeachSummary& sum)
{
    if (!TeachIsOpen()) return;
    const double t0 = NowMs();
    if (g.lastT0 > 0) {
        const double gap = t0 - g.lastT0;
        g.avgGapMs = g.avgGapMs > 0 ? g.avgGapMs * 0.9 + gap * 0.1 : gap;
    }
    g.lastT0 = t0;
    g.sum = sum;

    bool structural = points.size() != g.points.size() || axes.size() != g.axes.size();
    for (std::size_t i = 0; !structural && i < points.size(); ++i)
        if (PointStaticChanged(points[i], g.points[i])) structural = true;
    for (std::size_t i = 0; !structural && i < axes.size(); ++i)
        if (AxisStaticChanged(axes[i], g.axes[i])) structural = true;
    const bool pageChanged = page.known != g.page.known || page.activeMotor != g.page.activeMotor || page.setToOffset != g.page.setToOffset;
    g.page = page;

    if (structural) {
        g.points = points;
        g.axes = axes;
        g.selPoint = -1;
        BuildStatic();
        BuildFilterCombo();
        ApplyFilter();   // full render of the table
        g.panelMot = PanelMotor();
        if (g.detail) { GridMarkAll(g.detail); GridFlush(g.detail); }
        if (g.lamps) { GridMarkAll(g.lamps); GridFlush(g.lamps); }
    } else {
        // a click on the table moves the selection (screen state only: golden's SetSpeed(1) on axis select is not done)
        const int oldSel = g.selPoint;
        const int gs = g.list ? GridSelectedRow(g.list) : -1;
        if (gs != g.lastGridSel) {
            g.lastGridSel = gs;
            if (gs >= 0 && gs < (int)g.disp.size()) g.selPoint = g.disp[(std::size_t)gs];
        }
        const int oldPanel = g.panelMot;
        g.panelMot = PanelMotor();
        const int panel2 = PanelMotor2();
        bool detailDirty = g.selPoint != oldSel || g.panelMot != oldPanel || pageChanged;
        bool lampDirty = g.panelMot != oldPanel;
        for (std::size_t i = 0; i < axes.size(); ++i) {
            const TeachAxis& a = axes[i];
            TeachAxis& b = g.axes[i];
            const bool nowCh = AxisNowChanged(a, b), pan = nowCh || AxisPanelChanged(a, b), lp = AxisLampChanged(a, b);
            if (!pan && !lp) continue;
            if (nowCh) {   // only the rows that use this motor (their "current position" cells)
                const std::vector<int>& refs = g.axisRefs[i];
                for (std::size_t k = 0; k < refs.size(); ++k) MarkPoint(refs[k]);
            }
            if (b.motIndex >= 0 && b.motIndex == g.panelMot) { detailDirty = detailDirty || pan; lampDirty = lampDirty || lp; }
            if (b.motIndex >= 0 && b.motIndex == panel2) detailDirty = detailDirty || nowCh;
            CopyAxisValues(b, a);
        }
        for (std::size_t i = 0; i < points.size(); ++i) {
            if (!PointValueChanged(points[i], g.points[i])) continue;
            TeachPoint& b = g.points[i];
            b.hasValue[0] = points[i].hasValue[0]; b.value[0] = points[i].value[0];
            b.hasValue[1] = points[i].hasValue[1]; b.value[1] = points[i].value[1];
            MarkPoint((int)i);
            if ((int)i == g.selPoint) detailDirty = true;
        }
        if (g.selPoint != oldSel) {   // the red "panel row" colour moves
            MarkPoint(oldSel);
            MarkPoint(g.selPoint);
        }
        if (g.list) GridFlush(g.list);
        if (detailDirty && g.detail) { GridMarkAll(g.detail); GridFlush(g.detail); }   // only changed cells are painted
        if (lampDirty && g.lamps) { GridMarkAll(g.lamps); GridFlush(g.lamps); }
    }
    if (g.summary) LabelSetText(g.summary, BuildSummary());
    g.lastUpdateMs = NowMs() - t0;
}

void* TeachHwnd() { return g.hwnd; }

int TeachRowCount() { return g.list ? GridRowCount(g.list) : -1; }

std::wstring TeachCellText(int row, int column) { return g.list ? GridCellText(g.list, row, column) : L""; }

int TeachRowPoint(int row) { return (row >= 0 && row < (int)g.disp.size()) ? g.disp[(std::size_t)row] : -1; }

std::wstring TeachDetailText(int row) { return g.detail ? GridCellText(g.detail, row, 1) : L""; }

std::wstring TeachLampText(int led)
{
    const TeachAxis* a = AxisOfMot(g.panelMot);
    if (!a || !a->ledKnown || led < 0 || led >= kLedCount) return L"—";
    return MotorLedOn(LampRow(*a), led) ? L"1" : L"0";
}

int TeachSelectedPoint() { return g.selPoint; }

int TeachPanelMotor() { return g.panelMot; }

void TeachSelect(int row)
{
    RECT rc;
    if (!g.list || !GridCellRect(g.list, row, kTcColKey, &rc)) return;
    ::SendMessageW(g.list, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(rc.left + 2, rc.top + 2));   // as the operator's click
}

bool TeachSetFilterItem(int item)
{
    if (!g.hwnd || item < 0 || item > (int)g.filterTabs.size()) return false;
    OnFilterItem(item);
    return true;
}

bool TeachSetFilter(const std::string& tab)
{
    if (tab.empty()) return TeachSetFilterItem(0);
    for (std::size_t k = 0; k < g.filterTabs.size(); ++k)
        if (g.filterTabs[k] == tab) return TeachSetFilterItem((int)k + 1);
    return false;
}

int TeachFilterCount() { return g.filter ? (int)::SendMessageW(g.filter, CB_GETCOUNT, 0, 0) : -1; }

std::wstring TeachFilterText(int item)
{
    if (!g.filter) return L"";
    const LRESULT n = ::SendMessageW(g.filter, CB_GETLBTEXTLEN, (WPARAM)item, 0);
    if (n == CB_ERR || n < 0) return L"";
    std::vector<wchar_t> b((std::size_t)n + 1, 0);
    ::SendMessageW(g.filter, CB_GETLBTEXT, (WPARAM)item, (LPARAM)&b[0]);
    return std::wstring(&b[0]);
}

int TeachButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.all;
}

int TeachEnabledButtonCount()
{
    if (!g.hwnd) return -1;
    ButtonCount c = {0, 0};
    ::EnumChildWindows(g.hwnd, &CountButtons, (LPARAM)&c);
    return c.enabled;
}

void* TeachListGrid() { return g.list; }
void* TeachDetailGrid() { return g.detail; }
void* TeachLampGrid() { return g.lamps; }

std::wstring TeachSummaryText()
{
    if (!g.summary) return L"";
    wchar_t b[1800];
    b[0] = 0;
    ::GetWindowTextW(g.summary, b, 1800);
    return b;
}

std::wstring TeachNoteText()
{
    if (!g.note) return L"";
    wchar_t b[1200];
    b[0] = 0;
    ::GetWindowTextW(g.note, b, 1200);
    return b;
}

double TeachLastUpdateMs() { return g.lastUpdateMs; }

}  // namespace w906native
