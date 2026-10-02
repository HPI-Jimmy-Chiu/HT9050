# 普查 129「空砲彈」清單（筆電 20261001 01:2x～06:2x 產出；狀態更新到 12:0x）

> St02 1001 11:17 要求放一份到 git，讓大家自己挑還沒人接的項目。這是筆電 scratchpad `census129\` 的報告與表格複本，**唯讀參考**。
> 表格裡的行號是產出當時（main `f528311a` 前後）量的，**動手前請在目前的 main 重新核對**（之後幾批都是同一行改，行號大多沒變，但不保證）。
>
> 規矩跟其他工作卡一樣：**挑了先在自己的 `FROM_*.md` §1 認領（寫檔與行號）、先推再做**；別人的檔先問。
> 照 golden 翻、附 ctest、兩組態 gate（RULINGS_20261001 第 0 條：「照golden接上，現在所有功能都要接上」）。

## 這是什麼

「空砲彈」＝按了回 ok、畫面也沒報錯，但底下什麼都沒做（INBOX 129，使用者 0930 19:5x）。五類：

| 類 | 意思 | 檔 | 規模 |
|---|---|---|---|
| (a) | 函式本體是空的或只回常數，而 golden 同名函式有本體 | `census129_ab.md`／`.tsv` | (a)(b) 合計 944 列；人工讀 114 列；確認 56 個真的空砲彈 |
| (b) | `#if 0`／`GATE`／`TODO(W…)` 閘住的 golden 行，閘的理由已經過期 | 同上 | 同上 |
| (c) | 網頁送出的命令在 C++ 沒有分派、或分派到替身 | `census129_cd.md`／`.tsv` | (c)(d) 合計 975 列；信心 ≥70 的 34 列 |
| (d) | 全域旗標只有定義、沒有任何寫入點 | 同上 | 同上 |
| (e) | golden 有的物件／計時器／開機步驟，移植樹沒建立或沒跑 | `census129_e.md`／`.tsv` | 360 列（STATE 194、IO 輸出 92、通訊 34、寫檔 15、畫面 13、運動 12） |

每份 `.md` 開頭都有「怎麼讀 TSV」與分母；`.tsv` 是 UTF-8、tab 分隔，第一列是欄名。

## 目前狀態（1001 12:0x）

### 已在 main

| 批 | 項目 |
|---|---|
| 第十五批 | (c)(d) D1-006 `TempFuseLimitType`（加熱保險絲上限）；Motor Test 非 1203 軸回原點 |
| 第十六批 | (a)(b) #13 三溫機安全門 6 鎖、#17 急停通知 ATC（含 G-ATC-A／C）、#11 `BinCount.txt`、#5 Auto 盤氣缸放開、I41 空 socket 檢查、#7 上料氣缸預推 |
| 第十七批 | (e) E-T1-012 安全門鎖跟 SystemStart（含 Magazine 門）、E-T2-001 大風扇；(a)(b) #16 `TfMain::SetLotState` |
| St01 | (e) E-T3-004 [A01] 閒置自動切回 Operator（D-015，`FileRW/Main_A01AutoLogout.cpp`） |

### 有人在做／已派

| 項目 | 誰 | 在哪 |
|---|---|---|
| (a)(b) #6 冷卻風扇（`DoSwCoolingFan` 一族） | St02 | RULINGS_20261001 第 9 條；FROM_STEVEN §1 10:37 |
| (e) E-T2-008／E-TH-001／E-RT-007 加熱鏈（`Index16Heater`／`IndexHeatMode`／`HotplateHeatMode`／`SetTemp`、加熱執行緒、溫控器通訊層） | Ifor01 | `TO_IFOR.md` I-01（第一階段已認領）、I-03 |
| (e) E-FT2-011 換日時 JAM 次數存檔（`SaveJamRateByDay`） | 筆電第十二批做了 `cprod.cpp` 的掛勾；`FileRW/MainClose.cpp` 那一半是 St01 的 | TO_STEVEN §4（D-028） |
| (e) E-TM-002～-007：golden `TimerESDTimer` 整支 | St02 | `TO_STEVEN.md` §3 S-13 |
| (e) E-T3-000～009：golden `Timer3Timer` 剩下的段落（[A01] 除外；soak 倒數跟 Ifor01 對） | St02 | S-14 |
| (e) E-TM-008／-013／-014／-015：golden `Timer8Timer`＋`TimerTemperatureStorageMinuteTimer` | St02 | S-15 |
| (a)(b) #15 `RecordSafeDoorStates`、`csystem.cpp` 的 W7C1／g4 替身；(e) E-BOOT-005 `RunInfo.Factory`；另 `TfMain::Pause`（St01 D-032）、SECS RCMD REMOTE_START | 筆電第十八批 | TO_STEVEN §1 |
| (e) E-BOOT-002 開機 Servo On（1203 軸怎麼辦） | 等 Jimmy | NIGHT_REPORT §0 第 22 項 |

### 還沒人接（舉例；要接請先認領）

- (c)(d)：C3-005 Abort Home 的網頁接線、C3-006、C3-008、C3-119、D1-008～D1-013（筆電排著但還沒開工，要接可以先問）
- (e)：E-FT2-001 OLP host 連線（PTI／MTI／Greatek）、E-T2-003／E-BOOT-003 EP 調壓閥／ADAM-6024、E-T1-015 Index 吸嘴暫停時的收尾、E-FT1-001 警報框開著時 golden 在跑的工作、
  E-TM-001 面板按鍵掃描、E-TM-009～-012 Timer4～Timer7、E-TM-016 Timer9、E-TM-017 TimerDLL、E-IO-* 202 列（機台 IO 表比程式新，多數是表與程式的版本差，不是移植漏掉的）
