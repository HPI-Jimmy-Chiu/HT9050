"""AI(W906-CHGLOG) 20260927: generate common_ChangeLog.{h,cpp} from golden common.cpp (cp950) + golden .dfm Items lists.
Every golden line is copied verbatim except the enumerated substitutions (each asserted by count).  Output CRLF, UTF-8.
Usage: python tools/chglog/gen_common_changelog.py [--write]   (dry run prints the counts; W906_GOLDEN_ROOT overrides the golden path)"""
import re, sys
BS = chr(92)
import os
_HERE = os.path.dirname(os.path.abspath(__file__))
_TREE = os.path.normpath(os.path.join(_HERE, '..', '..')).replace(os.sep, '/') + '/'   # HT9011UC_Cpp_V3.33.906.0/
_GOLD = os.environ.get('W906_GOLDEN_ROOT', 'D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/').replace(os.sep, '/').rstrip('/') + '/'
G = _GOLD
T = _TREE
gl = open(G + 'common.cpp', 'rb').read().decode('cp950').splitlines()
assert '\ufffd' not in ''.join(gl)

def L(n):            # golden line n (1-based)
    return gl[n - 1]

def find(pat, lo, hi):
    hits = [n for n in range(lo, hi + 1) if pat in L(n)]
    assert len(hits) == 1, (pat, lo, hi, hits)
    return hits[0]

def brace_block(start):
    """lines start..end where start holds an if(...) and the block closes at matching brace"""
    depth, seen, n = 0, False, start
    while True:
        s = L(n)
        code = s.split('//')[0]
        depth += code.count('{') - code.count('}')
        if '{' in code:
            seen = True
        if seen and depth == 0:
            return start, n
        n += 1

# ---- golden anchors (asserted) ----
assert L(1802).startswith('AnsiString __fastcall TempChangeLog(AnsiString Group, AnsiString Name)') and L(2037) == '}'
assert L(622).startswith('void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)')
assert L(691).startswith('void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)')
assert L(974).startswith('void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value)')
assert L(1021).startswith('void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)')

bool_s, bool_e = brace_block(find('if(ret!=bValue && InitialOK==true)', 622, 689))
int_s, int_e = brace_block(find('if(ret!=Value && InitialOK==true)', 691, 873))
ul_s, ul_e = brace_block(find('if(ret!=Value && InitialOK==true)', 974, 1019))
assert L(875).startswith('void __fastcall WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)')
dbl_s, dbl_e = brace_block(find('if(ret!=Str && InitialOK==true)', 875, 972))
str_try1 = find('bStrIsFloat1=TryStrToFloat(ret.c_str(), freg);', 1021, 1099)
str_s, str_e = brace_block(find('if(InitialOK==true)', 1021, 1099))
assert str_s == str_try1 + 2
decl = {  # golden local declarations each block needs (bHasChange becomes the caller's variable)
    'bool': [find('AnsiString StrChangeName="";', 622, 636), find('bool bStr2HasFind=false;', 622, 636)],
    'int': [find('AnsiString StrChangeName="";', 691, 705), find('bool bStr2HasFind=false;', 691, 705)],
    'ul': [find('AnsiString StrChangeName="";', 974, 987)],
    'dbl': [find('AnsiString StrChangeName="";', 875, 890), find('bool bStr2HasFind=false;', 875, 890)],
    'str': [find('AnsiString StrChangeName="";', 1021, 1037), find('bool bStrIsFloat1=false;', 1021, 1037),
            find('bool bStrIsFloat2=false;', 1021, 1037), find('double freg=0;', 1021, 1037)],
}

# ---- golden .dfm Items lists (static: golden code never edits these at runtime -- measured 0927) ----
def dfm_items(path, obj):
    d = open(G + path, 'rb').read().decode('cp950').splitlines()
    oi = [i for i, l in enumerate(d) if re.match(r'\s*object %s:' % obj, l)]
    assert len(oi) == 1, (path, obj)
    i = oi[0]
    ind = len(d[i]) - len(d[i].lstrip())
    for j in range(i + 1, len(d)):
        s = d[j]
        if re.match(r'\s*object ', s) or (s.strip() == 'end' and len(s) - len(s.lstrip()) == ind):
            raise AssertionError('no Items in ' + obj)
        if 'Items.Strings = (' in s:
            items, k = [], j + 1
            while True:
                v = d[k].strip()
                last = v.endswith(')')
                v = v[:-1] if last else v
                assert v.startswith("'") and v.endswith("'") and "''" not in v, v
                items.append(v[1:-1])
                if last:
                    break
                k += 1
            return i + 1, j + 2, k + 1, items
    raise AssertionError(obj)

