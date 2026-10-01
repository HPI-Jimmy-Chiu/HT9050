# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/q27_lotstart_probe.py -- Q27 的端到端探針
#
#  AI(W906-Q27) 20260921
#
#  驗的是 `lot.start` 這條路真的走得通：
#     control.takeover -> lot.start(tag=LotID, value=OperatorID)
#     -> fLotInfo->edtSysLotID/edtSysOperatorID -> SetLotStart
#     -> RunInfo.bLotStart == true
#
#  ⚠⚠ **會寫真實檔**：`SetLotStart` -> `ReadWriteLotInfo(false)` 會寫
#     `D:\HT9045\config\config.ini` 的 `[Lot Info]`。
#     ⇒ 跑之前一定要 `python tools/realfile_guard.py snap <tag>`，
#       跑完 `check`，沒問題 `drop`、有問題 `restore`。
#       **這支探針自己不做那件事** —— 守衛要在外面，這樣忘了包就會被看見。
#
#  ⓘ 它驗不到的：**瀏覽器那顆鈕**。按鈕 -> HT9045Recipe.lotStart 那一段是
#     JS，這支只驗 JS 之後的整條路。UI 本身靠 `node --check` 與讀碼。
# =============================================================================
import json
import os
import subprocess
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, wait_ack   # noqa: E402

PORT = 8082
LOT = "Q27PROBE"
OPR = "AIPROBE"

g_pass = 0
g_fail = 0


def check(cond, msg):
    global g_pass, g_fail
    if cond:
        g_pass += 1
        print("  PASS: %s" % msg)
    else:
        g_fail += 1
        print("  FAIL: %s" % msg)


def newest_wb_serve():
    cands = []
    for d in os.listdir(ROOT):
        if not d.startswith("build"):
            continue
        p = os.path.join(ROOT, d, "wb_serve.exe")
        if os.path.isfile(p):
            cands.append((os.path.getmtime(p), p))
    if not cands:
        return None
    cands.sort()
    return cands[-1][1]


def main():
    exe = sys.argv[1] if len(sys.argv) > 1 else newest_wb_serve()
    if not exe:
        print("找不到 wb_serve.exe")
        return 2
    print("wb_serve : %s" % exe)
    print("   mtime : %s" % time.strftime("%Y-%m-%d %H:%M:%S",
                                          time.localtime(os.path.getmtime(exe))))

    log = open(os.path.join(ROOT, "_q27_serve.log"), "wb")
    proc = subprocess.Popen([exe, "--allow-cmd", "--seconds", "90",
                             "--port", str(PORT)],
                            cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    try:
        # 不要用固定 sleep：wb_serve 啟動要載機台組態、建 fTeach 的登錄表、
        # 載 ContactForce 表…，20260920 實測已超過 3 秒，而且會隨樹長大。
        # 固定睡太短會拿到 ConnectionRefused，那看起來像功能壞掉。
        sock = None
        leftover = b""
        deadline = time.monotonic() + 40.0
        lastErr = None
        while time.monotonic() < deadline:
            try:
                sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045",
                                              time.monotonic() + 5.0)
                break
            except Exception as e:                       # noqa: BLE001
                lastErr = e
                time.sleep(0.4)
        if sock is None:
            print("連不上 wb_serve：%s" % lastErr)
            return 3

        print("\n[1] 取單一操作權杖（lot.start 不在豁免名單裡）")
        send_text(sock, json.dumps({"type": "cmd", "id": 1,
                                    "cmd": "control.takeover"}))  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        ack = wait_ack(sock, leftover, 1, time.monotonic() + 10.0)
        leftover = b""      # read_frames 已經把它吃掉了
        check(bool(ack) and ack.get("ok") is True,
              "control.takeover ok（拿不到權杖的話下面全部會是 not-operator）")

        print("\n[2] ★ 空參數要被擋（與 golden main.cpp:5213-5217 同語意）")
        send_text(sock, json.dumps({"type": "cmd", "id": 2, "cmd": "lot.start",
                                    "tag": "", "value": ""}))
        ack = wait_ack(sock, leftover, 2, time.monotonic() + 10.0)
        check(bool(ack) and ack.get("ok") is False,
              "空 LotID/OperatorID -> ok:false（不是靜默接受）")

        print("\n[3] ★★ 正常的 lot.start")
        send_text(sock, json.dumps({"type": "cmd", "id": 3, "cmd": "lot.start",
                                    "tag": LOT, "value": OPR}))
        ack = wait_ack(sock, leftover, 3, time.monotonic() + 20.0)
        print("     ack = %s" % json.dumps(ack, ensure_ascii=False)[:200])
        check(bool(ack) and ack.get("ok") is True, "lot.start ok:true")

        # ⚠ 20260921 第一次跑時我把這裡寫錯了：去 ack["detail"] 找，
        #   但 wb_serve.cpp:3149-3151 的 CompleteCommand 是把 JsonWriter 的
        #   物件**攤平併進 ack 頂層**，不是塞進 detail。
        #   實測 ack = {"type":"ack","id":3,"ok":true,
        #               "lotId":"...","operatorId":"...","lotStart":true}
        #   ⇒ 當時「失敗」的是我的斷言，不是那條路。兩邊都看，頂層優先。
        detail = dict(ack) if isinstance(ack, dict) else {}
        if isinstance(detail.get("detail"), dict):
            detail.update(detail["detail"])
        elif isinstance(detail.get("detail"), str):
            try:
                detail.update(json.loads(detail["detail"]))
            except Exception:          # noqa: BLE001
                pass
        check(detail.get("lotId") == LOT,
              "ack 回報的 lotId 就是我們送的（%r）" % detail.get("lotId"))
        # ★ 這一格才是真正要驗的：不是「指令收到了」，是**機台的狀態真的變了**。
        #   wb_serve.cpp:3146-3148 明講 ok 用的是 RunInfo.bLotStart 本身。
        check(detail.get("lotStart") is True,
              "★ RunInfo.bLotStart == true（golden 的狀態，不是我們的回報）")

    finally:
        try:
            proc.wait(timeout=100)
        except Exception:              # noqa: BLE001
            proc.kill()
        log.close()

    print("\n==== 結果: %d PASS, %d FAIL ====" % (g_pass, g_fail))
    return 0 if g_fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
