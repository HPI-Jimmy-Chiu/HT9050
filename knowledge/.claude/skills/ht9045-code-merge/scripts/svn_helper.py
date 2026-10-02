#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HT9045 SVN Helper Script
自動檢查、修復 SVN 工作複本狀態

用法:
    svn_helper.py check --path <工作複本路徑>
    svn_helper.py repair --path <工作複本路徑> [--revision <rev>]
    svn_helper.py clone-svn --source <來源路徑> --target <目標路徑>
    svn_helper.py parse-folder --name <資料夾名稱>
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime

# === 常數定義 ===
SVN_REPO_URL = "file:///U:/SourceCode/SVN/HT9011UC_Code_V3.20"
HT9045_ROOT = r"D:\HT9045"
SVN_TEMP_DIR = r"D:\HT9045\HT9045_SVN_TempFile"

# 資料夾命名規則正則
FOLDER_PATTERN = re.compile(
    r'HT9011UC_Code_V(\d+)\.(\d+)\.(\d+)\.(\d+)_(\d{8})(?:_(.+))?'
)

class SvnStatus:
    OK = "OK"
    NO_SVN = "NO_SVN"
    CORRUPTED = "CORRUPTED"
    LOCKED = "LOCKED"
    UNKNOWN = "UNKNOWN"


def parse_folder_name(folder_name):
    """
    從資料夾名稱解析版本資訊
    回傳: dict 或 None
    """
    match = FOLDER_PATTERN.match(folder_name)
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
    """
    檢查 SVN 工作複本狀態
    回傳: (status, message)
    """
    if not os.path.exists(path):
        return (SvnStatus.UNKNOWN, f"路徑不存在: {path}")
    
    svn_dir = os.path.join(path, '.svn')
    if not os.path.exists(svn_dir):
        return (SvnStatus.NO_SVN, "找不到 .svn 資料夾")
    
    # 嘗試執行 svn info
    try:
        result = subprocess.run(
            ['svn', 'info', path],
            capture_output=True,
            text=True,
            encoding='cp950',
            errors='replace',
            timeout=30
        )
        
        if result.returncode == 0:
            # 解析版本資訊
            revision = None
            url = None
            for line in result.stdout.splitlines():
                if line.startswith('Revision:'):
                    revision = line.split(':')[1].strip()
                elif line.startswith('URL:'):
                    url = line.split(':', 1)[1].strip()
            
            return (SvnStatus.OK, f"SVN 正常 (Revision: {revision})")
        
        else:
            stderr = result.stderr.strip()
            if 'E155021' in stderr:
                return (SvnStatus.CORRUPTED, "SVN client 版本太舊")
            elif 'E155004' in stderr or 'locked' in stderr.lower():
                return (SvnStatus.LOCKED, "工作複本被鎖定")
            elif 'E155007' in stderr or 'E155036' in stderr:
                return (SvnStatus.CORRUPTED, f"SVN 資料損毀: {stderr}")
            else:
                return (SvnStatus.CORRUPTED, f"SVN 錯誤: {stderr}")
    
    except subprocess.TimeoutExpired:
        return (SvnStatus.CORRUPTED, "SVN 命令逾時")
    except FileNotFoundError:
        return (SvnStatus.UNKNOWN, "找不到 svn 命令")
    except Exception as e:
        return (SvnStatus.UNKNOWN, f"未知錯誤: {e}")


def find_reference_working_copy(target_revision, exclude_path=None):
    """
    在 HT9045 目錄中查找相同 Revision 的其他工作複本
    回傳: path 或 None
    """
    candidates = []
    
    for item in os.listdir(HT9045_ROOT):
        full_path = os.path.join(HT9045_ROOT, item)
        if not os.path.isdir(full_path):
            continue
        
        if exclude_path and os.path.normpath(full_path) == os.path.normpath(exclude_path):
            continue
        
        info = parse_folder_name(item)
        if info and info['revision'] == target_revision:
            # 檢查此路徑的 SVN 狀態
            status, _ = check_svn_status(full_path)
            if status == SvnStatus.OK:
                candidates.append((full_path, info))
    
    if not candidates:
        return None
    
    # 優先選擇 Steven 版本
    for path, info in candidates:
        if info.get('developer', '').lower() == 'steven':
            return path
    
    # 否則返回第一個可用的
    return candidates[0][0]


