# FP ORACLE FINDINGS —— 把基準線用完的那一份紀錄

> `AI(W906-BU-X1/X2) 20260908` 建立。**使用者裁決五（20260907）**：基準線用完即丟，
> 只保留 golden 樹 ＋ `D:\ProgramFiles\Borland\CBuilder6`。
>
> **這份檔案存在的目的，就是讓 `C:\MinGW` 可以被刪掉。**
> 它把 oracle 線唯一無可取代的那個答案永久記下來，附完整重現方式。

---

## §1 一句話結論

`double == 40.2` 在 **BCB6 是 `true`**、在 **oracle（MinGW.org g++ 6.3.0）是 `true`**、
在 **WinLibs g++ 16.2.0（機台預設線）是 `false`**。

→ **golden 的 40.2 補償功能在真機上是活的**（不是死碼），
→ **oracle 線的宣稱第一次被實證，而且成立**，
→ ⚠️ **但機台預設線在這一行的行為與真機不一致。**

---

## §2 這個問題是什麼，為什麼掛了那麼久

golden `cContact.cpp:18455-18456` 用**浮點相等**當閘門：

```cpp
if(DeviceForm_File.dKitDiameter==8 ||
   DeviceForm_File.dKitDiameter==40.2)
```

移植樹逐字複製（`cContact.cpp:388-389`、`cinitial.cpp:8416`）。

`40.2` 在二進位不可表示（0.2 = 1/5）。而 32-bit x87 的 `FLT_EVAL_METHOD == 2` 表示
**每一個運算與常數都以 long double（80-bit）精度求值** —— 比較式裡的字面值取 40.2 的
80-bit 捨入，而變數 `d` 是 64-bit 捨入再加寬，兩個捨入不同。

`tools/fp_equality_probe.cpp`（`AI(W906-GL-Triage) 20260826`）就是為這件事寫的，
而它的檔頭寫著 **`NOBODY HAS MEASURED IT`**，並列出兩個互斥假設：

| 假設 | 若成立 |
|---|---|
| BCB6 也不相等 | golden 有一個永遠不會 fire 的生產功能 → 真的 upstream 缺陷 |
| BCB6 相等 | **移植樹在這一行的數值行為與 BCB6 分歧** —— 正是 oracle 線存在的理由 |

probe 檔頭同時下了一道禁令：
> `Until it answers, neither cContact.cpp:389 nor cinitial.cpp:8416 may be changed`

**它從 20260826 到 20260907 都沒被跑過**，原因是 `C:\MinGW` 當時是空目錄，
而 BCB6 全機不存在（`D:\BCB6_1203_UI\README_1203_MONITOR.txt` 20260907 13:50 實測
「bcc32 / bpr2mak / ilink32 / any Borland install root — ALL ZERO hits on C: and D:」）。

**BCB6 於 20260907 14:49 抵達**（`bcc32.exe` 1,398,272 B，CreationTime 實測），
所以 20260908 是這個問題**第一次可以被真正回答**的日子 —— 不是用代理，是問本人。

---

## §3 實測結果（20260908）

三條線，同一份 `tools/fp_equality_probe.cpp`，未修改。

| | BCB6 5.6.4 | Oracle g++ 6.3.0 | WinLibs g++ 16.2.0 i686 |
|---|---|---|---|
| `double d = 40.2` 位元組 | `404419999999999A` | `404419999999999A` | `404419999999999A` |
| 字面值 `40.2` 位元組 | `404419999999999A` | `404419999999999A` | `404419999999999A` |
| `memcmp` 位元相同 | yes | yes | yes |
| **`d == 40.2`** | **`true`** | **`true`** | **`false`** |
| `via_param(40.2) == 40.2` | `true` | `true` | `false` |
| `volatile vd == 40.2` | `true` | `true` | `false` |
| `FLT_EVAL_METHOD` | **未定義**（印 -99） | 2 | 2 |
| 印出的十進位 | `40.20000000000000284` | `40.200000000000002842` | `40.200000000000002842` |

⚠️ **三者的位元完全相同。** 差異純粹在**比較的求值精度**，不是資料的捨入意外。

⚠️ **BCB6 的 `FLT_EVAL_METHOD` 是未定義的**（C99／C++11 才有的巨集，BCB6 的
`<cfloat>` 沒有它），所以 probe 走 `#else` 印 -99。**這不是量測失敗**，
而且它本身就是一個資訊：BCB6 沒有宣告自己的求值精度契約。

---

## §4 完整重現方式（`C:\MinGW` 被刪掉之後仍可驗 BCB6 與 WinLibs 兩條）

```powershell
$root = "D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
$scr  = "<任何全新暫存目錄，不要用 build\ 或 build_oracle_probe\>"
$src  = "$root\tools\fp_equality_probe.cpp"

# (1) ORACLE —— 只在 C:\MinGW 還在的時候可跑
#     ⚠️ PATH 必須前置 C:\MinGW\bin，否則 cc1plus 以 0xC0000135 零訊息死掉
$env:PATH = "C:\MinGW\bin;$env:PATH"
& "C:\MinGW\bin\g++.exe" -std=c++17 -O0 -static -include cfloat -o "$scr\fp_oracle.exe" $src
& "$scr\fp_oracle.exe"

# (2) NON-ORACLE（機台預設線）
$w = "$env:LOCALAPPDATA\Programs\ht9045-nonoracle-toolchain\mingw32\bin"
& "$w\g++.exe" -std=c++14 -O0 -static -include cfloat -o "$scr\fp_winlibs.exe" $src
& "$scr\fp_winlibs.exe"

# (3) BCB6 —— 真答案，永久可重現
$bcb = "D:\ProgramFiles\Borland\CBuilder6"
& "$bcb\Bin\bcc32.exe" -Od -I"$bcb\Include" -L"$bcb\Lib" -e"$scr\fp_bcb6.exe" $src
& "$scr\fp_bcb6.exe"
```

