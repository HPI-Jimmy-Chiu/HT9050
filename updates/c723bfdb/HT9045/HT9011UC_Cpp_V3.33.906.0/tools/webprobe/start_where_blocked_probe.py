# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/start_where_blocked_probe.py
#
#  AI(W906-START-WHERE) 20260922
#
#  問一個問題：**今天按下網頁的 START，它實際上停在哪一關？**
#
#  讀碼只能告訴你「有 71 個活的 return false」（見 tools/start_reject_map.py）。
#  這支去問那顆真的 exe。
#
#  ## 它怎麼知道停在哪
#
#  wb_serve 的 ForwardShowMyMessage（tools/wb_serve.cpp:149）把每一個
#  ShowMyMessage 轉成 `{"type":"modal"}` 訊框廣播出去。探針握著 WS 連線，
#  所以**收得到那句訊息文字** —— 而那句話就是「是哪一關擋的」。
#
#  ⚠⚠ 但 ShowErrorMessage 是另一回事：ForwardShowErrorMessage
#     （tools/wb_serve.cpp:168）會**永遠等** modal.answer，而且等的時候
#     連 tag 發布一起凍住（那是刻意忠於 golden 的 modal 語意）。
#     ⇒ 探針必須偵測 `{"type":"query"}` 並**立刻回答**，否則 wb_serve 卡死。
#       這裡一律回 kcode 的最低位元（golden 的第一顆鈕），並把事件記下來。
#
#  ## ⚠⚠ 會寫真實檔
#
#  lot.start -> ReadWriteLotInfo 會寫 D:\HT9045\config\config.ini 的 [Lot Info]；
#  StartFromWeb 內部還有別的寫入點。
#  ⇒ 跑之前一定要 `python tools/realfile_guard.py snap <tag>`，
#    跑完 `check`，沒問題 `drop`、有問題 `restore`。
#    **這支自己不做那件事** —— 守衛要在外面，忘了包就會被看見。
#
#  ## ⛔ 組態：預設只接受 SIMULATION 的 exe
#
#  出貨組態那顆會碰硬體。要用它必須明講 --allow-shipping，
#  而那應該只在**有人站在機台旁邊**的時候做。
#
#  用法:
#    python tools/webprobe/start_where_blocked_probe.py [exe] [--allow-shipping]
# =============================================================================
import json
import os
import re
import subprocess
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

PORT = 8083
LOT = "STARTPROBE"
OPR = "AIPROBE"


def config_of(exe):
    """那顆 exe 是哪個組態 —— 從它的 build dir 的 CMakeCache 問，不用猜。"""
    cache = os.path.join(os.path.dirname(exe), "CMakeCache.txt")
    try:
        with open(cache, "r", encoding="utf-8", errors="replace") as fh:
            for line in fh:
                if line.startswith("W906_NO_SOFT_SIMULTE:BOOL="):
                    return ("SHIPPING" if line.strip().endswith("ON")
                            else "SIMULATION")
    except OSError:
        pass
    return "UNKNOWN"


def pump(sock, leftover, want_id, deadline, events):
    """等 want_id 的 ack，沿途把 modal / query 記進 events。

    ⚠ 看到 query 就**立刻回答**，否則 wb_serve 會永遠等下去
      （tools/wb_serve.cpp:168 的無逾時是刻意的忠實翻譯，不是缺陷）。
    """
    for op, payload in read_frames(sock, leftover, deadline):
        if op != 1:
            continue
        try:
            j = json.loads(payload.decode("utf-8", "replace"))
        except ValueError:
            continue
        t = j.get("type")
        if t == "modal":
            events.append(("modal", j.get("text") or j.get("message") or
                           json.dumps(j, ensure_ascii=False)))
        elif t == "query":
            # ⚠ modal.answer 的 value 要的是**鈕的名字字串**，不是位元值
            #   （tools/wb_serve.cpp:186-208：wc.value.isString() 為 false 時
            #    ans 會是空的 -> k=0 -> "not an offered option" -> 迴圈繼續
            #    -> wb_serve 卡到 --seconds 逾時）。第一版就是這樣寫錯的。
            # ⇒ 直接用訊框給的 options[0]：那是 golden 顯示順序的第一顆鈕，
            #   不要自己從 kcode 推，推錯就等於沒答。
            code = j.get("code", "")
            opts = j.get("options") or []
            pick = opts[0] if opts else ""
            events.append(("query", "code=%s kcode=%s options=%s -> 自動回 %r"
                           % (code, j.get("kcode"), opts, pick)))
            if pick:
                send_text(sock, json.dumps({"type": "cmd", "id": 9000,
                                            "cmd": "modal.answer",
                                            "tag": str(j.get("qid", "")),
                                            "value": pick}))
            else:
                events.append(("query", "⚠ 這個 query 沒有任何 options ——"
                               " 答不掉，wb_serve 會等到 --seconds 逾時"))
        elif t == "ack" and j.get("id") == want_id:
            return j
    return None


