// =============================================================================
//  forms/fNote.h  --  non-VCL stand-in for golden's fNote form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtual destructor.  See forms/fMain.h for the facade-wide
//  contract.
// =============================================================================
#ifndef FORMS_FNOTE_H
#define FORMS_FNOTE_H

#include "forms/FormWidgets.h"

// =============================================================================
//  AI(W906-FW-NOTE-W33) 20260826: note.cpp Wave A -- 6 支唯讀方向 methods。
//  本檔既有內容一行未動（append-only）；這一段以下全部是新增。
//
//  GOLDEN 參照：HT9011UC_Code_V3.33.906.0_20260618/note.cpp（6,832 行，
//  cp950，本波以 open(p,'rb').read().decode('cp950') 解碼，0 U+FFFD）
//  ＋ note.h（未逐行全讀，只查證本波用到的成員宣告，逐條列在下面）。
//
//  分母（自行重量，不引用上游數字）
//  --------------------------------------------------------------------------
//  golden note.cpp 有 62 支 DISTINCT TfNote:: 定義 / 6,832 行。
//  量法與 forms/fOffSet.h 相同：字元狀態機剝掉 block 註解、行註解、字串與
//  字元字面值後再掃 TfNote::NAME(，並用 #if 0 追蹤器標記死區。
//  結果 62 hits、live 62、dead 0。
//  ⚠ 這個 62 與主迴圈給的 62 相符 —— 那是重量之後的結論，不是採信。
//
//  本波交付：6 支 / 131 golden 行（占 62 支的 9.7%、6,832 行的 1.9%）。
//
//  ⚠ 為什麼只有 6 支 —— 這是量出來的，不是偷懶
//  --------------------------------------------------------------------------
//  fNote 是**警報／JAM 對話框**，它的 method 幾乎都是操作員按鈕的入口：
//  BtnStartClick 呼叫 Start()（開始運轉）、BtnResetClick 呼叫
//  fMain->BtnResetClick、BtnPauseClick 是暫停、btnMoveToFront/RearClick
//  是把 Index 移到前／後、TimerFTPTimer 與 AlarmLogXmlToFTP 是對外上傳、
//  SaveErrEventLog / RecordProcess 家族是寫檔。也就是說「唯讀方向」在這個
//  表單上本來就是少數。逐支讀完之後真正能收的只有 6 支，其餘不是踩硬退出
//  線，就是相依落在 ht9045_forms 連不到的 archive（見 GATE REGISTER）。
//  這裡選擇「少而正確」，不用空殼補數字。
//
//  取批標準（唯讀方向）
//  --------------------------------------------------------------------------
//  只收「不動機台、不寫檔／持久化、不送對外命令」，且相依全部落在
//  ht9045_forms 可連的 archive（vclcompat / ht9045_globals / ht9045_core）
//  之內的 method。6 支全部逐行開 golden 讀完本體。
//
//  DEPENDENCIES ADDED（只加在 forms/fNote.cpp；本 header 不新增 include）
//    forms/fQwertyKey.h  -- fQwertyKey + ShowQwertyKey（同 target，無新 edge）
//  cmydef.h 本 .cpp 早已 include（W7-L2），PosCodeToStr 用到的 90 個馬達
//  常數全部由它提供 —— 本波逐一比對過 cmydef.h 的 extern 與 cmydef.cpp 的
//  const int 定義，90 個全在，0 個缺。
//
//  DEVIATIONS
//  --------------------------------------------------------------------------
//  (N-D1) TObject *Sender 未被讀時整支 drop；被 cast 使用時直接宣告成 cast
//         後的型別（TEdit *Sender）。同 forms/fSetup.cpp:72/:188 的既有慣例。
//         TMouseButton Button, TShiftState Shift, int X, int Y 逐支確認
//         golden 本體從未讀取，故 drop。
//  (N-D2) golden 的 TDateTime 有 public 資料成員 Val（BCB6 System::TDateTime），
//         所以 golden 寫 NowTime.Val（沒有括號）。vclcompat::TDateTime 把它
//         做成 method（vclcompat/TDateTime.h:44 `double Val() const`），
//         所以本移植寫 NowTime.Val()。**只是取值語法不同，數值語意相同**。
//         唯一受影響的是 edBundleIDChange。
//  (N-D3) strngrdMDBQuery->SendToBack() / ->Visible ——
//         vclcompat::TStringGrid 是 `: public TObject`（不是 TControl），
//         所以既沒有 Visible 也沒有 SendToBack()。照 forms/fConfiguration.h
//         與 forms/fContactCT.h 的「TStringGrid 擴充子類別」既有作法，在
//         facade-local 子類別補這兩樣。SendToBack() 是 offline no-op ——
//         **本樹沒有 Z 序**，同 vclcompat/Controls.h TControl::BringToFront()
//         的既有處置。Visible 只存值。
//
//  GATE REGISTER（一律整支退出：header 宣告、.cpp 不定義，讓 linker 當互鎖）
//  --------------------------------------------------------------------------
//  (N-1) BtnStartClick            golden :3806-3824
//        ⚠ **會啟動機台**：:3813/:3822 `Start();`（golden :3527-3804，278 行
//        的運轉啟動路徑），並且 :3814 NewRecordProcess(...) 是寫 log。
//  (N-2) BtnResetClick            golden :5205-5235
//        ⚠ **會動機台**：:5212/:5218 `fMain->BtnResetClick(fMain);`。
//  (N-3) BtnSkipClick             golden :2843-2870
//        呼叫 UpdateButtonStatus（golden :2764-2841，未翻，且本體含
//        ShowMyMessage 類的 modal 路徑）；SOFT_SIMULTE 臂還會 BtnPauseClick。
//  (N-4) btnMoveToFrontClick      golden :5555-5559
//        btnMoveToRearClick       golden :5561-5565
//        ⚠ **會動機台**。兩支只有兩行，設 bIndexMoveToFrontRear=true 與
//        iIndexMoveToFrontRearTask=1/1000 —— 那正是 IndexMoveToFrontRear
//        （golden :5494-5553，含馬達移動）的觸發旗標。設旗標＝下命令。
//  (N-5) edBundleIDKeyPress       golden :6676-6689
//        ⚠ 呼叫 BtnPauseClick（暫停機台）與 NewRecordProcess（寫 log）。
//  (N-6) TimerFTPTimer            golden :5567-5622
//        AlarmLogXmlToFTP         golden :5901-5970
//        btPrintSummaryClick      golden :5634-5659
//        ⚠ 對外傳輸／寫檔／fSecurity 權限檢查。
//  (N-7) SaveErrEventLog          golden :6691-6781
//        RTCalarmImageUnload      golden :1171-1221
//        FormCreate               golden :5751-5762
//        bCheckContAlarmNotUpload golden :6199-6268
//        ⚠ 寫檔／持久化。
//  (N-8) FormDestroy              golden :250-260
//        LogSoftwareOffTime() 的本體在 acarry_shims.cpp:255（**ht9045_sm**，
//        CMakeLists.txt:1353），ht9045_forms 不能 link ht9045_sm
//        （CMakeLists.txt:559-560、:612-646 的 cycle）。規則 5：不自建 shim。
//  (N-9) MovieToBack              golden :5426-5434
//        pnlMovieMouseDown        golden :5436-5440
//        PlayMovie                golden :4742-4760
//        StopMovie                golden :4762-4770
//        MediaPlayer1Notify       golden :4772-4776
//        TMyTray1MouseDown        golden :5401-5424
//        golden note.h:29 `TTMyTray *TMyTray1;` —— TTMyTray 是自製 VCL
//        widget。20260826 收工重跑更正措辭：樹上**有** TTMyTray 這個名字，
//        但全部是**前向宣告**（`class TTMyTray;` —— Motor/mymotor.h:74、
//        TfAOILaserScan.h:118 等），**沒有任何一處是完整定義**。
//        前向宣告只夠當指標型別；`TMyTray1->Top/Left/Width/Height` 要完整
//        型別，編不過。acatchtray_shims.h:304 也明寫 "DELIBERATELY NOT
//        ADDED"。（先前我寫成「全樹沒有家」，措辭不精確，已更正。）
//        另四支還要 TMediaPlayer。為了一塊 offline 完全不會播放的影片面板
//        去立兩個新 widget 型別，是把 facade 養過 measured need。
//  (N-10) tmrKeyInTimer           golden :6443-6458
//         edBinCodeEditKeyPress   golden :6614-6646
//         CheckBinCode            golden :6310-6441
//         ⚠ 三支都吃 golden note.h:456 `TEdit *myBinCodeEdit[TEST_MAX_BIN];`
//         （TEST_MAX_BIN=256，MachineType.h:414）。本檔既有的 TfNote ctor
//         帶顯式初始化串列，append-only 不准改，所以 256 顆 TEdit 沒有地方
//         配置；留 NULL 就是 null deref，不是預設值。
//         tmrKeyInTimer 另外要 `Focused()`，全樹皆無 —— 而 offline 沒有
//         視窗也沒有焦點，做成常數 false 會讓 golden 的迴圈本體**永遠到
//         不了**，那是假成功，不是翻譯。
//  (N-11) Reset                   golden :262-342
//         純 widget 上色／顯示，方向對，但顏色常數拿不到。
//         20260826 收工重跑（剝註解／字串 ＋ #if 0 追蹤，排除 build*）：
//           * `cDark`  —— 活碼命中 **0**，全樹沒有定義。
//           * `clTeal` —— 活碼命中 6 個，**全部在 cObserver.cpp**，而定義是
//             cObserver.cpp:172 `static const TColor clTeal = 0x00808000;`
//             —— `static` ＝ **TU-local**，跨 TU 用不到。
//             （⚠ 這條先前被我寫成「全樹皆無」，重跑後更正。）
//           * `TColor` 本身只在 acatchtray_shims.h:122-127 定義（ht9045_sm，
//             forms 連不到）；cObserver.cpp 是自己在 TU 內另立一套。
//         在這裡補顏色常數就是跨檔 shim（規則 5）。
//  (N-12) ShowTrayQty             golden :6066-6197
//         需要 `MOT[]`（Motor/mymotor.h:385 `extern class TTrayMotor
//         MOT[MAX_TRAY_MOTOR]`，本體在 Motor/mymotor.cpp → **ht9045_motor**）。
//         ht9045_forms 只 link vclcompat + ht9045_globals + ht9045_core
//         （CMakeLists.txt:714-721），連不到。
//  (N-13) FlushLabel              golden :3084-3139
//         需要自製 widget 型別 TBtnPanel，且本體寫的是 bLampSkip/bLampRetry/
//         … 十個 bLamp* 全域 —— 那些正是 ckernel DoPanelLamp 轉成**實體面板
//         指示燈輸出**的來源。方向上不是唯讀。
//  (N-14) DoUnlockPassword        golden :5384-5399
//         PanSpecialNoteClick     golden :5442-5460
//         ⚠ 判斷性退出（不在三條硬線上，理由寫明）：兩支都是**解鎖**路徑
//         （解 Alarm 鎖 / 解 Special Panel 鎖）。offline 的 ShowQwertyKey 是
//         「開了鍵盤立刻送出」，所以比對用的 fPassword->edPassword->Text
//         會停在空字串，密碼比對因此退化。安全相關的比對退化不應該在唯讀
//         波次裡靜默落地，交給專門處理權限的波次。
//  (N-15) palShtSensorSOPClick    golden :4778-4791
//         ShellExecute 開外部 PDF（對外動作），另一臂用 fCCLink（無家）。
//  (N-16) tvMNetCustomDrawItem    golden :5774-5787
//         strngrdMDBQueryDrawCell golden :5871-5881
//         palInArmClick           golden :5791-5854
//         reDescriptionMouseDown  golden :5883-5899
//         需要 Canvas / TCustomTreeView / TTreeNode / MDB 查詢 /
//         RecordProcess（寫 log）—— 皆無家或方向不符。
//  (N-17) ClearContAlarmList      golden :6270-6287
//         需要 `TList`，本樹唯一的宣告在 aHotPlateSubstrate.h:80；把該檔拉進
//         forms 層會一併帶入已知的「兩個 TMyKitSuck」ODR 陷阱
//         （aHotPlateSubstrate.h:365 vs mykitsuck.h:274，佈局不同、14 個
//         全域對撞）。為一支 OLP-gated 的清單清理不值得。
//
//  GATE 帳目（精確，不四捨五入）
//  --------------------------------------------------------------------------
//  上面 (N-1)..(N-17) 共**列名 34 支**。其中 **27 支**在 class 內以
//  「header 宣告、.cpp 不定義」落地，成為真正的 linker 互鎖；
//  另外 **7 支只列名不宣告**：
//    (N-7) 的 SaveErrEventLog / RTCalarmImageUnload / FormCreate /
//          bCheckContAlarmNotUpload
//    (N-16) 的 tvMNetCustomDrawItem / strngrdMDBQueryDrawCell / palInArmClick
//  理由：它們的簽章帶本波沒有查證過型別的參數（TCustomTreeView* /
//  TTreeNode* / TRect& / TGridDrawState 等），與其猜一個簽章不如不宣告 ——
//  猜錯的宣告會在未來接線時比缺宣告更難發現。它們的退出裁決仍然成立。
//  帳目：62 = 6（交付）+ 34（列名退出）+ 22（本波未評估）。
// =============================================================================

