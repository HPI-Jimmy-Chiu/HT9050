# 機台端解閘清單與前提重驗（2026-09-26）

> AI(W906-GATE-AUDIT) 20260926：唯讀稽核，本文件是這次唯一新增的檔案。沒有改任何程式、沒有建置、
> 沒有啟動 wb_serve／ioweb_probe／pci1203_linkprobe、沒有碰 1203 卡、沒有跑 ctest。
> 行號全部量於 HEAD `7961939`（分支 `integ/ioweb-8484bdb4`）；本文件引用的六個引擎檔（csystem.cpp、uhome.cpp、
> cinitial.cpp、csystem.h、database.cpp、cSocket.cpp）在工作樹與 HEAD 相同（同時段其他工作正在改 1203 監看器等別的檔，與本文件無關）。
> 行號會漂，要用就重量，不要抄。

---

## 0. 先講結論

1. **範圍** `95c398b..7961939` 共 10 顆 commit。真的拿掉 `#if 0` 的只有兩顆：
   * `0d253a0`（MT-E3b，20260925）：14 個 `#if 0` 閘，外加 2 個「不是 `#if 0`、但效果等於解閘」的改動，並新關了 1 個閘。
   * `8929d13`（MT-FIX1b，20260926）：4 個閘＋G15 的前半，並替 3 處 golden 的煞車放開加了一道 Servo-ON 檢查。
   * 其餘 8 顆 commit（MT-E1、MT-E1b、MT-E2、MT-E3a、LAT-1、MT-E3c、MT-FIX1、MT-FIX1a）**一個 `#if 0` 都沒拿掉**（見 §2 的指令與結果）。
2. **違反了 W2 的規則。** 週末計畫 W2 寫的是「一顆 commit 一個閘；解閘前重跑那個閘的前提」
   （`docs/WEEKEND_PLAN_20260925.md:285`）。這台機把 19 個閘擠在 2 顆 commit 裡開。
   commit 歷史不改（規則也不准改），本文件是**事後補做**的「前提重驗指令與結果」。
3. **重驗結果：19 個閘原本的前提（「有宣告、沒有定義」「沒有 port」「連結會失敗」）全部確實已經過期。**
   證據是 `nm` 對 `build_integ_ship_x86` 的目的檔與已連結的 `wb_serve.exe`：每一個原本缺的符號現在都是 `T`（已定義）。
   目的檔比原始碼新（§1），所以量到的就是 HEAD 的內容。
4. **但「前提過期」不等於「在真機上沒問題」。** 有幾件事只有機邊能驗，最要緊的四件：
   * G31a：wb_serve 開起來約 100 拍後會**自己**把馬達電源繼電器 `SwMotorRelay` 吸合（沒人按鍵）。
   * G04：HT9050 的四顆 EMG 感測器與 `SnServo` 在 IO 表上都是 `Enable=0`，所以「沒動力」在這台實際上**只看 `SnMotorPower`**。
     按下 EMG 如果不會讓 `SnMotorPower` 掉，G04 的鎖就永遠不會因為 EMG 觸發。
   * **8929d13 間接改變了 IO 頁**：`JsonBridge/IoBtnPanelClick.cpp:345` 拒絕放開 Index Z 煞車的條件只看
     `IsIndexMotorOutOfPower()`。以前在這台恆為 true（放不掉），現在有馬達電、沒 EMG 就放得掉，
     而且這條路**沒有經過** `W906_BrakeReleaseOK` 的 Servo-ON 檢查。它的註解（`:44`、`:48-50`）已過期。
   * 煞車分組對應哪幾軸（Index→M14、InOutArmZ→M03/M22、Cassette→M35/M38/M39）程式裡自己寫著「待 EastSun 機邊確認」。
5. **g2 G03（`csystem.cpp:15911`）：稽核員說得對，它寫的兩個理由都過期了。**
   但這一段在 HT9050 上根本走不到（它在 Galil Index 那一臂，而且要 Galil 回報的警報燈），
   所以**開或不開，這台機的行為都一樣**。可以照 W2 規則單獨一顆 commit 照 golden 開；對 HT9050 是零差異。詳見 §7。
6. **仍關著、但理由提到的相依現在已存在的候選**有 10 個，最要緊的是 **h4-G4（`csystem.cpp:28834`）**：
   它自己的註解寫「必須和讓 `iNozzleEvent` 變真的那一波一起開」——那一波（A4-6，20260924）已經落地，閘卻沒開，
   所以**今天回原點不會等吸嘴吹氣完成**。詳見 §8。這裡只列候選，**一個都沒開**。

---

## 1. 環境與方法

| 項目 | 值 |
|---|---|
| C++ 樹 | `D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`，HEAD `7961939` |
| 工作樹 | `git diff --stat HEAD -- csystem.cpp uhome.cpp cinitial.cpp csystem.h database.cpp cSocket.cpp` → 空輸出、exit 0 |
| 目的檔 | `build_integ_ship_x86`（WinLibs i686 g++ 16.2.0，`-std=c++14`，出貨組態） |
| 新舊 | `csystem.cpp.obj` 09-26 00:49:32 > `csystem.cpp` 00:47:20；`uhome.cpp.obj` 00:49:29 > `uhome.cpp` 00:47:38；`wb_serve.exe` 02:13:46 > 8929d13 的 commit 時間 02:12:43；`tests\test_mt_e3b_engine.exe` 01:27:50 > 其原始碼 00:49:37 |
| nm | `%LOCALAPPDATA%\Programs\ht9045-nonoracle-toolchain\mingw32\bin\nm.exe`（`CMakeCache.txt` 的 `CMAKE_NM`） |
| golden | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\`，用 Big5（code page 950）**唯讀**讀取；本文件引用的每個 golden 行號都逐行比對過原文 |
| 機台檔（唯讀） | `D:\HT9045\system\Mot_Table.csv`（09-25 16:30:17）、`IO_Table.csv`（09-25 11:22:55）、`Gerneral.ini` |

**沒有做的事**：沒有跑 ctest（測試證據引用 20260926 02:24:58 在 `build_integ_ship_x86` 跑的完整 ctest 紀錄，見 §10）；
沒有執行任何會開卡的程式。唯一執行的外部腳本是兩支**只讀原始碼**的靜態檢查
`tools\pci1203_readonly_gate.ps1`、`tools\pci1203_control_gate.ps1`（§9 末）。

---

## 2. 找出被打開的閘

指令（對每顆 commit 找「被刪掉的前置處理指令」與「新增的 `#if 0`／`#endif`」）：

```powershell
foreach ($c in '4b21dbe','9fd97ff','2c1afab','b90b0ca','eef1a78','0d253a0','5a0f02d','5dc6706','8929d13','7961939') {
  git show --format='%h %s' -U3 $c | Select-String '^-\s*#\s*(if|ifdef|ifndef|endif|else|elif)\b|^\+\s*#\s*(if 0|endif)'
}
# 另外查有沒有用別的形式閘（布林旗標、if(false)）：
git diff 95c398b 7961939 -U0 -- '*.cpp' '*.h' | Select-String '^[-+].*(ENABLED\s*=\s*(true|false)|if\s*\(\s*false\s*\)|if\s*\(\s*0\s*\))'
```

結果：

* `0d253a0`：刪掉 G9、G29、G31b、G31c、G07、G10、G13、G21、g1 G03、g1 G14、g1 G16、
  `SAFETY-GATE(W906-T6-VERIFYMOT)`、`SAFETY-GATE(W906-T6-ABORTHOME)`、`GATE (W906-HOME-C2-GALIHOMETASK)` 共 14 組；
  新增 1 組 `GATE (W906-MT-E3b-ABORTBTN)`。
