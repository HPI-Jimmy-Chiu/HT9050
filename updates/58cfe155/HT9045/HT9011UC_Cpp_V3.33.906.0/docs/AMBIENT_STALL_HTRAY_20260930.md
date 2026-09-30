# amb0930a SIM run: why production stalls 2 minutes after START

Run: SIM `wb_serve`, operator script Initial Start, HOME, Lot Start, START at about 11:01. Duration 930 s. Tag `amb0930a`.
Port tree: `D:\HT9045\.claude\worktrees\m0925\HT9011UC_Cpp_V3.33.906.0`. The run was on fb46c597. The worktree HEAD is now 0b8480c5, and `git diff fb46c597 HEAD` is empty for `Motor/`, `cinitial.cpp`, `ainarm9045.cpp`, `acatchtray.cpp` and `asendic_Loader.cpp`.
Golden: `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`, read through `iconv -c -f cp950`.
Nothing in either tree was edited, built or run for this report. It was written from source reads plus the recorded run data.

## 1. Verdict in one paragraph

The two tracers found one defect, seen from two sides.

The loader stage motor `MOT[MMTrayY]` is supposed to receive a full IC grid when a tray is supplied, through `SetTray(HAS_IC)`. In the port it gets the "has a tray" flag but an empty grid. The reason is that `TTrayMotor::fHTary` is never set to true, and `SetTray` only fills the grid when `fHTary` is true.

In golden, `fHTary` is set by `SetHTrayPanel()`, which `SetSimuScreenPara()` calls once at boot for 50 tray motors. The port has two breaks here:
- `SetHTrayPanel()` is a no-op.
- All 50 calls sit inside `#if 0 // GATE n5-G3`. That gate is justified as "would change no machine state", which is false.

Both consumers of the loader tray then do the golden-correct thing with a wrong state:
- The in-arm pick state machine sees "no IC" and parks at the wait position, looping 10 <-> 15.
- The tray arm sees "tray present and empty", which is golden's tray-end condition, so it carries the tray to the buffer.
- The loader then supplies the next tray, and the cycle repeats.

That is why only `MTrayX` keeps moving. Classification: **translation defect**. Golden SIM does not stall here. The golden-side real-machine record shows the grid filled after the same supply step.

## 2. The handshake chain (who waits for whom)

