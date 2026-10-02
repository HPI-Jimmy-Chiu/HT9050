"""
enhance_all_sections.py
=======================
讓所有 sections 渲染成「Customer 7」的水準：
  1. 若 output/screenshots/<group>/<section>-overview.png 存在，
     且 YAML 內 screenshots 為空，則寫入該截圖。
  2. 在 en.yaml 為截圖補一筆 caption 翻譯（若尚未存在）。

不會：
  - 自動填寫 desc/when/warning/typical 內容（須人工或 LLM 翻譯）
  - 改動 audience 設定（保留 customer 標記原樣）
"""

import os
import re

ROOT = r'd:\HT9045\.github\skills\ht9045-config\references'
DATA = os.path.join(ROOT, 'data')
SHOT_ROOT = os.path.join(ROOT, 'output', 'screenshots')
EN_YAML = os.path.join(ROOT, 'i18n', 'en.yaml')
LETTERS = list('ABCDEFGILMNOP')


def load_en_keys():
    keys = set()
    if not os.path.exists(EN_YAML):
        return keys
    with open(EN_YAML, encoding='utf-8') as f:
        for ln in f:
            if ':' in ln and not ln.lstrip().startswith('#'):
                k = ln.split(':', 1)[0].strip()
                if k:
                    keys.add(k)
    return keys


def append_en_entries(entries):
    """entries: list of (key, value) to append to en.yaml."""
    if not entries:
        return
    with open(EN_YAML, 'a', encoding='utf-8', newline='\n') as f:
        f.write('\n# === Auto-attached screenshot captions ===\n')
        for k, v in entries:
            esc = v.replace('\\', '\\\\').replace('"', '\\"')
            f.write(f'{k}: "{esc}"\n')


def patch_yaml(path, png_filename, shot_caption_id):
    with open(path, encoding='utf-8') as f:
        text = f.read()
    if 'screenshots: []' not in text:
        return False
    block = (
        'screenshots:\n'
        '  -\n'
        f'    file: {png_filename}\n'
        f'    caption_id: {shot_caption_id}\n'
    )
    new_text = text.replace('screenshots: []', block.rstrip('\n'))
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(new_text)
    return True


def main():
    en_keys = load_en_keys()
    new_i18n_entries = []
    patched = 0
    skipped = 0
    for letter in LETTERS:
        d = os.path.join(DATA, letter)
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if not fn.endswith('.yaml'):
                continue
            sec = fn[:-5]
            png = os.path.join(SHOT_ROOT, letter, f'{sec}-overview.png')
            if not os.path.exists(png):
                continue
            yaml_path = os.path.join(d, fn)
            shot_caption_id = f'{sec}.shot.overview'
            ok = patch_yaml(yaml_path, f'{sec}-overview.png', shot_caption_id)
            if ok:
                patched += 1
                if shot_caption_id not in en_keys:
                    # Use section's caption_id text as fallback caption later;
                    # for now write a simple placeholder using section name.
                    new_i18n_entries.append((shot_caption_id, f'{sec} overview'))
                    en_keys.add(shot_caption_id)
            else:
                skipped += 1
    append_en_entries(new_i18n_entries)
    print(f'Patched YAML: {patched}')
    print(f'Skipped (already had screenshot): {skipped}')
    print(f'New en.yaml entries: {len(new_i18n_entries)}')


if __name__ == '__main__':
    main()
