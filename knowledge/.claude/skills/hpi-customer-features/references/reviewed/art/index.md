# ART：客戶條件、caller與機型

同一主題整合HT9050與其他Handler，先讀[客戶分支](customer-branches.md)，再讀[caller／效果／機型](callers-effects.md)。下表的共同項限於已比對的函式條件，不能由CC符號推定某台已安裝ART、HANA介面或tester。

| 層次 | 共同項 | 差異與界線 |
|---|---|---|
| 客戶條件 | PTI的B03等待與RunMode保留、AMKOR的Tag 0呼叫 | HANA的Tag 0條件在兩版DoAutoRetest case 300不同 |
| 活動caller | AutoRetest執行需ART模式／bART_needRT2 | V912在DoAllProcess內；V906在W906_DoAllProcessLadder內，舊副本為字面#if 0 |
| 機型 | ART函式另受入／出料手臂安全位置與流程狀態守衛 | V906的馬達試運轉可在ART ladder前返回；後續HT9050 Shuttle dispatch不等於ART禁用條件 |
| 實際效果 | 呼叫Clarn_Data與SetLotState需再追被呼叫函式 | hook、停用清計數旗標、bCTClear、tester類型／bridge與字面gate影響實際效果 |

來源與核對日期見[manifest](../source-manifest.json)。這些是靜態查證；仍未核對各現場設定、完整上游排程及硬體時序。
