# -*- coding: utf-8 -*-
"""Build page/Main.MotionView9050.html from the concept page + JSON/machine-gate layer.
Re-runnable: always regenerates from IDE.MotionView9050-Concept.html."""
import io, re, sys
SRC = r'd:\HT9045\page\IDE.MotionView9050-Concept.html'
DST = r'd:\HT9045\page\Main.MotionView9050.html'
s = io.open(SRC, encoding='utf-8').read()

def rep(old, new, cnt=1):
    global s
    assert s.count(old) >= 1, 'NOT FOUND: ' + old[:90]
    s = s.replace(old, new, cnt)

def cut(start_marker, end_marker, new, include_end=True):
    global s
    a = s.index(start_marker); b = s.index(end_marker, a)
    if include_end: b += len(end_marker)
    s = s[:a] + new + s[b:]

# ---------- head ----------
rep('<title>【概念圖】HT9050 (HP-9050) 整機 Motion View — 版面提案</title>',
    '<title>Motion View — HT9050 (HP-9050)</title>\n<link rel="stylesheet" href="theme.css">')
rep('/* 自帶完整色票：不依賴 theme.css / theme.js，也不載入任何 JSON —— 直接雙擊即可看到內容 */',
    '/* 色票沿用概念頁；--cpanel 避免與 theme.css 的 --panel 衝突。本頁由 build 腳本自 IDE.MotionView9050-Concept.html 產生 */')
s = s.replace('--panel:', '--cpanel:').replace('var(--panel)', 'var(--cpanel)')

# 核心色票改吃 theme.css design token（與 Main.MotionView.html 同一組對應）；井號值只留作 theme.css 沒載到時的 fallback
rep('  --bg:#eef1f5; --cpanel:#ffffff; --ink:#1b2430; --dim:#6b7787; --line:#a9b4c2; --line2:#c6cedb;',
    '  /* 核心色票＝theme.css design token：隨 <html data-theme> 切 classic／dark／steel／contrast；'
    '井號值只是 theme.css 未載入時的 fallback，勿改回寫死色 */\n'
    '  --bg:var(--form-bg,#eef1f5); --cpanel:var(--panel,#ffffff); --ink:var(--text,#1b2430);\n'
    '  --dim:var(--text-dim,#6b7787); --line:var(--border,#a9b4c2); --line2:var(--pane-border,#c6cedb);')
rep('  font-family:"Microsoft JhengHei","Segoe UI",sans-serif;font-size:13px;line-height:1.5}',
    '  font-family:var(--font-ui,"Microsoft JhengHei","Segoe UI",sans-serif);font-size:13px;line-height:1.5}')
rep('.tickt{fill:var(--dim);font:9px Consolas,monospace;text-anchor:middle}\n</style>',
    '.tickt{fill:var(--dim);font:9px Consolas,monospace;text-anchor:middle}\n'
    '/* 機種閘門／資料來源狀態 */\n'
    '#gate{display:none}\n'
    'body.gated .wrap.main{display:none}\n'
    'body.gated #gate{display:block}\n'
    'html[data-mode="release"] .debugOnly{display:none}\n'
    '.simctl{display:flex;flex-wrap:wrap;gap:7px;align-items:center}\n'
    '.lead.src{border-left-color:var(--good)}\n'
    '.lead.src.warn{border-left-color:var(--warn)}\n'
    '.lead.src.err{border-left-color:var(--hot)}\n'
    '.note.warn{border-left-color:var(--warn);color:var(--ink)}\n'
    '.note.live{border-left-color:var(--good);font-weight:600}\n'
    '/* dark：立體圖底板／格點／語意色不在 theme.css token 內，這裡補一組壓暗值 */\n'
    ':root[data-theme="dark"]{\n'
    '  --mech:#8b95a8; --rail:#6d7484; --acc:#6f9fe8;\n'
    '  --ic:#4d8ae0; --ic2:#3fae70; --s0:#3aa0b4; --s1:#a086cf;\n'
    '  --test:#e0a83a; --idx2:#a086cf; --hot:#e05c4e; --warn:#e0a83a; --good:#4bbd7a;\n'
    '  --cell:#2f353c; --cellE:#343a42; --hl:#e0a83a;\n'
    '  --plate:#3a4048; --plate2:#333941; --plate3:#2c323a;\n'
    '  --tint-in:rgba(77,138,224,.10); --tint-tst:rgba(224,168,58,.10); --tint-out:rgba(63,174,112,.10);\n'
    '  --shadow:0 1px 3px rgba(0,0,0,.45);\n'
    '}\n'
    '</style>')

# ---------- body: lead first (before the gate adds its own <p class="lead concept">) ----------
cut('<p class="lead concept">', '</p>',
    '<p class="lead src" id="srcLead">正在載入 Machine-profile.json、MotionView9050-layout.json、Setup-current.json 與 Production-update.json…</p>')
