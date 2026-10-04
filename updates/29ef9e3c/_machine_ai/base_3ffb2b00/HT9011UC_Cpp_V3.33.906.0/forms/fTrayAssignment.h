// =============================================================================
//  forms/fTrayAssignment.h  --  non-VCL facade for golden's TfTrayAssignment
//  (cTrayAssignment.h/.cpp) -- the Tray Assignment recipe page (Loader /
//  Empty / Color / Auto1-6 / Fix1-6 tray types, buffer routing, tray
//  direction, AMR order, IC-sort counts).
//
//  AI(W906-FW-TRAYASSIGN-W23) 20260826: new file, FW wave FW-TRAYASSIGN-W23.
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/cTrayAssignment.h (311
//  lines) + cTrayAssignment.cpp (1,729 lines) + cTrayAssignment.dfm (70,861
//  bytes) -- all cp950, decoded this wave with
//  `io.open(p, encoding='cp950')`, 0 U+FFFD over all three.
//
//  ROLE
//  ----
//  TfTrayAssignment owns the whole `<DataPath><recipe>\Tray.Data` recipe
//  surface: ReadFile() is the ONLY reader of that file in the product (golden
//  main.cpp:8924, cinitial.cpp:7554 and Automation/auto9045.cpp:271 all call
//  it), FixCanUse() is what decides `Prod.iTrayType[]` -- the per-port
//  tNotUse/tTrayAuto/tTrayFix/tTrayBox map that acatchtray / aoutarm /
//  asendic_* dispatch on -- and DoIniDataToForm() pushes the whole thing into
//  the page. THIS WAVE LANDS THE FIRST COMPILED TfTrayAssignment ANYWHERE IN
//  THE PORT TREE.
//
//  THE PRE-EXISTING STAND-IN (READ THIS BEFORE WIRING ANYTHING)
//  --------------------------------------------------------------------------
//  `Automation/auto9045.cpp:344-359` holds `struct W5FA_TfTrayAssignmentExt`
//  with an `fShow` flag, three widget pointers (rgLoaderType / rgFixTrayMode /
//  ckUseFix1..3) and a NO-OP `ReadFile()`, instantiated as the TU-LOCAL global
//  `W5FA_FTrayAssignment` (:359) and used at auto9045.cpp:640-659. That is
//  the "tests/TU-local stand-in" shape of the archive-extraction trap
//  (docs/KNOWLEDGE.md): a DIFFERENT class with a DIFFERENT name, so there is
//  NO ODR conflict with this header and nothing here changes its behaviour --
//  but auto9045.cpp's tray-form-type setter still calls a `ReadFile()` THAT
//  DOES NOTHING even after this wave lands. Retiring that stand-in onto this
//  facade is a cross-file edit in Automation/ (outside this wave's write
//  boundary) and it must be done deliberately, because the real ReadFile()
//  re-reads Tray.Data and would overwrite the TrayForm fields auto9045.cpp
//  had just written.
//
//  Re-verified at wave close -- see the ABSENCE-CLAIM LEDGER at the file tail.
//
//  FORM CLASS SHAPE: plain non-VCL class, NOT TForm-derived -- same posture as
//  every translated form facade in this tree (forms/fQwertyKey.h:39-42,
//  forms/fShowBinSelect.h, forms/fSecurity.h's own survey: none re-derive
//  TForm). Widget stand-ins store data, they do not render -- "logic
//  faithful, rendering = web".
//
//  CTOR / Init() SPLIT (task-mandated; forms/fQwertyKey.h:44-56 precedent)
//  --------------------------------------------------------------------------
//  The ctor stuffs FIELDS ONLY. Golden's ENTIRE ctor body (cTrayAssignment.
//  cpp:29-60) moves into an explicit `Init()`, because it does two things a
//  static-initialisation-time ctor must never do in this tree:
//    (a) it READS TWO GLOBALS -- `bUseAuto2Empty` (cmydef.h:3391) and
//        `CosFunction.bLoaderTrayToAuto1` (CosFunction.h) -- whose own
//        initialisers live in OTHER translation units, and
//    (b) it CALLS `->Items->Add(...)` on four TRadioGroups.
//  This is exactly the shape that once cost this tree 88 of 134 SEGFAULTing
//  ctests (fLaserSensor's ctor calling InitLaserEdtList() with elLaser NULL).
//  Nothing constructs `fTrayAssignment` at static-init time here: the global
//  stays a NULL pointer, golden's own pre-CreateForm state.
//
//  WAVE SCOPE / DENOMINATOR -- measured, not quoted
//  --------------------------------------------------------------------------
//  Denominator MEASURED THIS WAVE by stripping `/* */` and `//` from the
//  cp950-decoded golden .cpp and then counting `TfTrayAssignment::`:
//  **31 occurrences, all 31 of them definitions** (no in-body qualified
//  calls). So the denominator is 31 golden methods, and the golden ctor is
//  one of them.
//
//    #   method                    golden lines   disposition
//    --  ------------------------  -------------  --------------------------
//     1  TfTrayAssignment ctor     :26-61         SPLIT -> ctor + Init()
//     2  FixCanUse                 :63-127        ACTIVE
//     3  ReadFile                  :129-470       ACTIVE, GATE (T2) x4, (T3)
//     4  DoIniDataToForm           :472-645       ACTIVE
//     5  LoadImage                 :647-664       DECLARED, BODY 100% GATE (T1)
//     6  FormShow                  :666-886       ACTIVE
//     7  imgLoaderClick            :888-908       ACTIVE (state half), GATE (T1)
//     8  ShowCompnet               :910-1068      ACTIVE, GATE (T4)
//     9  rgLoaderTypeClick         :1070-1081     ACTIVE
//    10  rgFixTrayModeClick        :1083-1112     ACTIVE
//    11  cbEmptyChange             :1114-1125     ACTIVE
//    12  ShowTrayDirectIMG         :1127-1148     ACTIVE (clamp half), GATE (T1)
//    13  RGLoaderClick             :1150-1190     ACTIVE
//    14  rgLoad_RTClick            :1192-1232     ACTIVE
//    15  cbLoaderChange            :1234-1240     ACTIVE
//    16  FormClose                 :1242-1252     ACTIVE
//    17  spbSaveClick              :1254-1316     **EXIT -- WRITE PATH (X1)**
//    18  SaveSetupFile             :1320-1542     **EXIT -- WRITE PATH (X2)**
//    19  sbtExitClick              :1544-1548     ACTIVE
//    20  sbNormalTestChange        :1550-1553     ACTIVE
//    21  GraphicToRadio            :1555-1565     ACTIVE, GATE (T1)
//    22  RadioToGraphic            :1567-1583     ACTIVE, GATE (T1)
//    23  sbNormalTest_RTChange     :1585-1588     ACTIVE
//    24  GraphicToRadio_RT         :1590-1608     ACTIVE, GATE (T1)
//    25  RadioToGraphic_RT         :1610-1626     ACTIVE, GATE (T1)
//    26  BinTrayDetect             :1630-1681     ACTIVE
//    27  RGAuto2Click              :1683-1690     ACTIVE
//    28  cbLoaderDropDown          :1692-1698     ACTIVE
//    29  edtICSort1_1MouseDown     :1700-1704     ACTIVE
//    30  FormShortCut              :1706-1713     **NOT PORTED (T5)**
//    31  cbEnableAMR_KYECClick     :1716-1726     ACTIVE
//
//  Totals: 28 of 31 golden methods ported (1 of the 28 -- LoadImage -- is a
//  declared, fully-gated body; 6 more carry a partial GATE), 2 EXITED for
//  write path, 1 NOT PORTED for a missing VCL message type. Every one of the
//  31 was opened and read line-by-line in full before this table was written.
//
//  FIDELITY VERIFICATION (run this wave, 20260826 -- statement SETS, not line
//  counts; memory note `compare-statement-sets-not-line-counts`)
//  --------------------------------------------------------------------------
//  Each ported method's golden body was sliced by the line span above, both
//  sides had `/* */` and `//` stripped, whitespace collapsed and were split
//  on `; { }`, and the resulting multisets were diffed. Result over the 27
//  ported methods with a comparable body (i.e. all 28 except LoadImage, whose
//  body is 100% gated):
//    * 15 of 27 are statement-set IDENTICAL to golden -- FixCanUse,
//      DoIniDataToForm, FormShow, rgFixTrayModeClick, cbEmptyChange,
//      RGLoaderClick, rgLoad_RTClick, FormClose, sbtExitClick,
//      sbNormalTestChange, sbNormalTest_RTChange, BinTrayDetect,
//      RGAuto2Click, cbLoaderDropDown, cbEnableAMR_KYECClick.
//    * 12 of 27 differ, and EVERY difference is one of the documented items
//      below -- there is no unexplained residue:
//        ctor+Init            the (D-2) defensive zero-fill, nothing else
//        ReadFile             4x3 (T2) Controls[] lines + the (T3) AMR arm
//        imgLoaderClick       (T1) block + (D-4) signature
//        ShowCompnet          the 3 (T4) Color lines
//        ShowTrayDirectIMG    (T1) block + (D-6) loop bound
//        GraphicToRadio / RadioToGraphic / GraphicToRadio_RT /
//        RadioToGraphic_RT    (T1) 3-line image load each
//        rgLoaderTypeClick / cbLoaderChange   (D-3) `(this)` argument drop
//        edtICSort1_1MouseDown                (D-5) cast collapse
//  Worth stating for ReadFile specifically: it is a 340-line method and the
//  ONLY divergences are the two registered gates. Nothing else moved.
//
//  SYNTAX CHECK: `g++ -fsyntax-only` with ht9045_sm's exact flags
//  (`-std=c++1z -Wall -Wextra -Wno-conversion-null -Wno-write-strings
//  -Wno-unused-parameter`, the 5 CXX_DEFINES, and
//  `@CMakeFiles/ht9045_sm.dir/includes_CXX.rsp`) -- **0 errors**, exactly 2
//  warnings, both deliberate and both golden's own: (G-b) `-Wunused-value`
//  and (G-j) `-Wparentheses`. No build.bat / cmake --build / ctest was run
//  (sibling waves own the build dir).
//
//  EXIT REGISTER -- WRITE PATH, per this wave's exit rule
//  --------------------------------------------------------------------------
//  (X1) spbSaveClick -- golden :1254-1316. Save button. Calls SaveSetupFile
//      (see X2), `fMain->BackupSetupFile()` (:1315), `EventReport(SECS_EVENT.
//      SaveRecipe)` (:1302, an outbound SECS event report), writes
//      `bHasSaveSet` (:1301) and `LastSet.OLPSetBinErr[2]` (:1307), and under
//      `#ifdef ASE_KaohSiung` calls `fBuilder->bSaveAsJobFile(S,"JOBFILE")`
//      (:1313). Two of those -- file write and SECS emit -- are exactly what
//      this wave's read-only direction excludes.
//  (X2) SaveSetupFile(szDir, S) -- golden :1320-1542. `MyForceDirectories
//      (szDir)` (:1326) CREATES the recipe directory, then ~60 `WriteIniData`
//      calls rewrite `<DataPath><recipe>\Tray.Data` (:1332-1533), one of them
//      (the `bP46_LoadTrayModeByHandler` arm, :1332) into the SHARED
//      production file `D:\HT9045\config\config.ini`, and `AddSpace(szDir)`
//      (:1539) rewrites the file again. NOT translated.
//      CONSEQUENCE, stated plainly: the five EXTERNAL golden call sites of
//      these two stay blocked -- cBuilder.cpp:548 and :566, csystem.cpp:22561
//      (SaveSetupFile) and Command.cpp:13230 / :13237 (spbSaveClick). They
//      are the write path's own wave, and per the campaign's standing ruling
//      the write path is a SAFETY-CRITICAL design item queued for the user,
//      not something a translation wave lands on the side.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (T1) TImage / TPicture -- `class TImage` and `class TPicture` have ZERO
//      compiled declaration anywhere in this tree. This is the SAME gate four
//      other facades already hold, not a new judgement: forms/fLotInfo.h:294-
//      298, forms/fTemp_Set.h:141, EJ1N/MyOmronPanel.h:93, VacuumUnit/
//      MyVacuumPanel.h:58. Inventing a local TImage here would fork away from
//      all four, so the 13 image members (imgLoader, imgAuto1..imgAuto6,
//      imgFix1..imgFix6) and the two graphic-mode images (ImgNormalTest,
//      ImgReTest) are NOT DECLARED, and every `->Picture->LoadFromFile(...)`
//      stays a gated comment quoting golden verbatim. Sites:
//        LoadImage           golden :649-663  (whole body -- 13 loads)
//        imgLoaderClick      golden :890-892, :898-907
//        ShowTrayDirectIMG   golden :1129-1132, :1137-1146
//        GraphicToRadio      golden :1557-1559
//        RadioToGraphic      golden :1580-1582
//        GraphicToRadio_RT   golden :1592-1594
//        RadioToGraphic_RT   golden :1623-1625
//      The gated blocks also carry golden's `try/catch(...)` tails, whose
//      bodies call `MyDBIProcess("Exception", ...)` (real, aHotPlateSubstrate.
//      h:933) and `ShowMyMessage(...)` (real, canary_support.h:80). Those two
//      are reachable ONLY from inside the gated try, so they are gated with
//      it -- which is also why this .cpp does NOT need aHotPlateSubstrate.h.
//
//  (T2) `rgLoaderTrayMode->Controls[0..2]->Enabled` -- ReadFile, golden
//      :303-305, :312-314, :320-322, :336-338 (four triples). Golden reaches
//      INTO a TRadioGroup for its three generated child TRadioButtons;
//      vclcompat::TRadioGroup (Controls.h:450) carries `ItemIndex` and
//      `Items` (the captions) and has NO child-control collection at all.
//      Adding one would be inventing a widget hierarchy this tree does not
//      have. OFFLINE CONSEQUENCE, stated: the Loader-Tray-Mode radio group is
//      never disabled, so a web UI wired naively to this facade would let an
//      operator change it in the SCS / KYEC_XILINX / KLT-auto-SKIP /
//      KYEC_LEE cases where golden locks it. The `->ItemIndex` writes right
//      next to each triple ARE translated, so the VALUE golden forces is
//      still forced; only the greying-out is missing.
//
//  (T3) `fMain->cbRunStartMode->Text` -- ReadFile, golden :260. forms/fMain.h
//      has NO `cbRunStartMode` member (grep 20260826: the only hit in that
//      file is a prose comment at :571). The gated line is golden's
//      Eastsun-20260512 F011 Re-Test override:
//          if(TrayForm.bEnableAMRLoader==true &&
//             (fMain->cbRunStartMode->Text=="Re-Test Continuous" ||
//              fMain->cbRunStartMode->Text=="Re-Test Initial Start"))
//              TrayForm.bEnableAMR=false;
//          else
//              TrayForm.bEnableAMR = ReadIniData(szDir,"AMR","Enahle AMR",false);
//      ARM CHOSEN: the `else` arm (read from Tray.Data), unconditionally --
//      i.e. offline the Re-Test override CAN NEVER FIRE and bEnableAMR always
//      comes from the recipe. That is golden's own behaviour whenever the run
//      mode is not one of those two strings, so it is a REACHABLE golden
//      state, not an invented one; but it is NOT equivalent, and the
//      difference is visible exactly when an AMR-Loader recipe is opened in a
//      Re-Test mode. Do not close this gate by hard-coding the guard -- close
//      it by giving fMain a real cbRunStartMode.
//      (Golden's own spelling "Enahle AMR" / "Enahle AMR Loader" is a typo in
//      the INI KEY NAME. It is load-bearing -- SaveSetupFile writes the same
//      misspelling -- so it is translated verbatim. Do not "fix" it.)
//
//  (T4) `edNoRTBinFix1..3->Color = ... ? clYellow : clWindow;` -- ShowCompnet,
//      golden :1052-1054. DOUBLE-blocked: (a) there is no SHARED `clWindow`
//      this file can include, and (b) vclcompat::TCustomEdit (Controls.h:319)
//      has no `Color` member. The three flag reads (`bNoRTBinFixFlag[0..2]`,
//      cmydef.h:5840) are REAL, so the DATA is available -- only the colouring
//      is not, and colouring is the web layer's job in this tree anyway.
//
//      ⚠ AN ABSENCE CLAIM THAT WENT STALE INSIDE THIS WAVE -- recorded
//      because it is the exact failure mode the campaign keeps paying for.
//      At wave OPEN (20260826, ~13:50) `clWindow` had zero compiled
//      definitions tree-wide. At wave CLOSE (same day, ~14:38) the re-run
//      found one: **forms/fMotorTest.cpp:78-86**, landed by the parallel
//      FW-MOTORTEST wave, `const TColor clWindow = TColor(0x80000005);` in an
//      UNNAMED namespace. That is INTERNAL LINKAGE inside a .cpp, so it is
//      still unreachable from this translation unit and the gate stands
//      unchanged -- but the blanket sentence "clWindow has zero definition
//      tree-wide" would have been FALSE by the time this file was committed,
//      so it is not written that way. That same sibling also added `int
//      Color;` to a facade-local edit wrapper (forms/fMotorTest.h:525).
//      Together those are now a working precedent for closing (T4).
//      DELIBERATELY NOT FOLLOWED HERE: duplicating a second private
//      `clWindow` hours after the first appeared, in an unintegrated sibling
//      wave, is how a constant ends up defined in five places (the
//      SckArtState lesson, forms/fSCKART.h). The right close is ONE shared
//      `vclcompat::clWindow` next to the existing colour block
//      (vclcompat/LedCore.h:52-63, which stops at clBtnFace) plus `Color` on
//      vclcompat::TCustomEdit -- both outside this wave's two-file boundary.
//      Handing that consolidation decision to the integrating main loop,
//      which can see both waves at once, is the point of writing it down.
//
//  (T5) `TWMKey` / `VK_F5` -- FormShortCut, golden :1706-1713. BOTH have ZERO
//      hits anywhere in this tree (grep 20260826: `TWMKey` 0, `VK_F5` 0,
//      `CharCode` 0), and `FormShortCut` itself has 0 ports tree-wide, so
//      there is no established shape to follow. Defining a VCL message struct
//      here would be exactly the cross-file shim this wave's rule 5 forbids,
//      and a body without the `Msg.CharCode==VK_F5` guard would silently
//      change WHEN the reset happens -- so the method is NOT DECLARED at all
//      rather than shipped as a shell. Golden body, for the wave that lands a
//      Messages-compat layer:
//          if(Msg.CharCode==VK_F5) { Left=75; Top=10; }
//      `Left`/`Top` themselves ARE declared below (FormShow golden :680-681
//      writes the same two values), so only the key dispatch is missing.
//
//  (T6) Event wiring -- no `OnClick`/`OnChange`/`OnMouseDown` delegate is
//      assigned anywhere in this file. Stock vclcompat widgets carry no
//      delegate slot (forms/fQwertyKey.h GATE (Q-D1) / cSetUp.cpp
//      GATE(G-Delegate), the established tree-wide idiom). Handler BODIES are
//      translated in full; the web write-path layer will call
//      `fTrayAssignment->RGLoaderClick()` etc. directly.
//
//  GOLDEN NOTES (faithful -- translated as found, recorded so no future reader
//  mistakes any of these for a translation defect)
//  --------------------------------------------------------------------------
//  (G-a) **THE Tag PROVENANCE TRAP -- the most consequential item in this
//      file.** `imgLoaderClick` (golden :893/:897) indexes
//      `iTrayDirect[Ptr->Tag]`, and the golden .dfm supplies those Tag values
//      at DESIGN time: imgAuto1..imgAuto6 carry `Tag = 1..6` and
//      imgFix1..imgFix6 carry `Tag = 7..12` (measured this wave directly out
//      of cTrayAssignment.dfm lines 49/89/129/648/709/771/895/956/1018/1081/
//      1121/1161), imgLoader carrying the default 0. That is provenance case
//      (b) in vclcompat/Controls.h's own Tag note: NOTHING in this port loads
//      a .dfm Tag, so `->Tag` reads 0 for every widget. A web layer that
//      wires a click straight through to this handler would therefore cycle
//      the LOADER's direction no matter which tray image was clicked. The
//      handler is translated verbatim; the fix belongs in the wiring (pass
//      the tray index, or hydrate Tag), NOT here.
//  (G-b) ReadFile golden :178: `TrayForm.bTrayUpDownSet[eFix3]==false;` uses
//      `==` where every neighbouring line uses `=`. It is a comparison whose
//      result is discarded, i.e. the Fix3 up/down flag is left at whatever it
//      was instead of being cleared. Translated VERBATIM, which makes MinGW
//      emit one `-Wunused-value` warning from this file BY DESIGN -- that
//      warning is the golden bug's own signpost. Do not silence it.
//  (G-c) ReadFile golden :393/:398/:403 assign `.iTrayType` INTO `.Direction`
//      three times (`TrayForm.Auto[i].Direction = TrayForm.Auto[eAuto1].
//      iTrayType;`) in the "avoid having no data" fallback arms. Reads like a
//      copy/paste slip; only reachable for ports whose `Prod.iTrayType[i]`
//      is tNotUse, whose Direction is then unused. Verbatim.
//  (G-d) ReadFile golden :285 already tests `CUSTOMER_CODE==CC_SCS`, so the
//      `CUSTOMER_CODE==CC_SCS ||` half of the else-if at :300 is DEAD --
//      only its `CC_KYEC_XILINX` half can ever be taken. Verbatim.
//  (G-e) GraphicToRadio (:1561-1564) sets `RGAuto3->ItemIndex = Position&0x01;
//      RGAuto2->ItemIndex = Position&0x02; RGAuto1->ItemIndex = Position&0x04;
//      RGLoader->ItemIndex = Position&0x08;` -- i.e. three of the four get 0
//      or 2/4/8, NOT 0 or 1, which is out of range for these 2-3 item radio
//      groups. Its stated inverse RadioToGraphic (:1576-1577) computes
//      `j += index[i]<<i` on the assumption every index is 0 or 1. The two
//      are therefore NOT inverses. Both verbatim; same asymmetry in the _RT
//      pair (:1604-1607 / :1619-1620).
//  (G-f) ShowTrayDirectIMG golden :1133 loops `i < sizeof(MyImage)/4` over a
//      13-pointer array -- 13 iterations under BCB6's 32-bit pointers, but 26
//      under any 64-bit build. `iTrayDirect` has eTrayCount==33 slots
//      (MachineType.h:1104), so even the 64-bit reading stays in bounds; it
//      would merely clamp 13 extra slots. Translated as the literal `13`
//      (see DEVIATION (D-6)) because the array itself is gated by (T1).
//  (G-g) FormShow duplicates two blocks outright: the `USE_ROTATE_KIT==1`
//      gbRotateKit line (:677-678 and again :740-741) and the whole
//      `CosFunction.bSpecialP24` block (:685-694 and again :743-762, the
//      second copy a superset of the first). Both copies kept.
//  (G-h) DoIniDataToForm golden :630-639 indexes `chkICSort[i]` /
//      `edtICSort[i][j]` with `i<=iAutoRight`, and SaveSetupFile golden :1524
//      does the same with `i<=iFixRight`. Both arrays are dimensioned
//      MAX_AUTO_TRAY==6 (MachineType.h:401), and iFixRight indexes the Fix
//      range (up to eFix12==17), so golden's SAVE loop is an out-of-bounds
//      write whenever a Fix port is tTrayAuto. That loop is in the EXITED
//      SaveSetupFile (X2) and is therefore not translated here; the read-side
//      `iAutoRight` loop IS translated verbatim.
//  (G-j) BinTrayDetect golden :1640-1641 writes
//      `if(RGLoader->ItemIndex==2 && iBinTray==0 || RGAuto2->ItemIndex==2 &&
//      iBinTray==1)` with no parentheses. C++ precedence binds it as
//      `(A&&B) || (C&&D)`, which IS what the surrounding code means (the
//      enclosing `if` at :1635-1638 spells the same test WITH parentheses).
//      So this is a readability defect, not a logic defect. Kept verbatim,
//      which makes MinGW emit one `-Wparentheses` warning from this file --
//      the second and last of exactly two warnings this translation produces
//      on purpose (the other is (G-b)). Both are golden's own signposts; a
//      third warning appearing later is NOT one of these.
//  (G-i) ReadFile golden :135 hard-codes `D:\HT9045\config\config.ini` and
//      reads it (only) when `IniConfig.bP46_LoadTrayModeByHandler` is set.
//      That is the SHARED production config file. READ-ONLY here
//      (`ReadIniData` -> `INIFile->ReadInteger`, common.cpp:722-732); the
//      matching WRITE at golden :1332 lives in the EXITED SaveSetupFile.
//      Verbatim, and deliberately called out because this facade must never
//      grow a write to that path outside a user-sanctioned wave.
//
//  DEVIATIONS (port-only adaptations, zero intended behaviour change)
//  --------------------------------------------------------------------------
//  (D-1) `__fastcall` + `TComponent* Owner` ctor glue dropped; class is not
//      TForm-derived (established, see FORM CLASS SHAPE).
//  (D-2) golden ctor body -> explicit `Init()` (task-mandated; see CTOR /
//      Init() SPLIT). The ctor ALSO zero-fills `iTrayDirect[]` even though
//      Init() re-does golden's own zero loop verbatim: a facade read before
//      Init() must not be indeterminate, and the duplicate write is
//      idempotent.
//  (D-3) Unread VCL glue parameters dropped per-handler (forms/
//      fConfiguration.h WA-8 / forms/fDynamicTemp.h convention; verified
//      per-function this wave that each dropped parameter is never read):
//      every `TObject *Sender` in this class is unread, as are FormClose's
//      `TCloseAction &Action` and edtICSort1_1MouseDown's Button/Shift/X/Y.
//      The only Sender that IS read is imgLoaderClick's -- see (D-4).
//      Golden's three self-calls that only existed to satisfy those
//      signatures (`cbLoaderChange(this)` :1079, `rgLoad_RTClick(this)`
//      :1238, `RGLoaderClick(this)` :1239) become plain no-argument calls.
//  (D-4) imgLoaderClick: golden's `TImage *Ptr; Ptr=(TImage *)Sender;`
//      (:890-892) becomes a `TControl *Ptr` PARAMETER. `Tag` lives on
//      vclcompat::TControl (Controls.h, hoisted there by FW-TAG1), so the
//      only member the surviving half touches is available on the base; the
//      TImage-typed half is gated by (T1). Same shape as forms/fQwertyKey.h
//      DEVIATION (D-5) (TWinControl -> TControl). Golden's `char Dir[256];`
//      (:891) is dropped with the gated sprintf that was its only user.
//  (D-5) edtICSort1_1MouseDown: golden's cast `(TEdit *)Sender` (:1703) is
//      collapsed into the signature (`TEdit *Sender`), the forms/fSetup.cpp
//      RadioButton1KeyDown precedent.
//  (D-6) ShowTrayDirectIMG's loop bound is written as the literal `13`
//      instead of golden's `sizeof(MyImage)/4`, because `MyImage[]` is a
//      TImage array gated by (T1). See (G-f) for both readings of that
//      expression and why 13 is the faithful one.
//  (D-7) golden `private:` members (iTrayDirect / iLoadDirect / LoadImage /
//      GraphicToRadio / RadioToGraphic / GraphicToRadio_RT /
//      RadioToGraphic_RT) are collapsed to public -- the established
//      convention (forms/fTemp_Set.h:566-569 citation chain).
//  (D-8) Form-level `Left` / `Top` / `Width` / `Caption` are plain fields on
//      the facade (golden TForm properties). Only these four are
//      dereferenced by golden's own methods (`Left`/`Top` FormShow :680-681,
//      `Width` ShowCompnet :1006/:1009/:1034/:1037/:1064, `Caption` FormShow
//      :674), so `Height` is deliberately absent (forms/fDynamicTemp.h "only
//      what's dereferenced" rule). Setting them moves NOTHING -- there is no
//      window; the values are stored so golden's text stays literally true
//      and observable.
//  (D-9) `ShowModal()` / `Close()` are permanent offline no-op members
//      (forms/fTemp_Set.h:575-584 / forms/fQwertyKey.h:365-366 precedent).
//      `ShowModal()` exists because golden main.cpp:27443 calls it;
//      `Close()` because sbtExitClick (:1547) does. Net offline semantics of
//      sbtExitClick = the button un-latches and nothing closes.
//
//  EXCLUDED WIDGETS (golden __published members that NO ported method
//  dereferences by name -- forms/fDynamicTemp.h "only what's dereferenced"
//  rule; listed so the omission is auditable, not silent)
//  --------------------------------------------------------------------------
//    * decorative / layout only: Panel2, Panel3, Panel5, pnlTop, pnlLoad_FT,
//      pnlLoad_RT, GroupBox1, Shape1, Shape11, gbTrayArmMode,
//      grpCheckLoaderDirection, and every numbered TLabel EXCEPT Label47 and
//      Label49 (those two ARE used, ShowCompnet :1057/:1059) -- i.e. Label1,
//      Label2, Label3..Label46, Label50, Label51, Label63, Label64 are all
//      out. (`lblHint` / `lblHint2` ARE used -- FormShow :689/:750/:756.)
//    * AMR caption labels: lblAMR_1st_KYEC, lblAMR_2nd_KYEC, lblAMR_3rd_KYEC,
//      lblAMR_TrayNo_KYEC, lblAMR_IDUse_KYEC, lblAMR_FullTrayCountCap_KYEC,
//      lblAMR_InputTrayCountCap_KYEC, lblAMR_LotTrayCountCap_KYEC,
//      GroupBox2_KYEC.
//    * `edAMRFullTrayCount_KYEC` / `edAMRInputTrayCount_KYEC` -- golden
//      declares both (cTrayAssignment.h:253-254) and NEVER reads or writes
//      either; the AMR counts golden actually moves are the non-_KYEC pair.
//      Recorded because the names invite the opposite assumption.
//    * `spbSave` -- its only deref is `spbSave->Down=false;` at golden :1541,
//      inside the EXITED SaveSetupFile (X2).
//    * 15 TImage members -- gated, see (T1).
//    * golden's own unused data members `iLoaderType` (cTrayAssignment.h:301)
//      and `iLoadDirect` (:286): grepped the whole golden tree this wave --
//      `fTrayAssignment->iLoaderType` and `->iLoadDirect` have ZERO call
//      sites and neither is touched inside cTrayAssignment.cpp. Not declared.
//    * `fShow` IS declared -- golden reads it from outside (Command.cpp:7353,
//      :9946, Automation/auto9045.cpp:252).
//
//  DEPENDENCY AUDIT (all grepped this wave, 20260826, before translating)
//  --------------------------------------------------------------------------
//    TrayForm (SYSTEM_TRAY_FORM)   REAL  cprod.h:1359 (struct :1300-1358)
//    Prod (PROD_INFO_ST)           REAL  cprod.h:1138 -- .iTrayType[] :1069,
//                                        .iIsFailT6[] :1067, .iT6CatData[]
//                                        :512, .iIfErrorT6 :520
//    LevelSet / .AccessLevel[256]  REAL  cprod.h:1153 / :1150
//    RunInfo.bLotStart             REAL  cprod.h:2818 / :2757
//    TestIF_File.bEnableTrayID2    REAL  cprod.h:2577 / :2281
//    DeviceForm_File.XDimension    REAL  cprod.h:1245 / :1168
//    UserDefForm_File[4].XPitch/YPitch  REAL  cprod.h:1364 / :1258-1259
//    ArmSpeed_File[InArm].bAutoSKIP     REAL  cprod.h:2876 / :2838;
//                                        InArm=0 MachineType.h:682
//    FT / RT                       REAL  cprod.h:3247-3248 (extern const int)
//    IniConfig (HT9045_CONFIG)     REAL  Config.h:1499
//    CosFunction                   REAL  CosFunction.h:488
//    s6TrayName[eTrayCount]        REAL  cmydef.h:138
//    eTrayCount=33 / MAX_AUTO_TRAY=6    REAL  MachineType.h:1104 / :401
//    e6TrayName eAuto1..eMag14     REAL  MachineType.h:1070-1103
//    eTrayType tNotUse..tTrayBox   REAL  MachineType.h:580-586
//    eRotateType eCynRotate / eCynRot / eMotRot / eNoUse
//                                  REAL  MachineType.h:485 / :496 / :497 / :502
//    Fix3K_UseCylinder             REAL  MachineType.h:1301
//    Type_HT9045                   REAL  MachineType.h:426
//    bcTrayAssign=3                REAL  MachineType.h:920
//    CheckRange<T>                 REAL  MachineType.h:1525 (inline template)
//    iAutoRight / iFixRight / iFixRightHalf / iMagAtAuto
//                                  REAL  cmydef.h:2923 / :2930 / :2931 / :2941
//    USE_ROTATE / USE_ROTATE_KIT / iRotate_Type / iRotate_Out_Tray6
//                                  REAL  cmydef.h:2911 / :2907 / :2910 / :2909
//    AUTO_EMPTY_COLOR / FIX3_FULL_PLACE / USE_COLOR_TRAY_SENSOR /
//    USE_TRAY_MAPPING / USE_KEYENCE_EMPTY
//                                  REAL  cmydef.h:2919 / :2964 / :2989 /
//                                        :3006 / :3019
//    CUSTOMER_CODE / MachineTypeChoice / iTestBinCount / iBinBoxType /
//    bUseAuto2Empty / bEnable_KLT_Function / iHasReadeLastData /
//    iBackupLoadDir / iBundleIn / iBundleOut / bNoRTBinFixFlag[3] /
//    iRunStartMode / AccessLevel / iDefEngineerLevel / N_INTEGER
//                                  REAL  cmydef.h:3181 / :3286 / :3396 /
//                                        :2938 / :3391 / :4528 / :4617 /
//                                        :4946 / :5691 / :5692 / :5840 /
//                                        :3112 / :3503 / :3590 / :289
//    iAMRCoverTray / iHWFix_BinBox REAL  cmydef.h (extern int; defined
//                                        cmydef.cpp)
//    MManualTray1..6               REAL  cmydef.h:2252-2254 / :2504-2506
//    bAuthCriticalPara[26]         REAL  cAuthority.h:64
//    MOT[].TrayFeedHasIC()         REAL  Motor/mymotor.h:385 / :369
//    ReadIniData(...,int/bool/AnsiString)  REAL  common.h:235-238 (read-only:
//                                        common.cpp:692-757 -> INIFile->Read*)
//    GetLastOpenFN() / DataPath / BmpPath  REAL  common.h:301 / :72 / :74
//    HasICUnderMachine / HasAnyICInMachine REAL  csystem.h:105 / :109
//    ShowMyMessage(S1,S2,...)      REAL  canary_support.h:80
//    Barcode_Reader(int)           REAL  BarcodeReader.h:111
//    fSecurity->Insufficient(int,bool)     REAL  forms/fSecurity.h:561 / :595
//    fShowBinSelect->SetAutoVisible()      REAL  forms/fShowBinSelect.h:969 /
//                                        :1081
//    fMain->hanaART->IsHanaArtAvailable()  REAL  forms/fMain.h:188 / :83
//    fQwertyKey->ShowQwertyKey(...) REAL  forms/fQwertyKey.h:370 / :406
//    vclcompat::TScrollBar         REAL  vclcompat/ScrollBar.h:93 -- spelled
//                                        FULLY QUALIFIED on purpose: that
//                                        header deliberately does NOT enter
//                                        the global namespace because
//                                        handlerlog.h:122 already owns
//                                        `::TScrollBar` (see its NAME
//                                        COLLISION note).
//    TPanel / TGroupBox / TEdit / TCheckBox / TComboBox / TRadioGroup /
//    TRadioButton / TSpeedButton / TPageControl / TTabSheet / TLabel /
//    TControl                      REAL  vclcompat/Controls.h
//
//  CMAKE STATUS -- NOT WIRED by this wave (CMakeLists.txt is outside the write
//  boundary). SUGGESTED landing: **ht9045_sm** (CMakeLists.txt:1353 block,
//  next to cSetUp.cpp / cShowBinSelect.cpp), NOT ht9045_forms. Reason,
//  measured: this .cpp needs MOT[]/TTrayMotor (Motor/mymotor.h),
//  HasICUnderMachine / HasAnyICInMachine (csystem.h) and fShowBinSelect, none
//  of which are inside the ht9045_forms link diet (vclcompat +
//  ht9045_globals + ht9045_core -- `target_link_libraries(ht9045_forms ...)`,
//  CMakeLists.txt:714-720, read this wave). Landing it in ht9045_forms would
//  create exactly the undeclared forms->sm archive back-edge that
//  CMakeLists.txt:612-646 ("NO UNDECLARED BACK-EDGE") records as a past
//  defect -- one that linked anyway for a while purely because GNU ld
//  extracts archive members lazily.
//    Corollary the wiring wave must decide: the `fTrayAssignment` GLOBAL is
//    read by ht9045_forms-resident consumers in golden (ProductionInfo, the
//    SECSGEM SV table). Whoever un-gates those faces the same layering
//    question forms/fQwertyKey.h already documented for fQwertyKey.
// =============================================================================
#ifndef FORMS_FTRAYASSIGNMENT_H
#define FORMS_FTRAYASSIGNMENT_H

#include "vclcompat/vcl_compat.h"    // AnsiString / TObject at global scope
#include "vclcompat/Controls.h"      // TPanel/TGroupBox/TEdit/TCheckBox/TComboBox/
                                     // TRadioGroup/TRadioButton/TSpeedButton/
                                     // TPageControl/TTabSheet/TLabel/TControl
#include "vclcompat/ScrollBar.h"     // vclcompat::TScrollBar (fully qualified -- see audit)
#include "MachineType.h"             // eTrayCount (array dim), MAX_AUTO_TRAY

// ---------------------------------------------------------------------------
//  TfTrayAssignmentPageControl -- facade-only wrapper (compose, don't fork
//  vclcompat -- forms/fShowBinSelect.h:772 precedent, same shape).
//  vclcompat::TPageControl carries only ->ActivePageIndex (an int); golden
//  ALSO compares `pgRunMode->ActivePage` against a TTabSheet* --
//  GraphicToRadio_RT, golden :1598 (and spbSaveClick :1271-1272, EXITED).
//  Default nullptr: golden's own pre-.dfm-load state.
// ---------------------------------------------------------------------------
class TfTrayAssignmentPageControl : public vclcompat::TPageControl
{
public:
    TTabSheet *ActivePage = nullptr;
};

// ===========================================================================
//  TfTrayAssignment -- non-VCL facade
//  (golden cTrayAssignment.h:13-306, `TfTrayAssignment : public TForm`)
// ===========================================================================
class TfTrayAssignment
{
public:
    TfTrayAssignment();   // PORT: fields only -- golden ctor body lives in Init()
    void Init();          // golden ctor body :29-60 (see CTOR / Init() SPLIT)

    // -- top-level containers (golden h:16, h:229) ---------------------------
    TPanel *palTrayAssign = new TPanel();     // FormShow :789/:795/:801/:811/:817/:827/:852
    TPanel *palAMR        = new TPanel();     // ShowCompnet :1010/:1014/:1038/:1065 (->Left)

    // -- Loader / Empty / Color group (golden h:23-29) -----------------------
    TGroupBox *gbColorTray = new TGroupBox(); // ShowCompnet :913
    TGroupBox *gbEmptyTray = new TGroupBox(); // ShowCompnet :912
    TComboBox *cbLoader    = new TComboBox();
    TComboBox *cbEmpty     = new TComboBox();
    TComboBox *cbColor     = new TComboBox();

    // -- Auto1..6 groups (golden h:17-22, h:207-218) -------------------------
    TGroupBox *gbAuto1 = new TGroupBox(), *gbAuto2 = new TGroupBox();
    TGroupBox *gbAuto3 = new TGroupBox(), *gbAuto4 = new TGroupBox();
    TGroupBox *gbAuto5 = new TGroupBox(), *gbAuto6 = new TGroupBox();
    TEdit *edAuto1Type = new TEdit(), *edAuto2Type = new TEdit();
    TEdit *edAuto3Type = new TEdit(), *edAuto4Type = new TEdit();
    TEdit *edAuto5Type = new TEdit(), *edAuto6Type = new TEdit();
    TCheckBox *chkTrayEndRecvAuto1 = new TCheckBox(), *chkTrayEndRecvAuto2 = new TCheckBox();
    TCheckBox *chkTrayEndRecvAuto3 = new TCheckBox(), *chkTrayEndRecvAuto4 = new TCheckBox();
    TCheckBox *chkTrayEndRecvAuto5 = new TCheckBox(), *chkTrayEndRecvAuto6 = new TCheckBox();

    // -- run-mode tab host (golden h:33-53) ----------------------------------
    TfTrayAssignmentPageControl *pgRunMode = new TfTrayAssignmentPageControl();
    TTabSheet *tsNormalTestGroup = new TTabSheet(), *tsReTestGroup = new TTabSheet();
    TTabSheet *tsNormalTestGraph = new TTabSheet(), *tsReTestGraph = new TTabSheet();
    TPanel    *pnlFTTrayFrom     = new TPanel(),    *pnlRTTrayFrom = new TPanel();
    // ImgNormalTest / ImgReTest are GATE (T1) -- not declared.
    vclcompat::TScrollBar *sbNormalTest    = new vclcompat::TScrollBar();
    vclcompat::TScrollBar *sbNormalTest_RT = new vclcompat::TScrollBar();

    // -- FT / RT buffer routing radio groups (golden h:39-47, h:188,
    //    h:204-206, h:220-226) ----------------------------------------------
    TRadioGroup *RGLoader = new TRadioGroup(), *rgLoad_RT  = new TRadioGroup();
    TRadioGroup *RGAuto1  = new TRadioGroup(), *rgAuto1_RT = new TRadioGroup();
    TRadioGroup *RGAuto2  = new TRadioGroup(), *rgAuto2_RT = new TRadioGroup();
    TRadioGroup *RGAuto3  = new TRadioGroup(), *rgAuto3_RT = new TRadioGroup();
    TRadioGroup *RGAuto4  = new TRadioGroup(), *rgAuto4_RT = new TRadioGroup();
    TRadioGroup *RGAuto5  = new TRadioGroup(), *rgAuto5_RT = new TRadioGroup();
    TRadioGroup *RGAuto6  = new TRadioGroup(), *rgAuto6_RT = new TRadioGroup();
    TLabel      *lblHint  = new TLabel(),      *lblHint2   = new TLabel();
    TRadioButton *rbTemp  = new TRadioButton();   // FormShow :868 (->SetFocus)

    // -- exit strip (golden h:55) -------------------------------------------
    TSpeedButton *sbtExit = new TSpeedButton();
    // spbSave (golden h:56): EXCLUDED -- only deref is inside EXITED
    // SaveSetupFile (X2), golden :1541.

    // -- mode / option group boxes (golden h:62-70, h:163, h:179-184, h:228) -
    TCheckBox   *chkCheckLoadDirection = new TCheckBox();
    TGroupBox   *gbColorTrayMode       = new TGroupBox();
    TCheckBox   *chkColorTray          = new TCheckBox();
    TGroupBox   *gbUnloadMode          = new TGroupBox();
    TCheckBox   *chkAutoTrayFeed       = new TCheckBox();
    TCheckBox   *cbManuTakeAutoFailTray_FT = new TCheckBox();
    TCheckBox   *cbManuTakeAutoFailTray_RT = new TCheckBox();
    TRadioGroup *rgLoaderTrayMode      = new TRadioGroup();
    TRadioGroup *rgLoaderType          = new TRadioGroup();
    TRadioGroup *rgFixTrayMode         = new TRadioGroup();
    TGroupBox   *gbRotateKit           = new TGroupBox();
    TComboBox   *cbInputRotateKIT      = new TComboBox();
    TComboBox   *cbOutputRotateKIT     = new TComboBox();
    TCheckBox   *cbMoveAfterTrayGoOut  = new TCheckBox();

    // -- IC-sort count panel (golden h:71-161) ------------------------------
    TGroupBox *grpICSortFunc = new TGroupBox();
    TGroupBox *grpICSort1 = new TGroupBox(), *grpICSort2 = new TGroupBox();
    TGroupBox *grpICSort3 = new TGroupBox(), *grpICSort4 = new TGroupBox();
    TGroupBox *grpICSort5 = new TGroupBox(), *grpICSort6 = new TGroupBox();
    TCheckBox *chkICSort1 = new TCheckBox(), *chkICSort2 = new TCheckBox();
    TCheckBox *chkICSort3 = new TCheckBox(), *chkICSort4 = new TCheckBox();
    TCheckBox *chkICSort5 = new TCheckBox(), *chkICSort6 = new TCheckBox();
    TEdit *edtICSort1_1 = new TEdit(), *edtICSort1_2 = new TEdit(), *edtICSort1_3 = new TEdit();
    TEdit *edtICSort1_4 = new TEdit(), *edtICSort1_5 = new TEdit(), *edtICSort1_6 = new TEdit();
    TEdit *edtICSort2_1 = new TEdit(), *edtICSort2_2 = new TEdit(), *edtICSort2_3 = new TEdit();
    TEdit *edtICSort2_4 = new TEdit(), *edtICSort2_5 = new TEdit(), *edtICSort2_6 = new TEdit();
    TEdit *edtICSort3_1 = new TEdit(), *edtICSort3_2 = new TEdit(), *edtICSort3_3 = new TEdit();
    TEdit *edtICSort3_4 = new TEdit(), *edtICSort3_5 = new TEdit(), *edtICSort3_6 = new TEdit();
    TEdit *edtICSort4_1 = new TEdit(), *edtICSort4_2 = new TEdit(), *edtICSort4_3 = new TEdit();
    TEdit *edtICSort4_4 = new TEdit(), *edtICSort4_5 = new TEdit(), *edtICSort4_6 = new TEdit();
    TEdit *edtICSort5_1 = new TEdit(), *edtICSort5_2 = new TEdit(), *edtICSort5_3 = new TEdit();
    TEdit *edtICSort5_4 = new TEdit(), *edtICSort5_5 = new TEdit(), *edtICSort5_6 = new TEdit();
    TEdit *edtICSort6_1 = new TEdit(), *edtICSort6_2 = new TEdit(), *edtICSort6_3 = new TEdit();
    TEdit *edtICSort6_4 = new TEdit(), *edtICSort6_5 = new TEdit(), *edtICSort6_6 = new TEdit();

    // -- Fix1..6 groups (golden h:164-203) ----------------------------------
    TGroupBox *grpFix1 = new TGroupBox(), *grpFix2 = new TGroupBox();
    TGroupBox *grpFix3 = new TGroupBox(), *grpFix4 = new TGroupBox();
    TGroupBox *grpFix5 = new TGroupBox(), *grpFix6 = new TGroupBox();
    TLabel    *labFix1 = new TLabel(),    *labFix2 = new TLabel();
    TLabel    *labFix3 = new TLabel(),    *labFix4 = new TLabel();
    TLabel    *labFix5 = new TLabel(),    *labFix6 = new TLabel();
    TComboBox *cbFix1  = new TComboBox(), *cbFix2  = new TComboBox();
    TComboBox *cbFix3  = new TComboBox(), *cbFix4  = new TComboBox();
    TComboBox *cbFix5  = new TComboBox(), *cbFix6  = new TComboBox();
    TCheckBox *ckUseFix1 = new TCheckBox(), *ckUseFix2 = new TCheckBox();
    TCheckBox *ckUseFix3 = new TCheckBox(), *ckUseFix4 = new TCheckBox();
    TCheckBox *ckUseFix5 = new TCheckBox(), *ckUseFix6 = new TCheckBox();

    // -- AMR / VTest strip (golden h:230-263) -------------------------------
    TCheckBox *cbSpecTrayCnt       = new TCheckBox();
    TLabel    *Label47             = new TLabel();       // ShowCompnet :1057
    TEdit     *edAMRInputTrayCount = new TEdit();
    TLabel    *Label49             = new TLabel();       // ShowCompnet :1059
    TEdit     *edAMRFullTrayCount  = new TEdit();
    TGroupBox *grpVTestNoTBin      = new TGroupBox();    // ShowCompnet :1062-1063
    TEdit     *edNoRTBinFix1       = new TEdit();
    TEdit     *edNoRTBinFix2       = new TEdit();
    TEdit     *edNoRTBinFix3       = new TEdit();
    TCheckBox *cbVTestNoRTBin      = new TCheckBox();
    TCheckBox   *cbEnableAMR_KYEC       = new TCheckBox();                      //Eastsun 20260512 F009 整合
    TCheckBox   *cbEnableAMRLoader_KYEC = new TCheckBox();                      //Eastsun 20260512 F009 整合
    TRadioGroup *rgAMRReaderPos_KYEC    = new TRadioGroup();                    //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTray1st_KYEC      = new TComboBox();                      //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTray2nd_KYEC      = new TComboBox();                      //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTray3rd_KYEC      = new TComboBox();                      //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTrayID1st_KYEC    = new TComboBox();                      //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTrayID2nd_KYEC    = new TComboBox();                      //Eastsun 20260512 F009 整合
    TComboBox   *cbAMRTrayID3rd_KYEC    = new TComboBox();                      //Eastsun 20260512 F009 整合

    // -- golden private: User declarations (h:285-291), public collapse (D-7)
    int iTrayDirect[eTrayCount];   // 0=Loader, 1..6=Auto1..6, 7..12=Fix1..6 (see G-a)
    // iLoadDirect (golden h:286): EXCLUDED -- zero derefs golden-tree-wide.

    // -- golden public: User declarations (h:293-305) ------------------------
    bool fShow = false;                        // read externally (see EXCLUDED)
    TCheckBox *chkICSort[MAX_AUTO_TRAY];       // golden h:303, filled by Init()
    TEdit     *edtICSort[MAX_AUTO_TRAY][6];    // golden h:304, filled by Init()
    // iLoaderType (golden h:301): EXCLUDED -- zero derefs golden-tree-wide.

    // -- form-level fields / no-ops (DEVIATIONS (D-8)/(D-9)) -----------------
    int        Left  = 0;   // FormShow :680 writes 75
    int        Top   = 0;   // FormShow :681 writes 10
    int        Width = 0;   // ShowCompnet :1006/:1034 write 1280 / 861
    AnsiString Caption;     // FormShow :674
    void ShowModal() {}     // golden TForm::ShowModal -- offline no-op (D-9)
    void Close()     {}     // golden TForm::Close     -- offline no-op (D-9)

    // -- methods (golden line spans in the WAVE SCOPE table above) -----------
    void FixCanUse();                              // golden :63-127
    void ReadFile();                               // golden :129-470
    void DoIniDataToForm();                        // golden :472-645
    void LoadImage();                              // golden :647-664 (body = GATE (T1))
    void FormShow();                               // golden :666-886
    void imgLoaderClick(TControl *Ptr);            // golden :888-908 (D-4)
    void ShowCompnet();                            // golden :910-1068
    void rgLoaderTypeClick();                      // golden :1070-1081
    void rgFixTrayModeClick();                     // golden :1083-1112
    void cbEmptyChange();                          // golden :1114-1125
    void ShowTrayDirectIMG();                      // golden :1127-1148
    void RGLoaderClick();                          // golden :1150-1190
    void rgLoad_RTClick();                         // golden :1192-1232
    void cbLoaderChange();                         // golden :1234-1240
    void FormClose();                              // golden :1242-1252
    void sbtExitClick();                           // golden :1544-1548
    void sbNormalTestChange();                     // golden :1550-1553
    void GraphicToRadio();                         // golden :1555-1565
    void RadioToGraphic();                         // golden :1567-1583
    void sbNormalTest_RTChange();                  // golden :1585-1588
    void GraphicToRadio_RT();                      // golden :1590-1608
    void RadioToGraphic_RT();                      // golden :1610-1626
    int  BinTrayDetect(int iBinTray, int bAuto12); // golden :1630-1681
    void RGAuto2Click();                           // golden :1683-1690
    void cbLoaderDropDown();                       // golden :1692-1698
    void edtICSort1_1MouseDown(TEdit *Sender);     // golden :1700-1704 (D-5)
    void cbEnableAMR_KYECClick();                  // golden :1716-1726
    // spbSaveClick   golden :1254-1316 -- EXIT (X1), write path.  NOT declared.
    // SaveSetupFile  golden :1320-1542 -- EXIT (X2), write path.  NOT declared.
    // FormShortCut   golden :1706-1713 -- GATE (T5), no TWMKey.   NOT declared.
};

// ---------------------------------------------------------------------------
//  ABSENCE-CLAIM LEDGER -- commands actually run, and when.  Re-run at every
//  hand-off: absence claims in this tree go stale under parallel waves
//  (docs/KNOWLEDGE.md; a "fQwertyKey has no port" claim was only two days old
//  when it wrongly blocked a whole batch of methods).
//    20260826 -- run at wave open AND re-run at wave close:
//      `grep -rn "TfTrayAssignment\|fTrayAssignment" --include=*.h
//        --include=*.cpp .`
//         -> outside build/ and tools/dfm2rc data artifacts, the ONLY hits are
//            Automation/auto9045.cpp:17 (a comment), :336-359 (the TU-local
//            W5FA_TfTrayAssignmentExt stand-in) and :640-659 (its uses).
//            0 compiled `TfTrayAssignment` declarations before this file.
//            See THE PRE-EXISTING STAND-IN above.
//      `Grep "class (TImage|TPicture|TScrollBar|TShape)\b" --glob *.h`
//         -> TImage 0, TPicture 0, TShape 0; TScrollBar 2 (handlerlog.h:122
//            global stand-in + vclcompat/ScrollBar.h:93, namespaced).
//      `\bTWMKey\b` / `\bVK_F5\b` / `\bCharCode\b` over every .h/.cpp
//         -> 18 / 9 / 10 raw hits, and at wave close EVERY hit outside this
//            file is PROSE IN A COMMENT recording the same gap (cSetUp.cpp:250,
//            cBinSel.cpp:4222, uTemp_Set.cpp:88/:6618-6619,
//            uYieldMonitoring.cpp:2614-2616, forms/fSetup.h:124-127,
//            forms/fTemp_Set.h:87-88).  0 declarations, 0 uses.  GATE (T5)
//            therefore matches what four sibling facades already decided.
//      `\bclWindow\b` -> STALE BY WAVE CLOSE.  See GATE (T4): one TU-local
//            definition appeared during this wave (forms/fMotorTest.cpp:86,
//            unnamed namespace).  Gate unchanged; wording corrected.
//      `grep -n "cbRunStartMode" forms/fMain.h`  -> 1 hit, a prose comment
//            (:571).  No member.  Re-checked at close.
//      `(class|struct) TfTrayAssignment` / `extern .*TfTrayAssignment`
//         -> 0 outside this file.  Command.cpp:10244's
//            `fTrayAssignment->fShow` is inside an `#if 0`
//            (TfMain::SettingsIsWindowOpened, Command.cpp:10238), and
//            cinitial.cpp:9176-9177's `fTrayAssignment->ReadFile()` likewise.
//            Both are un-gate candidates for the wiring wave.
//      `grep -rn "class TfTrayAssignmentPageControl"`  -> 0 outside this file.
//    Known-PRESENT (do NOT re-claim absent): fQwertyKey (forms/fQwertyKey.h),
//    ShowModal (forms/fQwertyKey.h:365), Barcode_Reader (BarcodeReader.h:111,
//    body BarcodeReader.cpp:445), TShiftState / TMouseButton
//    (vclcompat/ShiftState.h) -- the last pair are unused here only because
//    (D-3) drops the parameters that would have needed them.
//
//  AI(W906-FW-TRAYASSIGN-W23) 20260826: golden cTrayAssignment.h:308
//  `extern PACKAGE TfTrayAssignment *fTrayAssignment;` -- PACKAGE glue
//  dropped.  Defined (zero-initialised, NEVER constructed at static init --
//  golden's own pre-CreateForm state) in forms/fTrayAssignment.cpp.
// ---------------------------------------------------------------------------
extern TfTrayAssignment *fTrayAssignment;

#endif // FORMS_FTRAYASSIGNMENT_H
