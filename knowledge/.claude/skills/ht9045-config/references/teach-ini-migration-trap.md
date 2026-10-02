# teach.ini 升版陷阱：Shuttle 教點 section 拼字分岔（MInShutte1 vs MInShuttle1）

> 首例：星科（JSCC）HXHT-O-101-9045，V3.32.731.2 → V3.33.908.5 升版後
> 「In Arm 放料到 Shuttle 報 Program Error in SetShuttleStatus」＋「in shuttle sensor 偵測點位偏移」
> 建檔 2026-09-02 (RogerYang)

---

## 1. 一句話

Handler 的 teach 點從 `tech.dat`（二進位）遷移到 `teach.ini`（文字）之後，
**Shuttle 1/2 的 section 名稱在歷史版本間分岔成 `MInShutte1`（拼錯，少一個 l）與 `MInShuttle1`（正確）兩種**。
當一台機的 teach.ini 裡**兩個 section 同時存在**、而拼錯那組是全 0 時，
908.x 的相容層會**優先讀拼錯那組**，於是 Shuttle 教點整組被讀成 0。

---

## 2. 為什麼拼字是錯的卻不能直接改

`cinitial.cpp` `InitialMotorName()`：

```cpp
MOT[MInShuttle1].SetAlias(11, "MInShutte1");   //不可以改成MInShuttle1, 會造成Teaching異常
MOT[MInShuttle2].SetAlias(12, "MInShutte2");
```

- 全專案**只有這一處** `SetAlias`，不會被 `Mot_Table.csv` 覆寫（Mot_Table 只決定軸號 `iMot`）。
- `MOT[].Alias` **同時被當成 teach.ini 的 section 名**。
- 所以直接把拼字改正 → 現場既有 `[MInShutte1]` 的機台全部讀不到教點。**改 Alias 必須配套 migration。**

---

## 3. 讀寫邏輯（V3.33.908.x）

`uteach.cpp` `TECH_PARA::ReadFromFile()`（Steven 20250926 加的相容層）：

```cpp
if(MOT[MotorSelect].Alias=="MInShutte1")
{
    if(CheckSectionExist(asTeachPath, "MInShutte1"))
        read "MInShutte1";        // ← 只判「section 在不在」，不判內容有效性
    else
        read "MInShuttle1";       // fallback
}
```

`SaveToFile()` 同樣的優先序（存在就寫拼錯那組，否則寫正確那組）。

`uteach.cpp` `TfTeach::ReadFile()` 的整體流程：

```
CheckKeyExist("Teach INI","Update2") ?
  否 → bUseIniFile=false
       → ReadData("d:\HT9045\system\tech.dat", &Tech.iZLoad, sizeof(TECH))   ※回傳值沒被檢查
       → 全部 SaveToFile(true) 寫回 teach.ini
       → 寫 Update2=1
  是 → bUseIniFile=true
       → if(CheckSectionExist("MInShuttle1")==false || CheckSectionExist("MInShuttle2")==false)
             從 tech.dat 補救，只回寫 MInShuttle1/2 的 TechPara     ※只檢查「正確拼字」那組
       → 全部 ReadFromFile()
```

呼叫時機：**`main.cpp` `DoReadLastData()` 開機就會跑**（`fTeach->ReadFile()`），
以及 `TfTeach::FormShow()` 每次開 Teach 畫面再跑一次。不是只有進 Teach 畫面才讀。

---

## 4. 四種 teach.ini 狀態

| 狀態 | `[MInShutte1]` | `[MInShuttle1]` | 908.x 讀到 | 結果 |
|---|---|---|---|---|
| A | 無 | 無 | 從 tech.dat 轉檔，寫入 `[MInShuttle1]` | OK |
| B | 有值 | 無 | `[MInShutte1]` | OK（多數現場） |
| C | 無 | 有值 | fallback → `[MInShuttle1]` | OK |
| **D** | **全 0** | **有值** | **`[MInShutte1]` → 全 0** | **壞** |
| E | 有值(舊) | 有值(新) | `[MInShutte1]` → 過期值 | 潛在壞 |

