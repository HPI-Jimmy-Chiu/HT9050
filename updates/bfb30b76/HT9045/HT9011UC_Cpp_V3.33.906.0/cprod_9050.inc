// =============================================================================
//  cprod_9050.inc -- HT9050 tray-stack teach fields of PROD_INFO_ST (textually included by cprod.h inside the struct).
//
//  AI(W906-F9050-BD) 20261004: F9050-BD batch B (ledger H087) -- 910 cprod.h:490-516 (Eastsun 0819), verbatim, at 910's
//  place: cprod.h:490 includes this file right after TrayZ_Mid and before iBS1Right_Half, and 910's TrayZ_Down (910
//  cprod.h:488) is cprod.h:488, between TrayZ_Up and TrayZ_Mid.  So iTemperature .. iBS2Right_Half are in 910's order (all
//  int, no padding; tests/test_flow9050_tray.cpp pins it with offsetof), and the out-of-bounds layer reads of 910 defect (a)
//  (below) hit the same neighbours as in 910.  Prod is never written to a file (no binary dump, no ini key: boot rebuilds it
//  from Tech + offsets, e.g. the TrayZ_Up / TrayZ_Mid assignments at cinitial.cpp:6009-6020) and every reader names its
//  field, so moving every field behind TrayZ_Up changes no file format; tests/test_globals.cpp's anchors (cFileName @0,
//  LastEditDate @256, sizeof > 50000) still hold.
//  910 defect (a) (pending Frank/Jimmy, TO_FRANK s4 1001 18:1x), translated as written: TrayZ_Up / TrayZ_Down are
//  [MAX_TRACK]=9 but DoLoad_9050 / DoLoadNewICTray_9050 / DoCatchFromLoader_9050 index them with the Loader layer count
//  (from -1 up; the probe alone walks to 19), and the Empty / Auto1-3 arrays below are [MAX_LOADER_LAYER_9050]=20 and are
//  first read at [-1].  As in 910: TrayZ_Up[9..17] reads TrayZ_Down[0..8] and [18..19] TrayZ_Mid[0..1]; TrayZ_Down[-1]
//  reads TrayZ_Up[8], [9..17] TrayZ_Mid[0..8], [18] TrayZ_Home, [19] iLoaderInitDetectLayer; EmptyZ_Up[-1] reads
//  iCatchExtraLift_9050; every other [-1] / [20..] read lands on the field before / after it in 910's order, and past
//  iAuto3CatchExtraLift_9050 come iBS1Right_Half / iBS2Right_Half.
//  HAZARD (pending Frank/Jimmy, ledger s0 #2): 910 assigns NONE of the fields in this file, nor TrayZ_Down -- they stay 0
//  (global zero-init), so on a real HT9050 HOME sends MLoaderZ to TrayZ_Home = 0 and every Down / Home move and every
//  Empty / Auto1-3 Z move goes to 0.  TrayZ_Up[0..8] are NOT 0: SetTechDataToProd_TrayArm (cinitial.cpp:6009-6020, as 910)
//  fills them with golden teach values + 200 (Tech.iTrayLoaderZ[0..5], Tech.iTrayAuto4Z[0..2] for 6..8), so the Loader "Up"
//  moves of layers 0..8 go to those positions.
// =============================================================================
    int TrayZ_Home;                                                             //Eastsun 20260819
    int iLoaderInitDetectLayer;                                                     //Eastsun 20260819 : 9050 case 1100 探測層數的起始猜測值 (e.g. 10)
    int iCatchExtraLift_9050;                                                    //Eastsun 20260819 : 9050 TrayArm catch-from-loader 交接時 MLoaderZ 額外抬升距離
    // Eastsun 20260819 : 9050 Empty 區 TrayArm catch 教值
    int EmptyZ_Up[MAX_LOADER_LAYER_9050];
    int EmptyZ_Down[MAX_LOADER_LAYER_9050];
    int EmptyZ_Home;
    int iEmptyInitDetectLayer;
    int iEmptyCatchExtraLift_9050;
    // Eastsun 20260819 : 9050 Auto1 區 TrayArm catch 教值
    int Auto1Z_Up[MAX_LOADER_LAYER_9050];
    int Auto1Z_Down[MAX_LOADER_LAYER_9050];
    int Auto1Z_Home;
    int iAuto1InitDetectLayer;
    int iAuto1CatchExtraLift_9050;
    // Eastsun 20260819 : 9050 Auto2 區 TrayArm catch 教值
    int Auto2Z_Up[MAX_LOADER_LAYER_9050];
    int Auto2Z_Down[MAX_LOADER_LAYER_9050];
    int Auto2Z_Home;
    int iAuto2InitDetectLayer;
    int iAuto2CatchExtraLift_9050;
    // Eastsun 20260819 : 9050 Auto3 區 TrayArm catch 教值
    int Auto3Z_Up[MAX_LOADER_LAYER_9050];
    int Auto3Z_Down[MAX_LOADER_LAYER_9050];
    int Auto3Z_Home;
    int iAuto3InitDetectLayer;
    int iAuto3CatchExtraLift_9050;
