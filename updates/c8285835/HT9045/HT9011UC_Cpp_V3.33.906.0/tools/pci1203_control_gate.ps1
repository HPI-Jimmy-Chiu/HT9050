# =============================================================================
#  tools/pci1203_control_gate.ps1
#  Mechanically enforce the WRITE surface's allowlist -- the sibling of
#  tools/pci1203_readonly_gate.ps1, for the one file that is ALLOWED to command
#  the PCIE-1203.
#
#  AI(W906-1203CTL-8) 20260911: new file. Cited by EtherCAT/Pci1203Control.h and
#  by CMakeLists.txt since 20260911; this is that file.
#
#  WHY THIS GATE CANNOT BE A COPY OF THE READ-ONLY ONE
#  ---------------------------------------------------
#  The monitor's gate asks "are there any mutating calls?" and the answer must
#  be zero. Here writing is the POINT, so that question has no useful answer.
#  This gate asks three narrower ones instead:
#
#     1. Is every vendor call the .cpp makes named in the .h allowlist?
#        (same check as the monitor's, and it is the reviewable surface)
#     2. Are the calls DELIBERATELY EXCLUDED still absent?
#        Position redefinition, fieldbus reconfiguration, firmware download.
#        These are the ones that look harmless and are not.
#     3. Is the write surface still OPT-IN, and still DRY BY DEFAULT?
#        Structural checks on MachineType.h and on the module itself.
#
#  And the fourth, which matters most:
#
#     4. Does the detector still detect? -SelfTest feeds it synthetic
#        violating AND legitimate lines every run and fails if either is
#        judged wrong. A gate whose matcher silently stops matching passes
#        forever -- CLAUDE.md's "不可能當掉的 gate 不是 gate".
#
#  WHAT WOULD MAKE THIS GO RED
#    * a vendor call appears in Pci1203Control.cpp that the header does not list
#    * any of the forbidden-by-name calls appears anywhere in the module
#    * the module or its header is deleted or renamed
#    * WB_PUMP_1203_CONTROL becomes enabled by default in MachineType.h
#    * WB_PUMP_1203_CONTROL_LIVE becomes enabled by default
#    * TPci1203Control::Open stops defaulting to dry, or the dry branch stops
#      returning before the vendor switch
#    * the detector's self-test disagrees with itself
#
#  USAGE
#    powershell -NoProfile -ExecutionPolicy Bypass -File tools\pci1203_control_gate.ps1
#    ...            -NoSelfTest      skip the detector self-test (not advised)
#  Exit 0 = the write surface is exactly what its header says. Exit 1 = it is not.
# =============================================================================
[CmdletBinding()]
param(
    [switch]$NoSelfTest
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$src  = Join-Path $repo 'EtherCAT\Pci1203Control.cpp'
$hdr  = Join-Path $repo 'EtherCAT\Pci1203Control.h'
$mt   = Join-Path $repo 'MachineType.h'

$fail = 0

function Strip([string]$line) {
    # Drop a trailing // comment and any /* */ span. Both this file's header and
    # the .cpp's banner NAME the forbidden calls on purpose; a gate that cannot
    # tell a prohibition from a violation is useless. Same rule, same reason, as
    # the read-only gate.
    #
    #AI(W906-1203GEAR-1) 20260915: ⚠ AND STRING LITERALS, which is a third case
    #  the first two did not cover. Every command in this module formats an
    #  AUDIT LINE naming the call it is about to make:
    #      "Acm_DevWriteSDOData(station=%d, %04Xh:%u, U32) = %lu   [%s, 軸%s]"
    #  That is a description, not a call -- but it is neither a // comment nor a
    #  /* */ span, so it reached the SDO detector, which dutifully reported the
    #  "index" as a fragment of Chinese text. The audit line exists precisely so
    #  a human can check what was issued; it must not be mistaken for issuing.
    #  ⓘ Stripping strings can only REMOVE matches, never create one, so this
    #  cannot hide a real violation: code inside a string literal does not run.
    return ($line -replace '/\*.*?\*/', '' `
                  -replace '//.*$', '' `
                  -replace '"(\\.|[^"\\])*"', '""')
}

# -----------------------------------------------------------------------------
#  The FORBIDDEN family. Named individually rather than by pattern, because
#  unlike the monitor's gate a pattern would match the very calls this module
#  exists to make (Acm_AxMoveAbs is forbidden THERE and required HERE).
#
#  Each one is here because it is dangerous in a way that does not look
#  dangerous -- the reasons are in Pci1203Control.h's "DELIBERATELY ABSENT".
# -----------------------------------------------------------------------------
$forbidden = @(
    # 'Acm_AxSetCmdPosition',    # AI(W906-MT-E1) 20260925: ALLOWED (user EastSun ruling 20260925 「開放，照舊版做法」; internal kind kCmdAxSetCmdPos, no wire name -- Pci1203Control.h allowlist). Was: redefines where the axis thinks it is
    # 'Acm_AxSetActualPosition', # AI(W906-MT-E1) 20260925: ALLOWED, same ruling (kCmdAxSetActPos). Was: the next absolute move goes elsewhere
    'Acm_DevWriteRegData',       # can change a station's ESC address
    'Acm_DevSetSlaveID',         # reassigns SubDevice IDs on a live fieldbus
    'Acm_DevDownLoadMapInfo',    # reconfigures the master's IO mapping
    'Acm_DevLoadMapFile',
    'Acm_DevSaveMapFile',
    'Acm_DevSlaveFwDownload',    # firmware
    'Acm_DevWriteEEPROM',
    # ⚠ Acm_DevWriteSDOData WAS HERE AND IS NOT ANY MORE. AI(W906-1203ALM-5)
    #   20260912, user ruling: the 絕對編碼器重置 (Fn008) button. That service is
    #   CoE object 2710h and there is no other way to reach it.
    #   It is NOT simply unbanned -- check 5 below requires every call site in
    #   the module to address index 0x2710, so the module can perform Fn008 and
    #   still cannot set an arbitrary drive parameter. Deleting a blanket ban
    #   protects less than replacing it with a rule that survives.
    # 'Acm_AxSetExtDrive',       # AI(W906-MT-E1) 20260925: ALLOWED, same ruling (golden jog: ExtDrive 1 + Acm_AxJog, stop + ExtDrive 0; kCmdAxSetExtDrive). Was: hands the axis to an external drive source
    'Acm_DevEnableEvent',
    # ⚠ ADDED 20260911 BY THIS GATE'S OWN SELF-TEST, which flagged
    #   `Acm_DaqDoSetBitEx2` as a line the detector MISSED. The first draft of
    #   this list did not contain the Ex forms at all, on the unstated
    #   assumption that "DO writes are allowed". They are not: the allowlist
    #   permits the FLAT form, which addresses the master's single output image.
    #   The Ex form takes a ring and a SubDevice address and writes into an
    #   arbitrary station -- a strictly wider capability, and on a machine whose
    #   SubDevice IDs are currently CONFLICTING it would energise a coil on a
    #   station nobody named.
    #   ⓘ Listing the Ex names and NOT the base names is what keeps
    #   Acm_DaqDoSetBit itself legal: the matcher requires the "Ex" before the
    #   optional suffix, so `Acm_DaqDoSetBit(` does not match and
    #   `Acm_DaqDoSetBitEx2(` does.
    # ⚠⚠ Acm_DaqDoSetBitEx / SetByteEx WERE HERE AND ARE NOT ANY MORE.
    #   AI(W906-1203RING-1) 20260922, and the reason is that the ban's own
    #   stated rationale was INVERTED by measurement.
    #
    #   It read: "The Ex form takes a ring and a SubDevice address and writes
    #   into an arbitrary station -- a strictly wider capability, and on a
    #   machine whose SubDevice IDs are currently CONFLICTING it would energise
    #   a coil on a [wrong module]". That argument assumes the FLAT form is
    #   correctly aimed. Measured 20260922, it is not:
    #     * the IO map's `Index` field is the RING, and this module ignored it,
    #       so ring-0 and ring-1 bytes were merged under one station number;
    #     * the flat port used was the map entry's ARRAY POSITION, and the array
    #       carries a sentinel entry and an Index-15 entry, so it is not the
    #       card's flat port number at all.
    #   So the flat call was the one aimed by a wrong number, and banning Ex
    #   kept the module on it. An operator could not control an ECx-C32-HON that
    #   Common Motion Utility drives fine -- which is how this was found.
    #
    #   ⓘ The conflict the ban worried about is bounded here rather than gone:
    #   (ring 1, station 1) is shared by a junction and the 32DO, and a junction
    #   has no output process data, so exactly one device there has coils. The
    #   map agrees -- one ring-1 run for 0x001, four bytes, which is 32 channels.
    #   Verified: Acm_DaqDoGetByteEx(ring 1, station 1, port 0..3) answers
    #   SUCCESS on exactly four ports.
    #
    #   ★ NOT SIMPLY UNBANNED, exactly like Acm_DevWriteSDOData above: check 5c
    #   below requires every Ex write site to take its ring and station from the
    #   MONITOR'S OWN MAP ATTRIBUTION (d.ring / d.station) and never from
    #   anything an operator typed. The module can address the module the card
    #   says owns the byte, and still cannot be pointed at an arbitrary station.
    #   ⚠ The Ex2 forms stay banned. $forbiddenRe appends [A-Za-z0-9_]* to every
    #   entry, so removing the base name would have unbanned Ex2 with it -- the
    #   very case this gate's own self-test was written to catch.
    'Acm_DaqDoSetBitEx2',
    'Acm_DaqDoSetByteEx2',
    'Acm_DaqDoSetBytes',         # multi-port block write; not on the allowlist
    'Acm_DaqAoSetCurrData'
)
$forbiddenRe = '\b(' + (($forbidden | ForEach-Object { [regex]::Escape($_) }) -join '|') + ')[A-Za-z0-9_]*\s*\('

function Test-Forbidden([string]$line) {
    return [bool]([regex]::IsMatch((Strip $line), $forbiddenRe))
}

# -----------------------------------------------------------------------------
#  0. Self-test.
# -----------------------------------------------------------------------------
if (-not $NoSelfTest) {
    $mustFlag = @(
        '    Acm_DevWriteEEPROM(dev, 0, 0, 0);',   # AI(W906-MT-E1) 20260925: was Acm_AxSetCmdPosition, which is allowed now
        '    r = Acm_DevSetSlaveID(dev, 0xFFFE, pos, pos + 1);',
        '    Acm_DevWriteRegData(dev, 0, addr, 0x0010, 4, &v);',
        '    Acm_DaqDoSetBitEx2(dev, 0, 0, 1, 1);'   # Ex/2 suffixes must not escape
    )
    $mustPass = @(
        '    ret = Acm_AxMoveAbs(ax, (F64)c.value);',
        '    ret = Acm_DaqDoSetBit(dev, (U16)(c.port * 8 + c.bit), (U8)1);',
        '    ret = Acm_AxSetSvOn(ax, (U32)1);',
        '    ret = Acm_SetF64Property(ax, SpeedPropId(c.speed), (F64)c.value);',
        '    // Acm_AxSetCmdPosition / Acm_AxSetActualPosition are absent on purpose',
        '//      Acm_DevWriteRegData / Acm_DevSetSlaveID / Acm_DevDownLoadMapInfo /'
    )
    $stFail = 0
    foreach ($l in $mustFlag) {
        if (-not (Test-Forbidden $l)) { Write-Host "SELFTEST FAIL (missed): $l" -ForegroundColor Red; $stFail++ }
    }
    foreach ($l in $mustPass) {
        if (Test-Forbidden $l) { Write-Host "SELFTEST FAIL (false positive): $l" -ForegroundColor Red; $stFail++ }
    }
    if ($stFail -gt 0) {
        Write-Host "SELFTEST: $stFail case(s) wrong -- the detector is broken, so its GREEN means nothing." -ForegroundColor Red
        exit 1
    }
    Write-Host ("SELFTEST ok: {0} forbidden lines flagged, {1} legitimate lines passed" -f $mustFlag.Count, $mustPass.Count) -ForegroundColor DarkGray
}

# -----------------------------------------------------------------------------
#  1. The module must exist.
# -----------------------------------------------------------------------------
foreach ($f in @($src, $hdr, $mt)) {
    if (-not (Test-Path $f)) {
        Write-Host "FAIL: missing $f -- the module this gate protects is gone or renamed." -ForegroundColor Red
        $fail++
    }
}
if ($fail -gt 0) { exit 1 }

$srcLines = Get-Content $src -Encoding UTF8
$hdrLines = Get-Content $hdr -Encoding UTF8

# -----------------------------------------------------------------------------
#  2. No forbidden call anywhere in the module.
# -----------------------------------------------------------------------------
$hits = @()
for ($i = 0; $i -lt $srcLines.Count; $i++) {
    if (Test-Forbidden $srcLines[$i]) { $hits += "  Pci1203Control.cpp:$($i+1): $($srcLines[$i].Trim())" }
}
if ($hits.Count -gt 0) {
    Write-Host "FAIL: deliberately-excluded vendor call(s) in the write surface:" -ForegroundColor Red
    $hits | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "These are the calls that look harmless and are not. See the DELIBERATELY" -ForegroundColor Yellow
    Write-Host "ABSENT block in EtherCAT/Pci1203Control.h before adding any of them." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("forbidden set: 0 of {0} excluded calls present ({1} lines scanned)" -f $forbidden.Count, $srcLines.Count) -ForegroundColor Green
}

# -----------------------------------------------------------------------------
#  3. Every vendor call made must be named in the .h ALLOWLIST block.
#
#  ⚠ SCOPED TO THE ALLOW BLOCK, not the whole header -- the exact trap the
#  read-only gate documents. The header NAMES the forbidden calls too, in the
#  list of things that must stay absent, so a whole-file grep would report a
#  forbidden call as "documented in the allowlist". A permission and a
#  prohibition are not the same sentence.
# -----------------------------------------------------------------------------
$called = @{}
foreach ($line in $srcLines) {
    foreach ($m in [regex]::Matches((Strip $line), '\b(Acm2?_[A-Za-z0-9_]+)\s*\(')) {
        $called[$m.Groups[1].Value] = $true
    }
}
$allowStart = ($hdrLines | Select-String -Pattern 'THE ALLOWLIST' | Select-Object -First 1).LineNumber
$allowEnd   = ($hdrLines | Select-String -Pattern 'DELIBERATELY ABSENT' | Select-Object -First 1).LineNumber
if (-not $allowStart -or -not $allowEnd -or $allowEnd -le $allowStart) {
    Write-Host "FAIL: cannot locate the allow block in Pci1203Control.h (looked for 'THE ALLOWLIST' .. 'DELIBERATELY ABSENT')." -ForegroundColor Red
    Write-Host "      Without a delimited allow block this check degrades to a whole-file grep, which cannot tell a permission from a prohibition." -ForegroundColor Yellow
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
#  4. STILL OPT-IN, and STILL DRY BY DEFAULT.
#
#  This is the check the header's whole safety story rests on, and it is the one
#  a well-meaning edit removes first ("it is annoying to have to define two
#  macros"). Both switches must be COMMENTED OUT in MachineType.h.
#
#  ⚠ The test is on the ACTIVE directive, not on the text appearing somewhere:
#  MachineType.h explains both macros at length, so a naive grep for the name
#  matches the prose. An ACTIVE #define is a line whose first non-space
#  characters are `#define`.
# -----------------------------------------------------------------------------
#  ⚠⚠ AI(W906-1203CTL-16) 20260911, USER RULING: THIS CHECK CHANGED SHAPE, and
#  the change is a narrowing, not a relaxation.
#
#  It used to fail if EITHER macro was active. The user opened the page under
#  F5 and found it empty -- "怎這個畫面都沒有?" -- because the feature was
#  complete and switched off. So WB_PUMP_1203_CONTROL is now ON in the tree.
#
#  What that macro actually arms is DRY RUN: commands are validated, formatted
#  and recorded, and NOTHING is issued. So the thing the old check protected --
#  "a committed tree must not be able to move a machine" -- is protected by the
#  LIVE macro alone, and that one is still enforced exactly as before.
#
#  ⚠ THE PAIR IS NOW ASYMMETRIC AND BOTH HALVES ARE CHECKED POSITIVELY:
#    WB_PUMP_1203_CONTROL       must be ACTIVE   (or the page is empty again)
#    WB_PUMP_1203_CONTROL_LIVE  must be INACTIVE (or a click moves a servo)
#  Checking the first POSITIVELY is what stops the feature being silently
#  switched off again by a merge or a revert -- which is the failure the user
#  actually experienced, and it left no trace anywhere.
# -----------------------------------------------------------------------------
#  AI(W906-BU-C4) 20260918, USER RULING «甲» -- THE LIVE HALF IS NOW
#  EXPECTED TO BE **ACTIVE**, AND THIS TREE CAN MOVE A MACHINE.
#
#  The line above ("LIVE must be INACTIVE ... turn it on for a session, not in
#  a file") is the 1203 author's rule. It is overridden, deliberately, and the
#  override is NOT a relaxation of safety -- it is a different safety model.
#  Recording both, because whoever reads this next has to be able to tell them
#  apart:
#
#    1203 author's model : the committed tree must be incapable of motion;
#                          arming is a per-session act by whoever is standing
#                          at the machine.
#    THIS project's model: the user's 20260918 ruling -- there are exactly TWO
#                          builds, SOFT_SIMULTE or machine-capable, and a third
#                          state ("built from this tree but cannot drive
#                          anything") is the one being ruled out. Arming also
#                          has to live in git, per the standing rule that armed
#                          state must not diverge between machines.
#
#  => BOTH macros are now expected ACTIVE, and LIVE being OFF is a FAIL, because
#    that is the rejected third state -- not because turning it off is unsafe.
#
#  WHAT THIS MEANS IN PRACTICE, stated here rather than buried: on any build
#    with HAVE_PCI1203, ONE CLICK IN A WEB PAGE CAN ENERGISE A COIL AND TURN A
#    SERVO, with no further opt-in. Since the 20260916b package that now
#    includes pci1203.ax.setOtP / setOtN, which write Pn50A/Pn50B -- a click can
#    also DISABLE AN OVER-TRAVEL LIMIT. The gate still says so, loudly, every
#    run -- it just no longer calls it a failure. A gate that is permanently red
#    is a gate nobody reads.
#
#  To reverse: set the hashtable entry below back to $false and comment the
#  #define out in MachineType.h. Both, or the gate and the tree disagree.
$mtLines = Get-Content $mt -Encoding UTF8
#AI(W906-P13-STARTRING) 20260923: WB_PUMP_1203_START_RING 也進期望表。
#  使用者 20260923 裁決「甲：也打開」（盤點 P13）。它放行
#  EtherCAT/Pci1203Monitor.cpp 的一次 Acm_MasStartRing(dev, 0) —— 啟動 EtherCAT
#  循環交換。**只有 ring 0（motion）**；ring 1 是 DI/DO 與 ECAT-2515，也就是帶線圈
#  的那一環，20260912 特地把迴圈從 ring<=1 收窄成只打 0。啟動 ring 1 是另一個決定。
#
#  為什麼要進這張表：這張表的職責就是「武裝狀態不可在機器之間、或在樹與 gate
#  之間分岔」。不加的話，有人日後把那個 #define 註解掉，gate 會一聲不響地照樣綠 ——
#  而症狀會出現在同事的機台端（指令回 SUCCESS 但軸沒動），不是在這裡。
#  To reverse: 這裡改回 $false，**同時**把 MachineType.h 的 #define 註解掉。兩邊一起。
$expectActive = @{ 'WB_PUMP_1203_CONTROL'      = $true
                   'WB_PUMP_1203_CONTROL_LIVE' = $true
                   'WB_PUMP_1203_START_RING'   = $true; 'WB_ENGINE_IO_1203' = $true; 'WB_ENGINE_MOTOR_1203' = $false; 'WB_ENGINE_INDEXZ_1203' = $false }   #AI(W906-INDEXZ) 20260930: + the Index Z1 Gali_* -> 1203 route (MachineType.h EOF, EtherCAT/Pci1203GaliRoute.cpp), OFF by default (INBOX 113). To arm: $true here AND uncomment it in MachineType.h -- both   # AI(W906-IOWEB-P17) 20260925: + the engine IO route (MachineType.h EOF), user ruling "請幫我接上1203"   # AI(W906-ECAT-ROUTE) 20260929: + the ENGINE MOTOR route (MachineType.h EOF, EtherCAT/Pci1203MotorRoute.cpp), OFF by default (design 7.1 "預設關"). To arm: $true here AND uncomment it in MachineType.h -- both
foreach ($macro in @('WB_PUMP_1203_CONTROL', 'WB_PUMP_1203_CONTROL_LIVE', 'WB_PUMP_1203_START_RING', 'WB_ENGINE_IO_1203', 'WB_ENGINE_MOTOR_1203', 'WB_ENGINE_INDEXZ_1203')) {   # AI(W906-ECAT-ROUTE) 20260929: + WB_ENGINE_MOTOR_1203   #AI(W906-INDEXZ) 20260930: + WB_ENGINE_INDEXZ_1203
    $active = @()
    for ($i = 0; $i -lt $mtLines.Count; $i++) {
        # ⚠ WB_PUMP_1203_CONTROL is a PREFIX of WB_PUMP_1203_CONTROL_LIVE, so the
        #   name must be followed by end-of-line or a comment -- otherwise the
        #   LIVE line would count as the plain one being active and the gate
        #   would report the safe macro as armed while missing the dangerous one.
        if ($mtLines[$i] -match ('^\s*#\s*define\s+' + [regex]::Escape($macro) + '\s*(//.*)?$')) {
            $active += "  MachineType.h:$($i+1): $($mtLines[$i].Trim())"
        }
    }
    $isActive = ($active.Count -gt 0)
    $isLive   = ($macro -eq 'WB_PUMP_1203_CONTROL_LIVE'); $isEngine = ($macro -eq 'WB_ENGINE_IO_1203'); $isEngineMot = ($macro -eq 'WB_ENGINE_MOTOR_1203'); $isEngineIdx = ($macro -eq 'WB_ENGINE_INDEXZ_1203')   # AI(W906-ECAT-ROUTE) 20260929   #AI(W906-INDEXZ) 20260930
    #AI(W906-BU-C4) 20260918: the four branches used to share one pair of
    #  messages, which was safe only while the two macros were expected to be in
    #  OPPOSITE states. Now that both are expected ACTIVE, the shared green text
    #  ("...as the 20260911 user ruling requires (dry run)") would print the
    #  words "dry run" for the macro that is precisely NOT dry. Per-macro text.
    if ($isActive -and -not $expectActive[$macro]) {
        Write-Host "FAIL: $macro is ACTIVE in MachineType.h:" -ForegroundColor Red
        $active | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        Write-Host "The expectation table says it should not be. Reconcile the table and the tree" -ForegroundColor Yellow
        Write-Host "together -- see the 20260918 ruling banner above." -ForegroundColor Yellow
        $fail++
    } elseif (-not $isActive -and $expectActive[$macro]) {
        Write-Host "FAIL: $macro is NOT active in MachineType.h." -ForegroundColor Red
        if ($isLive) {
            Write-Host "That leaves the tree in the third state the user ruled out on 20260918:" -ForegroundColor Yellow
            Write-Host "built from this tree, but unable to drive anything. The ruling is that there" -ForegroundColor Yellow
            Write-Host "are exactly two builds -- SOFT_SIMULTE, or machine-capable." -ForegroundColor Yellow
        } elseif ($isEngineMot) { Write-Host "The ENGINE's TMyEtherCatMotor calls then keep their stub arms: nothing MainProc commands reaches a 1203 axis (ruling RULINGS_20260929 #11 wants them on the card once armed)." -ForegroundColor Yellow } elseif ($isEngineIdx) { Write-Host "HT9050's Index Z1 (M14) then stays on golden's no-card Galil branch: the Index flow runs its steps and Z1 never presses (INBOX 113)." -ForegroundColor Yellow } elseif ($isEngine) { Write-Host "The engine's SW[]/Sen[]/Cylinder[] 1203 points then fall back to the stub (read 0, write nothing); user ruling 20260925 wants them on the card." -ForegroundColor Yellow } else {
            Write-Host "The 1203 page then renders with NO CONTROLS AT ALL and says 'the write surface" -ForegroundColor Yellow
            Write-Host "is NOT armed'. That is what the user hit on 20260911; it is a user ruling that" -ForegroundColor Yellow
            Write-Host "this stays on." -ForegroundColor Yellow
        }
        $fail++
    } elseif ($isActive -and $isLive) {
        # Expected, and still the single most consequential fact this gate knows.
        Write-Host ""
        Write-Host "  *** LIVE: $macro is ACTIVE (user ruling, 20260918) ***" -ForegroundColor Red
        $active | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        Write-Host "  On a build with HAVE_PCI1203, one click in a web page can energise a coil," -ForegroundColor Yellow; if ($expectActive['WB_ENGINE_IO_1203']) { Write-Host "  -- and with WB_ENGINE_IO_1203 the ENGINE's own SW[]/Cylinder[] writes reach the card with NO click at all (tower lamp, buzzer, door lock, motor relay ...)." -ForegroundColor Yellow }
        Write-Host "  turn a servo, and (since the 20260916b package) DISABLE AN OVER-TRAVEL" -ForegroundColor Yellow
        Write-Host "  LIMIT via pci1203.ax.setOtP / setOtN. No further opt-in. This is intended." -ForegroundColor Yellow
        Write-Host "  Not a failure; see the ruling banner in this file for how to reverse it." -ForegroundColor Yellow
        Write-Host ""
    } elseif ($isActive) {
        Write-Host "opt-in: $macro is ACTIVE, as the 20260911 user ruling requires" -ForegroundColor Green; if ($isEngineMot) { Write-Host "  *** WB_ENGINE_MOTOR_1203: the ENGINE's own motion (MainProc moves / stops / homes of 1203 axes) reaches the card with NO click -- on a HAVE_PCI1203 build with an armed control and WB_PUMP_1203_START_RING, wb_serve installs it at start ***" -ForegroundColor Yellow }; if ($isEngineIdx) { Write-Host "  *** WB_ENGINE_INDEXZ_1203: golden's Index flow presses HT9050's Z1 (M14) through the 1203 with NO click -- on a HAVE_PCI1203 build with an armed control, an OPEN card and WB_PUMP_1203_START_RING, wb_serve installs it at start (first automatic run with EastSun present) ***" -ForegroundColor Yellow }   # AI(W906-ECAT-ROUTE) 20260929   #AI(W906-INDEXZ) 20260930
    } else {
        Write-Host "opt-in: $macro is not active -- a click cannot reach the machine" -ForegroundColor Green
    }
}

# Self-test for check 4: prove the active/commented matcher separates them.
if (-not $NoSelfTest) {
    $stFail = 0
    if ('// #define WB_PUMP_1203_CONTROL' -match '^\s*#\s*define\s+WB_PUMP_1203_CONTROL\s*(//.*)?$') {
        Write-Host "SELFTEST FAIL: a commented-out #define read as active" -ForegroundColor Red; $stFail++
    }
    if ('#define WB_PUMP_1203_CONTROL' -notmatch '^\s*#\s*define\s+WB_PUMP_1203_CONTROL\s*(//.*)?$') {
        Write-Host "SELFTEST FAIL: an active #define read as commented out" -ForegroundColor Red; $stFail++
    }
    # And prove the prose in MachineType.h does NOT trip it -- that is the false
    # positive this shape exists to avoid.
    if ('//  ⚠ WB_PUMP_1203_CONTROL_LIVE 才是真的會動的那一個。' -match '^\s*#\s*define\s+WB_PUMP_1203_CONTROL') {
        Write-Host "SELFTEST FAIL: prose naming the macro read as a definition" -ForegroundColor Red; $stFail++
    }
    if ($stFail -gt 0) { exit 1 }
    Write-Host "SELFTEST ok: active/commented/prose #define forms are separated" -ForegroundColor DarkGray
}

# -----------------------------------------------------------------------------
#  5. THE DRY-RUN BRANCH MUST RETURN BEFORE THE VENDOR SWITCH.
#
#  Dry run is only worth anything if nothing after it can issue. Structural, not
#  flow-analytic, on purpose: a cheap check that cannot be quietly satisfied is
#  worth more than an accurate one nobody maintains. The invariant is that the
#  `if (impl_->dryRun)` block contains a `return` and that the first vendor call
#  in the LIVE switch comes after it.
# -----------------------------------------------------------------------------
$dryAt = -1; $dryReturnAt = -1; $firstVendorAt = -1
for ($i = 0; $i -lt $srcLines.Count; $i++) {
    $code = Strip $srcLines[$i]
    if ($dryAt -lt 0 -and $code -match 'if\s*\(\s*impl_->dryRun\s*\)') { $dryAt = $i; continue }
    if ($dryAt -ge 0 -and $dryReturnAt -lt 0 -and $code -match '^\s*return\b') { $dryReturnAt = $i }
    if ($firstVendorAt -lt 0 -and $code -match '^\s*ret\s*=\s*Acm2?_') { $firstVendorAt = $i }
}
if ($dryAt -lt 0) {
    Write-Host "FAIL: no `if (impl_->dryRun)` branch found in Pci1203Control.cpp." -ForegroundColor Red
    Write-Host "      Dry run is a first-class mode (honesty rule 3), not an optional one." -ForegroundColor Yellow
    $fail++
} elseif ($dryReturnAt -lt 0 -or ($firstVendorAt -ge 0 -and $dryReturnAt -gt $firstVendorAt)) {
    Write-Host "FAIL: the dry-run branch does not return before the first vendor call." -ForegroundColor Red
    Write-Host ("      dryRun test at :{0}, first `ret = Acm_*` at :{1}" -f ($dryAt+1), ($firstVendorAt+1)) -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("dry run: returns at :{0}, before the first vendor call at :{1}" -f ($dryReturnAt+1), ($firstVendorAt+1)) -ForegroundColor Green
}

# -----------------------------------------------------------------------------
#  6. NOTHING IS ARMED WITHOUT A CALL TO Pci1203ControlEnable, and that call
#     must have exactly one caller, inside the macro guard.
#
#  The same call-site argument the monitor's header makes: no macro definition
#  can arm a TEST, because the code that arms is not in any test binary. That
#  stops being true the moment a second caller appears.
# -----------------------------------------------------------------------------
#  THE PREFILTER IS `git grep`, NOT A DIRECTORY WALK, and that is measured
#    rather than assumed. In the A tree:
#        git grep --untracked -l         0.5 s   -> 4 files
#        Get-ChildItem -Recurse ...      still running after 24 minutes
#    The walk enumerates build/ before Where-Object can exclude it. This tree
#    has a standing rule against recursive scans for exactly this reason, and a
#    gate that does not finish is the same as not having one.
#
#  `--untracked` matters and is not cosmetic: a brand-new call site in a file
#  that has not been `git add`ed yet is precisely the case this check exists to
#  catch. Plain `git grep` would miss it and report a clean "exactly 1".
#  Verified by planting a throwaway untracked .cpp containing the name and
#  confirming it appeared in the list (then deleting it).
#  git ignores build/ already; third_party IS tracked, so that half is kept.
#
#  Stage 2 below is the ORIGINAL line-by-line analysis, unchanged. A file where
#  the bare name never appears cannot contain a call site, so nothing is skipped.
#AI(W906-Q34-5b) 20260923: the prefilter names BOTH entry points now --
#  Pci1203ControlEnable and the new Pci1203AxisIniTick (1203INI-1, below). A file
#  that mentions only the second one would otherwise never reach stage 2, and the
#  "exactly 1" for it would be a count of nothing. Same for the slow fallback.
#  ⚠ PKG replaced this whole prefilter with its Get-ChildItem -Recurse walk; that
#  part is deliberately NOT taken (the 24-minute measurement above).
$candidates = @()
$gitOut = @(& git -C $repo grep --untracked -l -e 'Pci1203ControlEnable' -e 'Pci1203AxisIniTick' -e 'W906_InstallPci1203IoRoute' -e 'W906_InstallPci1203MotorRoute' -e 'SetEcatMotorRoute' -e 'Pci1203MotorRouteTable' -e 'W906_EcRouteSlot_' -e 'W906_InstallPci1203GaliRoute' -e 'W906_SetGaliRoute' -- '*.cpp' '*.h' 2>$null)   # AI(W906-IOWEB-P17) 20260925: + the engine IO route installer   # AI(W906-ECAT-ROUTE) 20260929: + the engine motor route installer   # AI(W906-ENG1203) 20260929: + the route SLOT's writers (review build #1: a second install path must not bypass the installer's preconditions)
if ($LASTEXITCODE -le 1 -and $gitOut.Count -gt 0) {
    $candidates = @($gitOut |
        Where-Object { $_ -notmatch '^third_party/' } |
        ForEach-Object { Join-Path $repo ($_ -replace '/', '\') })
} else {
    Write-Host "call sites: git unavailable, falling back to a directory walk (slow)" -ForegroundColor Yellow
    $candidates = @(
        Get-ChildItem $repo -Recurse -Include *.cpp, *.h -File |
            Where-Object { $_.FullName -notmatch '\\build' -and $_.FullName -notmatch '\\third_party\\' } |
            Select-String -Pattern 'Pci1203ControlEnable', 'Pci1203AxisIniTick', 'W906_InstallPci1203IoRoute', 'W906_InstallPci1203MotorRoute', 'SetEcatMotorRoute', 'Pci1203MotorRouteTable', 'W906_EcRouteSlot_', 'W906_InstallPci1203GaliRoute', 'W906_SetGaliRoute' -SimpleMatch -List -Encoding UTF8 |
            ForEach-Object { $_.Path })
}
Write-Host ("call sites: {0} file(s) mention the name; analysing those" -f $candidates.Count)
#AI(W906-1203INI-1) 20260917: ⚠ AND THE SECOND ENTRY POINT, WHICH IS NEW.
#  Pci1203AxisIniTick() re-applies recorded card settings and therefore WRITES
#  without an operator pressing anything -- (AI(W906-IOWEB-P17) 20260925: one of TWO such paths now; the other is the engine IO route, W906_InstallPci1203IoRoute, counted below by the same rule) -- the only path in this module that
#  does. The argument this check exists to protect ("no test binary can arm or
#  drive the write surface, because the code that does is not in one") is only
#  as strong as its weakest entry point, so the new one is counted by the same
#  rule and with the same expectation of exactly one caller.
#  ⓘ Counted in the same sweep rather than a copied loop: two loops drift, and
#  the one that drifts is the one nobody re-reads.
$callers = @()
$tickers = @(); $routers = @(); $motorRouters = @(); $slotWriters = @(); $galiRouters = @(); $galiSlotWriters = @()   #AI(W906-INDEXZ) 20260930: + W906_InstallPci1203GaliRoute call sites and W906_SetGaliRoute( writers outside tests\ (expected: the installer's one)   # AI(W906-IOWEB-P17) 20260925: W906_InstallPci1203IoRoute call sites   # AI(W906-ECAT-ROUTE) 20260929: + W906_InstallPci1203MotorRoute call sites   # AI(W906-ENG1203) 20260929: + SetEcatMotorRoute( / Pci1203MotorRouteTable( / W906_EcRouteSlot_( outside tests\ (expected: the installer's one)
$candidates |
    ForEach-Object {
        $p = $_
        $n = 0
        foreach ($l in (Get-Content $p -Encoding UTF8)) {
            $n++
            $code = Strip $l; if ($code -match '\bW906_InstallPci1203IoRoute\s*\(' -and $code -notmatch '^\s*void\s+W906_InstallPci1203IoRoute\s*\(') { $routers += ("  " + $p.Substring($repo.Length + 1) + ":${n}") }; if ($code -match '\bW906_InstallPci1203MotorRoute\s*\(' -and $code -notmatch '^\s*void\s+W906_InstallPci1203MotorRoute\s*\(') { $motorRouters += ("  " + $p.Substring($repo.Length + 1) + ":${n}") }; if ($p.Substring($repo.Length + 1) -notmatch '^tests[\\/]' -and $code -match '\b(SetEcatMotorRoute|Pci1203MotorRouteTable|W906_EcRouteSlot_)\s*\(' -and $code -notmatch '^\s*inline\s+(void\s+SetEcatMotorRoute|const\s+TEcatMotorRoute\s*\*&?\s*(W906_EcRouteSlot_|EcatMotorRoute))\s*\(' -and $code -notmatch '^\s*const\s+TEcatMotorRoute\s*\*\s*Pci1203MotorRouteTable\s*\(\s*\)\s*[;{]') { $slotWriters += ("  " + $p.Substring($repo.Length + 1) + ":${n}") }; if ($code -match '\bW906_InstallPci1203GaliRoute\s*\(' -and $code -notmatch '^\s*void\s+W906_InstallPci1203GaliRoute\s*\(') { $galiRouters += ("  " + $p.Substring($repo.Length + 1) + ":${n}") }; if ($p.Substring($repo.Length + 1) -notmatch '^tests[\\/]' -and $code -match '\bW906_SetGaliRoute\s*\(' -and $code -notmatch '^\s*void\s+W906_SetGaliRoute\s*\(') { $galiSlotWriters += ("  " + $p.Substring($repo.Length + 1) + ":${n}") }   #AI(W906-INDEXZ) 20260930: + the Index Z1 route's installer and slot writers (Motor\GaliRoute.h's declaration and Motor\myGALILmotor.cpp's definition start with `void`)   # AI(W906-ECAT-ROUTE) 20260929   # AI(W906-ENG1203) 20260929: + the route SLOT's writers outside tests\ (review build #1); definitions / the declaration are not call sites (Motor\EcatMotorRoute.h's three inline accessors, Pci1203MotorRoute.{h,cpp}'s table getter)
            if ($code -match '\bPci1203AxisIniTick\s*\(') {
                if ($code -match '^\s*(bool|void|int|inline|static)\s+Pci1203AxisIniTick\s*\(') { continue }
                if ($p -eq $hdr) { continue }
                $tickers += ("  " + $p.Substring($repo.Length + 1) + ":${n}")
            }
            if ($code -match '\bPci1203ControlEnable\s*\(') {
                # ⚠ A DEFINITION IS NOT A CALL SITE, and the first draft of this
                #   check counted one: EtherCAT/Pci1203Control.cpp's own
                #   `bool Pci1203ControlEnable(bool dryRun, std::string& why)`
                #   matched, so the gate reported 2 callers and went red on a
                #   correct tree. A return type immediately before the name is
                #   what separates the two forms.
                if ($code -match '^\s*(bool|void|int|inline|static)\s+Pci1203ControlEnable\s*\(') { continue }
                if ($p -eq $hdr) { continue }        # the declaration
                $callers += ("  " + $p.Substring($repo.Length + 1) + ":${n}")
            }
        }
    }
if ($callers.Count -ne 1) {
    Write-Host ("FAIL: Pci1203ControlEnable has {0} call site(s); exactly 1 is expected (tools/wb_serve.cpp)." -f $callers.Count) -ForegroundColor Red
    $callers | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "More than one caller breaks the argument that no ctest executable can arm the" -ForegroundColor Yellow
    Write-Host "write surface. Re-count rather than inheriting that claim." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("call sites: Pci1203ControlEnable has exactly 1 --{0}" -f $callers[0]) -ForegroundColor Green
}

#AI(W906-1203INI-1) 20260917: the start-up re-apply, same rule.
#AI(W906-Q34-5b) 20260923: message says tools/wb_serve.cpp -- A's one caller is
#  there (PKG's is tools/wb_publish.cpp, which A no longer has -- T4-RETIRE).
if ($tickers.Count -ne 1) {
    Write-Host ("FAIL: Pci1203AxisIniTick has {0} call site(s); exactly 1 is expected (tools/wb_serve.cpp)." -f $tickers.Count) -ForegroundColor Red
    $tickers | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "This one writes the card WITHOUT an operator action, so a second caller is a" -ForegroundColor Yellow
    Write-Host "second unattended writer. Re-count rather than inheriting the claim." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("call sites: Pci1203AxisIniTick has exactly 1 --{0}" -f $tickers[0]) -ForegroundColor Green
}; if ($routers.Count -ne 1) { Write-Host ("FAIL: W906_InstallPci1203IoRoute has {0} call site(s); exactly 1 is expected (tools/wb_serve.cpp). It arms the ENGINE's IO onto the card -- a second caller is a second unattended writer." -f $routers.Count) -ForegroundColor Red; $routers | ForEach-Object { Write-Host $_ -ForegroundColor Red }; $fail++ } else { Write-Host ("call sites: W906_InstallPci1203IoRoute has exactly 1 --{0}  (engine IO route, IOWEB-P17)" -f $routers[0]) -ForegroundColor Green }; if ($motorRouters.Count -ne 1) { Write-Host ("FAIL: W906_InstallPci1203MotorRoute has {0} call site(s); exactly 1 is expected (tools/wb_serve.cpp). It arms the ENGINE's motion onto the card -- a second caller is a second unattended motion source." -f $motorRouters.Count) -ForegroundColor Red; $motorRouters | ForEach-Object { Write-Host $_ -ForegroundColor Red }; $fail++ } else { Write-Host ("call sites: W906_InstallPci1203MotorRoute has exactly 1 --{0}  (engine motor route, ECAT-ROUTE)" -f $motorRouters[0]) -ForegroundColor Green }; if ($slotWriters.Count -ne 1 -or $slotWriters[0] -notmatch '^\s*EtherCAT[\\/]Pci1203MotorRoute\.cpp:\d+$') { Write-Host ("FAIL: the engine motor route SLOT has {0} writer(s) outside tests\ (SetEcatMotorRoute( / Pci1203MotorRouteTable( / W906_EcRouteSlot_( ); exactly 1 is expected, the installer's in EtherCAT\Pci1203MotorRoute.cpp. Any other path installs the route WITHOUT the installer's preconditions (SOFT_SIMULTE, WB_ENGINE_MOTOR_1203, START_RING, armed control) -- and Pci1203MotorRouteInstalled() would still say false." -f $slotWriters.Count) -ForegroundColor Red; $slotWriters | ForEach-Object { Write-Host $_ -ForegroundColor Red }; $fail++ } else { Write-Host ("call sites: the engine motor route slot has exactly 1 writer --{0}  (the installer; review build #1)" -f $slotWriters[0]) -ForegroundColor Green }; if ($galiRouters.Count -ne 1) { Write-Host ("FAIL: W906_InstallPci1203GaliRoute has {0} call site(s); exactly 1 is expected (tools/wb_serve.cpp). It arms HT9050's Index Z1 onto the card -- a second caller is a second unattended motion source." -f $galiRouters.Count) -ForegroundColor Red; $galiRouters | ForEach-Object { Write-Host $_ -ForegroundColor Red }; $fail++ } else { Write-Host ("call sites: W906_InstallPci1203GaliRoute has exactly 1 --{0}  (Index Z1 route, INBOX 113)" -f $galiRouters[0]) -ForegroundColor Green }; if ($galiRouters.Count -eq 1) { $wsLines = @(Get-Content -LiteralPath (Join-Path $repo 'tools\wb_serve.cpp') -Encoding UTF8); $enableAt = -1; for ($wi = 0; $wi -lt $wsLines.Count; $wi++) { if ((Strip $wsLines[$wi]) -match '\bPci1203MonitorEnable\s*\(') { $enableAt = $wi + 1; break } }; $gm = [regex]::Match($galiRouters[0], '^\s*tools[\\/]wb_serve\.cpp:(\d+)$'); if (-not $gm.Success -or $enableAt -lt 0 -or [int]$gm.Groups[1].Value -le $enableAt) { Write-Host ("FAIL: W906_InstallPci1203GaliRoute must be called in tools\wb_serve.cpp AFTER Pci1203MonitorEnable (:{0}); it is at{1}. Before that the monitor does not exist, the route's card-open check refuses and the route never installs (review round 2 C of INBOX 113)." -f $enableAt, $galiRouters[0]) -ForegroundColor Red; $fail++ } else { Write-Host ("boot order: W906_InstallPci1203GaliRoute (tools\wb_serve.cpp:{0}) runs after Pci1203MonitorEnable (:{1})  (review round 2 C; ctest GaliRouteLive part S pins it too)" -f $gm.Groups[1].Value, $enableAt) -ForegroundColor Green } }; if ($galiSlotWriters.Count -ne 1 -or $galiSlotWriters[0] -notmatch '^\s*EtherCAT[\\/]Pci1203GaliRoute\.cpp:\d+$') { Write-Host ("FAIL: the Index Z1 route slot has {0} writer(s) outside tests\ (W906_SetGaliRoute( ); exactly 1 is expected, the installer's in EtherCAT\Pci1203GaliRoute.cpp. Any other path installs the route WITHOUT the installer's preconditions (the gate, an armed control, an open card, the M14 row)." -f $galiSlotWriters.Count) -ForegroundColor Red; $galiSlotWriters | ForEach-Object { Write-Host $_ -ForegroundColor Red }; $fail++ } else { Write-Host ("call sites: the Index Z1 route slot has exactly 1 writer --{0}  (the installer)" -f $galiSlotWriters[0]) -ForegroundColor Green }   #AI(W906-INDEXZ) 20260930   # AI(W906-ECAT-ROUTE) 20260929   # AI(W906-ENG1203) 20260929: + the slot-writer count

# -----------------------------------------------------------------------------
#  5. SDO WRITES MAY ADDRESS 2710h AND NOTHING ELSE.
#     AI(W906-1203ALM-5) 20260912.
#
#  Acm_DevWriteSDOData used to be in the $forbidden list above. It came out so
#  the 絕對編碼器重置 (Fn008) button could exist -- that service IS CoE object
#  2710h and there is no other route to it. Taking a name off a ban list and
#  leaving nothing behind would hand the module the ability to write ANY drive
#  parameter over SDO, which is far wider than what was asked for.
#
#  So the ban is replaced, not deleted: every Acm_DevWriteSDOData call site in
#  this module must name index 0x2710. The index is the 4th argument
#  (handle, ring, station, INDEX, subindex, type, size, buffer).
#
#  WHAT MAKES THIS GO RED: an SDO write to any other object, a call whose index
#  is a variable the gate cannot see, or the detector ceasing to detect -- the
#  self-test below proves the last one every run.
# -----------------------------------------------------------------------------
$sdoRe = '\bAcm_DevWriteSDOData\s*\('

#AI(W906-1203OT-1) 20260915: ⚠ THE SPLIT IS PAREN-AWARE NOW, AND IT WAS NOT.
#  The old rule was a regex whose index group was `([^,]+)` -- "everything up to
#  the next comma". That is right only while no index helper takes more than one
#  argument. Pci1203OtIndex(which, stationAxis) takes two, so the extractor
#  returned the FRAGMENT `Pci1203OtIndex(c.otWhich` and the gate rejected a call
#  that was on its own permitted list.
#  ⓘ It failed LOUDLY, which is the right direction and why this is a defect
#  worth fixing rather than a near miss -- but a detector that hands back a
#  mangled string is comparing the wrong thing against the allowlist, and the
#  next mangling might land on something the list happens to contain.
function Get-SdoArg4([string]$text) {
    # $null when the text is not an SDO write; otherwise its 4th argument, with
    # nesting respected. Commas inside (), <> template args and [] do not split.
    $m = [regex]::Match($text, $sdoRe)
    if (-not $m.Success) { return $null }
    $i = $m.Index + $m.Length      # first character after the opening paren
    $depth = 0
    $arg = 1
    $sb = New-Object System.Text.StringBuilder
    while ($i -lt $text.Length) {
        $ch = $text[$i]
        if ($ch -eq '(' -or $ch -eq '[') { $depth++ }
        elseif ($ch -eq ']') { $depth-- }
        elseif ($ch -eq ')') {
            if ($depth -eq 0) { return $null }   # call closed before arg 4
            $depth--
        }
        elseif ($ch -eq ',' -and $depth -eq 0) {
            if ($arg -eq 4) { return $sb.ToString().Trim() }
            $arg++
            [void]$sb.Clear()
            $i++
            continue
        }
        if ($arg -eq 4) { [void]$sb.Append($ch) }
        $i++
    }
    return $null                    # ran off the end: unparseable, not "absent"
}

function Test-SdoIndex([string]$line) {
    # Returns $null when the line is not an SDO write; otherwise the index text.
    return (Get-SdoArg4 (Strip $line))
}

#AI(W906-1203GEAR-1) 20260915: ⚠ THE DETECTOR USED TO HAVE A BLIND SPOT AND IT
#  WAS FOUND BY THIS GATE FAILING ON A CALL IT SHOULD HAVE PASSED.
#  Test-SdoIndex reads ONE line. Every SDO write in the module used to fit on
#  one line, so that was invisible -- until the gear-apply write was written as
#
#      ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,
#                                Pci1203GearApplyIndex(a.stationAxis), 0,
#
#  whose index argument is on the SECOND line. The regex found no 4th argument,
#  Test-SdoIndex returned $null, and the loop's `continue` treated it as "not an
#  SDO write at all". The count said 2 call sites where there are 3.
#  ⚠ THAT IS THE DANGEROUS DIRECTION: wrapping a line would have removed a write
#  from the gate's view entirely, silently, while the gate still printed green.
#
#  Two changes, and the second matters more than the first:
#    * join continuation lines so a wrapped call is read properly;
#    * and if a line NAMES Acm_DevWriteSDOData but no index can be extracted,
#      that is now a FAILURE rather than a skip. Unparseable must not mean
#      unchecked.
function Get-SdoCallSites([string[]]$lines) {
    $out = @()
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $code = Strip $lines[$i]
        if ($code -notmatch '\bAcm_DevWriteSDOData\s*\(') { continue }
        # Join up to three following lines -- enough for any wrapped call here,
        # and bounded so a runaway cannot swallow the rest of the file.
        $joined = $code
        for ($k = 1; $k -le 3 -and ($i + $k) -lt $lines.Count; $k++) {
            if ($joined -match '\)\s*;') { break }
            $joined = $joined + ' ' + (Strip $lines[$i + $k])
        }
        $idx = Get-SdoArg4 $joined
        $out += [pscustomobject]@{
            Line  = $i + 1
            Index = $(if ([string]::IsNullOrWhiteSpace($idx)) { $null } else { $idx })
            Text  = $lines[$i].Trim()
        }
    }
    return $out
}

#  The exact index spellings permitted at a call site, each with its reason.
$sdoIndexOk = @{
    '0x2710' =
        'SERVOPACK Adjusting Command -- Fn008 絕對編碼器重置 (s14.6.7)'
    'idx' =
        'the gear object, assigned on the line above from Pci1203GearIndex() + Pci1203GearAxisBase(); check 5b bounds what those can return'
    'Pci1203GearApplyIndex(a.stationAxis)' =
        'User Parameter Configuration 2700h/2F00h -- makes the written gear take effect (s14.6.2)'
    #AI(W906-1203HOME-1) 20260915: Pn000, the motor's forward direction.
    #  ⚠ This one WIDENS what the module can reach -- 2000h is Pn000, a
    #  different parameter block from the gear objects -- so it is listed
    #  explicitly, with its reason, rather than arriving as a side effect of a
    #  looser rule. Check 5b bounds it to exactly 0x2000 + the axis offset.
    'Pci1203DriveDirIndex(a.stationAxis)' =
        'Pn000 2000h/2800h -- Rotation Direction Selection, read-modify-write (p.707)'
    #AI(W906-1203STORE-1) 20260915: 1010h Store Parameters.
    #  ⚠ This one commits EVERY parameter in the drive's RAM to non-volatile
    #  memory, not just the object the operator was looking at -- the widest
    #  single effect any write on this list has. It is here because without it
    #  every drive parameter this module writes is lost at the next power cycle
    #  (p.141), which is a defect that reads as "it was never written".
    #  Bounded by check 5b: Pci1203StoreIndex() can only return 0x1010.
    'Pci1203StoreIndex()' =
        '1010h:1 Store Parameters -- write the "save" signature (p.585)'
    #AI(W906-1203OT-1) 20260915: Pn50A/Pn50B, the drive's overtravel allocation.
    #  ⚠ This one can DISABLE AN OVERTRAVEL LIMIT, which is the widest safety
    #  effect on this list. It is here because the user asked for it explicitly
    #  and repeatedly, the card's own limit switch demonstrably does not do it,
    #  and the alternative -- a loose one-off tool outside the gated module --
    #  would have the same effect with none of the confirmation, validation or
    #  review this path carries. Bounded by check 5b to 0x250A/0x250B.
    'Pci1203OtIndex(c.otWhich, a.stationAxis)' =
        'Pn50A/Pn50B 250Ah/250Bh -- overtravel allocation, read-modify-write (s5.10.2)'
    #AI(W906-1203ENC-1) 20260916: Pn21D, the encoder resolution compatibility.
    #  ⚠ The widest-reaching object on this list. It changes how many counts the
    #  drive believes one revolution has, so every position, speed and taught
    #  coordinate is re-scaled at once -- and it does that by DISCARDING encoder
    #  bits, so it costs real positioning accuracy rather than merely renaming
    #  the numbers. It is here because the operator asked for it by name once
    #  the electronic gear turned out to be bounded at a ratio of 64000 and
    #  their mechanism needed to go further. Bounded by check 5b to 0x221D.
    'Pci1203EncCompatIndex(a.stationAxis)' =
        'Pn21D 221Dh/2A1Dh -- encoder resolution compatibility, read-modify-write (p.193)'
    #AI(W906-1203DHOME-1) 20260917: the DS402 homing objects.
    #  ⚠ THIS ENTRY WIDENS THE MODULE INTO THE 6xxxh PROFILE AREA for the first
    #  time -- everything above it is 1010h or the 2xxxh manufacturer area. That
    #  is worth a reviewer's attention rather than a shrug, so it is listed with
    #  its reason and bounded by check 5b to exactly four objects.
    #  It is here because the four HOME speed boxes on the panel were writing
    #  card properties that this machine's DS402 axes never consult: measured by
    #  setting them to 1234/4321/111111, homing, and reading 8000/2000/10000 off
    #  the drive. 6099h/609Ah/607Ch are the objects a home here actually obeys.
    #  ⓘ 6098h (the METHOD) is deliberately NOT reachable from this module --
    #  Acm_AxHome writes it from its mode argument, and a second writer would be
    #  a second source of truth for one value. The monitor READS it; reads are
    #  not what this check is about.
    'Pci1203DriveHomeIndex(c.driveHome, a.stationAxis)' =
        'DS402 homing objects 6099h/609Ah/607Ch (+0x800 for axis B) -- the speeds and offset the drive homes with (s14.9, p.559)'
    #AI(W906-MT-FIX1) 20260926: the CiA 402 TORQUE LIMITS, kCmdAxTorqueLimitSet
    #  (INTERNAL -- no wire name; the laptop's Motor Test layer issues it).
    #  Interface agreed with the laptop 20260925; user EastSun approved adding
    #  exactly these two objects to the SDO write list.
    #  ⚠ SPELLED AS TWO LITERALS, ONE PER OBJECT, and that is the point of listing
    #  them this way rather than as a helper: the object number is visible IN the
    #  permitted spelling, so this table alone proves the call can reach 60E0h /
    #  60E1h and nothing else. The only computed part is Pci1203GearAxisBase(),
    #  which check 5b bounds to 0x000 / 0x800 (axis B = 68E0h / 68E1h).
    #  ⚠ This widens the module in the 6xxxh profile area again (after the DS402
    #  homing block). A torque limit set wrong is not a move -- but 0 means the
    #  axis produces no torque at all and cannot hold a load, which is why the
    #  command validates by station / CiA 402 / UINT16 and reads both back.
    '(U16)(0x60E0 + Pci1203GearAxisBase(sa_))' =
        'CiA 402 60E0h Positive Torque Limit Value (68E0h for axis B), UINT16 in 0.1 % -- kCmdAxTorqueLimitSet, internal'
    '(U16)(0x60E1 + Pci1203GearAxisBase(sa_))' =
        'CiA 402 60E1h Negative Torque Limit Value (68E1h for axis B), UINT16 in 0.1 % -- kCmdAxTorqueLimitSet, internal'
}

if (-not $NoSelfTest) {
    $stFail = 0
    if ((Test-SdoIndex '    Acm_DevWriteSDOData(dev, 0, st, 0x2710, 1, 9, 10, cmd);') -ne '0x2710') { $stFail++ }
    if ((Test-SdoIndex '    Acm_DevWriteSDOData(dev, 0, st, 0x6040, 0, 4, 2, &v);')   -ne '0x6040') { $stFail++ }
    if ($null -ne (Test-SdoIndex '    Acm_DevReadSDOData(dev, 0, st, 0x603F, 0, 4, 2, &v);')) { $stFail++ }
    #AI(W906-1203OT-1) 20260915: the COMMA-IN-THE-INDEX case, pinned. This is
    #  the one the old `([^,]+)` regex got wrong, and the failure it produced --
    #  a truncated spelling rejected against the allowlist -- looked exactly
    #  like a genuine violation. Both halves are asserted: that a two-argument
    #  helper survives, and that a single-argument one still does.
    $twoArg = '    Acm_DevWriteSDOData(dev, 0, st, Pci1203OtIndex(c.otWhich, a.stationAxis), 0, 4, 2, &v);'
    if ((Test-SdoIndex $twoArg) -ne 'Pci1203OtIndex(c.otWhich, a.stationAxis)') {
        Write-Host ("SELFTEST FAIL: a 2-arg index helper is read as '" +
                    (Test-SdoIndex $twoArg) + "', not whole.") -ForegroundColor Red
        $stFail++
    }
    if ((Test-SdoIndex '    Acm_DevWriteSDOData(dev, 0, st, Pci1203StoreIndex(), 0, 6, 4, &s);') -ne 'Pci1203StoreIndex()') { $stFail++ }
    #  And a call that never reaches a 4th argument must read as unparseable
    #  ($null), which the caller turns into a FAILURE -- not as "no SDO write".
    if ($null -ne (Test-SdoIndex '    Acm_DevWriteSDOData(dev, 0, st);')) {
        Write-Host "SELFTEST FAIL: a truncated SDO call yielded an index." -ForegroundColor Red
        $stFail++
    }
    if ($stFail -gt 0) {
        Write-Host "SELFTEST FAIL: the SDO index detector is broken, so its GREEN means nothing." -ForegroundColor Red
        exit 1
    }
    Write-Host "SELFTEST ok: SDO index detector reads 2710h, reads a non-2710h, ignores reads, keeps a 2-arg helper whole, rejects a truncated call" -ForegroundColor DarkGray

    #AI(W906-1203GEAR-1) 20260915: the detector working is not the same as the
    #  RULE working. Since check 5 now compares against a table of permitted
    #  spellings rather than one literal, the table is what has to be proven to
    #  reject things -- a hashtable lookup that accepted everything would look
    #  identical to one that worked.
    $stFail = 0
    foreach ($good in @('0x2710', 'idx', 'Pci1203GearApplyIndex(a.stationAxis)',
                        'Pci1203DriveDirIndex(a.stationAxis)',
                        'Pci1203StoreIndex()',
                        'Pci1203OtIndex(c.otWhich, a.stationAxis)',
                        'Pci1203EncCompatIndex(a.stationAxis)',
                        'Pci1203DriveHomeIndex(c.driveHome, a.stationAxis)',
                        '(U16)(0x60E0 + Pci1203GearAxisBase(sa_))',
                        '(U16)(0x60E1 + Pci1203GearAxisBase(sa_))')) {   # AI(W906-MT-FIX1) 20260926: + the two torque limits
        if (-not $sdoIndexOk.ContainsKey($good)) {
            Write-Host "SELFTEST FAIL (permitted spelling rejected): $good" -ForegroundColor Red; $stFail++
        }
    }
    #AI(W906-MT-FIX1) 20260926: + the torque-limit spelling's NEIGHBOURS, which is
    #  what proves the table matches the literal and not a prefix: 60E2h, and
    #  607Ah (Target Position) / 6040h (Controlword) in the same spelling shape.
    foreach ($bad in @('0x6040', '0x2002', 'anyIdx', '0x2710 + 1', 'idx2',
                       '(U16)(0x60E2 + Pci1203GearAxisBase(sa_))',
                       '(U16)(0x607A + Pci1203GearAxisBase(sa_))',
                       '(U16)(0x6040 + Pci1203GearAxisBase(sa_))',
                       '(U16)(0x60E0 + 1)')) {
        if ($sdoIndexOk.ContainsKey($bad)) {
            Write-Host "SELFTEST FAIL (forbidden spelling accepted): $bad" -ForegroundColor Red; $stFail++
        }
    }
    #  And the 5b literal check must reject a literal outside the object set.
    $probeAllowed = @('0x1010','0x2000','0x221D','0x250A','0x250B','0x2700','0x2701','0x2702','0x2703','0x2704',
                      '0x607C','0x6098','0x6099','0x609A',
                      '0x800','0x000','0xF','0x000F','0x00F0','0xA')
    #AI(W906-1203OT-1) 20260915: ⚠ 0x250A WAS IN THIS REJECT LIST AND IS NOT ANY
    #  MORE. It was here as a named example of "an object the gear helpers must
    #  not be able to reach"; the overtravel work made it one they must. Leaving
    #  it would have made the self-test contradict the rule it is testing, which
    #  is the one failure mode a self-test cannot diagnose for you. 0x250C is the
    #  replacement example -- adjacent to the two now permitted, so it still
    #  proves the set is enumerated and not a prefix match.
    #AI(W906-1203DHOME-1) 20260917: ⚠ 0x607A AND 0x609B ARE THE IMPORTANT NEW
    #  REJECTS, and they are chosen to be adjacent to the four objects that were
    #  just permitted. 607Ch/6098h/6099h/609Ah now being on the list is exactly
    #  what makes "surely 607Ah is fine too" a plausible future edit -- and
    #  607Ah is Target Position: a literal that reaches it turns this module
    #  into one that can command a move by writing a parameter. 609Bh is the
    #  neighbour on the other side. Their presence here proves 5b enumerates the
    #  set rather than matching a 6xxxh prefix or a range.
    foreach ($nope in @('0x2710','0x2002','0x6040','0x250C','0x607A','0x609B')) {
        if ($probeAllowed -contains $nope) {
            Write-Host "SELFTEST FAIL: 5b would accept $nope as a gear-helper literal." -ForegroundColor Red; $stFail++
        }
    }
    foreach ($yes in @('0x2701','0x2000','0x221D','0x250A','0x250B',
                       '0x607C','0x6098','0x6099','0x609A')) {
        if (-not ($probeAllowed -contains $yes)) {
            Write-Host "SELFTEST FAIL: 5b would reject $yes." -ForegroundColor Red; $stFail++
        }
    }
    #AI(W906-1203GEAR-1) 20260915: the WRAPPED-CALL blind spot, pinned. This is
    #  the case that was silently uncounted until the gear work hit it: a call
    #  whose index argument sits on the next line. Both properties are asserted
    #  -- that it is FOUND at all, and that its index is read correctly.
    $wrapped = @(
        '            ret = Acm_DevWriteSDOData(dev, 0, (U16)a.station,',
        '                                      Pci1203GearApplyIndex(a.stationAxis), 0,',
        '                                      6 /*ECAT_TYPE_U32*/, 4, &one);'
    )
    # ⚠ @() is not decoration. A one-element result unwraps to a scalar in
    # PowerShell 5.1, so $ws.Count is EMPTY rather than 1 -- which made this
    # self-test report "not seen at all" for a call it had found correctly.
    $ws = @(Get-SdoCallSites $wrapped)
    if ($ws.Count -ne 1) {
        Write-Host "SELFTEST FAIL: a wrapped SDO write was not seen at all ($($ws.Count) found)." -ForegroundColor Red; $stFail++
    } elseif ($ws[0].Index -ne 'Pci1203GearApplyIndex(a.stationAxis)') {
        Write-Host "SELFTEST FAIL: wrapped SDO index read as '$($ws[0].Index)'." -ForegroundColor Red; $stFail++
    }
    #  And an unparseable call must be REPORTED, not skipped.
    $mangled = @('    ret = Acm_DevWriteSDOData(dev);')
    $ms = @(Get-SdoCallSites $mangled)
    if ($ms.Count -ne 1 -or $null -ne $ms[0].Index) {
        Write-Host "SELFTEST FAIL: an unparseable SDO write was skipped instead of flagged." -ForegroundColor Red; $stFail++
    }
    #  A call named inside a STRING LITERAL is a description, not a call. This
    #  is the audit-line shape every command in the module builds.
    $inString = @(
        '            std::snprintf(call, sizeof(call),',
        '                "Acm_DevWriteSDOData(station=%d, %04Xh:%u, U32) = %lu", a.station);'
    )
    if ((@(Get-SdoCallSites $inString)).Count -ne 0) {
        Write-Host "SELFTEST FAIL: an SDO call named inside a string literal was counted as a call." -ForegroundColor Red; $stFail++
    }
    #  ...but the real call must STILL be seen when a string sits on the line.
    $realPlusString = @('    ret = Acm_DevWriteSDOData(dev, 0, st, 0x2710, 1, 9, 10, cmd);  // "not a call"')
    $rs = @(Get-SdoCallSites $realPlusString)
    if ($rs.Count -ne 1 -or $rs[0].Index -ne '0x2710') {
        Write-Host "SELFTEST FAIL: string-stripping hid a REAL SDO write." -ForegroundColor Red; $stFail++
    }
    #AI(W906-MT-FIX1) 20260926: the torque-limit call shape, pinned -- a wrapped
    #  call whose index is a CAST OF A SUM with a nested helper call, followed by
    #  a /* */ type comment on the next line. It must be found, and its index
    #  read whole (the parens inside it must not end or split the argument).
    $tl = @(
        '            return Acm_DevWriteSDOData(dev_, 0, st_, (U16)(0x60E1 + Pci1203GearAxisBase(sa_)), 0,',
        '                                       4 /*ECAT_TYPE_U16*/, 2, &v);'
    )
    $tls = @(Get-SdoCallSites $tl)
    if ($tls.Count -ne 1 -or $tls[0].Index -ne '(U16)(0x60E1 + Pci1203GearAxisBase(sa_))') {
        Write-Host ("SELFTEST FAIL: the torque-limit SDO write was read as '" + $(if ($tls.Count -gt 0) { $tls[0].Index } else { '<not seen>' }) + "'.") -ForegroundColor Red; $stFail++
    }
    if ($stFail -gt 0) {
        Write-Host "SELFTEST FAIL: the SDO index RULE is broken, so its GREEN means nothing." -ForegroundColor Red
        exit 1
    }
    Write-Host ("SELFTEST ok: index table accepts its {0} spellings and rejects 9 others; wrapped call seen; call-in-string ignored; unparseable call flagged; torque-limit call read whole; 5b literal set is tight" -f $sdoIndexOk.Count) -ForegroundColor DarkGray
}

# -----------------------------------------------------------------------------
#  AI(W906-1203GEAR-1) 20260915: ⚠ THE RULE CHANGED, AND IT DID NOT GET WEAKER.
#
#  The electronic-gear buttons write 2700h-2704h, so "the index must be the
#  literal 0x2710" can no longer express what this module is allowed to do.
#  The tempting repair -- accept any literal in a list -- would NOT have worked
#  either, because the gear index is COMPUTED: the object depends on which half
#  of a two-axis SGDXW the axis is (axis B = axis A + 0x800), so the argument at
#  the call site is an expression, not a number.
#
#  An expression the gate cannot read would mean the gate stops proving
#  anything, so the rule is split in two and BOTH must hold:
#
#    5a. Every call site's index argument is one of a SMALL SET OF EXACT
#        SPELLINGS. Not "an expression" -- these specific ones, listed here.
#    5b. The helpers those spellings call can only PRODUCE allowed indices,
#        proved by extracting every hex literal from EtherCAT/Pci1203Gear.h and
#        checking the set. That header is the only place the arithmetic lives,
#        which is what makes 5b sufficient rather than decorative.
#
#  WHAT MAKES THIS GO RED: an SDO write whose index spelling is not listed; a
#  new hex literal appearing in Pci1203Gear.h; or the detector ceasing to
#  detect -- the self-test proves the last one every run.
# -----------------------------------------------------------------------------


$sdoBad = @()
$sdoOk  = 0
$srcLines = Get-Content $src -Encoding UTF8
foreach ($site in (Get-SdoCallSites $srcLines)) {
    if ($null -eq $site.Index) {
        # ⚠ Named the call, could not read its index. NOT a skip -- see the
        # banner on Get-SdoCallSites. An unreadable index is an unchecked write.
        $sdoBad += ("  Pci1203Control.cpp:" + $site.Line +
                    ": index could not be parsed -- " + $site.Text)
        continue
    }
    if ($sdoIndexOk.ContainsKey($site.Index)) { $sdoOk++ }
    else { $sdoBad += ("  Pci1203Control.cpp:" + $site.Line + ": index " +
                       $site.Index + " -- " + $site.Text) }
}
if ($sdoBad.Count -gt 0) {
    Write-Host "FAIL: SDO write(s) whose index spelling is not on the permitted list:" -ForegroundColor Red
    $sdoBad | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "Permitted spellings are:" -ForegroundColor Yellow
    $sdoIndexOk.Keys | Sort-Object | ForEach-Object {
        Write-Host ("  {0,-38} {1}" -f $_, $sdoIndexOk[$_]) -ForegroundColor Yellow
    }
    Write-Host "An index this gate cannot read means it cannot prove the module is unable to" -ForegroundColor Yellow
    Write-Host "set arbitrary drive parameters -- which is the whole point of the check." -ForegroundColor Yellow
    $fail++
} else {
    Write-Host ("SDO writes: {0} call site(s), all on permitted index spellings" -f $sdoOk) -ForegroundColor Green
}

#  5a-bis. `idx` is only meaningful if it really is the gear index. Require the
#  assignment to exist and to be built from the two shared helpers -- otherwise
#  a future edit could point `idx` at anything and keep this gate green.
#  ⚠ Joined over a 3-line window for the SAME reason as the call sites above:
#  the assignment in the module is wrapped across two lines, and a single-line
#  match would silently find none -- which would have read as "the assignment is
#  missing" rather than as "the matcher cannot see it".
$idxAssign = [System.Collections.ArrayList]@()
for ($i = 0; $i -lt $srcLines.Count; $i++) {
    $j = Strip $srcLines[$i]
    for ($k = 1; $k -le 2 -and ($i + $k) -lt $srcLines.Count; $k++) {
        $j = $j + ' ' + (Strip $srcLines[$i + $k])
    }
    if ($j -match 'idx\s*=.*Pci1203GearIndex\s*\(.*Pci1203GearAxisBase\s*\(') {
        [void]$idxAssign.Add($i + 1)
    }
}
if (($sdoIndexOk.ContainsKey('idx')) -and $idxAssign.Count -lt 1) {
    Write-Host "FAIL: 'idx' is permitted as an SDO index but no assignment of the form" -ForegroundColor Red
    Write-Host "      idx = Pci1203GearIndex(...) + Pci1203GearAxisBase(...) was found." -ForegroundColor Red
    Write-Host "Without it, 'idx' could name any object and this check would still pass." -ForegroundColor Yellow
    $fail++
} else {
    # ⚠ a COUNT OF WINDOWS, not of assignments -- the 3-line windows overlap, so
    # one wrapped assignment is seen from more than one starting line. The check
    # is ">= 1"; printing it as "assignment(s)" would report a number that does
    # not mean what it says.
    Write-Host ("SDO index 'idx': matched in {0} source window(s) built from the shared gear helpers" -f $idxAssign.Count) -ForegroundColor Green
}

# -----------------------------------------------------------------------------
#  5b. The gear helpers can only produce allowed objects.
#     Every hex literal in EtherCAT/Pci1203Gear.h must be in this set. The file
#     is small and contains nothing but the index arithmetic, which is what
#     makes an exhaustive literal check meaningful here and not elsewhere.
# -----------------------------------------------------------------------------
$gearHdr = Join-Path $repo 'EtherCAT\Pci1203Gear.h'
if (-not (Test-Path $gearHdr)) {
    Write-Host "FAIL: EtherCAT/Pci1203Gear.h is missing -- check 5b cannot run, so the" -ForegroundColor Red
    Write-Host "      bound on computed SDO indices is unproven." -ForegroundColor Red
    $fail++
} else {
    #  0x2700..0x2704 are the objects; 0x800/0x000 are the axis-B offset; 0xF is
    #  a digit mask used when decoding, not an object.
    #AI(W906-1203HOME-1) 20260915: 0x2000 (Pn000, the direction) and 0x000F
    #  (its digit mask) joined the set. Both are named here so a reviewer sees
    #  the widening; 0x000F is a MASK, not an object, and it is what makes the
    #  direction write preserve the three digits it does not own.
    #AI(W906-1203STORE-1) 20260915: 0x1010 (Store Parameters) joins the set.
    #AI(W906-1203ENC-1) 20260916: 0x221D (Pn21D) joins the OBJECT set, and
    #  0x00F0 / 0xA join as a MASK and a DIGIT VALUE -- the same category as
    #  0xF and 0x000F above, not objects.
    #    0x00F0  the n.□□X□ nibble, which is what makes the Pn21D write preserve
    #            the two digits it does not own
    #    0xA     the manual's spelling of the 26-bit selection (p.193). It never
    #            reaches an index: Pci1203EncCompatIndex can only return
    #            0x221D + the axis-B offset, and 0xA appears solely in the
    #            comparison that rejects the values the manual marks Reserved.
    #AI(W906-1203OT-1) 20260915: 0x250A (Pn50A) and 0x250B (Pn50B) join it, and
    #  they are the ones to look at hardest: writing digit 8 into the right nibble
    #  of these two is what makes the drive stop honouring P-OT / N-OT. That is a
    #  SAFETY LIMIT being switched off, on the operator's explicit instruction and
    #  behind a station-number confirmation. Two literals, not a range, so the
    #  helper cannot drift to a neighbouring Pn50x by arithmetic.
    #AI(W906-1203DHOME-1) 20260917: 0x607C / 0x6098 / 0x6099 / 0x609A join the
    #  OBJECT set -- the DS402 homing block. ⚠ This is the first time this file
    #  is allowed to name anything in the 6xxxh profile area, where Target
    #  Position (607Ah) and Controlword (6040h) also live, so it is four exact
    #  literals rather than a range and the self-test above pins 607Ah and 609Bh
    #  as rejects. Why they are needed: the panel's four HOME speed boxes were
    #  writing card properties that a DS402 axis never reads -- measured by
    #  setting them to 1234/4321/111111, homing, and finding 8000/2000/10000 on
    #  the drive. These four are the objects a home on this machine obeys.
    #  ⓘ 0x6098 is here because Pci1203DriveHomeMethodIndex() computes it for the
    #  monitor to READ. It is deliberately absent from $sdoIndexOk, so no write
    #  in Pci1203Control.cpp can reach it.
    $allowedLits = @('0x1010','0x2000','0x221D','0x250A','0x250B','0x2700','0x2701','0x2702','0x2703','0x2704',
                     '0x607C','0x6098','0x6099','0x609A',
                     '0x800','0x000','0xF','0x000F','0x00F0','0xA')
    $badLits = @()
    $ln = 0
    foreach ($l in (Get-Content $gearHdr -Encoding UTF8)) {
        $ln++
        $code = Strip $l
        foreach ($m in [regex]::Matches($code, '0[xX][0-9A-Fa-f]+')) {
            $v = $m.Value
            if ($allowedLits -notcontains $v) { $badLits += ("  Pci1203Gear.h:${ln}: " + $v + " -- " + $l.Trim()) }
        }
    }
    if ($badLits.Count -gt 0) {
        Write-Host "FAIL: EtherCAT/Pci1203Gear.h contains hex literal(s) outside the allowed set:" -ForegroundColor Red
        $badLits | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        Write-Host ("Allowed: " + ($allowedLits -join ', ')) -ForegroundColor Yellow
        Write-Host "A new literal here widens what the computed SDO index can reach, which is" -ForegroundColor Yellow
        Write-Host "exactly what check 5a is relying on this file NOT to do." -ForegroundColor Yellow
        $fail++
    } else {
        Write-Host ("gear helpers: every hex literal in Pci1203Gear.h is in the allowed object set") -ForegroundColor Green
    }
}

# -----------------------------------------------------------------------------
#  5c. The Ex digital-output writes address the module the CARD says owns the
#      byte -- never a station an operator typed.
#
#  AI(W906-1203RING-1) 20260922. This is the rule that REPLACES the outright ban
#  on Acm_DaqDoSetBitEx / Acm_DaqDoSetByteEx (see the note in $forbidden). The
#  ban's worry was real -- the Ex form can energise a coil on any station on any
#  ring -- and deleting it without putting something in its place would be
#  exactly the "widened silently" move this gate exists to prevent.
#
#  The rule: ring and station must come from `d.ring` / `d.station`, where `d`
#  is the monitor's own Pci1203DoSample for the port being written. That value
#  comes from the card's IO map, so the write can only reach the module the card
#  attributes the byte to. An operator supplies the BIT; they cannot supply the
#  address.
#
#  WHAT MAKES IT GO RED: an Ex write whose ring or station argument is anything
#  else -- a literal, a command field, a variable this check cannot see.
$exSites = @()
$exBad   = @()
for ($i = 0; $i -lt $srcLines.Count; $i++) {
    $j = Strip $srcLines[$i]
    if ($j -notmatch '\bAcm_DaqDoSet(Bit|Byte)Ex\s*\(') { continue }
    # join a 3-line window: these calls wrap, like every other call site here
    for ($k = 1; $k -le 2 -and ($i + $k) -lt $srcLines.Count; $k++) {
        $j = $j + ' ' + (Strip $srcLines[$i + $k])
    }
    $exSites += ($i + 1)
    if ($j -notmatch '\(U16\)d\.ring\s*,\s*\(U16\)d\.station') {
        $exBad += ("  Pci1203Control.cpp:" + ($i + 1) + ": " + $srcLines[$i].Trim())
    }
}
if ($exBad.Count -gt 0) {
    Write-Host "FAIL: Ex digital-output write(s) not addressed from the card's own map:" -ForegroundColor Red
    $exBad | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "Ring and station must be (U16)d.ring, (U16)d.station -- the monitor's" -ForegroundColor Yellow
    Write-Host "attribution for that byte. Anything else lets this module energise a" -ForegroundColor Yellow
    Write-Host "coil on a station nobody checked." -ForegroundColor Yellow
    $fail++
} elseif ($exSites.Count -gt 0) {
    Write-Host ("Ex DO writes: {0} call site(s), all addressed from the card's IO map" -f
                $exSites.Count) -ForegroundColor Green
} else {
    #  ⚠ Not silence. Zero sites means the module went back to the flat form,
    #  which is the form that was measured to be aimed by the wrong number --
    #  so it is worth one line rather than looking like a passing check.
    Write-Host "Ex DO writes: none present (module is on the flat form)" -ForegroundColor Yellow
}

# -----------------------------------------------------------------------------
#  5d. THE TWO ENGINE ROUTES MAKE NO VENDOR CALL OF THEIR OWN.
#      AI(W906-ECAT-ROUTE) 20260929 (design docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md 7.3).
#
#  EtherCAT/Pci1203IoRoute.cpp (engine IO) and EtherCAT/Pci1203MotorRoute.cpp +
#  Motor/EcatMotorRoute.h (engine motion) both say in their headers that every
#  write goes through TPci1203Control::Execute and every read is a monitor
#  sample -- i.e. that THIS module's allowlist (check 3) is still the complete
#  list of vendor calls wb_serve can make. Nothing checked that claim until now.
#  Rule: after Strip (comments and string literals removed -- both files NAME
#  the calls they route to, on purpose), no `Acm_<name>(` / `Acm2_<name>(` in
#  code. A route that needs a new vendor call adds it to Pci1203Control.cpp and
#  its allowlist, never here.
# -----------------------------------------------------------------------------
$routeRe    = '\bAcm2?_[A-Za-z0-9_]+\s*\('
$routeFiles = @('EtherCAT\Pci1203IoRoute.cpp', 'EtherCAT\Pci1203IoRoute.h',
                'EtherCAT\Pci1203MotorRoute.cpp', 'EtherCAT\Pci1203MotorRoute.h', 'Motor\EcatMotorRoute.h', 'Motor\EcatMotorRoute.cpp', 'EtherCAT\Pci1203GaliRoute.cpp', 'EtherCAT\Pci1203GaliRoute.h', 'EtherCAT\Pci1203GaliRouteCore.cpp', 'EtherCAT\Pci1203GaliRouteCore.h', 'Motor\GaliRoute.h', 'EtherCAT\Pci1203TorqueHook.cpp')   #AI(W906-INDEXZ) 20260930: + the Index Z1 Gali_* route (INBOX 113)   # AI(W906-ENG1203) 20260929: + the engine-side fCMD bookkeeping (review HIGH-1) -- it too makes no vendor call
if (-not $NoSelfTest) {
    if ((Strip 'r = Acm_AxMoveAbs(ax, 1.0);') -notmatch $routeRe) { Write-Host "SELFTEST FAIL: 5d missed a vendor call in code" -ForegroundColor Red; exit 1 }
    if ((Strip '    // golden: Acm_AxMoveAbs(ax, 1.0) -- routed') -match $routeRe) { Write-Host "SELFTEST FAIL: 5d read a comment as a call" -ForegroundColor Red; exit 1 }
    if ((Strip '    Say(b, p, "x", "Acm_AxHome not sent");') -match $routeRe) { Write-Host "SELFTEST FAIL: 5d read a string literal as a call" -ForegroundColor Red; exit 1 }
}
$routeBad = @()
$routeLines = 0
foreach ($rf in $routeFiles) {
    $rp = Join-Path $repo $rf
    if (-not (Test-Path $rp)) {
        Write-Host "FAIL: route file missing: $rf (check 5d has nothing to prove)" -ForegroundColor Red
        $fail++
        continue
    }
    $ln = 0
    foreach ($l in (Get-Content $rp -Encoding UTF8)) {
        $ln++
        $routeLines++
        if ((Strip $l) -match $routeRe) { $routeBad += ("  ${rf}:${ln}: " + $l.Trim()) }
    }
}
if ($routeBad.Count -gt 0) {
    Write-Host "FAIL: a vendor call in an engine route file (it must go through Pci1203Control::Execute):" -ForegroundColor Red
    $routeBad | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    $fail++
} else {
    Write-Host ("engine routes: {0} file(s), {1} line(s), no Acm_* call in code -- every route write is an Execute()" -f
                $routeFiles.Count, $routeLines) -ForegroundColor Green
}
if ($fail -gt 0) { exit 1 }
# AI(W906-BU-C-P2) 20260918: this line used to end with "and is dry by default",
# which CONTRADICTED the body of the same run. Since the 20260918 user ruling
# ("甲"), WB_PUMP_1203_CONTROL_LIVE is ACTIVE and the gate prints
#   *** LIVE: WB_PUMP_1203_CONTROL_LIVE is ACTIVE (user ruling, 20260918) ***
# a few dozen lines above. A summary line that disagrees with its own body is
# worse than no summary: the body scrolls away, the RESULT line is what gets
# quoted. The gate exits non-zero when measured state != $expectActive, so by
# the time control reaches here the two agree and reading the expectation is
# sound.
$liveWord = if ($expectActive['WB_PUMP_1203_CONTROL_LIVE']) {
    "LIVE -- on a build with HAVE_PCI1203=1, one browser click can move the machine"
} else {
    "dry by default"
}
Write-Host ("RESULT: PASS -- the 1203 write surface matches its allowlist, is opt-in, and is " + $liveWord + ".") -ForegroundColor Green
exit 0
