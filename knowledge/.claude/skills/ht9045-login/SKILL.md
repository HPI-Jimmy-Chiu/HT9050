---
name: ht9045-login
description: >
  HT9045 登入與密碼知識庫：三種密碼本（文字密碼本 tech.com／userid.com、二進位 login.dat、下拉選單模式）怎麼選、
  golden（V912）8 條「登入密碼寫檔／讀檔」路徑、EncodeStr／DecodeStr 金鑰轉換（HontechPassword）、login.dat 的
  PASS_WORD 結構（64004 位元組）與外部工具 PW_Editor、最高權限 HONPREC、Supervisor 密碼、bPasswordSecret 首次整本轉換、
  golden 既有缺陷（Edit 寫回舊密碼、SECS/GEM 遠端改密碼不看加密、EncodeStr 產生分隔字元），以及 V906 移植樹現況
  （auth.login、security.passwd、WebLogin.cpp、WebLevelSet.cpp、網頁不送密碼、WS 只聽 127.0.0.1）與 Steven 的裁決
  （S55、Q8、Q9、Q10＝B1、Q11＝B、Q24、Q25～Q29、Q30＝B）。
  Use when：登入失敗、密碼改不掉、改密碼顯示成功但沒改、login.dat、tech.com、userid.com、密碼本、加密模式、
  bPasswordSecret、bUseLoginDatToSetLevel、PW_Editor、EncodeStr、DecodeStr、HONPREC 密碼、Supervisor 密碼、
  網頁登入 auth.login、Status.Security 改密碼、security.passwd、WebLogin.cpp、客戶碼 791、levelset.dat 權限、
  A01 閒置自動登出（切回 Operator）、A01 自動重開。
  關鍵字：login, 登入, 密碼, password, login.dat, tech.com, userid.com, PASS_WORD, USER, ReadPassword, SavePassword,
  ChangePassword, EncodeStr, DecodeStr, asKeyStr, HontechPassword, bPasswordSecret, bUseLoginDatToSetLevel,
  PW_Editor, password_editor, HONPREC, HONTECH, szSupervisor, MES1675, WAR1677, WAR1678, WAR1682, auth.login,
  auth.select, security.passwd, WebLogin_BookLogin, W906_SecurityPasswdOp, binary-book, levelset.dat, Insufficient,
  A01, 自動登出, iOperatorModeCount, iOpenA01Count, bAutoSwitchToOperatorMode, iA01ChangeOpTime, bAutoOpenConfigA01,
  W906_A01AutoLogoutTick, W906_WebLoginForceOperator, D-015, palSetup, palConfig, Tools 選單, Config 選單,
  act.main.menuVisible, W906_A01MenuVisibleOp, W906_ModalWaitTick, W906_A01CONFIGINI_PATH, D-015 A01b、
  告警框密碼（D-026）：dialog.auth、TfNote::DoPassword、DoUnlockPassword、GetJamLevel、bNeedPassWord、bAlarmUnlockPassWord、
  asUnlockPassword、AlarmUnlock.ini、W906_ALARMUNLOCK_PATH、W906_NoteAuthVerify、W906_NoteAuthAnswerGate、auth-required、verifyAuth、
  另外兩個密碼框（D-034）：SpecialPanel、PanSpecialNoteClick、bErrPan_err、SpecialErrNote.ini、special-note、W906_SpecialPanelLocked、
  MyMessageBox::DoPassword_MBox、mbox-password、W906_DoPasswordMBox、W906_ReauthConfigI37、[I37_1] FIFO、i37_1、reauthAll、W906_ReauthHasAnswerFor。
  golden 8 條路徑與移植樹對照全文 → references/password-paths.md；login.dat 位元組格式與 PW_Editor → references/login-dat-format.md
---

# HT9045 登入與密碼

> 所有路徑都是絕對路徑。golden＝V912 量產碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；網頁＝`D:\HT9045\web\page\`。
> 內容出自 St01 工程線（ST01-E）20260927 逐行核對 V912 的結果（Steven todo ★ Q10／Q11，`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-pending.md`／`decisions-decided.md`）。
> 行號是 20260927 當下的；移植樹 `WebLogin.cpp` 已因 Q11（`c9d0dd00`）位移過，本檔已更新到修改後的行號；Q9（St02 `64e2c048`）已做，表內 login.dat 寫入與關頁那兩列是 20260927 晚上更新的行號。20260927 ST01-E 核對過一輪（7 處更正＋補充已併入）。

## 1. 用哪一本密碼本（由客戶碼決定）

golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10843-10863`：

| 條件 | pwPath | 模式 |
|---|---|---|
| `CUSTOMER_CODE==CC_AMKOR_China` 或 `CC_QUALCOMM` | `D:\HT9045\system\userid.com` | 文字密碼本 |
| `CosFunction.bUseLoginDatToSetLevel`（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp` 裡 13 處設 true：大多是單一客戶碼，例 791＝CC_SJ_Semiconductor `:1604`；但 `:2387` 是 VTEST_Funtion、`:3250` 是 MaximFunction（功能群組，不是單一客戶碼）；`:3812`（華天南京）不是固定 true，讀 `D:\HT9045\config\LoggingType.ini [Login] UseLoginDat`（預設 false）） | `D:\HT9045\system\login.dat` | 二進位密碼本 |
| `CosFunction.bOEEFunction` | 空字串 | pwPath 空字串時 `FileExists` 為假 ⇒ **下拉選單模式**（`cbUserSelect`，golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10865-10881`） |
| 其他 | `C:\winnt\system32\tech.com` | 檔案存在＝文字密碼本；不存在＝**下拉選單模式** |

