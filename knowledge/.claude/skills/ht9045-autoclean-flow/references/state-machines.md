# Auto Clean State Machine Reference

## DoAutoCleanKit() — 主協調器

**檔案**: `AutoClean/AutoClean.cpp` L4367  
**Task 變數**: `iDoAutoCleanTask`

### 完整 Task 流程圖

```
Task 1-2: Fix3 Cylinder 處理
├─ Task 1: 檢查 bUseFix3CylinderActive
│           Yes → UseFix3Cylinder(0) [Task 2]
│           No  → DoStructUnitConvert + DoInArm_9045_Type [Task 5]
└─ Task 2: Fix3 使用完成確認

Task 3: Full View Check (RTC)
├─ 條件: CosFunction.bFullTestBeforeAutoClean && REAL_TIME_CCD
├─ fContact->DoFullViewCheck() [Task 3]
└─ 完成 → Task 5

Task 5: **開始 AUTO CLEAN**
├─ 取得 Clean Pad 數量
├─ 數量 >= device count → Task 20
├─ 否則 → ERROR [Task 2000]
└─ SECS Event #34 (AutoClean Start)

Task 10: 初始化 Shuttle + Index tasks
└─ Task 20

Task 20: Z 安全 + Arm 選擇
├─ MoveInArmZToPlateSafe(Task) [等待]
├─ 依 iAutoClean_SelectArm 選擇:
│  0: Arm1 only → Task 100
│  1: Arm2 only → Task 2100
│  2: Both → Task 100 → Task 2100
└─ CloseMode 檢查切換

Task 100: **InArm IC 狀態判定點**
├─ InArm 無 IC + Shuttle 有 Clean IC → Task 540 (from Shuttle)
├─ InArm 有 HAS_CLEAN_IC → Task 400 (to Shuttle)
└─ 否則 → Task 200 (from Kit)

Task 200-300: **Pick from CleanKit 階段**
├─ Task 200: DoAutoCleanPickfromCleanKit(euShuttle1, true) [初始化]
├─ Task 300: DoAutoCleanPickfromCleanKit(euShuttle1, false) [迴圈]
│  ├─ Return 1: 成功 → 繼續
│  ├─ Return 2: 錯誤 → Task 10 (重載)
│  └─ Return 0: 進行中 [等待]

Task 400-530: **Place to Shuttle + Sensor Check**
├─ Task 400: DoPlaceToShuttle(euShuttle1)
├─ Task 510-522: Shuttle sensor 檢查 (搖/敲/振)
│  ├─ Ret 1: OK
│  ├─ Ret 2: 需搖動 → Task 520/521/522
│  ├─ Ret 3: 重試 → Task 511
│  └─ 其他: Task 515
├─ Task 515: 選擇下一階段 (Task 540 或 2100)
└─ Task 530: InSHT1InLF 確認

Task 540-800: **Shuttle 定位 → Pick from Shuttle**
├─ Task 540: MoveInArmXYToShuttle [等待]
├─ Task 600: 檢查 FLCarryKit + Shuttle IC 狀態
│  ├─ 無 IC + 無 Clean IC → Task 1200
│  ├─ Shuttle 有 CLEAN_FINISH_IC → Task 800 (pick)
│  └─ 巢狀判定邏輯
├─ Task 800: DoPickFromShuttle(euShuttle1, iShuttleRowKit)

Task 1000-1100: **Z 安全 + 掉料檢查**
├─ Task 1000: MoveInArmZToPlateSafe
├─ Task 1050: 掉料檢查 (JAM0128)
│  ├─ 掉料 → K_SKIP → HAS_NULL_CLEAN_IC
│  └─ Task 1100

Task 1100-1200: **Place to CleanKit 階段**
├─ Task 1100: DoAutoCleanPlaceToCleanKit(true) [初始化]
├─ Task 1200: DoAutoCleanPlaceToCleanKit(false) [迴圈]
│  ├─ 成功 + InArm 空:
│  │  ├─ SelectArm==0 → Task 2000 (完成 arm1)
│  │  ├─ SelectArm==2 → 檢查更多 pad → Task 1300
│  │  └─ 否則 → Task 2000
│  └─ InArm 有 IC → Task 1000

Task 1300: **切換至 Arm2**
├─ MoveInArmXYToShuttle(euShuttle2)
├─ 支援雙臂: Task 2600 (2-arm loop)
└─ 否則: Task 2100 (arm2 single)

Task 2000: **完成 AUTO CLEAN**
├─ 確認所有 Shuttle/Index 無 Clean IC
├─ 清除馬達鎖定
├─ bRunAutoClean = false
├─ ReadWriteAutoCleanCount(false) [持久化]
├─ SearchCleanNum() [更新顯示]
├─ SECS Event #50 (AutoClean Finish)
├─ FTP 上傳 (SPIL)
└─ Task → 初始值

Task 2100-2600+: **Arm2 流程（Arm1 的鏡像）**
└─ bSupport2Arm=true 時可同步運作
```

