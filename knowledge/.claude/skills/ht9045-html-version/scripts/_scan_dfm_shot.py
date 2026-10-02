# -*- coding: utf-8 -*-
"""產生 page/screenshot_meta.js：
(1) ALL_DFM 全部 dfm 表單的 shot/html/i18n 狀態
(2) DYNAMIC_CLASSES 使用執行期動態建立 UI 的 class 與是否已納入 VCL 元件頁

⚠ 本腳本**只**負責上面兩個區塊。screenshot_meta.js 裡另外還有五個區塊
  （FILE_IO_STATUS / SIM0_DECL_STATUS / PAGE_WIRE_STATUS / TAG_WIRE_STATUS /
  FORM_SHOW_STATUS），是別的產生器附加上去的，寫檔時必須原樣保留 —— 見 patch_meta()。"""
import os, re, glob, io, json
BASE = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
SHOT = r'D:\HT9045\IMG\ScreenShot'
# 比照 gen_tag_status.py：page\ 是原始樹、web\page\ 是部署樹，兩份都要寫。
# 只寫其中一份的話，瀏覽器實際載入的是另一份，表①② 會看起來「沒有更新」。
METAS = [r'D:\HT9045\page\screenshot_meta.js',
         r'D:\HT9045\web\page\screenshot_meta.js']

# ⚠ BASE 固定是 V910，但 Setup.AGV.html 是用 **V912** 的 Automation\AGV.dfm 轉的：
#   V910 那份的 E84 燈號還停在 Label33 這種 IDE 自動命名，元件名與頁面對不上。
#   這裡刻意不改 BASE —— 表① 其餘 130 多個表單的權威來源仍是 V910，
#   為了一個表單換掉 BASE，會讓另外 130 多筆的 form/cls/i18n 一起漂移。
#   AGV 這一筆在表① 的 shot/html 旗標不受影響（只看 dfm 檔名與 converted_dfm）。

shot_forms = set()
for fn in os.listdir(SHOT):
    shot_forms.add(os.path.splitext(fn)[0].split('.')[0])

# 已 dfm→html 轉換（JOBS）＋手寫頁的 dfm base
converted_dfm = {
    'cOffSet', 'cSpeed', 'iosetview', 'cConfiguration', 'cCounterSel', 'cCounterClear', 'cBuilder',
    'DIOInterFaceCFG', 'LtcSensor', 'cTowerLight', 'OmronEJ1N', 'QAMode', 'BarCode', 'MyCCLinkSensor',
    'uCleaning', 'cContact', 'cTesterIF', 'GroundMan', 'cLd_ULd', 'cSecurity', 'cTrayForm', 'SCK_ART',
    'uYieldMonitoring', 'cHotPlate', 'cObserver', 'cSetUp', 'SmartDiagnostic', 'cStartCondition',
    'uTemp_Set', 'cBinSel', 'uteach', 'uMotorTest', 'uhome',
    'ShuttleMove',      # Steven 20260918: HW.ShuttleMove.html
    # Steven 20260919：今日三筆 dfm→HTML。AGV 的來源樹是 V912（見上方 BASE 註解）。
    'ContactForce',     # Setup.ContactForce.html
    'VacuumUnit',       # HW.VacuumUnit.html
    'AGV',              # Setup.AGV.html
    'main', 'cSortCT', 'cContactCT', 'uLotInfo', 'cTemperFrom', 'cTestCategory', 'cShowBinSelect', 'uShowMessage',
    'HandlerSys', 'mymessbox', 'note', 'Password', 'fMain', 'cTrayAssignment',
}

# 非顯示用資料／通訊模組，HTML 模擬不需建立對應表單。
html_not_required = {
    'rs232': 'TCOM2 通訊資料模組，不會顯示視窗',
    'database': 'TDataModule1 資料庫資料模組，不會顯示視窗',
    'adam6024': '僅承載未啟用的 TClientSocket，不會顯示操作視窗',
    'BarcodeXML': 'HTTP/XML 通訊服務容器，不會顯示操作視窗',
    'FileTransfer': '空白檔案傳輸服務表單，沒有視覺控制項',
    'HT9045': '空白主程式殼層，沒有視覺控制項',
    'language': '空白語系服務表單，沒有視覺控制項',
    'MyBinDisp': 'TDataModule3 Bin Display 通訊資料模組，不會顯示視窗',
    'MyTempture': 'TDataModule2 溫控通訊資料模組，不會顯示視窗',
    'SCK_WebService': 'HTTPRIO Web Service 通訊容器，不會顯示操作視窗',
    'systools': '空白系統工具服務表單，沒有視覺控制項',
    'TrayStepMotor': 'TDataModule Tray Step Motor 通訊資料模組，不會顯示視窗',
    'ATCInterface': '舊版 ATC 通訊格式，已不再使用',
    'ATC_Handler_Side': 'ATC Handler Side 不需 HTML 操作介面',
    'CCDInterface': 'CCD Interface 已不再支援',
    'ArmOffsetData': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'cAutoAlignment': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'SmartSetup': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'AutomationSimulator': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'mainAT': 'Automation Simulator 子專案畫面，不屬於 HT9045 主機台 UI',
    'BinDisp': 'BinDispTester 子專案畫面，不屬於 HT9045 主機台 UI',
    'CCINPUT': 'CCLink 子專案畫面，不屬於 HT9045 主機台 UI',
    'cDataHandling': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'cheksocket': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'Cassette': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'RFID': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'fMain': 'PMAlarm 子專案畫面，不屬於 HT9045 主機台 UI',
    'MemoryAlarm': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
    'TriMachineDeforst': '未編入 HT9045.bpr，亦無子專案 .bpr 引用',
}

