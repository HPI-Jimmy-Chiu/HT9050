# repo內cJSON入口與accessors

以 `Public/cJSON.c` 的function與 `Public/cJSON.h` 的Memory Management註解定位，
不是替換成外部最新版cJSON契約。本單元保存11完整C定義，純C與CPP完成數分開記。

`cJSON_Parse(value)` 呼叫 `cJSON_ParseWithOpts(value,0,0)`。
ParseWithOpts先擋NULL，再 `strlen(value)+sizeof("")` 取得含NUL的buffer長度。
它沒有使用reader的file_size或fread結果；內嵌NUL會限制可見字串長度。

ParseWithLengthOpts以global_hooks配置item、呼叫parse_value；
只有require_null_terminated為true才跳過尾部空白並要求當下是NUL。
Parse的預設參數是false，所以不能把「回傳tree」等同「整個檔案只含一個合法JSON且沒有尾隨內容」。
具體parse_value、字符串／數字子parser、depth上限與allocator失敗的完整callee圖待續。

失敗分支會Delete部分item、更新global_error並回NULL；
成功回item。global_error／global_hooks是共享狀態，這份文件未證明thread safety。

| accessor | 本體條件 |
| --- | --- |
| GetArraySize | NULL回0，其餘計child sibling數，沒有檢查array type；size_t轉int有原始overflow註解 |
| GetArrayItem／get_array_item | 負index回NULL，從child走next到指定位置；沒有檢查array type |
| GetObjectItem／get_object_item | object或name NULL回NULL；走child key，比較分支由case_sensitive決定 |
| IsArray／IsNumber | NULL回false，檢查type低8bit |
| Delete | 依IsReference／StringIsConst旗標處理child與字串，用global_hooks釋放node |

這些本體說明借用指標與釋放入口；實際部署lib版本、link結果與執行期heap未查證。
