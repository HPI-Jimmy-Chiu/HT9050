# 警報、警告與恢復（SGDXS）

> 來源：SIEP C710812 02H 第 16 章（p.697-756）與相關章節；「p.」＝02H 的 PDF 頁碼。
> 警報表的「停止」「Reset」兩欄是**用程式從 p.702-707 的表格文字直接抽出來的**，沒有手抄；「意義」欄是中文摘要，英文原文以手冊為準。
> Steven 的規則原文在 `D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md`（Q88／Q89：20261003 22:5x；Q96：20261004 07:4x；Q98：20261004 17:0x）。

## 1. 怎麼知道有警報／警告

| 管道 | 警報（p.702） | 警告（p.741） |
|---|---|---|
| 驅動器面板 | 逐字顯示，例「A.020」 | 例「A.910」 |
| 6041h | **bit3 Fault＝1** | **bit7 Warning＝1** |
| 603Fh | 存目前的警報碼 | 存目前的警告碼 |
| Emergency message | 通知主站（通訊不穩時可能送不到） | 同左 |
| CN1 ALM 輸出（CN1-3／-4） | 有警報時 OFF（開路）；HWBB 狀態不輸出 ALM（p.240、p.578） | /WARN 要另外分配腳位（p.241） |

面板沒顯示警報碼卻異常＝驅動器系統錯誤，要換驅動器（p.702）。

## 2. 603Fh 與 A.xxx 的對應

- 手冊的警報表寫成「020h」「A12h」，面板與內文寫成「A.020」「A.A12」「A.Eb1」（p.702、p.576）。本檔兩種都列。
- 603Fh（UINT、RO、可 PDO）＝「最後一次發生的警報／警告碼」（p.665）；emergency message 的 byte4-5 也放同一個碼（p.598）。
- **數值編碼手冊未逐字寫出**（例：A.A12 讀到的是不是 0x0A12）。表上的寫法「A12h」暗示是十六進位直讀，但要上機讀一次確認。
- 警告也進 603Fh（p.741）⇒ 只看 603Fh≠0 分不出警報或警告，**要同時看 6041h bit3／bit7**。
- Fault Reset 之後 603Fh 會不會歸 0：手冊未載明。
- 1001h Error Register bit0＝generic error（p.634）。
- 雙軸 SGDXW 的 B 軸錯誤碼在 683Fh（樹註解引用 SIEP C710812 05，本來源未涵蓋）。
- HT9050 現況：監看器有讀 603Fh，但 1203 的錯誤框只給得出 ALM／LMT（WAR24MMM8「Motor Alarm」），驅動器自己的碼沒帶進框——見
  [ht9050-1203-runtime-traps.md](../../ht9045-motor-control/references/ht9050-1203-runtime-traps.md) §3。

## 3. 停止方式（Gr.1／Gr.2）與煞車

| 群組 | 怎麼停 | 預設 | 頁 |
|---|---|---|---|
| Gr.1 | 照 Pn001＝n.□□□X | **動態煞車（DB）停、停後 DB**（0） | p.210、p.760 |
| Gr.2 | 照 Pn00B＝n.□□X□（＋Pn00A、Pn001） | **零速停止**（Pn00B n.□□0□），停後照 Pn001 | p.210-211、p.765 |
| 扭力控制中 | 一律用 Gr.1 的方法 | – | p.210 |

- 停止方式定義：DB 停、自由滑行、零速停止（速度命令設 0）、減速停止（Pn406 緊急停止扭力）；停後狀態：DB 作用、滑行、零位鎖定（p.209）。
- Pn00B=n.□□1□ 可讓 Gr.2 跟 Gr.1 用同一種停法（多軸連動時避免停法不同撞機）（p.210）。
- **警報一發生，馬達立刻斷電，不管 Pn506 的延遲**，垂直軸在煞車咬合前可能因重力移動（p.207）；/BK 在警報時 OFF（煞車作用）（p.206）。
- 運轉中發生警報：/BK 在「速度低於 Pn507」或「斷電後經過 Pn508」任一條件成立時 OFF（p.207-208）。
- 主迴路或控制電源在伺服 OFF 前就斷：SGDXS-R70A～-200A（含 HT9050 的 2R8A、5R5A、200A）DB 停（p.209）。
- 605Eh Fault Reaction 只有 0＝伺服 OFF（p.672）。

## 4. 警報一覽（p.702-707）

「Reset」欄＝手冊「Alarm Reset Possibility」：**Yes＝排除原因後可用 alarm reset 清掉；No＝不能清**（p.702）。
「HT9050」欄：n/a＝依 HT9050 的馬達／驅動器型號推斷用不到的功能（線性馬達、Σ-LINK II、外部編碼器、選購模組；型號見 hardware 檔），推斷不是手冊說的。
面板顯示的大小寫（A.Eb1、A.d04、A.CC0）照手冊寫法。

