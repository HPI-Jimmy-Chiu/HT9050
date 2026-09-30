# =============================================================================
#  tools/pci1203_readonly_gate.ps1
#  Mechanically enforce that EtherCAT/Pci1203Monitor.cpp stays READ-ONLY.
#
#  AI(W906-1203MON-3) 20260907: new file.
#
#  WHY A TOOL AND NOT A COMMENT
#  Pci1203Monitor.h carries an allowlist of twelve vendor calls and a prose
#  rule: "a monitor that can command motion is not a monitor". Prose does not
#  survive a well-meaning future edit -- and the edit that breaks this will
#  look entirely reasonable ("just reset the alarm so the screen is tidy").
#  On this machine the PCIE-1203 card is PHYSICALLY PRESENT with Status OK, and
#  the monitor's Poll() is called from a display refresh path. So the rule gets
#  a gate.
#
#  WHAT WOULD MAKE THIS GO RED (the question CLAUDE.md says to ask of any gate)
#    1. A forbidden vendor call appears in the monitor TU  -> exit 1
#    2. The monitor TU disappears or is renamed            -> exit 1
#    3. The allowlist in the .h no longer covers the calls
#       actually made in the .cpp                          -> exit 1
#    4. This gate's own detector stops detecting
#       (proved every run by -SelfTest)                    -> exit 1
#
#  Item 4 matters most: a gate whose detector silently stops matching passes
#  forever. -SelfTest feeds the detector a synthetic violating line and FAILS
#  if the detector does not flag it. It runs by default.
#
#  USAGE
#    powershell -NoProfile -ExecutionPolicy Bypass -File tools\pci1203_readonly_gate.ps1
#    ...            -NoSelfTest      skip the detector self-test (not advised)
#  Exit 0 = read-only property holds. Exit 1 = it does not.
# =============================================================================
[CmdletBinding()]
param(
    [switch]$NoSelfTest
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$mon  = Join-Path $repo 'EtherCAT\Pci1203Monitor.cpp'
$hdr  = Join-Path $repo 'EtherCAT\Pci1203Monitor.h'

# -----------------------------------------------------------------------------
#  The detector.
#
#  Matches a vendor entry point that MUTATES. Deliberately shaped to catch the
#  family, not a hand-listed set, so a vendor call nobody has seen yet is
#  caught by default rather than let through by default:
#     Acm_*Move*      Acm_*Jog*     Acm_*Home*    Acm_*Stop*
#     Acm_*Set*       Acm_*Reset*   Acm_*Write*   Acm_Daq*Do*Set*
#     Acm_*Enable*    Acm_*Add*
#
#  ⚠ Acm_GetErrorMessage / Acm_*Get* are NOT matched -- they are observations.
#  ⚠ Acm_DevOpen/DevClose/AxOpen/AxClose are NOT matched: unavoidable, and
#    named explicitly in the header's allowlist as such.
# -----------------------------------------------------------------------------
$forbidden = '\bAcm2?_[A-Za-z0-9_]*(Move|Jog|Home|Stop|Reset|Write|Add)[A-Za-z0-9_]*\s*\(' +
             '|\bAcm2?_(Set|[A-Za-z0-9_]*Set)[A-Za-z0-9_]*\s*\(' +
             '|\bAcm2?_[A-Za-z0-9_]*Enable[A-Za-z0-9_]*\s*\('

function Test-Line([string]$line) {
    # Strip a trailing // comment and any /* */ span before testing: the header
    # and this file's own banner NAME the forbidden calls on purpose, and a
    # gate that cannot tell a prohibition from a violation is useless.
    $code = $line -replace '/\*.*?\*/', '' -replace '//.*$', ''
    return [bool]([regex]::IsMatch($code, $forbidden))
}

$fail = 0

# -----------------------------------------------------------------------------
#  0. Self-test: prove the detector detects, and prove it does not over-detect.
# -----------------------------------------------------------------------------
if (-not $NoSelfTest) {
    $mustFlag = @(
        '    Acm_AxMoveAbs(ax, 0, 100.0);',
        '    r = Acm_AxSetSvOn(ax, 1);',
        '    Acm_DaqDoSetBitEx(dev, 0, 0, 1, 1);',
        '    Acm_AxResetError(ax);',
        '    Acm_DevWriteSDOData(dev, 0, 0, 1, 1, 1, 2, &v);',
        '    Acm_AxMoveHome(ax, 0, 0);',
        '    Acm_DevEnableEvent(dev, evt);',
        # AI(W906-MT-E3a) 20260925: the mutating twins of the two reads added in MT-E3a
        '    Acm_DaqDoSetBytes(dev, 0, n, buf);',
        '    r = Acm_AxMoveTorque(ax, 100, 0);'
    )
    $mustPass = @(
        '    U32 r = Acm_AxGetState(ax, &state);',
        '    Acm_AxGetActualPosition(ax, &actPos);',
        '    Acm_DaqDiGetByteEx(dev, 0, 0, i, &b);',
        '    Acm_DevGetSlaveStates(dev, 0, 0, &s);',
        '    Acm_GetErrorMessage(code, buf, len);',
        '    ret = Acm_DevOpen(devNum, &dev);',
        '    Acm_AxClose(&h);',
        '    // Acm_AxMoveAbs is deliberately absent from this file',
        '    //      Acm_AxSetSvOn / Acm_AxMoveHome / Acm_AxResetError',
        # AI(W906-MT-E3a) 20260925: the two reads MT-E3a adds must NOT be flagged
        '    const U32 r = Acm_DaqDiGetBytes(impl_->dev, 0, static_cast<U16>(n),',
        '        const U32 r = Acm_AxGetActTorque(ax, &t);'
    )
    $stFail = 0
    foreach ($l in $mustFlag) {
        if (-not (Test-Line $l)) { Write-Host "SELFTEST FAIL (missed): $l" -ForegroundColor Red; $stFail++ }
    }
    foreach ($l in $mustPass) {
        if (Test-Line $l) { Write-Host "SELFTEST FAIL (false positive): $l" -ForegroundColor Red; $stFail++ }
    }
    if ($stFail -gt 0) {
        Write-Host "SELFTEST: $stFail case(s) wrong -- the detector is broken, so its GREEN means nothing." -ForegroundColor Red
        exit 1
    }
    Write-Host ("SELFTEST ok: {0} violating lines flagged, {1} legitimate lines passed" -f $mustFlag.Count, $mustPass.Count) -ForegroundColor DarkGray
}

# -----------------------------------------------------------------------------
#  1/2. The monitor TU must exist and must be clean.
# -----------------------------------------------------------------------------
foreach ($f in @($mon, $hdr)) {
    if (-not (Test-Path $f)) {
        Write-Host "FAIL: missing $f -- the module this gate protects is gone or renamed." -ForegroundColor Red
        $fail++
    }
}
if ($fail -gt 0) { exit 1 }

$hits = @()
$n = 0
foreach ($line in (Get-Content $mon -Encoding UTF8)) {
    $n++
    if (Test-Line $line) { $hits += "  Pci1203Monitor.cpp:${n}: $($line.Trim())" }
}

if ($hits.Count -gt 0) {
    Write-Host "FAIL: mutating vendor call(s) in the READ-ONLY monitor TU:" -ForegroundColor Red
    $hits | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host ""
    Write-Host "A monitor that can command motion is not a monitor. See the ONE RULE" -ForegroundColor Yellow
    Write-Host "banner in EtherCAT/Pci1203Monitor.cpp and the allowlist in its header." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host "Pci1203Monitor.cpp: 0 mutating vendor calls ($n lines scanned)" -ForegroundColor Green
}

# -----------------------------------------------------------------------------
#  3. Every vendor call the .cpp actually makes must be named in the .h allowlist.
# -----------------------------------------------------------------------------
$called = @{}
$n = 0
foreach ($line in (Get-Content $mon -Encoding UTF8)) {
    $n++
    $code = $line -replace '/\*.*?\*/', '' -replace '//.*$', ''
    foreach ($m in [regex]::Matches($code, '\b(Acm2?_[A-Za-z0-9_]+)\s*\(')) {
        $called[$m.Groups[1].Value] = $true
    }
}
# ⚠ SCOPE THE SEARCH TO THE ALLOW BLOCK, NOT THE WHOLE HEADER.
#   Found 20260907 by end-to-end-proving this gate rather than trusting its
#   self-test: with Acm_AxResetError injected into the .cpp, check 1 correctly
#   went red -- but THIS check reported it as "documented in the allowlist",
#   because the header NAMES it, in the list of calls that must stay ABSENT.
#   "Mentioned in the header" is not "permitted by the header". A grep over the
#   whole file cannot tell a permission from a prohibition, so it would hand a
#   false green to any forbidden call that check 1's regex family does not
#   happen to match -- which is precisely the case this check exists to cover.
#   The allow block runs from the THE ALLOWLIST banner to the DELIBERATELY
#   ABSENT banner; everything after that point is a prohibition.
$hdrLines = Get-Content $hdr -Encoding UTF8
$allowStart = ($hdrLines | Select-String -Pattern 'THE ALLOWLIST' | Select-Object -First 1).LineNumber
# ⚠ ANCHOR ON THE BANNER, NOT ON THE PHRASE. AI(W906-1203ALM-9) 20260914.
#   This searched for 'DELIBERATELY ABSENT' anywhere, so a comment that merely
#   MENTIONED the prohibition list ended the allow block early -- and every call
#   documented after that point was reported as undocumented. Measured: adding a
#   legitimate allowlist entry whose prose said "the DELIBERATELY ABSENT list
#   below" turned the gate red on a correct tree.
#   The banner's real wording is "⚠ WHAT IS DELIBERATELY ABSENT, and must stay
#   absent:", so match the distinctive part. A detector one paraphrase away from
#   breaking is the same class of fault this gate exists to catch.
$allowEnd   = ($hdrLines | Select-String -Pattern 'WHAT IS DELIBERATELY ABSENT' | Select-Object -First 1).LineNumber
if (-not $allowEnd) {
    # Fall back to the old, looser anchor rather than silently scanning the
    # whole file -- but SAY SO, because the banner may have been reworded.
    Write-Host "WARN: banner 'WHAT IS DELIBERATELY ABSENT' not found; falling back to the loose phrase." -ForegroundColor Yellow
    $allowEnd = ($hdrLines | Select-String -Pattern 'DELIBERATELY ABSENT' | Select-Object -First 1).LineNumber
}
if (-not $allowStart -or -not $allowEnd -or $allowEnd -le $allowStart) {
    Write-Host "FAIL: cannot locate the allow block in Pci1203Monitor.h (looked for 'THE ALLOWLIST' .. 'DELIBERATELY ABSENT')." -ForegroundColor Red
    Write-Host "      Without a delimited allow block this check would silently degrade to a whole-file grep, which cannot tell a permission from a prohibition." -ForegroundColor Yellow
    exit 1
}
$hdrText = ($hdrLines[($allowStart)..($allowEnd - 2)]) -join "`n"
$unlisted = @()
foreach ($k in ($called.Keys | Sort-Object)) {
    if ($hdrText -notmatch [regex]::Escape($k)) { $unlisted += $k }
}
if ($unlisted.Count -gt 0) {
    Write-Host "FAIL: vendor call(s) made by the .cpp but NOT in the .h allowlist:" -ForegroundColor Red
    $unlisted | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    Write-Host "The allowlist is the reviewable surface. Add it there, with a reason, or drop the call." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("allowlist: all {0} vendor call(s) used by the .cpp are documented in the .h" -f $called.Count) -ForegroundColor Green
    ($called.Keys | Sort-Object) | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
}

# -----------------------------------------------------------------------------
#  4. OWNERSHIP GUARD.  AI(W906-1203MON-9) 20260907.
#
#  ATTACHED mode (1203MON-8) means `impl_->dev` can be PRODUCTION's uiDevhand.
#  An unguarded Acm_DevClose on it closes the machine's own motion card -- mid
#  lot, mid motion, with nothing anywhere pointing back at this file.
#
#  WHY THIS IS A GATE AND NOT A TEST: the dangerous path only exists once
#  production has actually opened the card, which needs Gerneral.ini's
#  INSTALL_ETHETCAT keys AND a physical card in a machine. It cannot be
#  exercised on a desk, and "cannot be tested" is the exact case where a
#  STRUCTURAL invariant earns its keep -- the shape of the code is checkable
#  today even though its behaviour is not.
#
#  The invariant: every Acm_DevClose in the monitor TU must have an `ownsDev`
#  test within the few lines above it. Deliberately shape-based, not
#  flow-analysis: a cheap check that cannot be quietly satisfied is worth more
#  than an accurate one nobody maintains.
$monLines = Get-Content $mon -Encoding UTF8
$closeRows = @()
for ($i = 0; $i -lt $monLines.Count; $i++) {
    $code = $monLines[$i] -replace '/\*.*?\*/', '' -replace '//.*$', ''
    if ($code -match '\bAcm_DevClose\s*\(') { $closeRows += $i }
}
if ($closeRows.Count -eq 0) {
    # Not a pass. Either the close was dropped (leaking a device handle that
    # then blocks the NEXT open, production's included) or the file moved.
    Write-Host "FAIL: no Acm_DevClose found in the monitor TU at all." -ForegroundColor Red
    Write-Host "      Owned mode must still release its own handle -- a leaked device handle is what makes the next open fail." -ForegroundColor Yellow
    $fail++
} else {
    $unguarded = @()
    foreach ($r in $closeRows) {
        $from = [Math]::Max(0, $r - 6)
        $window = ($monLines[$from..$r]) -join "`n"
        if ($window -notmatch '\bownsDev\b') { $unguarded += "  Pci1203Monitor.cpp:$($r+1): $($monLines[$r].Trim())" }
    }
    if ($unguarded.Count -gt 0) {
        Write-Host "FAIL: Acm_DevClose with no 'ownsDev' test within 6 lines above:" -ForegroundColor Red
        $unguarded | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        Write-Host "In ATTACHED mode the device handle belongs to PRODUCTION. Closing it takes the" -ForegroundColor Yellow
        Write-Host "machine's motion card down. See the TWO MODES section in Pci1203Monitor.h." -ForegroundColor Yellow
        $fail++
    } else {
        Write-Host ("ownership: all {0} Acm_DevClose call(s) are ownsDev-guarded" -f $closeRows.Count) -ForegroundColor Green
    }
}

# Self-test for check 4: prove the window matcher both catches and clears.
if (-not $NoSelfTest) {
    $bad  = @('    HAND h = impl_->dev;', '    Acm_DevClose(&h);')
    $good = @('    if (impl_->dev != 0 && impl_->ownsDev) {', '        HAND h = impl_->dev;', '        Acm_DevClose(&h);')
    $badWin  = ($bad  -join "`n")
    $goodWin = ($good -join "`n")
    $stFail = 0
    if ($badWin  -match '\bownsDev\b') { Write-Host "SELFTEST FAIL: unguarded close window read as guarded" -ForegroundColor Red; $stFail++ }
    if ($goodWin -notmatch '\bownsDev\b') { Write-Host "SELFTEST FAIL: guarded close window read as unguarded" -ForegroundColor Red; $stFail++ }
    if ($stFail -gt 0) { exit 1 }
    Write-Host "SELFTEST ok: ownership window matcher flags unguarded and clears guarded" -ForegroundColor DarkGray
}

if ($fail -gt 0) { exit 1 }
Write-Host "RESULT: PASS -- the 1203 monitor is read-only and its device close is ownership-guarded." -ForegroundColor Green
exit 0
