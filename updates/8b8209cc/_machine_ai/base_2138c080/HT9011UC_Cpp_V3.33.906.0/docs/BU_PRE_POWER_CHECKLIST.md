# 通電前檢查表（BU 戰役）—— 在任何一顆軸得到電之前逐項確認

> `AI(W906-BU-CHECKLIST) 20260908` v2。
> **v1（commit `513d1b9`）有數個危險缺陷，由三路對抗式複驗抓出，本版全部更正。**
> 每一處更正都標了 `⚠️ v1 錯在哪`，因為 v1 已經 commit 過，有人可能看過它。
>
> **這份檔的用途是在機台旁邊拿著逐項勾。** 只放「通電前」的事。
>
> ⚠️ **每一項都寫「怎麼確認」。** 這棵樹的紀錄是：
> **一個沒有做的檢查，和一個通過的檢查，長得一模一樣。**

---

## ⛔ 0. 讀之前先知道兩件事

### 0.1 煞車咬死**今天就是活的**，不是「開了 G9 才會」

⚠️ **v1 把這件事寫成 G9 的條件後果，那是錯的。**

**這一條是用機器碼證的，不是用 grep** —— 因為「原始碼裡有這行」不代表它被編譯：

```powershell
$bin = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin"
$obj = "build_nonoracle\CMakeFiles\ht9045_sm.dir\csystem.cpp.obj"
& "$bin\nm.exe" --defined-only $obj | findstr IsEMGPressed
& "$bin\objdump.exe" -d -C $obj      # 找 IsEMGPressed() 區段內的 call
```

實測結果：

| 證據 | 值 |
|---|---|
| `IsEMGPressed()` 已編譯 | `00029302 T __Z12IsEMGPressedv` |
| **它的機器碼內真的有這條指令** | `29433: e8 5d f4 ff ff  call 28895 <IndexMotorBreakerOFF()>` |
| `aTester_Front.cpp.obj` | `U __Z20IndexMotorBreakerOFFv`（`:8938`）|
| `aTester_Rear.cpp.obj` | `U __Z20IndexMotorBreakerOFFv`（`:8934`）|
| `IsEMGPressed` 有活的消費者 | `fTeach.cpp.obj` 帶 `U __Z12IsEMGPressedv` |

而**釋放端全部不可達**：`IndexMotorBreakerON()` 全樹**只有兩個**呼叫點（已逐一列舉）——
`csystem.cpp:18773`（關著的 GATE G05）與 `csystem.cpp:22405`（在
`CountMotorPowerDelay()` 內，而該函式全樹**唯一**呼叫點 `csystem.cpp:19225`
就在 `#if 0 // GATE G21` 的橫幅 `:19214` 底下）。

> **★ 按下 E-STOP 就足以咬住 index 煞車，而樹裡沒有任何可達的東西會放開它。**
> 這與 `MotorPowerOnDelay` 的值無關 —— 釋放路徑本身不可達。

### 0.2 **全機每一個感測器今天都是 disabled** —— 不只安全門

⚠️ **v1 完全沒提這件事。** 鏈：

```
InitialHandler()            cinitial.cpp:16739   ← 零呼叫點
  └ InitHontechHardware()   cinitial.cpp:10867
      └ InitialSensor()     cinitial.cpp:2628    ← 從 IO_Table.csv 設每個 Sen[] 的 Enable
          └ InitialSafeDoor() cinitial.cpp:2943
```

`InitialHandler()` **全樹零呼叫點** —— 原始碼掃描（7 筆命中全是註解／宣告／定義本身）
＋ 機器碼確認（**沒有任何 .obj 帶 `U __Z14InitialHandlerv`**）。
而 `TMySensor` 的建構子 `mysensor.cpp:49` 就是 `Enable = 0`。

→ **從 `IO_Table.csv` 讀 Enable 的那一步從來沒有執行過。**
→ 每一個 `Sen[]` 在執行期都是 `Enable == false`。

