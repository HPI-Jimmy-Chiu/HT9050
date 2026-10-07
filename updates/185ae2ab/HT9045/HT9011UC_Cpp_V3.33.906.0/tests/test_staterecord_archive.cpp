#include "StateRecordArchive.h"
#include <cstdio>

static int failures=0;
#define CHECK(c) do { if(!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while(0)
static void Put(const std::wstring& p, const char* bytes, DWORD n) {
    HANDLE f=CreateFileW(p.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CHECK(f!=INVALID_HANDLE_VALUE);
    if(f!=INVALID_HANDLE_VALUE) { DWORD wrote=0; CHECK(WriteFile(f,bytes,n,&wrote,NULL) && wrote==n); CloseHandle(f); }
}
int main() {
    wchar_t tmp[MAX_PATH], unique[MAX_PATH];
    CHECK(GetTempPathW(MAX_PATH,tmp)>0);
    CHECK(GetTempFileNameW(tmp,L"sr",0,unique)!=0);
    CHECK(DeleteFileW(unique)); CHECK(CreateDirectoryW(unique,NULL));
    const std::wstring root(unique), leaf=L"2026-10-06 19_04_29", dir=root+L"\\"+leaf;
    DWORD err=0;
    CHECK(!w906_sr::Cleanup(root,L"..",err));
    CHECK(!w906_sr::Cleanup(root,L"2026-10-06 19_04_29\\..",err));
    CHECK(!w906_sr::Cleanup(root,L"",err));
    CHECK(CreateDirectoryW(dir.c_str(),NULL));
    CHECK(CreateDirectoryW((dir+L"\\nested").c_str(),NULL));
    const std::wstring odd=dir+L"\\nested\\+\u00a6+s -\u6e2c\u8a66.txt";
    Put(odd,"abc",3);
    std::string hash;
    CHECK(w906_sr::Sha256(odd,hash,err));
    CHECK(hash=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(SetFileAttributesW(odd.c_str(),FILE_ATTRIBUTE_READONLY));
    CHECK(w906_sr::Cleanup(root,leaf,err));
    CHECK(GetFileAttributesW(dir.c_str())==INVALID_FILE_ATTRIBUTES);
    CHECK(CreateDirectoryW(dir.c_str(),NULL));
    const std::wstring locked=dir+L"\\locked.txt";
    Put(locked,"",0);
    CHECK(w906_sr::Sha256(locked,hash,err));
    CHECK(hash=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    HANDLE f=CreateFileW(locked.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    CHECK(f!=INVALID_HANDLE_VALUE);
    CHECK(!w906_sr::Cleanup(root,leaf,err));
    CHECK(err==ERROR_SHARING_VIOLATION);
    CHECK(GetFileAttributesW(locked.c_str())!=INVALID_FILE_ATTRIBUTES);
    CloseHandle(f);
    CHECK(w906_sr::Cleanup(root,leaf,err));
    CHECK(!w906_sr::Sha256(locked,hash,err) && hash.empty());
    CHECK(RemoveDirectoryW(root.c_str()));
    std::printf("StateRecordArchive: %d failures\n",failures);
    return failures ? 1 : 0;
}
