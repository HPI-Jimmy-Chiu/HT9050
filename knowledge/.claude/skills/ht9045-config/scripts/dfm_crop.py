# -*- coding: utf-8 -*-
"""DFM-based screenshot cropper (auto-discovery)."""
import re
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("[FATAL] Pillow required: pip install Pillow")
    sys.exit(1)

DFM_PATH = Path(r"d:\HT9045\HT9011UC_Code_V3.33.903.0_20260417\cConfiguration.dfm")
SCREENSHOT_ROOT = Path(r"d:\HT9045\.github\skills\ht9045-config\references\output\screenshots")

Y_OFFSET = 23
ROW_HEIGHT_DEFAULT = 28
TOP_PAD = 2
BOT_PAD = 4
# Gap between adjacent crops to avoid 1-px residual border of previous section
GAP = 6
# Extra bottom padding for tail-of-tab CheckBox rows (avoids 1-px stripe look)
TAIL_MIN_HEIGHT = 26

# Pairing strategy per group:
#   "sequential" (default): tab<X>NN.png pairs with the NN-th DFM tab
#                            in document order (filename number is just
#                            an index, not a section number).
#   "section":              tab<X>NN.png pairs with the DFM tab whose
#                            first section is [<X>NN]
#                            (filename number == section number).
GROUP_STRATEGY = {
    "N": "section",
}

CAPTION_RE = re.compile(r"Caption\s*=\s*'(\[(?P<sec>[A-Z]\d+(?:-\d+)?)\][^']*)'")
TOP_RE = re.compile(r"^\s*Top\s*=\s*(-?\d+)")
HEIGHT_RE = re.compile(r"^\s*Height\s*=\s*(\d+)")
OBJECT_RE = re.compile(r"^(\s*)object\s+(\w+):\s*(\w+)")
END_RE = re.compile(r"^(\s*)end\b")


class DfmObject:
    __slots__ = ("name", "type", "indent", "top", "height", "caption",
                 "section", "parent", "children")

    def __init__(self, name, type_, indent, parent):
        self.name = name
        self.type = type_
        self.indent = indent
        self.top = 0
        self.height = 0
        self.caption = None
        self.section = None
        self.parent = parent
        self.children = []
        if parent is not None:
            parent.children.append(self)

    def abs_top(self):
        y = self.top
        p = self.parent
        while p is not None and p.type not in ("TTabSheet", "TForm"):
            y += p.top
            p = p.parent
        return y


def _join_continuations(text):
    """Normalize BCB6 multi-line Caption literals into one line.
    Handles both:
        Caption =
          '[A35] foo' +
          'bar'
    and:
        Caption = '[A35] foo' +
          'bar'
    """
    lines = text.splitlines()
    out = []
    i = 0
    cap_empty = re.compile(r"^(\s*)(Caption\s*=)\s*$")
    cap_inline_cont = re.compile(r"^(\s*)(Caption\s*=\s*)'(.*)'\s*\+\s*$")
    quoted = re.compile(r"^\s*'(.*)'(\s*\+?)\s*$")
    while i < len(lines):
        line = lines[i]
        m1 = cap_empty.match(line)
        m2 = None if m1 else cap_inline_cont.match(line)
        if m1 or m2:
            if m1:
                indent, key = m1.group(1), m1.group(2)
                parts = []
                j = i + 1
            else:
                indent, key = m2.group(1), m2.group(2).rstrip()
                parts = [m2.group(3)]
                j = i + 1
            while j < len(lines):
                nm = quoted.match(lines[j])
                if not nm:
                    break
                parts.append(nm.group(1))
                cont = nm.group(2).strip().endswith("+")
                j += 1
                if not cont:
                    break
            if parts:
                joined = "".join(parts).replace("''", "'")
                out.append("{}{} '{}'".format(indent, key.rstrip(), joined))
                i = j
                continue
        out.append(line)
        i += 1
    return "\n".join(out)