TABLES = [  # (form, widget, dfm path)
    ('fCleaning', 'rgCleanKitType', 'AutoClean/uCleaning.dfm'),
    ('fCleaning', 'rgAutoCleanSelectArm', 'AutoClean/uCleaning.dfm'),
    ('fCleaning', 'ContactMode', 'AutoClean/uCleaning.dfm'),
    ('fContact', 'coD41', 'cContact.dfm'),
    ('FTestIF', 'rgInterfaceType', 'cTesterIF.dfm'),
    ('FTestIF', 'rgBaudRate', 'cTesterIF.dfm'),
    ('FTestIF', 'rgBitLength', 'cTesterIF.dfm'),
    ('FTestIF', 'rgParity', 'cTesterIF.dfm'),
    ('FTestIF', 'rgStopBit', 'cTesterIF.dfm'),
]
tab_code = []
for form, w, path in TABLES:
    oline, a, b, items = dfm_items(path, w)
    # coD41 exists twice in golden (cConfiguration.dfm too); fContact-> is cContact.dfm, asserted by path
    lits = ', '.join('"%s"' % x for x in items)
    tab_code.append('static AnsiString CL_%s_%s(int i)   // golden %s:%d-%d (object %s at :%d)' % (form, w, path.split('/')[-1], a, b, w, oline))
    tab_code.append('{')
    tab_code.append('    static const char* const a[] = {%s};' % lits)
    tab_code.append('    return CL_Pick(a, %d, i);' % len(items))
    tab_code.append('}')
    tab_code.append('')
gp_line, gp_a, gp_b, gp_items = dfm_items('cTesterIF.dfm', 'cbGPIBType')
tif = open(G + 'cTesterIF.cpp', 'rb').read().decode('cp950').splitlines()
assert tif[45].strip().startswith('if(CUSTOMER_CODE==CC_ASE_KaohSiung)') and tif[47].strip() == 'cbGPIBType->Clear();' and tif[57].strip() == 'cbGPIBType->Refresh();'
ase = []
for n in range(49, 58):
    m = re.match(r'\s*cbGPIBType->Items->Add\("([^"]*)"\);', tif[n - 1])
    assert m, tif[n - 1]
    ase.append(m.group(1))
tab_code += [
    'static AnsiString CL_FTestIF_cbGPIBType(int i)   // golden TFTestIF ctor cTesterIF.cpp:46-59 (ASE Kaohsiung rebuilds the list) else cTesterIF.dfm:%d-%d' % (gp_a, gp_b),
    '{',
    '    if(CUSTOMER_CODE==CC_ASE_KaohSiung)',
    '    {',
    '        static const char* const a[] = {%s};' % ', '.join('"%s"' % x for x in ase),
    '        return CL_Pick(a, %d, i);' % len(ase),
    '    }',
    '    static const char* const d[] = {%s};' % ', '.join('"%s"' % x for x in gp_items),
    '    return CL_Pick(d, %d, i);' % len(gp_items),
    '}',
    '',
]

SUBS_ITEMS = re.compile(r'(fCleaning|fContact|FTestIF)->(\w+)->Items->Strings\[(\w+)\]')
MARK_CSTR = '   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)'
MARK_ITEM = '   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget\'s Items; here CL_<form>_<widget>() (file banner (b))'

def port_line(s, kind):
    orig = s
    n_items = len(SUBS_ITEMS.findall(s))
    s = SUBS_ITEMS.sub(lambda m: 'CL_%s_%s(%s).c_str()' % (m.group(1), m.group(2), m.group(3)), s)
    s = s.replace(',Group , StrChangeName)', ',Group.c_str() , StrChangeName.c_str())')
    s = re.sub(r'AnsiString\(ConvertToMMType\((\w+)\)\)', r'AnsiString(ConvertToMMType(\1)).c_str()', s)
    if kind == 'dbl' and s.strip().startswith('if(ret!=Str && InitialOK==true)'):
        s = s.replace('if(ret!=Str && InitialOK==true)', 'if(ret!=Str.ToDouble() && InitialOK==true)', 1)
        return s + '   //AI(W906-CHGLOG) 20260927: bcc32 5.6.4 compiles ret!=Str as Variant(ret)!=Variant(Str), a NUMERIC compare (NB2 R87, measured: 1.5 vs "1.5000" equal); vclcompat would compare AnsiString(ret)="1.5" with "1.5000" as strings and log every double write'
    if kind == 'str':
        s = s.replace('Str2.sprintf("%s==>%s", ret, Value);', 'Str2.sprintf("%s==>%s", ret.c_str(), Value.c_str());')
    if s != orig:
        s = s + (MARK_ITEM if n_items else MARK_CSTR)
    return s