**bcc32 的旗標對照**（給下一個人）：`-Od` ＝ 不最佳化（不是 `-O0`）、
`-e<路徑>` ＝ 輸出檔名、`-I`／`-L` 要顯式給，否則找不到 `bcc32.cfg`。
`-include cfloat` 是 GCC 專屬，bcc32 沒有 —— **不需要**，probe 的 `#ifdef` 已經處理。
probe 本身**沒有任何 C++11 語法**（只用 `<cstdio>`／`<cstring>`／`memcpy`），所以 bcc32 直接編得過。

⚠️ **`-O0`／`-Od` 是刻意的**：最佳化器可能把比較常數摺疊掉而藏住效應，
那會回答另一個問題。

---

## §5 三個後果

### §5.1 ✅ golden 那個功能是活的 —— upstream 缺陷假設被推翻

BCB6 回 `true` → 40.2 kit 直徑的 Z 高度補償**在出貨機上會觸發**。
`docs/UPSTREAM_DEFECT_REPORT.md` 的 pending 區若列了「40.2 分支永不 fire」，
**那一條要撤回**（它描述的是移植樹的行為，不是 golden 的）。

### §5.2 ✅ oracle 線的宣稱第一次被實證，而且成立

`C:\MinGW` g++ 6.3.0 與 BCB6 在唯一有意義的那個案例上**答案一致**。
「唯一能重現 BCB6 x87 算術」這句話從 20260826 掛到現在都沒被測過 ——
**今天測了，過了。**

⚠️ **但這不等於「逐位元相同」**。量到的是**這一個相等閘門的布林結果一致**，
不是任意運算的位元相等。不要外推。

### §5.3 ⚠️ 機台預設線在這一行與真機行為不一致 —— 佇列，不做

WinLibs 回 `false` → 在 `build_nonoracle` 與 `build_x64`（要跑機台的兩條線）上
`cContact.cpp:389` 與 `cinitial.cpp:8416` 走錯的臂，**Z 高度補償永遠不 fire，而真機會 fire**。

這同時解釋了 ctest 桶 B：`cContact` 在非 oracle 線失敗、offset==0，就是這個原因。

**處置：佇列，不動。** 理由有兩層：

1. probe 的禁令解除了，但答案本身說「**照 golden 逐字抄**」與「**行為等於 BCB6**」
   在這一行是**相反的指令** —— 逐字抄產生的是不同行為。
2. 它碰的是**接觸高度**。依 `BU_CAMPAIGN_PLAN.md` §9，安全關鍵行為變更
   → 累積到佇列、繼續下一個非安全項、**不停、不等、也不自己做**。

**待使用者裁決的選項**（先記，不催）：
| 選項 | 內容 | 代價 |
|---|---|---|
| (a) 不動 | 移植樹保持逐字忠實，接受行為分歧 | 機台的 Z 補償與真機不同 |
| (b) 加容差 | 改成 `fabs(d-40.2) < eps` | 偏離 golden 原文，但行為等於 BCB6 |
| (c) 改求值精度 | 對該 TU 加 `-ffloat-store` 或 `-fexcess-precision=standard` | 只影響那一個檔，不改原始碼；需驗證不影響其他數值 |
| (d) 上游修 | 請 golden 改掉浮點相等 | 保護不到現場（現場跑 V899） |

### §5.4 BU-X1b：14 個閘門逐一列出（20260908 實測，cp950 逐檔解碼）

「golden 14 / 移植樹 2」**精確成立**。

**golden（14）**：

| 檔:行 | 內容 | 備註 |
|---|---|---|
| `adam6024.cpp:1021` | `if(fDiameter==40.2)` | ⚠️ **不同變數，而且看起來是 `float`** —— 見下方 §5.5 |
| `cContact.cpp:539` | `else if(...dKitDiameter==8 \|\| ...dKitDiameter==40.2)` | 移植樹已有對應（`cContact.cpp:389`） |
| `cContact.cpp:901` | `DeviceForm_File.dKitDiameter==40.2) &&` | 尚未翻譯 |
| `cContact.cpp:6589` | 同式 | 尚未翻譯 |
| `cContact.cpp:7148` / `:7160` / `:7196` / `:7428` | 同式 ×4 | 尚未翻譯 |
| `cContact.cpp:9750` / `:9765` / `:9799` / `:9973` | 同式 ×4 | 尚未翻譯 |
| `cContact.cpp:18456` | 同式（probe 檔頭引用的那一個） | 尚未翻譯 |
| `cinitial.cpp:6911` | 同式 | 移植樹已有對應（`cinitial.cpp:8418`） |

**移植樹的生產閘門只有 2 個**：`cContact.cpp:389`、**`cinitial.cpp:8418`**。
（另 8 個命中全是註解／測試／probe 自己：`test_cContact.cpp:436`、
`fp_equality_probe.cpp:13/26/82/83/85`、`JsonWriter.cpp:263`、`cContact.h:275`。）

⚠️ **行號更正**：`fp_equality_probe.cpp:15` 的檔頭與本檔前一版都寫
~~`cinitial.cpp:8416`~~ —— 實測是 **`:8418`**，漂了 2 行。

→ **剩下 12 個會隨 `cContact` 家族翻譯（BU-T，約 11,347 行、切 4-6 波）一個一個到達。**
每一個到達時都是同一個決策，所以 §5.3 的裁決要一次做完，不要逐個問。

### §5.5 ⚠️ `adam6024.cpp:1021` 是不同的子案例，不要跟其他 13 個混

`if(fDiameter==40.2)` —— 變數名是 **`fDiameter`**（`f` 前綴，其餘 13 個都是
`DeviceForm_File.dKitDiameter`）。**若它真的是 `float`**，行為就不一樣了：
`float` 與 `double` 字面值比較時 `float` 會被提升，而 40.2 的 `float` 捨入
與 `double` 捨入本來就不同 → **在包含 BCB6 的所有工具鏈上都是 `false`**，
而且與 `FLT_EVAL_METHOD` 無關。

**尚未驗證**（該檔尚未翻譯，型別未確認）。翻 `adam6024` 的那一波必須先確認
`fDiameter` 的宣告型別，**不要套用 §5.3 的裁決結果到這一行**。

### §5.6 這個陷阱在樹裡已經有第二個實例

