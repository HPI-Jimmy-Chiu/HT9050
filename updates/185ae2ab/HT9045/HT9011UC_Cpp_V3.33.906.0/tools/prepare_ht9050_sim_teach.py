"""Create a separate HT9050 simulation teaching file. Never change the source."""
import argparse
import hashlib
import re
from datetime import datetime
from pathlib import Path


def simulation_copy(data):
    section = re.search(rb"(?m)^\[MTrayX\][\s\S]*?(?=^\[|\Z)", data)
    if section is None:
        raise ValueError("Missing [MTrayX]")
    body, count = re.subn(rb"(?m)^(setEditTrayColorX=)[^\r\n]*", rb"\g<1>20000", section[0])
    if count != 1:
        raise ValueError("Expected exactly one setEditTrayColorX in [MTrayX]")
    return (b"; SIMULATION ONLY: Color=20000 is synthetic, not a machine teaching value.\r\n"
            + data[:section.start()] + body + data[section.end():])


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--source", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()
    if args.source.resolve() == args.output.resolve():
        ap.error("Source and output must be different files")
    before = args.source.read_bytes()
    result = simulation_copy(before)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.output.exists():
        backup = args.output.with_name(args.output.name + ".bak_" + datetime.now().strftime("%Y%m%d_%H%M%S_%f"))
        backup.write_bytes(args.output.read_bytes())
        print("Previous simulation file backed up:", backup)
    args.output.write_bytes(result)
    assert args.source.read_bytes() == before, "Source changed during preparation"
    print("SIMULATION ONLY:", args.output)
    print("Source SHA256:", hashlib.sha256(before).hexdigest())
    print("Changed only [MTrayX] setEditTrayColorX to 20000; preserved other teaching values.")


if __name__ == "__main__":
    main()
