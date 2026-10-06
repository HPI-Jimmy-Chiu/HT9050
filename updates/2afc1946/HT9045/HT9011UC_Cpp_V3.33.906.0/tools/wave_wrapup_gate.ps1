# ---------------------------------------------------------------------------
#  wave_wrapup_gate.ps1 -- run EVERY wrap-up check in one place, one exit code.
#
#  AI(W906-GL-0w) 20260827.
#
#  WHY THIS EXISTS (it is a fix for a mistake I actually made)
#  ----------------------------------------------------------
#  Waves GL-0s / 0t / 0u / 0v each reported "four gates green".  That was true of
#  the four I happened to run.  A FIFTH gate -- tools\soft_simulte_gate.ps1, which
#  guards prose about SOFT_SIMULTE and is therefore safety-relevant -- had been
#  RED the whole time and I never ran it.  Worse, GL-0s built a weaker duplicate
#  of it that reported PASS, so the green count went UP while the real gate stayed
#  red.
#
#  A hand-assembled gate list drifts the moment somebody adds a gate.  So this
#  runner does not just run a list: it CROSS-CHECKS the list against what is
#  actually in tools\, and FAILS if it finds ANY tools\*.ps1 it was never told about.
#  A runner that can silently omit a gate is the thing it is supposed to prevent.
#  ⚠️ It globbed only *_gate.ps1 at first, and stub_shadow_audit.ps1 -- a gate in every way
#  that matters, exit 1 on a finding -- slipped through on its NAME.  GL-0x widened it to
#  every .ps1: each is run, or listed in $NotRun with a reason.
#
#  WHAT IT RUNS
#  ------------
#    1. pe_truncation_check.ps1   PE section/COFF lengths -- truncated or zero-header exes
#    2. macro_order_gate.ps1      #ifdef evaluated before MachineType.h is reachable
#    3. production_audit.ps1      nine machine-output roots, per-file MD5
#    4. soft_simulte_gate.ps1     prose about SOFT_SIMULTE vs the macro's real state
#    5. stub_shadow_audit.ps1     which body the LINKED exe actually runs (ExpectExit 1:
#                                 44 stubs shadow translated bodies -- queued user ruling)
#    5. GL-G6 safety condition    no vendor API among IMPORTED symbols; system DLLs only
#    6. cite_check scale invariant a broken scan is deterministic and passes idempotence,
#                                 so only SCALE catches it (GL-0q(b) cost a whole wave)
#    7. working-tree hygiene      .cpp/.h change count, and CRLF/BOM on changed text files
#
#  WHAT IT DOES NOT DO
#  -------------------
#  It does not build.  Iron rule 6's two builds are a separate obligation and are
#  blocked whenever the user's wb_publish.exe holds the linker output.
#
#  USAGE
#    tools\wave_wrapup_gate.ps1              # exit 0 = every check passed
#    tools\wave_wrapup_gate.ps1 -SkipSlow    # omit 6 (a full -AllTree pass, ~2 min)
# ---------------------------------------------------------------------------
[CmdletBinding()]
param(
  [switch] $SkipSlow
)

$ErrorActionPreference = "Continue"
$Tree = Split-Path $PSScriptRoot -Parent
$bin  = Join-Path $env:LOCALAPPDATA "Programs\ht9045-nonoracle-toolchain\mingw32\bin"

# The gates this runner knows about.  Cross-checked against tools\ below.
#
# ExpectExit: some of these are AUDITS whose non-zero exit encodes a KNOWN, ALREADY-QUEUED
# count rather than a regression.  Registering the expected code means the runner fails when
# the number CHANGES -- which is the event worth knowing -- instead of being permanently red
# and therefore ignored.  Same reasoning as $Adjudicated in soft_simulte_gate.ps1.
$Gates = @(
  @{ Name = 'pe_truncation_check'; Script = 'tools\pe_truncation_check.ps1'; Args = @(); ExpectExit = 0 },
  @{ Name = 'macro_order_gate';    Script = 'tools\macro_order_gate.ps1';    Args = @(); ExpectExit = 0 },
  @{ Name = 'production_audit';    Script = 'tools\production_audit.ps1';    Args = @(); ExpectExit = 0 },
  @{ Name = 'soft_simulte_gate';   Script = 'tools\soft_simulte_gate.ps1';   Args = @(); ExpectExit = 0 },
  # AI(W906-GL-6m) 20260901: iron rule 3's SECOND HALF ("記進 docs/UPSTREAM_DEFECT_REPORT.md").
  # GL-6l measured that half for the first time: 72 production .cpp/.h carry in-source
  # `GOLDEN BUG`/`GOLDEN DEFECT` notes and the report named only 19 of them.
  # ⚠⚠ THE REASON THIS IS A GATE AND NOT A ONE-OFF NUMBER: the measurement broke the FIRST time it
  # was repeated, and it broke looking like success.  GL-6l wrote the list of 53 unregistered
  # filenames INTO the report as a gap appendix; the naive metric ("does the report mention this
  # filename?") then returned YES for all 53, so the gap appeared to fall 53 -> ~0 WITH NOTHING
  # REGISTERED.  A diagnostic written into the document being diagnosed destroys the diagnostic.
  # The gate therefore splits the report at the appendix heading and counts ONLY the defect body;
  # appendix-only mentions count as NOT registered, and it FAILS if that heading goes missing
  # rather than silently reverting to scanning the whole file.
  # ⚠ It also separates "appears nowhere" from "appendix only", because GL-6l's appendix wrote
  # sibling headers as `X.cpp`/`.h` -- so aRotateKIT_In.h, aRotateKIT_Out.h, automation.h,
  # SCK_ART_Remainder.h, SecsSvEcRegistration.h and TrayCore.h were listed nowhere in searchable
  # form even though the author believed they were listed.  Compressing a list broke its
  # searchability; that class stays visible in its own bucket.
  # ⚠ PROVEN TO FIRE, both directions, 20260901: renaming the appendix heading -> exit 1 with the
  # specific message; adding one unregistered filename to the DEFECT BODY -> NAMED_IN_BODY 21->22
  # and UNREGISTERED 51->50, exit 1 asking for the registered values to be updated (a DROP is also
  # a failure, deliberately, because a drop can mean the metric was gamed again).  Both reverted.
  # ⚠ AND GATE 0 WENT RED FIRST: "tools\*.ps1 共 23 ... ✗ 未列入: golden_note_registry_gate.ps1",
  # measured before this entry existed.  The entry silences a check that was PROVEN to fire.
  # ⓘ Its numbers are FILE counts, never defect counts -- one file can hold several claims, some
  # are negations, and some are language-compatibility notes that do not belong in the report.
  @{ Name = 'golden_note_registry_gate'; Script = 'tools\golden_note_registry_gate.ps1'; Args = @(); ExpectExit = 0 },
  # AI(W906-GL-0x) 20260827: this one exits 1 BY DESIGN -- "44 stubs shadow translated
  # bodies, so that golden logic does not execute".  Retiring those 44 is a USER RULING that
  # is already queued (safety queue: do not do, do not ask), so the 1 is expected.
  #
  # ⚠ AI(W906-GL-2i) 20260828 -- THE OLD COMMENT HERE CLAIMED "what is NOT expected is the count
  # moving; the runner reports the line so a change is visible".  BOTH HALVES WERE FALSE, and
  # GL-2i proved it by accident: the count went 44 -> 42 and THIS GATE STILL PASSED, because the
  # registration checks only ExpectExit = 1 and the audit exits 1 for ANY non-zero count.  The
  # runner also printed no count, so nothing was visible either.
  # A registration that cannot fail is not a registration.  See the SUM check after the loop.
  # ⚠ AI(W906-GL-2p) 20260828: NOW A SET {0,1}, AND THE REASON IS MEASURED, NOT A LOOSENING.
  # This wave edited Motor\myGALILmotor.h -- a HEADER.  That makes every dependent .obj older
  # than its source, so the audit can decide NOTHING: all 44 rows moved to UNDECIDED-NO-DWARF,
  # the headline count became 0, and it exited 0 instead of 1.  Nothing regressed -- the SUM
  # check below read 0 + 44 = 44 and passed, which is precisely why GL-2i registered the sum
  # rather than the headline.  So for this audit the exit code tracks "how much was decidable
  # today", which a comment-only header edit legitimately drives to zero; the SUM is the part
  # that carries the finding.  {0,1} = "some or none decidable", NOT "any code is fine".
  @{ Name = 'stub_shadow_audit';   Script = 'tools\stub_shadow_audit.ps1';   Args = @(); ExpectExit = @(0,1) },
  # AI(W906-GL-4q) 20260830: the citations in the documents that STEER every wave.
  # cite_check guards 16,959 citations INSIDE the tree; CLAUDE.md and the gl-wave-loop SKILL.md sit
  # OUTSIDE it, under a root that is not even a git repo -- and GL-4p measured four drifted
  # citations in them, one of which would have been "fixed" onto DEAD code by anyone trusting
  # grep's first hit.  ExpectExit is @(0) only: unlike stub_shadow_audit there is no legitimate
  # "partially decidable" state here.
  # ⚠ It is an ANCHOR REGISTRY, not an existence check, and that is load-bearing: all four of
  # GL-4p's drifted citations pointed at lines that EXIST and are IN RANGE, so an existence/range
  # gate would detect none of its own motivating cases.  Measured in the proof-of-red: with three
  # of the pre-fix numbers restored, SHAPE stayed 0 while ANCHOR fell to 7/10.
  # ⚠ Gate 0 went RED first ("tools\*.ps1 共 21 ... ✗ 未列入: steering_cite_check.ps1", exit 1,
  # measured before this line existed).
  @{ Name = 'steering_cite_check'; Script = 'tools\steering_cite_check.ps1'; Args = @(); ExpectExit = @(0) },
  # AI(W906-GL-1c) 20260827: three consecutive waves fixed the SAME silent-truncation defect
  # in a different tools\census\*.py each time (GL-0z expired_gate_scan [:30], GL-1b
  # macro_seam real[:3], GL-1c body_size rows[:30]).  Third repeat -> fix the mechanism, not
  # another instance.  Registers every display slice in that directory; an unregistered
  # slice, or a registry entry that no longer matches the file, fails.
  @{ Name = 'silent_cap_gate';     Script = 'tools\silent_cap_gate.ps1';     Args = @(); ExpectExit = 0 },
  # AI(W906-GL-2e) 20260828: FOURTH repetition of one defect class -> fix the mechanism.
  # GL-1z / GL-2a / GL-2c / GL-2d each hand-worked "a comment tells you to do something that is
  # ALREADY DONE" and each time the answer was the same.  This scanner keys on the tree's one
  # regular convention -- `#if 0 // PT-W?? RETIRED (Symbol)`, measured 218 named of 230 -- and
  # reports action-bearing comments naming a symbol that already has such a gate.
  # ⚠ 12 is the CURRENT, MEASURED backlog, not a target of zero: each needs its own
  # line-count-neutral rewrite.  Registering it means the number cannot drift unnoticed.
  # GL-2f 20260828: 18 -> 12.  Six ainarm9045.cpp sites fixed; naming the six unnamed RETIRED
  # gates widened the key 218 -> 224 and surfaced TWO more (:5052, :10571), both fixed too; and
  # one row (:6333) is now an ADJUDICATED false positive -- printed, not counted.
  # Its control pair runs on the MAIN path (positive SetMotorScaleSpeed / negative
  # SetOutArmSpeed) and refuses to print a count if either fails.
  # AI(W906-GL-2g) 20260828: 18 -> 12 -> 0 OPEN findings.  ⚠ 0 DOES NOT MEAN "no stale
  # instructions left in the tree" -- it means no action-bearing comment names one of the 224
  # RETIRED-and-named symbols without already carrying the annotation.  The tool's own header
  # lists what it still cannot decide (subjects outside those 224, the 6 unnamed RETIRED gates).
  # 4 adjudicated false positives are printed but not counted, all four 弱 evidence.
  # AI(W906-GL-2h) 20260828: ⚠ AND THE SECOND NUMBER IS WHY THE FIRST ONE IS NOT A VICTORY.
  # The tool was SILENTLY DROPPING every action-bearing comment whose subject is not one of the
  # 224 named symbols -- 49 of them, invisible, while it printed "0 findings".  Now disclosed and
  # registered: 42 after GL-2h removed 7 over-matches of bare `MUST GO` (four were GOLDEN'S OWN
  # 2012/2016 author comments about which bin an IC goes to).  ⚠ 42 is a READING LIST, not a
  # defect count, and it is not 42 unknowns -- several rows were already judged in earlier waves
  # (cinitial.cpp:8131/:8537/:9429 scratch-probe history; csystem.cpp:23541/:23602 premises that
  # still hold).  If this number DROPS, check it was by reading a row, not by rewording one.
  # AI(W906-GL-2i) 20260828: 42 -> 38, AND THE DROP IS FULLY ACCOUNTED FOR BY ROWS THAT WERE
  # READ, not reworded away: RotateKit\aRotateKIT_Out.cpp's D-1 and D-2 (2), csystem.cpp:9427 (1),
  # ainarm2.cpp:3119 (1) = 4.  ⚠ THE GATE CAUGHT THIS, not me -- I edited the three files and
  # left the registration at 42, and this is exactly the drift the second number exists to see.
  # AI(W906-GL-2j) 20260828: 38 -> 32 待讀, and the drop is again fully accounted for -- the six
  # bucket-① rows (csystem.cpp:23541/:23602/:18551/:30687/:21539, ainarm9045.cpp:7821).  Five were
  # ANNOTATED after verifying their premise still holds; ONE (:21539, GATE G03) went into the new
  # $ReadNoAction registry because its text was already correct and only its QUOTATION matched.
  # ⚠ MEASURED SUBTLETY: the skip rule keys on THE MATCHED LINE, not the paragraph, so an
  # annotation on a continuation line leaves the row on the list.  Three rows needed the marker
  # moved onto the matched line before they cleared.
  # AI(W906-GL-2k) 20260828: 32 -> 28 待讀, again fully accounted for -- the four bucket-② rows
  # (cinitial.cpp:8131/:8537/:9429 annotated, Command.cpp:10708 into $ReadNoAction).  ⚠ The RESUME
  # predicted ALL FOUR would be $ReadNoAction "history, nothing to fix"; measuring said otherwise:
  # the three cinitial gates cited body lines that had ALL drifted by exactly +63.
  # AI(W906-GL-2l) 20260828: 28 -> 20 待讀 -- the whole ainarm2.cpp cluster (8 rows: :784 / :1014 /
  # :1902 / :3414 / :3887 / :5826 / :7320 / :7353).  EVERY ONE had wrong line numbers and FOUR were
  # instructions ALREADY CARRIED OUT.  ⚠ Those four name symbols that ARE in the RETIRED index --
  # the tool missed them only because the name sits outside the ±3 window (in the banner heading or
  # in the signature below).  Blind spot now quantified in the tool's header, with the structural
  # fix specified and queued.
  @{ Name = 'stale_instruction_scan'; Script = 'tools\stale_instruction_scan.ps1'; # ⚠ AI(W906-GL-2s) 20260828: BOTH NUMBERS ARE NOW 0, AND READ WHAT THAT DOES AND DOES NOT MEAN.
  # The reading list opened at 49 (GL-2h disclosed it), fell to 42 when GL-2h removed 7 predicate
  # over-matches, and 42 rows were then disposed of one at a time: 34 FIXED IN THE FILE, 8 entered
  # $ReadNoAction as correct-as-written with a measured reason. Plus 4 adjudicated false
  # positives, printed and not counted.
  # ⚠ 0 + 0 DOES NOT MEAN THE TREE HAS NO STALE INSTRUCTIONS. It means: no action-bearing comment
  # names one of the 224 RETIRED-and-named symbols without carrying its annotation, AND no such
  # comment is left undecided. The tool's header lists what it still cannot see -- subjects outside
  # those 224, the 6 unnamed RETIRED gates, and the ±3 window blind spot GL-2l quantified (half of
  # ainarm2.cpp's rows were findable-in-principle and invisible in practice).
  Args = @('-Expect','0','-ExpectUnmatched','0'); ExpectExit = 0 },
  # AI(W906-GL-1s) 20260828: cite_check's quote matcher was WIDENED this wave (multi-line joins,
  # then optional whitespace next to punctuation), and a widened matcher is precisely what rots
  # invisibly: loosen it once more and everything becomes a "hit", which reads as a clean report.
  # Its -SelfTest runs fourteen fixtures -- SEVEN of them negative -- through the SAME two functions
  # the report uses, so it cannot pass while the real path diverges.  It caught a live bug on
  # its first run (a 1-element result unrolls to a PSCustomObject, which has no .Count).
  @{ Name = 'cite_check -SelfTest'; Script = 'tools\cite_check.ps1'; Args = @('-SelfTest'); ExpectExit = 0 },
  # AI(W906-GL-2y) 20260829: NEW GATE -- the class no other discriminator can see.
  # cite_check decides whether the cited FILE and LINE exist; this one decides whether the cited
  # line CARRIES the symbol the citing line declares.  GL-2x proved the gap by measurement: fixing
  # six such citations in csystem.cpp moved cite_check's queue total by EXACTLY ZERO, because
  # golden aoutarm9045.h:79 holds `HasGapsInTheTray` -- real code, in range, nothing to flag.
  # ⚠️ ITS FIRST VERSION REPORTED 92 AND ~HALF WERE ITS OWN FALSE POSITIVES (golden-only tree
  # resolution -- the very defect GL-2x had just documented in cite_check, rebuilt one wave later).
  # It now checks BOTH trees and asserts WRONG-LINE only when NEITHER carries the symbol, with two
  # REAL-DATA controls moving in opposite directions on the main path.  Those controls, not the
  # eight extractor fixtures, are what caught the bug: all eight fixtures passed while the number
  # was half fiction, because the extractor was never the broken part -- the RESOLUTION was.
  @{ Name = 'cite_symbol_check'; Script = 'tools\cite_symbol_check.ps1'; Args = @(); ExpectExit = 0 }
  # AI(W906-GL-3s) 20260829: THE THIRD OCCURRENCE, SO THE MECHANISM CHANGED.
  # "The correction is written at the new home and never propagates back to the old seam" was
  # found and fixed BY HAND three times -- fQwertyKey (GL-3o), fSetup (GL-3p), fNote (GL-3r).
  # ⚠ AND THE TOOL'S FIRST RUN FOUND SIX MORE THAT ALL THREE HAND AUDITS HAD WALKED PAST
  # (forms\fSecurity.h:318, Command.cpp:14840, forms\fContactCT.h:120,
  # SECSGEM\uHGemHT9045.cpp:2859/:4179, Command.cpp:14835), which is the clearest argument that
  # restating a lesson is not the same as installing a check.  Registered counts are 0 / 2 / 4
  # (fQwertyKey / fNote / fSecurity), each nonzero row ITEMISED in the script's registry and
  # queued -- the stub_shadow_audit = 44 precedent, so the debt cannot drift silently.
  @{ Name = 'stale_premise_scan'; Script = 'tools\stale_premise_scan.ps1'; Args = @(); ExpectExit = 0 }
  # AI(W906-GL-7a) 20260901: the class NOTHING here could see, and it got past everything once.
  # GL-6z's first draft of a comment replaced 1 comment line with 3 (uHeaterThread.cpp 2105 -> 2107).
  # cite_check still resolved every citation (a file growing internally invalidates nothing it
  # CARRIES, while every citation pointing INTO it below the edit silently shifts); this runner
  # printed the new line count with no expected value to compare it against; and cite_symbol_check /
  # stale_premise_scan / macro_order_gate are all line-count-agnostic.  What caught it was reading
  # `git diff --numstat` and seeing `3  1`, i.e. a habit, not a check.  Now it is a check.
  # ⚠ The invariant is NARROW ON PURPOSE: "added == removed" only when EVERY changed line on both
  # sides is a comment.  Translation waves add real code and legitimately change line counts; a gate
  # that reddened on those would get -Allow pasted in permanently and then guard nothing.  Files with
  # any code change report MIXED-SKIPPED, and new/deleted files are skipped by construction.
  # ⚠ PROVEN TO FIRE, both in a self-test and end to end: GL-7a built a throwaway git repo in the
  # scratchpad and ran six controls -- comment-only 3-for-1 => VIOLATION exit 1 (GL-6z's actual
  # mistake); comment-only 1-for-1 => OK; pure code +3/-0 => MIXED-SKIPPED; mixed +3/-2 =>
  # MIXED-SKIPPED; the red case with -Allow => ALLOWED; a brand-new file => skipped.  Its own
  # -SelfTest is 21/21 with the VIOLATION path among the assertions.
  @{ Name = 'line_neutrality_gate'; Script = 'tools\line_neutrality_gate.ps1'; Args = @(); ExpectExit = 0 }
  # AI(W906-GL-7b) 20260901: the ENUMERATOR half of "the cited line is the wrong line".
  # cite_symbol_check cannot see enumerators -- its symbol test (cite_symbol_check.ps1:330)
  # requires the identifier to be followed by `(` or `;`, so it validates functions and
  # statements and SKIPS `tcCCD=10,`.  Two of the three note-citation errors found while
  # working uHeaterThread.cpp's A..I block were enumerator citations, and both passed a
  # green runner: GL-6t's MachineType.h:538 (eIndexHeatMode, really :533/:565) and GL-6y's
  # MachineType.h:637 (tcCCD, really golden :633).
  # ⚠ AI(W906-GL-7e) 20260901: registered 32 -> 1.  All 32 were REPAIRED (18 files, 33 lines) --
  # the citations carried 20260825 PRE-IMPORT port line numbers, per GL-7c's root cause, and each
  # was relocated by finding its enumerator (offsets are NOT uniform: mostly +27, but
  # e9045_1x4_4_13 was 1014->1050).  The remaining 1 is an ITEMISED KNOWN FALSE POSITIVE named in
  # the tool header (aTester_Front.cpp:7247 -- `A / B  Hdr.h:X/:Y` pairs POSITIONALLY and those two
  # identifiers straddle a line break, so the proximity window mispairs it).  Registered rather
  # than suppressed so a SECOND finding still goes red.
  # ⚠⚠ A SIXTH false-positive class surfaced during that repair and it is the only one that made
  # me CREATE a defect: following the tool changed a CORRECT citation into a wrong one.  It was
  # caught by auditing all 32 repairs afterwards for multi-number shapes (6 hits, 5 harmless).
  # See docs/DEVLOG.md CCXVIII -- and note two other GL-7e mistakes recorded there that no gate
  # saw: a whole-file line-ending conversion (CRLF->LF on aTester_Front/Rear, 24k+25k line diff)
  # and three files given a trailing newline they never had.  git diff --numstat caught both.
  # ⚠ AI(W906-GL-7d) 20260901: registered 36 -> 32.  Range-citation support (Hdr.h:A-B, which
  # this tree writes) removed four false positives; the tool header itemises them.  That was the
  # FIFTH false-positive class in this tool, and all five were found by opening the cited file.
  # ⚠ AI(W906-GL-7c) 20260901: ROOT CAUSE of the remaining population is ONE EVENT -- 15 of 18
  # sampled citations resolve exactly against HT9011UC_Cpp_V3.33.906.0_noBuild\MachineType.h
  # (1,616 lines), the port tree BEFORE the 20260825 laptop import; today's file is 1,643 lines.
  # The repair is per-citation (the import inserted lines unevenly), and this tool already
  # reports the correct line.  Repairing them touches .cpp/.h, so it owes iron rule 6's builds.
  # ⚠ REGISTERED AS A POPULATION AND NOT A VERDICT.  Six of the original 36 were
  # hand-checked; four were real and two exposed false-positive classes now fixed.  The
  # other 30 are NOT individually verified, and five of them are off by EXACTLY -5 across
  # five unrelated enums, which looks like one event rather than five slips.  The number is
  # pinned so the debt cannot drift silently (stub_shadow_audit = 44 precedent); the
  # root-cause measurement is queued in the RESUME.  Read the tool's header before touching
  # this number.
  # ⚠ IT TOOK FOUR FALSE-POSITIVE CLASSES TO GET HERE, each now locked by -SelfTest (40
  # assertions): whole-line identifier pairing (45 findings, mostly its own), keeping only
  # the first occurrence of a repeated enumerator, PowerShell's case-INSENSITIVE hashtables
  # and -eq matching HandlerArm to HANDLERARM, and judging citations that point at a line
  # which USES an enumerator rather than declares it.
  # ⚠ PROVEN TO FAIL: -Expect 0 gives exit 1 against the same tree (measured).
  @{ Name = 'enum_cite_check'; Script = 'tools\enum_cite_check.ps1'; Args = @(); ExpectExit = 0 }
  # AI(W906-GL-7o) 20260902: the 15th gate, and the first one that watches THE WRAP-UP ITSELF
  # rather than the tree.  Measured this wave: the newest DEVLOG wave section was CCXXVII while
  # the newest RESUME block was CCXV -- the heartbeat's "update the trailing RESUME" step had
  # been skipped for TWELVE CONSECUTIVE WAVES and nothing noticed, because all fourteen existing
  # gates look at code, citations or counts.  A stale RESUME is expensive in a specific way: the
  # cold-start protocol reads it FIRST, so the next session inherits a twelve-wave-old picture of
  # what is done, what is queued, and what the user has been asked.
  # ⚠ TWO RELATIONAL INVARIANTS, SO THIS GATE NEVER NEEDS A REGISTERED NUMBER:
  #   (1) max(wave number with a DEVLOG section) == max(wave number with a RESUME block)
  #   (2) the file tail still carries the pointer saying the newest RESUME is NOT at the tail --
  #       DEVLOG.md splices new sections into the MIDDLE, so a reader who follows the heartbeat
  #       literally lands on the 20260824 RESUME, which looks perfectly normal.  I did that this
  #       wave and it is how the drift was found at all.
  # ⚠ PROVEN TO FIRE, on the real file, before the fix: exit 1 with
  #   「最新段落是 CCXXVII 但最新 RESUME 只到 CCXV —— 落後 12 波」 plus the missing tail pointer.
  # ⚠⚠ AND ITS FIRST NUMBERS WERE WRONG -- worth recording because the -SelfTest was green while
  #   they were wrong.  It printed 「CCV / XCIII / 落後 112 波」 because the records were
  #   [hashtable]s and `Sort-Object -Property Wave` does NOT fall back to a hashtable KEY, so it
  #   sorted on $null and "newest" became whichever row came first in the file.  The self-test
  #   missed it because it took the maximum with `$_.Wave`, and MEMBER ACCESS on a hashtable DOES
  #   do the key fallback: THE TEST EXERCISED A DIFFERENT ACCESSOR THAN THE CODE UNDER TEST.
  #   Fixed twice over: [pscustomobject] records, and both paths now go through Get-Newest.
  #   Two further self-test gaps were found the same way and are recorded at the call sites:
  #   Mandatory [string] rejects '' before the body can return -1, and Mandatory [string[]]
  #   rejects EVERY empty ELEMENT -- so a fixture without blank lines passes while the tool
  #   cannot read a real DEVLOG at all.  -SelfTest is 32/32 with a blank-line fixture.
  @{ Name = 'devlog_resume_gate'; Script = 'tools\devlog_resume_gate.ps1'; Args = @(); ExpectExit = 0 },
  # AI(W906-ST-LEDGER) 20260923: THE SAME LOOPHOLE THIS RUNNER EXISTS TO CLOSE, ONE LEVEL DOWN.
  # The cross-check below globs '*.ps1' (line ~366), so every Python census is invisible to it.
  # Measured 20260923: of 42 tools that call themselves a gate/CI check, 16 are invoked by NO
  # script at all (self-references excluded).  Three of those expose --check -- they are shaped
  # like CI and wired to none.  The worst is start_sites_census.py: CLAUDE.md:68 and WebStart.h:36
  # both print its "--check 32 29 3" as the CI form, "數字變了就紅燈", and git grep over
  # *.sh/*.ps1/*.bat/*.yml/*.cmake/CMakeLists.txt finds ZERO callers.  It has never been run by
  # anything except a human who remembered.  A check nothing runs is documentation, not a gate.
  # census_gate.ps1 runs all three AND cross-checks tools\*.py the same way this file does .ps1,
  # so the .py side can no longer grow a census that nobody calls.
  @{ Name = 'census_gate';        Script = 'tools\census_gate.ps1';        Args = @(); ExpectExit = 0 },
  # AI(W906-ST-LEDGER) 20260923: these two were UNREGISTERED -- gate 0 had been printing
  # "✗ 未列入: exe_startup_gate.ps1, pci1203_control_gate.ps1, pci1203_readonly_gate.ps1"
  # and therefore this whole runner was exiting 1 before it proved anything. They are the
  # two gates that hold the 1203 WRITE surface to its allowlist, on a tree the user armed
  # on 20260918 (WB_PUMP_1203_CONTROL_LIVE). Both measured PASS 20260923 before registering.
  # ⚠ They are NOT redundant with each other: the monitor gate asks "are there ANY mutating
  #   calls?" (must be zero) and the control gate asks three narrower questions, because on
  #   the write surface writing is the point. pci1203_control_gate.ps1 only MENTIONS the
  #   read-only one in its header prose ("the sibling of ...") -- it does not invoke it, so
  #   registering only one would leave the other unrun. (A substring-based caller census
  #   reports that mention as a call; it is a false positive.)
  @{ Name = 'pci1203_readonly_gate'; Script = 'tools\pci1203_readonly_gate.ps1'; Args = @(); ExpectExit = 0 },
  @{ Name = 'pci1203_control_gate';  Script = 'tools\pci1203_control_gate.ps1';  Args = @(); ExpectExit = 0 }
)

