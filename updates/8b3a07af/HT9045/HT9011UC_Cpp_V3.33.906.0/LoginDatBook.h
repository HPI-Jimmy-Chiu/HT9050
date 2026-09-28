// =============================================================================
//  LoginDatBook.h -- Q9 (Steven S131 = B): write support for the BINARY password book, D:\HT9045\system\login.dat
//  (CosFunction.bUseLoginDatToSetLevel), on the raw 64004-byte image.  A RECORDED ADDITION: golden's handler never writes
//  this book in book mode (its ChangePassword takes the text path and FormClose restores memory, so nothing changes);
//  the external PW_Editor is the only writer (D:\HT9045\Password_V1.00.905_20260525\password_editor.cpp:152-202).
//
//  AI(W906-SEC-Q9) 20260927 (St02-E).  Header-only (like JamIniMerge.h): WebLogin.cpp and tests/test_login_dat_book.cpp
//  include it, so no CMake source list changes.  ctest: Security_LoginDatBook.
//
//  Layout = golden PASS_WORD (906_0625_Steven cprod.h:2684-2691; V906 cprod.h:2689-2695):
//    int RecordCT;  char ID[1000][30];  char PassWord[1000][30];  int Level[1000];   = 4 + 30000 + 30000 + 4000 = 64004
//    offsets RecordCT 0, ID 4, PassWord 30004, Level 60004.  WebLogin.cpp static_asserts that the port struct matches.
//  Fields are golden EncodeStr (common.cpp; key asKeyStr "HontechPassword"), zero-padded to 30 like PW_Editor's strncpy.
//  EncodeStr keeps the length and never emits 0, but it has no padding: a 30-byte value would leave no NUL (the golden
//  reader, AnsiString(char[30]), would over-read into the next field), and a plaintext 0x01 is not recoverable
//  (0x01 encodes to the key byte).  So a value must be 1..29 bytes and every plaintext byte >= 0x20.
//  RecordCT is never touched (PW_Editor writes 1000; golden readers ignore it).
//
//  Writes: backup the current file to <book>.bak_<yyyymmdd_hhnnss> (no backup -> no write), write the new image, read it
//  back and compare the WHOLE image, check that every byte outside the touched slot's three fields is unchanged, and on
//  any failure copy the backup back.  kKeepLatestBackup: keep the newest backup and prune older ones (St02-M's default,
//  asked of Steven; Q27 = A would delete it -- flip the constant).
//  Nothing here prints or logs; error texts never contain user input.
// =============================================================================
#pragma once

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "common.h"              // EncodeStr / DecodeStr (golden common.cpp:267-321)