all_dfm = []
for dfm in glob.glob(os.path.join(BASE, '**', '*.dfm'), recursive=True):
    try:
        with open(dfm, encoding='cp950', errors='replace') as f:
            first = f.readline().strip()
    except Exception:
        continue
    m = re.match(r'object (\w+): (\w+)', first)
    if not m:
        continue
    form, cls = m.group(1), m.group(2)
    base = os.path.splitext(os.path.basename(dfm))[0]
    has_shot = form in shot_forms or base in shot_forms
    has_html = base in converted_dfm
    html_not_required_reason = html_not_required.get(base)
    # 多國語言：同名 .cpp 含 iLanguageCountry（機台語言切換機制）
    cpp = os.path.splitext(dfm)[0] + '.cpp'
    has_i18n = False
    if os.path.exists(cpp):
        try:
            with open(cpp, encoding='cp950', errors='replace') as f:
                has_i18n = 'iLanguageCountry' in f.read()
        except Exception:
            pass
    all_dfm.append({'form': form, 'cls': cls, 'dfm': os.path.basename(dfm),
                    'shot': has_shot, 'html': has_html, 'htmlNotRequired': bool(html_not_required_reason),
                    'htmlNotRequiredReason': html_not_required_reason or '', 'i18n': has_i18n})
all_dfm.sort(key=lambda r: r['form'].lower())

dynamic_classes = [
    {'cls': 'TMyOmronPanel',   'file': 'EJ1N/MyOmronPanel.cpp',  'host': 'fOmron / OmronEJ1N',      'ui': 'GroupBox+Image(PV/℃/STOP/Event)+edSV+CheckBox', 'inVCL': True},
    {'cls': 'TMotorTestClass', 'file': 'uMotorTest.cpp',         'host': 'fMotorTest / uMotorTest（＋Main.MotorView.html）', 'ui': '每馬達列 CheckBox+Label+Edit×2＋Panel＋MotorLabel＋HomeLed(TALed)＋10×MotorLed(TALed)；Motor View 11 顆狀態 LED 動態生成，列數依 Visible(enable) 增減', 'inVCL': True},
    {'cls': 'THomeClass',      'file': 'uhome.cpp',              'host': 'fHome / uhome',           'ui': 'Label+ALed+Edit', 'inVCL': True},
    {'cls': 'TMyBinPanel',     'file': 'cBinSel.cpp',            'host': 'fBinSel / cBinSel',       'ui': 'Bin 指派表格（已改 Setup.BinSel.html HTML table，取代 TMyTray）', 'inVCL': True},
    {'cls': 'TMyYieldPanel',   'file': 'uYieldMonitoring.cpp',   'host': 'fYieldMonitoring',        'ui': 'Panel+Label+Edit+ComboBox+CheckBox', 'inVCL': True},
    {'cls': 'TMySecurity',     'file': 'cSecurity.cpp',          'host': 'fSecurity / cSecurity',   'ui': 'Panel+SpeedButton（權限等級）', 'inVCL': True},
    # Steven 20260919：THTSLKClass 這一列代表的是一個**家族**，不是單一 class。
    # 只寫「Label+Edit」會讓人以為四種列型長得一樣，轉頁時就會漏掉 NS 那一列。
    {'cls': 'THTSLKClass',     'file': 'ContactForce.cpp',       'host': 'fContactForce / ContactForce（Setup.ContactForce.html）',
     'ui': '家族共四種列型：THTSLKClass 本身兩列（含 NS 那一列）；'
           'THTSLKIndClass（ContactForce.h:105）、THTDieForceSLKClass（:53）、'
           'THTDieForceOneByOneSLKClass（:79）各一列。'
           'HTWidgets.makeContactForceGroup 的四個變體 std／ind／dieforce／dieforce1 即對應之',
     'inVCL': True},
    # Steven 20260919：VacuumUnit 每個吸嘴一塊，4 欄×2 列。
    {'cls': 'TMyVacuumPanel',  'file': 'VacuumUnit/MyVacuumPanel.cpp', 'host': 'fVacuumUnit / VacuumUnit',
     'ui': 'GroupBox 81×177（VACUUM_UNIT_WIDTH/HEIGHT）＋ImgVacuumPanel（TImage，執行期 Canvas 畫三行字：'
           '目前真空值 clMaroon／Event clBlack／閥值 clBlue，不是三個 Panel）＋edSV＋btnSV(Set)＋'
           'bplOn/bplOff（^ v，TBtnPanelLane）＋myld1（TMyLed LEDSqLarge）。每個吸嘴一個，4 欄×2 列',
     'inVCL': True},
    {'cls': 'TMyATPanel',      'file': 'AutoTemperature.cpp',    'host': 'fMainAT（無截圖）',        'ui': 'ATC 溫控動態面板', 'inVCL': True},
    {'cls': 'TQwertyKeyClass', 'file': 'myQwertyKeyBoard.cpp',   'host': 'fQwertyKey（虛擬鍵盤）',  'ui': '動態按鍵配置（僅封裝畫面便於操作，免實作）', 'inVCL': False, 'na': True},
]

