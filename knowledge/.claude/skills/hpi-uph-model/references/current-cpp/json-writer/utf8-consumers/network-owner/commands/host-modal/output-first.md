# output-first前綴、barrier與yield

[上層](index.md)；[W906_ServiceOutputs](raw/function-04-part-01.md)／[分類](raw/function-08-part-01.md)／[inert](raw/function-09-part-01.md)／[yield hook](raw/function-10-part-01.md)完整原文。

## 編譯期分類

WB_PUMP_1203_CONTROL啟用時，IsOutputCmd僅收io.btnPanelClick、pci1203.do.setBit、pci1203.do.setByte、pci1203.ax.stop、pci1203.ax.emgStop、motor.stop。
未啟用時一律false；card.rescan、jog／move／home不在這份output分類，不能因STOP安全意圖推為任意提前跳過它們。
IsInertCmd完全比對sys.ping、log.event、ui.windows.put、cfg.resync；inert留在原carry位置，這裡不早跑它們。

## ServiceOutputs順序

busy或queue/server空pointer先返回；!runnable且queue size0也返回。busy為重入抑制static bool，不是thread mutex或例外自動還原保證。
先掃carry是否任何非output且非inert的barrier。若有就不drainfresh，讓後來命令保留在queue；否則drainfresh後按序append到carry。
清runnable後，迴圈只跨過開頭inert尋第一個output；遇第一個非inert非output就停，即使後面有STOP也不越過此barrier。
每次output先copy wc／erase；Q44CmdRefused以cmd及字串value檢查，拒絕就false ACK／continue，尚不設ran或phase timing。
第一個可執行output且phase>0先IoTiming(phase+1)，ran=true並WdMark2；macro下IOclick走DispatchIoClick，motor.stop走MotorAccessWire(stopOnly=true)並CompleteCommand，其餘Dispatch1203Ex(...,true)。
dispatch可能觸發modal／改carry，因此每次重新掃頭。ran才設outputsServed、IoTiming(phase)與WdMark，最後busy=false。
這些breadcrumb／flags記程式流程，未證明實體IO已到位、driver成功或browser已收到ACK；dispatch callee各自成功責任未在此完整驗證。

## yield與限制

W906_OutputYieldHook只呼叫ServiceOutputs(3)。原安裝comment提到monitor啟用時SetYieldHook；本次只保留該comment與wrapper，完整monitor啟用／解除caller尚待接續。
原P25／P25c／D012時間、STOP不越jog的理由及capacity註記照存；本輪沒有1203 Poll、motor、機台或時序測試。
g_carry相關全域rationale見[evidence](evidence.md)，不由static資料推跨thread同步、所有部署macro一致或main全dispatch覆蓋。
