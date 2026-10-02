"""Shift existing per-section crops down by N pixels.

Use case: previous crops have a thin white edge at the top.
We template-match each ``<section>-overview.png`` against its source
``tab<G><NN>.png`` to recover the original (top, bottom) and re-crop with
``top += SHIFT``, preserving width & height.

Usage:
    python shift_crops.py L           # shift L group
    python shift_crops.py L A B       # multiple groups

Constants:
    SHIFT  : pixels to push the crop downward.
"""

from __future__ import annotations

import sys
from pathlib import Path
from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parent.parent / "references" / "screenshots"
SHIFT = 2  # pixels to nudge top downward

# Tab source mapping per group (which sections belong to each tab file).
TAB_FOR_SECTION = {
    "L": {
        # tabL01: L01-L10
        **{f"L{i:02d}": "tabL01" for i in range(1, 11)},
        # tabL02: L11-L20
        **{f"L{i:02d}": "tabL02" for i in range(11, 21)},
        # tabL04: L21-L30
        **{f"L{i:02d}": "tabL04" for i in range(21, 31)},
        # tabL05: L31-L32
        **{f"L{i:02d}": "tabL05" for i in range(31, 33)},
        # tabL06: L33-L35
        **{f"L{i:02d}": "tabL06" for i in range(33, 36)},
        # tabL07: L36-L46
        **{f"L{i:02d}": "tabL07" for i in range(36, 47)},
    },
}


def find_y_offset(tab_arr: np.ndarray, crop_arr: np.ndarray) -> int:
    """Return best top Y of crop inside tab using row-wise mean-abs-diff."""
    H, W = crop_arr.shape[:2]
    best_y = 0
    best_score = 1e18
    for y in range(tab_arr.shape[0] - H + 1):
        diff = np.abs(tab_arr[y:y + H].astype(int) - crop_arr.astype(int)).mean()
        if diff < best_score:
            best_score = diff
            best_y = y
    return best_y


def shift_group(group: str) -> int:
    src_dir = ROOT / group
    if not src_dir.is_dir():
        print(f"  [skip] {src_dir} not found")
        return 0

    section_to_tab = TAB_FOR_SECTION.get(group, {})
    if not section_to_tab:
        print(f"  [skip] no tab mapping for group {group}")
        return 0

    tab_cache = {}
    count = 0
    for crop_path in sorted(src_dir.glob(f"{group}*-overview.png")):
        section = crop_path.stem.replace("-overview", "")
        tab_stem = section_to_tab.get(section)
        if not tab_stem:
            print(f"  [{section}] no tab source mapped, skip")
            continue
        tab_path = src_dir / f"{tab_stem}.png"
        if not tab_path.exists():
            print(f"  [{section}] tab source {tab_path.name} missing, skip")
            continue

        if tab_stem not in tab_cache:
            tab_cache[tab_stem] = np.array(Image.open(tab_path).convert("L"))
        tab_arr = tab_cache[tab_stem]
        tab_img = Image.open(tab_path)

        crop_img = Image.open(crop_path)
        crop_arr = np.array(crop_img.convert("L"))

        if crop_arr.shape[1] != tab_arr.shape[1]:
            print(f"  [{section}] width mismatch, skip")
            continue

        top = find_y_offset(tab_arr, crop_arr)
        new_top = max(0, top + SHIFT)
        new_bot = min(tab_arr.shape[0], new_top + crop_arr.shape[0])
        new_crop = tab_img.crop((0, new_top, tab_arr.shape[1], new_bot))
        new_crop.save(crop_path)
        print(f"  [{section}] {top} -> {new_top} (+{SHIFT})")
        count += 1
    return count


def main(argv):
    groups = argv[1:] if len(argv) > 1 else ["L"]
    print(f"Shift crops in groups: {', '.join(groups)} by {SHIFT}px down")
    total = 0
    for g in groups:
        print(f"-- group {g} --")
        total += shift_group(g)
    print(f"Done. {total} crops shifted.")


if __name__ == "__main__":
    main(sys.argv)
