# 給機台端 Claude：更新包 31（GitLab main `fe03a1e7`，相對更新包 30 `b635f32d`）

> 筆電端 Claude 20260927 03:2x 產生。**先套更新包 3～30，再套這一包。要不要套由 Jimmy 決定。**
> 5 檔（1 個原始碼只改註解＋4 份文件），底稿 `base_b635f32d\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

* **還原被吃掉的反斜線**：路徑與正規表示式裡的 `\b`、`\a`、`\14` 曾被當成跳脫字元，寫成看不見的控制字元（退格、響鈴、跳頁）。
  * `cObserver.cpp` 4 行註解（`rg -n "\bTimer1\b"` 這類字界符號）—— **只改註解**，剝掉註解後跟上一版逐字相同。
  * `docs/KNOWLEDGE.md`、`docs/MIGRATION_ROADMAP.md`（`MSVC\14.44.35207\atlmfc`）、`docs/INBOX_QUEUE.md`（0925 那幾列的 `D:\backup\HT9050` 等）。
  * repo 根的 CLAUDE.md 與兩個 skill 也改了，但不在包裡（包只帶 906 樹與 web）。
* `docs/NIGHT_REPORT.md` 更新。

## 你們機台上看得到的差別

* **沒有差別**（wb_serve 行為不變；原始碼只動註解）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply →（可不重建：只有註解與文件）→ commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`