def copy_svn_folder(source_path, target_path):
    """
    複製 .svn 資料夾
    """
    source_svn = os.path.join(source_path, '.svn')
    target_svn = os.path.join(target_path, '.svn')
    
    if not os.path.exists(source_svn):
        raise FileNotFoundError(f"來源 .svn 不存在: {source_svn}")
    
    # 如果目標已存在，先備份
    if os.path.exists(target_svn):
        backup_svn = target_svn + f".bak_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        print(f"  備份現有 .svn 至: {backup_svn}")
        shutil.move(target_svn, backup_svn)
    
    print(f"  複製 .svn: {source_svn} -> {target_svn}")
    shutil.copytree(source_svn, target_svn)
    
    return True


def run_svn_cleanup(path):
    """
    執行 svn cleanup
    """
    try:
        result = subprocess.run(
            ['svn', 'cleanup', path],
            capture_output=True,
            text=True,
            encoding='cp950',
            errors='replace',
            timeout=60
        )
        return result.returncode == 0
    except Exception as e:
        print(f"  cleanup 失敗: {e}")
        return False


def repair_svn_working_copy(path, target_revision=None):
    """
    修復 SVN 工作複本
    """
    print(f"\n=== 修復 SVN 工作複本 ===")
    print(f"路徑: {path}")
    
    # 1. 解析資料夾名稱取得 revision（如果未指定）
    if target_revision is None:
        folder_name = os.path.basename(path)
        info = parse_folder_name(folder_name)
        if info:
            target_revision = info['revision']
            print(f"從資料夾名稱解析 Revision: {target_revision}")
        else:
            print("無法從資料夾名稱解析 Revision，請使用 --revision 參數指定")
            return False
    
    # 2. 查找參考工作複本
    print(f"\n查找 Revision {target_revision} 的參考來源...")
    ref_path = find_reference_working_copy(target_revision, exclude_path=path)
    
    if ref_path:
        print(f"找到參考來源: {ref_path}")
        
        # 3. 複製 .svn 資料夾
        try:
            copy_svn_folder(ref_path, path)
        except Exception as e:
            print(f"複製 .svn 失敗: {e}")
            return False
        
        # 4. 執行 cleanup
        print("\n執行 svn cleanup...")
        run_svn_cleanup(path)
        
        # 5. 驗證修復結果
        print("\n驗證修復結果...")
        status, msg = check_svn_status(path)
        if status == SvnStatus.OK:
            print(f"✓ 修復成功: {msg}")
            return True
        else:
            print(f"✗ 修復後仍有問題: {msg}")
            return False
    
    else:
        print(f"找不到 Revision {target_revision} 的參考來源")
        print("\n可嘗試的解決方案:")
        print(f"  1. 從 SVN checkout 一份乾淨版本:")
        print(f"     svn checkout -r {target_revision} \"{SVN_REPO_URL}\" \"<目標路徑>\"")
        print(f"  2. 手動複製已知正常的 .svn 資料夾")
        return False


def cmd_check(args):
    """check 子命令"""
    path = os.path.abspath(args.path)
    print(f"檢查 SVN 狀態: {path}\n")
    
    # 解析資料夾名稱
    folder_name = os.path.basename(path)
    info = parse_folder_name(folder_name)
    if info:
        print(f"版本資訊:")
        print(f"  版本號: {info['full_version']}")
        print(f"  Revision: {info['revision']}")
        print(f"  日期: {info['date']}")
        print(f"  開發者: {info['developer']}")
        print()
    
    # 檢查 SVN 狀態
    status, msg = check_svn_status(path)
    
    print(f"SVN 狀態: {status}")
    print(f"詳細資訊: {msg}")
    
    if status == SvnStatus.OK:
        # 顯示修改檔案數
        try:
            result = subprocess.run(
                ['svn', 'status', path],
                capture_output=True,
                text=True,
                encoding='cp950',
                errors='replace'
            )
            modified = len([l for l in result.stdout.splitlines() if l.startswith('M')])
            added = len([l for l in result.stdout.splitlines() if l.startswith('A')])
            deleted = len([l for l in result.stdout.splitlines() if l.startswith('D')])
            unversioned = len([l for l in result.stdout.splitlines() if l.startswith('?')])
            
            print(f"\n工作複本變更:")
            print(f"  修改: {modified}")
            print(f"  新增: {added}")
            print(f"  刪除: {deleted}")
            print(f"  未版控: {unversioned}")
        except Exception:
            pass
    
    elif status in (SvnStatus.NO_SVN, SvnStatus.CORRUPTED):
        print("\n建議執行修復:")
        print(f"  python svn_helper.py repair --path \"{path}\"")
    
    elif status == SvnStatus.LOCKED:
        print("\n建議執行 cleanup:")
        print(f"  svn cleanup \"{path}\"")
    
    return 0 if status == SvnStatus.OK else 1


def cmd_repair(args):
    """repair 子命令"""
    path = os.path.abspath(args.path)
    revision = args.revision
    
    success = repair_svn_working_copy(path, revision)
    return 0 if success else 1


