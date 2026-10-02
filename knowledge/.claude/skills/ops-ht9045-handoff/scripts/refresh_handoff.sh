#!/bin/sh
# 更新 D:\HT9045_handoff 的唯讀快照＋三方聊天合併檔（St01 巡檢用）
D=/d/HT9045_handoff; R=/d/HT9045; mkdir -p $D
git -C $R fetch -q origin main v906/steven-handoff v906/steven-cbridge-review6 v906/steven-gpib-widget || echo "fetch failed"
snap() { git -C $R show "$1:docs/handoff/$2" > "$D/$2.tmp" 2>/dev/null && mv "$D/$2.tmp" "$D/$2" || rm -f "$D/$2.tmp"; }
snap origin/main TO_STEVEN.md
snap origin/main CHAT_JIMMY.md
snap origin/v906/steven-handoff FROM_STEVEN.md
snap origin/v906/steven-handoff CHAT_ST01.md
snap origin/v906/steven-handoff CHAT_ST02.md
{
  printf '# 三方聊天合併（唯讀；由 CHAT_JIMMY／CHAT_ST01／CHAT_ST02 按時間合成）\r\n\r\n'
  for f in CHAT_JIMMY.md CHAT_ST01.md CHAT_ST02.md; do [ -f "$D/$f" ] && grep -h '^- 20[0-9]\{6\} [0-9][0-9]:[0-9][0-9]' "$D/$f"; done | sed 's/\r$//' | sort -s -k2,3 | sed 's/$/\r/'
} > "$D/CHAT_合併.md"
TO=$(git -C $R log -1 --format=%h origin/main -- docs/handoff/); FR=$(git -C $R rev-parse --short origin/v906/steven-handoff)
printf '%s\r\n' "這個資料夾是交接檔的唯讀快照，不要在這裡改。" \
 "TO_STEVEN.md／CHAT_JIMMY.md     ← origin/main（Jimmy 寫）" \
 "FROM_STEVEN.md／CHAT_ST01.md／CHAT_ST02.md ← origin/v906/steven-handoff（St01／St02 寫）" \
 "CHAT_合併.md ← 三個聊天檔按時間合起來" \
 "St01 的協調 session（ST01-M）每 30 分鐘巡檢時更新一次。" \
 "最後更新：$(date '+%Y%m%d %H:%M')，main=$TO steven-handoff=$FR" > "$D/_README.txt"
ls "$D"
