# =============================================================================
#  tools/webprobe/q3_dio_owner_probe.py -- Q3＝A（RULINGS_20260926 S125）：DIO 設定檔第二個寫入口的防呆。
#
#  AI(W906-FRW-Q3) 20260927（Steven 團隊 St01）：new file. Drives a running wb_serve over the real WebSocket and
#  checks that tools/wb_serve.cpp CRouteOwner（→ CRouteOwnerDio，檔尾）now recognises the DYNAMIC DIO ini names
#  that the C route FileRW/TTLCfg.cpp owns（golden TfDIOFrom::GetDIOFileName DIOInterFaceCFG.cpp:47-70）:
#
#    0. GET /api/system/  -> the "dio" entry (typeName / profiles / saveInSetup / path)
#    1. control.acquire
#    2. system.file.put tag=dio                         -> 409 owned by C route ... FileRW/TTLCfg.cpp
#    3. system.file.put tag=dio (dryRun)                -> NOT 409（預演照常允許；之後因空 payload 被拒）
#    4. system.file.put tag=<非 C 路的 ini>             -> NOT 409（其他檔不受影響）
#    5. system.file.put tag=teach                       -> 409 ... FileRW/Teach.cpp（固定檔名表 kOwned 沒變）
#    6. recipe.doc.put tag=<配方裡的 <DIO>.ini 副本>     -> 409 ... FileRW/TTLCfg.cpp（配方裡沒有副本時 SKIP）
#
#  Writes NOTHING: every put carries an EMPTY payload {"sections":{}}. If the gate were missing, wb_serve
#  answers "no sections/<key>/raw fields in the payload" before touching any file (wb_serve.cpp system.file.put /
#  recipe.doc.put), so a failing run cannot modify a DIO / system / recipe file either.
#  Steps 2-5 need wb_serve started with --allow-system-write (otherwise they print SKIP).
#  Exit 0 = every expectation met.
# =============================================================================
import argparse, json, os, sys, urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from w4_motor_probe import Conn, check, FAILS   # noqa: E402

EMPTY = json.dumps({'sections': {}})
EMPTY_DRY = json.dumps({'sections': {}, 'dryRun': True})
# SysFileTable 裡、不在 CRouteOwner kOwned 表、也不是 DIO 的 ini（依序挑第一個 available 的）
OTHER_INI = ['errNote', 'description', 'machineLife', 'arms', 'secsGem', 'padInterface', 'eventLogLevel',
             'motorTest', 'colorSensor']


def get_json(host, port, path):
    return json.loads(urllib.request.urlopen('http://%s:%d%s' % (host, port, path), timeout=5).read().decode('utf-8', 'replace'))


def err_of(r):
    return str(r.get('error', ''))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--host', default='127.0.0.1')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--path', default='/ht9045')
    ap.add_argument('--seconds', type=float, default=40.0)
    a = ap.parse_args()

    print('[0] GET /api/system/ -> dio entry')
    idx = get_json(a.host, a.port, '/api/system/')
    files = idx.get('files') or []
    dio = [f for f in files if f.get('name') == 'dio']
    check(bool(dio), 'system index lists "dio"')
    dio = dio[0] if dio else {}
    profiles = [p.lower() for p in (dio.get('profiles') or [])]
    print('  dio: typeName=%r saveInSetup=%r source=%r path=%r profiles=%d' % (
        dio.get('typeName'), dio.get('saveInSetup'), dio.get('source'), dio.get('path'), len(profiles)))
    other = [f['name'] for n in OTHER_INI for f in files if f.get('name') == n and f.get('kind') == 'ini' and f.get('available')]

    c = Conn(a.host, a.port, a.path, a.seconds)
    print('[1] control.acquire')
    r = c.cmd('control.acquire')
    check(r.get('ok') is True, 'control.acquire', json.dumps(r, ensure_ascii=False)[:300])

    print('[2] system.file.put tag=dio (B route) -> 409 owned by C route (TTLCfg)')
    r = c.cmd('system.file.put', 'dio', EMPTY)
    if 'allow-system-write' in err_of(r):
        print('  SKIP [2]-[5]: wb_serve was not started with --allow-system-write (%s)' % err_of(r)[:120])
    elif not dio.get('path'):
        print('  SKIP [2]: Tester.Data [DIO] TypeName is empty -- no DIO file resolved (%s)' % dio.get('note', ''))
    else:
        check(r.get('ok') is False and '409 owned by C route' in err_of(r) and 'TTLCfg' in err_of(r),
              'system.file.put dio -> 409 owned by C route (FileRW/TTLCfg.cpp)', json.dumps(r, ensure_ascii=False)[:400])

        print('[3] system.file.put tag=dio dryRun -> not 409 (dry run passes the owner gate)')
        r = c.cmd('system.file.put', 'dio', EMPTY_DRY)
        check('409' not in err_of(r), 'dryRun is not refused by the owner gate', json.dumps(r, ensure_ascii=False)[:400])

        print('[4] system.file.put tag=<non-C-route ini> -> not 409 (other files unaffected)')
        if other:
            r = c.cmd('system.file.put', other[0], EMPTY)
            check('409' not in err_of(r) and 'no sections' in err_of(r),
                  'system.file.put %s -> reaches the payload check (not owned)' % other[0], json.dumps(r, ensure_ascii=False)[:400])
        else:
            print('  SKIP: none of %s is available on this machine' % OTHER_INI)

        print('[5] system.file.put tag=teach -> fixed-name table unchanged (FileRW/Teach.cpp)')
        r = c.cmd('system.file.put', 'teach', EMPTY)
        check(r.get('ok') is False and '409 owned by C route' in err_of(r) and 'Teach.cpp' in err_of(r),
              'system.file.put teach -> 409 FileRW/Teach.cpp', json.dumps(r, ensure_ascii=False)[:400])

    print('[6] recipe.doc.put tag=<recipe copy of a DIO profile> -> 409 (TTLCfg)')
    try:
        docs = get_json(a.host, a.port, '/api/recipe/').get('documents') or []
    except Exception as e:  # noqa: BLE001
        docs = []
        print('  GET /api/recipe/ failed: %r' % e)
    copies = [d for d in docs if d.lower() in profiles]
    if not copies:
        print('  SKIP: the active recipe has no <DIO>.ini copy (documents=%d, profiles=%d)' % (len(docs), len(profiles)))
    else:
        r = c.cmd('recipe.doc.put', copies[0], EMPTY)
        check(r.get('ok') is False and '409 owned by C route' in err_of(r) and 'TTLCfg' in err_of(r),
              'recipe.doc.put %s -> 409 owned by C route (FileRW/TTLCfg.cpp)' % copies[0], json.dumps(r, ensure_ascii=False)[:400])

    print('\n%d failed' % len(FAILS))
    return 1 if FAILS else 0


if __name__ == '__main__':
    sys.exit(main())