判斷式（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10865-10881`）：只有「pwPath 檔案存在」或 `bUseLoginDatToSetLevel` 才顯示帳號密碼登入，其餘都是下拉選單模式——所以 AMKOR China／QUALCOMM 的 `D:\HT9045\system\userid.com` 不存在時也是下拉模式。

Steven01 這台 `D:\HT9045\system\Gerneral.ini` 的 `CUSTOMER_CODE=791` ⇒ 走 login.dat；`C:\winnt\system32\tech.com` 不存在。

## 2. 金鑰轉換 EncodeStr／DecodeStr

- 金鑰 `asKeyStr="HontechPassword"`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:46`、`:201`，註解「密碼本的金鑰, 不能改」）。
- `EncodeStr`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\common.cpp:267-293`）：每個字元 `(c-1) XOR 金鑰[p]`，金鑰 15 字循環；結果是 0 時改放金鑰那個字元（避免 \0 截斷，20210330 修正）。`DecodeStr`（同檔 `:295-321`）反算。
- **這是換字，不是加密**：知道金鑰就能還原。
- 例：`1234` → `x^\G`（`78 5E 5C 47`）；帳號 `op1` → `&o^`（`26 6F 5E`）。
- 潛在問題（golden 同）：有些字母在某些位置會轉成空白、逗號、冒號、Tab、CR、LF（例：第 1 個字是 'i' 變空白、'C' 變換行），加密模式下文字密碼本那一行會被切錯（切欄 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cpublic.cpp:312-349`），那個帳號登不進去。數字 0～9 安全。

## 3. 八條路徑一覽（寫入點、讀出點、移植樹對照見 references/password-paths.md）

| # | 檔 | 存的是 |
|---|---|---|
| 1 | 文字密碼本，`bPasswordSecret`=false | 明文 |
| 2 | 文字密碼本，`bPasswordSecret`=true 且 `D:\HT9045\system\Gerneral.ini [Password] Change`=0 | 第一次按改密碼鈕時整本密碼欄轉 EncodeStr、Change=1 |
| 3 | 文字密碼本，`bPasswordSecret`=true 且 Change=1 | 密碼欄 EncodeStr |
| 4 | 文字密碼本，SECS/GEM 遠端改密碼 | 明文（不看加密旗標，golden 潛在缺陷） |
| 5 | `D:\HT9045\system\login.dat`（二進位，`bUseLoginDatToSetLevel`） | 帳號、密碼都 EncodeStr；**handler 不產生這本**（外部工具 PW_Editor 產生） |
| 6 | `D:\HT9045\system\login.dat`（下拉選單模式） | 明文，帳號欄留空 |
| 7 | `D:\HT9045\system\Gerneral.ini [VENDER] HONPREC`（最高權限） | 明文 |
| 8 | `D:\HT9045\system\lastdata.dat`（`LastSet.szSupervisor`） | 明文 |

`D:\HT9045\system\levelset.dat` 沒有密碼，只有 256 個權限等級。

## 4. golden 既有缺陷