29 台對照組實測：B=24 台、C=4 台、無=1 台、**D=1 台（本案）**。
A/B/C 都安全，**只有並存會壞**。

---

## 5. 壞掉後的實際影響（本案量化）

`[MInShutte1]` 底下 9 個 key 對應的 `Tech` 變數：

| key | Tech 變數 | 用途 | 該機真值 | 讀成 0 後 |
|---|---|---|---|---|
| `setEditOutSht1KitPos` | `Tech.OutSH1ZDetectPos` | **in-shuttle sensor 掃描基準** | 20458 | 被守門硬套 **20599**（偏 +141＝1.41mm）|
| `setEditOutSht2KitPos` | `Tech.OutSH2ZDetectPos` | 同上（Shuttle 2）| 20541 | 硬套 **20469**（偏 −72＝0.72mm）|
| `setEditInSht1Left` | `Tech.iInShuttle1Left` | Shuttle 1 左定位 | −82 | 0 |
| `setEditInSht1Right` | `Tech.iInShuttle1Right` | Shuttle 1 右定位 | 38423 | 0 |
| `setEditOutSht1OneRowKit` | `Tech.OutSH1ZOneRowDetectPos` | 單排 kit center | 20333 | 硬套 20565 |
| 其餘 | OctSiteKit / IS1BarCode / OS1BarCode / Sht1Laser / SH1_16SiteKit | | | 0 |

關鍵鏈路：

```
cinitial.cpp  SThreadPara.base_pos[0] = Tech.OutSH1ZDetectPos + 2000;   // shuttle 1 kit center point
cinitial.cpp  Prod.InSHT[0].iLeft  = Tech.iInShuttle1Left  + Offset... ;
cinitial.cpp  Prod.InSHT[0].iRight = Tech.iInShuttle1Right + Offset... ;
```

`base_pos[]` 就是 shuttle sensor thread 的掃描基準 → **「in shuttle sensor 偵測點位偏移」的直接來源**。

守門（`cinitial.cpp` `ReadTechData()`，條件 `CosFunction.bOutShuttleSensorCanNotDisable`）：

```cpp
if(Tech.OutSH1ZDetectPos==0) Tech.OutSH1ZDetectPos=20599;
if(Tech.OutSH2ZDetectPos==0) Tech.OutSH2ZDetectPos=20469;
if(Tech.OutSH1ZOneRowDetectPos==0) Tech.OutSH1ZOneRowDetectPos=20565;
if(Tech.OutSH2ZOneRowDetectPos==0) Tech.OutSH2ZOneRowDetectPos=20453;
```

→ 這個守門讓症狀變成「**偏一點點但機台還能跑**」，而不是直接停機，所以很難察覺。

---

## 6. 病態 section 是怎麼被追加進去的

`common.cpp` `CheckAndReadIniData(..., int Value)`：

```cpp
if(!INIFile->ValueExists(Group, Name))
    INIFile->WriteInteger(Group, Name, Value);   // ← 讀不到就把 default 寫進去，順帶建 section
else
    Value=INIFile->ReadInteger(Group, Name, Value);
```

**`CheckAndReadIniData` 有寫入副作用**：key 不存在時會建立 section 並寫入當下變數值。
若呼叫當下 `Tech` 尚未載入（全 0），就會在檔尾追加一整組 0。

`double` / `bool` / `AnsiString` 各版本同樣有此副作用（`WriteString` / `WriteBool`）。
**要「純探測」某個 key/section 存不存在，不可以用 `CheckAndReadIniData`。**

佐證：本案 `[MInShuttle1]` 在 line 177（跟其他馬達 section 排在一起＝正常轉檔順序），
`[MInShutte1]` 在 line 584（排在 `[Teach INI]` line 580 **之後**＝檔尾追加）。
INI section 順序反映建立順序 → 拼錯那組是**後來**才被加上去的。

