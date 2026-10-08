# INI 日期與字串轉換 callee

接續 [typed API](../ini-typed-enumeration/typed.md)、[共用 binding](../ini-core/binding.md) 與 [保存生命期](../ini-writer-store/lifetime.md)。V906 vclcompat 的原文 pin `ade15129a22aad0fe478aca389e35acfd19c42d5`；四來源 byte 已與 main `214f680def84be422f0d4c10e51f89d068463e21` 重核一致。

[manifest](source-manifest.json) 保存 12 個完整 cpp 定義、2 個 inline header 定義、完整 TDateTime.h 與 epoch 常數，共 16 片段。這些是上輪已選讀 intake，本單元完成文件化；本輪新增 cpp 選讀 0、canonical 主題新增 0，不宣稱全 cpp 檔或全部 caller 已查完。

- [字串與整數](string.md)：Trim 的 byte 條件、str 參照、int constructor 與 assignInt。
- [日期解析](parse.md)：sscanf 賦值數、部分輸入、非空失敗與 caller def。
- [serial 與 encode/decode](serial.md)：double／Word、epoch、floor、毫秒進位界線。
- [格式與秒精度](format.md)：token、month/minute、AM/PM 與 DateTimeToStr。
- [版本、機型及待查](limits.md)：歷史註解、HT9050／其他 Handler、V912 與現場界線。

回 [caller 索引](../index.md)。只做來源及文件靜態查證；沒有執行 C++、INI 保存、runtime 或機台。這個新單元的工作基準包含 Ready MR !333；整批交付前必須確認其依賴已整合，避免重複提交上一批 27 檔。
