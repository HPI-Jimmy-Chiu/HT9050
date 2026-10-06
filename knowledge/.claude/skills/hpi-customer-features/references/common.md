# 客戶差異的共同查證流程

這裡的共同項是查證方法，不宣稱所有Handler的客戶功能相同。基準與詞法搜尋範圍見[版本索引](versions/index.md)，後續文件裁決見[main更新](main-update-20261006.md)。

1. 先確定來源版本、CUSTOMER_CODE與[機型身分](machines.md)，用同版本MachineType.h查CC符號數值及別名。
2. 由[主題路由](topics/index.md)讀已有人工表；保留其來源版本、Task／case、開關與未核對說明。
3. 沒有人工列時，用[符號樹](customers/index.md)找function候選，再讀該版本原始碼的完整條件、caller、機型dispatch、INSTALL_／USE_／FUNC_CC_和建置條件。
4. 確認後才在取得該主題編輯範圍內新增人工六欄表：客戶碼、函式／Task、開關、行為差異、來源版本、相關機台；scanner不覆寫這些文字。

同一symbol在兩版出現，只證明存在文字定位。相同數值也可能有別名；只在一版命中不能證明另一版沒有等效行為。`FUNC_CC_`旗標與`CUSTOMER_CODE`比較分開，測試、宣告、generated或字面停用候選另標。

目前掃描沒有解析完整C++ AST、追全caller、執行preprocessor或編譯，沒有讀取機台執行期設定或跑實機。表中的function為詞法候選；lambda沿用外層、constructor initializer或其他不支援語法明標unresolved，查原始碼後才能提升查證等級。

Task／case若有業務意義，人工核對後再補；不從搜尋近旁數字推測Task。活定位用source commit／版本樹／檔名＋function或變數，沒有程式行數欄。

20261006局部人工成果見[核對樹](reviewed/index.md)：ART客戶條件／caller與名稱解析共8列，HT9050及其他機型共用／分流一起查。來源manifest保留釘住commit與blob；不改候選ID或原主題人工表。
