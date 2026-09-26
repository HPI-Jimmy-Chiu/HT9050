/* ht9045_nonstop_page.js -- Alert.Note.NonStop.html / Alert.MyMessageBox.NonStop.html 的頁內程式
 * ---------------------------------------------------------------------------
 * //Steven 20260922
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260922_Steven.md
 * ---------------------------------------------------------------------------
 * 這是 dialog-page.js 的不停機版本，協定完全一樣，但刻意**不共用**那一支：
 * dialog-page.js 有九顆動作鍵的互斥選擇邏輯（KeyComp[] ↔ BtnPanel）、
 * ShowErrSite、TMyTray1、dbFlush 面板閃爍 —— 那些在不停機的情境下一個都不該有。
 * 把兩套塞進同一支，等於讓「會停機」與「不會停機」共用一條分支很多的程式，
 * 改壞其中一邊的風險遠大於多一支 80 行的檔。
 *
 * ---------------------------------------------------------------------------
 * NonStop 的定義（使用者 20260922 裁定）
 * ---------------------------------------------------------------------------
 * **C++ 那邊不呼叫 StopAllMotor()。** 就這一件事。
 *
 *   會停  note.cpp:805-808  ShowErrorMessage() 內 if(Code!="WAR1635") StopAllMotor();
 *                           else StopAllMotor(false) -- 兩條路都停機
 *   不停  mymessbox.cpp:303 if(!iUnLoaderCount){ ... StopAllMotor(); }
 *                           -> iUnLoaderCount != 0 就跳過
 *
 * ⚠ 這一頁**不能讓機台不停**。StopAllMotor() 在 request 送到瀏覽器之前就跑完了。
 *   這一頁只負責「照實畫」：C++ 說沒停（nonStop:true）才畫成沒停。
 *   唯一真的能避免停機的，是 web 端在**根本沒呼叫 C++** 的情況下自己擋下來
 *   —— 權限不足就是這一條，見 ht9045_nonstop_alarm.js 的 raise()。
 *
 * ---------------------------------------------------------------------------
 * 協定（與 dialog-page.js 相同的三步）
 * ---------------------------------------------------------------------------
 *   ready  -> postMessage {type:'HT_DIALOG_READY',  kind}
 *   收     <- {type:'HT_DIALOG_REQUEST', kind, request}
 *   確認   -> postMessage {type:'HT_DIALOG_ACTION', kind, requestId,
 *                          action:{name:'ACKNOWLEDGE', code:0}}
 *
 * kind 取自 <body data-dialog="...">：'alarmNonStop' | 'messageNonStop'
 */
