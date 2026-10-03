#!/usr/bin/env bash
# AI(W906-FPFAST) 20261003 -- optimisation-level matrix for the same probe: which of -O2 / -ffloat-store / the
# excess-precision mode decides the X (80-bit intermediate) rows?  WinLibs 16.2 i686 -std=c++14 and oracle 6.3.0.
set -u
POS="$(cd "$(dirname "$0")" && pwd)"
FP="$(cd "$POS/.." && pwd)"
LOW="python $(cygpath -w "$FP/lowrun.py") --mask 0xFC000"
PR="$(cygpath -w "$POS/fpfast_probe.cpp")"
OUT=/d/HT9045/.claude/worktrees/fp/Obj/V906/fpfast_pos
OR=/c/MinGW/bin
WL=/d/HT9045/install/mingw32-16.2.0/mingw32/bin
mkdir -p "$OUT"; cd "$POS" || exit 1
b() { # name bin std flags...
  local name="$1" bin="$2" std="$3"; shift 3
  $LOW --path-prepend "$(cygpath -w "$bin")" -- "$(cygpath -w "$bin/g++.exe")" "$std" "$@" -static -o "$(cygpath -w "$OUT/$name.exe")" "$PR"
  echo "  build $name rc=$?"
}
b m_w_O2_std    "$WL" -std=c++14 -O2 -fexcess-precision=standard
b m_w_O2_fast   "$WL" -std=c++14 -O2 -fexcess-precision=fast
b m_w_O2fs_std  "$WL" -std=c++14 -O2 -ffloat-store -fexcess-precision=standard
b m_w_O2fs_fast "$WL" -std=c++14 -O2 -ffloat-store -fexcess-precision=fast
b m_w_O0fs_fast "$WL" -std=c++14 -O0 -ffloat-store -fexcess-precision=fast
b m_o_O3_none   "$OR" -std=c++1z -O3
b m_o_O3_fast   "$OR" -std=c++1z -O3 -fexcess-precision=fast
b m_o_O2fs      "$OR" -std=c++1z -O2 -ffloat-store
names="m_w_O2_std m_w_O2_fast m_w_O2fs_std m_w_O2fs_fast m_w_O0fs_fast m_o_O3_none m_o_O3_fast m_o_O2fs"
for n in $names; do
  t0=$(date +%s); "$OUT/$n.exe" > "run2_$n.txt" 2>&1; rc=$?; t1=$(date +%s); echo "  run $n rc=$rc ($((t1-t0)) s)"
done
echo "== case | bcb6 | $names"
cols=""
for n in $names; do cols="$cols <(cut -d' ' -f2 run2_$n.txt)"; done
eval paste -d"' '" "<(cut -d' ' -f1 run_bcb6.txt)" "<(cut -d' ' -f2 run_bcb6.txt)" $cols | column -t
echo "== oracle -O3: none vs fast exe identical except PE timestamp?"
cmp -l "$OUT/m_o_O3_none.exe" "$OUT/m_o_O3_fast.exe" | wc -l
