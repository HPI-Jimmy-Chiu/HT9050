# Sim 佇列與 SRQ 紀錄

定位 `TesterWrite`、`SetTalkAddressed`、`TakeBridgeWrites`、`TakeSrqBytes`、`ibrsv`，對照 [manifest](../source-manifest.json)。

| 定位 | 選讀來源行為 |
| --- | --- |
| TesterWrite(cmd) | in_.push_back(cmd)，再Recompute |
| SetTalkAddressed(on) | talk_=on，再Recompute |
| ibrsv 的v | srq_.push_back(v)，Recompute後返回sta_ |
| TakeBridgeWrites | 新空vector與out_做swap，返回v；out_取得空容器 |
| TakeSrqBytes | 新空vector與srq_做swap，返回v；srq_取得空容器 |

兩個Take函式沒有重新計算sta_、err_或cnt_。TesterWrite／SetTalkAddressed／ibrsv會重新計算sta_，但Recompute本身保留err_／cnt_。本輪沒有核對全體使用者、同步保護或序列化條件；SRQ只是此Sim的整數紀錄，實際匯流排／Tester行為另查。

回 [Sim 索引](index.md)、[狀態](state.md)、[界線](../limits.md)。