| 面板顯示 | 列表碼 | Alarm Name（手冊原文） | 意義 | 停止 | **Reset** | HT9050 | 頁 |
|---|---|---|---|:-:|:-:|---|---|
| A.020 | 020h | Parameter Checksum Error | 驅動器內的參數資料有錯（參數 checksum） | Gr.1 | No |  | p.702 |
| A.021 | 021h | Parameter Format Error | 驅動器內的參數資料有錯（格式） | Gr.1 | No |  | p.702 |
| A.022 | 022h | System Checksum Error | 驅動器內的參數資料有錯（系統 checksum） | Gr.1 | No |  | p.702 |
| A.024 | 024h | System Alarm | 驅動器內部程式錯誤 | Gr.1 | No |  | p.703 |
| A.025 | 025h | System Alarm | 驅動器內部程式錯誤 | Gr.1 | No |  | p.703 |
| A.030 | 030h | Main Circuit Detector Error | 主迴路偵測資料錯誤 | Gr.1 | Yes |  | p.703 |
| A.040 | 040h | Parameter Setting Error | 參數設定超出範圍（含 2701h 比值超範圍，p.709） | Gr.1 | No |  | p.703 |
| A.041 | 041h | Encoder Output Pulse Setting Error | Pn212／Pn281 編碼器輸出脈波設定超範圍或不符條件 | Gr.1 | No |  | p.703 |
| A.042 | 042h | Parameter Combination Error | 多個參數的組合超出範圍 | Gr.1 | No |  | p.703 |
| A.044 | 044h | Semi-Closed/Fully-Closed Loop Control Parameter Setting Error | 半閉／全閉迴路相關參數不一致 | Gr.1 | No | n/a 全閉／外部編碼器 | p.703 |
| A.046 | 046h | SigmaLINK II Command/ Response Parameter Setting Error | Σ-LINK II 回應／命令資料設定錯誤 | Gr.1 | No | n/a Σ-LINK II | p.703 |
| A.047 | 047h | Encoder with Functional Safety - Safety Mode Setting Error | 接上了具功能安全的編碼器 | Gr.1 | Yes | n/a 選購模組 | p.703 |
| A.050 | 050h | Combination Error | 驅動器與馬達容量不匹配 | Gr.1 | Yes |  | p.703 |
| A.051 | 051h | Unsupported Device Alarm | 接上不支援的裝置 | Gr.1 | No |  | p.703 |
| A.070 | 070h | Motor Type Change Detected | 接上的馬達種類與上次不同 | Gr.1 | No |  | p.703 |
| A.080 | 080h | Linear Encoder Pitch Setting Error | Pn282 線性尺規節距仍是預設值 | Gr.1 | No | n/a 線性 | p.703 |
| A.100 | 100h | Overcurrent Detected | 功率電晶體過電流或散熱片過熱 | Gr.1 | No |  | p.703 |
| A.101 | 101h | Motor Overcurrent Detected | 馬達電流超過容許值 | Gr.1 | No |  | p.703 |
| A.102 | 102h | Motor Overcurrent Detected 2 | 馬達電流超過容許值（2） | Gr.1 | No |  | p.703 |
| A.300 | 300h | Regeneration Error | 回生相關錯誤 | Gr.1 | Yes |  | p.703 |
| A.320 | 320h | Regenerative Overload | 回生過載 | Gr.2 | Yes |  | p.703 |
| A.330 | 330h | Main Circuit Power Supply Wiring Error | 主迴路 AC／DC 輸入設定或配線錯誤 | Gr.1 | Yes |  | p.703 |
| A.400 | 400h | Overvoltage | 主迴路 DC 電壓過高 | Gr.1 | Yes |  | p.703 |
| A.410 | 410h | Undervoltage | 主迴路 DC 電壓過低 | Gr.2 | Yes |  | p.703 |
| A.450 | 450h | Main-Circuit Capacitor Overvoltage | 主迴路電容劣化或異常 | Gr.1 | No |  | p.703 |
| A.510 | 510h | Overspeed | 馬達超過最高轉速 | Gr.1 | Yes |  | p.703 |
| A.511 | 511h | Encoder Output Pulse Overspeed | 編碼器分周輸出超速（Pn212／Pn281） | Gr.1 | Yes |  | p.703 |
| A.520 | 520h | Vibration Alarm | 馬達速度出現異常振盪 | Gr.1 | Yes |  | p.703 |
| A.521 | 521h | Autotuning Alarm | 免調整功能自動調整時偵測到振動 | Gr.1 | Yes |  | p.703 |
| A.550 | 550h | Maximum Motor Speed Setting Error | Pn385 最高轉速設定大於馬達最高轉速 | Gr.1 | Yes |  | p.703 |
| A.710 | 710h | Instantaneous Overload | 大幅超過額定扭力運轉數秒～數十秒（瞬間過載） | Gr.2 | Yes |  | p.704 |
| A.720 | 720h | Continuous Overload | 持續超過額定扭力運轉（連續過載） | Gr.1 | Yes |  | p.704 |
| A.730 | 730h | Dynamic Brake Overload | 動態煞車作用時動能超過 DB 電阻容量 | Gr.1 | Yes |  | p.704 |
| A.731 | 731h | Dynamic Brake Overload | 動態煞車作用時動能超過 DB 電阻容量 | Gr.1 | Yes |  | p.704 |
| A.740 | 740h | Inrush Current Limiting Resistor Overload | 主迴路電源頻繁 ON／OFF（突波限流電阻過載） | Gr.1 | Yes |  | p.704 |
| A.7A1 | 7A1h | Internal Temperature Error 1 (Control Board Temperature Error) | 控制板周圍溫度異常 | Gr.2 | Yes |  | p.704 |
| A.7A2 | 7A2h | Internal Temperature Error 2 (Power Board Temperature Error) | 功率板周圍溫度異常 | Gr.2 | Yes |  | p.704 |
| A.7A3 | 7A3h | Internal Temperature Sensor Error | 溫度感測電路錯誤 | Gr.2 | No |  | p.704 |
| A.7Ab | 7Abh | SERVOPACK Built-in Fan Stopped | 驅動器內建風扇停止 | Gr.1 | Yes |  | p.704 |
| A.810 | 810h | Encoder Backup Alarm | 編碼器電源全部中斷（或內建電池電量 0），位置資料遺失 | Gr.1 | No |  | p.704 |
| A.820 | 820h | Encoder Checksum Alarm | 編碼器記憶體 checksum 錯誤 | Gr.1 | No |  | p.704 |
| A.830 | 830h | Encoder Battery Alarm | 控制電源 ON 後電池電壓／內建電池電量低於規定 | Gr.1 | Yes |  | p.704 |
| A.840 | 840h | Encoder Data Alarm | 編碼器內部資料錯誤 | Gr.1 | No |  | p.704 |
| A.850 | 850h | Encoder Overspeed | 上電時編碼器正在高速轉動 | Gr.1 | No |  | p.704 |
| A.860 | 860h | Encoder Overheated | 編碼器內部溫度過高 | Gr.1 | No |  | p.704 |
| A.861 | 861h | Motor Overheated | 馬達內部溫度過高 | Gr.1 | No |  | p.704 |
| A.862 | 862h | Overheat Alarm | 過熱保護輸入 TH 超過 Pn61B | Gr.1 | Yes |  | p.704 |
| A.890 | 890h | Encoder Scale Error | 線性尺規故障 | Gr.1 | No | n/a 線性 | p.704 |
| A.891 | 891h | Encoder Module Error | 線性尺規錯誤 | Gr.1 | No | n/a 線性 | p.704 |
| A.8A0 | 8A0h | External Encoder Error | 外部編碼器錯誤 | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.8A1 | 8A1h | External Encoder Module Error | 串列轉換單元錯誤 | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.8A2 | 8A2h | External Incremental Encoder Sensor Error | 外部編碼器錯誤（增量感測器） | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.8A3 | 8A3h | External Absolute Encoder Position Error | 外部絕對編碼器位置資料錯誤 | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.8A5 | 8A5h | External Encoder Overspeed | 外部編碼器超速 | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.8A6 | 8A6h | External Encoder Overheated | 外部編碼器過熱 | Gr.1 | Yes | n/a 外部編碼器 | p.704 |
| A.A10 | A10h | EtherCAT DC Synchronization Error | 驅動器無法與 Sync0 同步 | Gr.2 | Yes |  | p.704 |
| A.A11 | A11h | EtherCAT State Error | 驅動器在 Operation Enabled 時 EtherCAT AL 沒有進 Operational | Gr.2 | Yes |  | p.705 |
| A.A12 | A12h | EtherCAT Output Data Synchronization Error | PDO 接收事件無法與 Sync0 同步（製程資料通訊失敗） | Gr.2 | Yes |  | p.705 |
| A.A20 | A20h | Parameter Setting Error | 參數設定超出範圍（含 2702h～2704h 比值、607Bh、607Ch，p.722） | Gr.1 | No |  | p.705 |
| A.A41 | A41h | Communication Device Initialization Error | ESC（EtherCAT 晶片）初始化錯誤 | Gr.1 | No |  | p.705 |
| A.A47 | A47h | Loading Servo Information Error | 讀取驅動器資訊失敗 | Gr.1 | No |  | p.705 |
| A.b33 | b33h | Current Detection Error 3 | 電流偵測電路錯誤 | Gr.1 | No |  | p.705 |
| A.bE2 | bE2h | Firmware error | 驅動器韌體錯誤 | Gr.1 | No |  | p.705 |
| A.bF0 | bF0h | System Alarm 0 | 驅動器內部程式錯誤 0 | Gr.1 | No |  | p.705 |
| A.bF1 | bF1h | System Alarm 1 | 驅動器內部程式錯誤 1 | Gr.1 | No |  | p.705 |
| A.bF2 | bF2h | System Alarm 2 | 驅動器內部程式錯誤 2 | Gr.1 | No |  | p.705 |
| A.bF3 | bF3h | System Alarm 3 | 驅動器內部程式錯誤 3 | Gr.1 | No |  | p.705 |
| A.bF4 | bF4h | System Alarm 4 | 驅動器內部程式錯誤 4 | Gr.1 | No |  | p.705 |
| A.bF5 | bF5h | System Alarm 5 | 驅動器內部程式錯誤 5 | Gr.1 | No |  | p.705 |
| A.bF6 | bF6h | System Alarm 6 | 驅動器內部程式錯誤 6 | Gr.1 | No |  | p.705 |
| A.bF7 | bF7h | System Alarm 7 | 驅動器內部程式錯誤 7 | Gr.1 | No |  | p.705 |
| A.bF8 | bF8h | System Alarm 8 | 驅動器內部程式錯誤 8 | Gr.1 | No |  | p.705 |
| A.bFb | bFbh | System Alarm B | 驅動器內部程式錯誤 B | Gr.1 | No |  | p.705 |
| A.bFd | bFdh | System Alarm D | 驅動器內部程式錯誤 D | Gr.1 | No |  | p.705 |
| A.C10 | C10h | Servomotor Out of Control | 馬達失控 | Gr.1 | Yes |  | p.705 |
| A.C20 | C20h | Phase Detection Error | 相位偵測不正確 | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C21 | C21h | Polarity Sensor Error | 極性感測器錯誤 | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C22 | C22h | Phase Information Disagreement | 相位資訊不一致 | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C50 | C50h | Polarity Detection Failure | 極性偵測失敗 | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C51 | C51h | Overtravel Detected during Polarity Detection | 極性偵測中偵測到超程 | Gr.1 | Yes | n/a 線性（極性偵測） | p.705 |
| A.C52 | C52h | Polarity Detection Not Completed | 極性偵測完成前就伺服 ON | Gr.1 | Yes | n/a 線性（極性偵測） | p.705 |
| A.C53 | C53h | Out of Range of Motion for Polarity Detection | 極性偵測移動距離超過 Pn48E | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C54 | C54h | Polarity Detection Failure 2 | 極性偵測失敗（2） | Gr.1 | No | n/a 線性（極性偵測） | p.705 |
| A.C80 | C80h | Encoder Clear Error or Multiturn Limit Setting Error | 絕對編碼器多圈資料清除或設定沒有成功 | Gr.1 | No |  | p.705 |
| A.C90 | C90h | Encoder Communications Error | 編碼器與驅動器無法通訊 | Gr.1 | No |  | p.705 |
| A.C91 | C91h | Encoder Communications Position Data Acceleration Rate Error | 編碼器位置資料計算錯誤 | Gr.1 | No |  | p.705 |
| A.C92 | C92h | Encoder Communications Timer Error | 編碼器通訊計時器錯誤 | Gr.1 | No |  | p.705 |
| A.CA0 | CA0h | Encoder Parameter Error | 編碼器內參數損毀 | Gr.1 | No |  | p.705 |
| A.Cb0 | Cb0h | Encoder Echoback Error | 與編碼器通訊內容不正確 | Gr.1 | No |  | p.706 |
| A.CC0 | CC0h | Multiturn Limit Disagreement | 編碼器與驅動器的多圈上限不同 | Gr.1 | No |  | p.706 |
| A.Cd1 | Cd1h | SigmaLINK II Node Configuration Error | 偵測到 Σ-LINK II 不能連的組態 | Gr.1 | No | n/a Σ-LINK II | p.706 |
| A.Cd2 | Cd2h | SigmaLINK II Power Supply Short-Circuit Detected | Σ-LINK II 電源系統錯誤 | Gr.1 | No | n/a Σ-LINK II | p.706 |
| A.Cd3 | Cd3h | SigmaLINK II Configuration Data Checksum Error | Σ-LINK II 組態資料儲存失敗 | Gr.1 | No | n/a Σ-LINK II | p.706 |
| A.Cd4 | Cd4h | SigmaLINK II Node Change Detected | Σ-LINK II 儲存組態與偵測到的節點不同 | Gr.1 | No | n/a Σ-LINK II | p.706 |
| A.Cd7 | Cd7h | SigmaLINK II I/O Device Communications Error | 與 Σ-LINK II I/O 裝置通訊錯誤 | Gr.2 | No | n/a Σ-LINK II | p.706 |
| A.Cd8 | Cd8h | SigmaLINK II I/O Device Status Error | Σ-LINK II I/O 裝置自己報錯 | Gr.2 | No | n/a Σ-LINK II | p.706 |
| A.CF1 | CF1h | Reception Failed Error in External Encoder | 外部編碼器無法通訊 | Gr.1 | No | n/a 全閉／外部編碼器 | p.706 |
| A.CF2 | CF2h | Timer Stopped Error in External Encoder | 外部編碼器通訊計時器錯誤 | Gr.1 | No | n/a 全閉／外部編碼器 | p.706 |
| A.d00 | d00h | Position Deviation Overflow | 位置偏差超過 Pn520 | Gr.1 | Yes |  | p.706 |
| A.d01 | d01h | Position Deviation Overflow Alarm at Servo ON | 伺服 OFF 期間偏差超過 Pn526 後伺服 ON | Gr.1 | Yes |  | p.706 |
| A.d02 | d02h | Position Deviation Overflow Alarm for Speed Limit at Servo ON | 伺服 ON 限速（Pn529／Pn584）期間下命令，偏差超過 Pn520 | Gr.2 | Yes |  | p.706 |
| A.d04 | d04h | Overtravel Alarm | 伺服 ON 時偵測到超程（Pn00D=n.2□□□ 才會出） | Gr.1 | Yes |  | p.706 |
| A.d10 | d10h | Motor-Load Position Deviation Overflow | 全閉迴路時馬達與負載偏差過大 | Gr.2 | Yes | n/a 全閉／外部編碼器 | p.706 |
| A.d30 | d30h | Position Data Overflow | 位置回授資料超過 ±1879048192 | Gr.1 | No |  | p.706 |
| A.E00 | E00h | EtherCAT Initialization Timeout Error 1 | 伺服控制模組與 EtherCAT 模組之間初始化通訊失敗（1） | Gr.2 | Yes |  | p.706 |
| A.E02 | E02h | EtherCAT Internal Synchronization Error 1 | 伺服控制模組與 EtherCAT 模組同步錯誤（1） | Gr.1 | Yes |  | p.706 |
| A.E71 | E71 | Safety Module Detection Failure | 安全模組偵測失敗 | Gr.1 | No | n/a 選購模組 | p.706 |
| A.E72 | E72h | Feedback Option Module Detection Failure | 回授選購模組偵測失敗 | Gr.1 | No | n/a 選購模組 | p.706 |
| A.E74 | E74h | Unsupported Safety Module | 接上不支援的安全模組 | Gr.1 | No | n/a 選購模組 | p.706 |
| A.E75 | E75h | Unsupported Feedback Option Module Alarm | 接上不支援的回授選購模組 | Gr.1 | No | n/a 選購模組 | p.706 |
| A.E81 | E81h | Safety Module Detection Disagreement | 安全模組型號與先前不同 | Gr.1 | No | n/a 選購模組 | p.706 |
| A.EA0 | EA0h | EtherCAT Initialization Timeout Error 2 | 伺服控制模組與 EtherCAT 模組之間初始化通訊失敗（2） | Gr.1 | No |  | p.706 |
| A.EA2 | EA2h | EtherCAT Internal Synchronization Error 2 | 伺服控制模組與 EtherCAT 模組同步錯誤（2） | Gr.1 | Yes |  | p.707 |
| A.Eb1 | Eb1h | Safety Function Signal Input Timing Error | /HWBB1 與 /HWBB2 輸入時序錯誤（差 10 秒以上） | Gr.1 | No |  | p.707 |
| A.EC6 | EC6h | Safety Module System Error 2 | 安全模組周邊電路錯誤 | Gr.1 | No | n/a 選購模組 | p.707 |
| A.EC8 | EC8h | Gate Drive Error 1 | 閘極驅動電路錯誤（1） | Gr.1 | No |  | p.707 |
| A.EC9 | EC9h | Gate Drive Error 2 | 閘極驅動電路錯誤（2） | Gr.1 | No |  | p.707 |
| A.F10 | F10h | Power Supply Line Open Phase | 主電源 ON 時 R／S／T 某相電壓低超過 1 秒（缺相） | Gr.2 | Yes |  | p.707 |
| FL-1 | FL-1 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-2 | FL-2 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-3 | FL-3 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-4 | FL-4 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-5 | FL-5 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-6 | FL-6 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| FL-7 | FL-7 | System Alarm | 驅動器內部程式錯誤（只顯示在面板，不進履歷，p.702） | – | No |  | p.707 |
| CPF00 | CPF00 | Digital Operator Communications Error 1 | 數位操作器與驅動器無法通訊（1） | – | No |  | p.707 |
| CPF01 | CPF01 | Digital Operator Communications Error 2 | 數位操作器與驅動器無法通訊（2） | – | No |  | p.707 |