---

## DoAutoCleanPickfromCleanKit() — 吸取狀態機

**檔案**: `AutoClean/AutoClean.cpp` L2549  
**Task 變數**: `iAutoCleanPickFromCleanKitStageTask`  
**回傳值**: 0=進行中, 1=完成, 2=錯誤

```
Task 1: **Z 安全初始化**
├─ MOT[MInArmX/Y].StopMotor()
├─ MoveInArmZToPlateSafe(Task)
└─ InOutArmSuckActiveSet() [清除旗標]

Task 10: **檢查 Kit 是否有 Clean IC**
├─ TrayHasCleanIC():
│  ├─ CheckCleaningCount() 檢查
│  │  ├─ FALSE: bResetCleanCount=true → Task 30 (錯誤)
│  │  └─ TRUE: 繼續
│  ├─ SearchiAutoCleanNum() [尋找可用 Pad]
│  ├─ GetShuttleState() → iShuttleRowKit
│  └─ Task 20
├─ InArmSuck 有 IC:
│  └─ Task 3000
└─ 無 IC + CheckCleaningCount() FAIL:
   └─ Task 30 (錯誤)

Task 20-21: **移動至 Kit 位置**
├─ SearchCleanKitUpDown(iShuttleRowKit, iSht)
├─ MoveInArmXYPickCleanKit(bAutoPick, iShuttleRowKit, iSht)
└─ Task 200

Task 30-31: **Clean Pad 計數錯誤**
├─ WAR1922 alarm
├─ iAutoCleanAlarm=1
└─ Return 2 (錯誤)

Task 200-300: **驗證 XY 到位**
├─ 檢查 encoder 位置 ±20 容差
│  ├─ 偏差 > 20: Task 21 (重試)
│  └─ OK: Z 移至 pick 高度
└─ Task 1000

Task 1000: **核心吸取 PickFromCleanKit()**
├─ 吸取 Clean Pad
├─ 吸嘴錯誤 → Task 1050
└─ Task 3000

Task 1050-1102: **吸取重試處理**
├─ iRetryCT++ 遞增
├─ 超過上限: JAM0110 (K_RETRY|K_SKIP)
│  ├─ K_SKIP: HAS_NULL_CLEAN_IC → Task 1101
│  ├─ K_RETRY: 重置 → Task 1102/1101
│  └─ 回到 Task 21

Task 3000: **Z 安全 + 掉料檢查**
├─ MoveInArmZToPlateSafe
├─ CheckInArmSuckFromCleanKitICFallDown(true)
│  ├─ 掉料: Task 300 (重試吸取)
│  └─ OK: Task 3100

Task 3100-3300: **計數遞增 + 完成**
├─ 遞增 AutoCleanStringGrid 計數
├─ Task 3200: 8-Picker 特殊模式檢查
└─ Task 3300: Return 1 (成功)
```

---

## DoAutoCleanPlaceToCleanKit() — 放回狀態機

**檔案**: `AutoClean/AutoClean.cpp` L4178  
**Task 變數**: `iAutoCleanPlaceToCleanKitTask`  
**回傳值**: true=完成, false=進行中

```
Task 1: Z 安全 + InOutArmSuckActiveSet()
Task 5: 延遲 + InitialSet()
Task 10: CheckInArmSuckICFallDownToHasNullIC()
├─ 掉料: Task 5
├─ InArm 空: return true
└─ Task 11

Task 11: **移動至 Kit**
├─ 全部 NULL_IC: 跳過移動 → Task 100
├─ 有真實 IC: MoveInArmXYPickCleanKit + Z 移動
└─ Task 100

Task 100: **Z 到 Place 高度**
├─ 暫停模式處理 → Task 200
├─ Picker motor delay 100ms → Task 110
└─ Task 300

Task 300: **核心放料 PlaceToCleanKit()**
└─ Task 400

Task 400: **Z 安全 + 完成檢查**
├─ InArm 空: return true (成功)
└─ 繼續
```

---

## DoIndexAutoClean() — Index 接觸循環

**檔案**: `AutoClean/AutoClean.cpp` L7231  
**Task 變數**: `iDoIndexAutoCleanTask`  
**最大狀態機 ~1900 行**

### Arm1 流程 (Task 1-1900)

