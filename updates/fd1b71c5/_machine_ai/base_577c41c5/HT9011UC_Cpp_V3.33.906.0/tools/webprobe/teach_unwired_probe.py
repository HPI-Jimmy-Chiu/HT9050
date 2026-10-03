# -*- coding: utf-8 -*-
"""tools/webprobe/teach_unwired_probe.py -- batch 38: open web/page/HW.teach.html (file://, headless Edge) and check the B38 same-line additions:
  U  the 106 TEACH_UNWIRED_B38 buttons are greyed (class teach-unwired, data-unwired starts with '<id>：')
  H  SetButton070 / GoButton070 / SetButton071 / GoButton071 are display:none (golden FormCreate :1427-1430)
  M  btClearMemo clears Memo1 / Memo2 (golden btClearMemoClick :4710-4714)
  A  teachAttachMotorButtons with a fake motor table {MInArmPitch, MLoaderY}: btnInXPitch1 / btnLoaderY bound and select their motor,
     btnOutXPitch1 (MOutArmPitch not in the table) greyed with the Mot_Table reason
  C  controls: a teach-access.json Set/Go button and a Motor* button are NOT greyed by B38
  T  AI(W906-ARMCELL) 20261002: the Teach "Arm Cell" tab (web/page/ht9045_teach_armcell.js, RULINGS_20261002 #18) -- the 18th sheet,
     shown / hidden by the page's binder, greyed on file:// with the reason, no text input, not motor-bound; a canned runtime block
     drives the module (catalog, reversible greying, Q3 default, "到位" only for this page's job). CONTROL: the HEAD web tree -> red.
AI(W906-B38-UNWIRED) 20261002: read-only (file:// page, no wb_serve, no machine files); not a ctest (needs Edge).  When a greyed button
gets wired, take it out of TEACH_UNWIRED_B38 (HW.teach.html :191) -- U iterates that list, so this probe follows.
Usage: python tools/webprobe/teach_unwired_probe.py [<repo root holding web/page>]   (default: this checkout).  Exit 0 = all pass."""
import json, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
TREE = sys.argv[1] if len(sys.argv) > 1 else os.path.normpath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, HERE)
from s12_form_probe import Cdp, launch_edge   # noqa: E402
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []


def check(ok, what):
    print(('PASS ' if ok else 'FAIL ') + what)
    if not ok:
        FAILS.append(what)


