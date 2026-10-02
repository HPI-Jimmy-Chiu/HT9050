#!/usr/bin/env python3
"""
HT9011UC SVN Overlap Analysis Script
比較來源（Developer）與目標（Steven）的 SVN 修改檔案，偵測重疊。
用法: overlap_analysis.py --source <source_path> --target <target_path>
      [--skip-ext ...] [--check-cross-rev] [--temp-dir <dir>]

--check-cross-rev:
    當 source revision < target revision 時，對候選直接複製檔執行
    svn cat -r <source_rev> 跨版本比對，有 committed 差異者改為 cross_rev_merge。
    同時輸出 ChangeToFloatNonPcnt 基線計數，供合併後驗收使用。
"""
import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import re
import tempfile


def get_svn_modified(path):
    """取得 svn status 中 M 開頭的檔案（相對路徑）"""
    result = subprocess.run(['svn', 'status', path],
                            capture_output=True, text=True, encoding='cp950', errors='replace')
    files = []
    for line in result.stdout.splitlines():
        m = re.match(r'^M\s+(.+)$', line)
        if m:
            full = m.group(1).strip()
            rel = os.path.relpath(full, path)
            files.append(rel)
    return files


def get_svn_revision(path):
    """取得 working copy 的 SVN Revision，失敗回傳 None"""
    result = subprocess.run(['svn', 'info', path],
                            capture_output=True, text=True, encoding='cp950', errors='replace')
    for line in result.stdout.splitlines():
        m = re.match(r'^Revision:\s*(\d+)', line)
        if m:
            return int(m.group(1))
    return None


def svn_cat_revision(target_file, revision, out_path):
    """取出 target_file 在 revision 時的快照，寫入 out_path。回傳是否成功"""
    result = subprocess.run(['svn', 'cat', '-r', str(revision), target_file],
                            capture_output=True)
    if result.returncode != 0:
        return False
    with open(out_path, 'wb') as f:
        f.write(result.stdout)
    return True


def md5_file(path):
    h = hashlib.md5()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(8192), b''):
            h.update(chunk)
    return h.hexdigest()


def count_change_to_float(path):
    """統計 path 下所有 .cpp 檔案中 ChangeToFloatNonPcnt 的出現次數"""
    count = 0
    for root, _dirs, files in os.walk(path):
        for fname in files:
            if fname.lower().endswith('.cpp'):
                fpath = os.path.join(root, fname)
                try:
                    with open(fpath, 'rb') as f:
                        count += f.read().count(b'ChangeToFloatNonPcnt')
                except OSError:
                    pass
    return count


def main():
    parser = argparse.ArgumentParser(description='HT9011UC SVN Overlap Analysis')
    parser.add_argument('--source', required=True, help='Source (developer) path')
    parser.add_argument('--target', required=True, help='Target (Steven) path')
    parser.add_argument('--skip-ext', nargs='*', default=['.bpr', '.bpf', '.res'],
                        help='Extensions to skip')
    parser.add_argument('--check-cross-rev', action='store_true',
                        help='當 source_rev < target_rev 時，對候選直接複製檔做 svn cat 跨版本比對')
    parser.add_argument('--temp-dir', default=None,
                        help='跨版本比對用暫存目錄（預設自動建立，完成後自動刪除）')
    args = parser.parse_args()

    source_files = set(get_svn_modified(args.source))
    target_files = set(get_svn_modified(args.target))
    skip_exts = set(e.lower() for e in args.skip_ext)

    direct_copy = []
    cross_rev_merge = []
    overlap = []
    new_files = []
    skip = []

    # --- 跨版本比對準備 ---
    do_cross_rev = False
    source_rev = None
    temp_dir = args.temp_dir
    temp_dir_created = False

    if args.check_cross_rev:
        source_rev = get_svn_revision(args.source)
        target_rev = get_svn_revision(args.target)
        if source_rev is not None and target_rev is not None:
            if source_rev < target_rev:
                do_cross_rev = True
                print(f"[INFO] 跨版本比對啟動：source_rev={source_rev}, target_rev={target_rev}")
                if temp_dir is None:
                    temp_dir = tempfile.mkdtemp(prefix='ht9045_crossrev_')
                    temp_dir_created = True
                    print(f"[INFO] 暫存目錄：{temp_dir}")
            else:
                print(f"[INFO] source_rev={source_rev} >= target_rev={target_rev}，"
                      f"版號相同或 source 較新，跳過跨版本比對")

    # --- ChangeToFloatNonPcnt 基線統計 ---
    baseline = count_change_to_float(args.target)
    print(f"[合併前基線] ChangeToFloatNonPcnt 計數：{baseline}")

    # --- 主分類迴圈 ---
    for rel in sorted(source_files):
        ext = os.path.splitext(rel)[1].lower()
        if ext in skip_exts:
            skip.append(rel)
            continue

        src_path = os.path.join(args.source, rel)
        tgt_path = os.path.join(args.target, rel)

        if not os.path.exists(tgt_path):
            new_files.append(rel)
            continue

        if rel not in target_files:
            # 目標無本地修改：候選直接複製
            if do_cross_rev:
                safe_name = rel.replace('\\', '_').replace('/', '_')
                tmp_base = os.path.join(temp_dir, f'crossrev_{safe_name}')
                ok = svn_cat_revision(tgt_path, source_rev, tmp_base)
                if not ok:
                    # 在 source_rev 時該檔不存在 → 視為新增
                    new_files.append(rel)
                    continue
                if md5_file(tmp_base) != md5_file(tgt_path):
                    # target 在 source_rev 後有 committed 變更 → 不可直接複製
                    cross_rev_merge.append(rel)
                    print(f"  [跨版本 3-way] {rel}  (target 在 rev {source_rev} 後有 committed 變更)")
                else:
                    direct_copy.append(rel)
            else:
                direct_copy.append(rel)
        else:
            if md5_file(src_path) != md5_file(tgt_path):
                overlap.append(rel)
            # else: 兩端內容相同，無需任何動作

    # --- 輸出結果 ---
    print("\n=== Overlap Analysis ===")
    print(f"Source modified: {len(source_files)}")
    print(f"Target modified: {len(target_files)}")

    print(f"\nDirect copy ({len(direct_copy)}):")
    for f in direct_copy:
        print(f"  {f}")

    if cross_rev_merge:
        print(f"\nCross-revision 3-way merge ({len(cross_rev_merge)}):")
        for f in cross_rev_merge:
            print(f"  {f}")

    print(f"\nOverlap - need 3-way merge ({len(overlap)}):")
    for f in overlap:
        print(f"  {f}")

    print(f"\nNew files ({len(new_files)}):")
    for f in new_files:
        print(f"  {f}")

    print(f"\nSkip ({len(skip)}):")
    for f in skip:
        print(f"  {f}")

    print(f"\n[合併前基線] ChangeToFloatNonPcnt 計數：{baseline}")
    print("合併完成後請執行驗收（參見 SKILL.md Step 2 基線統計）：")
    print(f'  Select-String "ChangeToFloatNonPcnt" "{args.target}\\*.cpp" -Recurse | Measure-Object')

    if temp_dir_created and os.path.exists(temp_dir):
        shutil.rmtree(temp_dir, ignore_errors=True)


if __name__ == '__main__':
    main()
