# redirect_mak.ps1
# Version: 1.0.0 | 2026-09-09
# Purpose: rewrite a bpr2mak-generated Makefile so the build writes into
#          tree-private Obj<tag>/Out<tag> instead of the SHARED D:\HT9045\Obj
#          and D:\HT9045\EXE where the production HT9045.exe lives.
#
# Why this exists: every HT9011UC_Code_* tree's .bpr points PROJECT at
# D:\HT9045\EXE\HT9045.exe and all obj/PCH at D:\HT9045\Obj. Building any
# non-shipping tree therefore overwrites the shipping EXE and poisons the
# shared PCH cache. The V910 campaign worked around this by hand-editing the
# .mak after bpr2mak; that was never encoded anywhere, so nothing carried
# over to V912. This script encodes it, and build_bcb.bat calls it by
# DEFAULT -- forgetting now yields a safe private build, not a clobbered
# production EXE. Pass OUT_MODE=shared to build_bcb.bat to opt out.
#
# The file is read and written through Latin-1 so arbitrary 8-bit bytes
# (Big5 comments) round-trip unchanged; only the path substrings change.

param(
    [Parameter(Mandatory = $true)][string]$MakPath,
    [Parameter(Mandatory = $true)][string]$ProjectDir,
    [string]$Tag = ""
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $MakPath)) {
    Write-Host "[redirect_mak] ERROR: makefile not found: $MakPath"
    exit 1
}

$ProjectDir = (Resolve-Path -LiteralPath $ProjectDir).Path.TrimEnd('\')

if (-not $Tag) {
    $leaf = Split-Path -Leaf $ProjectDir
    $m = [regex]::Match($leaf, 'V\d+\.\d+\.(\d+)')
    if ($m.Success) { $Tag = $m.Groups[1].Value } else { $Tag = "Local" }
}

$objDir = Join-Path $ProjectDir ("Obj" + $Tag)
$outDir = Join-Path $ProjectDir ("Out" + $Tag)
New-Item -ItemType Directory -Force -Path $objDir | Out-Null
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$latin1 = [Text.Encoding]::GetEncoding(28591)
$raw = [IO.File]::ReadAllText($MakPath, $latin1)

function Replace-Literal {
    param([string]$Text, [string]$Find, [string]$ReplaceWith)
    $ev = [Text.RegularExpressions.MatchEvaluator] { param($m) $ReplaceWith }
    return [regex]::Replace($Text, [regex]::Escape($Find), $ev, 'IgnoreCase')
}

$nObjAbs = ([regex]::Matches($raw, [regex]::Escape('D:\HT9045\Obj'), 'IgnoreCase')).Count
$nExeAbs = ([regex]::Matches($raw, [regex]::Escape('D:\HT9045\EXE'), 'IgnoreCase')).Count
$nObjRel = ([regex]::Matches($raw, [regex]::Escape('..\Obj'), 'IgnoreCase')).Count
$nExeRel = ([regex]::Matches($raw, [regex]::Escape('..\EXE'), 'IgnoreCase')).Count

# Absolute forms first, then relative -- order matters.
$raw = Replace-Literal -Text $raw -Find 'D:\HT9045\Obj' -ReplaceWith $objDir
$raw = Replace-Literal -Text $raw -Find 'D:\HT9045\EXE' -ReplaceWith $outDir
$raw = Replace-Literal -Text $raw -Find '..\Obj'        -ReplaceWith $objDir
$raw = Replace-Literal -Text $raw -Find '..\EXE'        -ReplaceWith $outDir

[IO.File]::WriteAllText($MakPath, $raw, $latin1)

# --- verification gate: nothing may still point at the shared roots ---
$check = [IO.File]::ReadAllText($MakPath, $latin1)
$leakObj = ([regex]::Matches($check, [regex]::Escape('D:\HT9045\Obj'), 'IgnoreCase')).Count
$leakExe = ([regex]::Matches($check, [regex]::Escape('D:\HT9045\EXE'), 'IgnoreCase')).Count
$leakRel = ([regex]::Matches($check, '(?<![A-Za-z0-9_.])\.\.\\(Obj|EXE)\\', 'IgnoreCase')).Count
$projLine = ([regex]::Match($check, '(?m)^PROJECT\s*=\s*(.+)$')).Groups[1].Value.Trim()

Write-Host "[redirect_mak] tag     : $Tag"
Write-Host "[redirect_mak] obj dir : $objDir"
Write-Host "[redirect_mak] out dir : $outDir"
Write-Host ("[redirect_mak] rewrote : {0}x 'D:\HT9045\Obj'  {1}x '..\Obj'  {2}x 'D:\HT9045\EXE'  {3}x '..\EXE'" -f $nObjAbs, $nObjRel, $nExeAbs, $nExeRel)
Write-Host "[redirect_mak] PROJECT : $projLine"

if ($leakObj -gt 0 -or $leakExe -gt 0 -or $leakRel -gt 0) {
    Write-Host "[redirect_mak] ERROR: makefile still references the shared roots (Obj=$leakObj EXE=$leakExe rel=$leakRel)"
    exit 1
}
if (-not $projLine.StartsWith($outDir, [System.StringComparison]::OrdinalIgnoreCase)) {
    Write-Host "[redirect_mak] ERROR: PROJECT does not point into $outDir"
    exit 1
}

Write-Host "[redirect_mak] OK - shared D:\HT9045\Obj and D:\HT9045\EXE will not be touched"
exit 0
