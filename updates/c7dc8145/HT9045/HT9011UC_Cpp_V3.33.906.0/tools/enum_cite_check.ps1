# ---------------------------------------------------------------------------
#  enum_cite_check.ps1 -- "when a comment cites a header line FOR AN ENUM, is
#  that actually the line the enumerator (or the enum) lives on?"
#
#  AI(W906-GL-7b) 20260901.
#
#  WHY THIS EXISTS -- A MEASURED 2-OF-3 HIT RATE, NOT A HUNCH
#  ---------------------------------------------------------
#  cite_symbol_check.ps1 exists for exactly this class: the cited line EXISTS,
#  holds real code, and is the WRONG line.  It cannot see enumerators.  Its
#  symbol-carrying test at cite_symbol_check.ps1:330 is
#
#      $seg -cmatch "(?<![A-Za-z0-9_])" + [regex]::Escape($ident) + "\s*[\(;]"
#
#  i.e. the identifier must be followed by `(` or `;`, so it validates FUNCTION
#  declarations and statements.  An enumerator is followed by `=` or `,`, so such
#  citations are SKIPPED as unresolvable -- not judged and not reported.  The gate
#  is silent there by construction; "gate green" means "green within what it can
#  judge".
#
#  Two of the three in-source note errors found while working uHeaterThread.cpp's
#  A..I block were enumerator citations, and BOTH slipped past a green runner:
#    * GL-6t: report defect #18's note cited `MachineType.h:538` for
#      eIndexHeatMode.  golden :538 is `ctScannerCategory=4` -- a DIFFERENT enum.
#      The real enum is at golden :533 / port :565.
#    * GL-6y: note H cited `MachineType.h:637` for tcCCD.  golden :637 is
#      `tcDUT1=29, tcDUT2=30, tcDUT3=31, tcDUT4=32`.  tcCCD=10 is at golden :633.
#  Both are "plausible file, plausible line, real code, wrong line", and both
#  needed a human to open the header.
#
#  ⚠ WHY A SEPARATE TOOL AND NOT A WIDER cite_symbol_check
#  -------------------------------------------------------
#  cite_symbol_check has a REGISTERED invariant (WRONG-LINE = 0) that
#  wave_wrapup_gate asserts on, and its stdout is parsed.  Widening its
#  discriminator would move a number the runner pins and change the surface other
#  code reads -- the tool-stdout-is-an-interface failure.  A separate tool with
#  its own ratchet is the tree's existing pattern.
#
#  ⚠ RESOLVED AGAINST BOTH TREES, BECAUSE THAT MISTAKE IS ALREADY ON RECORD
#  ------------------------------------------------------------------------
#  cite_symbol_check's own header (:23-30) records that its FIRST version
#  reported 92 findings of which about half were its own false positives, because
#  it resolved every citation against the golden tree and never looked at the
#  port tree.  So this tool parses the header in BOTH trees and reports a
#  citation only when it is wrong in EVERY tree where the header exists and the
#  identifier is found.  One tree agreeing is enough to stay silent.
#
#  ⚠ BIASED TOWARD FALSE NEGATIVES ON PURPOSE
#  ------------------------------------------
#  A citation is judged ONLY when both of these hold:
#    (a) the cited header contains an enum this tool could parse, and
#    (b) the citing line names an identifier that is either an ENUMERATOR of that
#        enum or the enum's TAG.
#  Anything else is skipped silently.  A missed wrong citation is a gap; a false
#  alarm teaches people to ignore the gate.  Those costs are not symmetric.
#
#  VERDICTS
#    OK-MEMBER   cited line is where that enumerator lives
#    OK-TAG      enum tag cited, and the cited line is inside that enum's body
#    WRONG-LINE  wrong in every tree that could judge it   <- the only red
#    (skipped)   not judged; not counted, not printed unless -ShowSkipped
#
#  EXIT CODES
#    0  WRONG-LINE count equals -Expect (default 0)
#    1  WRONG-LINE count differs from -Expect
#    2  usage error / self-test failure
#
#  USAGE
#    tools\enum_cite_check.ps1
#    tools\enum_cite_check.ps1 -SelfTest
#    tools\enum_cite_check.ps1 -Expect 3 -ShowSkipped
# ---------------------------------------------------------------------------

