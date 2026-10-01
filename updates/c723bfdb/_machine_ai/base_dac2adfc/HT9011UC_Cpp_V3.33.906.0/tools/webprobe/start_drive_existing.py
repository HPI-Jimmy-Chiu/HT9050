# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/start_drive_existing.py
#
#  AI(W906-START-WHERE-GDB) 20260922
#
#  驅動一個**已經在跑**的 wb_serve（例如跑在 gdb 底下的那一個），
#  送 control.acquire -> lot.start -> start.run，然後就結束。
#
#  為什麼要另外一支：`start_where_blocked_probe.py` 自己會 spawn wb_serve，
#  而在 gdb 底下時 wb_serve 是 gdb 起的，不是探針起的。
#
#  ⚠⚠ 會寫真實檔（lot.start -> config\config.ini 的 [Lot Info]）。
#     外面一定要包 realfile_guard。這支自己不做那件事 —— 守衛要在外面，
#     這樣忘了包就會被看見。
#
#  用法:  python tools/webprobe/start_drive_existing.py <port>
# =============================================================================
import json
import os
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402


def pump(sock, leftover, want_id, deadline):
    """等 want_id 的 ack；看到 query 就用 options[0] 回答，免得宿主永遠等。"""
    for op, payload in read_frames(sock, leftover, deadline):
        if op != 1:
            continue
        try:
            j = json.loads(payload.decode("utf-8", "replace"))
        except ValueError:
            continue
        t = j.get("type")
        if t == "modal":
            print("    <modal> %s" % (j.get("text") or j.get("message") or ""))
        elif t == "query":
            opts = j.get("options") or []
            print("    <query> code=%s options=%s" % (j.get("code"), opts))
            if opts:
                send_text(sock, json.dumps({"type": "cmd", "id": 9000,
                                            "cmd": "modal.answer",
                                            "tag": str(j.get("qid", "")),
                                            "value": opts[0]}))
        elif t == "ack" and j.get("id") == want_id:
            return j
    return None


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8084
    sock = None
    leftover = b""
    deadline = time.monotonic() + 60.0
    last = None
    while time.monotonic() < deadline:
        try:
            sock, leftover = ws_handshake("127.0.0.1", port, "/ht9045",
                                          time.monotonic() + 5.0)
            break
        except Exception as e:                            # noqa: BLE001
            last = e
            time.sleep(0.5)
    if sock is None:
        print("連不上 127.0.0.1:%d -- %s" % (port, last))
        return 3

    steps = [
        (1, {"cmd": "control.acquire"}),
        (2, {"cmd": "lot.start", "tag": "GDBPROBE", "value": "AIPROBE"}),
        (3, {"cmd": "start.run", "value": "gdb"}),
    ]
    for i, (cid, body) in enumerate(steps):
        msg = {"type": "cmd", "id": cid}
        msg.update(body)
        print("[%d] %s" % (cid, body["cmd"]))
        send_text(sock, json.dumps(msg))
        ack = pump(sock, leftover if i == 0 else b"", cid,
                   time.monotonic() + 60.0)
        print("    ack: %s" % json.dumps(ack, ensure_ascii=False)[:200])
    sock.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
