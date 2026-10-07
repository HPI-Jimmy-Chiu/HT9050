#pragma once
// State Record archive helpers. Unicode paths stay Unicode throughout cleanup.
// Only a timestamp-named direct child may be removed; never follow junctions.
#include <windows.h>
#include <wincrypt.h>
#include <string>

namespace w906_sr {
inline std::wstring FromAnsi(const char* text) {
    const int n = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0);
    if (n <= 0) return std::wstring();
    std::wstring out(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_ACP, 0, text, -1, &out[0], n);
    out.resize(static_cast<size_t>(n - 1));
    return out;
}
inline bool TimestampName(const std::wstring& s) {
    if (s.size() != 19) return false;
    for (size_t i = 0; i < s.size(); ++i) {
        const wchar_t sep = (i == 4 || i == 7) ? L'-' :
            i == 10 ? L' ' : (i == 13 || i == 16) ? L'_' : L'\0';
        if (sep ? s[i] != sep : (s[i] < L'0' || s[i] > L'9')) return false;
    }
    return true;
}
inline bool RemoveTree(const std::wstring& path, DWORD& error) {
    const DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        error = GetLastError();
        return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
    }
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) && !(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
        WIN32_FIND_DATAW data;
        HANDLE find = FindFirstFileW((path + L"\\*").c_str(), &data);
        if (find == INVALID_HANDLE_VALUE) { error = GetLastError(); return false; }
        bool ok = true;
        do {
            if (std::wstring(data.cFileName) == L"." || std::wstring(data.cFileName) == L"..") continue;
            if (!RemoveTree(path + L"\\" + data.cFileName, error)) { ok = false; break; }
        } while (FindNextFileW(find, &data));
        const DWORD end = GetLastError();
        FindClose(find);
        if (!ok) return false;
        if (end != ERROR_NO_MORE_FILES) { error = end; return false; }
    }
    if ((attr & FILE_ATTRIBUTE_READONLY) && !(attr & FILE_ATTRIBUTE_REPARSE_POINT) &&
        !SetFileAttributesW(path.c_str(), attr & ~FILE_ATTRIBUTE_READONLY)) {
        error = GetLastError(); return false;
    }
    const BOOL ok = (attr & FILE_ATTRIBUTE_DIRECTORY) ? RemoveDirectoryW(path.c_str()) : DeleteFileW(path.c_str());
    if (!ok) error = GetLastError();
    return ok != FALSE;
}
inline bool Cleanup(const std::wstring& parent, const std::wstring& leaf, DWORD& error) {
    error = ERROR_INVALID_NAME;
    if (parent.empty() || !TimestampName(leaf)) return false;
    error = ERROR_SUCCESS;
    return RemoveTree(parent + L"\\" + leaf, error);
}
inline bool Sha256(const std::wstring& path, std::string& hex, DWORD& error) {
    hex.clear(); error = ERROR_SUCCESS;
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE) { error = GetLastError(); return false; }
    HCRYPTPROV provider = 0;
    HCRYPTHASH hash = 0;
    bool ok = CryptAcquireContextW(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) != FALSE;
    // MinGW 6.3's old wincrypt.h lacks CALG_SHA_256; Windows SDK defines SHA-256 SID as 12.
    const ALG_ID sha256 = ALG_CLASS_HASH | ALG_TYPE_ANY | 12;
    if (ok) ok = CryptCreateHash(provider, sha256, 0, 0, &hash) != FALSE;
    if (!ok) error = GetLastError();
    BYTE block[65536]; DWORD size = 0;
    while (ok) {
        if (!ReadFile(file, block, sizeof(block), &size, NULL)) { error = GetLastError(); ok = false; break; }
        if (!size) break;
        if (!CryptHashData(hash, block, size, 0)) { error = GetLastError(); ok = false; }
    }
    BYTE digest[32]; DWORD len = sizeof(digest);
    if (ok && !CryptGetHashParam(hash, HP_HASHVAL, digest, &len, 0)) { error = GetLastError(); ok = false; }
    if (ok) {
        const char* digits = "0123456789abcdef";
        for (DWORD i = 0; i < len; ++i) { hex += digits[digest[i] >> 4]; hex += digits[digest[i] & 15]; }
    }
    if (hash) CryptDestroyHash(hash);
    if (provider) CryptReleaseContext(provider, 0);
    CloseHandle(file);
    return ok;
}
} // namespace w906_sr
