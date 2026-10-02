/*
 * ht9045_menu_icons.js — 主畫面 Tools / Config 下拉選單的小圖示
 *
 * AI(W906-MENU-ICON) 20260925:
 *   用途：web/page/main.html 的 Tools（#palSetup .smi）與 Config（#palConfig .smi）
 *   兩個下拉選單，在每個按鈕文字的「左邊」加一個小線條圖示，讓操作員不用讀字
 *   也認得出按鈕。
 *
 *   ‧ 圖示全部是自己手繪的線條圖（24x24 座標、24px 顯示＝1 格 1 像素、筆寬 2px；見下方 MENU15-CRISP），
 *     沒有下載任何圖檔、沒有外部網址、沒有字型、沒有 emoji。
 *   ‧ 一律 stroke="currentColor"、fill="none"、背景透明 —— 顏色完全跟著該項目的
 *     文字顏色走，所以 classic / dark / steel / contrast 四種主題、以及灰色刪除線的
 *     「ruleoff」與斜體的「todo」狀態都會自動跟著變。這個檔**絕不寫死任何顏色**。
 *   ‧ 對應鍵：先讀 data-htitle（release 模式 main.html 會把 title 搬到那裡），
 *     再讀 title；而且只取開頭的識別字（後續程式可能把 title 改成
 *     「sbX（依設定檔未啟用：…）」這種長字串）。
 *   ‧ 找不到圖示的項目維持原樣，不放佔位符。
 *   ‧ 已經有 .smic 子元素的項目會跳過，所以重複呼叫 decorate() 是安全的。
 *   ‧ ⚠ main.html 的 i18n apply() 換語言時必須保留 .smic（由呼叫端負責處理）；
 *     若 apply() 會整個覆寫項目內容，換完語言後再呼叫一次
 *     window.HTMenuIcons.decorate() 即可補回。
 *   ‧ 本檔不含任何 CSS；.smic 的樣式由 main.html 自己定義。
 *
 * AI(W906-MENU15-CRISP) 20261001: EastSun「你這邊不要直接用放大的 解析度太低太醜」
 *   原本 SVG 寫死 width/height 14、筆寬 2.2，main.html 再用 CSS 把它拉成 21px（×1.5）：24 格畫在 21px 上＝每格 0.875px，
 *   筆寬變 1.925px，直線全落在半個像素上 → 每條線兩邊都是灰邊，看起來糊、像放大的小圖。
 *   改成直接畫在最後的大小：SVG 本身 24×24px、viewBox 0 0 24 24（1 格＝1px，不再有任何縮放），筆寬 2px，
 *   橫線／直線／方框的座標都移到整數格（2px 筆畫剛好蓋滿兩個像素，不再有灰邊）。圖形意思不變，只是對齊。
 *   Bar Code 的直條改成實心 rect（fill="currentColor"，仍然不寫死顏色），寬 3／1／2／1／3px、左右對稱。
 */
