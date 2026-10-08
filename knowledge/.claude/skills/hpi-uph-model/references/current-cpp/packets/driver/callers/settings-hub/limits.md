# 同題版本、機型、客戶與未查範圍

| 分流 | 本層局部證據 | 仍待續 |
| --- | --- | --- |
| V906 共同 | PublishSettings、Aux 九 body、Hub 六 body、thread 四 body、三 header | 全 producer／consumer、public helper caller、例外與併發 |
| HT9050／9045／9046／其他 Handler | 所選二十 cpp body 無 MachineTypeChoice／Model 分支；共同用 TestIF／IniConfig／InitialOK／SystemStart | 不代表全樹或現場無差異；核對 [機型樹](../../../../../machines/index.md) 的實際 recipe、port／模式；不推定 HT9050 site／容量 |
| 客戶與 INI | PortOn 七個客戶常數作預設，INI 的 RS232 可覆蓋；Prepare 帶 CUSTOMER_CODE | header 歷史數字未核對為當前代碼矩陣；不能由其他 helper 無客戶分支推定全流程相同 |
| V912／BCB6 | 保留 golden 路徑、歷史裁決與行號註解 | 未選讀 V912 完整對應 body；不能套用 V906 的 atomic／thread／seed |
| NI／Sim／COM | 有效 GPIB、實際 TestIF、enabled、PortOn、driver 注入分開 | consumer LoadSetupData／開 COM、NI DLL、析構及 WbThread callee |
| UPH／容量／ABI | framing pack 與 heartbeat 都有獨立口徑 | 全 counter、機台 UPH 校正、容量／ABI、S8 發布語意未結案 |

來源可能呼叫 INI 寫入、備份、thread／driver；本次只讀 Git blob 做文件、hash 與引用查核，沒有執行這些函式、build／ctest 或機台程式，也沒修改 runtime。

manifest 的完整原文可能含歷史行號／裁決，保存但不作活文件主要定位。沿用 [engine 局部生命期](../../../../consumers/command/transport/mailbox/engine/index.md) 與 [mailbox](../../../../consumers/command/transport/mailbox/index.md) 各自界線，局部證據相加不代表全系統結案。

回 [索引](index.md)、[Aux](recipe.md)、[Hub](hub.md)、[thread](thread.md)。