| # | Role | Task / variable | Port | Golden |
|---|---|---|---|---|
| 1 | Producer: stack to car | `LoadTask` (DoLoad) runs 600 -> 800 (`CheckBox1`/`ALed1` SIM feed) -> 900 `DoLoadNewICTray` / `DoTrayZLoadTrayToWait`, which ends with `MOT[MMTrayY_Car].SetTray(HAS_IC)` | `asendic_Loader.cpp:2719` (DoLoad), `:2293`, `:1745` | `asendic_Loader.cpp:2448` |
| 2 | Producer: car to loader stage | `SupplyNewIC_From_LoaderCar` = `DoSupplyNewICTray`; case 1300 default arm `MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);` (INSTALL_OCR=0, TrayMap=0, bEnableAMR=0, so the final `else` is taken) | fn `:426`, case `:1267`, call `:1401` | fn `:155`, case `:996`, call `:1130` |
| 3 | Shared state (THE DEFECT) | `TTrayMotor::SetTray`: `fHasTray = true; if (fHTary) { fHasTray = true; InitNewTray(data, false, Func); }`, and InitNewTray does `Tray.ClearData(); Tray.SetData(data);` | `Motor/mymotor.cpp:2389-2397`, InitNewTray `:2326-2330` | `Motor/mymotor.cpp:1506-1514`, InitNewTray `:1192-1196` |
| 3a | Who sets `fHTary` | Golden: `void TTrayMotor::SetHTrayPanel(TTMyTray *ptr) { fHTary=true; pHTray=ptr; }`. Port: the body is `(void)ptr; // fHTary = true; // pHTray = ptr;`, and the only assignment in the whole tree is the ctor's `fHTary = false;` | `Motor/mymotor.cpp:1949-1957`, `:1942` | `Motor/mymotor.cpp:1500-1504` |
| 3b | Who calls SetHTrayPanel | `SetSimuScreenPara()` (one-shot `if(flag)`) -> 50 calls, including `MOT[MMTrayY].SetHTrayPanel(fMain->mtLoader);`. Port: the calls sit inside `#if 0 // GATE n5-G3` | body `cinitial.cpp:13851`, `if(flag)` `:14020`, gate `:14088-14581`, MMTrayY `:14522`, `flag=false` `:14583` | `cinitial.cpp:5855`, `if(flag)` `:5997`, calls `:6430-6464`, `:6468-6471`, `:6476-6489`, `flag=false` `:6492` |
| 3c | Boot path to 3b | Port: `SetWorkParameter()` `cinitial.cpp:7084` -> `SetSimuScreenPara()` `:7227`. This is live; wb_serve stdout:126 shows "SetWorkParameter (after reads) -> true". Golden: `TfMain::FormShow` (main.cpp:9141) -> `InitialHandler()` main.cpp:9559 -> `SetWorkParameter()` cinitial.cpp:5845 -> `SetSimuScreenPara()` cinitial.cpp:13503 | see left | see left |
| 4 | Consumer A: in-arm pick | `InArmPickFromLoadTask` = `iPickFromLoadStageTask` (`int &Task=iPickFromLoadStageTask;`). Case 10 `if(MOT[MMTrayY].HasIC())` is false -> `MoveInArm2XYToWait()` -> `Task=15`. Case 15 `if(MOT[MMTrayY_Car].fHasTray \|\| MOT[MMTrayY].fHasTray \|\| MOT[MMTrayZ].fHasTray) { Task=10; return false; }` | alias `cStateRecord.cpp:1675`; fn `ainarm9045.cpp:2459`, `:2482`, case 10 `:2559-2613` (else arm `:2586-2610`), case 15 `:2643-2664` | fn `ainarm9045.cpp:7619`, `:7642`, case 10 `:7719`, case 15 `:7803-7824` |
| 4a | HasIC used by case 10 | `TTrayMotor::HasIC`: `if(fHasTray==false) return false; ... else if (Tray.HasIC()) return true;`. `TMyTray::HasIC` scans `Data[][]` | `Motor/mymotor.cpp:1960-1976`, `mytray.cpp:231-238` | `Motor/mymotor.cpp:1054-1075` |
| 5 | Consumer B: tray arm | `CatchTrayTask` (DoCatchTray) case 100: `if(MOT[MMTrayY].Tray.HasIC()==false && MOT[MMTrayY].fHasTray==true && bPlaceToHotplate==false && InArmXYZSafe()==true)` -> 200 (InitialCatchFromLoader) -> 250 `DoCatchFromLoader` -> 2000 / 2120 `DoPlaceToBuffer` -> 2130..2160 -> 1. The `else` goes to 300 (or 1100 in cassette mode) | fn `acatchtray.cpp:6134`, case 100 `:6309`, condition `:6331-6334`, `else` `:6563-6579`, case 200 `:6721`, 250 `:6794` | fn `acatchtray.cpp:6000`, case 100 `:6175`, condition `:6197-6200`; DoCatchFromLoader `:1487` |
| 6 | Loop closes | Consumer B removes the tray (`TrayArmCatchFromLoaderTask`, `TrayArmPlaceToBufferTask`). LoadTask 600 -> 1000 then supplies the next one (back to step 2). A tray is almost always on the car, TrayZ or TrayY, so case 15 always takes its `Task=10` arm and never reaches `DoAutoSkipCheck()` -> `Task=1` | as above | as above |

So nobody waits on a missing event. Both consumers are correct translations reading a state that the producer's write left half-done. Consumer A would wait forever; consumer B makes sure the wait never ends by throwing each tray away.

## 3. The single blocking fact

**`MOT[MMTrayY].fHasTray == true && MOT[MMTrayY].Tray.HasIC() == false` right after every `DoSupplyNewICTray` case 1300.** The cause is that `TTrayMotor::fHTary` is false on every tray motor in the port, so `SetTray(HAS_IC)` never reaches `InitNewTray` -> `Tray.SetData(HAS_IC)`.

### Evidence in the run data

All counts are merged over the 9 snapshots with a throwaway parser that was deleted after use.

1. **Directly measured.** The `tasklist_600.csv` flag rows (DecisionVariables generated 11:10:25) give `MOT[MMTrayY].fHasTray, true` and `MOT[MMTrayY].Tray.HasIC(), false`. At that moment:
   - the last SupplyNewIC 1300 was at 11:10:14.958;
   - `TrayArmCatchFromLoaderTask` was at 160, so the tray was still on the stage;
   - `CatchTrayTask` was at 250.

   The other 8 snapshots were taken while no tray was on the stage (`fHasTray=false`).
