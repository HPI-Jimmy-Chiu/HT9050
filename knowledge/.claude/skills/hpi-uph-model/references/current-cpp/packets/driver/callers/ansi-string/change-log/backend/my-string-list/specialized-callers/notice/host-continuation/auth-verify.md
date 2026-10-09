# AuthVerify：處理成功、接受與一次性 pass

[上層](index.md)；[現行 NoticeGate](../host.md)；[原文](source-manifest.json)。

`WebLogin.cpp::W906_NoteAuthVerify` 在noteauth Lock下處理。
它的bool與JSON中的accepted是兩層結果；有效payload被拒絕、取消、
仍需下一階段驗證，甚至reply builder fallback時，最後仍可能return true。
false的早退路徑包含currentId空、JSON不是object、target/action/button欄位不合規，
或blocking選項未提供等；不能將host的處理成功直接當授權完成。

## 綁定與輸入檢查

target.requestId必為string且等於currentId，action.name為string。
pressedButton若存在非null必為string，只接受空、BtnStart、BtnPause；
cancelled若存在必為bool。非取消時userId若存在非null必須string，password必須string。
blocking依SelOfName、KeyOf、Find及offeredBits檢查所選動作；
非blocking只允許ACKNOWLEDGE。authId放進reply，未在此正文拿來比對target。
caller傳currentId的來源與整條request binding還需續查。

## accepted不一定會arm pass

| 正文條件 | accepted／狀態 |
|---|---|
| 找不到notice或未shown | accepted初值true、reason標示，無pass設定 |
| blocking或停機notice且SpecialLocked | panel驗證；取消／錯誤會false；通過但還需unlock/login也false |
| 非blocking且Refusal有reason | 保持初值true，以reason解釋；不等於retire成功 |
| 非blocking且任一start flag為true | 保持true、machine-running reason；此分支不arm pass |
| 其餘Press路徑 | accepted取Press結果；true才設passValid、passK與passPressed，false清passValid |

SpecialPanel通過後用probe檢查還需的密碼階段，可能回accepted=false且specialNext。
這份state=completed是本次驗證回覆狀態，不等於整個多階段授權或ack鏈完成。
一次性pass仍須由後續gate對request、key、pressedButton核對並消耗；
cancelled的完整效果取決於分支與Press，不能僅由flag推定沒有任何狀態改變。

## Reply與清理界線

JsonWriter回schema、dialog-auth、authId、accepted及message等；
userId明確null、verifiedAt/error為null。接受時message=null，拒絕時依special／login／unlock組訊息。
另附login=WebLogin_StateJson()的RawValue；未保存該callee完整正文，
不能只由本函式宣稱整個reply沒有任何敏感資料或已驗完整登入狀態。
builder失敗回固定accepted=false／reply could not be built，最後仍true。
reply指標在正文直接解參照，沒有本地null guard；caller責任未完整追查。

Parsed tree由局部Free解構WipeNode後cJSON_Delete；Cred由WipeCred清user與password。
本段保留這些清理原文，不宣稱const valueJson、其他複本、allocator或整個process記憶體均已清除。
log只列request/action/button/accepted/cancelled/asked等，沒有把valueJson或Cred直接印出；
本輪沒有讀取帳密／權杖，也沒有執行驗證、登入或登出。
