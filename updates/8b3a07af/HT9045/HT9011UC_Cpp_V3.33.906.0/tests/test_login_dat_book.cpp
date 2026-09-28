// =============================================================================
//  test_login_dat_book.cpp -- Q9 (Steven S131 = B): LoginDatBook.h, the write support for the BINARY password book
//  (login.dat, CosFunction.bUseLoginDatToSetLevel) behind WebLogin.cpp W906_PwBinaryBook.
//
//  AI(W906-SEC-Q9) 20260927 (St02-E).  Suite name (add_test): Security_LoginDatBook
//
//    1. EncodeStr vectors (golden common.cpp:267-321, key "HontechPassword") and round trips; 0x01 does not round-trip,
//       so ValueOk refuses it;
//    2. a PW_Editor-style image (RecordCT 1000, slots 0..2 and 4 used, slot 3 free, bytes after a NUL): FindUser /
//       UserExists / FirstFree / DecodedPw, and FieldText stays inside 30 bytes;
//    3. New: the first free slot, the file stays 64004 bytes, every byte outside the slot is unchanged, the golden read
//       loop (WebLogin_BookLogin / main.cpp cbUserSelectChange: AnsiString(char[30]) + DecodeStr) finds user, level and
//       password, the backup is the old file, an older backup is pruned (kKeepLatestBackup, Steven Q9 backup = A);
//    4. Edit: only the slot's 30 PassWord bytes change; a wrong old password (the handler's check) writes nothing;
//    5. Delete: the slot is zeroed with level 0 (PW_Editor) and is free again;
//    6. refusals: a 30-byte value, a byte < 0x20, an empty value; 64003 / 64005 bytes and a missing file (not created);
//       an image that changes two slots (nothing written, no new backup);
//    7. the file damaged after the write (AfterWriteHookRef): false, restored byte for byte, the backup kept;
//    8. no plaintext: stdout / stderr captured during a New / Edit / Delete stay empty; no error text, written book or
//       backup contains a test password.
//  Every file is under %TEMP%\ht9045_q9_<tick> (the test stops if that is under D:\HT9045).  D:\HT9045\system\login.dat
//  is never opened; its size and write time are checked unchanged.  The sandbox is removed on a green run.
// =============================================================================
#include "LoginDatBook.h"

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

using logindat::Image;

static const char* const kPw1 = "Tst#Pw9x";                    // never printed
static const char* const kPw2 = "NewPw\xA4\xA4\xA4\xE5";       // cp950 bytes, never printed
static std::vector<std::string> g_errs;                         // every error text, checked in section 8

static std::string Hex(const AnsiString& s)
{
    std::string o;
    char b[4];
    for (int i = 1; i <= s.Length(); ++i) { std::snprintf(b, sizeof(b), "%02X", (unsigned char)s[i]); o += b; }
    return o;
}

static bool WriteRaw(const std::string& p, const void* data, std::size_t n)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(data, 1, n, f) == n;
    return (std::fclose(f) == 0) && ok;
}

static std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char buf[4096];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

