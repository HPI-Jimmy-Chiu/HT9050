# BCB6 header declarations -> JSON/Define-index.json (HTML 搜尋索引，不輸出執行期值)
import json
import os
import re
from datetime import datetime

BASE = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"
OUT = r"D:\HT9045\JSON\Define-index.json"
HEADERS = ("cmydef.h", "Config.h", "cprod.h")


def clean(text):
    return re.sub(r"\s+", " ", text).strip()


def classify(kind, declaration, header=None):
    if kind == "define":
        return "static-constant", "No runtime JSON required; include as a searchable constant catalog."
    if header == "Config.h" or "IniConfig" in declaration or "HT9045_CONFIG" in declaration:
        return "config-value", "Use Config.json; this is a persisted config value or schema field."
    if kind in ("struct", "class", "field"):
        return "schema-only", "Export the field schema only; runtime values require an explicit producer JSON contract."
    if "extern" in declaration:
        return "runtime-candidate", "Do not read from HTML. Add a runtime bridge only when a page needs this live value."
    return "reference-only", "Source declaration catalog only."


def parse_header(name):
    path = os.path.join(BASE, name)
    lines = open(path, encoding="cp950", errors="replace").read().splitlines()
    entries, structures = [], []
    current = None
    pending = None
    brace_depth = 0
    for number, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith("//"):
            continue
        define = re.match(r"#define\s+(\w+)(?:\s+(.*?))?(?:\s*//.*)?$", line)
        if define:
            declaration = "#define " + define.group(1) + (" " + clean(define.group(2) or "") if define.group(2) else "")
            category, reason = classify("define", declaration, name)
            entries.append({"kind": "define", "name": define.group(1), "declaration": declaration, "line": number,
                            "jsonClass": category, "reason": reason})
            continue
        begin = re.match(r"(?:typedef\s+)?(struct|class)\s*(\w+)?", line)
        if begin and "{" not in line:
            pending = (begin.group(1), begin.group(2) or "anonymous")
            continue
        if (begin and "{" in line) or (pending and line.startswith("{")):
            kind, struct_name = pending if pending else (begin.group(1), begin.group(2) or "anonymous")
            pending = None
            current = struct_name
            brace_depth = line.count("{") - line.count("}")
            structures.append({"kind": kind, "name": current, "line": number, "fields": []})
            category, reason = classify(kind, line, name)
            entries.append({"kind": kind, "name": current, "declaration": clean(line), "line": number,
                            "jsonClass": category, "reason": reason})
            continue
        if current:
            brace_depth += line.count("{") - line.count("}")
            field = re.match(r"(?:public:|private:|protected:)?\s*([\w:]+(?:\s*\*+)?)\s+(\w+(?:\s*\[[^]]+\])*)\s*;", line)
            if field:
                declaration = clean(line)
                category, reason = classify("field", declaration, name)
                item = {"kind": "field", "name": field.group(2), "declaration": declaration, "line": number,
                        "owner": current, "jsonClass": category, "reason": reason}
                entries.append(item)
                structures[-1]["fields"].append(item["name"])
            closing = re.match(r"}\s*(\w+)\s*;", line)
            if closing:
                final_name = closing.group(1)
                structures[-1]["name"] = final_name
                for entry in entries:
                    if entry.get("owner") == current:
                        entry["owner"] = final_name
                    if entry["kind"] in ("struct", "class") and entry["name"] == current:
                        entry["name"] = final_name
                        entry["declaration"] = "typedef struct " + final_name
                current = final_name
            if brace_depth <= 0:
                current = None
            continue
        if line.startswith("extern ") and line.endswith(";"):
            declaration = clean(line)
            name_match = re.search(r"(\w+)(?:\s*\[[^]]+\])?\s*;$", declaration)
            category, reason = classify("extern", declaration, name)
            entries.append({"kind": "extern", "name": name_match.group(1) if name_match else "(unparsed)",
                            "declaration": declaration, "line": number, "jsonClass": category, "reason": reason})
    return {"file": name, "entries": entries, "structures": structures,
            "summary": {"entries": len(entries), "defines": sum(x["kind"] == "define" for x in entries),
                        "externs": sum(x["kind"] == "extern" for x in entries), "schemaFields": sum(x["kind"] == "field" for x in entries)}}


def main():
    headers = [parse_header(name) for name in HEADERS]
    entries = [entry for header in headers for entry in header["entries"]]
    doc = {"schemaVersion": "1.0.0", "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
           "source": {"toolchain": "BCB6", "headers": list(HEADERS), "encoding": "cp950"},
           "policy": {"static-constant": "catalog only", "config-value": "Config.json", "schema-only": "schema, not values",
                      "runtime-candidate": "requires explicit runtime JSON producer", "reference-only": "catalog only"},
           "headers": headers,
           "summary": {"entries": len(entries), "defines": sum(x["kind"] == "define" for x in entries),
                       "externs": sum(x["kind"] == "extern" for x in entries), "schemaFields": sum(x["kind"] == "field" for x in entries)}}
    with open(OUT, "w", encoding="utf-8") as stream:
        json.dump(doc, stream, ensure_ascii=False, indent=1)
        stream.write("\n")
    print("ok:", OUT, doc["summary"])


if __name__ == "__main__":
    main()