`JsonWriter.cpp:263` 的註解自己寫著「This is the same excess-precision trap as the
dKitDiameter == 40.2 entry in ...」→ **excess-precision 不是 `cContact` 專屬問題**，
它已經被獨立遇到過一次。§5.3 的裁決應該同時涵蓋那一處。

---

## §6 基準線退役：理由變乾淨了

**不是「oracle 沒用」** —— 它剛剛證明自己有用。
**而是「本人在這台上」** —— BCB6 於 20260907 14:49 抵達並經實證可用
（本檔 §3、§4 就是用它產生的），而使用者裁決保留它。

> **oracle 是代理，BCB6 是本人。本人在，就不需要代理。**

所以 `C:\MinGW`（536.4 MB／8,610 檔）與 `build_oracle_probe`（143 支 exe）
在本檔落地之後可以移除（BU-X4）。

**移除前的檢查清單**：
1. ✅ 本檔存在且記錄了完整重現方式（§4 的 (2)(3) 兩條不需要 `C:\MinGW`）
2. ⬜ `build.bat:37/:57` 的 `C:\MinGW` FATAL 依賴解除（或 `build.bat` 一起退役）
3. ⬜ 約 11 個引用 oracle 線的設定／工具站點清理
4. ⬜ `BU_CAMPAIGN_PLAN.md` §7 的 gate 集不再提 oracle 線
5. ⬜ ctest 桶 B（`cContact`／`GA1_LastSet`）改登記為「非 oracle 線預期失敗」，
   並附本檔連結說明根因 —— **不要讓下一個人以為那是回歸**

⚠️ **`build_oracle_probe` 在本檔落地前不要刪** —— 它是跑 §4 (1) 的地方。
本檔落地後它就沒有獨佔價值了（唯一的例外：它是這台**唯一未武裝 `HAVE_PCI1203`** 的
建置組態，`CMakeCache.txt` 裡沒有 `ADVMOT_APP_LIB`；若要保留，理由是那個，不是 oracle）。

---

## §7 P18 裁決：兩處改成容差比較（20260923，使用者裁決「先試丙，影響其他就全面改乙」）

**丙（只對 cContact.cpp／cinitial.cpp 加 `-ffloat-store`，不改原始碼）量過，有影響：**
在 oracle 線（MinGW.org g++ 6.3.0，本樹 build.bat 用的那套）對兩個 TU 各編兩次，`objdump -d` 逐函式比對（去掉位址相依的運算元）：

| TU | 函式數 | 機器碼改變 | 例 |
|---|---:|---:|---|
| `cContact.cpp` | 12 | **6** | ComputeMaxIndexForceLimit、ComputeMinForce、ComputeTotalAirForce、ComputeIndexDownPos、ComputeDutCount（比較所在的是 ComputeTestZCompensationHight） |
| `cinitial.cpp` | 311 | **31** | InitialMotorParameter、SetTechDataToProd 族、ChangeSite、ARM_OFFSET::GetX/GetY/GetArmX…（比較所在的是 DoSetupSystemToProd） |

除了含那兩個比較的函式之外還有 35 個函式的機器碼變了 ⇒ 裁決條件 1 成立 ⇒ 拿掉旗標，改**乙**。
（量測腳本在當晚的 scratchpad `p18_fp.py`：取 ninja 的真實編譯指令、+/- `-ffloat-store`、objdump 比對。）

**乙的寫法**（兩處都原地改一行、行尾加 `//AI(W906-P18)`，不位移其後的行號）：

```cpp
// cContact.cpp:672（ComputeTestZCompensationHight）、cinitial.cpp:8574（DoSetupSystemToProd）
(dKitDiameter - 40.2 < 1e-6 && dKitDiameter - 40.2 > -1e-6)   // 原文 dKitDiameter == 40.2
```

* **為什麼不用 fabs**：cContact.cpp 沒有 include math 標頭，加一行 include 會讓其後行號全部位移（這支檔被很多文件引用行號）。兩個不等式等價，不需要標頭。
* **eps＝1e-6 的理由**：`dKitDiameter` 是操作員輸入的鋼徑（mm，工單 Contact.Data），有效位數最多到 0.001 mm ⇒ 兩個「不同」的鋼徑至少差 1e-3；
  同一個十進位 40.2 的 64-bit 與 80-bit 捨入差約 2.8e-15（§3 的位元組就是這個差）。1e-6 離兩邊各有至少三個數量級（1e-3 / 1e-6 = 1000；1e-6 / 2.8e-15 ≈ 3.6e8）。
* **結果**：BCB6、oracle g++ 6.3.0、WinLibs 16.2.0 三條線都判 40.2 為真 ⇒ 全部與 BCB6（真機行為）一致，§1 那個「機台預設線與真機不一致」消失。
* **`== 8` 不動**：8 在二進位可精確表示，三條線結果本來就一致，照 golden 原文。
* **範圍**：裁決只問這兩處。全樹其他「浮點變數 == 十進位小數字面值」的地方**沒有改**，盤點列進當晚的晨報，由使用者決定要不要一起改。

---

## §8 A3 六處（20260924，使用者裁決 A3「你分析正確，可以執行」）

**裁決內容**：§7 盤點出的六處「浮點變數 == 十進位小數字面值」**先量**，移植樹算出來的結果與 BCB6 不同才改；
改法同 §7（`(x - c < 1e-6 && x - c > -1e-6)`，同一行替換、行號零位移），標記 `//AI(W906-P18-A3) 20260924:`。

### §8.1 量測方法