#  ⚠⚠ REGISTERED VALUE IS 1, AND THE ONE IS AN ITEMISED KNOWN FALSE POSITIVE OF THIS TOOL
#  ------------------------------------------------------------------------------------
#  THE ONE:  aTester_Front.cpp:7247  cites MachineType.h:552 for DropPlaceShiftContact.
#  THE CITATION IS CORRECT AND THE TOOL IS WRONG.  MachineType.h:552 is
#  `DropContact = 1,` and :558 is `DropPlaceShiftContact = 7,`; the comment reads
#      :7246  ... DropContact=1 /
#      :7247  DropPlaceShiftContact=7 MachineType.h:552/:558; VacuumONMode=0
#  i.e. `A / B  Hdr.h:X/:Y` pairs POSITIONALLY -- X belongs to A, Y to B -- and here the two
#  identifiers straddle a line break, so this tool's proximity window hands :552 to
#  DropPlaceShiftContact.  Verified by hand against both trees 20260901.
#  ⚠ IT IS REGISTERED RATHER THAN SUPPRESSED so that a SECOND finding still turns the gate
#  red.  If this number ever needs raising, name the new item here the same way.
#  ⚠ DO NOT EDIT THE COMMENT TO SILENCE THE GATE -- that would make a correct citation wrong.
#  This is the shape GL-7e nearly shipped as a real defect; see docs/DEVLOG.md CCXVIII.
#
#  ⚠ THE POPULATION WAS REPAIRED, NOT ABSORBED
#  AI(W906-GL-7e) 20260901: all 32 were fixed (18 files, 33 lines).  The cause was the one
#  GL-7c named: citations carried 20260825 PRE-IMPORT port line numbers.  Offsets are NOT
#  uniform (mostly +27, but e9045_1x4_4_13 was 1014->1050), so each was relocated by finding
#  its enumerator, never by adding a constant.
#  ⚠⚠ AND A SIXTH FALSE-POSITIVE CLASS SURFACED DURING THE REPAIR, THE ONLY ONE THAT MADE ME
#  CREATE A DEFECT: A / B  Hdr.h:X/:Y pairs POSITIONALLY, not by proximity.  At
#  aTester_Front.cpp:7246-7247 the two identifiers straddle a line break
#  (... DropContact=1 / then DropPlaceShiftContact=7 MachineType.h:525/:531), so :525 was
#  DropContact's line and the proximity window handed it DropPlaceShiftContact.  Following the
#  tool changed a CORRECT citation to a wrong one.  Caught by auditing all 32 for
#  multi-number/multi-identifier shapes afterwards (6 hits, 5 harmless, 1 real).
#  ⚠ THE TOOL IS NOT FIXED FOR THIS CLASS -- positional pairing in free prose is exactly the
#  fragile-heuristic-over-ambiguous-prose trap.  Instead: when a finding's line carries more
#  than one line number, READ IT before repairing.  That check is in the RESUME.
#  ⚠ Companion BARE line numbers (the :Y half) are structurally invisible here -- this tool
#  only matches file:line.  Five were stale from the same event and were fixed by hand:
#  VacuumONMode :537->:564, _8Site1X4 :460->:487, rsmContinuStart :603->:630,
#  _16Site4X4 :457->:484, eATCUninstall :690->:717.  Verified NOT stale and left alone:
#  CC_TSMC_TAINAN (MachineType.h:186) and CC_AMKOR_Korea (:347) -- the insertions were below them.
#
#  ⚠ HISTORICAL: the value was 36, then 32 (GL-7d range support), then 0 (GL-7e repair).
#  While it stood at 36/32 it was A POPULATION, NOT A VERDICT
#  (GL-7d 20260901: 36 -> 32.  Range-citation support removed FOUR false positives --
#  this tree writes Hdr.h:A-B and the first version matched only A.  atester_shims.h:274
#  cites golden adam6024.h:47-50 with eEPSwArm1 really at golden :48, and
#  acatchtray_shims.h:449 cites golden AGV.h:215-220 with eAtkTfPutEmptyTray at :217 --
#  both correct, both had been flagged.  ⚠ WebBridgeTags.h:129 correctly REMAINS: it cites
#  MachineType.h:637-641 for tcHeatGun1, which is the PRE-IMPORT range; port-now has it at
#  :668, outside that range.  That was the FIFTH false-positive class in this tool; every
#  one of the five was found by opening the cited file, none by re-reading the code.)
#  --------------------------------------------------------------------
#  GL-7b measured 36 WRONG-LINE findings on first clean run.  SIX were hand-checked
#  against both trees; FOUR were real and TWO exposed false-positive classes that are
#  now fixed and locked by -SelfTest.  THE OTHER 30 ARE NOT INDIVIDUALLY VERIFIED.
#  Registering the number stops the debt drifting silently (the stub_shadow_audit = 44
#  precedent); it does NOT assert that 36 independent mistakes exist.
#
#  ⚠⚠ ROOT CAUSE FOUND (GL-7c 20260901) -- IT IS ONE EVENT, NOT 36 MISTAKES.
#  15 of 18 sampled citations resolve EXACTLY against
#      d:\HT9045\HT9011UC_Cpp_V3.33.906.0_noBuild\MachineType.h   (1,616 lines)
#  i.e. the PORT TREE AS IT STOOD BEFORE THE 20260825 LAPTOP IMPORT.  Today's port
#  MachineType.h is 1,643 lines; the import added 27 lines, and citations written
#  before it were never updated.  Measured, identifier by identifier:
#      identifier         cited   pre-import   port-now   golden
#      tCIDNotUse           589          589        616      584
#      ContactModeTotal     535          535        562      530
#      _8Site2X4            452          452        479      447
#      HeadOnly             538          538        565      533
#      tcHotPlate1          637          637        664      632
#      eTrayMap             910          910        937      905
#      InArm                682          682        709      677
#      rsmFIFOMode          615          615        642      610
#      ... 15 of 18 exact against pre-import; residual 3 are eISABase (717 vs 718),
#      e9045_2x4_8 (1014 vs 1036) and tcHeatGun1 (637 vs 641) -- separate small errors.
#  ⚠ THE REPAIR IS PER-CITATION, NOT "+27": the import inserted lines at specific
#  places, so the offset is not uniform.  This tool already reports where each
#  enumerator lives TODAY -- that reported line IS the repair value.
#  ⚠⚠ AND A CORRECTION TO THIS FILE'S OWN EARLIER NOTE: the WHY-PROXIMITY section
#  says a near-constant +27 "looked like a systematic shift and was NOT".  That was
#  HALF WRONG.  The +27 is real -- it is this file's growth at the import.  What was
#  artefact was only the particular pairs hand-checked there, where a MIS-PAIRED
#  identifier happened to sit ~27 lines away.  Two true things were layered and I
#  dismissed both on the strength of two samples.  The lesson is not "distrust
#  constant offsets"; it is "a sample that refutes one explanation does not refute
#  the pattern".
#
#  ⚠ HISTORICAL NOTE, now superseded by the paragraph above.  Five findings are off by EXACTLY
#  -5 against today's golden, across five DIFFERENT enums:
#      ContactModeTotal   cited :535  golden :530     (tests\test_globals.cpp:43)
#      eTrayMap           cited :910  golden :905     (ainarm2.cpp:2600)
#      eartUninstall      cited :908  golden :903     (ainarm9045.cpp:4126)
#      bcTotal            cited :933  golden :928     (BarcodeReader.cpp:10)
#      bcTemperature      cited :921  golden :916     (MyTempPanel.cpp:847)
#  A constant offset across unrelated enums looks like ONE event -- e.g. a batch of
#  citations recorded against a golden snapshot 5 lines shorter -- not five slips.
#  ⚠ DO NOT "FIX" 36 CITATIONS ONE BY ONE BEFORE MEASURING THAT.  GL-7b already burned
#  one theory built on a constant offset: an earlier run showed a near-constant +27 that
#  turned out to be where MIS-PAIRED identifiers happened to live, and two hand-checks
#  killed it.  Measure the cause first; the queue item is in docs/DEVLOG.md's RESUME.
#
#  ⚠ 'wrong in: golden' / 'wrong in: port' (only one tree named) means the OTHER tree
#  could not judge it at all -- usually because the cited line is not inside an enum
#  body there.  Those are weaker findings than 'port+golden' and should be triaged first.

