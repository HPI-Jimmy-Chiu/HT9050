#!/usr/bin/env python3
"""
HT9011UC Whitespace Normalize Script
Tab -> 4 spaces, trim trailing whitespace. Preserves Big5 (CP950) encoding.
用法: whitespace_normalize.py <target_dir> [--extensions .cpp .h .c .dfm]
"""
import argparse
import os
import sys

SOURCE_EXTENSIONS = {'.cpp', '.h', '.c', '.asm', '.dfm', '.rc'}

def normalize_file(path, tab_spaces=4):
    """Normalize whitespace in a single CP950 file. Returns True if changed."""
    try:
        with open(path, 'rb') as f:
            raw = f.read()
        text = raw.decode('cp950', errors='replace')
    except Exception as e:
        print(f"  SKIP (read error): {path} — {e}")
        return False

    spaces = ' ' * tab_spaces
    new_text = text.replace('\t', spaces)

    lines = new_text.split('\n')
    for i, line in enumerate(lines):
        if line.endswith('\r'):
            body = line[:-1].rstrip()
            lines[i] = body + '\r'
        else:
            lines[i] = line.rstrip()

    new_text = '\n'.join(lines)

    if new_text != text:
        new_bytes = new_text.encode('cp950', errors='replace')
        with open(path, 'wb') as f:
            f.write(new_bytes)
        return True
    return False

def main():
    parser = argparse.ArgumentParser(description='HT9011UC Whitespace Normalize')
    parser.add_argument('target', help='Target directory or single file')
    parser.add_argument('--extensions', nargs='*', default=None,
                        help='File extensions to process (default: .cpp .h .c .asm .dfm .rc)')
    parser.add_argument('--tab-spaces', type=int, default=4, help='Spaces per tab')
    args = parser.parse_args()

    exts = set(args.extensions) if args.extensions else SOURCE_EXTENSIONS

    if os.path.isfile(args.target):
        changed = normalize_file(args.target, args.tab_spaces)
        print(f"{'Changed' if changed else 'Unchanged'}: {args.target}")
        return

    changed_count = 0
    total_count = 0
    for root, dirs, files in os.walk(args.target):
        for fname in files:
            ext = os.path.splitext(fname)[1].lower()
            if ext in exts:
                fpath = os.path.join(root, fname)
                total_count += 1
                if normalize_file(fpath, args.tab_spaces):
                    changed_count += 1
                    print(f"  Cleaned: {os.path.relpath(fpath, args.target)}")

    print(f"\nTotal: {total_count} files, Changed: {changed_count}")

if __name__ == '__main__':
    main()