手冊的註（p.702）：接進階安全模組時才會出的 A.047、A.048、A.E11～A.E15、A.E1F、A.E28、A.E29、A.E2B、A.E35、A.E36、A.E3A～A.E3C、A.E3E、
A.E78～A.E7C、A.EB0、A.EB2、A.EB3、A.EB9、A.EC0、A.EC1、A.EC3～A.EC5 另見 SIEP C710812 25／26；A.E75 只在裝全閉選購模組時出現。

### HT9050 最常遇到的幾個（處置原文，p.707-733）

| 警報 | 手冊列的原因 → 處置 | 頁 |
|---|---|---|
| A.A10 | Sync0 時序抖動 → **斷電重開、重新建立通訊** | p.721 |
| A.A11 | 馬達運轉中 EtherCAT 離開 OP → **reset 後重新建立通訊** | p.721 |
| A.A12 | 雜訊 → 查配線、做抗雜訊；主站沒在固定週期更新 PDO → 改主站；線材／接頭不良 → 重接 | p.721 |
| A.EA2 | Sync0 抖動造成內部同步抖動 → 斷電重開、重新建立通訊；驅動器故障 → 修或換 | p.732 |
| A.E02 | 傳送週期抖動 → 排除主站的週期抖動；驅動器故障 → 斷電重開，仍出現就換 | p.731 |
| A.410 | 電源電壓低／運轉中掉壓／瞬停（改過 Pn509 就調小）／保險絲斷／DC 電抗器跳線 | p.714 |
| A.810 | 絕對編碼器第一次上電；編碼器線拔過；**控制電源 +5 V 與電池都沒供電**；SGMX□-□□□H 內建電池電量到 0 → 做編碼器設定（絕對編碼器重置） | p.718 |
| A.830 | 電池沒接或接觸不良；電壓低於 2.7 V → 換電池 | p.718 |
| A.d00 | U／V／W 配線錯；命令速度或加速度太大；Pn520 對工況太小 | p.730 |
| A.d01 | 伺服 OFF 期間偏差超過 Pn526 後伺服 ON → 調 Pn526 | p.730 |
| A.d04 | 伺服 ON 中偵測到超程 → 修上位命令讓機構不超出超程與軟體極限、查超程配線、抗雜訊 | p.730 |
| A.Eb1 | /HWBB1 與 /HWBB2 動作時間差 10 秒以上 → 查輸出側與輸入側電路、線有沒有斷 | p.732 |

