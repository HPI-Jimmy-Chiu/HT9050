#!/usr/bin/env python3
"""fetch_staterecord.py -- AI(W906-SRRELAY) 20261005: fetch the State Records the machine pushed to GitHub and extract the
new ones for analysis (RULINGS_20261004 #1; Jimmy 1005 A/A/A: the machine pushes, NB2-1 picks them up every round).

Where they are: GitHub HPI-Jimmy-Chiu/HT9050, branch machine/integ-ioweb, dispatch/<dir>/ with the record archive
(<yyyy-MM-dd HH_mm_ss>.zip as wb_serve's State Record makes it) and REQUEST.md (the operator's note; written by
tools/staterecord/push_staterecord.ps1).  A dispatch folder counts as a record when it holds a .zip / .7z whose name starts
with the press time, or when its name contains "staterecord" and it holds a .zip / .7z (hand-made pushes).

usage:
  python fetch_staterecord.py                 fetch + extract every record not extracted here yet (the default)
  python fetch_staterecord.py --list          list every record on the branch and whether it is extracted here
  python fetch_staterecord.py --record DIR    (re-)extract one dispatch folder, e.g. 20261005_staterecord_101530
options: --dest DIR (default D:\\HT9045_Staterecord\\from_machine)  --repo URL  --branch NAME  --json FILE
Each record lands in <dest>\\<dispatch folder>\\ (REQUEST.md + the archive's own top folder); _FETCHED.json is written
last, so a record whose extraction was cut short is extracted again next time.  The git cache is a blob-less clone in
<dest>\\_gitcache (the branch also carries machine packages of 100+ MB -- only the record archives are downloaded).
exit: 0 = ok (with or without new records), 2 = git / network / archive error.
Not golden, not built, no ctest: it only reads GitHub and writes under --dest.
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile
from datetime import datetime

REPO = "https://github.com/HPI-Jimmy-Chiu/HT9050"
BRANCH = "machine/integ-ioweb"
DEST = os.path.join("D:" + os.sep, "HT9045_Staterecord", "from_machine")
TIME_RX = re.compile(r"^\d{4}-\d{2}-\d{2} \d{2}_\d{2}_\d{2}")
ARCH_EXT = (".zip", ".7z")


class Fail(Exception):
    pass


def git(cache, *args, binary=False, out=None):
    cmd = ["git"] + (["-C", cache] if cache else []) + list(args)
    r = subprocess.run(cmd, stdout=out if out is not None else subprocess.PIPE, stderr=subprocess.PIPE)
    if r.returncode != 0:
        raise Fail("git %s failed (exit %d): %s" % (" ".join(args), r.returncode,
                                                    r.stderr.decode("utf-8", "replace").strip()))
    if out is not None:
        return None
    return r.stdout if binary else r.stdout.decode("utf-8", "replace")


def update_cache(dest, repo, branch):
    cache = os.path.join(dest, "_gitcache")
    if not os.path.isdir(os.path.join(cache, ".git")):
        os.makedirs(dest, exist_ok=True)
        if os.path.exists(cache):
            shutil.rmtree(cache)
        git(None, "clone", "-q", "--filter=blob:none", "--no-checkout", "--single-branch", "--branch", branch, repo, cache)
    else:
        git(cache, "fetch", "-q", "origin", "+refs/heads/%s:refs/remotes/origin/%s" % (branch, branch))
    rev = git(cache, "rev-parse", "refs/remotes/origin/%s" % branch).strip()
    return cache, rev


def list_records(cache, rev):
    names = git(cache, "ls-tree", "-r", "--name-only", "-z", rev, "--", "dispatch/").split("\0")
    dirs = {}
    for n in names:
        parts = n.split("/")
        if len(parts) != 3 or parts[0] != "dispatch":
            continue  # only files directly inside dispatch/<dir>/
        dirs.setdefault(parts[1], []).append(parts[2])
    recs = []
    for d, files in sorted(dirs.items()):
        archives = [f for f in files if f.lower().endswith(ARCH_EXT)]
        timed = [f for f in archives if TIME_RX.match(f)]
        if timed or ("staterecord" in d.lower() and archives):
            recs.append({"dir": d, "archives": timed or archives, "request": "REQUEST.md" in files})
    return recs


def request_md5(text):
    """{file name: MD5} from the 'File:' lines push_staterecord.ps1 writes."""
    found = {}
    for m in re.finditer(r"`([^`]+)`\s+[\d,]+\s+bytes\s+MD5\s+([0-9A-Fa-f]{32})", text):
        found[m.group(1)] = m.group(2).upper()
    return found


def note_of(text):
    m = re.search(r"## What happened \(operator note\)\s*\n(.*?)(\n## |\Z)", text, re.S)
    return (m.group(1) if m else text).strip()


def unicode_path_extra(info):
    """The Info-ZIP Unicode Path extra field (0x7075): 7-Zip on a cp950 machine stores the Big5 name in the header and
    the UTF-8 name here (measured 1005 on '2DFTPPara - <Big5 copy>.ini' of the 1001 reference record)."""
    ex, i = info.extra or b"", 0
    while i + 4 <= len(ex):
        hid = ex[i] | (ex[i + 1] << 8)
        size = ex[i + 2] | (ex[i + 3] << 8)
        data = ex[i + 4:i + 4 + size]
        if hid == 0x7075 and len(data) > 5 and data[0] == 1:
            try:
                return data[5:].decode("utf-8")
            except UnicodeDecodeError:
                return None
        i += 4 + size
    return None


def zip_name(info):
    """Entry name: the UTF-8 flag, else the Unicode Path extra field, else the raw header bytes (zipfile decoded them as
    cp437 into orig_filename) tried as UTF-8 then cp950.  Characters Windows refuses in a name become '_'."""
    if info.flag_bits & 0x800:
        name = info.filename
    else:
        name = unicode_path_extra(info)
        if name is None:
            raw = info.orig_filename.encode("cp437", "replace")
            for enc in ("utf-8", "cp950"):
                try:
                    name = raw.decode(enc)
                    break
                except UnicodeDecodeError:
                    pass
            else:
                name = info.filename
    return re.sub(r'[<>:"|?*\x00-\x1f]', "_", name.replace("\\", "/"))


def extract_zip(path, target):
    n = 0
    with zipfile.ZipFile(path) as z:
        for info in z.infolist():
            name = zip_name(info)
            if name.startswith("/") or ".." in name.split("/"):
                raise Fail("unsafe entry in %s: %r" % (path, name))
            out = os.path.join(target, *[p for p in name.split("/") if p])
            if name.endswith("/"):
                os.makedirs(out, exist_ok=True)
                continue
            os.makedirs(os.path.dirname(out), exist_ok=True)
            with z.open(info) as src, open(out, "wb") as dst:
                shutil.copyfileobj(src, dst)
            n += 1
    return n


def find_7z():
    for p in (shutil.which("7z"), os.path.join(os.environ.get("ProgramFiles", ""), "7-Zip", "7z.exe"),
              os.path.join(os.environ.get("ProgramFiles(x86)", ""), "7-Zip", "7z.exe")):
        if p and os.path.isfile(p):
            return p
    return None


def extract_one(cache, rev, rec, dest):
    d = rec["dir"]
    target = os.path.join(dest, d)
    marker = os.path.join(target, "_FETCHED.json")
    if os.path.isdir(target):
        shutil.rmtree(target)  # a cut-short or forced re-extraction starts clean
    os.makedirs(target)
    req = ""
    if rec["request"]:
        req = git(cache, "cat-file", "blob", "%s:dispatch/%s/REQUEST.md" % (rev, d), binary=True).decode("utf-8", "replace")
        with open(os.path.join(target, "REQUEST.md"), "w", encoding="utf-8", newline="\n") as f:
            f.write(req)
    md5s = request_md5(req)
    files = []
    for a in rec["archives"]:
        tmp = os.path.join(target, "_download_" + a)
        with open(tmp, "wb") as f:
            git(cache, "cat-file", "blob", "%s:dispatch/%s/%s" % (rev, d, a), out=f)
        h = hashlib.md5()
        with open(tmp, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        md5 = h.hexdigest().upper()
        if a in md5s and md5s[a] != md5:
            raise Fail("%s/%s: MD5 %s, REQUEST.md says %s" % (d, a, md5, md5s[a]))
        size = os.path.getsize(tmp)
        if a.lower().endswith(".zip"):
            count = extract_zip(tmp, target)
        else:
            z7 = find_7z()
            if not z7:
                raise Fail("%s/%s is a .7z and there is no 7-Zip on this computer" % (d, a))
            r = subprocess.run([z7, "x", "-y", "-o" + target, tmp], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if r.returncode != 0:
                raise Fail("7z x %s failed (exit %d)" % (a, r.returncode))
            count = None
        os.remove(tmp)
        files.append({"archive": a, "bytes": size, "md5": md5, "md5_checked": a in md5s, "files_extracted": count})
    info = {"dir": d, "commit": rev, "fetched": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "archives": files, "path": target}
    with open(marker, "w", encoding="utf-8") as f:
        json.dump(info, f, ensure_ascii=False, indent=1)
    info["note"] = note_of(req) if req else "(no REQUEST.md -- pushed by hand; ask the machine side what happened)"
    return info


def main():
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")  # cp950 consoles
    except AttributeError:
        pass
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--record")
    ap.add_argument("--dest", default=DEST)
    ap.add_argument("--repo", default=REPO)
    ap.add_argument("--branch", default=BRANCH)
    ap.add_argument("--json")
    a = ap.parse_args()
    try:
        cache, rev = update_cache(a.dest, a.repo, a.branch)
        recs = list_records(cache, rev)
        for r in recs:
            r["extracted_here"] = os.path.isfile(os.path.join(a.dest, r["dir"], "_FETCHED.json"))
        result = {"branch": a.branch, "head": rev, "dest": a.dest, "records": len(recs), "new": []}
        if a.list:
            print("%s @ %s: %d record(s)" % (a.branch, rev[:7], len(recs)))
            for r in recs:
                print("  %s %s  %s" % ("x" if r["extracted_here"] else "-", r["dir"], ", ".join(r["archives"])))
            result["list"] = recs
        else:
            todo = [r for r in recs if r["dir"] == a.record] if a.record else [r for r in recs if not r["extracted_here"]]
            if a.record and not todo:
                raise Fail("no record folder dispatch/%s on %s" % (a.record, a.branch))
            for r in todo:
                info = extract_one(cache, rev, r, a.dest)
                result["new"].append(info)
                print("NEW %s -> %s" % (r["dir"], info["path"]))
                for f in info["archives"]:
                    print("    %s  %d bytes  MD5 %s%s" % (f["archive"], f["bytes"], f["md5"],
                                                         " (checked)" if f["md5_checked"] else " (no REQUEST.md MD5)"))
                print("    note: " + info["note"].replace("\n", "\n          "))
            if not todo:
                print("no new State Record on %s @ %s (%d on the branch, all extracted in %s)" % (a.branch, rev[:7], len(recs), a.dest))
        if a.json:
            with open(a.json, "w", encoding="utf-8") as f:
                json.dump(result, f, ensure_ascii=False, indent=1)
        return 0
    except (Fail, OSError, zipfile.BadZipFile) as e:
        print("ERROR: %s" % e)
        return 2


if __name__ == "__main__":
    sys.exit(main())