// ---------------------------------------------------------------------------
//  Facade-local widget subclass（DEVIATION N-D3）
// ---------------------------------------------------------------------------
class TfNoteMDBGrid : public vclcompat::TStringGrid
{
public:
    // golden `fNote->strngrdMDBQuery->SendToBack(); ->Visible=false;`
    // （note.cpp:5859-5860 與 :5866-5867）。
    // vclcompat::TStringGrid 繼承 TObject 而非 TControl，所以這兩樣都沒有。
    bool Visible = false;
    // offline no-op：本樹沒有視窗、沒有 Z 序，與
    // vclcompat/Controls.h TControl::BringToFront() 的既有處置對稱。
    void SendToBack() {}
    virtual ~TfNoteMDBGrid() {}
};

// ===========================================================================
//  W906-AutoCleanFoundation ADD (20260721): TfNote -- FIRST home for fNote
//  (golden note.h `class TfNote : public TForm`) anywhere in this migrated
//  tree. Multiple earlier waves (Automation/AGV_PortScan.h's TfAGV::Timer2Timer
//  note, BarCode/BarCode_Bottom2DID.h's fNote->t2DCode/bMyServoOffInArm note)
//  found fNote had no home and deliberately left their own fNote-dependent
//  code gated/untranslated rather than stand up "a whole new TfNote facade
//  CLASS" for what was, in each of those cases, a single narrow call with no
//  other payoff. This wave's need is different in kind, not just degree: TWO
//  genuinely in-scope Part A functions (DoInArmPineRelease's servo-off-during-
//  alarm recovery cycle; InitialAutoCleanTask's SPIL FTP jam-code upload trace)
//  read/write exactly 4 plain-data fields -- no VCL lifecycle, no widgets, no
//  settings-editor surface like the deferred cases above. Standing up THIS
//  minimal a home costs nothing extra and unblocks a faithful (not gated)
//  translation of both functions; it does not retroactively un-gate any of
//  the earlier waves' own deferred fNote call sites (still all self-gated,
//  unaffected by this addition).
// ===========================================================================
//
// ===========================================================================
//  AI(W906-W7-L2) 20260803: SECOND growth of TfNote -- the golden ckernel.cpp
//  surface.  This wave is the substrate pass in front of the W7-L2 translation
//  of golden ckernel.cpp (2589 lines, 17 functions, 0% ported).  Everything
//  below is backed by a golden line I decoded from cp950 and read.
//
//  (1) EXHAUSTIVE fNote CENSUS OF GOLDEN ckernel.cpp -- MEASURED, NOT QUOTED.
//      A scan of all 2589 lines finds `fNote` on exactly 18 lines carrying 30
//      `fNote->` dereferences, in 4 of the file's 17 functions.  Broken down by
//      function and by member:
//        ShowRunLed()   (golden :704-932, DEFERRED by the W7-L2 recon):
//            :708  fNote->fShow                      (1 deref)
//            :711-:716 fNote->edErrorCode->Text      (18 derefs, one per
//                      MESxxxx literal in the bTowerLightUseLD LD/ULD chain)
//            :720  fNote->AlarmType==3               (1 deref)
//            ( = 20 for the function, which reproduces the recon's "fNote 20,
//              edErrorCode 18" for ShowRunLed exactly)
//        ShowRunLabel() (golden :935-1726, DEFERRED):
//            :971  fNote->fShow==true                (1 deref)
//        DoPanelLamp()  (golden :1728-1897, THIS FRONT):
//            :1749 fNote->fShow==false               (1 deref, the only one)
//        ScanPannelKey() (golden :1919-2406, THIS FRONT):
//            :2007 :2038 :2184 :2215  fNote->fShow               (4 derefs)
//            :2009 :2040 :2186 :2217  fNote->IsTestSitICFallDown() (4 calls)
//      1 + 8 = 9 live sites inside the two ranges this front owns -- the recon
//      count is CONFIRMED exactly, and so is its warning that the two `fMain->`
//      tokens at golden :1981 and :2158 are NOT live.  Re-read verbatim, both
//      lines are identical and read
//        `if(Sen[SnFKStart].IsOn() || bAseStart)` / `...SnRKStart...`
//      followed by the trailing comment
//        `//Ifor 20160114 Auto Decay 搬移 至 fMain->ScanKey 判斷`.
//      So the CODE on those lines is live but contains no form dereference at
//      all; `fMain->ScanKey` occurs only inside the `//` comment text.  They
//      cost this facade nothing.
//
//  (2) DISCREPANCY, REPORTED LOUDLY.  The recon's dependency item C.7 says
//      fNote needs "`bool fShow;` and `bool IsTestSitICFallDown();`".  That is
//      right about the CALL SITES and wrong about the SUBSTRATE, because it
//      never opened IsTestSitICFallDown's BODY.  Golden's body (note.cpp:5462-
//      5492) reads TWO more things that had no home here: the form's own
//      `TEdit *edErrorCode` (golden note.h:48) and the note.cpp file-scope
//      global `int iPosition` (golden note.cpp:83).  Both are landed below;
//      without them the method could only have been a fake-success stub.
//
//  (3) `int AlarmType` (golden note.h:432) is landed too even though NEITHER
//      of this front's two functions touches it.  Stated plainly so it is not
//      mistaken for a measured need: it is the one and only remaining fNote
//      member dereferenced anywhere in golden ckernel.cpp (census (1) above,
//      ShowRunLed :720), it is a plain int with a golden constructor value,
//      and this file is under a one-owner-at-a-time serialised editing regime
//      -- leaving it out guarantees a second serialised edit to this same file
//      when ShowRunLed/ShowRunLabel come off the DEFERRED list.  Its offline
//      value is still justified as a branch selection below, like every other.
//
//  (4) NOT LANDED, ON PURPOSE.  `bMyServoOffOutArm` (golden note.h:417) and
//      `t2DCode` are dereferenced by ported aoutarm9045.cpp:388/:1460 and
//      BarCode/BarCode_Bottom2DID.cpp:1180 -- but every one of those sites is
//      inside a `#if 0 // TODO(W7)` block owned by ANOTHER wave, none of them
//      is in ckernel.cpp, and un-gating them needs edits to files this wave
//      does not own.  Adding the members without un-gating the call sites
//      would grow the facade for zero reachable behaviour.  Reported to the
//      integrator instead.
// ===========================================================================
class TfNote
{
public:
    bool       bMyServoOffInArm;     // [DATA] golden note.h:417ish (bool) -- "an alarm-triggered ServoOff on the in-arm is pending recovery"
    // -----------------------------------------------------------------------
    //  AI(W906-HOME-C2) 20260920: in-arm 的三個手足。
    //
    //  golden note.h 有四個同一族的旗標，本門面先前只抄了 in-arm 那一個。
    //  歸零的本體（golden uhome.cpp:1180 ProcessMotorHome，case 1）在
    //  「歸零後要重置狀態」那一段**四個一起清**：
    //      fNote->bMyServoOffInArm=false;        // golden uhome.cpp:1287
    //      fNote->bMyServoOffOutArm=false;       // golden uhome.cpp:1288
    //      fNote->bMyServoOffOutShuttle1=false;  // golden uhome.cpp:1293
    //      fNote->bMyServoOffOutShuttle2=false;  // golden uhome.cpp:1294
    //  少三個的後果不是編不過而已 —— 是**歸零之後那三個「我手動把它
    //  servo off 過」的記號不會被清掉**，下一輪還被當成待復歸。
    //
    //  ⚠ 它們也會讓 `aoutarm.cpp:1039` 的 GATE G7 有機會退休：
    //    那一格閘住的唯一理由就是「golden `fNote->bMyServoOffOutArm`
    //    (note.h:417) 本樹沒有」。**這一波不動它** —— 退那格要連同
    //    它整段 servo-off 臂（含 `fAllMotorHome=false` 與 CC_SIGURD_PeiXing
    //    的訊息）一起量，是另一顆 commit 的事。這裡只把資料落地。
    // -----------------------------------------------------------------------
    bool       bMyServoOffOutArm;      // [DATA] golden note.h:417 族
    bool       bMyServoOffOutShuttle1; // [DATA] golden note.h:417 族（ChungHung 20140522）
    bool       bMyServoOffOutShuttle2; // [DATA] golden note.h:417 族（ChungHung 20140522）
    int        iMyServoOffInArmPosX; // [DATA] golden note.h:418 (int) -- encoder pos to jog back to (X)
    int        iMyServoOffInArmPosY; // [DATA] golden note.h:419 (int) -- encoder pos to jog back to (Y)
    AnsiString aJamCodeFilePath;     // [DATA] golden note.h:434 (AnsiString) -- SPIL FTP jam-code trace file path

