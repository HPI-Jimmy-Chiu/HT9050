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
      'a drag on the empty part of a container not selected = a rubber band over its components (nothing moves); selected, the same drag moves it (WinForms / WPF)' = [bool]$R.containerBandOk
      'Ctrl+Space+click zooms in, Ctrl+Alt+Space+click out (Blend)' = [bool]$R.clickZoomOk
      'F9 hides / shows the element handles (WPF)' = [bool]$R.f9Ok
      'margin adorners (WPF): the selected one''s Left / Top from its container''s edges, the numbers as its style; F9 / the form: none' = [bool]$R.marginsOk
      '0.161 margin adorners on four sides: right / bottom to the container''s inner edges' = [bool]$R.margins4Ok
      'IO lamp / panel button: their own properties read (LEDStyle, Value, colours / Style, Down) and changed (class / CSS-variable edits)' = [bool]$R.ioLookOk
      'F2 edits the text right on the surface (WPF): Enter writes it, the design keys leave the box alone' = [bool]$R.textBoxOk
      'F2 then Esc writes nothing' = [bool]$R.textBoxEscOk
      'grid shown and snapping apart (WPF two buttons)' = [bool]$R.gridApartOk
      'Ctrl+= / Ctrl+- zoom a step (Blend)' = [bool]$R.zoomStepOk
      'Alt at the release = into the container under the pointer (Blend reparent); the button back, nothing written' = (($R.reparentDropOk -eq $true) -or ($R.reparentDropOk -eq 'no panel'))
      'Alt held from the press = only no-snapping: no reparent (VS vs Blend: the same key)' = (($R.reparentDropOk -eq 'no panel') -or [bool]$R.altHeldNoReparent)
      'Esc during a drag = back where it was, nothing written' = [bool]$R.escDragOk
      'select here (Blend Set Current Selection): what is under the right-click, the top one first, the form last' = [bool]$R.stackAtOk
      'the left handle past the right edge: the width stops at 2 px, the right edge stays' = (($R.wPastOk -eq $true) -or ($R.wPastOk -eq 'no handle'))
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
      '0.152 Underline / StrikeOut, ReadOnly / MaxLength / TabOrder, Checked (the input in the label)' = [bool]$R.f152Ok
      '0.156 Tab order by clicks (WinForms View > Tab Order): 1, 2 in click order, the page blind, Esc = done' = [bool]$R.f156Ok
      '0.158 Picture: the look has the src, setLook src = an attr edit, a javascript: one refused' = [bool]$R.f158Ok
      'zoom 200%: 20px drag = 10px' = [bool]$R.zoomDragOk
      'Ctrl+wheel zooms in'        = [bool]$R.wheelZoomOk
      'zoomWheel setting (WPF Zoom by using): Alt+wheel / the wheel alone zoom, Ctrl+wheel then only scrolls' = [bool]$R.wheelHowOk
      'the hint never covers the artboard toolbar; the zoom buttons name the wheel in use' = [bool]$R.hintBesideOk
      'artboard toolbar (WPF): zoom in / list / 50%, grid, snaplines; the page never sees its clicks' = [bool]$R.toolbarOk
      'zoom 12.5% to 800% (WPF): both ends in the list, + stops at 800%, 12.5% label, below it clamped' = [bool]$R.zoomRangeOk
      'Toggle artboard background (WPF): dark, then the page own again' = [bool]$R.artboardOk
      'show element bounds (Visual Studio / Blend): every component dashed, nothing moved, told to the extension; another page turning it off = off here' = [bool]$R.boundsOk
      '0.155 machine screen frame (WPF DesignWidth/Height): the button asks, setScreen draws from the form corner, null hides' = [bool]$R.screenOk
      'nothing picked yet = the form is the selection (WPF: the Properties window shows the Window)' = [bool]$R.initSelectsForm
      'information bar (WPF): a page JS error shown at the top; its links ask for the list / hide it; the page never sees them' = [bool]$R.infoBarOk
    }
    if ($R.PSObject.Properties.Name -contains 'hiddenTarget') { $must['hidden tab revealed'] = [bool]$R.hiddenTabRevealed }
    if ($R.PSObject.Properties.Name -contains 'wrapperEdit') { $must['input moves its wrapper'] = [bool]$R.wrapperOk }
    if ($R.PSObject.Properties.Name -contains 'editMany') { $must['one change on several controls = ONE batch edit'] = [bool]$R.editManyOk }
    if ($R.PSObject.Properties.Name -contains 'editManyLayout') { $must['reset to DFM: position / size in the same batch; a locked one keeps its place, said'] = [bool]$R.editManyLayoutOk }
    if ($R.PSObject.Properties.Name -contains 'editManyForceOk') { $must['reset to DFM (force): the value is written even where the page already shows it; without force nothing'] = [bool]$R.editManyForceOk }
    if ($R.PSObject.Properties.Name -contains 'placeTried') { $must['toolbox placing: crosshair, a drag = point in each container + size, a click = no size, Esc gives up, nothing written'] = [bool]$R.placeOk; $must['toolbox placing with grid snapping on: the point and the size land on the grid (WPF)'] = [bool]$R.placeGridOk }
    if ($R.PSObject.Properties.Name -contains 'placeSticky') { $must['toolbox placing: armed to stay = every click places one (C++Builder Shift+tool); Shift while placing keeps it; Ctrl held = selecting for the moment (Blend); Esc puts it down'] = [bool]$R.placeStickyOk }
    if ($R.PSObject.Properties.Name -contains 'note') { $must['design note (Blend annotation): a yellow sticky at the component, gone when taken off'] = [bool]$R.noteOk }
    if ($R.PSObject.Properties.Name -contains 'crumbs') { $must['breadcrumb (Blend): the selection path at the top, a crumb selects that layer'] = [bool]$R.crumbsOk }
    if ($R.PSObject.Properties.Name -contains 'measure') { $must['measuring (Figma/Blend): Alt over another component shows the gaps in px, gone when Alt is let go'] = [bool]$R.measureOk }
    if ($R.PSObject.Properties.Name -contains 'guides') { $must['rulers and guides (Blend): a guide dragged out of the ruler, a component snaps onto it, back onto the ruler = gone'] = [bool]$R.guidesOk }
    if ($R.PSObject.Properties.Name -contains 'pcCtx') { $must['page control right-click (EastSun): the menu context names the PageControl and the sheet (tab / component on it / the one shown), none elsewhere'] = [bool]$R.pcCtxOk }
    if ($R.PSObject.Properties.Name -contains 'ledCap') { $must['labeled LED caption = its .lledCap (not typed into the lamp), an unlabeled LED has none; TLabeledEdit EditLabel.Caption = its .elab'] = [bool]$R.ledCapOk }
    if ($R.PSObject.Properties.Name -contains 'smartTag') { $must['smart tag (WinForms): the triangle at the selected one, a click = its tasks asked for; none with two selected'] = [bool]$R.smartTagOk }
    if ($R.PSObject.Properties.Name -contains 'sizeOp') { $must['Size / Scale (C++Builder): grow to the largest + a number, then 200% (position and size), each ONE edit'] = [bool]$R.sizeOpOk }
    if ($R.PSObject.Properties.Name -contains 'autoScroll') { $must['a drag held at the view edge scrolls the page and the component follows (WinForms / WPF)'] = [bool]$R.autoScrollOk }
    if ($R.PSObject.Properties.Name -contains 'tabTried') { $must['Tab order: badges in the page order, two swapped = red 1/2 and 2/1, grey for the rest, off = none'] = [bool]$R.tabOk }
    if ($R.PSObject.Properties.Name -contains 'sameTypeTried') { $must['select same type: the whole page / only its container, the selection stays primary'] = [bool]$R.sameTypeOk }
    if ($R.PSObject.Properties.Name -contains 'wrapTried') { $must['WordWrap label: AutoSize read on, on keeps the width; a hidden one refused (never 0 x 0)'] = [bool]$R.wrapOk }
    if ($R.PSObject.Properties.Name -contains 'gapResizeTried') { $must['spacing snap with a handle: the row test uses the real top / bottom'] = [bool]$R.gapResizeOk }
    if ($R.PSObject.Properties.Name -contains 'panelAlignTried') { $must['panel Alignment on its caption span: right written there, centre = declaration removed'] = [bool]$R.panelAlignOk }
    if ($R.PSObject.Properties.Name -contains 'alignTried') { $must['label Alignment written (text-align); AutoSize off with the DFM size = exactly that size'] = [bool]$R.alignOk }
    if ($R.PSObject.Properties.Name -contains 'gapTried') { $must['spacing snapline: lands 8 px from the neighbour on the same row, a blue line while dragging'] = [bool]$R.gapOk }
    if ($R.PSObject.Properties.Name -contains 'padSnap') { $must['padding snapline (WinForms): dragged to 10 px from the container edge, it lands at 8 px'] = [bool]$R.padSnapOk }
    if ($R.PSObject.Properties.Name -contains 'gapSettingTried') { $must['snapSpacing setting (WPF snapping spacing): 12 lands 12 px away, 0 = no spacing snap'] = [bool]$R.gapSettingOk }
    if ($R.PSObject.Properties.Name -contains 'autoSizeTried') { $must['AutoSize label: the handle refused with the reason; off = measured size written, on = auto'] = [bool]$R.autoSizeOk }
    if ($R.PSObject.Properties.Name -contains 'zoomSelTried') { $must['zoom to selection: the largest step it fits, and it is in view'] = [bool]$R.zoomSelOk }
    if ($R.PSObject.Properties.Name -contains 'panTried') { $must['the hand: Space + drag and a middle-button drag pan the view, nothing picked or written'] = [bool]$R.panOk }
    if ($R.PSObject.Properties.Name -contains 'ghostTried') { $must['DFM place: dashed frame where the DFM says, labelled, a drag snaps onto it, off = gone'] = [bool]$R.ghostOk }
    if ($R.PSObject.Properties.Name -contains 'groupResize') { $must['a resize handle with 2 selected: both get the same size change, ONE batch edit'] = [bool]$R.groupResizeOk }
    if ($R.PSObject.Properties.Name -contains 'allEdit') { $must['panel with 2 selected: every one, ONE batch edit; a locked one = none; one selected = only it'] = [bool]$R.allOk }
    if ($R.PSObject.Properties.Name -contains 'gridCmd') { $must['Format (WinForms / BCB6): Align to Grid / Size to Grid = multiples of the grid'] = [bool]$R.gridCmdOk; $must['Format: spacing Increase = one grid block, Remove = side by side, the primary one stays'] = [bool]$R.spaceStepOk }
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
      $must['an AutoSize label (width:auto) moves by Left / Top -- its size not needed for a move (1006)'] = [bool]$R.autoLabelMoveOk
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
    'design mode, a tab a page RULE hides (Above9050, .ht9050-only, no machine): shown AND marked; what is on its sheet (grpLoader_9050) = hidden-when-running in the tree; on the 9050 neither' = [bool]($N -and $N.aboveMarked -and $N.g9050Row -eq 1 -and $L -and -not $L.aboveMarked -and $L.g9050Row -eq 0)
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
  '<script>window.__out=[];window.acquireVsCodeApi=function(){return Object.freeze({postMessage:function(m){window.__out.push(JSON.parse(JSON.stringify(m)));},getState:function(){return null;},setState:function(){}});};</script>' +
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
    'csv table: paste (Excel text) goes to the extension with the selected block (top-left and bottom-right)' = [bool]($C -and $C.pasteOk)
    'csv table: F2 + Esc writes nothing; Tab / Home move' = [bool]($C -and $C.escOk -and $C.moveOk)
    'csv table: find jumps to the match' = [bool]($C -and $C.findOk)
    'csv table: a double click (the second press''s click count) = the editor, a caret (Excel: where it was clicked)' = [bool]($C -and $C.dblOk)
    'csv table: Ctrl+X = copied now, moved when pasted (the paste carries the cut block), nothing edited before' = [bool]($C -and $C.cutOk)
    'csv table: an IME word goes into the cell; the IME''s own Enter picks the word, a second Enter writes it' = [bool]($C -and $C.imeOk)
    'csv table: a file that cannot be written back: typing edits nothing, says why' = [bool]($C -and $C.roOk)
    'csv table, right-click menu (Excel): inside the block it stays, every item with its key; clear contents = ONE edit; outside = that cell' = [bool]($C -and $C.menuOk -and $C.menuClear -and $C.menuMove)
    'csv table, right-click menu: Esc closes it; Shift+F10 opens it, Down / Enter runs an item (Copy), the keys back with the table' = [bool]($C -and $C.menuEsc -and $C.menuKeys)
    'csv table, right-click menu: Paste asks the extension for the clipboard, it goes in at the selected cell; read only = the changing items off' = [bool]($C -and $C.menuReadClip -and $C.menuPaste -and $C.menuRoOk)
    'csv table, Name Box (Excel): shows the active cell; Ctrl+G = into it; B5:C7 + Enter = that block; a place not in the table = said, nothing moved; Esc; 12 = row 12' = [bool]($C -and $C.nameBoxOk)
    'csv table, AutoFit (Excel): a double click on a column boundary = as wide as its longest value (nothing cut off)' = [bool]($C -and $C.autoFitOk)
    'csv table, Alt+Down (Excel pick list): this column''s values once each (numbers by value, then A-Z), not the header / its own; Down + Enter = that one, ONE edit; typed first = the ones starting with it; Esc closes the list only' = [bool]($C -and $C.pickList -eq '9,12,Mark,Motor' -and $C.pickSet -eq '12' -and $C.pickClosed -and $C.pickTyped -eq 'Mark,Motor' -and $C.pickEscKeepsEdit)
    'csv table, AutoFilter (Excel, 1006): the column button lists its values with counts; IO unticked + OK = those rows hidden (row numbers kept), the count said, the column marked' = [bool]($C -and $C.filter -and $C.filter.listOk -and $C.filter.shown -eq '[1,3,5,6]' -and $C.filter.numsOk -and $C.filter.sizeOk -and $C.filter.closed -and $C.filter.mark)
    'csv table, AutoFilter: Down steps over hidden rows; Ctrl+C / Delete = the rows in sight only; Ctrl+X and a paste running into a hidden row = refused, said' = [bool]($C -and $C.filter -and $C.filter.down -eq 3 -and $C.filter.copy -eq '[["a1","Motor"],["a3","Motor"]]' -and $C.filter.del -eq '1:0 1:1 3:0 3:1' -and $C.filter.cutRefused -and $C.filter.pasteRefused)
    'csv table, AutoFilter: Ctrl+- on whole rows = only the rows in sight, ONE message (bottom run first); the rows in sight follow the delete' = [bool]($C -and $C.filter -and $C.filter.delRows -eq '[{"r":3,"count":1},{"r":1,"count":1}]' -and $C.filter.afterDel -eq '[3,4]')
    'csv table, AutoFilter: an edited row stays in sight until Ctrl+Alt+L (Reapply); Ctrl+Shift+L clears; Alt+Down on the title = the list, a search + Enter = the ones found' = [bool]($C -and $C.filter -and $C.filter.kept -eq '[3,4]' -and $C.filter.reapplied -eq '[3]' -and $C.filter.cleared -and $C.filter.altDown -eq 1 -and $C.filter.searchList -eq 'IO' -and $C.filter.searched -eq '[1,2,4]')
    'csv table, duplicate values (Excel): the column menu marks the values in two rows or more, the status bar counts them, off again' = [bool]($C -and $C.filter -and $C.filter.dupOk)
    'csv table, sort (Excel): the filter window''s sort buttons send sortRows for that column (the extension asks before it writes)' = [bool]($C -and $C.filter -and $C.filter.sortOk)
    'csv table, fill series (Excel): from the first two values (step) or one (+1), a trailing number in text too (M02 M03, X0 X1), down or to the right, ONE edit' = [bool]($C -and $C.filter -and $C.filter.seriesOk)
    'csv table, freeze columns (Excel): the menu freezes up to the column; scrolled sideways, the frozen one stays in sight, the others move' = [bool]($C -and $C.filter -and $C.filter.freezeOk)
    'csv table, insert / delete columns (Excel): a whole column selected, Ctrl+Shift++ = insertCols before it, Ctrl+- = deleteCols; a few cells = refused' = [bool]($C -and $C.filter -and $C.filter.colsOk)
    'csv table, error checking (Excel): text in a column of numbers = an orange corner, counted in the status bar, the menu goes to it' = [bool]($C -and $C.filter -and $C.filter.oddOk)
    'csv table: the driver ran' = [bool]($C -and -not $C.driverError)
  }
  foreach ($k in $cmust.Keys) {
    if ($cmust[$k]) { Write-Host "PASS  $k" } else { Write-Host "FAIL  $k"; $bad++ }
  }
} finally {
  Remove-Item $csvHtml -Force -ErrorAction SilentlyContinue
}