### ⚠️⚠️ 而 `IsOn()` 與 `IsOff()` 對 disabled 感測器**都回 `false`**

`mysensor.cpp:127-131`（`IsOn`）與 `:177-181`（`IsOff`）是同一段：
`if(Enable == false) { State = -1; return false; }`

**所以 `IsOff()==false` 這個寫法會把「沒啟用」讀成「訊號在」。**
全樹（448 個 `.cpp`，排除 tests／build）：

| 寫法 | 筆數 | 今天讀到什麼 |
|---|---:|---|
| **`IsOff()==false`** | **139** | **全部為真 ＝「訊號在」** ← 危險方向 |
| `IsOn()==false` | 57 | 全部為真 ＝「訊號不在」← 安全方向 |

⚠️ **這不是移植缺陷** —— golden 的感測器是啟用的，那個慣用法在 golden 下正確。
它是「**軟體跑起來不證明感測邏輯有意義**」的量化理由。
⚠️ 數字是原始碼正則掃描，**沒有排除 `#if 0`**，所以是上界不是精確值。

### 三個直接後果

1. **「這台有安全門連鎖」是假的。** `CheckSafeDoorIsClosed()` 跳過所有門、回報
   「關著」—— **非 SIM 臂也一樣**。目前擋住 motion 的是 §4 那個
   `MotorIdleSafeDoorCheck == NULL` 的**意外**，不是門連鎖。
2. **§0.1 的煞車保護例外永遠不會觸發。** `IndexMotorBreakerOFF()` 有一個
   `if(Sen[SnFMotorDown].IsOn() && ...)` 分支會在「軸已經停在下方」時**不咬**
   —— 但 `IsOn()` 對 disabled 回 false，所以**永遠走 else，永遠咬**。
3. ⚠️ **把這條鏈接回去是「一次翻轉 139 個判斷點」的行為變更**，不是修 bug。
   要接就當成一整波做，不要在機邊順手接。

---

## 1. 先做這三件，不可跳過

- [ ] **備份** `D:\HT9045\system\` ＋ `config\` ＋ `IniData\` 到帶日期的目錄
- [ ] **`tools\production_audit.ps1 -Snapshot`**
      ⚠️ `_AUDIT_BASELINE` 只有 9 個檔，那是**雜湊基準不是副本** —— 備份才救得了你
- [ ] **清掉 8045 的持有者**
      ```powershell
      netstat -ano | Select-String ':8045|:8046'
      Get-Process | Where-Object { $_.ProcessName -match 'wb_publish|wb_gateway|wb_serve' }
      ```
      判準是「**沒有 PID 持有 8045**」，不是「我看過了」

---

## 2. 你在哪一條建置線上？

⚠️⚠️ **v1 給的判別法是錯的，而且朝危險方向失效 —— 它永遠回答「安全，繼續」。**

v1 寫：`nm <exe> | Select-String 'mn_start_line'`，有 ＝ 非 SIM 臂。
實測 **兩條線的 `wb_publish.exe` 都有** `_mn_start_line@8`
（`build_nonoracle` `00900ced T`／`build_sim_nonoracle` `008e0509 T`）。
根因：`Motor\vendor_offline_motionnet.cpp:149` **無條件**定義該符號
（該檔零個前置處理器指令），而那個 TU 是 `ht9045_motor` 的無條件成員，
所以連結後的 exe 永遠含它。**v1 的檢查在你正站在 SIM 臂上時也會說「繼續」。**

### ✅ 正確的判別法在**目的檔**層級，不是 exe

```powershell
$nm = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\nm.exe"
& $nm build_nonoracle\CMakeFiles\ht9045_sm.dir\cinitial.cpp.obj | findstr mn_start_line
```

- [ ] 出現 **`U _mn_start_line@8`**（未定義引用）＝ **非 SIM 臂**，可以繼續
- [ ] **沒有任何輸出** ＝ **SIM 臂，停下**

實測：`build_nonoracle` 命中 1、`build_sim_nonoracle` 命中 **0**。
機制：`cinitial.cpp:7922` 的 `#ifndef SOFT_SIMULTE` 底下 `:7927` 呼叫 `mn_start_line`，
SIM 臂那一段不編譯，所以該 TU 不產生未定義引用。