2. **In-arm never sees IC.** `InArmPickFromLoadTask` has 349 entries and only the values {1, 10, 15}. Its transitions are 10->15 173 times and 15->10 173 times. It never takes 15->1, and it never reaches 12, 20 or 200. The 10->15 step is reachable only through the `HasIC()==false` arm (ainarm9045.cpp:2586-2609). It continues after every supply: for example, 1300 at 11:10:14.958 is followed by 10->15 at 11:10:15.957.
3. **Tray arm judges every new tray empty.** There are 18 `SupplyNewIC_From_LoaderCar` 1300 entries (first 11:03:40.623, last 11:14:27.722). All 18 are followed by `CatchTrayTask` 200, and in all 18 the entry before the 200 is 100. In DoCatchTray case 100, `Task=200` is written only inside the `Tray.HasIC()==false && fHasTray==true` block (`:6388`, `:6399`, `:6410`, `:6483`, `:6560`). The first cycle took 62 s because CatchTray was busy in 400..1100. Later cycles took 0.5 to 3.5 s. Examples: 1300 at 11:10:14.958 -> 200 at 11:10:15.957; 1300 at 11:14:27.722 -> 200 at 11:14:30.573.
4. **Axes.** `api.jsonl` has 851 samples. `MInArmX` and `MInArmY` last change at t=182.6 s (about 11:03:23). Their final cmdPos is 5326 / -48898, exactly `Prod.iInArmSafeX` / `Prod.iInArmSafeY` in the `tasklist_870` flag rows, i.e. the wait position from `MoveInArm2XYToWait()`. `MTrayX` changes 46 times, up to t=868.8 s. These axes never change at all: MOutShuttle1/2, MLoaderZ, MEmptyZ, MAuto1Z..3Z, MInRotate, MOutRotate, MCCDY.
5. **Static.** Grep of `fHTary|pHTray` over the port tree (Grep tool, 20260930):
   - the only assignment is `mymotor.cpp:1942` `fHTary = false;`, apart from commented lines `:1955-1956`;
   - every `pHTray->` is either commented or inside `#if 0 // AI-W6C-GOLDEN-BEGIN InitNewTray` (`mymotor.cpp:2074-2323`);
   - `pHTray` is `protected` (`mymotor.h:320-332`), so no other file touches it.

### Golden-side witness (real machine, same recipe family)

Record: `D:\HT9045\Staterecord\2025-12-11 17_47_57\Task_ListWithTime.csv` (V3.33.880):
- `SupplyNewIC_From_LoaderCar` reaches 1300 at 17:37:05.500, and the flag rows then read `MOT[MMTrayY].Tray.HasIC(), true` / `MOT[MMTrayY].fHasTray, true`.
- `CatchTrayTask` then only alternates 400/600 (500 entries) and never goes 100 -> 200, so the tray is not taken away.

This is the same `SetTray(HAS_IC)` filling the grid in golden. That record is a different hang (InArmTask stays at 15000, InArmPickFromLoadTask stays 1), so it is not a production pick baseline. It does prove the loader-grid state that the port lacks.

## 4. Classification

**translation-defect.** Golden binds the panels unconditionally: there is no `SOFT_SIMULTE` guard around golden cinitial.cpp:5997-6490, so SIM and real machine behave the same. The port dropped the binding because gate n5-G3 was filed as UI-only. The gate's own premise is at `cinitial.cpp:14064-14074`: "this block does not compute anything ... running this block would change no machine state". It cites a stale `Motor/mymotor.cpp:1640-1643`; the function is now at :1949.

A second note repeats the same premise: T4-C6 at `cinitial.cpp:7223-7226` ("feeds the simulation SCREEN geometry only", "沒有其他活敘述").

Both premises are false: `fHTary` gates the `InitNewTray` call in `SetTray`, which is IC-grid machine state.

**The real-machine build has the same defect.** On the machine every supplied loader tray would also be judged empty and swept to the buffer.

It is not a SIM-logistics gap, and not config. The Z elevators not moving is separate: live `D:\HT9045\system\Gerneral.ini:544-553` `[TrayZ] *_Z_USE_MOTOR=0`, so cylinders are used (port `asendic.cpp:311-315` = golden `asendic.cpp:177-179`).

## 5. Corrections to the tracer reports