# AI(W906-GL-0x) 20260827: THE NAMING LOOPHOLE THAT LET A GATE HIDE.
# The first version globbed tools\*_gate.ps1.  stub_shadow_audit.ps1 is a gate in every way
# that matters -- it exits non-zero on a finding -- and it is not named that way, so the
# cross-check could not see it.  That is the SAME failure the cross-check exists to prevent,
# one level up.  So the check now covers EVERY tools\*.ps1: each is either run, or listed
# here with a reason.  A new script with no entry fails the runner.
$NotRun = @{
  'wave_wrapup_gate.ps1'          = 'this runner itself'
  'live_lines.ps1'                = 'a query tool -- needs -Source/-Lines, no tree-wide verdict'
  'ladder_recon.ps1'              = 'generates GL-1a s blocker report; no pass/fail verdict'
  'gl_s0_recon.ps1'               = 'needs -GoldenFile/-Starts; per-function query, not a gate'
  'gl_s0_queue.ps1'               = 'regenerates GL_S0d; verified byte-identical 20260827'
  'gl_s0_callers.ps1'             = 'regenerates GL_S0e; verified byte-identical 20260827'
  'stub_vs_golden.ps1'            = 'ledger generator, needs -Filter/-OutFile to do anything'; 'f5_close_replay.ps1' = 'AI(W906-F5-CLOSE) 20261004: an interactive replay (WebView2 window + a booting wb_serve that writes system\lastdata*.dat) -- run by hand with tools\realfile_guard.py snap/check around it, never from a runner'
  'soft_simulte_marker_place.ps1' = 'WRITES .cpp FILES IN PLACE (relocates markers). Never run from a gate runner -- it is a source edit and needs iron rule 6 s two builds.'
  # AI(W906-GL-4j) 20260830: a dot-sourced EDIT PRIMITIVE, not a gate -- it has no tree-wide
  # verdict to give, so there is nothing for this runner to assert about it.  It is the
  # hand-written "assert the old text, then read the line back and compare -ceq" procedure from
  # DEVLOG CXLI..CXLV turned into one place, with the CXLI comma-vs-plus trap removed structurally
  # (named Old/New fields instead of a positional pair, and an array value is refused outright).
  # ⚠ It has its OWN -SelfTest (20/20, both directions per fixture); run that, not this runner.
  # ⚠ AND THE LISTING BELOW IS WHY IT IS HERE AT ALL: adding the file made THIS gate go red first
  # ("✗ 未列入: edit_verified.ps1", exit 1, measured 20260830 before this entry existed), which is
  # exactly what gate 0 is for.  The entry silences a check that was PROVEN to fire.
  'edit_verified.ps1'             = 'a dot-sourced edit primitive (Set-LinesVerified), not a gate -- no tree-wide verdict. Has its own -SelfTest: powershell -File tools\edit_verified.ps1 -SelfTest'
  # AI(W906-GL-4n) 20260830: a PROBE, not a gate, and the distinction is deliberate.  A held-open
  # artifact is a LEGITIMATE state -- the user runs wb_publish/wb_gateway to look at the HMI -- so
  # a runner that failed on it would be showing red for "someone is using the machine".
  # ⚠ WHY IT EXISTS: the premise "exe 鎖仍在，鐵則 6 的兩組建置仍是未償義務" was carried unmeasured
  # from GL-4h to GL-4m and froze the whole translation queue.  Measured 20260830: 1,234 artifacts
  # across five lanes, TWO held, both in build_nonoracle, and NOTHING in build_sim_nonoracle -- one
  # of the two lanes iron rule 6 names.  The SIM lane then built clean (exit 0, 282 TUs) and the
  # non-SIM lane with -k gave 0 compiler errors and 156/158 targets.
  # ⚠ It is NOT a check that cannot fail: -RequireFree exits 1 when the named lanes are not fully
  # writable, which is what a wave intending a FULL build should call first.  Its own -SelfTest is
  # 7/7 with every fixture asserted in both directions, including the counting bug it shipped with
  # for one run (-Include is SILENTLY IGNORED under -LiteralPath: it reported 3,836 artifacts for a
  # lane holding 398, and the LOCKED count stayed right, so the report looked plausible).
  # ⚠ AND GATE 0 WENT RED FIRST: "tools\*.ps1 共 20 ... ✗ 未列入: build_lock_probe.ps1", exit 1,
  # measured before this entry existed.  The entry silences a check that was PROVEN to fire.
  'build_lock_probe.ps1'          = 'an informational probe (which build artifacts are held open, and by which PID), not a gate -- a lock is a legitimate state. Own -SelfTest 7/7; use -RequireFree to make it a hard check before a wave that needs a full build.'
  # AI(W906-ST-LEDGER) 20260923: it EXECUTES every F5-startable exe, five times each.
  # Port-isolated (BasePort 8300) and it refuses to start while ctest is running, so it is
  # careful -- but wb_serve / wb_publish resolve to the REAL recipe files (CLAUDE.md, the
  # 20260917 A1 ruling), so this is five real-file-touching runs per exe. The user's
  # 20260918 ruling allows writing real files, but as a deliberate backup -> verify -> delete
  # cycle, not as a side effect of a wrap-up gate that nobody is watching. It also defaults
  # to every build* dir, and this tree has 240 of them.
  # ⇒ run it ON PURPOSE (tools\exe_startup_gate.ps1 -Dirs <one build dir>), not from here.
  'exe_startup_gate.ps1'          = 'EXECUTES the built exes 5x each against REAL recipe files; must be a deliberate run with a backup, not a wrap-up side effect. Also defaults to all 240 build* dirs.'
  # AI(W906-ST-LEDGER) 20260923: the 07:30 entry point, not a wave gate. It RUNS census_gate
  # itself (so including it here would double-run every census), and by default it does a
  # `git fetch` -- network I/O has no place inside a wrap-up gate. Run it by hand in the
  # morning: tools\morning_staleness_check.ps1  (add -NoFetch to stay offline).
  'morning_staleness_check.ps1'   = 'the 07:30 entry point: fetches, then runs census_gate + the "did the ground move under my analysis" check. Wraps this runner s own children, so running it from here would double-run them.'
  # AI(W906-GL-5c) 20260831: queue item 15's candidate counter, moved out of the scratchpad so the
  # next wave can actually run it.  It is a PROBE: its output is a worklist, not a verdict, and the
  # number it prints is EXPECTED to fall as claims get adjudicated -- registering it as a gate would
  # mean registering a number designed to change.
  # ⚠ GATE 0 WENT RED FIRST: "tools\*.ps1 共 22 ... ✗ 未列入: wrapped_claim_probe.ps1", exit 1,
  # measured before this entry existed (log: gl5c_runner2.log).  The entry silences a check that was
  # PROVEN to fire -- same order as build_lock_probe.ps1 and steering_cite_check.ps1.
  # ⚠ It self-tests $NOTE_PAT on every run (10/10, 5 fire / 5 withhold -- GL-5f pinned the
  # "no longer" removal, GL-5g the "RETIRED AT " addition).  The probe it replaces LIFTED
# $NOTE_PAT BY AST AND NEVER USED IT, which is exactly how a discriminator nobody watched fire
  # ends up producing a count that cannot be reconciled with anything.
  'wrapped_claim_probe.ps1'       = 'an informational probe for queue item 15 (how many WRAPPED absence-claims remain unaccounted for), not a gate -- it prints a worklist whose size is meant to shrink, so it has no registered invariant. ⚠ Read its SUPPRESSED list, not only the residual one: suppression fails by silently dropping work, and 2 of 8 suppressions measured on 20260831 fired on polarity-free tokens.'
  'ninja_setup.ps1'               = 'AI(W906-NINJA-ROLLOUT) 20261006: a per-PC setup tool (find / install ninja.exe, list build dirs by generator, -Switch re-creates a make build dir as Ninja), not a gate -- no tree-wide verdict, and -Switch renames build dirs'
}

$fails = New-Object System.Collections.ArrayList
$rows  = New-Object System.Collections.ArrayList

Write-Host "=== 0. gate 清單自我交叉檢查（防止「新增的 gate 被靜默漏掉」）==="
# AI(W906-GL-0w) 20260827: the first version compared TWO DIFFERENT SETS and reported
# pe_truncation_check.ps1 / production_audit.ps1 as "no longer exists" -- they exist and
# had just run green.  Cause: $onDisk is globbed on *_gate.ps1 and those two are not named
# that way, so a set-difference against the glob is meaningless for them.  Existence must
# be asked of the filesystem per script; the glob's only job is finding UNLISTED gates.
$onDisk = @(Get-ChildItem (Join-Path $Tree 'tools') -Filter '*.ps1' -File | ForEach-Object { $_.Name })
$known  = @($Gates | ForEach-Object { Split-Path $_.Script -Leaf })
$unknown = @($onDisk | Where-Object { $known -notcontains $_ -and -not $NotRun.ContainsKey($_) })
$missing = @($Gates | Where-Object { -not (Test-Path -LiteralPath (Join-Path $Tree $_.Script)) } |
             ForEach-Object { Split-Path $_.Script -Leaf })
$staleSkip = @($NotRun.Keys | Where-Object { $onDisk -notcontains $_ })
Write-Host ("  tools\*.ps1 共 {0}；本 runner 跑 {1} 支，明列不跑 {2} 支" -f $onDisk.Count, $known.Count, $NotRun.Count)
if ($staleSkip.Count -gt 0) {
  [void]$fails.Add("不跑清單過期：列了 $($staleSkip.Count) 支已不存在的 -> $($staleSkip -join ', ')")
  Write-Host ("  ✗ 不跑清單裡已不存在: {0}" -f ($staleSkip -join ', ')) -ForegroundColor Red
}
if ($unknown.Count -gt 0) {
  [void]$fails.Add("gate 清單不完整：tools\ 有 $($unknown.Count) 支這個 runner 沒列的 gate -> $($unknown -join ', ')")
  Write-Host ("  ✗ 未列入: {0}" -f ($unknown -join ', ')) -ForegroundColor Red
}
if ($missing.Count -gt 0) {
  [void]$fails.Add("gate 清單過期：列了 $($missing.Count) 支已不存在的 -> $($missing -join ', ')")
  Write-Host ("  ✗ 已不存在: {0}" -f ($missing -join ', ')) -ForegroundColor Red
}
if ($unknown.Count -eq 0 -and $missing.Count -eq 0 -and $staleSkip.Count -eq 0) { Write-Host "  ok" -ForegroundColor Green }

foreach ($g in $Gates) {
  Write-Host ""
  Write-Host ("=== {0} ===" -f $g.Name)
  $sp = Join-Path $Tree $g.Script
  if (-not (Test-Path -LiteralPath $sp)) {
    [void]$fails.Add("$($g.Name): 腳本不存在 $($g.Script)")
    Write-Host "  ✗ 腳本不存在" -ForegroundColor Red
    continue
  }
  $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $sp @($g.Args) 2>&1
  $code = $LASTEXITCODE
  # AI(W906-GL-4a) 20260830: keep stale_premise_scan's output so the 裁決／SCOPED invariants below
  # can parse THIS run instead of launching a second tree-wide scan.  My first version did run it
  # twice and pushed the whole runner past its 600 s budget -- measured, not guessed.
  # AI(W906-GL-5z) 20260901: NO -Width here, and that is now MEASURED IN BOTH CONTEXTS rather
  # than assumed -- see the note beside the two anchored counters below.  I briefly added
  # `-Width 8192` this wave on a wrong inference and then reverted it: Out-String does not wrap,
  # Out-FILE does.  Do not add one back without re-reading that note.
  if ($g.Name -eq 'stale_premise_scan') { $spOutCache = ($out | Out-String) }
  $tail = @($out | Where-Object { $_ -match '^通過|^不通過|^稽核|GATE OK|GATE FAIL|^FAIL' })
  if ($tail.Count -gt 0) { $tail | Select-Object -Last 3 | ForEach-Object { Write-Host ("  {0}" -f $_) } }
  [void]$rows.Add([PSCustomObject]@{ Check = $g.Name; Exit = $code })
    # AI(W906-GL-2p) 20260828: ExpectExit may now be a SET, and that is not a loosening -- it is
  # the only honest shape for an audit whose exit code encodes a COUNT.  Measured this wave:
  # editing Motor\myGALILmotor.h (a HEADER) made every dependent .obj stale, so
  # stub_shadow_audit could decide NOTHING; its 44 rows all moved to UNDECIDED-NO-DWARF, the
  # headline count went to 0 and it exited 0 instead of the registered 1.  Nothing regressed --
  # the SUM invariant below read 0 + 44 = 44 and passed, which is exactly why GL-2i registered
  # the sum instead of the headline.  So the exit code carries no signal this runner should
  # fail on; the sum does.  ⚠️ Keep the set as small as the measured reality: {0,1} means
  # "some or none decidable", NOT "any exit code is fine".
  $want = if ($null -ne $g.ExpectExit) { @($g.ExpectExit | ForEach-Object { [int]$_ }) } else { @(0) }
  if ($want -notcontains $code) {
    [void]$fails.Add("$($g.Name): exit $code（登記為 $($want -join "/")）")
    Write-Host ("  ✗ exit {0}，登記為 {1}" -f $code, ($want -join "/")) -ForegroundColor Red
  } elseif ($code -ne 0) {
    # AI(W906-GL-2i) 20260828: this line used to promise "數字一變就會當掉" -- FALSE.  Only the
    # EXIT CODE is registered here, and these audits exit non-zero for ANY non-zero count, so the
    # count can move freely without failing.  Whatever count matters must get its own invariant
    # (see the stub_shadow_audit SUM check below).  Say what is actually checked, nothing more.
    Write-Host ("  ok (exit {0} = 登記的離開碼；⚠ 只驗離開碼，不驗筆數 —— 筆數要另立不變量)" -f $code) -ForegroundColor DarkYellow
  } else { Write-Host "  ok (exit 0)" -ForegroundColor Green }
}

Write-Host ""
Write-Host "=== GL-G6 安全條件（看匯入，不是看全部符號）==="
$nm = Join-Path $bin 'nm.exe'; $od = Join-Path $bin 'objdump.exe'
if (-not (Test-Path $nm) -or -not (Test-Path $od)) {
  [void]$fails.Add('GL-G6: 找不到 nm/objdump')
  Write-Host "  ✗ 找不到 nm/objdump" -ForegroundColor Red
} else {
  # ⚠ AI(W906-GL-5o) 20260831: build_oracle_probe ADDED to this loop.  The oracle lane BUILT FOR
  # THE FIRST TIME today (158/158 targets, 0 errors), and until this line it was not covered by
  # GL-G6 at all -- i.e. the one lane whose numbers are allowed to be compared with BCB6 was the
  # one lane whose vendor-import safety nobody checked.  A gate that skips the lane that matters
  # most is the "dead control" shape this campaign keeps finding.
  # ⚠ ITS TOOLS COME FROM C:\MinGW, NOT $bin.  MinGW.org's nm/objdump must read MinGW.org's own
  # objects, so the lane carries its own binutils path below.
  foreach ($lane in @('build_sim_nonoracle', 'build_nonoracle', 'build_oracle_probe')) {
    $laneNm = $nm; $laneOd = $od
    if ($lane -eq 'build_oracle_probe') {
      $laneNm = 'C:\MinGW\bin\nm.exe'; $laneOd = 'C:\MinGW\bin\objdump.exe'
      if (-not (Test-Path $laneNm) -or -not (Test-Path $laneOd)) {
        Write-Host ("  ⓘ {0} 找不到 C:\MinGW 的 nm/objdump，跳過" -f $lane) -ForegroundColor DarkYellow; continue
      }
    }
    $exe = Join-Path $Tree "$lane\wb_serve.exe"
    if (-not (Test-Path $exe)) { Write-Host ("  ⓘ {0} 沒有 wb_serve.exe，跳過" -f $lane) -ForegroundColor DarkYellow; continue }
    $imp = @(& $laneNm -u $exe 2>$null | Select-String 'Acm_|MNET_|Mnet|mnet_|GCmd|GOpen|DMCC|P1203|ADVMOT')
    $dll = @(& $laneOd -p $exe 2>$null | Select-String 'DLL Name' | ForEach-Object { ($_.Line -replace '.*DLL Name:\s*', '').Trim() })
    # ⚠ AI(W906-GL-5o) 20260831: PSAPI ADDED to the whitelist, and this is a JUSTIFIED widening of a
    # SAFETY gate, so the evidence is here rather than in a commit message anyone has to go find:
    #   * PSAPI.DLL is "Process Status Helper", CompanyName "Microsoft Corporation", present in BOTH
    #     C:\Windows\System32 and C:\Windows\SysWOW64 -- a Windows system DLL, not a vendor library.
    #   * The tree LINKS IT ON PURPOSE: CMakeLists.txt:545 `target_link_libraries(ht9045_globals
    #     PUBLIC psapi version)`, for cpublic.cpp's FindAndKillProcess.  `version` was already
    #     whitelisted as VERSION.dll; psapi simply had no oracle-lane exe to show up in before.
    #   * MEASURED why only the oracle lane shows it: build_nonoracle imports
    #     KERNEL32/msvcrt/USER32/VERSION/WS2_32 and build_oracle_probe imports those PLUS PSAPI.DLL.
    #     mingw-w64 routes the psapi entry points through kernel32/API sets; MinGW.org imports the
    #     real DLL.  It is an import-resolution difference between toolchains, NOT a new dependency.
    #   * And it is fully resolved: 0 undefined psapi symbols (EnumProcess*/GetModuleFileNameEx*/
    #     GetProcessMemoryInfo/GetModuleBaseName* all satisfied).
    # ⚠ THE GATE'S ACTUAL JOB IS UNCHANGED: no vendor motion library, no Acm_/MNET_/DMC*/ADVMOT
    # import.  That check is the `$imp` line above and it stays exactly as strict.
    $bad = @($dll | Where-Object { $_ -notmatch '^(KERNEL32|msvcrt|USER32|VERSION|WS2_32|ADVAPI32|SHELL32|ole32|OLEAUT32|GDI32|COMDLG32|WINSPOOL|COMCTL32|PSAPI)\.dll$' })
    Write-Host ("  {0,-22} 匯入廠商符號={1}  DLL={2}  非系統 DLL={3}" -f $lane, $imp.Count, $dll.Count, $bad.Count)
    if ($imp.Count -gt 0) { [void]$fails.Add("GL-G6 $lane`: 匯入了 $($imp.Count) 個廠商符號") }
    if ($bad.Count  -gt 0) { [void]$fails.Add("GL-G6 $lane`: 連進非系統 DLL -> $($bad -join ', ')") }
  }
}

# AI(W906-GL-2i) 20260828: the stub_shadow_audit count, ACTUALLY registered.
#
# ⚠ WHY THE SUM AND NOT THE HEADLINE NUMBER.  A comment-only edit makes that file's .obj older
# than its source, and the audit then WITHHOLDS the verdict for symbols whose real body lives
# there -- the row moves from STUB-SHADOWS-LIVE-REAL to UNDECIDED-NO-DWARF.  GL-2i did exactly
# that: editing RotateKit\aRotateKIT_Out.cpp's comments moved CheckRotateOutNotFinish and
# MoveOutRotateToDegreeAtSameTime, giving 44/0 -> 42/2.  So the headline number legitimately
# moves in a wave that changed no code, and registering it would fire false alarms.
# THE SUM DOES NOT MOVE: a row can only shift between those two buckets, never leave both.
# That is what makes it registerable -- and a registration that cannot fail is not one.
# AI(W906-GL-2k) 20260828: THE INDENT-LOSS CHECK.
#
# Twice in two waves (GL-2j, GL-2k) a line-slice replacement dropped the block's leading
# whitespace: the new comment text landed at column 0 inside an indented register block.
# NEITHER GUARD CAN SEE IT -- criterion A' ignores it (all the change is inside comments) and the
# line-count guard is satisfied.  Only reading the block back caught it, both times.
#
# So make it mechanical.  The predicate is exact, not a heuristic: a line whose HEAD version
# STARTED WITH WHITESPACE and whose new version DOES NOT.  No style opinion, no threshold.
# ⚠ Only valid while line counts are neutral (this campaign's rule), so the check SKIPS a file
# whose count changed and says so rather than comparing misaligned indices.
function Test-IndentLoss($tree, $gitExe) {
    $bad = @()
    $changed = @(& $gitExe -C $tree diff --name-only -- '*.cpp' '*.h' 2>$null)
    foreach ($rel in $changed) {
        $now = @([System.IO.File]::ReadAllLines((Join-Path $tree $rel)))
        $head = @(& $gitExe -C $tree show "HEAD:$rel" 2>$null)
        if ($head.Count -ne $now.Count) {
            Write-Host ("  ⓘ {0}: 行數 {1} -> {2}，索引對不齊，跳過縮排檢查（不是通過）" -f $rel, $head.Count, $now.Count) -ForegroundColor DarkYellow
            continue
        }
        for ($i = 0; $i -lt $now.Count; $i++) {
            if ($head[$i] -eq $now[$i]) { continue }
            if (($head[$i] -match '^\s') -and ($now[$i] -notmatch '^\s') -and ($now[$i].Length -gt 0)) {
                $bad += ("{0}:{1}" -f $rel, ($i + 1))
            }
        }
    }
    return $bad
}

