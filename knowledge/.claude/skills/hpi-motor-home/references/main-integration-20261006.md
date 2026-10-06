# 最新main整合紀錄

20261006 MR drift清理：本分支已整合main `3aa5abaf322b6169ecf7e1009623ecddc6c7903b`。原文、原Claude metadata與資源保存；各reference原本標註的讀碼版本仍是該段查證基準，不因Git合併自動變成全機型／實機驗證。

來源定位用版本／檔名＋function／特定變數，Task／case輔助。舊程式行號僅保留在歷史原文。推送前檢查main為ancestor、本機連結／錨點與保存內容；後續Skill共用!263，合併後從最新main開下一張。

## Index 1203類別分流

最新main已有W906_IDX1203_GALI_BRANCH定義。Motor/myGALILmotor.cpp::W906_IdxIs1203仍要求INDEX_MOTION_CARD非0、四個Index軸名之一、Motor存在且Enable、CardType為PCI1203；不成立則維持Galil路徑。W906_IndexMove與W906_Idx1203SingalHome在類別分流後用MotorMove／MotorHome，原Gali函式名稱不能推定必走Galil SDK。SetSpeed／GetMotorAlarm／PCIL132_StopMotor與W906_EcForeignStopResetFcmd也加入同條件；不得只看MTestZ1名字跳過1203的停止或fCMD清理。這是只讀函式核對，沒有替機台修改INDEX_MOTION_CARD、表格或runtime gate。

來源：[Galil／Index分流](../../../../HT9011UC_Cpp_V3.33.906.0/Motor/myGALILmotor.cpp)：W906_IdxIs1203／W906_IndexMove／W906_Idx1203SingalHome；[MyMotor](../../../../HT9011UC_Cpp_V3.33.906.0/Motor/mymotor.cpp)：SetSpeed／GetMotorAlarm／PCIL132_StopMotor；[Ecat route](../../../../HT9011UC_Cpp_V3.33.906.0/Motor/EcatMotorRoute.cpp)：W906_EcForeignStopResetFcmd。

## 樣板變更的回查範圍

樣板／共用規則若變更，回查 Shuttle、Index、Tray、Motor Home、Motor Control、Temperature、EP、MotionView 八個入口的 references 分層、共同／機型差異／執行期界線、function／變數定位與原文保存驗證；整批 !263 的 InArm、OutArm、IO、Bin Display 同步回查。舊入口、metadata、歷史原文及資源持續保存，部署／caller 盤點前不刪舊名。
