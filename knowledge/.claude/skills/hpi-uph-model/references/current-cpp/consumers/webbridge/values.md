# Grid取值與空白界線

所選三完整body文字與兩常數、一caller片段見[manifest](source-manifest.json)；以下只推本地語句，不推作用中機型、producer是否已更新或傳送成功。

`W906_UphLive(cust)` 回傳 `cust && fShowBinSelect != 0`。它是本地gate；`cust`如何取得、form／grid生命週期與鎖仍待查，函式名中的Live不是本批執行了LIVE。

`W906_StageUphStatusTags(snap,cust)` 在live時讀 `fShowBinSelect->UPH_StringGrid`，依 `RowCount`／`ColCount`逐列包裝：列間換行、格間tab；`W906_UphCellAppend`在每格內容裡把tab、LF、CR替成空白，直到字串終止。不是CSV檔案或重新計算UPH。

| 本地條件／定位 | `mainsb.uph`傳入值 | 界線 |
| --- | --- | --- |
| 初值；或`CC_ASE_KaohSiung`且`bG10ShowImmediateUPH` | `kMainSbUphBlank=-2` | 應用中的空白標記；不能當負產能 |
| 非上述customer支路，`Cells[3][0]=="UPH"`且`Cells[3][1]==""` | `kMainSbUphNoNumber=-1` | 有表頭但尚無最新數字，不能當零產能 |
| 同表頭、最新格非空 | `(long long)std::atoi(cell.c_str())` | 是表格字串轉值；沒有在這個支路補格式／範圍／新鮮度驗證 |
| 沒有"UPH"表頭 | 保持`-2`初值 | 不把註解中的首次計算敘述當本批所有caller已證實 |

`binsel.uph.grid`與`mainsb.uph`交給 `stageStr`／`stageInt`時valid實參為live；`binsel.uph.tabVisible`交 `stageBool`時為cust，值是 `IniConfig.bShowUPH`。tab是否顯示與UPH資料是否可用是兩個gate，不能互換。stage helper、snapshot匯出及頁面消費尚未查完。

本body直接consumer為grid，不直接讀 `iUPH_LoaderCount`或呼叫 `CalculateUPH`；原始註解的計算來源聲明保留，完整producer鏈仍回[計算樹](../../calculate.md)查。函式也處理indexTime與version latch，但其完整writer／caller與時序不在本地UPH結論中。

`W906_StageShowBinSelectTags`所選return區段呼叫 `W906_StageUphStatusTags(snap,cust)`；完整上層body僅保存hash、未稱所有caller派工／customer來源已驗。回[界線](limits.md)。

[V906／V912 Command consumer](../command/flow.md)空格回字串"0"，沒有本地表頭gate；與本段blank／no-number sentinel不同，不能互換。
