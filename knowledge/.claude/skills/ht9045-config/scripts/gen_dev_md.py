"""
gen_dev_md.py
=============
從 YAML 資料源 + i18n 翻譯，重新生成 13 份 config-fields-X.md（開發者用）。

輸出至：references/output/developer/config-fields-X.md

執行：
    python gen_dev_md.py [--lang en]    # 預設 en
"""

import os
import sys
import re

from yaml_loader import load_yaml

REFS_DIR = r"d:\HT9045\.github\skills\ht9045-config\references"
DATA_DIR = os.path.join(REFS_DIR, "data")
I18N_DIR = os.path.join(REFS_DIR, "i18n")
OUT_DIR = os.path.join(REFS_DIR, "output", "developer")
LETTERS = list("ABCDEFGILMNOP")
VERSION = "V3.33.903.0_20260417"

GROUPS = {
    'A': ('Function', '自動化 / 流程控制'),
    'B': ('Report', '報表 / 紀錄'),
    'C': ('Hardware', '硬體選配'),
    'D': ('Index', 'Index / 下壓設定'),
    'E': ('In/Out Arm', '進出手臂設定'),
    'F': ('Shuttle', 'Shuttle 梭式機構'),
    'G': ('Visible', 'UI 顯示控制'),
    'I': ('Tester', '測試介面 / 良率'),
    'L': ('Temperature', '溫度控制'),
    'M': ('Monitor', 'Monitor 強制模式'),
    'N': ('Network', '網路 / 上傳'),
    'O': ('Count', 'Log / 計數 / 統計'),
    'P': ('Tray', 'Tray / 料流'),
}


def section_sort_key(sec):
    m = re.match(r'^([A-Z])(\d+)(?:-(\d+))?(?:-(\d+))?$', sec)
    if m:
        return (m.group(1), int(m.group(2)),
                int(m.group(3)) if m.group(3) else 0,
                int(m.group(4)) if m.group(4) else 0)
    return (sec, 0, 0, 0)


def load_i18n(lang):
    """Load <lang>.yaml as flat dict; fallback to en for missing values."""
    path = os.path.join(I18N_DIR, f"{lang}.yaml")
    en_path = os.path.join(I18N_DIR, "en.yaml")
    en_dict = _flat_load(en_path)
    if lang == 'en':
        return en_dict
    lang_dict = _flat_load(path)
    # Merge with fallback
    out = dict(en_dict)
    for k, v in lang_dict.items():
        if v not in (None, '', '""'):
            out[k] = v
    return out


def _flat_load(path):
    """Flat key-value YAML loader (one line per key)."""
    out = {}
    if not os.path.exists(path):
        return out
    with open(path, encoding='utf-8') as f:
        for line in f:
            ln = line.rstrip('\n')
            if not ln or ln.lstrip().startswith('#'):
                continue
            if ':' not in ln:
                continue
            key, _, val = ln.partition(':')
            key = key.strip()
            val = val.strip()
            if val.startswith('"') and val.endswith('"'):
                val = val[1:-1].replace('\\"', '"').replace('\\\\', '\\')
            out[key] = val
    return out


def t(i18n, key, default=''):
    v = i18n.get(key, '')
    return v if v else default


def load_sections_for(letter):
    """Load all YAML files under data/<letter>/ → sorted list of section dicts."""
    folder = os.path.join(DATA_DIR, letter)
    if not os.path.isdir(folder):
        return []
    entries = []
    for fn in os.listdir(folder):
        if not fn.endswith('.yaml'):
            continue
        path = os.path.join(folder, fn)
        data = load_yaml(path)
        if data and data.get('section'):
            entries.append(data)
    entries.sort(key=lambda d: section_sort_key(d['section']))
    return entries


def render_letter_md(letter, sections, i18n):
    name_en, name_zh = GROUPS[letter]
    out = []
    out.append(f"# HT9045_CONFIG 欄位速查：群組 [{letter}] {name_en}（{name_zh}）")
    out.append("")
    out.append(f"> 來源：YAML 資料源 (`references/data/{letter}/`)")
    out.append(f"> 多語：i18n/")
    out.append(f"> 版本：{VERSION}")
    out.append("")
    out.append("> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString")
    out.append("")
    out.append("---")
    out.append("")

    # Count rows
    total_vars = sum(len(s.get('variables') or []) for s in sections)
    out.append("## 已綁定 UI 元件的欄位")
    out.append("")
    out.append(f"共 {total_vars} 個欄位，分屬 {len(sections)} 個區段。")
    out.append("")
    out.append("| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |")
    out.append("|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|")

    for sd in sections:
        sec = sd['section']
        cap = t(i18n, sd.get('caption_id', ''), sd.get('caption_id', ''))
        fdesc = cap   # by default same
        # Try to get an explicit desc fallback
        desc_short = t(i18n, sd.get('desc_id', ''), '')
        if desc_short and len(desc_short) < 120:
            fdesc = desc_short
        ecid = sd.get('ecid', '') or ''
        ectyp = sd.get('ec_type', '') or ''

        for v in (sd.get('variables') or []):
            ui_comp = v.get('component') or ''
            ui_cell = f"`{ui_comp}`" if ui_comp else '—'
            ini_k = v.get('ini_key') or ''
            ini_cell = f"`{ini_k}`" if ini_k else '—'
            comment = (v.get('code_comment') or '').replace('|', '&#124;').replace('\n', ' ').strip()
            cap_cell = (cap or '').replace('|', '&#124;')
            fdesc_cell = (fdesc or '').replace('|', '&#124;')
            out.append(
                f"| {sec} | {cap_cell} | {ui_cell} | `{v['name']}` | {v['type']} | "
                f"{ini_cell} | {ecid} | {ectyp} | {fdesc_cell} | {comment} |"
            )

    out.append("")
    out.append("---")
    out.append("")
    out.append("## 未綁定 UI 元件的欄位")
    out.append("")
    out.append(f"_目前所有 `{letter}##` 開頭的欄位均已在主表中列出（透過 YAML 資料源）。_")
    out.append("")

    return '\n'.join(out)


def main():
    lang = 'en'
    if '--lang' in sys.argv:
        lang = sys.argv[sys.argv.index('--lang') + 1]

    print("=" * 60)
    print(f"Generate developer md (lang={lang})")
    print("=" * 60)

    os.makedirs(OUT_DIR, exist_ok=True)

    i18n = load_i18n(lang)
    print(f"  i18n loaded: {len(i18n)} keys")

    for letter in LETTERS:
        sections = load_sections_for(letter)
        if not sections:
            print(f"  [{letter}] no YAML found - skip")
            continue
        md = render_letter_md(letter, sections, i18n)
        out_path = os.path.join(OUT_DIR, f"config-fields-{letter}.md")
        with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(md)
        size = os.path.getsize(out_path)
        print(f"  config-fields-{letter}.md  ({size:,} bytes, {len(sections)} sections)")

    print("\nDone.")


if __name__ == '__main__':
    main()