* `8929d13`：刪掉 G31a、G04、G34、G05；G15 改成只閘後半（`GATE G15 (second half)`）。
* 其餘 8 顆：只有新程式碼自己的 `+#endif`（配 `#ifdef`／`#ifndef`），沒有任何 `-#if 0`。旗標形式的搜尋只命中 Light Scale 的 UI 欄位，不是閘。

**編號注意**：csystem.cpp 有兩套編號。`g1` 群（翻 golden 1..3042 那段）和 `g2` 群（DoSystem 一帶）
各有自己的 G03、G14、G15、G16、G17、G18、G19、G22……。本文件一律寫明是哪一群。
「稽核員說過期的 g2 G03」是 `ScanAllMotorStatus` 那一個，不是煞車那一個。

---

## 3. 共同的重驗指令與結果

```powershell
$nm = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin\nm.exe"
$o  = "build_integ_ship_x86\CMakeFiles\ht9045_sm.dir"
& $nm -C --defined-only "$o\csystem.cpp.obj" | Select-String 'Breaker(ON|OFF|On)\(|IsEMGPressed\(|CountMotorPowerDelay\(|IsIndexMotorOutOfPower\(|LockIndexMotorAndDoHomeProcess\(|DoMotorPowerOn\(|VerifyMotorAction\(|iEMGPressDelay|W906_BrakeReleaseOK|W906_BrakeServoOnHook|IsSystemPowerOff\('
& $nm -C --defined-only   "$o\uhome.cpp.obj"   | Select-String 'GaliMotorServoOff|InitGali_HomeTask|sbAbortHomeClick'
& $nm -C --undefined-only "$o\csystem.cpp.obj" | Select-String 'TfHome'
& $nm -C "build_integ_ship_x86\wb_serve.exe"   | Select-String ' (T|D|B) (W906_BrakeReleaseOK|TfHome::|iEMGPressDelay$|VerifyMotorAction|IsIndexMotorOutOfPower|CountMotorPowerDelay)'
```

實際輸出（節錄，20260926 量）：

```
csystem.cpp.obj:  T IndexMotorBreakerOFF()        T IndexMotorBreakerON()
                  T MagazineBreakerOFF()          T MagazineBreakerON()
                  T InOutArmZBreakerOFF()         T InOutArmZBreakerON()
                  T LDCarRotArmZBreakerOFF()      T LDCarRotArmZBreakerOn()
                  T CassetteBreakerOFF()          T CassetteBreakerON()
                  T IsEMGPressed()  T IsSystemPowerOff()  T IsIndexMotorOutOfPower()
                  T LockIndexMotorAndDoHomeProcess()  T CountMotorPowerDelay()
                  T DoMotorPowerOn()  T VerifyMotorAction()
                  T W906_BrakeReleaseOK(char const*, char const*)
                  D iEMGPressDelay          B W906_BrakeServoOnHook
uhome.cpp.obj:    T TfHome::InitGali_HomeTask()
                  T TfHome::GaliMotorServoOff(vclcompat::AnsiString)
                  T TfHome::sbAbortHomeClick(void*)
csystem.cpp.obj (undefined, 由 uhome.cpp.obj 滿足):
                  U TfHome::sbAbortHomeClick(void*)   U TfHome::GaliMotorServoOff(vclcompat::AnsiString)
wb_serve.exe:     T VerifyMotorAction()  T W906_BrakeReleaseOK(...)  T CountMotorPowerDelay()
                  T IsIndexMotorOutOfPower()  T W906_DoMotorPowerOnBegin()
                  T TfHome::sbAbortHomeClick(void*)  T TfHome::GaliMotorServoOff(...)  T TfHome::InitGali_HomeTask()
                  D iEMGPressDelay  B W906_BrakeServoOnHook
```

另外用字串確認打開的區塊真的編進了出貨執行檔（這些字串只出現在被打開的區塊裡）：
`wb_serve.exe` 含 `DoSystem G05`、`CountMotorPowerDelay G16`、`HOME start`、`brake release %s: %s (%s)%s%s`、
`VerifyMotorAction: 2 s without motion`、`GaliMotorServoOff - ` → 全部 True。
`tests\test_mt_e3b_engine.exe` 含 `PART H: brake-release servo-ON guard` → True。

golden 的煞車函式本體位置（Big5 唯讀比對）：`IndexMotorBreakerON` :1076、`IndexMotorBreakerOFF` :1082、
`IsEMGPressed` :1416、`IsIndexMotorOutOfPower` :1501、`LockIndexMotorAndDoHomeProcess` :1514、`CountMotorPowerDelay` :1543、
`MagazineBreakerON/OFF` :24083/:24088、`InOutArmZBreakerON/OFF` :24093/:24099、`LDCarRotArmZBreakerOn/OFF` :24105/:24110、
`CassetteBreakerON/OFF` :24115/:24122、`DoMotorPowerOn` :19102。移植樹：`csystem.cpp:19097`／`:19103`／`:19723`／`:19903`／`:19923`／`:19979`、
`:29390`～`:29429`、`:14381`。

---

## 4. 逐閘紀錄：`0d253a0`（MT-E3b，20260925）

**這一整批的授權**（引自程式註解）：

* **R1**（EastSun 20260925）：「the station-16 brake outputs are ON = release, same as golden, so the whole gated brake family goes back to golden」（`csystem.cpp:14399-14401`）。
  也就是 `*BreakerOFF` = 抓住煞車、`*BreakerON` = 放開煞車。舊註解有幾處把方向寫反，這次一起更正。
* **R2**：「M14 MTestZ1 uses the 1203 path like the other PCI1203 rows; do NOT edit Gerneral.ini (INDEX_MOTION_CARD stays 0) -- a Mot_Table row whose CardModel is PCI1203 is a 1203 axis」（`database.cpp` R2 註解，實作 `database.cpp:2576-2579`）。
* **R8**：「VerifyMotorAction = COMPLETELY golden ... AND golden MainProc pauses while Motor Test / Teach is open」（`csystem.cpp:29966` 起的說明）。
* **R9**：操作員的 Motor Power 按鈕「must not be blocked」（`csystem.cpp:16139`）。
* **R10**：machinerecord.dat 不還原、測試寫暫存副本（不是閘，是測試圍堵）。

### 4.1 G9 — `DoMotorPowerOn` 送電時抓住煞車

* **位置**：`csystem.cpp:14402-14404`（註解 `:14395`），函式 `DoMotorPowerOn` `:14381`。
* **golden**：`csystem.cpp:19116-19118`（`IndexMotorBreakerOFF(); MagazineBreakerOFF(); CassetteBreakerOFF();`）。
* **原本的前提**：三個 `*BreakerOFF` 有宣告（`csystem.h:143/:145/:317`）但全樹沒有定義，用 `nm --undefined-only` 證明過。
  （舊註解還把方向寫反，說成「煞車沒放開」。）
* **重驗**：§3 → 三個都是 `T`。**過期。**
* **授權**：R1。
* **測試**：`DoMotorPowerOn` 本身**沒有直接測試**。它的跨拍複本 `W906_DoMotorPowerOnIteration`（`csystem.cpp:14428`）由 `MT_E3b_Engine` PART F 測（送電後 `SwInArmZBreaker` 被抓住、約 1 秒結束）。
* **機邊待驗**：Motor Power ON 時，繼電器吸合的同時站 16 的煞車輸出要變 OFF（抓住）。呼叫者有 HOME（uhome.cpp）、`forms/fMain.cpp`、G31a、G31b。

### 4.2 g1 G03 — `IndexMotorBreakerOFF` 尾端的四個輔助煞車

