# 原文 07／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `AtomicWrite`；種類 `complete_header_inline_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `954c221367d9a1adaba3ec925cbe6d0e203ff866daa129d04c5d1aac21d551f6`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline bool AtomicWrite(const std::string& path, const std::string& body)
{
    const std::string tmp = path + ".tmp_wb";
    {
        FILE* o = ::fopen(tmp.c_str(), "wb");      // "wb" -> 不做 CRLF 轉換
        if (!o) return false;
        if (!body.empty())
            ::fwrite(body.data(), 1, body.size(), o);
        ::fflush(o);
        ::fclose(o);
    }
    // ⚠⚠ 20260922 端到端實測改大的。原本是 5 次 × 20 ms = 100 ms，
    //   而探針自己在跑 `copytree` 備份那個目錄時就足以撞出 sharing violation
    //   —— 結果 `.json` 寫成功、墊片沒寫成，而那正是
    //   「在量產的 file: 模式下完全沒反應且不報錯」的形狀。
    //   ⇒ 20 次 × 50 ms = 1 秒。對 100 ms 輪詢的對方來說這仍然是即時的，
    //     但足以撐過防毒掃描或備份程式短暫開檔。
    for (int attempt = 0; attempt < 20; ++attempt) {
        if (::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
            return true;
        ::Sleep(50);
    }
    std::printf("  ⚠ 原子替換失敗（重試 20 次）：%s\n", path.c_str());
    ::remove(tmp.c_str());
    return false;
}

<!-- preserved-content:end -->
```