推定成因：該機升級路徑上多經過一個中間版本
（介於 Steven 20250507「修改 group name」與 20250926「加相容層」之間），
那一版直接用 `Alias` 當 section 名存檔，而存檔當下 `Tech` 還是 0。

---

## 7. 現場診斷 SOP

1. 取 `D:\HT9045\system\teach.ini`
2. 檢查是否 `[MInShutte1]` 與 `[MInShuttle1]` **並存**（`[MInShutte2]` / `[MInShuttle2]` 同）
3. 判準用 **`setEditInSht1Right`**（正常值必為萬位數：38423 / 47385 / 45746…）
   - ⚠️ 不要用 `setEditInSht1Left` 當判準，它正常值就可能在 0 附近（−82 / 7 / 19 / 32 / 70）
4. 若拼錯那組 `setEditInSht1Right==0` → 命中本缺陷

批次掃描（PowerShell，唯讀）：

```powershell
$l = Get-Content -LiteralPath 'teach.ini' -Encoding Default
$g = ($l -contains '[MInShuttle1]'); $b = ($l -contains '[MInShutte1]')
if($g -and $b){ 'BOTH -> 檢查哪一組 setEditInSht1Right 為 0' }
```

---

## 8. 現場止血（零風險、可逆、5 分鐘）

1. 備份 `teach.ini`
2. **只刪 `[MInShutte1]` 與 `[MInShutte2]` 兩段**（含底下所有 key），保留 `[MInShuttle1]` / `[MInShuttle2]`
3. 重開 Handler，進 Teach 畫面確認 Shuttle 欄位有跑出正確值
4. 跑料觀察 MES0404 / MES0405 是否收斂

原理：section 不存在 → `CheckSectionExist` 回 false → fallback 讀到有值的那組。

### ⛔ 不要刪整個 teach.ini

刪整檔會走 `bUseIniFile==false` 分支，**從 `tech.dat` 重建**，兩層風險：

1. `tech.dat` 通常是舊快照（本案 tech.dat = 2026/3/23，teach.ini = 2026/8/30）
   → **五個月的 teach 調整全部退回**
2. `ReadData()` 的回傳值在 `uteach.cpp` **沒被檢查**；現場 `tech.dat` 大小實測有
   **2560 / 3672 / 3688 / 3728** 四種（`TECH` 結構跨版本大小不同）
   → 若 `sizeof(TECH)` 大於檔案大小，後半段欄位維持 0 再被寫進 teach.ini，**災情更大**

---

## 9. 程式修法（已實作 20260902 於 908.16_NB_AI，待編譯上機驗）

### 方案取捨

| 方案 | 舊版→新版 | 新版→新版 | 降回舊版 | 能否淘汰舊拼字 |
|---|---|---|---|---|
| A. 直接把 `MOT[].Alias` 拼字改正 | ❌ 打壞只有舊名的 24 台 | — | ❌ | — |
| B. 刪除無效 section、統一單一名稱 | ✅ | ✅ | ❌ 讀不到 | ✅ |
| C. 兩組永久同步、先有值者為主 | ✅ | ✅ | ✅ | ❌ **錯誤被固化** |
| **D. 遷移導向：讀兩組認、寫只認正確拼字** | ✅ | ✅ | ✅ | ✅ |

> ⚠️ C 案是第一版實作，被否決。原因：「哪組先有值就以哪組為主寫」會讓只有舊名的 24 台
> **永遠**以拼錯的 `MInShutte1` 為主，沒有收斂路徑 —— 那不是過渡，是把錯誤固化。
> **兩組並存本質是過渡狀態，舊拼字那組勢必要淘汰，設計必須帶方向。**

### D 案設計（實際採用）

核心是**把「Alias 當 section 名」解耦**：`MOT[].Alias` 維持原拼字不動（零風險），
只用它識別「這是哪一支 Shuttle」，section 名改由程式常數決定。

