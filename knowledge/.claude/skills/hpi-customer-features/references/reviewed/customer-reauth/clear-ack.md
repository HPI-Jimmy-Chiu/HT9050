# 暫存清除、ack與效果界線

來源：[wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)的 `main`／`wc.cmd=="editlist.save"`／`W906ReauthWipe`／`st==200`；[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `W906_ReauthTake`／`W906_ReauthClear`／`Wipe`／`WipeJson`／`WipeNode`／`W906_ReauthAck`／`reauth::Stash`／`g`。

## 這條caller的時序

主迴圈在運轉／開窗閘／value前段通過後解析root，建立W906ReauthWipe物件，再呼叫W906_ReauthTake。reauthRefused非空時不進入PageSave；通過後才處理widgets／answers與對應save caller。該scope退出時析構呼叫W906_ReauthClear；存檔try的例外另轉500，FormUnlock後才續走ack判斷。這只是所選editlist.save分支，不宣稱所有HTTP／其他命令或中止情形有相同契約。

W906_ReauthTake在返回前呼叫WipeJson清分離的內嵌／root reauth，拒絕時另Clear。WipeJson會處理字串值、WipeNode遞迴child／next並刪除分離的JSON；Wipe對string現有字元填0後clear。Clear對g.answers中的password／userId呼叫Wipe，再以Stash()重設g。這些是已讀緩衝的清除敘述，不代表傳輸、原始wc.value、所有複本／容量或程序全部記憶體都已清除；未作安全性全量驗證。

## ack不是磁碟證據

caller只有st==200才呼叫W906_ReauthAck。Ack會把g.results的point、answered／asked／handled／passed、取消／權限／原因等欄位加入回覆；g.answers中used=false的答案另有「本次未到DoPassword」的記錄。空／不適合的ack或沒有答案／結果時可能不追加；多結果時有reauthAll。這次未追完used／results的所有寫者，不由Ack存在推定實際登入路徑已走完。

所選Ack直接輸出的欄位沒有password，login子物件則由r.login或WebLogin_StateJson提供；本次未追完那些來源，不能用這段檢查宣稱所有回覆內容已全面驗證。st==200與解析通過、答案被使用、登入passed、PageSave savedMark及[INI落盤](../customer-storage/ini.md)是不同證據，不能互相取代。

後續需核對實際驗證callee、所有stash／結果寫者、owner排隊至執行／併發、HTTP入口及最後consumer與磁碟效果。本次沒有送任何reauth答案、呼叫登入／存檔API或讀取現場憑證；只補文件與靜態來源manifest。