(function (global) {
  'use strict';

  var kind = document.body.getAttribute('data-dialog');   // alarmNonStop | messageNonStop
  var isAlarm = (kind === 'alarmNonStop');
  var isTop = (global.parent === global);
  var current = null;

  function $(id) { return document.getElementById(id); }
  function setText(id, s) { var el = $(id); if (el) el.textContent = (s === null || s === undefined) ? '' : String(s); }
  function show(id, on) { var el = $(id); if (el) el.style.display = on ? '' : 'none'; }
  function post(msg) { if (!isTop) global.parent.postMessage(msg, '*'); }

  /* golden 的 request 形狀：arguments（code/kCode/errorPart/...）＋ display（message/
   * unitName/description）。NonStop 另外可能帶 nonStopInfo（本地發起時由
   * ht9045_nonstop_alarm.js 填，或由 AlarmNonStop.json 查表補上）。 */
  function renderAlarm(req) {
    var a = req.arguments || {}, d = req.display || {}, n = req.nonStopInfo || {};
    var code = String(a.code || '');
    var msg = d.message || '';
    if (a.errorPart && a.errorPart !== ' ') msg += ' : ' + a.errorPart;

    setText('nsTitle', n.titleZh || n.title || '權限異常');
    setText('nsMsg', n.title || msg || code);
    setText('nsMsgZh', n.titleZh && n.title ? n.titleZh : (n.title ? msg : ''));
    setText('nsCode', code || '—');
    setText('nsUnit', d.unitName || '—');
    setText('nsDesc', d.description || '');
    show('nsDescRow', !!(d.description && String(d.description).trim()));
    /* 這一行是整頁存在的理由：讓操作員一眼確定機台沒有停。
     * request 沒有明說時不要硬講 —— 寫成保留字樣，不編。 */
    setText('nsRun', req.nonStop === false ? '⚠ 機台狀態未知' : '機台未停機，仍在運轉');
    setText('nsHint', n.hint || '此告警不影響生產，確認後即可繼續。');

    /* golden 的 KCode 在這裡一律當 0 —— 不停機的告警不該有動作鍵。
     * 若 request 竟然帶了非 0 的 kCode，那代表路由判斷錯了（這一則其實會停機），
     * 不要默默照畫，寫到 console 讓它被發現。 */
    if (Number(a.kCode) > 0) {
      console.error('[NonStop] request 帶了 kCode=' + a.kCode + '（code=' + code + '），'
                  + '但不停機的告警不應有動作鍵。請檢查 AlarmNonStop.json 的路由是否誤判。');
    }
  }

  function renderMessage(req) {
    var d = req.display || {}, a = req.arguments || {}, n = req.nonStopInfo || {};
    /* golden ShowUnloaderTrayMessage(S1, S2)：S1 英文、S2 中文，兩行分開顯示 */
    /* AI(W906-SMM) 20260925：C++ 的 ShowUnloaderTrayMessage 走 Message-dialog-request 契約的欄位名
     * （display.primaryText／secondaryText、arguments.s1／s2，tools/wb_serve.cpp 檔尾 MbPost）；
     * 原本只認本地 raise 的 mainMessage／chineseMessage，C++ 來的那一則會畫成空白。兩種都收。 */
    var s1 = d.mainMessage || d.message || d.primaryText || a.message || a.s1 || '';
    var s2 = d.chineseMessage || d.secondaryText || a.chineseMessage || a.s2 || '';
    setText('nsTitle', n.title || '提示');
    setText('nsS1', s1);
    setText('nsS2', s2);
    setText('nsRun', req.nonStop === false ? '⚠ 機台狀態未知' : '機台未停機，仍在運轉');
    setText('nsHint', n.hint || '處理完畢後按確認。');
  }

  function render(req) {
    current = req;
    if (isAlarm) renderAlarm(req); else renderMessage(req);
  }

  function acknowledge() {
    if (!current) return;
    post({
      type: 'HT_DIALOG_ACTION', kind: kind,
      requestId: current.requestId,
      action: { name: 'ACKNOWLEDGE', code: 0 },
      pressedButton: 'OK'
    });
  }

  function onMessage(e) {
    var m = e && e.data;
    if (!m || m.kind !== kind) return;
    if (m.type === 'HT_DIALOG_REQUEST') render(m.request || {});
  }

  function boot() {
    var ok = $('nsOk');
    if (ok) ok.addEventListener('click', acknowledge);
    /* Enter / Space 也能確認 —— 這一頁只有一顆鍵，不需要學 fNote 的互斥選擇。
     * Esc 刻意**不**綁：告警要被看見，誤觸 Esc 關掉等於沒顯示過。 */
    document.addEventListener('keydown', function (ev) {
      if (ev.key === 'Enter' || ev.key === ' ') { ev.preventDefault(); acknowledge(); }
    });
    global.addEventListener('message', onMessage);
    post({ type: 'HT_DIALOG_READY', kind: kind });
    if (ok) ok.focus();
  }

  /* 對外，給本地發起（raise）與 Console 手動測試用 */
  global.HT9045NonStopPage = { render: render, acknowledge: acknowledge,
                               kind: function () { return kind; } };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot);
  else boot();
})(window);
