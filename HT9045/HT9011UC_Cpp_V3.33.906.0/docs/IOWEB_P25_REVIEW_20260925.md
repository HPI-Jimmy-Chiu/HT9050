> 筆電端 20260925 15:5x 對機台端 IOWEB-P25（未 commit，只有描述）的設計審查：3 位唯讀審查者（重入／carry／喚醒）＋1 位彙整兼反駁者（workflow wf_ed8b0d6a-dd6）。
> 已整份轉交「機台端與筆電端進度同步」session。之後把 P25 收進 main 時，照本檔的「確定的風險」逐條核對。

# IOWEB-P25 審查意見（筆電端 → 機台端 EastSun）

**總評：** 方向正確，量測有說服力（排隊 5248 ms → 43.6 ms），(a) 修的兩個 603Fh／uptime 問題在 main 上也確認存在，不需要回退。還要補三件事：
- hook 只能跑 DO，這要做成一道真的擋得住的牆；
- socket 執行緒登記 pending ack 的順序；
- waitForPush 的逾時算法。

以下檔:行都相對於 `HT9011UC_Cpp_V3.33.906.0/`（`web/` 開頭的除外），main 指筆電的 9d6f8e1a。

---

## 問題 1：在 Poll 的 yield hook 裡執行輸出，重入安全嗎？

**結論：有條件安全。**
- main 上 DO 兩種命令的執行路徑不會改動監看器的任何容器。成功後只呼叫 ForceDoReread，就地改 `dos[port]`（Pci1203Monitor.cpp:2006-2028）。
- setByte 寫的是命令帶來的值，不讀快取（Pci1203Control.cpp:1684-1693）。
- Pci1203Control.cpp 和 Pci1203Monitor.cpp 兩個檔裡，ShowErrorMessage／ShowMyMessage／MessageBox 的呼叫都是 0 處。

所以只要 hook 裡真的只跑 DO，Poll 的迴圈就不會失效。

### 確定的風險與建議改法

**1. 白名單是唯一的一道牆。**
- 現況：
  - wb_serve.cpp:5147 用 `pci1203.` 前綴，把所有 pci1203.* 命令都交給 `Execute`。
  - `pci1203.card.rescan` 會走 Rescan()，也就是 Close()＋Open()（Pci1203Monitor.cpp:1839-1857）。Close 會清掉 axHandles（:1871），Open 會重新配置 dis／dos（:753-754），OpenAxes_ 會重新配置 axes（:1420）。
  - Poll 在迴圈裡一直握著 `axes[i]`／`dis[i]`／`dos[i]` 的參考和軸 handle（:2175、:2831、:2893）。
  - absEncoderReset 會在 Execute 裡 `Sleep(100)`，最多 60 次（Pci1203Control.cpp:2080-2086）。
- 改法：
  - (a) ServiceOutputs 在 `Pci1203ParseWire` 之後用 `cmd.kind == kCmdDoSetBit || cmd.kind == kCmdDoSetByte` 判斷。不要用名稱前綴，也不要直接呼叫「原樣搬到檔尾」的整段 pci1203.* 分派。
  - (b) 監看器加一個 `inPoll_` 旗標，Poll 進入時設、離開時清。`inPoll_` 為真時，Rescan／Close／Open／OpenAxes_ 一律拒絕；`Pci1203Control::Execute` 只收 DO 兩種 kind。這是第二道牆，不靠呼叫端自律。

**2. 防重入。**
- ServiceOutputs 加 `static bool inService`。
- hook 只在 `inPoll_ && !inService && !inModal` 時才呼叫 ServiceOutputs。

**3. 任何需要讀改寫的地方，改讀「寫入影子」，不要讀監看器快取。**
- main 的 setByte 本身不是讀改寫。但如果 io.btnPanelClick（或其他呼叫端）的做法是「讀 `dos[port].byteData` → 改一個 bit → setByte」，就會蓋掉同一個 byte 的其他 bit，原因有兩個：
  - `dos[]` 是週期回讀，Poll 做到一半時可能還是上一拍的值；
  - ForceDoReread 讀失敗時會靜默保留舊值（Pci1203Monitor.cpp:2002-2005 的註解、:2025）。