[CmdletBinding()]
param(
    [int]    $Expect      = 1,
    [switch] $ShowSkipped,
    [switch] $SelfTest
)

$ErrorActionPreference = 'Continue'

$PORT_ROOT   = Split-Path -Parent $PSScriptRoot
$GOLDEN_ROOT = Join-Path (Split-Path -Parent $PORT_ROOT) 'HT9011UC_Code_V3.33.906.0_20260618'

# ---------------------------------------------------------------------------
#  WHY-PROXIMITY -- the false-positive trap this tool fell into once
#
#  The first live run reported 45 WRONG-LINE findings and MOST WERE ITS OWN
#  FALSE POSITIVES, for one reason: it collected candidate identifiers from the
#  ENTIRE citing line.  This tree's comments routinely name several identifiers
#  and several line numbers in one sentence, so any citation could be paired
#  with an unrelated name.  Two hand-checked examples:
#
#    ckernel.cpp:385 says
#      "(golden MachineType.h:533 / port MachineType.h:565 -- this cited
#       MachineType.h:538, which is wrong in BOTH trees; golden :538 is
#       `ctScannerCategory=4`, an unrelated enum ...)"
#    BOTH citations there are CORRECT -- the line merely MENTIONS
#    ctScannerCategory as the thing living at the WRONG line.  The tool paired
#    the correct citations with ctScannerCategory and called them wrong.
#
#    uHeaterThread.cpp:222 cites MachineType.h:632 for tcHotPlate1 and, earlier
#    in the same very long line, mentions tcLBUp=69.  The tool paired the
#    citation with tcLBUp.
#
#  ⚠ A near-constant +27 offset across the findings looked like a systematic
#  shift and was NOT: it is where the MIS-PAIRED identifier happened to live.
#  Two hand-checks killed a theory that a summary statistic had made look solid.
#
#  So candidates now come only from a window around THIS citation.  That keeps
#  the promise made in the header -- bias toward false negatives -- which the
#  first version stated and then broke.  It is also the GL-4r lesson applied:
#  rather than build a fragile heuristic to understand ambiguous prose, judge
#  only the prose that is already unambiguous, and stay silent on the rest.
# ---------------------------------------------------------------------------
$ID_WINDOW_BEFORE = 60
$ID_WINDOW_AFTER  = 24

# ---------------------------------------------------------------------------
#  Read a source file, decoding golden as cp950 and the port as UTF-8.
#  ⚠ golden is Big5: 186 of its 194 .cpp files are not valid UTF-8 (CLAUDE.md).
#  Decoding it as UTF-8 mangles Chinese comments, and a mangled line can still
#  parse -- silent wrong answers, which is the whole thing this file avoids.
# ---------------------------------------------------------------------------
function Read-SourceLines {
    param([string] $Path, [switch] $Big5)
    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    if ($Big5) { $enc = [System.Text.Encoding]::GetEncoding(950) }
    else       { $enc = New-Object System.Text.UTF8Encoding($false) }
    return [System.IO.File]::ReadAllLines($Path, $enc)
}

# Strip // and /* */ comment text from one line, so an enumerator NAMED inside a
# comment is never mistaken for a declaration.
function Remove-CommentText {
    param([string] $Text)
    $s = [regex]::Replace($Text, '/\*.*?\*/', ' ')
    $i = $s.IndexOf('//')
    if ($i -ge 0) { $s = $s.Substring(0, $i) }
    return $s
}