def cmd_clone_svn(args):
    """clone-svn 子命令"""
    source = os.path.abspath(args.source)
    target = os.path.abspath(args.target)
    
    print(f"複製 .svn 資料夾")
    print(f"  來源: {source}")
    print(f"  目標: {target}")
    
    try:
        copy_svn_folder(source, target)
        run_svn_cleanup(target)
        
        status, msg = check_svn_status(target)
        print(f"\n結果: {status} - {msg}")
        return 0 if status == SvnStatus.OK else 1
    
    except Exception as e:
        print(f"\n錯誤: {e}")
        return 1


def cmd_parse_folder(args):
    """parse-folder 子命令"""
    folder_name = args.name
    info = parse_folder_name(folder_name)
    
    if info:
        print(f"資料夾名稱: {folder_name}")
        print(f"解析結果:")
        for key, value in info.items():
            print(f"  {key}: {value}")
        return 0
    else:
        print(f"無法解析資料夾名稱: {folder_name}")
        print(f"預期格式: HT9011UC_Code_V<Major>.<Minor>.<Revision>.<Patch>_<Date>_<Developer>")
        return 1


def cmd_list_versions(args):
    """list-versions 子命令 - 列出所有可用版本"""
    print(f"掃描 {HT9045_ROOT} 中的工作複本...\n")
    
    versions = []
    for item in sorted(os.listdir(HT9045_ROOT)):
        full_path = os.path.join(HT9045_ROOT, item)
        if not os.path.isdir(full_path):
            continue
        
        info = parse_folder_name(item)
        if info:
            status, _ = check_svn_status(full_path)
            info['path'] = full_path
            info['svn_status'] = status
            versions.append(info)
    
    if not versions:
        print("找不到符合命名規則的資料夾")
        return 0
    
    # 依 revision 分組
    by_rev = {}
    for v in versions:
        rev = v['revision']
        if rev not in by_rev:
            by_rev[rev] = []
        by_rev[rev].append(v)
    
    print(f"{'Rev':<6} {'Developer':<15} {'SVN':<12} {'Date':<10} Path")
    print("-" * 100)
    
    for rev in sorted(by_rev.keys(), reverse=True):
        for v in by_rev[rev]:
            status_mark = "[OK]" if v['svn_status'] == SvnStatus.OK else "[NG]"
            print(f"{rev:<6} {v['developer']:<15} {status_mark:<12} {v['date']:<10} {v['path']}")
    
    return 0


def main():
    parser = argparse.ArgumentParser(
        description='HT9045 SVN Helper - 檢查與修復 SVN 工作複本',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
範例:
  %(prog)s check --path "D:\\HT9045\\HT9011UC_Code_V3.33.898.0_20260313_Steven"
  %(prog)s repair --path "D:\\HT9045\\HT9011UC_Code_V3.33.898.1_RogerYang_20260317"
  %(prog)s list-versions
  %(prog)s parse-folder --name "HT9011UC_Code_V3.33.898.0_20260313_Steven"
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', required=True)
    
    # check 子命令
    p_check = subparsers.add_parser('check', help='檢查 SVN 工作複本狀態')
    p_check.add_argument('--path', required=True, help='工作複本路徑')
    
    # repair 子命令
    p_repair = subparsers.add_parser('repair', help='修復 SVN 工作複本')
    p_repair.add_argument('--path', required=True, help='工作複本路徑')
    p_repair.add_argument('--revision', type=int, help='指定 Revision（若未指定則從資料夾名稱解析）')
    
    # clone-svn 子命令
    p_clone = subparsers.add_parser('clone-svn', help='從另一個工作複本複製 .svn 資料夾')
    p_clone.add_argument('--source', required=True, help='來源路徑（有正常 .svn）')
    p_clone.add_argument('--target', required=True, help='目標路徑（需要 .svn）')
    
    # parse-folder 子命令
    p_parse = subparsers.add_parser('parse-folder', help='解析資料夾名稱中的版本資訊')
    p_parse.add_argument('--name', required=True, help='資料夾名稱')
    
    # list-versions 子命令
    p_list = subparsers.add_parser('list-versions', help='列出所有可用版本')
    
    args = parser.parse_args()
    
    if args.command == 'check':
        return cmd_check(args)
    elif args.command == 'repair':
        return cmd_repair(args)
    elif args.command == 'clone-svn':
        return cmd_clone_svn(args)
    elif args.command == 'parse-folder':
        return cmd_parse_folder(args)
    elif args.command == 'list-versions':
        return cmd_list_versions(args)


if __name__ == '__main__':
    sys.exit(main())
