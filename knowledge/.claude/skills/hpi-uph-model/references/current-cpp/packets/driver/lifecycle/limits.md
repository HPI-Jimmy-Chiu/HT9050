# 同題分流與查證界線

| 分流 | 本層已查 | 尚未查 |
| --- | --- | --- |
| V906共用 | GpibEngine的driver注入／NI選擇與本地解除；[manifest](source-manifest.json)八函式 | 全部factory／注入／SetGpibDriver／GetGpibDriver使用者與thread／重入契約 |
| NI／Sim差異 | Start未注入就嘗試NI，注入分支沿用IGpibDriver；[NI／Sim實作](../implementations/index.md)另有來源證據 | 誰在SOFT_SIMULTE或ctest建立Sim、實際DLL／ABI與送達 |
| V912 BCB6 | 同題 [封包版本](../../versions.md)已有來源差異 | 本層未讀V912 engine／driver對應實作 |
| HT9050／其他Handler／客戶 | 本層選讀Start沒有機型／客戶的分支；保留同題路由 | 真正部署、上層TestType選擇、建置旗標／driver啟用與機型矩陣 |
| UPH整體 | driver返回／狀態與選擇局部證據互相銜接 | 全producer／consumer、容量site、實機UPH與校正 |

atomic指標的store／load與不delete注入指標不等於物件壽命已保證。header的單一TesterComm thread與注入存活前提保留為來源敘述，不能標成此次已驗證的全局契約。

沒有執行Start／Stop、DLL、C++、build或runtime，也沒有改來源碼／執行期設定。回 [本層索引](index.md)、[wrapper](../index.md)、[既有機型樹](../../../../machines/index.md)。
