# WSA refs與Sync.h原契約

[上層](index.md)；[WsaAcquire](raw/source-01.md)／[WsaRelease](raw/source-02.md)；[nonblocking](raw/source-03.md)／[error文字](raw/source-04.md)。
[Sync.h全文](raw/source-08.md)；[WSA globals與原理由](raw/source-09.md)。

WsaAcquire以WbGuard鎖g_wsaMx。g_wsaRefs>0時只加一回true；否則WSAStartup(MAKEWORD(2,2),&wsad)。
rc非0，err非空才寫「WSAStartup failed, rc=...」，回false；成功令g_wsaRefs=1。
wsad後續內容／協商版本未在此查驗，也未核其他模組的WSA calls；host app共存說明保留為原comment。
WsaRelease持同一鎖，refs<=0即return；減一到0才WSACleanup，沒有檢查cleanup回傳值。
SetNonBlocking用u_long nb=1呼叫ioctlsocket(FIONBIO)，丟棄回傳；名稱不是設定成功證據。
WsaErrText只組what與傳入code文字，本body不自行讀WSAGetLastError，也不證明code就是當前OS error。

| Sync.h選定正文 | 此來源可直接核對的行為 |
|---|---|
| 平台guard | _WIN32才include windows.h；其他平台#error |
| WbMutex | constructor／destructor呼叫Initialize／DeleteCriticalSection，lock／unlock用Enter／Leave |
| WbGuard | constructor lock，destructor unlock；copy declarations私有 |
| WbThread constructor | h_=NULL、id_=0 |
| start | 有handle回false；new Payload保存fn／arg；CreateThread回NULL時delete Payload並false，否則true |
| joinable | 只看h_!=NULL，不驗thread當下是否仍在執行 |
| join | handle空即return；WaitForSingleObject(INFINITE)、CloseHandle、h_=NULL、id_=0；不核兩個API結果 |
| destructor | 有handle便CloseHandle、h_=NULL；正文沒有join或stopFlag更新 |
| Trampoline | 複製Payload的fn／arg後delete；fn非空才呼叫，最後return 0 |
| WbSleepMs | Sleep(static_cast<DWORD>(ms))，原20260926修正comment保留 |

整份header作兩context之一，inline functions只保存，不增加七CPP的完成數；既有WbSleepMs等摘錄不重算。
原MinGW.org GCC6.3 win32 thread model、atomic probe與lock重入說明是歷史metadata；本輪沒有重做compiler／link／runtime驗證。
header的「self-connect」說明與Server.cpp的UDP wake實作分別保留，以Start／Wake正文定位現行路徑，勿將原說明當新平台裁決。
也不把destructor原「detaches」說明擴成未查證的OS保證；這裡只記實際CloseHandle呼叫及沒有join。