    // -----------------------------------------------------------------------
    //  AI(W906-W7-L2) 20260803 ADD -- the golden ckernel.cpp surface.
    //  Every offline value below is GOLDEN'S OWN CONSTRUCTOR VALUE (read out
    //  of `__fastcall TfNote::TfNote(TComponent* Owner)`, golden note.cpp:161-
    //  onwards), not an invented "safe" default -- and each is then justified
    //  as the branch it selects, naming the golden line.
    // -----------------------------------------------------------------------

    //  [DATA] golden note.h:393 `bool fShow;`  -- "the alarm/note dialog is up".
    //  OFFLINE VALUE false, from golden note.cpp:164 `fShow=false;` in the ctor.
    //  The only two golden writers are note.cpp:2167 (`fShow=true;` in the form
    //  Show path) and note.cpp:2519 (`fShow=false;` in Reset) -- both are VCL
    //  form lifecycle with no ported home, so offline it is a permanent false.
    //  BRANCHES THAT false SELECTS (this is the point of the field, not a
    //  formality):
    //    * golden ckernel.cpp:1749 `if(fNote->fShow==false)` -- false OPENS the
    //      alarm-reset-lamp-extinguish arm at :1749-1756, i.e. DoPanelLamp DOES
    //      run `SW[SwFKAlarmReset].Off(); SW[SwRKAlarmReset].Off();` whenever
    //      bNeedMusicAndAlarmOn==false.  true would have left both lamps latched.
    //    * golden ckernel.cpp:2007/:2038/:2184/:2215 `if(fNote->fShow)` -- false
    //      takes the ELSE arms (:2029-2036, :2060-2067, :2206-2213, :2237-2244),
    //      which accept front/rear Retry and Skip UNCONDITIONALLY and never call
    //      IsTestSitICFallDown() at all.  That is golden's normal running state
    //      (no alarm dialog displayed), and it is exactly the state the W7-L2
    //      recon's DoPanelLamp test presumes (`fNote->fShow=false`).
    //  Consistent with every other offline fNote->fShow decision already taken
    //  in this tree: atester_32Site.cpp:298-302 `#define W5_32S_FNOTE_FSHOW
    //  (false)` and Automation/AGV_PortScan.h:61-70.  Those two seams are now
    //  RETIREABLE against this member -- see forms/fNote.cpp.
    bool fShow;

