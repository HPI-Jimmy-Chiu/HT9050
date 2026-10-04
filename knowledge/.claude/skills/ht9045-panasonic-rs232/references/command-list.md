# 國際牌 MINAS A5／A4 通訊指令表

> ★＝HT9045 golden 0618 實際有送。**位元組欄＝區塊第 3 個位元組＝(mode<<4)|command**（規則與證據見 `protocol.md` §5）。
> 「送」「回」欄寫的是 N（參數個數）與參數內容，不含 N／axis／第 3 位元組／checksum。每個回覆最後一個參數都是錯誤碼（位元意義見 `protocol.md` §5.1）。
> 手冊：「只能用表上的指令，用別的指令驅動器動作不保證」（A5II p.423）。

## 1. A5／A5II（A5II p.423–438；A5中 p.329–344，印刷頁碼相同）

### NOP（command 0）

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 頁 |
|---|---|---|---|---|---|
| 0／1 | 10h | Read out of CPU version | N＝0 | N＝3：版本高、版本低、err（例：上位 30h、下位 13h；A5中 p.330 標為 Ver3.13） | A5II p.424 |
| 0／5 | 50h | Read out of driver model | N＝0 | N＝0Dh：12 個 ASCII（例 "MADHT1105***"）、err | A5II p.424 |
| 0／6 | 60h | Read out of motor model | N＝0 | N＝0Dh：12 個 ASCII（例 "MSME012S1***"）、err | A5II p.424 |

### INIT（command 1）

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 頁 |
|---|---|---|---|---|---|
| 1／7 | 71h | Capture and release of execution right | N＝1：1＝取得、0＝釋放 | N＝1：err（取得失敗＝in use） | A5II p.425；範例 p.419 |
| 1／8 | 81h | Setup of RS232 protocol parameter | N＝4：T1、T2、T6、RTY（RTY 4 位元） | N＝1：err（T1／T2／T6／RTY 錯誤位元） | A5II p.425 |
| 1／9 | 91h | Setup of RS485 protocol parameter | 同 1／8 | 同 1／8 | A5II p.425 |

- 1／8、1／9：指令完成前仍用舊值，完成後從下一個指令起用新值；單位 T1 0.1 s、T2 0.1 s、T6 1 ms（A5II p.425）。

### POS, STATUS, I/O（command 2）

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 單位／說明 | 頁 |
|---|---|---|---|---|---|---|
| 2／0 | 02h | Read out of status | N＝0 | N＝3：control mode、status、err | control mode 0＝位置、1＝速度、2＝扭力、3＝全閉環；status 有正轉中、反轉中、低於 DB 允許速度（<30 r/min）、Torque in-limit | A5II p.426 |
| 2／1 | 12h | Read out of command pulse counter | N＝0 | N＝5：32 位元計數、err | 絕對座標，負方向為負 | A5II p.426 |
| 2／2 | 22h | Read out of feedback pulse counter | N＝0 | N＝5：32 位元、err | 編碼器累計脈波 | A5II p.427 |
| 2／4 | 42h | Read out of present speed | N＝0 | N＝3：16 位元、err | r/min，負方向為負 | A5II p.427 |
| **2／5 ★** | **52h** | **Read out of present torque output** | **N＝0** | **N＝3：16 位元 L、H、err** | **「Rated motor torque＝2000」換算；16 位元；扭力命令負方向為負** | **A5II p.427；A5中 p.333** |
| 2／6 | 62h | Read out of present positional command deviation | N＝0 | N＝5：32 位元、err | 指令單位；編碼器落在負方向時為正 | A5II p.428 |
| 2／7 | 72h | Read out of input signal | N＝0 | N＝5：32 位元輸入、err | 依參數分配後的內部邏輯（Servo-ON、Alarm clear、CW／CCW over-travel inhibit、Gain switching、Torque limit switching、Safety input 1／2…），不等於 X4 腳位 | A5II p.428 |
| 2／8 | 82h | Read out of output signal | N＝0 | N＝7：32 位元輸出、16 位元 alarm data、err | Servo-Ready、Servo-Alarm、Positioning complete、Mechanical brake released、Zero speed detection、Torque in-limit…；alarm data 位元有 Overload protection、Over-regeneration、Battery、Fan、Encoder overheat、Lifetime detection… | A5II p.429 |
| 2／9 | 92h | Read out of present speed, torque and positional command deviation | N＝0 | N＝9：速度 16、扭力 16、偏差 32、err | 單位同 2／4、2／5、2／6 | A5II p.430 |
| 2／A | A2h | Read out of status, input signal and output signal | N＝0 | N＝0Dh：control mode、status、輸入 32、輸出 32、alarm data 16、err | 位元意義同 2／0、2／7、2／8 | A5II p.430 |
| 2／C | C2h | Read out of external scale | N＝0 | N＝0Bh：encoder ID 16、status 16、絕對位置 48 位元、err | 只有全閉環控制能用，其他回 command error | A5II p.431 |
| 2／D | D2h | Read out of absolute encoder | N＝0 | N＝0Bh：encoder ID 16、status、單圈 17 位元、多圈 16 位元、0、err | 增量型編碼器回 command error；範例 p.418（`0B 01 D2 03 11 00 00 D8 FF 01 00 00 00 00 36`＝單圈 01FFD8h、多圈 0） | A5II p.432 |
| 2／E | E2h | Read out of external scale deviation and sum of pulses | N＝0 | N＝9：FB 脈波和 32、偏差 32、err | | A5II p.432 |

