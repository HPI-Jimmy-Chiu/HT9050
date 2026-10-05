# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。盤點 wb_serve 送出的每一個執行期 tag 現在有沒有接到畫面上，
# 寫入 screenshot_meta.js 的 TAG_WIRE_STATUS。
# 當日完整變更紀錄：<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""gen_tag_status.py -- 產生 TAG_WIRE_STATUS 並寫進 screenshot_meta.js。

AI(W906-FW-TAGSUB) 20260916。

回答一個問題：wb_serve 每 500ms 送過來的那些 tag，有幾個真的顯示在畫面上。

兩邊的資料都是量出來的，不是人工維護的清單：
  送出什麼  -> 連上執行中的 wb_serve，抓第一個 snapshot 訊框（權威來源是
               PublishHandlerTags 的實際輸出，不是 WebBridgeTags.h 的註解；
               那份註解 20260916 實測已經過期，寫 61 而實際是 117）
  接了什麼  -> 掃交付包 client/ 底下每個 ht9045_wire_*.js 的 tags: {} 區塊

分級：
  wired    已接到某頁的某個元素
  nohtml   沒接：現行 HTML 上沒有可驗證的目標元素
           （這是目前 113/117 的原因，不是漏掉）

⚠ 「有沒有值」與「有沒有接」是兩件事。
  live=false 表示這一刻伺服器對它送 null（來源還沒載入），與接線無關；
  wb_serve 是 handler 的替身，多數來源本來就不在它的行程裡。