    //  [DATA] golden note.h:432 `int AlarmType;` -- 1=Jam / 3=MES-message class
    //  (golden note.cpp:846 has MyDBIEvent write it out through &fNote->AlarmType).
    //  OFFLINE VALUE 0, from golden note.cpp:218 `AlarmType =0;` in the ctor.
    //  BRANCH IT SELECTS: golden ckernel.cpp:720 `else if(fNote->AlarmType==3)`
    //  is FALSE at 0, so ShowRunLed -- a function this front DEFERS and does NOT
    //  translate -- falls to the else at :724-726 and sets
    //  RunState=LED_ErrJam (the jam-siren tower-light state) rather than
    //  LED_Message at :722.  0 is also not 1, which closes golden note.cpp:2528
    //  and :6765.  See banner note (3): landed ahead of its consumer on purpose.
    int AlarmType;

    //  [DATA] golden note.h:48 `TEdit *edErrorCode;` -- the note dialog's error-
    //  code box; the dfm2rc IR agrees it is a TEdit
    //  (build/.../note_layout.gen.cpp:148, "fNote.pnlNote.pnlButton.edErrorCode",
    //  class "TEdit").  Stock control, so per the facade contract rule 3 this is
    //  the unified vclcompat/Controls.h type, NOT a new bespoke struct.
    //  OFFLINE VALUE: allocated in the ctor with Text=="" (vclcompat::TCustomEdit
    //  default).  BRANCH IT SELECTS: golden note.cpp:5472's
    //  `edErrorCode->Text=="MES1640" || =="MES1641"` is FALSE on an empty Text,
    //  which CLOSES the "one-cycle-finish after a drop still needs the door open
    //  and Z1 pressed" arm of IsTestSitICFallDown (JerryYang 20220923).  It is a
    //  real settable field, not a constant, so a test can open that arm.
    vclcompat::TEdit *edErrorCode;
    // AI(W906-FW-SecUnlock) 20260819: golden note.h:49-50 -- the two fields
    // Command.cpp's MachineStatus Bit8_HandlerJam feeds into
    // fSecurity->GetBit8 ("<Edit3> <edUnitName>" is the jam-message key);
    // landed to dissolve that gate now that fSecurity is real (FW-SecCC).
    vclcompat::TEdit *Edit3;
    vclcompat::TEdit *edUnitName;