- **Edit 寫回舊密碼**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:756-789`，`:779` 用 `edLoginOldPassword`）：文字密碼本、沒開加密時，Edit 顯示 MES1675「修改使用者完成」，實際新密碼被丟掉。V899 同。**Steven Q11＝B：移植樹已修（`c9d0dd00`）**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` Edit 段 `:1120-1159`，寫新密碼 `:1150`，新密碼空白回 WAR1678 `:1123-1127`（加密、非加密都擋），存檔 `:1162`；V912 不改（RULINGS_20260927 第 1 條），只通報 Jimmy。
- **login.dat 模式下改密碼**（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:700-791`）：`FileExists(login.dat)` 為真（同檔 `:611`），改密碼走文字路徑，二進位檔被當文字讀。New 顯示 MES1673「新增完成」（`:726`）；Edit／Delete 對不到 EncodeStr 過的帳號，顯示 WAR1677（`:753`、`:786`）。而且 `:791` 一律把 login.dat 存成文字，要等關 Security 頁時 `SavePassword`（`:464`）才用記憶體蓋回去——**中間當機的話 login.dat 就壞了**。
- **SECS/GEM 遠端改密碼**（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:32819-32850`）不看 `bPasswordSecret`，加密模式下被改的那行變明文，下次登入 DecodeStr 對不上。另外它用 pwPath 的 `LoadFromFile`／`SaveToFile`（`:32826-32849`），在 `bUseLoginDatToSetLevel` 機台上會把二進位 login.dat 當文字讀、再存成文字（例：華天南京 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:3812-3816` 開了 UseLoginDat 時）。只記給 Jimmy；移植樹沒翻這段。
- **PW_Editor 與 handler 的 EncodeStr 有一處不同**：轉出 0 時 PW_Editor 寫 00（字串斷掉），handler 寫金鑰字元。移植樹寫 login.dat 要用 handler 版才讀得回來；代價是用 handler 版寫的檔，那個字在 PW_Editor 打開時會顯示錯（PW_Editor 的 DecodeStr `D:\HT9045\Password_V1.00.648\password_editor.cpp:217` 只認 0），這一格兩邊不可能一致（見 references/login-dat-format.md）。

## 5. V906 移植樹現況（20260927）

| 功能 | 位置 | 狀態 |
|---|---|---|
| 網頁登入 `auth.login`（文字密碼本／login.dat 讀出） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383-470`（`WebLogin_BookLogin`；login.dat `:411-439`） | 照 golden |
| 下拉模式登入 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:210` | 照 golden |
| HONPREC 讀取（缺鍵補寫） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:176-187` | 照 golden |
| 網頁改密碼 `security.passwd`（第 1～3、6 條） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:563-1167`（入口 `W906_SecurityPasswdOp` `:829`） | 照 golden；Q11 缺陷已修（`c9d0dd00`，Edit `:1120-1159`） |
| login.dat 寫入 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:930-934` → 檔尾 `W906_PwBinaryBook`（＋`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\LoginDatBook.h`） | Q9＝B 已做：新增／刪除／修改（handler 版 EncodeStr、64004 位元組、備份→寫→驗證→還原、留最新一份 `.bak`）；寫完真的 login.dat 會重載 USER；St02 `64e2c048`（`v906/steven-st02-on-cbridge`，推到 `7f0c24b1`），已合進 St01 分支 `221cc069`；ctest `Security_LoginDatBook` 兩組態過（ST01-M 代跑 `742024b7`） |
| 開機 ReadPassword | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3471` | 有 |
| 關 Security 頁 SavePassword | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:121`（`W906EnvSet`）、`:124`（`SkipPasswordGuard(bool)`）、`:624`；開關 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:532`／`:535`／`:2119` | Q24＝B 已做：照 golden 呼叫 SavePassword／ReadPassword；設了 `W906_LOGINDAT_PATH`／`W906_PWBOOK_PATH`／`W906_LEVELSET_PATH` 任一個才跳過；St02 `7f0c24b1`，已合進 St01 分支 `221cc069` |
| SECS/GEM 遠端改密碼（第 4 條） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:512` | 沒翻（只計數） |
| Supervisor 密碼寫入（第 8 條） | — | 沒翻（讀取 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:90-91` 有做） |

**網頁傳輸**（golden 沒有這段；Steven Q10＝B1：維持現狀）：
- C++ → 網頁：回應一律不含密碼（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:591`、名單 `:884`），不 printf；頁面不 console.log（`D:\HT9045\web\page\ht9045_wire_statussecurity.js:392`）。
- 網頁 → C++：密碼原字放在 WS 訊息（登入 `D:\HT9045\web\page\ht9045_recipe_client.js:550-551`，改密碼 `D:\HT9045\web\page\ht9045_wire_statussecurity.js:541`）。
- WS 只聽 127.0.0.1（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.h:39`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4305`），封包不離開這台電腦。

## 6. Steven 的裁決（登入／密碼／權限）

| 題 | 裁決 | 意思 |
|---|---|---|
| S55（20260926） | 「名單可以，密碼需要加密」 | 名單可以回傳；密碼不能明文出現在網頁或傳輸內容 ⇒ 實作成回應不帶任何密碼 |
| Q8（S130） | A | 傳輸層不另外加密 |
| Q9（S131） | B | 之後支援 login.dat 讀寫（等 Q10；Q10 已定 B1）。做法備註（不是裁決本身）：64004 位元組、帳號／密碼 EncodeStr，用 handler 版；用 handler 版寫的檔，「轉出 0」那個字在 PW_Editor 打開時會顯示錯（`D:\HT9045\Password_V1.00.648\password_editor.cpp:217` 只認 0）；**已做**（St02 `64e2c048`，已合進 St01 `221cc069`；備份＝A 保留最新一份 `.bak`） |
| Q10（S132） | B1 | 存檔照 golden（已是）；網頁傳輸維持現狀，程式不改 |
| Q11（S133） | B | 修 Edit 寫回舊密碼，並擋新密碼空白（已修 `c9d0dd00`） |
| Q24（S144） | B | SavePassword／ReadPassword 照 golden 跑；**已做**（St02 `7f0c24b1`，只有設了測試縫才跳過） |
| Q30（S151） | B | `FormClose`／`SecurityExitClick` 接上、先修越界（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:618-624` 呼叫 golden FormClose）；SavePassword／ReadPassword 原本因 Q24 排後先跳過，Q24 做完後改成照 golden 呼叫、只有設了測試縫才跳過（`SkipPasswordGuard`，`:618-621`） |
| Q25～Q29（levelset.dat） | Q25 A、Q26 B、Q27 A、Q28 B、Q29 A | 存檔前重算 `Insufficient(29)`；停用格子只擋那一格；備份驗證過就刪；SecurityPalVisible 重排與 5 階 radio 現在做；檔案不在或大小不對拒寫 |

裁決正式紀錄：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md`（St01 分支；S 編號）。