# ---------------------------------------------------------------------------
#  Parse every enum in a set of lines.
#  Returns an array of hashtables: Tag, Start, End, Members (name -> line).
#
#  Handles the shapes this tree actually uses -- MachineType.h:632-648 is
#  `enum eTempControll{a=0, b=1, ...` with MANY MEMBERS PER LINE continuing over
#  17 lines, which is why per-line-per-comma attribution is required rather than
#  "one member per line".
# ---------------------------------------------------------------------------
function Get-EnumTable {
    param([string[]] $Lines)
    $out = @()
    if ($null -eq $Lines) { return $out }

    $i = 0
    while ($i -lt $Lines.Count) {
        $code = Remove-CommentText $Lines[$i]
        $m = [regex]::Match($code, '\benum\s+(?:class\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*(?::[^{]*)?\{')
        if (-not $m.Success) { $i++; continue }

        $tag     = $m.Groups[1].Value
        $start   = $i + 1
        # ⚠⚠ CASE-SENSITIVE ON PURPOSE.  A PowerShell hashtable is case-INSENSITIVE by
        # default and C++ identifiers are not, so `@{}` made `HandlerArm` match the
        # enumerator `HANDLERARM`.  MEASURED false positive: port ATC/ATCSystem.h:358 is
        # `HANDLERARM,` (an enumerator of ATCCommandIndex) and :383 is `int HandlerArm;`
        # (a member of a DIFFERENT class).  aTester_Front.cpp:9310 correctly cites :383 and
        # its own text even says "an int MEMBER of a different class" -- the tool contradicted
        # a comment that was right.  This is why cite_symbol_check.ps1 uses -cmatch.
        $members = New-Object System.Collections.Hashtable ([System.StringComparer]::Ordinal)
        $depth   = 0
        $j       = $i
        $first   = $true

        while ($j -lt $Lines.Count) {
            $c = Remove-CommentText $Lines[$j]
            $body = $c
            if ($first) {
                $b = $c.IndexOf('{')
                if ($b -ge 0) { $body = $c.Substring($b + 1) }
                $first = $false
            }
            # track brace depth on the raw (comment-stripped) line
            $depth += ([regex]::Matches($c, '\{')).Count
            $depth -= ([regex]::Matches($c, '\}')).Count

            # cut the body at a closing brace so `};` trailers are not scanned
            $cb = $body.IndexOf('}')
            if ($cb -ge 0) { $body = $body.Substring(0, $cb) }

            foreach ($piece in ($body -split ',')) {
                $p = $piece.Trim()
                if ($p -eq '') { continue }
                # an enumerator is `name` or `name = <expr>` and nothing else
                $em = [regex]::Match($p, '^([A-Za-z_][A-Za-z0-9_]*)\s*(?:=\s*[^=].*)?$')
                if ($em.Success) {
                    $name = $em.Groups[1].Value
                    # ⚠ A LIST, not a single line.  The first version kept only the
                    # first occurrence and called every other line wrong -- measured
                    # false positive: port ATCSystem.h has HandlerArm at BOTH :358 and
                    # :383, and aTester_Front.cpp:9310 correctly cites :383.  An
                    # enumerator name can legitimately appear in more than one enum.
                    if (-not $members.ContainsKey($name)) { $members[$name] = @() }
                    $members[$name] = @($members[$name]) + ($j + 1)
                }
            }

            if ($depth -le 0 -and -not ($j -eq $i -and $c.IndexOf('}') -lt 0)) {
                if ($depth -le 0) { break }
            }
            $j++
        }

        $out += @{ Tag = $tag; Start = $start; End = ($j + 1); Members = $members }
        $i = $j + 1
    }
    # ⚠ @() so a SINGLE enum still returns an ARRAY.  PowerShell unrolls a one-element
    # array to the bare element, and $tbl[0] on a bare hashtable yields $null --
    # which is how the self-test below came to throw twice while still reporting OK.
    return @($out)
}

# ---------------------------------------------------------------------------
#  Candidate identifiers for ONE citation: only those inside the proximity
#  window around it.  Factored out so the self-test can reach it -- the bug this
#  replaces lived in an inline loop where no test could see it.
# ---------------------------------------------------------------------------
function Get-CitationCandidates {
    param(
        [string] $Text,
        [int]    $CiteIndex,
        [int]    $CiteLength,
        [int]    $Before = 60,
        [int]    $After  = 24
    )
    $wStart = [Math]::Max(0, $CiteIndex - $Before)
    $wEnd   = [Math]::Min($Text.Length, $CiteIndex + $CiteLength + $After)
    $window = $Text.Substring($wStart, $wEnd - $wStart)
    $ids = @()
    foreach ($im in [regex]::Matches($window, '[A-Za-z_][A-Za-z0-9_]*')) { $ids += $im.Value }
    return @($ids | Select-Object -Unique)
}

# ---------------------------------------------------------------------------
#  Judge one citation against one parsed enum table.
#  Returns 'OK-MEMBER' | 'OK-TAG' | 'WRONG-LINE' | 'SKIP', plus the expected
#  line when it can name one.
# ---------------------------------------------------------------------------
function Get-CitationVerdict {
    param(
        [hashtable[]] $EnumTable,
        [string[]]    $Identifiers,
        [int]         $CitedLine,
        [int]         $CitedLineEnd = 0
    )
    if ($CitedLineEnd -lt $CitedLine) { $CitedLineEnd = $CitedLine }
    if ($null -eq $EnumTable -or $EnumTable.Count -eq 0) {
        return @{ Verdict = 'SKIP'; Expected = 0; Which = '' }
    }
    # ⚠⚠ THE CITED LINE MUST ITSELF BE INSIDE AN ENUM BODY.
    # Without this, a citation that legitimately points at a line which USES an
    # enumerator gets judged against the line that DECLARES it.  MEASURED false
    # positive: OCRInsp.cpp:341 cites `golden OCR.h:277`, which really is
    # `AnsiString sOCR_Send[OCR_MAX_CMD];` -- the citation is about that field
    # declaration, and OCR_MAX_CMD (a real enumerator at golden OCR.h:273) merely
    # appears in it as an array bound.  Proximity cannot separate those two cases
    # because the enumerator is genuinely adjacent; the cited line's own ROLE can.
    $insideEnum = $false
    foreach ($e in $EnumTable) {
        if ($CitedLineEnd -ge $e.Start -and $CitedLine -le $e.End) { $insideEnum = $true; break }
    }
    if (-not $insideEnum) {
        return @{ Verdict = 'SKIP'; Expected = 0; Which = '' }
    }
    # (b1) an identifier that is an ENUMERATOR -- strongest signal, decides alone.
    # ⚠ Gather across EVERY enum before deciding.  The first version returned on the
    # first enum that contained the name, so a second enum declaring the same
    # enumerator was never consulted -- and THE SELF-TEST CAUGHT THAT, not a live
    # run.  Measured real case: port ATCSystem.h has HandlerArm at :358 and :383,
    # and aTester_Front.cpp:9310 correctly cites :383.
    foreach ($id in $Identifiers) {
        $where = @()
        foreach ($e in $EnumTable) {
            if ($e.Members.ContainsKey($id)) { $where += @($e.Members[$id]) }
        }
        if ($where.Count -gt 0) {
            $where = @($where | Select-Object -Unique | Sort-Object)
            $inRange = $false
            foreach ($wl in $where) { if ($wl -ge $CitedLine -and $wl -le $CitedLineEnd) { $inRange = $true } }
            if ($inRange) {
                return @{ Verdict = 'OK-MEMBER'; Expected = $CitedLine; Which = $id }
            }
            return @{ Verdict = 'WRONG-LINE'; Expected = ($where -join '/'); Which = $id }
        }
    }
    # (b2) an identifier that is an enum TAG -- the cited line must be in its body.
    # Same gather-then-decide shape as (b1): a tag could in principle appear twice
    # (e.g. the same enum declared in two guarded arms), and returning on the first
    # match would call the second one wrong.
    foreach ($id in $Identifiers) {
        $bodies = @()
        foreach ($e in $EnumTable) {
            if ($e.Tag -ceq $id) { $bodies += ,@($e.Start, $e.End) }   # -ceq: see the CASE-SENSITIVE note in Get-EnumTable
        }
        if ($bodies.Count -gt 0) {
            foreach ($b in $bodies) {
                if ($CitedLineEnd -ge $b[0] -and $CitedLine -le $b[1]) {
                    return @{ Verdict = 'OK-TAG'; Expected = $CitedLine; Which = $id }
                }
            }
            return @{ Verdict = 'WRONG-LINE'; Expected = (($bodies | ForEach-Object { $_[0] }) -join '/'); Which = $id }
        }
    }
    return @{ Verdict = 'SKIP'; Expected = 0; Which = '' }
}