HEAD = '/* 由 _scan_dfm_shot.py 產生：截圖索引補充資料 */'
BLK = {
    'ALL_DFM': 'window.ALL_DFM = ' + json.dumps(all_dfm, ensure_ascii=False, indent=1) + ';',
    'DYNAMIC_CLASSES': 'window.DYNAMIC_CLASSES = ' + json.dumps(dynamic_classes, ensure_ascii=False, indent=1) + ';',
}


def patch_meta(path):
    """只取代 ALL_DFM / DYNAMIC_CLASSES 兩個區塊，檔案其餘內容原樣保留。

    Steven 20260919：舊版是 mode='w' 整檔覆寫。但 screenshot_meta.js 現在還裝著
    另外五個區塊（FILE_IO_STATUS / SIM0_DECL_STATUS / PAGE_WIRE_STATUS /
    TAG_WIRE_STATUS / FORM_SHOW_STATUS），都是別的產生器附加上去的。
    整檔覆寫會把那五塊一起刪掉，而畫面上**只表現成「表③⑤⑥⑦ 是空的」**，
    不會丟例外、不會報錯 —— 這種靜默損壞最難發現，所以改成整塊取代。
    做法參考同目錄的 gen_tag_status.py / gen_form_show_status.py。
    """
    if not os.path.isfile(path):
        io.open(path, 'w', encoding='utf-8', newline='').write(
            HEAD + '\n' + BLK['ALL_DFM'] + '\n' + BLK['DYNAMIC_CLASSES'] + '\n')
        return 'created'
    src = io.open(path, encoding='utf-8').read()
    out, miss = src, []
    for name in ('ALL_DFM', 'DYNAMIC_CLASSES'):
        # 區塊結尾認「行首的 ];」：JSON indent=1 讓內層物件收在 ' }'，
        # 只有最外層陣列會頂到第 0 欄，所以這個錨點在區塊內不會誤中。
        pat = re.compile(r'window\.' + name + r' = \[.*?^\];', re.S | re.M)
        if not pat.search(out):
            miss.append(name)
            continue
        blk = BLK[name]
        out = pat.sub(lambda m: blk, out, count=1)
    if miss:
        # 找不到錨點就直接失敗。退回整檔覆寫會把另外五個區塊洗掉，
        # 寧可什麼都不寫，也不要靜默毀掉別人的資料。
        raise SystemExit('X %s 找不到區塊：%s（格式變了？未寫入）' % (path, '、'.join(miss)))
    if out != src:
        io.open(path, 'w', encoding='utf-8', newline='').write(out)
        return 'updated'
    return 'unchanged'


for _meta in METAS:
    print('%-9s %s' % (patch_meta(_meta), _meta))

ns = sum(1 for r in all_dfm if not r['shot'])
nh = sum(1 for r in all_dfm if r['html'])
nna = sum(1 for r in all_dfm if r['htmlNotRequired'])
ni = sum(1 for r in all_dfm if r['i18n'])
print(f'ALL_DFM: {len(all_dfm)} forms（無截圖 {ns}／已轉html {nh}／免轉html {nna}／有多國語言 {ni}）; '
      f'DYNAMIC_CLASSES: {len(dynamic_classes)} ({sum(1 for d in dynamic_classes if not d["inVCL"] and not d.get("na"))} pending)')