def emit_block(lines_range, kind):
    out, changed = [], 0
    for n in lines_range:
        s = port_line(L(n), kind)
        changed += (s != L(n))
        out.append(s)
    return out, changed

def fn(kind, sig, golden_fn_span, blk_span, extra_first=None):
    body, ch = emit_block(range(blk_span[0], blk_span[1] + 1), kind)
    decls = [L(n) + '   // golden :%d' % n for n in decl[kind]]
    head = ['// golden WriteIniData %s (common.cpp:%d-%d): the change-log block :%d-%d, verbatim; locals from %s.' % (
        kind, golden_fn_span[0], golden_fn_span[1], blk_span[0], blk_span[1], '/'.join(':%d' % n for n in decl[kind])),
        sig, '{'] + decls
    if extra_first:
        head += extra_first
    return head + body + ['}', ''], ch

out_fns, changes = [], {}
f, changes['bool'] = fn('bool', 'void W906_ChangeLog_Bool(AnsiString FileName, AnsiString Group, AnsiString Name, bool ret, bool bValue, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)',
                        (622, 689), (bool_s, bool_e))
out_fns += f
f, changes['int'] = fn('int', 'void W906_ChangeLog_Int(AnsiString FileName, AnsiString Group, AnsiString Name, int ret, int Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)',
                       (691, 873), (int_s, int_e))
out_fns += f
f, changes['ul'] = fn('ul', 'void W906_ChangeLog_ULong(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long ret, unsigned long Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)',
                      (974, 1019), (ul_s, ul_e))
out_fns += f
f, changes['dbl'] = fn('dbl', 'void W906_ChangeLog_Double(AnsiString FileName, AnsiString Group, AnsiString Name, double ret, double Value, AnsiString Str, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)',
                       (875, 972), (dbl_s, dbl_e))
out_fns += f
str_pre = [L(str_try1), L(str_try1 + 1)]
f, changes['str'] = fn('str', 'void W906_ChangeLog_Str(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString ret, AnsiString Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)',
                       (1021, 1099), (str_s, str_e), extra_first=['    // golden :%d-%d' % (str_try1, str_try1 + 1)] + str_pre)
out_fns += f

# ---- TempChangeLog with index guards ----
GUARDS = [
    ('Name=asTempCtrl[iSiteAdd];', 'iSiteAdd>=0 && iSiteAdd<tcTotalCount', 1),
    ('Name=asTempCtrl[iSiteAdd-1];', 'iSiteAdd-1>=0 && iSiteAdd-1<tcTotalCount', 1),
    ('Name="OffSet_"+asATCTempName[iSiteAdd];', 'iSiteAdd>=0 && iSiteAdd<32', 1),
    ('Name=asIniDelayName[iSiteAdd];', 'iSiteAdd>=0 && iSiteAdd<10', 2),
    ('Name=asAutoCleanSpeed[iSiteAdd];', 'iSiteAdd>=0 && iSiteAdd<4', 1),
]
tcl, gcount = [], {g[0]: 0 for g in GUARDS}
for n in range(1802, 2038):
    s = L(n)
    for stmt, cond, _ in GUARDS:
        if s.strip() == stmt or s.strip().startswith(stmt + ' '):
            ind = s[:len(s) - len(s.lstrip())]
            s = ind + 'if(' + cond + ') ' + s.lstrip() + '   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked'
            gcount[stmt] += 1
    tcl.append(s)
for stmt, cond, k in GUARDS:
    assert gcount[stmt] == k, (stmt, gcount[stmt])
tcl[0] = tcl[0] + '   // golden common.cpp:1802-2037'

