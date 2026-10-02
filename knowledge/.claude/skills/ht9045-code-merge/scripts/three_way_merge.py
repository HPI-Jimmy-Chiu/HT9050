#!/usr/bin/env python3
"""
HT9011UC 3-Way Merge Script
用法: three_way_merge.py --base <base> --source <source> --target <target> --output <output>

後端優先順序：
  1. diff3 (Git for Windows)  — 對 Both-Added / 純插入 邊界案例最穩定
  2. difflib fallback         — 僅在 diff3 不可用時使用

後驗驗證（post-merge hook）：
  - 花括號深度平衡檢查（偵測 3-way 產生的函數結構損壞）
  - 重複 #define 檢查（偵測 Both-Added 靜默重複）
  - 合併行數合理性檢查（偵測 source==base 邊界案例造成的內容遺失）
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

# -----------------------------------------------------------------------
# I/O helpers
# -----------------------------------------------------------------------

def read_cp950(path):
    with open(path, 'r', encoding='cp950', errors='replace') as f:
        return f.readlines()

def write_cp950(path, lines):
    with open(path, 'w', encoding='cp950', errors='replace', newline='') as f:
        f.writelines(lines)

# -----------------------------------------------------------------------
# diff3 backend（首選）
# -----------------------------------------------------------------------

DIFF3_CANDIDATES = [
    r'C:\Program Files\Git\usr\bin\diff3.exe',
    r'C:\Program Files (x86)\Git\usr\bin\diff3.exe',
    shutil.which('diff3') or '',
]

def find_diff3():
    for p in DIFF3_CANDIDATES:
        if p and os.path.isfile(p):
            return p
    return None

def three_way_merge_diff3(base_path, source_path, target_path, output_path,
                          source_label='source', target_label='target'):
    """
    使用 diff3 執行三方合併。
    回傳 (has_conflicts: bool, used_diff3: bool)。
    has_conflicts=True 表示輸出中含有 <<<< 衝突標記。
    """
    diff3 = find_diff3()
    if not diff3:
        return None, False  # 無 diff3，通知 caller 使用 fallback

    # diff3 -m <mine/target> <base> <other/source>
    result = subprocess.run(
        [diff3, '-m',
         '--label', target_label,
         '--label', 'SVN_BASE',
         '--label', source_label,
         target_path, base_path, source_path],
        capture_output=True
    )
    # returncode: 0=clean merge, 1=conflicts present, 2=error
    if result.returncode not in (0, 1):
        print(f"[WARN] diff3 執行失敗 (rc={result.returncode})，切換至 difflib", file=sys.stderr)
        return None, False

    # diff3 以 bytes 輸出，直接寫入保留原始 CP950 編碼
    with open(output_path, 'wb') as f:
        f.write(result.stdout)

    has_conflicts = (result.returncode == 1)
    return has_conflicts, True

# -----------------------------------------------------------------------
# difflib backend（fallback）
# -----------------------------------------------------------------------

def _parse_hunks(diff_lines):
    hunks = []
    hunk = None
    for line in diff_lines:
        if line.startswith('@@'):
            if hunk:
                hunks.append(hunk)
            m = re.match(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', line)
            if m:
                old_start = int(m.group(1))
                old_count = int(m.group(2)) if m.group(2) is not None else 1
                new_start = int(m.group(3))
                new_count = int(m.group(4)) if m.group(4) is not None else 1
                hunk = {
                    'old_start': old_start, 'old_count': old_count,
                    'new_start': new_start, 'new_count': new_count,
                    'removed': [], 'added': []
                }
        elif hunk is not None:
            if line.startswith('-') and not line.startswith('---'):
                hunk['removed'].append(line[1:] + '\n')
            elif line.startswith('+') and not line.startswith('+++'):
                hunk['added'].append(line[1:] + '\n')
    if hunk:
        hunks.append(hunk)
    return hunks

def _hunk_range(h):
    """
    回傳 hunk 在 base 中的行號區間 (start, end)，1-based。
    純插入（old_count=0）時回傳 (old_start, old_start)，
    以便 ranges_overlap 能正確偵測同插入點衝突。
    """
    s = h['old_start']
    c = h['old_count']
    if c == 0:
        return (s, s)          # 純插入：以插入點做點區間
    return (s, s + c - 1)

def _ranges_overlap(r1, r2, c1_pure, c2_pure):
    """
    判斷兩個 hunk 是否重疊，正確處理「純插入 vs 純插入」邊界案例。
    c1_pure / c2_pure：該 hunk 的 old_count 是否為 0。
    """
    if c1_pure and c2_pure:
        return r1[0] == r2[0]          # 同一插入點 → 衝突
    if c1_pure:
        return r2[0] <= r1[0] <= r2[1]
    if c2_pure:
        return r1[0] <= r2[0] <= r1[1]
    return r1[0] <= r2[1] and r2[0] <= r1[1]

def three_way_merge_difflib(base_lines, source_lines, target_lines):
    """
    difflib fallback：僅在 diff3 不可用時使用。
    回傳 (merged_lines, conflicts)。
    conflicts 為 list of dict，含 base_start / source_hunk / target_hunk。
    """
    import difflib

    s_diff = list(difflib.unified_diff(base_lines, source_lines, n=0, lineterm=''))
    t_diff = list(difflib.unified_diff(base_lines, target_lines, n=0, lineterm=''))

    s_hunks = _parse_hunks(s_diff)
    t_hunks = _parse_hunks(t_diff)

    conflicts = []
    conflict_s = set()
    conflict_t = set()

    for si, sh in enumerate(s_hunks):
        sr = _hunk_range(sh)
        s_pure = (sh['old_count'] == 0)
        for ti, th in enumerate(t_hunks):
            tr = _hunk_range(th)
            t_pure = (th['old_count'] == 0)
            if _ranges_overlap(sr, tr, s_pure, t_pure):
                if sh['removed'] == th['removed'] and sh['added'] == th['added']:
                    conflict_t.add(ti)   # 完全相同的修改：只套用一次
                else:
                    conflicts.append({
                        'base_start': sr[0],
                        'source_hunk': sh,
                        'target_hunk': th
                    })
                    conflict_s.add(si)
                    conflict_t.add(ti)

    all_hunks = []
    for si, sh in enumerate(s_hunks):
        if si not in conflict_s:
            all_hunks.append(('source', sh))
    for ti, th in enumerate(t_hunks):
        if ti not in conflict_t:
            all_hunks.append(('target', th))

    all_hunks.sort(key=lambda x: x[1]['old_start'], reverse=True)

    merged = list(base_lines)
    for _origin, hunk in all_hunks:
        start = hunk['old_start'] - 1
        count = hunk['old_count']
        merged[start:start + count] = hunk['added']

    return merged, conflicts

# -----------------------------------------------------------------------
# Post-merge validation hook（兩個後端共用）
# -----------------------------------------------------------------------

def validate_merged_output(lines, filepath):
    """
    合併後立即執行的後驗檢查。
    回傳 issues 清單；若為空表示通過。
    """
    issues = []
    basename = os.path.basename(filepath)

    # --- 1. 花括號深度平衡（偵測函數結構損壞）---
    depth = 0
    for i, line in enumerate(lines, 1):
        stripped = line.strip()
        # 跳過純注解行
        if stripped.startswith('//') or stripped.startswith('*'):
            continue
        # 去除行內字串與字元常數（簡化，非完整剖析）
        clean = re.sub(r'"[^"]*"', '""', stripped)
        clean = re.sub(r"'[^']*'", "''", clean)
        # 去除行尾注解
        clean = re.sub(r'//.*', '', clean)
        depth += clean.count('{') - clean.count('}')
        if depth < 0:
            issues.append(f"  ⚠ L{i}: 花括號深度為負（depth={depth}），可能為函數結構損壞")
            depth = 0  # 重置以繼續偵測後續問題
    if depth != 0:
        issues.append(f"  ⚠ 檔案結尾花括號不平衡（殘餘 depth={depth}）")

    # --- 2. 重複 #define（偵測 Both-Added 靜默重複）---
    defines = {}
    for i, line in enumerate(lines, 1):
        m = re.match(r'^\s*#\s*define\s+(\w+)', line)
        if m:
            name = m.group(1)
            if name in defines:
                issues.append(
                    f"  ⚠ L{i}: 重複 #define {name}（首次在 L{defines[name]}）"
                )
            else:
                defines[name] = i

    # --- 3. 合併行數合理性（偵測 source==base 邊界案例造成的大量內容遺失）---
    # 此項目需在 main() 呼叫時傳入 base/source/target 行數做比較，
    # 此處只做輸出行數 = 0 的極端防護
    if len(lines) == 0:
        issues.append("  ⚠ 合併輸出為空，請立即還原備份！")

    return issues

def check_merge_line_loss(base_len, source_len, target_len, merged_len, filepath):
    """
    source==base 邊界案例：若 source 未修改但 target 有大量新增，
    difflib 可能靜默選 base 版本，導致 merged 比 target 少很多行。
    """
    issues = []
    expected_min = max(base_len, source_len, target_len)
    # 合理閾值：合併結果不應比三者最大值少超過 10%
    if merged_len < expected_min * 0.9:
        issues.append(
            f"  ⚠ 合併行數 {merged_len} 遠少於預期下限 {int(expected_min*0.9)}"
            f"（base={base_len}, src={source_len}, tgt={target_len}）"
            f"，疑似 source==base 邊界案例導致內容遺失！請還原備份手動合併。"
        )
    return issues

# -----------------------------------------------------------------------
# Merge risk classifier（Step 2 分類輔助）
# -----------------------------------------------------------------------

def classify_merge_risk(source_path, base_path):
    """
    在合併前評估此檔案的合併風險等級。
    回傳 'safe' / 'medium' / 'high' 及說明字串。
    """
    import difflib
    try:
        src  = read_cp950(source_path)
        base = read_cp950(base_path)
    except Exception:
        return 'unknown', '無法讀取檔案'

    diff = list(difflib.unified_diff(base, src, n=0))
    changed_lines = sum(
        1 for l in diff
        if l.startswith(('+', '-')) and not l.startswith(('+++', '---'))
    )
    hunk_count = sum(1 for l in diff if l.startswith('@@'))

    if changed_lines > 50 or hunk_count > 10:
        return 'high', f'修改 {changed_lines} 行，分散在 {hunk_count} 個區塊 → 建議手動合併'
    elif hunk_count > 3:
        return 'medium', f'修改 {changed_lines} 行，{hunk_count} 個區塊 → 自動合併後必須人工審查輸出'
    return 'safe', f'修改 {changed_lines} 行，{hunk_count} 個區塊'

# -----------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description='HT9011UC 3-Way Merge')
    parser.add_argument('--base',         required=True,  help='Base (SVN revision) file')
    parser.add_argument('--source',       required=True,  help='Source (developer) file')
    parser.add_argument('--target',       required=True,  help='Target (Steven) file')
    parser.add_argument('--output',       required=True,  help='Output merged file')
    parser.add_argument('--dry-run',      action='store_true', help='Only report conflicts, do not write output')
    parser.add_argument('--risk-check',   action='store_true', help='Print merge risk level and exit')
    parser.add_argument('--source-label', default='source', help='Label for source in conflict markers')
    parser.add_argument('--target-label', default='target', help='Label for target in conflict markers')
    args = parser.parse_args()

    # --- 風險評估模式 ---
    if args.risk_check:
        level, reason = classify_merge_risk(args.source, args.base)
        print(f"[RISK] {level.upper()}: {reason}")
        sys.exit(0 if level == 'safe' else (1 if level == 'medium' else 2))

    base   = read_cp950(args.base)
    source = read_cp950(args.source)
    target = read_cp950(args.target)

    print(f"Base: {len(base)} lines | Source: {len(source)} lines | Target: {len(target)} lines")

    has_conflicts = False
    used_diff3    = False

    # --- 嘗試 diff3 後端 ---
    if not args.dry_run:
        result, used_diff3 = three_way_merge_diff3(
            args.base, args.source, args.target, args.output,
            source_label=args.source_label,
            target_label=args.target_label,
        )
        if used_diff3:
            has_conflicts = result
            print(f"[INFO] 後端: diff3 ({'有衝突' if has_conflicts else '無衝突'})")
            merged = read_cp950(args.output)
        else:
            print("[INFO] diff3 不可用，切換至 difflib 後端")

    # --- difflib fallback ---
    if not used_diff3:
        merged, conflicts = three_way_merge_difflib(base, source, target)
        has_conflicts = bool(conflicts)
        print(f"[INFO] 後端: difflib ({'有衝突' if has_conflicts else '無衝突'})")

        if conflicts:
            for i, c in enumerate(conflicts):
                print(f"\n--- Conflict #{i+1} at base line {c['base_start']} ---")
                print(f"  Source: -{len(c['source_hunk']['removed'])} +{len(c['source_hunk']['added'])} lines")
                print(f"  Target: -{len(c['target_hunk']['removed'])} +{len(c['target_hunk']['added'])} lines")
            print("\n⚠ 需要手動解決衝突區塊。")
            if args.dry_run:
                sys.exit(1)

        if not args.dry_run:
            write_cp950(args.output, merged)

    print(f"Merged: {len(merged)} lines  →  {args.output}")

    # --- Post-merge validation ---
    print("\n[POST-MERGE VALIDATION]")
    all_issues = []
    all_issues += validate_merged_output(merged, args.output)
    all_issues += check_merge_line_loss(len(base), len(source), len(target), len(merged), args.output)

    if all_issues:
        print("  發現以下問題，請人工確認：")
        for issue in all_issues:
            print(issue)
        sys.exit(2)
    else:
        print("  ✓ 花括號平衡  ✓ 無重複 #define  ✓ 行數合理")

    sys.exit(1 if has_conflicts else 0)

if __name__ == '__main__':
    main()
