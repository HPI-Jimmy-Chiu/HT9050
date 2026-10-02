"""
restore_changeToFloat.py
=========================
Scan SVN diff output for lines where ChangeToFloatNonPcnt was removed
(replaced by bare division). For each such case, restore the protected
version in the working copy.

Strategy:
  1. For each modified file, run `svn diff` and parse unified-diff hunks.
  2. Identify hunks where a removed line (-) contains ChangeToFloatNonPcnt
     and the added line (+) has a bare division instead.
  3. In the working copy, find the bare-division line and replace it with
     the protected version.
  4. Also detect entire deleted blocks (functions) containing ChangeToFloatNonPcnt.

All source files are Big5 (CP950).
"""

import argparse
import os
import re
import subprocess
import sys

def parse_args():
    parser = argparse.ArgumentParser(
        description="Scan SVN diff and restore ChangeToFloatNonPcnt calls that were "
                    "accidentally replaced by bare division during merge."
    )
    parser.add_argument(
        "--project", required=True,
        help="目標版本根路徑，例：D:\\HT9045\\HT9011UC_Code_V3.33.901.0_20260408_Steven_RogerYang"
    )
    return parser.parse_args()

_args = parse_args()
PROJECT = _args.project
LOG_FILE = os.path.join(PROJECT, "restore_changeToFloat.log")

def run_svn_diff(project_dir):
    """Run svn diff and return raw bytes."""
    result = subprocess.run(
        ["svn", "diff", project_dir],
        capture_output=True, timeout=120
    )
    return result.stdout

def decode_cp950(data):
    return data.decode("cp950", errors="replace")

def read_cp950(path):
    with open(path, "rb") as f:
        return f.read().decode("cp950", errors="replace")

def write_cp950(path, text):
    with open(path, "wb") as f:
        f.write(text.encode("cp950", errors="replace"))

def parse_diff(diff_text):
    """Parse unified diff text, yield (filepath, removed_lines, added_lines, hunk_context)."""
    current_file = None
    hunks = []
    
    lines = diff_text.splitlines(True)
    i = 0
    while i < len(lines):
        line = lines[i]
        
        # Detect file header
        if line.startswith("Index: "):
            current_file = line[7:].strip()
            i += 1
            continue
        
        # Detect hunk header
        hunk_match = re.match(r"^@@ -(\d+),?\d* \+(\d+),?\d* @@", line)
        if hunk_match and current_file:
            old_start = int(hunk_match.group(1))
            new_start = int(hunk_match.group(2))
            i += 1
            
            # Collect hunk lines
            removed = []  # (old_lineno, text)
            added = []    # (new_lineno, text)
            context = []
            old_ln = old_start
            new_ln = new_start
            
            while i < len(lines):
                l = lines[i]
                if l.startswith("@@") or l.startswith("Index: ") or l.startswith("==="):
                    break
                if l.startswith("-"):
                    removed.append((old_ln, l[1:].rstrip("\r\n")))
                    old_ln += 1
                elif l.startswith("+"):
                    added.append((new_ln, l[1:].rstrip("\r\n")))
                    new_ln += 1
                elif l.startswith(" "):
                    context.append((new_ln, l[1:].rstrip("\r\n")))
                    old_ln += 1
                    new_ln += 1
                elif l.startswith("\\"):
                    pass  # "\ No newline at end of file"
                else:
                    break
                i += 1
            
            hunks.append((current_file, removed, added, context, new_start))
            continue
        
        i += 1
    
    return hunks

def find_changeToFloat_removals(hunks):
    """From parsed hunks, find cases where ChangeToFloatNonPcnt was removed."""
    results = []  # list of (file, old_line_text, new_line_text, new_lineno)
    
    for (filepath, removed, added, context, new_start) in hunks:
        # Find removed lines with ChangeToFloatNonPcnt
        removed_ctf = [(ln, t) for (ln, t) in removed if "ChangeToFloatNonPcnt" in t]
        if not removed_ctf:
            continue
        
        # Find corresponding added lines (bare division)
        added_bare = [(ln, t) for (ln, t) in added if "/" in t and "ChangeToFloatNonPcnt" not in t and "ChangeToFloat" not in t]
        
        # For entirely deleted lines (no corresponding add), record separately
        if not added_bare and removed_ctf:
            # These are deleted lines/blocks - need full block restore
            for (ln, t) in removed_ctf:
                results.append({
                    "file": filepath,
                    "type": "deleted_block",
                    "old_line": t,
                    "old_lineno": ln,
                    "new_line": None,
                    "new_lineno": None,
                })
            continue
        
        # Try to match removed ChangeToFloatNonPcnt lines with added bare-division lines
        # by comparing the surrounding variable/expression names
        for (old_ln, old_text) in removed_ctf:
            best_match = None
            best_score = 0
            
            for (new_ln, new_text) in added_bare:
                # Extract key tokens from old line (variable names, numbers)
                old_tokens = set(re.findall(r'[A-Za-z_]\w+', old_text))
                new_tokens = set(re.findall(r'[A-Za-z_]\w+', new_text))
                
                # Remove common function/type names
                old_tokens.discard("ChangeToFloatNonPcnt")
                old_tokens.discard("double")
                new_tokens.discard("double")
                
                overlap = len(old_tokens & new_tokens)
                if overlap > best_score:
                    best_score = overlap
                    best_match = (new_ln, new_text)
            
            if best_match and best_score >= 2:
                results.append({
                    "file": filepath,
                    "type": "replaced",
                    "old_line": old_text,
                    "old_lineno": old_ln,
                    "new_line": best_match[1],
                    "new_lineno": best_match[0],
                })
    
    return results