rep('<body>\n<div class="wrap">',
    '<body>\n'
    '<div id="gate" class="wrap">\n'
    '  <header class="top"><div><h1>HT9050 (HP-9050) 整機 Motion View</h1>\n'
    '    <div class="meta">此頁僅在機種判定為 <span class="mono">HT9050</span> 時顯示</div></div></header>\n'
    '  <p class="lead concept"><b id="gateMsg">機種判定中…</b><br>\n'
    '    機種由 <span class="mono">Machine-profile.json</span> 依 <span class="mono">?machine=</span> → 啟動選項 →\n'
    '    <span class="mono">Gerneral.ini [Version] Model</span> 前綴（HT-9050／HP-9050）→ localStorage → default 決定。<br>\n'
    '    HT9045／HT9046 請用 <a href="Main.MotionView.html">Main.MotionView.html</a>；要強制檢視本頁可加 <a id="gateForce" href="?machine=HT9050">?machine=HT9050</a>。</p>\n'
    '</div>\n'
    '<div class="wrap main">')
rep('''    <h1>【概念圖】HT9050 (HP-9050) 整機 Motion View</h1>
    <div class="meta">
      比照 <span class="mono">Main.MotionView.html</span> 的斜投影立體視圖與下方欄位配置，
      改成 HP-9050 機構（1 Picker＋內建 rotator／In・Out Shuttle 獨立馬達／Index 僅 1 個 Z 軸／5 軌）。
    </div>
  </div>
  <div class="meta">概念稿 v1　2026-09-09</div>''',
'''    <h1>HT9050 (HP-9050) 整機 Motion View</h1>
    <div class="meta">
      斜投影立體視圖＋下方欄位（與 <span class="mono">Main.MotionView.html</span> 同契約）；
      HP-9050 機構：1 Picker＋內建 rotator／In・Out Shuttle 獨立馬達（Out Shuttle X＋Y）／Index 單 Z／Out P&amp;P Y 懸臂氣缸／5 軌。
    </div>
  </div>
  <div class="meta"><span id="mvMachineTag" class="tag">machine=?</span>　<span id="modeTag" class="tag">等待 JSON</span></div>''')
# 正式頁載入後不自己動：SIM 控制列只在 debug 模式出現，且預設停在起始畫面
rep('''    <button id="btnPlay" type="button">⏸ 暫停</button>
    <button id="btnStep" type="button">⏭ 下一步</button>
    <button id="btnReset" type="button">⟲ 重置</button>
    <label style="display:flex;align-items:center;gap:4px">速度
      <select id="selSpeed"><option value="0.5">0.5×</option><option value="1" selected>1×</option>
        <option value="2">2×</option><option value="4">4×</option></select></label>
    <label style="display:flex;align-items:center;gap:4px" title="勾選後 Out Shuttle 出測區時才會走 Y 行程到 TOP AOI 拍攝">
      <input type="checkbox" id="chkTopAOI">TOP AOI（Option）</label>''',
'''    <span class="simctl debugOnly" title="SIM 手動播放：本頁載入後不自己動，畫面動不動由 Production-update.json 決定">
      <button id="btnPlay" type="button" class="on">▶ 播放</button>
      <button id="btnStep" type="button">⏭ 下一步</button>
      <button id="btnReset" type="button">⟲ 重置</button>
      <label style="display:flex;align-items:center;gap:4px">速度
        <select id="selSpeed"><option value="0.5">0.5×</option><option value="1" selected>1×</option>
          <option value="2">2×</option><option value="4">4×</option></select></label>
      <label style="display:flex;align-items:center;gap:4px" title="勾選後 Out Shuttle 出測區時才會走 Y 行程到 TOP AOI 拍攝">
        <input type="checkbox" id="chkTopAOI">TOP AOI（Option）</label>
    </span>''')
rep('<span class="tag">動畫為<b>示意</b>：秒數、行程、配位皆非實測</span>',
    '<span class="tag" id="simTag">畫面不自己動：有 Runtime 快照就投影，沒有就停在起始畫面（debug 可手動跑 SIM，秒數取自 flow.steps[].sec）</span>')
cut('  <p class="note" style="margin:0 0 8px">\n    <b>實作後這裡會顯示：</b>', '</p>',
    '  <p class="note" id="paramNote" style="margin:0 0 8px">—</p>')
rep('<p class="note runtime-status">等待 C++ Runtime 在 <span class="mono">Production-update.json.state.motionView</span> 發布逐格快照。</p>',
    '<p class="note runtime-status" id="liveStatus">等待 C++ Runtime 在 <span class="mono">Production-update.json.state.motionView</span> 發布逐格快照。</p>')
rep('<div class="tray-location-title"><b>Tray Up / Car Status</b><span>等待 Runtime</span></div>',
    '<div class="tray-location-title"><b>Tray Up / Car Status</b><span id="trayLocSrc">等待 Runtime</span></div>')