- [ ] 兩條線的 exe **檔名一模一樣**（`wb_publish.exe`／`wb_gateway.exe`／
      `wb_serve.exe`／`pci1203_linkprobe.exe`），大小不同（16,548,935 vs 16,345,664）

### SIM 臂到底少了什麼（v1 講得太寬）

⚠️ **v1 說 SIM 臂「沒有安全門連鎖」。不精確** ——
`CheckSafeDoorIsClosed` 在 SIM 臂**仍被定義且仍被呼叫**
（`Command.cpp`、`ainarm_SearchPlacePlate.cpp` 都還帶著未定義引用）。
**真正少掉的是「START 時」那一道**：`ckernel.cpp:1051` `#ifndef SOFT_SIMULTE`／
`:1052 if(CheckSafeDoorIsClosed()==false)`／`:1054 SystemStart=false;`／
`:1055 StopAllMotor();`／`:1058 #endif`。
（v1 的錯是往保守方向錯，操作指示不變 —— 但那個主張本身不真。）

### 為什麼選 32-bit

- [ ] 加密鎖 `KeyProDLL.dll` 是 **`Machine=0x014C`（i386）**、198,144 B
      —— 我自己解析過 PE 標頭。x64 行程無法載入 32-bit DLL。
- [ ] ⚠️ **但別把它讀成「32-bit 就載得到」** —— 全機唯一一份在
      **golden 樹裡**（`HT9011UC_Code_V3.33.906.0_20260618\Public\`），
      **不在任何 build 產物旁邊**。而 `Public\HTKeyProShim.cpp:50` 用**裸檔名**
      `LoadLibraryA("KeyProDLL.dll")`，`:51-52` 載不到就 return，
      `:72-75` **fail-OPEN，回 1 ＝ 鎖有效**。→ **兩條線今天都載不到，而且不會報錯。**

---

## 3. 煞車

- [x] **G03**（`csystem.cpp:18356`）已開 —— commit `2ae6aea`
- [x] **G14**（`csystem.cpp:18947`）已開 —— commit `2ae6aea`
- [ ] ⛔ **G9（`csystem.cpp:16913`）仍關閉，而且不可單獨開**

### G9 的鏈（⚠️ v1 有四個行號是錯的，本版已重量）

`*BreakerOFF()` ＝ **咬住**、`*BreakerON()` ＝ **放掉**（從機制確認，不是從命名）。

| 環節 | 正確位置 | v1 寫的 |
|---|---|---|
| `DoMotorPowerOn` 第三個活呼叫點 | `MainHome.cpp:487` | ✅ 對 |
| `MotorPowerOnDelay=0` | **`MainHome.cpp:526`** | ❌ `:498`（那是註解行 —— **被我 20260908 自己加的註解區塊往下推了 28 行**，而那段註解本身還寫著 `:498`）|
| 釋放的守衛 | **`csystem.cpp:22395`** | ❌ `:22386` |
| `IndexMotorBreakerON()` 釋放點 | **`csystem.cpp:22405`** | ❌ `:22396` |
| 大寫後果段 | **`csystem.cpp:22424-22432`** | ❌ `:22420-22422` |

⚠️⚠️ **而且那個大寫段說的是反的**：它寫
`THE INDEX FRONT/REAR Z BRAKES ARE STILL RELEASED` ——
**v1 把讀者送去那裡確認「咬死」，但那段講的是「還放著」。**

### 真正的機制比 v1 講的更強（也更糟）

v1 說釋放被跳過是因為 `MotorPowerOnDelay=0`。**更強的事實**：
`CountMotorPowerDelay()` 全樹**唯一**呼叫點 `csystem.cpp:19225` 在 `#if 0 // GATE G21` 內
→ **`:22405` 的釋放無論 `MotorPowerOnDelay` 是什麼都不可達。**
→ 修 `MotorPowerOnDelay` 沒有用；**要開 G21 或 G05。**

