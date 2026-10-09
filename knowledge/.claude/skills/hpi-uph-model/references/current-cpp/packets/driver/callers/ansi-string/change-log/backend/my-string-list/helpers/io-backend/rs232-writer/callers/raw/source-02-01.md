# 原文 02／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::TMyStringList(AnsiString`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `f62f49038cdd8f3b952f32308f06cc11099e4d71c3a293d94b880e8595b760c5`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
TMyStringList::TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow)
    : TStringList(),
      HTPath(Path),
      HTFileName(FileName),
      HTFirstRow(FirstRow),
      HTMaxLineCount(MaxLineCount),
      HTSaveType(SaveType),
      HTAutoSave(AutoSave)
{
    MaxLineCount        =1;
    Path                =sPath;
    FileName            =sFileName;
    FirstRow            =sFirstRow;
    SaveType            =TByDay;

    AutoSave            =true;
    bUseFTRT            =false;
    bFilePathWithDate   =true;                                                  //Steven 20210623 : 預設存檔要有日期當資料夾
    MyList              =new TStringList();
}

<!-- preserved-content:end -->
```