# ---------- docs folds + footer ----------
cut('<details class="fold" open><summary>與 HT9045 版', '</footer>',
'''<details class="fold"><summary>資料來源與契約</summary>
<section class="panel scroll">
  <table class="doc">
    <thead><tr><th>項目</th><th>來源</th><th>目前狀態</th></tr></thead>
    <tbody id="srcTable"></tbody>
  </table>
  <p class="note">機種判定：<span class="mono">?machine=</span> → 啟動選項 → <span class="mono">Gerneral.ini [Version] Model</span> 前綴（HT-9050／HP-9050）→ localStorage → default。非 HT9050 時本頁只顯示提示，不繪圖。</p>
  <p class="note">LIVE 模式：<span class="mono">Production-update.json.state.motionView</span> 有資料時停止 SIM 動畫，逐格投影
    （loader／auto[]／empty 的 cells・sensorOn、hotPlate cells・soakDone、inPP／outPP hold・vacuum・at・z、outPP cylinder、
    inShuttle／outShuttle kit・inTest・yAtPick、index hold・z、dut hasIC）。缺欄位一律顯示「—／不明」，不推算。</p>
  <p class="note">盤形：<span class="mono">layout.defaults.useRecipeForms</span> 為 true 才讀 <span class="mono">Setup-current.json</span>（Tray.Data／HotPlate.Data 的 X／Y Division）；
    目前現場 Recipe 皆為 HT9045 盤形，預設 false → 用 <span class="mono">layout.defaults</span>。</p>
</section></details>

<footer>
  HT9050 / HP-9050 Motion View（HTML-only；BCB6 無 HT9050，<span class="mono">runtimeSupported:false</span>）　·
  版面編輯 <span class="mono">IDE.MotionView9050-LayoutEditor.html</span>　·　純動畫概念稿 <span class="mono">IDE.MotionView9050-Concept.html</span>　·
  UPH <span class="mono">IDE.MotionView9050-UPH.html</span>　·　本檔由 <span class="mono">_build_main_motionview9050.py</span> 自概念頁產生，勿手改
</footer>''')

# ---------- scripts ----------
rep('<script>\n"use strict";', '<script src="theme.js"></script>\n<script src="settings.js"></script>\n<script>\n"use strict";')

