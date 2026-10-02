# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 產生 wb_serve.cpp 的 40 筆 SysFileEntry 初始化列。
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""gen_sysfile_table.py -- 產生 wb_serve.cpp 的 SysFileEntry 表格內容。

AI(W906-FW-SYSFILE) 20260915。

為什麼用產生的：表格有 30+ 筆，手打會打錯，而且路徑一律要取 golden 的全域
（不是字面字串）。這支把「鍵名 / 全域 / 檔名 / 格式」四欄攤開，產出可直接
貼進 wb_serve.cpp 的 C++ 初始化列。

規則：
  * 有專屬檔案全域的（asErrNotePath 之類）-> path=&該全域, suffix=0
  * 只有目錄全域的（asSystemPath / AuthPath）-> path=&目錄全域, suffix="檔名"
  * 二進位（.dat / .db3）一律不列 —— ini/csv 機制碰不到，硬接只會產生假資料
"""
import io, os, sys
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# (key, global, suffix or None, csv?, 實體路徑用來檢查存在)
ROOT = r'D:\HT9045'
E = [
    # --- 既有 7 筆（保持不動，這裡只是為了完整輸出） ---
    ('gerneral',  'asGeneralPath', None, False, r'system\Gerneral.ini'),
    ('teach',     'asTeachPath',   None, False, r'system\teach.ini'),
    ('motTable',  'MotTablePath',  None, True,  r'System\Mot_Table.csv'),
    ('ioTable',   'IoTablePath',   None, True,  r'System\IO_Table.csv'),
    ('config',    'AuthPath',      'config.ini',  False, r'config\config.ini'),
    ('dio',       None,            None, False, None),      # 動態解析
    ('lastSet',   'AuthPath',      'LastSet.ini', False, r'config\LastSet.ini'),

    # --- 專屬檔案全域 ---
    ('errNote',       'asErrNotePath',                 None, False, r'system\SpecialErrNote.ini'),
    ('description',   'ConfigMemoPath',                None, False, r'config\Description.ini'),
    ('setupInf',      'LastDataPath',                  None, False, r'SetUp.inf'),
    ('trayForm',      'TrayTablePath',                 None, True,  r'System\TrayForm.csv'),
    ('plateForm',     'PlateTablePath',                None, True,  r'System\PlateForm.csv'),
    ('trayStepSpeed', 'asTrayStepSpeedByMachinePatch', None, False, r'system\TrayStepSpeed.ini'),
    ('machineLife',   'asMachineLifePath',             None, False, r'system\MachineLife.ini'),
    ('arms',          'asARSMParaPath',                None, False, r'system\ARMS.ini'),
    ('secsGem',       'SecsGemPath',                   None, False, r'SECS\SECS\SYSTEM\Gerneral.ini'),

    # --- system\ 目錄 + 固定檔名 ---
    ('contactInfo',   'asSystemPath', 'ContactInfo.ini',      False, r'system\ContactInfo.ini'),
    ('autoTemp',      'asSystemPath', 'AutoTemperature.ini',  False, r'system\AutoTemperature.ini'),
    ('atcSystem',     'asSystemPath', 'ATC.ini',              False, r'system\ATC.ini'),
    ('barcode',       'asSystemPath', 'Barcode.ini',          False, r'system\Barcode.ini'),
    ('padInterface',  'asSystemPath', 'PadInterfacePara.ini', False, r'system\PadInterfacePara.ini'),
    ('eventLogLevel', 'asSystemPath', 'EvenLogLevel.ini',     False, r'system\EvenLogLevel.ini'),
    ('socketCount',   'asSystemPath', 'SocketCount.ini',      False, r'system\SocketCount.ini'),
    ('motorTest',     'asSystemPath', 'MotorTest.ini',        False, r'system\MotorTest.ini'),
    ('colorSensor',   'asSystemPath', 'ColorSensorType.ini',  False, r'system\ColorSensorType.ini'),
    ('mvData',        'asSystemPath', 'MVData.ini',           False, r'system\MVData.ini'),
    ('rpDefault',     'asSystemPath', 'RPDefault.ini',        False, r'system\RPDefault.ini'),
    ('alarmDesc',     'asSystemPath', 'AlarmDescription.ini', False, r'system\AlarmDescription.ini'),

    # --- config\ 目錄 + 固定檔名 ---
    ('securityNew',   'AuthPath', 'Security_new.def',        False, r'config\Security_new.def'),
    ('criticalPara',  'AuthPath', 'CriticalParaControl.ini', False, r'config\CriticalParaControl.ini'),
    ('esdConfig',     'AuthPath', 'ESDconfig.ini',           False, r'config\ESDconfig.ini'),
    ('atcConfig',     'AuthPath', 'ATC.ini',                 False, r'config\ATC.ini'),

    # --- PMAlarm\（各有專屬全域） ---
    ('pmMonth',       'sPMList_Month',       None, False, r'PMAlarm\PM_Month.ini'),
    ('pmQuarter',     'sPMList_Quarter',     None, False, r'PMAlarm\PM_Quarter.ini'),
    ('pmYear',        'sPMList_Year',        None, False, r'PMAlarm\PM_Year.ini'),
    ('pmTemperature', 'sPMList_Temperature', None, False, r'PMAlarm\PM_Temperature.ini'),
    ('pmEsd',         'sPMList_ESD',         None, False, r'PMAlarm\PM_ESD.ini'),
    ('pmIonFan',      'sPMList_IonFan',      None, False, r'PMAlarm\PM_IonFan.ini'),
    ('pmSetting',     'sPMSetting',          None, False, r'PMAlarm\PM_Setting.ini'),
]


def main():
    lines, miss, n = [], [], 0
    for key, g, suf, csv, rel in E:
        if g is None:
            lines.append('        { "%-13s 0,              0,            false },'
                         % (key + '",'))
            n += 1
            continue
        gp = '&' + g
        sf = ('"%s"' % suf) if suf else '0'
        lines.append('        { "%-13s %-31s %-24s %-5s },'
                     % (key + '",', gp + ',', sf + ',', 'true' if csv else 'false'))
        n += 1
        if rel and not os.path.isfile(os.path.join(ROOT, rel)):
            miss.append((key, rel))
    print('    static const SysFileEntry kTab[%d] = {' % n)
    print('\n'.join(lines).rstrip(','))
    print('    };')
    print('    *n = %d;' % n)
    print()
    print('// 本機不存在的（索引會回 available:false，契約仍完整）: %d' % len(miss))
    for k, r in miss:
        print('//   %-14s %s' % (k, r))


main()
