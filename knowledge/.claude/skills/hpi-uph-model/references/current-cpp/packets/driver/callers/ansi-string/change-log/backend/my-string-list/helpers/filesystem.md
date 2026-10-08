# vclcompat filesystem與path helper

[上層](index.md)；定位 `vclcompat/SysUtils.cpp` 的8個選定定義及對應header，保留各自平台分支。

| Function | 現行本體 |
| --- | --- |
| FileExists | stat失敗return false；成功且型別不是S_IFDIR才true。不是只判regular file，也沒有分出不存在、權限或其他stat錯誤。 |
| DirectoryExists | 先在std::string上反覆去除末尾 `/` 或反斜線，直到長度1；再stat並判S_IFDIR。 |
| CreateDir | _WIN32用_mkdir，其他用mkdir(path,0777)；回傳呼叫是否等於0。只建單層。 |
| ForceDirectories | 空字串false，已存在目錄true；逐字累積path，在每個separator前建未存在component，最後再建完整path並return DirectoryExists(path)。中間CreateDir結果未核。 |
| DeleteFile | std::remove(path.c_str())==0；不是本體內呼叫Win32 DeleteFileA。 |
| lastSepPos | 由尾往前找 `/` 或反斜線，沒有時-1。 |
| ExtractFilePath | 最後separator之前含separator；沒有separator則空字串。 |
| ExtractFileExt | 最後separator後由尾往前找第一個點，回傳從點到尾的字串；無點則空。 |

DirectoryExists的歷史註解說strip single separator，現行while會反覆移除；原文仍保留。長度大於1的drive根字串也會被這個while處理，不能把「保留長度1根」泛化成所有Windows drive／UNC根都已驗證。ForceDirectories逐字辨separator，不是filesystem canonicalization；未核中間mkdir不等於忽略最終結果，最終結果由DirectoryExists決定。

## 名稱與平台界線

SysUtils TU在_WIN32下include windows.h／direct.h，並undef與自身API衝突的巨集。`vcl_compat.h` 也在_WIN32 include windows.h後undef DeleteFile，再於 `VCLCOMPAT_NO_GLOBAL_USING` 未定義的區段匯出 `using vclcompat::DeleteFile`、exists／mkdir／path與日期函式。這保留一般umbrella入口下的名字選擇；不是每種自訂include／define組合的preprocessor或link驗證。

選定CreateDir保存Windows與非Windows兩支，不將POSIX fallback當成Windows部署結果。std::remove、stat、_mkdir與WindowsAPI實際錯誤、encoding、sharing、UNC／symlink／ACL行為，本輪沒有實測或全面規格查證。
