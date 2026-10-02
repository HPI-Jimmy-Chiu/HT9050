# -*- coding: utf-8 -*-
"""
SAFE insert: splits Description.ini by lines that start at column 0 with [CODE]
(section headers never have leading whitespace; bracket refs in desc text do).
"""
import re, os, shutil

DESC_INI = r'D:\HT9045\.github\skills\ht9045-config\references\Description.ini'
BACKUP   = DESC_INI + '.bak2'

NEW_ENTRIES = {
    'C24': {
        'en_title':  'Initial Start Check Cylinder',
        'zh_title':  'Initial Start 檢查氣缸狀態',
        'en_desc':   'When enabled, performs a cylinder status check during the Initial Start sequence to verify all cylinders are in the correct home position before production begins.',
        'zh_desc':   'Initial Start 流程中自動檢查所有氣缸是否回到原點位置，確保開始生產前機構狀態正常。',
    },
    'E87': {
        'en_title':  'Pickup Error At Loader Need Open Door',
        'zh_title':  'Loader 吸取異常時需開安全門',
        'en_desc':   'When enabled, if a pickup error occurs at the Loader, the handler will require the operator to open the safety door before the error can be cleared.',
        'zh_desc':   'Loader 發生吸取異常時，要求操作者開啟安全門才能繼續排除故障，提升操作安全性。',
    },
    'E88': {
        'en_title':  'InArm Height Follow 7000 Mode',
        'zh_title':  'InArm 高度跟隨 (7000 模式)',
        'en_desc':   'Enables the In Arm Z-axis height to follow the 7000-series machine profile, adapting the pick height dynamically for specific kit configurations.',
        'zh_desc':   '啟用後 In Arm Z 軸高度跟隨 7000 系列機台的取料高度設定，適應特定 Kit 硬體配置。',
    },
    'E89': {
        'en_title':  'InArm Hot Plate Pitch Use Scale',
        'zh_title':  'InArm Hot Plate Pitch 使用 Scale 補償',
        'en_desc':   'When enabled, the In Arm applies the Scale compensation value when calculating the Hot Plate pitch movement, improving placement accuracy at different temperature modes.',
        'zh_desc':   'In Arm 在計算 Hot Plate Pitch 移動量時套用 Scale 補償值，提升不同溫度模式下的放置精度。',
    },
    'F36': {
        'en_title':  'Out Shuttle Lose IC - Reset Set All To Error',
        'zh_title':  'Out Shuttle 掉料 Reset 後設為全部 Error',
        'en_desc':   'When an Out Shuttle lose-IC event occurs, pressing Reset will mark all IC positions on that Shuttle as Error bin, preventing untested ICs from entering the Unloader.',
        'zh_desc':   'Out Shuttle 發生掉料異常後，按 Reset 時自動將該 Shuttle 上所有 IC 位置標記為 Error Bin，防止未測 IC 流入 Unloader。',
    },
    'L39': {
        'en_title':  'Wait For Temperature In Range Before Auto Run',
        'zh_title':  '進入溫度範圍後才開始運行',
        'en_desc':   'Controls whether the handler waits for the temperature to reach the target range before starting Auto Run. Sub-items configure the enablement and stabilization wait time.',
        'zh_desc':   '控制機台是否在溫度進入設定範圍後才開始自動運行。子項目設定啟用狀態與溫度穩定等待時間。',
    },
    'N19': {
        'en_title':  'Secondary FTP Upload Settings',
        'zh_title':  '第二組 FTP 上傳設定',
        'en_desc':   'Configures a secondary FTP server (User / Password / Host / Path) for data upload, independent of the primary N06 FTP settings.',
        'zh_desc':   '設定第二組 FTP 伺服器帳號（使用者名稱 / 密碼 / 主機 / 路徑），與主要 N06 FTP 設定獨立作業。',
    },
    'N40': {
        'en_title':  'Handler Data Backup to FTP',
        'zh_title':  '機台資料備份至 FTP',
        'en_desc':   'Enables automatic backup of handler data (logs, setup files, etc.) to a designated FTP server. Configure the FTP account, host, and upload path in the sub-items.',
        'zh_desc':   '啟用機台資料（Log、Setup 檔等）自動備份至指定 FTP 伺服器，子項目設定 FTP 帳號、主機與上傳路徑。',
    },
    'N41': {
        'en_title':  'Handler Data Backup to Local Disk',
        'zh_title':  '機台資料備份至本機磁碟',
        'en_desc':   'Enables automatic backup of handler data to a local disk path. Configure the destination path in the sub-item.',
        'zh_desc':   '啟用機台資料自動備份至本機磁碟路徑，子項目設定目標資料夾路徑。',
    },
    'P47': {
        'en_title':  'Use Tray Thickness to Adjust Z Height',
        'zh_title':  '依 Tray 厚度自動補償 Z 軸高度',
        'en_desc':   'When enabled, the handler automatically compensates the Z-axis pick/place height based on the configured Tray thickness value, reducing placement errors caused by Tray height variation.',
        'zh_desc':   '啟用後，機台根據設定的 Tray 厚度值自動補償 In/Out Arm Z 軸高度，降低不同 Tray 厚度造成的取放料誤差。',
    },
    'P61': {
        'en_title':  'Use Tray Tap Function',
        'zh_title':  '使用 Tray Tap 功能',
        'en_desc':   'When enabled, activates the Tray Tap mechanism to gently tap the Tray to ensure ICs are properly seated before placement, reducing displacement errors.',
        'zh_desc':   '啟用後觸發 Tray Tap 機構輕拍 Tray，確保 IC 在放置前已正確到位，減少置偏異常。',
    },
}

