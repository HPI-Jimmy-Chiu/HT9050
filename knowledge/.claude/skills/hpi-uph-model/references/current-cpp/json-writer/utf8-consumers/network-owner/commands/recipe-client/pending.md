# pending id、timeout及ACK shaping

[上層](index.md)；主頁版[cmd](raw/source-01-part-01.md#cmd)／[cmd0](raw/source-01-part-01.md#cmd0)／[unwrapAck](raw/source-01-part-01.md#unwrapack)，overlay版[cmd](raw/source-02-part-01.md#cmd)。

## 訊框及Promise生命期

兩份底層cmd都等connect().then(s)，在new Promise executor內nextId++、msg={type:'cmd',id:id,cmd:name}，再把extra的所有own enumerable keys複製到msg。
本body沒有保護type／id／cmd不被extra蓋掉；本地pending仍以原id建立。因此本輪只記錄rawCmd／caller的責任邊界，不推成伺服器收到的id必與pending相同。
pending[id]先保存resolve／reject；timer到期若pending還在，delete再reject no ack within cfg.timeoutMs。
隨後pending[id]換成wrapped resolve／reject，兩者先clearTimeout，再呼叫原callback；ACK與close走wrapped callback時會清timer。
最後才JSON.stringify(msg)並s.send。executor中的同步throw會令Promise reject，但這兩份body都沒有同步catch去delete pending或clear timer。
因此序列化／send同步失敗與正常ACK／close不同：caller Promise已reject，pending仍可能留到timeout／close／對應ACK。沒有執行JS量測，不宣稱記憶體洩漏或機台故障。
timer在ACK刪除pending前後的具體event-loop次序、本次只按正文讀；不稱端到端race驗證。

## 主頁cmd wrapper

cmd(name,extra)呼叫tokenTrack(name,extra,cmd0(name,extra),false)；cmd0的Promise先建出，tokenTrack再抓haveToken與計數。
所有指令都經tokenTrack呼叫點，但control.acquire／control.release在tokenTrack內直接回原p；本單元不把wrapper說成所有命令都先acquire。
overlay的cmd就是底層送出，沒有主頁版tokenTrack包裝。

## unwrapAck

m存在且m.value是字串才try JSON.parse；parse後j truthy且typeof object就回j，包含array，沒有額外排除array。
空物件可回，null／字串／數字／boolean parse結果回原m；parse錯也回原m。這個函式不檢驗ok、schema、欄位或server是否執行命令。
各API是否then(unwrapAck)、保留整ACK或另做auth shaping要看caller，不把這一支的規則套到所有回應。