# ---------------------------------------------------------------------------
#  SELF-TEST
# ---------------------------------------------------------------------------
if ($SelfTest) {
    # ⚠⚠ 'Stop' IS LOAD-BEARING.  With $ErrorActionPreference = 'Continue' (the file
    # default), an assertion whose ARGUMENT throws is skipped without touching $fail,
    # so the harness printed 'SELF-TEST OK' while two assertions had thrown
    # ($tblCase[0] was $null -- see the @() fix in Get-EnumTable).  A test harness that
    # cannot fail is the same defect class this whole tools\ directory exists to catch,
    # so it is fixed structurally rather than by remembering to read the output.
    $ErrorActionPreference = 'Stop'
    $fail = 0; $n = 0
    function Assert-Eq { param($Got, $Want, [string] $What)
        $script:n++
        if ("$Got" -ne "$Want") { Write-Host ("  x {0}: got '{1}', want '{2}'" -f $What, $Got, $Want); $script:fail++ }
    }

    # Fixture reproducing MachineType.h's real shape: many members per line,
    # continued over several lines, with a SECOND unrelated enum after it.
    $fx = @(
        '// line 1 filler',                                                        # 1
        'enum eTempControll{tcHotPlate1=0, tcHotPlate2=1, tcShuttle1=2,',          # 2
        'tcHead1=4, tcSocket=8, tcChamber=9, tcCCD=10,',                           # 3
        'tcAa1=11, tcAb1=12,                       //Steven comment here',         # 4
        'tcDUT1=29, tcDUT2=30, tcDUT3=31, tcDUT4=32,',                             # 5
        'tcLBUp=69, tcLBDown=70};',                                                # 6
        '',                                                                        # 7
        'enum eIndexHeatMode{HeadOnly=0, BothSide=1, IndexOnly=2, NoHeat=3};',     # 8
        'enum eScannerCat{ctScannerCategory=4, ctOther=5};'                         # 9  <- golden :538 really is inside ANOTHER enum (report defect #18's own words)
    )
    $tbl = Get-EnumTable -Lines $fx
    Assert-Eq $tbl.Count 3 'three enums parsed (eTempControll, eIndexHeatMode, eScannerCat)'

    $tc = $tbl | Where-Object { $_.Tag -eq 'eTempControll' }
    Assert-Eq (@($tc.Members['tcHotPlate1']) -join '/') 2  'tcHotPlate1 on line 2'
    Assert-Eq (@($tc.Members['tcCCD']) -join '/')       3  'tcCCD on line 3 (many-per-line)'
    Assert-Eq (@($tc.Members['tcAb1']) -join '/')       4  'member on a line with a trailing comment'
    Assert-Eq (@($tc.Members['tcDUT1']) -join '/')      5  'tcDUT1 on line 5'
    Assert-Eq (@($tc.Members['tcLBDown']) -join '/')    6  'last member before };'
    Assert-Eq $tc.Members.ContainsKey('eTempControll') $false 'tag is not a member'

    $ih = $tbl | Where-Object { $_.Tag -eq 'eIndexHeatMode' }
    Assert-Eq (@($ih.Members['HeadOnly']) -join '/') 8 'second enum parsed'
    Assert-Eq $ih.Members.ContainsKey('ctScannerCategory') $false 'const outside enum is not a member'

    # -- THE RED CASES: the two real errors this tool was built for -----------
    # GL-6y / note H: tcCCD cited at the tcDUT1 line (fixture line 5, real 637)
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcCCD') -CitedLine 5
    Assert-Eq $v.Verdict  'WRONG-LINE' 'tcCCD cited at the tcDUT line is WRONG-LINE'
    Assert-Eq $v.Expected 3            'and it names the right line'

    # GL-6t / defect #18: eIndexHeatMode cited at a line outside its body
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('eIndexHeatMode') -CitedLine 9
    Assert-Eq $v.Verdict  'WRONG-LINE' 'enum TAG cited outside its body is WRONG-LINE'
    Assert-Eq $v.Expected 8            'and it names the enum start'

    # -- MULTI-OCCURRENCE: the false positive fixed this wave -----------------
    # ⚠ Measured: port ATCSystem.h has HandlerArm at BOTH :358 and :383, and
    # aTester_Front.cpp:9310 correctly cites :383.  Keeping only the first
    # occurrence made that a false positive.
    $fxDup = @(
        'enum eArmA{HandlerArm=0, OtherArm=1};',
        'enum eArmB{HandlerArm=10, ThirdArm=11};',
        'enum eArmC{FourthArm=20};'
    )
    $tblDup = Get-EnumTable -Lines $fxDup
    Assert-Eq (@((@($tblDup))[0].Members['HandlerArm']) -join '/') '1' 'first enum records line 1'
    $v = Get-CitationVerdict -EnumTable $tblDup -Identifiers @('HandlerArm') -CitedLine 1
    Assert-Eq $v.Verdict 'OK-MEMBER' 'citing the first occurrence is OK'
    $v = Get-CitationVerdict -EnumTable $tblDup -Identifiers @('HandlerArm') -CitedLine 2
    Assert-Eq $v.Verdict 'OK-MEMBER' 'citing the SECOND occurrence is also OK (was a false positive)'
    # line 3 is inside an enum body but is not either HandlerArm occurrence
    $v = Get-CitationVerdict -EnumTable $tblDup -Identifiers @('HandlerArm') -CitedLine 3
    Assert-Eq $v.Verdict 'WRONG-LINE' 'a line matching NEITHER occurrence is still red'
    # and a line OUTSIDE every enum body is skipped, not red (the use-site rule)
    $v = Get-CitationVerdict -EnumTable $tblDup -Identifiers @('HandlerArm') -CitedLine 99
    Assert-Eq $v.Verdict 'SKIP' 'a line outside every enum body is SKIPPED'

    # -- CASE SENSITIVITY: the third false positive, and a PowerShell trap ----
    # ⚠ A default @{} hashtable is case-INSENSITIVE, so `HandlerArm` matched the
    # enumerator `HANDLERARM`.  Measured against port ATC/ATCSystem.h (:358 vs :383).
    $fxCase = @('enum ATCCommandIndex{ATCENABLED=0, HANDLERARM, TESTSTART};')
    $tblCase = Get-EnumTable -Lines $fxCase
    Assert-Eq (@($tblCase))[0].Members.ContainsKey('HANDLERARM') $true  'exact-case enumerator is found'
    Assert-Eq (@($tblCase))[0].Members.ContainsKey('HandlerArm') $false 'DIFFERENT-CASE name must NOT match'
    $v = Get-CitationVerdict -EnumTable $tblCase -Identifiers @('HandlerArm') -CitedLine 383
    Assert-Eq $v.Verdict 'SKIP' 'a differently-cased identifier is skipped, not judged'
    $v = Get-CitationVerdict -EnumTable $tblCase -Identifiers @('atccommandindex') -CitedLine 99
    Assert-Eq $v.Verdict 'SKIP' 'lower-cased TAG must not match either (-ceq)'
    # -- USE-SITE vs DECLARATION: the fourth measured false positive ----------
    # ⚠ OCRInsp.cpp:341 cites golden OCR.h:277 = `AnsiString sOCR_Send[OCR_MAX_CMD];`
    # OCR_MAX_CMD is a real enumerator at :273, but the citation is about the FIELD
    # on :277.  The cited line is not inside an enum body, so it must be SKIPPED.
    $fxUse = @(
        'enum OCRCmd{OCR_A=0,',            # 1
        'OCR_MAX_CMD};',                   # 2
        'int filler;',                     # 3
        'AnsiString sOCR_Send[OCR_MAX_CMD];'  # 4  <- a USE site, not in the enum
    )
    $tblUse = Get-EnumTable -Lines $fxUse
    $v = Get-CitationVerdict -EnumTable $tblUse -Identifiers @('OCR_MAX_CMD') -CitedLine 4
    Assert-Eq $v.Verdict 'SKIP' 'citation to a USE site is skipped, not judged'
    $v = Get-CitationVerdict -EnumTable $tblUse -Identifiers @('OCR_MAX_CMD') -CitedLine 2
    Assert-Eq $v.Verdict 'OK-MEMBER' 'citation to the DECLARATION is still judged'
    $v = Get-CitationVerdict -EnumTable $tblUse -Identifiers @('OCR_MAX_CMD') -CitedLine 1
    Assert-Eq $v.Verdict 'WRONG-LINE' 'wrong line INSIDE the enum body is still red'
    # -- RANGE CITATIONS: the fifth measured false-positive class -------------
    # ⚠ This tree writes `Hdr.h:637-641` and `golden AGV.h:215-220`.  Matching only
    # the range START flagged correct citations: atester_shims.h:274 cites
    # `golden adam6024.h:47-50` and eEPSwArm1 really is at golden :48;
    # acatchtray_shims.h:449 cites `golden AGV.h:215-220` and eAtkTfPutEmptyTray is
    # at :217.  A range passes if the enumerator lies anywhere inside it.
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcCCD') -CitedLine 2 -CitedLineEnd 5
    Assert-Eq $v.Verdict 'OK-MEMBER' 'enumerator INSIDE a cited range is OK'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcCCD') -CitedLine 4 -CitedLineEnd 6
    Assert-Eq $v.Verdict 'WRONG-LINE' 'enumerator OUTSIDE a cited range is still red'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcCCD') -CitedLine 3 -CitedLineEnd 3
    Assert-Eq $v.Verdict 'OK-MEMBER' 'a degenerate range (A==B) behaves like a single line'
    # a range that only OVERLAPS an enum body still gets judged (insideEnum uses the range)
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcDUT1') -CitedLine 5 -CitedLineEnd 9
    Assert-Eq $v.Verdict 'OK-MEMBER' 'range spanning past the enum end still resolves'
    # -- the green cases -----------------------------------------------------
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('tcCCD') -CitedLine 3
    Assert-Eq $v.Verdict 'OK-MEMBER' 'correct enumerator citation'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('eIndexHeatMode') -CitedLine 8
    Assert-Eq $v.Verdict 'OK-TAG' 'enum tag cited inside its body'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('eTempControll') -CitedLine 4
    Assert-Eq $v.Verdict 'OK-TAG' 'tag cited mid-body is fine'

    # -- must SKIP, not guess ------------------------------------------------
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('SomeFunction') -CitedLine 3
    Assert-Eq $v.Verdict 'SKIP' 'unrelated identifier is skipped, not judged'
    $v = Get-CitationVerdict -EnumTable @() -Identifiers @('tcCCD') -CitedLine 3
    Assert-Eq $v.Verdict 'SKIP' 'no parsable enum => skipped'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @() -CitedLine 3
    Assert-Eq $v.Verdict 'SKIP' 'no identifiers => skipped'

    # -- enumerator beats tag when both appear -------------------------------
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers @('eTempControll','tcCCD') -CitedLine 5
    Assert-Eq $v.Verdict  'WRONG-LINE' 'enumerator decides even when the tag also matches'
    Assert-Eq $v.Which    'tcCCD'      'and it reports the enumerator'

    # -- comment stripping ---------------------------------------------------
    Assert-Eq (Remove-CommentText 'tcCCD=10, // tcDUT1=29').Trim() 'tcCCD=10,' 'line comment stripped'
    Assert-Eq (Remove-CommentText 'a=1, /* b=2 */ c=3,').Contains('b=2') $false 'block comment stripped'

    # -- PROXIMITY PAIRING: the regression that produced 45 false positives ---
    # ⚠ These fixtures are the two hand-checked lines from the first live run.
    # Both must now be SKIPPED, not judged.  See WHY-PROXIMITY in the header.
    $fp1 = '//     (golden MachineType.h:533 / port MachineType.h:565 -- this cited "MachineType.h:538", which is wrong in BOTH trees; golden :538 is `ctScannerCategory=4`, an unrelated enum.)'
    $c1  = [regex]::Match($fp1, 'MachineType\.h:533')
    $cand = Get-CitationCandidates -Text $fp1 -CiteIndex $c1.Index -CiteLength $c1.Length
    Assert-Eq ($cand -contains 'ctScannerCategory') $false 'far-away identifier is NOT a candidate'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers $cand -CitedLine 533
    Assert-Eq $v.Verdict 'SKIP' 'ckernel.cpp:385 shape is now SKIPPED, not a false positive'

    $fp2 = '//   registered as report #24: an LB-Up/Down (tcLBUp=69 / tcLBDown=70) undertemperature reports pos, still 0 from :148, i.e. asTempCtrl[0] = Hot Plate 1 (MachineType.h:632).'
    $c2  = [regex]::Match($fp2, 'MachineType\.h:632')
    $cand = Get-CitationCandidates -Text $fp2 -CiteIndex $c2.Index -CiteLength $c2.Length
    Assert-Eq ($cand -contains 'tcLBUp') $false 'identifier 100+ chars away is NOT a candidate'

    # -- and the REAL error must still be caught: the identifier is adjacent ---
    $tp = '//      surrounding code uses the tcCCD enumerator (which happens to BE 10, MachineType.h:637).'
    $c3 = [regex]::Match($tp, 'MachineType\.h:637')
    $cand = Get-CitationCandidates -Text $tp -CiteIndex $c3.Index -CiteLength $c3.Length
    Assert-Eq ($cand -contains 'tcCCD') $true 'nearby identifier IS a candidate'
    $v = Get-CitationVerdict -EnumTable $tbl -Identifiers $cand -CitedLine 5
    Assert-Eq $v.Verdict 'WRONG-LINE' 'note H shape is still caught after the narrowing'

    Write-Host ""
    if ($fail -eq 0) {
        Write-Host ("SELF-TEST OK -- {0} assertions; BOTH real-world red cases (tcCCD/:637 and eIndexHeatMode/:538) are among them." -f $n)
        Write-Host "  Also locked: proximity pairing, multi-occurrence names, and CASE SENSITIVITY -- each was a measured false positive."
        exit 0
    }
    Write-Host ("SELF-TEST FAILED -- {0} of {1} assertions failed." -f $fail, $n)
    exit 2
}

