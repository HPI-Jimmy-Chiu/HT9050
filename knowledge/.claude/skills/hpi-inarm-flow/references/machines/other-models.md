# HT9045與其他配置

| 配置 | 流程／幾何分流 | 來源 |
|---|---|---|
| 9045多吸嘴8-site及其他site | iInArmType選AxEx／AxxG／ACEG、iXStep與各DoInArm_9045_XxY_Z；TestMode不等於物理嘴數 | [原流程](../flow/original-entry.md)／[基準軸與Type](../geometry/index.md) |
| V899 e9045_2x4_16 | 原吸取調查的預設版本／16-site映射；只作對照，修正交付走授權V912 | [吸取層](../vacuum/index.md) |
| HT9045S／固定與可變Pitch | USE_PICKER_COUNT=0、USE_IN_OUT_ARM_Y_PITCH、基準E／F與iInArmType決策分開 | [原Type推導](../flow/references/InArm_TMyKitSuck_and_TypeDecision.md) |
| HT9046AU等延伸配置 | Type計算中有OutArm2Suck.iXStep；本批不宣稱已逐條重驗全部站點與OutArm流程 | 原流程及當前DoInArm_9045_Type，再查[相鄰OutArm](../related.md) |

## 查證方式

先從來源版本的DoInArm／DoInArm_9045／DoInArm_9045_Type追Task，再取對應配置函式；原文「其他檔相同」只當歷史調查，不作跨機型一致的證明。未核對機型保留「未核對」，不要寫同通用。
