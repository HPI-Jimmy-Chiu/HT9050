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
 * AI(W906-FRW-S167) 20260928 [W906] Q44（最小版）「按 Exit 必須先停下才能關閉」。Steven 20260928：「至少馬達跟溫度的要停下來，
 *   這個只要兩個命令就可以做到了」＋追問三題都選建議。step 2 回 shutdown.q44 ⇒ C++ 在主迴圈裡（連線還在）送兩個命令
 *   （停全部馬達、關加熱器繼電器）並讀回確認；本頁每 800 ms 問一次 act.main.closeProgram {"op":"status"}：
 *     stopping  → 「機台停機中…」（逐項列：已停／等讀回）；
 *     confirmed → 「已停，程式結束中…」，接著連線斷 → 「已結束」；
 *     blocked   → 紅字「無法關閉：以下還沒停」＋逐項原因＋［重試停機］［強制關閉］［收起］。擋下之後不再輪詢，權杖還回去
 *                 （讓 1203 頁／IO 頁／登入能用）；按鈕時再拿。
 *     forced    → 「強制關閉中…程式即將結束」。
 *   ［強制關閉］：先看 C++ 回的等級（q44.force：needLevel＝golden Exit 的 LevelSet.AccessLevel[6]、level＝目前登入等級），不夠就說明、不送；
 *     夠了再確認一次（列出還沒停的），送 {"op":"force","confirm":true}；C++ 重查等級、記 log「強制關閉，未停：…」。
 *   其餘關站步驟（step 2 的 shutdown.notStopped 預估）照舊列出，不擋關閉（待辦 D-007）。
 *   重新整理之後再按 Exit：伺服器回 guard=closing ⇒ 接回同一個蓋層（問 status）。
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
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
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
  //   AI(W906-FRW-S167) 20260928：蓋層加 Q44 的逐項清單、三個鈕、收起後的橫條。
  var shutting = false, ended = false;
  var ui = null;
  var Q44_POLL_MS = 800;                                                     // 伺服器防連點：同一個 value 400 ms 內重送會回 busy:
  var q44 = { on: false, timer: null, phase: '', last: null, inflight: false, coolUntil: 0, took: false, hidden: false, lastOkAt: 0, fails: 0 };

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
  function mkBtn(text, css) {
    var b = el('button', 'margin:6px 8px 0 0;padding:6px 14px;font-size:15px;cursor:pointer;' + (css || ''), text);
    b.type = 'button';
    return b;
  }
  function removeById(id) { var o = $(id); if (o && o.parentNode) o.parentNode.removeChild(o); }

  function buildOverlay() {
    removeById('ht9045ExitOverlay'); removeById('ht9045ExitBanner');
    var ov = el('div', 'position:fixed;left:0;top:0;right:0;bottom:0;z-index:2147483000;background:rgba(0,0,0,0.72);' +
                       'display:flex;align-items:center;justify-content:center;font-family:inherit;');
    ov.id = 'ht9045ExitOverlay';
    var box = el('div', 'background:#fff;color:#111;max-width:760px;width:92%;max-height:86vh;overflow:auto;padding:18px 22px;' +
                        'border:3px solid #c00;box-shadow:0 4px 24px rgba(0,0,0,0.5);font-size:14px;line-height:1.5;');
    var title = el('div', 'font-size:26px;font-weight:bold;color:#c00;margin-bottom:6px;', '機台停機中…程式即將結束');
    var sub = el('div', 'margin-bottom:10px;', '');
    var stat = el('div', 'font-weight:bold;margin:8px 0;padding:6px 8px;background:#fff3cd;border:1px solid #e0b000;', '');
    var list = el('div', 'margin:4px 0 6px 0;');                             // Q44 逐項
    var btns = el('div', 'display:none;margin:6px 0 4px 0;');
    var bRetry = mkBtn('重試停機', 'font-weight:bold;');
    var bForce = mkBtn('強制關閉', 'color:#fff;background:#a00;border:1px solid #600;');
    var bHide = mkBtn('收起（去 1203 頁／IO 頁／登入）', '');
    btns.appendChild(bRetry); btns.appendChild(bForce); btns.appendChild(bHide);
    var hint = el('div', 'display:none;color:#333;font-size:13px;margin:4px 0 8px 0;',
      '可以到 1203 頁或 IO 頁把它們關掉，或在機台上照程序處理（例如按 EMG），再按［重試停機］。' +
      '讀回永遠不會成功時（例如 1203 卡失聯），照機台程序停機後按［強制關閉］（要 Exit 的權限等級，會記 log）。' +
      '在這之前程式不會結束，也回不去生產；要生產只能重開 wb_serve。');
    var other = el('div', 'margin-top:10px;');
    var foot = el('div', 'color:#555;font-size:12px;margin-top:4px;', '實際結果：wb_serve 主控台 "[Q44]"／"MainClose:" 開頭的那幾行（最後一行是完整 JSON）。');
    box.appendChild(title); box.appendChild(sub); box.appendChild(stat); box.appendChild(list); box.appendChild(btns);
    box.appendChild(hint); box.appendChild(other); box.appendChild(foot);
    ov.appendChild(box);
    document.body.appendChild(ov);
    var banner = el('div', 'display:none;position:fixed;left:0;right:0;top:0;z-index:2147483000;background:#c00;color:#fff;' +
                           'padding:6px 12px;font-weight:bold;font-size:15px;cursor:pointer;', '');
    banner.id = 'ht9045ExitBanner';
    document.body.appendChild(banner);
    bRetry.addEventListener('click', q44Retry);
    bForce.addEventListener('click', q44Force);
    bHide.addEventListener('click', function () { q44Hide(true); });
    banner.addEventListener('click', function () { q44Hide(false); });
    ui = { ov: ov, box: box, title: title, sub: sub, stat: stat, list: list, btns: btns, bRetry: bRetry, bForce: bForce, bHide: bHide,
           hint: hint, other: other, banner: banner };
  }
  function setTitle(text, color) { ui.title.textContent = text; ui.title.style.color = color; ui.box.style.borderColor = color; }
  function setStat(text, kind) {
    ui.stat.textContent = text;
    var c = kind === 'ok' ? ['#e6f4ea', '#060'] : kind === 'bad' ? ['#f8d7da', '#c00'] : ['#fff3cd', '#e0b000'];
    ui.stat.style.background = c[0]; ui.stat.style.borderColor = c[1];
  }

  function showShutdown(r, noAck) {
    shutting = true;
    var sd = (r && r.shutdown) || {};
    buildOverlay();
    ui.sub.textContent = noAck
      ? '沒有收到 Exit 最後一步的確認，但連線已經在關。請看 wb_serve 主控台確認停機結果。'
      : (sd.detail || 'wb_serve 會照 golden FormClose 的順序停機，然後關掉連線。');
    setStat(sd.q44 ? '停機中：停全部馬達、關加熱器繼電器，等讀回確認…' : '等待 wb_serve 關閉連線…', 'wait');
    var ns = sd.notStopped || [];
    if (ns.length) {
      ui.other.appendChild(el('div', 'font-weight:bold;color:#a60;margin-top:8px;',
        '其他關站步驟預估沒有停下來的（不擋關閉，列在待辦；未停：移植樹的替身、空殼、沒翻、SIM 或 DRY RUN）：'));
      var ul = el('ul', 'margin:4px 0 8px 18px;padding:0;');
      ns.forEach(function (t) { ul.appendChild(el('li', '', t)); });
      ui.other.appendChild(ul);
    }
    if (sd.q44 && (sd.q44Covered || []).length) {                            // AI(W906-FRW-S167) 20260928：停馬達、關加熱器繼電器在上面的 Q44 清單，這裡不重列
      ui.other.appendChild(el('div', 'color:#555;font-size:12px;margin:2px 0 6px 0;',
        '停全部馬達、關加熱器繼電器這 ' + sd.q44Covered.length + ' 步在關站段之前先做、並讀回確認（上面的清單，以讀回為準），這裡不重列。'));
    }
    var c = sd.counts;
    if (c) ui.other.appendChild(el('div', 'color:#555;font-size:12px;',
      '關站段預估：已停／已做 ' + (c.done || 0) + '、這台沒有 ' + (c.noop || 0) + '、替身 ' + (c.stub || 0) + '、沒翻 ' + (c.missing || 0) +
      '、會被拒 ' + (c.failed || 0) + '、看不出來 ' + (c.unverified || 0) + '（phase=' + (sd.phase || '?') + '，預估不是結果）'));
    if (window.console) console.log('[Exit] shutdown plan', sd);

    var t0 = Date.now(), warned = false;
    var timer = setInterval(function () {
      if (!recipeConnected()) {
        clearInterval(timer);
        setEnded();
        return;
      }
      // 60 秒還沒斷：沒有 Q44 的舊伺服器，或 Q44 已經確認／強制之後還沒斷（擋下等按鈕是正常的，不警告）
      var stuck = !q44.on || q44.phase === 'confirmed' || q44.phase === 'forced';
      if (!warned && stuck && Date.now() - t0 > 60000) {
        warned = true;
        setStat('60 秒了連線還沒關：wb_serve 可能卡在存檔或停機的某一步。請看主控台，必要時在機台上按 EMG。', 'bad');
      }
    }, 500);
    if (sd.q44) q44Start();
  }

  function setEnded() {
    ended = true;
    if (q44.timer) { clearTimeout(q44.timer); q44.timer = null; }
    q44Hide(false);
    ui.btns.style.display = 'none'; ui.hint.style.display = 'none';
    if (q44.on && q44.phase === 'blocked') {
      setTitle('連線斷了', '#c00');
      setStat('wb_serve 已結束（可能是主控台 Ctrl-C 或 --seconds 到期），上面還沒停的項目沒有確認。請看主控台，必要時在機台上按 EMG。', 'bad');
      return;
    }
    setTitle('已結束', '#060');
    setStat('wb_serve 已關閉連線（程式已結束）。要再操作，請重新啟動 wb_serve 再重新整理這個頁面。', 'ok');
  }

  // ---- AI(W906-FRW-S167) 20260928 [W906] Q44：輪詢、逐項清單、重試、強制、收起 ------------------------------------
  function q44Start() {
    q44.on = true;
    if (!q44.timer) q44.timer = setTimeout(q44Poll, Q44_POLL_MS);
  }
  function q44Send(v) {
    var pre = clientHolds() ? Promise.resolve() : raw('control.acquire').then(function () { q44.took = true; }, function () { /* 別人持有：下面的指令自己會回 not-operator */ });
    return pre.then(function () { return raw(CMD, { value: JSON.stringify(v) }); }).then(unwrap, function (e) {
      var t = (e && e.message) || String(e);
      if (/socket closed before ack|cannot open/.test(t)) return { closedBeforeAck: true };
      return parseErr(e);
    });
  }
  function q44Release() {
    // 同 closeProgram 尾端的規則：本頁用 raw control.acquire 拿的（ht9045_recipe_client.js 的 haveToken 不會跟著設）才由本頁還
    if (q44.took && !clientHolds()) raw('control.release').then(null, function () {});
    q44.took = false;
  }
  function q44Poll() {
    q44.timer = null;
    if (ended || !shutting || q44.phase === 'blocked') return;               // 擋下之後不輪詢，等按鈕
    q44Send({ op: 'status' }).then(function (r) {
      if (r && !r.closedBeforeAck && !isBusy(r)) {
        if (r.q44) { q44.fails = 0; q44.lastOkAt = Date.now(); q44Show(r); }
        else if (++q44.fails >= 3) setStat('問不到停機結果：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + '；繼續問…', 'bad');
      }
    }, function () {}).then(function () {
      if (!ended && shutting && q44.phase !== 'blocked' && !q44.timer) q44.timer = setTimeout(q44Poll, Q44_POLL_MS);
    });
  }
  var MARK = { ok: ['已停', '#060'], absent: ['這台沒有', '#555'], unverified: ['看不到結果（不擋）', '#a60'], wait: ['等讀回', '#a60'], blocked: ['還沒停', '#c00'] };
  function q44Items(q) {
    while (ui.list.firstChild) ui.list.removeChild(ui.list.firstChild);
    var items = q.items || [];
    if (!items.length) return;
    var ul = el('ul', 'margin:4px 0 4px 18px;padding:0;');
    items.forEach(function (it) {
      var m = MARK[it.state] || [it.state || '?', '#111'];
      var li = el('li', 'margin:2px 0;');
      li.appendChild(el('span', 'font-weight:bold;color:' + m[1] + ';', '［' + m[0] + '］'));
      li.appendChild(el('span', 'font-weight:bold;', ' ' + (it.what || it.key || '')));
      li.appendChild(el('div', 'color:#333;font-size:13px;', it.reason || ''));
      ul.appendChild(li);
    });
    ui.list.appendChild(ul);
  }
  function q44Show(r) {
    var q = r.q44 || {};
    q44.last = q; q44.phase = q.phase || '';
    q44Items(q);
    var blocked = q44.phase === 'blocked';
    ui.btns.style.display = blocked ? 'block' : 'none';
    ui.hint.style.display = blocked ? 'block' : 'none';
    if (q44.phase === 'confirmed') {
      setTitle('已停，程式結束中…', '#060');
      setStat('馬達與加熱器繼電器讀回確認已停（或這台沒有）；wb_serve 照 golden 做其餘關站步驟後結束。', 'ok');
    } else if (q44.phase === 'forced') {
      setTitle('強制關閉中…程式即將結束', '#c00');
      setStat('已強制關閉，未停：' + (q.forcedText || '?') + '（記在 wb_serve 主控台與 D:\\HT9045\\Error\\BootLog.txt）', 'bad');
    } else if (blocked) {
      setTitle('無法關閉：以下還沒停', '#c00');
      var ns = q.notStopped || [];
      setStat('還沒停：' + (ns.length ? ns.join('；') : '?') + '（第 ' + (q.attempt || 0) + ' 次停機）', 'bad');
      var f = q.force || {};
      ui.bForce.title = '要權限等級 ≥ ' + f.needLevel + '（Exit 的 LevelSet.AccessLevel[6]），目前 ' + f.level;
      ui.banner.textContent = '關站中：無法關閉，還沒停 ' + ns.length + ' 項 —— 按這裡打開（程式在這之前不會結束）';
      q44Release();                                                          // 讓 1203 頁／IO 頁／登入拿得到權杖
    } else {
      setTitle('機台停機中…', '#c00');
      setStat('停機中（第 ' + (q.attempt || 1) + ' 次）：停全部馬達、關加熱器繼電器，等讀回確認（最多 ' + Math.round((q.settleMs || 5000) / 1000) + ' 秒）…', 'wait');
    }
  }
  function setBtns(off) {
    [ui.bRetry, ui.bForce].forEach(function (b) {
      b.disabled = !!off; b.style.opacity = off ? '0.5' : '';
    });
  }
  function q44Retry() {
    if (q44.inflight || Date.now() < q44.coolUntil || q44.phase !== 'blocked') return;
    q44.inflight = true; setBtns(true);
    q44Send({ op: 'retry' }).then(function (r) {
      if (r && r.executed && r.q44) {
        q44Show(r);                                                          // phase=stopping（伺服器下一圈重送兩個命令）
        if (!q44.timer) q44.timer = setTimeout(q44Poll, Q44_POLL_MS);
      } else if (r && !r.closedBeforeAck && !isBusy(r)) {
        alert('沒有重試：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : ''));
        if (r.q44) q44Show(r);
      }
    }).then(function () {
      q44.inflight = false; setBtns(false); q44.coolUntil = Date.now() + coolMs();
    });
  }
  function q44Force() {
    if (q44.inflight || Date.now() < q44.coolUntil || q44.phase !== 'blocked') return;
    q44.inflight = true; setBtns(true);
    // 先問一次現況（登入等級可能剛換過；還沒停的清單也用最新的），再看等級、再確認一次
    q44Send({ op: 'status' }).then(function (r) {
      if (r && r.q44 && !isBusy(r)) q44Show(r);
      if (q44.phase !== 'blocked') return;                                   // 狀態變了（例：別的頁面按了重試）
      var q = q44.last || {}, f = q.force || {};
      if (f.allowed === false) {
        alert('強制關閉要權限等級 ≥ ' + f.needLevel + '（Exit 的等級 LevelSet.AccessLevel[6]），目前是 ' + f.level + '。\n' +
              '請按［收起］用較高的等級登入後再按，或照機台程序處理後按［重試停機］。');
        return;
      }
      var ns = q.notStopped || [];
      if (!window.confirm('確定要強制關閉？\n\n以下還沒停，強制關閉後會維持現狀：\n' +
                          ns.map(function (t) { return '・' + t; }).join('\n') +
                          '\n\n會記在 log：強制關閉，未停：…')) return;
      return q44Send({ op: 'force', confirm: true }).then(function (r2) {
        if (r2 && r2.executed && r2.q44) q44Show(r2);
        else if (r2 && !r2.closedBeforeAck && !isBusy(r2)) {
          alert('沒有強制關閉：' + (r2.guard || '?') + (r2.detail ? '（' + r2.detail + '）' : ''));
          if (r2.q44) q44Show(r2);
        }
      });
    }, function () {}).then(function () {
      q44.inflight = false; setBtns(false); q44.coolUntil = Date.now() + coolMs();
    });
  }
  function q44Hide(h) {
    if (!ui) return;
    q44.hidden = !!h;
    ui.ov.style.display = h ? 'none' : 'flex';
    ui.banner.style.display = h ? 'block' : 'none';
    if (!h && !ended && q44.phase === 'blocked') {                            // 打開時再問一次（等級可能換了）
      q44Send({ op: 'status' }).then(function (r) { if (r && r.q44 && !isBusy(r)) q44Show(r); }, function () {});
    }
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
    if (shutting) {                                                           // AI(W906-PROD-S121) 20260926: 關站中
      if (ui && q44.hidden) q44Hide(false);                                   // AI(W906-FRW-S167) 20260928: 收起時再按 Exit ＝打開蓋層
      return Promise.resolve({ executed: false, guard: 'closing-local' });
    }
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
          q44.took = took;                                                    // AI(W906-FRW-S167) 20260928: 擋下時還權杖用
          showShutdown(r, !!r.closedBeforeAck);
          return r;
        }
        // AI(W906-FRW-S167) 20260928：伺服器已經在關站（例：重新整理之後再按 Exit）⇒ 接回同一個蓋層
        if (r && r.guard === 'closing') {
          st.lastError = '';
          q44.took = took;
          showShutdown({ shutdown: { q44: { phase: '' }, detail: 'Exit 已經確認過，程式正在關站：先停機並讀回確認，停了才結束。' } }, false);
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
        if (shutting) return r;                                               // AI(W906-PROD-S121) 20260926：關站中不還權杖、按鈕保持停用（Q44 擋下時由 q44Release 還）
        var rel = (took && !clientHolds()) ? raw('control.release').then(null, function () {}) : Promise.resolve();
        return rel.then(function () {
          busy = false; setBusy(b, false);
          coolUntil = Date.now() + coolMs();
          return r;
        });
      });
  }

  function start() {
    var b = $('sbCloseProgram');
    if (!b) return;
    b.title = 'sbCloseProgram（golden sbCloseProgramClick main.cpp:29051 → FormClose：存生產資料，先停馬達與加熱器繼電器並讀回確認，停了才照 golden 停機並結束）';   // AI(W906-FRW-S167) 20260928
    b.addEventListener('click', function () { closeProgram(); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045MainClose = { close: closeProgram, state: function () { return st; }, q44: function () { return q44; } };
})();
