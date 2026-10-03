#!/usr/bin/env bash
# AI(W906-FPFAST) 20261003 -- step 5: build.bat (quick = libs + tests + exes) into <worktree>/Obj/V906/<dir>,
# then ctest on the new / touched / FP-sensitive tests only.  Everything under lowrun.py (IDLE priority, mask).
# usage: build_and_test.sh <build_dir_name> <mask> <tag>
set -u
BD="$1"; MASK="$2"; TAG="$3"
FP="$(cd "$(dirname "$0")" && pwd)"
LOWPY="$(cygpath -w "$FP/lowrun.py")"
TREE=/d/HT9045/.claude/worktrees/fp/HT9011UC_Cpp_V3.33.906.0
BAT="$(cygpath -w "$TREE/build.bat")"
OBJ=/d/HT9045/.claude/worktrees/fp/Obj/V906
LOG="$FP/build_$TAG.log"
echo "[bt $TAG] $(date -Iseconds) build.bat quick -> $BD (mask $MASK)"
python "$LOWPY" --mask "$MASK" --env "V906_BUILD_DIR=$BD" -- cmd /c "$BAT" quick > "$LOG" 2>&1
echo "[bt $TAG] $(date -Iseconds) build.bat exit=$?"
grep -E "^\[build\]|FAILED: |error:|ninja: build stopped" "$LOG" | tail -15
TESTS='^(FPFAST_Pin|Adam6024_Pressure|ADAM6024_Comm|Adam6024_Apax|Adam6024_Flow|TransformFuntion|ContactForce|ContactForceLoad|cContact|ContactCTCore|ExternFunction|SCK_ART|SCK_ART_Remainder|CpublicFoundation|PTW1_TextProcess|GA1_LastSet|GA2_C1_cinitial)$'
echo "[bt $TAG] $(date -Iseconds) ctest -R $TESTS"
python "$LOWPY" --mask "$MASK" --path-prepend 'C:\MinGW\bin' -- ctest --test-dir "$(cygpath -w "$OBJ/$BD")" -R "$TESTS" --timeout 600 --output-on-failure > "$FP/ctest_$TAG.log" 2>&1
echo "[bt $TAG] $(date -Iseconds) ctest exit=$?"
grep -E "tests passed|Test +#|The following tests FAILED|^\s+[0-9]+ - " "$FP/ctest_$TAG.log" | tail -40