```
Task 1: ZHome, bInedxCleanFinish[0/1]=false
Task 100: Z 安全，路由分派
Task 200: iContactCount=0

Task 300-400: Y 臂移至 Front/Rear
├─ CleanKit: 鎖定 Shuttle1（FLCarryKit 有 IC 時）

Task 400-500: Z 下降到 TestZ1_Pick + 延遲
Task 500-600: **吸取迴圈**
├─ FLCarryKit → FTestSuck 資料搬移
├─ 吸嘴錯誤 → Task 650

Task 650-670: **錯誤恢復**
├─ bAutoCleanShuttle1MoveToLeft=true
├─ JAM0312 (K_SKIP: 標記 HAS_NULL_CLEAN_IC)
├─ 等待 Shuttle 回 LEFT → 重吸

Task 700-720: Z 安全 + 延遲

Task 800: 吸嘴全通道 Normal ON
Task 900: Y 移至 Middle
Task 1000: Z Home

Task 1050-1060: **Drop 階段**（ContactMode==0 時跳過）
├─ Z 移至 iAutoCleanZ_Drop[]
├─ 關閉真空 → 等待 Drop_Wait → 開啟真空

Task 1150: **Contact 接觸**
├─ Z 移至 iAutoCleanZ_Contact[]
├─ iContactCount++
├─ 等待 iAutoClean_ContactTime

Task 1200: **接觸維持**
├─ iContactCount < ContactCount → Task 1300 (微位移)
└─ 全部接觸完成 → Task 1350

Task 1300-1310: **Z 微位移 (Shift)**
├─ Z 移至 iAutoCleanZ_Shift[]
├─ 掉料檢查 (JAM0314)
└─ → Task 1050 (下一輪接觸)

Task 1400: Z 安全

Task 1500: Y 移至 Front
Task 1600: Z 到 Place 高度

Task 1700: **Destroy 吹回**
├─ FTestSuck → FLCarryKit 資料搬移
├─ 掉料: JAM0327

Task 1800: Z 安全
Task 1900: bInedxCleanFinish[0]=true
├─ 雙臂: → Task 2200
└─ CleanAir: → 完成
```

### Arm2 流程 (Task 2100-3900)

| Arm1 Task | Arm2 Task | 差異 |
|-----------|-----------|------|
| 300 | 2300 | Y 移 BTestSuck |
| 400-600 | 2400-2600 | BLCarryKit/BTestSuck |
| 650 | 2650 | Shuttle2 錯誤恢復 |
| 700-1900 | 2700-3900 | MOT[MTestZ2] |

---

## DoShuttle1AutoClean() — Shuttle1 定位

**檔案**: `AutoClean/AutoClean.cpp` L5945  
**Task 變數**: `iDoShuttle1AutoCleanTask`

```
Task 1: 檢查 MOT[MInShuttle1].IsCanMove()
├─ CleanKit → Task 200
└─ 錯誤恢復 → Task 3000

Task 200: **決定 LEFT 或 RIGHT**
├─ Index 忙碌 → Task 2000 (RIGHT)
├─ FLCarryKit 有 Clean IC → Task 2000 (RIGHT)
├─ FLCarryKit 空 → Task 1000 (LEFT)
└─ 混合狀態 + !bInedxCleanFinish[0] → Task 2000

Task 1000: **LEFT 移動**
├─ 檢查 MTestZ1 安全高度
└─ MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft)

Task 2000: **RIGHT 移動**
└─ MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight)

Task 3000: **錯誤恢復**
└─ 慢速 (10%) 回 LEFT
```

---

## DoShuttle2AutoClean()

與 DoShuttle1AutoClean 結構相同，控制:
- MOT[MInShuttle2]
- BLCarryKit / BTestSuck (Rear)
- bInedxCleanFinish[1]
- Task 範圍對應 Index Task 2400-3900

---

## 函式呼叫矩陣

| 主函式 | 子函式 | 說明 |
|--------|--------|------|
| DoAutoCleanKit | DoAutoCleanPickfromCleanKit | 吸取 Clean Pad |
| DoAutoCleanKit | DoPlaceToShuttle | 放到 Shuttle |
| DoAutoCleanKit | DoPickFromShuttle | 從 Shuttle 取回 |
| DoAutoCleanKit | DoAutoCleanPlaceToCleanKit | 放回 Kit |
| DoAutoCleanKit | CheckShuttleSensor_Clean | Shuttle 感測器檢查 |
| DoAutoCleanPickfromCleanKit | MoveInArmXYPickCleanKit | XY 移動定位 |
| DoAutoCleanPickfromCleanKit | MoveInOutArmZToKitPickPlace | Z 移動 |
| DoAutoCleanPickfromCleanKit | PickFromCleanKit | 核心吸取 |
| DoAutoCleanPickfromCleanKit | CheckInArmSuckFromCleanKitICFallDown | 掉料檢查 |
| DoAutoCleanPlaceToCleanKit | PlaceToCleanKit | 核心放料 |
| DoIndexAutoClean | DoSocketSensorAlarm | Socket 浮料處理 |
| DoIndexAutoClean | RecDebug | 除錯追蹤 |
