# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 新檔。為 FILE_IO_STATUS 補 liveApi/liveOk 欄，需先啟動 wb_serve。（保存版）
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""把 liveApi 欄加進 screenshot_meta.js 的 FILE_IO_STATUS。

AI(W906-FW-LIVEAPI) 20260915。

CPP 模式下，FILE_IO_STATUS 原本的 status/jsonFile 記的是已停用的 JSON 快照路線。
這支依「實際跑著的 wb_serve 回報的 /api/system 與 /api/text 索引」補上 liveApi，
讓每一列能回答：這個檔現在是哪條 API 在服務、通不通。

匹配用 basename（小寫），因為 FILE_IO_STATUS 的路徑寫法與 API 回報的大小寫不一致
（System vs system、iniData vs IniData）。basename 在這 40 筆裡幾乎唯一，例外是
ATC.ini 與 Gerneral.ini（system 與 config / SECS 各一份），靠父目錄再分一次。
"""
import io
import json
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

BS = chr(92)                     # 反斜線，避開字面量轉義問題
META = 'D:' + BS + 'HT9045' + BS + 'page' + BS + 'screenshot_meta.js'
TMP = 'C:/Users/steven/AppData/Local/Temp/'

sysidx = json.load(open(TMP + 'sysidx.json', encoding='utf-8'))
textidx = json.load(open(TMP + 'textidx.json', encoding='utf-8'))


def split_path(p):
    """回傳 (basename小寫, 父目錄名小寫)。"""
    p = p.replace('/', BS).rstrip(BS)
    parts = [x for x in p.split(BS) if x]
    base = parts[-1].lower() if parts else ''
    parent = parts[-2].lower() if len(parts) >= 2 else ''
    return base, parent


# basename -> [(api, available, parent)]
idx = {}
for f in (sysidx.get('files') or sysidx.get('entries') or []):
    base, parent = split_path(f['path'])
    idx.setdefault(base, []).append(('/api/system/' + f['name'], f['available'], parent))
for r in textidx.get('roots', []):
    base, parent = split_path(r['path'])
    idx.setdefault(base, []).append(('/api/text/' + r['name'], r['available'], parent))

# FILE_IO_STATUS 有些列描述的是「一整個資料夾」或帶括號的寫法，basename 抓不到，
# 這些逐一指定。沒列在這裡、也匹配不到的，就是真的沒有 API 在服務。
MANUAL = [
    ('DioCfg',          '/api/system/dio', True),
    ('PM_*.ini',        '/api/system/pm* (7 支)', True),
    ('AlarmCodeList',   '/api/system/alarmCodeList', True),
    ('ReleaseNote',     '/api/text/releaseNote', True),
]


def find_api(file_field):
    for key, api, av in MANUAL:
        if key.lower() in file_field.lower():
            return api, av
    head = file_field.split('（')[0].split('(')[0].strip()
    base, parent = split_path(head)
    cand = idx.get(base)
    if not cand:
        return '', None
    if len(cand) > 1:
        for api, av, par in cand:
            if par and par == parent:
                return api, av
    return cand[0][0], cand[0][1]


src = open(META, encoding='utf-8').read()
m = re.search(r'FILE_IO_STATUS = ' + re.escape('[') + r'\n(.*?)\n' + re.escape('];'), src, re.S)
if not m:
    raise SystemExit('找不到 FILE_IO_STATUS 區塊')

out, n_live, n_miss, report = [], 0, 0, []
for ln in m.group(1).split('\n'):
    if '"file"' not in ln or '"liveApi"' in ln:
        out.append(ln)
        continue
    row = json.loads(ln.strip().rstrip(','))
    api, av = find_api(row['file'])
    row['liveApi'] = api
    row['liveOk'] = ('yes' if av else 'no') if api else ''
    n_live, n_miss = (n_live + 1, n_miss) if api else (n_live, n_miss + 1)
    report.append((row['file'][:60], api or '-', row['liveOk']))
    out.append(' ' + json.dumps(row, ensure_ascii=False) + ',')

for i in range(len(out) - 1, -1, -1):
    if out[i].rstrip().endswith(','):
        out[i] = out[i].rstrip()[:-1]
        break

src = src[:m.start(1)] + '\n'.join(out) + src[m.end(1):]
open(META, 'w', encoding='utf-8', newline='').write(src)

print('%-62s %-30s %s' % ('file', 'liveApi', 'ok'))
for r in report:
    print('%-62s %-30s %s' % r)
print()
print('有 live API: %d    沒有: %d' % (n_live, n_miss))