* **BCB6 本人不在這台**：`D:\ProgramFiles\Borland\CBuilder6` 在 20260922 換筆電後不存在（C:／D: 深度 5 搜 `bcc32.exe` 0 筆），
  §4 的 (3) 在這台跑不了。所以「BCB6 算出什麼」用三件事推出，每件都有收據：
  1. **§3 實測**：BCB6 `-Od` 下 `double d=40.2; d==40.2` 為真 —— 字面值以 64-bit double 常數比較、變數從記憶體載入。
  2. **golden 出貨組態** `HT9045.bpr:201` CFLAG1 = `-Od … -r- …`（不最佳化、不配暫存器變數；V912 的 `.bpr:204` 同樣是 `-Od … -r-`）
     ⇒ 每個 `double` 區域變數都住在堆疊，`double d = x/10.0;` 一定以 FSTP qword 寫回記憶體（捨入成 64-bit）再比較。
  3. **x87 精度控制**：BCB6（Delphi RTL `Default8087CW=$1332`）與 oracle（探針量到 CW=`0x037F`）都是 64-bit 尾數 extended。
     ⇒ BCB6 的 `d = 28/10.0` = extended FDIV 之後 FSTP qword，探針用 `long double` 照做（下表「BCB6 模擬」欄）。
* **字串解析**：移植樹 `ReadIniData(double)` → `vclcompat::TIniFile::ReadFloat` → `parseFloatDef` → `std::strtod`（`vclcompat/IniFiles.cpp:50-60`）；
  golden 是 `TIniFile::ReadFloat` → `StrToFloat`（extended 累積十進位整數尾數、除以 10^k、回傳 Double 時再捨入一次）。
  探針照後者用 `long double` 算，並量出那個 extended 值離 double 捨入中點還有幾個 extended ulp —— 離得越遠，
  BCB6 RTL 的實作細節越不可能改變結果。
* **探針**（`%TEMP%\a3probe\a3probe.cpp`，不進版控）：`C:\MinGW\bin\g++.exe -std=c++1z`（與 `build.ninja` 同旗標；本樹沒有任何 `-m`／`-f` 浮點旗標），
  `-O0` 連 `build_a3d\libvclcompat.a`、`-O3 -DNDEBUG` 連 `build_a3r\libvclcompat.a`，**用移植樹自己的 ReadFloat** 讀一份暫存 ini，
  再用與每一處現場相同的程式形狀（全域結構、區域變數、if/else 鏈、`noinline`）比較。
* **真函式**：再用 `tests/test_transform_funtion.cpp` 的 (f) 段直接打 `TransformFuntion` 的那四個比較，Debug 與 Release 各跑一次（見 §8.4）。
  探針只是形狀相同；**決定改不改的是真函式在 Release 的結果。**
* WinLibs（§3 的第三條線）這台沒有裝，沒量。

### §8.2 這台機器上真的會出現的字串（唯讀掃描）

| 來源 | 找到的值 |
|---|---|
| `IniData\Data\*\Contact.Data` `Kit Diameter=`（64 份） | `3.0000`×25、`4.0000`×17、`6.0000`×21、`8.0000`×1 —— **沒有 5.6** |
| 同上 `Die Force Kit Diameter=` | `2.0000`×2、`3.0000`×4、`5.0000`×1 |
| `IniData\Data\*\HotPlate.Data` `X Pitch=`（行首有空白） | `20.000`×34、`20`×6、`40.000`×5、`26.660`×4、`25.000`×4、`20.00`×3、`15.000`×3、`90.000`×2、`50.000`×2、`60.000`、`35` —— **沒有 26.67**（4 份已是改寫後的 26.660） |
| `system\ContactInfo.ini` `[SLK Type]` | `Type=30,40,60,56`（golden `atof`，整數精確）；KYEC 時 golden `ContactForce.cpp:1114-1122`（移植樹 `ContactForce.cpp:821-828`）改寫成 `28`／`58` |

配方以 `%0.4f` 存，所以 5.6 kit 會是 `5.6000`、40.2 是 `40.2000`。探針把這些與短寫法（`5.6`、`26.67`、`26.670`）都測了。

### §8.3 字串 → double：三方位元組完全相同

| 字串 | 移植樹 ReadFloat | 字面值 | BCB6 模擬 | extended 低 11 bit（離中點 0x400） |
|---|---|---|---|---|
| `5.6000`／`5.6` | `4016666666666666` | 同 | 同 | `0x333`（205 ulp） |
| `40.2000`／`40.2` | `404419999999999A` | 同 | 同 | `0x4CD`（205 ulp） |
| `26.670`／`26.67` | `403AAB851EB851EC` | 同 | 同 | `0x429`（41 ulp） |
| `26.660` | `403AA8F5C28F5C29` | 同 | 同 | `0x7AE`（942 ulp） |
| `28`/10.0、`58`/10.0、`56`/10.0 | —（BCB6：FDIV extended＋FSTP qword） | `2.8`／`5.8`／`5.6` | `4006666666666666`／`4017333333333333`／`4016666666666666` = 字面值 | — |

`-O0` 與 `-O3` 兩個探針這張表逐字相同。⇒ **「從記憶體載入的 double 跟字面值比」這個形狀，三方永遠一致**；
會出事的只有「**算出來、還沒寫回記憶體**」的值。

### §8.4 逐處結果

| 位置（移植樹，本版行號） | 值從哪來 | 字串 | 探針 -O0 | 探針 -O3 | BCB6 預期與根據 | 處置 |
|---|---|---|---|---|---|---|
| `adam6024.cpp:193` `fDiameter==5.6` | `fDiameter = DeviceForm_File.dKitDiameter`／`dDieForceKitDiameter`（`:174/:178`，記憶體載入，無運算） | `5.6000` | 真 | 真 | 真（§8.3 位元組相同＋§3 的比較方式） | **一致，不改** |
| `adam6024.cpp:423` `fDiameter==5.6` | 同上（`:220/:224`；中間 `iHeadDeviceCT` switch 只會賦 3.0 等精確值） | `5.6000` | 真 | 真 | 真 | **一致，不改** |
| `adam6024.cpp:504` `d==2.8`（KYEC） | `double d = cft.SLKClass.items[i].dDiameter/10.0;` | `28` | 真 | **假** | 真（`-Od -r-` ⇒ `d` FSTP 回記憶體；模擬值 = 字面值） | **不一致 → 改容差** |
| `adam6024.cpp:508` `d==5.8`（KYEC） | 同上 | `58` | 真 | **假** | 真 | **不一致 → 改容差** |
| `forms/fContact.cpp:1013` `DeviceForm.dKitDiameter==5.6 && …`（§7 寫的 `:982`，被 8bfbab2 合併推到 1013） | 記憶體裡的 `DeviceForm` | `5.6000` | 真 | 真 | 真 | **一致，不改** |
| `forms/fHotPlate.cpp:234` `XPitch==26.67` | `XPitch = ReadIniData(...,"X Pitch",0.1)`（另一個 TU 回傳的 double，存進全域再比） | `26.670`、`26.67` | 真（改寫成 26.66） | 真 | 真 | **一致，不改** |

