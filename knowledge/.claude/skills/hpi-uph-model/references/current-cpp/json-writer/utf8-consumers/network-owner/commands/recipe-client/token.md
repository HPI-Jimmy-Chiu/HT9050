# token取得、重試、idle及hold

[上層](index.md)；主頁版[acquire](raw/source-01-part-01.md#acquire)／[takeover](raw/source-01-part-01.md#takeover)／[tokenTrack](raw/source-01-part-01.md#tokentrack)／[tokenHeld](raw/source-01-part-01.md#tokenheld)／[armTokenIdle](raw/source-01-part-01.md#armtokenidle)；overlay版[acquire](raw/source-02-part-01.md#acquire)。

## 取得與本地狀態

主頁acquire在haveToken truthy時只armTokenIdle再Promise.resolve；否則送control.acquire，成功後haveToken=true並armTokenIdle。
takeover總送control.takeover，成功後同樣設true與重設idle；不在此body先查其他operator，也不自行比對帳密。
overlay acquire有token直接resolve，沒有armTokenIdle；送出成功才設true。兩份的實際operator是否持有與server驗證，仍由C++和runtime另核。
兩版release保存於全文context：主頁先清本地haveToken／tokenTimer再送control.release；overlay等成功回覆才清haveToken。這兩個object method不追加12選定函式credit。

## tokenTrack的有限重試

control.acquire與control.release直接回p，不增加tokenInflight；其他命令先抓sentWithToken=haveToken並++，fulfilled與rejected都--。
fulfilled時若當下haveToken就armTokenIdle，回原v。
rejected只在!retried、送出時sentWithToken truthy、e.message嚴格等not-operator、name不等motor.access時，清haveToken→acquire→再cmd0並tokenTrack(...,true)。只重試一次。
motor.access排除的原註解談stop排隊後重送jog的風險，全文保留；這不是把所有帶motor字串的命令都排除，也不是本輪已做stop／jog機台測試。
sentWithToken是在wrapper呼叫時抓的本地旗標，不能當server接收當下的權杖事實。其他錯誤或第二次失敗時，若haveToken仍truthy重設idle後throw。
acquire重試成功但第二次ACK拒絕、網路重排等端到端情境未跑；本單元是靜態範圍。

## idle與hold

tokenIdleMs初始30000，cfg.timeoutMs初始15000；兩者責任不同，不與server操作員租期混作同一數值。
armTokenIdle先clear舊timer，再設新timer。callback先tokenTimer=null；!haveToken直接return，tokenInflight>0或tokenHeld()就再arm，否則api.release().catch吞拒絕。
tokenHeld按tokenHolds順序；任一predicate truthy或throw就true，全部falsey才false。setTokenHold只在fn是function時push，沒有本次正文中的移除或去重；tokenIdleMs setter只接ms>0。
長時間hold／inflight可重排idle timer，但不代表server租期被刷新；keepAlive object method另送control.acquire才有server往返，沒有在idle callback中自動呼叫它。
這份client的token是機台操作員協定資料，與Codex／Claude帳號權杖無關；本輪未使用任何機台或其他session權杖。