# ---------------------------------------------------------------------------
#  LIVE RUN
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $PORT_ROOT)) {
    Write-Host ("enum_cite_check: port root not found: {0}" -f $PORT_ROOT); exit 2
}

# Cache parsed enum tables per (tree, header).
$cache = @{}
function Get-CachedEnums {
    param([string] $Root, [string] $Header, [switch] $Big5)
    $key = "$Root|$Header"
    if ($cache.ContainsKey($key)) { return $cache[$key] }
    $hits = @(Get-ChildItem -Path $Root -Recurse -Filter $Header -File -ErrorAction SilentlyContinue |
              Where-Object { $_.FullName -notmatch '\\(build[^\\]*|third_party)\\' })
    $tbl = $null
    if ($hits.Count -ge 1) {
        $lines = Read-SourceLines -Path $hits[0].FullName -Big5:$Big5
        $tbl = Get-EnumTable -Lines $lines
    }
    $cache[$key] = $tbl
    return $tbl
}

$srcFiles = @(Get-ChildItem -Path $PORT_ROOT -Recurse -Include *.cpp,*.h -File -ErrorAction SilentlyContinue |
              Where-Object { $_.FullName -notmatch '\\(build[^\\]*|third_party)\\' })

$wrong    = @()
$okCount  = 0
$skipCount= 0
$skipRows = @()