(function () {
  'use strict';

  var HEAD = '<svg viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="currentColor"' +
             ' stroke-width="2" stroke-linecap="round" stroke-linejoin="round"' +
             ' aria-hidden="true" focusable="false">';
  function S(body) { return HEAD + body + '</svg>'; }

  // 溫度計本體（放在左半邊，右半邊留給各自的記號：± / 波形 / A）
  var THERMO = '<path d="M6 14.5V5a2 2 0 0 1 4 0v9.5a4 4 0 1 1-4 0z"/>';

  // 離開：門框 + 往外的箭頭（Tools 與 Config 共用）
  var EXIT = S('<path d="M10 4H6a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h4"/>' +
               '<path d="M10 12H21"/>' +
               '<path d="M16 7l5 5-5 5"/>');

  var ICONS = {
    /* ---------------- palSetup（Tools） ---------------- */

    // Contact：Z 軸下壓接觸 —— 箭頭往下壓到一條面上
    sbContact: S('<path d="M12 3v12"/>' +
                 '<path d="M7 10l5 5 5-5"/>' +
                 '<path d="M3 20h18"/>'),

    // Bin：分類料盒 —— 有蓋的收納盒
    sbBin: S('<rect x="3" y="4" width="18" height="5" rx="1"/>' +
             '<path d="M5 9v9.5A1.5 1.5 0 0 0 6.5 20h11a1.5 1.5 0 0 0 1.5-1.5V9"/>' +
             '<path d="M10 13h4"/>'),

    // Test IF：測試機介面 —— 插頭 + 線
    sbTester: S('<path d="M9 2.5V7M15 2.5V7"/>' +
                '<path d="M6 7h12v3.5a6 6 0 0 1-12 0z"/>' +
                '<path d="M12 16.5v5"/>'),

    // Temp. offset：溫度計 + 正負號
    sbTempOffset: S(THERMO +
                    '<path d="M18 4v8M14 8h8M14 15h8"/>'),

    // Tray Form：3x3 格盤
    sbTrayForm: S('<rect x="3" y="3" width="18" height="18" rx="2"/>' +
                  '<path d="M9 3v18M15 3v18M3 9h18M3 15h18"/>'),

    // Plate Form：加熱板 —— 平板 + 三道熱氣
    sbPlateForm: S('<path d="M7 13c1.6-1.5-1.6-3 0-4.5s-1.6-3 0-4.5' +
                   'M12 13c1.6-1.5-1.6-3 0-4.5s-1.6-3 0-4.5' +
                   'M17 13c1.6-1.5-1.6-3 0-4.5s-1.6-3 0-4.5"/>' +
                   '<rect x="3" y="16" width="18" height="5" rx="1"/>'),

    // Tray Assign：箭頭指派到格盤
    sbTrayAssign: S('<path d="M3 12h5"/>' +
                    '<path d="M5 9l3 3-3 3"/>' +
                    '<rect x="11" y="4" width="10" height="16" rx="1.5"/>' +
                    '<path d="M16 4v16M11 12h10"/>'),

    // Load/Unload：一上一下兩個箭頭
    sbLdUld: S('<path d="M8 20V4M4 8l4-4 4 4"/>' +
               '<path d="M16 4v16M12 16l4 4 4-4"/>'),

    // Set Up：三條滑桿
    sbSetup: S('<path d="M3 6h9M16 6h5M3 12h3M10 12h11M3 18h11M18 18h3"/>' +
               '<circle cx="14" cy="6" r="2"/>' +
               '<circle cx="8" cy="12" r="2"/>' +
               '<circle cx="16" cy="18" r="2"/>'),

    // Yield：座標軸 + 長條圖
    sbYield: S('<path d="M3 3v18h18"/>' +
               '<path d="M8 17v-4M13 17V9M18 17V5"/>'),

    // ATC：自動溫控 —— 儀表刻度盤 + 指針
    sbATC: S('<path d="M4.2 18.5A9 9 0 1 1 19.8 18.5"/>' +
             '<path d="M12 14l4-4.5"/>' +
             '<circle cx="12" cy="14" r="1.3"/>'),

    // CCD：相機
    sbCCD: S('<path d="M3 9a2 2 0 0 1 2-2h2.5l2-3h5l2 3H19a2 2 0 0 1 2 2v8' +
             'a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>' +
             '<circle cx="12" cy="13" r="3.5"/>'),

    // Sensor Adj.：感測頭 + 往外發的三道波
    sbShuttleSensor: S('<rect x="3" y="8" width="6" height="8" rx="1"/>' +
                       '<path d="M12 9a4 4 0 0 1 0 6"/>' +
                       '<path d="M15.5 6.5a7.5 7.5 0 0 1 0 11"/>' +
                       '<path d="M19 4a11 11 0 0 1 0 16"/>'),

    // Auto Clean：閃亮（清潔完成）
    sbAutoClean: S('<path d="M10 3l1.8 5.2L17 10l-5.2 1.8L10 17l-1.8-5.2L3 10l5.2-1.8z"/>' +
                   '<path d="M19 14v6M16 17h6"/>' +
                   '<path d="M19 3v4M17 5h4"/>'),

    // OCR：掃描框四角 + 文字行
    sbOCR: S('<path d="M3 8V5a2 2 0 0 1 2-2h3M16 3h3a2 2 0 0 1 2 2v3' +
             'M21 16v3a2 2 0 0 1-2 2h-3M8 21H5a2 2 0 0 1-2-2v-3"/>' +
             '<path d="M8 10h8M8 14h5"/>'),

    // Dyna. Temp：溫度計 + 動態波形（與 Temp. offset 的 ± 區分）
    sbDynamicTemp: S(THERMO +
                     '<path d="M13 13h2l2-5 2.5 9 1.5-4h1"/>'),

    // Socket：IC 晶片 + 接腳
    sbSocket_ASE_KR: S('<rect x="6" y="6" width="12" height="12" rx="1.5"/>' +
                       '<path d="M9 2v4M15 2v4M9 18v4M15 18v4' +
                       'M2 9h4M2 15h4M18 9h4M18 15h4"/>'),

    // QA Mode：盾牌 + 勾
    sbQAMode: S('<path d="M12 2 5 5v6c0 4.6 3 8.4 7 10 4-1.6 7-5.4 7-10V5z"/>' +
                '<path d="M8.5 12l2.5 2.5 4.5-5"/>'),

    // Bar Code：粗細交錯的直條（實心 rect、寬 3／1／2／1／3px 對齊整數像素、左右對稱；顏色仍是 currentColor）
    sbBarCode: S('<g fill="currentColor" stroke="none"><rect x="3" y="5" width="3" height="14"/><rect x="8" y="5" width="1" height="14"/>' +
                 '<rect x="11" y="5" width="2" height="14"/><rect x="15" y="5" width="1" height="14"/><rect x="18" y="5" width="3" height="14"/></g>'),

    // Rotate：單一順時針旋轉箭頭
    sbRotate: S('<path d="M20 12a8 8 0 1 1-2.34-5.66L21 9"/>' +
                '<path d="M21 4v5h-5"/>'),

    // Laser：雷射光束（閃電）
    spbLaser: S('<path d="M13 2 4 14h7l-1 8 9-12h-7z"/>'),

    // Auto Retest：兩道循環箭頭 + 中間小播放鍵（與 Rotate 的單箭頭區分）
    spbAutoRetest: S('<path d="M4 12a8 8 0 0 1 13.66-5.66L20 9"/>' +
                     '<path d="M20 4v5h-5"/>' +
                     '<path d="M20 12a8 8 0 0 1-13.66 5.66L4 15"/>' +
                     '<path d="M4 20v-5h5"/>' +
                     '<path d="M10 9v6l5-3z"/>'),

    // Tray Function：料盤 + 定位圖釘（tray mapping）
    spbTrayMapping: S('<path d="M12 12s-3.5-3.2-3.5-6.5a3.5 3.5 0 0 1 7 0c0 3.3-3.5 6.5-3.5 6.5z"/>' +
                      '<rect x="3" y="15" width="18" height="6" rx="1"/>' +
                      '<path d="M9 15v6M15 15v6"/>'),

    // Vacuum Adj.：吸嘴（上方管子＋下方吸盤）＋右上真空錶  // AI(W906-VACUNIT-MENU) 20260930
    sbVacuumUnit: S('<path d="M9 3h4v6H9z"/>' +
                    '<path d="M11 9v3M5 17c0-2.8 2.7-5 6-5s6 2.2 6 5z"/>' +
                    '<path d="M4 20h14"/>' +
                    '<circle cx="18.5" cy="6.5" r="3"/>' +
                    '<path d="M18.5 6.5l1.4-1.4"/>'),

    // 1203 Setting：EtherCAT 卡 —— 三個互連的節點
    sbPci1203Setting: S('<circle cx="12" cy="5" r="3"/>' +
                        '<circle cx="4.5" cy="19" r="3"/>' +
                        '<circle cx="19.5" cy="19" r="3"/>' +
                        '<path d="M10.6 7.65 5.9 16.35M13.4 7.65l4.7 8.7M7.5 19h9"/>'),

    // Exit（Tools）
    sbExitSetup: EXIT,

    /* ---------------- palConfig（Config） ---------------- */

    // C.Select：打開的資料夾（挑選設定檔）
    sbSelete: S('<path d="M3 19V6a2 2 0 0 1 2-2h3.5l2 3H17a2 2 0 0 1 2 2v2"/>' +
                '<path d="M3 19l2.6-7A1.5 1.5 0 0 1 7 11h13.4a1 1 0 0 1 .95 1.3L19 19z"/>'),

    // C.Clear：橡皮擦
    sbClear: S('<path d="M14.5 4l6 6-10 10-6-6z"/>' +
               '<path d="M8.5 10l6 6"/>' +
               '<path d="M11 20h9"/>'),

    // Builder：槌子
    sbBuilder: S('<path d="M8.7 6.8 11.8 3.7l8.5 8.5-3.1 3.1z"/>' +
                 '<path d="M12.9 11.1 4 20"/>'),

    // Start Mode：圓圈裡的播放鍵
    sbStartMode: S('<circle cx="12" cy="12" r="9.5"/>' +
                   '<path d="M10 8v8l6-4z"/>'),

    // Password：鑰匙
    sbPassword: S('<circle cx="7.5" cy="16.5" r="4"/>' +
                  '<path d="M10.3 13.7 20 4M18 6l2.5 2.5M15.5 8.5l2 2"/>'),

    // Configure：齒輪（8 齒，座標為手算）
    sbConfiguration: S('<path d="M10.5 4.96 10.98 2.25 13.02 2.25 13.5 4.96 15.92 5.96 18.17 4.38' +
                       ' 19.62 5.83 18.04 8.08 19.04 10.5 21.75 10.98 21.75 13.02 19.04 13.5' +
                       ' 18.04 15.92 19.62 18.17 18.17 19.62 15.92 18.04 13.5 19.04 13.02 21.75' +
                       ' 10.98 21.75 10.5 19.04 8.08 18.04 5.83 19.62 4.38 18.17 5.96 15.92' +
                       ' 4.96 13.5 2.25 13.02 2.25 10.98 4.96 10.5 5.96 8.08 4.38 5.83' +
                       ' 5.83 4.38 8.08 5.96z"/>' +
                       '<circle cx="12" cy="12" r="3"/>'),

    // Teaching：十字準星
    sbTeaching: S('<circle cx="12" cy="12" r="7.5"/>' +
                  '<path d="M12 2v4M12 18v4M2 12h4M18 12h4"/>' +
                  '<circle cx="12" cy="12" r="1"/>'),

    // Motor Test：伺服馬達側視 —— 本體（散熱鰭）+ 法蘭 + 軸
    sbMotorTest: S('<rect x="3" y="7" width="11" height="10" rx="1.5"/>' +
                   '<rect x="14" y="5" width="3" height="14" rx="1"/>' +
                   '<path d="M17 12h5"/>' +
                   '<path d="M7 10v4M10 10v4"/>'),

    // Home Monitor：房子
    sbHomeMonitor: S('<path d="M3 11l9-8 9 8"/>' +
                     '<path d="M5 9v11h14V9"/>' +
                     '<path d="M10 20v-6h4v6"/>'),

    // Tower Light：三節燈塔 + 立桿 + 底座
    sbTowerLight: S('<rect x="7" y="2" width="10" height="12" rx="1.5"/>' +
                    '<path d="M7 6h10M7 10h10"/>' +
                    '<path d="M12 14v6"/>' +
                    '<path d="M7 21h10"/>'),

    // DIO Setting：撥動開關
    sbDioSet: S('<rect x="2" y="7" width="20" height="10" rx="5"/>' +
                '<circle cx="16.5" cy="12" r="2.2"/>'),

    // Sen. Latch：感測訊號 + 鎖
    sbSensorLatch: S('<circle cx="6" cy="8" r="1.2"/>' +
                     '<path d="M3 5a4.2 4.2 0 0 0 0 6M9 5a4.2 4.2 0 0 1 0 6"/>' +
                     '<rect x="12" y="13" width="8" height="8" rx="1.5"/>' +
                     '<path d="M14 13v-2a2 2 0 0 1 4 0v2"/>'),

    // Omron Temp.：盤面溫控器 —— 外框 + 數字顯示窗 + 三顆按鍵
    sbOmron: S('<rect x="3" y="3" width="18" height="18" rx="2"/>' +
               '<rect x="6" y="6" width="12" height="6" rx="1"/>' +
               '<path d="M7 17h2M11 17h2M15 17h2"/>'),

    // SECS/GEM：主機伺服器（兩層機櫃）
    sbSecsGem: S('<rect x="3" y="3" width="18" height="6" rx="1.5"/>' +
                 '<rect x="3" y="15" width="18" height="6" rx="1.5"/>' +
                 '<path d="M7 6h.01M7 18h.01M11 6h6M11 18h6"/>'),

    // Auto Temp.：溫度計 + A（與其他溫度計區分）
    sbAutoTemp: S(THERMO +
                  '<path d="M13.5 18 17 7l3.5 11M14.8 14h4.4"/>'),

    // Air Con.：雪花（六臂 + 分叉）
    spbAirConditioner: S('<path d="M12 2v20M20.66 7 3.34 17M20.66 17 3.34 7"/>' +
                         '<path d="M14.3 3.87 12 5.8 9.7 3.87M20.19 9.93 17.37 8.9 17.89 5.95' +
                         'M17.89 18.05 17.37 15.1 20.19 14.07M9.7 20.13 12 18.2 14.3 20.13' +
                         'M3.81 14.07 6.63 15.1 6.11 18.05M6.11 5.95 6.63 8.9 3.81 9.93"/>'),

    // PM Alarm：鈴鐺
    sbPMAlarm: S('<path d="M6 16v-5a6 6 0 0 1 12 0v5l2 2H4z"/>' +
                 '<path d="M12 3v2"/>' +
                 '<path d="M10 21a2 2 0 0 0 4 0"/>'),

    // Manual：打開的書
    btnHelp: S('<path d="M2 5h6a4 4 0 0 1 4 4v12a3 3 0 0 0-3-3H2z"/>' +
               '<path d="M22 5h-6a4 4 0 0 0-4 4v12a3 3 0 0 1 3-3h7z"/>'),

    // Exit（Config）—— 與 Tools 的 Exit 相同
    sbExit: EXIT
  };

  // 取項目的對應鍵：data-htitle 優先（release 模式），再 title；只取開頭識別字。
  function keyOf(el) {
    var raw = (el.getAttribute && (el.getAttribute('data-htitle') || el.getAttribute('title'))) || '';
    var m = String(raw).match(/^[A-Za-z_][A-Za-z0-9_]*/);
    return m ? m[0] : '';
  }

  function hasIcon(el) {
    for (var c = el.firstChild; c; c = c.nextSibling) {
      if (c.nodeType === 1 && (' ' + c.className + ' ').indexOf(' smic ') >= 0) return true;
    }
    return false;
  }

  function lookup(el) {
    var k = keyOf(el);
    if (k && Object.prototype.hasOwnProperty.call(ICONS, k)) return ICONS[k];
    // 後備：title 被整個換掉時，改用 id（例如 sbPci1203Setting 本身有 id）
    var id = el.id || '';
    if (id && Object.prototype.hasOwnProperty.call(ICONS, id)) return ICONS[id];
    return null;
  }

  // 替 root（預設 document）底下 Tools / Config 選單的每個項目加圖示。
  // 回傳本次新加的圖示數；已有 .smic 的項目跳過，重複呼叫安全。
  function decorate(root) {
    var scope = root || (typeof document !== 'undefined' ? document : null);
    if (!scope || !scope.querySelectorAll) return 0;
    var list = scope.querySelectorAll('#palSetup .smi, #palConfig .smi');
    var added = 0;
    for (var i = 0; i < list.length; i++) {
      var el = list[i];
      if (hasIcon(el)) continue;
      var svg = lookup(el);
      if (!svg) continue;
      var doc = el.ownerDocument || document;
      var span = doc.createElement('span');
      span.className = 'smic';
      span.innerHTML = svg;
      el.insertBefore(span, el.firstChild);
      added++;
    }
    return added;
  }

  if (typeof window !== 'undefined') {
    window.HTMenuIcons = { decorate: decorate, ICONS: ICONS };
  }

  if (typeof document !== 'undefined') {
    if (document.readyState === 'loading') {
      document.addEventListener('DOMContentLoaded', function () { decorate(); });
    } else {
      decorate();
    }
  }
})();
