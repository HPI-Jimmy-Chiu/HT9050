# 代登記：ht9050-construction 的 todo／done／decisions

檔案都在 `D:\HT9045\.claude\skills\ht9050-construction\references\`，UTF-8 無 BOM、**CRLF**，分支 `v906/steven-cbridge-review6`。

## todo.md／done.md

- ID `<分類>-<三位數>`，A–J 分類（A 需求、B 機構、C 電控、D C++、E HTML、F 橋接、G 設定與 Recipe、H 外部整合、I 安全、J 驗證）。
- todo 欄：ID｜工作｜狀態｜負責｜風險｜依賴／阻塞｜完成證據｜下一步（awk -F'|' 的 NF＝10）。
- done 欄：ID｜完成項目｜最終狀態｜日期｜commit／檔案｜驗證證據｜環境／限制（NF＝9）。
- 狀態：`INTEGRATING`、`SPEC_ONLY`、`BLOCKED`、`NOT_STARTED`、`UNKNOWN`、`IMPLEMENTED`（程式已在、證據不齊）。
- **沒合入 main 的不進 done**（留 todo 標 IMPLEMENTED）；合入 main 才整列搬 done（todo 那列刪掉，done 註明「原 todo X-0nn」）。
- ctest 證據算 IMPLEMENTED；有環境相符的實測才標 VERIFIED。
- 驗證型項目（瀏覽器看過、跟 BCB6 對照、實機）證據沒有就留在 todo，即使程式已進 main（例 E-005、H-010）。
- 從沒進過 todo 的完成項目，用 x1xx 編號進 done（例 E-102、H-105）。
- RULINGS_20260927 第 4 條：0927～0928 兩組態全量 gate＝基準＋模擬驗證通過就算完成。

## decisions-pending.md／decisions-decided.md

- 20260927 從 todo.md 的 ★ 節拆出（Steven 要求）：**pending＝還要 Steven 回的**，**decided＝已裁決或已照建議做**。
- 題號：**Q**＝St01 要 Steven 決定；**R**＝St01 已照建議先做、可推翻；**W**＝St02 的題（照 St02 `progress-st02.md`「待使用者裁決」順序，選項大小寫照原表）。
- 維護分工：**Q／R＝ST01-E**（同時記 St01 分支的 `HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md` S 編號）；**W＝ST01-M**。
- Steven 回了：該題整段從 pending 搬到 decided，「目前狀態」改成 `**已裁決（Steven 日期時間，在哪邊，誰轉述）**：「原話」⇒ 意思`；方向已定但還要查證的寫「方向已定」。
- 兩個檔 ST01-E 也在改：動之前照共用工作樹規則先問，改完回 hash；它的工程師在改時不要同時改。
- **寫法（Steven 20260927 21:xx：「給我的決策文件裡面，相關的檔案都要使用絕對路徑，不要使用代號；功能也是要白話說明，淺顯易懂的方式」）**：
  - 檔案一律完整絕對路徑（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\...`、golden `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\...`、`D:\HT9045\web\page\...`），不寫只有 `:行號` 或 `tools\wb_serve.cpp` 這種半路徑；程式在別人分支、這台沒有的，寫明「在 St02 的分支，St01 這台還沒有」。
  - 不拿代號當說明（TS-8、CC-E2、S-a、D-f、N25-3、O07、HTSET,354…）：先用白話講它是什麼，代號放括號。
  - 每題用這幾段：**這是什麼功能**（機台／畫面上做什麼、操作員看到什麼）→ 問題或影響 → **選項** → **建議** → **例子**（每個選項會發生什麼）→ 目前狀態。範例：decisions-pending 的 W36～W41（`6dca4e00`）。
  - St02 送來的題也照這個寫法改寫後再登記（St02 已知會，FROM_STEVEN §4 20260927 21:20）。
- 語言：這兩個檔與 todo／done 用繁體中文（UTF-8）；FROM_STEVEN 與給 Jimmy 的列可以英文（RULINGS_20260927 第 8 條＋Steven 18:1x）。

## 登記時常見的連帶

- St02 的決定牽涉 St01 的檔（例 W11 放 St01 的 StartCondition.cpp）→ 先問 ST01-E 誰寫、怎麼接，再回 St02。
- Steven 的裁決跟 Jimmy 有關（改 V912、底層溫控、sysguard 預期變動）→ FROM_STEVEN §3 知會。
- RULINGS_20260927 第 1 條：HT9050 專案**只改 906 C++**，不動 V912／V899；原本「請 Jimmy 在 V912 一起修」的選項要重問。