- 例子：同一個 byte 的 bit0 和 bit3 連點兩下，第二下用舊的 byte 計算，會把 bit0 改回去。
- 改法：
  - 優先用 setBit 帶目標值。
  - 一定要整個 byte 寫時，改讀寫入影子：每個 port 一格 `lastWritten`，開卡時用一次硬體讀取初始化，之後每次 setBit／setByte 成功都同步更新。

**4. hook 的插入點。**
- 每個 hook 都必須放在該樣本的「讀取＋存回」完全做完之後。main 的 DO 迴圈是先把 `Acm_DaqDoGetByte(Ex)` 讀進區域變數，再存 `s.byteData`／`s.valid`（:2891-2935）；DI 迴圈同樣是這個形狀。
- 同一軸的 5 個狀態讀取和 `Acm_GetLastError(ax)`（:2367）之間也不可以插 hook。這個錯誤碼是 per-handle 的，而且和 Control 共用同一個軸 handle。現在 DO 用的是 device handle，不會污染它；但如果之後把 stop 命令放進 hook，就會污染。

**5. 順手補：Run_ 的 DO 分支加一行 `mon->devHandle_() == 0` 就拒絕。**
- attached 模式 detach 之後，`impl_->opened` 仍然是真（Pci1203Monitor.cpp:2075-2088 只清了 `dev` 和 `cardS.open`），DO 會被送到 handle 0。
- HT9050 應該是 owned 模式，目前走不到這條路。

### 要機台端確認

1. 每個 hook 呼叫點是否都在樣本存回之後？有沒有任何一個落在「5 個狀態讀取」和 `Acm_GetLastError(ax)` 之間？
2. 白名單用 kind 還是用名稱判斷？是不是直接呼叫搬到檔尾的整段 pci1203.* 分派？
3. hook 會不會在 Open／ScanSlaves_／OpenAxes_／603Fh 重試／ForceDoReread 裡觸發？SetYieldHook 是在 Pci1203MonitorEnable（wb_serve.cpp:3935，COLD-1 最長等 90 s）之前還是之後安裝？
4. ExpectCfg_／MarkCfgDue_ 的儲存形式是固定陣列（軸數 × 5 組），還是 vector／map？DO 分支有沒有新加呼叫它們？main 的 DO 分支只呼叫 ForceDoReread（Pci1203Control.cpp:1680、:1692）。
5. io.btnPanelClick（P17）：
   - 開或關是網頁帶目標值，還是伺服端翻轉？如果是翻轉，依據是 `dos[]` 快取、即時讀卡，還是寫入影子？golden 是 `Ptr->Down=!Ptr->Down`（golden iosetview.cpp:1087），而 Down 是按鈕自己的狀態，換到網頁架構就等於「網頁帶目標值」。
   - SwServerON／SwMotorRelay／Breaker 分支會清 `MOT[].HomeFlag` 和 `fAllMotorHome`，這段有沒有照 golden 帶過來？
6. HT9050 上 `card().mode` 是 owned 還是 attached？

---

## 問題 2：carry 規則（輸出可越過 sys.ping／log.event／ui.windows.put／cfg.resync）

**結論：「越過」本身無害。** 四個被跳過的命令在 main 上都不會改到輸出要讀的狀態：
- sys.ping 只回 ack（wb_serve.cpp:4170）；
- cfg.resync 是純讀（:4172-4197）；
- ui.windows.put 只寫視窗總表；
- log.event 只留痕。

control.acquire／release 在 socket 執行緒當場處理，不進佇列（WebBridgeServer.cpp:1386-1407）。權杖檢查也在 push 之前就做完（:1442-1449）。要補的是停止命令，以及 carry 的幾個邊界。

### 確定的風險與建議改法

**1. 停止命令現在比輸出慢。**
- 現況：motor.stop、pci1203.ax.stop、pci1203.ax.emgStop 被歸成「其他命令」。它們要等主 drain 才執行，而且還會擋住排在後面的輸出。
- main 已經把 motor.stop 當成安全命令處理：
  - 免權杖（WebBridgeServer.cpp:1446）；
  - modal 期間照收（wb_serve.cpp:654）；
  - W906_MotorAccessWire 把它鎖死在 action=stop（WebMotorAccessLive.cpp:376-380）。
