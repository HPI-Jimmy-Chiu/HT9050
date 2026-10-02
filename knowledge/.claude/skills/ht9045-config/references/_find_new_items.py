# -*- coding: utf-8 -*-
"""
Find new Config codes in V3.33.900.0 not yet in Description.ini,
look up cConfiguration.cpp for context, and print new entries.
"""
import re

CFG_H    = r'D:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\Config.h'
CFG_CPP  = r'D:\HT9045\HT9011UC_Code_V3.33.900.0_20260331\cConfiguration.cpp'
DESC_INI = r'D:\HT9045\.github\skills\ht9045-config\references\Description.ini'

with open(CFG_H,   encoding='cp950', errors='replace') as f: h_content   = f.read()
with open(CFG_CPP, encoding='cp950', errors='replace') as f: cpp_content = f.read()
with open(DESC_INI,encoding='cp950', errors='replace') as f: ini_content = f.read()

ini_sections = set(re.findall(r'^\[([A-Za-z0-9_\-]+)\]', ini_content, re.M))

# Codes to actually add (manually verified - commented-out ones excluded)
NEW_CODES = {
    'C24': {
        'en_title':   'Initial Start Check Cylinder',
        'zh_title':   'Initial Start 檢查氣缸狀態',
        'en_desc':    'When enabled, performs a cylinder status check during the Initial Start sequence to verify all cylinders are in the correct home position before production begins.',
        'zh_desc':    'Initial Start 流程中自動檢查所有氣缸是否回到原點位置，確保開始生產前機構狀態正常。',
    },
    'E87': {
        'en_title':   'Pickup Error At Loader Need Open Door',
        'zh_title':   'Loader 吸取異常時需開安全門',
        'en_desc':    'When enabled, if a pickup error occurs at the Loader, the handler will require the operator to open the safety door before the error can be cleared.',
        'zh_desc':    'Loader 發生吸取異常時，要求操作者開啟安全門才能繼續排除故障，提升操作安全性。',
    },
    'E88': {
        'en_title':   'InArm Height Follow 7000 Mode',
        'zh_title':   'InArm 高度跟隨 (7000 模式)',
        'en_desc':    'Enables the In Arm Z-axis height to follow the 7000-series machine profile, adapting the pick height dynamically for specific kit configurations.',
        'zh_desc':    '啟用後 In Arm Z 軸高度跟隨 7000 系列機台的取料高度設定，適應特定 Kit 硬體配置。',
    },
    'E89': {
        'en_title':   'InArm Hot Plate Pitch Use Scale',
        'zh_title':   'InArm Hot Plate Pitch 使用 Scale 補償',
        'en_desc':    'When enabled, the In Arm applies the Scale compensation value when calculating the Hot Plate pitch movement, improving placement accuracy at different temperature modes.',
        'zh_desc':    'In Arm 在計算 Hot Plate Pitch 移動量時套用 Scale 補償值，提升不同溫度模式下的放置精度。',
    },
    'F36': {
        'en_title':   'Out Shuttle Lose IC - Reset Set All To Error',
        'zh_title':   'Out Shuttle 掉料 Reset 後設為全部 Error',
        'en_desc':    'When an Out Shuttle lose-IC event occurs, pressing Reset will mark all IC positions on that Shuttle as Error bin, preventing untested ICs from entering the Unloader.',
        'zh_desc':    'Out Shuttle 發生掉料異常後，按 Reset 時自動將該 Shuttle 上所有 IC 位置標記為 Error Bin，防止未測 IC 流入 Unloader。',
    },
    'L39': {
        'en_title':   'Wait For Temperature In Range Before Auto Run',
        'zh_title':   '進入溫度範圍後才開始運行',
        'en_desc':    'Controls whether the handler waits for the temperature to reach the target range before starting Auto Run. Sub-items configure the enablement and stabilization wait time.',
        'zh_desc':    '控制機台是否在溫度進入設定範圍後才開始自動運行。子項目設定啟用狀態與溫度穩定等待時間。',
    },
    'N19': {
        'en_title':   'Secondary FTP Upload Settings',
        'zh_title':   '第二組 FTP 上傳設定',
        'en_desc':    'Configures a secondary FTP server (User / Password / Host / Path) for data upload, independent of the primary N06 FTP settings.',
        'zh_desc':    '設定第二組 FTP 伺服器帳號（使用者名稱 / 密碼 / 主機 / 路徑），與主要 N06 FTP 設定獨立作業。',
    },
    'N40': {
        'en_title':   'Handler Data Backup to FTP',
        'zh_title':   '機台資料備份至 FTP',
        'en_desc':    'Enables automatic backup of handler data (logs, setup files, etc.) to a designated FTP server. Configure the FTP account, host, and upload path in the sub-items.',
        'zh_desc':    '啟用機台資料（Log、Setup 檔等）自動備份至指定 FTP 伺服器。子項目設定 FTP 帳號、主機與上傳路徑。',
    },
    'N41': {
        'en_title':   'Handler Data Backup to Local Disk',
        'zh_title':   '機台資料備份至本機磁碟',
        'en_desc':    'Enables automatic backup of handler data to a local disk path. Configure the destination path in the sub-item.',
        'zh_desc':    '啟用機台資料自動備份至本機磁碟路徑。子項目設定目標資料夾路徑。',
    },
    'P47': {
        'en_title':   'Use Tray Thickness to Adjust Z Height',
        'zh_title':   '依 Tray 厚度自動補償 Z 軸高度',
        'en_desc':    'When enabled, the handler automatically compensates the Z-axis pick/place height based on the configured Tray thickness value, reducing placement errors caused by Tray height variation.',
        'zh_desc':    '啟用後，機台根據設定的 Tray 厚度值自動補償 In/Out Arm Z 軸高度，降低不同 Tray 厚度造成的取放料誤差。',
    },
    'P61': {
        'en_title':   'Use Tray Tap Function',
        'zh_title':   '使用 Tray Tap 功能',
        'en_desc':    'When enabled, activates the Tray Tap mechanism to gently tap the Tray to ensure ICs are properly seated before placement, reducing displacement errors.',
        'zh_desc':    '啟用後觸發 Tray Tap 機構輕拍 Tray，確保 IC 在放置前已正確到位，減少置偏異常。',
    },
}

print("Entries to add to Description.ini:")
print(f"Count: {len(NEW_CODES)}")
print()

for code, info in sorted(NEW_CODES.items()):
    # Check it's really not in ini already
    if code in ini_sections:
        print(f"[{code}] ALREADY EXISTS - skip")
        continue
    print(f"[{code}]")
    print(f"\tEnglish_Title={info['en_title']}")
    print(f"\tKorean_Title=")
    print(f"\tChinese_Title={info['zh_title']}")
    print(f"\tEnglish_Description={info['en_desc']}")
    print(f"\tKorean_Description=")
    print(f"\tChinese_Description={info['zh_desc']}")
    print()