# JSON override layer after the default TRAY
rep('var HPF={cols:2,rows:2};\nvar TRAY={cols:2,rows:1};',
'''var HPF={cols:2,rows:2};
var TRAY={cols:2,rows:1};

/* ================= JSON 覆寫層（正式頁）：幾何／站點／行程／秒數／盤形全部由 JSON 決定，JS 內只留落回值 ================= */
var LAYOUT=null, LIVE=null, DUR_JSON={},
    SRC={tray:"JS 預設",hp:"JS 預設",geo:"JS 預設",sec:"JS 預設",axes:"JS 預設",flag:"—",layout:"—",machine:"—",live:"—"};
function jv(section,key,dflt){ var it=section&&section[key]; return it&&it.value!==undefined?it.value:dflt; }
function nm(v,d){ return (typeof v==="number"&&isFinite(v))?v:d; }
function pt(o,d){ return (o&&typeof o.x==="number"&&typeof o.y==="number")?{x:o.x,y:o.y}:d; }
function applyLayout(L){
  LAYOUT=L||{}; var i,m,s=L.stations||{},k=L.strokes||{},z=k.z||{},steps=(L.flow&&L.flow.steps)||[];
  if(Array.isArray(L.modules)) for(i=0;i<L.modules.length;i++){ m=L.modules[i];
    if(m&&m.id&&Array.isArray(m.rect)&&m.rect.length===4) MODS[m.id]=m.rect.map(Number); }
  if(L.footprintMm&&L.footprintMm.w>0) FP={w:+L.footprintMm.w,h:+L.footprintMm.h};
  ST.LOADER=pt(s.loader,ST.LOADER); ST.HP=pt(s.hotPlate,ST.HP); ST.ISHT=pt(s.inShuttle,ST.ISHT);
  ST.TEST=pt(s.test,ST.TEST); ST.OSHT=pt(s.outShuttle,ST.OSHT);
  if(Array.isArray(s.auto)&&s.auto.length>=3) ST.AUTO=[pt(s.auto[0],ST.AUTO[0]),pt(s.auto[1],ST.AUTO[1]),pt(s.auto[2],ST.AUTO[2])];
  ST.SHT_Y=nm(k.shuttleY,ST.SHT_Y); ST.RAIL_Y_IN=nm(k.railYIn,ST.RAIL_Y_IN); ST.RAIL_Y_OUT=nm(k.railYOut,ST.RAIL_Y_OUT);
  ST.TRAY_RAIL_Y=nm(k.trayRailY,ST.TRAY_RAIL_Y);
  if(k.inRail)  ST.IN_RAIL ={x0:nm(k.inRail.x0,ST.IN_RAIL.x0),  x1:nm(k.inRail.x1,ST.IN_RAIL.x1)};
  if(k.outRail) ST.OUT_RAIL={x0:nm(k.outRail.x0,ST.OUT_RAIL.x0),x1:nm(k.outRail.x1,ST.OUT_RAIL.x1)};
  ST.OSHT_Y_PICK=nm(k.outShuttleYPick,ST.OSHT_Y_PICK); ST.OSHT_Y_STROKE=nm(k.outShuttleYTopAOI,ST.OSHT_Y_STROKE);
  if(k.outArmYCylinder) ST.OUT_CYL={back:nm(k.outArmYCylinder.back,ST.OUT_CYL.back),front:nm(k.outArmYCylinder.front,ST.OUT_CYL.front)};
  ST.Z_GANTRY=nm(z.gantry,ST.Z_GANTRY); ST.Z_ARM_SAFE=nm(z.armSafe,ST.Z_ARM_SAFE); ST.Z_IDX_RAIL=nm(z.indexRail,ST.Z_IDX_RAIL);
  ST.Z_IDX_SAFE=nm(z.indexSafe,ST.Z_IDX_SAFE); ST.Z_KIT=nm(z.kit,ST.Z_KIT); ST.Z_DUT=nm(z.dut,ST.Z_DUT);
  DUR_JSON={}; for(i=0;i<steps.length;i++) if(steps[i]&&steps[i].id){
    if(steps[i].sec>0) DUR_JSON[steps[i].id]=+steps[i].sec;
    if(steps[i].label) for(var j=0;j<STEP_DEF.length;j++) if(STEP_DEF[j].k===steps[i].id) STEP_DEF[j].lbl=String(steps[i].label);
  }
  /* 軸讀數清單：JS 內的預設整份被 axes.bindings（在籍）＋ axes.absent（HT9050 沒有的 HT9045 軸）取代 */
  if(L.axes&&(Array.isArray(L.axes.bindings)||Array.isArray(L.axes.absent))){
    AXES={bindings:L.axes.bindings||[],absent:L.axes.absent||[]};
    SRC.axes="layout axes：在籍 "+AXES.bindings.length+" 軸／HT9050 無 "+AXES.absent.length+" 項";
  }
  SRC.layout="MotionView9050-layout.json v"+(L.schemaVersion||"?")+"（"+((L.modules||[]).length)+" 模組）";
  SRC.geo=s.loader?"layout stations／strokes":"JS 預設（layout 無 stations）";
  SRC.sec=Object.keys(DUR_JSON).length?"layout flow.steps[].sec":"JS 預設";
}
function applyForms(settings){
  var df=(LAYOUT&&LAYOUT.defaults)||{};
  if(df.hotPlateForm&&df.hotPlateForm.cols>0&&df.hotPlateForm.rows>0){ HPF={cols:+df.hotPlateForm.cols,rows:+df.hotPlateForm.rows}; SRC.hp="layout defaults.hotPlateForm"; }
  if(df.trayForm&&df.trayForm.cols>0&&df.trayForm.rows>0){ TRAY={cols:+df.trayForm.cols,rows:+df.trayForm.rows}; SRC.tray="layout defaults.trayForm"; }
  if(df.useRecipeForms!==true){ SRC.tray+="（useRecipeForms:false）"; SRC.hp+="（useRecipeForms:false）";
    SRC.flag="—（useRecipeForms:false，不讀 HT9045 Recipe 的 HotPlate.Data）"; return; }
  var recipe=(settings&&(settings.recipe||settings))||{}, docs=recipe.documents||{},
      tray=(docs.tray||{}).sections||{}, hp=(docs.hotPlate||{}).sections||{},
      tc=+jv(tray.Type0,"X Division",0), tr=+jv(tray.Type0,"Y Division",0),
      hc=+jv(hp["Hotplate Form"],"X Division",0), hr=+jv(hp["Hotplate Form"],"Y Division",0);
  if(tc>0&&tr>0){ TRAY={cols:tc,rows:tr}; SRC.tray="Setup-current.json Tray.Data"; }
  if(hc>0&&hr>0){ HPF={cols:hc,rows:hr}; SRC.hp="Setup-current.json HotPlate.Data"; }
  var uf=jv(hp["Hotplate Form"],"Using Flag",null);
  SRC.flag=(uf==null)?"—（Setup-current.json 無 HotPlate.Data Using Flag）":String(uf);
}''')

# caps recomputed after JSON
rep('''var HP_CAP=HPF.rows*HPF.cols, AUTO_CAP=TRAY.rows*TRAY.cols*3,
    PRIME_C=HP_CAP, LOT=PRIME_C+1+AUTO_CAP;          /* 結束時 Hot plate 滿＋KIT 上 1 顆＋Auto 滿 */''',
'''var HP_CAP=0, AUTO_CAP=0, PRIME_C=0, LOT=0;         /* 結束時 Hot plate 滿＋KIT 上 1 顆＋Auto 滿 */
function recomputeCaps(){ HP_CAP=HPF.rows*HPF.cols; AUTO_CAP=TRAY.rows*TRAY.cols*3; PRIME_C=HP_CAP; LOT=PRIME_C+1+AUTO_CAP; }
recomputeCaps();''')

