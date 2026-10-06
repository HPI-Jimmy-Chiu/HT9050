# 共用層與兩套A／B／C名詞

同一Skill整合HT9050與其他Handler；畫面技術共用，資料值、軸／IO、客戶權限與實際功能必須分流。

| 層 | 查證方式 | 需分開的項目 |
|---|---|---|
| 來源 | 機台快照、BCB golden、V906 C++、Web頁面與generator | 同版號的Code與Cpp樹、V912對照與906 0618 golden、歷史V910來源 |
| 畫面 | DFM元件名→HTML id→proxy／橋接→caller | 手寫／產生／截圖過渡、HT9050 MotionView保留頁、當前開頁狀態 |
| 資料 | route與owner→read／write→tag／ack | 配方／system／config／生產資料，不由JSON檔名推定現在寫者 |
| 動作 | form.event／motor.access等實際分派→guard→golden本體 | 讀取與動作、SystemStart／SoftStart、客戶與硬體條件、未接線／停用 |
| 權限 | 登入狀態、level與個別caller的reauth | 全域登入、設定重驗、告警解鎖、vendor與CC專屬分支 |
| 顯示與視窗 | WebWindowRegistry與WebPageTable的不同答案 | stale保守判斷、實際畫面在場、程式開窗、hub只放開jog |
| 紀錄 | cMyDB／LogObjects／CSV實際呼叫與落點 | 歷史SQLite、MDB Updater工具、CSV頁面與BCB外掛，不只看DB命名 |

**路A／路B／路C**指資料與保存路徑：舊靜態JSON模擬、wb_serve檔案API／runtime、golden表單橋。**橋接A形狀／C形狀**指同一路C中的兩種產生方式：逐鍵表單與HTEditList。兩套字母不能混稱或由名稱判完成度；詳細定義保留在 [原路C契約](json/references/route-c-golden-bridge.md)與[原產生器文件](bridge/references/generators.md)。

原「全唯讀」「需allow-system-write」「50／500ms」「未建log」等語句帶各自日期，讀 [目前main](runtime/index.md)後再查實際caller。本批保存原裁決全文；沒有重跑動態驗收或把歷史量測改標現在通過。
