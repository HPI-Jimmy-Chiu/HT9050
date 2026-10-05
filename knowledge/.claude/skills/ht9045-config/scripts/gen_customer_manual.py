"""
gen_customer_manual.py
======================
從 YAML 資料源 + i18n 翻譯，產生「給客戶」的多語系說明書（Markdown）。

過濾規則：
  - 僅輸出 audience.customer == true 的區段
  - 截圖檔名統一從 ../screenshots/<group>/<file> 引用（英文版共用）
  - 每國一份輸出檔

執行：
    python gen_customer_manual.py                     # 全部 7 國
    python gen_customer_manual.py --lang en           # 單一語系
    python gen_customer_manual.py --lang zh-TW

產出：
    references/output/customer/<lang>/HT9045_Config_Manual.md
    references/output/customer/<lang>/HT9045_Config_Manual.html  (執行 md_to_html.py 後)
"""

import os
import sys

from yaml_loader import load_yaml

REFS_DIR = r"d:\HT9045\.github\skills\ht9045-config\references"
DATA_DIR = os.path.join(REFS_DIR, "data")
I18N_DIR = os.path.join(REFS_DIR, "i18n")
OUT_DIR = os.path.join(REFS_DIR, "output", "customer")
LETTERS = list("ABCDEFGILMNOP")

LANGS = ['en', 'zh-TW', 'vi', 'ja', 'ko', 'id', 'th']

LANG_LABELS = {
    'en': 'HT9045 Configuration Manual (Customer Edition)',
    'zh-TW': 'HT9045 機台設定手冊（客戶版）',
    'vi': 'Hướng dẫn cấu hình HT9045 (Bản khách hàng)',
    'ja': 'HT9045 設定マニュアル（顧客版）',
    'ko': 'HT9045 설정 매뉴얼 (고객용)',
    'id': 'Panduan Konfigurasi HT9045 (Edisi Pelanggan)',
    'th': 'คู่มือการตั้งค่า HT9045 (ฉบับลูกค้า)',
}

SECTION_HEADINGS = {
    'en': {
        'desc': 'Description',
        'when': 'When to Use',
        'warning': 'Warning',
        'typical': 'Typical Setting',
        'screenshots': 'Screenshots',
        'related': 'Related Sections',
    },
    'zh-TW': {
        'desc': '功能說明',
        'when': '使用時機',
        'warning': '注意事項',
        'typical': '建議設定',
        'screenshots': 'UI 截圖',
        'related': '相關設定',
    },
    'vi': {
        'desc': 'Mô tả', 'when': 'Khi nào sử dụng', 'warning': 'Cảnh báo',
        'typical': 'Cài đặt tiêu biểu', 'screenshots': 'Ảnh chụp màn hình', 'related': 'Mục liên quan',
    },
    'ja': {
        'desc': '機能説明', 'when': '使用シーン', 'warning': '注意事項',
        'typical': '推奨設定', 'screenshots': 'UI スクリーンショット', 'related': '関連設定',
    },
    'ko': {
        'desc': '기능 설명', 'when': '사용 시기', 'warning': '주의 사항',
        'typical': '권장 설정', 'screenshots': 'UI 스크린샷', 'related': '관련 설정',
    },
    'id': {
        'desc': 'Deskripsi', 'when': 'Kapan digunakan', 'warning': 'Peringatan',
        'typical': 'Pengaturan tipikal', 'screenshots': 'Tangkapan layar', 'related': 'Bagian terkait',
    },
    'th': {
        'desc': 'คำอธิบาย', 'when': 'เมื่อใดควรใช้', 'warning': 'คำเตือน',
        'typical': 'การตั้งค่าทั่วไป', 'screenshots': 'ภาพหน้าจอ', 'related': 'หัวข้อที่เกี่ยวข้อง',
    },
}


def _flat_load(path):
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


def load_i18n(lang):
    en_dict = _flat_load(os.path.join(I18N_DIR, "en.yaml"))
    if lang == 'en':
        return en_dict
    lang_dict = _flat_load(os.path.join(I18N_DIR, f"{lang}.yaml"))
    out = dict(en_dict)
    for k, v in lang_dict.items():
        if v not in (None, '',):
            out[k] = v
    return out


