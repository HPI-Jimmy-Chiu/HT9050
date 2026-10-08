# 共用入口、差異與查證界線

本層仍放在同一 Handler UPH Skill，HT9050 與其他 Handler 不另建鏡像。已讀十二個 body 沒有 MachineTypeChoice 分支；只表示這些選讀段使用共同入口，不能推成全機型設定、COM 配置或測試流相同。

| 軸 | 本層已讀差異 | 尚待證據 |
|---|---|---|
| 機型 | tick／bridge／Aux／TComm 共用符號 | HT9050／9045／9046 的完整 caller、機台實際 COM 與 recipe |
| 客戶 | Aux 七客戶預設及 INI；CloseGpibProgram MTI／PTI 的 iLotStatus | CUSTOMER_CODE 與 CustomerCode 的完整來源、其他 caller |
| 模式 | SPEA #if 0、TCP ON_LINE、GPIB／OffLineGpibWay、RS232、TTL card／視窗 guard | 模式切換、relaunch、SPEA 外部介面 |
| 平台 | TComm 的 _WIN32、明確 SIM 與開 COM fallback | 實際 driver 與 reader／writer 成功、錯誤清理 |
| 版本 | V906 UTF-8 來源 pin 與 body hash | V912 Big5 的完整對應及量產行為，不把移植註解改稱重驗 |
| 量測 | poll／連線旗標／等待值與 UPH 顆數分開 | 站點容量、封包 ABI、實機 UPH、S8 發布語意 |

同題 [機型樹](../../../../../machines/index.md) 保留 HT9050 Hot／Ambient 與其他 Handler 的模型差異。本層只補 transport caller；動畫、模型預設及歷史機台毫秒數不轉成當前實測結果。

原文中的 golden 檔案行號與裁決註解僅是歷史定位，完整保存在 [manifest](source-manifest.json)；活摘要用檔名、function、變數。
