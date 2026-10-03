# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/s7_thermo_probe.py -- JSON 橋接層 S7（溫控 producer）的 e2e probe。
#
#  AI(W906-JSONBRIDGE-S7) 20260923: 新檔。
#
#  驗兩件事：
#    (1) GET /api/struct/temp.zone/schema 的形狀與內容
#    (2) tag 串流裡真的有 71x3 個 temp.zone.* 而且**每一個都是 null**
#
#  ⚠ (2) 的重點不是「有沒有 tag」，是「值是不是 null」。送 0 會是一個合法的
#    攝氏溫度，而三個資料源今天都不活（證據在 JsonBridge/StageThermo.h 檔頭）。
#    這支 probe 存在的主要理由就是釘住這一條 —— 將來有人「順手」把 null 換成
#    0 讓畫面好看，這裡會紅。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --seconds 25 --port 8192
#      python tools\webprobe\s7_thermo_probe.py --port 8192
#
#  Exit 0 = 每一項期望都達成。
# =============================================================================
import argparse
import json
import os
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, read_frames   # noqa: E402

FAILED = []
PASSED = [0]


def check(cond, what):
    if cond:
        PASSED[0] += 1
    else:
        FAILED.append(what)
        sys.stdout.write('FAIL  %s\n' % what)


def http_json(port, path):
    url = 'http://127.0.0.1:%d%s' % (port, path)
    with urllib.request.urlopen(url, timeout=5) as r:
        return json.loads(r.read().decode('utf-8'))


def probe_schema(port):
    sys.stdout.write('--- schema ---\n')
    d = http_json(port, '/api/struct/temp.zone/schema')

    check(d.get('binding') == 'temp.zone', 'binding == temp.zone')
    check(d.get('channels') == 71, 'channels == 71 (MachineType.h eTempControll)')
    check(len(d.get('items', [])) == 71, 'items 有 71 筆')
    check(d.get('fmt') == '%5.1f', "fmt == '%5.1f'（golden 的顯示格式）")

    # AI(W906-INST-LIVE) 20261003 (Ifor01)：inst 活了（Steven 1003 17:34「inst 先單獨上線」）
    #   ⇒ anyLive 是 True；哪一欄活要看 fields.<欄位>.live。下一個欄位變活時，這支 probe
    #   要跟著改，而不是把這條刪掉。
    check(d.get('anyLive') is True,
          'anyLive 是 true（inst 活了；pv／comm 還沒）')

    f = d.get('fields', {})
    staged = sorted(k for k, v in f.items() if v.get('staged'))
    notstaged = sorted(k for k, v in f.items() if not v.get('staged'))
    check(staged == ['comm', 'inst', 'pv'],
          'staged 欄位 == pv/comm/inst（有真實全域撐著的那三個）')
    # ⚠ overAny 是 SKILL.md §4.8（全文已於 20260923 拆到 references/api-shape.md） 列的第 7 個 tag，而且**不是 per-channel**
    #   （它是 71 個通道的彙整）。第一版 schema 完全沒提它 —— 既不 staged
    #   也沒宣告 not-staged。這個 schema 的賣點就是「每個欄位都誠實描述」，
    #   沉默缺席剛好是它最不該做的事。這條斷言就是為了讓它不能再消失。
    # AI(W906-INST-LIVE) 20261003 (Ifor01)：補上 TP-1b（1002，14863cdb）加的 color——那次只改了
    #   StageThermo.cpp 與 test_jsonbridge_s7，漏了這支 probe。
    check(notstaged == ['color', 'overAny', 'ready', 'state', 'sv'],
          'not-staged == color/overAny/ready/state/sv（ShowThermo 自 TP-1 起有在跑，'
          '但它的輸入 pv 還沒有來源）')

    # 每一個欄位都要講得出理由，而且理由要指到檔名行號 —— 沒有出處的
    # 「還沒好」等於沒有資訊，下一個人無法複驗。
    for k, v in f.items():
        check(bool(v.get('why')), 'fields.%s 有 why' % k)
        check(bool(v.get('from')), 'fields.%s 有 from' % k)
        want_live = (k == 'inst')                       # AI(W906-INST-LIVE) 20261003：只有 inst 活
        check(v.get('live') is want_live,
              'fields.%s.live 是 %s' % (k, 'true' if want_live else 'false'))
        why = v.get('why', '')
        check(('.cpp:' in why) or ('.h ' in why) or ('.cpp ' in why),
              'fields.%s 的 why 指到檔名（可複驗）' % k)

    idx = [i.get('idx') for i in d.get('items', [])]
    check(idx == list(range(71)),
          'idx 連續 0..70（列舉值由編譯器求，不是產生器寫死）')

    names = [i.get('name') for i in d.get('items', [])]
    check(names[0] == 'tcHotPlate1', '第一個通道是 tcHotPlate1')
    check(names[-1] == 'tcLBDown', '最後一個通道是 tcLBDown')
    check(len(set(names)) == 71, '71 個通道名互不重複')

    groups = set(i.get('group') for i in d.get('items', []))
    check('atc' in groups and 'hotplate' in groups and 'shuttle' in groups,
          'group 有 atc/hotplate/shuttle')
    check(sum(1 for i in d['items'] if i['group'] == 'atc') == 34,
          'ATC 區 34 個通道（tcAa1..tcBh2）')
    return d


