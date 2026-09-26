/* HT9xxx Manual 多語系字典檔（en / zh 繁中 / ja 日 / ko 韓）
   - toolbar：Main.html 工具列按鈕，key = 元件 id
   - terms  ：專業名詞對照，key = 英文原文（手動維護；查無詞條時保留英文原文）
   - 本檔由 IDE.I18nEditor.html 產生 */
window.HTI18N = {
  langs: ['en', 'zh', 'ja', 'ko'],

  toolbar: {
    sbCloseProgram: {en:'Exit', zh:'結束', ja:'終了', ko:'종료'},
    sbSetting: {en:'Tools ▾', zh:'工具 ▾', ja:'ツール ▾', ko:'도구 ▾'},
    sbConfig: {en:'Config ▾', zh:'設定 ▾', ja:'設定 ▾', ko:'설정 ▾'},
    sbOffset: {en:'Offset', zh:'補償', ja:'Offset', ko:'Offset'},
    sbSpeed: {en:'Speed', zh:'速度', ja:'Speed', ko:'Speed'},
    sbIO: {en:'IO', zh:'IO', ja:'IO', ko:'IO'},
    sbMessage: {en:'Message', zh:'訊息', ja:'メッセージ', ko:'메시지'},
    sbDebug: {en:'Debug ▾', zh:'除錯模式 ▾', ja:'デバッグ ▾', ko:'디버그 ▾'},
    // AI(W906-IOWEB-P29) 20260925: Tools ▾ 子選單新增的「1203 Setting」（main.html #sbPci1203Setting → pci1203.html）
    sbPci1203Setting: {en:'1203 Setting', zh:'1203 設定', ja:'1203 設定', ko:'1203 설정'}
  },

  terms: {
    'Vacuum': {zh:'真空', ja:'真空', ko:'진공'},
    'Loader': {zh:'入料區', ja:'ローダー', ko:'로더'},
    'Unloader': {zh:'出料區', ja:'アンローダー', ko:'언로더'},
    'Shuttle': {zh:'穿梭台', ja:'シャトル', ko:'셔틀'},
    'Socket Sensor': {zh:'Socket 感測器', ja:'ソケットセンサー', ko:'소켓 센서'},
    'Temperature': {zh:'溫度', ja:'温度', ko:'온도'},
    'Start Mode': {zh:'啟動模式', ja:'起動モード', ko:'시작 모드'},
    'Set': {zh:'設置', ja:'セット', ko:'세트'}
  },

  t: function (key, lang) {
    var e = this.terms[key];
    if (!e || lang === 'en' || !e[lang]) return key;
    return e[lang];
  }
};