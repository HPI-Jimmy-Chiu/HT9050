"""
patch_descriptions_from_ini.py
==============================
讀 Description.ini，將 English/Chinese Title/Description 補回對應 i18n YAML：
  en.yaml   <-  English_Title  -> <SEC>.caption  (空值才補)
                English_Description -> <SEC>.desc (空值才補)
  zh-TW.yaml <- Chinese_Title  -> <SEC>.caption
                Chinese_Description -> <SEC>.desc

只在現有 key 為空字串時補；不覆寫既有翻譯。
"""

import os
import re

REFS = r'd:\HT9045\.github\skills\ht9045-config\references'
INI = os.path.join(REFS, 'Description.ini')
EN = os.path.join(REFS, 'i18n', 'en.yaml')
ZH = os.path.join(REFS, 'i18n', 'zh-TW.yaml')


def parse_ini():
    """Return dict: {section: {field: value}}.  Multi-line values joined."""
    out = {}
    cur = None
    cur_key = None
    if not os.path.exists(INI):
        return out
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
                # continuation
                out[cur][cur_key] = (out[cur][cur_key] + ' ' + stripped.strip()).strip()
    return out


def parse_yaml_flat(path):
    """Return list of (key, value, raw_line). Skip comments / blanks unchanged."""
    items = []
    with open(path, encoding='utf-8') as f:
        for raw in f:
            line = raw.rstrip('\n')
            stripped = line.strip()
            if not stripped or stripped.startswith('#'):
                items.append((None, None, line))
                continue
            if ':' in line:
                key, _, val = line.partition(':')
                k = key.strip()
                v = val.strip()
                if v.startswith('"') and v.endswith('"') and len(v) >= 2:
                    inner = v[1:-1].replace('\\"', '"').replace('\\\\', '\\')
                else:
                    inner = v
                items.append((k, inner, line))
            else:
                items.append((None, None, line))
    return items


def write_yaml_flat(path, items):
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        for k, v, raw in items:
            if k is None:
                f.write(raw + '\n')
            else:
                esc = (v or '').replace('\\', '\\\\').replace('"', '\\"')
                f.write(f'{k}: "{esc}"\n')


def patch(path, ini_data, title_field, desc_field):
    items = parse_yaml_flat(path)
    # First pass: build map of (section -> caption value) so we can detect
    # desc==caption duplicates that came from auto-fallback.
    caption_map = {}
    for k, v, _ in items:
        if k and k.endswith('.caption'):
            sec = k[:-len('.caption')]
            caption_map[sec] = (v or '').strip()
    filled_cap = 0
    filled_desc = 0
    new_items = []
    for k, v, raw in items:
        if k is None:
            new_items.append((k, v, raw))
            continue
        if k.endswith('.caption') or k.endswith('.desc'):
            sec = k.rsplit('.', 1)[0]
            ini_sec = ini_data.get(sec)
            if ini_sec:
                cur = (v or '').strip()
                if k.endswith('.caption'):
                    nv = ini_sec.get(title_field, '').strip()
                    if nv and not cur:
                        new_items.append((k, nv, raw))
                        filled_cap += 1
                        continue
                else:
                    nv = ini_sec.get(desc_field, '').strip()
                    cap_text = caption_map.get(sec, '')
                    # Overwrite when empty OR when current desc is just a copy
                    # of caption (auto-fallback) and INI provides a richer text.
                    if nv and (not cur or (cur == cap_text and nv != cur)):
                        new_items.append((k, nv, raw))
                        filled_desc += 1
                        continue
        new_items.append((k, v, raw))
    write_yaml_flat(path, new_items)
    return filled_cap, filled_desc


def main():
    ini = parse_ini()
    print(f'INI sections: {len(ini)}')
    c, d = patch(EN, ini, 'English_Title', 'English_Description')
    print(f'en.yaml  : filled {c} captions, {d} descriptions')
    c, d = patch(ZH, ini, 'Chinese_Title', 'Chinese_Description')
    print(f'zh-TW.yaml: filled {c} captions, {d} descriptions')


if __name__ == '__main__':
    main()