def parse_dfm(path):
    root = DfmObject("__root__", "TForm", -1, None)
    stack = [root]
    text = path.read_text(encoding="cp950", errors="replace")
    text = _join_continuations(text)
    for line in text.splitlines():
        m_obj = OBJECT_RE.match(line)
        if m_obj:
            indent = len(m_obj.group(1))
            name, type_ = m_obj.group(2), m_obj.group(3)
            while stack and stack[-1].indent >= indent:
                stack.pop()
            parent = stack[-1] if stack else root
            obj = DfmObject(name, type_, indent, parent)
            stack.append(obj)
            continue
        m_end = END_RE.match(line)
        if m_end and stack:
            indent = len(m_end.group(1))
            if stack[-1].indent == indent:
                stack.pop()
            continue
        if not stack:
            continue
        cur = stack[-1]
        m_top = TOP_RE.match(line)
        if m_top:
            cur.top = int(m_top.group(1))
            continue
        m_h = HEIGHT_RE.match(line)
        if m_h:
            cur.height = int(m_h.group(1))
            continue
        m_cap = CAPTION_RE.search(line)
        if m_cap:
            cur.caption = m_cap.group(1)
            cur.section = m_cap.group("sec")
    return root


def find_object(node, name):
    if node.name.lower() == name.lower():
        return node
    for c in node.children:
        r = find_object(c, name)
        if r is not None:
            return r
    return None


def discover_tabs(container):
    result = []

    def walk(node):
        if node.type == "TTabSheet":
            result.append(node)
        for c in node.children:
            walk(c)

    for c in container.children:
        walk(c)
    return result


def collect_sections(tab, group_letter):
    seen = {}

    def walk(node):
        if node.type == "TTabSheet":
            # never include any TTabSheet caption (tab titles like '[L01] - [L10]')
            if node is tab:
                for c in node.children:
                    walk(c)
            return
        if node.section and node.section.startswith(group_letter):
            sec = node.section
            atop = node.abs_top()
            h = node.height if node.height > 0 else ROW_HEIGHT_DEFAULT
            if sec not in seen or atop < seen[sec][0]:
                seen[sec] = (atop, h)
        for c in node.children:
            walk(c)

    for c in tab.children:
        walk(c)
    items = [(sec, atop, h) for sec, (atop, h) in seen.items()]
    items.sort(key=lambda x: x[1])
    return items


def list_source_screenshots(group_dir, letter):
    if not group_dir.is_dir():
        return []
    pat = re.compile(r"^tab" + letter + r"\d+\.png$", re.IGNORECASE)
    files = [p for p in group_dir.iterdir() if pat.match(p.name)]
    files.sort(key=lambda p: int(re.search(r"\d+", p.stem).group()))
    return files


def crop_tab(screenshot, sections, out_dir, dry_run=False):
    if not screenshot.exists():
        return 0
    img = Image.open(screenshot)
    W, H = img.size
    out_dir.mkdir(parents=True, exist_ok=True)
    top_level = [s for s in sections if "-" not in s[0]]
    n = 0
    for i, (sec, atop, h) in enumerate(top_level):
        is_last = (i + 1 >= len(top_level))
        next_delta = None
        if not is_last:
            next_delta = top_level[i + 1][1] - atop
        # CheckBox-class (declared h<=24) when packed tightly (next_delta<22)
        # has only ~18px visual room; use a fixed caption window centered on
        # the caption text row to avoid bleeding into neighbors.
        if h <= 24 and next_delta is not None and next_delta < 22:
            crop_top = max(0, atop + Y_OFFSET + 1)
            crop_bottom = atop + Y_OFFSET + 26
        else:
            crop_top = max(0, atop + Y_OFFSET - TOP_PAD)
            visual_h = max(h, 20)
            ideal_bottom = atop + Y_OFFSET + visual_h
            if not is_last:
                next_atop = top_level[i + 1][1]
                next_top = next_atop + Y_OFFSET - TOP_PAD
                crop_bottom = min(ideal_bottom, next_top - 2)
            else:
                crop_bottom = ideal_bottom + BOT_PAD
                if crop_bottom - crop_top < TAIL_MIN_HEIGHT:
                    crop_bottom = crop_top + TAIL_MIN_HEIGHT
        crop_bottom = min(H, crop_bottom)
        if crop_bottom <= crop_top:
            continue
        out_path = out_dir / (sec + "-overview.png")
        if dry_run:
            print("    [DRY] {}: y={}..{} ({}px)".format(sec, crop_top, crop_bottom, crop_bottom - crop_top))
        else:
            img.crop((0, crop_top, W, crop_bottom)).save(out_path)
            print("    [OK ] {}: y={}..{} ({}px)".format(sec, crop_top, crop_bottom, crop_bottom - crop_top))
        n += 1
    return n


