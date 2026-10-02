# Config YAML/i18n -> JSON/Config-help.json (HTML 模擬端唯一說明資料來源)
import json
import os
import sys
from datetime import datetime

ROOT = os.path.join(os.path.dirname(os.path.dirname(__file__)), "references")
OUT = r"D:\HT9045\JSON\Config-help.json"
sys.path.insert(0, os.path.dirname(__file__))
from yaml_loader import load_yaml

LANGS = ("en", "zh-TW", "vi", "ja", "ko", "id", "th")


def translations():
    result = {}
    for lang in LANGS:
        result[lang] = load_yaml(os.path.join(ROOT, "i18n", lang + ".yaml")) or {}
    return result


def text(table, lang, key):
    return table.get(lang, {}).get(key) or table["en"].get(key, "")


def main():
    table = translations()
    sections = []
    by_component = {}
    data_root = os.path.join(ROOT, "data")
    for group in sorted(os.listdir(data_root)):
        group_path = os.path.join(data_root, group)
        if not os.path.isdir(group_path):
            continue
        for name in sorted(os.listdir(group_path)):
            if not name.endswith(".yaml"):
                continue
            item = load_yaml(os.path.join(group_path, name)) or {}
            section = item.get("section")
            if not section:
                continue
            values = {}
            for lang in LANGS:
                values[lang] = {
                    "caption": text(table, lang, item.get("caption_id", "")),
                    "description": text(table, lang, item.get("desc_id", "")),
                    "whenToUse": text(table, lang, item.get("when_to_use_id", "")),
                    "warning": text(table, lang, item.get("warning_id", "")),
                    "typicalValue": text(table, lang, item.get("typical_value_id", "")),
                }
            entry = {
                "section": section, "group": item.get("group", group),
                "ui": item.get("ui", []), "variables": item.get("variables", []),
                "ecid": item.get("ecid"), "ecType": item.get("ec_type"),
                "customerCodes": item.get("customer_codes", []), "translations": values,
            }
            sections.append(entry)
            for ui in entry["ui"]:
                component = ui.get("component") if isinstance(ui, dict) else None
                if component:
                    by_component[component] = section
    doc = {
        "schemaVersion": "1.0.0",
        "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
        "source": {"toolchain": "BCB6", "data": "ht9045-config/references/data/*.yaml", "translations": "ht9045-config/references/i18n/*.yaml"},
        "note": "HTML 端只讀本 JSON；修改說明或翻譯請編輯 YAML/i18n 後重生。",
        "languages": list(LANGS), "sections": sections, "byComponent": by_component,
    }
    with open(OUT, "w", encoding="utf-8") as stream:
        json.dump(doc, stream, ensure_ascii=False, indent=1)
        stream.write("\n")
    print("ok:", OUT, "sections", len(sections), "components", len(by_component))


if __name__ == "__main__":
    main()