# 原文 02

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`，function `int TMyStringList::GetLastLine()`。
來源 commit `26a6dcf0b8c1acf106a06e6cf4e700d20053d703`；完整摘錄 SHA256 `b6c904361034cc009abd0e5c9adc38c3e220df94bd17b0c9148cee7eea678054`。
此頁保留完整正文及原註解，歷史 golden／nm／test 敘述不是本輪驗證。

```cpp
<!-- preserved-content:start -->
int TMyStringList::GetLastLine()                                                //Steven 20191016 : 取得目前檔案的行數
{
    AnsiString sFileName=GetFileName();
    TStringList *File;
    int iCount=0;

    if(FileExists(sFileName))                                                   //Steven 20200221 : 修正沒有檔案就不要讀檔
    {
        File=new TStringList();
        File->LoadFromFile(sFileName);
        iCount=File->Count;
        File->Clear();
        delete File;
    }
    sLastFileName=sFileName;                                                    //Steven 20191107 : 紀錄現在的檔名

    return iCount;
}

<!-- preserved-content:end -->
```