**G9 若要開，必須連同** golden `csystem.cpp:133` `int iEMGPressDelay=3;`
＋ GATE G34（`:18754`）＋ GATE G05（`:18767`）**同一顆 commit**。

### `StopAllMotor()` 是空 stub

- [ ] `aHotPlateSubstrate.cpp:1090` `void StopAllMotor() {}`。
      golden **只有** `StopAllMotor(bool bIndexCanStop=true)`，移植樹多發明一個無參數空多載
      遮住它（真本體在 `Motor\myGALILmotor.cpp:5612`）。
      → **G14 目前只恢復 `SystemStart=false`，「停馬達」那半是 no-op。**

### ★ G-BRAKE-OUT：jog 前的最後一道

- [ ] 任何 index Z 軸（**MTestZ1／MTestZ2**）被 jog **之前**，在 gdb 讀：
      ```
      p SW[SwFMotorBreaker].OutValue
      p SW[SwBMotorBreaker].OutValue
      ```
      **必須確認煞車是「放掉」的。**
      兩者在這台都是 `Enable=1`（`IO_Table.csv:609`／`:578`，行號已驗）。
- [ ] ⚠️ **按過 E-STOP 之後一定要重驗這兩個值**（見 §0.1）

---

## 4. `MotorIdleSafeDoorCheck` 是 NULL —— 那是目前唯一擋著 motion 的東西

- [ ] ⚠️ `MotorIdleSafeDoorCheck` **是 `NULL`**：唯一的生產指派
      `cinitial.cpp:4462` 在 `#if 0 // GATE 3`（`:4461`）內，**在建置產物裡沒有痕跡**
- [ ] `Motor\HTMotor.cpp:111-127` 的 `CheckIsSafeDoorOpen()`，`:122` 退回
      `return (Enable == true)` —— 行號已驗，語意方向**未反轉**

### ⚠️ v1 說「motion 被擋住」，那太寬了

實測有門檢查的呼叫點：`mymotor.cpp.obj` 只有 **4 個**（`Home`／`MotorHome`／
`JogP`／`JogN`）＋ `myGALILmotor.cpp.obj` 23 個。
**而活的 `MotorMovePosition`／`MotorMove`／`MotorMoveShuttleShake` 是離線 stub、
完全沒有門檢查**（它們帶檢查的那份在 `#if 0` 內）。
→ **「擋住」只涵蓋 home 與 jog，不涵蓋一般移動。**

### ⚠️ v1 說「走 1203 的 jog 根本沒有門檢查」，那是錯的

**沒有任何驅動類別有這個檢查** —— SMC／SYNTEK／MN200／Sim 全部是 0，
`myEthercatmotor.cpp` 也是 0。檢查住在**與卡無關的 `TMyMotor` 層**，
所以**走 `TMyMotor` 的 jog 在 1203 臂上一樣有檢查**。1203 沒有比別人差。

- [ ] ⛔ **不要移除那條 NULL 退路。** 安全門 callback 的 patch **刻意未套用**
      （harness 曾對它丟 SECURITY WARNING）
- [ ] ⚠️ **GATE 3 的關閉理由已失效**：它寫著 `IdleCheckSafeDoor` 在本樹無定義，
      實測它**定義在 `csystem.cpp:23657`**（`nm`：兩臂都是 `T`）。
      **但理由失效不等於可以開** —— 見 §0.2：開了之後門仍然全部 disabled。

