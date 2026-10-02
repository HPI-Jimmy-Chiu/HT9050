import re
import sys

nsi = r'D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI.nsi'

new_cv = sys.argv[1] if len(sys.argv) > 1 else ''
new_bv = sys.argv[2] if len(sys.argv) > 2 else ''

if new_cv and not new_cv.endswith('_'):
    new_cv = new_cv + '_'
if new_bv and not new_bv.startswith('_'):
    new_bv = '_' + new_bv

with open(nsi, 'rb') as f:
    lines = f.read().decode('cp950').splitlines(keepends=True)

def rebuild(lines, key, active_line, preset_lines):
    first_idx = None
    new_lines = []
    for line in lines:
        stripped = line.rstrip('\r\n')
        if re.match(r'^;?!define ' + re.escape(key) + r'\b', stripped):
            if first_idx is None:
                first_idx = len(new_lines)
        else:
            new_lines.append(line)
    if first_idx is not None:
        block = [active_line + '\n'] + [p + '\n' for p in preset_lines]
        new_lines[first_idx:first_idx] = block
    return new_lines

cv_presets = [
    ';!define CustomVersion "TF-AMD_"',
    ';!define CustomVersion "KL_"',
    ';!define CustomVersion "MTK_"',
    ';!define CustomVersion "EVAN_"',
    ';!define CustomVersion "QROVO_"',
]
bv_presets = [
    ';!define BetaVersion "_BETA"',
    ';!define BetaVersion "_EurekaLog"',
    ';!define BetaVersion "_CodeGuard"',
]

lines = rebuild(lines, 'CustomVersion', '!define CustomVersion "' + new_cv + '"', cv_presets)
lines = rebuild(lines, 'BetaVersion', '!define BetaVersion "' + new_bv + '"', bv_presets)

with open(nsi, 'wb') as f:
    f.write(''.join(lines).encode('cp950'))

print('OK')
for line in lines:
    s = line.rstrip('\r\n')
    if s.startswith('!define CustomVersion') or s.startswith('!define BetaVersion') or s.startswith('!define HandlerType "HT9'):
        print(' ', s)