namespace logindat {

const std::size_t kSize  = 64004;
const std::size_t kIdOff = 4;
const std::size_t kPwOff = 30004;
const std::size_t kLvOff = 60004;
const int         kSlots = 1000;
const std::size_t kField = 30;
const bool kKeepLatestBackup = true;     // false = delete the backup after a verified write (the Q27 = A policy)

typedef std::vector<unsigned char> Image;

inline unsigned char* IdField(Image& img, int slot) { return &img[kIdOff + kField * (std::size_t)slot]; }
inline unsigned char* PwField(Image& img, int slot) { return &img[kPwOff + kField * (std::size_t)slot]; }
inline const unsigned char* IdField(const Image& img, int slot) { return &img[kIdOff + kField * (std::size_t)slot]; }
inline const unsigned char* PwField(const Image& img, int slot) { return &img[kPwOff + kField * (std::size_t)slot]; }

inline int Level(const Image& img, int slot)
{
    int v = 0;
    std::memcpy(&v, &img[kLvOff + 4 * (std::size_t)slot], 4);
    return v;
}
inline void SetLevel(Image& img, int slot, int level) { std::memcpy(&img[kLvOff + 4 * (std::size_t)slot], &level, 4); }

// Exactly kSize bytes, or false (never creates the file).
inline bool Read(const std::string& path, Image* img)
{
    img->clear();
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    Image buf(kSize + 1);
    const std::size_t n = std::fread(&buf[0], 1, buf.size(), f);
    std::fclose(f);
    if (n != kSize) return false;
    buf.resize(kSize);
    img->swap(buf);
    return true;
}

// A field as text, bounded to 30 bytes (the golden reader is unbounded).
inline AnsiString FieldText(const unsigned char* f)
{
    std::size_t n = 0;
    while (n < kField && f[n]) ++n;
    return AnsiString(std::string(reinterpret_cast<const char*>(f), n).c_str());
}
inline AnsiString DecodedId(const Image& img, int slot) { return DecodeStr(FieldText(IdField(img, slot))); }
inline AnsiString DecodedPw(const Image& img, int slot) { return DecodeStr(FieldText(PwField(img, slot))); }

// First slot whose decoded ID equals user (UpperCase, like the login compare) at this level; -1 if none.
inline int FindUser(const Image& img, const AnsiString& user, int level)
{
    const AnsiString u = user.UpperCase();
    for (int i = 0; i < kSlots; ++i)
        if (Level(img, i) == level && IdField(img, i)[0] != 0 && DecodedId(img, i).UpperCase() == u) return i;
    return -1;
}
// Any slot with this user, any level (a New duplicate check).
inline bool UserExists(const Image& img, const AnsiString& user)
{
    const AnsiString u = user.UpperCase();
    for (int i = 0; i < kSlots; ++i)
        if (IdField(img, i)[0] != 0 && DecodedId(img, i).UpperCase() == u) return true;
    return false;
}
// PW_Editor's rule for an empty slot (password_editor.cpp:204-241 shows a slot only when ID[0] and PW[0] are both set).
inline int FirstFree(const Image& img)
{
    for (int i = 0; i < kSlots; ++i)
        if (IdField(img, i)[0] == 0 && PwField(img, i)[0] == 0) return i;
    return -1;
}

// 1..29 plaintext bytes, all >= 0x20 (see the banner).
inline bool ValueOk(const AnsiString& plain)
{
    const int n = plain.Length();
    if (n < 1 || n > (int)kField - 1) return false;
    const char* p = plain.c_str();
    for (int i = 0; i < n; ++i)
        if ((unsigned char)p[i] < 0x20) return false;
    return true;
}
// EncodeStr into a zero-padded 30-byte field.  false (field untouched) if the value is not allowed.
inline bool SetField(unsigned char* field, const AnsiString& plain)
{
    if (!ValueOk(plain)) return false;
    const AnsiString enc = EncodeStr(plain);
    if (enc.Length() != plain.Length()) return false;
    std::memset(field, 0, kField);
    std::memcpy(field, enc.c_str(), (std::size_t)enc.Length());
    return true;
}
// PW_Editor's delete (password_editor.cpp:186-189): both fields zeroed, level 0.
inline void ClearSlot(Image& img, int slot)
{
    std::memset(IdField(img, slot), 0, kField);
    std::memset(PwField(img, slot), 0, kField);
    SetLevel(img, slot, 0);
}

// Every byte outside slot's ID / PassWord / Level ranges is equal.
inline bool OnlySlotDiffers(const Image& a, const Image& b, int slot)
{
    if (a.size() != kSize || b.size() != kSize) return false;
    const std::size_t id = kIdOff + kField * (std::size_t)slot, pw = kPwOff + kField * (std::size_t)slot, lv = kLvOff + 4 * (std::size_t)slot;
    for (std::size_t i = 0; i < kSize; ++i)
    {
        if ((i >= id && i < id + kField) || (i >= pw && i < pw + kField) || (i >= lv && i < lv + 4)) continue;
        if (a[i] != b[i]) return false;
    }
    return true;
}

inline std::string BackupPathFor(const std::string& book)
{
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    char s[48];
    std::snprintf(s, sizeof(s), ".bak_%04d%02d%02d_%02d%02d%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return book + s;
}

// Delete <book>.bak_* except keep (the one just made).
inline void PruneBackups(const std::string& book, const std::string& keep)
{
    std::string dir = book;
    const std::size_t cut = dir.find_last_of("\\/");
    dir = (cut == std::string::npos) ? std::string() : dir.substr(0, cut + 1);
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((book + ".bak_*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do
    {
        const std::string p = dir + fd.cFileName;
        if (p != keep && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ::DeleteFileA(p.c_str());
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// ctest only (Security_LoginDatBook): called after the write and before the read-back, so a test can damage the file and
// check the restore.  Always null in wb_serve (nothing sets it).
typedef void (*AfterWriteHook)(const std::string& book);
inline AfterWriteHook& AfterWriteHookRef() { static AfterWriteHook h = 0; return h; }

// Write `after` over `book` (whose current bytes are `before`), touching only `slot`.  See the banner.
inline bool WriteVerified(const std::string& book, const Image& before, const Image& after, int slot, std::string* err)
{
    if (!OnlySlotDiffers(before, after, slot)) { *err = "internal: the new image changes more than one slot"; return false; }
    const std::string bak = BackupPathFor(book);
    if (!::CopyFileA(book.c_str(), bak.c_str(), FALSE)) { *err = "cannot back up the password book; nothing written"; return false; }
    bool ok = false;
    FILE* f = std::fopen(book.c_str(), "wb");
    if (f)
    {
        ok = std::fwrite(&after[0], 1, kSize, f) == kSize;
        ok = (std::fclose(f) == 0) && ok;
    }
    if (ok && AfterWriteHookRef()) AfterWriteHookRef()(book);
    Image back;
    if (ok) ok = Read(book, &back) && back == after && OnlySlotDiffers(before, back, slot);
    if (!ok)
    {
        ::CopyFileA(bak.c_str(), book.c_str(), FALSE);                         // restore; the backup is kept
        *err = "the password book did not verify after the write; restored from " + bak;
        return false;
    }
    if (kKeepLatestBackup) PruneBackups(book, bak);
    else ::DeleteFileA(bak.c_str());
    return true;
}

}  // namespace logindat