**接上之後才生效的四段式斷言（G-DOORCB）**，四項全過才算有門連鎖：
- [ ] (a) `MotorIdleSafeDoorCheck != NULL`
- [ ] (b) `InitialOK == true`
- [ ] (c) `Sen[SnSafeDoor1..3,6..9].Enable == true` **且** `.Type == 1`
      ← ⚠️ **今天必定不成立**（§0.2）
- [ ] (d) 執行檔來自 `build_nonoracle`（用 §2 的目的檔判別法）

⚠️ `mysensor.cpp:177-181` 對 `Enable==false` 回 false，而 false ＝「門**關著**」
（消費端 `csystem.cpp:23468`／`:23477` 已驗）。
- [ ] ⛔ **嚴禁為了消警報關掉任何安全門那一列的 `Enable`**
      ⓘ 修正 v1：`IO_Table.csv` **有**在稽核基準裡，所以那個編輯**會**被報成「變更」——
      稽核抓得到「改了」，只是不判斷「安不安全」

- [ ] ⚠️ **09-09 的門連鎖會讀 SIM IO 後端**：`MyLaneIo.cpp:139`
      `pIO = new TSimIOBackend();`，`SetBackend` 無生產呼叫點
      → `IO_CARD_TYPE=4`／`ISABase=3` **不會**把門的讀取導到硬體

---

## 5. 宣稱物理前提的 shim —— ⚠️ v1 這一節有五項是錯的

**判別方法**：問連結器誰定義了那個符號。
shim 檔的 `.obj` 沒有它 ＝ shim 是死碼、真本體接手了。

### ✅ 已退役（v1 誤列為活的 fail-open，**其中一項還被我標星**）

| shim | 全樹唯一定義者 |
|---|---|
| ⚠️ **`MoveInArmZToPlateSafe`**（v1 標星「Z 立刻到安全位」）| **`ainarm2.cpp.obj`** ＝ 真本體 |
| `CheckHeaterOK` | `uHeaterThread.cpp.obj` |
| `DoLockUnloader` | `csystem.cpp.obj` |
| `CheckOutArmToTask50` | `aoutarm9045.cpp.obj` |
| `IsOutArmCleanOutFinish` | `csystem.cpp.obj` |

### ⚠️ 仍然是唯一定義、確實還在宣稱成功（4 個）

| [ ] | shim | 宣稱 | golden 本體 |
|---|---|---|---:|
| [ ] | `ainarm9045.cpp:2383` `CheckShuttleSensor_9045` | **「飛梭上沒有浮料」** | `ainarm9045.cpp:2070`，160 行 |
| [ ] | `aHotPlateSubstrate.cpp:586` `IsPickSuckFinish` | 「取料完成」 | `mykitsuck.cpp:917`，12 行 |
| [ ] | `aHotPlateSubstrate.cpp:587` `IsPickDestroyFinish` | 「吹料完成」 | `mykitsuck.cpp:930`，12 行 |
| [ ] | `aHotPlateSubstrate.cpp:588` `IsPickFinish` | 「取料循環完成」 | golden 未找到定義 |

### ✅ 五個「多定義者」已全部定案（20260908 BU-V2，`objdump -t` ＋ 反組譯）

**三個根本不是衝突** —— 先前的寬鬆子字串比對抓到的是不同符號：

| 符號 | 為什麼不是衝突 |
|---|---|
| **`UseFix3Cylinder`** | `cmydef` 那個是**變數** `_bUseFix3CylinderActive`（區段 3）；函式 `__Z15UseFix3Cylinderi` **唯一定義** → **確實是活的 fail-open，v1 標星是對的** |
| `InArmSideAllClose` | 四個**不同**的 mangled 名：`__Z17...i`／`__Z24...WithKitii`／`__Z24..._4x4_16i`／`__Z20..._32ii` |
| `DoFullViewCheck` | `TfContact::InitDoFullViewCheck` vs `TfContactShim::` → **不同類別** |

