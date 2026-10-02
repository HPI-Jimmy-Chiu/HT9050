/* ht9045_unported_form_c.js -- HW.OmronEJ1N / HW.MyCCLinkSensor: golden forms that have no C++ behind them.
 *
 * AI(W906-UNPORTED-FORM) 20261002: the every-component check (compK ioS) found every button, edit and combo on these two pages
 *   without a handler: pressing them did nothing and said nothing (Omron's Save only told "no fields"). The golden forms are not
 *   ported (no TfOmron / TfCCLink in the C++ tree, no EJ1N thread, no CC-Link / CanBus / EtherCAT shuttle-sensor board code), and
 *   on this machine golden cannot open them anyway (USE_16_HEATER=0 -> main sbOmron hidden, main.cpp:23686-23691;
 *   SHUTTLE_SENSOR_TYPE=0 Takex -> main sbShuttleSensor hidden, main.cpp:23608, Contact pnlSensorAdj hidden, cContact.cpp:1313).
 *   So each control is greyed with golden's handler and the reason (title + data-unwired, the marker the Teach / IO pages use).
 *   Nothing is sent. Whether to port the forms is EastSun's decision; when one is ported, drop its page from PAGES.
 */
(function () {
  'use strict';
  var PAGES = {
    'HW.OmronEJ1N.html': {
      file: 'EJ1N/OmronEJ1N.cpp',
      why: 'golden TfOmron（EJ1N/OmronEJ1N.cpp）沒有移植到 C++：沒有 MyOmronThread（EJ1N RS-232 輪詢，AT／RT／SV／Run-Stop／溫控型式命令），' +
           '網頁沒有東西可以送。這台 Gerneral.ini USE_16_HEATER=0（不是 EJ1N），golden 主畫面的 Omron 鈕照 main.cpp:23686-23691 隱藏。要不要移植等 EastSun 決定',
      handlers: {
        "btGetModuler": "OnClick=btGetModulerClick :946",
        "btRunStop": "OnClick=btRunStopClick :1604",
        "btSetThermoType": "OnClick=btSetThermoTypeClick :1249",
        "Button1": "OnClick=Button1Click :1913",
        "cbMrcSrc": "OnChange=cbMrcSrcChange :453",
        "ComboBox5": "OnChange=cbMrcSrcChange :453",
        "ComboBox3": "OnChange=cbMrcSrcChange :453",
        "ComboBox6": "OnChange=cbMrcSrcChange :453",
        "ComboBox7": "OnChange=cbMrcSrcChange :453",
        "ComboBox8": "OnChange=cbMrcSrcChange :453",
        "ComboBox9": "OnChange=cbMrcSrcChange :453",
        "ComboBox1": "OnChange=cbMrcSrcChange :453",
        "ComboBox2": "OnChange=cbMrcSrcChange :453",
        "ComboBox10": "OnChange=cbMrcSrcChange :453",
        "ComboBox11": "OnChange=cbMrcSrcChange :453",
        "ComboBox12": "OnChange=cbMrcSrcChange :453",
        "ComboBox13": "OnChange=cbMrcSrcChange :453",
        "ComboBox14": "OnChange=cbMrcSrcChange :453",
        "ComboBox15": "OnChange=cbMrcSrcChange :453",
        "Edit1": "OnClick=Edit1Click :2152",
        "btSend": "OnClick=btSendClick :844",
        "btSetSV": "OnClick=btSetSVClick :1326",
        "btResetCOM": "OnClick=btResetCOMClick :1855",
        "Button3": "OnClick=Button1Click :1913",
        "btAT": "OnClick=btATClick :1417",
        "btATOff": "OnClick=btATOffClick :1983",
        "btAT1by1": "OnClick=btAT1by1Click :1990",
        "btRT": "OnClick=btRTClick :1997",
        "btRT1by1": "OnClick=btRT1by1Click :2004",
        "btRTOff": "OnClick=btRTOffClick :2011",
        "btSaveData": "OnClick=btSaveDataClick :2028"
      }
    },
    'HW.MyCCLinkSensor.html': {
      file: 'CCLink/MyCCLinkSensor.cpp',
      why: 'golden TfCCLink（CCLink/MyCCLinkSensor.cpp）表單沒有移植到 C++（只有判斷式 CCLink/MyCCLinkSensor_predicates.cpp）：梭車感測器板' +
           '（CC-Link／CanBus／EtherCAT）的讀值、設定、存檔與 btShuttlePositionMove 都沒有路徑。這台 SHUTTLE_SENSOR_TYPE=0（Takex），' +
           'golden 主畫面 sbShuttleSensor（main.cpp:23608）與 Contact 的 pnlSensorAdj（cContact.cpp:1313）都隱藏 —— golden 在這台打不開這一頁。要不要移植等 EastSun 決定',
      handlers: {
        "edIn1_9": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn1_9": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_1": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_2": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_3": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_4": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_5": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_8": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_7": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn1_6": "OnMouseDown=btIn2_8MouseDown :1465",
        "edIn1_8": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_7": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_6": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_5": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_4": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_3": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_2": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn1_1": "OnMouseDown=edIn2_1MouseDown :1459",
        "btSetIn1": "OnClick=btSetIn1Click :1770",
        "btSave": "OnClick=btSaveClick :2305",
        "btRead": "OnClick=btReadClick :2311",
        "btIn2_9": "OnMouseDown=btIn2_8MouseDown :1465",
        "edIn2_9": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn2_1": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_2": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_3": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_4": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_5": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_8": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_7": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn2_6": "OnMouseDown=btIn2_8MouseDown :1465",
        "edIn2_8": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_7": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_6": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_5": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_4": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_3": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_2": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn2_1": "OnMouseDown=edIn2_1MouseDown :1459",
        "btSetIn2": "OnClick=btSetIn2Click :1707",
        "btShuttlePositionMove": "OnClick=btShuttlePositionMoveClick :2168",
        "edOut2": "OnMouseDown=edIn2_1MouseDown :1459",
        "btOut2": "OnMouseDown=btIn2_8MouseDown :1465",
        "edOut1": "OnMouseDown=edIn2_1MouseDown :1459",
        "btOut1": "OnMouseDown=btIn2_8MouseDown :1465",
        "btIn3_1": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_2": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_3": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_4": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_5": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_8": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_7": "OnMouseDown=btIn3_1MouseDown :2082",
        "btIn3_6": "OnMouseDown=btIn3_1MouseDown :2082",
        "edIn3_8": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_7": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_6": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_5": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_4": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_3": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_2": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn3_1": "OnMouseDown=edIn2_1MouseDown :1459",
        "SetSocket": "OnClick=SetSocketClick :1940",
        "SetRotate": "OnClick=SetRotateClick :2007",
        "edIn3_9": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_9": "OnMouseDown=btIn3_9MouseDown :2125",
        "edIn3_10": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_10": "OnMouseDown=btIn3_9MouseDown :2125",
        "edIn3_11": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_11": "OnMouseDown=btIn3_9MouseDown :2125",
        "edIn3_12": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_12": "OnMouseDown=btIn3_9MouseDown :2125",
        "edIn3_13": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_13": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_14": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_14": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_15": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_15": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_16": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_16": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_17": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_17": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_18": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_18": "OnMouseDown=btIn3_13MouseDown :2187",
        "edIn3_20": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_20": "OnMouseDown=btIn3_13MouseDown :2187",
        "edtIn3_19": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn3_19": "OnMouseDown=btIn3_13MouseDown :2187",
        "SetColor": "OnClick=SetColorClick :2230",
        "btIn4_1": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_2": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_3": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_4": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_5": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_8": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_7": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn4_6": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_8": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_7": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_6": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_5": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_4": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_3": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_2": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_1": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn4_9": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_9": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_10": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_10": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_11": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_11": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_12": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_12": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_13": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_13": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_14": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_14": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_16": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_16": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn4_15": "OnMouseDown=edIn2_1MouseDown :1459",
        "btIn4_15": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_1": "OnMouseDown=btIn5_1MouseDown :2907",
        "btIn5_2": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_3": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_4": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_5": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_8": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_7": "OnMouseDown=btIn4_1MouseDown :2792",
        "btIn5_6": "OnMouseDown=btIn4_1MouseDown :2792",
        "edIn5_8": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_7": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_6": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_5": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_4": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_3": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_2": "OnMouseDown=edIn2_1MouseDown :1459",
        "edIn5_1": "OnMouseDown=edIn2_1MouseDown :1459",
        "spbSave": "OnClick=spbSaveClick :1689",
        "edPercentage": "OnMouseDown=edPercentageMouseDown :1695",
        "edDefaultValue": "OnMouseDown=edDefaultValueMouseDown :1701",
        "btnCanBusSeach": "OnClick=btnCanBusSeachClick :2337",
        "btnReadCanBusSetting": "OnClick=btnReadCanBusSettingClick :2457",
        "edtInShtLtcPercent": "OnMouseDown=edPercentageMouseDown :1695",
        "sbExit": "OnClick=sbExitClick :491",
        "sbReset": "OnClick=sbResetClick :1454"
      }
    }
  };
  var name = (location.pathname.split('/').pop() || '');
  var P = PAGES[name];
  if (!P) return;
  function mark() {
    var list = document.querySelectorAll('button, input, select, textarea, .btnpanel');
    Array.prototype.forEach.call(list, function (el) {
      if (el.classList.contains('exitbtn') || el.closest('.pcTabs')) return;      // closing the window stays (golden Close / the X)
      var h = el.id && P.handlers[el.id];
      var why = (h ? 'golden ' + h.replace(/ :(\d+)/g, '（' + P.file + ':$1）') + '：' : '') + P.why;
      if ('disabled' in el) el.disabled = true;
      el.setAttribute('aria-disabled', 'true');
      el.setAttribute('data-unwired', why);
      el.style.cursor = 'not-allowed';
      var a = el.hasAttribute('data-htitle') ? 'data-htitle' : 'title';
      var t = el.getAttribute(a) || '';
      if (t.indexOf('（網頁停用）') < 0) el.setAttribute(a, (t ? t + '\n' : '') + '（網頁停用）' + why);
    });
  }
  window.HT9045UnportedForm = { page: name, list: P, mark: mark };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', mark); else mark();
})();