* **位置**：`csystem.cpp:19148-19151`（註解 `:19142`）。
* **golden**：`csystem.cpp:1103-1106`。
* **原本的前提**：`MagazineBreakerOFF`／`InOutArmZBreakerOFF`／`LDCarRotArmZBreakerOFF`／`CassetteBreakerOFF` 在 csystem 第二波（golden :24088-:24122），當時沒有本體。
* **重驗**：§3 → 四個都是 `T`（移植樹 `csystem.cpp:29395`／`:29406`／`:29417`／`:29429`）。**過期。**
* **授權**：R1。
* **測試**：`MT_E3b_Engine` PART F 間接測到：`W906_DoMotorPowerOnIteration` 只呼叫 Index／Magazine／Cassette 三個，
  而斷言 `SwInArmZBreaker` 被抓住 —— 那只能來自這段尾巴裡的 `InOutArmZBreakerOFF()`。
* **機邊待驗**：從此每一個呼叫 `IndexMotorBreakerOFF()` 的地方（`LockIndexMotorAndDoHomeProcess`、`IsEMGPressed`、G04…）都會一起抓住手臂 Z 與 Cassette 的煞車。
  HT9050 會真的寫到的輸出：`SwInArmZBreaker`（站 16 ch5）、`SwOutArmZBreaker`（ch6）、`SwCassetteLD/Auto1/Auto2MotBreaker`（ch24/26/27），都是 `Enable=1`；
  `SwMagazineMotorBreaker` 是 `Enable=0`，`SwLoadCarRFIDZBreaker` 表上沒有這一列。

### 4.3 g1 G14 — `IsEMGPressed` 按下 EMG 時抓住輔助煞車

* **位置**：`csystem.cpp:19831-19834`（註解 `:19827`）。
* **golden**：`csystem.cpp:1445-1448`。
* **原本的前提**：同 g1 G03，四個輔助煞車沒有本體。
* **重驗**：§3 → 過期。
* **授權**：R1。
* **測試**：**沒有直接測試**（沒有測試真的觸發 EMG 分支）。
* **機邊待驗**：HT9050 的四顆 EMG 感測器 `SnFrontLeftEMG`／`SnFrontRightEMG`／`SnRearLeftEMG`／`SnRearRightEMG` 與 `SnServo` 在
  `D:\HT9045\system\IO_Table.csv` 都是 `Enable=0`，`Gerneral.ini:216 SafePlcIO=0`，所以這一臂在這台**實際上不會進來**。
  EMG 在 HT9050 上要靠「切斷馬達電 → `SnMotorPower` 掉」由 G04 接手（見 4.18）。

### 4.4 g1 G16 — `CountMotorPowerDelay` 倒數完放開輔助煞車

* **位置**：`csystem.cpp:20024-20027`（註解 `:20017`；8929d13 又在 `:19996-19999` 加了 Index 的檢查）。
* **golden**：`csystem.cpp:1561-1564`（前一行 :1560 是 `IndexMotorBreakerON()`）。
* **原本的前提**：四個 `*BreakerON` 沒有本體。
* **重驗**：§3 → 過期。
* **授權**：R1；8929d13 再依 EastSun 20260926「對應軸 Servo On 才放」加上 `W906_BrakeReleaseOK`（見 4.21）。
* **測試**：`MT_E3b_Engine` PART H：倒數歸零時，假 hook 說 InOutArmZ 沒激磁 → `SwInArmZBreaker`／`SwOutArmZBreaker` 維持抓住；Cassette 激磁 → `SwCassetteLDMotBreaker` 放開。
* **機邊待驗**：只有 G21 會呼叫 `CountMotorPowerDelay`（4.8）。見 4.21 的 Servo-ON 對應表。

### 4.5 g2 G07 — 系統斷電時抓住煞車

* **位置**：`csystem.cpp:16411-16415`（註解 `:16409`）。
* **golden**：`csystem.cpp:4333-4337`。
* **原本的前提**：五個 `*BreakerOFF` 只有宣告。
* **重驗**：§3 → 過期。
* **授權**：R1。
* **測試**：**沒有直接測試。**
* **機邊待驗**：HT9050 的 `SnSystemPower` 在 IO 表是 `Enable=0`，這一臂在這台**進不來**，機邊無從驗。

### 4.6 g2 G10 — 馬達斷電（運轉中）時抓住煞車

* **位置**：`csystem.cpp:16463-16467`（註解 `:16459`）。
* **golden**：`csystem.cpp:4369-4373`。
* **原本的前提**：同 G07。
* **重驗**：§3 → 過期。
* **授權**：R1。
* **測試**：**沒有直接測試。**
* **機邊待驗**：運轉中（`SystemStart`）拔掉馬達電 → 煞車輸出要變 OFF。注意 `:16461-16462` 的註解「閒置時的斷電是 G04 的事，而 G04 還閘著」已經過期（G04 在 8929d13 打開了）。

### 4.7 g2 G13 — DoSystem 的 EMG 條件補回 `IsEMGPressed() ||`

* **位置**：`csystem.cpp:16538`（註解 `:16534`）。
* **golden**：`csystem.cpp:4414`。
* **原本的前提**：`IsEMGPressed` 只有宣告（`csystem.h:147`），本體在另一群還沒落地。
* **重驗**：§3 → `T IsEMGPressed()`。**過期。**
* **授權**：R1（EMG 那一半的煞車族）。
* **測試**：**沒有直接測試。**
* **機邊待驗**：因為 HT9050 的 EMG 感測器全是 `Enable=0`，這一臂在這台只會經由 `bIsIndexMotorOutOfPower` 進來，
  畫面會顯示 golden 最後一個分支的訊息「EMG/Servo 訊號異常，請檢查 IO 模組」，**不會**是 WAR1630～1633。
  `:16536-16537` 的註解「G04 閘著所以 bIsIndexMotorOutOfPower 恆為 false」已過期。

### 4.8 g2 G21 — 伺服電源穩定前不准 START

* **位置**：`csystem.cpp:16830-16836`（註解 `:16821`）。
* **golden**：`csystem.cpp:4649-4655`。
* **原本的前提**：`CountMotorPowerDelay()` 沒有宣告也沒有定義。
* **重驗**：§3 → `T CountMotorPowerDelay()`，`csystem.h` 有宣告。**過期。**
* **授權**：R1（它是 G16 唯一的觸發點，不開它 G16 就是死碼）。
* **測試**：`W6_6_Hub` 間接：它的 PART C 因為 G21 打開後會把 `SystemStart` 拉回 false，所以改成重新設定守衛（`tests/test_w6_6_hub.cpp:224-230`）。沒有斷言 G21 本身。
* **機邊待驗**：`SnMotorPower`（ring1 站 2 ch30，`Enable=1`）亮滿 `MotorPowerOnDelay` 秒（開機 10、斷電後 4）之前按 START 會被拉回停機。

### 4.9 g2 G29 — 面板 [Power Off] 鍵呼叫 `GaliMotorServoOff`

* **位置**：`csystem.cpp:16118-16121`（註解 `:16114`）。
* **golden**：`csystem.cpp:4146-4149`。
* **原本的前提**：`TfHome::GaliMotorServoOff` 不是移植樹 TfHome 的成員；而且 `W7C2_FHOME_SERVOOFF` 也是空巨集，要一起解。
* **重驗**：§3 → `T TfHome::GaliMotorServoOff(vclcompat::AnsiString)` 在 `uhome.cpp.obj`（本體 `uhome.cpp:4984`，golden `uhome.cpp:4988-5016`）；
  `csystem.cpp.obj` 有 `U TfHome::GaliMotorServoOff`（真的會呼叫）。**過期。**
