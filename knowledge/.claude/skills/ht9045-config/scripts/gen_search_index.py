"""
gen_search_index.py
===================
從 YAML 資料 + i18n，產生 search_index.json。
輸出：references/output/search_index.json
格式：[{ sec, group, caps: {lang: text}, descs: {lang: text} }, ...]
"""

import os
import json

from yaml_loader import load_yaml
from gen_customer_manual import load_i18n, t

REFS = r'd:\HT9045\.github\skills\ht9045-config\references'
DATA = os.path.join(REFS, 'data')
OUT  = os.path.join(REFS, 'output', 'search_index.json')
LETTERS = list('ABCDEFGILMNOP')
LANGS = ['en', 'zh-TW', 'vi', 'ja', 'ko', 'id', 'th']


def main():
    # Pre-load all i18n
    i18n_map = {lang: load_i18n(lang) for lang in LANGS}

    index = []
    for letter in LETTERS:
        folder = os.path.join(DATA, letter)
        if not os.path.isdir(folder):
            continue
        for fn in sorted(os.listdir(folder)):
            if not fn.endswith('.yaml'):
                continue
            data = load_yaml(os.path.join(folder, fn))
            if not data:
                continue
            sec = data.get('section', '')
            cap_id = data.get('caption_id', '')
            desc_id = data.get('desc_id', '')
            caps = {}
            descs = {}
            for lang in LANGS:
                i18n = i18n_map[lang]
                caps[lang]  = t(i18n, cap_id,  sec)
                descs[lang] = t(i18n, desc_id, '')
            index.append({
                'sec':   sec,
                'group': letter,
                'caps':  caps,
                'descs': descs,
            })

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(index, f, ensure_ascii=False, separators=(',', ':'))

    print(f'Wrote {len(index)} entries -> {OUT}')


if __name__ == '__main__':
    main()
