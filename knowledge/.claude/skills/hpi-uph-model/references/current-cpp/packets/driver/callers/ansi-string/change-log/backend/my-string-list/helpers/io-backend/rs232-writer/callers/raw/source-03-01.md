# 原文 03／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::~TMyStringList()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `a816f239ac0336fc04854825b56dd46cd97edebad179ab8e3505ff0cff3b4003`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
TMyStringList::~TMyStringList()
{
    try
    {
        MySaveToFile();
        MyList->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete MyList;
    }
    catch(...)
    {

    }
}

<!-- preserved-content:end -->
```
