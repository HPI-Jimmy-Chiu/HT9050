"""
create_missing_parent_yaml.py
=============================
為有 PNG 截圖但沒 YAML 的 sections 自動建立 YAML + i18n 條目。
這些通常是「父 section」（如 A10, N07）只有 GroupBox 容器截圖，但子項已存在。

流程：
  1. 掃描 screenshots/ 下所有 <sec>-overview.png
  2. 若 data/<letter>/<sec>.yaml 不存在 → 建立
  3. 從 Description.ini 取 title/desc 寫入 en.yaml + zh-TW.yaml
"""

import os
import re

REFS = r'd:\HT9045\.github\skills\ht9045-config\references'
DATA = os.path.join(REFS, 'data')
SHOT = os.path.join(REFS, 'output', 'screenshots')
INI  = os.path.join(REFS, 'Description.ini')
EN   = os.path.join(REFS, 'i18n', 'en.yaml')
ZH   = os.path.join(REFS, 'i18n', 'zh-TW.yaml')
LETTERS = list('ABCDEFGILMNOP')


def parse_ini():
    out = {}
    cur = None
    cur_key = None
    with open(INI, encoding='cp950', errors='replace') as f:
        for raw in f:
            line = raw.rstrip('\n').rstrip('\r')
            if not line.strip():
                cur_key = None
                continue
            m = re.match(r'^\[([^\]]+)\]\s*$', line)
            if m:
                cur = m.group(1).strip()
                out[cur] = {}
                cur_key = None
                continue
            if cur is None:
                continue
            stripped = line.lstrip('\t ')
            if '=' in stripped and not stripped.startswith(';'):
                key, _, val = stripped.partition('=')
                key = key.strip()
                val = val.strip()
                out[cur][key] = val
                cur_key = key
            elif cur_key is not None:
                out[cur][cur_key] = (out[cur][cur_key] + ' ' + stripped.strip()).strip()
    return out


def existing_i18n_keys(path):
    keys = set()
    with open(path, encoding='utf-8') as f:
        for ln in f:
            if ':' in ln and not ln.lstrip().startswith('#'):
                k = ln.split(':', 1)[0].strip()
                if k:
                    keys.add(k)
    return keys


def append_i18n(path, entries):
    if not entries:
        return
    with open(path, 'a', encoding='utf-8', newline='\n') as f:
        f.write('\n# === Auto-created parent section entries ===\n')
        for k, v in entries:
            esc = v.replace('\\', '\\\\').replace('"', '\\"')
            f.write(f'{k}: "{esc}"\n')


def make_yaml(sec, letter, has_png):
    lines = [
        f'# Auto-generated parent section YAML',
        f'# Section: {sec}',
        f'',
        f'section: {sec}',
        f'group: {letter}',
        f'caption_id: {sec}.caption',
        f'desc_id: {sec}.desc',
        f'when_to_use_id: {sec}.when',
        f'warning_id: {sec}.warning',
        f'typical_value_id: {sec}.typical',
        f'ui: []',
        f'variables: []',
        f'ecid: null',
        f'ec_type: null',
        f'audience:',
        f'  developer: true',
        f'  operator: true',
        f'  customer: false',
        f'customer_codes: []',
        f'related_sections: []',
        f'related_functions: []',
    ]
    if has_png:
        lines.extend([
            f'screenshots:',
            f'  -',
            f'    file: {sec}-overview.png',
            f'    caption_id: {sec}.shot.overview',
        ])
    else:
        lines.append('screenshots: []')
    return '\n'.join(lines) + '\n'


def main():
    ini = parse_ini()
    en_keys = existing_i18n_keys(EN)
    zh_keys = existing_i18n_keys(ZH)
    new_en = []
    new_zh = []
    created = 0

    for letter in LETTERS:
        shot_dir = os.path.join(SHOT, letter)
        if not os.path.isdir(shot_dir):
            continue
        for fn in sorted(os.listdir(shot_dir)):
            if not fn.endswith('-overview.png'):
                continue
            sec = fn.replace('-overview.png', '')
            yaml_path = os.path.join(DATA, letter, sec + '.yaml')
            if os.path.exists(yaml_path):
                continue

            # Create YAML
            os.makedirs(os.path.join(DATA, letter), exist_ok=True)
            content = make_yaml(sec, letter, True)
            with open(yaml_path, 'w', encoding='utf-8', newline='\n') as f:
                f.write(content)
            created += 1
            print(f'  Created {sec}.yaml')

            # i18n entries
            ini_sec = ini.get(sec, {})
            en_cap = ini_sec.get('English_Title', sec)
            en_desc = ini_sec.get('English_Description', en_cap)
            zh_cap = ini_sec.get('Chinese_Title', '')
            zh_desc = ini_sec.get('Chinese_Description', zh_cap)

            for key_id, val in [
                (f'{sec}.caption', en_cap),
                (f'{sec}.desc', en_desc),
                (f'{sec}.when', ''),
                (f'{sec}.warning', ''),
                (f'{sec}.typical', ''),
                (f'{sec}.shot.overview', f'{sec} overview'),
            ]:
                if key_id not in en_keys:
                    new_en.append((key_id, val))
                    en_keys.add(key_id)

            for key_id, val in [
                (f'{sec}.caption', zh_cap or en_cap),
                (f'{sec}.desc', zh_desc or en_desc),
                (f'{sec}.when', ''),
                (f'{sec}.warning', ''),
                (f'{sec}.typical', ''),
            ]:
                if key_id not in zh_keys:
                    new_zh.append((key_id, val))
                    zh_keys.add(key_id)

    append_i18n(EN, new_en)
    append_i18n(ZH, new_zh)
    print(f'\nCreated {created} YAML files')
    print(f'New en.yaml entries: {len(new_en)}')
    print(f'New zh-TW.yaml entries: {len(new_zh)}')


if __name__ == '__main__':
    main()
