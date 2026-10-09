# 原文 06／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::MySaveToFile()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `4a8ec353c4706a79264ab7592f0342fdd5bafba6064cddf62775d1634662cc32`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TMyStringList::MySaveToFile()
{
    AnsiString sPathName;
    AnsiString sFileName;
    AnsiString Str;
    FILE *pFile;

    if(HTAutoSave==false || MyList==NULL || MyList->Count==0)                   //沒資料就不用存檔
        return;

    sFileName=GetFileName();

    //AI(W906-GB-P4) 20260926: `MyList->Text` is a vclcompat TextProxy; wrapped in AnsiString() (no value change:
    //   TStringList::GetText joins every line with "\r\n", like BCB6 TStrings::GetTextStr).
    if(FileExists(sFileName)==false && HTFirstRow!="")
    {
        Str=HTFirstRow+"\r\n"+AnsiString(MyList->Text);
    }
    else
    {
        Str=AnsiString(MyList->Text);
    }

    Str=StringReplace(Str, "\r\n", "\n", TReplaceFlags()<<rfReplaceAll);
    pFile=fopen(sFileName.c_str(), "a");
    if(pFile!=NULL)
    {
        fputs(Str.c_str(), pFile);
        fclose(pFile);
    }

    pFile=NULL;
    MyList->Clear();
}

<!-- preserved-content:end -->
```
