"""AI(W906-CHGLOG) 20260927 -- independent check of common_ChangeLog.cpp: undo each marked substitution, then every golden line of the copied spans
must appear, in order, as a contiguous run in the output.  Reports unmarked differences (should be 0)."""
import re
import os
_HERE = os.path.dirname(os.path.abspath(__file__))
_TREE = os.path.normpath(os.path.join(_HERE, '..', '..')).replace(os.sep, '/') + '/'   # HT9011UC_Cpp_V3.33.906.0/
_GOLD = os.environ.get('W906_GOLDEN_ROOT', 'D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/').replace(os.sep, '/').rstrip('/') + '/'
G = _GOLD + 'common.cpp'
P = _TREE + 'common_ChangeLog.cpp'
gl = open(G, 'rb').read().decode('cp950').splitlines()
out = open(P, 'rb').read().decode('utf-8').split('\r\n')
MARK = re.compile(r'   //AI\(W906-CHGLOG\) 20260927: .*$')

def undo(s):
    if not MARK.search(s):
        return s, False
    s = MARK.sub('', s)
    s = re.sub(r'CL_(fCleaning|fContact|FTestIF)_(\w+)\((\w+)\)\.c_str\(\)', r'\1->\2->Items->Strings[\3]', s)
    s = s.replace(',Group.c_str() , StrChangeName.c_str())', ',Group , StrChangeName)')
    s = re.sub(r'AnsiString\(ConvertToMMType\((\w+)\)\)\.c_str\(\)', r'AnsiString(ConvertToMMType(\1))', s)
    s = s.replace('Str2.sprintf("%s==>%s", ret.c_str(), Value.c_str());', 'Str2.sprintf("%s==>%s", ret, Value);')
    s = s.replace('if(ret!=Str.ToDouble() && InitialOK==true)', 'if(ret!=Str && InitialOK==true)')
    s = re.sub(r'^(\s*)if\([^)]*\) (Name=.*)$', r'\1\2', s)          # index guards
    return s, True

u = []
marked = 0
for s in out:
    t, m = undo(s)
    marked += m
    u.append(t)
u[[i for i, s in enumerate(u) if s.startswith('AnsiString __fastcall TempChangeLog(')][0]] = \
    u[[i for i, s in enumerate(u) if s.startswith('AnsiString __fastcall TempChangeLog(')][0]].replace('   // golden common.cpp:1802-2037', '')

def contiguous(a, b):   # golden lines a..b (1-based) appear contiguously in u
    want = gl[a - 1:b]
    for i in range(len(u) - len(want) + 1):
        if u[i:i + len(want)] == want:
            return i + 1
    return None

spans = [('TempChangeLog', 1802, 2037), ('bool', 637, 671), ('int', 706, 855), ('ulong', 988, 1002), ('double', 891, 954), ('str-try', 1053, 1054), ('str', 1055, 1081)]
bad = 0
for name, a, b in spans:
    at = contiguous(a, b)
    print('%-14s golden :%d-%d (%d lines) -> %s' % (name, a, b, b - a + 1, ('output :%d' % at) if at else 'NOT FOUND'))
    bad += at is None
# the local declarations copied into each function (golden lines + '   // golden :N')
for n in (626, 628, 695, 697, 978, 879, 881, 1025, 1027, 1028, 1029):
    want = gl[n - 1] + '   // golden :%d' % n
    hit = sum(1 for s in out if s == want)
    print('decl :%d %s' % (n, 'ok' if hit == 1 else 'MISSING %d' % hit))
    bad += hit != 1
print('marked lines undone:', marked, ' spans/decls missing:', bad)
import sys
sys.exit(1 if bad else 0)