    //  [METHOD] golden note.h:430 `bool __fastcall IsTestSitICFallDown();`,
    //  body golden note.cpp:5462-5492.  FAITHFULLY TRANSLATED (not a stub) --
    //  the body is VCL-free once edErrorCode is a stand-in; see forms/fNote.cpp
    //  for the line-by-line correspondence and for the branch analysis of what
    //  its offline return value selects in ScanPannelKey.
    virtual bool IsTestSitICFallDown();

    TfNote();
    virtual ~TfNote() {}

    // =======================================================================
    //  AI(W906-FW-NOTE-W33) 20260826 ADD -- note.cpp Wave A。
    //  這行以上全部是 W906-AutoCleanFoundation / W7-L2 / FW-SecUnlock 的
    //  既有內容，一行未動。波次範圍與 GATE REGISTER 見檔頭 banner。
    //
    //  ⚠ 新成員一律用 in-class initialiser（forms/fSetup.h:477-499 慣例），
    //  因為既有的 TfNote ctor 帶顯式初始化串列而 append-only 不准改它；
    //  in-class initialiser 對「不在串列裡的成員」照樣生效，所以不需要
    //  也不應該去動那個 ctor。
    // =======================================================================
    TfNoteMDBGrid *strngrdMDBQuery = new TfNoteMDBGrid();  // golden note.h:105 (TStringGrid) -- N-D3
    vclcompat::TButton *btCloseMDBQuery = new vclcompat::TButton();  // golden note.h:150 -- 只用到 ->Visible（TControl 自帶）
    vclcompat::TEdit *edBundleID = new vclcompat::TEdit();           // golden note.h:317

