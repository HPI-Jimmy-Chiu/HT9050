# golden（V912）登入密碼 8 條路徑與移植樹對照

> golden＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`；移植樹＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`；網頁＝`D:\HT9045\web\page\`。
> 出自 St01 工程線（ST01-E）20260927 逐行核對（Steven todo ★ Q10）。行號是 20260927 當下的；移植樹 `WebLogin.cpp` 的行號是 Q11（`c9d0dd00`）修改後的。

## 表一　golden 所有「登入密碼寫進檔、讀出檔」的路徑

| # | 檔 | 什麼時候走這條 | 檔裡存的是 | 寫入點 | 讀出時怎麼解 |
|---|---|---|---|---|---|
| 1 | 文字密碼本 `C:\winnt\system32\tech.com`／`D:\HT9045\system\userid.com`，每行「帳號 等級 密碼」 | 檔案存在，且 `bPasswordSecret`=false（預設 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:4125`） | 明文 | `ChangePassword`：New `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:723`、Delete `:748`、Edit `:779`（寫回舊密碼）、存檔 `:791` | 不解碼：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15141-15160` 讀密碼本進記憶體，`:15188` 之後用 `UpperCase()` 比對（不分大小寫；例 `:15264-15266`，ASECL 分支 `:15200`） |
| 2 | 同上 | `bPasswordSecret`=true（AMKOR China `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp:2030`、RF360 `:2467`、QUALCOMM `:2572`），且 `D:\HT9045\system\Gerneral.ini [Password] Change`=0 | 第一次按任何改密碼鈕：整本密碼欄改 EncodeStr、Change=1、跳「Password Secret finish!!」，這次不開對話框 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:613-621`、`:646-666` | — |
| 3 | 同上 | `bPasswordSecret`=true，且 Change=1 | 密碼欄 EncodeStr（帳號、等級不轉） | New `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:718`；Edit 用 EncodeStr(舊) 比對 `:759`、寫 EncodeStr(新) `:774` | 先 DecodeStr 再比對 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:15156-15159` |
| 4 | 同第 1～3 列 | SECS/GEM 主機遠端改密碼（呼叫點 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\SECSGEM\uHGemHT9045.cpp:2614`） | 明文（不看加密旗標） | `TfMain::ChangePassword` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:32819-32850` | 同第 1、3 列 |
| 5 | `D:\HT9045\system\login.dat`（二進位 PASS_WORD，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:2710-2716`） | `bUseLoginDatToSetLevel` 的客戶碼（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\CosFunction.cpp` 的 :1018／:1130／:1604／:1638／:1723／:2164／:2280／:2387／:2405／:2693／:3250／:3629／:3812；其中 `:2387` VTEST_Funtion、`:3250` MaximFunction 是功能群組，`:3812` 華天南京不是固定 true，讀 `D:\HT9045\config\LoggingType.ini [Login] UseLoginDat`（預設 false）） | 帳號、密碼都 EncodeStr | handler 不產生：外部工具 PW_Editor（`D:\HT9045\Password_V1.00.648\password_editor.cpp:73-122`，EncodeStr `:97`／`:100`）或網路下載（golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10852` 只設 `pwName="login.dat"`；真正下載在 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\KYECFTP\FTPClient.cpp:4537` `NMFTP2->Download`）；handler 只做開機 ReadPassword（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1374-1386`，呼叫點 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:9420`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cinitial.cpp:5847`），以及關 Security 頁時 SavePassword（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:464` → `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1360-1372`）原封寫回 | 帳號、密碼都先 DecodeStr `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14986-15029` |
| 6 | 同一個 `D:\HT9045\system\login.dat` | 下拉選單模式：Security 頁按 Engineer／Supervisor 鈕 | 明文，放第（等級－1）格，帳號留空 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:795-811`（`:809` strncpy）→ SavePassword `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1360-1372` | 不解碼，比對 `USER.PassWord[i]` `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13250-13320` |
| 7 | `D:\HT9045\system\Gerneral.ini [VENDER] HONPREC` | 所有模式 | 明文（缺鍵寫預設；舊鍵名 HONTECH 搬過來） | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13221-13232` | 不解碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:13264-13265` |
| 8 | `D:\HT9045\system\lastdata.dat`（`LastSet.szSupervisor`） | Configuration 頁 Supervisor 密碼鈕 | 明文 | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp:6052-6059`（跟 lastdata.dat 存，`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.cpp:1925`） | 不解碼 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:10568-10569` |
| — | `D:\HT9045\system\levelset.dat` | — | 沒有密碼，256 個權限等級（`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cprod.h:1164-1168`、`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.cpp:1474-1515`） | — | — |

