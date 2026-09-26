# 給機台端 Claude：更新包 22（GitLab main `d1bd26ad`，相對更新包 21 `71eda9b5`）

> 筆電端 Claude 20260926 22:4x 產生。**先套更新包 3～21，再套這一包。要不要套由 Jimmy 決定。**
> 6 檔（`tools/wb_serve.cpp`、`forms/fMain.cpp`／`fMain.h`／`cBinSel.cpp` 只改註解、兩份文件），底稿 `base_71eda9b5\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

1. **開機紀錄 BootLog 照 golden 接上**：golden `WinMain` 會寫 `D:\HT9045\Error\BootLog.txt`（「24V／硬體沒好時查當機用」，超過 512 KB 輪替成 `.old`），
   移植樹以前從來沒寫。現在 wb_serve 在三個點各寫一行：`WinMain Enter`（開機第一步）、`All CreateForm Done, before Application->Run`（進主迴圈前）、
   `Application->Run returned (normal exit)`（正常結束）。**開機當掉時，看這個檔停在哪一行，就知道當在哪一段。**
2. 其餘是註解更正（`forms/fMain.cpp`／`fMain.h`／`cBinSel.cpp`、wb_serve 一處），編譯器看到的碼不變。

## 你們機台上看得到的差別

* `D:\HT9045\Error\BootLog.txt` 會出現，每次開關 wb_serve 各多三行（沒走到「normal exit」那一行＝不是正常結束）。

## 在機台上要看的

1. 開一次 wb_serve、正常關掉：BootLog 三行都在，時間合理。
2. 不會讓馬達動、不會改 IO。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled；一次 MachineMotors_HT9050 BAD_COMMAND，單獨重跑通過），子檢查 0 差異；
模擬組態 wb_serve 實跑 15 秒，BootLog 三行都在；跑完 system／config／IniData 照快照還原、MD5 對回。
⚠ 附帶量到（不是這一包造成的）：wb_serve 內嵌的 GPIB 引擎關站時會照 golden GPIB 程式寫 `D:\GPIB9045\system\general.ini`（`LastFile=`）、`GpibString.dat`、`D:\GPIBLOG\Log\`。