    // -----------------------------------------------------------------------
    //  METHODS -- 6 支，全部唯讀方向。golden span 寫在行末。
    //  event handler 本體有翻但**不接線**（facade 規則 3）；這 6 支今天
    //  全樹 0 個 caller（重跑 20260826，排除 build*）。
    // -----------------------------------------------------------------------
    void       edtLotCountClick(vclcompat::TEdit *Sender);   // golden note.cpp:5764-5767
    void       edtLotIDClick(vclcompat::TEdit *Sender);      // golden note.cpp:5769-5772
    void       PanelMain6MouseDown();                        // golden note.cpp:5856-5862
    void       btCloseMDBQueryClick();                       // golden note.cpp:5864-5869
    AnsiString PosCodeToStr(int Pos);                        // golden note.cpp:6530-6612（宣告 note.h:457）
    void       edBundleIDChange();                           // golden note.cpp:6648-6674

    // -----------------------------------------------------------------------
    //  GATED -- 宣告在此、forms/fNote.cpp **不定義**，由 linker 當互鎖
    //  （forms/fMotorTest.h 先例）。理由逐支見檔頭 GATE REGISTER。
    //  只列出本波實際讀過 golden 本體並做出退出裁決的支數；未評估的 26 支
    //  不在這裡憑空宣告。
    // -----------------------------------------------------------------------
    void       BtnStartClick();                              // GATE (N-1)  golden :3806-3824
    void       BtnResetClick();                              // GATE (N-2)  golden :5205-5235
    void       BtnSkipClick();                               // GATE (N-3)  golden :2843-2870
    void       btnMoveToFrontClick();                        // GATE (N-4)  golden :5555-5559
    void       btnMoveToRearClick();                         // GATE (N-4)  golden :5561-5565
    void       edBundleIDKeyPress(char &Key);                // GATE (N-5)  golden :6676-6689
    void       TimerFTPTimer();                              // GATE (N-6)  golden :5567-5622
    void       AlarmLogXmlToFTP(AnsiString sAlarmCode, AnsiString sMsg); // GATE (N-6) golden :5901-5970
    void       btPrintSummaryClick();                        // GATE (N-6)  golden :5634-5659
    void       FormDestroy();                                // GATE (N-8)  golden :250-260
    void       MovieToBack();                                // GATE (N-9)  golden :5426-5434
    void       pnlMovieMouseDown();                          // GATE (N-9)  golden :5436-5440
    void       PlayMovie(AnsiString asPathFile);             // GATE (N-9)  golden :4742-4760
    void       StopMovie();                                  // GATE (N-9)  golden :4762-4770
    void       MediaPlayer1Notify();                         // GATE (N-9)  golden :4772-4776
    void       TMyTray1MouseDown();                          // GATE (N-9)  golden :5401-5424
    void       tmrKeyInTimer();                              // GATE (N-10) golden :6443-6458
    void       edBinCodeEditKeyPress(vclcompat::TEdit *Sender, char &Key); // GATE (N-10) golden :6614-6646
    bool       CheckBinCode();                               // GATE (N-10) golden :6310-6441
    void       Reset();                                      // GATE (N-11) golden :262-342
    void       ShowTrayQty();                                // GATE (N-12) golden :6066-6197
    void       FlushLabel();                                 // GATE (N-13) golden :3084-3139
    bool       DoUnlockPassword();                           // GATE (N-14) golden :5384-5399
    void       PanSpecialNoteClick();                        // GATE (N-14) golden :5442-5460
    void       palShtSensorSOPClick();                       // GATE (N-15) golden :4778-4791
    void       reDescriptionMouseDown();                     // GATE (N-16) golden :5883-5899
    void       ClearContAlarmList();                         // GATE (N-17) golden :6270-6287
};
extern TfNote *fNote;   // golden: extern PACKAGE TfNote *fNote; (note.h:462)

