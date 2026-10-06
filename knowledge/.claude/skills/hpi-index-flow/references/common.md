# Index 共同項與機型差異

查證基準：GitLab main `e184ef205`，20261006；本次只查文件與程式，未操作機台。

| 項目 | 共同項／介面 | HT9045 等歷史機型 | HT9050／V906 現況 |
|---|---|---|---|
| 主流程入口 | Index 的 Task 驅動流程需追 dispatcher 與 cursor | 舊正文有 DoTestHeadMotor → DoTestY → Front/Rear/32Site 分支，依版本核對 | csystem.cpp 仍有通用 DoTestHeadMotor 呼叫；未發現 DoTestHeadMotorFP 呼叫 |
| Index Task | 通用與 FP 函式都取 iTestHeadMotorTask | 各子流程有自己的 Task，見原版子狀態機 | FP 與通用入口共用 cursor；存在函式不等於已安全切換 |
| 軸配置 | 下壓前都必須按實際硬體確認干涉區 | 原資料含 Index Y、Z1/Z2 與雙臂／32 Site；不能概括到所有機型 | 文件確認僅 Z1；水平定位由 Shuttle 獨立 X，勿套通用 Y/Z2 路線 |
| 安全互鎖 | 依原裁決與實作分別查證 | Contact/Pick 案例為 V899，HasIC 資料與實際持料需一致 | W-44 以 socket 外安全區為前提，方向由各自 Right−Left；原裁決與待完成事項見 HT9050 分支 |
| EP D24/D26 | 設定語意、單位與回授判定須依版本確認 | 原版 reference 記錄 kPa、D26_1 log/alarm gate | V906 CheckAndRecodrEP live body 是 return false stub；完整舊碼位於 #if 0，不能當成當前有效 alarm |

## 已直接核對的程式

- `HT9011UC_Cpp_V3.33.906.0/atester.cpp`：通用 `DoTestHeadMotor()` 使用 `iTestHeadMotorTask`；`CheckAndRecodrEP()` 完整保留碼與 live stub 成對。
- `HT9011UC_Cpp_V3.33.906.0/atester_FinePitch.cpp`：`DoTestHeadMotorFP()` 也引用 `iTestHeadMotorTask`，檔頭說明 slice2 尚待接入。
- `HT9011UC_Cpp_V3.33.906.0/csystem.cpp`：保留通用呼叫；原文需再按 compiled/live gate 判別，不能由 grep 命中推斷每一處都是執行路徑。

## 詳細資料

- [HT9045 與相關舊型流程](machines/ht9045/index.md)
- [HT9050 FinePitch 與原裁決](machines/ht9050/index.md)
- [EP 語意與當前實作缺口](ep/index.md)
- [原版完整入口](history/ht9045-index-flow/index.md)（保留 V897／BCB6 等原版本資訊；未宣稱全部適用 V906／V912）
- [客戶條件](customers.md)
