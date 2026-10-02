#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HT9045/HT9011UC 程式碼合併主腳本

用法:
    merge_main.py --source <來源路徑> --target <目標路徑> --developer <開發者>
    merge_main.py --source <來源路徑> --target <目標路徑> --developer <開發者> --skip-build

功能:
    1. SVN 狀態檢查與自動修復
    2. 重疊偵測與分類
    3. 直接複製 / 3-way merge
    4. 空白標準化
    5. BCB6 編譯驗證
    6. 報告產生
"""

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime

# === 路徑設定 ===
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
HT9045_ROOT = r"D:\HT9045"
SVN_TEMP_DIR = r"D:\HT9045\HT9045_SVN_TempFile"
REPORT_ROOT = r"D:\HT9045\Merge Report"
BCB_ROOT = r"D:\ProgramFiles\Borland\CBuilder6"
PYTHON_PATH = r"C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe"
SVN_REPO_URL = "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20"

# 來源檔案副檔名
SOURCE_EXTENSIONS = {'.cpp', '.h', '.c', '.asm', '.dfm', '.rc'}

# 跳過的檔案類型
SKIP_EXTENSIONS = {'.bpr', '.bpf', '.res', '.tds', '.obj', '.exe', '.dll'}


def log(msg, level='INFO'):
    """輸出日誌"""
    timestamp = datetime.now().strftime('%H:%M:%S')
    print(f"[{timestamp}] [{level}] {msg}")


def parse_folder_name(folder_name):
    """從資料夾名稱解析版本資訊"""
    pattern = re.compile(
        r'HT9011UC_Code_V(\d+)\.(\d+)\.(\d+)\.(\d+)_(\d{8})(?:_(.+))?'
    )
    match = pattern.match(folder_name)
    if not match:
        return None
    
    major, minor, revision, patch, date_str, developer = match.groups()
    return {
        'major': int(major),
        'minor': int(minor),
        'revision': int(revision),
        'patch': int(patch),
        'date': date_str,
        'developer': developer or 'Unknown',
        'full_version': f"V{major}.{minor}.{revision}.{patch}"
    }


def check_svn_status(path):
    """檢查 SVN 狀態"""
    if not os.path.exists(os.path.join(path, '.svn')):
        return False, "找不到 .svn 資料夾"
    
    try:
        result = subprocess.run(
            ['svn', 'info', path],
            capture_output=True,
            text=True,
            encoding='cp950',
            errors='replace',
            timeout=30
        )
        return result.returncode == 0, result.stderr if result.returncode != 0 else "OK"
    except Exception as e:
        return False, str(e)


def repair_svn_if_needed(path):
    """必要時修復 SVN"""
    ok, msg = check_svn_status(path)
    if ok:
        log(f"SVN 狀態正常: {path}")
        return True
    
    log(f"SVN 狀態異常: {msg}", 'WARN')
    
    # 嘗試從資料夾名稱取得 revision
    folder_name = os.path.basename(path)
    info = parse_folder_name(folder_name)
    if not info:
        log("無法從資料夾名稱解析 Revision", 'ERROR')
        return False
    
    target_rev = info['revision']
    log(f"從資料夾名稱解析 Revision: {target_rev}")
    
    # 查找參考來源
    for item in os.listdir(HT9045_ROOT):
        full_path = os.path.join(HT9045_ROOT, item)
        if not os.path.isdir(full_path) or full_path == path:
            continue
        
        ref_info = parse_folder_name(item)
        if ref_info and ref_info['revision'] == target_rev:
            ref_ok, _ = check_svn_status(full_path)
            if ref_ok:
                log(f"找到參考來源: {full_path}")
                # 複製 .svn
                source_svn = os.path.join(full_path, '.svn')
                target_svn = os.path.join(path, '.svn')
                
                if os.path.exists(target_svn):
                    backup = target_svn + f".bak_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
                    shutil.move(target_svn, backup)
                    log(f"備份舊 .svn 至: {backup}")
                
                shutil.copytree(source_svn, target_svn)
                log("複製 .svn 完成")
                
                # 執行 cleanup
                subprocess.run(['svn', 'cleanup', path], capture_output=True)
                
                ok2, _ = check_svn_status(path)
                if ok2:
                    log("SVN 修復成功")
                    return True
    
    log("無法自動修復 SVN", 'ERROR')
    return False


def get_svn_modified_files(path):
    """取得 SVN 修改的檔案清單（相對路徑）"""
    result = subprocess.run(
        ['svn', 'status', path],
        capture_output=True,
        text=True,
        encoding='cp950',
        errors='replace'
    )
    
    files = []
    for line in result.stdout.splitlines():
        match = re.match(r'^M\s+(.+)$', line)
        if match:
            full_path = match.group(1).strip()
            rel_path = os.path.relpath(full_path, path)
            files.append(rel_path)
    
    return files


def get_svn_unversioned_files(path):
    """取得未版控的檔案（可能是新增檔案）"""
    result = subprocess.run(
        ['svn', 'status', path],
        capture_output=True,
        text=True,
        encoding='cp950',
        errors='replace'
    )
    
    files = []
    for line in result.stdout.splitlines():
        match = re.match(r'^\?\s+(.+)$', line)
        if match:
            full_path = match.group(1).strip()
            rel_path = os.path.relpath(full_path, path)
            # 只包含原始碼檔案
            ext = os.path.splitext(rel_path)[1].lower()
            if ext in SOURCE_EXTENSIONS:
                files.append(rel_path)
    
    return files


def md5_file(path):
    """計算檔案 MD5"""
    h = hashlib.md5()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(8192), b''):
            h.update(chunk)
    return h.hexdigest()


def analyze_overlap(source_path, target_path):
    """
    分析重疊檔案
    回傳: (direct_copy, overlap, new_files, skip)
    """
    source_modified = set(get_svn_modified_files(source_path))
    target_modified = set(get_svn_modified_files(target_path))
    source_new = set(get_svn_unversioned_files(source_path))
    
    direct_copy = []
    overlap = []
    new_files = []
    skip = []
    
    for rel in sorted(source_modified):
        ext = os.path.splitext(rel)[1].lower()
        
        # 跳過專案檔
        if ext in SKIP_EXTENSIONS:
            skip.append(rel)
            continue
        
        src_path = os.path.join(source_path, rel)
        tgt_path = os.path.join(target_path, rel)
        
        if not os.path.exists(tgt_path):
            log(f"警告: 目標不存在 {rel}", 'WARN')
            skip.append(rel)
            continue
        
        if rel not in target_modified:
            direct_copy.append(rel)
        else:
            # 檢查是否相同
            if md5_file(src_path) != md5_file(tgt_path):
                overlap.append(rel)
            # 否則跳過（內容相同）
    
    # 新增檔案
    for rel in sorted(source_new):
        new_files.append(rel)
    
    return direct_copy, overlap, new_files, skip


def get_svn_base_file(source_path, rel_path, output_dir):
    """取得 SVN base 版本檔案"""
    full_path = os.path.join(source_path, rel_path)
    
    # 取得檔案的 base revision
    result = subprocess.run(
        ['svn', 'info', full_path],
        capture_output=True,
        text=True,
        encoding='cp950',
        errors='replace'
    )
    
    base_rev = None
    for line in result.stdout.splitlines():
        if line.startswith('Last Changed Rev:'):
            base_rev = line.split(':')[1].strip()
            break
    
    if not base_rev:
        return None
    
    # 組合 SVN URL
    file_url = f"{SVN_REPO_URL}/{rel_path.replace(os.sep, '/')}"
    
    # 輸出路徑
    output_path = os.path.join(output_dir, rel_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # 取得 base 版本
    result = subprocess.run(
        ['svn', 'cat', '-r', base_rev, file_url],
        capture_output=True
    )
    
    if result.returncode == 0:
        with open(output_path, 'wb') as f:
            f.write(result.stdout)
        return output_path
    
    return None


def backup_file(path, developer):
    """建立備份"""
    if os.path.exists(path):
        bak = path + f".pre_{developer.lower()}_merge.bak"
        shutil.copy2(path, bak)
        return bak
    return None


def copy_file_cp950_safe(src, dst):
    """複製檔案（保持原始編碼）"""
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy2(src, dst)


def three_way_merge(base_path, source_path, target_path, output_path):
    """
    執行 3-way merge
    回傳: (success, conflicts_count)
    """
    script = os.path.join(SCRIPT_DIR, 'three_way_merge.py')
    
    result = subprocess.run(
        [PYTHON_PATH, script,
         '--base', base_path,
         '--source', source_path,
         '--target', target_path,
         '--output', output_path],
        capture_output=True,
        text=True
    )
    
    # 解析輸出找衝突數
    conflicts = 0
    for line in result.stdout.splitlines():
        if 'Conflicts:' in line:
            try:
                conflicts = int(line.split(':')[1].strip())
            except:
                pass
    
    return result.returncode == 0, conflicts


def normalize_whitespace(path, tab_spaces=4):
    """
    空白標準化
    回傳: True if changed
    """
    try:
        with open(path, 'rb') as f:
            raw = f.read()
        
        text = raw.decode('cp950', errors='replace')
        original = text
        
        # Tab -> 空格
        text = text.replace('\t', ' ' * tab_spaces)
        
        # 處理每行
        lines = text.split('\n')
        for i, line in enumerate(lines):
            if line.endswith('\r'):
                body = line[:-1].rstrip()
                lines[i] = body + '\r'
            else:
                lines[i] = line.rstrip()
        
        # 移除檔尾空行
        while lines and (lines[-1] == '' or lines[-1] == '\r'):
            lines.pop()
        
        # 壓縮連續空行
        compressed = []
        blank_count = 0
        for line in lines:
            is_blank = (line == '' or line == '\r')
            if is_blank:
                blank_count += 1
                if blank_count <= 1:
                    compressed.append(line)
            else:
                blank_count = 0
                compressed.append(line)
        
        text = '\n'.join(compressed)
        
        if text != original:
            with open(path, 'wb') as f:
                f.write(text.encode('cp950', errors='replace'))
            return True
        
        return False
    
    except Exception as e:
        log(f"空白標準化失敗 {path}: {e}", 'WARN')
        return False


def run_bcb_build(target_path):
    """
    執行 BCB6 編譯
    回傳: (success, errors, warnings)
    """
    log("執行 BCB6 編譯...")

    build_script = os.path.join(SCRIPT_DIR, 'build_verify_safe.ps1')
    log_path = os.path.join(target_path, 'build.log')

    result = subprocess.run(
        [
            'powershell',
            '-NoProfile',
            '-ExecutionPolicy', 'Bypass',
            '-File', build_script,
            '-TargetPath', target_path,
            '-BprFile', 'HT9045.bpr',
            '-LogPath', log_path,
        ],
        capture_output=True,
    )

    output = result.stdout.decode('cp950', errors='replace')
    output += result.stderr.decode('cp950', errors='replace')

    if not os.path.exists(log_path):
        with open(log_path, 'w', encoding='utf-8') as f:
            f.write(output)

    # 計算 errors/warnings
    build_output = output
    if os.path.exists(log_path):
        with open(log_path, 'r', encoding='utf-8', errors='replace') as f:
            build_output = f.read()

    errors = len(re.findall(r'Error E\d+', build_output))
    warnings = len(re.findall(r'Warning W\d+', build_output))
    
    # 檢查 EXE 是否產生
    exe_path = os.path.join(HT9045_ROOT, 'EXE', 'HT9045.exe')
    success = (result.returncode == 0) and os.path.exists(exe_path) and errors == 0
    
    return success, errors, warnings, log_path


def generate_report(data, output_dir=None):
    """產生合併報告"""
    developer = data['developer']
    date_str = datetime.now().strftime('%Y%m%d')
    year = datetime.now().strftime('%Y')

    # 固定輸出到 D:\HT9045\Merge Report\YYYY
    report_root = REPORT_ROOT
    report_dir = os.path.join(report_root, year)
    os.makedirs(report_dir, exist_ok=True)
    
    md_path = os.path.join(report_dir, f"MergeReport_{developer}_{date_str}.md")
    
    # 產生 Markdown
    lines = [
        f"# HT9011UC {developer} 版本合併報告",
        "",
        f"**日期**：{datetime.now().strftime('%Y-%m-%d')}  ",
        f"**來源版本**：{os.path.basename(data['source_path'])}  ",
        f"**目標版本**：{os.path.basename(data['target_path'])}  ",
        f"**SVN Base Revision**：{data.get('base_revision', 'N/A')}  ",
        f"**執行者**：AI Merge Assistant",
        "",
        "---",
        "",
        "## 1. 修改概覽",
        "",
        "| 指標 | 數量 |",
        "|------|------|",
        f"| 來源修改檔案數 | {data['source_modified_count']} |",
        f"| 目標修改檔案數 | {data['target_modified_count']} |",
        f"| 直接複製 | {len(data['direct_copy'])} |",
        f"| 3-way Merge | {len(data['overlap'])} |",
        f"| 新增檔案 | {len(data['new_files'])} |",
        f"| 跳過 | {len(data['skip'])} |",
        f"| 空白標準化 | {data['normalized_count']} |",
        f"| 編譯結果 | {'**SUCCESS**' if data['build_success'] else '**FAILED**'} ({data['build_errors']} errors, {data['build_warnings']} warnings) |",
        "",
        "---",
        "",
        "## 2. 直接複製檔案",
        "",
        "| 檔案 |",
        "|------|",
    ]
    
    for f in data['direct_copy']:
        lines.append(f"| {f} |")
    
    if data['overlap']:
        lines.extend([
            "",
            "## 3. 3-way Merge 檔案",
            "",
            "| 檔案 | 衝突數 |",
            "|------|--------|",
        ])
        for f, conflicts in data['overlap_details']:
            lines.append(f"| {f} | {conflicts} |")
    
    if data['new_files']:
        lines.extend([
            "",
            "## 4. 新增檔案",
            "",
            "| 檔案 |",
            "|------|",
        ])
        for f in data['new_files']:
            lines.append(f"| {f} |")
    
    lines.extend([
        "",
        "---",
        "",
        f"**報告產生時間**：{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
    ])
    
    with open(md_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))
    
    log(f"報告已產生: {md_path}")
    
    # 轉換為 HTML
    html_script = os.path.join(SCRIPT_DIR, 'md2html.py')
    html_path = md_path.replace('.md', '.html')
    
    subprocess.run([PYTHON_PATH, html_script, md_path, html_path], capture_output=True)
    
    return md_path, html_path


def main():
    parser = argparse.ArgumentParser(
        description='HT9045/HT9011UC 程式碼合併工具',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--source', required=True, help='來源（開發者）路徑')
    parser.add_argument('--target', required=True, help='目標（Steven）路徑')
    parser.add_argument('--developer', required=True, help='開發者名稱')
    parser.add_argument('--skip-build', action='store_true', help='跳過編譯驗證')
    parser.add_argument('--skip-normalize', action='store_true', help='跳過空白標準化')
    parser.add_argument('--dry-run', action='store_true', help='只分析不執行')
    
    args = parser.parse_args()
    
    source_path = os.path.abspath(args.source)
    target_path = os.path.abspath(args.target)
    developer = args.developer
    
    print("=" * 60)
    print(f"HT9045/HT9011UC 程式碼合併")
    print(f"時間: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)
    print(f"來源: {source_path}")
    print(f"目標: {target_path}")
    print(f"開發者: {developer}")
    print()
    
    # Step 0: SVN 檢查與修復
    log("Step 0: 檢查 SVN 狀態...")
    
    if not repair_svn_if_needed(source_path):
        log("來源 SVN 狀態異常，無法繼續", 'ERROR')
        return 1
    
    if not repair_svn_if_needed(target_path):
        log("目標 SVN 狀態異常，無法繼續", 'ERROR')
        return 1
    
    # Step 1: SVN 分析
    log("Step 1: 分析 SVN 修改...")
    
    source_modified = get_svn_modified_files(source_path)
    target_modified = get_svn_modified_files(target_path)
    
    log(f"來源修改: {len(source_modified)} 檔案")
    log(f"目標修改: {len(target_modified)} 檔案")
    
    # Step 2: 重疊偵測
    log("Step 2: 重疊偵測...")
    
    direct_copy, overlap, new_files, skip = analyze_overlap(source_path, target_path)
    
    log(f"直接複製: {len(direct_copy)}")
    log(f"需 3-way merge: {len(overlap)}")
    log(f"新增檔案: {len(new_files)}")
    log(f"跳過: {len(skip)}")
    
    if args.dry_run:
        print("\n=== Dry Run 結束 ===")
        print("直接複製:", direct_copy)
        print("3-way merge:", overlap)
        print("新增:", new_files)
        print("跳過:", skip)
        return 0
    
    # Step 3: 執行合併
    log("Step 3: 執行合併...")
    
    # 準備 SVN base 目錄
    base_dir = os.path.join(SVN_TEMP_DIR, f"merge_base_{developer.lower()}")
    os.makedirs(base_dir, exist_ok=True)
    
    # 直接複製
    for rel in direct_copy:
        src = os.path.join(source_path, rel)
        dst = os.path.join(target_path, rel)
        backup_file(dst, developer)
        copy_file_cp950_safe(src, dst)
        log(f"  COPY: {rel}")
    
    # 3-way merge
    overlap_details = []
    for rel in overlap:
        log(f"  MERGE: {rel}")
        
        src = os.path.join(source_path, rel)
        dst = os.path.join(target_path, rel)
        
        # 取得 base
        base_path = get_svn_base_file(source_path, rel, base_dir)
        if not base_path:
            log(f"    無法取得 SVN base，跳過", 'WARN')
            continue
        
        # 備份
        backup_file(dst, developer)
        
        # 執行 merge
        success, conflicts = three_way_merge(base_path, src, dst, dst)
        overlap_details.append((rel, conflicts))
        
        if conflicts > 0:
            log(f"    {conflicts} 個衝突需手動解決", 'WARN')
    
    # 新增檔案
    for rel in new_files:
        src = os.path.join(source_path, rel)
        dst = os.path.join(target_path, rel)
        copy_file_cp950_safe(src, dst)
        log(f"  NEW: {rel}")
    
    # Step 4: 空白標準化
    normalized_count = 0
    if not args.skip_normalize:
        log("Step 4: 空白標準化...")
        
        all_modified = direct_copy + overlap + new_files
        for rel in all_modified:
            path = os.path.join(target_path, rel)
            ext = os.path.splitext(rel)[1].lower()
            if ext in SOURCE_EXTENSIONS and os.path.exists(path):
                if normalize_whitespace(path):
                    normalized_count += 1
        
        log(f"標準化: {normalized_count} 檔案")
    
    # Step 5: BCB6 編譯
    build_success = False
    build_errors = 0
    build_warnings = 0
    build_log = ""
    
    if not args.skip_build:
        log("Step 5: BCB6 編譯驗證...")
        build_success, build_errors, build_warnings, build_log = run_bcb_build(target_path)
        
        if build_success:
            log(f"編譯成功 ({build_warnings} warnings)")
        else:
            log(f"編譯失敗 ({build_errors} errors, {build_warnings} warnings)", 'ERROR')
            log(f"Build log: {build_log}")
    else:
        log("Step 5: 跳過編譯")
        build_success = True  # 假設成功
    
    # Step 6: 產生報告
    log("Step 6: 產生報告...")
    
    report_data = {
        'developer': developer,
        'source_path': source_path,
        'target_path': target_path,
        'source_modified_count': len(source_modified),
        'target_modified_count': len(target_modified),
        'direct_copy': direct_copy,
        'overlap': overlap,
        'overlap_details': overlap_details,
        'new_files': new_files,
        'skip': skip,
        'normalized_count': normalized_count,
        'build_success': build_success,
        'build_errors': build_errors,
        'build_warnings': build_warnings,
    }
    
    md_path, html_path = generate_report(report_data)
    
    # 完成
    print()
    print("=" * 60)
    print("合併完成！")
    print("=" * 60)
    print(f"直接複製: {len(direct_copy)} 檔案")
    print(f"3-way merge: {len(overlap)} 檔案")
    print(f"新增檔案: {len(new_files)} 檔案")
    print(f"空白標準化: {normalized_count} 檔案")
    print(f"編譯: {'成功' if build_success else '失敗'}")
    print(f"報告: {md_path}")
    
    return 0 if build_success else 1


if __name__ == '__main__':
    sys.exit(main())