## 5. 怎麼清（Alarm Reset）

- **一定要先排除原因再 reset**；沒排除就 reset 繼續跑，可能損壞設備或起火（p.734）。
- 三種方法（p.734-735）：
  1. **Fault Reset 命令：6040h bit7 由 0 變 1**——清警報**也清警告**（p.734、p.665）。
  2. SigmaWin+：[Display Alarm] → [Reset axes]。
  3. 數位操作器：[ALARM RESET] 鍵。
- 清掉後狀態機回 Switch ON Disabled（p.600），要重新 Shutdown → Switch ON → Enable operation 才會伺服 ON。
- Software reset（Fn030／SigmaWin+）可以不斷電就重置驅動器、也能清警報，但要伺服 OFF、馬達停止，執行後約 5 秒沒回應（p.278）；
  2710h 的服務清單沒有它（p.654）⇒ **從 EtherCAT 主站做 software reset 的方法手冊未載明**。

### Fault Reset 清不掉、要別的程序的

| 警報 | 要怎麼清 | 頁 |
|---|---|---|
| A.810、A.820 | **Fault Reset 不行**；做絕對編碼器重置（Fn008／SigmaWin+／2710h 1008h），伺服 OFF 才能做，完成後斷電重開；多圈資料會變成 −2～+2 圈，機械原點位置會變 | p.222-224、p.654 |
| A.8□□（編碼器內部監視警報） | 斷電重開 | p.222 |
| 無電池絕對編碼器第一次上電的 A.810 | 做一次絕對編碼器重置後就不再出現 | p.222 |
| A.070 | 只能用 Reset Motor Type Alarm（Fn021／SigmaWin+），先把參數改成新馬達，做完斷電重開；一般 reset 與斷電都清不掉 | p.739-740 |
| 選購模組類（A.E71、A.E72、A.E81…） | 只能用 Reset Option Module Configuration Error（Fn014），做完斷電重開；一般 reset 與斷電都清不掉 | p.737-739 |
| A.CC0 | 多圈上限設定（Fn013／SigmaWin+／2710h 1013h）：先寫驅動器、斷電重開、再寫進馬達 | p.270-272、p.654 |
| A.020 | 參數初始化後重設；寫入次數超過上限或故障要換驅動器 | p.707 |
| A.021 | 從同型號同版本的驅動器寫參數後斷電重開 | p.708 |