* **授權**：R1＋閘自己寫的解閘條件（「TfHome 長出這個方法時與 W7C2_FHOME_SERVOOFF 一起解」）。
* **測試**：`GaliMotorServoOff` 本體由 `MT_E3b_Engine` PART B 測（煞車抓住、繼電器切掉、`InitGali_HomeTask` 的旗標、不清 HomeFlag）。這個呼叫點本身沒有測試。
* **機邊待驗**：HT9050 的 `SnFKPowerOff`／`SnRKPowerOff` 是 `Enable=0`，這一臂在這台進不來。

### 4.10 g2 G31b — 面板 [Power On] 鍵跑 `DoMotorPowerOn`

* **位置**：`csystem.cpp:16142`（註解 `:16137`）。
* **golden**：`csystem.cpp:4154`。
* **原本的前提**：先是「`DoMotorPowerOn` 沒定義」（20260810 已更正），後來剩「G9 還閘著」。
* **重驗**：§3 → `T DoMotorPowerOn()`，G9 已開（4.1）。**過期。**
* **授權**：R1、R9。
* **測試**：**沒有直接測試。**
* **機邊待驗**：`SnFKPowerOn`／`SnRKPowerOn` 是 `Enable=0`，這台進不來。注意它是 golden 的 1 秒忙等，會卡住整個 tick 執行緒。

### 4.11 g2 G31c — [Power On] 時先抓住五組煞車

* **位置**：`csystem.cpp:16153-16157`（註解 `:16150`）。
* **golden**：`csystem.cpp:4158-4162`。
* **原本的前提**：五個 `*BreakerOFF` 只有宣告。
* **重驗**：§3 → 過期。
* **授權**：R1。
* **測試**：**沒有直接測試。**
* **機邊待驗**：同 G31b，這台進不來。

### 4.12 `SAFETY-GATE(W906-T6-VERIFYMOT)` — Motor Test／Teach 開著時的馬達動作鎖

* **位置**：呼叫 `csystem.cpp:30472-30475`（註解 `:30469`）；本體 `VerifyMotorAction` `csystem.cpp:30050`。
* **golden**：呼叫 `csystem.cpp:16963`；本體 `uteach.cpp:5345-5426`（`MoveDelay` :5345、函式 :5346）。
* **原本的前提**：`VerifyMotorAction()` 在 `forms/fTeach.h:830` 刻意只宣告不定義，20260923 實測 `undefined reference`。
* **重驗**：§3 → `T VerifyMotorAction()`（`csystem.cpp.obj` 與 `wb_serve.exe`）。**過期。**
* **授權**：R8。
* **測試**：`MT_E3b_Engine` PART E（動作中上鎖、停 2 秒後 `StopAllMotor`＋1203 停止＋AllBtnUp、`*Lock by Homeing`、停用的馬達不上鎖）；PART D2（頁面開著時 MainProc 暫停）。
  測試自己寫明**沒測**門開的那一臂。
* **機邊待驗**：
  * 門的判斷來自 `CheckSafeDoorIsClosed()`。週末計畫記載 HT9050 表上只有 `SnSafeDoor1` 是真的 1203 點（站 2 ch25），
    其他門點是舊 HT9045 位址（`docs/WEEKEND_PLAN_20260925.md:293-296`）。要在機邊確認門讀值：
    讀成「開」的話，Motor Test 一動就會被這裡停掉；讀成「關」而門真的開了，則不會停。
  * 一個 LoopMove 的等待超過 2 秒（3／6 秒選項）會被 golden 的 2 秒閒置停機結束 —— 這是 golden 行為，不是移植錯。

### 4.13 `SAFETY-GATE(W906-T6-ABORTHOME)` — 回原點畫面開著時停機就中止回原點

* **位置**：`csystem.cpp:32673-32674`（註解 `:32667`）；本體 `TfHome::sbAbortHomeClick` `uhome.cpp:5027`。
* **golden**：`csystem.cpp:18923-18924`；`uhome.cpp:4980-4986`。
* **原本的前提**：`sbAbortHomeClick`／`GaliMotorServoOff` 沒翻進 TfHome。
* **重驗**：§3 → `T TfHome::sbAbortHomeClick(void*)`，`csystem.cpp.obj` 有 `U`。**過期。**
* **授權**：閘自己寫的解法（「把 sbAbortHomeClick／GaliMotorServoOff 翻進 TfHome」）；註解裡沒有單獨的裁決引文。
* **測試**：`MT_E3b_Engine` PART C（本體）、PART D1（MainProc 停機臂真的觸發）。
* **機邊待驗**：回原點途中停機 → 停所有馬達（含 1203 監看器的軸）、切 `SwMotorRelay`／`SwServerON`、抓住全部煞車。**之後要重新按 Motor Power**（golden 行為）。
* **同時新關了一個閘**：`GATE (W906-MT-E3b-ABORTBTN)`，`uhome.cpp:5031-5033`，golden `uhome.cpp:4984` `sbAbortHome->Down=false;`。
  理由：TfHome 沒有 `sbAbortHome` 這個按鈕元件（它在網頁 Home Monitor 上，而且不是切換鍵）。前提成立（全樹沒有這個成員）。

### 4.14 `GATE (W906-HOME-C2-GALIHOMETASK)` — 回原點時呼叫 `InitGali_HomeTask`

* **位置**：`uhome.cpp:2233`（註解 `:2219`）；本體 `TfHome::InitGali_HomeTask` `uhome.cpp:688`。
* **golden**：呼叫 `uhome.cpp:2453`；本體 `uhome.cpp:646-662`。
* **原本的前提**：(1) `TfHome::InitGali_HomeTask()` 沒有翻；(2) Galil 的 609 個重複符號樁還在，真本體進不了同一個連結。
* **重驗**：§3 → `T TfHome::InitGali_HomeTask()`；(2) `Motor/mymotor.cpp:1601` `#if 0 // GALI-STUB RETIRED (W906-P0-5)`。**兩個都過期。**
  本體只寫記憶體旗標，不下 Galil 命令。
* **授權**：閘自己的解閘條件；註解裡沒有單獨的裁決引文。
* **測試**：`MT_E3b_Engine` PART B 斷言它寫的旗標（經由 `GaliMotorServoOff`）；`ProcessMotorHome` 這個呼叫點沒有直接測試。
* **機邊待驗**：無硬體輸出，不需要。

### 4.15 （非 `#if 0`）`W7C2_FHOME_SERVOOFF` 空巨集退役

* **位置**：`csystem.cpp:4072-4073`（註解 `:4067`）；呼叫點 `csystem.cpp:5277`（`DoOneCycleFinishCheck`，`IniConfig.bOneCycleNeedPowerOff`）。
* **golden**：`csystem.cpp:13840` `fHome->GaliMotorServoOff("DoOneCycleFinishCheck");`。
* **原本的前提**：`fHome->GaliMotorServoOff` 不存在，所以用 `do { (void)(f); } while(0)` 代替。
* **重驗**：同 4.9。**過期。**
* **授權**：G29 閘自己寫「要與 W7C2_FHOME_SERVOOFF 一起解」。
* **測試**：這個呼叫點**沒有測試**。
* **機邊待驗**：只有 `bOneCycleNeedPowerOff` 開著、One Cycle 結束按 SKIP／HOME 時才會切馬達電。

### 4.16 （非 `#if 0`）MainProc 的「Motor Test／Teach 開著」判斷改讀瀏覽器

