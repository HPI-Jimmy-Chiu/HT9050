# -*- coding: utf-8 -*-
"""Export BCB6 Security LevelSet data for the HTML simulation (UTF-8 JSON)."""
import json
import os
import re
import struct
from datetime import datetime, timezone

BASE = r'D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2'
SYSTEM = r'D:\HT9045\system'
OUT = r'D:\HT9045\JSON'
LEVELSET_FILE = os.path.join(SYSTEM, 'levelset.dat')

def read_level_set():
    try:
        with open(LEVELSET_FILE, 'rb') as source:
            payload = source.read()
    except OSError:
        return [0] * 256, 'missing'
    if len(payload) != 1024:
        raise ValueError('levelset.dat must be 1024 bytes, got %d' % len(payload))
    return list(struct.unpack('<256i', payload)), 'loaded'

def cpp_visibility_rules(source):
    rules = {}
    for match in re.finditer(r'mySecurityPal\[\s*(\d+)\s*\]->SetVisible\((.*?)\);', source, re.S):
        expression = re.sub(r'//.*', '', match.group(2))
        rules[int(match.group(1))] = re.sub(r'\s+', ' ', expression).strip()
    return rules

def main():
    cpp_path = os.path.join(BASE, 'cSecurity.cpp')
    with open(cpp_path, 'r', encoding='cp950', errors='replace') as source:
        cpp = source.read()
    rows = re.findall(r'new TMySecurity\("([^"]+)"\s*,\s*[^,]+,\s*(sb\w+)\)', cpp)
    levels, level_state = read_level_set()
    rules = cpp_visibility_rules(cpp)
    items = [{
        'index': index, 'id': 'security-item-%d' % index, 'caption': caption,
        'container': container, 'accessLevel': levels[index], 'defaultVisible': True,
        'visibilityRule': rules.get(index, 'true'),
    } for index, (caption, container) in enumerate(rows)]
    document = {
        'schemaVersion': '1.0.0',
        'source': {'toolchain': 'BCB6', 'levelSetFile': LEVELSET_FILE, 'levelSetBytes': 1024,
                   'definition': 'LAST_LEVEL_SET.AccessLevel[256] (int32 little-endian)',
                   'securitySource': cpp_path, 'generatedAt': datetime.now(timezone.utc).isoformat()},
        'levels': {'fourLevel': ['Operator', 'Engineer', 'Supervisor', 'HonPrec'],
                   'fiveLevel': ['Open', 'Operator', 'Engineer', 'Supervisor', 'HonPrec']},
        'accessLevel': levels, 'items': items,
        'constraints': {'forcedOnLoad': [
            {'when': 'CUSTOMER_CODE == CC_KYEC_LEE', 'indices': [35, 114, 128], 'level': 2},
            {'when': 'CUSTOMER_CODE == CC_KYEC_LEE', 'indices': [104], 'level': 3},
            {'when': 'always', 'indices': [163], 'level': 3}],
            'onClose': ['AccessLevel[87] >= iDefSupervisorLevel', 'AccessLevel[129] <= AccessLevel[130]',
                        'AccessLevel[86] >= iDefEngineerLevel unless CUSTOMER_CODE == CC_SIGURD_PeiXing']},
        'runtime': {'visibilityFile': 'Security-visibility-runtime.json', 'updateFile': 'Security-access-update.json',
                    'state': 'requires-cpp-runtime',
                    'note': 'C++ publishes SecurityPalVisible() results and validates HTML requests.'},
        'summary': {'accessLevelEntries': len(levels), 'securityItems': len(items), 'levelSetState': level_state},
    }
    os.makedirs(OUT, exist_ok=True)
    target = os.path.join(OUT, 'Security-access.json')
    with open(target, 'w', encoding='utf-8', newline='\n') as output:
        json.dump(document, output, ensure_ascii=False, indent=2)
        output.write('\n')
    print('written:', target, 'items:', len(items), 'level entries:', len(levels))

if __name__ == '__main__':
    main()