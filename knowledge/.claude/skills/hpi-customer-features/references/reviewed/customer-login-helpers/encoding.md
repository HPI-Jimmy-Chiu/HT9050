# pw::Acp 與 Typed 的失敗分流

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的匿名namespace `pw::Acp`；前層已讀 `reauth::Typed`，見[其helper路由](../customer-login/state.md)。本次只核對body及caller這一段，不宣稱所有平台／字元或登入caller已相容。

## Acp 的已讀路徑

Acp先檢查所有byte是否低於0x80，符合則以u.c_str()寫out並回true。其他輸入先呼叫MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS)取wn，wn<=0回false；建立wstring後再次呼叫轉換。再用WideCharToMultiByte(CP_ACP)取an並檢查an<=0／usedDef，通過後配置字串、第二次轉換、再查usedDef，最後以a.c_str()寫out並回true。

所讀body沒有另比較第二次MultiByteToWideChar／WideCharToMultiByte的返回長度；API／compat實作、目前CP_ACP與呼叫條件、字串constructor及全部失敗生命週期仍待查。這些是指定表達式／檢查的位置，沒有執行轉碼或驗證現場字元資料。

## caller 並非相同的拒絕契約

Typed的已讀body在Acp成功時回a，Acp失敗時改回AnsiString(u.c_str())；所以不能從Acp有false路徑，推定全部登入輸入在caller被拒絕。後續比較、C字串／長度、其他caller及權限變更需各自核對。

本層沒有讀入現場user／password／環境設定，也沒有測試BCB6字元行為；同題整合HT9050及其他機台的方法，實際版本／客戶／runtime仍分流，原候選與13列局部證據保持。