### 不是警報、但會擋伺服 ON 的狀態

| 狀態 | 怎麼恢復 | 頁 |
|---|---|---|
| HWBB（/HWBB1 或 /HWBB2 OFF） | 兩個輸入都 ON → 6040h Shutdown → Switch ON＋Enable operation；條件：所有安全輸入 ON、還沒送 Servo ON、沒有在跑會自己 servo ON 的工具功能 | p.574-576 |
| FSTP（強制停止輸入） | FSTP OFF 時送過 Enable operation 的話，FSTP 回 ON 仍維持強制停止；要 Disable operation（回 BB）再 Enable operation | p.289 |
| 超程（P-OT／N-OT） | 可以往反方向動；Pn022=n.□□□1 時衝過開關也只准往回走 | p.200、p.203 |
| 警告（A.9xx） | 不一定停馬達；可用 Fault Reset 清，與伺服 ON／OFF、超程狀態無關 | p.202-203、p.734 |

## 6. 警報履歷（p.735-737）

- 最多保存最近 **10** 筆；欄位：編號（越舊編號越大）、警報碼與名稱、累積運轉時間（自控制電源與主迴路 ON 起，100 ms 為單位，約可記 13 年）（p.735-736）。
- **同一個警報一小時內連續發生只記一次**；超過一小時再發生才再記（p.736）。
- **Reset 與主迴路斷電都不會清履歷**；要用 Fn006 或 SigmaWin+ [Clear] 清，且參數不能是禁止寫入（p.736）。
- 不進履歷的：A.E50、A.E60（手冊原文如此，但這兩碼不在 p.702-707 的警報表上）、FL-1～FL-7（p.735、p.702）。
- 讀履歷的工具：Fn000／SigmaWin+（p.735）。從 EtherCAT 物件讀履歷的方法：手冊未載明。
- 另有 Alarm Tracing（警報前後資料，p.521）。