### PARAMETER（command 7）

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 說明 | 頁 |
|---|---|---|---|---|---|---|
| **7／0 ★** | **07h** | **Individual read out of parameter** | **N＝2：parameter type、parameter No.** | **N＝5：32 位元值（L→H，符號延伸）、err** | 類別或編號超出範圍回 No. error | **A5II p.433；A5中 p.339** |
| **7／1 ★** | **17h** | **Individual writing of parameter** | **N＝6：type、No.、32 位元值（L→H，要先符號延伸）** | **N＝1：err** | **只是暫時改，要保存再送 7／2**；沒用到的參數一律設 0；值超出範圍回 data error | **A5II p.433；範例 p.419** |
| 7／2 | 27h | Writing of parameter to EEPROM | N＝0 | N＝1：err | 寫完才回；最長約 5 s；欠壓回 control LV 不寫 | A5II p.433；範例 p.419 |
| 7／6 | 67h | Individual read out of user parameter | N＝2：type、No. | N＝11h：type、No.、值 32、MIN 32、MAX 32、property 16、err | property：not in use、display inhibited、change at initialization、read only | A5II p.434 |
| 7／7 | 77h | Read out of two or more user parameter | N＝10h：8 組（type、No.） | N＝81h：8 組（type、No.、值、MIN、MAX、property）、err | | A5II p.435 |
| 7／8 | 87h | Writing of two or more user parameter | N＝30h：8 組（type、No.、值 32） | N＝11h：8 組（type、No.）、err | 沒用的設 0 | A5II p.436 |

- 「parameter type」手冊沒定義；HT9045 用 `type=00h、No.=0Dh` 存取 Pr0.13（golden 0618 rs232.cpp:1378、:1426），p.419 範例 `type=00h、No.=02h` → 推定 type＝類別號（Class）。

### ALARM（command 9）

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 說明 | 頁 |
|---|---|---|---|---|---|---|
| 9／0 | 09h | Read out of present alarm data | N＝0 | N＝3：alarm No.（main）、（sub）、err | 沒警報時 alarm No.＝0；代碼見 A5II 第 6 章 Protective function（p.358 起） | A5II p.436 |
| 9／2 | 29h | Batch read out of alarm history | N＝0 | N＝1Dh：最近 14 筆（main、sub）、err | | A5II p.437 |
| 9／3 | 39h | Clear of user alarm history | N＝0 | N＝1：err | 失敗回 data error；欠壓回 control LV | A5II p.437 |
| 9／4 | 49h | Alarm clear | N＝0 | N＝1：err | 只清得掉能清的 | A5II p.437 |
| 9／B | B9h | Absolute clear | N＝0 | N＝1：err | 清絕對編碼器錯誤與多圈資料；不是 17 位元絕對型回 command error | A5II p.438 |

## 2. A4（A4中 p.290–304）

A4中的中文字無法從文字層解出；下表的 command／mode、N、資料欄位是從英數字讀出來的，**名稱**只在能確認時寫（A4中有英文、或結構與 A5 同碼同長度時標「推定」）。