$EXPECT_SHADOW_SUM = 44
$auditDoc = Join-Path $Tree 'docs\GL_S0b_SHADOW_AUDIT.md'
Write-Host ""
Write-Host "=== stub_shadow_audit 計數不變量（exit code 抓不到這個）==="
if (-not (Test-Path $auditDoc)) {
  Write-Host "  ✗ 找不到 docs\GL_S0b_SHADOW_AUDIT.md —— 稽核沒產出，不能當通過" -ForegroundColor Red
  [void]$fails.Add("stub_shadow_audit: 產出文件不存在")
} else {
  $shadow = $null; $undec = $null
  foreach ($ln in [System.IO.File]::ReadAllLines($auditDoc)) {
    if ($ln -match '^\|\s*`STUB-SHADOWS-LIVE-REAL`\s*\|\s*(\d+)\s*\|') { $shadow = [int]$Matches[1] }
    if ($ln -match '^\|\s*`UNDECIDED-NO-DWARF`\s*\|\s*(\d+)\s*\|')     { $undec  = [int]$Matches[1] }
  }
  if ($null -eq $shadow -or $null -eq $undec) {
    Write-Host "  ✗ 解析不到那兩個桶的數字 —— 文件格式變了，拒絕當成通過" -ForegroundColor Red
    [void]$fails.Add("stub_shadow_audit: 無法解析計數（文件格式可能已變）")
  } else {
    $sum = $shadow + $undec
    Write-Host ("  STUB-SHADOWS-LIVE-REAL {0} ＋ UNDECIDED-NO-DWARF {1} = {2}（登記 {3}）{4}" -f `
                $shadow, $undec, $sum, $EXPECT_SHADOW_SUM,
                $(if ($sum -eq $EXPECT_SHADOW_SUM) { 'ok' } else { '✗' }))
    if ($sum -ne $EXPECT_SHADOW_SUM) {
      [void]$fails.Add("stub_shadow_audit 計數: 遮蔽＋未定 = $sum，登記 $EXPECT_SHADOW_SUM")
    }
    if ($undec -gt 0) {
      Write-Host ("  ⓘ 其中 {0} 筆因 obj 比原始碼舊而暫時無法判定（純註解波次的正常現象，非回歸）" -f $undec) -ForegroundColor DarkGray
    }
  }
}

# =============================================================================
#  stale_premise_scan 的「裁決數」與「SCOPED 數」不變量
#
#  AI(W906-GL-4a) 20260830: NEW.  這道是被一個實際漂掉的數字逼出來的，不是預防性的。
#  ⚠ GL-3z 的 DEVLOG（兩處）與 commit 訊息都寫「裁決 22」，實測登記表當時只有 **20**。
#  18（GL-3y）＋2（GL-3z 的 fContactCT）= 20，我寫成 22，而**沒有任何東西會擋**：
#  這兩個數字每一波都出現在 DEVLOG 的驗收段，卻從來沒有登記值，所以它們是
#  「$EXPECT_TREEWIDE 存在的理由」的翻版 —— 只被回報、不被守。
#  裁決數特別值得守，因為每一筆裁決都是一個**排除**：它把一列從 I1 的計數裡拿掉。
#  工具自己已經每次重驗裁決的 Evidence 子字串（stale_premise_scan.ps1:873），
#  所以「證據漂掉」會紅；但「裁決總數悄悄多一筆」原本不會。現在會。
# =============================================================================
# AI(W906-GL-5c) 20260831: 27 -> 29.  TWO NEW ROWS, AND THEY EXIST BECAUSE THIS WAVE MADE THEM
# REACHABLE.  Both claims were WRAPPED across two lines, i.e. invisible to Test-DeadLine (which
# needs the entity AND the dead phrase on ONE line).  I nearly registered forms\fQwertyKey.h:33
# while it was still wrapped -- that row would have printed NOTHING, left this number at 27, and
# looked exactly like protection.  A CONTROL THAT CANNOT FIRE IS NOT A CONTROL.  Annotating each
# claim on its own line is what turned both into live, re-verified exclusions:
#   forms\fQwertyKey.h:33            AMBIGUOUS-SUBJECT   (subject excludes this very header)
#   Public\MyProductionRecord.cpp:896 PREMISE-STILL-TRUE (verdict holds, EVIDENCE SCOPE stale)
# ⚠ Each row carries an Evidence substring that stale_premise_scan.ps1:1198 re-verifies EVERY run,
# so a drifting adjudication fails instead of silently excluding.
# ⚠ THE SECOND ROW EXISTS BECAUSE OF A MISTAKE WORTH KEEPING: :896's note first said it was
# "deliberately NOT tagged RE-MEASURED/EXPIRED" -- and by NAMING those markers the line came to
# BEAR them, so $NOTE_PAT matched, the line was classified NOTED, and a LIVE claim was silently
# suppressed.  $NOTE_PAT cannot tell MENTIONING a marker from CARRYING one.
# AI(W906-GL-5e) 20260831: 29 -> 30.  forms\fQwertyKey.h:34, AMBIGUOUS-SUBJECT.
# ⚠ THIS ROW EXISTS BECAUSE THE PREVIOUS WAVE'S ANNOTATION CREATED THE CANDIDATE.  The probe rule is
# "dead phrase on this line, entity on the PREVIOUS line", so GL-5c writing TfQwertyKey onto :33
# promoted :34 into the candidate set.  Not a false positive: :34 is the SECOND of the two greps that
# forms\fQwertyKey.h:36-37 itself requires to be re-run at wave close, and nobody had re-run it.
# => ANNOTATING A WRAPPED CLAIM CAN SURFACE ITS NEIGHBOUR.  That is a feature, not noise, but it means
# the candidate set is not monotonically shrinking and must be RE-MEASURED, never subtracted.
# AI(W906-GL-5g) 20260831: 30 -> 29.  A HAND-ADJUDICATION WAS RETIRED, NOT ADDED -- the first time
# this number has gone DOWN.  $NOTE_PAT learned the marker "RETIRED AT " (tools\stale_premise_scan.ps1),
# so Command.cpp:10440 now classifies as NOTED unaided and its 'NOTED (pattern gap)' row was removed.
# ⚠ ORDER MATTERED AND WAS FOLLOWED: pattern changed FIRST, scan re-run with the row STILL PRESENT
# (it printed "Command.cpp:10440 -> NOTED (pattern gap)", count still 30), THEN the row removed and
# the count re-measured at 29.  That sequence proves the row was REDUNDANT rather than just deleted.
# ⚠ MEASURED SIDE EFFECTS: none.  Exactly one flagged row tree-wide gains a note from "RETIRED AT ",
# and it is that one.  DEAD-NO-NOTE stayed 0 and SCOPED-AND-TRUE stayed 13.
$EXPECT_ADJUDICATED = 29    # GL-4d: 24 -> 27（fYieldMonitoring 2 ＋ fCounterClear 1）
# AI(W906-GL-5f) 20260831: 12 -> 13.  ONE row moved, and it moved into a BETTER classification, not
# into a finding: dropping `no longer` from $NOTE_PAT (tools\stale_premise_scan.ps1) stopped
# Motor\myMN200motor.cpp:271 being suppressed by unrelated prose at :259 ("this tree's de-VCL'd
# MachineDefine.h no longer chain-includes ..."), and Get-LineClass then reached
# Test-ScopedAndTrue on its own and returned SCOPED-AND-TRUE.  That verdict is COMPUTED from the
# tree, not read out of a comment's vocabulary, which is the whole point of the change.
# ⚠ A/B ATTRIBUTION ON THE REAL TOOL, not on a replica:
#     with `no longer`   fNote entity: SCOPED-AND-TRUE 0, NOTED 7   total SCOPED 12
#     narrowed           fNote entity: SCOPED-AND-TRUE 1, NOTED 6   total SCOPED 13
# ⚠⚠ AND I ALMOST ADDED AN ADJUDICATION ROW HERE INSTEAD.  My on/off replica predicted the row would
# become DEAD-NO-NOTE, so the plan was $EXPECT_ADJUDICATED 30 -> 31.  The proof-of-red run came back
# exit 0 / DEAD-NO-NOTE 0 -- because the replica modelled only part of Get-LineClass and missed the
# Test-ScopedAndTrue fallback at :400.  ⚠ GL-5g CORRECTION TO THE NEXT SENTENCE, WHICH I WROTE
# HERE: reading the code (:1267-1279) shows the adjudication lookup runs for ANY class except
# NONE, so a SCOPED-AND-TRUE row WOULD have fired -- it is only a row whose line yields NONE (no
# dead phrase + entity on the SAME line, i.e. the wrapped-claim case) that can never fire.  The
# right reason not to add one here was different and still stands: the row was not NEEDED, and
# adding it would have masked a verdict COMPUTED from the tree behind a hand-adjudication.
# RUNNING THE REAL GATE BEFORE REGISTERING IS STILL WHAT CAUGHT IT.
$EXPECT_SCOPED      = 13    # GL-4d: 11 -> 12 —— +1 就是 fFixAICCD 自己那一列，之前因為該實體未登記而不被計入
Write-Host ""
Write-Host "=== stale_premise_scan 裁決／SCOPED 計數不變量（exit code 抓不到這個）==="
$spOut = $spOutCache
if ([string]::IsNullOrWhiteSpace($spOut)) {
  # ⚠ Do NOT silently pass.  If the cache is empty the gate above did not run, and re-running here
  # would hide that while doubling the runtime.
  Write-Host "  ✗ 上面的 stale_premise_scan gate 沒有留下輸出 —— 不重跑，直接當失敗" -ForegroundColor Red
  [void]$fails.Add("stale_premise 計數: 拿不到 gate 的輸出（那道 gate 沒跑？）")
  $spOut = ''
}
# ⚠⚠ AI(W906-GL-4o) 20260830: ANCHORED TO THE ADJUDICATION ROW'S SHAPE, was a bare marker.
# WHY: GL-4l added a FOLD to stale_premise_scan's evidence lines and used `ⓘ ` for it -- the same
# marker the adjudication rows print -- so nine evidence lines silently became nine phantom
# adjudications and this gate went RED with 裁決 36 vs the registered 27.  The gate was telling the
# truth about `ⓘ ` and a LIE about adjudications, which is the worse failure: the verdicts had not
# moved, only the number that GUARDS them.
# The adjudication row is emitted as "    ⓘ {File}:{Line} -> {Expect}" (stale_premise_scan.ps1,
# the $adjHit loop), i.e. EXACTLY FOUR leading spaces at line start.  Measured on real captured
# output 20260830: 27 lines carry the marker, ALL 27 have exactly 4 leading spaces, and no other
# line in the whole report carries it.
# ⚠ ` {4}` NOT `\s{4}`: \s also matches CR/LF/TAB, so it would drift back toward matching anywhere.
# ⚠ REVERSE CONTROL (measured on the REAL captured output, not a toy string) -- the point is that
# the anchor is not WEAKER, it is CORRECT:
#      情境                      舊(裸符號)   新(錨定)
#      baseline                      27         27
#      one GENUINE extra 裁決        28         28    <- still caught, so it is still a gate
#      an evidence line with `ⓘ `    28         27    <- no longer miscounted (the CXLVIII bug)
#      both at once                  29         28    <- counts the real one only
# ⚠ stale_premise_scan's fold marker STAYS `※` even though this anchor would now tolerate `ⓘ `:
# the anchor and the distinct marker are independent defences, and changing published output again
# for cosmetics buys nothing.
$nAdj = @([regex]::Matches($spOut, '(?m)^ {4}ⓘ ')).Count
$nSco = 0
# ⚠⚠ AI(W906-GL-5w) 20260901: ANCHORED TO THE COUNT ROW'S SHAPE, was a bare token anywhere.
# WHY: the 裁決 counter above was anchored by GL-4o, but THIS one was left unanchored -- and
# `SCOPED-AND-TRUE (\d+)` SUMS EVERY HIT ANYWHERE IN THE OUTPUT.  stale_premise_scan prints row
# DETAIL lines that carry raw source-comment text, so a comment containing the literal shape
# `SCOPED-AND-TRUE 5` would add 5 to the total.
# ⚠ MEASURED 20260901, INCLUDING THE REASON I FIRST WROTE HERE, WHICH WAS WRONG:
# I justified this as "fix it before batch 1 lands, batch 1 brings ~1,484 unvetted citations".
# Then I measured batch 1: `SCOPED-AND-TRUE` + digits appears **0 times** in its .cpp/.h -- and
# 0 times on main.  So there is NO incoming risk from batch 1; the exposure is purely POTENTIAL
# in both trees.  The real reason to anchor it is the ASYMMETRY: the 裁決 counter next to it IS
# anchored (GL-4o) and this one was not, and that mismatch already produced a real false alarm
# in GL-5v -- I measured this counter's sibling with a loose grep, "found" a break that did not
# exist, and the fix for that non-problem was the only genuine defect of that wave.
# (Queue items state a CAUSE; measure the cause before acting on it.)
# ⚠ THE COUNT ROWS HAVE **TWO** LEADING SPACES, NOT FOUR (measured, not assumed):
#     '  SCOPED-AND-TRUE 3'          <- the per-entity count row, 2 spaces
#     '    · [SCOPED-AND-TRUE] ...'  <- a GL-5v detail row, 4 spaces + `· [`
# so the anchor is ` {2}` -- copying the 裁決 counter's ` {4}` would have matched nothing and
# reported 0.  (0 still goes RED here because $EXPECT_SCOPED is 13, i.e. a broken parse fails
# loudly rather than silently passing -- that stops being true if the registered value ever
# becomes 0, so do not register 0 without re-reading this.)
# ⚠ ` {2}` NOT `\s{2}`: \s also matches CR/LF/TAB and would drift back toward matching anywhere.
# ⓘ AI(W906-GL-5z) 20260901: GL-5w's "no -Width needed" conclusion STANDS, and it is now measured
# in BOTH contexts instead of one.  Recorded at length because I got this wrong in the middle of
# GL-5z and briefly "corrected" a correct note -- the retraction is the useful part.
#   Out-STRING  (what this line uses):  does NOT wrap.  A 424-char row survives intact, default
#       and `-Width 8192` byte-identical, measured INSIDE a background task whose
#       $Host.UI.RawUI.BufferSize.Width is 120.  A hand-built malicious row designed to put
#       `SCOPED-AND-TRUE 5` just past column 119 forged ZERO matches under either setting.
#   Out-FILE    (how logs get captured):  DOES wrap, at 119, but only in a BACKGROUND task.
#       Fingerprint on real data: a 15,430-line build captured to 19,285 file lines = 3,855
#       continuations, max line exactly 119, zero lines >= 120, with one line ending
#       `[-Wimplicit-fallthrough` and the NEXT line being just `=]`.  The same capture made from a
#       foreground call keeps its 1,241-char lines.
# So the wrapping hazard is real but lives in the CAPTURE pipeline, not in this line.  My first
# pass this wave transferred the Out-File evidence onto Out-String without testing Out-String --
# the same shape of error as measuring the wrong arm.  Reverted.
# ⚠ THE LESSON THAT DOES APPLY HERE, because it bit this very wave: any `| Out-File` capture made
# inside a background task is wrapped at 119, so REGEX MEASUREMENTS OVER SUCH A FILE ARE UNSAFE.
# It silently corrupted GL-5z's own first warning-category table -- 105 of 1,343 `[-W...]` tags
# parsed, because the closing bracket had moved to the next line -- and undercounted the warnings
# themselves (2,389 vs 2,490 real).  Counts of tokens appearing EARLY in a line (`error:`,
# `warning:`) survive wrapping and stayed correct, which is why "0 errors", object counts and
# archive sizes from those same logs are still good.  Capture with
# [System.IO.File]::WriteAllLines to bypass the formatting pipeline entirely.
# ⚠ REVERSE CONTROL, measured on the REAL captured output (not a toy string), and **the injection
# was verified to actually fire** before reading the numbers -- my first attempt at the "genuine
# extra row" cell silently injected nothing (`0$` cannot match CRLF text, and [regex]::Replace has
# no count overload so the `1` I passed became RegexOptions), which would have let a DEAD control
# read as "both still catch it":
#      情境                                   舊(裸token)   新(錨定)
#      baseline                                    13          13    <- not weaker
#      one GENUINE extra SCOPED row                14          14    <- still caught, still a gate
#      detail text carrying `SCOPED-AND-TRUE 5`    23          13    <- no longer pollutable
#      both at once                                24          14    <- counts the real one only
foreach ($m in [regex]::Matches($spOut, '(?m)^ {2}SCOPED-AND-TRUE +(\d+)')) { $nSco += [int]$m.Groups[1].Value }
if ($spOut -notmatch 'GATE OK' -and $spOut -notmatch 'GATE FAIL') {
  Write-Host "  ✗ 解析不到 stale_premise_scan 的輸出 —— 拒絕當成通過" -ForegroundColor Red
  [void]$fails.Add("stale_premise 計數: 無法解析輸出")
} else {
  Write-Host ("  裁決 {0}（登記 {1}）{2}" -f $nAdj, $EXPECT_ADJUDICATED, $(if ($nAdj -eq $EXPECT_ADJUDICATED) { 'ok' } else { '✗' }))
  if ($nAdj -ne $EXPECT_ADJUDICATED) { [void]$fails.Add("stale_premise 裁決數 = $nAdj，登記 $EXPECT_ADJUDICATED —— 每一筆裁決都是一個排除，數字要跟著理由一起改") }
  Write-Host ("  SCOPED-AND-TRUE {0}（登記 {1}）{2}" -f $nSco, $EXPECT_SCOPED, $(if ($nSco -eq $EXPECT_SCOPED) { 'ok' } else { '✗' }))
  if ($nSco -ne $EXPECT_SCOPED) { [void]$fails.Add("stale_premise SCOPED 數 = $nSco，登記 $EXPECT_SCOPED") }
}

if (-not $SkipSlow) {
  Write-Host ""
  Write-Host "=== cite_check 規模不變量（壞掉的掃描也是決定性的，只有規模抓得到）==="
  # Measured 20260827.  Tree unchanged => these must not change.  GL-0q(b): a
  # case-insensitive $s/$S collision cut every source file's scan after line 1;
  # syntax check, exit code, idempotence and all four gates PASSED anyway.
  # AI(W906-GL-1l) 20260828: 16645 -> 16655.  The invariant FIRED and it was right to: GL-1l
# corrected five gate comments that each asserted `EnableTraymapCheckFunction` has no port body,
# and each correction names the two locations that disprove it -- `ainarm2.cpp:2631` and
# `aHotPlateSubstrate.h:1081`.  Five lines x two new filename-carrying citations = EXACTLY +10,
# counted and confirmed before touching this number.  Raising a baseline is only legitimate
# when the delta is explained to the citation; otherwise the invariant is being silenced.
#
# AI(W906-GL-1m) 20260828: 16655 -> 16668.  GL-1m corrected SEVEN gate comments in cprod.cpp
# whose premise "X@not declared anywhere in ported tree" is false, and each correction cites
# the declaration that disproves it.  DELTA RECONCILED MECHANICALLY, not by hand: counting
# `file:line` matches in the `+` and `-` lines of `git diff -U0 -- cprod.cpp` gives +13 added,
# 0 removed, net +13 -- exactly the tool's 16668 - 16655.  A hand tally of the `#if 0` first
# lines gave 3 and would have been wrong, because the citations are on continuation lines.
# AI(W906-GL-1n) 20260828: 16668 -> 16675.  GL-1n corrected three comments that rested on the
# retired TfObserverShim -- forms/fObserver.h's pre-swap banner, Command.cpp's GATE REGISTER
# item 10 (which "re-verified" the claim by reading that banner rather than the code), and
# cMyDB.cpp:1349.  Reconciled the same mechanical way: `git diff -U0` gives 8 citations added
# and 1 removed, net +7 -- exactly the tool's 16675 - 16668.
# AI(W906-GL-1p) 20260828: 16675 -> 16678.  GL-1p corrected three gate comments whose
# "TfLotInfo has no such member" premise is dead, each naming the declaration that disproves it
# (forms/fLotInfo.h:1513 / :1213 / :1304).  Reconciled mechanically: `git diff -U0` gives 4
# citations added, 1 removed, net +3 -- exactly the tool's 16678 - 16675.
# AI(W906-GL-1q) 20260828: 16678 -> 16681.  GL-1q stamped 41 verified-still-true absence claims
# with the tree's own `re-verified absent` marker and corrected two gate comments whose premises
# it had verified dead in earlier waves without ever fixing the text.  Reconciled mechanically:
# `git diff -U0` gives 26 citations added and 23 removed (the marker insertions land on lines
# that already carried citations, counted on both sides), net +3 -- exactly 16681 - 16678.
# AI(W906-GL-1r) 20260828: 16681 -> 16683.  Two citations added by ainarm9045.cpp:5263's premise
# correction.  ⚠ THE FIRST RECONCILIATION SAID +5 AND DID NOT MATCH: the diff sweep counted 3
# citations added inside tools/census/expired_gate_scan.py, and cite_check scans only .cpp/.h.
# Broken down per file: ainarm2.cpp +2/-2 = 0, ainarm9045.cpp +2/-0 = 2, the .py +3 (not
# counted) -> net +2.  A mismatch means find the reason, never add a fudge term.
# AI(W906-GL-1s) 20260828: 16683 -> 16684, and this one was PREDICTED BEFORE MEASURING.
# GL-1s corrected rotted citations in five files, almost all pure swaps (one citation out, one
# in, net 0): asortarm.cpp x4, canary_support.cpp x2, csystem.cpp x2, Command.cpp x2,
# ainarm9045.cpp's stale DoAutoSkipCheck integration note.  Exactly two lines move the count --
# +1 for the newly added `acatchtray_shims.h:350` (which is WHY the TMyMessageBoxShim citations
# were stale: FW-G24 extracted the class to mymessbox_shim.h and that header includes it back),
# and the quote subset drops 130 -> 129 because ainarm9045.cpp's note no longer needs to quote
# the retired stub.  Predicted +1, measured +1.
# AI(W906-GL-1t) 20260828: 16684 -> 16685, predicted before measuring again.  GL-1t worked the
# MANY class: 8 pure swaps in aoutarm9045.cpp (a family of "REPLACES the one-line offline stub
# aoutarm_shims.cpp:NN" comments that had ALL drifted as that file grew retirement blocks), one
# each in cinitial.cpp and forms/fObserver.h, all net 0.  The +1 is csystem.cpp's GATE G01 WHY
# block, where FOUR citations in ONE paragraph were stale and the replacement names two more
# (ainarm2.cpp:925 / :867).  The quote subset drops 129 -> 128 because that block no longer
# quotes a gate line that had moved.
# AI(W906-GL-1u) 20260828: 16685 -> 16684.  GL-1u closed the MANY class (4 -> 2, and both
# survivors are citations GL-1t verified CORRECT).  Nine citations in ainarm9045.cpp pointed at
# the wrong stub block -- the real CheckShuttleSensor_9045 family sits at :2406-2435, all ten
# already PT-W7d RETIRED with live bodies further down, PROVED with the new
# `live_lines.ps1 -PreprocOnly` in BOTH configs.  Reconciled mechanically: `git diff -U0` gives
# 14 citations added and 15 removed, net -1 -- exactly the tool's 16684 - 16685.  (The removals
# are prose that no longer needs to name a stub line now that the premise is dead.)
# AI(W906-GL-1w) 20260828: 16684 -> 16694, and this one GOES UP because the corrections had to
# NAME where things actually are.  Both of Command.cpp's "STUB COLLISIONS" blocks were entirely
# stale -- all five methods have their only body in Command.cpp itself (:1789 / :1207 / :1218 /
# :10894 / :11062), and one block still told a future integrator to delete forms/fMain.cpp:282-283,
# which today are LIVE TfMain::Reset / BtnOneCycleClick.  Naming the five real homes plus the two
# retirement records costs citations.  Reconciled mechanically: `git diff -U0` gives 20 added,
# 10 removed, net +10 -- exactly 16694 - 16684.
# AI(W906-GL-1x) 20260828: 16694 -> 16702.  GL-1x worked the QUEUE DOCUMENT's two strongest
# buckets instead of the biggest one.  Measured: of 22 WRONG-FILENAME rows, TEN were filenames
# ABBREVIATED IN A LIST (a shared prefix dropped: `_EC.cpp` for uHGemHT9045_EC.cpp,
# `_2x2_4_14.cpp` for aoutarm9045_2x2_4_14.cpp) and TEN were REAL Windows-SDK/libc headers that
# simply are not in either HT9045 tree (they live in the toolchain's
# i686-w64-mingw32\include\) -- neither class is a defect.  Expanding the abbreviations both
# fixes the citations and removes them from the bucket, which is why the count RISES: a full
# filename is longer than an abbreviated one -- but ONLY for a citation the tool could not
# resolve before; expanding one it ALREADY counted is a REPLACEMENT, net 0.  ⚠ I first wrote
# 16703 here from that wrong intuition and the measurement said 16702; the last expansion
# (_2x8_32.cpp -> aoutarm9045_2x8_32.cpp) swapped one citation for another.  Measure, then record.
# Reconciled mechanically: `git diff -U0` gave +16/-8 = +8 at the point of measurement.
# AI(W906-GL-1z) 20260828: 16702 -> 16703.  GL-1z added a SYSTEM-HEADER bucket to cite_check
# (ten WRONG-FILENAME rows were real toolchain headers, measured present under
# mingw32\i686-w64-mingw32\include), which moves no citations, and then corrected THREE gates in
# cinitial.cpp whose "0 definitions tree-wide" premise is false -- n2-2 (:8152), n2-4 (:8560),
# n2-13 (:9456).  Each names its real home now, and one of them needed one more citation than it
# removed.  Reconciled mechanically: `git diff -U0` gives +6/-5 = net +1.
# AI(W906-GL-2a) 20260828: 16703 -> 16709.  GL-2a SEARCHED for the shape GL-1z hit by accident:
# gate comments carrying an ACTION-BEARING SELF-DESCRIPTION.  Corrected the DoInArm_SuckerMap
# cluster -- ainarm2.cpp's banner had SIX stale numbers and two gates (ainarm2.cpp:4429,
# ckernel.cpp:1047) whose `no ported definition` premise is false (body ainarm2.cpp:4478,
# compiled in, declared aHotPlateSubstrate.h:1088 which ckernel.cpp already includes).
# Naming real homes and the surviving/retired shadows costs citations.  Reconciled
# mechanically: `git diff -U0` gives +14/-8 = net +6.
# AI(W906-GL-2b) 20260828: 16709 -> 16717.  GL-2b worked the action-bearing line and hit the
# SAFETY-ADJACENT one the tree flags itself: aoutarm.cpp (G4) says SetOutArmSpeed `must be
# retired before any real-machine run`, and its `untranslated` premise is DEAD -- real body
# cinitial.cpp:10563, decl cinitial.h:144, compiled in both configs, and aoutarm.cpp already
# includes cinitial.h.  Naming the real body plus the two mis-cited sibling shadows
# (csystem.cpp:5137-5138 not :1268; AutoClean.cpp:186 not :177) costs citations.
# Reconciled mechanically: `git diff -U0` gives +12/-4 = net +8.
# AI(W906-GL-2c) 20260828: 16717 -> 16716.  GL-2c worked the `MAIN LOOP deletes` slice (5 rows)
# and found EVERY ONE OF THEM ALREADY DONE: the three ainarm9045.cpp stubs (:2337 / :2365 /
# :2377, not the cited :2295 / :2305 / :2309) are PT-W7d RETIRED and not compiled in either
# config; aoutarm.cpp:551's shadow is retired at :550; and SetInArmHome's `two edits that must
# land together` have both landed (aHotPlateSubstrate.h:915 has golden arity, the no-op is
# retired at .cpp:1081).  Replacing five `must be deleted` instructions with `already done`
# removes more citations than it adds.  Reconciled mechanically: +13/-14 = net -1.
# AI(W906-GL-2d) 20260828: 16716 -> 16720.  GL-2d worked the `MUST BE RETIRED` slice.  Same
# verdict as GL-2c: THE RETIREMENTS ARE DONE.  SetMotorScaleSpeed's no-op is retired at
# acatchtray_shims.cpp:179-181 (cited :166), so the ~80 call sites now reach the live body
# cinitial.cpp:13321; SetTechDataToProd_AutoClean's duplicate is gated at cinitial.cpp:11840
# (cited :11734) leaving AutoClean.cpp:313 (cited :302) live -- both measured -PreprocOnly in
# both configs.  N3-G8's premise is dead too (DoStructUnitConvert real at cUnitConvert.cpp:675,
# decl :118) but it STAYS SHUT: it must pair with G7 and it changes recipe unit conversion.
# Naming real homes costs citations.  Reconciled mechanically: +15/-11 = net +4.
# AI(W906-GL-2g) 20260828: 16720 -> 16721 (+18/-17 = net +1).  Nine stale-instruction paragraphs
# rewritten across five files plus the atester_shims.cpp:222 gate comment, which named the WRONG
# FILE for DoFRTCAutoModelVerify's real body (said aTester_Rear.cpp; it is aTester_Front.cpp:5923).
# Correcting stub line numbers is mostly a REPLACEMENT, so a nine-paragraph wave moving the total
# by 1 is the expected shape, not a sign nothing happened.  Reconciled with `git diff -U0`.
# AI(W906-GL-2i) 20260828: 16721 -> 16722 (+7/-6 = net +1).  Three rows off the -Unmatched
# reading list: RotateKit\aRotateKIT_Out.cpp's D-1/D-2 (cited aoutarm_shims.cpp:91 and :156 --
# actually :90 and :178, and :156 is DoSortArmSuckPreOn's gate), and csystem.cpp:9427 (call sites
# :4157/:5983 -> :4166/:5992).  ainarm2.cpp stays 881: its edit swapped one citation for another.
# AI(W906-GL-2j) 20260828: 16722 -> 16727 (+6/-1 = net +5).  Six bucket-① reading-list rows worked
# in csystem.cpp (G24 :23541, G-ATK :23602, G29 :18551, h4-G4 :30687, G03 :21539) and
# ainarm9045.cpp (k4-G4 :7821).  Five premises VERIFIED STILL TRUE -- naming WHERE the absence is
# recorded and WHY a grep misleads adds citations without removing any, so a net +5 is the right
# shape for a wave that confirmed rather than corrected.  One real citation fix: :724 -> :733.
# AI(W906-GL-2m) 20260828: 16727 -> 16728 (+1/-0).  Three structural-discriminator rows annotated
# (ainarm9045.cpp:3272, aTester_Front.cpp:10663, cinitial.cpp:7485); only one added a
# filename-qualified citation (atester_shims.cpp:207) -- the rest are bare :NNNN in-file refs,
# which the citation predicate deliberately does not count.
# AI(W906-GL-2n) 20260828: 16728 -> 16730 (+2/-0).  Three ainarm9045.cpp reading-list rows
# (:3046 / :6890 / :8584) plus the RESUME's queue item (1) at :3774, whose premise turned out to
# be dead (there is NO GATE k6-G1 anywhere in the tree).
# AI(W906-GL-2o) 20260828: 16730 -> 16736 (+6/-0).  Three reading-list rows worked, and naming
# CMakeLists targets / real symbol homes / the uTemp_Set definition costs citations without
# removing any: tests\test_ga1_language.cpp (:9 / :23 / :37), SECSGEM\uHGemHT9045_SV.cpp:333,
# SECSGEM\uHGemHT9045_EC.cpp:144.
# AI(W906-GL-2r) 20260828: 16736 -> 16739 (+3/-0).  Three "re-run the absence commands at
# integration" rows answered by actually re-running them (aTester_Front.cpp:5450,
# aTester_Rear.cpp:5707, atester.cpp:12005); naming where each symbol landed costs citations.
# AI(W906-GL-2x) 20260829: 16739 -> 16738 (net -1), and the -1 is reconciled line by line rather
# than accepted: AutoClean\AutoClean.cpp:230 DROPPED `cmydef.cpp:5816` (that claim -- "the enclosing
# gate opens at :5816" -- was simply false; :5816 is `int iAQLBin=0;` and the TODO(W6) block opens at
# :6011, now stated as a bare range), which is -1.  cinitial.cpp:13603 traded `cUnitConvert.h:18` for
# `forms\fShowMessage.h:28` and kept the old number as ~~struck~~, so it is +1 counted / -1 skipped =
# net 0.  Every other edit this wave swapped one citation for exactly one citation (27 wrong-FILE
# fixes, 2 gate headers, 16 BARE line numbers in cmydef.cpp's index banner -- bare numbers carry no
# filename and are not counted here at all).
# ⚠️ MY RAW-REGEX CROSS-CHECK READ 16,756 AND THAT IS NOT A CONTRADICTION: this invariant is
# cite_check's 「帶檔名的引用」, which skips ~~strikethrough~~ citations and cite_check's own report
# block.  A one-liner over every .cpp/.h counts a DIFFERENT QUANTITY WITH THE SAME NAME.  Trust this
# gate, not the one-liner (GL-2u recorded the same lesson about a hand count that was low by half).
# AI(W906-GL-3c) 20260829: 16738 -> 16737 (net -1), reconciled rather than accepted.
# ainarm2.cpp:5631 DROPPED `ainarm2.h:81`, a claim that was simply FALSE: measured, golden
# declares Check_QA_ModeCount in NO header at all (its only golden homes are ainarm2.cpp:160,
# the body -- which the same comment cites CORRECTLY -- and :1653, a call site), and golden
# ainarm2.h:81 is `extern int iHotWhichShuttle[2][50][50];`.  The line now says so.
# The other three edits this wave are citation-for-citation, and aoutarm9045_2x8_32.cpp:374's
# wrong-FILE fix keeps its old value as ~~csystem.h:90~~, which cite_check SKIPS by design --
# so that one is +1 counted / -1 struck = net 0 (the skipped total is printed: 2).
# AI(W906-GL-3h) 20260829: 16737 -> 16738 (net +1), reconciled rather than accepted.  Rewriting
# cinitial.cpp's COLLISION NOTICE removed 4 citations (ckernel_shims.cpp:278 + :400 at :6967,
# ckernel_shims.cpp:400 at :7171, cinitial.cpp:6972 at ckernel_shims.cpp:279) and added 5 counted
# ones (ckernel_shims.cpp:278, :111-117, :211, ckernel_shims.h:137, cinitial.cpp:6981) -- naming
# the STILL-OUTSTANDING items costs citations, which is the right trade.  A 6th was added as
# ~~ckernel_shims.cpp:400~~, which cite_check SKIPS by design (struck total 2 -> 3), so raw regex
# sees +6/-4 = +2 while this invariant sees +5/-4 = +1.  Both numbers are correct for what they
# measure; only this one is authoritative.
# AI(W906-GL-3l) 20260829: 16738 -> 16734 (net -4), reconciled per row.  Eight WRONG-FILENAME rows
# were fixed: FOUR were an ELIDED PATH PREFIX (`declared ...shims.h:59` really means
# ainarm9045_2x4_16_shims.h:59 -- the LINE NUMBERS were all correct, only the `...` made a real
# file look like a nonexistent basename), which is citation-for-citation; the other FOUR cited
# SCRATCH PROBE FILES that exist nowhere (_n4probe.cpp x3, probe_g1_full.cpp x1) and were rewritten
# as `<file> line NNNN, scratch`, which KEEPS THE PROVENANCE but stops being a citation no tool or
# human can resolve -- hence -4.  ainarm2.cpp stays 880 (its one fix was citation-for-citation:
# shims.h:361 -> acatchtray_shims.h:384).
# AI(W906-GL-3m) 20260829: 16734 -> 16739 (net +5).  The last two WRONG-FILENAME rows were fixed by
# replacing a DEAD REASON, not just a name: tests\test_w6_3_catchtray.cpp said Use_AMR() is false
# because "the W6.x FormsFacade stub hardwires" it, citing a FormsFacade.cpp that exists in NEITHER
# tree.  Removing 2 bogus citations and naming the REAL chain costs 7 (forms/fAGV.cpp:38,
# Automation/AGV_predicates.cpp:49 x2, cmydef.cpp:5885 x2, database.cpp:1601, forms/fAGV.h:21).
# Naming a real causal chain is worth more than the citation budget it spends.
# AI(W906-GL-3o) 20260829: 16739 -> 16750 (net +11).  EIGHT WRONG-LINE ROOTS in the
# NO-CODE-NEARBY bucket were corrected across 25 citing sites in 15 files, all comment-only:
#   aoutarm.cpp:264-274 -> :266-279 (the G6 write-up; :264-265 is the PREVIOUS item's delta)
#   MainCalcCore.h:78 -> :84        (:78 is about DLL-version checks, not bUT150Install)
#   fMain.h:386-400/:388/:383-402 -> :417-426 + :1156-1165  (10 sites; the torque record)
#   acatchtray_shims.h:41-44 -> :120-128 / :126  (:41-44 is the "cite names not lines" note)
#   fSetup.h:76/:69 -> :359 class + :395 `bool fShow;`
#   StringGrid.h:43-44/:43-50 -> :75  (the std::out_of_range/vector::at sentence)
#   StringGrid.h:19-21 absence claim -> EXPIRED (FixedCols exists at :200 since 20260820)
#   fTemp_Set.h:276-278 -> :281-283  (:276 is gate (S11), the subject is (S13))
# The count RISES because a correct citation is usually more specific than a wrong one: the fix
# names BOTH halves of a claim (`:420-421` for "still does not have" AND `:1156-1165` for "why
# deliberately not added"), and adds the sibling evidence a reader needs (aTester_Front.cpp:3023
# for the two labels fMain.h never mentions, fSetup.h:132-134, StringGrid.h:227, cMyDB.cpp:1661).
# ⚠ ATTRIBUTION IS BY CONSTRUCTION, NOT BY REGEX: `git status` was clean at wave start and only
# these 15 files differ from HEAD, so the whole +11 belongs to this wave.  An independent
# approximate regex over `git diff -U0` reported +4; it undercounts because it does not model
# ranges or bare `:NNN` continuations the way cite_check does.  cite_check is authoritative.
# ainarm2.cpp stays 880 -- it was not touched.
# AI(W906-GL-3p) 20260829: 16750 -> 16756 (net +6).  Audited the whole fQwertyKey family: of the
# 31 files that mention fQwertyKey, 29 carry the FW-QWKEY2 (20260824) supersession note and TWO DO
# NOT -- forms/fSetup.cpp and forms/fSetup.h, whose WA-1 register still states "fQwertyKey has no
# port anywhere in this tree" as a live fact.  Measured dead: class forms/fQwertyKey.h:294, extern
# :406, definition forms/fQwertyKey.cpp:41, compiled via CMakeLists.txt:723, LIVE in BOTH configs
# (live_lines.ps1 -PreprocOnly).  Added the note to both files (comment-only) and QUEUED the actual
# un-gate of the 5 ed*Click sites -- that is a code change and iron rule 6 needs both builds + ctest.
# ⚠ THE COUNT WENT UP, AND SO DID THE ★ QUEUE (1045 -> 1046).  That is the honest outcome: this wave
# converted an unstated falsehood into a stated debt, and a stated debt costs citations.
# ⚠⚠ INSERTING COMMENT LINES INTO forms/fSetup.h SHIFTED EVERYTHING BELOW IT BY +8 AND BROKE SIX
# CITATIONS INTO IT -- four of them written by GL-3o one commit earlier, two pre-existing
# (fLotInfo.cpp:4141 fSetup.h:271, Command.cpp:15244 fSetup.h:131-133).  All six re-measured and
# fixed in the same commit.  This is exactly what acatchtray_shims.h:40-43 warns about ("this banner
# shifts every line below it, and a stale line citation is exactly the kind of claim this project
# keeps having to correct") -- so: AFTER ANY LINE-ADDING EDIT TO A HEADER, GREP FOR CITATIONS INTO
# IT AND SHIFT THE ONES BELOW THE INSERTION POINT.  ainarm2.cpp still 880.
# AI(W906-GL-3q) 20260829: 16756 -> 16758 (net +2).  Read all 11 rows of cite_check's own
# "quote not found in target" arm; TEN of the eleven citations are CORRECT (two of those were only
# GONE because the citation resolves against the WRONG TREE -- both sentences say "golden" and
# golden really does carry the quoted text).  The ONE real defect was a misquote at
# Motor\myMN200motor.h:38, which claimed database.h:63 reads "// UI wave -- opaque" when it reads
# "// UI wave   -- MyBinDisp.h  (database.h:5)" ("opaque" is the BANNER's word at :60).  Quoting the
# line ACCURATELY costs +2 because the true text embeds its own "(database.h:5)" reference and the
# fix also names ":60" -- a verbatim quote is worth two citations.
# ⚠ THE FIX WAS DELIBERATELY MADE LINE-COUNT NEUTRAL (one long comment line, criterion A' clean)
# AFTER measuring that a +1-line version would have forced +1 shifts on EIGHT citations into
# myMN200motor.h (myMN200motor.cpp:94, mymotor.cpp:42/:55/:60/:88, vendor_offline_motionnet.cpp:19,
# cinitial.cpp:3647, csystem.cpp:30072).  GL-3p's own rule found them immediately -- and the right
# response to "this fix costs 8 collateral edits" is to make the fix not cost them.
# PREFER LINE-COUNT-NEUTRAL COMMENT FIXES IN HEADERS, precisely because growth is charged per
# citation.  ainarm2.cpp still 880.
# AI(W906-GL-3r) 20260829: 16758 -> 16775 (net +17).  Audited the fNote family the way GL-3p
# audited fQwertyKey: of the 61 files mentioning fNote, FIVE still asserted "TfNote VCL form, no
# home" as a live fact with no supersession note (Automation\AGV_PortScan.h, ainarm9045.cpp,
# aoutarm9045.cpp, BarCode\BarCode_Bottom2DID.h, BarCode\BarCode_Bottom2DID.cpp) while
# forms/fNote.cpp:122-133 and forms/fNote.h:129-132 ALREADY call those seams RETIREABLE.
# ⚠ THE PREMISE IS DEAD TO DIFFERENT DEGREES PER MEMBER -- measured, not assumed:
#     bMyServoOffInArm  EXISTS  forms/fNote.h:99      -> ainarm9045.cpp:913's gate is OPENABLE
#     fShow             EXISTS  forms/fNote.h:133     -> AGV_PortScan.h's guard is fully unblocked
#     bMyServoOffOutArm ABSENT  0 decls tree-wide     -> aoutarm9045.cpp's two gates STAND
#     t2DCode           ABSENT  0 decls tree-wide     -> BarCode_Bottom2DID's t2DCode gates STAND
#     Select[]          ABSENT                        -> canary_support.h:167-168 was ALREADY right
# So the fix was NOT "the premise expired, open everything": each site got the per-member truth, and
# canary_support.h:168's narrower wording ("the ported TfNote has no Select[] member") was the
# in-tree precedent for how to say it.  NOTHING WAS UN-GATED -- ainarm9045.cpp:913 is queued, and it
# is SAFETY-ADJACENT (golden's servo-off position-loss protection, JerryYang 20161227).
# The +17 is the per-member evidence (forms/fNote.h:96/:99/:133/:175, forms/fNote.cpp:43/:122-133,
# forms/fNote.h:129-132) replacing one vague phrase repeated five times.
# ⚠ ALL FIVE EDITS WERE MADE LINE-COUNT NEUTRAL ON PURPOSE: both headers carry citations BELOW the
# edited region (AGV_PortScan.h:126/:211 = 2; BarCode_Bottom2DID.h:144 x6 = 6), so growth would have
# cost 8 collateral shifts -- measured first, per GL-3q.  ainarm2.cpp still 880.
# AI(W906-GL-3t) 20260829: 16775 -> 16795 (net +20).  Applied stale_premise_scan's first-run output:
# all six expired premises fixed, plus the ONE the tool provably cannot see (forms\fObserver.h --
# the false negative documented in its own banner).  Five files, comment-only, ALL LINE-COUNT
# NEUTRAL, measured first: fContactCT.h has a citation at :140 and fObserver.h has four
# (:283/:557/:710/:784), all BELOW the edited regions, so growth would have cost five shifts.
# +20 is per-dependency evidence replacing a repeated phrase: fSecurity.h:458/:567/:576/:595,
# cSecurity.cpp:46/:1682, fNote.h:157/:158-161/:162/:163, cObserver.cpp:1760, cSecurity.cpp:1565.
# ⚠ THE DECISIVE FINDING IS NOT "the premise expired" BUT "IT EXPIRED PER DEPENDENCY":
#   GATE [L1] (uHGemHT9045.cpp:2859/:4179) -- BOTH halves dead (fSecurity real AND SetLevelSet real
#     at fSecurity.h:567, body cSecurity.cpp:1682) -> OPENABLE, queued.
#   fObserver.h GATE (1) -- the fSecurity half is dead but `fMain->AlarmUnitMap` IS STILL ABSENT
#     (every tree-wide hit is a comment or a gated call site) -> THE GATE STANDS.
# Same family, opposite conclusions.  Anyone acting on "premise expired" alone would open both.
# ainarm2.cpp still 880.
# AI(W906-GL-3u) 20260829: 16795 -> 16802 (net +7).  Fourth family, fShowBinSelect: all six
# worksheet rows were genuine expired "no port object at all" halves (forms/fShowBinSelect.h:822
# class, :1081 extern, cShowBinSelect.cpp:159 instance).  Three files, comment-only, line-count
# neutral.  Only GATE G18 (btnAutoCleanClick) still stands; G05/G07/G09 and fYieldMonitoring's 14
# sites are openable on this axis and are QUEUED.
# ⚠⚠ THIS WAVE PUT THREE FALSE CLAIMS INTO THE TREE AND THEN TOOK THEM BACK OUT.  A hand-rolled
# member regex whose alternation was `(?:\[|;|\()` -- NO `=` -- missed every in-class initialised
# member (`TLabel *labArmDiff = new TLabel();`), so I wrote "8 of 10 still absent, gates STAND" into
# three files.  ALL TEN are present (:829, :861-868, :971, :983).  What caught it was
# stale_premise_scan's I2 disagreeing with the registry -- i.e. only because the per-member facts
# had been REGISTERED.  Third variant of the recorded rule: A HAND-ROLLED SCANNER MUST NOT BE USED
# TO OVERRULE THE TOOL.  Find-MemberDecl was also fixed (it could not see METHODS at all, which is
# the dangerous direction: a false ABSENT reads as "the gate still stands").
# ⚠⚠⚠ GATE-LABEL COLLISION, SAFETY-RELEVANT AND NOT PREVIOUSLY WRITTEN DOWN: the iron rule names
# G03/G09/G14/G16/G31 with NO FILE, and measured 20260829 ALL FIVE labels occur in multiple
# unrelated files (G09 in uHGemHT9045.cpp:5364 AND csystem.cpp:18833 AND :21851; G14 in five files).
# "G09 openable" from a SECS-side reading must never license touching csystem.cpp's G09.
# ainarm2.cpp still 880.
# AI(W906-GL-3w) 20260829: 16802 -> 16811 (net +9).  Fifth family, fBinSel: of six worksheet rows,
# FOUR were genuine expired "no port anywhere" premises (forms/fBinSel.h:565 class TfBinSel, :691
# extern, cBinSel.cpp:150 instance) and every member their gates need EXISTS -- Save() :671,
# spbSaveClick() :661, sBinTraySetT3Pos[] :616, ChangeActivePageIndex() :660.  Two .cpp files,
# comment-only, line-count neutral.  The other two rows are CORRECT claims, adjudicated.
# ⚠⚠ THESE ARE THE MOST DANGEROUS "OPENABLE" ROWS QUEUED SO FAR.  spbSaveClick is, in golden's own
# words, "寫入 FT SetupFile" (Command.cpp:10407): un-gating GATE [E5] or the Command.cpp BinPos /
# BINCOUNT gates would let a SECS/GPIB command WRITE A PRODUCTION SETUP FILE on this machine.  The
# warning is written at every one of the four call sites.  NOTHING WAS OPENED.
# ainarm2.cpp still 880.
# AI(W906-GL-3x) 20260829: 16811 -> 16816 (net +5).  Sixth family, fTemp_Set: five worksheet rows ->
# TWO genuine expired premises corrected (uHGemHT9045.cpp:7069 GATE G34, csystem.cpp:29581 GATE
# H3-6's fTemp_Set bullet), THREE correct claims adjudicated.  Two files, comment-only, line-count
# neutral.
# ⚠⚠ THE HEADLINE IS A "DO NOT OPEN", NOT AN "OPENABLE".  GATE G34's BOTH halves died
# (fTemp_Set.h:487 class, :1560 extern, uTemp_Set.cpp:222 definition, SaveRemoteTempOffset :526,
# ReadRemoteTempOffset :528) AND IT STILL MUST STAY SHUT: SaveRemoteTempOffset's body calls
# MyForceDirectories(DataPath + GetLastOpenFN()) at uTemp_Set.cpp:6222, while its own (S3) gates
# cover ONLY the two WriteIniData calls (:6272-6274, :6311-6313).  Un-gating would let a SECS HOST
# CREATE A DIRECTORY UNDER THE PRODUCTION DataPath -- the "disclosure is not containment" shape
# CLAUDE.md records for test_automation's SetTemperature().
# ⚠ HOW THAT WAS ESTABLISHED: live_lines.ps1 REFUSED to rule on :6222 (six identical
# `MyForceDirectories(szDir);` lines in that file), so the evidence is the GATE MAP instead -- the
# only `#if 0 // SAFETY GATE (S3)` blocks are :6272-6274 and :6311-6313 and :6222 is outside both.
# Structural fact, not inference.  :6225 was confirmed live by the compiler in both configs.
# ⚠ csystem.cpp:29581 is the other shape: ONE BULLET of a MULTI-REASON gate expired and GATE H3-6
# STILL STANDS on the rest (fTrayAssignment/FTestIF/fLd_ULd/fHotPlate/fSpeed absent, fSetup lacking
# both members, fContact lacking DoIniDataToForm, fOffSet lacking iNowOffsetSel, fYieldMonitoring
# lacking SaveSetupFile, iSaveStander/iSaveSpecial untranslated).  Per-dependency, one level up.
# ⚠⚠ AND I2 CAUGHT A THIRD REGISTRY ERROR OF MINE: I registered ATC_Power / CheckAirMachineStatus as
# ABSENT because the CLAIM is about them having no DEFINITION -- but Find-MemberDecl measures
# DECLARATIONS, and the port declares both (fTemp_Set.h:575/:576, mirroring golden).  Same root cause
# all three times: THE REGISTRY'S Expect FIELD MUST RECORD WHAT THE TOOL MEASURES, NOT MY READING OF
# THE CLAIM.  ainarm2.cpp still 880.
# AI(W906-GL-3y) 20260829: 16816 -> 16818 (net +2).  THREE families in one wave, and only TWO source
# lines changed -- because most of what was found was ALREADY CORRECT:
#   fDynamicTemp  5 rows, ZERO edits.  All five (DynamicTemp.cpp:124/:225/:462/:474/:498) have
#                 `COM2->TempComm6` as their SUBJECT, not fDynamicTemp; TempComm6 has 0 declarations
#                 tree-wide and 0 hits in atester_shims.h, exactly as fDynamicTemp.h:112-113 says.
#                 All five gates STAND, all five claims TRUE -> 5 adjudications.
#   fSortCT       1 row, ZERO edits.  pnlYield/pnlYieldART 0 in forms/fSortCT.h while pnlLoad IS at
#                 :29 -- the comment is right word for word; the rule reports it only because
#                 pnlLoad is the present neighbour in the window.
#   fSpeed        2 rows, 2 EDITS -- a THIRD distinct per-dependency outcome: HALF THE CLAIM DIED
#                 AND THE SURVIVING HALF IS LOAD-BEARING.  `class TfSpeed` exists (forms/fSpeed.h:326)
#                 with DoIniDataToForm() at :745, but there is NO `TfSpeed *fSpeed` POINTER OBJECT
#                 anywhere -- 0 extern, 0 instance -- so `fSpeed->` would not link and GATE [E7]
#                 (uHGemHT9045.cpp:3686) STANDS.  Both sites now say WHICH half died.
# ⚠ AND THE DEAD-CONTROL CHECK REFUSED fSpeed's REGISTRATION, for the second time in this campaign.
# Rewording its rows removed the $DEAD_PAT trigger itself (the old "has NO ported class or object"
# matched `has no port` as a SUBSTRING of "NO ported"), so fSpeed now has ZERO rows of any class and
# an I1 of 0 for it could never go red.  Not registered; the measurement is in DEVLOG CXXXV.
# Registry now holds ELEVEN entities, DEAD-NO-NOTE 0 for all, SCOPED-AND-TRUE 7, adjudications 18.
# ainarm2.cpp still 880.
# AI(W906-GL-3z) 20260829: 16818 -> 16825 (net +7).  The last two triaged families, and SEVEN rows
# resolved: FIVE genuine expired premises corrected, TWO correct claims adjudicated.  Four files,
# comment-only, all line-count neutral.
#   fContactCT (4 rows)  forms/fContactCT.h:277 class, :341 extern, cContactCT.cpp:95 instance, and
#     every member (Y1) needs -- sgYield :281, rgYieldType :282, GetLowYield_AutoClean :316,
#     ClearData :320.  fYieldMonitoring (Y1) is OPENABLE.  ⚠ BUT GATE G06 (uHGemHT9045.cpp:5275)
#     STANDS on a DIFFERENT axis: fProductionInfo->CalculateNowArmSiteBinQty and
#     ->UpdateControlBinCount both have 0 declarations (re-measured).  Multi-reason gate again.
#     Adjudicated: fContactCT.h:27 (past-tense narrative in the file that CREATES the class) and
#     fStartCondition.h:165 (subject is `class TCanvas` -- 0 declarations, claim STANDS).
#   fLotInfo (3 rows)  ALL THREE EXPIRED -- the first entity whose whole worksheet was stale.
#     The five AutoClean-display members are at forms/fLotInfo.h:914-918 and :912-913 says they
#     landed EXACTLY to unblock fYieldMonitoring's (Y3) block; edtBarcodeRecipe is at :1637.
#     ⚠ uHGemHT9045_SV.cpp #8 is only PARTIALLY expired: fLotInfo.h / fCleaning.h / atester_shims.h
#     ARE still missing those widgets (verified per member), but its closing clause "there is no
#     other candidate home" is DEAD -- forms/fObserver.h:767 declares `TPanel *lbSerialNumber01`,
#     its own comment calling it the "uHGem serial/firmware surface".  That row now says RE-AUDIT
#     PER MEMBER instead of reading as settled.
# Registry now holds THIRTEEN entities, DEAD-NO-NOTE 0 for all, SCOPED-AND-TRUE 11,
# adjudications 22.  ainarm2.cpp still 880.
# ⚠ AI(W906-GL-4a) 20260830 CORRECTION TO THE LINE ABOVE: "adjudications 22" IS WRONG -- the
# registry held **20** at that commit (18 after GL-3y + 2 for fContactCT; I wrote 22).  Measured by
# counting `@{ File =` rows inside every Adjudicated block of `git show HEAD:tools/...`.  The wrong
# figure also reached DEVLOG CXXXVI in two places and its commit message, and is corrected there.
# NOTHING GUARDED IT, which is why this file now carries $EXPECT_ADJUDICATED (see that block).
#
# AI(W906-GL-4a) 20260830: 16825 -> 16830 (net +5).  RECONCILED LINE BY LINE against
# `git diff -U0 -- cinitial.cpp` before touching this number -- seven comment lines changed in the
# n5-G17/n5-G18 gate blocks, and exactly five of the edits add a filename-qualified citation:
#   cinitial.cpp:16200  + cmydef.cpp:6102   (the flag's definition -- the fact the block was missing)
#   cinitial.cpp:16200  + cmydef.h:5806     (its extern)
#   cinitial.cpp:16215  + ProductionInfo.h:556-557   (golden's two accessor DECLARATIONS)
#   cinitial.cpp:16216  + cmydef.cpp:6102   (repeated on the gate-label line so the label is self-contained)
#   cinitial.cpp:16242  + cmydef.cpp:6103   (the out-arm twin's flag)
# NET ZERO, deliberately, for three others: ProductionInfo/ProductionInfo.cpp:5907-5910 was
# re-parenthesised not removed; :5912 became :5912-5915; and "Un-gate with
# ProductionInfo/ProductionInfo.cpp" carried no :NNN so it never counted.  The bare `:5876`/`:5793`/
# `:5914` I added land in the "裸 :NNN" bucket (21695), not this one.
# ⚠ cinitial.cpp's own per-file figure is 829 and is NOT registered -- only ainarm2.cpp (880) is,
# and it was not touched.
#
# AI(W906-GL-4b) 20260830: 16830 -> 16833 (net +3).  Reconciled line by line against
# `git diff -U0`.  Nine comment lines in csystem.cpp (items (A7) :28758-28761 and GATE H3-10
# :29985-29989) and six in SECSGEM/uHGemHT9045_SV.cpp (item #8, :300-305):
#   +1  uHGemHT9045_SV.cpp:304  forms/fObserver.h:767-774   (GROUP A's eight real declarations)
#   +1  uHGemHT9045_SV.cpp:305  forms/fLotInfo.cpp:480-482  (the dead C++ the old wording missed)
#   +1  uHGemHT9045_SV.cpp:305  csystem.cpp:29998-30019     (GATE H3-10, the other dead site)
#   -1  uHGemHT9045_SV.cpp:305  forms/fObserver.h:767       (superseded by the :767-774 range)
#   +1  csystem.cpp:28761       uLotInfo.h:856              (golden home of pl_ATCChillerSV)
# NET ZERO for the two forms/fLotInfo.h citations: both were :131 (a DRIFTED citation -- that line
# is a comment about bInitFormcomponent) and both became :905, the real `class TfLotInfo`.
# The bare `(:1165)` and the path `tools/dfm2rc/layout_out/` carry no filename:line, so neither
# counts here.  ainarm2.cpp re-measured 880, untouched.
# AI(W906-GL-4c) 20260830: 16833 -> 16845 (net +12).  Reconciled against `git diff -U0`.  Forty
# comment lines in four files (RotateKit/aRotateKIT.cpp 12, aRotateKIT_In.cpp 15, aRotateKIT_Out.cpp
# 13 counting the banner pair, cinitial.cpp 5) correcting EIGHT wrong golden citations, four wrong
# cross-file ranges, one false absence clause and three false "NO compiled body" premises.
# The +12 are all NEW filename-qualified citations that the corrections had to add as evidence:
#   aRotateKIT.cpp        +3   csystem.cpp:21122 (x2, body + gate label), ainarm2.cpp:5361
#   aRotateKIT_In.cpp     +4   aRotateKIT_Out.cpp:374, csystem.cpp:21122, ainarm2.cpp:5361,
#                              aRotateKIT_In.cpp:389-393
#   aRotateKIT_Out.cpp    +5   CosFunction.h:406, CosFunction.h:405, aRotateKIT_Out.cpp:925,
#                              csystem.cpp:21122, ainarm2.cpp:5361
#   cinitial.cpp          +0   every change was a REPLACEMENT of a wrong range by a right one
# Net zero for the replaced-in-place ones: golden fRotate.h :21-25->:22-26, :32->:34, :45-46->:46-47,
# :39->:41, :135->:133; aRotateKIT_In.cpp :329-336->:335-342, :330-336->:335-342, :~324-330->:329-336,
# :321-328->:328-334, :338-343->:344-349; aRotateKIT.cpp :238-242->:250-254.
# Bare `:NNN` forms (golden :725-739, :242-245, the (:34)/(:50)/(:46)/(:31)/(:57) member tags) land
# in the "裸 :NNN" bucket (21726), not this one.  ainarm2.cpp re-measured 880, untouched.
# AI(W906-GL-4d) 20260830: 16845 -> 16857 (net +12).  Thirty-seven comment lines across five files
# (Command.cpp 10, cCounterClear.cpp 20, forms/fCounterClear.h 5, csystem.cpp 1,
# aoutarm9045_2x3_6.cpp 1).  The citation-ADDING edits:
#   Command.cpp:10621   NAME-COLLISION note -- forms/fLotInfo.h:1051, golden uLotInfo.h:504,
#                       golden uYieldMonitoring.h:104, golden uLotInfo.h:540-542
#   Command.cpp:15262   golden uYieldMonitoring.h:448, the six btnApplyClick call sites
#                       (Command.cpp:15989/:16247/:16805/:16820/:16885/:16895), csystem.cpp:3606,
#                       and a cross-reference to Command.cpp:10621
#   cCounterClear.cpp   csystem.cpp:6680-6685, :5089-5097, :6165, :6686, :5095-5097 and
#                       forms/fCounterClear.h:188 -- MINUS the four wrong ones they replace
#                       (:6441, :4851-4854, :5921, :6442)
#   forms/fCounterClear.h  csystem.cpp:5089 replaces :4851-4854          (net 0)
#   csystem.cpp:5093       :6165 replaces :~5921                        (net 0)
#   aoutarm9045_2x3_6.cpp:42  aoutarm9045_1x1_1.cpp:579-582 replaces :572 (net 0)
# ⚠ PER-LINE ATTRIBUTION IS APPROXIMATE HERE, AND SAYING SO IS THE POINT: two of the added lines
# are long and carry four and eight filename-qualified citations respectively, so hand-apportioning
# the +12 across them would be a guess dressed as a reconciliation.  Earlier waves reconciled to the
# exact unit because their edits were one citation per line; this one cannot, so the MEASURED total
# is what the invariant guards.  ainarm2.cpp re-measured 880 and was not touched.
# AI(W906-GL-4e) 20260830: 16857 -> 16961 (net +104), and this one DOES reconcile to the unit:
#   25 gate labels x 4 new filename-qualified citations each = 100.  Each of the 25 fFixAICCD
#   labels in the 14 aoutarm9045_*.cpp files gained the same note naming forms/fFixAICCD.h:48,
#   forms/fFixAICCD.cpp:11, FormsFacade.h:84 and aoutarm9045_1x2_2.cpp:658 (the live sibling call).
#   +4 from the two MIXED file heads: aoutarm9045_2x8_16.cpp and _2x8_8.cpp each gained
#   forms/fFixAICCD.h:48 and FormsFacade.h:84 (their `.cpp:11/:13` and `:1125`/`:1911` are bare
#   :NNN forms and land in the 21,785 bucket, not this one).
#   The other five head corrections (_1x1_1, _2x1_2, _2x3_6, _2x4_8, _2x4_4) cite only bare
#   :NNN label pointers, so they are net zero here.
# 100 + 4 = 104.  ainarm2.cpp re-measured 880, untouched.
# AI(W906-GL-4f) 20260830: 16961 -> 16958 (net -3).  A DECREASE, and it reconciles to the unit.
# 34 comment lines rewritten in cCounterClear.cpp:536/:538-:570 (the MAIN-LOOP INTEGRATION NOTE).
# SIX filename-qualified citations REMOVED, all of them wrong:
#   :536 csystem.cpp:4851-4854 (an in-arm suck loop, not the seam)
#   :547 csystem.cpp:5921      (SetRunStartMode; the live call is :6165)
#   :549 csystem.cpp:5142      (a #define; the real `#endif // W7C1_SEAM` is :5386)
#   :552 csystem.cpp:10410     (the inert call is :10654)
#   :567 csystem.cpp:6442      (the macro is :6686)
#   :568 csystem.cpp:7544      (the call site is :7788, and it is DEAD, not "LIVE")
# THREE added, all measured:
#   :544 csystem.cpp:5089-5097          (where the seam's retirement is recorded)
#   :565 tools/wb_serve.cpp:513-514     (the LIVE ClearCount callers the note denied existed)
#   :566 CMakeLists.txt:2553            (proof wb_serve is a real add_executable)
# The corrected line numbers are written as bare :NNN forms inside prose, so they land in the
# 21,795 bucket rather than this one -- which is why fixing six wrong citations LOWERS this count.
# ainarm2.cpp re-measured 880, untouched.
# AI(W906-GL-4g) 20260830: 16958 -> 16957 (net -1).  16 comment lines across four files
# (ainarm9045.cpp:9605-9608, AutoClean/AutoClean.cpp:3711-3714, ainarm2.cpp:313/:1926/:4198-4199,
# csystem.cpp:11419-11422), correcting TWO outright false "no live caller" claims and three
# imprecise ones.  Reconciled to the unit:
#   -1  ainarm9045.cpp:9608  `ainarm9045.cpp:2355` became a bare `:2355`
#   -1  csystem.cpp:11422    `csystem.cpp:549` became a bare `:549`
#   +1  csystem.cpp:11420    gained `csystem.cpp:2583` (the dead call site the claim denied)
#   net zero: ainarm9045.cpp:6972 -> :10083 (same file), and ainarm2.cpp:1926's
#             ainarm9045.cpp:1203 -> :1210 (a drift fix, same file).
# ⚠ ainarm2.cpp was edited this wave and its REGISTERED single-file count is unchanged at 880 --
# that invariant exists for exactly this case and it confirmed the prediction rather than the
# prediction excusing the check.
# AI(W906-GL-4h) 20260830: 16957 -> 16959 (net +2).  Nine comment lines in cinitial.cpp
# (:62-68 -- the expired "not ported yet" item -- plus :2624 and the :2928 gate label).
# Reconciled to the unit: the only NEW filename-qualified citations are mykitsuck.h:486 and
# mykitsuck.cpp:2818 (the CopyKitSuck extern and body).  The other four member citations are
# written as trailing "/:468/:469/:471/:464" and "/:224/:225/:227/:217" runs, which are bare
# :NNN forms and land in the 21,808 bucket.  Net zero for the removed ones: "MyKitSuck.cpp" and
# the three "EtherCAT/MyNUEC1.cpp" mentions never carried a :NNN, so they never counted here.
# ainarm2.cpp re-measured 880, untouched this wave.
# AI(W906-GL-4s) 20260830: 16959 -> 16963 (net +4), RECONCILED TOKEN BY TOKEN against
# `git diff` of the single changed line (aoutarm9045_2x8_32.cpp:3124 -- the fProductionInfo label
# whose "no FormsFacade stand-in" clause was measured EXPIRED and corrected).
# ⚠ I predicted EIGHT and the gate measured FOUR, so this is reconciled rather than re-registered:
#   counted (full file:line):  forms/fProductionInfo.h:53   forms/fProductionInfo.cpp:15
#                              FormsFacade.h:83             asendic_Auto.cpp:776        = +4
#   NOT counted (bare `:NNNN` continuations, which cite_check does not treat as citations):
#                              :63   :68   :826   :1696                                  = +0
# The old line carried `golden ProductionInfo.h TfProductionInfo` with NO line number, so it
# contributed zero citations -- hence a pure +4 with nothing removed.
# AI(W906-GL-4v) 20260830: 16963 -> 16969 (net +6), reconciled TOKEN BY TOKEN.  Two WRAPPED dead
# claims in cShowBinSelect.cpp were measured EXPIRED and corrected in place; each correction cites
# the evidence that killed it:
#   :22  forms/fSecurity.h:458   forms/fObserver.h:103   forms/fSecurity.h:333        = +3
#   :27  forms/fCleaning.h:65    uCleaning.h:280         forms/fCleaning.h:33         = +3
# Nothing removed (the original claim text is kept, per GL-4e's pattern), so a pure +6.
# ⚠ Predicted +6 and measured +6 -- unlike GL-4s, where I predicted 8 and the gate said 4 because I
# had counted bare `:NNNN` continuations that cite_check does not treat as citations.  These six are
# all full file:line forms.
# AI(W906-GL-4x) 20260830: 16969 -> 16974 (net +5), reconciled token by token.  Two more WRAPPED
# dead claims measured EXPIRED and corrected, this time in cContactCT.cpp:
#   :21  forms/fContactCT.h:277   forms/fYieldMonitoring.h:94                        = +2
#   :23  forms/fSecurity.h:458    forms/fSecurity.h:333    cShowBinSelect.cpp:22      = +3
# ⚠ `:341` in the :21 text is a BARE continuation, which cite_check does not count -- that is why
# this is +5 and not +6 (GL-4s's lesson, applied before running rather than after).
# AI(W906-GL-4y) 20260830: 16974 -> 16978 (net +4).  One more WRAPPED claim measured EXPIRED and
# corrected, in uYieldMonitoring.cpp:27 (the fContactCT facade -- the SECOND stale copy of that one
# fact).  Tokens added on that line:
#   forms/fContactCT.h:277   cContactCT.cpp:19   forms/fYieldMonitoring.h:94
#   forms/fContactCT.h:27                                                              = +4
#   stale_premise_scan.ps1:786                                                          NOT counted
# ⚠ `:341` is a bare continuation and is NOT counted either (GL-4s's lesson).
# ⚠⚠ NEW MEASURED FACT: cite_check does NOT count a `.ps1:NNN` citation, only .cpp/.h.  I predicted
# +5 on the strength of its banner ("every file.ext:NNN") and the gate said 16978, i.e. +4.
# ⚠ WHAT MADE THIS CHEAP: the prediction AND its falsification condition were written into this
# comment BEFORE the run ("if the gate reports 16978 instead, the .ps1 form is NOT counted"), so the
# gate's answer identified the wrong assumption immediately instead of prompting a hunt.  Worth
# repeating: when a registered number rests on an assumption, write the assumption down first.
# AI(W906-GL-4z) 20260831: 16978 -> 16986 (net +8).  Three WRAPPED claims in Command.cpp measured
# EXPIRED and corrected (the fShowBinSelect stand-in twice, and edATCAmbientTemper):
#   :157    forms/fShowBinSelect.h:822   ainarm9045.cpp:7620                          = +2
#   :3450   Command.cpp:157   forms/fShowBinSelect.h:822   ainarm9045.cpp:7620         = +3
#   :10463  forms/fMain.h:1118   main.h:737   forms/fTemp_Set.h:202                    = +3
# ⚠ Bare continuations (:1081, :830, :3455, :3461, :341-style) are NOT counted, and neither is a
# .ps1 citation (GL-4y's measured fact).
# ⚠ CHECKED BEFORE RUNNING, because a golden-only path could have moved a GUARDED shape:
# `main.h` does not exist in the port tree but DOES in golden, and 424 `main.h:NNN` citations
# already exist tree-wide -- they resolve against golden and land in the UNGUARDED
# CODE-NEARBY / NO-CODE-NEARBY buckets, not in any of the six registered shapes.  So +1 golden
# citation cannot move a registered shape.
# AI(W906-GL-5a) 20260831: 16986 -> 16990 (net +4).  Two more WRAPPED claims measured EXPIRED and
# corrected:
#   bthermo.cpp:4544      forms/fTemp_Set.h:487                                      = +1
#   forms/fTemp_Set.h:203 forms/fMain.h:1118   main.h:737   Command.cpp:10463         = +3
# ⚠ Bare continuations (:1560, :541, :4577, :4582, :4564) are not counted, nor .ps1 (GL-4y).
# ⚠ TWO OTHER CANDIDATES IN THIS BATCH WERE MEASURED AND DELIBERATELY LEFT ALONE, so they add
# nothing here: aTester_Rear.cpp:2837 (already adjudicated AMBIGUOUS-SUBJECT -- its own Why calls the
# claim "impeccably current") and cBinSel.cpp:2743 (still TRUE -- the four ed* "hits" in
# forms/fBinSel.h:388-389 are the CLAIM'S OWN QUOTED GREP COMMAND, not declarations).
# AI(W906-GL-5b) 20260831: 16990 -> 16993 (net +3).  Three more WRAPPED claims measured EXPIRED and
# corrected:
#   ainarm9045.cpp:7277   forms/fShowBinSelect.h:1081   Command.cpp:157                = +2
#   BarCode\BarCode_Bottom2DID.cpp:202   forms/fNote.h:96                              = +1
#   ckernel.cpp:1533                                                                   = +0
# ⚠ ckernel's correction adds ZERO counted citations even though it names three declarations: it
# writes "forms/fLotInfo.h -- :1083 ... :1525 ... :1526", i.e. the filename carries no line number
# and the three numbers are BARE continuations, which cite_check does not count.  Same rule as
# GL-4s/GL-4y; noted here because "I cited three lines and the total moved by zero" would otherwise
# look like a miscount.
# AI(W906-GL-5c) 20260831: 16993 -> 16994 (net +1).  Only ONE of the two edits cites anything
# countable:
#   forms/fQwertyKey.h:33   fQwertyKey.cpp:41                                             = +1
#   Public\MyProductionRecord.cpp:896                                                     = +0
# ⚠ :896 names FOUR locations (`class TfLotInfo` at :905, cbRunMode/edtSysLotID/edtCusLotID at
# :908/:911/:960) and adds ZERO, because it writes "forms/fLotInfo.h is now 1651 lines" with the
# filename carrying no line number -- every one of those is a BARE continuation.  Same rule as
# GL-4s / GL-4y / GL-5b's ckernel case.  PREDICTED +1 BEFORE MEASURING; measured +1.
# ⚠ NEITHER EDIT RETIRES A CLAIM, so unlike GL-4v..GL-5b this wave's edits are ANNOTATIONS ON
# CLAIMS THAT ARE STILL TRUE: forms/fQwertyKey.h:33 is correct and self-scoped (its subject is
# everything EXCEPT that header -- it is the ODR argument for the :26 "NEW CLASS" banner, so the
# :294/:406 declarations that look like a refutation are what it justifies), and
# MyProductionRecord.cpp:896's verdict holds while its EVIDENCE SCOPE went stale (it claims to have
# read "the whole file, 134 lines"; forms/fLotInfo.h is now 1651).
# ⚠ Both notes deliberately avoid the words RE-MEASURED / EXPIRED.  Those are read as RETRACTION
# markers by $NOTE_PAT in tools\stale_premise_scan.ps1, and tagging a LIVE claim with one is the
# exact defect DEVLOG CLXV measured (a `RE-MEASURED` line that CONFIRMED a claim was being read as
# one that retracted it).  Consequence: :896 correctly stays in item 15's residual set.
# AI(W906-GL-5c) 20260831: 16994 -> 16999 (net +5).  PREDICTED +5 BEFORE MEASURING; measured +5.
#   forms\fBinSel.h:210        cBinSel.cpp:2805   cBinSel.h:255                        = +2
#   forms\fLotInfo.cpp:3082    BarcodeReader.cpp:40   Public/HTEdit.cpp:123            = +2
#   tools\wb_serve.cpp:288     uTemp_Set.cpp:283                                       = +1
# ⚠ Every other location these three notes name is a BARE continuation and does not count:
# fBinSel.h names :2854/:2840/:3019/:2971/:200, fLotInfo.cpp names :50/:57/:71/:122/:156/:3085-3086,
# wb_serve.cpp names :599.  Same rule as GL-4s / GL-4y / GL-5b / GL-5c.
# ⚠ GL-5d's forms\fBinSel.h:210 is the wave's real find and it is NOT a comment defect in the usual
# sense: the claim's grep was aimed at `cBinSel_utf8.cpp`, A FILE THAT EXISTS IN NEITHER TREE, so its
# "0 hits" was a grep against nothing -- while golden cBinSel.cpp:2805-2838 holds a real 34-line
# body called from two live golden handlers.  The CONCLUSION survives (its only two consumers are on
# the same WAVE B QUEUE at :200) but the coupling is now recorded: when those handlers land,
# bCheckTrayCanUse must land with them.
# AI(W906-GL-5e) 20260831: 16999 -> 17001 (net +2).  One edit, forms\fQwertyKey.h:34:
#   forms/fQwertyKey.cpp:44   myQwertyKeyBoard.cpp:29                                 = +2
# ⚠ The note also names :233 (the enum), :249, :253, :76, :35 and :26 -- all BARE continuations,
# none counted.  Same rule as GL-4s / GL-4y / GL-5b / GL-5c / GL-5d.
# ⚠ PREDICTED +2 BEFORE RUNNING, and this line IS the measurement: a wrong prediction shows up as
# the cite_check scale gate going red, which is what the registered invariant is for.
# ⚠⚠ AI(W906-GL-5m) 20260831: 17001 -> 16781 (net -220).  THE LARGEST SINGLE MOVE THIS REGISTRY HAS
# SEEN, and it is a REMOVAL of citations that lived inside text that had itself become false.
#
# WHAT MOVED.  GL-5l/GL-5m retired all 122 SOFT_SIMULTE `STALE (GL-0e|GL-2p)` marker paragraphs
# (soft_simulte_gate went 122 -> 0).  Those paragraphs QUOTED citations as part of their prose --
# `MachineType.h:48`, `tools\live_lines.ps1`, `GL_CAMPAIGN_PLAN.md 4.6`, `canary_support.cpp:113`,
# `cMyDB.cpp:1831` and so on -- roughly two per marker across 215 marker paragraphs.  The
# replacement retirement notes DELIBERATELY use prose form ("MachineType.h 第 48 行") instead of
# `file:line`, so they add almost nothing back.  Reconciled mechanically per the GL-1z/GL-2a
# convention, `git diff -U0` against 54588ab^ over *.cpp/*.h: 10 citations added, 228 removed,
# net -218 by the reconciliation regex against cite_check's measured -220.
# ⚠ THE 2-CITATION GAP IS NOT RECONCILED AND I AM NOT REGISTERING IT AS IF IT WERE.  My
# reconciliation regex is looser than cite_check's predicate (it does not implement same-line
# filename inheritance, the exact rule that made my steering accounting short by 3 in GL-5k).  The
# authority is cite_check's 16781; the -218 is corroboration that the move is a bulk marker
# removal and not a scan that broke.
#
# ⚠⚠ WHY PADDING THE NOTES BACK TO 17001 WOULD HAVE BEEN THE WRONG FIX, stated because it was
# tempting: inserting `MachineType.h:48` into each note would have added ~215 citations and landed
# near the old number.  That is fabricating citations to satisfy a counter -- the mirror image of
# "do not re-register a number to turn a gate green".  A number that moves for a stated, measured
# reason is healthy; a number held still by padding is not.
#
# ⚠ WHAT THIS WAVE PROVED ABOUT THIS REGISTRY, and it is the reason the notes are LINE-NEUTRAL:
# GL-5l first DELETED the 892 marker lines outright.  That shortened 106 files by 8-22 lines each
# and silently invalidated every citation pointing past a cut -- `PAST-EOF` 0 -> 4 and
# `PAST-EOF-BIG` 0 -> 5 caught only the 9 that fell off the END of a file, and steering_cite_check's
# ANCHOR check caught 4 more in the steering docs.  IN-RANGE DRIFT IS INVISIBLE TO EVERY GATE HERE
# (steering_cite_check:93 says exactly that).  GL-5m therefore REPLACED each marker with a note of
# EXACTLY THE SAME LINE COUNT: all 106 files are byte-for-byte the same length as before, verified
# +926/-926 with net 0, and all nine PAST-EOF rows plus all four ANCHOR failures went away without
# editing a single citation.  ⚠ ANY FUTURE BULK COMMENT REMOVAL MUST DO THE SAME.
# AI(W906-GL-5o) 20260831: 16781 -> 16782 (net +1).  One edit, tests\test_agv_e84.cpp: the note
# explaining the putenv->_putenv portability fix cites the failing line itself
# (`tests/test_agv_e84.cpp:178`).  Reconciled with `git diff -U0` BEFORE touching this number.
# ⚠ MY RECONCILIATION SAID +2 AND cite_check SAYS +1, AND I AM REGISTERING cite_check's NUMBER.
# The second one my regex counted is `CMakeLists.txt:545` inside the GL-G6 justification in THIS
# FILE -- a .ps1, which is tooling, not tree source, so cite_check's tree-wide predicate does not
# count it.  Same lesson as GL-5k/GL-5m: my reconciliation regex is a corroborator, never the
# authority.
# AI(W906-GL-5p) 20260831: 16782 -> 16784 (net +2), and this time my reconciliation and cite_check
# AGREE exactly, which they did not in GL-5o.  Both new citations point at the SAME precedent:
#   tests/test_agv_e84.cpp   +1  ->  `test_bootstrap.cpp:61`  (the correction noting that GL-5o
#                                    re-derived what FW0 had already documented on 20260817)
#   tests/test_bootstrap.cpp +1  ->  `test_bootstrap.cpp:66`  (the guard-narrowing note naming the
#                                    line whose declaration it guards)
# Reconciled with `git diff -U0` per file BEFORE touching this number.
# AI(W906-GL-5z) 20260901: 16784 -> 16785 (net +1), and the delta is CONFINED TO ONE FILE, measured
# per file rather than reasoned about:
#   Command.cpp  275 -> 276 filename-bearing citations (+1); tree-wide 16784 -> 16785 (+1).
#   Nothing else in the tree changed this wave, so file delta == tree delta, which is the check.
# WHAT CHANGED: the (B1) GOLDEN BUG entry in Command.cpp's FW3-WC banner was corrected.  B1 cited
# `golden :8316/:8317/:8330/:8331` and said "both occurrences".  MEASURED against golden:
#   golden Command.cpp has THREE `memset(CmdData, 0x00, sizeof(CmdData))` -- :8317, :8341, :8351
#   B1's `:8330` is actually `{` and `:8331` is `if (GetLastError()==ERROR_ALREADY_EXISTS)`
#   i.e. the intended second pair is :8340/:8341 -- the citation was off by ten
# and the THIRD occurrence (golden :8351, this tree's Command.cpp:8482) was neither listed in B1
# nor marked at its own site, and it is the WORST of the three: it sits in the `else` of
# `if (hFileMap!=NULL)`, where MapViewOfFile was never called, so CmdData is NULL (it is a
# file-scope `INFO *` at LastSet.cpp:41, hence zero-initialised) or stale from an earlier call.
# ⚠ The edit was deliberately LINE-NEUTRAL (+5/-5, 18,119 lines before and after) because
# CLAUDE.md anchors Command.cpp:12470 and :12482 BELOW it, and in-range citation drift is invisible
# to every gate (the GL-5m rule).  Verified after the edit that :9176/:12470/:12482 still land on
# their described content.
# ⓘ NOT fixed, per iron rule 3: all three are golden's own code, verbatim.  The site marker at
# :8482 and the registration of the SECOND family found this wave (EJ1N/uModbusCommand.cpp
# :125/:138/:158, golden :33/:46/:66 -- same `sizeof(char* parameter)` bug, currently unrecorded
# anywhere) are QUEUED, not done here: both add lines, and adding lines above :12470 is the very
# thing this wave avoided.
# AI(W906-GL-6a) 20260901: 16785 -> 16786 (net +1), again CONFINED TO ONE FILE and measured per file:
#   EJ1N/uModbusCommand.cpp  0 -> 1 filename-bearing (the new `EJ1N/uModbusCommand.cpp:33` marker)
#   Command.cpp              276 -> 276 UNCHANGED -- its marker deliberately carries NO line number
#                            ("see FW3-WC GROUP banner", matching the wording at :8444/:8470)
#   tree-wide                16785 -> 16786.  File delta == tree delta, which is the check.
# WHAT CHANGED: GL-5z left two queue items that both needed a line-shift wave -- the missing site
# marker at Command.cpp:8482 and the unmarked second memset family.  Both are now done WITHOUT any
# shift, by hanging the markers on the END of the existing statement lines instead of inserting
# comment lines above them.  Measured line-neutral: Command.cpp 18,119 -> 18,119 (+1/-1),
# EJ1N/uModbusCommand.cpp 251 -> 251 (+3/-3).
# ⚠ This DEVIATES from the standalone-comment style of the two markers at :8444/:8470, and the
# deviation is deliberate and stated in the comment itself: a standalone line there would move
# CLAUDE.md's Command.cpp:12470 / :12482 anchors, and in-range citation drift is invisible to every
# gate (GL-5m).  Trailing markers cost one style inconsistency; inserted lines cost silent drift.
# ⓘ Iron rule 6 satisfied properly, not nominally: BOTH arms of main were built and BOTH changed
# files were confirmed recompiled in each (`Building CXX object ... Command.cpp` x2 targets,
# `... uModbusCommand.cpp` x1), 0 errors, 13 archives each.  Comment-only edits CAN break a build
# (GL-5k's `#endif` inside a string literal), and these add non-ASCII text, so this was checked
# rather than assumed.
# AI(W906-GL-6b) 20260901: 16786 -> 16789 (net +3), reconciled per file BEFORE touching the number:
#   Command.cpp    276 -> 277  (+1)   the "NOT a defect" note at :9433 naming golden Command.cpp:9234
#   cinitial.cpp   829 -> 831  (+2)   the (S2)-class substitution note at :16838 naming
#                                     Command.cpp:8339-8352 (the precedent) and cinitial.cpp:7850/:8226
#   sum +3 == tree-wide +3, which is the check.  Both files LINE-NEUTRAL (+1/-1 each; Command.cpp
#   18,119 -> 18,119 and cinitial.cpp 18,933 -> 18,933).
# THE TWO -Wformat= SITES, adjudicated in opposite directions -- that is the point of the wave:
#   cinitial.cpp:16838  FIXED.  `sprintf(char[2048], "%s", <vclcompat::AnsiString>)` passes a
#       std::string-backed object through real C varargs = UB.  golden cinitial.cpp:7850 is
#       byte-identical and works on BCB6 only because ITS AnsiString is a single-pointer layout
#       (the ABI coincidence this file's own (S2) at Command.cpp:8339-8352 already documented and
#       already fixed 17 times with `.c_str()`).  This was the LAST unfixed site of that class --
#       the compiler says so: exactly 2 -Wformat= in the whole tree, and the other one is below.
#       ⚠ Behaviour checked, not assumed: golden's empty-string path writes "(null)" (what printf
#       emits for a NULL pointer) and golden's OWN read-back at cinitial.cpp:8226 maps "(null)"
#       back to "".  With .c_str() an empty string writes empty and reads back as "" directly.
#       Same final value, and old files still carrying "(null)" are still caught by that mapping.
#       Verified in BOTH arms: 0 errors, cinitial.cpp recompiled in each, and the :16838 warning
#       is GONE in both (0 occurrences) -- i.e. the compiler confirms the UB is closed.
#   Command.cpp:9433    NOT FIXED, deliberately, and annotated at the site so nobody "tidies" it.
#       `%08X` with `unsigned long ulvalue` (:9422) is a pedantic type mismatch with IDENTICAL
#       representation here: measured twice (a runtime probe and a static_assert) that
#       sizeof(unsigned long) == sizeof(unsigned int) == 4 on this toolchain, and long is 32-bit on
#       Windows generally.  golden Command.cpp:9234 is byte-identical.  Changing it to %08lX would
#       edit golden's format string for zero behaviour difference -- iron rule 3.
# AI(W906-GL-6d) 20260901: 16789 -> 16793 (net +4), reconciled per file BEFORE touching the number:
#   aoutarm9045_1x3_2_14.cpp    7 -> 8   (+1)  site note naming its MIRROR sibling's line
#   aoutarm9045_2x3_6_14.cpp   12 -> 13  (+1)  site note naming its MIRROR sibling's line
#   vclcompat/MemoryStream.cpp  0 -> 2   (+2)  false-positive note naming MemoryStream.h's SizeProxy
#   sum +4 == tree-wide +4, which is the check.  Baselines read from the untouched worktree copies.
# ALL THREE -Wmaybe-uninitialized SITES ARE FALSE POSITIVES -- no code changed, only comments:
#   the two aoutarm ones: the read loop and the write loop use the SAME bounds
#   (OutArmSuck.iPickRow / .iPickCol) and the file writes those fields ZERO times, so "reachable"
#   implies "already assigned".  Measured, not argued.
#   MemoryStream.cpp: `Size(this)` in a mem-initializer is well-defined -- SizeProxy's ctor
#   (MemoryStream.h:76) only STORES the pointer; the dereference is in operator int(), which runs
#   after construction.  This is the port's OWN shim, so a real defect here would be ours to fix --
#   it was checked and there isn't one.
# ⚠ BUT READING THOSE TWO aoutarm LINES FOUND A DIFFERENT, REAL GOLDEN DEFECT (report #10): each
# sibling recomputes ONE of the two column variables before the ErrorLog call and leaves the OTHER
# holding the previous loop's value -- and they pick OPPOSITE ones (golden 1x3_2_14.cpp:554
# recomputes iSuckCol; golden 2x3_6_14.cpp:545 recomputes iShtCol).  Copy-paste, half-edited.
# So: warning pointed at the right line for the wrong reason.  Do NOT "fix" it by adding an
# initialiser -- that silences the warning and keeps the defect.
# Line-neutrality: both aoutarm files +1/-1 (1603 and 1581 unchanged).  MemoryStream.cpp is +18/-0
# (84 -> 102) and that shift is safe because citations to `MemoryStream.cpp:NN` measure ZERO
# tree-wide, in CLAUDE.md and in SKILL.md.
# AI(W906-GL-6e) 20260901: 16793 -> 16794 (net +1), confined to one file, measured per file:
#   Motor/mySMCmotor.cpp  5 -> 6  (+1)   the "NOT a defect" note naming golden mySMCmotor.cpp:805
#   tree-wide 16793 -> 16794.  File delta == tree delta.  Line-neutral (1,222 -> 1,222, +1/-1).
# THE -Wparentheses BUCKET (159) IS NOW FULLY ADJUDICATED, AND IT CONTAINS ZERO NEW DEFECTS.
# Grouping first was the whole trick -- 159 warnings, only FOUR distinct messages:
#   154  suggest parentheses around '&&' within '||'   pure style; `a&&b||c` already means
#        `(a&&b)||c`, which is what every one of those sites wants.  Not examined individually
#        BY DESIGN, and that is a stated cap, not an omission.
#     3  suggest parentheses around '-' in operand of '&'  -- `HotPlateForm.iPlateSelect & 2-i`
#        at ainarm_SearchPlacePlate.cpp:665, ainarm2.cpp:3685, ainarm2.cpp:6394.  `-` binds
#        tighter than `&`, so this is `iPlateSelect & (2-i)`: i=0 tests bit 1, i=1 tests bit 0 --
#        an INVERTED plate/bit mapping that golden's own trailing comment documents
#        (`//MMPlate1+0=NO 2 HP`).  Intended, not a bug.  golden has the identical line at
#        ainarm2.cpp:542/:2234, ainarm_SearchPlacePlate.cpp:611, ainarm9045_2x8_32.cpp:2647.
#     1  comparisons like 'X<=Y<=Z' ... -- atester.cpp:617, i.e. ALREADY report #7.  ⚠ So the
#        "23 high-signal" set and this 159 set OVERLAP BY ONE; they are not 182 distinct warnings.
#     1  suggest parentheses around '-' inside '<<'  -- Motor/mySMCmotor.cpp, annotated this wave.
# ⚠ MEASURED, NOT RECITED: both precedence claims were checked with a compiled probe --
#   static_assert((5 & 2-1) == (5 & (2-1))) and static_assert((1<<3-1) == (1<<(3-1))) both pass,
#   and at runtime iPlateSelect=3 gives 2 for i=0 and 1 for i=1.  The two adjacent lines in
#   mySMCmotor (`1<<(iOutPort-1)` and `(1<<iOutPort-1)`) are therefore EQUIVALENT, which is why
#   that one gets a note rather than a fix.
# ⓘ Coverage of the whole warning audit, quantified this wave because "libraries only" was too
#   vague: that capture compiled 277 of the tree's 587 .cpp files = 47.2%.  But 279 of the 310
#   uncovered files (90%) are tests/ (140) + tools/ (139).  The genuinely unaudited PRODUCTION
#   surface is 31 files, and 23 of those are the port's OWN code (WebBridge/ 12 +
#   WebBridgeTags.cpp + WebAuth.cpp + vclcompat/ 9) where a defect would be ours to fix.
# AI(W906-GL-6g) 20260901: 16794 -> 16802 (net +8), reconciled per file BEFORE touching the number:
#   cinitial.cpp     831 -> 838  (+7)   the -Wnarrowing "NOT a defect" note at :4534, which names
#                                       golden cinitial.cpp:4090, Motor/HTMotor.h:219 and :101/:50,
#                                       and the four driver bounds (myEthercatmotor.cpp:289,
#                                       myMN200motor.cpp:357, mySMCmotor.cpp:361,
#                                       myGALILmotor.cpp:1009 guarded by :1007)
#   ainarm9045.cpp   900 -> 901  (+1)   the int->double note at :157, naming aoutarm9045.cpp:288-289
#   sum +8 == tree-wide +8.  Both files LINE-NEUTRAL (18,933 and 11,602 unchanged, +1/-1 each).
# ⚠ THE BASELINE FOR cinitial.cpp IS 831, NOT the 829 the wt_scale worktree reports: GL-6b already
# edited that file (+2), so the worktree copy is STALE for it.  Using 829 would have produced a
# +10 vs +8 mismatch and sent me looking for a phantom.  When reconciling, check whether an
# earlier wave in the SAME session already touched the file.
# ALL 25 -Wnarrowing ARE NON-DEFECTS, and the 25 collapse into 3 families:
#   A (16)  int -> double, ainarm9045.cpp:157-158 + aoutarm9045.cpp:288-289.  PROVEN lossless:
#           double's 53-bit mantissa covers a 32-bit int, and static_assert((int)(double)INT_MAX
#           == INT_MAX) plus the INT_MIN version both compile.  Zero value change; GCC warns only
#           because C++ list-init defines non-constant int->double as narrowing.
#   B (8)   unsigned int -> BYTE, cinitial.cpp:4534-4541, feeding SetGroup(1, 4, bDevNo_In).
#           Cannot truncate: SetGroup's own API is BYTE bDevNo[] (Motor/HTMotor.h:219), so a byte
#           IS the right width, and every driver bounds iPortID to 0..99 -- the one unmodded
#           assignment (myGALILmotor.cpp:1009) is guarded by if(Addr<10) at :1007.  iPortID is
#           `unsigned int` in BOTH trees (port HTMotor.h:101 = golden :50).  golden identical at
#           cinitial.cpp:4090 (and uteach.cpp:5789).
#   C (1)   int -> unsigned, SECSGEM/uHGemHT9045.cpp:648 `unsigned CEDIDContent[]={i}` where i
#           starts at SECS_EVENT.DoStart.  SecsEventType.h:43 has `DoStart=1` -- positive, and
#           tests/test_uHGemClass.cpp:1263 asserts it == 1.  Never wraps.  golden identical at
#           SECSGEM/uHGemHT9045.cpp:444.
# Site notes added to A's and B's first line only (the two a future auditor would worry about),
# both saying explicitly DO NOT add a cast to silence it.
# AI(W906-GL-6h) 20260901: 16802 -> 16807 (net +5), reconciled per file, and this time the
# BASELINE-STALENESS CHECK GL-6g added was actually run first: `git log --name-only aba6233..HEAD`
# shows neither file was touched earlier this session, so the wt_scale baselines are valid here.
#   Motor/mySYNTEKmotor.cpp  5 -> 7  (+2)   sentinel note naming HTMotor.h:99-101 and golden :48-50
#   ATC/ATCSystem.cpp        4 -> 7  (+3)   sentinel note naming ATCSystem.h:314 and the two callers
#   sum +5 == tree-wide +5.  Both LINE-NEUTRAL (931 and 2,327 unchanged, +1/-1 each).
# ALL 18 -Wsign-compare ARE NON-DEFECTS.  Four groups:
#   13  cAuthority.cpp:460-462/:475-476/:483/:499/:517-522 -- the textbook
#       `for(int i=0; i<sizeof(X)/sizeof(X[0]); i++)` shape.  i starts at 0 and the arrays are
#       tiny, so the int->unsigned conversion is value-preserving.  Benign.
#    3  Motor/mySYNTEKmotor.cpp:437 `if(Address==-1 || iBoardID==-1 || iPortID==-1)`.  All three
#       are `unsigned int` (HTMotor.h:99-101 = golden :48-50), so -1 becomes 0xFFFFFFFF -- BUT THE
#       GUARD IS CORRECT, because the sentinel is WRITTEN the same way: this file's ctor at
#       :206-211 does `Address=-1; iBoardID=-1; iPortID=-1;` when Addr==-1.  Written as -1, tested
#       as -1, both convert identically.  ⚠ Changing the sentinel would BREAK the guard (an
#       unconfigured motor would be let through SetSpeed).  golden mySYNTEKmotor.cpp:262 identical.
#    1  ATC/ATCSystem.cpp:1311 `if(iChannel==-1)` -- same self-consistent shape, and here the
#       sentinel is the DEFAULT ARGUMENT: ATCSystem.h:314 declares
#       `void GetNowSiteOnOff(unsigned int iChannel=-1);`.  Real callers pass a channel
#       (ATC/ATCInterface.cpp:1017/:1064 pass i).  golden ATCSystem.cpp:1122 identical.
#    1  Command.cpp:11509 `for(int i=0; i<sizeof(str); i++)` writing str_buf[i] -- checked for
#       overflow: `char str[256]` and `char str_buf[256]` (both at :11487-11488), loop writes
#       [0..255].  IN BOUNDS.  golden Command.cpp:10695 identical.
# ⓘ Site notes added to the two SENTINEL sites only -- those are the two where a well-meaning
# "fix" (replacing ==-1, or picking a different sentinel) would actually change behaviour.  The 13
# loop-bound ones need no note; they are the standard idiom.
# AI(W906-GL-6j) 20260901: 16807 -> 16810 (net +3), confined to one file.  The GL-6g
# baseline-staleness check was run FIRST (`git log --name-only aba6233..HEAD` shows uHGemClass.cpp
# untouched this session), so the wt_scale baseline is valid:
#   SECSGEM/uHGemClass.cpp  68 -> 71  (+3)   the defect-11 site note at :1677
#   tree-wide 16807 -> 16810.  LINE-NEUTRAL (5,051 unchanged, +1/-1).
# -Wconversion: 18 raw / 16 deduped.  FIFTEEN ARE BENIGN, ONE IS A REAL GOLDEN DEFECT (report #11).
#   cpublic.h:214 (2 raw / 1 site)  `UnsignInt=Value;` where Value is __int64 -- BENIGN and the
#       proof is the line above it: `if(Value>4294967295 || Value<0) return false;` bounds Value to
#       exactly unsigned int's range before the assignment.  GCC cannot see the correlation.
#   SECSGEM x14 `int` -> `unsigned char`/`short`.  Adjudicated by asking, per site, whether the
#       SOURCE can be negative or >255:
#         ProcessS7F23FromatReceipe / FTClick -- measured NO negative returns; HCACK is
#             `unsigned char HCACK=1` -- benign.
#         uHGemClass.cpp:1664 -- BENIGN, and it is the CONTRAST that proves #11: its `ret` came
#             from S2F15_CheckNewEquipmentConstant and :1657 guards `if(ret==-1)` (the -1 case
#             returns at :1660), so :1664 can only see ret != -1.
#         ⚠ uHGemClass.cpp:1677 -- THE DEFECT.  `ret` from S2F15_UpdateNewEquipmentConstant, which
#             returns -1 at :4402/:4407/:4413, is passed to LocalAcknowledge's `unsigned char`
#             third parameter (SecsWireCodec.h:391) -> 255.  The S2F16 EAC sent to the host becomes
#             0xFF, and SECS-II only defines 0/1/2/3.  golden uHGemClass.cpp:776 identical, and
#             golden has the SAME guard asymmetry (:756 guards, :776 does not).  Registered as
#             report #11; NOT fixed here (iron rule 3).
#   ⓘ ONE ITEM DELIBERATELY LEFT UNVERIFIED, stated rather than glossed: the two
#     `Mode=atoi(strGrdAlarm->Cells[2][j])+0x80` sites (uHGemClass.cpp:2783/:2948).  Cells[2] holds
#     AlarmCode (tests/test_uHGemEquipment.cpp:363 shows "41", :364 shows "MES0101" -> atoi 0), and
#     +0x80 is the SECS ALCD set-bit convention, so a code >= 128 would overflow the byte.  Whether
#     any deployed AlarmCode reaches 128 is DATA-dependent and was NOT measured.  Queued.
# AI(W906-GL-6k) 20260901: 16810 -> 16811 (net +1), confined to one file.  Baseline-staleness check
# run FIRST (`git log --name-only aba6233..HEAD` shows ainarm9045S_2x4_4_13.cpp untouched this
# session), so the wt_scale baseline is valid:
#   ainarm9045S_2x4_4_13.cpp  1 -> 2  (+1)   the defect-12 site note at :2762
#   tree-wide 16810 -> 16811.  LINE-NEUTRAL (3,178 unchanged, +1/-1).
# -Wimplicit-fallthrough=: 1,086 raw / 1,086 deduped -- the LARGEST bucket in the OFF arm, and it
#   is 1,085 intentional + ONE REAL GOLDEN DEFECT (report #12).  This tree's `switch` statements
#   are STATE MACHINES and the fall-through is the deliberate "advance a step and run it in the
#   SAME tick" idiom.  Mechanically classified (comments stripped first) into FOUR shapes:
#     A  `<var>=N;` immediately before `case N:`                                     1000
#     C  `if(<var>!=N) break;` immediately before `case N:`                            57
#          (the comment on these usually reads "add in arm speed" -- the guard's ONLY
#           purpose is to control the fall-through, which is why it is decisive evidence)
#     D  the `break;` is COMMENTED OUT with a documented reason                        10
#          e.g. aoutarm9045_*.cpp: `//jou 980424 add out arm speed` + `//            break;`
#     ☆  unclassified -> read one by one                                              19
#   ⚠⚠ THE 1,000 SHAPE-A SITES MUST NEVER GET A `break;` ADDED.  That would make every step wait
#      one extra scan cycle -- a REAL-MACHINE TIMING CHANGE, not a warning cleanup.
#   Of the ☆ 19: 3 already carry an in-tree fall-through note (OCRInsp.cpp:761 says "faithful
#     fall-through to case 50 (no break in golden :353) ... Not fixed"); several are shape-C
#     VARIANTS where the guard tests a semantic sentinel instead of the label
#     (acatchtray.cpp:8436 `if(iReadCIDAction!=ePortTotal) break;`, aTester_Front.cpp:1260 and
#     aTester_Rear.cpp:1313 `if(iNN==NN_2Row) break;`); acarry.cpp:7427/:7558 are a machine-config
#     CUMULATIVE fall-through (2x5 sets bFlag[5] then falls into 2x6's bFlag[6..7]; the
#     progression 8site->4567, 10->567, 12->67, 16->none is monotone); asendic_Loader_RT.cpp:353
#     falls through only when the timer is un-expired and case 200 re-tests the same timer.
#   ⚠ ainarm9045S_2x4_4_13.cpp:2762 -- THE DEFECT.  `case 2100:` (ion-fan give-way) has no
#     `break;`, so control continues into `case 15000:`'s heater check in the same tick.  Two
#     independent contrasts prove it is an oversight, not the idiom:
#       (1) golden's OWN sibling: `case 999:` at golden :2475, SAME author, SAME date
#           (`Eastsun 20260521 整合`), SAME DoInArmIonFanGiveWay() call -- HAS `break;` at :2480.
#       (2) tree-wide there are 52 `case 2100:` sites and the compiler reports a fall-through at
#           exactly ONE of them.  ⚠ That 52-vs-1 figure came from cross-referencing the compiler's
#           own -Wimplicit-fallthrough site set; a hand-written forward scanner (look for `break;`
#           below the label) produced ~25 FALSE POSITIVES because a long case body runs past the
#           window.  USE THE COMPILER, NOT A HAND-ROLLED SCANNER -- this tree's oldest lesson.
#     Reachable: Task=2100 is assigned at :1541/:1664/:1676/:1747/:2748.  Consequence: when
#     DoInArmIonFanGiveWay() returns FALSE the fan has not moved yet and Task stays 2100, but the
#     heater-check body runs anyway and any of its :2772 Task=1 / :2797 Task=75 / :2801 Task=50 /
#     :2806 Task=100 abandons the wait.  Registered as report #12; NOT fixed here (iron rule 3).
#   ⚠ MY OWN DISCRIMINANT'S WINDOW WAS THE BUG, and it made 6 sites look like open questions.
#     Shape A was implemented as "within 7 lines ABOVE `case N:` there is `=N;`".  This tree's case
#     bodies are routinely dozens of lines long, so the assignment can sit much higher IN THE SAME
#     case body.  Re-running with "search the WHOLE enclosing case body for `=<next case label>;`"
#     resolved 6 of the last 9 immediately -- all plain shape A with a long gap:
#       AutoClean.cpp:9101       case@8995 -> case 2001@9102   assign :9022   gap 80 lines
#       aTester_Front.cpp:8680   case@8607 -> case 115@8689    assign :8624   gap 65
#       aTester_Rear.cpp:8671    case@8604 -> case 115@8682    assign :8621   gap 61
#       acatchtray.cpp:3403      case@3372 -> case 2351@3405   assign :3379   gap 26
#       acatchtray.cpp:4222      case@4166 -> case 260@4260    assign :4244   gap 16
#       acatchtray.cpp:4311      case@4272 -> case 400@4346    assign :4340   gap 6
#     ⚠ THE TRAP: the leftover set is small and looks exactly like "the genuinely hard cases", so
#       a tool defect reads as a property of the data.  My first draft of this block wrote "8 still
#       unadjudicated".  When a few items resist classification, SUSPECT THE DISCRIMINANT FIRST.
#   The final 3 were read and checked against golden -- all non-defects:
#     AutoRetest.cpp:1102   golden :769 `Task=1;` before `case 100:` (siblings golden :113/:535
#       write `Task=100;`).  BEHAVIOURALLY EQUIVALENT: case 1's entire body is that one assignment,
#       so both spellings yield the same retry loop and both set Task=200 on success.
#     MyBinDisp.cpp:2996/:3106   golden :2858-2862 / :2965-2969 byte-identical (ZeroMemory->memset
#       is the documented port adaptation).  `case 1:` does Addr=0 plus two zero-fills then falls
#       into `case 100:`'s while(1) worker.  golden assigns Task=100 at 36 sites in that file, so
#       case 100 is a real entry point and case 1 is "initialise, then work in the same tick".
#   FINAL TALLY -- all 1,086 adjudicated, 0 left open:
#     A 1006 + C 60 + D 10 + already-noted-in-tree 3 + machine-config-cumulative 2 +
#     timer-retest 1 + read-then-non-defect 3 + DEFECT #12 1  =  1086.
# AI(W906-GL-6l) 20260901: VALUE DELIBERATELY UNCHANGED at 16811 even though this wave edited two
# .cpp files.  Recording that here because "source edited but the registered citation count did not
# move" is exactly the shape that looks like a forgotten update.  Measured, both arms of the
# accounting:
#   aTester_Front.cpp  filename-carrying 661 -> 661   bare :NNN 797 -> 797
#   aTester_Rear.cpp   filename-carrying 425 -> 425   bare :NNN 698 -> 700  (+2)
# The four appended notes cite REPORT IDs (#13/#14) and bare `golden :3969-3971` / `golden :3971`,
# never `filename:line`, so the tree-wide filename-carrying total is untouched.  The GL-6g
# baseline-staleness check was run FIRST (`git log --name-only aba6233..HEAD` shows neither file
# touched this session), so the wt_scale baseline is valid.  LINE-NEUTRAL (+2/-2 per file).
# -Wunused-but-set-variable: 110 raw / 110 deduped, 73 distinct variable names, 36 files.
#   ⚠ THIS BUCKET PRODUCED NO NEW GOLDEN DEFECT OF ITS OWN.  The decisive test was whether golden
#   READS a variable that the port stopped reading (a translation gap) or golden never reads it
#   either (golden's own dead assignment).  Compared occurrence counts per (file, variable) across
#   both trees with COMMENTS STRIPPED:  golden-more = 0,  identical = 59,  port-more = 14.
#   ⚠ The comment-INCLUSIVE version of that same comparison produced exactly ONE candidate
#     (AutoClean.cpp bIsSuckICFallDown, port 36 / golden 37) and it was a FALSE POSITIVE created by
#     counting the port's own added comments.  Strip comments before comparing occurrence counts.
#   Representative benign shape -- aHotPlateSubstrate.cpp:533-556 (golden MyKitSuck.cpp:458):
#     iHasNullICCount and iNullICCount are incremented in the scan loop but only iOtherCount is
#     read (`if(iOtherCount) return true;`).  Dead bookkeeping, golden-identical.  16 of the 110
#     sites are these two counters in that one file.
#   ★ WHAT THE BUCKET DID PRODUCE came from READING ONE OF THE WARNED LINES, not from the warning:
#     aTester_Front.cpp:5289 `static bool bVerifyNG` sits under a note block headed
#     `GOLDEN BUGS PRESERVED, NOT FIXED` (:5181-5193) listing FOUR golden bugs, and TWO OF THEM
#     HAD NEVER REACHED docs/UPSTREAM_DEFECT_REPORT.md.  Verified independently against golden
#     (cp950-decoded) and registered as report #13 (missing parens -> (A && B) || C, so
#     REAL_TIME_CCD=false still raises ShowIndexMotorError on a Z2 encoder fault) and #14 (the Z2
#     encoder is checked against Prod.TestZ1_Safe).  Both present in BOTH twins:
#     golden aTester_Front.cpp:3969-3971 and golden aTester_Rear.cpp:3906-3907.
#     Reachability measured myself, not taken from the note: golden tree-wide,
#     DoFRTCUseSocketFloat = 5 hits and DoBRTCUseSocketFloat = 5 hits, each being the definition
#     plus four of its OWN string literals -> NO CALLER -> golden dead code today.
#   ⚠⚠ SYSTEMIC FINDING, quantified: iron rule 3 says golden defects go into the upstream report.
#     Of 637 production .cpp/.h, 72 carry in-source `GOLDEN BUG`/`GOLDEN DEFECT` notes; the report
#     names 19 of those files; 53 ARE NEVER MENTIONED.  That 53 is a FILE-COUNT LOWER BOUND, not a
#     defect count -- one file can hold several claims, some claims are negations, some are
#     language-compatibility notes (e.g. aTester_Rear.cpp:7453, golden :5412 `static bRetryRTC=`
#     K&R implicit int) that do NOT belong in a functional-defect report.  Full list and the
#     recommended catch-up method are in the report's new appendix.
# AI(W906-GL-6n) 20260901: 16811 -> 16814 (net +3), confined to one file.  Baseline-staleness check
# run FIRST (`git log --name-only aba6233..HEAD` shows ckernel.cpp untouched this session), so the
# wt_scale baseline is valid:
#   ckernel.cpp  filename-carrying 218 -> 221  (+3)   bare :NNN 706 -> 712
#   The +3 are the three FILES the defect-15 note cites: ckernel.cpp, cmydef.cpp, main.cpp.
#   LINE-NEUTRAL (+1/-1; the note was appended to the existing `DO NOT "CORRECT" ANY OF THIS.` line).
# This wave is the FIRST of the in-source-note catch-up line opened by GL-6l/GL-6m: ckernel.cpp was
# the largest unregistered file (19 GOLDEN BUG notes, all from AI(W906-W7-L2) 20260803).  Its
# self-declared headline one is now report #15.
#   ⚠ Every claim in that note was re-verified against golden by me, NOT quoted (iron rule 4):
#     golden ckernel.cpp:2176 `ret=SnRKPowerOff` and :2181 `ret=SnRKPowerOn` return ALREADY-REAR ids;
#     :2382-2383 then adds SnRKPowerOff again; cmydef.cpp:804=14 / :805=15 / :819=28 / :820=29 make
#     14+14=28 (SnRKManualStep) and 15+14=29 (SnRKManualTStart); the release sweep :2395 asks
#     Sen[i].IsOn() for the SAME index, so the latch clears on the next call -> auto-repeat.
#   ⚠ I also independently checked the note's OWN safety claim ("largest index this function can
#     ever produce is 29", which is what makes the bK[64]-vs-i<32 mismatch harmless): enumerated
#     every `ret=` RHS in golden :1919-:2406 -> 15 distinct symbols, max SnRKPowerOn=15, so max
#     p = 29 < 32.  The claim holds.  It holds only because SnRKManualStep/SnRKManualTStart are
#     never returned as `ret` (they appear nowhere in golden ckernel.cpp) -- i.e. harmless by
#     COINCIDENCE, and a future id > 17 would latch permanently.  Recorded as such in the report.
#   ⚠ Consequence pinned at the dispatch, not guessed: golden main.cpp:2638-2645 turns these keys
#     into NewRecordProcess("MES2117","POWER OFF pressed") / ("MES2128","POWER ON pressed").  The
#     code's own comment says the customer REQUIRED the press to be recorded -- so the defect
#     corrupts exactly the audit trail it was added to produce.  Front keys are unaffected.
# AI(W906-GL-6o) 20260901: 16814 -> 16819 (net +5).  USER RULING 20260901 ("都依據建議執行") on the
# four pre-real-machine items; this wave is STEP 1 of the password-gate recommendation only --
# make the hardcoded password-book path redirectable.  BOTH GATES (P-R1)/(P-S1) STAY CLOSED.
#   forms/fPassword.cpp  filename-carrying 1 -> 1   (+0)  LINE-NEUTRAL, 2 literals replaced in place
#   forms/fPassword.h    filename-carrying 24 -> 29 (+5)  the new block cites fPassword.cpp,
#                        wb_serve.cpp:225-253, common.cpp:102, Public/cBootLog.cpp:33-38 and
#                        docs/GL_0m_CITE_TREEWIDE.md
#   Baseline-staleness check run FIRST: neither file touched this session -> wt_scale valid.
# ⚠ WHY THE .cpp HAD TO STAY LINE-NEUTRAL AND EVERYTHING NEW WENT IN THE .h: CLAUDE.md cites
#   forms/fPassword.cpp:380/:389/:394/:399/:404/:409/:414/:422/:426-:433, which
#   tools\steering_cite_check.ps1 guards.  Measured first: `fPassword.h:<line>` occurs 0 times in
#   any .cpp/.h (all 110 hits are inside the GENERATED docs/GL_0m_CITE_TREEWIDE.md), so growing the
#   header breaks nothing gated.  All ten cited .cpp lines re-checked in place after the edit.
# ⚠ THE REDIRECT CARRIES AN ASSERTION, because a redirect that silently fails looks exactly like
#   one that works.  forms/fPassword.h has a constexpr streq + static_assert pinning the DEFAULT to
#   golden's literal character-for-character; it deliberately does not fire when the path is
#   overridden with -D.  PROVEN IN BOTH DIRECTIONS: default -> g++ exit 0; default edited to
#   "...\WRONG.ini" -> `error: static assertion failed: EVENLOG_LEVEL_INI default drifted from
#   golden's hardcoded literal`, exit 1.
# ⚠⚠ AND A PROCESS MISTAKE WORTH KEEPING: I proved the red by editing the header and then running
#   `git checkout -- forms/fPassword.h` to undo it.  That reverted the file to HEAD and therefore
#   THREW AWAY THE UNCOMMITTED REAL EDIT IN THE SAME FILE, not just the scratch one.  It was caught
#   only because the next build failed with "'EVENLOG_LEVEL_INI' was not declared in this scope" --
#   i.e. by a compiler, not by me noticing.  NEVER use `git checkout --` to undo a scratch edit in a
#   file that also holds uncommitted work: save and restore the content, or commit first.
# AI(W906-GL-6r) 20260901: 16819 -> 16821 (net +2), confined to ckernel.cpp.
# ⚠ THE BASELINE-STALENESS CHECK MATTERED HERE, unlike the last few waves: `git log --name-only
#   aba6233..HEAD` shows ckernel.cpp WAS edited this session (GL-6n, commit 3402259), so the
#   wt_scale worktree baseline is STALE for this file and would have produced a phantom delta.
#   Used `git show HEAD:ckernel.cpp` instead: filename-carrying 221 -> 223.
#   The +2 are cmydef.h:4401 and cmydef.cpp:4546 (cmydef.h:4414 was already cited).
# LINE-NEUTRAL (4 lines for 4; 4,130 unchanged).
# This is GL-6n's follow-up: ckernel.cpp had 19 in-source golden-note claims and GL-6n registered
# only the one the file itself flagged as headline.  This wave takes the next one, ckernel.cpp:4047
# ("THE PRIORITY IS PURE SOURCE ORDER ... ELEVEN"), and it turned into report defect #17 with TWO
# parts plus a correction of the tree's own note:
#   17(a) golden ckernel.cpp:2529-2586 InitialTestDelayStatus is ELEVEN INDEPENDENT `if`s with
#         ZERO `else` (measured: 11 ifs / 0 else), every arm assigning `str` unconditionally, so
#         the LAST arm in source order wins.  Source order 11,6,3,2,4,5,1,7,9,8,10 -> effective
#         priority is its REVERSE: 10>8>9>7>1>5>4>2>3>6>11.  The newest arm [11] (golden :2532,
#         Steven 20221214) was added FIRST and therefore ranks LAST.  The in-source note claimed
#         exactly this and every number of it checked out.
#   17(b) ⚠ WHAT THE NOTE MISSED, found by measuring the WRITE side: [10] ("SOT monitor time over
#         delay", golden :2582, //kevin 20181101 add SOT) has EXACTLY ONE occurrence in the whole
#         golden tree -- that read.  It is NEVER WRITTEN.  So the arm with the HIGHEST effective
#         priority is dead and its message can never appear.  Mirror image of defect #4 (written
#         then overwritten): this one is read but never written.
#   ⓘ AND ONE CLAIM OF THE NOTE'S WAS WRONG, so it was corrected in place: it said indices
#     0/12/13/14 "produce an EMPTY status string".  Never-read is true, the consequence is not --
#     they are never WRITTEN either (writes exist only for 1-9 and 11, all in atester.cpp
#     :10079-:10256), so that outcome is UNREACHABLE, not a live gap.
# ⚠ My first version of the note edit was +13 lines.  Measured the risk (0 of 194 `ckernel.cpp:<n>`
#   citations point above :4062; CLAUDE.md's highest is :4024) and it was provably harmless -- then
#   compressed it to 4-for-4 anyway, because a convention that only holds when someone remembers to
#   measure is not a convention.
# AI(W906-GL-6t) 20260901: 16821 -> 16823 (net +2), confined to ckernel.cpp.
# Baseline-staleness check run FIRST and it MATTERED again: ckernel.cpp was edited this session by
# BOTH GL-6n and GL-6r, so wt_scale is stale; used `git show HEAD:ckernel.cpp` -> 223 -> 225.
# The +2 are `golden MachineType.h:533` and `port MachineType.h:565`.  LINE-NEUTRAL (1 for 1).
# GL-6n's follow-up continues: ckernel.cpp's 19 in-source claims, third one taken (:375, "THE WHOLE
# LOOP IS DEAD").  Registered as report defect #18.
#   ⚠ EVERY claim in that banner was re-verified and ALL HOLD: golden ckernel.cpp:161-183's loop
#     body contains ONLY `continue` (no return, no break, no assignment, no call -- read line by
#     line); the only false path is :159-160 `if(iHeaterCooling!=0) return false;`; `int i` at :156
#     exists solely for the dead loop; the iIndexHeatMode ladder covers 0-3 and not 4/5.
#   ⚠ THE ONE THING THE BANNER GOT WRONG was its citation: it wrote "MachineType.h:538", which is
#     wrong in BOTH trees -- golden :538 is `ctScannerCategory=4` (an unrelated enum).  The enum is
#     golden :533 / port :565.  Corrected in place.
#   ★ WHAT MAKES IT WORTH REPORTING is what the dead loop gates.  tcTotalCount = 71 (golden
#     MachineType.h:648-649), so the loop spins 71 times doing nothing -- and CheckThermo()'s return
#     IS used, at golden ckernel.cpp:405-409 which shows
#     "Kit tempture too high!!" / "治具目前溫度尚未冷卻,不能進行測試!!" and returns false to block
#     the test, plus :1613 which drives the CoolTime display.  Since the loop cannot return false,
#     that interlock's ONLY input is iHeaterCooling, whose sole non-zero write tree-wide is
#     uHeaterThread.cpp:1041 `iHeaterCooling=Temperature.fAbitColdTime;` -- a TIMER, not a
#     temperature reading.  So the "kit not cooled yet" guard reads no zone temperature at all.
#     ⚠ Stated precisely in the report: this is NOT "no guard" -- iHeaterCooling does encode a
#     cooling period.  What is missing is the per-zone comparison the SKIP scaffolding implies.
#   ⚠ AND FIXING IT WOULD RED AN EXISTING TEST, which the report says up front:
#     tests/test_w7_l2_ckernel.cpp:1289/:1292 already pin that CheckThermo() is TRUE in every cell
#     of a 90-cell grid (6 iIndexHeatMode x 5 temperature modes x 3 iHeaterCooling).  That pin is
#     correct translation acceptance; it going red after an upstream fix is expected, not a
#     regression.  The report also warns the fix must handle iIndexHeatMode 4/5 at the same time,
#     because restoring the comparison without them would make those two modes skip nothing.
# AI(W906-GL-6t, second half) 20260901: 16823 -> 16824 (+1), uHeaterThread.cpp 83 -> 84.
# The +1 is `golden ckernel.cpp:906` (the consumer of the flag); the rest of that note's references
# are bare :NNNN.  wt_scale baseline was VALID for this file (staleness check: untouched this
# session).  LINE-NEUTRAL (1 for 1).
# Registered report defect #19 -- and the reason it got registered at all is worth keeping:
#   golden_note_registry_gate went red because defect #18 CITED uHeaterThread.cpp:1041 as evidence,
#   which moved the file into the defect body and made UNREGISTERED drop 50 -> 49 while that file's
#   own nine documented golden bugs (A..I, :206-244) were all still unregistered.  A false credit.
#   The gate's rule that a DROP is also a failure is what surfaced it.  The fix was to do the work:
#   bug A verified against golden and registered as #19.  See that gate's own comment for detail.
# AI(W906-GL-6u) 20260901: 16824 -> 16827 (+3), confined to uHeaterThread.cpp (84 -> 87).
# PREDICTED BEFORE MEASURING, per the reconciliation discipline established at GL-1l..GL-1t.
# `git diff -U0 -- uHeaterThread.cpp` filtered for filename-carrying `file:line` gives
# 3 added / 0 removed: `CosFunction.cpp:579` (bug B's sole flag writer) on one line, and
# `cmydef.cpp:5408` + `main.cpp:9539` (bug C's constant definitions and their assignments)
# on another.  Everything else the two notes reference is bare :NNN and does not count.
# LINE-NEUTRAL (2 for 2) -- the two additions extend existing comment lines in the
# GOLDEN BUGS PRESERVED VERBATIM block at :212-218, they do not insert lines.
# Registered report defects #20 (bug B) and #21 (bug C).
#   ⚠ The registry gate's counts do NOT move this wave, and that was CHECKED rather than
#   assumed -- the trap GL-6t documented (citing a file as evidence buys a false credit)
#   applies here too, because #20/#21 newly cite CosFunction.cpp/.h, cmydef.cpp/.h,
#   uTemp_Set.cpp and vclcompat/AnsiString.h.  All six carry ZERO in-source GOLDEN BUG /
#   GOLDEN DEFECT notes, so none of them is in that gate's denominator: NAMED stays 23,
#   UNREGISTERED stays 49.  uHeaterThread.cpp was already named (via #19), so registering
#   two more of its own bugs cannot move a per-FILE metric either -- which is a real
#   limitation of that gate, not a pass: it counts files, so bugs D..I of this same file
#   are invisible to it.  They are stated as outstanding in #19's port-status section.
# AI(W906-GL-6v) 20260901: 16827 -> 16828 (+1), confined to uHeaterThread.cpp (87 -> 88).
# PREDICTED BEFORE MEASURING.  `git diff -U0 -- uHeaterThread.cpp` filtered for
# filename-carrying `file:line` gives 1 added / 0 removed: `cmydef.cpp:3706`, the global
# that golden :168's `static bool bShuttleCooling` shadows -- cited because the static
# storage is WHY the latch (and therefore the hysteresis) was the design intent.
# Everything else bug E's note references is bare :NNN.  LINE-NEUTRAL (1 for 1).
# Registered report defect #22, and it is the heaviest of this file's batch so far:
#   the dead `else` arm contains the START threshold (golden :477, whose own comment says
#   "常溫溫度高於1度才開始吹氣"), so the hysteresis collapses to a single recomputed
#   threshold and the consequence lands on a physical valve (golden :1325-1328
#   SW[SwShuttleCooling].On()/.Off()).  Four correct instances of the same idiom exist in
#   the same file -- one of them (:390-406) on the SAME variable, line-for-line identical
#   hysteresis body, differing only by not having the stray pre-set.
#   ⚠ Registry gate counts do not move again (uHeaterThread.cpp was already named via #19,
#   and cmydef.cpp carries 0 in-source golden notes).  Same file-vs-defect unit limitation
#   recorded at GL-6u; D/F/G/H/I of this block remain outstanding and are listed as such.
# AI(W906-GL-6w) 20260901: UNCHANGED at 16828, and that was predicted too.  GL-6w rewrote bug F's
# note (4 for 4, line-neutral) but every citation it added is a bare :NNN -- the CosFunction.cpp /
# uTemp_Set.cpp citations went only into the report .md, and cite_check scans only .cpp/.h.
# "This wave needs no ratchet change" is itself a prediction worth verifying; measured +0/-0.
#
# AI(W906-GL-6x) 20260901: 16828 -> 16829 (+1), confined to uHeaterThread.cpp (88 -> 89).
# PREDICTED BEFORE MEASURING.  `git diff -U0` filtered for filename-carrying `file:line` gives
# 1 added / 0 removed: `MachineType.h:632` (tcHotPlate1=0), cited because channel 0 is what the
# broken low-temperature report actually names.  LINE-NEUTRAL (5 for 5).
# GL-6x WITHDREW note D as NOT A DEFECT and registered report #24 instead:
#   D claimed :1164/:1302/:1614 "omit the +100" that eight sibling sites apply.  All 11
#   WAR15%02d sites classified by guard, zero counterexamples: +100 marks the HIGH/FAULT family,
#   no offset marks the LOW family -- one channel, two codes.  :244's own comment records the
#   offset migrating i+1 -> i+50 -> i+100 and :204's records WAR1560 -> WAR15110 (tcCCD=10 OVER,
#   10+50 -> 10+100), both inside the high family only.  ⚠ ACTING ON D WOULD HAVE MADE IT WORSE:
#   adding +100 to the low sites turns "too cold" into the "too hot" code.
#   The real defect the same sweep found: golden :659 / :960 (LB LOW arms) write the index to
#   pos1 while loop 2's low reporters :1164/:1302 read pos, so an LB undertemperature names
#   Hot Plate 1 (pos still 0 from :148).  ⚠ golden :1409 has the same SHAPE and is NOT a defect --
#   loop 3 reads pos1 in all its consumers (:1605/:1609/:1614), so it is self-consistent.  The
#   discriminator is producer/consumer pairing, not variable naming.
#   → SECOND wave in a row where an in-source note was wrong, after F/#23's mechanism.
#   "Registered" is not "correct"; both facts are now stated in the report.
# AI(W906-GL-6y) 20260901: 16829 -> 16830 (+1 net), confined to uHeaterThread.cpp (89 -> 90).
# PREDICTED BEFORE MEASURING.  `git diff -U0` filtered for filename-carrying `file:line` gives
# 2 added / 1 removed: -`MachineType.h:637` (note H's citation, WRONG), +`MachineType.h:633`
# (correct -- tcCCD=10 lives there; :637 is tcDUT1=29..tcDUT4=32), +`cmydef.cpp:2761`
# (bHeatOKBellowError's definition, cited as the sibling-latch control group).
# LINE-NEUTRAL (2 for 2).  Registered report defects #25 (bug I) and #26 (bug H).
#
# ⚠ COVERAGE GAP FOUND, NOT A GATE FAILURE -- QUEUED, NOT FIXED THIS WAVE:
#   cite_symbol_check did not flag `MachineType.h:637` even though it is exactly the class that
#   tool exists for (line exists, holds real code, is the wrong line).  Reason is specific and
#   legitimate: its symbol-carrying test at cite_symbol_check.ps1:330 requires the identifier to
#   be followed by `(` or `;`, i.e. it validates FUNCTION declarations and statements.  An enum
#   member (`tcCCD=10,`) is followed by `=`, so such citations are skipped as unresolvable rather
#   than checked.  So the gate is silent by construction here, not broken.
#   ⚠ AND THIS IS THE SECOND WRONG MachineType.h ENUM CITATION: GL-6t corrected #18's
#   `MachineType.h:538` (eIndexHeatMode, actually :533) the same way.  Two of the three
#   note-citation errors found so far are enum-member citations -- i.e. the gap has a measured
#   hit rate, which is the argument for building an enum-member discriminator.  Queued in the
#   RESUME with this evidence; building it is its own wave, not a side effect of this one.
# AI(W906-GL-6z) 20260901: 16830 -> 16832 (+2), confined to uHeaterThread.cpp (90 -> 92).
# PREDICTED BEFORE MEASURING.  `git diff -U0` filtered for filename-carrying `file:line` gives
# 2 added / 0 removed: `csystem.cpp:1215` and `main.cpp:13002`, the first of the ten places that
# clear bHeatOKBellowError -- cited because the OPEN QUESTION this wave raises (does path (b) of
# golden :1293-1298 need SystemStart?) is decided by whether any of those clears follows
# SystemStart going false.  LINE-NEUTRAL (1 for 1).
# WITHDREW note G as NOT A DEFECT at all three sites, and registered report #27.
#   G claimed `&&`/`||` mixed without parentheses at golden :1125-1126, :1221-1226, :1293-1298.
#   Evaluated against C precedence and against golden's own comments: :1125-1126 is two
#   COMPLEMENTARY cases (iHome==0 / iHome!=0) each with its own threshold; :1293-1298 is exactly
#   the two paths golden :1299's comment describes; and :1221-1226 ALREADY HAS every paren.
#   Style observation, not a bug -- and the note now says so, so nobody goes paren-hunting for a
#   behaviour change that does not exist.
#   The real finding the enumeration produced: golden :1224's disjunct is `A && B && D` while
#   :1222 is `A && B` -- a STRICT SUBSET, verified by truth table (all 8 rows of (A,B,D) identical
#   with and without it).  bHeatOKOverError therefore LOOKS like an entry condition and
#   contributes nothing; it is really used inside the block at :1234 to pick CheckHeater 9 vs 10.
#
# ⚠⚠ PROCESS NOTE -- I BROKE LINE-NEUTRALITY HERE AND THE NUMSTAT CAUGHT IT, NOT A GATE:
#   the first version of G's note replaced 1 comment line with 3, taking the file 2105 -> 2107.
#   Comment growth inside a file is invisible to cite_check (every citation still resolves) and
#   to the wrap-up gate (it reports line counts but pins none for this file), so the only thing
#   that surfaced it was reading `git diff --numstat` and seeing `3  1` instead of `1  1`.
#   Fixed by merging the three lines back into one before building.  ⚠ THE HABIT THAT SAVED IT
#   was checking numstat every wave as part of the delta reconciliation -- keep doing that.
#
# ⚠ uHeaterThread.cpp's A..I note block is now fully worked: A=#19, B=#20, C=#21, E=#22, F=#23,
#   H=#26, I=#25, D and G WITHDRAWN, plus #24 and #27 found while measuring the two withdrawn
#   ones, plus one open question.  TWO OF NINE NOTES (22%) WERE NOT DEFECTS -- that ratio is
#   recorded in the report for whoever opens the next file's note block.
# AI(W906-GL-7f) 20260901: 16832 -> 16840 (+8), confined to ckernel.cpp (225 -> 233).
# PREDICTED BEFORE MEASURING.  `git diff -U0 -- ckernel.cpp` filtered for filename-carrying
# `file:line` gives 8 added / 0 removed: csystem.cpp:15833 (CheckMotorHome's all-motor loop),
# ainarm9045_1x1_1.cpp:1002 (the clear that only runs on success), ainarm9045.cpp:2564
# (bCheckYPitchHome), cmydef.cpp:3125 + database.cpp:711 (the USE_IN_Y_IS_AUTO_PITCH switch),
# ckernel.cpp:427 (the flag's only read), cmydef.cpp:4778 + cmydef.h:4688 (its declaration).
# LINE-NEUTRAL (8 for 8), ckernel.cpp stays at 4130 lines.
# Registered report defect #28 -- and it CORRECTS the note's claim about INTENT, which is the
# third time in this campaign an in-source note's reasoning (not just its line numbers) was wrong:
#   the note said the arm's only real effect is skipping the CheckMotorHome() gate and that "a
#   clearing =false, or an actual home call, was plainly meant".  Measured: skipping that gate IS
#   the point -- CheckMotorHome (csystem.cpp:15833) demands HomeFlag==1 for ALL TOTAL_MOTOR
#   motors, so during a genuine Y-pitch home the axis is legitimately off-home and without the arm
#   ScanSystemSensor would raise "Must home again" in a NORMAL state.  Changing :429 to =false or
#   deleting the arm would REINTRODUCE that false alarm.
#   The real defect is the exemption's SCOPE (covers every motor, not just MInArmPitchY) and its
#   RELEASE (cleared only on success; the 10 s timeout raises WAR0123, returns to Task 1600 and
#   leaves the flag true, and 1600 sets it true again).  So a persistently failing Y-pitch home
#   suspends motor-home policing entirely.  Fix spans ~100 variant files -- a campaign, not a line.
# AI(W906-GL-7g) 20260901: 16840 -> 16851 (+15/-4 = net +11), confined to ckernel.cpp (233 -> 244).
# PREDICTED BEFORE MEASURING.  The two notes at :527 and :565 were rewritten to carry the evidence
# for report defect #29: the writer cinitial.cpp:11445 / :11483, the FOUR golden sites that DO
# bounds-check the same field (SortingBinTray.cpp:2551, atester.cpp:2588, auto9045.cpp:1435,
# acatchtray.cpp:477) plus acatchtray.cpp:601 / aoutarm.cpp:624-625 for iIfErrorT6, the type
# declarations cprod.h:511 / :519, and the corrected eTrayCount lines.
# ⚠ The -4 is real removals: the old wording's MachineType.h:1104 (a PRE-IMPORT port line number,
# 1104+27=1131) plus three citations that moved into re-worded sentences.
# LINE-NEUTRAL (14 for 14), ckernel.cpp stays at 4130 lines.
# Registered report defect #29 -- MEMORY SAFETY, and the strongest control group of the campaign
# so far: golden bounds-checks Prod.iT6CatData[] in FOUR other places and one of them
# (SortingBinTray.cpp:2551) carries the comment "Steven 20230929 : 加上保護" -- somebody added
# protection for this exact field in 2023 and never came back to ckernel.cpp.
#   golden ckernel.cpp:213/:214 are FUNCTION-LOCAL arrays, and :231/:232 are WRITES, so a negative
#   index overwrites the stack below them.  Negative is the field's established encoding: its only
#   writer ends in `- 1` (cinitial.cpp:11445, comment "//Auto1 = 0").
#   ⚠ The minimum behaviour-preserving fix is to add ONLY the lower bound -- widening the upper
#   bound to eTrayCount would change which categories the function accepts (9..32 are skipped now).
#
# ⚠⚠ A SEVENTH STRUCTURAL LIMIT OF enum_cite_check FOUND HERE, AND QUEUED NOT FIXED:
#   the stale MachineType.h:1104 citation above was NOT flagged by that gate, because the citation
#   is SPLIT ACROSS TWO LINES -- `eTrayCount` ended one comment line and `(MachineType.h:1104` began
#   the next, and the tool's proximity window is within a single line.  It stayed silent rather than
#   guessing, which is the declared bias, but a real stale citation got through.  A 2-line window is
#   the obvious extension and is exactly the cross-line prose pairing that produced GL-7e's
#   only-defect-I-created; queued with this data point rather than implemented mid-wave.
# AI(W906-GL-7i) 20260901: 16851 -> 16854 (+3), confined to ckernel.cpp (244 -> 247).
# PREDICTED BEFORE MEASURING.  The :3932 note was rewritten for report defect #30 and the three
# new filename-carrying citations are Motor/myGALILmotor.cpp (the driver that sets the flag AND
# queues the alarm in the same breath) and docs/elec/Component/HAlarm.cpp twice (Set's idempotence
# at :110-113 and PopUpAlarm at :264-280).  LINE-NEUTRAL (7 for 7), ckernel.cpp stays at 4130.
# ⚠ HAlarm.cpp is in docs/elec/Component/ -- the directory CLAUDE.md used to list as absent and
# which this session measured as PRESENT.  Citing it is what let the "infinite drain loop" worry
# be RULED OUT by reading code rather than reasoned about.
# AI(W906-GL-7j) 20260901: 16854 -> 16863 (+11/-2 = net +9), confined to ckernel.cpp (247 -> 256).
# PREDICTED BEFORE MEASURING.  Two notes rewritten: :466 (report defect #31) gains the setter
# Motor/mykitsuck.cpp:195-196, the three configuration pairs in cinitial.cpp, OffDestroy at
# mykitsuck.cpp:2113-2117, mykitsuck.h:98 and :11-12, and the sole caller uhome.cpp:2365;
# :1069 is WITHDRAWN and cites the same cinitial.cpp pairs as its refutation.  The -2 are
# citations that disappeared with the withdrawn wording.  LINE-NEUTRAL (23 for 23), 4130 lines.
# Registered report defect #31 AND withdrew a second note without registering anything for it:
#   #31 is real -- golden :193 uses iMaxRow as the COLUMN bound, so StopAllDestroy (called only
#   from the HOMING sequence, uhome.cpp:2365) leaves columns 2..3 on a 2x4 machine and 2..7 on a
#   2x8 machine with their destroy solenoid still energised.  Arrays are [4][8], so this is
#   UNDER-coverage, not an overrun.
#   ⚠ BOTH notes' claims about the OUT-ARM grid are WITHDRAWN on one measurement: SetItemAmount is
#   called on InArmSuck and OutArmSuck in MATCHED PAIRS with equal arguments and nowhere else
#   (cinitial.cpp:5565/:5566, :5576/:5577, :5585/:5586), so the two grids are always the same size
#   and "a machine whose out-arm grid is larger" does not exist.  Registering that verbatim would
#   have sent upstream hunting a configuration the tree cannot produce.
#   ⚠ ckernel.cpp:1069 is therefore NOT a defect and gets NO report entry -- its bounds
#   (golden :543 iMaxRow / :545 iMaxCol) are correct, unlike :193.
# AI(W906-GL-7k) 20260901: 16863 -> 16869 (+6), confined to ckernel.cpp (256 -> 262).
# PREDICTED BEFORE MEASURING.  The :2964 note was rewritten for report defect #32 and gains
# cmydef.cpp:1880 (the SwFKPause constant), cinitial.cpp:1007 (its name) and cinitial.cpp:1537
# (+ :1542, the iControlPanelMode==1 disable that BOUNDS the exposure), uPadInterface.cpp:215
# (the pad-interface mirror), plus the ckernel.cpp self-citations for the trigger path.
# LINE-NEUTRAL (19 for 19), ckernel.cpp stays at 4130.
# Registered report defect #32 -- verified line by line against golden:
#   front arm drives BOTH Pause lamps in BOTH inner branches (:1801/:1802, :1815/:1816);
#   rear arm drives SwFKPause in its inner IF only (:1849) and its inner else (:1860-1872) has
#   SwRKPause.Off() at :1862 but no SwFKPause.Off(); and the trailing front-clear block
#   (:1873-1882) clears TEN other front-key lamps and misses this one.  SwFKPause has exactly
#   four writers tree-wide, all in this function, so nothing else repairs it.
#   ⚠ SCOPE DELIBERATELY BOUNDED IN THE WRITE-UP: cinitial.cpp:1537's iControlPanelMode==1 path
#   disables every front-key lamp, so single-panel machines are unaffected.  Stating the bound is
#   what keeps a real indication fault from being read as a machine-wide one.
#   ⚠ The note's SECOND claim (Power-lamp write ORDER differs) is explicitly recorded as NOT a
#   defect -- four independent outputs, order cannot matter.  Written down so it is not re-checked.
# AI(W906-GL-7l) 20260901: 16869 -> 16876 (+8/-1 = net +7), confined to ckernel.cpp (262 -> 269).
# PREDICTED BEFORE MEASURING.  The :3913 note is WITHDRAWN and now carries the evidence that
# withdraws it: Motor/mymotor.cpp:1798-1819 (GetErrorIndex's value space), csystem.cpp:4064 and
# the uhome.cpp call sites, uhome.cpp:3059/:3143 (the hard-coded 8), and note.cpp:4291-4296 /
# note.cpp:1077 (how the number becomes an alarm-table key).  LINE-NEUTRAL (13 for 13), 4130 lines.
# ⚠ NO REPORT ENTRY THIS WAVE -- the note was withdrawn, not registered.  ckernel.cpp:3913's
# iRef==9 -> iRef=7 remap is a DELIBERATE, uniformly applied clamp:
#   (1) all 13 GetErrorIndex -> ShowMotorErrorMessage(JamCode, iRef+1) paths carry it, zero
#       exceptions, so golden never emits iRef+1 == 10;
#   (2) two sites pass a hard-coded 8 for the same purpose (uhome.cpp:3059, :3143);
#   (3) GetErrorIndex returns 0..7 and 9 -- never 8 -- so without the remap the emitted space
#       would be 1..8 plus a stray 10 with a HOLE at 9; with it the space is exactly 1..8.
#   A clamp that closes a hole and removes an out-of-range value is a convention.
# ⚠ HONEST LIMIT RECORDED IN THE NOTE: whether a WAR24<mot>10 row exists in the alarm table could
#   NOT be measured here -- D:\HT9045\system\alarm.DB is Paradox and its code column is not plain
#   ASCII (column names and messages are readable; decoding the codes would be guesswork).  The
#   three source-level measurements settle it without the table, and the note says so rather than
#   implying the table was consulted.
# ⚠ This is the FOURTH in-source note withdrawn this campaign (uHeaterThread.cpp D and G,
#   ckernel.cpp:1069, now :3913) -- the running evidence that notes are leads, not a defect list.
# AI(W906-GL-7m) 20260901: 16876 -> 16879 (+3), confined to ckernel.cpp (269 -> 272).
# PREDICTED BEFORE MEASURING.  SIX notes rewritten (46 lines for 46, line-neutral, 4130 lines) but
# only three new filename-carrying citations, because most of the added evidence is same-file
# golden :NNN: cContact.cpp:2258 / :239 / :1171 -- the lifecycle of fContact->bSetupStep that shows
# the safe-lock-fail path consumes the software step request.
# TIER 3 OF ckernel.cpp DONE, as ONE combined report entry #33 rather than five thin ones:
#   33-A golden :2451  iCcwLed%d missing its colon -- REAL: the row goes to MyDBIProcessNew
#        ("Motion","WAR240004"), so name:value parsers lose that field.  ⚠ Fixing changes logged
#        text, so it is registered rather than repaired.
#   33-B golden :2564  "Fisrt" -- REAL, operator-visible; the port cannot fix it because the
#        regression tests assert golden's string, which is also why it survived upstream.
#   33-C golden :77/:89 duplicate dead store -- ZERO effect.  ⚠ The useful part is that :77 runs
#        BEFORE the IsSafeLockCheck() early-out at :85-86, so the software step request is consumed
#        even when the step is denied; the physical key self-recovers because Sen[SnRKManualStep]
#        is a level input.  Recorded as an observation for upstream, NOT claimed as a defect.
#   33-D golden :347-353 two empty brace pairs -- ZERO effect.
#   33-E golden :2434/:2459/:2464 write-only static -- ZERO effect; exactly three occurrences in
#        the whole cp950-decoded file, all writes.
# ⚠ The SIXTH note in that group, golden :2440, is WITHDRAWN -- && short-circuits, so the
#   dereference needs Comp itself to be NULL, and Comp comes from the alarm FIFO which only ever
#   carries the object that alarmed.  Safe by construction.  Withdrawn rather than deleted because
#   it is still a fragility if anyone ever lets a NULL into that queue.
# ⚠ COMBINING WAS DELIBERATE: five separate entries would dilute #28-#32.  Every item keeps its own
#   verdict and severity inside #33, so nothing is hidden -- the alternative was padding.
# AI(W906-GL-7n) 20260901: 16879 -> 16887 (+8), confined to ainarm9045.cpp (0 -> 8; this file had
# no filename-carrying citations before).  PREDICTED BEFORE MEASURING.  The :2768 note now carries
# the reachability evidence for report defect #34: SmartSetup.cpp / uCleaning.cpp / auto9045.cpp
# for the two unhandled modes, the caller list (AutoClean.cpp, InOutArmZteach.cpp,
# ainarm9045S_1x4_4.cpp), and CMakeLists.txt:372 for the no--Werror point.
# LINE-NEUTRAL (7 for 7), ainarm9045.cpp stays at 11602 lines.
# First wave on ainarm9045.cpp.  ⚠ THIS FILE IS SHAPED DIFFERENTLY from uHeaterThread.cpp and
# ckernel.cpp: its 17 note HEADERS include six BLOCK notes that each list several numbered
# sub-claims (5+0+4+6+5+3 = 23 sub-items measured), so the real claim count is ~34, not 17.
# That is several waves; GL-7n did the survey plus the strongest tier-1 item.
# AI(W906-GL-7o) 20260902: 16887 -> 16889 (+2), both in ainarm9045.cpp's :10236 block note.
# PREDICTED BEFORE MEASURING, mechanically from `git diff -U0` filtered for filename-carrying
# `file:line`.  The block was rewritten 11 lines for 11 and the two RETAINED citations
# (cprod.h:1308, MachineType.h:642) cancel out, so the delta is exactly the two NEW ones:
# MachineType.h:382 (MAX_ARM_Col) and cinitial.cpp:5585 (the 2x8 grid) -- the evidence for
# report defect #36.  ⚠ Written as MachineType.h:382 and NOT as a range `:382-383` on purpose:
# whether cite_check counts a range as one citation or two is not something I have measured, and
# a prediction must not rest on an unmeasured counting rule.
# ⚠ The same wave also repaired a corrupted line in this file that carries NO citation and
# therefore does not move this number: :7336's ABSENCE COMMAND held two literal 0x08 BACKSPACE
# bytes where `\b` belonged (`"<BS>LotRecordUPH<BS>"`), so the recorded verification command
# could not be re-run as written.  Found while checking my own edit for control characters.
# AI(W906-GL-7o) 20260902: 16889 -> 16890 (+1), again confined to ainarm9045.cpp's :10236 block
# note, rewritten 11 lines for 11.  PREDICTED BEFORE MEASURING: retained unchanged were
# MachineType.h:382 and cinitial.cpp:5585; REMOVED were cprod.h:1308 and MachineType.h:642
# (both of which were WRONG against golden -- see below); ADDED were cprod.h:1307,
# cprod.cpp:111 and MachineType.h:610.  -2 +3 = +1.
# ⚠ WHY THOSE TWO CITATIONS WERE REMOVED: sub-item 3 of that note is now WITHDRAWN as NOT a
# defect, and its two citations were both off.  golden cprod.h:1308 is a DIFFERENT array
# (AutoFromEmptyColor), the real one is :1307; golden MachineType.h:642 is eTempControll's
# tcCCD_2=52, and rsmFIFOMode=12 lives at :610.  The claim itself conflated TWO VARIABLES that
# differ only by a `LastSet.` prefix: the GLOBAL iRunStartMode (cmydef.cpp:3332) only ever holds
# RT=0 / FT=1 (cprod.cpp:111, measured across all 26 writers and 15 comparisons), so
# LoaderToEmptyColor[2] is indexed in range, while the ">=12" belongs to LastSet.iRunStartMode.
# ⚠ The replacement text deliberately writes "the old note said 1308" WITHOUT a colon, so the
# superseded number cannot be re-parsed as a citation -- same reason GL-7o avoided a `:382-383`
# range: a prediction must not rest on a counting rule I have not measured.
# AI(W906-GL-7r) 20260902: 16890 -> 16891 (+1), in ainarm9045.cpp's :8030 block note,
# rewritten 14 lines for 14.  PREDICTED BEFORE MEASURING: the old sub-items (1)-(5) carried
# ZERO filename-carrying citations (all bare `:NNNN`), and the rewrite adds exactly one --
# `golden mytray.cpp:41-43`, the evidence that ClearData/SetData only ever loop < XItem /
# < YItem and therefore never clear the cell the defect reads.
# ⚠ It is written `golden mytray.cpp:41-43 and :178-179` ON PURPOSE, both halves:
#   * the `golden ` prefix, because GL-7q measured that an unmarked citation is resolved
#     against THIS tree (cite_check.ps1:683) -- five of my own citations were wrong that way;
#   * `:41-43` as a RANGE and the companion as a bare `:178-179`, because GL-7q also measured
#     that cite_check.ps1:672's regex folds a range into ONE match while a bare `:NNNN`
#     goes down the separate companion path and does not add to this total.
# So this is the first delta in the campaign predicted from MEASURED counting rules rather
# than from avoiding the shapes whose rules were unknown.
# AI(W906-GL-7s) 20260902: 16891 -> 16892 (+1), in ainarm9045.cpp's :4236 block note,
# rewritten 12 lines for 12 (all FOUR sub-items kept -- the room for expanding (1) and (2)
# came from compressing (3) and (4) equivalently, not from dropping a claim).
# PREDICTED BEFORE MEASURING: the old sub-items (1)-(4) carried ZERO filename-bearing
# citations (all bare `:NNNN`); the rewrite adds exactly one, `golden MachineType.h:454`,
# which is the measured location of the ONE eTestMode member the 17 cases miss
# (_32Site4X8M = 16, single-arm 32 site) -- the whole basis of report defect #40.
# ⚠ `golden ` prefix per GL-7q's measured rule (cite_check.ps1:683 defaults an unmarked
# citation to THIS tree), which matters especially here: port MachineType.h:454 is a
# different line entirely, and the port/golden offset in that header is +5.
$EXPECT_TREEWIDE = 16892
  # AI(W906-GL-2a) 20260828: 881 -> 883.  The DoInArm_SuckerMap banner rewrite in ainarm2.cpp
  # names the real body, the shared declaration and both shadows, which is +8 citations
  # against -6 removed = net +2.  ⚠ THE GATE CAUGHT THIS, not me: I updated the
  # tree-wide number and forgot the single-file one, and a per-file invariant is exactly what
  # notices that.  Reconciled with `git diff -U0 -- ainarm2.cpp`.
  # AI(W906-GL-2c) 20260828: 883 -> 881, BACK to where GL-1r left it.  The SetInArmHome note
  # rewrite drops more stale citations than it adds (+6/-8 = net -2), reconciled with
  # `git diff -U0 -- ainarm2.cpp`.  A per-file invariant moving DOWN is as much a signal as up.
  # AI(W906-GL-2d) 20260828: 881 -> 882 (+4/-3 = net +1), from correcting the two paired-gate
  # citations in ainarm2.cpp:1053-1059 (ainarm9045.cpp:7974 not :7967; cInArmPlacement.cpp:733
  # not :724).  Reconciled with `git diff -U0 -- ainarm2.cpp`.
  # AI(W906-GL-2g) 20260828: 882 -> 881 (+3/-4 = net -1).  The two ainarm2.cpp paragraphs now say
  # "already retired" instead of listing a stub line to delete, which drops one citation.
  # Reconciled with `git diff -U0 -- ainarm2.cpp` BEFORE touching this number.
  # AI(W906-GL-3c) 20260829: 881 -> 880 (net -1).  ainarm2.cpp:5631 dropped `ainarm2.h:81`:
  # measured, golden declares Check_QA_ModeCount in NO header (only ainarm2.cpp:160, the body,
  # which that comment already cites correctly).  ⚠️ THE GATE CAUGHT THIS, not me -- I edited
  # the line and updated only the tree-wide figure, and the single-file invariant is precisely
  # the isolation check that exists for that mistake.
  # AI(W906-GL-5m) 20260831: 880 -> 879 (net -1).  ainarm2.cpp carried ONE SOFT_SIMULTE STALE
  # marker paragraph, retired with the other 121; its prose cited one `file:line`, and the
  # equal-line-count retirement note that replaced it uses prose form instead.  Reconciled with
  # `git diff -U0 -- ainarm2.cpp` before touching this number: +8/-8 lines, one citation removed.
  # ⚠ ainarm2.cpp's LINE COUNT IS UNCHANGED (8042 before and after) -- that is the whole point of
  # the equal-line-count replacement, and it is why this single-file isolation check moved by
  # exactly 1 instead of by the dozens a bulk deletion would have caused.
  $EXPECT_ONEFILE  = 879      # ainarm2.cpp
  $cc = Join-Path $Tree 'tools\cite_check.ps1'
  $o1 = & powershell -NoProfile -ExecutionPolicy Bypass -Command "& '$cc' -Sources 'ainarm2.cpp'" 2>&1
  $n1 = 0
  foreach ($l in $o1) { if ($l -match '^帶檔名的引用:\s*(\d+)') { $n1 = [int]$Matches[1] } }
  Write-Host ("  單檔 ainarm2.cpp: {0}（登記 {1}）{2}" -f $n1, $EXPECT_ONEFILE, $(if ($n1 -eq $EXPECT_ONEFILE) { 'ok' } else { '✗' }))
  if ($n1 -ne $EXPECT_ONEFILE) { [void]$fails.Add("cite 規模: ainarm2.cpp = $n1，登記 $EXPECT_ONEFILE") }
  # AI(W906-GL-3k) 20260829: this run now also asks for the SHAPE breakdown, which cite_check
  # only prints when -QueueOut is given.  It writes to a SCRATCH path, never the tracked
  # docs\GL_0p_CITE_QUEUE.md -- a gate must not rewrite the artifact it is checking.  Same run,
  # so no extra runtime: one -AllTree pass yields the count AND the shapes.
  $shapeTmp = Join-Path $env:TEMP ("gl_shape_probe_" + $PID + ".md")
  $o2 = & powershell -NoProfile -ExecutionPolicy Bypass -Command "& '$cc' -AllTree -QueueOut '$shapeTmp'" 2>&1
  $n2 = 0
  foreach ($l in $o2) { if ($l -match '^帶檔名的引用:\s*(\d+)') { $n2 = [int]$Matches[1] } }
  Write-Host ("  全樹: {0}（登記 {1}）{2}" -f $n2, $EXPECT_TREEWIDE, $(if ($n2 -eq $EXPECT_TREEWIDE) { 'ok' } else { '✗' }))
  if ($n2 -ne $EXPECT_TREEWIDE) { [void]$fails.Add("cite 規模: 全樹 = $n2，登記 $EXPECT_TREEWIDE") }

  # AI(W906-GL-3k) 20260829: PER-SHAPE BASELINES -- and DELIBERATELY ONLY SIX OF THE EIGHT.
  #
  # WHY THIS EXISTS: GL-3j drove PAST-EOF from 67 to 0 over ten waves, and NOTHING kept it there.
  # cite_symbol_check already carries two registered baselines that were PROVEN to go red;
  # cite_check's shapes had none, so a regression would have been silent -- and this campaign's
  # whole subject is defects that look like nothing.
  #
  # ⚠ WHICH SHAPES ARE GUARDED IS A JUDGEMENT, AND HERE IS THE REASONING:
  #   PAST-EOF / PAST-EOF-BIG / FILE-ONLY-IN-OTHER-TREE  -> EXACT 0.  "The cited line cannot
  #       exist in the tree it resolves to" is a STRUCTURAL fact; any growth is a regression.
  #   WRONG-FILENAME  -> EXACT.  A research queue; it moves only when someone works it.
  #   SYSTEM-HEADER   -> EXACT.  A filesystem fact about the toolchain's include roots.
  #   NOT-ON-THIS-BOX -> EXACT.  A fact about D:\HT9045\elec being absent here (CLAUDE.md says
  #       these are NOT defects), so it moves only if that root appears.
  #
  # ⚠⚠ CODE-NEARBY AND NO-CODE-NEARBY ARE NOT GUARDED, ON PURPOSE, AND THAT IS STATED IN THE
  # OUTPUT.  They are computed from a +-25-line window, so ANY comment edit anywhere can move a
  # row between them; pinning them would produce a gate that fails every wave for no defect --
  # i.e. noise that trains people to update the number without reading it.  The queue document's
  # own measured base rate says CODE-NEARBY carries almost no information anyway (93.8% of random
  # (file,line) pairs have code within +-25 lines, so 82% is BELOW chance).
  # A number printed and openly unguarded is more honest than a number guarded by a band so wide
  # it can never fail.
  $EXPECT_SHAPES = [ordered]@{
    'PAST-EOF'                = 0
    'PAST-EOF-BIG'            = 0
    'FILE-ONLY-IN-OTHER-TREE' = 0
    # AI(W906-GL-3l) 20260829: 10 -> 2.  Eight fixed (four elided `...shims.*` prefixes, four
    # scratch-probe references).  The remaining TWO are BOTH FormsFacade.cpp:24, and they are
    # NOT a filename problem alone: the real facade is forms\fAGV.cpp:38, AND its claim ("the
    # W6.x FormsFacade stub hardwires TfAGV::Use_AMR()==false") HAS EXPIRED -- fAGV.cpp:38 now
    # reads `return AGV_Use_AMR();` and tests\test_amr.cpp:458 asserts it returns TRUE for
    # Keyence+BundleID.  So a test's stated coverage rationale ("structurally unreachable") may
    # no longer hold; that is a test-coverage decision, not a comment fix, and it is queued
    # rather than silently rewritten.
    # AI(W906-GL-3m) 20260829: 2 -> 0.  Both remaining rows were tests\test_w6_3_catchtray.cpp's
    # FormsFacade.cpp:24, whose CLAIM had expired as well as its filename.  MEASURED STATICALLY,
    # and the conclusion SURVIVED while the reason did not:
    #   * TfAGV::Use_AMR() (forms/fAGV.cpp:38) delegates to AGV_Use_AMR()
    #     (Automation/AGV_predicates.cpp:49), which needs USE_COVER_TRAYID != tCIDNotUse AND
    #     IniConfig.bA65_BundleIDList == true.
    #   * USE_COVER_TRAYID is DEFINED as tCIDNotUse (cmydef.cpp:5885), and database.cpp:1601 reads
    #     it from ini WITH THAT SAME DEFAULT, so an absent key does not change it.
    #   * Nothing else writes it except tests that set it deliberately -- and
    #     test_w6_3_catchtray.cpp is NOT one of them (grepped every writer).
    #   * forms/fAGV.h:21-27 ALREADY recorded this on 20260728: the "DUMMY/hardcoded" story is
    #     stale, the predicates are real, and they are false BY CONFIG "not by a hardcoded return".
    # So the branch IS still offline-unreachable (the CHECK still holds) but for a different reason,
    # and the comment now says the measured one.
    # ⚠ STATIC DETERMINATION, NOT A TEST RUN: settled by reading the definition, the ini default
    # and every writer.  What would invalidate it: that test loading an ini that sets
    # USE_COVER_TRAYID non-default.
    # ⚠ AI(W906-GL-4t) 20260830 CORRECTION: the reason given here used to be "the exe lock
    # (PID 23324/8368) blocks iron rule 6's builds".  THAT PREMISE IS FALSE and was carried
    # unmeasured for six waves -- GL-4n measured 2 of 1,234 artifacts held (both in
    # build_nonoracle, none in build_sim_nonoracle) and built BOTH lanes; GL-4s then compiled a
    # real .cpp edit in both.  What the lock actually blocks is RELINKING those two exes, and
    # therefore RUNNING them -- which is still enough to keep this determination static, because a
    # ctest run is what would settle it.  The conclusion stands; the stated reason was wrong.
    'WRONG-FILENAME'          = 0
    'SYSTEM-HEADER'           = 10
    # AI(W906-GL-7i) 20260901: 45 -> 46.  The +1 is report defect #30's new citation to
  # docs/elec/Component/HAlarm.cpp:110-113, which is what let the "infinite drain loop" worry be
  # RULED OUT by reading HAlarm::Set's idempotence rather than reasoning about it.
  # ⚠⚠ THE CITATION IS CORRECT AND THE TOOL CANNOT SEE IT YET, WHICH IS A QUEUE ITEM:
  # cite_check.ps1:422 sets $extraRoots = @('D:\HT9045\elec'), a root that does NOT exist here, so
  # anything whose context mentions elec\ is bucketed NOT-ON-THIS-BOX.  MEASURED 20260901:
  #   D:\HT9045\elec              -> False
  #   D:\HT9045\docs\elec         -> TRUE, 78 .cpp/.h/.pas including HAlarm.cpp, halarm.h,
  #                                  HCylinder.cpp -- exactly the component library those
  #                                  citations point at.
  # Adding D:\HT9045\docs\elec to $extraRoots would let the tool RESOLVE this whole bucket, i.e.
  # turn 46 "unverifiable here" rows into verified ones (or into real findings).  That moves
  # several shape counts at once (NOT-ON-THIS-BOX down, CODE-NEARBY / NO-CODE-NEARBY up, possibly
  # new WRONG-FILENAME) and needs per-shape reconciliation, so it is ITS OWN WAVE and is queued in
  # the RESUME with this measurement -- not bundled onto the end of GL-7i.
  'NOT-ON-THIS-BOX'         = 46
  }
  $UNGUARDED_SHAPES = @('CODE-NEARBY', 'NO-CODE-NEARBY')
  $seenShapes = @{}
  foreach ($l in $o2) {
    if ($l -match '^\s+([A-Z][A-Z-]+)\s+(\d+)\s+[\d.]+%\s*$') { $seenShapes[$Matches[1]] = [int]$Matches[2] }
  }
  if ($seenShapes.Count -eq 0) {
    Write-Host "  ✗ 讀不到任何 shape 行 —— cite_check 的輸出格式變了，這組不變量已失效而不是通過" -ForegroundColor Red
    [void]$fails.Add("cite shape: 解析不到 shape 行（格式可能改了）")
  } else {
    foreach ($k in $EXPECT_SHAPES.Keys) {
      if (-not $seenShapes.ContainsKey($k)) {
        Write-Host ("  ✗ shape {0} 沒出現在輸出裡 —— 不能當成 0" -f $k) -ForegroundColor Red
        [void]$fails.Add("cite shape: $k 未出現在輸出中")
        continue
      }
      $got = $seenShapes[$k]; $want = $EXPECT_SHAPES[$k]
      Write-Host ("  shape {0,-24} {1,5}（登記 {2}）{3}" -f $k, $got, $want, $(if ($got -eq $want) { 'ok' } else { '✗' }))
      if ($got -ne $want) { [void]$fails.Add("cite shape: $k = $got，登記 $want") }
    }
    foreach ($k in $UNGUARDED_SHAPES) {
      if ($seenShapes.ContainsKey($k)) {
        Write-Host ("  shape {0,-24} {1,5}   ⓘ 刻意不設登記值（±25 行視窗，任何註解編輯都會讓列在兩者間移動）" -f $k, $seenShapes[$k]) -ForegroundColor DarkGray
      }
    }
  }
  Remove-Item $shapeTmp -ErrorAction SilentlyContinue
} else {
  Write-Host ""
  Write-Host "=== cite_check 規模不變量：已用 -SkipSlow 跳過 —— 這是刻意略過，不是通過 ===" -ForegroundColor DarkYellow
}

Write-Host ""
Write-Host "=== 工作樹衛生 ==="
$git = Join-Path $env:LOCALAPPDATA 'Programs\git-mingit\cmd\git.exe'
if (Test-Path $git) {
  Push-Location $Tree
  $st = @(& $git status --porcelain)
  Pop-Location
  $src = @($st | Where-Object { $_ -match '\.(cpp|h|hpp|c)\s*$' })
  Write-Host ("  變更檔 {0}；其中 .cpp/.h {1}" -f $st.Count, $src.Count)
  if ($src.Count -gt 0) {
    Write-Host "  ⓘ 有原始碼變更 -> 鐵則 6 的兩組建置是**另一項義務**，本 runner 不做" -ForegroundColor DarkYellow
  }
  # AI(W906-GL-0z) 20260827: `py` added.  It was missing, so a changed Python file was
  # silently skipped -- and Python is the ONE language here where a BOM is a hard syntax
  # error, not cosmetic: GL-0y's launcher was rejected outright with
  # `SyntaxError: invalid non-printable character U+FEFF` because PowerShell 5.1's
  # `Out-File -Encoding utf8` writes one.  So the file type where a BOM is fatal was the
  # only type the BOM check did not look at.  (These files became runnable on this box only
  # in GL-0y, via GDB's embedded Python 3.9.7 -- before that the omission cost nothing.)
  foreach ($line in $st) {
    if ($line -notmatch '\.(md|ps1|txt|py|cpp|h|hpp|c)\s*$') { continue }
    $rel = ($line.Substring(3)).Trim('"')
    $fp = Join-Path $Tree $rel
    if (-not (Test-Path -LiteralPath $fp)) { continue }
    $bytes = [System.IO.File]::ReadAllBytes($fp)
    $crlf = 0; $lf = 0
    for ($i = 1; $i -lt $bytes.Length; $i++) {
      if ($bytes[$i] -eq 10) { if ($bytes[$i - 1] -eq 13) { $crlf++ } else { $lf++ } }
    }
    $bom = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)

    # ⚠⚠ AI(W906-GL-4w) 20260830: COMPARE THE EOL COMPOSITION AGAINST `git show HEAD:`.
    # THE HOLE THIS CLOSES, paid for in GL-4v: tools\edit_verified.ps1 wrote hard-coded LF, so a
    # TWO-LINE comment edit in cShowBinSelect.cpp (a CRLF file) rewrote all 2,893 line endings --
    # `git diff --numstat` said 2893 2893.  The check below did NOT fire, because the file was
    # UNIFORM before (all CRLF) and UNIFORM after (all LF): "mixed endings" never became true.
    # ⚠ And nothing else saw it either: the helper's read-back uses ReadAllLines and criterion A'
    # uses Get-Content, BOTH OF WHICH DISCARD LINE ENDINGS.  Only numstat spoke, and only because a
    # human looked.
    # ⚠ THIS ALSO REMOVES A LATENT FALSE POSITIVE: the tree has NINE legitimately mixed .cpp/.h
    # files (measured 20260830 over 1,185: CRLF-only 74 / LF-only 1,102 / mixed 9).  The old rule
    # would have failed on any edit to one of those for a condition that PREDATES the edit.
    # Comparing against HEAD asks the right question -- "did THIS edit change the endings" -- so a
    # pre-existing mix passes while a newly introduced one still fails.
    # ⚠ A NEW file has no HEAD version to compare with, so for those the uniformity rule still
    # applies: a file this campaign is adding has no excuse to be mixed.
    $headBytes = $null
    Push-Location $Tree
    try {
      $tmpH = Join-Path $env:TEMP ("wwg_head_" + $PID + "_" + ([IO.Path]::GetFileName($rel)))
      & cmd /c "`"$git`" show HEAD:`"$($rel -replace '\\','/')`" > `"$tmpH`" 2>nul"
      if ((Test-Path -LiteralPath $tmpH) -and ((Get-Item -LiteralPath $tmpH).Length -gt 0)) {
        $headBytes = [System.IO.File]::ReadAllBytes($tmpH)
      }
      Remove-Item -LiteralPath $tmpH -Force -ErrorAction SilentlyContinue
    } finally { Pop-Location }

    $eolChanged = $false; $hCrlf = -1; $hLf = -1
    if ($null -ne $headBytes) {
      $hCrlf = 0; $hLf = 0
      for ($i = 1; $i -lt $headBytes.Length; $i++) {
        if ($headBytes[$i] -eq 10) { if ($headBytes[$i - 1] -eq 13) { $hCrlf++ } else { $hLf++ } }
      }
      # ⚠ Compare the SHAPE (which kinds are present), not the counts -- adding or deleting lines
      # legitimately changes the counts, and this check is about the KIND of ending, not how many.
      $wasCrlf = ($hCrlf -gt 0); $wasLf = ($hLf -gt 0)
      $isCrlf  = ($crlf  -gt 0); $isLf  = ($lf  -gt 0)
      $eolChanged = (($wasCrlf -ne $isCrlf) -or ($wasLf -ne $isLf))
    }

    if ($eolChanged) {
      [void]$fails.Add("行尾組成被改了: $rel (HEAD CRLF=$hCrlf/LF=$hLf -> 現在 CRLF=$crlf/LF=$lf) —— 兩行的編輯不該重寫整檔行尾，見 DEVLOG CLVIII")
      Write-Host ("  ✗ {0,-44} 行尾組成改變 HEAD(CRLF={1},LF={2}) -> 現在(CRLF={3},LF={4})" -f $rel, $hCrlf, $hLf, $crlf, $lf) -ForegroundColor Red
    } elseif ($null -eq $headBytes -and $crlf -gt 0 -and $lf -gt 0) {
      [void]$fails.Add("新檔行尾混雜: $rel (CRLF=$crlf, LF=$lf)")
      Write-Host ("  ✗ {0,-44} CRLF={1} 裸LF={2}  新檔不該行尾混雜" -f $rel, $crlf, $lf) -ForegroundColor Red
    } elseif ($bom -and $rel -match '\.py$') {
      # A BOM is FATAL for Python and merely cosmetic elsewhere, so it fails only here.
      # Not a blanket rule on purpose: TRANSFER_NOTES.txt is legitimately CRLF *with* BOM,
      # and failing that would be a false alarm on an author-written delivery document.
      [void]$fails.Add("Python 檔帶 BOM（在這台會直接 SyntaxError）: $rel")
      Write-Host ("  ✗ {0,-44} CRLF={1,-5} 裸LF={2,-6} BOM=True  Python 帶 BOM 會 SyntaxError" -f $rel, $crlf, $lf) -ForegroundColor Red
    } else {
      $eolNote = if ($null -eq $headBytes) { '（新檔）' } else { '行尾與 HEAD 相同' }
      Write-Host ("  {0,-44} CRLF={1,-5} 裸LF={2,-6} BOM={3}  {4}" -f $rel, $crlf, $lf, $bom, $eolNote)
    }
  }
  # AI(W906-GL-2k) 20260828: see Test-IndentLoss above for why this exists.
  if ($src.Count -gt 0) {
    $indentBad = @(Test-IndentLoss $Tree $git)
    if ($indentBad.Count) {
      [void]$fails.Add("縮排掉了（替換行從第 0 欄開始，HEAD 版本有縮排）: $($indentBad -join ', ')")
      Write-Host ("  ✗ 縮排掉了 {0} 行 -> {1}" -f $indentBad.Count, ($indentBad -join ', ')) -ForegroundColor Red
    } else {
      Write-Host "  縮排：變更行皆未掉縮排 ok" -ForegroundColor Green
    }
  }
} else {
  Write-Host "  ⓘ 找不到 git，跳過工作樹衛生" -ForegroundColor DarkYellow
}

Write-Host ""
if ($fails.Count -gt 0) {
  Write-Host ("收工 gate 不通過：{0} 項" -f $fails.Count) -ForegroundColor Red
  $fails | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
  exit 1
}
Write-Host "收工 gate 全部通過。" -ForegroundColor Green
Write-Host "⚠️ 這不包含鐵則 6 的兩組建置，也不包含 ctest。非 oracle 線的數字不可與 BCB6 比較。" -ForegroundColor DarkYellow
exit 0