def main():
    argv = [a for a in sys.argv[1:]]
    allow_shipping = "--allow-shipping" in argv
    argv = [a for a in argv if not a.startswith("--")]
    exe = argv[0] if argv else os.path.join(ROOT, "build", "wb_serve.exe")

    if not os.path.isfile(exe):
        print("找不到 %s" % exe)
        return 2
    cfg = config_of(exe)
    print("wb_serve : %s" % exe)
    print("   組態  : %s" % cfg)
    print("   mtime : %s" % time.strftime("%Y-%m-%d %H:%M:%S",
                                          time.localtime(os.path.getmtime(exe))))
    if cfg != "SIMULATION" and not allow_shipping:
        print("\n⛔ 拒絕執行：這不是 SIMULATION 組態，它會碰硬體。")
        print("   要用出貨組態必須明講 --allow-shipping，")
        print("   而那應該只在有人站在機台旁邊的時候做。")
        return 4

    logp = os.path.join(ROOT, "_startwhere_serve.log")
    log = open(logp, "wb")
    # --seconds 必須 > 下面所有步驟逾時的總和（15+60+30+90 = 195），
    # 否則 wb_serve 會在最後一步中途自己收攤，而那看起來會像「START 掛住了」。
    proc = subprocess.Popen([exe, "--allow-cmd", "--seconds", "300",
                             "--port", str(PORT)],
                            cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    sock = None
    try:
        # 不用固定 sleep：啟動要載機台組態，實測已超過 3 秒且會隨樹長大。
        leftover = b""
        deadline = time.monotonic() + 45.0
        lastErr = None
        while time.monotonic() < deadline:
            try:
                sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045",
                                              time.monotonic() + 5.0)
                break
            except Exception as e:                        # noqa: BLE001
                lastErr = e
                time.sleep(0.4)
        if sock is None:
            print("連不上 wb_serve：%s" % lastErr)
            return 3

        ev = []
        print("\n[1] control.takeover")
        send_text(sock, json.dumps({"type": "cmd", "id": 1,
                                    "cmd": "control.takeover"}))  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        ack = pump(sock, leftover, 1, time.monotonic() + 15.0, ev)
        leftover = b""
        print("    ack: %s" % json.dumps(ack, ensure_ascii=False)[:160])

        print("\n[2] ★ 先不按 Lot Start，直接 start.run —— 預期被 Q29 的互鎖擋下")
        ev_a = []
        send_text(sock, json.dumps({"type": "cmd", "id": 2, "cmd": "start.run",
                                    "value": "probe"}))
        ack_a = pump(sock, b"", 2, time.monotonic() + 60.0, ev_a)
        print("    ack: %s" % json.dumps(ack_a, ensure_ascii=False)[:220])
        for kind, text in ev_a:
            print("    <%s> %s" % (kind, text))

        print("\n[3] lot.start")
        ev_b = []
        send_text(sock, json.dumps({"type": "cmd", "id": 3, "cmd": "lot.start",
                                    "tag": LOT, "value": OPR}))
        ack_b = pump(sock, b"", 3, time.monotonic() + 30.0, ev_b)
        print("    ack: %s" % json.dumps(ack_b, ensure_ascii=False)[:220])
        for kind, text in ev_b:
            print("    <%s> %s" % (kind, text))

        print("\n[4] ★★ 再按一次 start.run —— 這一次走得多遠？")
        ev_c = []
        send_text(sock, json.dumps({"type": "cmd", "id": 4, "cmd": "start.run",
                                    "value": "probe"}))
        ack_c = pump(sock, b"", 4, time.monotonic() + 90.0, ev_c)
        print("    ack: %s" % json.dumps(ack_c, ensure_ascii=False)[:220])
        for kind, text in ev_c:
            print("    <%s> %s" % (kind, text))

        print("\n" + "=" * 74)
        print("結論（這是觀察，不是判決 —— 組態換了答案就會換）")
        print("=" * 74)
        print("  組態                       : %s" % cfg)
        for label, a, e in (("沒按 Lot Start", ack_a, ev_a),
                            ("按了 Lot Start", ack_c, ev_c)):
            acc = (a or {}).get("accepted")
            print("  %s -> accepted=%s softStart=%s systemStart=%s"
                  % (label, acc, (a or {}).get("softStart"),
                     (a or {}).get("systemStart")))
            for kind, text in e:
                print("      擋在: <%s> %s" % (kind, text))
        return 0
    finally:
        try:
            if sock:
                sock.close()
        except Exception:                                  # noqa: BLE001
            pass
        try:
            # 探針做完就不等它跑完 --seconds 300 —— 直接收掉。
            proc.terminate()
            proc.wait(timeout=20)
        except Exception:                                  # noqa: BLE001
            proc.kill()
        log.close()
        # wb_serve 自己 printf 的那兩行才是權威（ack 只是轉述）
        try:
            with open(logp, "r", encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    if re.search(r"start\.run:|query qid=", line):
                        print("  [serve] %s" % line.rstrip())
        except OSError:
            pass


if __name__ == "__main__":
    sys.exit(main())