同一條 if/else 鏈裡的 `==6.0`、`==4.0`、`==3.0`、`==0`（以及 `fContact.cpp:1665-1671` 的 `==3`、`==4`、`==8`）：
這些值在二進位**可精確表示**，64-bit 與 80-bit 是同一個數，不可能分歧 —— 照 golden，不動。

**真函式驗證**（`TransformFuntion`，改之前）：

| 斷言 | build_a3d（Debug，`-g`） | build_a3r（Release，`-O3`） |
|---|---|---|
| f1/f2 `:504` KYEC 缸徑 3.0 選到 28 那列 | PASS | **FAIL**（退回第 0 列） |
| f3/f4 `:508` KYEC 缸徑 6.0 選到 58 那列 | PASS | **FAIL** |
| f5/f6 `:514` 缸徑 5.6 選到 56 那列 | PASS | **FAIL** |
| f7/f8 `:478` Die Force 缸徑 5.6 選到 56 那列 | PASS | **FAIL** |
| 全檔 | 15 PASS / 0 FAIL | 7 PASS / **8 FAIL** |

**改之後**（四行換成容差比較）：build_a3d 與 build_a3r 都是 15 PASS / 0 FAIL。

⇒ 探針的形狀結果在 759 行的真函式裡重現：**Release 組態選錯 SLK 那一列**（`fLoadRate`、`dContactOffset` 用的是第 0 列），
也就是 EP 輸出的下壓力用錯負荷率。Debug 組態（以及 `build.bat` 預設的空 build type，機台跑的就是這個）與 BCB6 一致。

### §8.5 A3 範圍外、同機制的兩處（依 20260917 常設裁決「≥90% 確認是問題且知道怎麼解就修」）

`adam6024.cpp:478`（Die Force 表）與 `:514`（SLK 表）的 `if(d==fDiameter)` **不是字面值**，所以不在 §7 的盤點裡；
但 `d` 是同一個 `dDiameter/10.0`，與記憶體裡的 `fDiameter` 比，機制與 `:504/:508` 完全相同：
56/10.0 在 -O3 留在 80-bit，與 5.6 的 64-bit 捨入判不等（探針與真函式都量到，上表 f5–f8）。

* 兩個條件都滿足：**確認是問題**（Release 真函式選錯列，量到）＋ **知道怎麼解**（使用者已核可的同一個寫法）。
* **只改 :504/:508 不改 :514 沒有意義**：5.6 kit 在 Release 仍會選錯列；KYEC 的 28/58 改寫成 3.0/6.0 之後是精確值，那條才會過。
* `-O0`／預設組態的行為不變：`d` 是整數 token/10（不同值至少差 0.1）、`fDiameter` 來自 `%0.4f` 配方（不同值至少差 1e-4），
  1e-6 內「相等」與 `==` 判定相同。
* **使用者可以否決這兩處**：還原就是把兩行改回 `if(d==fDiameter)`，並刪掉測試 f5–f8。

### §8.6 量了、沒改、要知道的

* **`forms/fContact.cpp:1669`／`:1671`（8bfbab2 新到的 `TfContact::ReadFile`）**：`dKitDiameter==5.6` 與 `dKitDiameter==40.2`，
  值是 `= ReadIniData(...)` 剛讀進來的記憶體 double。探針 -O0／-O3 皆為真 = BCB6 ⇒ 依 A3 規則**不改**。
  ⚠ 但 `:1671` 就是 §1 的 FP402 形狀：在 **WinLibs 16.2.0** 上它會像 §3 一樣判假。§7 的 P18 把它的兩個兄弟改了（理由是 WinLibs 那條線）；
  這一處要不要比照 P18，**是使用者的決定**（WinLibs 這台沒裝，`build.bat` 也只驅動 `C:\MinGW`）。
* **`adam6024.cpp:450`／`:739`／`:770`／`:774` 的 `dDiameter==fDiameter*10`**：`fDiameter*10` 是運算式暫存值，
  g++ 在 -O0 與 -O3 **都**以 80-bit 比較（探針：56 vs 5.6000×10 兩級皆假、40 vs 4.0000×10 兩級皆真）⇒ 移植樹兩個組態一致、不改。
  BCB6 `-Od` 同樣把暫存值留在 FPU 堆疊比較，所以**推論**也是假 —— 也就是 golden 的「逐顆／獨立 EP」表對 5.6 kit 永遠對不到。
  這一句**沒有用 BCB6 實測**（本人不在這台），是 golden 層級的疑點，照翻不修。
* 全樹其他「算出來再比相等」的形狀沒有逐一盤點；本節只涵蓋 §7 的六處＋上面兩組。

### §8.7 全量 ctest（改之後，四個全新建置目錄，161 項）

| 組態 | Debug | Release（-O3） |
|---|---|---|
| 出貨（`-DW906_NO_SOFT_SIMULTE=ON`，build_a3ds／build_a3rs） | 4 失敗：`config_db` `config_loaders` `dfm2rc_idempotent` `GA1_ReadGeneralIni` | 同左 4 項 |
| 模擬（預設，build_a3d／build_a3r） | 19 失敗（上面 4 項＋15 項 SOFT_SIMULTE 相依） | 同左 19 項 |

出貨組態的 4 項是既有基準（6 項）的子集；`dfm2rc_fidelity`、`dfm2rc_rc_compiles` 現在會過（Windows SDK 已裝好）。
模擬組態多出的 15 項（`SimIO` `W6_Canary` `W6_4_TesterAnchor` `HanaART` `BarCodeHelpers` `BarCode8CCDGlue` `AGV_E84` `Automation`
`W7_L1_Auto2` `W7_L1_Color` `W7_L1_Loader` `W7_L1_AutoRT` `GA2_C1_cinitial` `WB_SimPump` `mainproc_guard`）
與同一天另一個**沒有本次修改**的工作樹在模擬組態下的失敗清單逐項相同 —— 是組態相依，不是本次回歸
（§7 也記過 `GA2_C1_cinitial` 在 SOFT_SIMULTE 下的期望值是照出貨組態寫的）。