def t(i18n, key, default=''):
    v = i18n.get(key, '')
    return v if v else default


def collect_customer_sections():
    """Walk all YAML; return list of dicts where audience.customer is true, sorted."""
    out = []
    for letter in LETTERS:
        folder = os.path.join(DATA_DIR, letter)
        if not os.path.isdir(folder):
            continue
        for fn in sorted(os.listdir(folder)):
            if not fn.endswith('.yaml'):
                continue
            data = load_yaml(os.path.join(folder, fn))
            if not data:
                continue
            aud = data.get('audience') or {}
            if aud.get('customer') is True:
                out.append(data)
    out.sort(key=lambda d: d.get('section', ''))
    return out


def render_section(sd, i18n, headings, level=2):
    sec = sd['section']
    cap = t(i18n, sd.get('caption_id', ''), sec)
    h = '#' * level
    out = []
    out.append(f"{h} {sec} — {cap}")
    out.append("")

    desc = t(i18n, sd.get('desc_id', ''), '')
    if not desc:
        # Fallback: use the caption text so section is never empty.
        desc = cap
    if desc:
        out.append(f"**{headings['desc']}**")
        out.append("")
        out.append(desc)
        out.append("")

    when = t(i18n, sd.get('when_to_use_id', ''), '')
    if when:
        out.append(f"**{headings['when']}**")
        out.append("")
        out.append(when)
        out.append("")

    warning = t(i18n, sd.get('warning_id', ''), '')
    if warning:
        out.append(f"> ⚠️ **{headings['warning']}**: {warning}")
        out.append("")

    typical = t(i18n, sd.get('typical_value_id', ''), '')
    if typical:
        out.append(f"**{headings['typical']}**: {typical}")
        out.append("")

    shots = sd.get('screenshots') or []
    if shots:
        out.append(f"**{headings['screenshots']}**")
        out.append("")
        group = sd.get('group', '')
        for s in shots:
            if not isinstance(s, dict):
                continue
            fn = s.get('file', '')
            cap_id = s.get('caption_id', '')
            alt = t(i18n, cap_id, '') or cap or fn
            out.append(f"![{alt}](../../screenshots/{group}/{fn})")
            out.append("")
            out.append(f"_{alt}_")
            out.append("")

    related = sd.get('related_sections') or []
    if related:
        out.append(f"**{headings['related']}**: " + ", ".join(f"`{r}`" for r in related))
        out.append("")

    out.append("---")
    out.append("")
    return '\n'.join(out)


def render_manual(lang, sections, i18n):
    headings = SECTION_HEADINGS.get(lang, SECTION_HEADINGS['en'])
    title = LANG_LABELS.get(lang, LANG_LABELS['en'])
    out = []
    out.append(f"# {title}")
    out.append("")
    out.append(f"> Language: **{lang}**  •  Sections: **{len(sections)}**")
    out.append("")
    out.append("---")
    out.append("")
    for sd in sections:
        out.append(render_section(sd, i18n, headings))
    return '\n'.join(out)


def main():
    target_langs = LANGS
    if '--lang' in sys.argv:
        target_langs = [sys.argv[sys.argv.index('--lang') + 1]]

    print("=" * 60)
    print("Generate customer manual (multi-language)")
    print("=" * 60)

    sections = collect_customer_sections()
    print(f"  Customer sections: {len(sections)}")
    for s in sections:
        print(f"    - {s['section']}")

    if not sections:
        print("  No sections marked audience.customer=true. Skip.")
        return

    for lang in target_langs:
        i18n = load_i18n(lang)
        md = render_manual(lang, sections, i18n)
        out_dir = os.path.join(OUT_DIR, lang)
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, "HT9045_Config_Manual.md")
        with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(md)
        print(f"  [{lang}] wrote {out_path}  ({os.path.getsize(out_path):,} bytes)")

    print("\nNext: convert to HTML with 鴻勁紅 template:")
    print('  python D:\\HT9045\\.claude\\skills\\make-report-skill\\scripts\\md_to_html.py "<md_path>" --template red')


if __name__ == '__main__':
    main()