## 7. 警告一覽（p.741-742）

| 面板顯示 | 名稱（手冊） | 意義 |
|---|---|---|
| A.900 | Position Deviation Overflow | 位置偏差超過 Pn520 × Pn51E／100 |
| A.901 | Position Deviation Overflow Alarm at Servo ON | 伺服 ON 時偏差超過 Pn526 × Pn528／100 |
| A.905 | Error Detection Warning | 錯誤偵測功能偵測到異常 |
| A.910 | Overload | A.710／A.720 過載警報之前先出；不理會繼續跑可能變警報 |
| A.911 | Vibration | 運轉中異常振動；偵測位準同 A.520；用 Pn310 選出警報或警告 |
| A.912 | Internal Temperature Warning 1 | 控制板周圍溫度異常 |
| A.913 | Internal Temperature Warning 2 | 功率板周圍溫度異常 |
| A.920 | Regenerative Overload | A.320 之前先出 |
| A.923 | SERVOPACK Built-in Fan Stopped | 內建風扇停止 |
| A.930 | Absolute Encoder Battery Error | 電池電壓或內建電池電量低 |
| A.932 | SigmaLINK II I/O Device Communications Warning | Σ-LINK II I/O 裝置通訊錯誤 |
| A.933 | SigmaLINK II I/O Device Status Warning | Σ-LINK II I/O 裝置報錯 |
| A.93b | Overheat Warning | TH 輸入超過 Pn61C |
| A.942 | Speed Ripple Compensation Information Disagreement | 編碼器與驅動器存的轉速漣波補償資訊不一致 |
| A.971 | Undervoltage | A.410 之前先出 |
| A.9A0 | Overtravel | 伺服 ON 中偵測到超程（Pn00D=n.1□□□ 才出） |
| A.9b0 | SERVOPACK Preventative Maintenance Warning | 驅動器某個耗材到壽命 |
| A.9b1 | Servomotor Preventative Maintenance Warning | 馬達某個耗材到保養時間 |