BANNER = r'''// ===========================================================================
//  common_ChangeLog.cpp -- AI(W906-CHGLOG) 20260927
//
//  The CHANGE-LOG half of golden's WriteIniData overloads (common.cpp:622-1099)
//  and TempChangeLog (common.cpp:1802-2037), GENERATED from golden (cp950) by
//  tools/chglog/gen_common_changelog.py: every golden line is verbatim except the
//  substitutions marked on their lines.
//
//  WHY A SEPARATE FILE IN ht9045_sm: common.cpp lives in ht9045_core, which
//  links only vclcompat + ht9045_public.  The change-log block needs
//  InitialOK / asTempCtrl / TestIF_File (ht9045_globals), RecordChangeLogProcess
//  (ht9045_db) and fContactForm (ht9045_forms).  The 20260721 attempt to
//  un-gate TempChangeLog inside common.cpp broke the link of test_common /
//  test_ini_helpers (they link ht9045_core alone).  So common.cpp's five
//  WriteIniData bodies call a function POINTER (common.h EOF) at golden's exact
//  spot -- after the old value is read, before the write -- and this file,
//  in the top library, supplies the bodies.  wb_serve installs them at boot
//  (tools/wb_serve.cpp:3858, W906_InstallChangeLogHooks()); tests that want
//  them call the functions directly.  Nothing else changes: a program that
//  does not install the hooks behaves exactly as before this change.
//
//  DEVIATIONS (each also marked on its line):
//   (a) varargs: golden passes AnsiString straight to sprintf (BCB6's AnsiString
//       is one pointer); here .c_str().
//   (b) Item captions: golden reads fCleaning->X / fContact->X / FTestIF->X
//       ->Items->Strings[i].  Those facades do not hold the lists (fCleaning.h
//       has no widgets; FTestIF's list members are empty or gated), so:
//         - lists golden never edits at runtime come from golden's .dfm, verbatim
//           (measured 0927: golden edits Items only in cContact.cpp:205-234
//           (cbContactMode) and cTesterIF.cpp:48-89 (cbGPIBType, cbDIOType));
//         - cbContactMode: the real facade fContactForm (forms/fContact.h, its
//           Init() runs golden :205-235 at wb_serve boot);
//         - cbGPIBType: golden's ctor choice (ASE Kaohsiung list, else .dfm);
//         - cbDIOType: golden InitcbDIOType's listing of DIOCFGPath *.ini, taken
//           when the log line is written instead of when the form was built.
//       An index outside the list gives "".  NB2 R87 measured BCB6 (VCL source
//       + a probe): a TComboBox (cbContactMode, coD41, cbGPIBType, cbDIOType,
//       ContactMode) also gives "" -- same; a TRadioGroup (the other 7) raises
//       EStringListError before the write, so that key is NOT saved.  Whether
//       to reproduce that is NIGHT_REPORT decision 25 (NB2 R87-RANGE); today "".
//   (c) TempChangeLog: golden indexes asTempCtrl / its three local tables with
//       no range check (undefined behaviour on a malformed key); here an
//       out-of-range index leaves Name unchanged.
//   (d) WriteIniData(double), golden common.cpp:891 `ret!=Str` (ret double,
//       Str the "%0.4f" AnsiString): bcc32 5.6.4 builds Variant(ret) !=
//       Variant(Str), a NUMERIC compare (NB2 R87: probe + -S assembly; 106
//       customer double records, 0 with equal sides).  vclcompat would take
//       AnsiString(ret) ("1.5") != "1.5000" as a string compare and log every
//       double write, so the port writes ret!=Str.ToDouble().
//   NOT HERE:
//   - FormHS->RecordChangeLogByLot (the CosFunction.bUseChangeLogByLot tail):
//     customer option, still gated in common.cpp.
//   - Where the record lands: RecordChangeLogProcess -> MyDBIProcess("ChangeLog")
//     reaches the SECSGEM/uHGemEquipment.cpp:3480 stand-in until cMyDB P4
//     (St02 0927 03:06): the old==>new text is counted, not yet written.
// ==========================================================================='''

HDR = '''// AI(W906-CHGLOG) 20260927: see common_ChangeLog.cpp's banner.
#ifndef COMMON_CHANGELOG_H
#define COMMON_CHANGELOG_H

#include "vclcompat/vcl_compat.h"

AnsiString __fastcall TempChangeLog(AnsiString Group, AnsiString Name);         // golden common.h:263 (common.cpp:1802-2037)

// golden WriteIniData change-log blocks; common.cpp calls them through the W906_ChangeLogHook_* pointers (common.h EOF)
void W906_ChangeLog_Bool(AnsiString FileName, AnsiString Group, AnsiString Name, bool ret, bool bValue, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_Int(AnsiString FileName, AnsiString Group, AnsiString Name, int ret, int Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_ULong(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long ret, unsigned long Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);
void W906_ChangeLog_Double(AnsiString FileName, AnsiString Group, AnsiString Name, double ret, double Value, AnsiString Str, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);  // Str = golden's "%0.4f" of Value
void W906_ChangeLog_Str(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString ret, AnsiString Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange);

// Points the five common.h hooks at the functions above (wb_serve boot, tools/wb_serve.cpp:3858).
void W906_InstallChangeLogHooks();

#endif // COMMON_CHANGELOG_H'''