* **位置**：`csystem.cpp:30451-30452`（註解 `:30448`）；`W906_FormFShow`（`csystem.h`）。
* **golden**：`csystem.cpp:16952` `if(fMotorTest->fShow || fTeach->fShow)`。
* **原本的前提**：fShow 在新架構由瀏覽器持有，C++ 這邊恆為 false，這一整臂走不到（「設計題，不補樁」）。
* **重驗**：`W906_FormFShowHook` 由 `WebMotorAccessLive.cpp:1015` `W906_MotorAccessEngineHooks()` 註冊（wb_serve 才有；ctest 沒有 → 與舊行為相同）。
* **授權**：R8。
* **測試**：`MT_E3b_Engine` PART D2。
* **機邊待驗**：**只要瀏覽器的視窗總表說 Motor Test 或 Teach 開著，MainProc 就整個暫停**（golden）。
  5dc6706（MT-FIX1）修了「重新整理後總表卡在開著」—— 機邊要確認關頁、重新整理、斷線後引擎會恢復。

### 4.17 （不是閘，但是 G04 的前提）`database.cpp` R2 偏離

* **位置**：`database.cpp:2576-2579`。golden `database.cpp:2353-2363` 在 `INDEX_MOTION_CARD==0` 時把四個 Index 列強制改成 SMC。
* **改動**：自己那一格寫 `PCI1203` 的 Index 列（HT9050 的 M14 MTestZ1）照原樣讀。
* **重驗**：`D:\HT9045\system\Mot_Table.csv` 的 M14：`MTestZ1, BoardID 14, Port 0, Enable 1, CardModel PCI1203`；
  `Gerneral.ini:165 INDEX_MOTION_CARD=0`。`cinitial.cpp:3916`／`:3923` 把這一格抄進 `MOT[MTestZ1].CardType`。
* **測試**：`MT_E3b_Engine` PART G。
* 注意：機台的 Mot_Table 與 repo 的 `machines/HT9050/Mot_Table.csv` **不同**（MD5 `BC2E4C69…` vs `79CCD044…`；M14 的 Direction 機台是 0、repo 是 1）。ctest 餵的是 repo 那份。

---

## 5. 逐閘紀錄：`8929d13`（MT-FIX1b，20260926）

**這一批的授權**（引自 commit 訊息與程式註解，EastSun 20260926）：

* **G16**：「對應軸 Servo On 才放」—— golden 的每一個 `*BreakerON()` 放開都要先過 `W906_BrakeReleaseOK(group, site)`。
* **G04／G05**：HT9050 上「沒動力」＝ `SnMotorPower` 沒亮或 EMG。
* **G31a**：「照舊版打開」。
* commit 訊息最後一句：「Not yet run on the machine.」

### 5.1 g2 G31a — 開機約 100 拍後自動送馬達電

* **位置**：`csystem.cpp:16068`（註解 `:16063`；之前 MT-E3b 留的「為什麼還不開」在 `:16049-16062`）。
* **golden**：`csystem.cpp:4116`（`if(flag && DelayMotNo>=100)` 之內）。
* **原本的前提**：先是「`DoMotorPowerOn` 沒定義」，後來「G9 還閘著」，MT-E3b 時改成新理由：
  沒人按鍵就送電，而且 G21＋G16 打開後 4 秒會放開手臂 Z／Cassette 煞車，這棵樹不保證那時驅動器已激磁。
* **重驗**：`T DoMotorPowerOn()`、G9 已開；「沒激磁就放煞車」由同日的 `W906_BrakeReleaseOK` 擋住（`T W906_BrakeReleaseOK`，`B W906_BrakeServoOnHook`，註冊於 `WebMotorAccessLive.cpp:1024`）。
* **授權**：EastSun 20260926「照舊版打開」。
* **測試**：**沒有直接測試。**
* **機邊待驗**（重要）：
  * wb_serve 開起來後，`CheckMotorPowerShutDown` 被呼叫約 100 次時會**自己**吸合 `SwMotorRelay`（ring1 站 1 ch29，`Enable=1`），只做一次（`flag`）。
  * 那一下是 golden 的 1 秒忙等，**tick 執行緒（含 1203 Poll、網頁輸出佇列）會停 1 秒**。
  * 之後 console 應該印出 `brake release HELD: ...`（沒激磁的群組），直到操作員讓那些軸 Servo ON。

### 5.2 g2 G04 — 沒動力時鎖 Index、抓煞車、強制回原點

* **位置**：`csystem.cpp:16315-16330`（註解 `:16311`；MT-E3b 的「為什麼還不開」在 `:16294-16310`）。判斷式 `IsIndexMotorOutOfPower` `csystem.cpp:19903`，HT9050 臂 `:19912-19913`。
* **golden**：`csystem.cpp:4279-4294`（含 :4294 的 `else`）；判斷式 golden `:1501`。
* **原本的前提**：(1) 20260809：五個符號只有宣告；(2) MT-E3b：符號都在了，但判斷式在 HT9050 **永遠為 true**
  （它看 Galil 掃描才會寫的 `MOT[MTestZ1/Z2].Led[iServoOn]`，這台沒 Galil 卡、M15 也沒裝），照原文打開會每一拍都把機台鎖住。
* **重驗**：(1) §3 → 全部 `T`，過期。(2) 8929d13 在 `csystem.cpp:19912-19913` 加了
  `if(MOT[MTestZ1].CardType == "PCI1203") return IsEMGPressed() || Sen[SnMotorPower].IsOff();`；
  `MOT[MTestZ1].CardType` 在這台是 `PCI1203`（4.17）→ 有馬達電且沒 EMG 時為 false。**過期。**
  非 1203 的 Index 設定仍走 golden 的判斷。
* **授權**：EastSun 20260926（沒動力＝`SnMotorPower` 沒亮或 EMG）。
* **測試**：`MT_E3b_Engine` PART H（兩臂：非 1203 列 → true；1203 列 → 等於 `IsEMGPressed()`）；
  `mainproc_guard` 的測試夾具改成「有電的機台」（`tests/test_mainproc_guard.cpp:93-99`），否則 G04 會把 `iHome` 設成 1、干擾它要測的守衛。
  G04 那一臂本身（鎖 Index）**沒有直接斷言**。
* **機邊待驗**（重要）：
  * EMG 感測器在這台全是 `Enable=0`、`SafePlcIO=0`，所以 `IsEMGPressed()` 實際上不會成立，**「沒動力」只看 `SnMotorPower`**。
    要在機邊確認按下 EMG 會讓 `SnMotorPower`（ring1 站 2 ch30）掉下來。
  * 馬達沒電時（例如開機到 G31a 送電之前）**每一拍**都會跑：`iHome=1`、`fAllMotorHome=false`、`StopAllMotor()`、
    `Cylinder[C_Auto_Selector[i]].Off()`、五組煞車抓住、`DoIndexZ1Z2Free(0)`。這是 golden 行為；機邊確認這些每拍輸出沒有副作用。
  * **IO 頁的連帶變化**：`JsonBridge/IoBtnPanelClick.cpp:345` 只在 `IsIndexMotorOutOfPower()` 時拒絕放開 `SwFMotorBreaker`（站 16 ch4，`Enable=1`）／`SwBMotorBreaker`。
    8929d13 之後，有馬達電、沒 EMG 時就**可以**從 IO 頁放開 Index Z 煞車，而這條路**不經過** Servo-ON 檢查。
    需要 EastSun 決定 IO 頁要不要也套 `W906_BrakeReleaseOK`。它的註解 `:44`（「G04 閘著」）與 `:48-50`（「恆為 true，網頁放不掉」）已過期。

### 5.3 g2 G34 — `iEMGPressDelay` 每秒倒數

