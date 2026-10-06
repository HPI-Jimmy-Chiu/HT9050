#!/usr/bin/env python3
"""唯讀 Git 客戶碼候選定位；詞法搜尋不代表 C++ 語意／建置／機台驗證。"""
import argparse
import bisect
import collections
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

SCOPES = {
    "v912": ("HT9011UC_Code_V3.33.912.0_20260908_Jimmy", "cp950"),
    "v906-cpp": ("HT9011UC_Cpp_V3.33.906.0", "utf-8"),
}
SKIP = re.compile(
    r'R"(?P<delimiter>[^\s()\\]{0,16})\([\s\S]*?\)(?P=delimiter)"'
    r'|//[^\n]*|/\*[\s\S]*?\*/|(?:u8|u|U|L)?"(?:\\.|[^"\\])*"'
    r"|(?:u8|u|U|L)?'(?:\\.|[^'\\])*'"
)
TOKENS = re.compile(r"[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+[uUlL]*|\d+[uUlL]*|::|==|!=|<=|>=|&&|\|\||->|[^\s]")
COMPARATORS = {"==", "!=", "<", ">", "<=", ">="}
CONTROLS = {"if", "for", "while", "switch", "catch", "sizeof", "alignof", "decltype", "noexcept"}


def git(repo, *args):
    return subprocess.check_output(["git", "-C", str(repo), *args])


def mask_literals(source):
    return SKIP.sub(lambda m: re.sub(r"[^\n]", " ", m.group()), source)


def integer(value):
    value = re.sub(r"[uUlL]+$", "", value)
    # C++ literal syntax: 0x is hexadecimal, a leading zero is octal.
    base = 16 if value.lower().startswith("0x") else 8 if len(value) > 1 and value.startswith("0") else 10
    try:
        return int(value, base)
    except ValueError:
        return None  # Do not guess malformed/generated numeric candidates.


def symbols(source):
    result = {}
    for match in re.finditer(r"^\s*#\s*define\s+(CC_\w+)\s+(\w+)", mask_literals(source), re.M):
        name, raw = match.groups()
        record = {"symbol": name, "definition": raw, "value": None}
        if re.fullmatch(r"(?:0[xX][0-9A-Fa-f]+|\d+)[uUlL]*", raw):
            record["value"] = integer(raw)
        result[name] = record
    for _ in range(len(result)):
        changed = False
        for record in result.values():
            target = result.get(record["definition"])
            if record["value"] is None and target and target["value"] is not None:
                record["value"] = target["value"]
                changed = True
        if not changed:
            break
    return result


def lexical_context(source):
    """保留條件式標記；只追蹤字面 0／1，不決定實際 preprocessor 配置。"""
    masked = mask_literals(source)
    frames, offsets, states, rows = [], [], [], []
    offset = 0
    for row in masked.splitlines(keepends=True):
        match = re.match(r"\s*#\s*(\w+)(.*)", row)
        if match:
            op, value = match.groups()
            value = " ".join(value.split())[:160]
            if op in ("if", "ifdef", "ifndef"):
                literal = value if op == "if" and value in ("0", "1") else None
                frames.append({"guard": f"#{op} {value}", "literal": literal, "inactive": literal == "0", "else_seen": False})
            elif op == "else" and frames:
                frame = frames[-1]
                frame["guard"] += " / #else"
                frame["inactive"] = frame["literal"] == "1" if frame["literal"] is not None else False
                frame["else_seen"] = True
            elif op == "elif" and frames:
                frame = frames[-1]
                # Ambiguous chains deliberately become unresolved rather than selected.
                frame["guard"] += f" / #elif {value}"
                frame["inactive"] = frame["literal"] == "1" or value == "0"
                frame["literal"] = None
            elif op == "endif" and frames:
                frames.pop()
            rows.append(re.sub(r"[^\n]", " ", row))
        else:
            rows.append(row)
        offsets.append(offset)
        states.append((tuple(f["guard"] for f in frames), any(f["inactive"] for f in frames)))
        offset += len(row)
    return "".join(rows), offsets, states


def functions(tokens):
    """簡單定義定位：constructor initializer／不支援語法保留 unresolved。"""
    opens, pairs, stack, locators = [], {}, [], []
    for i, token in enumerate(tokens):
        value = token[0]
        if value == "(":
            opens.append(i)
        elif value == ")" and opens:
            pairs[i] = opens.pop()
    for i, token in enumerate(tokens):
        value = token[0]
        if value == "{":
            name = None
            j = i - 1
            while j >= 0 and tokens[j][0] in {"const", "override", "final", "noexcept"}:
                j -= 1
            if j in pairs:
                begin = pairs[j]
                k = begin - 1
                if k >= 0 and re.fullmatch(r"[A-Za-z_]\w*", tokens[k][0]) and tokens[k][0] not in CONTROLS:
                    name = tokens[k][0]
                    while k >= 2 and tokens[k - 1][0] == "::" and re.fullmatch(r"[A-Za-z_]\w*", tokens[k - 2][0]):
                        k -= 2
                        name = tokens[k][0] + "::" + name
                    if k >= 1 and tokens[k - 1][0] == "~":
                        name = "~" + name
                    # A colon (not ::) after the last body/delimiter suggests initializer syntax.
                    boundary = k - 1
                    while boundary >= 0 and tokens[boundary][0] not in {";", "{", "}"}:
                        if tokens[boundary][0] == ":":
                            name = "<unresolved:initializer>"
                            break
                        boundary -= 1
            stack.append(name)
        current = next((name for name in reversed(stack) if name), "<file-scope-or-unresolved>")
        locators.append(current)
        if value == "}" and stack:
            stack.pop()
    return locators