FTP 下載密碼本：login.dat 見第 5 列；Greatek 走 FTP 下載**文字**密碼本，在 golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp:14956-14970`。

FTP、EP、ESD、RMS 等設備密碼不是登入密碼，不在這裡。

## 表二　移植樹每一條路怎麼做（20260927）

| # | 移植樹 | 跟 golden 一樣嗎 |
|---|---|---|
| 1～3 寫入 | `security.passwd`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:563-1167`，入口 `W906_SecurityPasswdOp` `:829`）：整本轉換 `:1014-1024`、New `:1072-1093`、Delete `:1094-1119`、Edit `:1120-1159`（寫新密碼 `:1150`、新密碼空白回 WAR1678 `:1123-1127`）、存檔 `:1162` | 一樣，只有 Q11 缺陷已修（`c9d0dd00`，Steven Q11＝B）：Edit 寫新密碼，新密碼空白回 WAR1678（加密、非加密都擋） |
| 1～3 讀出 | `WebLogin_BookLogin`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:383-470`，解碼條件 `:455-458`） | 一樣 |
| 4 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fMain.cpp:512` 的 `TfMain::ChangePassword` 只累加呼叫次數 | 還沒做 |
| 5 讀出 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:411-439`（fread＋DecodeStr）；名單 `:741-755` | 一樣 |
| 5 寫入 | `security.passwd` 回 `guard:"binary-book"`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:923-939`）；開機 ReadPassword `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:3471`；關頁照 golden 呼叫 FormClose，但用 `SkipPasswordGuard` 跳過 SavePassword／ReadPassword（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLevelSet.cpp:618-624`、`:100`、`:121`；開關 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:511`、`:532`、`:535`、`:2119`；Q24 排後、Q30＝B） | 不一樣：寫入直接回報做不到；等 Q9 |
| 6 | 下拉模式 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:941-978`（`:967` 存明文、`:968` 寫 login.dat）；登入 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:210` | 一樣 |
| 7 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:176-187` | 一樣 |
| 8 | 讀取 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebLogin.cpp:90-91`；寫入鈕沒翻 | 讀取一樣，寫入還沒做 |

## 例子（帳號 op1、等級 1、密碼 1234）

| 情況 | 位元組 |
|---|---|
| 第 1 列：tech.com 沒加密 | `op1 1 1234` ＝ `6F 70 31 20 31 20 31 32 33 34 0D 0A` |
| 第 3 列：tech.com 加密 | `op1 1 x^\G` ＝ `6F 70 31 20 31 20 78 5E 5C 47 0D 0A` |
| 第 5 列：login.dat（791，**handler 版** EncodeStr） | 第 k 格：帳號在位移 4+30k ＝ `26 6F 5E 00 …`（'p' 在第 2 位剛好「轉出 0」，handler 寫金鑰字元 `6F`；PW_Editor 會寫 `26 00`，帳號被截成 "&"）；密碼在 30004+30k ＝ `78 5E 5C 47 00 …`；等級在 60004+4k ＝ `01 00 00 00` |
| 第 6 列：login.dat（下拉模式） | Engineer 第 0 格：帳號位移 4 全 `00`；密碼位移 30004 ＝ `31 32 33 34 00 …`（明文） |
| 網頁 → C++ | `"oldPassword":"1234"`（Q10＝B1，維持） |
