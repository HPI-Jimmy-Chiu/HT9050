/* HT9050 概念頁驗證腳本 —— 貼到 DevTools console（或 Playwright page.evaluate）執行
 * 頁面：file:///D:/HT9045/page/IDE.MotionView9050-Concept.html（可帶 #hash 覆寫秒數）
 * 每個 cycle 取 60 個時間點掃描（In P&P 軌與測區軌平行，不能只看主鏈索引）：
 *   1. chips 合計 = LOT / LOT（料件守恆）
 *   2. Hot plate 亮格數 = 卡片「料 n / N」 = chips「Hot plate」
 *   3. 大 IC（D.ic / D.ic2 / D.ic3）不得落在已亮的 Hot plate 格點上（雙重顯示）
 *   4. Loader 盤格數 = 卡片「盤內 n / N」
 *   5. 吸嘴 inset 持料指示與 chips「In P&P」/「Out P&P」一致
 *   6. 暖機最後一輪結束時 Hot plate 補到 HP_CAP-1（B 仍在吸嘴）；kit0 開始 Hot plate=HP_CAP-1（C 立刻被取走）
 *   7. 每個正式 cycle 起點（SHT_IN）Hot plate 必須是滿的（HP_CAP），且 In Shuttle KIT 上有料
 * 回傳 {ok, errors[]}；ok=true 表示全部通過
 */
(function verifyConcept(){
  var errors=[], prevPlay=PLAY, prevT=T, prevCyc=CYC, N=60;
  PLAY=false;
  var maxC=LOT-1, c, s, chips, hpLit, card, tray, trayCard, inIC, outIC;
  function chipMap(){
    var m={}; document.querySelectorAll('#invBox .chip').forEach(function(e){
      var mm=e.textContent.trim().match(/^(.+?)\s+(\d+)/); if(mm) m[mm[1]]=+mm[2]; });
    return m;
  }
  for(c=0;c<=maxC;c++){ CYC=c;
    var L=cycleLen();
    for(s=0;s<N;s++){
      T=(s+0.5)*L/N; lastPhase=-1; render();
      var tag="C"+c+" t="+T.toFixed(2)+": ";
      var tot=document.getElementById('invBox').lastChild.textContent.trim();
      if(!new RegExp(LOT+" / "+LOT+"$").test(tot)) errors.push(tag+"守恆失敗 "+tot);
      chips=chipMap();
      hpLit=D.dHP.filter(function(e){return e.getAttribute('fill')==='var(--s0)';}).length;
      card=parseInt(document.getElementById('cntHP').textContent);
      if(hpLit!==card||hpLit!==chips['Hot plate']) errors.push(tag+"Hot plate 不一致 lit="+hpLit+" card="+card+" chip="+chips['Hot plate']);
      [D.ic,D.ic2,D.ic3].forEach(function(b){ if(b.getAttribute('opacity')!=='1') return;
        var cx=+b.getAttribute('cx'), cy=+b.getAttribute('cy');
        D.dHP.forEach(function(e){ if(e.getAttribute('fill')==='var(--s0)'){
          var x=+e.getAttribute('x')+1.5,y=+e.getAttribute('y')+1.5; if(Math.hypot(x-cx,y-cy)<6) errors.push(tag+"大 IC 與 Hot plate 格點重複"); } }); });
      tray=D.trayUnit.Loader.cells.filter(function(e){return e.getAttribute('visibility')==='visible';}).length;
      trayCard=parseInt(document.getElementById('cntTray').textContent);
      if(tray!==trayCard) errors.push(tag+"Loader 盤格 "+tray+" ≠ 卡片 "+trayCard);
      inIC=D.headIC.getAttribute('opacity')==='1'?1:0; outIC=D.headOIC.getAttribute('opacity')==='1'?1:0;
      if(inIC!==(chips['In P&P']||0)) errors.push(tag+"In P&P inset "+inIC+" ≠ chip "+(chips['In P&P']||0));
      if(outIC!==(chips['Out P&P']||0)) errors.push(tag+"Out P&P inset "+outIC+" ≠ chip "+(chips['Out P&P']||0));
    }
    /* 暖機最後一輪：IN_HP 放下後 Hot plate 應補滿 */
    if(c===PRIME_C-1){ T=cycleLen()-0.001; lastPhase=-1; render();
      var hpEnd=parseInt(document.getElementById('cntHP').textContent);
      if(hpEnd!==HP_CAP-1) errors.push("暖機最後一輪末端 Hot plate="+hpEnd+"（B 仍在吸嘴），下一輪開始應為 "+HP_CAP); }
    /* 正式 cycle 起點：Hot plate 滿、In Shuttle KIT 有料 */
    if(c>PRIME_C){ T=0.001; lastPhase=-1; render();
      var hp0=parseInt(document.getElementById('cntHP').textContent);
      if(hp0!==HP_CAP) errors.push("C"+c+" 起點 Hot plate="+hp0+"，應為滿盤 "+HP_CAP);
      if(!/has/.test(UI.kit[0].className)) errors.push("C"+c+" 起點 In Shuttle KIT 應有料"); }
  }
  CYC=PRIME_C; T=0.001; lastPhase=-1; render();
  var hpFull=parseInt(document.getElementById('cntHP').textContent);
  if(hpFull!==HP_CAP-1) errors.push("kit0 第一次 IN_KIT 開始：Hot plate 應為 "+(HP_CAP-1)+"（滿盤取走 1 顆），實得 "+hpFull);
  CYC=prevCyc; T=prevT; PLAY=prevPlay; lastPhase=-1; render();
  var out={ok:errors.length===0, LOT:LOT, HP_CAP:HP_CAP, PRIME_C:PRIME_C, CYCLE:+CYCLE.toFixed(2),
           samples:(maxC+1)*N, errors:errors};
  console.log(out.ok?"✅ verifyConcept PASS":"❌ verifyConcept FAIL", out);
  return out;
})();