static std::vector<std::string> Backups(const std::string& book)
{
    std::vector<std::string> out;
    const std::size_t cut = book.find_last_of("\\/");
    const std::string dir = book.substr(0, cut + 1);
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((book + ".bak_*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do out.push_back(dir + fd.cFileName); while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return out;
}

static bool HasBytes(const std::string& hay, const char* needle) { return hay.find(needle) != std::string::npos; }

// Golden's reader on the written file: fread the whole PASS_WORD, then AnsiString(ID[i]) / AnsiString(PassWord[i]) +
// DecodeStr (906_0625_Steven main.cpp cbUserSelectChange; V906 WebLogin.cpp WebLogin_BookLogin :411-440).
struct GoldenPassWord { int RecordCT; char ID[1000][30]; char PassWord[1000][30]; int Level[1000]; };
static_assert(sizeof(GoldenPassWord) == 64004, "golden PASS_WORD is 64004 bytes");

static bool GoldenFinds(const std::string& book, const char* user, int level, const char* pw)
{
    static GoldenPassWord u;
    std::memset(&u, 0, sizeof(u));
    FILE* f = std::fopen(book.c_str(), "rb");
    if (!f) return false;
    std::fread((char*)&u.RecordCT, sizeof(GoldenPassWord), 1, f);
    std::fclose(f);
    for (int i = 0; i < 1000; i++)
    {
        const AnsiString id = DecodeStr(AnsiString(u.ID[i]));
        if (id == "") continue;
        if (id == AnsiString(user) && u.Level[i] == level && DecodeStr(AnsiString(u.PassWord[i])) == AnsiString(pw)) return true;
    }
    return false;
}

static Image EditorImage()
{
    Image img(logindat::kSize, 0);
    const int rc = 1000;
    std::memcpy(&img[0], &rc, 4);                                               // PW_Editor writes RecordCT = 1000
    logindat::SetField(logindat::IdField(img, 0), "operator"); logindat::SetField(logindat::PwField(img, 0), "op1");  logindat::SetLevel(img, 0, 1);
    logindat::SetField(logindat::IdField(img, 1), "Engineer"); logindat::SetField(logindat::PwField(img, 1), "eng2"); logindat::SetLevel(img, 1, 2);
    logindat::SetField(logindat::IdField(img, 2), "super");    logindat::SetField(logindat::PwField(img, 2), "sup3"); logindat::SetLevel(img, 2, 3);
    logindat::SetField(logindat::IdField(img, 4), "tech");     logindat::SetField(logindat::PwField(img, 4), "tec4"); logindat::SetLevel(img, 4, 1);
    logindat::IdField(img, 1)[20] = 0x55;                                       // bytes after the NUL (old data a reader must ignore)
    return img;
}

static void DamageOutsideSlot(const std::string& book)                         // AfterWriteHookRef target, section 7
{
    FILE* f = std::fopen(book.c_str(), "r+b");
    if (!f) return;
    std::fseek(f, 50000, SEEK_SET);                                             // PassWord area, far from slot 0
    std::fputc(0x5A, f);
    std::fclose(f);
}

int main()
{
    printf("test_login_dat_book (Security_LoginDatBook)\n");

    // ---- the machine's own login.dat: never opened here; record it to check it is unchanged ----
    const char* const kReal = "D:\\HT9045\\system\\login.dat";
    WIN32_FILE_ATTRIBUTE_DATA real0;
    const bool realThere0 = ::GetFileAttributesExA(kReal, GetFileExInfoStandard, &real0) != 0;

    // ---- sandbox ----
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    const std::string sb = std::string(tmp) + "ht9045_q9_" + tick;
    std::string up = sb;
    for (std::size_t i = 0; i < up.size(); ++i) up[i] = (char)std::toupper((unsigned char)up[i]);
    if (up.compare(0, 9, "D:\\HT9045") == 0) { printf("  FAIL: sandbox %s is under D:\\HT9045 -- stopped\n", sb.c_str()); return 2; }
    ::CreateDirectoryA(sb.c_str(), NULL);
    const std::string book = sb + "\\login.dat";
    printf("  sandbox %s\n", sb.c_str());

    // =========================================================================
    printf("1. EncodeStr vectors\n");
    CHECK(Hex(EncodeStr("ABC")) == "082E2C", "\"ABC\" -> 08 2E 2C");
    CHECK(Hex(EncodeStr("I")) == "48", "\"I\" -> 48 (0 becomes the key byte)");
    CHECK(Hex(EncodeStr("0123456789ABCDEFG")) == "675F5F4656575D66564B33362D31200D29", "17 bytes: the key wraps after 15");
    CHECK(Hex(EncodeStr("\xA4\xA4")) == "EBCC", "cp950 A4 A4 -> EB CC");
    CHECK(DecodeStr(EncodeStr(kPw1)) == AnsiString(kPw1), "printable password round-trips");
    CHECK(DecodeStr(EncodeStr(kPw2)) == AnsiString(kPw2), "cp950 password round-trips");
    CHECK(Hex(EncodeStr("\x01")) == "48" && DecodeStr(EncodeStr("\x01")) == AnsiString("I"), "0x01 encodes to 48 and decodes to \"I\"");
    CHECK(!logindat::ValueOk("\x01"), "ValueOk refuses 0x01");
    CHECK(logindat::ValueOk(AnsiString(std::string(29, 'a').c_str())) && !logindat::ValueOk(AnsiString(std::string(30, 'a').c_str())),
          "ValueOk: 29 bytes yes, 30 bytes no");

    // =========================================================================
    printf("2. a PW_Editor-style image\n");
    const Image ed = EditorImage();
    CHECK(ed.size() == 64004, "image is 64004 bytes");
    CHECK(logindat::FirstFree(ed) == 3, "first free slot = 3");
    CHECK(logindat::FindUser(ed, "ENGINEER", 2) == 1, "FindUser is case-insensitive (UpperCase, like the login compare)");
    CHECK(logindat::FindUser(ed, "Engineer", 1) == -1, "FindUser needs the level");
    CHECK(logindat::UserExists(ed, "TECH") && !logindat::UserExists(ed, "nobody"), "UserExists at any level");
    CHECK(logindat::DecodedId(ed, 1) == AnsiString("Engineer"), "bytes after a NUL are ignored");
    CHECK(logindat::DecodedPw(ed, 2) == AnsiString("sup3"), "DecodedPw");
    {
        Image full = ed;
        std::memset(logindat::IdField(full, 9), 'Z', 30);
        CHECK(logindat::FieldText(logindat::IdField(full, 9)).Length() == 30, "a 30-byte field reads 30 bytes (the golden reader would run on)");
    }

    // =========================================================================
    printf("3. New\n");
    CHECK(WriteRaw(book, &ed[0], ed.size()), "sandbox book written");
    const std::string oldBak = book + ".bak_20000101_000000";
    CHECK(WriteRaw(oldBak, &ed[0], ed.size()), "an older backup exists");
    Image cur;
    CHECK(logindat::Read(book, &cur) && cur == ed, "Read = the image");
    int slot = logindat::FirstFree(cur);
    Image after = cur;
    CHECK(logindat::SetField(logindat::IdField(after, slot), "newUser") && logindat::SetField(logindat::PwField(after, slot), kPw1),
          "SetField ID and PassWord");
    logindat::SetLevel(after, slot, 2);
    std::string err;
    bool ok = logindat::WriteVerified(book, cur, after, slot, &err);
    g_errs.push_back(err);
    CHECK(ok, "WriteVerified New");
    std::string now = Bytes(book);
    CHECK(now.size() == 64004, "still 64004 bytes");
    {
        Image nowImg(now.begin(), now.end());
        CHECK(nowImg == after && logindat::OnlySlotDiffers(cur, nowImg, slot), "the file = the new image; only slot 3 differs");
    }
    CHECK(GoldenFinds(book, "newUser", 2, kPw1), "the golden read loop finds the new user, level and password");
    CHECK(GoldenFinds(book, "Engineer", 2, "eng2") && GoldenFinds(book, "tech", 1, "tec4"), "the golden read loop still finds the old users");
    {
        const std::vector<std::string> b = Backups(book);
        CHECK(b.size() == 1 && b[0] != oldBak && Bytes(b[0]) == std::string(ed.begin(), ed.end()),
              "one backup kept (the old file); the older one pruned");
    }

    // =========================================================================
    printf("4. Edit\n");
    CHECK(logindat::Read(book, &cur), "re-read");
    slot = logindat::FindUser(cur, "NEWUSER", 2);
    CHECK(slot == 3 && logindat::DecodedPw(cur, slot) == AnsiString(kPw1), "FindUser + DecodedPw = the old password");
    {
        const std::string before = Bytes(book);
        const bool match = (logindat::DecodedPw(cur, slot) == AnsiString("wrongpw"));
        CHECK(!match && Bytes(book) == before, "a wrong old password does not match; nothing written");
    }
    after = cur;
    CHECK(logindat::SetField(logindat::PwField(after, slot), kPw2), "SetField new password");
    ok = logindat::WriteVerified(book, cur, after, slot, &err);
    g_errs.push_back(err);
    CHECK(ok, "WriteVerified Edit");
    now = Bytes(book);
    {
        bool onlyPw = now.size() == 64004;
        const std::size_t pw0 = logindat::kPwOff + 30 * (std::size_t)slot;
        for (std::size_t i = 0; onlyPw && i < now.size(); ++i)
            if ((unsigned char)now[i] != cur[i] && !(i >= pw0 && i < pw0 + 30)) onlyPw = false;
        CHECK(onlyPw, "only the slot's 30 PassWord bytes changed");
    }
    CHECK(GoldenFinds(book, "newUser", 2, kPw2) && !GoldenFinds(book, "newUser", 2, kPw1), "the golden reader sees the new password only");

    // =========================================================================
    printf("5. Delete\n");
    CHECK(logindat::Read(book, &cur), "re-read");
    slot = logindat::FindUser(cur, "newUser", 2);
    after = cur;
    logindat::ClearSlot(after, slot);
    ok = logindat::WriteVerified(book, cur, after, slot, &err);
    g_errs.push_back(err);
    CHECK(ok, "WriteVerified Delete");
    CHECK(logindat::Read(book, &cur), "re-read");
    {
        bool zero = logindat::Level(cur, slot) == 0;
        for (std::size_t i = 0; i < 30; ++i) zero = zero && logindat::IdField(cur, slot)[i] == 0 && logindat::PwField(cur, slot)[i] == 0;
        CHECK(zero, "ID and PassWord zeroed, level 0 (PW_Editor's delete)");
    }
    CHECK(logindat::FirstFree(cur) == slot && !logindat::UserExists(cur, "newUser"), "the slot is free again; the user is gone");
    CHECK(!GoldenFinds(book, "newUser", 2, kPw2), "the golden reader no longer finds it");

    // =========================================================================
    printf("6. refusals\n");
    {
        unsigned char f[30];
        std::memset(f, 0x77, sizeof(f));
        const bool r30 = logindat::SetField(f, AnsiString(std::string(30, 'a').c_str()));
        const bool rCtl = logindat::SetField(f, "ab\x1f");
        const bool r01 = logindat::SetField(f, "\x01");
        const bool rEmpty = logindat::SetField(f, "");
        bool untouched = true;
        for (std::size_t i = 0; i < 30; ++i) untouched = untouched && f[i] == 0x77;
        CHECK(!r30 && !rCtl && !r01 && !rEmpty && untouched, "30 bytes / a control byte / 0x01 / empty refused, field untouched");
    }
    {
        const std::string p3 = sb + "\\short.dat", p5 = sb + "\\long.dat", pm = sb + "\\missing.dat";
        std::vector<char> z(64005, 0);
        WriteRaw(p3, &z[0], 64003);
        WriteRaw(p5, &z[0], 64005);
        Image x;
        CHECK(!logindat::Read(p3, &x) && !logindat::Read(p5, &x), "64003 and 64005 bytes are not a login.dat");
        CHECK(!logindat::Read(pm, &x) && !Exists(pm), "a missing file is not created");
    }
    {
        CHECK(logindat::Read(book, &cur), "re-read");
        const std::string before = Bytes(book);
        const std::size_t nb = Backups(book).size();
        after = cur;
        logindat::SetField(logindat::IdField(after, 5), "x5");
        logindat::SetField(logindat::IdField(after, 6), "x6");
        ok = logindat::WriteVerified(book, cur, after, 5, &err);
        g_errs.push_back(err);
        CHECK(!ok && err.find("internal") == 0, "an image that changes two slots is refused");
        CHECK(Bytes(book) == before && Backups(book).size() == nb, "nothing written, no new backup");
    }

    // =========================================================================
    printf("7. damaged after the write -> restored\n");
    {
        CHECK(logindat::Read(book, &cur), "re-read");
        const std::string before = Bytes(book);
        after = cur;
        logindat::SetField(logindat::PwField(after, 0), "op1new");
        logindat::AfterWriteHookRef() = &DamageOutsideSlot;
        ok = logindat::WriteVerified(book, cur, after, 0, &err);
        logindat::AfterWriteHookRef() = 0;
        g_errs.push_back(err);
        CHECK(!ok && err.find("restored from") != std::string::npos, "the read-back fails; WriteVerified says restored");
        CHECK(Bytes(book) == before, "the book is the old file byte for byte");
        bool kept = false;
        const std::vector<std::string> b = Backups(book);
        for (std::size_t i = 0; i < b.size(); ++i) if (Bytes(b[i]) == before) kept = true;
        CHECK(kept, "the backup is kept");
    }

    // =========================================================================
    printf("8. no plaintext\n");
    {
        const std::string cap = sb + "\\capture.txt";
        std::fflush(stdout); std::fflush(stderr);
        const int save1 = _dup(1), save2 = _dup(2);
        const int fd = _open(cap.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
        bool okNew = false, okEdit = false, okDel = false, plainInBook = true, encInBook = false;
        if (fd >= 0 && save1 >= 0 && save2 >= 0)
        {
            _dup2(fd, 1); _dup2(fd, 2); _close(fd);
            Image c, a;
            logindat::Read(book, &c);
            int s = logindat::FirstFree(c);
            a = c;
            logindat::SetField(logindat::IdField(a, s), "probe8");
            logindat::SetField(logindat::PwField(a, s), kPw1);
            logindat::SetLevel(a, s, 1);
            okNew = logindat::WriteVerified(book, c, a, s, &err); g_errs.push_back(err);
            logindat::Read(book, &c);
            a = c;
            logindat::SetField(logindat::PwField(a, s), kPw2);
            okEdit = logindat::WriteVerified(book, c, a, s, &err); g_errs.push_back(err);
            const std::string raw = Bytes(book);
            plainInBook = HasBytes(raw, kPw1) || HasBytes(raw, kPw2);
            encInBook = HasBytes(raw, EncodeStr(kPw2).c_str());
            logindat::Read(book, &c);
            a = c;
            logindat::ClearSlot(a, s);
            okDel = logindat::WriteVerified(book, c, a, s, &err); g_errs.push_back(err);
            std::fflush(stdout); std::fflush(stderr);
            _dup2(save1, 1); _dup2(save2, 2);
        }
        if (save1 >= 0) _close(save1);
        if (save2 >= 0) _close(save2);
        CHECK(okNew && okEdit && okDel, "New / Edit / Delete under capture");
        CHECK(Exists(cap) && Bytes(cap).empty(), "nothing on stdout / stderr");
        CHECK(!plainInBook && encInBook, "the book holds the encoded password, never the plaintext");
        bool clean = true;
        const std::vector<std::string> b = Backups(book);
        for (std::size_t i = 0; i < b.size(); ++i)
        {
            const std::string r = Bytes(b[i]);
            if (HasBytes(r, kPw1) || HasBytes(r, kPw2)) clean = false;
        }
        for (std::size_t i = 0; i < g_errs.size(); ++i)
            if (HasBytes(g_errs[i], kPw1) || HasBytes(g_errs[i], kPw2) || HasBytes(g_errs[i], "probe8")) clean = false;
        CHECK(clean, "no backup and no error text contains a test password");
    }

    // ---- the machine's login.dat is unchanged ----
    {
        WIN32_FILE_ATTRIBUTE_DATA real1;
        const bool realThere1 = ::GetFileAttributesExA(kReal, GetFileExInfoStandard, &real1) != 0;
        const bool same = (realThere0 == realThere1) &&
            (!realThere0 || (real0.nFileSizeLow == real1.nFileSizeLow && real0.nFileSizeHigh == real1.nFileSizeHigh &&
                             CompareFileTime(&real0.ftLastWriteTime, &real1.ftLastWriteTime) == 0));
        CHECK(same, "D:\\HT9045\\system\\login.dat: same size and write time");
    }

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = ::FindFirstFileA((sb + "\\*").c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ::DeleteFileA((sb + "\\" + fd.cFileName).c_str());
            while (::FindNextFileA(h, &fd));
            ::FindClose(h);
        }
        ::RemoveDirectoryA(sb.c_str());
    }
    return g_fail == 0 ? 0 : 1;
}
