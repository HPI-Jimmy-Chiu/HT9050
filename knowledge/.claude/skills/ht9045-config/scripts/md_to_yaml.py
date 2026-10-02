"""
md_to_yaml.py
=============
從 13 份 config-fields-X.md 反向解析，產生 YAML 資料源（per-section）
與初始 i18n 翻譯檔（en, zh-TW）。

輸出：
  references/data/<group>/<section>.yaml
  references/i18n/en.yaml      (從 Function Description 自動填入)
  references/i18n/zh-TW.yaml   (從 Caption / 程式註解 抽取常用字)
  references/i18n/<vi/ja/ko/id/th>.yaml  (空骨架，待翻譯)
"""

import os
import re
import sys
from collections import OrderedDict

REFS_DIR = r"d:\HT9045\.github\skills\ht9045-config\references"
DATA_DIR = os.path.join(REFS_DIR, "data")
I18N_DIR = os.path.join(REFS_DIR, "i18n")
LETTERS = list("ABCDEFGILMNOP")
LANGS = ["en", "zh-TW", "vi", "ja", "ko", "id", "th"]

GROUPS = {
    'A': ('Function', 'Automation / Process Control'),
    'B': ('Report', 'Reports / Records'),
    'C': ('Hardware', 'Hardware Options'),
    'D': ('Index', 'Index / Press-Down'),
    'E': ('In/Out Arm', 'In/Out Arm Settings'),
    'F': ('Shuttle', 'Shuttle Mechanism'),
    'G': ('Visible', 'UI Visibility'),
    'I': ('Tester', 'Tester Interface / Yield'),
    'L': ('Temperature', 'Temperature Control'),
    'M': ('Monitor', 'Monitor Force Mode'),
    'N': ('Network', 'Network / Upload'),
    'O': ('Count', 'Log / Count / Statistics'),
    'P': ('Tray', 'Tray / Material Flow'),
}


# ─────────────────────────────────────────────
# Naive YAML serializer (avoid PyYAML dependency)
# ─────────────────────────────────────────────
def yaml_escape(s):
    if s is None:
        return ''
    s = str(s)
    if not s:
        return '""'
    # Use double-quoted scalar for simplicity / safety
    if any(ch in s for ch in [':', '#', '\n', '"', "'", '[', ']', '{', '}', '&', '*', '!', '|', '>', '%', '@', '`']) \
       or s.strip() != s \
       or s.lower() in ('true', 'false', 'null', 'yes', 'no'):
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
    return s


def yaml_block(s, indent=2):
    """Serialize multi-line string as YAML literal block."""
    lines = s.rstrip('\n').split('\n')
    pad = ' ' * indent
    return '|\n' + '\n'.join(pad + ln for ln in lines)


def write_yaml(data, fp, indent=0):
    """Minimal YAML writer (dict / list / scalar)."""
    pad = ' ' * indent
    if isinstance(data, dict):
        for k, v in data.items():
            if isinstance(v, dict):
                fp.write(f"{pad}{k}:\n")
                write_yaml(v, fp, indent + 2)
            elif isinstance(v, list):
                if not v:
                    fp.write(f"{pad}{k}: []\n")
                else:
                    fp.write(f"{pad}{k}:\n")
                    for item in v:
                        if isinstance(item, dict):
                            fp.write(f"{pad}  -\n")
                            write_yaml(item, fp, indent + 4)
                        else:
                            fp.write(f"{pad}  - {yaml_escape(item)}\n")
            elif isinstance(v, bool):
                fp.write(f"{pad}{k}: {'true' if v else 'false'}\n")
            elif isinstance(v, (int, float)):
                fp.write(f"{pad}{k}: {v}\n")
            elif v is None:
                fp.write(f"{pad}{k}: null\n")
            elif isinstance(v, str) and '\n' in v.strip():
                fp.write(f"{pad}{k}: {yaml_block(v, indent + 2)}\n")
            else:
                fp.write(f"{pad}{k}: {yaml_escape(v)}\n")


# ─────────────────────────────────────────────
# Markdown table parser
# ─────────────────────────────────────────────
BOUND_HDR_PAT = re.compile(r'^\|\s*區段\s*\|\s*Caption\s*\|')
UNBOUND_HDR_PAT = re.compile(r'^\|\s*變數名\s*\|\s*型別\s*\|\s*ECID\s*\|')