PRE = '''#include "vclcompat/vcl_compat.h"
#include "common.h"             // DIOCFGPath; the W906_ChangeLogHook_* pointers
#include "common_ChangeLog.h"
#include "cmydef.h"             // InitialOK, asTempCtrl[tcTotalCount], CUSTOMER_CODE
#include "cprod.h"              // TestIF_File
#include "cpublic.h"            // ConvertToMMType
#include "cMyDB.h"              // RecordChangeLogProcess
#include "MachineType.h"        // _8Site2X4, CC_ASE_KaohSiung
#include "forms/fContact.h"     // fContactForm (the TfContact facade; `fContact` itself is TfContactShim)

#include <windows.h>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
//  (b) Item captions
// ---------------------------------------------------------------------------
static AnsiString CL_Pick(const char* const* a, int n, int i)
{
    return (i >= 0 && i < n) ? AnsiString(a[i]) : AnsiString("");
}
'''

DIO = r'''static AnsiString CL_FTestIF_cbDIOType(int i)   // golden TFTestIF::InitcbDIOType cTesterIF.cpp:69-106 (its list, built on demand)
{
    AnsiString szFileName, out = "";
    int n = 0;
    WIN32_FIND_DATAA filedata;
    HANDLE filehandle = FindFirstFileA((DIOCFGPath + "*.ini").c_str(), &filedata);
    if(filehandle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                strcmp(filedata.cFileName, ".")==0 ||
                strcmp(filedata.cFileName, "..")==0)
                continue;

            if(ExtractFileExt(filedata.cFileName).LowerCase()==".ini")
            {
                szFileName=ChangeFileExt(ExtractFileName(filedata.cFileName), "");
                if(n==i)
                    out=szFileName;
                n++;
            }
        } while(FindNextFileA(filehandle, &filedata));
        FindClose(filehandle);
    }
    // golden's else-branch (MessageBox "DIO data has been lossed" + Terminate) belongs to building the form, not to a log line
    return out;
}

static AnsiString CL_fContact_cbContactMode(int i)   // golden cContact.cpp:205-235 via the facade that runs it (TfContact::Init)
{
    if(fContactForm==NULL || i<0 || i>=fContactForm->cbContactMode->Items->Count)
        return "";
    return fContactForm->cbContactMode->Items->Strings[i];
}
'''

INSTALL = '''// ---------------------------------------------------------------------------
void W906_InstallChangeLogHooks()
{
    W906_ChangeLogHook_Bool  = W906_ChangeLog_Bool;
    W906_ChangeLogHook_Int   = W906_ChangeLog_Int;
    W906_ChangeLogHook_ULong = W906_ChangeLog_ULong;
    W906_ChangeLogHook_Double = W906_ChangeLog_Double;
    W906_ChangeLogHook_Str   = W906_ChangeLog_Str;
}'''

body = [BANNER, PRE] + tab_code + [DIO, '// ---------------------------------------------------------------------------',
        '//  TempChangeLog -- golden common.cpp:1802-2037 (Ifor 20190930)',
        '// ---------------------------------------------------------------------------'] + tcl + [
        '', '// ---------------------------------------------------------------------------',
        '//  The five WriteIniData change-log blocks', '// ---------------------------------------------------------------------------'] + out_fns + [INSTALL]
cpp = '\n'.join(body) + '\n'
hdr = HDR + '\n'
for x in (cpp, hdr):
    assert '\ufffd' not in x and not re.search('[\x00-\x08\x0b\x0c\x0e-\x1f\x7f]', x)
print('blocks: bool %d-%d int %d-%d ulong %d-%d dbl %d-%d str %d-%d(+%d)' % (bool_s, bool_e, int_s, int_e, ul_s, ul_e, dbl_s, dbl_e, str_s, str_e, str_try1))
print('changed lines:', changes, 'guards:', gcount)
print('cpp lines', cpp.count('\n'))
if '--write' in sys.argv:
    open(T + 'common_ChangeLog.cpp', 'wb').write(cpp.replace('\n', '\r\n').encode('utf-8'))
    open(T + 'common_ChangeLog.h', 'wb').write(hdr.replace('\n', '\r\n').encode('utf-8'))
    print('written')