- 改法：
  - 把這三個 stop 也放進 ServiceOutputs 可以執行的集合，但遵守同一條屏障規則：前面只能是輸出或那四個惰性命令。
  - stop 不可以越過排在它前面的 jog／move／home，否則會變成「先停後動」：操作員按了停止，軸反而開始走。
  - 放進 hook 之前，先確認 `W906_MotorAccessWire(stopOnly=true)` 整條路徑上沒有阻塞呼叫。
  - pause.run 內含 StopAllMotor，比較重，要不要一起加由 EastSun 決定。

**2. g_carry 不可以被持有參考或索引。**
- main 的 `drain(out)` 是附加到 `out` 後面：先 reserve，再 push_back（CommandQueue.cpp:133-155）。
- 如果 g_carry 是 vector，外層照 main 的寫法持有 `const WebCommand& wc = g_carry[i]`（main 現在是 `drained[i]`，wb_serve.cpp:4169），而內層的 ServiceOutputs 或 modal 等待又 drain 進 g_carry，vector 一重新配置，外層的參考就懸空了。
- 改法：
  - g_carry 用 std::deque，每次從前端 pop 一筆到區域變數再執行。
  - 主 drain 也一次只取一筆，其餘留在 g_carry。這樣巢狀的 modal 等待和 ServiceOutputs 都看得到後面的 stop 和輸出。

**3. g_carry 要有上限。**
- CommandQueue 上限 64 筆（CommandQueue.h:105），用意是「佇列滿就立刻回失敗，不累積幾分鐘前的操作意圖」（CommandQueue.h:34-42）。整批倒進一個沒有上限的 g_carry，"command queue full" 就等於失效。
- 改法（擇一）：
  - carry 的頭是屏障命令時，ServiceOutputs 不再從佇列搬命令出來（反正後面的輸出也不能超車），讓積壓留在有上限的佇列裡；
  - 或者把 carry 的上限設成 capacity，超過就回 "command queue full"。

**4. 跳過清單要寫明成員條件：不改狀態。**
- 日後 cfg.* 如果會重載 config.ini 或 IO 表，那個命令就要移出清單，改當屏障。目前 BumpConfigVersion 在產品碼裡沒有呼叫點（MachineDefines.cpp:72、:84）。
- 建議把四種屬性做成一張共用表：「免權杖／modal 期間放行／可被輸出越過／本身是輸出」。三處都查這張表，一律完全比對、不用前綴，再加一條 ctest 釘住。

### 要機台端確認

1. modal 期間，carry 裡的輸出回什麼？建議照 main 回 modal-pending（golden 在 ShowModal 期間本來就點不到 IO 畫面）。不要留到 modal 結束後才執行，那時候已經是舊的操作意圖。
2. modal 結束後，carry 裡剩下的命令是否照原順序交回主 drain？main 的行為是：排在觸發 modal 那筆後面的命令，modal 結束後照常執行。如果 P25 改成被 modal 拿走並回 modal-pending，這是行為改變，請寫明。
3. ServiceOutputs 執行過輸出之後，有沒有讓本圈發布？main 的條件是 `publishNow = pumpBeat || !drained.empty() || ioPolled`（wb_serve.cpp:5352-5356）。如果 `drained` 換成了 g_carry，這個條件要一起改。
4. 43.6 ms 量的是 ack 還是燈號？用哪個命令量的？sys.ping 現在會被跳過，已經不能代表輸出的延遲。
5. io.btnPanelClick 要不要權杖？除非使用者另外裁決，不要把它加進 WebBridgeServer.cpp:1446 的豁免清單。

---

## 問題 3：用 waitForPush 取代 Sleep(2)

**結論：event 本身沒問題。**
- 生產者只有一條 socket 執行緒（WebBridgeServer.cpp:707）。
- 消費者也只有一條執行緒：主迴圈和 modal 等待是同一條執行緒上的巢狀呼叫。
- drain 是整批 swap（CommandQueue.cpp:133-155），多次 SetEvent 合併成一次也不會丟命令。