## 7. 注意

- 不要把任何密碼（明文或 EncodeStr 後）寫進回應、log、printf、console。
- 改 login.dat 前先備份，寫完逐位元組驗證（64004 位元組）。
- golden 引用一律寫 V912 的全路徑；同名檔（例 `cSecurity.cpp`）移植樹也有，不要混用。

## 8. [A01] 閒置自動切回 Operator（todo D-015，20260930，AI(W906-D015-A01)）

- **golden**：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp` `TfMain::Timer3Timer`（Timer3 沒寫 Interval＝1000 ms，`main.dfm:17333`，計數單位是秒；第一關 `:25830` `InitialOK`）：`:25957-25982` 某些畫面開著就歸 0（SPIL／VTEST 只看 fiosetview／fSpeed／fObserver／fContact；其餘另加 palSetup／palConfig／fOffSet；SJ 永遠不歸 0；KYEC 開著 Offset 照數）；`:26022-26078` `IniConfig.bA01AutoSwitchToOperatorMode && AccessLevel>=1` 每秒 +1，滿 `max(10, iA01ChangeOpTime)` 秒就登出；`:26080-26096` `CosFunction.bAutoOpenConfigA01` 時 A01 被關 30 分鐘（1800 秒）自動打開並寫 `AuthPath+"config.ini"` [Function] bAutoSwitchToOperatorMode；`cOffSet.cpp:2811` Offset 存檔時歸 0。設定值：config.ini [Function] bAutoSwitchToOperatorMode／iChangeOpTime（客戶碼鎖定 `cConfiguration.cpp`、`cprod.cpp:3016-3021`／`:3190`；SECS EC 35000／35001）。
- **移植樹**：本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Main_A01AutoLogout.cpp`（`W906_A01AutoLogoutTick`；wb_serve 主迴圈 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5953` 同一行的 pumpBeat 呼叫，自己限成每秒一次）。登出本體跟 D4（Off-Line → On-Line）共用 `W906_WebLoginForceOperator`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:531`，經安裝座 `W906_WebLoginForceOperatorHook`）；log 先出一行 `[A01] idle ...`，接著是 D4 那一行 `[WebLogin] D4 Off-Line -> On-Line`（是同一段本體，不是真的切線上）。Offset 存檔歸 0：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.gen.inc`（設定 `tools\editlist\Offset_File.py`，只能 `gen_editlist.py --only Offset_File` 重產）。ctest `D015_A01AutoLogout`。
- ⚠ **SIM 組態永遠不會自動登出**：golden `:26026-26028` `#ifdef SOFT_SIMULTE iOperatorModeCount=0;` 照留 ⇒ 預設的 SIM 建置（F5）看不到 A01；出貨組態（`cmake -DW906_NO_SOFT_SIMULTE=ON`）才會登出。A01 自動重開不受 SOFT_SIMULTE 影響（SIM 也會寫 config.ini）。
- 畫面開著嗎：一律 `W906_FormShowing`（頁面表）。fOffSet、fSpeed 只問頁面表（TfOffSet 沒有 fShow；引用全域 fSpeed 會把 cSpeed.cpp 拉進 ctest 而連結失敗，網頁 Speed 頁的旗標在 `FileRW\ArmSpeed_File.gen.inc:32`）；fiosetview／fObserver／fContact 成員＋頁面表。
- fOffSet 關窗：`W906_FormProgramShow("fOffSet", false, …)` ⇒ 頁面表 want=close ⇒ `D:\HT9045\web\background.html` applyPageTable 關掉 Offset 視窗（`WebPageTable.cpp:58` 原本就把這一行列為 fOffSet 的程式寫入）。
- **Tools／Config 下拉選單（golden palSetup／palConfig，D-015 A01b，20260930，AI(W906-D015-A01b)，Jimmy 20260930 的答覆）**：
  - golden：`:25965` 選單開著 ⇒ 每秒歸 0（SJ 不歸；SPIL／VTEST 那一組不看選單）；V912 `ChangeLevelAttr` `:12928-12935`＋`:13182-13190`（Eastsun 20260526 #026-4.C4／C5）權限從高降到 0 ⇒ `sbExitSetup->Click()`／`sbExit->Click()` 收起兩個選單 —— 所以 A01（`:26042`）、Logout 鈕（`:28102`）、D4（`:12624`）三條路都收，A01 本體 `:26045-26048` 再藏一次。
  - 頁面：`D:\HT9045\web\page\ht9045_main_st01_ev.js` `initMenuReport`（main.html 不改）：MutationObserver 看 `#palSetup`／`#palConfig` 的 style.display，開／關時送一條 WS `act.main.menuVisible {"setup":bool,"config":bool}`，載入時先送兩個 false；只有失敗才重送（busy: 等 420 ms；modal-pending／權杖在別處／斷線 1→10 秒退避；unknown cmd 停），不輪詢、不加 tag。tag `auth.level`（既有）從 >0 變成 0 ⇒ 兩個選單 `style.display='none'`（第一個訊框只記值＝golden `iOldAccessLevel=-999`）。
  - C++：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:4826`（St01 那一行，`act.main.runMode` 那一支前面）分派到 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Main_A01AutoLogout.cpp` `W906_A01MenuVisibleOp`（只記兩個旗標，tick 執行緒，不用鎖）；`SamplePages` 的 `p.palSetup`／`p.palConfig` 讀它們。過期保險：`W906_FormShowing("fMain", false)`（頁面表主畫面列，規則 3～7）是 false ⇒ 旗標不算（瀏覽器全關 ⇒ 立刻；當掉 ⇒ 總表 15 秒過期而且沒有 WebSocket）。登出本體之後照 `:26045-26048` 清旗標；每一拍開頭也照 ChangeLevelAttr 的「降到 0」清（`s_iOldAccessLevel`，不受 InitialOK 管）。
  - [W906]：旗標只有一份（最後回報的 HMI 算數；多 HMI 時一個當掉、別的還開著，它最後的「開著」留到有人再開關選單）；auth.level 是訊框粒度；`sbExit->Click()` 另跑的 `SetWorkParameter()`／`UpdateMainOperateMode()`（`:28440-28446`）頁面不跑（main.html 自己的 Exit 項也不跑，既有缺口）；權杖照 `act.main.*`；阻塞框開著時回報收到 modal-pending（框關掉後補送）；wb_serve 重開時頁面不補送；golden `DoMainPadProcess`（`:3970-3978`，運轉中藏選單）不在本項。
  - ctest：`D015_A01AutoLogout` [9]（C++ 旗標）、`D015_A01MenuPage`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\d015_menu_selftest.cjs`，node 離線，真的頁面腳本＋假 DOM；對照組 `W906_MAIN_ST01_EV` 指向改之前的檔必須紅）。
- **阻塞框開著時照數**（D-015 A01b）：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7621`（`W906_ModalWaitTick` 第一行，St01 D-012 那一行的同一行）也呼叫 `W906_A01AutoLogoutTick()` ⇒ 同 golden（VCL Timer3 在 ShowModal 的訊息迴圈裡照跑）告警框／是否框／ShowMyMessage 開著時照樣數、時間到照樣登出。重入：本檔呼叫鏈（頁面表問答、D4 本體 DoChangeLevel／NewRecordProcess／TemperatureEditDisable、fOffSet 程式關窗只設 want、WriteIniData）查過都不開框；仍加 `g_inTick` 重入保險（巢狀呼叫直接 return），ctest [10] 驗、拿掉保險會紅。FormJson 鎖可重入（同一條 tick 執行緒）。
- 其他 [W906]：主畫面的 chk_Honprec_Use 當作沒勾；config.ini 測試縫 `W906_A01CONFIGINI_PATH`（沒設＝golden `AuthPath+"config.ini"`），開機的轉向清單 `W906_PrintDataRedirects`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7675`）有設就印出來（D-015 A01b）。
## 9. 告警框（Alert.Note）的密碼層（todo D-026，20261001，AI(W906-D026)）

- **golden**（V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\note.cpp`）：　（20261003 E-030：golden 以 906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\note.cpp` 為準（Jimmy RULINGS_20261002 #20／#23-6）；下面沒標的行號是 V912，FormShow 這段 906＝V912−10、DoPassword 906＝V912−40（WAR04217 那 7 行之後再 −7））
  - 開框時 `TfNote::FormShow` 決定要不要問：`:1496-1502` `bAlarmUnlockPassWord`（golden 是 cmydef 全域變數 `cmydef.cpp:4413`，不是 TfNote 成員；移植樹每一則各存一份）＝`fSecurity->GetJemUnlockPassWord`（只有 `CosFunction.bUseAlarmUnlockPassWord`，CC_ASE_M）；`:1818` `Level=fSecurity->GetJamLevel(sJamArea, sJamCode)`（`D:\HT9045\Error\English\JAM0000.dat`，大部分客戶碼還把一串固定代碼拉到權限表第 35 項，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1205`）；SCC 清單、HandlerResultServer、`Level!=0` ⇒ `bNeedPassWord`；O16 同一個 JAM 連 N 次、O17 單位時間內 N 次、KYEC_LEE＋N07 另外把 `bNeedPassWord` 打開（906 `:1841-1925`／V912 `:1851-1935`）。V912 另有一條「條碼 WAR04217（Barcode CSV Compare 開著）一律要密碼」（V912 `:1937-1942`，Ifor 20260511），**906 沒有，20261003 E-030 照 906 拿掉**。
  - 按 Start／Pause（`TfNote::Start` `:3560`、`BtnPauseClick` `:3866`；有選鍵 `:3599`／`:3921` 先 `DoUnlockPassword` 再 `DoPassword`；KeyCode==0 通知只有 `DoPassword` `:3724`／`:4092`）。F15：JAM0508／JAM0509 只有 SKIP 要問。
  - `DoPassword`（906 `:5237-5382`／V912 `:5277-5429`）：`bWaitSecsGemReply` 直接過；等級＝GetJamLevel 再加 O16（第 35 項）／O17（第 166 項）／SCC／VTEST（WAR16123＋RETRY 直接過）／統計第 5 次加一級／TCP；**只要 `bNeedPassWord` 就跳登入框**（已登入也問）：有密碼本＝`cbUserSelectChange(NULL)`、沒有＝`stOperatorClick`（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13285` fNote 那一臂：Supervisor 那格再 Engineer 那格）；`AccessLevel<iLevel` ⇒ 回 false、框留著；夠 ⇒ 記「==Login for unlock alarm.==」；**有密碼本時問完一律登出成 Operator**（CC_PTI 除外，`:5410-5425`）。
  - `DoUnlockPassword`（`:5431-5445`）：鍵盤輸入跟 `asUnlockPassword` 比（開機讀 `C:\Windows\AlarmUnlock.ini` 第一行，`main.cpp:11381-11403`；檔案不在時 golden 會寫一個內建密碼進去——本樹不帶那個密碼）。對了這一則就不再問。
  - 密碼錯：golden 的 `ShowErrorMessage("WAR1677")` 在框開著時走 `:814-818`「Alarm at same time」不顯示。
- **移植樹**：
  - 本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 檔尾（`:1881` 起，St02 的 1～1419 行與 B5 不動）；宣告 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebNoteAuth.h`。
  - 開框：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:445`（ForwardShowErrorMessage，紀錄之後）`W906_NoteAuthArm`；信箱的 `auth` 物件 `:377`／`:381` → `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_dialog_mailbox.h:576` `AlarmRequestJson` 多一個參數（空字串＝原本 `required:false` 那一段，位元組不變）。
  - 問答：WS `dialog.auth`（tag＝authId、value＝dialog-bridge 的 Dialog-auth-verify JSON）。阻塞告警在等待迴圈 `:671` 收，通知在主迴圈 `:4889-4904` 收；本體 `W906_NoteAuthVerify`。通過＝留一張**一次性通行**（這一則、這個鍵、這個按鈕；**不綁連線、不綁操作權杖、不綁登入的人**——Steven Q64 (3) 20261001 09:4x「知道密碼的人就能操作、解除告警」，golden 也只比打的字與等級），緊接著的回答用掉它（通知的回答 dialog.notifyAck 不帶按鈕，通知那張通行不比按鈕；頁面送的是 BtnPause）：`:585` `W906_NoteAuthAnswerGate`（modal.answer／dialog.response）、`:4889` `W906_NoteAuthNoticeGate`（dialog.notifyAck）。沒有通行又是 golden 會問的 ⇒ 回 `auth-required: …`，框留著。
  - 實體面板 START／PAUSE：`:684` `W906_NoteAuthIoGate`，golden 會跳密碼框的時候不關框（[W906] 只能在畫面上輸入）。
  - 權杖：`dialog.auth` 比照 `dialog.response` 免（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebBridge\WebBridgeServer.cpp:1448`）；防連點白名單、關站放行本來就有。
  - 頁面：`D:\HT9045\web\page\ht9045_dialog_host.js:283-352` `HTDialogHost.verifyAuth`（dialog-bridge.js 不改）；登入框按取消＝補送 `{"cancelled":true}`（Q45-4＝A：空白帳密＝錯，有密碼本時變 Operator）。
