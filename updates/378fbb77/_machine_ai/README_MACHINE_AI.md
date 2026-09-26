# 給機台端 Claude：更新包 9（GitLab main `378fbb77`，相對更新包 8 `a6b553ef`）

> 筆電端 Claude 20260926 16:4x 產生。**先套更新包 3～8，再套這一包。要不要套由 Jimmy 決定。**
> 1 檔：`web/page/HW.teach.html`，底稿 `base_a6b553ef\`。

## 內容：教導頁 HOME 的權杖（NB2 R69 B4）

| 改了什麼 | 為什麼 |
|---|---|
| `HTMotorAccess.init` 加 `holdToken`（`#btnHome.down`） | HOME 超過 30 秒權杖被還，deadman 會 CancelAllJobs（不送停止、HomeFlag 停在 0） |
| 每秒 runtime 輪詢後 `teachSyncHome`：HOME 那一軸 `motion.homeJob` 連兩次 false 就彈起 `btnHome` | golden `uTeach.cpp` Timer1Timer :1392／:1403 HOME 做完 `btnHome->Down=false`；不補的話 hold 永遠成立、權杖永遠不還 |
| HOME 進行中每 60 秒 `HT9045Recipe.keepAlive()` | 同 MotorTest；伺服器閒置 10 分鐘會收回權杖 |

## 在機台上要看的

教導頁按 HOME：做完後按鈕在 2 秒內彈起；做完 30 秒後 IO 頁按 Output 不會再被「權杖在別的畫面」擋住。

## 步驟

Check → EastSun 同意 → 備份 → Apply（網頁檔，不用重建 C++）→ 重新整理瀏覽器驗證 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
