# Ifor01 的 V912 保留帳本（RULINGS_20261003 第 1 條）

> Ifor01（Ifor 的筆電 IFOR-NB2）。照 `RULINGS_20261003.md` 第 1 條：V912 是修正、或明顯比較好的地方保留 912，程式註解同時寫 golden 0618 與 V912 的行號，
> 並在「自己的帳本」記一列「為什麼算修正／比較好」（St02 是 `docs/ST02_GOLDEN906_AUDIT.md`，St01 是它的 registry）。這份是 Ifor01 的。
> 一列一件；欄位：日期｜卡／MR｜移植樹位置｜golden 0618｜V912｜為什麼保留 912｜依據。

| 日期 | 卡／MR | 移植樹位置 | golden 0618 | V912 | 為什麼保留 912 | 依據 |
|---|---|---|---|---|---|---|
| 20261004 | I-08（`v906/ifor-i08`） | `uTemp_Set.cpp` `ReadTempFile`／`spbSaveClick` 的 ATC 校正檔選擇（`AI(W906-I08)`） | `uTemp_Set.cpp:2976`／`:4428`：ATC（`eNewATCSystem`＋`bATCActiveCooling`＋`bATCUseTempAdjustment`）不分模式只用一份 `DefineTemp\Temperature_ATC.Data` | `uTemp_Set.cpp:3005-3012`／`:4467-4474`：Hot／AmbientHot 用 `Temperature_ATC.Data`，其他模式用 `Temperature_ATC_Cold.Data`（找不到時從 `Temperature.Data` 複製） | **修正**：冷測（例 -5～25℃，ATC_TYPE_36 可冷到常溫以下）跟熱測（例 85～150℃）的三點校正範圍差很多，一份表的基準點蓋不住兩段；而且移植樹的網頁溫度頁那條路（`FileRW/Temperature.gen.inc:4034-4041`／`:5532-5539`）本來就是 912 的分法——只改一邊會讓「開機／換配方／SetTemp」跟「網頁溫度頁」讀寫不同的檔，網頁上存的冷校正重開機後讀不到，而且沒有任何警告。舊機台升級的冷校正表：`docs/G031_FORMAT_DIFF.md`（1002）。同函式前一段 HeadChamber（0618 `:2921`／`:4388`）912 也沒分，照舊。 | RULINGS_20261003 第 22 條（#47＝A，Ifor 1002 08:4x 的建議）、第 1 條；TO_IFOR §4 1003 14:3x；ctest `AtcCalSplit` |