- **密碼**：只走本機 WS（127.0.0.1），比對只在 C++；回應、printf、log 都不含密碼（OPLOG 看到 `passw` 也不記內容）；打的字用完清成 0；頁面送出後把物件裡的密碼清空。
- **[W906]**：（1）golden 同一次按鍵裡問完就關框，網頁拆成 dialog.auth＋回答兩個指令，中間靠一次性通行；（2）解除密碼＋登入兩個框同一次按鍵（CC_ASE_M）拆成兩次 dialog.auth（回 `stage:"login"`）；（3）WAR1677 不發（golden 本來就不顯示）；（4）AlarmUnlock.ini 不在 ⇒ 什麼都比不中；（5）面板鍵不能代替畫面輸入（被擋的面板鍵不動畫面那張通行，免得頁面接著送的回答被拒、而 dialog-bridge.js 已經不再開登入框）。
- **沒做**：HandlerResultServer 面板鎖（`bNeedTCPAlarm` 的關框限制，解鎖指令沒翻）、SECS 工號檢查（St02，照 S25 結案）、Greatek FTP 密碼本。SpecialPanel 密碼與 `MyMessageBox::DoPassword_MBox` 在 20261002 D-034 做了，見 §10。
- **WAR04217 照 906（todo E-030，20261003，St01，`AI(W906-E030)`）**：V912 `note.cpp:1937-1942`（FormShow：`TestIF_File.bEnableBarCode && bEnableBarcodeCSVCompare && edErrorCode=="WAR04217"` ⇒ `bNeedPassWord=true`）與 `:5318-5323`（DoPassword：同條件 ⇒ `iLevel=1`，連 JAM0000.dat 設 2／3 級也蓋成 1）在 906 都沒有（906 FormShow `:1925` KYEC_LEE 之後直接 `:1927`；DoPassword `:5276` O17 之後直接 `:5278` SCC）。移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 兩段都拿掉 ⇒ WAR04217 跟其他代碼一樣：JAM 等級表 0 級（且 O16／O17／KYEC_LEE／SCC／TCP 都沒中）不問；設 N 級就要 N 級（V912 會降成 1）。目前移植樹沒有可達的 WAR04217：唯一發它的 `atester.cpp`（V912 `:1052-1067` 那段）條件被 T03 閘成 false（`DoBarcodeCSVCompare` 沒翻）。ctest `D026_NoteAuth` 第 4 節 406／412／413 釘住 906；對照組：把 V912 兩段放回去重編 ⇒ 第 4 節變紅。
- ctest：`D026_NoteAuth`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_note_auth.cpp`，假密碼本／假 JAM0000.dat／假 AlarmUnlock.ini 都在 build 目錄的 `note_auth_scratch`；對照組 `W906_D026_SRC_ROOT`）、`D026_NoteAuthPage`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\d026_note_auth_selftest.cjs`；對照組 `W906_DIALOG_HOST_JS`）。

