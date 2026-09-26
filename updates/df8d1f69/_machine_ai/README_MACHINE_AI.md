# 給機台端 Claude：更新包 20（GitLab main `df8d1f69`，相對更新包 19 `04c72d84`）

> 筆電端 Claude 20260926 21:4x 產生。**先套更新包 3～19，再套這一包。要不要套由 Jimmy 決定。**
> 15 檔（12 支程式／測試＋CMake 兩支＋兩份文件），底稿 `base_04c72d84\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

1. 🔴 **修當機（NB2 R72 ATC-1）**：筆電 0926 的 OPMODE 把 `ChangeATCSiteUse` 換成真本體之後，`Gerneral.ini [ATC] USE_ATC_MODE=4`（Hontech ATC）、
   測試模式 Single／Dual 的機台，**換配方、登入、切運轉模式、按 HOME 都會當**（對空的 `ATC_SYS_PAL` 取 `[0]`）。
   現在開機照 golden `main.cpp:9357-9361` 先 `InitialATC` 建 4 個通道、設 `ATCIniPath`。
   **先查你們機台 `system\Gerneral.ini` 的 `[ATC] USE_ATC_MODE`**：是 4 的話，更新到這一包之前不要換配方／按 HOME。不是 4 就不受影響（這台筆電是 5）。
   修好之後**不會**開始跟 ATC 通訊（真正收送的計時器移植樹沒人驅動，另案）。
2. **Jam 次數**（St01 J2）：關警報框時照 golden 累加 `iJamCount`／`iDayJamCount`（以前永遠是 0）⇒ 網頁 prod 的 Jam 數、SECS SV 1036、MTBA／Jam rate 會開始有數字。
   條件照 golden：JAM 碼、正式跑、不是答 TRAY END、不是重複警報、Unit < 9。
3. **UPH 表**（J3）：`CalculateUPH` 寫進真的 `fShowBinSelect` ⇒ 遠端 UPH 查詢（`Command.cpp:3455`）不再回空的。
4. **one cycle 結束存各 Site 計數**（J4）：照 golden 寫 `D:\HT9045\system\Arm0～2.dat`、`ArmHis0～2.dat`、`ArmByLot0～2.dat`（含 `_backup.dat`／`.ini`）。
5. **Index 時間平均**（J6）、**測試中秒數 `iCurrentTime`**（J12，每秒 +1）。

## 你們機台上看得到的差別

* `USE_ATC_MODE=4` 的機台：換配方／HOME 不再當。
* 跑 one cycle 之後，`system\Arm*.dat` 的修改時間會更新（以前不會寫）。
* 警報框答掉之後 Jam 數會加（網頁 prod、SECS）。

## 在機台上要看的

1. **這一包不會讓馬達多動**（沒有動作流程的改變）。
2. 套之前**加備** `D:\HT9045\system\Arm*.dat`、`ArmHis*.dat`、`ArmByLot*.dat`（含 `_backup.dat`、`.ini`），以及 `system\ATC.ini`（`USE_ATC_MODE=4` 且主動冷卻時會照 golden 補寫缺的鍵）。
3. 讓一個 JAM 警報出現，用 RETRY 答掉：網頁 prod 的 Jam 數 +1；答 TRAY END 則不加。
4. 跑一次 one cycle：`system\Arm0.dat` 的時間更新，內容跟畫面上的各 Site 計數一致。
5. `USE_ATC_MODE=4` 的機台：開機後換一次配方、按一次 HOME，不當。

## 步驟

同前幾包：Check → EastSun 同意 → 備份（上面第 2 點）→ Apply → **重新建置** → ctest 與筆電比（新測試 `NoteJamCount`、`AtcBootInit`、`Timer2TestSeconds`）→ commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩輪兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled；一次 `InitDIOStstus` BAD_COMMAND，單獨重跑通過），
子檢查只有新測試不同，system＋config＋IniData 0 變動（ctest 的 Arm*.dat 寫入經 `W906_MACHINERECORD_DIR` 落在 scratch）。
**筆電沒有 ATC、沒有實跑 one cycle**，上面第 3～5 步要機台端驗。
