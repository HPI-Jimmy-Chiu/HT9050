"""Generate Setup/Recipe JSON for the HT9045 HTML simulator.

BCB6 sources are read as CP950. JSON outputs are UTF-8 and are the only
Setup data source consumed by the HTML simulator.
"""
import json
import os
import re
from datetime import datetime


ROOT = r"D:\HT9045"
DATA_ROOT = os.path.join(ROOT, "IniData", "Data")
OUTPUT_ROOT = os.path.join(ROOT, "JSON")
SETUP_INFO = os.path.join(ROOT, "SetUp.inf")
GENERAL_INI = os.path.join(ROOT, "system", "Gerneral.ini")

DOCUMENTS = [
    ("testMode", "TestMode.Data", "cprod.cpp ReadTestMode()/SaveTestMode()"),
    ("hotPlate", "HotPlate.Data", "cHotPlate.cpp TfHotPlate::ReadFile()/SaveFile()"),
    ("handlerCondition", "HandlerCondition.Data", "uLotInfo.cpp ReadIniData()/WriteIniData()"),
    ("contact", "Contact.Data", "cContact.cpp TfContact::ReadFile()/SaveFile()"),
    ("temperature", "Temperature.Data", "uTemp_Set.cpp ReadTempFile()/WriteTempFile()"),
    ("armCondition", "ArmCondition.Data", "cSpeed.cpp ReadArmCondition()/WriteArmCondition()"),
    ("tray", "Tray.Data", "cTrayForm.cpp HTEditList ReadEditTextFromFile()/SaveEditTextToFile()"),
    ("tester", "Tester.Data", "uLotInfo.cpp ReadIniData()/WriteIniData()"),
    ("rotate", "Rotate.Data", "cRotate.cpp ReadRotateFile()/WriteRotateFile()"),
    ("udUld", "UdUld.Data", "cLd_ULd.cpp HTEditList ReadEditTextFromFile()/WriteEditTextToFile()"),
]

BIN_VARIANTS = [
    ("ft", "Binasgn.Data", "FT", 1),
    ("offline", "BinasgnOff-Line.Data", "OffLine", 2),
    ("offlineCopy", "BinasgnOff.Data", "OffLine copy", None),
    ("art", "Binasgn_ART.Data", "ART", [3, 4]),
    ("mrtFt", "Binasgn_MRT.Data", "MRT FT", 6),
    ("mrtRt", "Binasgn_MRT_RT.Data", "MRT RT", 5),
    ("offlineArt", "BinasgnOff_ART.Data", "OffLine + ART", None),
]

REQUIRED_FILES = [item[1] for item in DOCUMENTS]
REQUIRED_FILES.insert(5, "Binasgn.Data")


def read_cp950(path):
    with open(path, "r", encoding="cp950", errors="replace") as source:
        return source.read()


def typed(raw):
    text = raw.strip()
    if re.fullmatch(r"[-+]?\d+", text):
        return int(text), "int"
    if re.fullmatch(r"[-+]?(?:\d+\.\d*|\d*\.\d+)(?:e[-+]?\d+)?", text, re.I):
        return float(text), "float"
    if text.lower() in ("true", "false"):
        return text.lower() == "true", "bool"
    if "," in text:
        parts = [part.strip() for part in text.split(",")]
        if parts and all(re.fullmatch(r"[-+]?\d+", part) for part in parts):
            return [int(part) for part in parts], "int[]"
        if parts and all(re.fullmatch(r"[-+]?(?:\d+\.\d*|\d*\.\d+)", part) for part in parts):
            return [float(part) for part in parts], "float[]"
    return text, "string"


def parse_section_file(path, reader):
    section_order = []
    sections = {}
    duplicates = []
    ignored_lines = []
    current = None

    for line_number, line in enumerate(read_cp950(path).splitlines(), 1):
        stripped = line.strip()
        if not stripped or stripped.startswith(";") or stripped.startswith("#"):
            continue
        match = re.match(r"^\[(.+?)\]\s*$", stripped)
        if match:
            current = match.group(1)
            if current not in sections:
                sections[current] = {}
                section_order.append(current)
            continue
        if "=" not in stripped or current is None:
            ignored_lines.append({"line": line_number, "raw": stripped})
            continue
        key, raw_value = stripped.split("=", 1)
        key = key.strip()
        value, value_type = typed(raw_value)
        if key in sections[current]:
            duplicates.append({"section": current, "key": key, "line": line_number})
        sections[current][key] = {"value": value, "type": value_type, "raw": raw_value.strip()}

    return {
        "source": {
            "toolchain": "BCB6",
            "file": path,
            "encoding": "cp950",
            "reader": reader,
        },
        "summary": {
            "sections": len(sections),
            "keys": sum(len(section) for section in sections.values()),
            "duplicates": len(duplicates),
            "ignoredLines": len(ignored_lines),
        },
        "sectionOrder": section_order,
        "sections": sections,
        "duplicates": duplicates,
        "ignoredLines": ignored_lines,
    }


def find_value(document, section, key, default=None):
    entry = document.get("sections", {}).get(section, {}).get(key)
    return entry.get("value", default) if entry else default


def read_current_recipe_name():
    lines = read_cp950(SETUP_INFO).splitlines()
    return lines[0].strip() if lines else ""


