# AI(W906-HTDESIGNER) 20260929: check media\probe.js in a headless Edge.
# Builds a file:// copy of each page exactly as the designer shows it (same CSP, same
# injection), runs test\probe_driver.js inside it, prints the result.
# Read-only for the tree: the copies and the Edge profile live under %TEMP%.
# The only network the driver tries is 127.0.0.1:9 (closed port), and the CSP must
# block even that -- that is one of the checks.
param(
  [string[]]$Pages = @('Setup.HotPlate.html', 'Setup.Speed.html'),
  [string]$WebPage = '',
  [switch]$Screenshot
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$port = Resolve-Path (Join-Path $here '..\..\..')
if (-not $WebPage) { $WebPage = Join-Path (Split-Path $port -Parent) 'web\page' }
$code = Join-Path $env:LOCALAPPDATA 'Programs\Microsoft VS Code\Code.exe'
$edge = @("${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe", "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $edge) { Write-Host 'msedge.exe not found'; exit 2 }
$work = Join-Path $env:TEMP 'htdesigner_probe'
New-Item -ItemType Directory -Force $work | Out-Null
$profileDir = Join-Path $work 'edge-profile'
$bad = 0
foreach ($p in $Pages) {
  $src = Join-Path $WebPage $p
  # the copy lives in %TEMP%; its <base href> points back at the page folder, so
  # nothing is ever written into web\
  $out = Join-Path $work ('harness_' + [IO.Path]::GetFileNameWithoutExtension($p) + '.html')
  $env:ELECTRON_RUN_AS_NODE = '1'
  $b = Start-Process -FilePath $code -ArgumentList "`"$(Join-Path $here 'build_harness.js')`"", "`"$src`"", "`"$out`"", "`"$(Join-Path $here 'probe_driver.js')`"" -Wait -PassThru -NoNewWindow
  $env:ELECTRON_RUN_AS_NODE = $null
  if ($b.ExitCode -ne 0 -or -not (Test-Path $out)) { Write-Host "[$p] build_harness failed"; $bad++; continue }
  try {
    $url = ([Uri]$out).AbsoluteUri
    $dom = Join-Path $work ($p + '.dom.txt')
    $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
      '--allow-file-access-from-files', '--virtual-time-budget=6000', '--dump-dom', $url)
    $e = Start-Process -FilePath $edge -ArgumentList $eargs -Wait -PassThru -NoNewWindow -RedirectStandardOutput $dom
    $text = Get-Content $dom -Raw -Encoding utf8
    $m = [regex]::Match($text, '<pre id="HTDTEST">HTDTEST(\{.*?\})HTDEND</pre>', 'Singleline')
    if (-not $m.Success) { Write-Host "[$p] no result (edge exit $($e.ExitCode))"; $bad++; continue }
    $json = [System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value)
    [IO.File]::WriteAllText((Join-Path $work ($p + '.result.json')), $json, (New-Object System.Text.UTF8Encoding($false)))
    Write-Host "[$p]"
    $R = $json | ConvertFrom-Json
    ($R | Select-Object -Property * -ExcludeProperty selInfo, treeNodes | Format-List | Out-String -Width 220).Trim()
    # judge, do not just print
    $must = [ordered]@{
      'ready'                      = [bool]$R.ready
      'tree has the form root'     = [bool]$R.treeHasForm
      'design mode blocks clicks'  = [bool]$R.blockedInDesign
      'operate mode operates'      = [bool]$R.operateModeOperates
      'Ctrl+click multi-selects'   = [bool]$R.multiOk
      'group drag = one batch edit' = [bool]$R.groupOk
      'align top'                  = [bool]$R.alignOk
      'Ctrl+drag = copy dropped there (the originals back, no edit)' = [bool]$R.copyDropOk
      'Shift on a corner handle keeps the proportions (Blend / WPF)' = [bool]$R.shiftResizeOk
      'Ctrl+Shift+A clears the selection (WPF)' = [bool]$R.clearSelOk
      'the one asked for not on the page yet: the form shows, said initRoot (the extension keeps asking)' = [bool]$R.initRootOk
      'Alt+click goes one layer down per click (Blend)' = [bool]$R.altCycleOk
      'Shift+drag = a marquee from anywhere, Shift+click still adds (Blend)' = [bool]$R.shiftMarqueeOk
      'Ctrl+Space+click zooms in, Ctrl+Alt+Space+click out (Blend)' = [bool]$R.clickZoomOk
      'F9 hides / shows the element handles (WPF)' = [bool]$R.f9Ok
      'F2 edits the text right on the surface (WPF): Enter writes it, the design keys leave the box alone' = [bool]$R.textBoxOk
      'F2 then Esc writes nothing' = [bool]$R.textBoxEscOk
      'grid shown and snapping apart (WPF two buttons)' = [bool]$R.gridApartOk
      'Ctrl+= / Ctrl+- zoom a step (Blend)' = [bool]$R.zoomStepOk
      'Alt at the release = into the container under the pointer (Blend reparent); the button back, nothing written' = (($R.reparentDropOk -eq $true) -or ($R.reparentDropOk -eq 'no panel'))
      'snaplines on the text baselines (WPF): a small text lands on a big text baseline, dashed guide' = [bool]$R.baselineSnapOk
      'Ctrl+click on a selected one takes it out (no copy)' = [bool]$R.ctrlClickOff
      'Enter opens the event'      = [bool]$R.keyEnterOpens
      'CSP blocked ws://127.0.0.1:9' = [bool](@($R.violations) -match 'ws://127\.0\.0\.1:9').Count
      'no page JS errors'          = (@($R.pageErrors).Count -eq 0)
      'drag moves'                 = [bool]$R.dragOk
      'click is not a drag'        = [bool]$R.clickNoEdit
      'resize handle'              = [bool]$R.resizeOk
      '8 handles drawn'            = ($R.handlesShown -eq 8)
      'arrow nudge = one edit'     = [bool]$R.nudgeOk
      'caption edit'               = [bool]$R.captionOk
      'Esc selects the parent'     = [bool]$R.escParent
      'Tab selects the next'       = [bool]$R.tabNext
      'snaps to a line, guide drawn' = ([bool]$R.snapOk -and [bool]$R.snapGuide)
      'right-click selects'        = [bool]$R.ctxSelects
      'right-click: page blind, menu not blocked' = ([bool]$R.ctxNotBlocked -and [bool]$R.ctxAttr)
      'selectParent'               = [bool]$R.selectParent
      'the form resizes, does not move' = ([bool]$R.formResizeOk -and [bool]$R.formLayoutRoot)
      'Font.Bold / Font.Name'      = ([bool]$R.fontOk -and [bool]$R.lookBold)
      'zoom 200%: 20px drag = 10px' = [bool]$R.zoomDragOk
      'Ctrl+wheel zooms in'        = [bool]$R.wheelZoomOk
      'artboard toolbar (WPF): zoom in / list / 50%, grid, snaplines; the page never sees its clicks' = [bool]$R.toolbarOk
      'zoom 12.5% to 800% (WPF): both ends in the list, + stops at 800%, 12.5% label, below it clamped' = [bool]$R.zoomRangeOk
      'Toggle artboard background (WPF): dark, then the page own again' = [bool]$R.artboardOk
      'nothing picked yet = the form is the selection (WPF: the Properties window shows the Window)' = [bool]$R.initSelectsForm
      'information bar (WPF): a page JS error shown at the top; its links ask for the list / hide it; the page never sees them' = [bool]$R.infoBarOk
    }
    if ($R.PSObject.Properties.Name -contains 'hiddenTarget') { $must['hidden tab revealed'] = [bool]$R.hiddenTabRevealed }
    if ($R.PSObject.Properties.Name -contains 'wrapperEdit') { $must['input moves its wrapper'] = [bool]$R.wrapperOk }
    if ($R.PSObject.Properties.Name -contains 'editMany') { $must['one change on several controls = ONE batch edit'] = [bool]$R.editManyOk }
    if ($R.PSObject.Properties.Name -contains 'editManyLayout') { $must['reset to DFM: position / size in the same batch; a locked one keeps its place, said'] = [bool]$R.editManyLayoutOk }
    if ($R.PSObject.Properties.Name -contains 'editManyForceOk') { $must['reset to DFM (force): the value is written even where the page already shows it; without force nothing'] = [bool]$R.editManyForceOk }
    if ($R.PSObject.Properties.Name -contains 'placeTried') { $must['toolbox placing: crosshair, a drag = point in each container + size, a click = no size, Esc gives up, nothing written'] = [bool]$R.placeOk }
    if ($R.PSObject.Properties.Name -contains 'tabTried') { $must['Tab order: badges in the page order, two swapped = red 1/2 and 2/1, grey for the rest, off = none'] = [bool]$R.tabOk }
    if ($R.PSObject.Properties.Name -contains 'sameTypeTried') { $must['select same type: the whole page / only its container, the selection stays primary'] = [bool]$R.sameTypeOk }
    if ($R.PSObject.Properties.Name -contains 'wrapTried') { $must['WordWrap label: AutoSize read on, on keeps the width; a hidden one refused (never 0 x 0)'] = [bool]$R.wrapOk }
    if ($R.PSObject.Properties.Name -contains 'gapResizeTried') { $must['spacing snap with a handle: the row test uses the real top / bottom'] = [bool]$R.gapResizeOk }
    if ($R.PSObject.Properties.Name -contains 'panelAlignTried') { $must['panel Alignment on its caption span: right written there, centre = declaration removed'] = [bool]$R.panelAlignOk }
    if ($R.PSObject.Properties.Name -contains 'alignTried') { $must['label Alignment written (text-align); AutoSize off with the DFM size = exactly that size'] = [bool]$R.alignOk }
    if ($R.PSObject.Properties.Name -contains 'gapTried') { $must['spacing snapline: lands 8 px from the neighbour on the same row, a blue line while dragging'] = [bool]$R.gapOk }
    if ($R.PSObject.Properties.Name -contains 'autoSizeTried') { $must['AutoSize label: the handle refused with the reason; off = measured size written, on = auto'] = [bool]$R.autoSizeOk }
    if ($R.PSObject.Properties.Name -contains 'zoomSelTried') { $must['zoom to selection: the largest step it fits, and it is in view'] = [bool]$R.zoomSelOk }
    if ($R.PSObject.Properties.Name -contains 'panTried') { $must['the hand: Space + drag and a middle-button drag pan the view, nothing picked or written'] = [bool]$R.panOk }
    if ($R.PSObject.Properties.Name -contains 'ghostTried') { $must['DFM place: dashed frame where the DFM says, labelled, a drag snaps onto it, off = gone'] = [bool]$R.ghostOk }
    if ($R.PSObject.Properties.Name -contains 'groupResize') { $must['a resize handle with 2 selected: both get the same size change, ONE batch edit'] = [bool]$R.groupResizeOk }
    if ($R.PSObject.Properties.Name -contains 'allEdit') { $must['panel with 2 selected: every one, ONE batch edit; a locked one = none; one selected = only it'] = [bool]$R.allOk }
    if ($R.PSObject.Properties.Name -contains 'spaceTried') { $must['equal spacing: outer two stay, gaps equal, the primary moves too'] = [bool]$R.spaceOk }
    if ($R.PSObject.Properties.Name -contains 'centerTried') {
      $must['center in the container: one, then a block of two (their distance kept) = one edit'] = [bool]$R.centerOk
      $must['equal spacing with 2 / with a locked one: nothing, said once each'] = [bool]$R.spaceRefusedOk
    }
    if ($R.PSObject.Properties.Name -contains 'dragTip') { $must['live tip while dragging: left / top of the style'] = [bool]$R.dragTipOk }
    if ($R.PSObject.Properties.Name -contains 'nameTags') {
      $must['name tags on every component'] = [bool]$R.nameTagsOk
      $must['name tags off'] = [bool]$R.nameTagsOffOk
    }
    if ($R.PSObject.Properties.Name -contains 'wireMark') {
      $must['wiring marks: a red frame exactly on the button'] = [bool]$R.wireMarkOk
      $must['wiring marks off: gone'] = [bool]$R.wireMarkOffOk
    }
    if ($R.PSObject.Properties.Name -contains 'zoomFit') { $must['zoom to fit: the form fits the view'] = [bool]$R.zoomFitOk }
    if ($R.PSObject.Properties.Name -contains 'marquee') { $must['rubber band from the form background: what is inside gets selected'] = [bool]$R.marqueeOk }
    if ($R.PSObject.Properties.Name -contains 'rawEdits') { $must['grid HTML rows: style / title written, onclick never'] = [bool]$R.rawOk }
    if ($R.PSObject.Properties.Name -contains 'selAll') { $must['Ctrl+A: the whole container, the button still primary'] = [bool]$R.selAllOk }
    if ($R.PSObject.Properties.Name -contains 'gridTried') {
      $must['grid 8px: a drag and a resize land on multiples of 8, the grid is drawn'] = [bool]$R.gridOk
      $must['grid off: not drawn'] = [bool]$R.gridOffOk
    }
    if ($R.PSObject.Properties.Name -contains 'lockTried') { $must['locked: no drag, no arrows, no handles, said once each'] = [bool]$R.lockOk }
    if ($R.PSObject.Properties.Name -contains 'cmds') { $must['Del / copy / paste -> commands, a doubled paste once'] = [bool]$R.cmdsOk }
    if ($R.PSObject.Properties.Name -contains 'designHideTarget') {
      $must['hidden in the designer: invisible, clicks go through'] = [bool]$R.designHideOk
      $must['shown again, nothing written'] = [bool]$R.designShowOk
    }
    if ($R.PSObject.Properties.Name -contains 'setLayoutKey') {
      $must['setLayout from the panel (by key)'] = [bool]$R.setLayoutKeyOk
      $must['lookAll: a hidden control measured by its style'] = [bool]$R.lookAllOk
      $must['setLayout by id on a hidden control'] = [bool]$R.setLayoutHiddenOk
    }
    if ($R.PSObject.Properties.Name -contains 'anchMoveDom') {
      $must['left+right anchored: move keeps the size'] = [bool]$R.anchMoveOk
      $must['left+right anchored: resize moves only the right edge'] = [bool]$R.anchResizeOk
    }
    if ($R.driverError -or $R.editError) { $must['driver ran'] = $false }
    foreach ($k in $must.Keys) {
      if ($must[$k]) { Write-Host "PASS  $k" } else { Write-Host "FAIL  $k"; $bad++ }
    }
    if ($Screenshot) {
      $png = Join-Path $work ($p + '.png')
      $sargs = @('--headless=new', '--disable-gpu', '--no-first-run', "--user-data-dir=`"$profileDir`"",
        '--allow-file-access-from-files', '--virtual-time-budget=6000', '--window-size=1100,760', "--screenshot=`"$png`"", $url)
      Start-Process -FilePath $edge -ArgumentList $sargs -Wait -NoNewWindow | Out-Null
      Write-Host "screenshot: $png"
    }
  } finally {
    Remove-Item $out -Force -ErrorAction SilentlyContinue
  }
}

# machine type + machine settings (read-only copies): HW.IoSetView opens its Above9050 tab only when it
# knows it is on an HT9050. With the copies (a fixture, not the machine's files) it does; without, it does not.
$ioPage = Join-Path $WebPage 'HW.IoSetView.html'
if (Test-Path $ioPage) {
  $liveJson = Join-Path $work 'live_fixture.json'
  $fixture = '{"machine":"HT9050","docs":{"gerneral":{"path":"fixture","available":true,"sections":{"Version":{"Model":{"value":"HT-9045W","type":"string","raw":"HT-9045W","bcb":"HT-9045W"}}}},"config":{"path":"fixture","available":true,"sections":{}}}}'
  [IO.File]::WriteAllText($liveJson, $fixture, (New-Object System.Text.UTF8Encoding($false)))
  $lr = @{}
  foreach ($case in @('live', 'none')) {
    $out = Join-Path $work ('harness_live_' + $case + '.html')
    $hargs = @("`"$(Join-Path $here 'build_harness.js')`"", "`"$ioPage`"", "`"$out`"", "`"$(Join-Path $here 'live_driver.js')`"")
    if ($case -eq 'live') { $hargs += "`"$liveJson`"" }
    $env:ELECTRON_RUN_AS_NODE = '1'
    $b = Start-Process -FilePath $code -ArgumentList $hargs -Wait -PassThru -NoNewWindow
    $env:ELECTRON_RUN_AS_NODE = $null
    if ($b.ExitCode -ne 0 -or -not (Test-Path $out)) { Write-Host "[live $case] build_harness failed"; $bad++; continue }
    try {
      $dom = Join-Path $work ('live_' + $case + '.dom.txt')
      $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
        '--allow-file-access-from-files', '--virtual-time-budget=8000', '--dump-dom', ([Uri]$out).AbsoluteUri)
      Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow -RedirectStandardOutput $dom | Out-Null
      $text = Get-Content $dom -Raw -Encoding utf8
      $m = [regex]::Match($text, '<pre id="HTDTEST">HTDTEST(\{.*?\})HTDEND</pre>', 'Singleline')
      if ($m.Success) { $lr[$case] = [System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value) | ConvertFrom-Json; Write-Host "[live $case] $($m.Groups[1].Value)" }
      else { Write-Host "[live $case] no result" }
    } finally {
      Remove-Item $out -Force -ErrorAction SilentlyContinue
    }
  }
  Remove-Item $liveJson -Force -ErrorAction SilentlyContinue
  $L = $null; $N = $null; $LO = $null; $NO = $null
  if ($lr['live']) { $L = $lr['live'].design; $LO = $lr['live'].operate }
  if ($lr['none']) { $N = $lr['none'].design; $NO = $lr['none'].operate }
  $lmust = [ordered]@{
    'machine HT9050 given: the page shows it as an HT9050 (html.m-ht9050, data-machine)' = [bool]($L -and $L.mHt9050 -and $L.dataMachine -eq 'HT9050')
    'the Above9050 tab is shown and opened, like on the machine' = [bool]($L -and $L.aboveActive -and $L.aboveShown)
    '/api/system/gerneral and config answered from the copies' = [bool]($L -and (@($L.answered) -contains 'gerneral') -and (@($L.answered) -contains 'config'))
    "the page's JSON/View-rules.json read from the web root's JSON" = [bool]($L -and (@($L.moved) -contains 'View-rules.json'))
    'design mode shows everything: all 5 Stack 1 tabs (the golden ones too, on a 9050), grpLoaderFunc (Visible=False) marked, its own style untouched' = [bool]($L -and $L.tabsAll -eq 5 -and $L.tabsShown -eq 5 -and $L.funcDisplay -ne 'none' -and $L.funcMarked -and $L.funcOwnNone)
    'operate mode = as it runs on the 9050: only Above9050, grpLoaderFunc hidden' = [bool]($LO -and $LO.tabsShown -eq 1 -and $LO.aboveShown -and $LO.funcDisplay -eq 'none' -and -not $LO.funcMarked)
    'without them, design mode: every tab shows too (Above9050 as well)' = [bool]($N -and -not $N.mHt9050 -and $N.tabsShown -eq 5 -and $N.aboveShown -and @($N.answered).Count -eq 0)
    'without them, operate mode: the "no server" face (Above9050 hidden)' = [bool]($NO -and -not $NO.aboveShown -and $NO.tabsShown -eq 4 -and $NO.funcDisplay -eq 'none')
  }
  $VF = $null; $VN = $null
  if ($lr['live']) { $VF = $lr['live'].visOff; $VN = $lr['live'].visOn }
  $lmust['Visible off while designing: display:none written, the component still shows (marked)'] = [bool]($VF -and $VF.edit -eq 'grpLoader2 {"display":"none"}' -and $VF.own -eq 'none' -and $VF.display -ne 'none' -and $VF.marked)
  $lmust['Visible on again: display:none taken out, no mark'] = [bool]($VN -and $VN.edit -eq 'grpLoader2 {"display":null}' -and $VN.own -eq '' -and -not $VN.marked)
  foreach ($k in $lmust.Keys) {
    if ($lmust[$k]) { Write-Host "PASS  $k" } else { Write-Host "FAIL  $k"; $bad++ }
  }
}

# the CSV table (media/csv.js) on its own, with a stubbed acquireVsCodeApi: what it sends is checked
$media = Resolve-Path (Join-Path $here '..\media')
$csvHtml = Join-Path $work 'harness_csv.html'
$u = { param($p) ([Uri](Resolve-Path $p).Path).AbsoluteUri }
# (the extension's policy: style-src with 'unsafe-inline' -- the table places its cells with style="left/top"; without it every cell lands in the corner)
$page = '<!DOCTYPE html><html><head><meta charset="UTF-8"><meta http-equiv="Content-Security-Policy" content="default-src ''none''; style-src file: ''unsafe-inline''; script-src file: ''unsafe-inline''"><link rel="stylesheet" href="' + (& $u (Join-Path $media 'csv.css')) + '"></head><body><div id="root"></div>' +
  '<script>window.__out=[];window.acquireVsCodeApi=function(){return{postMessage:function(m){window.__out.push(JSON.parse(JSON.stringify(m)));},getState:function(){return null;},setState:function(){}};};</script>' +
  '<script src="' + (& $u (Join-Path $media 'csv.js')) + '"></script><script src="' + (& $u (Join-Path $here 'csv_driver.js')) + '"></script></body></html>'
[IO.File]::WriteAllText($csvHtml, $page, (New-Object System.Text.UTF8Encoding($false)))
try {
  $dom = Join-Path $work 'csv.dom.txt'
  $eargs = @('--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', "--user-data-dir=`"$profileDir`"",
    '--allow-file-access-from-files', '--virtual-time-budget=6000', '--window-size=1000,700', '--dump-dom', ([Uri]$csvHtml).AbsoluteUri)
  Start-Process -FilePath $edge -ArgumentList $eargs -Wait -NoNewWindow -RedirectStandardOutput $dom | Out-Null
  $text = Get-Content $dom -Raw -Encoding utf8
  $m = [regex]::Match($text, '<pre id="HTDTEST">HTDTEST(\{.*?\})HTDEND</pre>', 'Singleline')
  $C = $null
  if ($m.Success) { $C = [System.Net.WebUtility]::HtmlDecode($m.Groups[1].Value) | ConvertFrom-Json; Write-Host "[csv] $($m.Groups[1].Value)" } else { Write-Host '[csv] no result' }
  $cmust = [ordered]@{
    'csv table: ready, the first row taken as the header, only the rows in sight drawn (5003 rows)' = [bool]($C -and $C.ready -and $C.header -and $C.virtualOk)
    'csv table: laid out under the page''s policy -- columns side by side, rows one under the other, the header above' = [bool]($C -and $C.layoutOk)
    'csv table: click + type + Enter = one edit of that cell, shown at once, down one row' = [bool]($C -and $C.editorOpen -and $C.editOk -and $C.shownNow -and $C.movedDown)
    'csv table: Shift+arrows select a block, Ctrl+C = its rows x columns' = [bool]($C -and $C.copyOk)
    'csv table: Delete clears the block in ONE edit' = [bool]($C -and $C.clearOk)
    'csv table: paste (Excel text) goes to the extension with the top-left cell' = [bool]($C -and $C.pasteOk)
    'csv table: F2 + Esc writes nothing; Tab / Home move' = [bool]($C -and $C.escOk -and $C.moveOk)
    'csv table: find jumps to the match' = [bool]($C -and $C.findOk)
    'csv table: a file that cannot be written back: typing edits nothing, says why' = [bool]($C -and $C.roOk)
    'csv table: the driver ran' = [bool]($C -and -not $C.driverError)
  }
  foreach ($k in $cmust.Keys) {
    if ($cmust[$k]) { Write-Host "PASS  $k" } else { Write-Host "FAIL  $k"; $bad++ }
  }
} finally {
  Remove-Item $csvHtml -Force -ErrorAction SilentlyContinue
}
exit $bad
