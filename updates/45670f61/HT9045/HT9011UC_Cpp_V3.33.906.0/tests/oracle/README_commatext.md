# BCB6 oracle for TStringList CommaText / DelimitedText

AI(W906-COMMATEXT) 20261003 (INBOX 152). 中文摘要：這個資料夾的 `commatext_bcb6.cpp` 用 BCB6（bcc32 5.6.4 + 真的 VCL rtl.lib）
編譯執行，輸出釘成 `commatext_bcb6_expected.txt`；ctest `VclCommaTextBcb6` 拿同一張案例表跑 vclcompat，逐行比對。

## Files

| file | what |
|---|---|
| `commatext_cases.inc` | the case table + driver, C++98, pure ASCII. Compiled by BOTH bcc32 (oracle) and MinGW g++ (the ctest), so table, driver and output format cannot drift. |
| `commatext_bcb6.cpp` | the oracle's `main` (BCB6 only; not in CMake). |
| `commatext_bcb6_expected.txt` | the oracle's output, pinned. The ctest compares every non-`#` line. |
| `../test_vcl_commatext_bcb6.cpp` | ctest `VclCommaTextBcb6`: same table against `vclcompat::TStringList`. |

The exe, .obj and .tds are never committed; build them outside the tree.

## Rebuild the pinned file (needs BCB6 at `D:\ProgramFiles\Borland\CBuilder6`)

From a scratch folder that holds copies of `commatext_cases.inc` and `commatext_bcb6.cpp` (cmd.exe):

```bat
set B=D:\ProgramFiles\Borland\CBuilder6
set PATH=%B%\Bin;%PATH%
"%B%\Bin\bcc32.exe" -q -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tWC -tWM -I"%B%\Include";"%B%\Include\Vcl" -ocommatext_bcb6.obj commatext_bcb6.cpp
"%B%\Bin\ilink32.exe" -q -D"" -ap -Tpe -x -Gn -v -L"%B%\Lib";"%B%\Lib\Obj";"%B%\Lib\Release" c0x32.obj sysinit.obj commatext_bcb6.obj, commatext_bcb6.exe, , rtl.lib vcl.lib import32.lib cp32mt.lib, ,
commatext_bcb6.exe commatext_bcb6_expected.txt
commatext_bcb6.exe commatext_bcb6_expected.txt R01
commatext_bcb6.exe commatext_bcb6_expected.txt R02
```

(Compiler flags = the golden `HT9045.bpr` CFLAG1 with `-tWC` instead of `-tW`; linker = a VCL console app:
`c0x32.obj sysinit.obj` + `rtl.lib vcl.lib import32.lib cp32mt.lib`, `-ap`.)

The first run writes the table (S / D / V / G / B cases). `R01` and `R02` are each run in a process of their own and
appended: BCB6 raises `EAccessViolation` inside `SysUtils.AnsiExtractQuotedStr` for them (an unterminated quoted
field whose content is exactly two quote chars: `SetLength(Result, 0)`, then `Move` writes one byte through
`PChar('')`, which is System's read-only `@@zeroByte`, system.pas `_LStrToPChar`). Pinned on 20261003 on JIMMYCHIU-NB:
`__BORLANDC__=0x564, GetACP()=950, SysLocale.FarEast=0` (the header line). Three runs gave byte-identical files.

## What the port does not reproduce (and the ctest therefore checks against the port's own expectation)

`kPortOnly` in `../test_vcl_commatext_bcb6.cpp`:

* `S26 get`, `S29 get`: the unterminated quote that ends in a doubled pair makes the RTL write one byte over the
  item's NUL terminator; BCB6's later `GetDelimitedText` scans `PChar(S)` past `Length(S)` into that byte and whatever
  heap bytes follow -- undefined. The ITEM itself (`set` line) is compared and equal.
* `V01`: Win32 `CharNext` under ACP 950 hides a Big5 trail byte `0x7C` from the `'|'` delimiter. The port's strings
  are UTF-8 and it steps bytes (see `vclcompat/TStringList.cpp`, "CharNext").
* `V02`: `AnsiStrScan` follows `SysLocale.FarEast` (set only for a non-western THREAD locale), so this line moves with
  the oracle box's locale.
* `R01`, `R02`: the AV above; the port returns `''` for that item.