## 10. 另外兩個密碼框：SpecialPanel 與 MyMessageBox::DoPassword_MBox（todo D-034，20261002，AI(W906-D034)）

同 D-026／B5 的設計（Q45「甲」）：網頁只收集打的字，**比對只在 C++**；回應、printf、log 都不含密碼（RULINGS_20261001 #40）。St02 那一半（SECS 工號檢查 `CheckEmployeeID`）照 S25 結案，計畫書 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\D034_PLAN_20261002.md`（St02 MR !88）。

### 10.1 SpecialPanel（契約 kind `special-note`）

- **golden**（906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\note.cpp`；V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\note.cpp` 寫在（）裡，AI(W906-E030-CITE) 20261003）：
  - 開框 `TfNote::FormShow` `:1564-1606`（V912 :1574-1616）：`IniConfig.bIndexDropNeedPwdByIni`（[I] Index 掉料 JAM0303～0306／0314／0315）或 `IniConfig.bTesterTimeUpErrorNeedPassword`（WAR07352）⇒ `bShowSpecialPan`；是 ⇒ `bErrPan_err=true`。`D:\HT9045\system\SpecialErrNote.ini`（`asErrNotePath`）`[SUCK] TestSuck==1` ⇒ 讀 `[MESSAGE] En／Ch`（`lblSpecialNoteEn／Ch`）與 `[PASSWORD] Pwd`；否則 `Pwd=""`、`bErrPan_err=false`。
  - 鎖：`bErrPan_err==true && Pwd!=""` 時每一個按鍵都直接 return —— `BtnSkipClick` `:2845`（V912 :2867；八個選鍵）、`ScanKey` `:2886-2891`（V912 :2908-2913；面板鍵，Alarm Reset 除外）、`BtnStartClick` `:3818`（V912 :3858；出貨組態）、`BtnPauseClick` `:3830`（V912 :3870）、`BtnResetClick` `:5221`（V912 :5261；在 `fMain->BtnResetClick` 之後）。
  - 解：`PanSpecialNoteClick` `:5442-5460`（V912 :5489-5507）：點紅色面板 → 小鍵盤 → 打的字＝`Pwd` ⇒ `bErrPan_err=false`（CC_LINGSEN＋Tester Time Up ⇒ `iTestTimeUpErrContinueR=2`）。沒有登入、沒有等級。
  - `bErrPan_err`／`Pwd` 是 note.cpp **檔案層級全域**（`:91`／`:119`，兩棵同號），不是每一則各一份：下一則 FormShow 不會清 `bErrPan_err`（只會重讀 `Pwd`），沒解鎖就關掉的框會讓下一則也鎖著（golden 怪處，照翻）。
