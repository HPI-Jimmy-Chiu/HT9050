# IO與吸嘴機型差異

| 機型／配置 | 差異與查證入口 |
|---|---|
| HT9050 | ePCI1203路由的Ring／IP／Port；Bit不參與物理點。In／Out VC4群組強制開，四嘴合吸一顆；Index群組另開關 |
| HT9050門／EMG | 先核對機台快照再看InitialSafeDoor／IsMotorCanRun等caller；普通Sensor disabled回false與安全迴路實體掉電是兩件事，不根據本機表直接裁定機台故障 |
| HT9045／HT9046／HT9011UC | 原2×4吸嘴、動態iPickRow／Col／Step、In／Out基準軸及Pitch公式按配置；拓樸不能直接套單Picker |
| MotionNet／MN200／ISA／PLC配置 | 依IO_CARD_TYPE、ISABase與地址欄分派；相同Alias不保證接相同backend，安全PLC有Enable／InType覆寫 |
| HT7080等其他拓樸 | InitialSafeDoor有Type_HT7080分支；HT7xxx已clone在D:/HT1028，本批未核對該repo拓樸，不能因名稱相近就宣稱已共用 |

- [HT9050硬體差異原資料](../../../ht9050-hw/references/ht9050-vs-ht9045.md)。
- [原IO共通知識與HT9050補充](../io/original-entry.md)／[原吸嘴架構](../vacuum/original-entry.md)。
- [目前main](../runtime/current-main.md)／[快照規則](../../../../../machines/README.md)。
