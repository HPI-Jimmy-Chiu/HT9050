# 登入結果、I37 與密碼本局部查證

接續[callee 答案消耗／wrapper](../customer-reauth-callee/index.md)，補七個指定函式 body 與兩版 CC_PTI 定義；來源、blob、編譯條件及日期見[manifest](source-manifest.json)。同題整合 HT9050 與其他 Handler，但作用中客戶碼、機型、版本、權限及 runtime 仍各別查證。

| 路由 | 已讀範圍 | 尚未閉合 |
| --- | --- | --- |
| [I37／MBox 與 PTI](mbox.md) | W906_DoPasswordMBox、V912 DoPassword_MBox、CC_PTI 定義 | 全部 caller、現場建置／機型、UI 密碼輸入及最終存檔效果 |
| [密碼本比較](book.md) | WebLogin_BookCompare 的來源選擇、局部比較及返回／權限分支 | binary override、解碼／字串解析／讀檔 callee、所有狀態寫者與例外 |
| [StateJson 與局部 helper](state.md) | StateJson 字段、IsConfig／BookOverride／Typed | 字串來源／轉碼、所有 caller、回覆組裝及完整驗證流程 |

這是局部靜態讀碼；沒有讀現場密碼、登入／存檔請求、執行程式、build、機台或 runtime。新增一列客戶／建置差異，原候選與人工列保留；歷史 golden 字串不作當前全量對等證據。

- [接續BookPath namespace／格式與轉碼helper（局部）](../customer-login-helpers/index.md)
