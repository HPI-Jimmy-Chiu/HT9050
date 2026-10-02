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
    'Set': {zh:'設置', ja:'セット', ko:'세트'},
    // AI(W906-E027) 20261002 [W906] (St01 ST01-E2)：主畫面溫度條（ht9045_temperfrom_strip.js／Status.TemperFrom.html）；key 一律加 'Temp: ' 前綴，只給溫度條用（terms 是全站共用字典，「OK→正常」不能給別頁的 OK 鈕）
    'Temp: Set temp': {zh:'設定', ja:'設定', ko:'설정'},
    'Temp: Loading machine settings…': {zh:'讀取機台設定中…', ja:'機台設定を読み込み中…', ko:'장비 설정을 읽는 중…'},
    'Temp: OK': {zh:'正常', ja:'正常', ko:'정상'},
    'Temp: Low': {zh:'偏低', ja:'低い', ko:'낮음'},
    'Temp: Over temp': {zh:'過溫', ja:'過温', ko:'과열'},
    'Temp: Over': {zh:'過溫', ja:'過温', ko:'과열'},
    'Temp: Error': {zh:'異常', ja:'異常', ko:'이상'},
    'Temp: Comm error': {zh:'通訊異常', ja:'通信異常', ko:'통신 이상'},
    'Temp: No data': {zh:'沒有資料', ja:'データなし', ko:'데이터 없음'},
    'Temp: Not installed': {zh:'沒裝', ja:'未設置', ko:'미설치'},
    'Temp: Too low': {zh:'讀值過低', ja:'値が低すぎ', ko:'값이 너무 낮음'},
    'Temp: Expand all': {zh:'展開全部', ja:'すべて表示', ko:'전체 보기'},
    'Temp: Show every temperature (this window grows; Collapse or Esc restores it)': {zh:'展開全部溫度（本視窗放大；按「收合」或 Esc 恢復）', ja:'すべての温度を表示（このウィンドウが拡大。「閉じる」か Esc で戻る）', ko:'모든 온도 보기(이 창이 커짐, 접기 또는 Esc로 복귀)'},
    'Temp: Collapse': {zh:'收合', ja:'閉じる', ko:'접기'},
    'Temp: Back to the main-screen temperature strip (Esc)': {zh:'收合回主畫面溫度條（Esc）', ja:'メイン画面の温度バーに戻る（Esc）', ko:'메인 화면 온도 표시줄로 돌아가기(Esc)'},
    'Temp: Temperature overview': {zh:'溫度總覽', ja:'温度一覧', ko:'온도 개요'},
    'Temp: Tolerance': {zh:'允差', ja:'許容差', ko:'허용 오차'},
    'Temp: OK (within setpoint ± tolerance)': {zh:'正常（設定值 ± 允差內）', ja:'正常（設定値 ± 許容差内）', ko:'정상(설정값 ± 허용 오차 이내)'},
    'Temp: Low / heating up': {zh:'偏低／升溫中', ja:'低い／昇温中', ko:'낮음/승온 중'},
    'Temp: not installed or no data': {zh:'沒裝或沒有資料', ja:'未設置またはデータなし', ko:'미설치 또는 데이터 없음'},
    'Temp: Range bar: green = tolerance, line = current PV; bottom line = physical address': {zh:'範圍條：綠段＝允差，線＝目前 PV；格子最下一行＝實體位址', ja:'範囲バー：緑＝許容差、線＝現在の PV。最下行＝物理アドレス', ko:'범위 막대: 녹색=허용 오차, 선=현재 PV, 맨 아래 줄=물리 주소'},
    'Temp: Hot Plate': {zh:'加熱盤', ja:'ホットプレート', ko:'핫플레이트'},
    'Temp: In Shuttle': {zh:'入料穿梭台', ja:'入力シャトル', ko:'입력 셔틀'},
    'Temp: Chamber · Hot Air': {zh:'Chamber · 熱風', ja:'チャンバー · 熱風', ko:'챔버 · 열풍'},
    'Temp: SLK heads': {zh:'SLK 測試頭', ja:'SLK テストヘッド', ko:'SLK 테스트 헤드'},
    'Temp: Plate · SH': {zh:'加熱盤 · 穿梭台', ja:'プレート · シャトル', ko:'플레이트 · 셔틀'},
    'Temp: Head': {zh:'測試頭', ja:'ヘッド', ko:'헤드'},
    'Temp: Index · others': {zh:'Index · 其他', ja:'Index · その他', ko:'Index · 기타'},
    'Temp: Hot air · others': {zh:'熱風 · 其他', ja:'熱風 · その他', ko:'열풍 · 기타'},
    'Temp: 3-station Delta DTM': {zh:'3 站台達 DTM', ja:'3 局 Delta DTM', ko:'3국 Delta DTM'},
    'Temp: heaters': {zh:'組', ja:'組', ko:'조'},
    'Temp: Top row = socket row A, bottom row = row B, columns a →': {zh:'上排＝socket 第 A 排、下排＝第 B 排、欄 a →', ja:'上段＝ソケット A 列、下段＝B 列、列 a →', ko:'위=소켓 A열, 아래=B열, 열 a →'},
    'Temp: Brand unknown': {zh:'廠牌讀不到', ja:'メーカー不明', ko:'브랜드 불명'},
    'Temp: Station not set': {zh:'站號未設定', ja:'局番号未設定', ko:'국번 미설정'},
    'Temp: Cannot read machine settings; temperature cells cannot be laid out': {zh:'讀不到機台設定，溫度格子排不出來（不猜）', ja:'機台設定を読めないため、温度欄を配置できません', ko:'장비 설정을 읽을 수 없어 온도 칸을 배치할 수 없습니다'}
  },

  t: function (key, lang) {
    var e = this.terms[key];
    if (!e || lang === 'en' || !e[lang]) return key;
    return e[lang];
  }
};