所以 auto-reset event 不會漏掉喚醒，也不會叫醒錯的人。前提是：逾時有上限、不用 ResetEvent／PulseEvent、等待前先看 carry。

### 確定的風險與建議改法

**1. ack 遺失（main 上本來就有的順序缺陷，event 喚醒會放大）。**
- 現況：socket 執行緒先 QueuePush（WebBridgeServer.cpp:1461-1462），之後才登記 pending（:1477）。CompleteCommand 找不到 ticket 就靜默 return（:1602）。
- event 喚醒之後，DO 只要兩個 IOCTL 就做完。主執行緒有機會在 pending 登記之前就呼叫 CompleteCommand，結果是線圈已經動了，瀏覽器 15 s 後卻報 "no ack within"（web/page/ht9045_recipe_client.js:195）。
- 如果 io.btnPanelClick 是切換語意，操作員看到失敗再按一次，就把輸出切回去了。機率低，但後果是反向動作，而且修法很小。
- 改法（二選一）：
  - (a) 先登記 `pending_[ticket]` 再 QueuePush；push 失敗就 erase，再回 "command queue full"。
  - (b) tryPush 不呼叫 SetEvent，改成在 :1477 登記完之後呼叫一個新的 `queue->notify()`。
- 再補一支 ctest：drainer 執行緒一拿到命令就 CompleteCommand，連送 1 萬筆，斷言收到的 ack 數等於 1 萬。

**2. 逾時算法照抄 main。**
- main 的寫法在 wb_serve.cpp:4055-4059：取 nextPump 和 nextIo 中較近的那個，用有號差值 `(long)(next - now) > 0 ? next - now : 1`，再夾在 [1, 50] ms。
- 不要直接傳 `next - now`：截止時間剛過 1 ms 時，DWORD 差值是 0xFFFFFFFF，正好等於 INFINITE。改前 Poll 最長 5221 ms，截止時間被超過是常態。
- 被喚醒只代表「可能有命令」。PumpTick 仍然只看截止時間（B13 的 500 ms，wb_serve.cpp:2588）。
- 醒來後用一次 drain 把佇列倒空（一次 swap），不要一個 event 只處理一筆。

**3. modal 等待也改用同一個 event。**
- 把 wb_serve.cpp:532 的 `Sleep(100)` 換成 `waitForPush(100)`，不要用 INFINITE。這樣 modal 期間的 motor.stop 和警報回答可以少等約 100 ms。
- 等待前如果 carry 非空，把逾時設成 0。

**4. 檔頭契約和測試要一起改。**
- CommandQueue.h:20-32 寫的是 "no condition variable"，而且只列 drain 是 UI THREAD ONLY。waitForPush 要標明：只有 UI 執行緒可以呼叫、一定有上限、醒來只是提示。
- test_wb_state.cpp 補四個情境：
  - 先 push 再 wait → 立即返回；
  - 連 push N 筆 → 一次 wait、一次 drain 拿到 N 筆；
  - 沒有 push → 大約在逾時後回 false；
  - 佇列滿時 tryPush 立即回 false，之後一次 wait 就能取回全部 64 筆。

### 要機台端確認

1. waitForPush 的參數是不是沿用上面的算法？
2. event 的生命週期：
   - CreateEvent 是不是在 CommandQueue 建構子裡做？（cmdQueue 在 wb_serve.cpp:3847 建構，早於 :3864 的 server.Start。）
   - 是不是 auto-reset、每次 push 成功都 SetEvent、解構子有 CloseHandle？
   - 全程有沒有 ResetEvent 或 PulseEvent？有沒有地方用 `WaitForSingleObject(ev, 0)` 探詢？這會把訊號吃掉。
3. CreateEvent 失敗（handle 是 NULL）時，有沒有退回 Sleep？沒有的話 WaitForSingleObject 會立刻回 WAIT_FAILED，主迴圈變成 100% CPU 空轉。
4. 等待之前有沒有先看 `!g_carry.empty()`？
5. 除了主迴圈和 modal 等待，還有沒有別的地方 Wait 這個 event？建議在 waitForPush、drain、ServiceOutputs 開頭都加斷言：`GetCurrentThreadId()` 等於主執行緒。

