#!/usr/bin/env python3
"""從唯讀 scan JSON 產生本 Skill 的候選 reference 樹，人工主題表不覆寫。"""
import argparse
import collections
import hashlib
import json
import re
from pathlib import Path
from urllib.parse import quote

TOPICS = [
    ("hpi-shuttle-flow", "Shuttle"), ("hpi-index-flow", "Index"), ("hpi-tray-flow", "Tray"),
    ("hpi-inarm-flow", "InArm"), ("hpi-outarm-flow", "OutArm"), ("hpi-motor-home", "Motor Home"),
    ("hpi-motor-control", "Motor Control"), ("hpi-io-control", "IO"), ("hpi-temperature", "Temperature"),
    ("hpi-ep-pressure", "EP Pressure"), ("hpi-bin-display", "Bin Display"), ("hpi-lotinfo-recipe", "LotInfo／Recipe／QA"),
    ("hpi-alarm", "Alarm／Yield"), ("hpi-autostart-autoclean", "AutoStart／AutoClean"),
    ("hpi-gpib", "GPIB"), ("hpi-secs", "SECS"), ("hpi-rs232", "RS232"),
]


def cell(value):
    return str(value).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("|", "&#124;").replace("\n", " ")


def write(root, relative, text):
    target = root / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text.rstrip() + "\n", encoding="utf-8")
    return relative