---

## §9 整棵樹加 `-fexcess-precision=fast`（20261003，RULINGS_20261003 第 7 條＝NIGHT_REPORT §0 #77 A，`AI(W906-FPFAST)`）

**一句話**：機台建置用的 WinLibs g++ 16.2 i686（-O0）加了這個旗標之後，§9.3 的 17 個探針情境全部回到跟 BCB6 一樣；
筆電的 oracle（MinGW.org g++ 6.3.0）**每一個編譯單元的機器碼都沒有變**（模擬、出貨兩個組態全部重編比對，§9.2）。

### §9.1 改在哪裡、為什麼是那一行

* **`CMakeLists.txt:7`**（原本的空行，其後行號全部不動）：
  `add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:GNU>>:-fexcess-precision=fast>" "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-fexcess-precision=fast>")`
  只給 GNU 的 C 與 C++；MSVC（第二 oracle）與 windres（.rc）都看不到。目錄層級、排在第一個 target 之前 ⇒
  `third_party/sqlite3`、`tests/` 全部繼承。compile_commands.json 實測：1,594 條裡 1,593 條帶一次，只有 windres 那 1 條沒帶。
* **根因就在上一行**：`CMAKE_CXX_EXTENSIONS OFF`（:6）讓 WinLibs 線用嚴格的 `-std=c++14`；GCC ≥ 13 在嚴格模式的預設是
  `-fexcess-precision=standard`（FLT_EVAL_METHOD 2：十進位字面值以 long double 求值），`gnu++14` 的話預設本來就是 fast。
  g++ 6.3.0 的 C++ **只有** fast（給 `standard` 會 `sorry, unimplemented: -fexcess-precision=standard for C++`），所以 oracle 線一直是 fast。
* **三個 .bat 都不用改**：`build.bat`、`build_nonoracle.bat`（:do_configure）、`build_x64.bat`（:do_configure）只傳編譯器路徑、
  `-DHT9045_CXX_STANDARD=14`、`-DCMAKE_EXE_LINKER_FLAGS=-static`，沒有自己的 C／CXX FLAGS ⇒ 全部吃 `CMakeLists.txt:7`
  （`git grep CMAKE_CXX_FLAGS`：除了 `.vscode/tasks.json:170` 的註解，沒有任何腳本傳）。機台已 configure 好的
  `build_integ_ship_x86`／`build_integ_dbg_x86` 下一次 `cmake --build` 時，因為 CMakeLists.txt 變了會自動重新 configure 而吃到。
  x86_64（`build_x64.bat`，SSE）本來就沒有 excess precision，這行在那裡沒有作用。
* **EastSun 的 -O2 線**（`build_integ_ship_x86_o2`，`.vscode/tasks.json:169-172`）把 `-ffloat-store -fexcess-precision=standard`
  放在 CMAKE_CXX_FLAGS／CMAKE_C_FLAGS。用 WinLibs 照同一組旗標 configure 實測命令列是
  `… -ffloat-store -fexcess-precision=standard -fno-finite-loops -std=c++14 -fexcess-precision=fast …` ⇒ **後者勝，那條線也變成 fast**。
  後果不是全好，見 §9.3 的矩陣與 §9.5 第一點。
* `tools/gateverdict.sh:130` 的 R_NOTE 原寫「CMakeLists.txt 沒有 -ffloat-store / -fexcess-precision 保護」——現在有 -fexcess-precision 了，
  但 fast 不是那句話擔心的 -O3 保護（它正是允許 -O3 把算出來的值留在 80-bit 的模式），同一行改寫。

### §9.2 證明：oracle（g++ 6.3.0）的機器碼不變

**方法**（腳本在當天 scratchpad `fpfast/proof.py`）：用 build.bat 同一組參數 configure 兩個全新建置目錄
（`Obj/V906/build_fp`＝模擬、`build_fps`＝出貨 `-DW906_NO_SOFT_SIMULTE=ON`），取 compile_commands.json；
每一條**不重複的編譯命令**編兩次到 scratch——WITH＝原命令（＝建置真的會跑的那一條）、WITHOUT＝拿掉那一個旗標——
然後比：兩次的離開碼、stderr、物件檔位元組、`objdump -d -r --no-show-raw-insn` 逐段（段不同時再逐函式）、`objdump -s` 逐段（.text／.rdata／.data／.eh_frame…全部）。
編譯一律 IDLE 優先權、限定 6 顆核心。

* **WITHOUT 就是改之前的命令**：另把 HEAD 的 CMakeLists.txt（沒有那一行）configure 一次，1,594 條共同條目逐條
  「HEAD 的命令 ＝ WITH 拿掉旗標」，0 條不同。
* **比對器會抓到真的差異**（負向對照）：同一支腳本在 WITH 多加 `-ffloat-store`（§7 量過會改碼的那個旗標），
  `cContact.cpp` 與 `test_cContact.cpp` 兩條都判 DIFFERENT，列出 `ComputeMaxIndexForceLimit`、`ComputeMinForce`、`ComputeTotalAirForce`…（與 §7 一致）。

**證明結果**（oracle g++ 6.3.0；每一條唯一的編譯指令「加／不加」各編一次，逐位元組比 obj，再用 objdump 逐區段比反組譯與內容；1003 10:1x～12:2x，筆電）：

| 組態 | 比對的編譯指令 | 逐位元組相同 | 只差 `__DATE__`／`__TIME__` | 程式碼不同 | 失敗 |
|---|---:|---:|---:|---:|---:|
| SIM（模擬，Debug `-g1`） | 1104（涵蓋 1593 條 GNU 編譯、932 個原始檔） | 1101 | 3 | **0** | 0 |
| SHIP（出貨，`-DW906_NO_SOFT_SIMULTE=ON`） | 1097（涵蓋 1593 條、932 個原始檔；C++ 1094、C 3） | 1094 | 3 | **0** | 0 |
| SIM `-O3` 抽查 | 24（18 個原始檔） | 24 | 0 | **0** | 0 |

