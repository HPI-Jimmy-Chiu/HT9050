/* ht9045_testermode.js -- main.html 的 On-Line / Off-Line 顯示（golden TfMain::LoadTestModePicture）
 * ---------------------------------------------------------------------------
 * AI(W906-TESTERMODE) 20261007 (Jerry, J-18) 新檔（手寫）。
 * 起因：主畫面完全看不出 Tester 是 On-Line 還是 Off-Line —— imgTester 在 main.html:220 寫死
 *   style="color:green"，而 golden 的另一半（labTesterMode）移植樹根本沒有。1007 實測，使用者只能靠
 *   curl tag 才知道自己是不是連線狀態。
 *
 * golden 906 main.cpp:12259-12330 LoadTestModePicture()（非 I40 分支）：
 *     LastSet.iTester==ON_LINE  -> imgTester=ONLINE_ENABLE.bmp   labTesterMode "On-Line"  Visible=false
 *     LastSet.iTester==_2D_SORT -> imgTester=2D_SORT.bmp         labTesterMode "2D_SORT"  Visible=false
 *     其餘（OFF_LINE）          -> imgTester=OFFLINE_ENABLE.bmp  labTesterMode "Off-Line" Visible=TRUE
 *   I40 分支（IniConfig.bI40_bStartProductOnLine || CUSTOMER_CODE==CC_ASE_KaohSiung）只改 Off-Line 的樣子：
 *     labTesterMode "** Warning:Off-Line **"、Font->Size=20、Off_lineDisplay->BorderWidth 2 -> 10。
 *   ⇒ 注意 golden 的設計是「正常不吵，離線才跳出來」：On-Line 與 2D_SORT 時那個標籤是**隱藏**的。
 *
 * 資料：
 *   tag lastset.tester          (WebBridgeTags.cpp:839 = LastSet.iTester；0 OFF_LINE / 1 ON_LINE / 2 _2D_SORT，cmydef.h:85-87)
 *   tag lastset.testerWarnStyle (AI(W906-TESTERMODE) 20261007 = golden 上面那個分支條件，C++ 算好送)
 *   兩個都是 null ⇒ 設定還沒載入或還沒連上 ⇒ 顯示「不可知」，不猜一個狀態。
 *
 * [W906] 與 N07 橫幅共用 labTesterMode —— 這是照 golden，不是巧合：
 *   golden main.cpp:20978-20980 N07（SECS 斷線）時把同一個 labTesterMode 借去寫 "SECS DISCONNECTED"，
 *   main.cpp:20987-20993 解除時再呼叫 LoadTestModePicture() 還原。移植樹的 N07 那一半是
 *   ht9045_n07_banner.js（St02-E，AI(W906-C15-N07-UI)），它會自己建一個 #labTesterMode 再移除。
 *   所以這裡：N07 在作用中就**完全不碰**（它優先，同 golden），並且訂閱 n07.alarm，
 *   在它解除後重畫 —— 那就是 golden 那句 LoadTestModePicture()。不改 St02 的檔。
 *
 * [W906] imgTester 是 emoji 不是 bmp：golden 換的是三張圖，這裡換顏色＋title。顏色沿用這一頁既有的
 *   語意（green＝正常），Off-Line 用紅、2D_SORT 用藍以對應三張不同的圖；形狀不換，避免自創語意。
 */