* **位置**：`csystem.cpp:16345-16346`（註解 `:16343`）；全域 `int iEMGPressDelay=3;` `csystem.cpp:16266`。
* **golden**：`csystem.cpp:4299-4300`；全域 golden `:133`。
* **原本的前提**：`iEMGPressDelay` 全樹沒有宣告也沒有定義。
* **重驗**：§3 → `D iEMGPressDelay`（`csystem.cpp.obj` 與 `wb_serve.exe`）。**過期。**
* **授權**：隨 G04／G05 一起（EastSun 20260926）。
* **測試**：**沒有直接測試。**
* **機邊待驗**：無獨立輸出；它決定 G05 何時可以放煞車。

### 5.4 g2 G05 — 閒置時每拍放開煞車（現在要先過 Servo-ON）

* **位置**：`csystem.cpp:16375-16385`（註解 `:16369`）。
* **golden**：`csystem.cpp:4303-4313`（原文無條件放開）。
* **原本的前提**：(1) 五個 `*BreakerON` 只有宣告；(2) MT-E3b：它只在 G04 的 `else` 裡跑，G04 開不了它就不能開，
  單獨開會在馬達剛斷電、`SnMotorPower` 還沒掉的那幾拍放開煞車。
* **重驗**：(1) §3 過期；(2) G04 已開（5.2）；斷電那幾拍由 `W906_BrakeReleaseOK` 在 `bMotorPowerState==false` 時拒絕放開（`GaliMotorServoOff` 會立刻清掉它）。**過期。**
* **授權**：EastSun 20260926（G04／G05、G16）。**與 golden 不同**：每一組放開前多了 `W906_BrakeReleaseOK`。
* **測試**：`MT_E3b_Engine` PART H 測 `W906_BrakeReleaseOK` 本身（沒 hook＝照 golden；某組沒激磁＝不放；剛斷電＝不放）。G05 這一段本身沒有直接測試。
* **機邊待驗**：見 5.6。

### 5.5 g1 G15 前半 — `LockIndexMotorAndDoHomeProcess` 裡 `iEMGPressDelay=8;`

* **位置**：`csystem.cpp:19965`（註解 `:19962`）；後半 `fContact->InitCarlibrationTask();` 仍閘在 `:19966-19968`。
* **golden**：`csystem.cpp:1532`（後半 :1533）。
* **原本的前提**：`iEMGPressDelay` 與 `InitCarlibrationTask` 都沒有 port。
* **重驗**：前半 → `D iEMGPressDelay`，過期。後半 → 見 §8 B1（方法已存在，但 csystem 看到的 `fContact` 是替身，沒有這個方法）。
* **授權**：EastSun 20260926（隨 G04／G05）。
* **測試**：**沒有直接測試。**

### 5.6 （修改，不是開閘）三處 golden 煞車放開加上 Servo-ON 檢查

* **位置**：`CountMotorPowerDelay` `csystem.cpp:19999-20000`、`:20024-20027`；DoSystem G05 `csystem.cpp:16379-16383`；
  HOME 開始 `uhome.cpp:2135-2139`（golden `uhome.cpp:2369-2373`）。判斷 `W906_BrakeReleaseOK` `csystem.cpp:30013`；
  hook `W906_HookBrakeServoOn` `WebMotorAccessLive.cpp:991`，分組 `W906_BrakeGroupHasAlias` `:983`。
* **行為**：沒註冊 hook（ctest）→ 照 golden；wb_serve 下 → `bMotorPowerState==false` 或該組任一個啟用中的 PCI1203 軸沒有 SVON → 不放，下一個 G05 閒置拍再試。每組答案改變時印一行。
* **授權**：EastSun 20260926「對應軸 Servo On 才放」。
* **測試**：`MT_E3b_Engine` PART H。
* **機邊待驗**（重要）：
  * 分組對應是從 IO／Mot 表的名字推的，程式自己寫著「to be confirmed by EastSun on the machine」：
    Index → MTestZ1（M14）；InOutArmZ → MInArmZ\*／MOutArmZ\*（啟用中的是 M03 MInArmZA、M22 MOutArmZA）；
    Cassette → MLoaderZ（M35）、MAuto1Z（M38）、MAuto2Z（M39）；Magazine、LDCarRotArmZ 在這台沒有軸（一律回 true）。
  * SVON 位元來自 1203 監看器。`MachineType.h:159` 的 `WB_PUMP_1203_START_RING` 目前是關的（IOWEB-P9，20260924）。
    機邊要確認在這個設定下監看器讀得到各軸的 SVON；讀不到的話所有群組會一直「HELD」（安全方向，但 HOME 會走不動）。
  * 沒有任何程式會自己讓 1203 軸 Servo ON（golden 靠開機的 InitMotor）。實際順序要在機邊走一次：送電 → 讓軸 Servo ON → 看下一拍 `brake release ALLOWED`。

---

## 6. 開閘後留下的過期註解（沒有改，只記下來）

csystem.cpp 正由筆電端修改，依規定不動。等合併後再一起改：

| 位置 | 過期的內容 | 為什麼過期 |
|---|---|---|
| `csystem.cpp:14668-14678` | MT-E3b 的狀態總表寫「KEPT G04 G05 G34 … G31a」 | 8929d13 都開了 |
| `csystem.cpp:15907-15910` | g2 G03「判斷式在 HT9050 永遠為 true」 | 8929d13 改了判斷式（§7） |
| `csystem.cpp:16461-16462` | G10「閒置斷電是 G04 的事，G04 還閘著」 | G04 已開 |
| `csystem.cpp:16536-16537` | G13「G04 閘著，所以 bIsIndexMotorOutOfPower 恆為 false」 | G04 已開 |
| `csystem.cpp:16828-16829`、`:20021-20023` | G21／G16「golden 不看驅動器是否激磁，記錄給 EastSun，沒改」 | 8929d13 改了（`W906_BrakeReleaseOK`） |
| `JsonBridge/IoBtnPanelClick.cpp:44`、`:48-50` | 「G04 閘著」「IsIndexMotorOutOfPower 恆為 true，網頁放不掉 Index 煞車」 | 見 5.2，這是行為變更，不只是註解 |

---

## 7. g2 G03（`csystem.cpp:15911`）前提重驗

**閘的內容**：`ScanAllMotorStatus`（`csystem.cpp:15874`）的 Galil Index 警報分支，
golden `csystem.cpp:4007` 的條件是 `MOT[i].MovFlag && SystemStart==true && IsIndexMotorOutOfPower()==false`，
移植樹拿掉了最後一項（`:15914`）。golden 原文逐行比對一致（golden :4006-4007）。

**它寫過的兩個理由與重驗**：

| 理由 | 重驗指令 | 結果 | 判定 |
|---|---|---|---|
| 20260809：`IsIndexMotorOutOfPower` 只有宣告，呼叫會連結失敗 | `nm -C --defined-only csystem.cpp.obj \| Select-String IsIndexMotorOutOfPower`；`nm -C wb_serve.exe` 同 | `T IsIndexMotorOutOfPower()`（兩者都有） | **過期** |
| 20260925（MT-E3b）：判斷式在 HT9050 永遠為 true，打開會讓這個 Index 警報永遠不出現 | 讀 `csystem.cpp:19912-19913`；`Import-Csv D:\HT9045\system\Mot_Table.csv` 看 M14；讀 `cinitial.cpp:3916`／`:3923` | 8929d13 起，`MOT[MTestZ1].CardType=="PCI1203"`（機台 M14 就是 PCI1203）時判斷式＝`IsEMGPressed() \|\| Sen[SnMotorPower].IsOff()`，有電且沒 EMG 時為 false | **過期** |

**但這一段在 HT9050 上走不到，所以開不開都沒有可觀察的差異**：

