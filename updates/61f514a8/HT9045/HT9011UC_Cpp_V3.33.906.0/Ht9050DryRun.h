// =============================================================================
//  Ht9050DryRun.h -- AI(W906-HT9050-DRYRUN) 20261005: HT9050 motor-only dry-run cycle (EastSun 1005).
//
//  EastSun 1005「以目前為基底，動作皆為馬達動作，Loader Y上IC自動補，Loader Z則不動作，Index Z1 軸在Home點不動做，交換資訊流，
//  馬達動作先By Pass，按Start後可以Cycle Run不間斷，按Pause的時候則停止動作」+ Step1..Step10; 「將以上的步驟使用程式模擬是否能正確執行，
//  若能模擬正確執行再進行真正的機台動作」(tests/test_ht9050_dryrun.cpp is that simulation).
//
//  Not golden: a separate sequencer that replaces the engines inside the live DoAllProcess (csystem.cpp) while
//  W906_HT9050_DRYRUN is defined (MachineType.h) and the machine is HT9050 (9050GPIB: the origin hook answers).
//  Taught points only (teach.ini through Tech / Prod). No vacuum, no tray / lot data, no Loader Z, no Index Z.
// =============================================================================
#ifndef Ht9050DryRunH
#define Ht9050DryRunH

#include <string>

// true = the dry run owns DoAllProcess this tick (HT9050 + W906_HT9050_DRYRUN).
bool W906_Ht9050DryRunOn();
// One tick. paused = DoAllProcess's main guard would return (SoftStop / !SystemStart / !fAllMotorHome):
// the first paused tick stops every axis the dry run drives; the next running tick resumes the same step.
// firstRunTick = the first tick after a stop (csystem.cpp: iHandlerStartCount==0). In the real MainProc a PAUSE stops
// calling DoAllProcess at all (PauseFromWeb -> StopAllMotor, then SystemStart=false), so the axes were stopped
// behind the dry run's back: on that tick every driven axis's pending command is dropped (fCMD=false) and the
// step re-issues its moves -- otherwise MotorMove would see "motion done" and report a stopped axis as arrived.
void W906_Ht9050DryRunTick(bool paused, bool firstRunTick = false);
// Back to Step 1 (cycle counters kept).
void W906_Ht9050DryRunReset();

// Observers (tests, op log, web tag).
int         W906_Ht9050DryRunStep();       // 10..109, see Ht9050DryRun.cpp
int         W906_Ht9050DryRunCycles();     // completed cycles (IC placed on Auto1)
std::string W906_Ht9050DryRunWhy();        // what the current step is waiting for ("" = moving / nothing)

#endif