def render(scan_path, root):
    data = json.loads(scan_path.read_text(encoding="utf-8"))
    assert data["schema"] == 1 and re.fullmatch(r"[0-9a-f]{40}", data["source_commit"])
    scanner = root.parent / "scripts/customer_scan.py"
    assert hashlib.sha256(scanner.read_bytes()).hexdigest() == data["scanner_sha256"], "scanner changed; re-scan before rendering"
    summary_path = root / "versions/scan-summary.json"
    previous = json.loads(summary_path.read_text(encoding="utf-8")) if summary_path.exists() else None
    if previous and previous["source_commit"] != data["source_commit"]:
        raise ValueError("different baseline: review and preserve previous generated tree before replacing")
    commit = data["source_commit"]
    site = "https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/" + commit + "/"
    definitions = {version: {d["symbol"]: d for d in values} for version, values in data["definitions"].items()}
    by_symbol, groups, output_files = collections.defaultdict(list), collections.defaultdict(list), []
    for i, row in enumerate(data["findings"]):
        by_symbol[row["symbol"]].append((i, row))
    all_symbols = sorted(set(by_symbol) | {symbol for d in definitions.values() for symbol in d})
    unknown, no_refs = [], []
    for symbol in all_symbols:
        family = "numeric" if symbol[0].isdigit() else "flags" if symbol.startswith("FUNC_CC_") else "cc-" + symbol[3:4].lower()
        stem = re.sub(r"[^a-z0-9_-]", "-", symbol.lower())
        folder = f"customers/{family}/{stem}"
        records = by_symbol.get(symbol, [])
        intro = f"# {symbol} 候選定位\n\n來源commit `{commit}`；每列是詞法候選，機型／caller／建置／業務行為未全量核對。\n\n"
        intro += "| 版本 | MachineType.h定義 | 數值 |\n|---|---|---|\n"
        for version in ("v912", "v906-cpp"):
            d = definitions[version].get(symbol)
            intro += f"| {version} | {cell(d['definition']) if d else '未列為此版CC定義；另查宣告／旗標／數值比較'} | {d['value'] if d and d['value'] is not None else '未決定'} |\n"
            if symbol.startswith("CC_") and d is None and any(r["version"] == version for _, r in records):
                unknown.append((version, symbol))
        if symbol[0].isdigit():
            intro += "\n這是比較運算元，不自動視為客戶身分；同版數值別名只作查表線索，範圍比較／未定義數值另核對。\n"
        elif symbol.startswith("FUNC_CC_"):
            intro += "\n這是旗標識別符，不自動對應CC值、機型或已啟用狀態。\n"
        intro += "\n[資料權威](../../../authority.md)／[既有人工主題表](../../../topics/index.md)／[scanner界線](../../../scanner.md)。\n"
        if not records:
            no_refs.append(symbol)
            intro += "\n掃描範圍沒有本符號的詞法引用；定義仍保留，不推論無功能或版本獨有。\n"
        intro += "\n## 定位頁\n\n"
        for n in range(0, len(records), 80):
            filename = f"part-{n // 80 + 1:02}.md"
            intro += f"- [候選 {n + 1}～{min(n + 80, len(records))}]({filename})\n"
            page = f"# {symbol}：定位頁 {n // 80 + 1}\n\n[回符號入口](index.md)。來源commit `{commit}`。\n\nfunction為候選；unresolved、lambda外層、停用提示與test／generated不能提升成實際生效結論。未列Task／case，需人工追完整條件與caller。\n\n"
            page += "| 版本／來源類別 | 檔名／function候選 | 類型／條件 | preprocessor候選 | 同版數值別名 | 次數 |\n|---|---|---|---|---|---:|\n"
            for index, row in records[n:n + 80]:
                relative = row["path"].split("/", 1)[1]
                url = site + quote(row["path"], safe="/")
                source = f"[{cell(relative)}]({url}) :: {cell(row['function_candidate'])}"
                guard = " ; ".join(row["preprocessor_guards"]) or "無詞法條件標記；仍未核對build"
                if row["literal_inactive_candidate"]:
                    guard += "；字面停用候選"
                condition = row["kind"] + ("：" + row["condition_candidate"] if row["condition_candidate"] else "")
                page += f"| {row['version']}／{row['source_class']} | {source} | {cell(condition)} | {cell(guard)} | {cell(', '.join(row['numeric_aliases_same_version'])) or '—'} | {row['occurrences']} | <!-- scan-record:{index} -->\n"
            output_files.append(write(root, folder + "/" + filename, page))
        output_files.append(write(root, folder + "/index.md", intro))
        groups[family].append((symbol, stem, len(records)))
    customers = "# 客戶符號／旗標／數值候選樹\n\n兩版定義與function候選同題分流；這是定位工具，不是已驗證功能清單。先讀[共同方法](../common.md)與[原主題表](../topics/index.md)。\n\n| 群組 | 符號／運算元數 | 候選列 |\n|---|---:|---:|\n"
    for family, values in sorted(groups.items()):
        group = f"# {family} 候選符號\n\n[回符號樹](../index.md)。各symbol保留獨立定義，別名／改名不自動合併。\n\n| 符號／運算元 | 候選列 |\n|---|---:|\n"
        for symbol, stem, count in values:
            group += f"| [{symbol}]({stem}/index.md) | {count} |\n"
        output_files.append(write(root, f"customers/{family}/index.md", group))
        customers += f"| [{family}]({family}/index.md) | {len(values)} | {sum(v[2] for v in values)} |\n"
    output_files.append(write(root, "customers/index.md", customers))
    version_index = f"# 版本分開查\n\n唯讀來源commit `{commit}`。V912是BCB6／Big5，V906 Cpp是C++17／UTF-8；二者不是目前機台實際程式版本的推定。\n\n| 版本 | 掃描檔數 | MachineType.h定義數 | 入口 |\n|---|---:|---:|---|\n"
    for version, count in data["scanned_files"].items():
        name = version + ".md"
        version_index += f"| {version} | {count} | {len(definitions[version])} | [符號定義]({name}) |\n"
        prefix = f"# {version} 的CC定義\n\n來源commit `{commit}`，MachineType.h blob `{data['definition_blobs'][version]}`。這是程式符號／數值表，不含代理商／地區推論。\n\n[分版本入口](index.md)／[候選樹](../customers/index.md)。\n\n| 符號 | 原定義 | 數值／別名解析 |\n|---|---|---|\n"
        for symbol, d in sorted(definitions[version].items()):
            family = "cc-" + symbol[3:4].lower(); stem = symbol.lower()
            prefix += f"| [{symbol}](../customers/{family}/{stem}/index.md) | {cell(d['definition'])} | {d['value'] if d['value'] is not None else '未決定'} |\n"
        output_files.append(write(root, "versions/" + name, prefix))
    differences = "# 兩版CC定義的文字差異\n\n只比MachineType.h的symbol／定義／數值；這不是功能差異或912-only裁決。改名、別名與使用處需另查caller。\n\n| 符號 | V912定義／數值 | V906 Cpp定義／數值 |\n|---|---|---|\n"
    for symbol in sorted(set(definitions['v912']) | set(definitions['v906-cpp'])):
        a, b = definitions['v912'].get(symbol), definitions['v906-cpp'].get(symbol)
        if a == b:
            continue
        label = lambda d: cell(str(d['definition']) + ' / ' + str(d['value'])) if d else '此版MachineType.h未列此symbol'
        differences += f"| {symbol} | {label(a)} | {label(b)} |\n"
    output_files.append(write(root, "versions/definition-differences.md", differences))
    version_index += "\n[兩版定義差異](definition-differences.md)只比symbol與數值，不作功能結論。總計是檔數與詞法候選，不能解讀成客戶分支覆蓋率。[掃描摘要](scan-summary.json)釘住來源manifest與scanner；沒有命中不構成912-only。\n"
    output_files.append(write(root, "versions/index.md", version_index))
    topics = "# 各主題客戶知識路由\n\n保留原表與原owner；此索引不覆寫人工行為、機型條件與版本查證界線。連結存在不表示每列已重新審過。\n\n| 主題 | 原客戶表／入口 | 使用界線 |\n|---|---|---|\n"
    for skill, title in TOPICS:
        assert (root.parent.parent / skill / "references/customers.md").exists(), skill
        topics += f"| {title} | [{skill}](../../../{skill}/references/customers.md) | 按原表的function／Task、來源版本與相關機台查證；候選不自動回填 |\n"
    topics += "| Config | [客戶樹](../../../hpi-config/references/customer/index.md) | CC碼、名字與報告權威分開 |\n| Web HMI | [入口](../../../hpi-web-hmi/SKILL.md) | 尚無獨立customers.md；按登入／橋接／頁面條件核對，不推論無客戶差異 |\n| MotionView | [入口](../../../hpi-motionview/SKILL.md) | 尚無獨立customers.md；機型／資料與客戶條件另核對 |\n"
    topics += "\nShuttle是先前人工樣板；ST02的GPIB／SECS／RS232表沿用不重做。部分表含tester／廠商／通訊裝置或一般旗標，不能把每列升格成CUSTOMER_CODE分支。來源候選按CC分層，尚未完成全樹人工topic歸屬與六欄行為驗證。\n"
    output_files.append(write(root, "topics/index.md", topics))
    unresolved = sum(r["function_candidate"].startswith("<") for r in data["findings"])
    kinds = dict(collections.Counter(r["kind"] for r in data["findings"]))
    pending = f"""# 待補與孤兒候選\n\n來源commit `{commit}`；此處是詞法／文件待核對清單，不能當成機台缺陷。\n\n| 項目 | 實際數量／下一步 |\n|---|---|\n| 定位候選 | {len(data['findings'])}列；其中direct-comparison {kinds.get('direct-comparison', 0)}，其他是symbol／flag引用 |
| file-scope或unresolved | {unresolved}列；按原檔與symbol定位、檢查定義／macro／複雜語法，不冒稱精確function |
| 字面停用提示 | {sum(r['literal_inactive_candidate'] for r in data['findings'])}列；完整建置旗標與caller未求值 |
| 本版MachineType未列CC定義 | {len(unknown)}組版本／符號；可能來自其他宣告，先核對，不能擅自新增客戶 |
| 定義在範圍內無引用 | {len(no_refs)}個符號；可能有間接macro／其他樹，不推論不存在功能 |
| 全樹主題owner／行為 | 未完成；先使用[既有20題路由](topics/index.md)，不按檔名自動認領他人表 |
| 機型與實機 | 未做全量dispatch／caller／runtime驗證，HT9050和其他Handler差異保持待核對 |
\n## 編碼提示\n\nASCII識別符仍可搜尋；下列檔在指定版本decoder出現替換字元，中文註解與非ASCII語法需另核對原bytes。沒有改檔或重編碼。\n\n| 版本 | 檔案 | 替換字元數 |\n|---|---|---:|\n"""
    for row in data["decoding_notes"]:
        pending += f"| {row['version']} | {cell(row['path'])} | {row['replacement_characters']} |\n"
    pending += "\n## 非本批依賴\n\nUPH缺少HT9050原稿的部分仍待來源；不為補齊索引捏造內容。舊入口退休仍等部署／caller盤點，不刪相容路徑。\n"
    output_files.append(write(root, "pending.md", pending))
    summary = {key: data[key] for key in ("schema", "source_commit", "source_manifest_sha256", "scanner_sha256", "scanned_files", "definition_blobs", "limits", "decoding_notes")}
    summary.update(candidate_records=len(data["findings"]), kinds=kinds, unresolved_records=unresolved,
                   definition_counts={v: len(d) for v, d in definitions.items()}, reference_symbol_count=len(all_symbols),
                   generated_files=sorted(output_files), scan_json_sha256=hashlib.sha256(scan_path.read_bytes()).hexdigest(),
                   renderer_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest())
    write(root, "versions/scan-summary.json", json.dumps(summary, ensure_ascii=False, indent=2))
    print(json.dumps({"candidate_records": len(data["findings"]), "symbols": len(all_symbols), "generated_markdown": len(output_files)}, ensure_ascii=False))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scan_json", type=Path)
    args = parser.parse_args()
    render(args.scan_json, Path(__file__).resolve().parent.parent / "references")