1. 它在 `if(INDEX_MOTION_CARD==0 && (i==MTestY1||i==MTestZ1||i==MTestZ2||i==MTestY2))` 之內（`csystem.cpp:15881-15882`），
   這台 `Gerneral.ini:165 INDEX_MOTION_CARD=0`，所以會進來。
2. 但裡面要先 `MOT[i].Led[iAlarmLed]` 成立（`:15885`）。這個燈只有 `Gali_ScanMotStatusTIMO` 會寫（`Motor/myGALILmotor.cpp:1589`），
   而它只在 `bGali_CardInstall==true` 時被呼叫（`:1621-1622`、`:1642`）。
3. `bGali_CardInstall` 預設 false（`myGALILmotor.cpp:736`），只有 `Open_GaliCard()` 成功才設 true（`:4144`）；
   出貨組態下 `DMCGetControllerDesc` 是離線樁，固定回 `DMCERROR_CONTROLLER`（`Motor/vendor_offline_galil.cpp:75`），
   所以 `Open_GaliCard` 在 `:4102-4105` 就回 false，燈永遠不會亮。
4. → 整個警報分支（含這個閘）在 HT9050 上**一次都不會執行**。MT-E3b 那個理由即使在當時為真，對這台也是沒有作用的。

**結論**：前提**不成立**。它可以照 W2 規則「一顆 commit 一個閘」、照 golden 原文打開（把 `:15911-15915` 換回 golden :4007 那一行），
對 HT9050 是零行為差異；在有 Galil Index 的機種上則恢復 golden「沒動力時不顯示 Need home 訊息」的語意。
注意 `IsIndexMotorOutOfPower()` 會呼叫 `IsEMGPressed()`（有副作用：抓煞車、全軸 Servo OFF／ON），但 golden 也是這樣，而且短路求值只在前兩項成立時才會呼叫。
**不在這次動手**：csystem.cpp 正由筆電端修改。解的時候重跑上表兩個指令，並附上 `MT_E3b_Engine`／`mainproc_guard` 的結果。

---

## 8. 仍關著、但理由提到的相依現在已存在的候選（只列，不開）

**方法**：掃了 0d253a0／8929d13 動過的五個引擎檔的全部頂層 `#if 0`（`csystem.cpp` 193、`cinitial.cpp` 69、`uhome.cpp` 42、`database.cpp` 7、`cSocket.cpp` 2，共 313 個），
取出每個閘區塊裡呼叫的函式／成員／全域物件，對 `build_integ_ship_x86` 出貨函式庫（`libht9045_*.a`、`libvclcompat.a`、`wb_serve.dir\objects.a`）的 `nm --defined-only` 比對，
再逐一人工讀閘的註解。掃描腳本與結果留在 scratchpad（`gate_scan.ps1`、`gate_scan.csv`），不進 repo。

### A. 相依已存在、看不到別的阻擋（真正的候選）

| # | 閘 | golden | 原本的理由 | 現在的證據 | 這台（HT9050）會不會碰到 |
|---|---|---|---|---|---|
| A1 | **h4-G4** `csystem.cpp:28834`（`CheckNozzleEventFinish`） | csystem.cpp:23566-23595 | 兩套不同佈局的 `TMyKitSuck`；`mykitsuck.cpp` 沒註冊；沒有編譯中的程式會把 `iNozzleEvent` 設成 2。註解明寫「**THE GATE MUST BE OPENED IN THE SAME WAVE THAT LANDS iNozzleEvent**」 | A4-6（20260924）已統一：`aHotPlateSubstrate.h:97` 改 include `mykitsuck.h`、精簡鏡像在 `:105-647` 退役；`CMakeLists.txt:1888` 註冊 `mykitsuck.cpp`；`nm`：`T TMyKitSuck::CheckDestoryFinish()`（`mykitsuck.cpp.obj`，本體 `mykitsuck.cpp:2885`）；`mykitsuck.cpp:2415` `TMySucker::Destroy()` 寫 `iNozzleEvent=2`（不在任何 `#if` 內） | **會**。呼叫者 `uhome.cpp:978`（golden uhome.cpp:1326）是活的（`uhome.cpp.obj` 有 `U CheckNozzleEventFinish()`）。今天回原點**不會等吸嘴吹氣完成**就繼續。**安全相關，優先處理** |
| A2 | g1 G01 `csystem.cpp:18216`（`DoIonFanAutoClean`）與 h4-G2 `csystem.cpp:29181`（`DoAutoDecayCheck`） | :210-211／:23879-23883 | `MoveInArm2XYToDecayTeach` 全樹沒有本體 | `T MoveInArm2XYToDecayTeach()` 在 `ainarm2.cpp.obj`（本體 `ainarm2.cpp:925`） | 只在離子風扇自動清潔、ESD 自動衰減教導功能開著時。今天那兩個流程**永遠完成不了**（刻意的「大聲失敗」）。打開＝入料臂 XY 會動，要機邊驗 |
| A3 | g2 G11 `csystem.cpp:16494` | :4394 | `RecordSafeDoorStates()` 本體在另一群，還沒落地 | `T RecordSafeDoorStates()`（`csystem.cpp:21332`） | 會（每拍）。只是安全門開關的稽核紀錄，不是互鎖；打開前要確認它寫的檔在不在 production_audit 名單 |
| A4 | g2 G12 `csystem.cpp:16511`、g1 G23 `csystem.cpp:21105` | :4404-4407／:2669-2670 | `bCheckPLCConnet()` 沒有定義 | `T bCheckPLCConnet()`（`csystem.cpp:29761`） | 不會：`Gerneral.ini:216 SafePlcIO=0`（`Enable_PLCSafety_IO`），第一項就是 false |
| A5 | g2 G14 `csystem.cpp:16561` | :4430-4435 | `bCheckPLCAllSafedoorAndEMGEnable()` 沒有定義 | `T bCheckPLCAllSafedoorAndEMGEnable()`（`csystem.cpp:29793`） | 不會（同上）。在有安全 PLC 的機種上是安全關鍵 |
| A6 | g2 G19 `csystem.cpp:16728` | :4566-4605 | `CheckHotGun()`／`CheckHotGunFlow(int)` 沒有定義 | `T CheckHotGun()`、`T CheckHotGunFlow(int)`（`csystem.cpp:27686`／`:27708`） | 不會：`UseHotGunCheck=0`、`HotGunFlowEnable=0`、`UseHotGunFlowCheck=0` |
| A7 | G2 `csystem.cpp:13650`（`InitNewFixTray`） | :15934 | `SaveUnloaderInfo` 全樹沒有本體 | `T SaveUnloaderInfo(int)`（`cinitial.cpp.obj`，本體 `cinitial.cpp:5533`） | 看組態（ATK 的卸料資訊檔）。會寫檔；ctest 已有 `W906_UNLOADERINFO_ROOT` 圍堵 |
| A8 | `GATE (W906-HOME-W1-SPEEDCLOSE)` `csystem.cpp:31533` | （註解未標 golden 行號） | 20260923 已更正為「只缺全域實例 `fSpeed`」 | `B fSpeed`（`cSpeed.cpp.obj`）、`T TfSpeed::Close()` | 純畫面；`fShow` 由瀏覽器持有，打開前要決定讀哪個 fShow |

### B. 相依一半在，要先做決定（不是單純解閘）

