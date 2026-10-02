// DOM stub + 全時間軸驗證：整機合併版（雙機型：無/有 Y 變距）
const fs=require('fs');
let t=fs.readFileSync(process.argv[2],'utf8');
let js=t.substring(t.indexOf('<script>')+8, t.indexOf('</script>'));

function Node(tag){ this.tag=tag; this.attrs={}; this.childNodes=[]; this._text=null; this.className=''; }
Node.prototype.setAttribute=function(k,v){
  if(v===undefined||v===null||(typeof v==='number'&&isNaN(v))) throw new Error('bad attr '+k+'='+v+' <'+this.tag+'>');
  if(typeof v==='string'&&/NaN|undefined/.test(v)) throw new Error('NaN/undefined attr '+k+'="'+v+'" <'+this.tag+'>');
  this.attrs[k]=v; };
Node.prototype.getAttribute=function(k){return this.attrs[k]!==undefined?this.attrs[k]:null;};
Node.prototype.appendChild=function(c){this.childNodes.push(c);return c;};
Node.prototype.removeChild=function(c){const i=this.childNodes.indexOf(c);if(i>=0)this.childNodes.splice(i,1);return c;};
Object.defineProperty(Node.prototype,'firstChild',{get(){return this.childNodes[0]||null;}});
Object.defineProperty(Node.prototype,'children',{get(){return this.childNodes;}});
Object.defineProperty(Node.prototype,'textContent',{get(){return this._text;},
  set(v){ if(v===undefined) throw new Error('undefined textContent');
          if(/NaN|undefined/.test(String(v))) throw new Error('NaN/undefined text "'+v+'"'); this._text=v; }});
Object.defineProperty(Node.prototype,'innerHTML',{get(){return '';},set(v){
  if(/undefined|NaN/.test(String(v))) throw new Error('innerHTML bad: '+String(v).slice(0,200));
  this.childNodes.length=0; }});

const byId={};
['iso','tl','head','trayBox','hpBox','binBox','kitBox','axTable','invBox','logBox','statTab','statNote',
 'btnPlay','btnTheme','scrub','jmpA','jmpB','jmpC','rdT','rdOp','rdAct',
 'headO','hdPitchO','hdArmCfgO','fLoad','btnReset','btnSim','btnLive','rdSrc','srcNote','hdTray','hdHP','hdBin','hdPitch','hdArmCfg','hdLog','cntTray','cntHP','cntBin','cntS0','cntS1']
 .forEach(i=>{byId[i]=new Node('div'); byId[i].id=i;});
const axBody=new Node('tbody'), statBody=new Node('tbody');
const CFGB=[['ypitch','0'],['ypitch','1'],['pmode','0'],['pmode','1']].map(p=>{const n=new Node('button');
  n.setAttribute('data-k',p[0]); n.setAttribute('data-v',p[1]); return n;});
const SPDB=['2','8','30','90'].map(v=>{const n=new Node('button');n.setAttribute('data-v',v);return n;});
global.document={
  createElementNS:(ns,tag)=>new Node(tag), createElement:(tag)=>new Node(tag),
  getElementById:(i)=>{ if(!byId[i]) throw new Error('missing id '+i); return byId[i]; },
  querySelector:(s)=>{ if(s==='#axTable tbody') return axBody; if(s==='#statTab tbody') return statBody;
                       throw new Error('qs '+s); },
  querySelectorAll:(s)=>{ if(s==='.spd') return SPDB; if(s==='.cfg') return CFGB; throw new Error('qsa '+s); },
  documentElement:new Node('html'),
  addEventListener:function(){},
  body:{classList:{add:function(){},remove:function(){}}}
};
global.window={matchMedia:()=>({matches:false})};
global.requestAnimationFrame=()=>{};
global.FileReader=function(){};
global.alert=()=>{};

