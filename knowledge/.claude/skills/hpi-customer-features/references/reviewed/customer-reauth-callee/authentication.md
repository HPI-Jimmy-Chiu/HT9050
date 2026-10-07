# W906_Reauth 的權限與結果分支

來源：[WebLogin.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/76dd45f370a180f90917a13d4ceea3d41d7ad71e/HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp)的 `W906_Reauth`，定位 `config`、`out`、`REAL_TIME_CCD`、`iLevel`、`AccessLevel`、`bTechComExist`、`bFlag`。下述只覆蓋此 body；IsConfig、BookPath／BookOverride、Typed、SetupArm、WipeOnExit、WebLogin_BookCompare、stOperatorClick、ChangeLevelAttr 與 WebLogin_StateJson 尚未全量查證。

## 返回值與結果物件

config 由 IsConfig(tag) 決定；levelItem 分 config 92／其他 37，required 讀 LevelSet.AccessLevel[levelItem]，mode 依 FileExists(BookPath()) 分 book／select。這些是所讀表達式，並未讀機台現場值，也未驗證檔案可讀性等於存在。

config 且 iLevel==0 時，這段直接填 level／login 並回 true，沒有走後面的 asked／passed 賦值。因此不能只由一個結果旗標反推所有分支都驗證密碼。REAL_TIME_CCD 關閉時，所讀 body 不詢問，沿用初始 bFlag=true；這不代表所有 caller／權限政策都免驗證。

## 詢問、門檻與權限變化

REAL_TIME_CCD 開啟時先記 asked；沒有答案則填 handled=false／passed=false、結果 level／login，直接回 false。有答案時，取消會傳入空 user／password，而不是跳過後續流程。book 分支呼叫 WebLogin_BookCompare 並依返回碼填警報／原因；select 分支使用 SetupArm 後呼叫 stOperatorClick。兩條 nested 登入實作本次未讀完，不能由 reason 字串證明其成功。

所讀 body 在登入呼叫後以 AccessLevel<iLevel 決定 bFlag。config 且 book 存在的分支之後另將登入狀態及 AccessLevel 歸 Operator／0、呼叫 ChangeLevelAttr，記 loggedOut；所以門檻比較時的權限與最後 out.level 可以不同。最後填 passed=bFlag、level、login、結果物件並返回 bFlag；登入內容、callback 效果、其他權限寫者、V912 對照與完整 consumer 仍待查。

程式中 golden／reason 的歷史行號標籤不作本文件活定位。這是局部靜態讀碼，未呼叫登入 API、送出答案、讀現場密碼、建置或做機台／runtime 驗證。
