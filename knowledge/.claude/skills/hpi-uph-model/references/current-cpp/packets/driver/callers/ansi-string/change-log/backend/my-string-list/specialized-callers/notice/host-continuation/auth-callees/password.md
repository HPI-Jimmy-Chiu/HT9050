# Press、unlock與DoPassword的資格

[上層](index.md)；[BookCompare與最後狀態](state.md)；[原文](source-manifest.json)。

`WebLogin.cpp` 的noteauth::Press先依F15條件：JAM0508／JAM0509且sel!=0時
清bNeedPassWord。sel>=0才走DoUnlockPassword；解鎖通過後，用probe檢查
是否還需登入，若需則loginNext=true且回false。最後DoPassword false就false，
其餘true。成功不能直接解讀為退役／告警副作用已完成。

## 解鎖檔載入一次與取消

DoUnlockPassword在bUseAlarmUnlockPassWord及n.bAlarmUnlockPassWord都true時
設unlockAsks；cred空即false。LoadUnlockPassword先設s_unlockRead=true，
路徑取W906_ALARMUNLOCK_PATH或預設AlarmUnlock.ini；缺檔設s_unlockMissing，
本樹沒有寫內建密碼。讀到檔取第一行；空檔取空字串。
缺檔不match，已載入後不在此函式重新載入；讀取途中例外沒有本地重設s_unlockRead。

typed在cancelled時是空字串，其他經reauth::Typed；非missing且字串相等才
清n.bAlarmUnlockPassWord及設unlockPassed。不能由cancelled推定必定失敗，
例如空檔／空輸入的正文比較條件仍存在；這是來源條件，沒有載入實際密碼檔測試。
最後n.bAlarmUnlockPassWord仍true即false，包括功能旗標沒進上述比較的情況。

## DoPassword的mode、等級與早退

mode依FileExists(reauth::BookPath())決定book／select。
bWaitSecsGemReply為true時直接回初值true；VTEST WAR16123且RETRY也早退true。
這些早退不等於本次credential完成比較，也不保證解除後能進其他handler。

required先取GetJamLevel，再依O16或else-if O17抬等級、SCC指定code／VTEST條件、
statistics高階與level上限、HandlerResultServer的TCP alarm條件調整。
原WAR04217移除與906／V912歷史裁決comment完整保留，未重新比較913golden。

bNeedPassWord且bEnter=false才設asks；cred空false。取消會轉空user/pass，
book路徑呼叫WebLogin_BookCompare；select路徑SetupArm後stOperatorClick。
兩者後以AccessLevel<iLevel決定bFlag，沒有直接將BookCompare每個錯誤碼轉false。
BookCompare BAD_CREDENTIALS會把AccessLevel清0；若required也0，這份比較仍可true，
並帶WAR1677 alarm。資格判斷與比對錯誤碼必須分開，未宣稱這是現場故障。
NO_BOOK回傳路徑未在BookCompare清舊等級；mode查檔後至callee讀檔之間的完整競態未測。

成功等級先放o->accessLevel，之後PTI保留；其他客戶的book分支把caption／itemIndex／
AccessLevel重設Operator，ChangeLevelAttr並設loggedOut，bFlag可能仍true。
因此reply的本次驗證等級與最後live level可能不同。book資格失敗也會走此登出尾段。
static bEnter用前後賦值防連點，沒有RAII清理；例外／重入與thread ownership未證明。
