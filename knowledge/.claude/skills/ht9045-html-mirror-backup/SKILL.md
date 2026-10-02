---
name: ht9045-html-mirror-backup
description: "Use when: 把 HT9045 web HMI 的成果備份／交付到 U:\\共用區\\HT-9050 給 Jimmy 審核。現行做法是每日交付夾 HT9045_V906_changes_YYYYMMDD（_README_FIRST.txt ＋ ChangeLog ＋ 7z），**備份完成後一定要用 Outlook 寄一封不含附件的通知信給 Jimmy**（使用者 20260921 指示，授權直接寄出）；檔案層 mirror 與 JSON Simulator 那條路已於 20260915 退場，僅保留作為歷史。"
---

# HT9045 HTML Mirror Backup

## ⛔ 先讀：本檔下半部的 manifest 是**已退場**的 A 路（Steven 20260918 更新）

下面「Scope／Required Order／Manifest／Copy and Verify Template」整段描述的是
**JSON Simulator ＋ JSON shim 的檔案層 mirror**，也就是 20260915 使用者裁決退場的那條路：

> 資料一律走 `wb_serve` 的 `/api/recipe`、`/api/system`、`/api/text`，**不啟用 C# JSON Simulator**。

保留它是當**歷史**（雜湊驗證、UTF-8 無 BOM 的做法仍可參考）。
**但不要照它做備份** —— 它會把已經不是資料來源的 JSON shim 複製到共用區，
讓收到的人以為那還是有效的交付面。

---

## ⛔ 兩條前提（20260922 學到的，都踩過）

### 1. Steven 這一側**不做 commit、不做 push**

那是 Jimmy 的權限。Steven 的流程只有兩步：**備份到共用區 -> 寄信通知**。

⇒ **交付包就是唯一的保全，沒有第二份可以對照。**
   所以備份的正確性不是「盡量」，是**唯一防線**：
   每一包都要做下面那道逐檔雜湊驗證，不可以靠「應該沒問題」。

### 2. 同一天第二包起，一定要用 b / c 尾碼

**不可以覆蓋既有的日期資料夾。** 收件端會分不清手上那份是哪一次的，
而且被覆蓋掉的那一版**救不回來**（沒有 commit 當備援，見上一條）。

    HT9045_V906_changes_20260922     第一包
    HT9045_V906_changes_20260922b    同日第二包
    HT9045_V906_changes_20260922c    同日第三包

20260922 當天覆蓋了兩次才被使用者指出來，最後要另發一封更正信。

---

## ⛔ 打包後必做：逐檔雜湊驗證

**20260922 實測：不做這一步就會放上舊檔，而且沒有任何徵兆。**

那天重打包時是**手挑**「要更新的檔案清單」，漏掉 4 支在那之後才改到的檔，
7z 完整性測試（`7z t`）照樣通過、檔數也對（32），
**只有把包解開來逐檔比對雜湊才發現裡面是舊版**。

### 正確做法：先寫明列清單，不要手挑

清單每一筆寫清楚「**包內路徑 <- 工作區來源**」，然後：

1. 清空暫存區（不要在舊的暫存區上疊加 —— 上一包的殘留會混進來）
2. 照清單複製，複製前檢查每一個來源都存在
3. 確認暫存區檔數 == 清單筆數
4. 打包、`7z t`、看**檔數**（見下面 7z 的坑）
5. **把包解開，逐檔 `Get-FileHash -Algorithm SHA256` 比對工作區**
6. 32/32 全部一致才放上共用區

```powershell
& $z x $arc ('-o' + $tmp) '-p<pw>' -y -bso0 -bsp0 | Out-Null
foreach ($f in (Get-ChildItem $tmp -Recurse -File)) {
  $rel  = $f.FullName.Substring($tmp.Length + 1)
  $live = <依 $rel 對回工作區路徑>
  if ((Get-FileHash $f.FullName -Algorithm SHA256).Hash -ne
      (Get-FileHash $live       -Algorithm SHA256).Hash) { '不同: ' + $rel }
}
```

