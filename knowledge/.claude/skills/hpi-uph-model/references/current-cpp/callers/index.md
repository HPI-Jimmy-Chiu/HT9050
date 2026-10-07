# 配置 InArm 的 UPH 計數 caller

來源 pin `f2a1d78160e9f44f6bb351f0a3552ae36e87a031`；同一 [UPH Skill](../../../SKILL.md) 的 [目前 C++ 樹](../index.md)，保留 HT9050／其他 Handler 機型分流。

| 問題 | Reference |
| --- | --- |
| 指定呼叫條件、迴圈與 Tray 參數方向 | [call 區段](calls.md) |
| 字面盤點、版本閘門與未閉合部分 | [範圍與限制](limits.md) |
| blob／byte SHA、guard、所選body／snippet SHA | [manifest](source-manifest.json) |

| 固定來源檔 | 編碼 | 範圍 |
| --- | --- | --- |
| [V906 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp) | utf-8 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V906 ainarm9045_2x4_16.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Cpp_V3.33.906.0/ainarm9045_2x4_16.cpp) | utf-8 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V906 ainarm9045_2x4_16_shims.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Cpp_V3.33.906.0/ainarm9045_2x4_16_shims.cpp) | utf-8 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V906 ainarm9045_2x4_16_shims.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Cpp_V3.33.906.0/ainarm9045_2x4_16_shims.h) | utf-8 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V906 ainarm9045_2x8_32.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Cpp_V3.33.906.0/ainarm9045_2x8_32.cpp) | utf-8 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V912 Magazine.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/Magazine.cpp) | cp950 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V912 ainarm9045.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045.cpp) | cp950 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V912 ainarm9045_1x2_1.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045_1x2_1.cpp) | cp950 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V912 ainarm9045_2x4_16.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045_2x4_16.cpp) | cp950 | 指定call、宣告／定義或原文候選；範圍見manifest |
| [V912 ainarm9045_2x8_32.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/f2a1d78160e9f44f6bb351f0a3552ae36e87a031/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ainarm9045_2x8_32.cpp) | cp950 | 指定call、宣告／定義或原文候選；範圍見manifest |

本層讀七個call附近區段、三個AddLoadingCount定義／三個宣告的指定語句及一個V906 2x8_32 return-false stub；七個caller完整body只保存hash，語意尚未完整讀取。2289個追蹤cpp／h、10個原文候選檔、13函式形狀不是所有caller或runtime分派已驗證。