- **移植樹**：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` D-026 那一段裡（`FormShowSpecialPanelBlock`、`PanSpecialNoteClick`、全域 `bErrPan_err`／`Pwd` 在 `noteauth` 命名空間）：
  - 信箱 `auth`：鎖著 ⇒ `kind:"special-note"`、`special:true`、每一個提供的鍵都在 `actions`、`prompt` 帶 SpecialErrNote.ini 的 En／Ch；dialog-bridge.js／login-page.js 不用改（`userIdRequired` 只在後面還要登入、又有密碼本時才是 true）。
  - `dialog.auth`：鎖著時先比特殊密碼（不跑 DoPassword）：錯 ⇒ `stage:"special"`；對了而這一鍵還要 DoUnlockPassword／DoPassword ⇒ `accepted:false`、`stage:"unlock"／"login"`（同 D-026 的兩段式）；都不用 ⇒ `accepted:true`。
  - 三個閘（`W906_NoteAuthAnswerGate`／`W906_NoteAuthIoGate`／`W906_NoteAuthNoticeGate`）鎖著一律拒；通知只在「是 PAUSE 那一按」時鎖（同 D-026 的 pause 2）。
  - 面板鍵選鍵：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:7324`／`:7326`（筆電檔，FROM_STEVEN §1 認領 C1／C2）`W906_SpecialPanelLocked()` ⇒ `blocked`（golden ScanKey 直接 return，連選鍵、關蜂鳴器都不做）。
  - 測試縫：沒有 getenv；測試直接把全域 `asErrNotePath` 指到 scratch 資料夾（`D026_NoteAuth` 8b）。
