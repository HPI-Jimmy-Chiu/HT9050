/* AI(W906-MV9050-LIVE) 20261007: read-only projection of the current motor
 * snapshot. Teach anchors are emitted on the C++ tick thread; layout stations
 * are schematic, not engineering coordinates. No pulse/mm guess, no playback,
 * no motor or IO commands. Missing readings retain the last pose, dimmed.
 */
(function () {
  'use strict';
  var state = null, entered = false, lastSeq = null, lastAt = 0;
  var inventoryLayer = null, inventoryUnits = {};
  function inventoryUnknown() {
    if (inventoryLayer) { inventoryLayer.setAttribute('opacity','0.25'); inventoryLayer.setAttribute('data-live-quality','unknown'); }
    if(window.UI) window.UI.total.textContent='—（在籍資料不可用）';
    if(window.D && window.D.headIC){
      window.D.headIC.setAttribute('opacity','0');window.D.headOIC.setAttribute('opacity','0');
      window.D.headState.textContent='持料 —（資料不可用）';window.D.headOState.textContent='持料 —（資料不可用）';
    }
  }
  // Actual row/column grids; trays use Data[col][row], kits use Item[row][col].
  // The producer emits both row-major. Rebuild only when dimensions change.
  function inventory(snapshot, quality) {
    var grids=snapshot && snapshot.grids, d=window.D;
    if (!grids || !d.inHead.ownerSVGElement) { inventoryUnknown(); return 'IC 在籍資料未提供'; }
    var ns='http://www.w3.org/2000/svg', svg=d.inHead.ownerSVGElement;
    function node(parent,tag,attrs) {
      var n=document.createElementNS(ns,tag);
      Object.keys(attrs||{}).forEach(function(k){n.setAttribute(k,attrs[k]);}); parent.appendChild(n);return n;
    }
    if (!inventoryLayer || inventoryLayer.ownerSVGElement!==svg) {
      inventoryLayer=node(svg,'g',{'data-live-inventory':'true','pointer-events':'none'}); inventoryUnits={};
      Object.keys(d.trayUnit||{}).forEach(function(k){d.trayUnit[k].cells.forEach(function(c){c.setAttribute('visibility','hidden');});});
      (d.dHP||[]).forEach(function(c){c.setAttribute('visibility','hidden');});
    }
    inventoryLayer.setAttribute('opacity','1');inventoryLayer.setAttribute('data-live-quality','good');
    var defs=[['loader','Loader',null,0],['auto1','Auto1',null,8],['auto2','Auto2',null,9],['auto3','Auto3',null,10],['empty','Empty',null,null],['hotPlate','HotPlate',null,2],
      ['inPP',null,'InPP',1],['outPP',null,'OutPP',7],['inShuttle',null,'InShuttle',3],['outShuttle',null,'OutShuttle',6],['index',null,'Index',4]];
    var total=0, known=0, values={};
    defs.forEach(function(def){
      var key=def[0], g=grids[key], u=inventoryUnits[key], valid=g && Number.isInteger(g.cols) && Number.isInteger(g.rows) && g.cols>0 && g.rows>0 && g.cols<=30 && g.rows<=70 && Array.isArray(g.cells) && g.cells.length===g.cols*g.rows && g.cells.every(function(c){return c && typeof c.hasIC==='boolean' && number(c.raw);});
      if(!valid){if(u)u.group.setAttribute('opacity','0.25');values[key]=null;
        if(def[3]!=null && window.UI.chips[def[3]])window.UI.chips[def[3]].val.textContent='—';
        if(def[1] && d.trayUnit[def[1]])d.trayUnit[def[1]].label.textContent=def[1]+' IC —';return;}
      var module=def[1] && window.M(def[1]), x,y,z,w,h,angle=0;
      if(module){x=module.x;y=module.y;z=3;w=module.w*0.88;h=module.d*0.88;}
      else { w=140;h=110;
        if(key==='inPP'){x=state.inA.x;y=state.inA.y;z=state.inA.z+2;angle=state.InPPAngle||0;}
        if(key==='outPP'){x=state.outA.x;y=state.outA.y;z=state.outA.z+2;angle=state.OutPPAngle||0;}
        if(key==='inShuttle'){x=state.isx;y=window.ST.SHT_Y;z=9;}
        if(key==='outShuttle'){x=state.osx;y=state.osy;z=9;}
        if(key==='index'){x=window.ST.TEST.x;y=window.ST.SHT_Y;z=state.iz+2;}
      }
      var signature=g.cols+'x'+g.rows;
      if(!u || u.signature!==signature){
        if(u)inventoryLayer.removeChild(u.group);
        u={signature:signature,group:node(inventoryLayer,'g',{'data-inventory-unit':key}),cells:[],points:[],raw:[],present:[]};inventoryUnits[key]=u;
        g.cells.forEach(function(){u.cells.push(node(u.group,'polygon',{'stroke':'#738194','stroke-width':'0.5'}));});
      }
      u.group.setAttribute('opacity',def[2] && quality[def[2]]!==true?'0.3':'1');
      var count=0,rad=angle*Math.PI/180;
      g.cells.forEach(function(cell,i){
        if(cell.hasIC)count++;
        var col=i%g.cols,row=Math.floor(i/g.cols),cw=w/g.cols,ch=h/g.rows,dx=-w/2+(col+0.5)*cw,dy=h/2-(row+0.5)*ch;
        var points=[[-.38,.38],[.38,.38],[.38,-.38],[-.38,-.38]].map(function(p){
          var a=dx+p[0]*cw,b=dy+p[1]*ch,q=window.iso(x+a*Math.cos(rad)-b*Math.sin(rad),y+a*Math.sin(rad)+b*Math.cos(rad),z);return q[0].toFixed(1)+','+q[1].toFixed(1);
        }).join(' ');
        var el=u.cells[i];
        if(u.points[i]!==points){el.setAttribute('points',points);u.points[i]=points;}
        if(u.present[i]!==cell.hasIC){el.setAttribute('fill',cell.hasIC?(key.indexOf('auto')===0 || key==='outPP' || key==='outShuttle'?'#2baf68':'#2469d4'):'#edf0f4');el.setAttribute('data-has-ic',String(cell.hasIC));u.present[i]=cell.hasIC;}
        if(u.raw[i]!==cell.raw){el.setAttribute('data-raw',cell.raw);u.raw[i]=cell.raw;}
      });
      u.group.setAttribute('data-ic-count',count); values[key]=count;total+=count;known++;
      // The lower tray tables use exactly the same grid as the isometric view.
      if(window.mkGrid){
        var box=null,store=null,cls='cellT',hasCls='cellT has';
        if(key==='loader'){box=document.getElementById('trayBox');store=window.UI.tray;}
        else if(key==='hotPlate'){box=document.getElementById('hpBox');store=window.UI.hp;cls='cellH';hasCls='cellH s0';}
        else if(window.UI.dest && window.UI.dest[key]){store=window.UI.dest[key];box=store.length && store[0].closest('.gridwrap');hasCls='cellT tst';}
        if(box && store){
          if(box.getAttribute('data-runtime-grid')!==signature){window.mkGrid(box,g.rows,g.cols,cls,store);box.setAttribute('data-runtime-grid',signature);}
          g.cells.forEach(function(c,i){store[i].className=c.hasIC?hasCls:cls;});
        }
        if(window.UI.destCount && window.UI.destCount[key])window.UI.destCount[key].textContent='盤內 '+count+' / '+g.cells.length;
        var kitIndex={inShuttle:0,outShuttle:1,index:2};
        if(kitIndex[key]!=null && window.UI.kit[kitIndex[key]])window.UI.kit[kitIndex[key]].className=count>0?'cellK has':'cellK';
      }
      if(def[3]!=null && window.UI.chips[def[3]])window.UI.chips[def[3]].val.textContent=String(count);
      if(def[1] && d.trayUnit[def[1]])d.trayUnit[def[1]].label.textContent=def[1]+' IC '+count+'/'+g.cells.length;
      if(key==='loader' || key==='hotPlate')document.getElementById(key==='loader'?'hdTray':'hdHP').textContent=g.cols+'×'+g.rows+'（控制程式在籍）';
    });
    // AI(W906-MV9050-TRAYARM) 20261010: EastSun 1010 the Tray Arm frame carries the tray while MOT[MTrayX].fHasTray==true.
    // grids.trayArm = {hasTray, cols/rows/cells of MOT[MTrayX].Tray when valid}; drawn on the Tray Arm at its live X.
    var ta=grids.trayArm, tu=inventoryUnits.trayArm, taHas=!!(ta && ta.hasTray===true);
    if(!tu){tu={group:node(inventoryLayer,'g',{'data-inventory-unit':'trayArm'}),cells:[],signature:''};
      tu.plate=node(tu.group,'polygon',{'fill':'#cdd6e2','stroke':'#3d4d63','stroke-width':'1.2'});inventoryUnits.trayArm=tu;}
    tu.group.setAttribute('visibility',taHas?'visible':'hidden');tu.group.setAttribute('data-has-tray',String(taHas));
    if(taHas){
      var tx=state.tx,ty=window.ST.TRAY_RAIL_Y,tz=46,tw=150,th=88;
      tu.plate.setAttribute('points',[[-tw/2,th/2],[tw/2,th/2],[tw/2,-th/2],[-tw/2,-th/2]].map(function(p){var q=window.iso(tx+p[0],ty+p[1],tz);return q[0].toFixed(1)+','+q[1].toFixed(1);}).join(' '));
      var tg=ta.cols>0 && ta.rows>0 && ta.cols<=30 && ta.rows<=70 && Array.isArray(ta.cells) && ta.cells.length===ta.cols*ta.rows, tsig=tg?ta.cols+'x'+ta.rows:'';
      if(tu.signature!==tsig){tu.cells.forEach(function(c){tu.group.removeChild(c);});tu.cells=[];tu.signature=tsig;
        if(tg)ta.cells.forEach(function(){tu.cells.push(node(tu.group,'polygon',{'stroke':'#738194','stroke-width':'0.5'}));});}
      if(tg)ta.cells.forEach(function(cell,i){
        var col=i%ta.cols,row=Math.floor(i/ta.cols),cw=tw*0.9/ta.cols,ch=th*0.9/ta.rows,dx=-tw*0.45+(col+0.5)*cw,dy=th*0.45-(row+0.5)*ch;
        tu.cells[i].setAttribute('points',[[-.38,.38],[.38,.38],[.38,-.38],[-.38,-.38]].map(function(p){var q=window.iso(tx+dx+p[0]*cw,ty+dy+p[1]*ch,tz+1);return q[0].toFixed(1)+','+q[1].toFixed(1);}).join(' '));
        tu.cells[i].setAttribute('fill',cell && cell.hasIC===true?'#2469d4':'#edf0f4');
      });
    }
    d.headIC.setAttribute('opacity',values.inPP>0?'1':'0');d.headOIC.setAttribute('opacity',values.outPP>0?'1':'0');
    d.headState.textContent='持料 '+(values.inPP==null?'—':values.inPP)+' IC｜真空 —';
    d.headOState.textContent='持料 '+(values.outPP==null?'—':values.outPP)+' IC｜真空 —';
    document.getElementById('cntTray').textContent=values.loader==null?'—':values.loader+' / '+grids.loader.cells.length;
    document.getElementById('cntHP').textContent=values.hotPlate==null?'—':values.hotPlate+' / '+grids.hotPlate.cells.length;
    window.UI.total.textContent=total+'（已接 '+known+'/11 區；Socket 結果不重複計數）';
    document.getElementById('trayLocSrc').textContent='控制程式逐格在籍：藍＝IC、綠＝出料 IC、淡色＝空格';
    return 'IC '+total+' 顆（'+known+'/11 區）';
  }
  function number(v) { return typeof v === 'number' && isFinite(v); }
  function pose() {
    var s = window.ST;
    return { inA: { x:s.HP.x, y:s.HP.y, z:s.Z_ARM_SAFE },
      outA: { x:s.OSHT.x, y:s.OUT_CYL.back, z:s.Z_ARM_SAFE },
      isx:s.ISHT.x, osx:s.OSHT.x, osy:s.SHT_Y, iz:s.Z_IDX_SAFE, tx:s.LOADER.x };
  }
  function endpoint(name, axis, module) {
    var s = window.ST;
    if (axis === 'theta') return name === 'zero' ? 0 : (name === 'quarterTurn' ? 90 : null);
    if (axis === 'z') {
      if (name === 'safe') return module === 'Index' ? s.Z_IDX_SAFE : s.Z_ARM_SAFE;
      if (name === 'dut') return s.Z_DUT;
      if (name === 'down') return 8;
      return null;
    }
    var map = { loader:s.LOADER, inShuttle:s.ISHT, outShuttle:s.OSHT,
      outShuttlePick:{x:s.OSHT.x,y:s.SHT_Y-s.OSHT_Y_PICK},
      test:s.TEST, auto1:s.AUTO[0], auto3:s.AUTO[2] };
    return map[name] ? map[name][axis] : null;
  }
  function coordinate(row, binding) {
    var p = row && row.position, a = row && row.motionView;
    var value = p && (number(p.cmdPos) ? p.cmdPos : p.encPos);
    if (!row || row.enabled !== true || !number(value) || !a || !number(a.factStart) || !number(a.factEnd) || a.factStart === a.factEnd) return null;
    var start = endpoint(a.from, binding.axis, binding.module);
    var end = endpoint(a.to, binding.axis, binding.module);
    if (!number(start) || !number(end)) return null;
    // Same linear rule as SetScreenScale; negative/reversed teach ranges are valid.
    return start + (value - a.factStart) / (a.factEnd - a.factStart) * (end - start);
  }
  function dim(nodes, unknown) {
    nodes.forEach(function (key) {
      var el = window.D && window.D[key];
      if (el) { el.setAttribute('opacity', unknown ? '0.3' : '1');
        el.setAttribute('data-live-quality', unknown ? 'unknown' : 'good'); }
    });
  }
  var groups = {
    InPP:['inHead','inPost','inBeam','inCar','inLab','headAngleNeedle','headAngleText'],
    OutPP:['outHead','outPost','outBeam','outCyl','outCar','outLab','headOAngleNeedle','headOAngleText'],
    InShuttle:['ishtPlate','ishtKit','ishtLab'],
    OutShuttle:['oshtPlate','oshtKit','oshtLab'],
    Index:['idxPost','idxHead','idxLab'], TrayArm:['trayCar']
  };
  function unavailable() {
    inventoryUnknown();
    Object.keys(groups).forEach(function (k) { dim(groups[k], true); });
    var status=document.getElementById('liveStatus');
    if(status){status.textContent='即時位置資料不可用；淡色機構保留最後姿態。';status.className='note runtime-status warn';}
    var tag=document.getElementById('modeTag');if(tag)tag.textContent='資料不可用';
  }
  function enter() {
    window.stopSim();
    window.LIVE = window.LIVE || { machine:'HT9050', motorOnly:true };
    ['btnPlay','btnStep','btnReset','selSpeed','chkTopAOI'].forEach(function (id) {
      var el = document.getElementById(id); if (el) el.disabled = true;
    });
    if (entered) return;
    entered = true;
    // Do not keep startup demo IC inventories in a live motor-only view.
    window.applyLive({ machine:'HT9050' });
    window.D.headState.textContent = '真空 —'; window.D.headOState.textContent = '真空 —';
    ['cntTray','cntHP','cntHPOut','cntHPSoak'].forEach(function (id) {
      document.getElementById(id).textContent = '—';
    });
    window.UI.chips.forEach(function (c) { c.val.textContent = '—'; });
    window.UI.total.textContent = '—（未接在籍資料）';
    document.getElementById('simTag').textContent = '依目前馬達位置更新；不需播放。版面為示意座標。';
    document.getElementById('srcLead').textContent = '目前位置來自控制程式，依載入的教導點對應到示意版面。';
    document.getElementById('paramNote').textContent = '圖形反映目前位置；機台尺寸、站點與外觀仍為示意，不能用來量測實際距離。';
    document.getElementById('trayLocSrc').textContent = '在籍資料尚未接線';
  }
  function cylinder(rt, io) {
    var c = rt.runtime && rt.runtime.outArmYCylinder;
    if (c && c.source === 'simulation command' && typeof c.on === 'boolean') return c.on;
    if (!io || !io.runtime || io.runtime.connected !== true || !io.runtime.lastPollAt ||
        !isFinite(Date.parse(io.runtime.lastPollAt)) || Date.now()-Date.parse(io.runtime.lastPollAt)>3000) return null;
    var on = null, off = null;
    (io.points || []).forEach(function (p) {
      if (p.quality !== 'good' || typeof p.isOn !== 'boolean') return;
      var alias = String(p.ioId || '').split('@')[0];
      if (alias === 'C_OutArmSmallY_On') on = p.isOn;
      if (alias === 'C_OutArmSmallY_Off') off = p.isOn;
    });
    if (on === true && off === false) return true;
    if (off === true && on === false) return false;
    return null; // Both on/off or neither: intermediate/invalid, not a guessed endpoint.
  }
  function apply(rt, io) {
    if (!window.D || !window.D.inHead || !window.LAYOUT) return { ready:false, detail:'等待版面載入' };
    enter();
    var r = rt && rt.runtime, now = Date.now();
    if (!r || r.connected !== true || r.seq == null) { unavailable(); return { ready:false, detail:'尚未收到有效位置資料' }; }
    if (r.seq !== lastSeq) { lastSeq = r.seq; lastAt = now; }
    var stamp = Date.parse(r.lastPollAt);
    if (now-lastAt>3000 || !isFinite(stamp) || now-stamp>3000) {
      unavailable(); return { ready:false, detail:'位置資料過期，保留最後姿態' };
    }
    if (!state) state = pose();
    var rows = {}, indices = {}, missing = [], quality = {}, count = 0;
    (rt.motors || []).forEach(function (m) { rows[m.motorId] = m;
      if (number(m.motorIndex)) indices[m.motorIndex] = m; });
    (window.AXES.bindings || []).forEach(function (b) {
      if (b.type === 'cylinder') return;
      var row = b.motorNo ? indices[Number(b.motorNo.slice(1))] : rows[b.motorId];
      var v = coordinate(row, b), key = b.module + '.' + b.axis;
      if (!number(v)) { missing.push(b.label || b.motorId); quality[b.module] = false; return; }
      if (quality[b.module] !== false) quality[b.module] = true;
      var targets = { 'InPP.x':['inA','x'], 'InPP.y':['inA','y'], 'InPP.z':['inA','z'],
        'OutPP.x':['outA','x'], 'OutPP.y':['outA','y'], 'OutPP.z':['outA','z'],
        'InShuttle.x':['isx'], 'OutShuttle.x':['osx'], 'OutShuttle.y':['osy'],
        'Index.z':['iz'], 'TrayArm.x':['tx'] };
      var target = targets[key];
      if (target) { if (target.length === 2) state[target[0]][target[1]] = v; else state[target[0]] = v; count++; }
      if (b.axis === 'theta') { state[b.module+'Angle'] = v; count++; }
    });
    var cy = cylinder(rt, io);
    // SmallY's geometric purpose is not decided (AR-5); report it, don't replace M20.
    var simulated=rt.runtime.outArmYCylinder&&rt.runtime.outArmYCylinder.source==='simulation command';
    var cylText = 'SmallY 氣缸'+(simulated?'（模擬指令）':'（到位感測）')+'：' + (cy === null ? '未知' : (cy ? 'ON' : 'OFF'));
    window.placeAll(state.inA,state.outA,state.isx,state.osx,state.osy,state.iz,null,null,null,false,false);
    window.trPos(window.D.trayCar,state.tx,window.ST.TRAY_RAIL_Y,40);
    function rotation(module, prefix, head) {
      var angle = state[module+'Angle'];
      if (!number(angle)) return;
      var rad = angle*Math.PI/180, origin = window.iso(0,0,0);
      var points = [[-100,90],[100,90],[100,-90],[-100,-90]].map(function (p) {
        var q=window.iso(p[0]*Math.cos(rad)-p[1]*Math.sin(rad),p[0]*Math.sin(rad)+p[1]*Math.cos(rad),0);
        return (q[0]-origin[0]).toFixed(1)+','+(q[1]-origin[1]).toFixed(1);
      }).join(' ');
      window.D[head].setAttribute('points',points);
      window.D[prefix+'AngleNeedle'].setAttribute('transform','rotate('+angle+',210,70)');
      window.D[prefix+'AngleText'].textContent='θ = '+angle.toFixed(1)+'°';
    }
    rotation('InPP','head','inHead'); rotation('OutPP','headO','outHead');
    var icDetail=inventory(rt.motionViewInventory,quality);
    Object.keys(groups).forEach(function (k) { dim(groups[k], quality[k] !== true); });
    document.getElementById('modeTag').textContent = 'LIVE';
    document.getElementById('tPhase').textContent = '目前馬達位置';
    document.getElementById('logBox').textContent = '顯示目前位置與程式內逐格在籍；不需播放。持料隨手臂／飛梭／Index 移動。';
    var detail = '立體圖已接上 ' + count + ' 軸｜' + icDetail + '｜' + cylText + (missing.length ? '｜缺資料／教導對應：' + missing.join('、') : '');
    var status = document.getElementById('liveStatus');
    status.textContent = detail + '。淡色機構保留最後姿態，不能當成目前位置。';
    status.className = 'note runtime-status' + (missing.length ? ' warn' : ' live');
    return { ready:count>0, detail:detail };
  }
  window.HT9045Mv9050Mechanics = { apply:apply, unavailable:unavailable, coordinate:coordinate };
})();