⚠ `7z l` 的輸出**不要用空白切欄位**去取檔名 —— 路徑裡有空白就會切錯，
   結果是「全部檔案都不存在」這種明顯不對但很容易誤信的報告。
   用 `7z l <arc> -slt` 再抓 `^Path = (.+)$`。

---

## 現行做法：每日交付夾

**目的地**：`U:\共用區\HT-9050\HT9045_V906_changes_<YYYYMMDD>[b]\`
（同一天第二包加尾碼 `b`、第三包 `c`；實測既有 `20260915`／`20260915b`／`20260916`／
`20260916b`／`20260917`／`20260918`）

⚠ **備用路徑**（U: 沒掛上時）：
`\backsrv\RD\軟體備份區\邏輯機台\共用區\HT-9050\` —— 實測同一個位置，兩者都可達。

### 一包裡放什麼

| 檔案 | 內容 | 必要 |
|---|---|---|
| `_README_FIRST.txt` | **給 Jimmy 的交接信**，純文字。開頭寫基準 commit 與本包性質，然後是「要你做的事」逐項 | ✅ 一定要 |
| `CHANGES_<YYYYMMDD>_Steven.md` | 當日 ChangeLog（來源 `D:\docs\ChangeLog\`） | ✅ 一定要 |
| `HT9045_V906_changes_<YYYYMMDD>.7z` | 實際的檔案酬載 | 有改檔案就要 |
| 其他報告 `.md` | 該次特有的稽核／盤點報告 | 視情況 |

### `_README_FIRST.txt` 的寫法（照既有格式）

* 純文字，**不是 markdown** —— Jimmy 可能用記事本開
* 開頭固定三行：交付日期／Steven -> Jimmy、基準 commit（含分支）、**本包性質**
* 若本包不含 web 交付，**在最前面明講**（既有的 20260918 那包就寫了「** 這次沒有 web 交付 **」）
* 然後是「先看這個」（指向 ChangeLog 的哪幾節）與「要你做的 N 件事」
* ⚠ **用人名不用代名詞**（Steven／Jimmy）—— 跨人交付文件裡代名詞會失去指涉

### 驗證

1. 複製後逐檔比對雜湊（原本的 `Get-FileHash -Algorithm SHA256` 做法仍然適用）
2. 所有文字檔 **UTF-8 無 BOM**（BCB 程式碼除外，那維持 Big5）
3. 寫檔一律「先在記憶體 encode 成 bytes，成功才開檔以 `wb` 寫」

⚠ **不要把 `D:\HT9045\page\`（已退場的舊樹）當成交付內容。**
交付樹是 `D:\HT9045\web\`，原始樹是 `D:\HT9045\client\`。
唯一的例外是兩份 `screenshot_meta.js` 依使用者指示保持同步。

---

## 備份完成後：**一定要寄一封通知信**（使用者 20260921 指示）

> **這是流程的一部分，不是附加選項。** 交付夾複製完、雜湊驗過之後，
> 接著就寄信通知 Jimmy。使用者 20260921 明確授權：**直接寄出，不必先開草稿等確認。**

### 收件人與性質

| | |
|---|---|
| 收件者 | `jimmychiu@honprec.com` |
| 副本 | `steven@honprec.com`（寄件備份之一，見下） |
| 附件 | **不放**。信是「通知 ＋ 說明」，檔案在共用區 |
| 格式 | **純文字**（`BodyFormat = 1`）—— Jimmy 可能用記事本或手機看 |
| 主旨 | `HT9045 web HMI <YYYYMMDD> 備份已上傳共用區（含修改說明，N 件需 Jimmy 裁決）` |

⚠ **不放附件是使用者 20260921 的指示。** 早期版本把 `_README_FIRST.txt` ＋
ChangeLog ＋ 7z 三個檔一起夾帶，已取消 —— 共用區才是唯一的交付面，
信裡再附一份會出現兩個版本，而且沒有人知道哪一份才是最新。

### 內文結構（照這個順序）

1. 一句話：**備份已上傳**，本信是通知加說明、沒有附件
2. **備份位置** —— 完整路徑 ＋ `U:` 沒掛上時的 `\\backsrv\...` 備援路徑，
   並列出資料夾裡有哪幾個檔（檔名，不是內容）
3. 展開方向的警告：`client\` 是原始樹、`web\page\` 是部署樹，只能 client → web
4. 一句話現況 ＋ 基準分支與 commit ＋「未 commit／未 push，依慣例只備份待審」
5. **修改事項** —— 當日做了什麼，對到 ChangeLog 的節號
6. **注意事項** —— 需要 Jimmy 裁決的事排第一，並寫明「哪幾件需要回覆」
7. 刻意的界線（不需回覆，但講清楚免得誤會）
8. 驗證方式與**限制**（例如「全部是 headless，真瀏覽器還沒看過」）

⚠ **用人名不用代名詞**（Steven／Jimmy）—— 跨人信件裡代名詞會失去指涉。
⚠ 沒把握的事寫清楚是推論還是量測（例：「這一段是依規範推得，還沒在真的瀏覽器上按過」）。

### 寄件備份，三重

1. **副本**抄給 `steven@honprec.com`
2. `DeleteAfterSubmit = $false`，`SaveSentMessageFolder` 指到寄件備份匣
3. **檔案存查**：`D:\docs\ops\daily\<YYYYMMDD>_mail_to_Jimmy.txt`
   —— 內文**從草稿讀出來**再寫檔，確保與實際寄出的一字不差；
   檔頭補寄件者／收件者／副本／主旨／附件數／備份位置。UTF-8 無 BOM。

### 做法：Outlook COM（這台機器沒有 email 工具）

Outlook 已安裝且通常在執行中，帳號 `steven@honprec.com`。

```powershell
# 1) 內文先寫成 UTF-8 檔，再讀進來 —— 不要把內文直接內嵌在指令裡
$body = Get-Content -Raw -Encoding UTF8 'X:\...\mail_body.txt'