foreach ($f in $srcFiles) {
    $lines = Read-SourceLines -Path $f.FullName
    if ($null -eq $lines) { continue }
    for ($li = 0; $li -lt $lines.Count; $li++) {
        $text = $lines[$li]
        if ($text -notmatch '\.h\s*:\s*\d+') { continue }

        # ⚠ RANGES: this tree writes Hdr.h:637-641 and golden AGV.h:215-220.  The first
        # version matched only the range START and called the citation wrong whenever the
        # enumerator sat elsewhere in the range.  MEASURED false positives:
        # atester_shims.h:274 cites golden adam6024.h:47-50 and eEPSwArm1 really is at
        # golden :48; acatchtray_shims.h:449 cites golden AGV.h:215-220 and
        # eAtkTfPutEmptyTray is at :217.  Both correct, both flagged.  A range now passes if
        # the enumerator lies anywhere inside it.
        foreach ($cm in [regex]::Matches($text, '(?<![A-Za-z0-9_])([A-Za-z_][A-Za-z0-9_./\\-]*\.h)\s*:\s*(\d+)(?:\s*-\s*(\d+))?')) {
            $hdr  = Split-Path $cm.Groups[1].Value -Leaf
            $line = [int]$cm.Groups[2].Value
            $lineEnd = $line
            if ($cm.Groups[3].Success) { $lineEnd = [int]$cm.Groups[3].Value }
            if ($lineEnd -lt $line) { $lineEnd = $line }

            # ⚠⚠ PROXIMITY PAIRING -- see WHY-PROXIMITY in the header.
            # The first version collected identifiers from the WHOLE citing line and
            # produced 45 findings that were mostly its own false positives, because
            # this tree's comments routinely name several identifiers AND several
            # line numbers on one line.  Only identifiers in a small window around
            # THIS citation are candidates; everything else is skipped, not guessed.
            $wStart = [Math]::Max(0, $cm.Index - $ID_WINDOW_BEFORE)
            $wEnd   = [Math]::Min($text.Length, $cm.Index + $cm.Length + $ID_WINDOW_AFTER)
            $window = $text.Substring($wStart, $wEnd - $wStart)
            $ids = @()
            foreach ($im in [regex]::Matches($window, '[A-Za-z_][A-Za-z0-9_]*')) { $ids += $im.Value }
            $ids = @($ids | Select-Object -Unique)

            $verdicts = @()
            foreach ($t in @(@{ Root = $PORT_ROOT; Big5 = $false; Name = 'port' },
                             @{ Root = $GOLDEN_ROOT; Big5 = $true; Name = 'golden' })) {
                if (-not (Test-Path -LiteralPath $t.Root)) { continue }
                $tbl = Get-CachedEnums -Root $t.Root -Header $hdr -Big5:$t.Big5
                if ($null -eq $tbl) { continue }
                $v = Get-CitationVerdict -EnumTable $tbl -Identifiers $ids -CitedLine $line -CitedLineEnd $lineEnd
                if ($v.Verdict -ne 'SKIP') { $verdicts += @{ Tree = $t.Name; V = $v } }
            }

            if ($verdicts.Count -eq 0) {
                $skipCount++
                if ($ShowSkipped) { $skipRows += ("  skip  {0}:{1} -> {2}:{3}" -f $f.Name, ($li+1), $hdr, $line) }
                continue
            }
            # silent if ANY tree says it is right -- the cite_symbol_check lesson
            $anyOk = $false
            foreach ($x in $verdicts) { if ($x.V.Verdict -ne 'WRONG-LINE') { $anyOk = $true } }
            if ($anyOk) { $okCount++; continue }

            $d = $verdicts[0]
            $wrong += @{
                File = $f.FullName.Substring($PORT_ROOT.Length + 1)
                Line = $li + 1
                Hdr  = $hdr
                Cited= $line
                Want = $d.V.Expected
                Which= $d.V.Which
                Trees= (($verdicts | ForEach-Object { $_.Tree }) -join '+')
            }
        }
    }
}

