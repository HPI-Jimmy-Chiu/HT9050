# 給機台端 Claude：更新包 16（GitLab main `39bd8f1e`，相對更新包 15 `c65ddd85`）

> 筆電端 Claude 20260926 20:0x 產生。**先套更新包 3～15，再套這一包。要不要套由 Jimmy 決定。**
> 7 檔，底稿 `base_c65ddd85\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：`ChangeATCSiteUse` 接上另外兩處（接著更新包 14 的 OPMODE）

| 檔 | 改了什麼 |
|---|---|
| `TempCtrl/TriTemp.cpp:339` | 三溫機（Tri_Temp_Machine）外觀上的 `ChangeATCSiteUse() {}` 空樁改成轉呼叫真的 `TfMain::ChangeATCSiteUse`（它的 5 個呼叫點從此照 golden 做 ATC 站點重映射） |
| `forms/fMain.cpp:507`／`:931`、`forms/fMain.h:1274`、`forms/fMain_OperateMode.cpp` 檔尾 | `TfMain::Home` 裡的 `ChangeATCSiteUse()`（golden main.cpp:7069）解閘，經 `W906_ChangeATCSiteUseHook` 呼叫（fMain.cpp 在 ht9045_forms，本體在 ht9045_sm）；wb_serve 開機的同一個安裝函式一起裝 |
| `tests/test_opmode_body.cpp` | 多一條：裝 hook 後 `W906_ChangeATCSiteUseHook` 也不是 0 |
| 文件 | INBOX、NIGHT_REPORT |

**HT9050 不是三溫機、也沒有 ATC 連線**，所以這一包在你們機台上的實際效果很小：`ChangeATCSiteUse` 在 `InitialOK==false` 時直接 return，
新 ATC 那一大段照 golden「沒連線就跳過」，`ATC_InterfaceForm` 的 `iATC_MODE_TYPE` 目前固定 0。

## 在機台上要看的

1. 全量重編後 ctest：`OpModeBody`（16 項）通過。
2. 回原點（Home）照常；log 沒有新的錯誤。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異；system＋config＋IniData 0 變動。
