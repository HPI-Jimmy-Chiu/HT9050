"""Auto-crop tab screenshots into per-section overview images.

Workflow per group:
  1. Read every tab source image:  screenshots/<G>/tab<G><NN>.png
  2. Detect dark text rows (numpy pixel scan, axis=1 min).
  3. For each detected row, crop a fixed-height band centred on the row.
  4. Save per-row PNGs into the same folder, naming defined in CONFIG.

Output naming: see CONFIG[<group>]['rows'] (rowN -> section name).
If a tab is not yet mapped in CONFIG, rows are dumped as
``tab<G><NN>-row<NN>.png`` so you can preview and decide labels.

Usage:
    python crop_screenshots.py            # crop all configured groups
    python crop_screenshots.py L          # only group L
    python crop_screenshots.py A B        # multiple groups
"""

from __future__ import annotations

import sys
from pathlib import Path
from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parent.parent / "references" / "screenshots"

# Crop band relative to detected text-row centre.
TOP_OFFSET = -10    # was -6: pulled top down 4px to kill the white sliver
BOT_OFFSET = 18     # gives a 28px band, slight bias below to grab inputs
MIN_DARK_GAP = 12   # rows closer than this px are merged
DARK_THRESHOLD = 110  # pixel value below this counts as "text"

# Mapping: group -> tab-source-stem -> ordered list of section labels.
# Each entry corresponds to one detected dark-text row, top-to-bottom.
# An empty string skips that row (group-box header / decorative line).
CONFIG = {
    "L": {
        "tabL01": [
            "L01", "L02", "L03", "L04", "L05",
            "L06", "L07", "L08", "L09", "L10",
        ],
        "tabL02": [
            "L11", "L12", "L13", "L14", "L15",
            "L16", "L17", "L18", "L19", "L20",
        ],
        "tabL04": [
            "L21", "L22", "L23", "L24", "L25",
            "L26", "L27", "L28", "L29", "L30",
        ],
        "tabL05": ["L31", "L32"],
        "tabL06": ["L33", "L34", "L35"],
        "tabL07": [
            "L36", "L37", "L38", "L39", "L40",
            "L41", "L42", "L43", "L44", "L45", "L46",
        ],
    },
    # A/B/C/D/E mappings to be filled once we eyeball the row order.
}


def detect_text_rows(img: Image.Image) -> list[int]:
    """Return centre Y of each dark-text row (top->bottom)."""
    arr = np.array(img.convert("L"))
    row_min = arr.min(axis=1)
    dark_mask = row_min < DARK_THRESHOLD

    rows: list[int] = []
    in_run = False
    run_start = 0
    for y, dark in enumerate(dark_mask):
        if dark and not in_run:
            in_run = True
            run_start = y
        elif not dark and in_run:
            in_run = False
            centre = (run_start + y - 1) // 2
            if not rows or centre - rows[-1] >= MIN_DARK_GAP:
                rows.append(centre)
    if in_run:
        rows.append((run_start + arr.shape[0] - 1) // 2)

    rows = [y for y in rows if 15 <= y <= arr.shape[0] - 5]
    return rows


def crop_tab(tab_path: Path, labels):
    img = Image.open(tab_path)
    centres = detect_text_rows(img)
    written = []
    for idx, cy in enumerate(centres):
        top = max(0, cy + TOP_OFFSET)
        bot = min(img.height, cy + BOT_OFFSET)
        crop = img.crop((0, top, img.width, bot))
        if labels and idx < len(labels):
            label = labels[idx]
            if not label:
                continue
            out = tab_path.with_name(f"{label}-overview.png")
        else:
            out = tab_path.with_name(f"{tab_path.stem}-row{idx + 1:02d}.png")
        crop.save(out)
        written.append(out)
    return written


def process_group(group: str) -> int:
    src_dir = ROOT / group
    if not src_dir.is_dir():
        print(f"  [skip] {src_dir} not found")
        return 0
    mapping = CONFIG.get(group, {})
    count = 0
    for tab in sorted(src_dir.glob(f"tab{group}*.png")):
        labels = mapping.get(tab.stem)
        outs = crop_tab(tab, labels)
        tag = "labelled" if labels else "preview, rename manually"
        print(f"  [{tab.stem}] -> {len(outs)} crops ({tag})")
        count += len(outs)
    return count


def main(argv):
    if len(argv) > 1:
        groups = argv[1:]
    else:
        groups = sorted(
            d.name for d in ROOT.iterdir()
            if d.is_dir() and len(d.name) == 1 and d.name.isupper()
        )
    print(f"Crop tabs in groups: {', '.join(groups)}")
    total = 0
    for g in groups:
        print(f"-- group {g} --")
        total += process_group(g)
    print(f"Done. {total} crops written.")


if __name__ == "__main__":
    main(sys.argv)