# hash handled in startPage
# 正式頁不是概念頁：載入後停在起始畫面，要不要動由 Runtime JSON（或 debug 手動播放）決定
rep('var T=0, PLAY=true, SPD=1, CYC=0, lastTs=0, lastPhase=-1, lastCyc=-1;',
    'var T=0, PLAY=false, SPD=1, CYC=0, lastTs=0, lastPhase=-1, lastCyc=-1;   '
    '/* 正式頁載入後不自己動：LIVE 一律不跑，SIM 要按「▶ 播放」 */')

# LIVE 時不准用 SIM 蓋掉 Runtime 畫面
rep('document.getElementById("btnPlay").onclick=function(){\n  PLAY=!PLAY;',
    'document.getElementById("btnPlay").onclick=function(){\n'
    '  if(LIVE) return;                                   /* LIVE：畫面歸 Runtime，不許 SIM 插手 */\n'
    '  PLAY=!PLAY;')
rep('document.getElementById("btnStep").onclick=function(){\n  PLAY=false;',
    'document.getElementById("btnStep").onclick=function(){\n  if(LIVE) return;\n  PLAY=false;')
rep('document.getElementById("btnReset").onclick=function(){ T=0;',
    'document.getElementById("btnReset").onclick=function(){ if(LIVE) return; T=0;')

rep('''var HP=hashParams();
if(HP.hpc>0&&HP.hpr>0){ HPF={cols:+HP.hpc,rows:+HP.hpr}; }
if(HP.trc>0&&HP.trr>0){ TRAY={cols:+HP.trc,rows:+HP.trr}; }
buildOps(HP);''', '''var HP=hashParams();   /* 秒數／盤形覆寫；套用時機在 startPage() */''')

rep('  document.getElementById("hdTray").textContent=TRAY.cols+"×"+TRAY.rows+"（示意，實際依 Tray.Data）";\n'
    '  document.getElementById("hdHP").textContent=HPF.cols+"×"+HPF.rows+"（示意，實際依 HotPlate.Data）";',
    '  document.getElementById("hdTray").textContent=TRAY.cols+"×"+TRAY.rows+"（"+SRC.tray+"）";\n'
    '  document.getElementById("hdHP").textContent=HPF.cols+"×"+HPF.rows+"（"+SRC.hp+"）";')

# ---------- render → placeAll ----------
a = s.index('function render(){')
b = s.index('  if(TLph){', a)
head = '''function render(){
  if(LIVE) return;                                   /* LIVE：畫面只投影 Runtime 快照，不跑 SIM */
  var idx=opAt(T), inA=inArmAt(T), outA=outArmAt(T),
      isx=ishtXAt(T), osx=oshtXAt(T), osy=oshtYAt(T), iz=idxZAt(T),
      ic=icAt(T,inA,outA,isx,osx,iz,osy);
  placeAll(inA,outA,isx,osx,osy,iz,ic,icBAt(T,inA),icCAt(T,inA,isx),(idx>=0&&MAIN[idx].k==="TEST"),(idx>=0&&T>=K.TEST.b));
'''
old_render_head = '''function render(){
  var idx=opAt(T), inA=inArmAt(T), outA=outArmAt(T),
      isx=ishtXAt(T), osx=oshtXAt(T), osy=oshtYAt(T), iz=idxZAt(T),
      ic=icAt(T,inA,outA,isx,osx,iz,osy);
'''
body = s[a:b]
assert body.startswith(old_render_head), 'render head changed'
place = body[len(old_render_head):]
place = place.replace('(idx>=0&&MAIN[idx].k==="TEST")', 'testing').replace('(idx>=0&&T>=K.TEST.b)', 'tested')
place = place.replace('  var ic2=icBAt(T,inA);\n', '').replace('  var ic3=icCAt(T,inA,isx);\n', '')
assert 'icBAt(' not in place and 'icCAt(' not in place
place_fn = '/* 機構與料件擺位（SIM／LIVE 共用） */\nfunction placeAll(inA,outA,isx,osx,osy,iz,ic,ic2,ic3,testing,tested){\n' + place + '}\n'
s = s[:a] + place_fn + head + s[b:]