def process_group(letter, root, dry_run=False):
    container_name = "ts" + letter + "00"
    container = find_object(root, container_name)
    if container is None:
        print("[SKIP] container {} not found in DFM".format(container_name))
        return 0
    tabs = discover_tabs(container)
    if not tabs:
        print("[SKIP] {}: no descendant TTabSheets".format(container_name))
        return 0

    group_dir = SCREENSHOT_ROOT / letter
    sources = list_source_screenshots(group_dir, letter)

    print("\n=== Group {} ===".format(letter))
    print("  DFM tabs ({}): {}".format(len(tabs), [t.name for t in tabs]))
    print("  Source files ({}): {}".format(len(sources), [p.name for p in sources]))
    if not sources:
        return 0

    strategy = GROUP_STRATEGY.get(letter, "sequential")
    print("  Strategy: {}".format(strategy))

    pairs = []
    if strategy == "sequential":
        pairs = list(zip(tabs, sources))
        if len(tabs) != len(sources):
            print("  [WARN] tab/file count mismatch -> pairing {} in order".format(len(pairs)))
    else:  # section-match
        nums = [int(re.search(r"\d+", s.stem).group()) for s in sources]
        used_tabs = set()
        unmatched = []
        for shot, nn in zip(sources, nums):
            target_sec = "{}{:02d}".format(letter, nn)
            chosen = None
            for tab in tabs:
                if tab.name in used_tabs:
                    continue
                secs = collect_sections(tab, letter)
                top = [s for s in secs if "-" not in s[0]]
                if top and top[0][0] == target_sec:
                    chosen = tab
                    break
            if chosen is not None:
                pairs.append((chosen, shot))
                used_tabs.add(chosen.name)
            else:
                unmatched.append(shot.name)
        if unmatched:
            print("  [WARN] no DFM tab matched for: {}".format(unmatched))

    total = 0
    for tab, shot in pairs:
        sections = collect_sections(tab, letter)
        if not sections:
            continue
        print("  [{}] -> {}  ({} sections)".format(tab.name, shot.name, len(sections)))
        total += crop_tab(shot, sections, group_dir, dry_run=dry_run)
    return total


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("groups", nargs="*")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--offset", type=int, default=None)
    args = ap.parse_args()

    global Y_OFFSET
    if args.offset is not None:
        Y_OFFSET = args.offset

    print("Parsing DFM: {}".format(DFM_PATH))
    root = parse_dfm(DFM_PATH)
    print("Y_OFFSET = {}".format(Y_OFFSET))

    if args.groups:
        groups = [g.upper() for g in args.groups]
    else:
        groups = sorted(p.name for p in SCREENSHOT_ROOT.iterdir()
                        if p.is_dir() and len(p.name) == 1)

    grand = 0
    for g in groups:
        grand += process_group(g, root, dry_run=args.dry_run)
    print("\nDone. {} crops written.".format(grand))


if __name__ == "__main__":
    main()
