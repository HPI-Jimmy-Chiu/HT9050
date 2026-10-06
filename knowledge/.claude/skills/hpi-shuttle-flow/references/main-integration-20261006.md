# 最新main整合紀錄

20261006 MR drift清理：本分支已整合main `3aa5abaf322b6169ecf7e1009623ecddc6c7903b`。原文、原Claude metadata與資源保存；各reference原本標註的讀碼版本仍是該段查證基準，不因Git合併自動變成全機型／實機驗證。

來源定位用版本／檔名＋function／特定變數，Task／case輔助。舊程式行號僅保留在歷史原文。推送前檢查main為ancestor、本機連結／錨點與保存內容；後續Skill共用!263，合併後從最新main開下一張。

## W-44後續裁決

RULINGS_20261006第19條（Frank 15:3x，1A2A3A）補充：一側Shuttle進socket時，另一側在安全區即可放行；安全區先取原點往socket方向1000，方向按sign(Right−Left)。M108在原點＝原點燈亮且encoder在±100，disabled軸視為通過。這是新裁決，實作歸Frank的frank-nb2-2；本次合併main不代表該分支已生效。舊「另一側原點±100且停住」仍是原版程式的查證描述，不能當最新裁決，也不以1000改寫Z1±10的不同guard。Steven原裁決正文完整保留。

來源：[1006裁決](../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261006.md)第11／19條；[Shuttle原碼](../../../../HT9011UC_Cpp_V3.33.906.0/acarry.cpp)：Do_Auto_InSH／Do_Auto_OutSH與W906_Ht9050* helper。

## 樣板變更的回查範圍

樣板／共用規則若變更，回查 Shuttle、Index、Tray、Motor Home、Motor Control、Temperature、EP、MotionView 八個入口的 references 分層、共同／機型差異／執行期界線、function／變數定位與原文保存驗證；整批 !263 的 InArm、OutArm、IO、Bin Display 同步回查。舊入口、metadata、歷史原文及資源持續保存，部署／caller 盤點前不刪舊名。
