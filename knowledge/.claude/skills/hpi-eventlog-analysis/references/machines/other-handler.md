# 其他Handler

## 差異

HT9045／HT9046等按實際BCB6或V906來源分流；站點／機構名稱不是ELA客戶碼。各機型分派、CSV產生與runtime差異本批未完整查證，不能記成與HT9050相同。

## 入口與流程

讀[版本](../versions/index.md)，核對來源與日期／資料欄位，再選[分析](../analysis/index.md)或[報表](../reports/index.md)。V912 SendCommand_EventLog正文已局部對照；所有啟動與呼叫點未查。

## 安全

BCB6 Big5／pre-C++11與V906 UTF-8規則分開；文件整理不修改來源碼、runtime或上傳設定，不把別台測試證據套用此台。

## 查證來源

[局部來源](../runtime/source-review.md)及[原文](../ela/original-entry.md)各自保留版本界線，更多機型／caller差異待查。