| # | 閘 | 現況 |
|---|---|---|
| B1 | g1 G15 後半 `csystem.cpp:19966-19968`、g1 G17 `csystem.cpp:20040-20044`（`fContact->InitCarlibrationTask()`，golden :1533／:1568） | 方法已存在：`T TfContact::InitCarlibrationTask()`（`fContact.cpp.obj`）。但 csystem 看到的 `fContact` 是 `TfContactShim`（`atester_shims.h:251`），沒有這個方法；真正的表單實例叫 `fContactForm`（`forms/fContact.h:1634`，`B fContactForm`）。要先決定綁哪一個（和 FormHS／fSCKART 同一類設計題） |
| B2 | g2 G22 `csystem.cpp:16852`（golden :4657-4672） | (a) `ClearAllManualSuckTask()` 已存在（`mykitsuck.cpp.obj`，`mykitsuck.cpp` 已註冊）→ 這半過期；(b) `fMain->bStartKeyPressCheck` 仍 0 命中 → 這半成立。可以考慮拆成兩個閘 |
| B3 | g1 G22 `csystem.cpp:21025`（`CheckSafeDoorIsClosed` 的 0.5 秒門穩定窗，golden :2608-2624） | 理由「fTeach 仍是 NULL、沒有人 new」在 wb_serve 已不成立：`tools/wb_serve.cpp:2982` `fTeach = new TfTeach();`（20260924 匯入）。但 ctest 沒有 fTeach（原文是無保護的解參考），而且 `fShow` 由瀏覽器持有（現在有 `W906_FormFShow` 可用）。要決定怎麼改寫才不是照抄 |

### C. 理由的字面過期、但另一個阻擋仍成立（不是候選，只是註解要更新）

* g2 G18 `csystem.cpp:16698`（`fMonitor->OpenMonitorVedio`）：註解說「fMonitor 沒有 port」，現在有 `B fMonitor`、`T TfMonitor::OpenMonitorVedio(int)`；
  但那個方法本體整段在 `GATE M-4`（`forms/fMonitor.cpp:167-174` 的 `#if 0`），打開這裡只會換成無聲的空呼叫。g1 G27（`:21541`）在 20260916 已經這樣記過，G18 的註解還沒跟上。

### 查過、前提仍成立的（掃描誤報，列出以免重查）

* g1 G18／G19（`:20327`／`:20388`）：`iRecordTrayPickPosX/Y` 仍 0 命中。
* G3（`:13845`）`fMain->tESDError`、G30（`:16083`）`fMain->SitePanel`、G09（`:16439`）`HSys.BinDisCtrl`：仍缺。
* g2 G17（`:16678`）`fiosetview->ProcessIndexSuckDestroy1/2`：替身仍沒有這兩個成員。
* `SAFETY-GATE(W906-T6-*)` 一系列（`NOWSTATUS`、`SCCRTM`、`ATC30RESEND`、`ROTZERO`、`TEMPTAJOFS`、`OCREND`、`A37LOTINFO`、`GROUNDMAN`、`EQCQTY`、`UTACPPSEL`）：缺的都是表單鏡像上的欄位或同名衝突，仍成立。
* uhome.cpp 的 `W906-HOME-C2-*`（`TORQUE` 的 `iWriteAndCheckMotorTorque` 只存在於 atester.cpp 的 TU 內擴充類別、`VTESTPOS` 的 `rgHomeStopPos`、`AOISENDCMD` 的 `TFrmAOI::SendCommand`、`MEMOHOME`、`RTC*`、`PITCHX2` 等）：仍成立。
* cinitial.cpp 的 `n5-G*`、`n4-4`（`InitialTeachEditList`）、`n4-9`（`TfTeach::SaveFile` 不存在）：仍成立。

---

## 9. 機邊待驗總表

依重要性排序。全部都要**有人在機台旁**、依 EastSun 的程序做；本文件不做任何一項。

| # | 項目 | 來源 | 看什麼 |
|---|---|---|---|
| 1 | 開機自動送馬達電 | G31a（5.1） | wb_serve 啟動後多久 `SwMotorRelay` 吸合；那 1 秒網頁／1203 Poll 是否停住；console 的 `brake release HELD/ALLOWED` |
| 2 | 煞車分組與 Servo-ON 對應 | 5.6 | Index／InOutArmZ／Cassette 各對哪幾軸；沒激磁時煞車輸出維持 OFF；激磁後下一拍放開；`START_RING` 關的情況下 SVON 讀不讀得到 |
| 3 | IO 頁可以放開 Index Z 煞車 | 5.2 | 有馬達電、M14 沒激磁時，IO 頁按 `SwFMotorBreaker` 會不會真的放開 → 需要 EastSun 裁決要不要加 Servo-ON 檢查 |
| 4 | EMG 在 HT9050 的路徑 | G04、G13（5.2、4.7） | 按 EMG 時 `SnMotorPower` 會不會掉；G04 是否每拍鎖 Index；畫面訊息是「EMG/Servo 訊號異常」而不是 WAR1630～1633 |
| 5 | 馬達沒電時 G04 每拍的輸出 | 5.2 | `C_Auto_Selector` 氣缸 Off、五組煞車抓住、`DoIndexZ1Z2Free(0)` 每拍重寫是否有副作用 |
| 6 | 安全門讀值與 VerifyMotorAction | 4.12 | 門的實際讀值；Motor Test 動作時會不會被當成「門開」立刻停掉 |
| 7 | MainProc 暫停 | 4.16 | Motor Test／Teach 開著時引擎暫停；關頁、重新整理、斷線後會恢復 |
| 8 | 回原點中停機 | 4.13 | 停所有軸（含 1203）、切電、抓煞車；之後要重新按 Motor Power |
| 9 | 送電時煞車方向 | G9、g1 G03、R1（4.1、4.2） | 站 16 輸出 ON＝放開、OFF＝抓住，實物確認 |
| 10 | 伺服電源穩定前不准 START | G21（4.8） | `SnMotorPower` 亮滿 `MotorPowerOnDelay` 秒前按 START 會被拉回 |
| — | 這台進不來的臂 | G07、G29、G31b、G31c、g1 G14 | `SnSystemPower`、面板電源鍵、EMG 感測器都是 `Enable=0`；除非接線，否則機邊無從驗 |

**附帶的靜態檢查**（只讀原始碼，不碰卡）：

* `tools\pci1203_readonly_gate.ps1` → `RESULT: PASS`，exit 0。
* `tools\pci1203_control_gate.ps1` → exit 1，唯一的 FAIL 是
  `WB_PUMP_1203_START_RING is NOT active in MachineType.h`（`MachineType.h:159` 在 IOWEB-P9 20260924 刻意關掉）。
  `MachineType.h` 在 `95c398b..7961939` 沒有被改過，所以這不是本範圍造成的；是那支 gate 的期待（20260911 裁決）和 20260924 的設定不一致，要另外裁決。

---

## 10. 測試證據出處

* 20260926 02:24:58，`build_integ_ship_x86` 完整 ctest（8929d13 與 wb_serve.exe 建置之後）：
  `MT_E3b_Engine`、`mainproc_guard`、`W6_6_Hub`、`W6_6_CSystemCycle`、`homeclass`、`WB_SimPump`、`WB_State`、`MachineMotors_HT9050`、`WebMotorAccess` 全部 Passed。
  整體 166/176，失敗的 10 個是 `config_db`、`ini_helpers`、`config_loaders`、`ContactForceLoad`、`dfm2rc_rc_compiles`、`dfm2rc_fidelity`、`dfm2rc_idempotent`、`GA1_LastSet`、`GA1_ReadGeneralIni`、`MachineSuckers_HT9050` —— 沒有一個是本文件的閘相關測試。
  （其中 `ContactForceLoad`、`MachineSuckers_HT9050` 不在 CLAUDE.md 記的常駐清單裡，是否為回歸不在本文件範圍。）
* 8929d13 的 commit 訊息記載 `MT_E3b_Engine 79/0`、`production_audit 0 changes`。
* **本次沒有重跑任何測試**；只確認了測試執行檔比原始碼新、且含 PART H 的字串（§1、§3）。