js += "\n;global.__X={SCH:function(){return SCH;},render:render,setP:function(v){P=v;},cfg:cfg,"+
      "rebuild:rebuild,cnt:cnt,totalOf:totalOf,NT:NT,NH:NH,NP:NP,NB:NB,TRAY:TRAY,ARM:ARM,"+
      "TRAY_STEP:TRAY_STEP,TRAY_GAP:TRAY_GAP,HP_GAP:HP_GAP,PIT:PIT,SITE:SITE,"+
      "stepY:function(){return TRAY_STEP_Y;},gapY:function(){return TRAY_GAP_Y;},"+
      "snapAt:snapAt,runningOps:runningOps};\n";
eval(js);
const X=global.__X;

for(const [YP,PM] of [[0,0],[0,1],[1,0],[1,1]]){
  X.cfg.ypitch=YP; X.cfg.pmode=PM; X.rebuild();
  const S=X.SCH(), stepY=X.stepY();
  console.log('\n===== Y 變距 %s ／ Pitch %s =====', YP?'有':'無', PM?'Fixed':'Open-Close');
  console.log('ops=%d  總長=%s s  料件=%d', S.op.length, S.total.toFixed(0), S.TOTAL);

  // 1. 守恆 + 容量
  S.snap.forEach((sn,i)=>{
    const tot=X.totalOf(sn.s);
    if(tot!==S.TOTAL) throw new Error('守恆 '+tot+' @snap'+i);
    if(X.cnt(sn.s.arm)>X.NP||X.cnt(sn.s.oa)>X.NP) throw new Error('arm overflow @'+i);
    if(X.cnt(sn.s.hp)>X.NH) throw new Error('hp overflow @'+i);
    if(X.cnt(sn.s.sk)>8) throw new Error('socket overflow @'+i);
    if(X.cnt(sn.s.bin)>X.NB) throw new Error('bin overflow @'+i);
  });
  // 2. ★ 放料前吸嘴必須滿（Tray 還有料時）
  // 一趟放料可能拆成多個 HPP（上下排分拆），只要「該趟第一個 HPP」時吸嘴是滿的就合規
  let viol=0, violDetail=[];
  S.seq.forEach((o,i)=>{ if(o.kind!=='HPP') return;
    const prev=i>0?S.seq[i-1]:null;
    if(prev && prev.kind==='HPP') return;          // 同一趟的後續放下動作
    const b=S.snap[i].s;
    if(X.cnt(b.arm)<X.NP && X.cnt(b.tray)>0){ viol++; violDetail.push(
      'op'+o.id+' arm='+X.cnt(b.arm)+' tray='+X.cnt(b.tray)+' '+o.lbl.slice(0,50)); } });
  console.log('★ 該趟第一個放料動作時吸嘴未滿且 Tray 仍有料 = %d 次', viol);
  if(viol){ violDetail.slice(0,4).forEach(d=>console.log('   '+d)); throw new Error('取料規則違反'); }
  // 3. ★ 列優先：Tray 取料的列號不可回頭
  let lastRow=-1, back=0;
  S.op.filter(o=>o.kind==='TP').forEach(o=>{
    const rs=o.tray.map(k=>Math.floor(k/X.TRAY.cols));
    const mn=Math.min.apply(null,rs);
    if(mn<lastRow) back++;
    lastRow=Math.max(lastRow,mn);
  });
  console.log('★ Tray 取料列號回頭次數 = %d（列優先）', back);
  if(back) throw new Error('列優先違反');
  // 3b. ★★ 停位互鎖：需要 Shuttle 停在某側的動作，執行期間 Shuttle 必須真的在那一側
  //     （這類缺陷 build 順序看不出來，只有用時間軸才抓得到）
  let phBad=[], mvBad=[];
  const mvOf={S1:S.op.filter(o=>o.act==='S1').sort((a,b)=>a.a-b.a),
              S2:S.op.filter(o=>o.act==='S2').sort((a,b)=>a.a-b.a)};
  S.op.forEach(o=>{
    if(!o.needPh) return;
    const n=o.needPh[0], want=o.needPh[1], L=mvOf['S'+(n+1)];
    // 該 op 開始前，最後一個已完成的移動決定停位（起始為 iLeft=0）
    let ph=0;
    for(const m of L){ if(m.b<=o.a+1e-9) ph=(m.kind==='SR')?1:0; else break; }
    if(ph!==want) phBad.push(o.kind+'#'+o.id+' 需要 SHT'+(n+1)+(want?' iRight':' iLeft')+
      ' 但當時在'+(ph?'iRight':'iLeft'));
    // 動作進行中 Shuttle 不可移動
    for(const m of L) if(m.a<o.b-1e-9 && m.b>o.a+1e-9)
      mvBad.push(o.kind+'#'+o.id+' 進行中 SHT'+(n+1)+' 卻在移動('+m.kind+'#'+m.id+')');
  });
  console.log('★ 停位互鎖：停位不符 %d 件；動作中 Shuttle 移動 %d 件', phBad.length, mvBad.length);
  if(phBad.length){ phBad.slice(0,4).forEach(x=>console.log('   '+x)); throw new Error('停位互鎖違反'); }
  if(mvBad.length){ mvBad.slice(0,4).forEach(x=>console.log('   '+x)); throw new Error('動作中 Shuttle 移動'); }
  // 4. 互斥 / 依賴
  const byAct={}; S.op.forEach(o=>{(byAct[o.act]=byAct[o.act]||[]).push(o);});
  for(const a in byAct){ const L=byAct[a].slice().sort((p,q)=>p.a-q.a);
    for(let i=1;i<L.length;i++) if(L[i].a<L[i-1].b-1e-9) throw new Error('actor '+a+' overlap'); }
  const rs2=S.op.filter(o=>o.res).sort((p,q)=>p.resA-q.resA);
  for(let i=1;i<rs2.length;i++) if(rs2[i].resA<rs2[i-1].resB-1e-9) throw new Error('SOCKET overlap');
  S.op.forEach(o=>o.deps.forEach(d=>{const dd=S.m[d];
    if(dd&&o.a<dd.b-1e-9) throw new Error('dep violated');}));
  // 5. 幾何：依 paired / 單排分別檢查
  S.op.forEach(o=>{
    if(o.kind==='TP'||o.kind==='OPL'){
      const arr=(o.kind==='TP')?o.tray:o.bin;
      if(arr.length!==o.pk.length) throw new Error('cell/pk mismatch op'+o.id);
      // 依吸嘴排分組
      const grp={};
      for(let i=0;i<o.pk.length;i++){
        const pr=Math.floor(o.pk[i]/X.ARM.cols), pc=o.pk[i]%X.ARM.cols;
        (grp[pr]=grp[pr]||[]).push({pc:pc, r:Math.floor(arr[i]/X.TRAY.cols), c:arr[i]%X.TRAY.cols});
      }
      const prs=Object.keys(grp).map(Number).sort();
      if(!o.paired && prs.length>1) throw new Error('單排模式跨吸嘴排 op'+o.id);
      if(o.paired && prs.length!==2) throw new Error('配對模式卻只有一排 op'+o.id);
      prs.forEach(pr=>{
        const g=grp[pr].slice().sort((a,b)=>a.pc-b.pc);
        // 同一排必須同一個盤列
        g.forEach(x=>{ if(x.r!==g[0].r) throw new Error('同排跨盤列 op'+o.id); });
        // 吸嘴欄連續、盤欄以 TRAY_STEP 遞增
        for(let i=1;i<g.length;i++){
          if(g[i].pc!==g[i-1].pc+1) throw new Error('吸嘴欄不連續 op'+o.id);
          if(g[i].c!==g[i-1].c+o.kx) throw new Error('盤欄間距錯 op'+o.id);
        }
      });
      if(o.paired){
        const g0=grp[prs[0]].slice().sort((a,b)=>a.pc-b.pc);
        const g1=grp[prs[1]].slice().sort((a,b)=>a.pc-b.pc);
        if(g0.length!==g1.length) throw new Error('兩排支數不等 op'+o.id);
        if(g1[0].r-g0[0].r!==o.my) throw new Error('兩排盤列差錯 op'+o.id);
        for(let i=0;i<g0.length;i++){
          if(g0[i].c!==g1[i].c) throw new Error('兩排盤欄不同 op'+o.id);
          if(g0[i].pc!==g1[i].pc) throw new Error('兩排吸嘴欄不同 op'+o.id);
        }
      }
    }
    if((o.kind==='HPP'||o.kind==='HPK')&&o.hpc.length!==o.pk.length) throw new Error('hp mismatch op'+o.id);
    if(o.pitch!=null){ const u=o.pitch*100;
      if(u<X.PIT.min1-.5||u>X.PIT.max1+.5) throw new Error('X pitch 超出行程 op'+o.id); }
  });
  // 5-B. ★ Fixed（One by one）：bVariModeFIX=true → GetInArmToLoaderPosition_Single()
  //      會把其餘吸嘴（含另一排）全關掉 → 每針必須恰好 1 支，且下針次數 == 料件總數
  if(X.cfg.pmode===1){
    const bad1=S.op.filter(o=>(o.kind==='TP'||o.kind==='OPL')&&o.pk.length!==1);
    if(bad1.length) throw new Error('Fixed 模式一針不是 1 支：op'+bad1[0].id+' 取'+bad1[0].pk.length);
    const nTP=S.op.filter(o=>o.kind==='TP').length, nPL=S.op.filter(o=>o.kind==='OPL').length;
    if(nTP!==S.TOTAL||nPL!==S.TOTAL)
      throw new Error('Fixed 模式下針次數應 == 料件總數，實得 TP='+nTP+' OPL='+nPL);
    console.log('★ Fixed(One by one)：每針恰 1 支，TP=%d / OPL=%d == 料件 %d', nTP, nPL, S.TOTAL);
  }
  // 6. ★ HotPlate：填滿程度 + 取料是否涵蓋全部列 + FIFO 停留時間
  let maxHP=0; S.snap.forEach(sn=>{ const n=X.cnt(sn.s.hp); if(n>maxHP) maxHP=n; });
  const rowPick=new Array(16).fill(0), rowPlace=new Array(16).fill(0);
  S.op.forEach(o=>{
    if(o.kind==='HPK') o.hpc.forEach(k=>rowPick[Math.floor(k/8)]++);
    if(o.kind==='HPP') o.hpc.forEach(k=>rowPlace[Math.floor(k/8)]++);
  });
  // 正確的不變量：有放過料的列，一定要有被取過（不能有料卡在盤上）
  const stranded=rowPlace.map((v,i)=>(v&&!rowPick[i])?i:null).filter(v=>v!==null);
  const unused=rowPlace.map((v,i)=>v?null:i).filter(v=>v!==null);
  console.log('★ HotPlate 真料在籍峰值 %d/%d；放過但沒取過的列 [%s]；本次未用到的列 [%s]',
    maxHP, X.NH, stranded.join(','), unused.join(','));
  console.log('   逐列取料次數: %s', rowPick.join(' '));
  if(stranded.length) throw new Error('有列放過料卻從未被取料（料卡在盤上）');
  // FIFO：每一次 HPK 取到的料，停留時間（op 差）應接近最大值
  let stayMin=1e9, stayMax=0;
  S.seq.forEach((o,i)=>{ if(o.kind!=='HPK') return;
    const b=S.snap[i].s;
    o.hpc.forEach(k=>{ const st=o.id-b.hpAt[k];
      if(st<stayMin) stayMin=st; if(st>stayMax) stayMax=st; }); });
  console.log('   HotPlate 停留（動作數）最短 %d / 最長 %d', stayMin, stayMax);
  // ★ 水位 + null 佔位：真料不填滿，且 null 標記只增不減（實機行為）
  const hist={}; S.snap.forEach(sn=>{ const n=X.cnt(sn.s.hp); hist[n]=(hist[n]||0)+1; });
  const top=Object.keys(hist).sort((a,b)=>hist[b]-hist[a]).slice(0,3)
    .map(k=>k+'顆×'+hist[k]+'快照').join('  ');
  let nullDrop=0, prevN=0;
  S.snap.forEach(sn=>{ const n=X.cnt(sn.s.hpN);
    if(n<prevN-X.NP) nullDrop++;      // 允許放料時 null 減少（被重用），但不該大量歸零
    prevN=n; });
  const maxNull=Math.max.apply(null,S.snap.map(sn=>X.cnt(sn.s.hpN)));
  // 靜止水位應為 (可用容量 - NP)：留一組空格；只有「放完還沒取」的瞬間才到可用容量
  // 可用容量不是 NH —— 16 列用 (R,R+iYHalf) 配對最多 7 對＝14 列，2 列配不到對
  const restLvl=maxHP-X.NP;
  const restCnt=hist[restLvl]||0, fullCnt=hist[maxHP]||0;
  console.log('★ 真料在籍分布：%s（靜止 %d 顆 ×%d 快照，放完未取瞬間 %d 顆 ×%d）',
    top,restLvl,restCnt,maxHP,fullCnt);
  if(restCnt<fullCnt) throw new Error('靜止水位不是留一組空格（滿盤的時間比留白的還長）');
  // ★ 盤上還有 >=NP 顆時，不可以只取 4 顆就跑去 Shuttle
  //   成因：單排放料生出「單排 4 顆」的 team，而吸嘴兩排固定差 iYHalf 列，永遠湊不回 8 顆
  let underPick=[];
  S.seq.forEach((o,i)=>{ if(o.kind!=='HPK') return;
    const n=X.cnt(S.snap[i].s.hp);
    if(o.pk.length<X.NP && n>=X.NP) underPick.push('op'+o.id+'(取'+o.pk.length+'/盤上'+n+')'); });
  console.log('★ 盤上足量卻未取滿的 HPK = %d 次 %s', underPick.length, underPick.slice(0,5).join(' '));
  if(underPick.length) throw new Error('盤上還有整組料卻只取一部分：'+underPick.slice(0,3).join(' '));
  let over=S.snap.filter(sn=>X.cnt(sn.s.hp)>X.NH).length;
  if(over) throw new Error('真料超過盤容量');
  // 7. 全時間軸渲染
  const N=3000;
  for(let k=0;k<=N;k++){ let p=S.total*k/N; if(p>=S.total) p=S.total-0.001;
    X.setP(p); X.render(); }
  // 8. 期末 + 統計
  const fin=S.snap[S.snap.length-1].s;
  console.log('期末合計=%d　已退盤=%d', X.totalOf(fin), fin.sink);
  if(fin.sink!==S.TOTAL) throw new Error('not all ejected: '+fin.sink);
  const kinds={}; S.op.forEach(o=>kinds[o.kind]=(kinds[o.kind]||0)+1);
  console.log('op:', Object.keys(kinds).map(k=>k+'='+kinds[k]).join(' '));
  const st=S.stats;
  const brk=m=>Object.keys(m).sort((a,b)=>b-a).map(k=>k+'支×'+m[k]).join(' + ');
  console.log('Tray 下針: %s = %d 次（配對 %d 次）', brk(st.dipN), st.tp, st.tpPair);
  console.log('OutArm 放盤: %s = %d 次（配對 %d 次）', brk(st.oplN), st.opl, st.oplPair);
  console.log('HotPlate 放料 %d / 取料 %d 趟（理論下限 %d）；變距 X=%d Y=%d',
    st.hpp, st.hpk, Math.ceil(S.TOTAL/8), st.pitchN, st.pitchYN);
  console.log('相位:', S.ph.map(x=>x.n+'@'+x.t.toFixed(0)).join(' → '));
}
console.log('\nALL OK');