def probe_tags(port, schema):
    sys.stdout.write('--- tag 串流 ---\n')
    deadline = time.time() + 20.0
    sock, leftover = ws_handshake('127.0.0.1', port, '/ht9045', deadline)
    frames = read_frames(sock, leftover, deadline)

    snap = None
    for _ in range(40):
        try:
            fr = next(frames)
        except StopIteration:
            break
        # ⚠ read_frames 產出 (opcode, payload-bytes) 的 tuple，不是字串。
        #   opcode 1 = text。
        if not (isinstance(fr, tuple) and len(fr) == 2):
            continue
        op, raw = fr
        if op != 1:
            continue
        try:
            m = json.loads(raw.decode('utf-8'))
        except Exception:
            continue
        # ⚠ 第一個 snapshot 帶全量 tag，鍵是 `data` 不是 `tags`
        #   （之後是 patch，只送變動）。
        if m.get('type') == 'snapshot' and isinstance(m.get('data'), dict):
            snap = m['data']
            break
    sock.close()

    check(snap is not None, '收到 snapshot')
    if snap is None:
        return

    tz = {k: v for k, v in snap.items() if k.startswith('temp.zone.')}
    check(len(tz) == 213, 'temp.zone.* 有 213 個 tag（71 通道 x 3 欄位），實得 %d'
          % len(tz))

    # ⚠⚠ 這是整支 probe 最重要的一條：pv／comm 每一個都是 null（送 0 會是合法溫度）。
    nonnull = {k: v for k, v in tz.items() if v is not None and not k.endswith('.inst')}
    check(not nonnull,
          'temp.zone.*.pv／comm **每一個都是 null**（送 0 會是合法溫度），'
          '非 null 的有 %d 個: %s'
          % (len(nonnull), sorted(nonnull)[:5]))
    # AI(W906-INST-LIVE) 20261003 (Ifor01)：inst 活了 —— 每一個都是 bool（不是 null、不是數字）。
    badinst = {k: v for k, v in tz.items() if k.endswith('.inst') and not isinstance(v, bool)}
    check(not badinst,
          'temp.zone.*.inst 每一個都是 bool（bUT150Install[]），不是的有 %d 個: %s'
          % (len(badinst), sorted(badinst)[:5]))

    for suffix in ('pv', 'comm', 'inst'):
        n = sum(1 for k in tz if k.endswith('.' + suffix))
        check(n == 71, 'temp.zone.*.%s 有 71 個（實得 %d）' % (suffix, n))

    for suffix in ('ready', 'state', 'sv', 'overAny', 'color'):
        n = sum(1 for k in tz if k.endswith('.' + suffix))
        check(n == 0,
              'temp.zone.*.%s **沒有**進串流（不是「值未知」，是「還沒有東西'
              '算得出來」，送 null 會說成前者）' % suffix)

    # schema 說有哪些通道，串流就要有哪些通道 —— 兩邊對不上表示有一邊過期。
    want = set('temp.zone.%s.pv' % i['name'] for i in schema['items'])
    got = set(k for k in tz if k.endswith('.pv'))
    check(want == got, 'schema 的通道名與串流的 tag 名完全一致')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8192)
    args = ap.parse_args()

    schema = probe_schema(args.port)
    probe_tags(args.port, schema)

    sys.stdout.write('\n%d checks, %d failed\n' % (PASSED[0] + len(FAILED),
                                                   len(FAILED)))
    if FAILED:
        for f in FAILED:
            sys.stdout.write('  FAIL %s\n' % f)
        return 1
    sys.stdout.write('S7 probe: ALL PASS\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