def scan(source, definitions):
    masked, offsets, contexts = lexical_context(source)
    tokens = [(m.group(), m.start()) for m in TOKENS.finditer(masked)]
    values = [t[0] for t in tokens]
    locators = functions(tokens)
    records = collections.Counter()
    for i, (value, position) in enumerate(tokens):
        kind, symbol, condition = None, None, None
        if re.fullmatch(r"CC_\w+", value):
            symbol, kind = value, "symbol-reference"
        elif value.startswith("FUNC_CC_"):
            symbol, kind = value, "function-flag-reference"
        numeric = bool(re.fullmatch(r"(?:0[xX][0-9A-Fa-f]+|\d+)[uUlL]*", value))
        if symbol or numeric:
            if i >= 2 and values[i - 1] in COMPARATORS and values[i - 2] == "CUSTOMER_CODE":
                kind, condition = "direct-comparison", f"CUSTOMER_CODE {values[i - 1]} {value}"
            elif i + 2 < len(values) and values[i + 1] in COMPARATORS and values[i + 2] == "CUSTOMER_CODE":
                kind, condition = "direct-comparison", f"{value} {values[i + 1]} CUSTOMER_CODE"
            if kind == "direct-comparison" and numeric:
                numeric_value = integer(value)
                aliases = tuple(sorted(s for s, d in definitions.items() if numeric_value is not None and d["value"] == numeric_value))
                symbol = value
            else:
                aliases = ()
            if kind:
                context = contexts[max(0, bisect.bisect_right(offsets, position) - 1)]
                # Tokens in a CUSTOMER_CODE switch are symbol references, not proven case guards.
                records[(locators[i], kind, symbol, condition or "", context[0], context[1], aliases)] += 1
    return [
        {"function_candidate": fn, "kind": kind, "symbol": symbol, "condition_candidate": condition or None,
         "preprocessor_guards": list(guards), "literal_inactive_candidate": inactive,
         "numeric_aliases_same_version": list(aliases), "occurrences": count}
        for (fn, kind, symbol, condition, guards, inactive, aliases), count in sorted(records.items())
    ]


def source_class(path):
    parts = path.lower().split("/")
    if any(part in {"tests", "test", "harness", "fidelity"} for part in parts) or Path(path).name.lower().startswith("test_"):
        return "test-or-harness-candidate"
    if any(part in {"generated", "cpp_generated", "generated_pages"} for part in parts):
        return "generated-candidate"
    return "source-candidate-build-membership-unchecked"