$ol   = [Runtime.InteropServices.Marshal]::GetActiveObject('Outlook.Application')
$mail = $ol.CreateItem(0)
$mail.To          = 'jimmychiu@honprec.com'
$mail.CC          = 'steven@honprec.com'
$mail.Subject     = 'HT9045 web HMI 20260921 備份已上傳共用區（含修改說明，1 件需 Jimmy 裁決）'
$mail.BodyFormat  = 1          # 1 = 純文字
$mail.Body        = $body
$mail.DeleteAfterSubmit = $false
$mail.Save()

# 2) 寄出前先確認收件者真的解析到對的人
foreach ($r in $mail.Recipients) { $r.Resolve() | Out-Null; $r.Name + '  ' + $r.Address }

# 3) 寄出
$mail.Send()
```

### 寄完一定要驗（三個數字）

```powershell
$ns = $ol.GetNamespace('MAPI')
$ns.GetDefaultFolder(5).Items  | ? { $_.Subject -like 'HT9045*<YYYYMMDD>*' }   # 寄件備份匣：要找得到
($ns.GetDefaultFolder(16).Items | ? { $_.Subject -like 'HT9045*<YYYYMMDD>*' }).Count  # 草稿匣：要 0
($ns.GetDefaultFolder(4).Items).Count                                          # 寄件匣 Outbox：要 0
```

`Send()` 回來不等於信出去了 —— 卡在 Outbox 也是「沒有錯誤」。三個都對才算寄出。

### ⚠ 打包時的路徑坑（20260922 實測）

**`7z.exe` 是 Windows 程式，吃不了 Git Bash 的 POSIX 路徑。**
在 Bash 工具裡跑
`"/c/Program Files/7-Zip/7z.exe" a -t7z out.7z "/c/Users/.../stage/*"`
不會報錯 —— 它回 `Scan WARNINGS: 1`、**產生一個 32 bytes 的空壓縮檔**，
而且 `7z t` 對空檔照樣回 `Everything is Ok`。
⇒ 光看「完整性測試通過」會以為打包成功，其實裡面 0 個檔。

**打包一律用 PowerShell 工具跑**，路徑寫 Windows 形式：

```powershell
Push-Location $stage
# AI(W906-7ZPW) 20260925: 密碼只記本機（Jimmy 裁決 4A：換密碼、不進版控）。沒設就停，不要用舊密碼或空密碼打包。
if (-not $env:HT9045_7Z_PW) { throw '請先設定環境變數 HT9045_7Z_PW（7z 密碼向 Jimmy 口頭取得，不寫進任何檔）' }
& 'C:\Program Files\7-Zip\7z.exe' a -t7z $arc '*' "-p$env:HT9045_7Z_PW" -mhe=on -bso0 -bsp0
Pop-Location
```

**驗法不能只看 `7z t`，要看檔數**：
`(& $z l $arc '-p<pw>') | Select-Object -Last 1` 要印出預期的 `N files`。

### 四個踩過的坑

1. **`GetExchangeUser()` 對 SMTP 收件者回 `null`。**
   Jimmy 在 GAL 裡解析成顯示名「Jimmychiu(邱健銘3515)」，但 `GetExchangeUser()`
   是 `null`，要改讀 `$r.Address` 才看得到 `jimmychiu@honprec.com`。
   寄之前**一定**要把位址印出來確認 —— 顯示名對不代表位址對。

2. **`$r.Address` 對 Exchange 內部帳號會回 X.500 DN。**
   Steven 自己那筆會印出 `/o=Honprec/ou=Exchange Administrative Group...`。
   寫存查檔時要先試 `GetExchangeUser().PrimarySmtpAddress`，回 null 才退回 `$r.Address`。

3. **內文不要走 bash heredoc → python 寫檔那條路。** 實測反斜線被吃掉**兩層**
   （`\\\\backsrv` 變成 `\backsrv`，而行尾的 `\\` 變成 Python 的續行符把兩行黏在一起）。
   路徑寫錯的信送出去就收不回來。用會逐字寫入的方式產生內文檔。

4. **PowerShell 寫檔預設帶 BOM 或走 ANSI。** 存查檔用
   `[IO.File]::WriteAllText($p, $txt, (New-Object System.Text.UTF8Encoding($false)))`。

5. **Edit 工具會把 CRLF 正規化成 LF（20260922 實測，每次都會）。**
   ChangeLog 與信稿的行尾要維持 CRLF（與已交付的歷次一致）。
   信稿這點尤其要緊 —— 它會被 `Get-Content -Raw` 讀進 Outlook 的純文字 `.Body`。

   **解法：改這兩個檔不要用 Edit，用 Python 做字串取代**
   —— `newline=None` 讀（吃掉 CRLF）、`newline='\r\n'` 寫（原樣還原），
   全程不經過 Edit 就不會被正規化：

   ```python
   s = io.open(p, encoding='utf-8', newline=None).read()
   n = s.count(old); assert n == 1, '命中 %d 次，中止' % n   # 非唯一命中就停，不要賭
   io.open(p, 'w', encoding='utf-8', newline='\r\n').write(s.replace(old, new))
   ```

   驗法：`CR 位元組數 == LF 位元組數 == 行數`（有孤立的 CR 或 LF 就是壞了）。
   順便檢查 Markdown 的 code fence 數量是偶數 —— 取代時吃掉半個 fence 不會報錯，
   但整份文件後半會變成一塊程式碼。

---

## 以下為 A 路歷史紀錄（⛔ 不要照做，見本檔開頭）

## Scope

- Source root: `D:\HT9045`
- Mirror root: `U:\共用區\HT-9050\機台介面 HTML\HT9045`
- Purpose: copy the edited HTML/JSON delivery surface and JSON Simulator source without copying build artifacts.

## Required Order

1. Keep HTML, JSON, and `JSON\js` shim files as UTF-8 without BOM.
2. Before each backup, run:

```powershell
& 'D:\HT9045\.venv\Scripts\python.exe' 'D:\AI_TempFile\_gen_json_shim.py'
```

3. Validate changed JSON using a UTF-8 parser and validate edited JavaScript or C# with its focused compiler/check.
4. Create destination directories as needed, then copy only the manifest entries below.
5. Compare source and target with `Get-FileHash -Algorithm SHA256`; every file must match.
6. Do not copy `JSON-Simulator\bin` or `JSON-Simulator\obj`.

## Manifest

| Source relative path | Mirror relative path | Inclusion rule |
|---|---|---|
| `page\Main.MotionView.html` | same | Motion View implementation |
| `page\main-control.js` | same | Main-to-Simulator commands when changed |
| `page\simulator-bridge.js` | same | WebSocket bridge when changed |
| `page\settings.js` | same | Production JSON merge when changed |
| `page\i18n.js` | same | language broadcast support when changed |
| `JSON\MotionView-i18n.json` | same | always with Motion View language changes |
| `JSON\Production-update.json` | same | Runtime skeleton/snapshot |
| `JSON\Runtime-bridge-contract.json` | same | Runtime producer contract |
| `JSON\js\MotionView-i18n.js` | same | generated shim |
| `JSON\js\Production-update.js` | same | generated shim |
| `JSON\js\Runtime-bridge-contract.js` | same | generated shim |
| `JSON-Simulator\Assets\` | same | copy recursively when Simulator is included |
| `JSON-Simulator\Form1.cs` | same | Simulator source |
| `JSON-Simulator\JsonBridge.cs` | same | Simulator bridge source |
| `JSON-Simulator\Program.cs` | same | Simulator entry point |
| `JSON-Simulator\HT9045.JsonSimulator.csproj` | same | Simulator project definition |
| `JSON-Simulator\HT9045.JsonSimulator.csproj.user` | same | Simulator local project settings when present |
| `JSON-Simulator\README.md` | same | Simulator operation notes |
| `.github\skills\ht9045-html-mirror-backup\SKILL.md` | `.github\skills\ht9045-html-mirror-backup\SKILL.md` | retain the backup process at the mirror |

Project skills and operational reports do not belong below the HTML mirror root. Keep their authoritative copies in their own repository roots unless the recipient explicitly asks for a documentation mirror.

## Copy and Verify Template

```powershell
$src = 'D:\HT9045'
$dst = 'U:\共用區\HT-9050\機台介面 HTML\HT9045'
$files = @(
  'page\Main.MotionView.html', 'page\main-control.js', 'page\simulator-bridge.js',
  'page\settings.js', 'page\i18n.js',
  'JSON\MotionView-i18n.json', 'JSON\Production-update.json', 'JSON\Runtime-bridge-contract.json',
  'JSON\js\MotionView-i18n.js', 'JSON\js\Production-update.js', 'JSON\js\Runtime-bridge-contract.js',
  'JSON-Simulator\Form1.cs', 'JSON-Simulator\JsonBridge.cs', 'JSON-Simulator\Program.cs',
  'JSON-Simulator\HT9045.JsonSimulator.csproj', 'JSON-Simulator\HT9045.JsonSimulator.csproj.user',
  'JSON-Simulator\README.md', '.github\skills\ht9045-html-mirror-backup\SKILL.md'
)
foreach($relative in $files){
  $target = Join-Path $dst $relative
  New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null
  Copy-Item (Join-Path $src $relative) $target -Force
  if((Get-FileHash (Join-Path $src $relative)).Hash -ne (Get-FileHash $target).Hash){ throw "Hash mismatch: $relative" }
}
Copy-Item (Join-Path $src 'JSON-Simulator\Assets') (Join-Path $dst 'JSON-Simulator\Assets') -Recurse -Force
```