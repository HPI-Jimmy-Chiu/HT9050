# 原文 03

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`，function `void TMyStringList::MyInsertToFile(int iCount)`。
來源 commit `26a6dcf0b8c1acf106a06e6cf4e700d20053d703`；完整摘錄 SHA256 `135a4b0563d52f21c1eb11889847974efbfcc0075fcc0e5b05feda2b5034011d`。
此頁保留完整正文及原註解，歷史 golden／nm／test 敘述不是本輪驗證。

```cpp
<!-- preserved-content:start -->
void TMyStringList::MyInsertToFile(int iCount)
{
    if(HTAutoSave==false || MyList->Count==0)                                   //沒資料就不用存檔
        return;

    AnsiString Str;
    TStringList *File;
    File=new TStringList();

    if(FileExists(sLastFileName)==false)
    {
        sLastFileName=GetFileName();
    }
    else                                                                        //Steven 20200221 : 修正沒有檔案就不要讀檔
    {
        File->LoadFromFile(sLastFileName);
    }

    //AI(W906-GB-P4) 20260926: golden `Str=MyList->Text.SubString(0, MyList->Text.Length()-2);`.  Member access on the
    //   vclcompat TextProxy does not convert, so the property is wrapped in AnsiString() (rule 11); SubString(0, ...)
    //   keeps the BCB6 index-0-behaves-like-1 quirk (vclcompat AnsiString.cpp SubString), i.e. the text minus its
    //   trailing "\r\n".
    Str=AnsiString(MyList->Text).SubString(0, AnsiString(MyList->Text).Length()-2);
    Str.Insert(' ', 12);
    Str.Insert(' ', 26);

    if(FileExists(sLastFileName) && File->Count>iCount && iCount>0)             //Steven 20200215 : 加上保護機制
        File->Insert(iCount, Str);
    else
        File->Add(Str);

    File->SaveToFile(sLastFileName);                                            //Steven 20191107 : 紀錄現在的檔名
    File->Clear();
    MyList->Clear();
    delete File;
}

<!-- preserved-content:end -->
```