# ---------- boot / startPage ----------
old_tail_start = s.index('drawIso();\ndrawHead("head","var(--acc)","MInRotateKit");')
old_tail_end = s.index('requestAnimationFrame(loop);', old_tail_start) + len('requestAnimationFrame(loop);')
tail = s[old_tail_start:old_tail_end]
new_tail = '''/* ================= 啟動：機種閘門 → JSON → 繪圖 ================= */
var BOOTED=false;
function boot(){
  if(BOOTED) return; BOOTED=true;
''' + '\n'.join('  '+ln if ln.strip() else ln for ln in tail.split('\n')) + '''
}
function withMachine(id){
  var q=(location.search||"").replace(/^\\?/,"").split("&").filter(function(kv){ return kv&&kv.indexOf("machine=")!==0; });
  q.push("machine="+id); return location.pathname.replace(/^.*\\//,"")+"?"+q.join("&")+location.hash;
}
function gate(m){
  document.body.classList.add("gated");
  document.getElementById("gateMsg").textContent="目前機種 machine="+m.id+(m.label?"（"+m.label+"）":"")+"，來源 "+(m.source||"?")+"；本頁僅供 HT9050。";
  document.getElementById("gateForce").href=withMachine("HT9050");
}
function stationXY(key,d){
  var map={loader:ST.LOADER,hotPlate:ST.HP,inShuttle:ST.ISHT,test:ST.TEST,outShuttle:ST.OSHT,auto1:ST.AUTO[0],auto2:ST.AUTO[1],auto3:ST.AUTO[2]};
  return map[key]||d;
}
/* LIVE：只投影 state.motionView，缺欄位顯示「—／不明」，不推算 */
function applyLive(mv){
  var n=TRAY.rows*TRAY.cols, i, c, v=[0,0,0,0,0,0,0,0,0,0,0,0], tot=0;
  function arr(a){ return Array.isArray(a)?a:[]; }
  function unit(u,name,cls){ var cells=arr(u&&u.cells), on=0, tu=D.trayUnit[name];
    for(i=0;i<n;i++){ var has=cells[i]===1; if(has) on++;
      tu.cells[i].setAttribute("visibility",has?"visible":"hidden"); if(has) setSvgCls(tu.cells[i],"tray-presence-cell "+cls); }
    setSvgCls(tu.sensor,"fix-sensor-led "+(u&&u.sensorOn===true?"on":(u&&u.sensorOn===false?"off":"unknown"))); return on; }
  v[0]=unit(mv.loader,"Loader","untested");
  for(i=0;i<n;i++) setCls(UI.tray[i], arr(mv.loader&&mv.loader.cells)[i]===1?"cellT has":"cellT");
  document.getElementById("cntTray").textContent=(mv.loader?v[0]:"—")+" / "+n;
  var autos=arr(mv.auto), keys=["Auto1","Auto2","Auto3"], dk=["auto1","auto2","auto3"];
  for(c=0;c<3;c++){ v[8+c]=unit(autos[c],keys[c],"tested");
    for(i=0;i<n;i++) setCls(UI.dest[dk[c]][i], arr(autos[c]&&autos[c].cells)[i]===1?"cellT tst":"cellT");
    UI.destCount[dk[c]].textContent="盤內 "+(autos[c]?v[8+c]:"—"); }
  unit(mv.empty,"Empty","untested");
  var hp=mv.hotPlate||{}, hc=arr(hp.cells), sd=arr(hp.soakDone), sn=0;
  for(i=0;i<HP_CAP;i++){ var on=hc[i]===1; if(on){ v[2]++; if(sd[i]===true) sn++; }
    setFill(D.dHP[i],on?"var(--s0)":"var(--line2)"); setCls(UI.hp[i],on?(sd[i]===true?"cellH s0 sk":"cellH s0"):"cellH"); }
  document.getElementById("cntHP").textContent=(mv.hotPlate?v[2]:"—")+" / "+HP_CAP;
  document.getElementById("cntHPOut").textContent="—"; document.getElementById("cntHPSoak").textContent=mv.hotPlate?""+sn:"—";
  var ip=mv.inPP||{}, op=mv.outPP||{}, is=mv.inShuttle||{}, os=mv.outShuttle||{}, ix=mv.index||{}, du=mv.dut||{};
  D.headIC.setAttribute("opacity",ip.hold===true?"1":"0"); D.headOIC.setAttribute("opacity",op.hold===true?"1":"0");
  D.headState.textContent="真空 "+(ip.vacuum==null?"—":(ip.vacuum?"ON":"OFF"));
  D.headOState.textContent="真空 "+(op.vacuum==null?"—":(op.vacuum?"ON":"OFF"));
  if(ip.hold===true) v[1]++; if(op.hold===true) v[7]++;
  setCls(UI.kit[0], is.kit===1?"cellK has":"cellK"); setCls(UI.kit[1], os.kit===1?"cellK tst":"cellK");
  setCls(UI.kit[2], ix.hold===true?"cellK has":"cellK"); setCls(UI.kit[3], du.hasIC===true?"cellK has":"cellK");
  if(is.kit===1) v[3]++; if(ix.hold===true) v[4]++; if(du.hasIC===true) v[5]++; if(os.kit===1) v[6]++;
  for(i=0;i<UI.chips.length;i++){ UI.chips[i].val.textContent=""+v[i]; setCls(UI.chips[i].box,v[i]>0?"chip on":"chip"); tot+=v[i]; }
  UI.total.textContent=tot+"（LIVE）";
  document.getElementById("logBox").textContent="LIVE：動作記錄由 Runtime 提供（state.motionView.log 尚未定義）";
  document.getElementById("tPhase").textContent="LIVE"+(mv.paused?"（暫停）":"");
  /* 機構：二值狀態直接擺到站點，不內插 */
  var pi=stationXY(ip.at,ST.HP), po=stationXY(op.at,ST.OSHT), cyl=op.cylinder==="front"?1:0,
      inA={x:pi.x,y:pi.y,z:ip.z==="down"?8:ST.Z_ARM_SAFE},
      outA={x:po.x,y:lerp(ST.OUT_CYL.back,ST.OUT_CYL.front,cyl),z:op.z==="down"?8:ST.Z_ARM_SAFE,cyl:cyl},
      isx=is.inTest===true?ST.TEST.x:ST.ISHT.x, osx=os.inTest===true?ST.TEST.x:ST.OSHT.x,
      osy=os.yAtPick===true?ST.SHT_Y-ST.OSHT_Y_PICK:ST.SHT_Y,
      iz=ix.z==="down"?ST.Z_DUT:(ix.z==="kit"?ST.Z_KIT:ST.Z_IDX_SAFE);
  placeAll(inA,outA,isx,osx,osy,iz,null,null,null,du.hasIC===true,false);
  for(i=0;i<TLseg.length;i++) TLseg[i].setAttribute("opacity",0.2);
}
function stopSim(){
  PLAY=false;
  var b=document.getElementById("btnPlay"); if(b){ b.textContent="▶ 播放"; b.className="on"; }
}
function applyRuntimeState(settings){
  var pu=settings&&(settings.productionUpdate||settings.production), mv=pu&&pu.state&&pu.state.motionView,
      st=document.getElementById("liveStatus"), tag=document.getElementById("modeTag"), why="";
  /* 與 Main.MotionView.html 同一條規矩：每次收到 Runtime JSON 都先停下來，畫面動不動由 JSON 決定 */
  stopSim();
  /* HT9045 runtime 的 motionView（trays/ledGroups/trayLocations）與本頁契約不同：只有標明 machine:"HT9050" 才投影 */
  if(mv && mv.machine!=="HT9050"){ why="（Production-update 的 motionView 屬 "+(mv.machine||"HT9045 runtime")+"，非 HT9050 契約，已忽略）"; mv=null; }
  else if(mv && mv.available===false){ why="（motionView.available:false）"; mv=null; }
  if(!mv){ LIVE=null; SRC.live="無 HT9050 state.motionView → 停在 SIM 起始畫面"+why;
    st.textContent="等待 C++ Runtime 在 Production-update.json.state.motionView 發布 machine:'HT9050' 的逐格快照；"+
      "在那之前畫面靜止不動（runtimeSupported:false）。"+
      (document.documentElement.getAttribute("data-mode")==="release"?"":"要看機構動作示意可按工具列「▶ 播放」跑 SIM。")+why;
    st.className="note runtime-status warn"; tag.textContent="SIM（停止）";
    document.getElementById("trayLocSrc").textContent="等待 Runtime（SIM 不推算 Tray Up／Car）";
    render(); return; }
  LIVE=mv; SRC.live="LIVE state.motionView"+(mv.paused?"（暫停）":"");
  st.textContent="LIVE：C++ Runtime JSON 快照（"+(mv.paused?"暫停":"即時")+"）— 畫面只投影 state.motionView，不推算。";
  st.className="note runtime-status live"; tag.textContent="LIVE"; document.getElementById("trayLocSrc").textContent="Runtime";
  applyLive(mv);
}
/* HotPlate／Auto Clean／Option 模組：清單來自 Machine-profile.json 的 layoutModules；
   Option 有沒有列進 MotionView9050-layout.json modules[] 決定要不要淡出（沒列＝立體圖不繪） */
function renderModules(m){
  var el=document.getElementById("optNote"), lm=(m&&m.layoutModules)||[],
      mods=(LAYOUT&&LAYOUT.modules)||[], inLayout={}, std=[], opt=[], i, md, on, mono='<span class="mono">';
  for(i=0;i<mods.length;i++) if(mods[i]&&mods[i].id) inLayout[mods[i].id]=1;
  document.getElementById("hdUsingFlag").textContent="Using Flag "+SRC.flag;
  if(!lm.length){ el.textContent="Machine-profile.json profiles."+((m&&m.id)||"?")+" 未提供 layoutModules → 模組清單不明。"; return; }
  for(i=0;i<lm.length;i++){ md=lm[i]; on=!!inLayout[md.id];
    (md.option?opt:std).push('<span class="mono"'+((md.option&&!on)?' style="opacity:.5"':'')+'>'+
      md.no+". "+md.name+((md.option&&!on)?"（未配置）":"")+'</span>');
  }
  el.innerHTML="HotPlate：Plate1 / Plate2 依 "+mono+"Using Flag</span>（見卡片標題）。"+
    "Auto Clean："+(inLayout.AutoClean?"已配置，狀態等待 Runtime":"未配置")+"。<br>"+
    "標配 "+std.length+"："+std.join("・")+"<br>"+
    "Option "+opt.length+"："+opt.join("・")+"<br>"+
    "清單來自 "+mono+"Machine-profile.json profiles."+((m&&m.id)||"?")+".layoutModules</span>；"+
    "淡出＝未列入 "+mono+"MotionView9050-layout.json modules[]</span>，立體圖不繪。";
}
function showParams(m){
  var lead=document.getElementById("srcLead"), tb=document.getElementById("srcTable"), rows, i, tr;
  SRC.machine="Machine-profile.json → "+m.id+"（"+(m.label||"")+"；runtimeSupported:"+m.runtimeSupported+"；來源 "+(m.source||"?")+"）";
  document.getElementById("paramNote").innerHTML=
    "料盤 <b>"+TRAY.cols+"×"+TRAY.rows+"</b>（"+SRC.tray+"）　加熱盤 <b>"+HPF.cols+"×"+HPF.rows+"</b>（"+SRC.hp+"）　吸嘴 <b>1×1</b>（caps.pickerCount）　"+
    "站點／行程：<b>"+SRC.geo+"</b>　步序秒數：<b>"+SRC.sec+"</b>"+(Object.keys(HP).length?"＋URL hash":"")+"　動畫 Cycle <b>"+CYCLE.toFixed(1)+" s</b>　LOT "+LOT;
  lead.textContent="資料來源：Machine-profile.json（machine="+m.id+"）· "+SRC.layout+" · 盤形："+SRC.tray+" · Production-update.json："+SRC.live;
  lead.className="lead src"+(LIVE?"":" warn");
  rows=[["機種能力集","JSON/Machine-profile.json",SRC.machine],
        ["機構版面幾何・站點・行程","JSON/MotionView9050-layout.json",SRC.layout+"；"+SRC.geo],
        ["步序秒數／動作記錄文字（SIM）","JSON/MotionView9050-layout.json flow.steps[].sec／label",SRC.sec],
        ["軸讀數清單","JSON/MotionView9050-layout.json axes.bindings／axes.absent",SRC.axes],
        ["HotPlate／Option 模組清單","JSON/Machine-profile.json layoutModules",SRC.machine],
        ["Tray／HotPlate 盤形","JSON/Setup-current.json（useRecipeForms）／layout.defaults",SRC.tray+"；"+SRC.hp],
        ["逐格在籍／軸位置／LED","Production-update.json.state.motionView",SRC.live],
        ["Sim 換算（畫面 ↔ 機構座標）","JSON/Sim-scale.9050.json","未建立（無 BCB6 來源，需標 toolchain:'HTML-only'）"]];
  tb.innerHTML="";
  for(i=0;i<rows.length;i++){ tr=H(tb,"tr"); H(tr,"td","",rows[i][0]); H(tr,"td","mono",rows[i][1]); H(tr,"td","",rows[i][2]); }
  renderModules(m);
}
function startPage(settings){
  var m=(window.HTSettings&&HTSettings.machine())||{id:"?",source:"no-settings"};
  document.getElementById("mvMachineTag").textContent="machine="+m.id;
  if(m.id!=="HT9050"){ gate(m); return; }
  HTSettings.loadJson("MotionView9050-layout.json").then(function(L){
    applyLayout(L); applyForms(settings);
    if(HP.hpc>0&&HP.hpr>0){ HPF={cols:+HP.hpc,rows:+HP.hpr}; SRC.hp="URL hash"; }
    if(HP.trc>0&&HP.trr>0){ TRAY={cols:+HP.trc,rows:+HP.trr}; SRC.tray="URL hash"; }
    recomputeCaps();
    buildOps(Object.assign({},DUR_JSON,HP));
    boot();
    applyRuntimeState(settings); showParams(m);
    if(HTSettings.request) HTSettings.request();       /* 跟 background 要一份最新的，之後靠 HT_SETTINGS 推播 */
  }).catch(function(e){
    var lead=document.getElementById("srcLead");
    lead.textContent="載入失敗："+e.message+"（file:// 下請先重跑 _gen_json_shim.py）"; lead.className="lead src err";
  });
}
/* Runtime 更新的兩條路，跟 motionview-full.js 一樣兩個都聽：
   ① 獨立開頁：本頁自己的 settings.js 跑 refreshProduction() → HT_SETTINGS_REFRESHED
   ② 嵌在 background.html：background 每 250ms 比對 event.seq，有新事件才 postMessage HT_SETTINGS
      —— iframe 內不會發 HT_SETTINGS_REFRESHED，只聽它會完全收不到 Runtime 更新 */
function onRuntime(st){
  if(!BOOTED||!st) return;
  applyRuntimeState(st); showParams(HTSettings.machine());
}
window.addEventListener("HT_SETTINGS_REFRESHED",function(ev){ onRuntime(ev.detail&&ev.detail.settings); });
window.addEventListener("message",function(ev){ if(ev.data&&ev.data.type==="HT_SETTINGS") onRuntime(ev.data.settings); });
if(window.HTSettings) HTSettings.load().then(startPage).catch(function(e){ gate({id:"?",source:"settings.js 載入失敗："+e.message}); });
else gate({id:"?",source:"settings.js 未載入"});'''
s = s[:old_tail_start] + new_tail + s[old_tail_end:]

io.open(DST, 'w', encoding='utf-8', newline='\n').write(s)
print('ok', len(s), 'lines', s.count('\n'))
