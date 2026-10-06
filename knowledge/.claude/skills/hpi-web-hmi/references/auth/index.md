# 登入、權限與個別重驗

- [完整原入口](original-entry.md)：密碼本選路、金鑰轉換、golden缺陷、原裁決、idle降級與告警重驗。
- [password paths](references/password-paths.md)：八條路徑與BCB／移植樹對照。
- [login.dat格式](references/login-dat-format.md)：PASS_WORD與外部工具的原規格，二進位與文字模式不能互相覆寫。
- [原頁面Security資料](../pages/references/security-access-json.md)、[dialog bridge](../pages/references/dialog-bridge.md)、[目前main定位](../runtime/index.md)。

客戶碼、bUseLoginDatToSetLevel、bOEEFunction、bPasswordSecret決定不同讀寫路，機型不直接決定密碼本。登入成功、AccessLevel、個別設定reauth與告警解鎖是不同caller；Q11等差異保持BCB／V906版本，不把移植樹修正描述成V912已改。

目前函式定位：WebLogin.cpp的W906_Reauth、W906_ReauthHasAnswerFor、W906_ReauthConfigM01、W906_ReauthConfigI37、W906_NoteAuthVerify、W906_NoteAuthAnswerGate、W906_DoPasswordMBox；實際分派與條件仍需核對caller。本文沒有讀密碼本、登入帳號、驗密碼、呼叫API或改任何權限資料。