def run_scan(repo, ref):
    commit = git(repo, "rev-parse", "--verify", ref + "^{commit}").decode().strip()
    entries = []
    for item in git(repo, "ls-tree", "-r", "-z", commit).split(b"\0"):
        if not item:
            continue
        meta, rawpath = item.split(b"\t", 1)
        path = rawpath.decode("utf-8")
        _, objtype, blob = meta.decode().split()
        if objtype != "blob" or ".svn" in path.split("/") or Path(path).suffix.lower() not in {".cpp", ".h", ".hpp", ".inc"}:
            continue
        for version, (root, encoding) in SCOPES.items():
            if path.startswith(root + "/"):
                entries.append((version, path, blob, encoding))
                break
    # Git objects only. No checkout, process launch of the machine program or runtime reads.
    proc = subprocess.Popen(["git", "-C", str(repo), "cat-file", "--batch"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    files, definitions, header_blobs = {}, {}, {}
    try:
        for version, path, blob, encoding in entries:
            proc.stdin.write((blob + "\n").encode("ascii")); proc.stdin.flush()
            response = proc.stdout.readline().decode().split()
            if len(response) != 3 or response[1] != "blob":
                raise RuntimeError("Git blob unavailable: " + path)
            content = proc.stdout.read(int(response[2]))
            if proc.stdout.read(1) != b"\n":
                raise RuntimeError("Unexpected Git batch delimiter")
            text = content.decode(encoding, "replace")
            files[path] = (version, blob, text, text.count("\ufffd"))
            if path == SCOPES[version][0] + "/MachineType.h":
                definitions[version] = symbols(text)
                header_blobs[version] = blob
    finally:
        proc.stdin.close(); proc.stdout.close(); proc.wait()
    if set(definitions) != set(SCOPES):
        raise RuntimeError("Both version-specific MachineType.h definitions are required")
    findings, counts, decoding = [], collections.Counter(), []
    manifest = hashlib.sha256()
    for path, (version, blob, source, replacement_count) in sorted(files.items()):
        counts[version] += 1
        manifest.update((path + "\0" + blob + "\n").encode("utf-8"))
        if replacement_count:
            decoding.append({"version": version, "path": path, "replacement_characters": replacement_count})
        for record in scan(source, definitions[version]):
            findings.append({"version": version, "path": path, "blob": blob, "source_class": source_class(path), **record})
    return {
        "schema": 1, "source_commit": commit, "source_manifest_sha256": manifest.hexdigest(),
        "scanner_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "scanned_files": dict(sorted(counts.items())), "definition_blobs": header_blobs,
        "definitions": {v: list(sorted(d.values(), key=lambda x: x["symbol"])) for v, d in definitions.items()},
        "decoding_notes": decoding,
        "limits": ["Lexical candidates; no compilation, caller analysis, machine test or runtime read",
                   "Function locator is heuristic; lambdas inherit their enclosing function and unsupported syntax can be unresolved",
                   "Preprocessor directives are retained, not evaluated; literal 0/1 hints do not select a real build",
                   "Switch cases, expressions around operands and indirect CUSTOMER_CODE aliases require manual review",
                   "Symbol references include declarations, tests and generated sources; absence never establishes 912-only",
                   "Source manifest pins all scanned files; customer agent/region/language authority remains the report table"],
        "findings": findings,
    }


def self_test():
    definitions = symbols("#define CC_OLD 957\n#define CC_ALIAS CC_OLD\n// #define CC_FAKE 22\n#define CC_HEX 0x3BE\n")
    assert definitions["CC_ALIAS"]["value"] == 957 and definitions["CC_HEX"]["value"] == 958
    assert "CC_FAKE" not in definitions
    assert integer('017') == 15 and integer('0x10U') == 16 and integer('099') is None
    sample = '''
void Handler::Work() {
  // if (CUSTOMER_CODE == CC_COMMENT) {}
  const char* s = "CUSTOMER_CODE == CC_STRING";
  const char* raw = R"xx(CUSTOMER_CODE == CC_RAW)xx";
  if (CUSTOMER_CODE\n == CC_OLD) {}
  if (957 == CUSTOMER_CODE) {}
  if (CUSTOMER_CODE != 999) {}
  auto lambda = []() { return CUSTOMER_CODE == CC_ALIAS; };
#if 0
  if (CUSTOMER_CODE == CC_DISABLED) {}
#else
  if (CUSTOMER_CODE == CC_ELSE) {}
#endif
  if (FUNC_CC_SPECIAL) {}
}
void Other() { switch (CUSTOMER_CODE) { case CC_OLD: break; } }
'''
    found = scan(sample, definitions)
    by_symbol = {r["symbol"]: r for r in found}
    assert not {"CC_COMMENT", "CC_STRING", "CC_RAW"} & set(by_symbol)
    assert by_symbol["CC_OLD"]["function_candidate"] == "Other"
    assert by_symbol["957"]["numeric_aliases_same_version"] == ["CC_ALIAS", "CC_OLD"]
    assert by_symbol["999"]["numeric_aliases_same_version"] == []
    assert by_symbol["CC_DISABLED"]["literal_inactive_candidate"]
    assert not by_symbol["CC_ELSE"]["literal_inactive_candidate"]
    assert by_symbol["CC_ALIAS"]["function_candidate"] == "Handler::Work"
    assert by_symbol["FUNC_CC_SPECIAL"]["kind"] == "function-flag-reference"
    assert all("line" not in key for row in found for key in row)
    assert "客戶".encode("cp950").decode("cp950") == "客戶"
    assert scan("#if 1\nvoid F(){if(CUSTOMER_CODE==CC_OLD){}}\n#else\nvoid G(){CC_OLD;}\n#endif", definitions)[1]["literal_inactive_candidate"]
    initializer = scan("C::C(): field(0) { CC_OLD; }", definitions)
    assert initializer[0]["function_candidate"] == "<unresolved:initializer>"
    print("self-test passed: comments/strings/raw strings, aliases/numeric/reversed operands, multiline, literal guards, lambda, switch limits, initializer, Big5")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path)
    parser.add_argument("--ref", help="immutable source commit preferred")
    parser.add_argument("--output", type=Path, help="explicit JSON output; use a temporary directory")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return
    if not args.repo or not args.ref or not args.output:
        parser.error("--repo, --ref and --output are required for a scan")
    output = args.output.resolve()
    if output.exists():
        parser.error("output already exists; select a fresh output file to preserve previous evidence")
    repo = args.repo.resolve()
    if repo == output or repo in output.parents:
        parser.error("write scan evidence outside the repository; do not write into runtime or tracked data")
    result = run_scan(repo, args.ref)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"source_commit": result["source_commit"], "scanned_files": result["scanned_files"],
                      "candidate_records": len(result["findings"]), "output": str(output)}, ensure_ascii=False))


if __name__ == "__main__":
    main()