def restore_protections(results):
    """For each replacement, find the bare division in the working copy and restore the protected version."""
    log_entries = []
    files_modified = {}
    
    # Group by file
    by_file = {}
    for r in results:
        fpath = r["file"]
        if fpath not in by_file:
            by_file[fpath] = []
        by_file[fpath].append(r)
    
    for rel_path, entries in sorted(by_file.items()):
        full_path = os.path.join(PROJECT, rel_path.replace("/", os.sep))
        if not os.path.exists(full_path):
            log_entries.append(f"SKIP (not found): {rel_path}")
            continue
        
        content = read_cp950(full_path)
        lines = content.split("\n")
        modified = False
        
        for entry in entries:
            if entry["type"] == "deleted_block":
                log_entries.append(f"INFO (deleted block): {rel_path}:{entry['old_lineno']} - {entry['old_line'].strip()}")
                continue
            
            new_line = entry["new_line"]
            old_line = entry["old_line"]
            new_lineno = entry["new_lineno"]
            
            if new_line is None:
                continue
            
            # Try exact line number first (0-indexed = new_lineno - 1)
            idx = new_lineno - 1
            found = False
            
            # Search around the expected line number (+/- 20 lines)
            for offset in range(0, 40):
                for try_idx in [idx + offset, idx - offset]:
                    if try_idx < 0 or try_idx >= len(lines):
                        continue
                    
                    current = lines[try_idx]
                    # Normalize whitespace for comparison
                    current_stripped = current.strip()
                    new_stripped = new_line.strip()
                    
                    if current_stripped == new_stripped:
                        # Found the bare division line - replace with protected version
                        # Preserve original indentation
                        indent = ""
                        for ch in current:
                            if ch in (" ", "\t"):
                                indent += ch
                            else:
                                break
                        
                        old_content = old_line.strip()
                        lines[try_idx] = indent + old_content
                        modified = True
                        found = True
                        log_entries.append(f"RESTORED: {rel_path}:{try_idx+1}")
                        log_entries.append(f"  FROM: {current.strip()}")
                        log_entries.append(f"    TO: {old_content}")
                        break
                
                if found:
                    break
            
            if not found:
                log_entries.append(f"NOT_FOUND: {rel_path}:{new_lineno} - {new_line.strip()}")
        
        if modified:
            new_content = "\n".join(lines)
            write_cp950(full_path, new_content)
            files_modified[rel_path] = True
            log_entries.append(f"SAVED: {rel_path}")
    
    return log_entries, files_modified


def main():
    args = parse_args()  # noqa: F841 — PROJECT already set at module level
    print(f"Project: {PROJECT}")
    print("Step 1: Running svn diff...")
    diff_bytes = run_svn_diff(PROJECT)
    diff_text = decode_cp950(diff_bytes)
    print(f"  Diff size: {len(diff_text)} chars")
    
    print("Step 2: Parsing diff hunks...")
    hunks = parse_diff(diff_text)
    print(f"  Hunks found: {len(hunks)}")
    
    print("Step 3: Finding ChangeToFloatNonPcnt removals...")
    results = find_changeToFloat_removals(hunks)
    print(f"  Removals found: {len(results)}")
    
    replaced = [r for r in results if r["type"] == "replaced"]
    deleted = [r for r in results if r["type"] == "deleted_block"]
    print(f"    - Replaced (bare division): {len(replaced)}")
    print(f"    - Deleted blocks: {len(deleted)}")
    
    # Show summary by file
    files = {}
    for r in results:
        f = r["file"]
        files[f] = files.get(f, 0) + 1
    print("\n  Affected files:")
    for f in sorted(files):
        print(f"    {f}: {files[f]} changes")
    
    print("\nStep 4: Restoring protections in working copy...")
    log_entries, files_modified = restore_protections(results)
    
    print(f"\n  Files modified: {len(files_modified)}")
    for f in sorted(files_modified):
        print(f"    {f}")
    
    # Write log
    with open(LOG_FILE, "w", encoding="utf-8") as f:
        f.write("ChangeToFloatNonPcnt Restoration Log\n")
        f.write("=" * 50 + "\n\n")
        for entry in log_entries:
            f.write(entry + "\n")
    
    print(f"\nLog written to: {LOG_FILE}")
    
    # Summary
    restored = sum(1 for e in log_entries if e.startswith("RESTORED:"))
    not_found = sum(1 for e in log_entries if e.startswith("NOT_FOUND:"))
    deleted_info = sum(1 for e in log_entries if e.startswith("INFO (deleted"))
    print(f"\nSummary:")
    print(f"  Restored: {restored}")
    print(f"  Not found: {not_found}")
    print(f"  Deleted blocks (need manual review): {deleted_info}")


if __name__ == "__main__":
    main()