def parse_md(path, letter):
    """
    Returns list of section dicts.
    """
    with open(path, encoding='utf-8') as f:
        lines = f.readlines()

    # group rows by section
    sections = OrderedDict()  # sec_code -> {caption, ui[], variables[], ecid?, ec_type?}

    mode = None
    in_table = False
    past_sep = False

    for line in lines:
        s = line.rstrip('\n')

        if '## 已綁定' in s:
            mode = 'bound'
            in_table = False
            past_sep = False
            continue
        if '## 未綁定' in s:
            mode = 'unbound'
            in_table = False
            past_sep = False
            continue
        if s.startswith('## ') and mode:
            mode = None
            continue

        if mode == 'bound':
            if BOUND_HDR_PAT.match(s):
                in_table = True
                past_sep = False
                continue
            if in_table and re.match(r'^\|[\s\-:|]+\|$', s):
                past_sep = True
                continue
            if in_table and past_sep and s.startswith('|') and s.endswith('|'):
                cells = [c.strip() for c in s.split('|')[1:-1]]
                if len(cells) != 10:
                    continue
                sec, cap, ui, var, typ, ini_key, ecid, ectyp, fdesc, comment = cells
                if sec not in sections:
                    sections[sec] = {
                        'section': sec,
                        'group': letter,
                        'caption': cap,
                        'function_desc': fdesc,
                        'ui': [],
                        'variables': [],
                        'ecid': '',
                        'ec_type': '',
                    }
                sd = sections[sec]
                if cap and not sd['caption']:
                    sd['caption'] = cap
                if fdesc and not sd['function_desc']:
                    sd['function_desc'] = fdesc
                if ecid and ecid not in ('', '--'):
                    sd['ecid'] = ecid
                if ectyp and ectyp not in ('', '--'):
                    sd['ec_type'] = ectyp
                # UI component
                ui_clean = ui.strip('`').strip()
                if ui_clean and ui_clean != '—':
                    if not any(u.get('component') == ui_clean for u in sd['ui']):
                        sd['ui'].append({'component': ui_clean})
                # Variable
                var_clean = var.strip('`').strip()
                ini_clean = ini_key.strip('`').strip()
                if ini_clean == '—':
                    ini_clean = ''
                if var_clean and not any(v.get('name') == var_clean for v in sd['variables']):
                    sd['variables'].append({
                        'name': var_clean,
                        'type': typ,
                        'ini_key': ini_clean,
                        'component': ui_clean if ui_clean and ui_clean != '—' else '',
                        'code_comment': comment,
                    })

        elif mode == 'unbound':
            if UNBOUND_HDR_PAT.match(s):
                in_table = True
                past_sep = False
                continue
            if in_table and re.match(r'^\|[\s\-:|]+\|$', s):
                past_sep = True
                continue
            if in_table and past_sep and s.startswith('|') and s.endswith('|'):
                cells = [c.strip() for c in s.split('|')[1:-1]]
                if len(cells) != 5:
                    continue
                var, typ, ecid, fdesc, comment = cells
                var_clean = var.strip('`').strip()
                # Use variable name as section if no group section recoverable
                # We tag these as "_unbound_<var>" so they can be reviewed manually
                key = '_unbound_' + var_clean
                sections[key] = {
                    'section': '',                  # no formal section
                    'group': letter,
                    'caption': '',
                    'function_desc': fdesc,
                    'ui': [],
                    'variables': [{
                        'name': var_clean,
                        'type': typ,
                        'ini_key': '',
                        'component': '',
                        'code_comment': comment,
                    }],
                    'ecid': ecid if ecid not in ('', '--') else '',
                    'ec_type': '',
                    'unbound_only': True,
                }

    return sections


# ─────────────────────────────────────────────
# Section dict → YAML structure
# ─────────────────────────────────────────────
def to_yaml_dict(sec_data):
    sec = sec_data['section']
    if not sec:                            # unbound-only entry
        return None
    base_id = sec
    out = OrderedDict()
    out['section'] = sec
    out['group'] = sec_data['group']
    out['caption_id'] = f"{base_id}.caption"
    out['desc_id'] = f"{base_id}.desc"
    out['when_to_use_id'] = f"{base_id}.when"
    out['warning_id'] = f"{base_id}.warning"
    out['typical_value_id'] = f"{base_id}.typical"

    # UI list
    out['ui'] = []
    for u in sec_data['ui']:
        out['ui'].append(OrderedDict([
            ('component', u['component']),
            ('type', infer_ui_type(u['component'])),
            ('role', 'master'),
        ]))

    # Variables
    out['variables'] = []
    for v in sec_data['variables']:
        var_entry = OrderedDict([
            ('name', v['name']),
            ('type', v['type']),
            ('default', None),
            ('ini_section', sec),
            ('ini_key', v['ini_key']),
            ('component', v['component']),
            ('code_comment', v['code_comment']),
        ])
        out['variables'].append(var_entry)

    # ECID
    if sec_data['ecid']:
        out['ecid'] = sec_data['ecid']
    if sec_data['ec_type']:
        out['ec_type'] = sec_data['ec_type']

    # Audience defaults
    out['audience'] = OrderedDict([
        ('developer', True),
        ('operator', True),
        ('customer', False),  # 預設 false，需手動標記
    ])
    out['customer_codes'] = []
    out['related_sections'] = []
    out['related_functions'] = []
    out['screenshots'] = []

    return out