- Both tracers are right on the root cause. The only difference is emphasis (in-arm side vs tray side).
- "golden runs SetWorkParameter at boot (golden main.cpp:1090)": main.cpp:1090 is inside `SetRunStartMode` (main.cpp:363). The boot path is `TfMain::FormShow` (:9141) -> `InitialHandler()` :9559 -> cinitial.cpp:5845.
- "about 20 cycles": the merged history has exactly 18 supply/sweep cycles.
- "valueInRun inferred, not measured": it *was* measured once, in the `tasklist_600` flag rows (fHasTray=true, HasIC()=false).
- Golden's ctor (golden mymotor.cpp:1002-1008) does not assign `fHTary` at all. It relies on static zero-init of the global `MOT[]`. The port's explicit `fHTary=false` (:1942) is equivalent.
- `JsonBridge/ChanMvTrays.h:8-9` and `tests/test_w7_l1_auto_rt.cpp:156-161` already record "SetHTrayPanel is a no-op" and "the tray data argument of SetTray is not observable". The effect was noticed but filed as a test limitation, not as the stall.
- Prior research `backup/night_tools_20260927/research_0927/golden-sim-logistics.md` is wrong in two places. :87 ("整盤都是有料") is true only for golden. :228 ("No static blocker remains on the material-entry path") misses the fHTary gate.

## 6. Fix plan (faithful to golden)

1. **`Motor/mymotor.cpp:1949-1957`**: restore golden `mymotor.cpp:1500-1504` verbatim: `fHTary=true; pHTray=ptr;`. `pHTray` is already declared (`mymotor.h:328-329`, `#if 1`).

   This is safe with `ptr==NULL`:
   - there is no live `pHTray->` dereference (see §3.5);
   - `HasIC` only null-compares (`:1965`) and falls through to `else if (Tray.HasIC())`;
   - the other `if (fHTary)` bodies (`:2001`, `:2015`, `:2057`, `:2337`, `:2367`, `:2382`) are empty TODO(W7-UI) blocks.

   Update the comments at `mymotor.cpp:1951-1953` and `mymotor.h:324-327`, `:342-344`.
2. **`cinitial.cpp` SetSimuScreenPara**: add a live copy of golden's binding list inside the live `if(flag)`, i.e. after `#endif` `:14581` and before `}` `:14582`. The fMain / fAutoAlignment / fObserveMagazine widgets do not exist, so the argument is NULL. Use exactly golden's 50 motors in golden's order: :6430-6464, :6468-6471, :6476-6489.
   - MMPlate1, MMPlate2, MMTrayY, MMTrayY_Car, MMOCR
   - MManualTray1..6
   - MMAuto1..6
   - MMAuto1_Car..6_Car
   - MMEmpty, MMEmpty_Car, MMColor, MMColor_Car, MMEmpty1, MMEmpty1_Car
   - MMAutoCleanKit, MInRotateKit, MOutRotateKit
   - MMAOASampleTray, MMAOASamplePlate, MMInArmAOATray, MMOutArmAOATray
   - MMMagazineTary1..14

   Each is `MOT[X].SetHTrayPanel(NULL);`. Tag it `//AI(W906-HTRAY) 20260930:` and quote golden mymotor.cpp:1506-1514 as the reason (fHTary is machine state).

   Do **not** set `fHTary` in the ctor. Do **not** bind motors that golden leaves unbound (e.g. `MMTrayZ`), because golden's `SetTray` only raises `fHasTray` on those. Keep the verbatim `#if 0` block as it is: it still carries the real widget calls for a later UI un-gate.
3. **Correct the false rationale** so the gate is not re-closed:
   - `cinitial.cpp:14064-14074` (and the stale `:1640-1643` citation);
   - `cinitial.cpp:7223-7226`;
   - `JsonBridge/ChanMvTrays.h:8-9`;
   - `tests/test_amr.cpp:416-419`;
   - `tests/test_w7_l1_auto.cpp:635-638`;
   - `tests/test_w7_l1_auto_rt.cpp:156-161`.

   These tests do not bind, so their assertions stay valid. Only the "false offline" wording becomes "false unless SetSimuScreenPara ran".