---

## 其他注意事項

**1. W5-b 教導頁阻塞框（使用者 20260925 裁決，RULINGS_20260925 第 2 條；筆電 t6-mainproc 在製，還沒進 main）**
- 路徑：真機組態下按教導頁運動鈕 → IsCanQuickJogMove → CheckShuttleCanMove → `ShowErrorMessage("WAR16435", K_RETRY)`（forms/fTeach.cpp:363，main 已有這行）。
- 框開著的期間整條主迴圈停住，P25 的 Poll 和 ServiceOutputs 也不跑。這段時間輸出**不是** 0 延遲，會回 modal-pending。
- 這是 golden 的 ShowModal 行為，也是使用者的裁決，不要為了輸出 0 延遲在 modal 裡開洞。請在 P25 的說明裡寫明這個例外。
- W5-b 在 wb_serve.cpp 只加兩行（start.run 分派處，以及 W906_AlarmAnswerStartLikeGolden：手動教導中拒絕 START），和 P25 的主迴圈不重疊。但 motor.access 必須留在屏障那一邊。

**2. ForwardShowErrorMessage（wb_serve.cpp:435-657）是唯一的巢狀 drain 點**
- 整合時要保住 main 在機台 base 之後加的兩件事：
  - modal 期間照收 motor.stop（:654）；
  - 回答 START 會走 W906_AlarmAnswerStartLikeGolden → StartFromWeb（:622）。這段是在 modal 的出口裡跑的，StartFromWeb 期間主迴圈和 hook 都不跑。
- main 的 modal 對 sys.ping／log.event／ui.windows.put 一律回 modal-pending（:653-655）。網頁收到 ui.windows.put 被拒，就把 winSupported 設成 false，這條連線之後的視窗變化都不再送（web/page/ht9045_recipe_client.js:798-805、web/background.html:817）。
- 這不是 P25 造成的。但既然 P25 在改這段，建議順手讓這三個命令在 modal 期間直接執行（比照 cfg.resync，:626-652）。

**3. 34243800 的 pollCount／pollMs 語意：main 現在有兩個消費者**
- 兩個消費者：
  - (a) `/api/struct/io/runtime` 的 monitor 區段（JsonBridge/ChanIoPoints.cpp:376-392）：兩次讀到同一個 pollCount，代表那段時間沒有輪詢。
  - (b) W4 motor.access 的 W4B-5（WebMotorAccess.cpp:135-143）：每一軸下完運動命令後，要等 pollCount 前進才准下一個運動命令，否則 READY／位置還是命令前的樣本。
- P25 必須保證：
  - `++pollCount` 只在「狀態類都讀完」的 Poll 結尾做一次（main 在 :2941）；
  - 不在 hook 裡做，也不在提前返回的路徑做。main 在 attached 模式的 detach 路徑（:2086）已經會在沒讀任何東西的情況下 +1；owned 模式不走這條。
- pollMs 扣掉 yield 時間之後，語意變成「Poll 本身花的時間」，不再等於主迴圈停住的時間。runtime 的 `monitor.pollMs` 和 tag `pci1203.pollMs`（WebBridgeTags.cpp:1488）都會受影響。建議另外加一個 yieldMs 或 wallMs 欄位。
- api cache 改用 dirty 旗標之後，每次 Poll 都要把 runtime 標成 dirty（或者 pollCount 改成即時讀）。否則 DI/DO 沒變化時，cache 裡的 pollCount 不動，會被誤判成「輪詢停了」。

**4. api cache 是好事，但要確認是在主執行緒上建的**
- main 的 `/api` 路由在 socket 執行緒上跑（WebBridgeServer.h:199-203、wb_serve.cpp:3861）。IoRuntimeJson 直接在那條執行緒讀 `mon->di()`／`do_()` 和 `card()`（ChanIoPoints.cpp:348-392）。這在 main 上就是跨執行緒存取；rescan 時 dis／dos.assign 重新配置，還可能讀到已釋放的記憶體。
- 如果 P25 把 cache 建在主迴圈，ApiRoute 只在鎖內複製建好的字串，就等於順便修掉 main 的這個問題。
- 請確認 ApiRoute 裡沒有「發現 dirty 就當場重建」的路徑。