# the CSV table with REAL input (DevTools protocol: a real double click, real keys, an IME word) -- the
# synthetic events above passed while a real double click / a Chinese input method wrote nothing (20261001)
$realOut = Join-Path $work 'csv_realinput.json'
Remove-Item $realOut -Force -ErrorAction SilentlyContinue
$env:ELECTRON_RUN_AS_NODE = '1'
$rp = Start-Process -FilePath $code -ArgumentList "`"$(Join-Path $here 'csv_realinput.js')`"", "`"$media`"", "`"$realOut`"" -Wait -PassThru -NoNewWindow
$env:ELECTRON_RUN_AS_NODE = $null
$Q = $null
if (Test-Path $realOut) { $raw = Get-Content $realOut -Raw -Encoding utf8; Write-Host "[csv real input] $raw"; $Q = $raw | ConvertFrom-Json } else { Write-Host "[csv real input] no result (exit $($rp.ExitCode))" }
$qmust = [ordered]@{
  'csv table, real input: a double click opens the cell''s editor, the caret where it was clicked' = [bool]($Q -and $Q.dblOk)
  'csv table, real input: click + keys = typed over the cell, Enter = one edit, the keys stay with the table' = [bool]($Q -and $Q.typedOk)
  'csv table, real input: an IME word (Chinese input method) goes into the cell, Enter writes it' = [bool]($Q -and $Q.imeOk)
  'csv table, real input: a click on the empty part keeps the keys with the table' = [bool]($Q -and $Q.emptyClickOk)
  'csv table, real input (Excel): Ctrl+Enter = the value into every selected cell, the block and the active cell stay' = [bool]($Q -and $Q.ctrlEnterOk)
  'csv table, real input (Excel): Ctrl+D = the first row of the block into the rest' = [bool]($Q -and $Q.fillDownOk)
  'csv table, real input (Excel): Shift+Space = the row (no space typed); Ctrl+- deletes only whole rows, else says how' = [bool]($Q -and $Q.rowKeysOk)
  'csv table, real input (Excel): typed over, an arrow writes it and moves' = [bool]($Q -and $Q.typedArrowOk)
  'csv table, real input (Excel): F2 = the caret at the end; F2 again = Enter / Edit mode' = [bool]($Q -and $Q.f2Ok)
  'csv table, real input (Excel): Backspace = the cell from empty, Esc gives it back' = [bool]($Q -and $Q.backspaceOk)
  'csv table, real input (Excel): Enter inside a block moves the active cell, the block stays' = [bool]($Q -and $Q.enterInBlockOk)
  'csv table, real input: the empty row under the last one -- typing there adds a row' = [bool]($Q -and $Q.newRowOk)
  'csv table, real input (Excel): Ctrl+H replace all, whole cell = only the exact ones, ONE edit' = [bool]($Q -and $Q.replaceOk)
  'csv table, real input: a right click opens the menu on that cell; Down, Down, Enter = Copy; a click on clear contents = ONE edit; Shift+F10 / Esc' = [bool]($Q -and $Q.menuRealOk)
  'csv table, real input: Ctrl+G, typed c3, Enter = C3 active, the keys back with the table (the Name Box)' = [bool]($Q -and $Q.nameBoxRealOk)
  'csv table, real input: a double click on a column boundary = AutoFit (nothing cut off)' = [bool]($Q -and $Q.autoFitRealOk)
  'csv table, real input: the driver ran' = [bool]($Q -and -not $Q.error)
}
foreach ($k in $qmust.Keys) {
  if ($qmust[$k]) { Write-Host "PASS  $k" } else { Write-Host "FAIL  $k"; $bad++ }
}
exit $bad