4. **ctest** (new, e.g. `tests/test_htray_bind.cpp`, built with `-Wall -Wextra`). `fHTary` is protected, so observe it through `SetTray`:
   - T1 (unit): on a fresh `TTrayMotor`, `Tray.SetXYItem(4,4); SetTray(HAS_IC)` gives `HasIC()==false`, as golden's unbound behaviour. After `SetHTrayPanel(NULL)`, `SetTray(HAS_IC)` gives `HasIC()==true` and `Tray.HowManyIC()==16`. Then `SetTray(NULL_IC)` gives `HasIC()==false`.
   - T2 (integration): call `SetSimuScreenPara()` once. For each of the 50 golden-bound motors, `SetTray(HAS_IC)` must give `Tray.HasIC()==true`. Control: `MOT[MMTrayZ].SetTray(HAS_IC)` gives `fHasTray==true && Tray.HasIC()==false`.
   - Mutation check: reverting either edit must turn T1 or T2 red.

   Then run `build.bat gate` in a fresh build dir and compare the **failure list** with the standing five (`config_db`, `IniFiles`, `ini_helpers`, `config_loaders`, `GA1_ReadGeneralIni`). This is a whole-tree behaviour change toward golden: every `SetTray(x)` on the 50 motors now does ClearData+SetData.
5. **Same SIM run again** (same operator script `tools/flowcmp/script_S1.json`, new tag). Expected:
   - After the first SupplyNewIC 1300, `InArmPickFromLoadTask` goes 10 -> 12 -> 200 and on (no more 10<->15).
   - The flag rows show `MOT[MMTrayY].fHasTray=true` together with `Tray.HasIC()=true`.
   - `CatchTrayTask` stays in 100/300/400 and takes 100 -> 200 only once the grid has been picked empty.
   - MInArmZA / MInShutte1 / MTestZ1 and then the out side start moving.
   - The ChanMvTrays loader tag shows filled cells.

## 7. Not done / open

- Case 15's hold, i.e. which of `MMTrayY_Car` / `MMTrayY` / `MMTrayZ.fHasTray` is true at each 15->10, is not recorded (only MMTrayY is in the flag section). It is inferred from the always-cycling loader pipeline.
- Next stage, not yet exercised:
  - in-arm case 10's `CheckLoaderHasTray()` (ainarm9045.cpp:11864; `bCheckInShuttlePosAndNOIC()` -> 3 -> Task=20);
  - case 12 `SearchAndMoveInArmXYToLoad_9045`;
  - case 200 pick.

  The run shows `UserDefForm[0].YPitch=3840`, so the old YPitch=0 loop should not recur, but that is unproven.
- With `pHTray==NULL`, golden's `mtPlate1` name branch in `HasIC` (CleanPlate2HasIC, golden mymotor.cpp:1059-1072) stays unreachable. That is pre-existing and unchanged by this fix.
- Golden `InitNewTray` also writes `Tray.iWhichSite` under `if(fHTary)` when `bShowSiteMapFlag` is true (FIFO mode, golden mymotor.cpp:1202-1438). The port's live InitNewTray is an abbreviated stand-in (`mymotor.cpp:2326-2349`). After this fix `fHTary` is true, so that stand-in's FIFO gap becomes reachable. It is a separate item. Do not un-gate the verbatim block (`:2074-2323`) while `pHTray` is NULL: it dereferences `pHTray`.
- The real-machine WAR16122 loop described in `cStateRecord.cpp:872-875` (empty loader tray not recycled -> the same 10<->15) has a different mechanism. Do not merge the two diagnoses.

## 8. Fix applied (INBOX 121, 20260930, `AI(W906-HTRAY)`)

Line numbers in §1-§7 are for 0b8480c5, before the fix. After the fix:
- `Motor/mymotor.cpp:1949-1957`: SetHTrayPanel has golden's body (`fHTary=true; pHTray=ptr;`).
- `cinitial.cpp` SetSimuScreenPara:
  - the corrected n5-G3 rationale is :14064-14076 (it was :14064-14087; 24 lines became 13);
  - `#if 0 // GATE n5-G3` is at :14077 (was :14088), and the gated MMTrayY widget call at :14511 (was :14522);
  - the gate's `#endif` is at :14570 (was :14581);
  - the live list is :14571-14581: one tag line, then 10 lines holding golden's 50 motors in golden's order, each `SetHTrayPanel(NULL)`;
  - the closing `}` (:14582) and `flag=false;` (:14583) did not move, and nothing after them moved.
- New ctest `HtrayBind` (`tests/test_htray_bind.cpp`).
- Not done in this pass:
  - §6 step 4's full `build.bat gate` (only the targeted ctest ran);
  - §6 step 5, the SIM re-run (wb_serve was not run).
