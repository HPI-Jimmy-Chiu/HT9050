# Shuttle 共同結構

只記 V906 四個入口共同使用、未依機型分支的狀態機結構。物理軸、停位、互鎖與 case 意義依機型讀對應 reference。

## 狀態游標

acarry.cpp 四個入口都用 `int &Task` 引用現有游標，再由 `switch(Task)` 逐 tick 推進。
AutoSHT1Task／AutoSHT2Task 是不同入口可共用的狀態儲存位置；不能由變數名判斷機構。

## 分流與定位

csystem.cpp::DoAllProcess 此處依 MachineTypeChoice 分流，見 [ht9045.md](ht9045.md)／[ht9050.md](ht9050.md)。
客戶分支依實際 CUSTOMER_CODE 條件，見 [customers.md](customers.md)，不能套到不同入口。
卡步時記錄版本／機型、實際函式、Task、等待的 guard、旗標、cmd/enc；相同 Task 數字不代表相同動作或故障。

來源：V906 [acarry.cpp](../../../../HT9011UC_Cpp_V3.33.906.0/acarry.cpp)::Do_Auto_SHT1／Do_Auto_SHT2／Do_Auto_InSH／Do_Auto_OutSH 的 int &Task 與 switch；
[csystem.cpp](../../../../HT9011UC_Cpp_V3.33.906.0/csystem.cpp)::DoAllProcess Shuttle 分派。
