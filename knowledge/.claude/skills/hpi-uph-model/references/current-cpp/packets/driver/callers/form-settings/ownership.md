# Aux 初值、widget 所有權與釋放

定位 `TesterComm/Gpib/GpibAux.cpp` 的 TfRS232Main constructor／destructor，以及 `vclcompat/Comm.cpp` 的 TComm constructor；完整定義與 Comm.h 見 [manifest](source-manifest.json)。

## 建立

TfRS232Main initializer 將 CommTester 與七個 widget 指標設 0，bCommConnect=false、iHasCE=0；body 再 `new TComm(0)`。

| 欄位 | TComm constructor 初值 | Aux constructor 再設定 | LoadSetupData 的關係 |
|---|---|---|---|
| CommName | 空 AnsiString | COM13 | INI 缺欄時預設 COM3 |
| BaudRate | 9600 | 9600 | INI／recipe framing |
| ByteSize／Parity／StopBits | _8／None／_1 | _7／Even／_1 | INI／recipe framing |
| ParityCheck | false | false | 本次 LoadSetupData 未覆寫 |
| Outx／Inx XonXoffFlow | false／false | true／true | 本次 LoadSetupData 未覆寫 |
| ReadIntervalTimeout | 0 | 1 | INI 缺欄時預設 70 |

上述 LoadSetupData body 在 [consumer](../tick-consumer/consumer.md)；這是三個階段的不同初值，不是機台實際配置或完整呼叫次序。Comm.h 確認這些是公開資料欄位；TComm constructor 的 Impl 初始狀態另待補查。

Aux 將 OnReceiveData 設成檢查 SerialPoll 後 QueueRx(kRxAuxTester) 的 lambda；註解所稱 reader／DrainRx 執行緒與排程仍須完整 callee 證據。

若 SerialPoll 與七個 widget 全部存在，就使用其指標；否則建立七個 stand-in，並將 this 登入 RsFallbackOwners。這是程式上的 ownership 分流，不是機型差異。

## 釋放

destructor 先 delete CommTester／清指標，再以 `RsFallbackOwners().erase(this)>0` 決定是否 delete 七個 widget，最後都清指標。借用 SerialPoll widget 的分支不在此 delete。

原註解說 StopComm 後不再有 callback；本輪不沿用為已驗證結論。先前 [transport](../tick-consumer/transport.md) 已讀 StopComm 的有限 wait 且未檢查結果；TComm destructor、Impl、callback 全生命期與執行緒退出仍待補查。