| cmd／mode | 位元組 | 名稱 | 送 | 回 | 頁 |
|---|---|---|---|---|---|
| 0／1 | 10h | 讀 CPU 版本 | N＝0 | N＝3 | p.290 |
| 0／5 | 50h | 讀驅動器型號（例 "MADDT1503***"） | N＝0 | N＝0Dh | p.291 |
| 0／6 | 60h | 讀馬達型號（例 "MSMD012S1***"） | N＝0 | N＝0Dh | p.291 |
| 1／1 | 11h | RS232 協定參數 | N＝3：T1、T2、RTY＋M/S | N＝1 | p.291 |
| 1／2 | 21h | RS485 協定參數 | N＝3 | N＝1 | p.292 |
| 1／7 | 71h | 取得／釋放執行權 | N＝1：mode | N＝1 | p.292；範例 p.286 `01 01 71 01 8C` |
| 2／0 | 02h | 讀狀態 | N＝0 | N＝3：control mode、status、err | p.293 |
| 2／1 | 12h | 讀指令脈波計數 | N＝0 | N＝5 | p.293 |
| 2／2 | 22h | 讀回授脈波計數 | N＝0 | N＝5 | p.294 |
| 2／4 | 42h | 讀目前速度（r/min，16 位元） | N＝0 | N＝3 | p.294 |
| **2／5 ★** | **52h** | **讀目前扭力（「＝2000」、16 位元）** | **N＝0** | **N＝3：L、H、err** | **p.294** |
| 2／6 | 62h | 讀位置偏差（pulse，32 位元） | N＝0 | N＝5 | p.295 |
| 2／7 | 72h | 讀輸入訊號 | N＝0 | N＝5 | p.295 |
| 2／8 | 82h | 讀輸出訊號 | N＝0 | N＝7 | p.296 |
| 2／9 | 92h | 讀速度、扭力、偏差 | N＝0 | N＝9 | p.297 |
| 2／A | A2h | 讀狀態＋輸入＋輸出 | N＝0 | N＝0Dh | p.297 |
| 2／C | C2h | 讀外部光學尺 | N＝0 | N＝0Bh | p.298 |
| 2／D | D2h | 讀絕對編碼器 | N＝0 | N＝0Bh | p.299；範例 p.285 |
| 2／E | E2h | 讀外部尺偏差與脈波和 | N＝0 | N＝9 | p.299 |
| **8／0 ★** | **08h** | **個別讀參數** | **N＝1：參數編號（00h–7Fh）** | **N＝3：16 位元值 L、H、err** | **p.300** |
| **8／1 ★** | **18h** | **個別寫參數** | **N＝3：參數編號、16 位元值 L、H** | **N＝1：err** | **p.300；範例 p.286 `03 01 18 0B 00 00 D9`** |
| 8／4 | 48h | 寫 EEPROM | N＝0 | N＝1（欠壓 LV） | p.300；範例 p.286 `00 01 48 B7` |
| 9／0 | 09h | 讀目前警報（推定） | N＝0 | N＝2 | p.301 |
| 9／1 | 19h | 名稱無法解出 | N＝1 | N＝3 | p.301 |
| 9／2 | 29h | 批次讀警報歷史（推定） | N＝0 | N＝0Fh | p.301 |
| 9／3 | 39h | 清警報歷史（推定；有 LV 錯誤＝要寫 EEPROM） | N＝0 | N＝1 | p.302 |
| 9／4 | 49h | 清警報（推定） | N＝0 | N＝1 | p.302 |
| 9／B | B9h | 絕對值清除（推定；頁上有「17bit」） | N＝0 | N＝1 | p.302 |
| B／0 | 0Bh | 名稱無法解出（參數類） | N＝1 | N＝9 | p.303 |
| B／1 | 1Bh | 以「頁」批次讀參數（推定：送 page No.，回 No.0–No.0Fh 的值） | N＝1：page No. | N＝82h | p.303 |
| B／2 | 2Bh | 以「頁」批次寫參數（推定） | N＝21h：page No.＋16 個值 | N＝2：page No.、err | p.304 |

## 3. HT9045 送的指令逐一對照

| golden 0618 rs232.cpp | 封包 | A4 | A5 | 手冊 |
|---|---|---|---|---|
| :833、:909-911 `datatrq` | `00 axis 52 (AE−axis)` | 2／5 | 2／5 | A4中 p.294；A5II p.427 |
| :1390-1404（A4）／:1375-1389（A5） | `01 00 08 5E sum`／`02 00 07 00 0D sum` | 8／0 Pr5E | 7／0 Pr0.13 | A4中 p.300；A5II p.433 |
| :1442-1457（A4）／:1423-1441（A5） | `03 00 18 5E L H sum`／`06 00 17 00 0D L H 00 00 sum` | 8／1 Pr5E | 7／1 Pr0.13 | A4中 p.300；A5II p.433 |

- 扭力回覆的解碼（:1801-1813）只認 `BufferLength==7`（A5 也接受 9，因為 A5 讀參數回 9 個位元組）；A5 寫參數的高兩個位元組固定送 0（:1430-1431），所以只能寫 0–65535，扭力限制 0–500 夠用。
- 參數讀回值：A4 是 16 位元、A5 是 32 位元，golden 一律只取 `str[3]`、`str[4]` 組 16 位元（:1804-1806）。

## 4. 要加新指令時

1. 從 §1 查 command／mode、送幾個參數、回幾個參數；組 `N, axis, (mode<<4)|command, 參數…, checksum`（檢查碼見 `protocol.md` §8）。
2. 回覆長度＝N+4；golden 的 `Comm1ReceiveData` 只認長度 7（與 A5 的 9），而且**不分指令**——長度 7 的回覆都會被當成扭力解碼（:1801-1814）。加新指令要先把解碼改成依「最後送出的指令」分流，否則 2／0（7 個位元組）之類的回覆會寫進 `Torque[]`／`asReceiveTorue`。
3. 讀警報建議用 9／0（A5II p.436）；清警報 9／4（p.437）；狀態 2／0（p.426）；要保存參數才送 7／2，並照 p.419 先取執行權（1／7）。
4. 驗證時用 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_rs232_torque.cpp` 的假伺服器模式（它照 A5 協定回話）擴充。