Write-Host "=== enum_cite_check: does a cited header line really carry that enumerator? ==="
Write-Host ("  judged {0}   WRONG-LINE {1}   skipped {2} (not judgeable: no parsable enum, or the citing line names no enumerator/tag)" -f ($okCount + $wrong.Count), $wrong.Count, $skipCount)
if ($ShowSkipped) { foreach ($r in $skipRows) { Write-Host $r } }
Write-Host ""

foreach ($w in $wrong) {
    Write-Host ("x WRONG-LINE  {0}:{1}" -f $w.File, $w.Line)
    Write-Host ("     cites {0}:{1} while talking about '{2}', which lives at {0}:{3}  (wrong in: {4})" -f $w.Hdr, $w.Cited, $w.Which, $w.Want, $w.Trees)
}
if ($wrong.Count -gt 0) { Write-Host "" }

if ($wrong.Count -eq $Expect) {
    Write-Host ("GATE OK -- WRONG-LINE = {0}, matches the registered value." -f $wrong.Count)
    Write-Host "  ⚠ 'skipped' is NOT 'passed': a citation whose line names no enumerator or tag is not judged at all."
    exit 0
}
Write-Host ("GATE FAILED -- WRONG-LINE = {0}, registered value is {1}." -f $wrong.Count, $Expect)
Write-Host "  Fix the citation, or update the registered value ONLY with a per-item reason."
exit 1
