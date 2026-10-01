# -*- coding: utf-8 -*-
# =============================================================================
#  P0-3 完成條件的端到端量測（AI(W906-HOME-W1) 20260919）
#
#  問的是三件事，全部用實際的 tag 值回答，不看 log 文字：
#    1. start.run 之後 `guard.allMotorHome` 有沒有變 true
#    2. `guard.systemStart` 有沒有變 true
#    3. `machine.state` 是什麼
#
#  ⚠ 會寫真實檔（lot.start -> config.ini 的 [Lot Info]）。
#    呼叫端負責 realfile_guard snap/restore。
#
#  重用 tools/webprobe/cmd_probe.py 的 WS 實作（README 的姿態：這個目錄裡
#  不要有第二個 frame parser）。
# =============================================================================
import json
import os
import subprocess
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, read_frames, wait_ack   # noqa: E402

EXE = os.path.join(ROOT, "build_lot1s", "wb_serve.exe")
PORT = 8073
WATCH = ["guard.allMotorHome", "guard.systemStart", "guard.softStop",
         "guard.contactMode", "machine.state",
         "pump.ticks", "pump.tickKind", "pump.mainProcCalls",
         "pump.task.inArm", "pump.task.load"]


def snap_watch(sock, leftover, deadline, seen):
    # read_frames 是 generator，會把 socket 緩衝吃在自己肚子裡 ——
    # 這是 cmd_probe / snap_dump 既有的用法，照著用，不另寫第二個 parser。
    for op, payload in read_frames(sock, leftover, deadline):
        if op != 1:
            continue
        try:
            j = json.loads(payload.decode("utf-8", "replace"))
        except ValueError:
            continue
        d = j.get("data")
        if isinstance(d, dict):
            for k in WATCH:
                if k in d:
                    seen[k] = d[k]
    return b""


def main():
    log = open(os.path.join(ROOT, "_p03_probe_serve.log"), "wb")
    proc = subprocess.Popen([EXE, "--allow-cmd", "--seconds", "120",
                             "--port", str(PORT)],
                            cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    try:
        time.sleep(3.0)
        deadline = time.monotonic() + 100.0
        sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045", deadline)
        seen = {}

        leftover = snap_watch(sock, leftover, time.monotonic() + 3.0, seen)
        print("== 開站時 ==")
        for k in WATCH:
            print("   %-22s %s" % (k, json.dumps(seen.get(k))))

        send_text(sock, json.dumps({"type": "cmd", "id": 1, "cmd": "control.takeover"}))  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        a = wait_ack(sock, leftover, 1, time.monotonic() + 10.0)
        print("acquire   : %s" % json.dumps(a))

        send_text(sock, json.dumps({"type": "cmd", "id": 2, "cmd": "lot.start",
                                    "tag": "1", "value": "1"}))
        a = wait_ack(sock, b"", 2, time.monotonic() + 15.0)
        print("lot.start : %s" % json.dumps(a))

        send_text(sock, json.dumps({"type": "cmd", "id": 3, "cmd": "start.run",
                                    "value": "p03-probe-1"}))
        a = wait_ack(sock, b"", 3, time.monotonic() + 20.0)
        print("start.run#1: %s" % json.dumps(a))

        print("== 等歸零（每 2 秒量一次，最多 40 秒）==")
        t0 = time.monotonic()
        leftover = b""
        while time.monotonic() - t0 < 40.0:
            leftover = snap_watch(sock, leftover, time.monotonic() + 2.0, seen)
            print("   t=%4.1fs  home=%-5s sysStart=%-5s ticks=%-6s kind=%-4s "
                  "mainProc=%-6s inArm=%s"
                  % (time.monotonic() - t0,
                     json.dumps(seen.get("guard.allMotorHome")),
                     json.dumps(seen.get("guard.systemStart")),
                     json.dumps(seen.get("pump.ticks")),
                     json.dumps(seen.get("pump.tickKind")),
                     json.dumps(seen.get("pump.mainProcCalls")),
                     json.dumps(seen.get("pump.task.inArm"))))
            if seen.get("guard.allMotorHome") is True:
                break

        send_text(sock, json.dumps({"type": "cmd", "id": 4, "cmd": "start.run",
                                    "value": "p03-probe-2"}))
        a = wait_ack(sock, b"", 4, time.monotonic() + 20.0)
        print("start.run#2: %s" % json.dumps(a))

        t0 = time.monotonic()
        leftover = b""
        while time.monotonic() - t0 < 12.0:
            leftover = snap_watch(sock, leftover, time.monotonic() + 2.0, seen)
        print("== 收尾 ==")
        for k in WATCH:
            print("   %-22s %s" % (k, json.dumps(seen.get(k))))
        sock.close()
    finally:
        try:
            proc.terminate()
        except Exception:
            pass
        time.sleep(1.0)
        log.close()
    print("\n== wb_serve stdout 尾段 ==")
    with open(os.path.join(ROOT, "_p03_probe_serve.log"), "rb") as f:
        tail = f.read().decode("utf-8", "replace").splitlines()[-40:]
    for ln in tail:
        print("   " + ln)


if __name__ == "__main__":
    main()
