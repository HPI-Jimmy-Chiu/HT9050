# 給機台端 Claude：更新包 30（GitLab main `b635f32d`，相對更新包 29 `3f166785`）

> 筆電端 Claude 20260927 03:1x 產生。**先套更新包 3～29，再套這一包。要不要套由 Jimmy 決定。**
> 11 檔（程式 7、工具 1、文件 3），底稿 `base_3f166785\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

1. **良率計算**（新檔 `cmydef_TotalYield.cpp`、`CMakeLists.txt`、`cmydef.cpp` 註記、`ProductionInfo/uPAT_Function.cpp` 註記）：golden `GetTotalYield_double`／`GetTotalYield_Str` 放進活的檔。
   PAT 即時報表會呼叫它，但以前整棵樹都沒有定義；PAT 今天沒連進 wb_serve，所以**機台上沒有差別**，只是把一個潛在的連結錯誤先補好。
2. **INITMEM 檔頭註解改正**（`cmydef_InitialMemory.cpp`，只改註解）。
3. **三個解閘標記的 golden 行號更正**（`aTester_Rear.cpp:634`、`cinitial.cpp:7363`、`:13670`，只改整行註解，剝掉註解後逐字相同）。
4. **完成度普查工具**（`tools/census/census.py`）：數大括號前先拿掉註解與字串等（只影響量出來的百分比，不進建置）。
5. 測試：`InitialMemory` 多一節（19 條）。

## 你們機台上看得到的差別

* **沒有差別**（wb_serve 行為不變）。

## 在機台上要看的

1. 重新建置後 ctest：`InitialMemory` 19／19 通過。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查只多新測試，system＋config＋IniData 0 變動。