// AI(W906-FW-NOTE-W33) 20260826: golden note.cpp:158-159 的兩個 **file-scope
// global**（`TDateTime LastInputTime;` / `bool IsScanning=false;`），
// 不是 TfNote 的成員 —— 本波開 golden note.h 逐行查證過（note.h 的
// published/private/public 區塊都沒有這兩個名字）。
// forms/fNote.cpp 是本樹對 golden note.cpp 的 stand-in，所以 note.cpp 的
// file-scope 資料歸在這裡；真正的 note.cpp port 落地時所有權隨之搬走
// （與本檔既有的 iPosition 同一處置）。
// 宣告成 extern 是因為它們是 edBundleIDChange 的唯一狀態，測試要能觀察與驅動。
// golden 唯一的寫入者就是 edBundleIDChange 自己（:6654/:6659 寫
// LastInputTime、:6663/:6667 寫 IsScanning）—— 本波重跑確認 note.cpp 內
// 沒有其他寫入點。
extern TDateTime LastInputTime;   // golden note.cpp:158
extern bool      IsScanning;      // golden note.cpp:159

// AI(W906-W7-L2) 20260803: `iPosition` is golden's note.cpp:83 FILE-SCOPE
// GLOBAL (`int iPosition;`, external linkage, zero-initialised), not a member
// of TfNote -- kept a global here so the eventual real note.cpp port drops in
// without a signature change.  forms/fNote.cpp is this tree's stand-in for
// golden note.cpp, so note.cpp's file-scope data belongs here; when a real
// note.cpp lands, ownership of this definition moves with it.
// It is DECLARED (not just defined in the .cpp) because it is the sole
// discriminator IsTestSitICFallDown tests, so any test that wants to open that
// method's true-arms must be able to drive it.
// GOLDEN'S ONLY WRITER is `void ShowErrorUnit(int Pos)` at golden note.cpp:4511
// (`iPosition=Pos;` at :4514, declared golden note.h:469).  ShowErrorUnit has
// NO ported home anywhere in this tree: a tree-wide grep for `ShowErrorUnit`
// returns no definition and no call site -- the only textual hits are comment
// mentions (this file, and canary_support.cpp:375's "NOT reproduced" list).
// So offline iPosition never leaves its zero-init value.
// WHAT 0 SELECTS: 0 == MInArmX -- `const int MInArmX       =0;` at PORTED
// cmydef.cpp:2334 (golden cmydef.cpp:2330).  This file cites the PORTED tree for
// cmydef, consistently with forms/fNote.cpp's cmydef.cpp:2347-2350 citations.
// 0 is NOT any of MTestY1(13)/MTestZ1(14)/MTestZ2(15)/MTestY2(16)/MMIndex(503)
// nor MOutShuttle1(17)/MOutShuttle2(18) -- so golden note.cpp:5465-5468 and
// :5484-5485 both evaluate FALSE and IsTestSitICFallDown reports "no test-site
// IC fall-down", which is the correct state for a machine that has never had
// an error unit flushed onto the note panel.
extern int iPosition;

#endif // FORMS_FNOTE_H
