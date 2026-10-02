"""
gen_full_manual.py
==================
從 YAML 資料源 + i18n，產生「整份」（全 sections）多語系手冊。
每國 × 每群（A..P）一份 .md，方便閱讀與後續 HTML 轉換。

執行：
    python gen_full_manual.py                 # 全部 7 國 × 13 群
    python gen_full_manual.py --lang en
    python gen_full_manual.py --group A
"""

import os
import sys

from yaml_loader import load_yaml
from gen_customer_manual import (
    LANGS, LANG_LABELS, SECTION_HEADINGS, DATA_DIR, REFS_DIR,
    load_i18n, render_section, t,
)

OUT_DIR = os.path.join(REFS_DIR, "output", "full")
LETTERS = list("ABCDEFGILMNOP")

GROUP_TITLES = {
    'en': {
        'A': 'A — Function (Test Mode / Bin / Retest)',
        'B': 'B — Loader',
        'C': 'C — Unloader',
        'D': 'D — Index / Test Head',
        'E': 'E — Shuttle',
        'F': 'F — InArm / OutArm',
        'G': 'G — Tray Handling',
        'I': 'I — Inspection / Vision',
        'L': 'L — Temperature / ATC',
        'M': 'M — Motor / Servo',
        'N': 'N — Communication (SECS/GEM, FTP, Network)',
        'O': 'O — Operation / Statistics',
        'P': 'P — Peripheral / Misc',
    },
    'zh-TW': {
        'A': 'A — 功能設定（測試模式 / Bin / Retest）',
        'B': 'B — Loader',
        'C': 'C — Unloader',
        'D': 'D — Index / Test Head',
        'E': 'E — Shuttle',
        'F': 'F — InArm / OutArm',
        'G': 'G — Tray 處理',
        'I': 'I — 檢驗 / Vision',
        'L': 'L — 溫度 / ATC',
        'M': 'M — 馬達 / 伺服',
        'N': 'N — 通訊（SECS/GEM, FTP, Network）',
        'O': 'O — 操作 / 統計',
        'P': 'P — 周邊 / 其他',
    },
}


def collect_group_sections(letter):
    folder = os.path.join(DATA_DIR, letter)
    if not os.path.isdir(folder):
        return []
    out = []
    for fn in sorted(os.listdir(folder)):
        if not fn.endswith('.yaml'):
            continue
        data = load_yaml(os.path.join(folder, fn))
        if data:
            out.append(data)
    out.sort(key=lambda d: d.get('section', ''))
    return out


def _split_base(section):
    """N07-1 -> ('N07', '1'); A01 -> ('A01', '')."""
    if '-' in section:
        base, _, suf = section.partition('-')
        return base, suf
    return section, ''


def _suffix_key(suf):
    """Natural sort: '1' < '2' < '10'; mixed '01' equals same numeric weight."""
    if suf == '':
        return (0, 0, '')
    # Strip leading zeros for numeric compare
    try:
        return (1, int(suf), suf)
    except ValueError:
        return (2, 0, suf)


def group_by_base(sections):
    """Return list of (base, parent_or_None, children_sorted)."""
    buckets = {}
    for sd in sections:
        base, _ = _split_base(sd['section'])
        buckets.setdefault(base, []).append(sd)
    out = []
    for base in sorted(buckets.keys()):
        items = buckets[base]
        parent = None
        children = []
        for sd in items:
            _, suf = _split_base(sd['section'])
            if suf == '':
                parent = sd
            else:
                children.append(sd)
        children.sort(key=lambda d: _suffix_key(_split_base(d['section'])[1]))
        out.append((base, parent, children))
    return out


def render_group_manual(lang, letter, sections, i18n):
    headings = SECTION_HEADINGS.get(lang, SECTION_HEADINGS['en'])
    titles = GROUP_TITLES.get(lang, GROUP_TITLES['en'])
    main_title = LANG_LABELS.get(lang, LANG_LABELS['en'])
    group_title = titles.get(letter, f"{letter} — Group {letter}")
    out = []
    out.append(f"# {main_title}")
    out.append("")
    out.append(f"## {group_title}")
    out.append("")
    out.append(f"> Language: **{lang}**  •  Group: **{letter}**  •  Sections: **{len(sections)}**")
    out.append("")
    out.append("---")
    out.append("")
    for base, parent, children in group_by_base(sections):
        if parent is not None:
            out.append(render_section(parent, i18n, headings, level=2))
        elif children:
            # Synthesize parent header from first child
            first = children[0]
            cap = t(i18n, first.get('caption_id', ''), base)
            # Strip trailing "-1" descriptor from caption if present
            out.append(f"## {base}")
            out.append("")
            out.append("---")
            out.append("")
        for child in children:
            out.append(render_section(child, i18n, headings, level=3))
    return '\n'.join(out)


def main():
    target_langs = LANGS
    target_groups = LETTERS
    args = sys.argv[1:]
    if '--lang' in args:
        target_langs = [args[args.index('--lang') + 1]]
    if '--group' in args:
        target_groups = [args[args.index('--group') + 1]]

    print("=" * 60)
    print("Generate FULL manual (multi-language × group)")
    print("=" * 60)

    # Pre-collect to print overview
    group_data = {}
    for letter in target_groups:
        group_data[letter] = collect_group_sections(letter)
        print(f"  Group {letter}: {len(group_data[letter])} sections")

    for lang in target_langs:
        i18n = load_i18n(lang)
        for letter in target_groups:
            secs = group_data[letter]
            if not secs:
                continue
            md = render_group_manual(lang, letter, secs, i18n)
            out_dir = os.path.join(OUT_DIR, lang)
            os.makedirs(out_dir, exist_ok=True)
            out_path = os.path.join(out_dir, f"HT9045_Config_Manual_{letter}.md")
            with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
                f.write(md)
        print(f"  [{lang}] wrote {len(target_groups)} group files -> {os.path.join(OUT_DIR, lang)}")

    print("\nNext: convert to HTML with 鴻勁紅 template (per file).")


if __name__ == '__main__':
    main()
