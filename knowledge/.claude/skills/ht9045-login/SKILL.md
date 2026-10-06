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

# ht9045-login 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/auth/original-entry.md)

## 1. 用哪一本密碼本（由客戶碼決定）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#1-用哪一本密碼本由客戶碼決定)

## 2. 金鑰轉換 EncodeStr／DecodeStr

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#2-金鑰轉換-encodestrdecodestr)

## 3. 八條路徑一覽（寫入點、讀出點、移植樹對照見 references/password-paths.md）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#3-八條路徑一覽寫入點讀出點移植樹對照見-referencespassword-pathsmd)

## 4. golden 既有缺陷

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#4-golden-既有缺陷)

## 5. V906 移植樹現況（20260927）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#5-v906-移植樹現況20260927)

## 6. Steven 的裁決（登入／密碼／權限）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#6-steven-的裁決登入密碼權限)

## 7. 注意

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#7-注意)

## 8. [A01] 閒置自動切回 Operator（todo D-015，20260930，AI(W906-D015-A01)）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#8-a01-閒置自動切回-operatortodo-d-01520260930aiw906-d015-a01)

## 9. 告警框（Alert.Note）的密碼層（todo D-026，20261001，AI(W906-D026)）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#9-告警框alertnote的密碼層todo-d-02620261001aiw906-d026)

## 10. 另外兩個密碼框：SpecialPanel 與 MyMessageBox::DoPassword_MBox（todo D-034，20261002，AI(W906-D034)）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#10-另外兩個密碼框specialpanel-與-mymessageboxdopassword_mboxtodo-d-03420261002aiw906-d034)

### 10.1 SpecialPanel（契約 kind `special-note`）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#101-specialpanel契約-kind-special-note)

### 10.2 MyMessageBox::DoPassword_MBox（契約 kind `mbox-password`）

[讀取此節](../hpi-web-hmi/references/auth/original-entry.md#102-mymessageboxdopassword_mbox契約-kind-mbox-password)
