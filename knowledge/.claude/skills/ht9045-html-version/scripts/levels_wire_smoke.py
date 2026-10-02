# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。跑 web/tests/levels_wire_probe.html，驗證 sysLevels 的瀏覽器端寫入接合。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""levels_wire_smoke.py -- 把探針頁跑一次，把 <pre id="out"> 的內容印出來。

AI(W906-FW-LEVELSET) 20260916。

security_smoke.py 驗的是「真頁面的索引對位」；這一支驗的是「引擎送出去的東西」
（collectSysLevels 的鍵是 [NN] 嗎、只送改過的嗎、dryRun 契約走得到嗎）。
兩支合起來才覆蓋得到讀與寫。

伺服器要先跑著，而且要帶 --allow-cmd（dryRun 需要指令通道；
--allow-system-write 不需要，dryRun 本來就不寫）。
用法: py -3 levels_wire_smoke.py [--port 8045]
"""
import io
import os
import re
import subprocess
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
PORT = 8045
for _i, _a in enumerate(sys.argv):
    if _a == '--port' and _i + 1 < len(sys.argv):
        PORT = int(sys.argv[_i + 1])
URL = 'http://127.0.0.1:%d/tests/levels_wire_probe.html' % PORT
# 探針頁記了幾項就要收到幾項。改探針頁時這個數字要跟著改 —— 它是防「沒跑完
# 就被 dump」的唯一一道檢查。
EXPECT_CHECKS = 5


def main():
    cmd = [EDGE, '--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check',
           '--user-data-dir=' + os.path.join(os.environ.get('TEMP', '.'), 'wb_edge_probe'),
           '--virtual-time-budget=15000', '--dump-dom', URL]
    dom = subprocess.run(cmd, capture_output=True, timeout=120).stdout.decode('utf-8', 'replace')
    m = re.search(r'<pre id="out">(.*?)</pre>', dom, re.S)
    if not m:
        print('FAIL  探針頁沒有產出 <pre id="out">（DOM %d bytes）' % len(dom))
        return 1
    body = m.group(1).replace('&lt;', '<').replace('&gt;', '>').replace('&quot;', '"').replace('&amp;', '&')
    print(body.strip())
    if body.strip() == '(running)':
        print('\nFAIL  探針還停在 (running) —— 非同步沒跑完或 JS 早就拋了')
        return 1
    # Steven 20260916：一定要檢查「跑了幾項」，不能只數 FAIL。
    # 實測踩過：頁面第 5 項還在等 WebSocket ack 就被 dump 掉，那一項既不是 PASS
    # 也不是 FAIL，只是不存在 —— 光數 FAIL 行數會判成 ALL PASS。
    lines = [l for l in body.strip().splitlines() if l.strip()]
    n_fail = len([l for l in lines if l.startswith('FAIL')])
    n_pass = len([l for l in lines if l.startswith('PASS')])
    print('')
    if n_pass + n_fail != EXPECT_CHECKS:
        print('FAILED  只跑了 %d 項，期望 %d 項（有檢查沒跑完就被 dump 了）'
              % (n_pass + n_fail, EXPECT_CHECKS))
        return 1
    print('ALL PASS (%d 項)' % n_pass if n_fail == 0 else 'FAILED %d / %d' % (n_fail, EXPECT_CHECKS))
    return 1 if n_fail else 0


if __name__ == '__main__':
    sys.exit(main())
