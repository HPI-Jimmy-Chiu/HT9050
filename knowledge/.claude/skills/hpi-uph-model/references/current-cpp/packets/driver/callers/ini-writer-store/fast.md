# Fast 修改演算法與 span

定位 IniFiles.cpp 的 W906_IniApplyFast、W906FsWs／Next／Trim／Same／Header／Key、W906IniWs／Trim／Lower／EnsureNl；W906FsLine 的 s／n／e 分別是起點、文字長度、含行尾的終點。完整原文見 [manifest](source-manifest.json)。

## 解析與名稱

- W906FsNext 依 CR、LF、CRLF 切行；無行尾時 e=len。W906FsWs／W906IniWs 只接受空格與 tab，Trim 不移除其他控制字元或 NUL。
- W906FsSame 與 W906IniLower 只折疊 ASCII A..Z；其他 byte 直接比較。secIn／keyIn 先 Trim，再比較折疊後的名稱。
- W906FsHeader 在 trim 後以 '[' 開頭即視為 header；取第一個 ']' 前的名稱，沒有 ']' 就取到行尾，再 trim 空格與 tab。
- W906FsKey 遇左側空格／tab 後的 ';' 回 false，否則找第一個 '='、trim key；空 key 沒有額外拒絕。它沒有把 '#' 當成註解。

## 修改與插入

ApplyFast 先檢查最後一行：只有未以 CR／LF 結束、且整行都是空格／tab 時，才把有效 len 截到該行起點。不是刪除所有末尾空行。

搜尋取第一個匹配 section；不存在時在有效內容後補換行、append '[section]\r\nkey=value\r\n'。存在時只掃到下一個 header，取第一個匹配 key，保留從檔頭到該行 '=' 的 bytes，再替換整段 value、接回該行 terminator 與其後內容。

缺 key 時通常插在 lastKey.e（最後一個可解析 key 或該 header）後。若最後一個 section 沒有下一個 header，last 為空白行且 lastKey.s!=last.s，就走 EOF append；不能簡化成「永遠插在 section 尾端」。

W906IniEnsureNl 在字串為空或最後 byte 為 LF 時原樣返回，其他情況加 CRLF；原尾端為單獨 CR 時也會再加 CRLF。因此本輪不保證所有行尾、註解、value、檔案 bytes 一律不變。value 原樣接入，換行／NUL／特殊名稱的 caller 限制與容量 ABI 仍待續。

這是完整選讀函式的靜態路徑判讀，沒有執行 differential、golden、真檔或 crash recovery 測試。