- 總開關 Pn008=n.□X□□：0＝偵測警告（預設）、1＝除了 A.971 都不偵測（p.764）。
- 另需設定才會偵測（不受或不只受 Pn008 影響）（p.741）：A.911＝Pn310 n.□□□X；A.923 不受 Pn008 影響；A.930＝Pn008 n.□□□X（選 A.830 警報或 A.930 警告）；
  A.932／A.933＝Pn0DD；A.971＝Pn008 n.□□X□（不受 Pn008 n.□X□□ 影響）；A.9A0＝Pn00D n.X□□□（不受 Pn008 n.□X□□ 影響）；A.9b0／A.9b1＝Pn00F。
- A.9C□ 只在接進階安全模組時出現（p.741）。
- **A.9A0 不會停馬達、也不影響主站的動作命令**；但超程停止處理照做，所以軸可能沒到目標位置，要看回授位置確認停在安全處（p.202）。
- 警告清除：p.741 寫用 SigmaWin+ 清；p.734 寫 Fault Reset 命令可清警報或警告；超程警告在超程中用 Fault Reset 清掉後，要離開超程狀態才會再偵測（p.203）。

## 8. 對應 Steven 的常設規則

| 規則（Steven） | 手冊事實 | 軟體該怎麼對 |
|---|---|---|
| **斷電後一定要歸零**（Q88） | 主電源 OFF 或 HWBB → Operation Enabled 自動回 Switch ON Disabled（p.600 *4）；瞬停在 Pn509（預設 20 ms）內維持伺服 ON（p.248）；控制電源掉了驅動器等於重開，上電 ALM 最多 10 秒（p.135）；多圈絕對編碼器有電池時位置保留（p.265） | 不管編碼器有沒有記住位置，**照 Steven 規則重新歸零**；HOME 途中斷電＝停 HOME、所有軸 HomeFlag 清掉，電回來**不可**自動 reset＋激磁＋續做（Q88 M-a） |
| **警報清得掉 → 清（一次）→ 該軸重新歸零**（Q88、Q98） | Reset＝Yes 的警報在原因排除後可用 6040h bit7 0→1 清（p.702、p.734） | 送一次 Fault Reset → 6041h bit3 回 0 才算清掉 → 該軸 HomeFlag 清掉、重新歸零。表上 Yes 但 reset 後 bit3 還是 1 ⇒ 當成清不掉 |
| **警報清不掉 → 全機斷電重開後歸零**（Q88、Q98） | Reset＝No 的不能清（p.702）；部分還要額外工具程序（§5 表） | 報警、提示「全機斷電重置」；不自動重試、不繼續動。A.810／A.820／A.070／A.CC0／選購模組類光斷電不夠，提示要叫人做對應程序 |
| **先伺服 ON 才能放煞車**（Q89） | 送 Enable Operation 後至少等 50 ms＋煞車放開延遲才下命令（p.206）；延遲：SGMXJ-A5～-04 放開 60 ms、SGMXJ-06／-08 80 ms、SGMXA-30～-70 100 ms（p.206）；用 60FEh 放 /BK 後伺服 OFF 也不會自動煞（p.693） | 放煞車條件＝馬達電源已上＋**本輪** 6041h 到 Operation enabled＋延遲數完（Q89）。HT9050 的煞車是 IO 輸出 `Sw*Breaker`（[motors-9050.md](../../ht9050-hw/references/motors-9050.md) §4），不是驅動器 /BK——手冊的 50 ms＋延遲是針對 /BK 寫的，拿來當 IO 放煞車後到第一個移動命令的下限是推論 |
| **IO／馬達裝置 ERROR → 機台不可動；停止永遠不擋**（Q96） | Fault 時驅動器自己伺服 OFF（605Eh=0，p.672）；從 Operation enabled 往下的命令（Halt bit8、Quick stop、Disable operation、Shutdown、Disable voltage）在狀態機上一直有效（p.600-601、p.665-667） | 6041h bit3＝1 的軸（含同站雙軸另一軸，待確認）不送 Enable operation 與移動命令；HOME／START／手動都擋（Q96 原則）；**停止類命令不加任何閘** |