**5. (a) 的兩個修正在 main 上確認是真的 bug，整合時以機台版為準**
- 603Fh 重試和速度讀取的「每次 Poll 決定一次」寫在 `if (i == 0)` 裡（Pci1203Monitor.cpp:2382、:2502、:2716）。axis 0 沒有 handle 時，迴圈開頭就 continue（:2173），這個判斷永遠不會執行。
- `static DWORD s_nextSpeedRead = 0`（:2375）配上 `(long)(now - 0)`，在開機後 24.8～49.7 天之間是負的。

**6. 「一致之後就不再讀」的後果，請寫進註解**
- Pci1203AxisIniTick（wb_serve.cpp:4115，每圈都跑，比對 limitVal）以後只看得到開卡時和寫入後讀到的值。如果別的程式（例如廠商 Utility）改了設定，或卡片端重置但沒有 rescan，設定飄掉不會被發現，只能靠 rescan。
- 請確認：
  - limit 組「3 次不一致就停讀」或進入 30 s 重試窗口時，AxisIniTick 不會一直等下去；
  - absEncoderReset（Fn008）和 resetError 之後，有沒有把 drive 組標成待讀。

**7. 看門狗標記**
- ServiceOutputs 在 hook 裡執行時，用 `"1203 Poll > dispatch: <cmd>"` 這類階層字串標記，離開前還原成 `"1203 Poll"`（wb_serve.cpp:4147-4149）。否則 Poll 後半段卡住時，會被報成那筆輸出命令。
- 快照只在 Poll 結束之後發布，hook 裡不要呼叫 PublishHandlerTags。

**8. pci1203.do.* 直接寫卡，不經 TLaneIO 的安全門互鎖和 OutPortData 影子**
- 這是已知的問題：使用者已裁決測試期不處理安全門互鎖（第 17 條）；OutPortData 越界在整合 P17 時修（第 18 條）。
- 不要為了這條改走 `MyLaneIO.IOBitOn`。main 的 ht9045_io 沒有編進 HAVE_PCI1203，`TPci1203Backend::WriteBit` 是 `return 0` 的樁（IOBackend.cpp:171），而 IOBitOn 會把 0 當成功，結果是靜默地什麼都沒寫。

**9. 整合進 main 時的衝突範圍**（機台 base＝包內的 base_8484bdb4；main＝9d6f8e1a）
- **不會衝突，以機台版為準：** WebBridge/CommandQueue.*、EtherCAT/Pci1203Monitor.*、EtherCAT/Pci1203Control.*。main 自 base 以來沒動過這幾個檔。
- **tools/wb_serve.cpp**（main 自 base 以來 +437／−31）會撞到四處：
  1. 主迴圈 PumpTick 那一行。main 在 :4091 加了 W906_MotorAccessTick，正好是 P25「PumpTick 後呼叫 ServiceOutputs」的插入點。
  2. modal 等待的 else 分支（:654 的 motor.stop）。
  3. 分派鏈新增的 motor.access／motor.stop（:4982）。carry 分類時，motor.access 要當屏障，motor.stop 照第 2 題處理。
  4. StructRoute（:2453，+44 行）和 ApiRoute（:2500，+17 行）。這是 Steven S12-C 的 `/api/editlist` 和 `/api/form`，在 FormLock 下讀。P25 的 api cache 要把它們納入，或明確排除。
  - pci1203.* 分派本體（:5147-5205）自 base 以來沒動，「原樣搬到檔尾」只要是純搬移就能乾淨合併。
- **WebBridge/WebBridgeServer.cpp：** main 改了 :1446（豁免清單加了 modal.answer／dialog.response／motor.stop）。如果照第 3 題修 ack 順序，會改到 :1461-1477，就在這一行旁邊。
- **JsonBridge/ChanIoPoints.cpp、ChanIo.h：** 34243800 在 base 之後，機台版沒有 monitor 區段。api cache 要接上 IoRuntimeJsonFrom 的新多載。
- 請用 Remote Control 文字傳回 P25 相對 base 的完整 diff。筆電端收進 main 時，會補上第 1、3 題的 ctest。

