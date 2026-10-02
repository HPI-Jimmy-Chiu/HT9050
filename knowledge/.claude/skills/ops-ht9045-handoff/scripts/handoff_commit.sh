#!/bin/bash
# 用法：handoff_commit.sh <edit.py> <commit 訊息檔>
# 要改別的交接檔時設 HANDOFF_FILES（空白分隔，預設 "FROM_STEVEN.md CHAT_ST01.md"），例：HANDOFF_FILES=AUDIT_IO_20260926.md
# 不 checkout：抽出 steven-handoff 的 FROM_STEVEN.md／CHAT_ST01.md 到暫存資料夾，跑 edit.py（cwd＝暫存資料夾），
# 用 plumbing 做 commit 並 push；被拒就在最新版上重跑 edit.py 再推（最多 5 次）。
set -u
export MSYS_NO_PATHCONV=1
R=D:/HT9045; BR=v906/steven-handoff; EDIT="$1"; MSG=$(cygpath -m "$2")
# 20260930：第二個參數一定是「訊息檔」。傳字串會讓 commit-tree 失敗、$C 變空，舊版接著 push ":refs/heads/$BR" = 刪掉遠端分支（ST01-M 00:3x 真的刪過一次，馬上用 bfe5d127 還原）。
[ -f "$2" ] || { echo "usage: handoff_commit.sh <edit.py> <commit message FILE> -- '$2' is not a file"; exit 2; }
PY=/c/Users/steven/AppData/Local/Programs/Python/Python314/python.exe
for try in 1 2 3 4 5; do
  git -C $R fetch -q origin $BR || { echo "fetch failed"; sleep 3; continue; }
  BASE=$(git -C $R rev-parse origin/$BR)
  T=$(mktemp -d); TW=$(cygpath -m "$T")
  for f in ${HANDOFF_FILES:-FROM_STEVEN.md CHAT_ST01.md}; do git -C $R show $BASE:docs/handoff/$f > "$T/$f" 2>/dev/null || : > $T/$f; done
  ( cd $T && $PY "$EDIT" ) || { echo "edit failed"; rm -rf $T; exit 1; }
  IDX=$(cygpath -m "$(mktemp -u)")
  GIT_INDEX_FILE=$IDX git -C $R read-tree $BASE
  changed=0
  for f in $(cd "$T" && ls); do
    old=$(git -C $R rev-parse $BASE:docs/handoff/$f 2>/dev/null || echo none)
    new=$(git -C $R hash-object -w "$TW/$f")
    if [ "$old" != "$new" ]; then GIT_INDEX_FILE=$IDX git -C $R update-index --add --cacheinfo 100644,$new,docs/handoff/$f; changed=1; fi
  done
  [ $changed = 1 ] || { echo "no change"; rm -rf $T $IDX; exit 0; }
  if LC_ALL=C grep -q -P '[\x00-\x08\x0b-\x1f]' "$T"/*; then echo "control char found, abort"; rm -rf $T $IDX; exit 1; fi
  TREE=$(GIT_INDEX_FILE=$IDX git -C $R write-tree)
  C=$(git -C $R commit-tree $TREE -p $BASE -F "$MSG") && [ -n "$C" ] || { echo "commit-tree failed, nothing pushed"; rm -rf $T $IDX; exit 1; }
  rm -rf $T $IDX
  if git -C $R push -q origin $C:refs/heads/$BR 2>/dev/null; then echo "pushed $(git -C $R rev-parse --short $BASE)..$(git -C $R rev-parse --short $C)"; exit 0; fi
  echo "push rejected (try $try), retrying on latest"
done
echo "gave up"; exit 1