* 只差時間的三支在兩個組態都是 `TempCtrl/TriTemp.cpp`、`cObserver.cpp`、`WebBridgeTags.cpp`（`.rdata` 裡的編譯時間字串）；把日期與時間固定後重編，三支都逐位元組相同（`recheck.py`：RESULT ALL EXPLAINED）。
* 兩邊編譯的回傳碼、stderr、stdout 全部相同；g++ 6.3.0 對 C 與 C++ 都安靜接受這個旗標，沒有新的診斷。
* SHIP 那一輪：反組譯比對 122,499 個區段（11,736,320 行），`objdump -s` 內容比對 256,318 個區段。結果檔：`tools/fpfast/results/proof_SIM.*`、`proof_SHIP.*`、`recheck_SHIP.txt`。

### §9.3 正向對照：BCB6 本人、oracle、WinLibs

**BCB6 回到這台了**（§8.1 寫的「不在」已過期）：`D:\ProgramFiles\Borland\CBuilder6\Bin\bcc32.exe`（1,398,272 B，Borland C++ 5.6.4）。
WinLibs 16.2.0 i686 在 `D:\HT9045\install\mingw32-16.2.0\mingw32\bin`（與機台同一個 zip：`D:\software\winlibs-i686-gcc16.2.0-msvcrt.zip`）。

**§4 的原探針**（`tools/fp_equality_probe.cpp`，未修改）：BCB6 `d == 40.2`／`via_param`／`volatile` 三項 true；
oracle 加不加旗標都 true；WinLibs **不加 false、加了 true**。位元組三方都是 `404419999999999A`。

**擴充探針**（同一份原始碼給三條線，pre-C++11；`1`＝true）。L＝讀進來的 double 與十進位字面值比（cContact／cinitial／fContact／adam6024／Adam6024Pressure／fHotPlate 的形狀）；
R＝`Adam6024Pressure_St02.cpp:822` 的嚴格範圍；S＝算出來、存進 double、再比（adam6024 KYEC `d = 28/10.0; d == 2.8`）；X＝BCB6 依賴 80-bit 中間值的純運算（不應該隨旗標變）。

-O0（BCB6 是 `-Od`；oracle 與機台的主線都是空 build type＝-O0）：

| 情境 | BCB6 | 6.3.0 不加 | 6.3.0 加 | WinLibs 不加 | WinLibs 加 |
|---|:-:|:-:|:-:|:-:|:-:|
| L1 `double d=40.2; d==40.2` | 1 | 1 | 1 | **0** | 1 |
| L2 `double d=5.6; d==5.6` | 1 | 1 | 1 | **0** | 1 |
| L3 `volatile double vd=5.6; vd==5.6` | 1 | 1 | 1 | **0** | 1 |
| L4 `volatile double vd=40.2; vd==40.2` | 1 | 1 | 1 | **0** | 1 |
| L5 `via_param(5.6)==5.6` | 1 | 1 | 1 | **0** | 1 |
| L6 全域 `DeviceForm_File.dKitDiameter`（`"40.2000"`）`==40.2` | 1 | 1 | 1 | **0** | 1 |
| L7 `fDiameter`（`"5.6000"`）`==5.6` | 1 | 1 | 1 | **0** | 1 |
| L8 `fDiameter`（`"40.2000"`）`==40.2` | 1 | 1 | 1 | **0** | 1 |
| L9 `XPitch`（`"26.67"`）`==26.67` | 1 | 1 | 1 | **0** | 1 |
| R1 v=5.2：`v<0.8 \|\| v>5.2` | 0 | 0 | 0 | **1** | 0 |
| R2 v=0.8：同上 | 0 | 0 | 0 | 0 | 0 |
| S1 `d = 28/10.0; d==2.8` | 1 | 1 | 1 | **0** | 1 |
| S2 `d = 58/10.0; d==5.8` | 1 | 1 | 1 | **0** | 1 |
| X1 `56 == 5.6*10` | 0 | 0 | 0 | 0 | 0 |
| X2 `40 == 4.0*10` | 1 | 1 | 1 | 1 | 1 |
| X3 `(1e16+1)-1e16 == 1` | 1 | 1 | 1 | 1 | 1 |
| X4 `(1e308*10)/10 == 1e308` | 1 | 1 | 1 | 1 | 1 |

* BCB6 ＝ oracle（加不加都一樣）＝ **WinLibs 加了旗標**，17／17。WinLibs 不加：12 項跟 BCB6 相反（L1–L9、R1、S1、S2）。
* X 列（80-bit 中間值）五欄全同：旗標**沒有**動到 BCB6 依賴 80-bit 中間值的純運算。

-O2／-O3 矩陣（同一份探針；W＝WinLibs 16.2 `-std=c++14`，O＝oracle 6.3.0；fs＝`-ffloat-store`）：

| 情境 | BCB6 | W -O2 standard | W -O2 fast | W -O2 fs standard（EastSun 今天） | W -O2 fs fast（EastSun 本次之後） | W -O0 fs fast | O -O3（加不加同一顆 exe） | O -O2 fs |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| L1–L9 | 1 | **0** | 1 | **0** | 1 | 1 | 1 | 1 |
| R1 | 0 | **1** | 0 | **1** | 0 | 0 | 0 | 0 |
| R2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| S1、S2 | 1 | **0** | **0** | **0** | 1 | 1 | **0** | 1 |
| X1 | 0 | 0 | 0 | 0 | **1** | **1** | 0 | **1** |
| X2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| X3、X4 | 1 | 1 | 1 | 1 | **0** | **0** | 1 | **0** |

* `-ffloat-store` 在 fast 模式下（兩個編譯器、-O0 或 -O2 都一樣）把**運算式的暫存值**也捨入成 double ⇒ X 類跟 BCB6 不同；
  在 standard 模式下暫存值是 long double 型別，存回去也是 80-bit，所以 X 對、但字面值錯。
