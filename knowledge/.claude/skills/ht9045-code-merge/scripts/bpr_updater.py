#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HT9045 .bpr 專案檔更新工具

用法:
    bpr_updater.py add --bpr <專案檔> --file <檔案路徑>
    bpr_updater.py remove --bpr <專案檔> --file <檔案路徑>
    bpr_updater.py list --bpr <專案檔>

功能:
    新增或移除 .cpp/.h 檔案到 BCB6 專案檔
"""

import argparse
import os
import re
import sys
from datetime import datetime


def read_bpr(path):
    """讀取 .bpr 檔案（cp950 編碼）"""
    with open(path, 'rb') as f:
        raw = f.read()
    return raw.decode('cp950', errors='replace')


def write_bpr(path, content):
    """寫入 .bpr 檔案（cp950 編碼）"""
    # 先備份
    backup = path + f".bak_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    if os.path.exists(path):
        with open(path, 'rb') as f:
            with open(backup, 'wb') as out:
                out.write(f.read())
    
    with open(path, 'wb') as f:
        f.write(content.encode('cp950', errors='replace'))
    
    return backup


def get_relative_path(base_dir, file_path):
    """取得相對於專案目錄的路徑"""
    file_abs = os.path.abspath(file_path)
    base_abs = os.path.abspath(base_dir)
    
    try:
        rel = os.path.relpath(file_abs, base_abs)
        return rel
    except ValueError:
        return file_path


def add_file_to_bpr(bpr_path, file_path):
    """
    將檔案加入 .bpr 專案
    回傳: (success, message)
    """
    if not os.path.exists(bpr_path):
        return False, f"專案檔不存在: {bpr_path}"
    
    content = read_bpr(bpr_path)
    
    # 取得相對路徑
    base_dir = os.path.dirname(bpr_path)
    rel_path = get_relative_path(base_dir, file_path)
    
    # 檔案名稱（不含副檔名）
    file_name = os.path.splitext(os.path.basename(file_path))[0]
    ext = os.path.splitext(file_path)[1].lower()
    
    # 檢查是否已存在
    if rel_path.replace('\\', '/') in content.replace('\\', '/'):
        return False, f"檔案已存在於專案中: {rel_path}"
    
    # 1. 加入 OBJFILES（僅 .cpp）
    if ext == '.cpp':
        obj_name = f"..\\Obj\\{file_name}.obj"
        
        # 找 OBJFILES 區段並加入
        objfiles_match = re.search(r'(<OBJFILES value="[^"]+)', content)
        if objfiles_match:
            old = objfiles_match.group(1)
            # 在最後一個 .obj 後加入
            new = old.rstrip() + f" {obj_name}"
            content = content.replace(old, new, 1)
    
    # 2. 加入 FILE 區段
    # 找到最後一個 <FILE FILENAME= 區段
    file_entries = list(re.finditer(r'<FILE FILENAME="[^"]+\.cpp"[^/]*/>', content))
    
    if ext == '.cpp':
        container = 'CCompiler'
    elif ext == '.h':
        container = ''
    else:
        container = ''
    
    # 格式化路徑（使用反斜線）
    rel_path_fmt = rel_path.replace('/', '\\')
    
    new_entry = f'      <FILE FILENAME="{rel_path_fmt}" FORMNAME="" UNITNAME="{file_name}" CONTAINERID="{container}" DESIGNCLASS="" LOCALCOMMAND=""/>'
    
    if file_entries:
        last_entry = file_entries[-1]
        insert_pos = last_entry.end()
        content = content[:insert_pos] + '\n' + new_entry + content[insert_pos:]
    else:
        # 找 </FILELIST> 並在之前插入
        filelist_end = content.find('</FILELIST>')
        if filelist_end > 0:
            content = content[:filelist_end] + new_entry + '\n' + content[filelist_end:]
    
    # 寫入
    backup = write_bpr(bpr_path, content)
    
    return True, f"已加入 {rel_path}（備份: {backup}）"


def remove_file_from_bpr(bpr_path, file_path):
    """
    從 .bpr 專案移除檔案
    回傳: (success, message)
    """
    if not os.path.exists(bpr_path):
        return False, f"專案檔不存在: {bpr_path}"
    
    content = read_bpr(bpr_path)
    original = content
    
    base_dir = os.path.dirname(bpr_path)
    rel_path = get_relative_path(base_dir, file_path)
    file_name = os.path.splitext(os.path.basename(file_path))[0]
    
    # 移除 OBJFILES 中的項目
    obj_pattern = rf'\.\.\\Obj\\{re.escape(file_name)}\.obj\s*'
    content = re.sub(obj_pattern, '', content)
    
    # 移除 FILE 項目
    file_pattern = rf'^\s*<FILE FILENAME="[^"]*{re.escape(file_name)}[^"]*"[^/]*/>\s*$'
    content = re.sub(file_pattern, '', content, flags=re.MULTILINE)
    
    if content == original:
        return False, f"檔案不在專案中: {rel_path}"
    
    # 清理多餘空行
    content = re.sub(r'\n\s*\n\s*\n', '\n\n', content)
    
    backup = write_bpr(bpr_path, content)
    
    return True, f"已移除 {rel_path}（備份: {backup}）"


def list_files_in_bpr(bpr_path):
    """
    列出 .bpr 中的所有檔案
    """
    if not os.path.exists(bpr_path):
        print(f"專案檔不存在: {bpr_path}")
        return
    
    content = read_bpr(bpr_path)
    
    # 找所有 FILE 項目
    files = []
    for match in re.finditer(r'<FILE FILENAME="([^"]+)"[^>]*CONTAINERID="([^"]*)"', content):
        filename = match.group(1)
        container = match.group(2)
        files.append((filename, container))
    
    # 分類顯示
    cpp_files = [f for f, c in files if f.endswith('.cpp')]
    h_files = [f for f, c in files if f.endswith('.h')]
    other_files = [f for f, c in files if not f.endswith('.cpp') and not f.endswith('.h')]
    
    print(f"專案: {bpr_path}")
    print(f"總檔案數: {len(files)}")
    print()
    
    print(f"=== C++ 原始碼 ({len(cpp_files)}) ===")
    for f in sorted(cpp_files):
        print(f"  {f}")
    
    print(f"\n=== 標頭檔 ({len(h_files)}) ===")
    for f in sorted(h_files)[:20]:  # 只顯示前20個
        print(f"  {f}")
    if len(h_files) > 20:
        print(f"  ... 還有 {len(h_files) - 20} 個")
    
    print(f"\n=== 其他 ({len(other_files)}) ===")
    for f in sorted(other_files):
        print(f"  {f}")


def main():
    parser = argparse.ArgumentParser(
        description='HT9045 .bpr 專案檔更新工具'
    )
    
    subparsers = parser.add_subparsers(dest='command', required=True)
    
    # add 子命令
    p_add = subparsers.add_parser('add', help='加入檔案到專案')
    p_add.add_argument('--bpr', required=True, help='.bpr 專案檔路徑')
    p_add.add_argument('--file', required=True, help='要加入的檔案路徑')
    
    # remove 子命令
    p_remove = subparsers.add_parser('remove', help='從專案移除檔案')
    p_remove.add_argument('--bpr', required=True, help='.bpr 專案檔路徑')
    p_remove.add_argument('--file', required=True, help='要移除的檔案路徑')
    
    # list 子命令
    p_list = subparsers.add_parser('list', help='列出專案中的檔案')
    p_list.add_argument('--bpr', required=True, help='.bpr 專案檔路徑')
    
    args = parser.parse_args()
    
    if args.command == 'add':
        success, msg = add_file_to_bpr(args.bpr, args.file)
        print(msg)
        return 0 if success else 1
    
    elif args.command == 'remove':
        success, msg = remove_file_from_bpr(args.bpr, args.file)
        print(msg)
        return 0 if success else 1
    
    elif args.command == 'list':
        list_files_in_bpr(args.bpr)
        return 0


if __name__ == '__main__':
    sys.exit(main())
