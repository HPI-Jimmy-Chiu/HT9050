#!/usr/bin/env bash
# AI(W906-FPFAST) 20261003 -- mutation controls for ctest FPFAST_Pin (tests/test_fpfast.cpp).
#   C1  oracle g++ 6.3.0, test compiled WITHOUT the flag, real root        -> expect PASS (runtime half cannot see a drop on 6.3.0)
#   C2  same exe, root whose CMakeLists.txt is HEAD's (no flag line)        -> expect FAIL in [S] only
#   C3  WinLibs g++ 16.2 i686 -std=c++14, WITHOUT the flag, real root       -> expect FAIL in [R]
#   C4  WinLibs g++ 16.2 i686 -std=c++14, WITH the flag, real root          -> expect PASS
#   C5  C4's exe against the HEAD root                                      -> expect FAIL in [S] only
# Compiles go through lowrun.py (IDLE priority).  Flags = the build's for this TU (-std, -Wall -Wextra) minus/plus the flag.
set -u
CTL="$(cd "$(dirname "$0")" && pwd)"
FP="$(cd "$CTL/.." && pwd)"
LOW="python $(cygpath -w "$FP/lowrun.py") --mask 0xFC000"
WT=/d/HT9045/.claude/worktrees/fp
TREE=$WT/HT9011UC_Cpp_V3.33.906.0
SRC="$(cygpath -w "$TREE/tests/test_fpfast.cpp")"
ROOT="$(cygpath -w "$TREE")"
OR=/c/MinGW/bin
WL=/d/HT9045/install/mingw32-16.2.0/mingw32/bin
cd "$CTL" || exit 1
rm -rf mutroot *.exe out_*.txt
mkdir -p mutroot/tests
git -C "$WT" show HEAD:HT9011UC_Cpp_V3.33.906.0/CMakeLists.txt > mutroot/CMakeLists.txt
cp "$TREE/tests/CMakeLists.txt" mutroot/tests/CMakeLists.txt
echo "mutroot CMakeLists.txt has the flag line: $(grep -c 'fexcess-precision' mutroot/CMakeLists.txt) (expect 0); real: $(grep -c 'fexcess-precision=fast' "$TREE/CMakeLists.txt")"
MUT="$(cygpath -w "$CTL/mutroot")"

cc() { # name bin std extra...
  local name="$1" bin="$2" std="$3"; shift 3
  $LOW --path-prepend "$(cygpath -w "$bin")" -- "$(cygpath -w "$bin/g++.exe")" "$std" -Wall -Wextra "$@" -static -o "$(cygpath -w "$CTL/$name.exe")" "$SRC"
  echo "  compile $name rc=$?"
}
cc o63_noflag   "$OR" -std=c++1z
cc w16_noflag   "$WL" -std=c++14
cc w16_flag     "$WL" -std=c++14 -fexcess-precision=fast

runit() { # tag exe root
  "$CTL/$2.exe" "$3" > "out_$1.txt" 2>&1
  local rc=$?
  echo "== $1 ($2, root=$3) exit=$rc  $(grep -c 'PASS:' out_$1.txt) PASS / $(grep -c 'FAIL:' out_$1.txt) FAIL"
  grep 'FAIL:' "out_$1.txt" | sed 's/^/     /'
}
runit C1 o63_noflag "$ROOT"
runit C2 o63_noflag "$MUT"
runit C3 w16_noflag "$ROOT"
runit C4 w16_flag   "$ROOT"
runit C5 w16_flag   "$MUT"