用法: py -3 gen_tag_status.py [--port 8046]
"""
import base64
import glob
import io
import json
import os
import re
import socket
import struct
import sys
import time

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

HOST, PATH = '127.0.0.1', '/ht9045'
PORT = 8045
for _i, _a in enumerate(sys.argv):
    if _a == '--port' and _i + 1 < len(sys.argv):
        PORT = int(sys.argv[_i + 1])

# AI(W906-P5) 20260923: 使用者裁決「甲 —— web/page/ 是唯一權威」。
#   原本這裡指 D:\HT9045\client。那個目錄與 web/page/ 有 58 個同名 .js，
#   而接線實際部署在 web/page/ ⇒ 這支工具會對「已經接好的 tag」報「沒接」。
#   假答案的方向是「回報沒接」，最貴：它會讓人重做已經做完的事。
#   20260923 實測撞到：Phase 1 把 machine.state 接上 web/page/ 之後，
#   這支工具仍報 nohtml，因為它量的是另一份。
CLIENT = r'D:\HT9045\web\page'
METAS = [r'D:\HT9045\page\screenshot_meta.js',
         r'D:\HT9045\web\page\screenshot_meta.js']


def snapshot(observe_sec=4.0):
    """抓 snapshot，再把接下來數秒的 patch 套上去，回傳合併後的 tag 表。

    Steven 20260916：第一版只取第一個 snapshot 訊框，`live` 因此**低估**。
    那一幀是在連線當下發的，比伺服器 tick 迴圈的第一輪還早 —— 例如 clock.text
    在第一幀是 null，但 500ms 後就有值了（main.html 上看得到它在跳）。
    只看第一幀會把它記成「沒有值」，然後有人拿這個數字去判斷「producer 缺」。
    合併 patch 之後才是「這個伺服器實際送得出什麼」。

    ⚠ 仍然只反映**這一次執行**：pump.* 那組只有在 wb_publish --pump 之下才有值，
      用 wb_serve 觀察時它們是 null，那是模式差異不是缺陷。
    """
    s = socket.create_connection((HOST, PORT), timeout=10)
    key = base64.b64encode(os.urandom(16)).decode()
    s.sendall(('GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\n'
               'Connection: Upgrade\r\nSec-WebSocket-Key: %s\r\n'
               'Sec-WebSocket-Version: 13\r\n\r\n' % (PATH, HOST, PORT, key)).encode())
    buf = b''
    while b'\r\n\r\n' not in buf:
        buf += s.recv(4096)
    # 握手回應與第一個訊框常在同一個 TCP 區段，殘留位元組必須留著
    buf = buf.split(b'\r\n\r\n', 1)[1]
    tags, got = None, 0
    end = time.time() + 10 + observe_sec
    stop_after = None
    while time.time() < end:
        if len(buf) >= 2:
            b1 = buf[1] & 0x7F
            off, ln = 2, b1
            if b1 == 126 and len(buf) >= 4:
                ln = struct.unpack('>H', buf[2:4])[0]; off = 4
            elif b1 == 127 and len(buf) >= 10:
                ln = struct.unpack('>Q', buf[2:10])[0]; off = 10
            if len(buf) >= off + ln:
                m = json.loads(buf[off:off + ln].decode('utf-8', 'replace'))
                buf = buf[off + ln:]
                t = m.get('type')
                if t == 'snapshot':
                    tags = dict(m.get('data', {}))
                    stop_after = time.time() + observe_sec
                elif t == 'patch' and tags is not None:
                    # patch 的 null 代表「消失／現在不可知」，原樣套上去
                    tags.update(m.get('data', {}))
                    got += 1
                continue
        if stop_after is not None and time.time() > stop_after:
            break
        s.settimeout(1.0)
        try:
            c = s.recv(65536)
        except socket.timeout:
            continue
        if not c:
            break
        buf += c
    s.close()
    if tags is None:
        raise RuntimeError('沒收到 snapshot 訊框（wb_serve 有起來嗎？）')
    print('觀察 %.0fs：snapshot 1 幀 + patch %d 幀' % (observe_sec, got))
    return tags


def wired_map():
    """{tag: (page, elementId)}，從實體接線檔掃出來。"""
    out = {}
    for f in sorted(glob.glob(os.path.join(CLIENT, 'ht9045_wire_*.js'))):
        t = open(f, encoding='utf-8', errors='replace').read()
        pm = re.search(r"page:\s*'([^']+)'", t)
        page = pm.group(1) if pm else os.path.basename(f)
        m = re.search(r'\btags:\s*\{(.*?)\n  \}', t, re.S)
        if not m:
            continue
        for tag, eid in re.findall(r"'([a-zA-Z0-9_.]+)'\s*:\s*\[\s*'([A-Za-z0-9_]+)'", m.group(1)):
            out[tag] = (page, eid)
    return out


def main():
    data = snapshot()
    wired = wired_map()
    rows = []
    for tag in sorted(data.keys()):
        w = wired.get(tag)
        rows.append({
            'tag': tag,
            'status': 'wired' if w else 'nohtml',
            'page': w[0] if w else '',
            'element': w[1] if w else '',
            'live': data[tag] is not None,
        })

    # 接了但伺服器沒送的 tag：對照表打錯字就會長這樣，要看得見
    ghosts = sorted(set(wired) - set(data))
    for g in ghosts:
        rows.append({'tag': g, 'status': 'ghost', 'page': wired[g][0],
                     'element': wired[g][1], 'live': False})

    block = ('\n\n// AI(W906-FW-TAGSUB) 20260916: 執行期 tag 與畫面的接線狀態。\n'
             '// 由 scratchpad/gen_tag_status.py 量出來（tag 來自執行中 wb_serve 的\n'
             '// snapshot，接線來自 client/ht9045_wire_*.js 的 tags 區塊），不要手改。\n'
             'TAG_WIRE_STATUS = [\n'
             + '\n'.join(' ' + json.dumps(r, ensure_ascii=False) + ',' for r in rows).rstrip(',')
             + '\n];\n')
    for meta in METAS:
        if not os.path.isfile(meta):
            print('略過（不存在）：' + meta)
            continue
        src = open(meta, encoding='utf-8').read()
        src = re.sub(r'\n*// AI\(W906-FW-TAGSUB\).*?\nTAG_WIRE_STATUS = \[.*?\n\];\n', '\n',
                     src, flags=re.S)
        open(meta, 'w', encoding='utf-8', newline='').write(src.rstrip('\n') + block)
        print('已寫入 ' + meta)

    n = len(rows)
    nw = sum(1 for r in rows if r['status'] == 'wired')
    nl = sum(1 for r in rows if r['live'])
    print('')
    print('%-28s %-8s %-18s %s' % ('tag', 'status', 'page', 'element'))
    for r in rows:
        if r['status'] != 'nohtml':
            print('%-28s %-8s %-18s %s' % (r['tag'], r['status'], r['page'], r['element']))
    print('')
    print('共 %d 個 tag：已接 %d、無 HTML 目標 %d、對照表有但伺服器沒送 %d'
          % (n, nw, sum(1 for r in rows if r['status'] == 'nohtml'), len(ghosts)))
    print('其中這一刻有值（非 null）的有 %d 個 —— 與接線無關，是來源載入與否' % nl)
    if ghosts:
        print('⚠ ghost（對照表打錯字或伺服器改名）：' + ', '.join(ghosts))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