- **沒做**：Alert.Note 頁面上紅色 `PanSpecialNote` 的顯示（`dialog-page.js`，在 `tsRedAlarm` 隱藏分頁裡；現在靠登入框 Label5 的 prompt 顯示 En／Ch）；`TfNote::Timer1Timer` `:3278`／`:3286`（V912 :3307／:3315）的 Out Arm 掉料開第 6 門那一段（`bOpenSixDoor` 沒移植，那裡的 `Pwd` 是固定的鎖標記）；`forms/fNote_ShowError.cpp` 的 GATE B1 仍是 `#if 0`（通知那一條改由 NoticeGate 擋，註解過時，筆電的檔）。

### 10.2 MyMessageBox::DoPassword_MBox（契約 kind `mbox-password`）

- **golden** 906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\mymessbox.cpp:1208-1266`（V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\mymessbox.cpp:1228-1286`）：權限表**第 35 項**；`fInput->fShow==false`（伺服器沒有小鍵盤表單 ⇒ 一律走）；CC_PTI 另一條（出貨組態是廠商密碼 Gerneral.ini `[VENDER] HONPREC`，模擬組態 `#else #endif` 什麼都不做＝直接過）；其他：有密碼本 ⇒ `cbUserSelectChange(NULL)`、沒有 ⇒ `stOperatorClick`；`AccessLevel<iLevel && iLevel>0` ⇒ false（**第 35 項＝0 照樣跳登入框，打什麼都過**；沒有 REAL_TIME_CCD 條件）；有密碼本時問完**一律登出成 Operator**（`:1275-1282`）。
- golden 呼叫點與移植樹：
  - 906 `D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cConfiguration.cpp:7237-7253`（V912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:7353-7369`）`CheckConfigurationBeforeSave`：`cbI37_1`（FIFO）由關改開 ⇒ 問；錯 ⇒ `cbI37_1->Checked=false`、其他照存。**接上**：editlist.save 的 `reauth` point `"i37_1"` → `W906_ReauthConfigI37` → `W906_DoPasswordMBox`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp` 檔尾 D-034 段；宣告 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebReauth.h`）；產生檔 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc` 的 `IC_DoPasswordMBox`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\IniConfig.py` 的 replace＋member，`python tools/gen_editlist.py --only IniConfig` 重產，產生器輸出 LF、要轉回 CRLF）；沒帶答案照舊 `filerw::ELPasswordRefused`。CC_PTI 出貨組態照 Q45 #1～#3＝C：網頁版不提供（golden 的「錯」，不讀也不寫 Gerneral.ini）。
  - 同一次存檔 M01 也改了 ⇒ golden 問兩次 ⇒ `reauth` 可以是陣列 `[{point:"m01",…},{point:"i37_1",…}]`（重複的點整次拒），回應多一個 `reauthAll`；單一物件跟以前位元組相同。`IC_PasswordGuard` 改看 `W906_ReauthHasAnswerFor("IniConfig","m01")`。
  - extra.auth 的 `points[i37_1]`：`kind:"mbox-password"`、`armed`、`turnOnOnly`、`levelItem:35`、`level`、`logoutAfter`（有密碼本）；CC_PTI 出貨組態 `kind:"vendor"`、`armed:false`。
  - 頁面 `D:\HT9045\web\page\ht9045_iniconfig_auth_c.js`：FIFO 由關改開 ⇒ 登入小鍵盤（先 M01、再 FIFO）；取消＝`{cancelled:true}`。
  - `mymessbox.cpp:457-470` `pnlPauseClick` 的 `bMBoxNeedPassword`：**到不了** —— 只有 `ShowMyMessagePWD`（`:1164-1226`）會設它，而它沒移植（呼叫端是筆電的閘：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebStart.cpp` W906-ST-W7-C-DLG、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\PowerSavingMode.cpp` GATE (3)）。`W906_DoPasswordMBox` 的 `mboxArm`＝golden `main.cpp:13285`（stOperatorClick 的 Supervisor／Engineer 那一臂）已經備好給那一天用。
  - `pnlPauseClick` 的 SECS 分支（`:467-523`）與 `TSecsAlarmForm::btnOKClick`（`:1455-1490`）：只有 KYEC_LEE／JSCC_OS／SCC（`CosFunction.bUseN07_5`），RULINGS #25 客戶專屬先跳過；`fSecsAlarm` 在移植樹是 NULL。
- ctest：`WebLogin_Reauth` 6b（C++）、`D034_IniConfigFifoPage`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\webprobe\d034_iniconfig_fifo_selftest.cjs`，node 離線；對照組 `W906_INICONFIG_AUTH_JS`）、`D026_NoteAuth` 8b（SpecialPanel）。