### ⚠️⚠️ 兩個是真衝突，而且 **shim 贏了 —— 三條線一致**

`CheckRotateOutNotFinish` 與 `MoveOutRotateToDegreeAtSameTime` 在
`aoutarm_shims.cpp.obj` 與 `aRotateKIT_Out.cpp.obj` **兩邊都是
`(sec 1)` `scl 2` 強定義**。誰贏靠封存檔成員順序決定，實測結果：

| 線 | exe 內實際本體 | 真本體大小 | 結論 |
|---|---|---|---|
| `build_nonoracle` | shim | 21／235 條 | **shim 贏** |
| `build_sim_nonoracle` | shim | 21／235 條 | **shim 贏** |
| `build_x64`（F5 預設） | shim | 33／292 條 | **shim 贏** |

x64 的 `MoveOutRotateToDegreeAtSameTime` 反組譯就是它的全貌：

```
push %rbp / mov %rsp,%rbp / mov %ecx,0x10(%rbp) / mov %dl,0x18(%rbp)
mov $0x1,%eax        ← 無條件 return true
pop %rbp / ret
```

**「旋轉已到達指令角度」是憑空回報的，沒有任何東西動過。**
真本體（235／292 條）在每一個二進位裡都是死的。

- [ ] ⚠️ **這個結果是封存檔順序決定的，很脆弱** —— `CMakeLists.txt` 的來源清單
      一被重排，連進去的本體就會**靜默對調**。要靠它就要有 gate 釘住。
- [ ] ⚠️ **量測陷阱，記在這裡因為它差點讓我寫錯**：用「指令條數」比對本體會把
      **`nop` 對齊填充算進去**。x64 的 shim 看起來 18 條、32-bit 看起來 5 條，
      我因此一度判成「兩條線不一致」。**實際本體是 7 條，其餘 11 條是填充。**
      → **要比就反組譯，不要只比計數。**

ⓘ **golden 本體行數是可信的**：獨立的括號配對複驗逐一重現了
114／160／393／23／19／94／102／45／26／11／100，一個不差。

---

## 6. 離線 stub 的 fail-open／fail-closed

- [x] **CC-Link 已改 fail-closed** —— commit `d815f3f`；七個 `md*` 回 `-31`
      （golden 自己的 `DLL load error`，已驗）
- [x] **運動 stub 是 fail-closed（正確）** —— 46 ＋ 7 ＋ 173 個失敗碼，
      **零個回成功、零個前置處理器條件**（獨立複驗數字一致）
- [ ] ⚠️ **RotateKit 兩個 shim** 見 §5 最後一段 —— 「仍 fail-open」只在
      封存檔抽取讓 shim 贏的情況下成立，**尚未定案**

---

## 7. 資料編輯 —— 順序就是安全，一次一軸

⚠️ **這幾格是「一個人一個下午單獨做得完」的唯一項目，而它們正好武裝運動。
沒有任何 gate 會失敗**：稽核只驗「檔案變了」，不驗「新內容安不安全」。

### ⚠️⚠️ v1 的欄位編號是錯的，而照著做會改到**軟極限**

實測表頭（1-based）：

| 檔 | `BoardID` | `Port` | 第 3／4 欄是 |
|---|---|---|---|
| `Mot_Table.csv`（逗號分隔） | **第 3 欄** | 第 4 欄 | `BoardID`／`Port` |
| `Mot_Table-new.csv`（**TAB 分隔**）| **第 5 欄** | 第 6 欄 | **`SoftLimitN`／`SoftLimitP`** |

> **v1 寫「BoardID 在 `Mot_Table.csv` 是第 2 欄、在 `-new` 是第 4 欄」—— 兩個都錯。
> 照 v1 的「第 4 欄」去改 `-new`，改到的是 `SoftLimitP`，一個軟極限。**