---

## 筆電端核對紀錄（不必轉交）

**降級或推翻的審查意見：**

- **問題 1「pci1203.do.* 繞過 TLaneIO 互鎖和影子」**
  - 引用都正確，但這是已裁決的事項（RULINGS_20260925 第 17、18 條），不是新風險。
  - 審查者建議的選項 (a)「改走 MyLaneIO.IOBitOn」在 main 目前的建置下不能用：只有 wb_serve 帶 HAVE_PCI1203（CMakeLists.txt:3340），三個生產函式庫刻意不武裝（:3034-3046），`TPci1203Backend::WriteBit` 是樁（IOBackend.cpp:171）→ 靜默不寫。已改寫成第 8 條。
- **問題 2「IO 畫面恆當作沒開，自動邏輯下一拍蓋掉手動輸出」**
  - 引用正確（csystem.cpp:29921-29935、:30028-30031、:24590-24596）。
  - 但 TLaneIO 對 1203 的寫入目前到不了卡（同一個樁），HT9050 上 1203 輸出今天不會被蓋掉。要等生產函式庫武裝之後才成立，所以不列入回覆。
- **問題 2「modal-pending → 總表 15 s stale → 改變 W906_FShow」**
  - 「winSupported 被設成 false」這一段已確認。
  - 但 stale 不是它造成的：background.html 只在變化時（:811）或重連時（:1030-1034）送總表，沒有心跳；recvMs 也只在 put 時更新（WebWindowRegistry.cpp:96）。所以 main 上每個表單都會在最後一次變化 15 s 後變 stale，跟 modal-pending 無關。modal-pending 真正多造成的損害是「之後的變化整條連線都不再送」。已照這個縮小。
  - main 上「沒有心跳 → 15 s 後表單視為開著」本身是另一個待查項目。
- **問題 1「detach 後 DO 打到 handle 0」**
  - 只有 attached 模式才成立。依 main 的建置，生產函式庫不開卡，uiDevhand 恆為 0，HT9050 是 owned 模式。降為順手補。
- **問題 1「ExpectCfg_ 用動態容器會讓迭代器失效」**
  - 只有在 hook 裡的命令會呼叫 ExpectCfg_ 時才成立；main 的 DO 分支只呼叫 ForceDoReread（Pci1203Control.cpp:1680、:1692）。降為確認題。
- **問題 2／問題 3「modal 可能在 ServiceOutputs 裡發生」**
  - 對 pci1203.do.* 不成立：兩個 1203 檔裡的阻塞呼叫都是 0 處。縮小到只問 io.btnPanelClick。
- **問題 3「Sleep(2) 實際一片約 15.6 ms」**
  - 是推論，main 自己的註解寫「沒有量過」（wb_serve.cpp:4045-4046）。沒有寫進回覆。
- **問題 3「waitForPush 沒有逾時，主迴圈就會停」**
  - 這是有條件的說法。機台端已經實測在跑，推定逾時有上限；留作確認題，重點改成 DWORD 逾期時的算法。
- **路徑出入**
  - 審查者引用了兩個不同路徑的 recipe_client：web/page/ht9045_recipe_client.js:802 和 tools/websync/ht9045_recipe_client.js:678。兩份都存在，實際送出去的是 web/page 那份。

**三位審查者都漏掉、這次補上的：**
- W4B-5 把 pollCount 當成運動命令的新鮮度閘（WebMotorAccess.cpp:135-143）。
- `drain()` 是附加寫入，vector 會重新配置，外層參考會懸空（CommandQueue.cpp:133-155）。
- main 的 IoRuntimeJson 在 socket 執行緒上讀監看器（ChanIoPoints.cpp:348-392）。
- P25 (a) 的兩個 bug 在 main 上確實存在（Pci1203Monitor.cpp:2173、:2375、:2382）。
- AxisIniTick 在「一致之後不再讀」之後的盲點。
- 以機台 base 8484bdb4 量出來的具體衝突範圍。

**完成狀態：** 三份審查的關鍵引用都已逐條回 main（9d6f8e1a）核對。這次是唯讀，沒有改任何檔。