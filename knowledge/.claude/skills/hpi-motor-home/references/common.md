# Home 共同項與機型／驅動器差異

查證基準main `9d9dfa9c7`，20261006；原機台側20261003筆記保留日期，不能當成所有機型或今日main的有效設定。

## 直接核對的共用介面

`HT9011UC_Cpp_V3.33.906.0/Motor/mymotor.cpp`：

| 介面／狀態 | 共用責任 | 版本／路由差異 |
|---|---|---|
| MotorInitial | 重設iMyHomeTask、ResetTime與HomeFlag，未啟動運動 | V906硬體Motor為NULL時仍重設自身狀態，略過底層SetHomeobjectTask |
| Home | 呼叫Motor->HomeObject；本身沒有完整HomeFlag狀態機 | V906先檢查Motor NULL與安全門，實際home由drive／card route決定 |
| MotorHome | 單軸Task狀態機，控制成功／失敗與HomeFlag | 0未完成、1成功、2失敗；回傳值還有其他alarm／servo類錯誤，須追調用者 |
| THomeFlag／HomeClass | 全機編排與每軸順序，不等同MOT[].HomeFlag | 原版資料驅動註冊須維持HomeClass[i].index==i，機型旗標影響Visible／順序 |
| fAllMotorHome | 全機允許運轉的上層狀態 | Teach／MotorTest／網頁生命週期影響它，見原V906狀態文件 |

## 同一主題內的差異表

| 項目 | 卡片式（原golden／SMC等） | HT9050／PCI-1203 DS402 | 查證邊界 |
|---|---|---|---|
| 動作責任 | 卡片組成回原點動作與軌跡 | 驅動器執行method，1203轉送命令及狀態 | 新drive需核對profile、6098h與原點／offset，不由機型名稱猜 |
| 位置歸零 | 原流程完成後重設cmd／act | 原DS402路由不以軟體歸零掩蓋失敗 | [1203原文](drivers/pci1203/original-entry.md)保留規則與實機記錄 |
| 完成證據 | 原流程常看Home燈／HOMING→READY | 當前RouteHomeDone的DS402分支看命令後READY，cmd位置在±1內成功，範圍外失敗；cardSide仍另走分支 | cmd約0是當前route假設，非任意home offset或其他drive的通用定律 |
| MotorHome case20 | 依原點信號判定／retry | bW906HomeTrusted可承認drive回完而不要求原點燈亮 | 分清route完成與燈訊號，不清旗標假裝成功 |
| 步進起點 | 依卡／驅動器既有行為 | SW3D原機台筆記先離開原點；單軸／全機共用hook，方向按HomeDirection | 具體距離／重試／速度與極性保留版本，未在本批動機台 |

## 當前版本補註

- `EtherCAT/Pci1203MotorRoute.cpp` 的 `RouteHomeDone`：DS402與cardSide完成路線分開；pending、新樣本、ERROR_STOP與homeSent條件仍要保留。
- `Motor/mymotor.cpp` case20可接額外ZSafe步骤；`MachineType.h` 的 `W906_HT9050_FULLHOME_ZSAFE` 在main預設為註解掉，不能把Task40當成預設必跑。
- 原機台側HOMEPOS0／TRAYSAFE等臨時行為與GitLab main分開看，不能因保存了原筆記就宣稱main已廣泛略過保護。

[HT9045分支](machines/ht9045.md)／[HT9050分支](machines/ht9050.md)／[控制卡與驅動器](drivers/index.md)／[原共用流程](flow/generic/original-entry.md)。
