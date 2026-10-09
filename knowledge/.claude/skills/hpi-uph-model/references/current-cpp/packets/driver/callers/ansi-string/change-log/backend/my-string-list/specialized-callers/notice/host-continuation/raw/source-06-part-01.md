# 原文 06／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `MailboxPut`；種類 `complete_header_inline_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `3c8e8dfc6b4e9369210b3f8e1dec01fabc424498ad18960028d3486a5ca399c2`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline bool MailboxPut(const std::string& dir, const std::string& name,
                       const std::string& json)
{
    const bool okJson = AtomicWrite(dir + "\\" + name + ".json", json + "\n");

    std::string shim;
    shim  = "window.__HT9045_DATA__=window.__HT9045_DATA__||{};\r\n";
    shim += "window.__HT9045_DATA__[\"" + name + "\"]=" + json + ";\r\n";
    const bool okShim = AtomicWrite(dir + "\\js\\" + name + ".js", shim);

    // ⚠ 兩個都要成功才算數。只成功一個 = 在某一種啟動模式下完全沒作用，
    //   而且不報錯 —— 那比兩個都失敗更難查。
    //
    // ⚠⚠ 而且要**講出是哪一個失敗**。20260922 端到端實測時就踩到：
    //   `.json` 成功、墊片失敗，而量產（file: 協定）只讀墊片
    //   ⇒ 現場的症狀是「警報完全沒跳出來」，但 log 只會說「寫入失敗」，
    //     看不出是哪一半 —— 那會讓人往完全錯的方向查。
    if (okJson != okShim) {
        std::printf("  ⚠⚠ 信箱只寫成一半：.json=%s 墊片=%s（%s）\n"
                    "     量產是 file: 協定，**只讀墊片** ——"
                    " 墊片沒寫成等於警報完全不會出現。\n",
                    okJson ? "OK" : "FAIL", okShim ? "OK" : "FAIL", name.c_str());
    }
    return okJson && okShim;
}

<!-- preserved-content:end -->
```
