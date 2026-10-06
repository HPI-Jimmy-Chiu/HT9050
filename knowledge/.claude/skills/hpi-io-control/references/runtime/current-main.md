# IO目前main與歷史界線

20261006只讀核對main `372b91908`。本批沒有編譯、連卡或執行程式。

- `EtherCAT/Pci1203IoRoute.cpp::RouteWriteBit`／`RouteReadBit`均忽略Bit：Port/8找byte、Port%8找bit；Ring／IP定位環與站。`RouteReadByte`的Port直接是站內byte。不要把舊`Acm_DaqDoSetBit`範例或五碼Alias圖的bit欄當目前1203實體點。
- `MyLaneIo.cpp::IOBitOn`／`IOBitOff`先呼叫`IdleCheckSafeDoorByCylinder`，再檢地址、更新`OutPortData`、呼叫backend；快取在送出前更新，故快取改了不能證明寫入成功。原「所有IO函式都擋安全門」不能泛指讀取或其他命令面。
- `mysensor.cpp::IsOn`／`IsOff`先分流`iControlPanelMode==1`的Pad鍵，普通IO才檢Enable；disabled普通IO回false。`myswitch.cpp::On`／`Off`先更新OutValue，Pad按鈕也先於Enable判斷。
- `mycylin.cpp::OnSensor`／`OffSensor`在Enable=false回true，兩個到位sensor都沒開也可回true；`On`／`Off`仍改Status、Change及Task，`OnSwitch`／`OffSwitch`才依Enable決定是否輸出。不可把舊「不動作」摘要理解成物件欄位完全不變。
- `mykitsuck.cpp::Suck`是OnTask狀態機，用`W906_GetStatusAllOn`判吸到；群組只計有效且Enable的成員，至少一個、全部被計成員ON才true，並非不檢Enable地固定量四個。`W906_GetStatusAnyOn`與保持判斷用途不同。
- 同檔`W906_InitVc4Groups`依`W906_GpibModel=="9050GPIB"`或`MachineTypeChoice==Type_HT9050`強制InArm／OutArm群組開啟；Index仍依自己的開關。四個物理嘴共同吸一顆不等於Suck矩陣四格各有一顆。
- `cinitial.cpp::InitialSafeDoor`目前仍先強制部分Sen[SnSafeDoor*].Enable=true；後面有機型與配置分支。因此不能從IO_Table單獨推門的最終Enable，舊規則A／TEMP-DOORS等按原裁決與實作狀態查，文件整理不開關gate。
- `JsonBridge/IoBtnPanelClick.cpp::Click_`的`W906_IO_PAGE_NO_GUARDS=1`按既有裁決略過部分golden GUI guard並列guardsBypassed；它仍檢Alias、輸入／輸出類型、路由，實際MyLaneIO／1203 write gate另查。IO頁可按不代表HOME／START／馬達移動或真空頁也可通過。
- `TableAuditLive.cpp::W906_TableAuditRun`重讀IO／Mot原CSV，建立配置及名稱清單，呼叫`TableAuditRunPure`；載入時atoi已將錯字變0，不能用綁定後物件取代原文字驗表。啟動gate與warning/error處置另追目前caller。

## 來源定位

- [1203路由](../../../../../HT9011UC_Cpp_V3.33.906.0/EtherCAT/Pci1203IoRoute.cpp)：RouteWriteBit／RouteReadBit／RouteReadByte／CheckWrite_。
- [MyLaneIO](../../../../../HT9011UC_Cpp_V3.33.906.0/MyLaneIo.cpp)：IOBitOn／IOBitOff／OutPortData。
- [Sensor](../../../../../HT9011UC_Cpp_V3.33.906.0/mysensor.cpp)／[Switch](../../../../../HT9011UC_Cpp_V3.33.906.0/myswitch.cpp)／[Cylinder](../../../../../HT9011UC_Cpp_V3.33.906.0/mycylin.cpp)：上述function／Enable與Task。
- [真空](../../../../../HT9011UC_Cpp_V3.33.906.0/mykitsuck.cpp)／[結構](../../../../../HT9011UC_Cpp_V3.33.906.0/mykitsuck.h)：W906_iGroupCount／W906_pGroup／W906_GetStatusAllOn。
- [初始化](../../../../../HT9011UC_Cpp_V3.33.906.0/cinitial.cpp)／[IO按鈕](../../../../../HT9011UC_Cpp_V3.33.906.0/JsonBridge/IoBtnPanelClick.cpp)／[原表驗表](../../../../../HT9011UC_Cpp_V3.33.906.0/TableAuditLive.cpp)。
