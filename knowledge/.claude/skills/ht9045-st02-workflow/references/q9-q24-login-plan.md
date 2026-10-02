# Q9 + Q24: login.dat book-mode write, levelset save → SavePassword / ReadPassword

> **DONE, pushed 20260927 14:4x** to `v906/steven-st02-on-cbridge` `7f0c24b1`: `64e2c048` Q9 (LoginDatBook.h, WebLogin.cpp :411 / :923-939 / EOF, JS :474 / :555, ctest Security_LoginDatBook), `d1a2aee3` merge St01 de534b23, `7f0c24b1` Q24 (WebLevelSet.cpp :40 / :42 / :121 / :124 / :620-622 / :624). Both configs 0 errors. **Waiting:** St01 merges it back and runs Security_LoginDatBook + a W906_PWBOOK_PATH webprobe. The rest of this file is the plan as written before coding.

> **20260927 later (St02-E):** Q9 backup = **A** (Steven: keep the latest `<book>.bak_<ts>`, prune the older ones; `LoginDatBook.h kKeepLatestBackup=true`). **Q24 cleared by ST01-M** (FROM_STEVEN §4 13:50): WebLevelSet.cpp :40 / :42 / :121-124 / :620-624 line-neutral, on top of dd3aee54; :624 = `SkipPasswordGuard skipPw(W906EnvSet("W906_LOGINDAT_PATH") || W906EnvSet("W906_PWBOOK_PATH") || W906EnvSet("W906_LEVELSET_PATH"))`; post the diff to ST01-E before it merges. ST01-E agrees with the USER reload after a Q9 write. The code is in st02-on-cbridge (see current-state.md). St01's skill `ht9045-login` (a0629eaa) adds: PW_Editor V1.00.648 edits only the first 30 slots and shows a 'key byte + 1' character wrongly (its EncodeStr writes 00 there); use the handler EncodeStr (done).
> Status 20260927: **GO** on `v906/steven-st02-on-cbridge` (worktree `D:\AI_TempFile\st02-on-cbridge`, local 74bea5db includes
> St01 c9d0dd00). Read-only plan done (below), sent to St02-M; **code after St02-M confirms the plan** (and St01 OKs Q24's
> WebLevelSet.cpp lines, which are St01's and were NOT handed over).
> `WebLogin.cpp` is St02's (FROM_STEVEN §4 20260927 12:30). St01 is writing skill `ht9045-login` (not pushed yet).
> Paths below relative to `D:\AI_TempFile\st02-on-cbridge\HT9011UC_Cpp_V3.33.906.0\`; golden = 906_0625_Steven (bodies = 912).

## Rulings
- S132 Q10 = B1: storage follows golden; web transport unchanged; no code change for Q10.
- S131 Q9 = B: book-mode login.dat write support (a recorded addition: golden's handler never writes the book;
  PW_Editor does). EncodeStr, keep PASS_WORD 64004 bytes. No plaintext on the wire or in logs.
- Q24: levelset save calls SavePassword / ReadPassword as golden (cSecurity.cpp FormClose :462-466).
- S133 Q11 = B: done by St01 (c9d0dd00). Don't touch.

## Golden facts [read]
- PASS_WORD (cprod.h:2684-2691): `int RecordCT; char ID[1000][30]; char PassWord[1000][30]; int Level[1000];` = 4+30000+30000+4000
  = **64004**; offsets RecordCT 0, ID 4, PassWord 30004, Level 60004 (no padding). The port struct is identical (cprod.h:2689-2695);
  no static_assert exists. Local D:\HT9045\system\login.dat is 64004 bytes, RecordCT=1000, 8 used slots.
- One file, D:\HT9045\system\login.dat, two modes. Flag `CosFunction.bUseLoginDatToSetLevel` (CosFunction.h:90; true for JSCC_OS,
  SCC 943, SJ 791, JCET, …; default false). pwPath (main.cpp:10407-10427): AMKOR_China / QUALCOMM → userid.com; flag → login.dat;
  bOEEFunction → ""; else C:\winnt\system32\tech.com. Book when FileExists(pwPath) || flag (:10429-10439).
  - Book (flag on): cbUserSelectChange main.cpp:14463-14506: fread whole struct; per slot DecodeStr(ID), DecodeStr(PassWord), Level.
  - Dropdown: slots 0..2 are PLAINTEXT (cSecurity.cpp:807-809 strncpy + SavePassword; main.cpp:12729-12797 plaintext compares).
- ReadPassword cprod.cpp:1374-1386 (literal path; missing → memset 0), boot main.cpp:8993, cinitial.cpp:5844.
  SavePassword cprod.cpp:1360-1372: raw fwrite of the 64004-byte USER; failure → WAR1682. Called only at cSecurity.cpp:463
  (FormClose) and :809 (dropdown ChangePassword). **The handler never EncodeStr's login.dat.**
- FormClose cSecurity.cpp:462 SaveJamLevel / :463 SavePassword / :464 SetLevelSet / :466 ReadPassword (912 :463/:464/:465/:467).
- Golden book-mode defect: ChangePassword isn't gated by the flag and takes the TEXT path on the binary file (LoadFromFile :624,
  SaveToFile :790/:661 clobber it), then FormClose SavePassword rewrites memory USER ⇒ net no change.
- PW_Editor (D:\HT9045\Password_V1.00.905_20260525\password_editor.cpp): SavePassword :152-202 RecordCT=1000, EncodeStr ID and PW,
  strncpy 30 zero-pad, compacts to the front, Level 0..3; ReadPassword :204-241 shows a slot if ID[0] && PW[0].
- EncodeStr / DecodeStr common.cpp:267-293 / :295-321, asKeyStr = golden literal (not written here) (:46 and InitCommonString :201):
  encode t=(c-1)^key[p], t==0 → key[p]; decode e==key[p] → key[p] else e^key[p], then +1; key cycles every 15; same length;
  output never contains 0; NOT injective at 0x01; no padding (a 30-byte value leaves no NUL → golden reader over-reads).
  Vectors: "ABC" → 08 2E 2C; "I" → 48; "0123456789ABCDEFG" → 67 5F 5F 46 56 57 5D 66 56 4B 33 36 2D 31 20 0D 29;
  "\x01" → 48 (decodes to "I"); A4 A4 → EB CC.

## V906 today [read]
- cprod.cpp:1458-1484 SavePassword / ReadPassword live, literal paths, no seam; boot reads wb_serve.cpp:3471, cinitial.cpp:16992.
- common.cpp:495-545 EncodeStr / DecodeStr live (tests/test_common.cpp:266-276 round-trip only); asKeyStr static :238 / :423.
- cSecurity.cpp:687-901 ChangePassword `#if 0` (GATE SEC7). FormClose :510-536: :532 `if(W906_FormCloseSkipPassword==false)
  SavePassword();`, :535 same for ReadPassword; flag :2119 default false.
- Levelset save: wb_serve.cpp:5085 system.levels.put → WebLevelSet.cpp:466 W906_LevelSetPut → SecurityExitClick → FormClose inside
  `{ SkipPasswordGuard skipPw; … }` (WebLevelSet.cpp:623-630, struct :121-126) ⇒ **SavePassword / ReadPassword are skipped today**.
- WebLogin.cpp (1167 lines, CRLF): boot pwPath :80-106; WebLogin_UsesBook :158-161; WebLogin_Select (auth.select); WebLogin_BookLogin
  ~:386-515 (auth.login; binary branch :411-440 golden-faithful, fread into USER; `bookOverride` W906_PWBOOK_PATH forces text).
  WS security.passwd W906_SecurityPasswdOp :829 (dispatch wb_serve.cpp:5556): ops state / list / open / apply; GetMode :681-687
  (no file → select; NUL inside → book-binary; else text book); seams BookPath() :660-664 (W906_PWBOOK_PATH or pwPath),
  LoginDatPath() :665-669 (W906_LOGINDAT_PATH, SaveUser :709-719). select apply :941-978; text book :980-1166 (Q11 fix :1103-1108,
  :1149-1150); **book-binary open / apply refused ("binary-book") :923-939**; list via ReadBinaryNames :742-755.
- Page: web/page/Status.Security.html + web/page/ht9045_wire_statussecurity.js :411-590 (binary-book text :474, :553-556, :565).

## Proposed changes
Q9 (tag `//AI(W906-SEC-Q9)`, a recorded addition):
- New header-only `LoginDatBook.h` (like JamIniMerge.h; no CMake source change), on a raw 64004-byte image (offsets 4 / 30004 /
  60004): Read (exactly 64004 bytes, no create); FindUser (decoded ID UpperCase + Level); FirstFree (ID[0]==0 && PW[0]==0);
  SetField (EncodeStr; refuse >29 bytes or any byte <0x20 — keeps a NUL, avoids 0x01); Clear (zero 30+30, Level 0, as PW_Editor
  :186-189); WriteVerified (backup `<book>.bak_YYYYMMDD_HHMMSS`, no backup → no write; write "wb"; re-read + memcmp; bytes outside
  the touched slot equal the pre-image; decode-check the slot; success → delete backup (Q27=A policy — needs Steven's OK for a
  password file); failure → restore + keep backup). Never logs; errors never echo input. RecordCT untouched.
- WebLogin.cpp :923-939 (17 → 17 lines): the binary branch calls `W906_PwBinaryBook(op, root, book, iLevel, btnName, ok)`; the
  old guard JSON moves into it as the refusal branch.
- WebLogin.cpp EOF: `#include "LoginDatBook.h"`, `static_assert(sizeof(PASS_WORD)==64004)` + offsetof checks, and W906_PwBinaryBook:
  - writable only if (bUseLoginDatToSetLevel || W906_PWBOOK_PATH) && size==64004; else the old "binary-book" guard;
  - open → the dialog with the names at iLevel;
  - apply, golden text-book checks: empty user / old password → WAR1678; New: UpperCase duplicate → WAR1672, first free slot,
    Level=iLevel, full → guard "book-full" (new), ok → MES1673; Delete: needs confirmDelete else "cancelled" (no write), no match
    → WAR1677, ok → MES1674; Edit: empty new → WAR1678, user+level+old match else WAR1677 (no write), only PassWord[slot], ok → MES1675;
  - after a verified write with no override: memcpy(&USER, img, 64004) — **required**, or Q24's SavePassword writes the old USER
    back and silently undoes the Q9 edit.
- WebLogin.cpp :581-584, :600 comment text; optional :411 accept an override containing a NUL (so a probe can log into a %TEMP%
  binary book). JS :474, :553-556, :565 text.
Q24 (WebLevelSet.cpp — **St01's file, needs St01's OK**):
- :124 `explicit SkipPasswordGuard(bool skip) : old(W906_FormCloseSkipPassword) { if (skip) W906_FormCloseSkipPassword = true; }`
- :624 `SkipPasswordGuard skipPw(std::getenv("W906_LOGINDAT_PATH") || std::getenv("W906_PWBOOK_PATH"));`
  ⇒ production runs golden FormClose SavePassword + ReadPassword again; a probe with a seam still skips them (golden's literal path
  would otherwise overwrite the real login.dat).
- Comments :40, :42, :121, :620-622. Optional: ack reports `loginDat{memoryMatchedFileBefore, verified}`.
Owners: WebLogin.cpp (handed to St02), statussecurity.js (the login/password page, counts as handed over [inferred]),
WebLevelSet.cpp (St01, NOT handed over), cprod.cpp / common.cpp / cprod.h (Jimmy; no change), tests/CMakeLists.txt after :3416 (St02).

## ctest `Security_LoginDatBook` (tests/test_login_dat_book.cpp, links ht9045_core; %TEMP%\ht9045_q9_<tick> only)
1. encode vectors + round trip (printable, Big5) + the 0x01 non-round-trip;
2. a PW_Editor-style image (RecordCT 1000, slots 0..2, empty slot 5, garbage after a NUL);
3. New: first free slot, 64004 bytes, every byte outside the slot equal; a copy of the golden read loop finds user / level / pw;
4. Edit: only those 30 PW bytes change; wrong old / empty new → byte-identical file;
5. Delete: slot zeroed; no confirm → byte-identical;
6. refusals: 30-byte value, byte <0x20, 64003 / 64005 bytes, missing file;
7. verify / restore with a tampered buffer (restore byte-exact, backup kept);
8. no plaintext: redirect stdout / stderr, run all ops, grep that and every returned string for the test passwords → 0 hits.
Webprobe for St01: W906_PWBOOK_PATH → a %TEMP% binary book; open / apply New / Edit / Delete; grep responses and stdout; real
login.dat SHA256 unchanged, also after a levels.put.

## Risks / 上機要看
Risks: Q24 golden SavePassword writes USER and so reverts any PW_Editor edit made while the handler runs (golden); a book-mode machine
with no login.dat → select branch creates a plaintext-slot file, Q24 writes an all-zero file (golden-faithful); WAR1682 is modal;
Q9 New may fill slot 0/1 that golden password prompts compare as plaintext [inferred]; PW_Editor writing concurrently (no lock;
verify catches it); merge risk on WebLevelSet.cpp.
上機要看: (1) copy login.dat out, record SHA256 + size 64004, view in PW_Editor 905; (2) boot line `login: mode=book … pwPath=…login.dat`;
(3) New a test user → MES1673, `fc /b` vs the backup differs only at ID 4+30·s, PW 30004+30·s, Level 60004+4·s; (4) auth.login with
it; (5) PW_Editor shows it, save there, login still works; (6) Edit / wrong old (SHA unchanged) / Delete; (7) Q24: levelset save with
memory == file keeps SHA; PW_Editor edit while running then levelset save → the golden overwrite reported; (8) the test password
never in the console or D:\HT9045_Log; (9) BCB6 V899 / V912 log in with the V906-written file; (10) restore the original login.dat.