def infer_ui_type(comp):
    """Guess UI type from component name prefix."""
    c = comp.lower()
    for prefix, t in [
        ('cb', 'TCheckBox'),
        ('chk', 'TCheckBox'),
        ('ch', 'TCheckBox'),
        ('rg', 'TRadioGroup'),
        ('cbb', 'TComboBox'),
        ('cmb', 'TComboBox'),
        ('edt', 'TEdit'),
        ('ed', 'TEdit'),
        ('lab', 'TLabel'),
        ('grp', 'TGroupBox'),
        ('mmo', 'TMemo'),
        ('btn', 'TButton'),
        ('spe', 'TSpinEdit'),
        ('sb',  'TSpeedButton'),
    ]:
        if c.startswith(prefix):
            return t
    return 'Unknown'


# ─────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────
def main():
    print("=" * 60)
    print("md → YAML migration")
    print("=" * 60)

    os.makedirs(DATA_DIR, exist_ok=True)
    os.makedirs(I18N_DIR, exist_ok=True)

    all_i18n_en = OrderedDict()
    all_i18n_zh = OrderedDict()

    section_count = 0
    unbound_count = 0
    skipped_count = 0

    for letter in LETTERS:
        md_path = os.path.join(REFS_DIR, f"config-fields-{letter}.md")
        if not os.path.exists(md_path):
            print(f"  [WARN] missing {md_path}")
            continue

        sections = parse_md(md_path, letter)
        os.makedirs(os.path.join(DATA_DIR, letter), exist_ok=True)

        ltr_secs = 0
        ltr_unbound = 0
        for key, sd in sections.items():
            if sd.get('unbound_only'):
                unbound_count += 1
                ltr_unbound += 1
                continue

            doc = to_yaml_dict(sd)
            if not doc:
                skipped_count += 1
                continue

            # Filename: A01.yaml, A01-1.yaml, N10-3-1.yaml
            sec_safe = sd['section'].replace('/', '_')
            yaml_path = os.path.join(DATA_DIR, letter, f"{sec_safe}.yaml")
            with open(yaml_path, 'w', encoding='utf-8', newline='\n') as f:
                f.write(f"# Auto-generated from config-fields-{letter}.md\n")
                f.write(f"# Section: {sd['section']}\n")
                f.write(f"# Source caption: {sd['caption']}\n\n")
                write_yaml(doc, f)
            section_count += 1
            ltr_secs += 1

            # i18n entries
            base = sd['section']
            cap = sd['caption'].strip()
            fdesc = sd['function_desc'].strip()
            all_i18n_en[f"{base}.caption"] = cap or fdesc
            all_i18n_en[f"{base}.desc"] = fdesc or cap
            all_i18n_en[f"{base}.when"] = ""
            all_i18n_en[f"{base}.warning"] = ""
            all_i18n_en[f"{base}.typical"] = ""

            all_i18n_zh[f"{base}.caption"] = ""    # 待人工/機翻補
            all_i18n_zh[f"{base}.desc"] = ""
            all_i18n_zh[f"{base}.when"] = ""
            all_i18n_zh[f"{base}.warning"] = ""
            all_i18n_zh[f"{base}.typical"] = ""

        print(f"  [{letter}] sections={ltr_secs:3d}, unbound-only(skipped)={ltr_unbound}")

    # Write unbound-only as a single review file
    print(f"\n  Total: sections={section_count}, unbound-only={unbound_count}, skipped={skipped_count}")

    # Write i18n files
    print("\n  Writing i18n files ...")
    write_i18n(os.path.join(I18N_DIR, "en.yaml"), all_i18n_en, "English")
    write_i18n(os.path.join(I18N_DIR, "zh-TW.yaml"), all_i18n_zh, "繁體中文")
    for lang in ["vi", "ja", "ko", "id", "th"]:
        empty = OrderedDict((k, "") for k in all_i18n_en.keys())
        lang_label = {
            "vi": "Tiếng Việt",
            "ja": "日本語",
            "ko": "한국어",
            "id": "Bahasa Indonesia",
            "th": "ภาษาไทย",
        }[lang]
        write_i18n(os.path.join(I18N_DIR, f"{lang}.yaml"), empty, lang_label)

    print("\nDone.")


def write_i18n(path, kv, label):
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(f"# {label} translations for HT9045 Config\n")
        f.write(f"# Format: <section>.<field>: \"translation\"\n")
        f.write(f"# Empty values fall back to en.yaml at render time.\n\n")
        for k, v in kv.items():
            f.write(f"{k}: {yaml_escape(v)}\n")
    print(f"    {os.path.basename(path)} ({len(kv)} keys, {os.path.getsize(path):,} bytes)")


if __name__ == '__main__':
    main()