### 灰色地帶（待 Steven／EastSun 決定，不要自己猜）

1. **A.A10、A.EA2（以及 A.E02 的第二個原因）**：表上 Reset＝Yes，但手冊處置寫「斷電重開、重新建立通訊」（p.721、p.731-732）。照 Q98「先試清一次」會清得掉，但手冊建議斷電——要不要歸到「斷電重開」那一類？
2. **A.A11／A.A12**：Reset＝Yes，但 A.A10／A.A12／A.EA2 會讓 ESM 掉到 SAFEOP（p.702），沒回 OP 前 Switch ON 不會成立（p.600 *3）。清警報時主站要先把 ESM 帶回 OP；PCI-1203 這一步怎麼做（Acm_* 哪個呼叫）安川手冊未載明。
   homing skill 記錄「Yaskawa 斷電後 A.A12 要 Reset Error」（[ht9050-1203-homing](../../ht9050-1203-homing/SKILL.md) §5 第 10 點）。
3. **警告（A.9xx）算不算 Q96 的「裝置 ERROR」**：手冊說警告不一定停馬達（p.202）；機台政策手冊未載明。
4. **雙軸 SGDXW 一軸警報時另一軸**：本來源未涵蓋。
5. **A.810 反覆出現**：樹的 Fn008 段註解記錄過「做完 Fn008、斷電重開仍是 A.810」（`EtherCAT\Pci1203Control.cpp`，`kCmdAxAbsEncoderReset`）。手冊 p.718 的原因之一是「控制電源 +5 V 與電池都沒供電」——HT9050 馬達的編碼器碼是 U（多圈要電池，見 hardware 檔），機台有沒有電池、Pn002 怎麼設要上機看。
