# 原文 18／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Bridge.h`；function／region `rs232std::TMyStringList declaration`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `7d4944f54053f9e0c7afad029da5f8868a740ce368735a42d2f194961a13e123`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
class TMyStringList : public TStringList
{
public:
    // properties as public fields -- declared before the references below, which bind to them
    AnsiString  Path;
    AnsiString  FileName;
    AnsiString  FirstRow;
    int         MaxLineCount;
    TSaveType   SaveType;
    bool        AutoSave;
private:
    AnsiString& HTPath;
    AnsiString& HTFileName;
    AnsiString& HTFirstRow;
    int&        HTMaxLineCount;
    TSaveType&  HTSaveType;
    bool&       HTAutoSave;
    void SetPath(AnsiString P);
    void SetFirstRow(AnsiString P);
    void SetMaxLineCount(int Cnt);
    void SetSaveType(TSaveType Type);
    void SetFileName(AnsiString P);
    void SetAutoSave(bool P);
    void GetYesterdayInfo();
    TMyStringList(const TMyStringList&);
    TMyStringList& operator=(const TMyStringList&);
public:
    TMyStringList();
    TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow);
    ~TMyStringList();
    AnsiString AddText(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2 = "");
    AnsiString AddTextWithHex(AnsiString sUnitName, std::vector<Byte> bHex);
    void MySaveToFile();
    void MyInsertToFile(int iCount);
    int GetLastLine();
    AnsiString sLastFileName;
    AnsiString GetFileName();
    bool bFilePathWithDate;
    bool bUseFTRT;
    TStringList *MyList;
};
<!-- preserved-content:end -->
```