page = 'file:///' + TREE.replace(chr(92), '/') + '/web/page/HW.teach.html'
proc, prof, ws = launch_edge(9338)
try:
    cdp = Cdp(ws)
    cdp.call('Page.enable')
    cdp.call('Page.navigate', {'url': page})
    for _ in range(80):
        if cdp.eval("document.readyState") == 'complete' and cdp.eval("typeof TEACH_UNWIRED_B38") == 'object':
            break
        time.sleep(0.25)
    time.sleep(1.0)
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      var bad=[], n=0;
      TEACH_UNWIRED_B38.forEach(function(g){ g[1].split(' ').forEach(function(id){ n++;
        var e=document.getElementById(id);
        if(!e || !e.classList.contains('teach-unwired') || (e.getAttribute('data-unwired')||'').indexOf(id+'：')!==0) bad.push(id); }); });
      return {n:n, bad:bad, groups:TEACH_UNWIRED_B38.length};
    })())"""))
    check(r['n'] == 106 and not r['bad'], 'U: %d B38 buttons greyed with their reason (bad=%s)' % (r['n'], r['bad'][:5]))
    r = json.loads(cdp.eval("""JSON.stringify(['SetButton070','GoButton070','SetButton071','GoButton071'].map(function(id){
      var e=document.getElementById(id); return e ? getComputedStyle(e).display : 'missing'; }))"""))
    check(r == ['none'] * 4, 'H: golden-hidden four are display:none ' + str(r))
    r = cdp.eval("""(function(){ var a=document.getElementById('Memo1'), b=document.getElementById('Memo2');
      a.value='12\\n34'; b.value='56'; document.getElementById('btClearMemo').click(); return JSON.stringify([a.value,b.value]); })()""")
    check(r == '["",""]', 'M: btClearMemo clears Memo1 / Memo2 -> ' + r)
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      teachMotorConfig={motors:[{motorId:'MInArmPitch',motorIndex:3,enable:1},{motorId:'MLoaderY',motorIndex:60,enable:1},{motorId:'MInArmX',motorIndex:0,enable:1}]};
      teachAttachMotorButtons();
      var out={};
      ['btnInXPitch1','btnLoaderY','btnOutXPitch1','MotorInArmX'].forEach(function(id){
        var e=document.getElementById(id);
        out[id]={bound:!!(e&&e.getAttribute('data-motor-bound')), grey:!!(e&&e.classList.contains('teach-unwired')), why:(e&&e.getAttribute('data-unwired'))||''};
      });
      var sel=[];
      ['btnInXPitch1','btnLoaderY'].forEach(function(id){ teachActiveMotorId=null; try{ document.getElementById(id).click(); }catch(x){ sel.push('throw '+x.message); } sel.push(teachActiveMotorId); });
      out.sel=sel;
      return out;
    })())"""))
    check(r['btnInXPitch1']['bound'] and not r['btnInXPitch1']['grey'], 'A: btnInXPitch1 bound ' + json.dumps(r['btnInXPitch1'], ensure_ascii=False))
    check(r['btnLoaderY']['bound'] and not r['btnLoaderY']['grey'], 'A: btnLoaderY bound ' + json.dumps(r['btnLoaderY'], ensure_ascii=False))
    check(r['sel'] == ['MInArmPitch', 'MLoaderY'], 'A: clicks select MInArmPitch / MLoaderY ' + str(r['sel']))
    check(r['btnOutXPitch1']['grey'] and 'MOutArmPitch' in r['btnOutXPitch1']['why'], 'A: btnOutXPitch1 greyed (MOutArmPitch not in table) ' + r['btnOutXPitch1']['why'])
    check(r['MotorInArmX']['bound'] and not r['MotorInArmX']['grey'], 'C: MotorInArmX still bound, not greyed')
    r = json.loads(cdp.eval("""JSON.stringify(['SetButton030','GoButton030','btnJogP','btnHome'].map(function(id){
      var e=document.getElementById(id); return [id, !!e, !!(e&&e.classList.contains('teach-unwired'))]; }))"""))
    check(all(x[1] and not x[2] for x in r), 'C: Set/Go and motion controls not greyed ' + str(r))
    # ---- T: the Teach "Arm Cell" tab -- AI(W906-ARMCELL) 20261002 (RULINGS_20261002 #18; web/page/ht9045_teach_armcell.js) ----
    #   The 18th PageControl2 sheet, switched by the page's own binder; file:// -> greyed with the reason, GO sends nothing; no text
    #   input; nothing bound as a motor-select button; then a canned runtime armCell block drives the real module (catalog render,
    #   reversible greying, Q3 default, "到位" only for the job this page started).  CONTROL: the HEAD web tree (no Arm Cell) -> red.
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      var pc=document.getElementById('PageControl2');
      var own=function(sel){ return Array.prototype.filter.call(pc.querySelectorAll(sel), function(e){ return e.closest('.pcWrap')===pc; }); };
      var tabs=own('.tab'), panes=own('.pcPane'), last=tabs[tabs.length-1];
      return {n:tabs.length, np:panes.length, cap:last.textContent, t:last.getAttribute('data-t'),
              title:last.getAttribute('title')||last.getAttribute('data-htitle'), pane:own('.pcPane[data-p="17"]').length};
    })())"""))
    check(r['n'] == 18 and r['np'] == 18 and r['cap'] == 'Arm Cell' and r['t'] == '17' and r['title'] == 'tsArmCell : TTabSheet' and r['pane'] == 1,
          'T: PageControl2 has 18 sheets, the 18th is Arm Cell (data-t 17, tsArmCell : TTabSheet) with pane 17 ' + json.dumps(r, ensure_ascii=False))
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      var pc=document.getElementById('PageControl2');
      var own=function(sel){ return Array.prototype.filter.call(pc.querySelectorAll(sel), function(e){ return e.closest('.pcWrap')===pc; }); };
      var vis=function(){ return own('.pcPane').filter(function(p){ return getComputedStyle(p).display!=='none'; }).map(function(p){ return p.getAttribute('data-p'); }); };
      var tab=function(n){ return own('.tab').filter(function(t){ return t.getAttribute('data-t')===n; })[0]; };
      if(!tab('17')) return {a:null, act:false, b:null};
      tab('17').click(); var a=vis(), act=tab('17').classList.contains('act');
      tab('8').click(); var b=vis();
      return {a:a, act:act, b:b};
    })())"""))
    check(r['a'] == ['17'] and r['act'], 'T: clicking Arm Cell shows pane 17 only ' + json.dumps(r))
    check(r['b'] == ['8'], 'T: clicking Axle Control hides it again ' + json.dumps(r))
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      var g=document.getElementById('armCellGo'), p=document.getElementById('armCellPane');
      if(!g || !p) return {missing:true};
      var txt=p.querySelectorAll('input[type="text"],input:not([type])').length;
      var bound=Array.prototype.filter.call(document.querySelectorAll('[id^="armCell"]'), function(e){ return e.getAttribute('data-motor-bound'); }).map(function(e){ return e.id; });
      var before=(window.HTMotorAccess && HTMotorAccess.current) ? HTMotorAccess.current() : null;
      g.click();
      var after=(window.HTMotorAccess && HTMotorAccess.current) ? HTMotorAccess.current() : null;
      return {grey:g.classList.contains('teach-unwired'), why:g.getAttribute('data-unwired')||'', st:document.getElementById('armCellStatus').textContent,
              txt:txt, bound:bound, sent:!!after && after!==before, src:window.teachMotorSrc};
    })())"""))
    check(not r.get('missing') and r['grey'] and r['why'] and r['st'] == r['why'] and not r['sent'],
          'T: file:// -> Arm Cell greyed, GO says why and sends nothing ' + json.dumps(r, ensure_ascii=False))
    check(not r.get('missing') and r['txt'] == 0, 'T: no text input in the Arm Cell pane (attachKeyboards would make it read-only)')
    check(not r.get('missing') and not r['bound'], 'T: no armCell* element bound as a motor-select button ' + str(r.get('bound')))
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      if(!window.HTArmCell) return {missing:true};
      window.teachMotorSrc='C++';
      var ax=function(m){ return {motor:m,mi:0,present:true,enable:true,pci1203:true}; };
      var nz=function(a){ return [{i:0,j:0,alias:a,pci1203:true}]; };
      var area=function(arm,id,label,usable,why,c,r,zd,zw,zk){ return {arm:arm,id:id,label:label,v1:usable,installed:usable,usable:usable,why:why,cols:c,rows:r,zDownAllowed:zd,zDownWhy:zw,zKind:zk}; };
      var cat={ok:true, why:'', in:{x:ax('MInArmX'),y:ax('MInArmY'),nozzles:nz('MInArmZA'),note:'D1 note',why:''},
        out:{x:ax('MOutArmX'),y:ax('MOutArmY'),nozzles:nz('MOutArmZA'),note:'',why:''},
        areas:[area('in','Loader','Loader',true,'',3,8,true,'','pick'), area('in','HotPlate1','HotPlate 1',false,'HotPlate 1：Using Flag 不含這一盤',2,8,true,'','place'),
               area('out','BinBox','Bin Box',false,'Bin Box：這台沒有裝',1,1,false,'Bin Box：ZPlace 沒寫過','')],
        zDownDefault:false, zDownWhy:'MInArmZA ORG 極性還沒量', zSafePos:50};
      var job={active:false,jobId:0,seq:-1,reqId:'',step:0,stepName:'',result:'',why:'',arm:'',area:'',label:'',nozzle:'',col:-1,row:-1,zDown:false,
               target:{x:0,y:0,zSafe:0,z:null},activeMotor:null,motors:[],final:null,startedAt:'',endedAt:''};
      window.teachMotorRuntime={armCell:{catalogRev:7, catalog:cat, job:job}};
      HTArmCell.tick();
      var $=function(id){ return document.getElementById(id); };
      var hp=$('armCellArea_HotPlate1'), ld=$('armCellArea_Loader');
      var out={go:$('armCellGo').classList.contains('teach-unwired'), cells:document.querySelectorAll('#armCellTray .cell').length,
               ld:!!ld && !ld.classList.contains('teach-unwired') && ld.classList.contains('armCellSel'),
               hp:!!hp && hp.classList.contains('teach-unwired') && (hp.getAttribute('data-unwired')||'').indexOf('Using Flag')>=0,
               nozzle:$('armCellNozzle').value, zd:$('armCellZDown').checked, zNote:$('armCellZNote').textContent,
               col:$('armCellCol').options.length, row:$('armCellRow').options.length};
      hp.click(); out.hpSays=$('armCellStatus').textContent;
      $('armCellArmOut').click();
      var bb=$('armCellArea_BinBox'); out.bb=!!bb && bb.classList.contains('teach-unwired'); out.noLoader=!$('armCellArea_Loader');
      $('armCellArmIn').click();
      var S=HTArmCell.state(); S.pendingSeq=41;
      HTArmCell.onAck({seq:41}, {result:'cellMoving', cellActive:true, cellSeq:3, message:'accepted', note:'D1 note',
        plan:{arm:'in',area:'Loader',label:'Loader',nozzle:'MInArmZA',col:0,row:0,zDown:false,x:{motor:'MInArmX',target:1},y:{motor:'MInArmY',target:2},
              z:{motor:'MInArmZA',target:3,kind:'pick'},zSafe:50,zLift:['MInArmZA'],teach:{x:0,y:0,z:0},base:{x:0,y:0},prod:null,cell:{x:0,y:0},d2:null,notes:['D1 note']}}, null);
      out.accepted=$('armCellStatus').textContent; out.busyAfterAck=HTArmCell.busy();
      var j2=JSON.parse(JSON.stringify(job)); j2.jobId=2; j2.result='arrived'; j2.label='Loader'; j2.col=0; j2.row=0;
      j2.final={x:{cmdPos:1,actPos:1},y:{cmdPos:2,actPos:2},z:{cmdPos:3,actPos:3}};
      window.teachMotorRuntime={armCell:{catalogRev:7, catalog:cat, job:j2}}; HTArmCell.tick();
      out.other=$('armCellStatus').textContent;
      var j3=JSON.parse(JSON.stringify(j2)); j3.jobId=3;
      window.teachMotorRuntime={armCell:{catalogRev:7, catalog:cat, job:j3}}; HTArmCell.tick();
      out.mine=$('armCellStatus').textContent;
      window.teachMotorRuntime={armCell:{catalogRev:7, catalog:cat, job:JSON.parse(JSON.stringify(j3))}}; HTArmCell.tick();
      out.busy=HTArmCell.busy(); out.myJob=S.myJob;
      return out;
    })())"""))
    check(not r.get('missing') and not r['go'] and r['ld'] and r['cells'] == 24 and r['nozzle'] == 'MInArmZA' and r['col'] == 3 and r['row'] == 8,
          'T: a catalog -> GO usable, Loader selected, 3 x 8 tray cells, nozzle MInArmZA ' + json.dumps({k: r.get(k) for k in ('go', 'ld', 'cells', 'nozzle', 'col', 'row')}, ensure_ascii=False))
    check(not r.get('missing') and r['hp'] and 'Using Flag' in r['hpSays'], 'T: a greyed area says why on click (reversible greying) ' + str(r.get('hpSays')))
    check(not r.get('missing') and not r['zd'] and '預設不勾' in r['zNote'], 'T: Q3 zDownDefault false -> Z Down unchecked with the reason ' + str(r.get('zNote')))
    check(not r.get('missing') and r['bb'] and r['noLoader'], 'T: Out Arm lists its own areas (Bin Box greyed), not the In ones')
    check(not r.get('missing') and '受理' in r['accepted'] and '到位：' not in r['accepted'] and r['busyAfterAck'],
          'T: the ack is "accepted", not arrived; the page holds the token (busy) ' + str(r.get('accepted')))
    check(not r.get('missing') and '到位：' not in r['other'], 'T: another job\'s "arrived" is not shown as this page\'s ' + str(r.get('other')))
    check(not r.get('missing') and r['mine'].startswith('到位：') and not r['busy'] and r['myJob'] == 0,
          'T: this job\'s arrived -> green "到位", then released after two reads ' + str(r.get('mine')))
    # review m6 (AI(W906-ARMCELL) 20261002): the window closed -> not busy any more (no token hold, no keepAlive), the job forgotten
    r = json.loads(cdp.eval("""JSON.stringify((function(){
      if(!window.HTArmCell || !HTArmCell.winEdge) return {missing:true};
      var S=HTArmCell.state(); S.pendingSeq=51; S.myJob=7; S.lastActive='yes'; S.inactive=0;
      var before=HTArmCell.busy();
      HTArmCell.winEdge(false);
      var closed={busy:HTArmCell.busy(), myJob:S.myJob, pending:S.pendingSeq, closedFlag:S.winClosed};
      S.pendingSeq=52;
      var stillClosed=HTArmCell.busy();
      HTArmCell.winEdge(true);
      var reopened={busy:HTArmCell.busy(), closedFlag:S.winClosed};
      S.pendingSeq=0;
      return {before:before, closed:closed, stillClosed:stillClosed, reopened:reopened};
    })())"""))
    check(not r.get('missing') and r['before'] and not r['closed']['busy'] and r['closed']['myJob'] == 0 and r['closed']['pending'] == 0 and
          r['closed']['closedFlag'] and not r['stillClosed'] and r['reopened']['busy'] and not r['reopened']['closedFlag'],
          'T: review m6 -- window closed -> busy() false (token hold / keepAlive stop), the job forgotten; reopened -> busy counts again ' + json.dumps(r))
finally:
    proc._tree_kill()
print('FAILS', len(FAILS))
sys.exit(len(FAILS))
