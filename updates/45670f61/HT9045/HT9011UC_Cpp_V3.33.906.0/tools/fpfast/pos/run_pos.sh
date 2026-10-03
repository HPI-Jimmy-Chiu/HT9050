#!/usr/bin/env bash
# AI(W906-FPFAST) 20261003 -- positive control: BCB6 vs oracle g++ 6.3.0 vs WinLibs g++ 16.2.0 i686,
# each with / without -fexcess-precision=fast.  Every compile runs through lowrun.py (IDLE, mask 0x3F).
set -u
POS="$(cd "$(dirname "$0")" && pwd)"
FP="$(cd "$POS/.." && pwd)"
LOW="python $(cygpath -w "$FP/lowrun.py")"
TREE=/d/HT9045/.claude/worktrees/fp/HT9011UC_Cpp_V3.33.906.0
S4="$(cygpath -w "$TREE/tools/fp_equality_probe.cpp")"
PR="$(cygpath -w "$POS/fpfast_probe.cpp")"
OR=/c/MinGW/bin
WL=/d/HT9045/install/mingw32-16.2.0/mingw32/bin
BCB='D:\ProgramFiles\Borland\CBuilder6'
cd "$POS" || exit 1
rm -f *.exe *.obj *.tds *.o run_*.txt

echo "== versions"
PATH="$OR:$PATH" $OR/g++.exe --version | head -1
PATH="$WL:$PATH" $WL/g++.exe --version | head -1
ls -la /d/ProgramFiles/Borland/CBuilder6/Bin/bcc32.exe

build_gcc() { # name bin std flags... src
  local name="$1" bin="$2" std="$3" src="$4"; shift 4
  $LOW --path-prepend "$(cygpath -w "$bin")" -- "$(cygpath -w "$bin/g++.exe")" "$std" "$@" -static -include cfloat -o "$(cygpath -w "$POS/$name.exe")" "$src"
  echo "  build $name rc=$?"
}
build_bcb() { # name src
  local name="$1" src="$2"
  $LOW -- "$BCB\\Bin\\bcc32.exe" -Od -I"$BCB\\Include" -L"$BCB\\Lib" -e"$(cygpath -w "$POS/$name.exe")" "$src" > "$POS/bcc_$name.log" 2>&1
  echo "  build $name rc=$? (log bcc_$name.log)"
}

echo "== build: extended probe"
build_bcb  bcb6            "$PR"
build_gcc  o63_none   "$OR" -std=c++1z "$PR" -O0
build_gcc  o63_fast   "$OR" -std=c++1z "$PR" -O0 -fexcess-precision=fast
build_gcc  w16_none   "$WL" -std=c++14 "$PR" -O0
build_gcc  w16_fast   "$WL" -std=c++14 "$PR" -O0 -fexcess-precision=fast
# EastSun's -O2 line (build_integ_ship_x86_o2, .vscode/tasks.json:169-172): its CMAKE_CXX_FLAGS, then the tree-wide flag after them
O2="-O2 -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -ffloat-store -fexcess-precision=standard -fno-finite-loops"
build_gcc  w16_o2std      "$WL" -std=c++14 "$PR" $O2
build_gcc  w16_o2std_fast "$WL" -std=c++14 "$PR" $O2 -fexcess-precision=fast
echo "== build: s4 probe (tools/fp_equality_probe.cpp, unmodified)"
build_bcb  s4_bcb6         "$S4"
build_gcc  s4_o63_none "$OR" -std=c++1z "$S4" -O0
build_gcc  s4_o63_fast "$OR" -std=c++1z "$S4" -O0 -fexcess-precision=fast
build_gcc  s4_w16_none "$WL" -std=c++14 "$S4" -O0
build_gcc  s4_w16_fast "$WL" -std=c++14 "$S4" -O0 -fexcess-precision=fast

echo "== run"
for e in bcb6 o63_none o63_fast w16_none w16_fast w16_o2std w16_o2std_fast; do
  if [ -f "$POS/$e.exe" ]; then "$POS/$e.exe" > "run_$e.txt" 2>&1; echo "  $e rc=$?"; else echo "  $e MISSING"; fi
done
for e in s4_bcb6 s4_o63_none s4_o63_fast s4_w16_none s4_w16_fast; do
  if [ -f "$POS/$e.exe" ]; then "$POS/$e.exe" > "run_$e.txt" 2>&1; echo "  $e rc=$?"; else echo "  $e MISSING"; fi
done

echo "== table (extended probe): case | bcb6 | o63_none o63_fast | w16_none w16_fast | w16_O2std w16_O2std+fast"
paste -d' ' <(cut -d' ' -f1 run_bcb6.txt) <(cut -d' ' -f2 run_bcb6.txt) <(cut -d' ' -f2 run_o63_none.txt) <(cut -d' ' -f2 run_o63_fast.txt) \
  <(cut -d' ' -f2 run_w16_none.txt) <(cut -d' ' -f2 run_w16_fast.txt) <(cut -d' ' -f2 run_w16_o2std.txt) <(cut -d' ' -f2 run_w16_o2std_fast.txt) | column -t
echo "== s4 probe outputs"
for e in s4_bcb6 s4_o63_none s4_o63_fast s4_w16_none s4_w16_fast; do echo "--- $e"; cat "run_$e.txt"; done