def read_name_check_list():
    general = parse_section_file(GENERAL_INI, "main.cpp CheckAndReadIniDataGeneral()")
    list_name = find_value(general, "System", "SetupFileCheckList", "")
    list_path = os.path.join(ROOT, "system", str(list_name)) if list_name else ""
    entries = []
    if list_path and os.path.isfile(list_path):
        entries = [line.strip() for line in read_cp950(list_path).splitlines() if line.strip()]
    return list_name, list_path, entries


def generate():
    generated_at = datetime.now().astimezone().isoformat(timespec="seconds")
    current = read_current_recipe_name()
    recipe_path = os.path.join(DATA_ROOT, current)
    if not current or not os.path.isdir(recipe_path):
        raise RuntimeError("SetUp.inf current recipe directory does not exist: " + recipe_path)

    recipe_list = sorted(
        entry.name for entry in os.scandir(DATA_ROOT)
        if entry.is_dir() and not entry.name.startswith(".")
    )
    present = sorted(entry.name for entry in os.scandir(recipe_path) if entry.is_file())
    missing = [name for name in REQUIRED_FILES if name not in present]
    list_name, list_path, name_check_entries = read_name_check_list()

    index = {
        "schemaVersion": "1.0.0",
        "generatedAt": generated_at,
        "source": {
            "toolchain": "BCB6",
            "files": [SETUP_INFO, DATA_ROOT, GENERAL_INI, list_path],
            "encoding": "cp950",
            "readers": ["common.cpp GetLastOpenFN()", "main.cpp FindFirst/FindNext", "HS_Function.cpp CheckSetupNamelist_Hisi()"],
        },
        "current": current,
        "list": recipe_list,
        "nameCheckList": {
            "configuredFile": list_name,
            "entries": name_check_entries,
            "currentAllowed": current.upper() in {name.upper() for name in name_check_entries} if name_check_entries else None,
        },
        "integrity": {
            "requiredFiles": REQUIRED_FILES,
            "presentFiles": present,
            "missingFiles": missing,
            "complete": not missing,
        },
        "currentData": "Setup-current.json",
        "summary": {
            "recipes": len(recipe_list),
            "requiredFiles": len(REQUIRED_FILES),
            "missingRequiredFiles": len(missing),
        },
    }

    documents = {}
    for document_id, file_name, reader in DOCUMENTS:
        path = os.path.join(recipe_path, file_name)
        documents[document_id] = parse_section_file(path, reader) if os.path.isfile(path) else {"missing": True, "file": path}

    bin_variants = {}
    for variant_id, file_name, mode, bin_select_index in BIN_VARIANTS:
        path = os.path.join(recipe_path, file_name)
        item = {
            "fileName": file_name,
            "mode": mode,
            "binSelectIndex": bin_select_index,
            "available": os.path.isfile(path),
        }
        if item["available"]:
            item["document"] = parse_section_file(path, "cBinSel.cpp TfBinSel::ReadFile()/SaveFile()")
        bin_variants[variant_id] = item

    override_path = os.path.join(recipe_path, "configByRecipe.ini")
    override = parse_section_file(override_path, "cprod.cpp LoadConfigByRecipe()") if os.path.isfile(override_path) else {"missing": True, "file": override_path}
    recipe = {
        "schemaVersion": "1.0.0",
        "generatedAt": generated_at,
        "source": {
            "toolchain": "BCB6",
            "directory": recipe_path,
            "encoding": "cp950",
        },
        "recipeName": current,
        "note": "HTML reads this UTF-8 JSON only; it must not parse the original .Data/.ini files.",
        "documents": documents,
        "binasgn": {"variants": bin_variants},
        "configByRecipe": override,
        "quick": {
            "testerConnection": find_value(documents.get("testMode", {}), "TestMode", "Tester Connection"),
            "runningMode": find_value(documents.get("testMode", {}), "TestMode", "Running Mode"),
            "testMode": find_value(documents.get("handlerCondition", {}), "Configuration", "Test Mode"),
            "temperatureMode": find_value(documents.get("temperature", {}), "Mode", "Mode"),
            "temperature": find_value(documents.get("temperature", {}), "Mode", "Temperature"),
            "soakTime": find_value(documents.get("temperature", {}), "Time", "Soak"),
            "rotateActive": find_value(documents.get("rotate", {}), "SETTING", "ActiveRotate"),
            "trayTypes": [
                {"section": section, "name": find_value(documents.get("tray", {}), section, "Name")}
                for section in documents.get("tray", {}).get("sectionOrder", []) if re.fullmatch(r"Type\d+", section)
            ],
        },
        "summary": {
            "coreDocuments": len(DOCUMENTS) + 1,
            "availableBinVariants": sum(1 for item in bin_variants.values() if item["available"]),
            "missingRequiredFiles": missing,
        },
    }

    os.makedirs(OUTPUT_ROOT, exist_ok=True)
    outputs = {
        "Setup-index.json": index,
        "Setup-current.json": recipe,
    }
    for file_name, data in outputs.items():
        path = os.path.join(OUTPUT_ROOT, file_name)
        with open(path, "w", encoding="utf-8", newline="\n") as target:
            json.dump(data, target, ensure_ascii=False, indent=2)
            target.write("\n")
        print("ok:", path)
    print("recipe:", current, "documents:", len(documents), "bin variants:", recipe["summary"]["availableBinVariants"], "missing:", len(missing))


if __name__ == "__main__":
    generate()