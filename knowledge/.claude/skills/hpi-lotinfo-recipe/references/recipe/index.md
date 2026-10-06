# Recipe檔案樹

[原完整主體](original-entry.md)保留完整文件／變體／資料模型，按用途逐層讀：

| 層 | 文件 |
|---|---|
| 測試／工單 | [TestMode](references/TestMode.Data.md)、[HandlerCondition](references/HandlerCondition.Data.md)、[Tester](references/Tester.Data.md) |
| 接觸／溫度／壓力 | [Contact](references/Contact.Data.md)、[Temperature](references/Temperature.Data.md)、[ep](references/ep.txt.md) |
| 機構／盤 | [ArmCondition](references/ArmCondition.Data.md)、[Tray](references/Tray.Data.md)、[UdUld](references/UdUld.Data.md)、[Rotate](references/Rotate.Data.md)、[HotPlate](references/HotPlate.Data.md) |
| 分類／視覺 | [Binasgn及變體](references/Binasgn.Data.md)、[AOI](references/AOI.Data.md) |
| 覆蓋／完整性 | [configByRecipe](references/configByRecipe.ini.md)、[MD5](references/MD5.md)、[Information](references/Information.txt.md) |

這些是各原版本字段說明，不等於所有機台都有文件、所有鍵都存在、或現在允許B路寫入。讀 [目前API](../runtime/recipe.md)，以擁有該文件的Recipe為欄位盤點分母；原216／215數字僅為原次掃描。Big5原檔與UTF-8 JSON傳輸／Skill文件編碼分開，整理不重存任何.Data。