HEADER_RE = re.compile(r'^\[([A-Za-z0-9_\-]+)\]$')  # only at col-0, entire line

GROUP_ORDER = list('ABCDEFGILMNOP')

def code_sort_key(code):
    parts = re.split(r'[-_]', code)
    letter = parts[0][0].upper() if parts else 'Z'
    main_num = int(parts[0][1:]) if len(parts[0]) > 1 and parts[0][1:].isdigit() else 0
    sub_nums = []
    for p in parts[1:]:
        try: sub_nums.append(int(p))
        except: sub_nums.append(0)
    gi = GROUP_ORDER.index(letter) if letter in GROUP_ORDER else 99
    return (gi, main_num, sub_nums)

def make_entry(code, info):
    return (
        f'[{code}]\n'
        f'\tEnglish_Title={info["en_title"]}\n'
        f'\tKorean_Title=\n'
        f'\tChinese_Title={info["zh_title"]}\n'
        f'\tEnglish_Description={info["en_desc"]}\n'
        f'\tKorean_Description=\n'
        f'\tChinese_Description={info["zh_desc"]}\n'
    )

# ── Parse Description.ini safely ─────────────────────────────
with open(DESC_INI, encoding='cp950', errors='replace') as f:
    lines = f.readlines()

# Split into blocks: each block starts at a line that matches HEADER_RE
blocks = []   # list of (code, [lines])
cur_code = None
cur_lines = []

for line in lines:
    stripped = line.rstrip('\r\n')
    m = HEADER_RE.match(stripped)
    if m:
        if cur_code is not None:
            blocks.append((cur_code, cur_lines))
        cur_code = m.group(1)
        cur_lines = [line]
    else:
        if cur_code is not None:
            cur_lines.append(line)

if cur_code is not None:
    blocks.append((cur_code, cur_lines))

print(f'Parsed {len(blocks)} sections (expected 726)')
existing_codes = set(c for c, _ in blocks)

# Verify
dups = [c for c in existing_codes if sum(1 for cc, _ in blocks if cc == c) > 1]
if dups:
    print(f'WARNING: duplicate codes: {dups}')

# ── Insert new entries ────────────────────────────────────────
inserted = []
for new_code, info in NEW_ENTRIES.items():
    if new_code in existing_codes:
        print(f'  SKIP {new_code} (already exists)')
        continue

    new_key = code_sort_key(new_code)
    insert_pos = len(blocks)
    for i, (code, _) in enumerate(blocks):
        if code_sort_key(code) > new_key:
            insert_pos = i
            break

    entry_lines = make_entry(new_code, info).splitlines(keepends=True)
    blocks.insert(insert_pos, (new_code, entry_lines))
    existing_codes.add(new_code)
    print(f'  INSERT [{new_code}] at pos {insert_pos} (before [{blocks[insert_pos+1][0] if insert_pos+1 < len(blocks) else "EOF"}])')
    inserted.append(new_code)

# Backup and write
shutil.copy2(DESC_INI, BACKUP)
print(f'\nBackup: {BACKUP}')

with open(DESC_INI, 'w', encoding='cp950', errors='replace') as f:
    for code, blines in blocks:
        for bl in blines:
            f.write(bl)

print(f'Written: {DESC_INI}')
print(f'Total sections: {len(blocks)}  (added: {len(inserted)})')

# Quick verify
with open(DESC_INI, encoding='cp950', errors='replace') as f:
    verify_lines = f.readlines()
verify_sections = [HEADER_RE.match(l.rstrip()).group(1) for l in verify_lines if HEADER_RE.match(l.rstrip())]
print(f'Verify: {len(verify_sections)} top-level [CODE] headers')
for nc in inserted:
    assert nc in verify_sections, f'MISSING: {nc}'
    print(f'  OK: [{nc}] present')