(function () {
  'use strict';

  var OFF_LINE = 0, ON_LINE = 1, _2D_SORT = 2;          // cmydef.h:85-87
  var GREEN = 'green', RED = '#C00000', BLUE = '#1565C0', GRAY = '#888';

  // golden 的三張 bmp 對應到三個字元（理由見 setIcon）。刻意全部挑**文字**字元（不是 Emoji_Presentation），
  // 這樣 CSS color 也吃得到，形狀與顏色兩個信號都成立：
  //   ⊙ U+2299  連著（ONLINE_ENABLE）   ⊘ U+2298  斷線（OFFLINE_ENABLE）
  //   ⊞ U+229E  2D_SORT                 ⊜ U+229C  不可知
  var G_ON = '⊙', G_OFF = '⊘', G_2D = '⊞', G_UNKNOWN = '⊜';

  var st = { label: null };

  function tag(n) { return (window.HT9045Tags && HT9045Tags.has(n)) ? HT9045Tags.get(n) : null; }

  // N07 橫幅在作用中嗎？它借走 labTesterMode（golden 也是這個順序）。
  function n07Active() {
    try {
      return !!(window.HT9045N07Banner && window.HT9045N07Banner.state && window.HT9045N07Banner.state().on);
    } catch (e) { return false; }
  }

  function pane() { return document.querySelector('div.statusPane'); }

  // 只在需要顯示時才建，隱藏時移除 —— 對應 golden 的 Visible=false（VCL 的隱藏控制項不佔版面流）。
  function setLabel(text, warn) {
    var p = pane();
    if (!p) return;
    if (text === null) {                                   // 隱藏
      if (st.label && st.label.parentNode === p) p.removeChild(st.label);
      st.label = null;
      return;
    }
    if (!st.label || st.label.parentNode !== p) {
      var l = document.createElement('div');
      l.id = 'labTesterMode';
      st.label = l;
      p.appendChild(l);
    }
    st.label.textContent = text;
    st.label.title = 'labTesterMode（golden LoadTestModePicture main.cpp:12259-12330）';
    st.label.style.color = RED;                            // golden 這個標籤只在 Off-Line 才現身
    st.label.style.fontWeight = 'bold';
    st.label.style.fontSize = warn ? '20px' : '';          // golden I40 分支 Font->Size=20
  }

  // [W906] ⚠ 換的是**字元**，不是只換顏色。imgTester 的字是 🔗（U+1F517），那是 Emoji_Presentation 字元，
  //   由字型自己上色，**CSS color 對它無效** —— 1007 實測：同一頁的 imgRunMode 用 ▣（U+25A3，文字字元）
  //   設 color:green 就會變綠，imgTester 設了卻完全沒反應。所以只換顏色等於沒有信號。
  //   golden 本來也不是換顏色，是換三張不同的 bmp（ONLINE_ENABLE / OFFLINE_ENABLE / 2D_SORT），
  //   換字元才是忠實的對應。顏色照樣設，對文字字元有效、對 emoji 無害。
  function setIcon(glyph, color, title) {
    var i = document.getElementById('imgTester');
    if (!i) return;
    i.textContent = glyph;
    i.style.color = color;
    i.title = title;
  }

  // N07 作用中時只讓出**標籤**，圖示照常更新 —— golden 的 N07 區塊（main.cpp:20978-20980）動的是
  // labTesterMode 與 Off_lineDisplay，沒有碰 imgTester，而 LoadTestModePicture 仍然會換那張 bmp。
  // ⚠ 讓出時要先把**我們自己建的**那個標籤移除：N07 的 show() 會另外 createElement 一個 #labTesterMode，
  //   兩邊都留著就會出現兩個同 id 的元素（畫面重複，而且 N07 解除時只收走它自己那個，我們的會留下過期文字）。
  //   setLabel(null) 只動 st.label，不會碰到 N07 的那一個。
  function render() {
    var n07 = n07Active();

    var v = tag('lastset.tester');
    var warn = tag('lastset.testerWarnStyle') === true;

    var glyph, color, title, label = null;                 // label null ＝ golden 的 Visible=false
    if (v === ON_LINE) {                                   // golden: ONLINE_ENABLE.bmp，標籤隱藏
      glyph = G_ON;  color = GREEN;
      title = 'imgTester：On-Line（lastset.tester=1；golden ONLINE_ENABLE.bmp）';
    } else if (v === _2D_SORT) {                           // golden: 2D_SORT.bmp，標籤隱藏
      glyph = G_2D;  color = BLUE;
      title = 'imgTester：2D_SORT（lastset.tester=2；golden 2D_SORT.bmp）';
    } else if (v === OFF_LINE) {                           // golden: OFFLINE_ENABLE.bmp，標籤**顯示**
      glyph = G_OFF; color = RED;
      title = 'imgTester：Off-Line（lastset.tester=0；golden OFFLINE_ENABLE.bmp）';
      label = warn ? '** Warning:Off-Line **' : 'Off-Line';
    } else if (v === null || v === undefined) {            // 不可知：不猜 On 也不猜 Off
      glyph = G_UNKNOWN; color = GRAY;
      title = 'imgTester：tag lastset.tester 是 null（設定還沒載入或還沒連上）→ 不可知';
    } else {                                               // 認不得的值：說實話，不靜默當成 Off-Line
      glyph = G_UNKNOWN; color = GRAY;
      title = 'imgTester：lastset.tester=' + JSON.stringify(v) + ' 不是 0/1/2（cmydef.h:85-87）→ 不可知';
    }

    setIcon(glyph, color, title);                          // 圖示：N07 作用中也照常更新（golden 沒碰 imgTester）
    setLabel(n07 ? null : label, warn);                    // 標籤：N07 作用中讓給它
  }

  // [W906] 重畫一律延到下一輪：同一個訊框的訂閱者先後順序沒有保證，而 n07.alarm 變 false 時
  //   ht9045_n07_banner.js 的 hide() 必須先跑完（它要移除自己那個 labTesterMode、還原框線），
  //   我們才能判斷 n07Active() 並接手。直接在訂閱裡同步 render() 的話，先後顛倒就會看到
  //   n07Active() 還是 true 而跳過，標籤再也回不來。
  var pending = null;
  function renderSoon() {
    if (pending !== null) return;
    pending = setTimeout(function () { pending = null; render(); }, 0);
  }

  function start() {
    render();
    if (window.HT9045Tags && typeof HT9045Tags.subscribe === 'function') {
      HT9045Tags.subscribe(function (changed) {
        if (!changed) return;
        var h = Object.prototype.hasOwnProperty;
        // n07.alarm 也要聽：N07 解除後要把標籤還原，就是 golden 那句 LoadTestModePicture()
        if (h.call(changed, 'lastset.tester') || h.call(changed, 'lastset.testerWarnStyle') ||
            h.call(changed, 'n07.alarm')) renderSoon();
      });
    }
    if (window.HT9045Tags && typeof HT9045Tags.connect === 'function') HT9045Tags.connect().catch(function () {});
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();

  window.HT9045TesterMode = { render: render, state: function () { return st; } };
})();