* -O2／-O3 不加 `-ffloat-store`：算出來、指派給區域變數的值留在 80-bit 暫存器 ⇒ S 類錯（§8.4，A3 容差修的就是這一類）。
* EastSun 那條線的完整旗標組（`-O2 -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -ffloat-store -fexcess-precision=standard -fno-finite-loops`，
  以及後面再接 `-fexcess-precision=fast`）另外各編一次，17 項答案分別與「W -O2 fs standard」「W -O2 fs fast」兩欄逐項相同。
* oracle -O3 加／不加旗標的兩顆 exe 只差 2 bytes（PE 標頭時間戳）。

### §9.4 ctest 釘子 `FPFAST_Pin`（`tests/test_fpfast.cpp`）與它的對照

新測試，獨立執行檔（不連 god stack、不寫任何檔；argv[1]＝原始碼根目錄），三部分：

* **[R] 執行期**：10 個 BCB6 答案（§9.3 用 bcc32 實測）——讀進來的 double 跟 40.2／5.6／26.67 比、`v<0.8 || v>5.2` 的 5.2 與 0.8 邊界、
  `d = 28/10.0` 存進 volatile double 再跟 2.8 比。每個值都經過 volatile 或 noinline，Release -O3 也不會被常數摺疊。
* **[S] 原始碼**：`CMakeLists.txt` 必須剛好一行「活的」（不在註解裡）`add_compile_options(…-fexcess-precision=fast…)`，C 與 CXX 都有、都限 GNU、
  排在第一個 add_library／add_executable／add_subdirectory 之前；兩份 CMakeLists 都不可以有活的 `-fexcess-precision=standard`。
* **[X] 只在 x87（FLT_EVAL_METHOD 2）**：`(1e16+1)-1e16 == 1`、`(1e308*10)/10 == 1e308`、`56 == 5.6*10` 為假——旗標不可以動到的 80-bit 中間值
  （BCB6 實測同答案）。有人改成 SSE 數學、或全域加 `-ffloat-store`（§9.3 矩陣）會紅。

**為什麼一定要有 [S]**：g++ 6.3.0 的 C++ 只有 fast，所以在 oracle 上把那一行拿掉，[R] 照樣全綠（下表 C1）——只有 [S] 看得到。

對照（scratchpad `fpfast/ctl/run_ctl.sh`；編譯旗標＝建置裡這支測試的旗標，只差那一個）：

| | 編譯器／旗標 | 原始碼根 | 結果 |
|---|---|---|---|
| C1 | g++ 6.3.0，**不加** | 真的樹 | 21 PASS／0 FAIL，離開碼 0 ——執行期那半在 oracle 上看不到旗標被拿掉（預期） |
| C2 | 同 C1 | 根目錄放 HEAD 的 `CMakeLists.txt`（沒有那一行） | 15 PASS／**1 FAIL**（S1），離開碼 1 |
| C3 | WinLibs 16.2 `-std=c++14`，**不加** | 真的樹 | 12 PASS／**9 FAIL**（R1–R8、R10），離開碼 1；R9（0.8 邊界）兩種模式答案本來就一樣 |
| C4 | WinLibs 16.2，加 | 真的樹 | 21 PASS／0 FAIL |
| C5 | 同 C4 | HEAD 的 `CMakeLists.txt` | 15 PASS／**1 FAIL**（S1） |

### §9.5 範圍外、要知道的

1. **EastSun 的 -O2 線需要一個決定**（不擋本次）。§9.3 矩陣：**沒有任何一種 -O2 組合在 17 個情境全部等於 BCB6 `-Od`**——
   * 今天的設定 `-O2 -ffloat-store -fexcess-precision=standard`：L／R／S 錯 12 項（字面值以 long double 比）。
   * 本次之後的實際效果（`+ -fexcess-precision=fast` 排在後面而勝出）：L／R／S 全對，但 `-ffloat-store` 在 fast 模式下**連運算式的暫存值都捨入成 double**，
     X1／X3／X4 錯——X1 正是 `adam6024.cpp:450`／`:739`／`:770`／`:774` 的 `dDiameter==fDiameter*10` 形狀（BCB6 對 5.6 kit 永遠對不到，這條線會對到）。
     `-O0 -ffloat-store -fexcess-precision=fast` 一樣（所以是 `-ffloat-store` 造成，不是 -O2）。
   * 拿掉 `-ffloat-store`（`-O2 -fexcess-precision=fast`）：L／R／X 對，S 錯（§8.4 那一類：算出來的值留在 80-bit 暫存器；已知四處有 A3 容差）。
   * -O0 的線（機台 F5 的 `build_integ_ship_x86`／`build_integ_dbg_x86`、`build_nonoracle`）加了旗標之後 **17 項全對**。
2. 這個旗標只管「字面值與運算式以什麼精度求值」，**不管 -O2／-O3 把算出來的值留在 80-bit 暫存器**（§8.4 的 S 類；fast 正是允許它的模式）。
   §8.4／§8.5 的 A3 容差、St02 在 `Adam6024Pressure_St02.cpp` 的 A3 與 `(double)` 轉型都**照留**（裁決原文）。
   oracle 的 Release（-O3）在 S 類本來就跟 BCB6 不同，加不加旗標是同一顆執行檔（只差 PE 時間戳 2 bytes）。
3. §8.6 第二點的「推論」（BCB6 `56 == 5.6*10` 為假）今天用 bcc32 實測：**假**，推論成立（X1）。
4. §5.3 的選項表：這次等於 (c)「改求值精度」的全樹版，用的是 fast 不是 standard。§8.6 第一點的 `forms/fContact.cpp:1671`（FP402 形狀，沒改容差）在 WinLibs 上也回到真。
5. 機台端 ctest：WinLibs 線之前因為字面值這一類而紅的測試，現在可能變綠——拿失敗清單逐名比對時，**少掉的那幾支是預期的**，不是誤報。
6. `tools/hmi_shell/build_hmi_shell.bat` 直接叫 g++（WebView2 外殼，沒有機台邏輯、沒有浮點比較），不經 CMake，沒有加。