```
讀 : iFixed!=0        -> 讀 [MInShuttle1]     (已遷移 / 本來就正確)
     else iOld!=0     -> 讀 [MInShutte1]      (過渡：舊機台首次升版)
     else             -> 讀 [MInShuttle1]     (兩組皆無有效值)
寫 : 主  -> [MInShuttle1]  WriteIniData        (收斂方向，永遠寫這組)
     鏡像 -> [MInShutte1]  WriteIniDataNoLog   (★過渡，淘汰時刪這行)
```

只要開機跑過一次，有效值就會被寫進正確拼字組，**下次開機起讀取來源自動變成正確拼字** → 自然收斂。

### 改動點（`uteach.cpp`，由下往上）

| # | 位置 | 內容 |
|---|---|---|
| 1 | 全域區（`PitchX_Home_Index` 後） | `asShtTeachSecRead/Old/Fixed/ChkKey` + `GetShtTeachSecIndex()` + `DecideShuttleTeachSection()` |
| 2 | `TECH_PARA::ReadFromFile()` | 改讀 `asShtTeachSecRead[iSht]`，不再用 `CheckSectionExist` 決定 |
| 3 | `TECH_PARA::SaveToFile()` | 主寫 Fixed + 鏡像寫 Old |
| 4 | `TfTeach::ReadFile()` | 開頭呼叫 `DecideShuttleTeachSection()`；ReadFromFile 迴圈後補一次 Shuttle 同步 |

### 實作時踩到 / 必須遵守的點

- **探測 section/key 一律用 `ReadIniData`（純讀），絕不可用 `CheckAndReadIniData`** — 後者 key 不存在會 `WriteInteger` 建 section，正是病灶成因
- 鏡像寫入用 `WriteIniDataNoLog`，否則第一次同步會刷出一堆 ChangeLog
- `RecordProcess` 宣告在 `cMyDB.h`，而 `uteach.cpp` 沒 include 它 → 改用 `WriteIniDataNoLog` 把判定結果落到 `[Teach INI] ShtSecRead1/2`，零宣告風險且現場開檔即可查核
- 改動嚴格限縮在 `TECH_PARA`。已確認 `TechTwoPara` 沒有任何 `MInShuttle1/2`（全是 MInArmX/Y、MOutArmX/Y…），`TECH_SUCKPARA` 用固定 Group 不吃 Alias
- `TfTeach::ReadFile()` 內「`MInShuttle1` 不存在就從 tech.dat 補」那段**不要動** — 兩組並存的機台條件不成立，不會被舊 tech.dat 覆蓋

### 淘汰步驟（未來）

1. 確認現場機台都升過本版（`[Teach INI] ShtSecRead1/2` 皆顯示 `MInShuttle1/2`）
2. 刪除 `SaveToFile()` 內標 ★ 的鏡像寫入那一行
3. 可一併清掉 teach.ini 裡的 `[MInShutte1]` / `[MInShutte2]`

### Big5 檔案改寫注意

- 用 `.venv` python 以 `cp950` bytes 讀寫，PowerShell byte 寫入會被防毒擋
- 檔案是 **CRLF**，anchor 比對要用 `\r?\n` regex，否則 `\n` 比對必定失敗
- 改完必檢查**行尾 byte 是否為 0x5C** — Big5 雙位元組字第二 byte 若是反斜線，註解會續行吃掉下一行程式碼
- 改完必驗 `raw.decode('cp950').encode('cp950')==raw`

---

## 10. 教訓

- **`MOT[].Alias` 被當成持久化 key 用** → 改名等於改資料格式，必須配 migration
- **`CheckAndReadIniData` 有寫入副作用** → 拿它當「探測存在性」會製造出病態 section
- **相容層只判 section 存不存在、不判內容有效性** → 兩組並存時必然選錯
- **升版問題要先建對照組**：29 台一起掃，才能分辨「程式通病」與「這台的資料狀態」
- **section 在 INI 檔中的行號順序 = 建立順序**，是判斷「誰後來加的」的免費證據
