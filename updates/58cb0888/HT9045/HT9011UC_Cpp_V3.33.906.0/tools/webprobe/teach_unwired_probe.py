# -*- coding: utf-8 -*-
"""tools/webprobe/teach_unwired_probe.py -- batch 38: open web/page/HW.teach.html (file://, headless Edge) and check the B38 same-line additions:
  U  the 106 TEACH_UNWIRED_B38 buttons are greyed (class teach-unwired, data-unwired starts with '<id>：')
  H  SetButton070 / GoButton070 / SetButton071 / GoButton071 are display:none (golden FormCreate :1427-1430)
  M  btClearMemo clears Memo1 / Memo2 (golden btClearMemoClick :4710-4714)
  A  teachAttachMotorButtons with a fake motor table {MInArmPitch, MLoaderY}: btnInXPitch1 / btnLoaderY bound and select their motor,
     btnOutXPitch1 (MOutArmPitch not in the table) greyed with the Mot_Table reason
  C  controls: a teach-access.json Set/Go button and a Motor* button are NOT greyed by B38
  B  AI(W906-B39-ZALL) 20261002: the five Z buttons are bound and not greyed; Sort Z All Up stays greyed
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
    check(r['n'] > 0 and not r['bad'], 'U: %d TEACH_UNWIRED_B38 buttons greyed with their reason (bad=%s)' % (r['n'], r['bad'][:5]))   # AI(W906-B39-ZALL) 20261002: was == 106 (batch 38); the list shrinks as buttons get wired
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
    # AI(W906-B39-ZALL) 20261002: B -- the five Z buttons of batch 39 are bound (data-acc) and not greyed
    #   (file:// has no C++: if HTMotorAccess.init never resolved, run the page's own binder once with an empty catalog)
    r = json.loads(cdp.eval("""(function(){ var z=document.getElementById('btnInZAllUp'); if(z && !z.getAttribute('data-acc')) teachBindMotionButtons({commands:[]});
      return JSON.stringify(['btnInZAllUp','btnOutZAllUp','btnOutZAllDown','btnSetAllInArmZ','btnSetAllOutArmZ','btnSortZAllUp'].map(function(id){
      var e=document.getElementById(id); return [id, !!(e&&e.getAttribute('data-acc')), !!(e&&e.classList.contains('teach-unwired'))]; })); })()"""))
    check(all(x[1] and not x[2] for x in r[:5]) and (not r[5][1]) and r[5][2], 'B: Z All Up / Out Z All Down / Set All bound; Sort Z All Up still greyed ' + str(r))
finally:
    proc._tree_kill()
print('FAILS', len(FAILS))
sys.exit(len(FAILS))
