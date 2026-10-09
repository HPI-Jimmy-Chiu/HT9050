# 原文 07／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `LoadUnlockPassword`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `3ca011ec3036e0e74a7713156576a825e76173ec53a1f65e84919648c28f8464`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
void LoadUnlockPassword()
{
    if (s_unlockRead) return;
    s_unlockRead = true;
    const char* e = std::getenv("W906_ALARMUNLOCK_PATH");
    AnsiString sUnlockPath = (e && *e) ? AnsiString(e) : AnsiString("C:\\Windows\\AlarmUnlock.ini");
    if(FileExists(sUnlockPath)==false)
    {
        s_unlockMissing = true;                                 // [W906] golden :11386-11393 writes the file here
        std::printf("[note-auth] %s is missing: golden writes it with its built-in password; this tree does not carry it, "
                    "so no alarm unlock password matches\n", sUnlockPath.c_str());
    }
    else
    {
        TStringList *sTmp = new TStringList;
        sTmp->LoadFromFile(sUnlockPath);
        asUnlockPassword = (sTmp->Count > 0) ? AnsiString(sTmp->Strings[0]) : AnsiString("");   // [W906] golden Strings[0] on an empty file throws
        sTmp->Clear();                                                          //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete sTmp;
    }
}

<!-- preserved-content:end -->
```