- [ ] **用欄位名稱定位，不要用編號** —— 兩個檔的版面差異不只 `BoardID`，
      `-new` 把 `SoftLimitN/P` 提到第 3／4 欄
- [ ] 確認**哪一份 `Mot_Table` 是權威的**（`system\` 有五份；現行那份是
      **2025-09-09、一年前**的）
- [ ] 45 列裡**有 15 列 `BoardID`/`Port` 兩格都空**（M13-M18 ＝ MTestY1/Z1/Z2/Y2 ＋
      MOutShuttle1/2，M35-M43）—— 無法用位址對上卡，**這是正常不是錯誤**
- [ ] **先重標速度，再改 `CardModel`** —— `JogHighSpeed` 有 **900000**、870000、35000。
      ⚠️ **不重標就改卡別，第一次 jog 就是全速**
- [ ] `IO_CARD_TYPE` 0 → **4**（`PCI1203_IO`，`cmydef.h:6022`；六個 gate 已接線）
- [ ] `IO_Table.csv` 的 `ISABase` → **3**（`ePCI1203`，`MachineType.h`）
      ⚠️ 與 `PCI1203_IO=4` **是不同編號空間**
- [ ] ⚠️ **`SHUTTLE_SENSOR_TYPE`（→6 或 7）或 `VacuUnitType`（→1）—— blast radius 最大的一步**
      `INSTALL_ETHETCAT()`（`EtherCAT/MyEtherCAT.cpp:604-614`）為真的唯一條件，
      鏈：`cinitial.cpp:10882` → `myMN200motor.cpp:1469/1471` → `uiDevhand`（已驗）。
      **同時武裝飛梭感測器與真空單元的「輸出」寫入路徑。一次只翻一個。**
- [ ] **一顆軸**改 `CardModel=PCI1203` → servo on → 最低速 jog 幾 mm →
      確認位置回饋方向 → 驗軟極限真的擋 → **過了才第二顆**
- [ ] **E-stop 前置測試**：任何軸上電前，確認 EMG 真的斷電
      ⚠️ **而且照 §0.1，按下 E-STOP 會咬住 index 煞車而沒有東西放開它** ——
      測完 E-stop 要重驗 §3 的兩個 `OutValue`

---

## 8. 每一步之後

- [ ] `tools\production_audit.ps1`（無參數）—— 九根逐檔 MD5 必須**零變更**
- [ ] 若稽核紅了：**先找出是誰寫的**，復原，再繼續。
      20260908 抓到過一次 `IniData\Data\Fail Open\HandlerCondition.Data`，
      而寫它的測試是**通過的**（圍堵已於 commit `d71ed58` 補上）
- [ ] ⓘ **修正 v1**：`D:\HT9045\EXE` **今天已經存在**（空目錄，2026-09-02 14:00:37 建立），
      而稽核基準已記錄 `# exists=True` → `production_audit.ps1:120-123` 的
      「這個目錄出現了」警報**永遠不會再觸發**。**不要依賴它。**

---

## 9. 絕不在機邊做的事

1. 不解 **G9**（除非連同 G34 ＋ G05 ＋ `iEMGPressDelay`；且注意 G21 才是釋放的鎖）
2. 不**移除** `MotorIdleSafeDoorCheck` 的 NULL 退路
3. 不為了消警報關掉任何安全門感測器的 `Enable`
4. 不在 `build_sim_nonoracle` 上做任何機邊驗證（用 §2 的**目的檔**判別法確認）
5. 不動 `MachineType.h:48`（`SOFT_SIMULTE`）
6. 不跑 `pci1203_linkprobe --enumerate` 當煙霧測試
7. 不把 V906 的 exe 複製進 `D:\HT9045\EXE`
8. **不用欄位編號改 `Mot_Table`** —— 用欄位名稱（見 §7）
9. 不把「模擬跑起來」講成「機台跑過了」
