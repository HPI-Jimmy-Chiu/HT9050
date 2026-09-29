/* ht9045_main_close.js -- 主畫面 Exit 鈕（golden TfMain::sbCloseProgramClick，V912 main.cpp:29051-29131）
 * ---------------------------------------------------------------------------
 * AI(W906-PROD-S95) 20260926（Steven 團隊）：RULINGS_20260926 S95／S120-1「提前做生產資料那幾項」。
 *   C++：WS act.main.closeProgram（FileRW/MainClose.cpp W906_Main_CloseProgramOp），value={"step":0|1|2}。
 *   守衛（運轉中、KYEC 條碼、權限不足時所有馬達要在原點、實跑時機台內有 IC）全在 C++，每一步都重跑；
 *   這裡只照 C++ 回的結果問兩個確認框（golden 的兩個 ShowMyMessageBox_YES_NO）：
 *     step 0 → "Sure close??／確定要關閉程式??"（sbCloseProgramClick :29107）
 *     step 1 → C++ 寫 BinCount.txt、lastdata.dat 之後問 "Sure To Exit?／確定要離開？"（FormClose :11903）
 *     step 2 → C++ 寫 DailyJamRate、Arm*.dat（FormClose :11919／:12195）
 *   ⚠ （S95 原句）只做「先存生產資料」那一半：wb_serve 還沒有正常關站的入口，存完程式不會結束（回應 shutdown.implemented=false，照實告訴操作員）。
 * AI(W906-PROD-S121) 20260926（Steven 團隊）：RULINGS_20260926 S121「要通知 c++ 完全停工，馬達跟加熱都要關掉，安全第一」。
 *   step 2 之後 C++ 回 shutdown.implemented=true／pending=true：wb_serve 下一圈離開主迴圈，照 golden FormClose 停機
 *   （FileRW/MainClose.cpp W906_ProdCloseShutdown），寫 Program Close=1，關掉連線。頁面：
 *     ① 蓋一層「機台停機中…程式即將結束」，列出 C++ 預估**沒有停下來**的項目（shutdown.notStopped：替身、空殼、沒翻、SIM、DRY RUN）；
 *     ② 每 500 ms 看連線，斷了就改成「已結束」；60 秒還沒斷就說出來（請看主控台）。
 *   ⚠ 預估不是結果：真正做了什麼印在 wb_serve 主控台 "MainClose:" 那幾行（做完時網頁已經斷線，收不到）。
 *   ⚠ 關站中不還權杖、不再送任何指令（伺服器也會回 closing 擋掉）。
 * 防連點（S107）：送出中按鈕停用、完成後冷卻 HT9045Busy.coolMs()；伺服器回 busy: 不跳錯誤框（ht9045_busy_util.js）。
 * 權杖：跟 Index CLEAR 同一套（ht9045_showbinselect_wire.js）—— 這一次自己拿的才自己還。
 * ⚠ 不要搬進 ht9045_recipe_client.js／ht9045_wire_engine.js（Jimmy 登記的引擎檔）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var CMD = 'act.main.closeProgram';
  var st = { last: null, lastError: '', saves: 0 };

  function $(id) { return document.getElementById(id); }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function clientHolds() { var s = (window.HT9045Recipe && typeof HT9045Recipe.status === 'function') ? HT9045Recipe.status() : null; return !!(s && s.holdsToken); }
  function isBusy(r) { return !!(window.HT9045Busy && HT9045Busy.is(r)); }
  function send(step) {
    return raw(CMD, { value: JSON.stringify({ step: step }) }).then(unwrap, parseErr);
  }

  // golden ShowMyMessage 的字（C++ 回在 messages；s3 是 golden 的標題）
  function showMessages(r) {
    ((r && r.messages) || []).forEach(function (m) {
      alert((m.s3 ? '[' + m.s3 + ']\n' : '') + (m.s1 || '') + (m.s2 ? '\n' + m.s2 : ''));
    });
  }
  function describe(r) {
    if (!r) return '沒有回應';
    if (/unknown cmd|unknown-action/.test((r.detail || '') + (r.guard || ''))) {
      return 'Exit 的伺服器分派還沒接（' + CMD + ' → FileRW/MainClose.cpp），由整合者加一行。沒有存任何東西。';
    }
    if (/not-operator|control-held/.test(r.detail || '')) return '拿不到控制權杖（別的頁面正持有），沒有存檔';
    return '沒有關閉：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }
  function confirmText(r, fallback) { return (r.prompt || fallback).join('\n'); }

  // ---- AI(W906-PROD-S121) 20260926：關站畫面 ------------------------------------------------------------
  //   伺服器字串一律 textContent（不用 innerHTML）。不改 main.html：蓋層自己建在 body 最後面。
  var shutting = false;
  function el(tag, css, text) {
    var e = document.createElement(tag);
    if (css) e.style.cssText = css;
    if (text !== undefined && text !== null) e.textContent = String(text);
    return e;
  }
  function recipeConnected() {
    var s = (window.HT9045Recipe && typeof HT9045Recipe.status === 'function') ? HT9045Recipe.status() : null;
    return !!(s && s.connected);
  }
  function showShutdown(r, noAck) {
    shutting = true;
    var sd = (r && r.shutdown) || {};
    var old = $('ht9045ExitOverlay');
    if (old && old.parentNode) old.parentNode.removeChild(old);
    var ov = el('div', 'position:fixed;left:0;top:0;right:0;bottom:0;z-index:2147483000;background:rgba(0,0,0,0.72);' +
                       'display:flex;align-items:center;justify-content:center;font-family:inherit;');
    ov.id = 'ht9045ExitOverlay';
    var box = el('div', 'background:#fff;color:#111;max-width:760px;width:92%;max-height:86vh;overflow:auto;padding:18px 22px;' +
                        'border:3px solid #c00;box-shadow:0 4px 24px rgba(0,0,0,0.5);font-size:14px;line-height:1.5;');
    var title = el('div', 'font-size:26px;font-weight:bold;color:#c00;margin-bottom:6px;', '機台停機中…程式即將結束');
    var sub = el('div', 'margin-bottom:10px;', noAck
      ? '沒有收到 Exit 最後一步的確認，但連線已經在關。請看 wb_serve 主控台確認停機結果。'
      : (sd.detail || 'wb_serve 會照 golden FormClose 的順序停機，然後關掉連線。'));
    var stat = el('div', 'font-weight:bold;margin:8px 0;padding:6px 8px;background:#fff3cd;border:1px solid #e0b000;', '等待 wb_serve 關閉連線…');
    box.appendChild(title); box.appendChild(sub); box.appendChild(stat);
    var ns = sd.notStopped || [];
    if (ns.length) {
      box.appendChild(el('div', 'font-weight:bold;color:#c00;margin-top:8px;', '預估沒有停下來的（未停：移植樹的替身、空殼、沒翻、SIM 或 DRY RUN）：'));
      var ul = el('ul', 'margin:4px 0 8px 18px;padding:0;');
      ns.forEach(function (t) { ul.appendChild(el('li', '', t)); });
      box.appendChild(ul);
    }
    var c = sd.counts;
    if (c) box.appendChild(el('div', 'color:#555;font-size:12px;',
      '關站段預估：已停／已做 ' + (c.done || 0) + '、這台沒有 ' + (c.noop || 0) + '、替身 ' + (c.stub || 0) + '、沒翻 ' + (c.missing || 0) +
      '、會被拒 ' + (c.failed || 0) + '、看不出來 ' + (c.unverified || 0) + '（phase=' + (sd.phase || '?') + '，預估不是結果）'));
    box.appendChild(el('div', 'color:#555;font-size:12px;margin-top:4px;', '實際結果：wb_serve 主控台 "MainClose:" 開頭的那幾行（最後一行是完整 JSON）。'));
    ov.appendChild(box);
    document.body.appendChild(ov);
    if (window.console) console.log('[Exit] shutdown plan', sd);

    var t0 = Date.now(), warned = false;
    var timer = setInterval(function () {
      if (!recipeConnected()) {
        clearInterval(timer);
        title.textContent = '已結束';
        title.style.color = '#060';
        box.style.borderColor = '#060';
        stat.textContent = 'wb_serve 已關閉連線（程式已結束）。要再操作，請重新啟動 wb_serve 再重新整理這個頁面。';
        stat.style.background = '#e6f4ea'; stat.style.borderColor = '#060';
        return;
      }
      if (!warned && Date.now() - t0 > 60000) {
        warned = true;
        stat.textContent = '60 秒了連線還沒關：wb_serve 可能卡在存檔或停機的某一步。請看主控台，必要時在機台上按 EMG。';
        stat.style.background = '#f8d7da'; stat.style.borderColor = '#c00';
      }
    }, 500);
  }

  var busy = false, coolUntil = 0;
  function setBusy(b, on) {
    if (!b) return;
    b.style.opacity = on ? '0.5' : '';
    b.style.pointerEvents = on ? 'none' : '';
    b.setAttribute('aria-disabled', on ? 'true' : 'false');
  }

  // opts.answers：測試用，[true|false, true|false] 依序回答兩個確認框；沒給就用 window.confirm（golden 的兩行字）
  function closeProgram(opts) {
    if (shutting) return Promise.resolve({ executed: false, guard: 'closing-local' });   // AI(W906-PROD-S121) 20260926: 關站中
    if (busy || Date.now() < coolUntil) return Promise.resolve({ executed: false, guard: 'busy-local' });
    var b = $('sbCloseProgram');
    var answers = (opts && opts.answers) ? opts.answers.slice() : null;
    function ask(r, fallback) {
      if (answers && answers.length) return !!answers.shift();
      return window.confirm(confirmText(r, fallback));
    }
    busy = true; setBusy(b, true);
    var had = clientHolds(), took = false;
    return raw('control.acquire').then(function () { if (!had) took = true; }, function () { /* 已持有或他人持有：後面的指令自己會回錯 */ })
      .then(function () { return send(0); })
      .then(function (r) {
        showMessages(r);
        if (!(r && r.needConfirm)) return r;
        if (!ask(r, ['Sure close??', '確定要關閉程式??'])) return { executed: false, guard: 'confirm-no', cancelled: true, step: 0 };
        return send(1).then(function (r1) {
          showMessages(r1);
          if (!(r1 && r1.needConfirm)) return r1;
          if (!ask(r1, ['Sure To Exit?', '確定要離開？'])) return { executed: false, guard: 'confirm-no', cancelled: true, step: 1, writes: r1.writes };
          // AI(W906-PROD-S121) 20260926：step 2 之後伺服器會關站；ack 沒到連線就斷（伺服器已經在關）＝ closedBeforeAck
          return raw(CMD, { value: JSON.stringify({ step: 2 }) }).then(unwrap, function (e) {
            var t = (e && e.message) || String(e);
            if (/socket closed before ack/.test(t)) return { executed: false, closedBeforeAck: true, step: 2 };
            return parseErr(e);
          }).then(function (r2) { if (!r2.closedBeforeAck) showMessages(r2); return r2; });
        });
      })
      .then(function (r) {
        st.last = r;
        // AI(W906-PROD-S121) 20260926：關站 —— 蓋層、不跳 alert、不還權杖、按鈕一直停用
        if (r && (r.closedBeforeAck || (r.executed && r.shutdown && r.shutdown.implemented === true))) {
          st.saves += r.executed ? 1 : 0; st.lastError = '';
          showShutdown(r, !!r.closedBeforeAck);
          return r;
        }
        if (r && r.executed) {
          st.saves++; st.lastError = '';
          var lines = ['生產資料已存檔（golden FormClose 的生產資料段）：'];
          (r.writes || []).forEach(function (w) { lines.push('・' + w); });
          if (r.gaps && r.gaps.length) { lines.push('', '沒有做的（缺相依）：'); r.gaps.forEach(function (g) { lines.push('・' + g); }); }
          if (r.shutdown && r.shutdown.implemented === false) lines.push('', r.shutdown.detail || 'wb_serve 還沒有正常關站的入口，程式不會自己結束。');
          alert(lines.join('\n'));
        } else if (r && r.cancelled) {
          st.lastError = '';
          // golden：第一框回 NO 什麼都沒做；第二框回 NO 之前 golden 已經寫了 BinCount.txt 與 lastdata.dat（C++ 回在 writes），程式照常執行
          if (r.step === 1 && r.writes && r.writes.length && window.console) console.log('[Exit] 已取消；golden 在第二框之前已寫：', r.writes);
        } else if (isBusy(r)) {
          st.lastError = '';
          if (b) b.title = HT9045Busy.NOTE;
        } else if (r && r.guard === 'SystemStart') {
          st.lastError = '';                                                  // golden :29054-29055 運轉中按了沒有反應
          if (b) b.title = 'sbCloseProgram：運轉中，golden 按了沒有反應';
        } else {
          st.lastError = describe(r);
          alert(st.lastError);
        }
        return r;
      }, function (e) {
        var r = parseErr(e); st.last = r;
        if (!isBusy(r)) { st.lastError = describe(r); alert(st.lastError); }
        return r;
      })
      .then(function (r) {
        if (shutting) return r;                                               // AI(W906-PROD-S121) 20260926：關站中不還權杖、按鈕保持停用
        var rel = (took && !clientHolds()) ? raw('control.release').then(null, function () {}) : Promise.resolve();
        return rel.then(function () {
          busy = false; setBusy(b, false);
          coolUntil = Date.now() + ((window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400);
          return r;
        });
      });
  }

  function start() {
    var b = $('sbCloseProgram');
    if (!b) return;
    b.title = 'sbCloseProgram（golden sbCloseProgramClick main.cpp:29051 → FormClose：存生產資料，然後 wb_serve 照 golden 停機並結束）';   // AI(W906-PROD-S121) 20260926
    b.addEventListener('click', function () { closeProgram(); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045MainClose = { close: closeProgram, state: function () { return st; } };
})();
