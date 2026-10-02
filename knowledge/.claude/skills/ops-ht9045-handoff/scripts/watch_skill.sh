#!/bin/bash
# 盯 ht9050-construction：本機檔案「內容」變動（等停下來再報，順便標非 UTF-8）＋遠端各分支動到它的新 commit
D=/d/HT9045/.claude/skills/ht9050-construction; R=/d/HT9045; P=.claude/skills/ht9050-construction
snap() { (cd "$D" 2>/dev/null && find . -type f -print0 | xargs -0 md5sum 2>/dev/null | awk '{p=$2; sub(/^\*/,"",p); sub(/^\.\//,"",p); print p, $1}' | sort); }
remote_commits() { git -C $R log --remotes --format=%h -- $P 2>/dev/null | sort; }
base=$(snap); last=$base; seen=$(remote_commits); n=0
while true; do
  sleep 20; n=$((n+1))
  cur=$(snap)
  if [ "$cur" = "$last" ] && [ "$cur" != "$base" ]; then
    out=""
    for f in $(diff <(echo "$base") <(echo "$cur") | grep '^[<>]' | awk '{print $2}' | sort -u); do
      if [ ! -f "$D/$f" ]; then out="$out $f(刪除)"
      elif ! iconv -f UTF-8 -t UTF-8 "$D/$f" >/dev/null 2>&1; then out="$out $f(⚠非UTF-8)"
      else out="$out $f"; fi
    done
    echo "[skill 本機變動]$out"
    base=$cur
  fi
  last=$cur
  if [ $((n % 9)) -eq 0 ]; then
    git -C $R fetch -q origin 2>/dev/null || true
    now=$(remote_commits)
    for h in $(comm -13 <(echo "$seen") <(echo "$now")); do
      br=$(git -C $R branch -r --contains $h 2>/dev/null | sed 's/^ *//' | grep -v HEAD | head -3 | tr '\n' ' ')
      echo "[skill 遠端新 commit] $(git -C $R log -1 --format='%h %ad %an %s' --date=format:'%H:%M' $h | cut -c1-140) ｜分支：$br"
    done
    seen=$now
  fi
done
