# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/q30_8_mailbox_probe.py -- Q30 第 8 題的端到端驗證
#
#  AI(W906-Q30-8) 20260922
#
#  ## 要證明什麼
#
#  使用者原話：「出現異常時候要有畫面可以處理 retry/skip，**如果不小心關閉
#  網頁又開啟，也要跳出**，避免沒辦法解除導致 hang up」。
#
#  ⇒ 這支要證的**不是**「檔案有被寫出來」，而是那三件事：
#     A. 警報一跳，信箱兩個檔（.json ＋ js 墊片）都有 state=pending
#     B. **模擬「關掉網頁再開」** —— 一個 lastSeq=0 的新讀者仍然看得到它
#        （這是使用者最在意的那一條）
#     C. 答完之後 request 退役成 idle ⇒ 再開一次瀏覽器**不會**又彈一次
#
#  ## ⚠ 觸發器用現成的，不新增指令
#
#  `sys.echoErrorModal`（tools/wb_serve.cpp:2708）會呼叫**真的**
#  `ShowErrorMessage`，而且會在 pump 裡阻塞等答案 —— 正是要驗的那條路。
#  ⇒ 不需要另外做 `debug.query`。少一個「能從網頁叫出警報框」的攻擊面。
#
#  ## ⚠⚠ 會寫真實檔
#
#  寫的是 `D:\HT9045\web\JSON\`（同事的部署樹，沒有版控）。
#  這支**自己**會在跑完後把兩個通道檔還原成樣板狀態 —— 但外面仍然建議
#  先做一份 web/JSON 的複本，那個目錄不在 realfile_guard 的 TARGETS 裡。
#
#  用法:  python tools/webprobe/q30_8_mailbox_probe.py [exe]
# =============================================================================
import json
import io
import os
import shutil
import subprocess
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
MBOX = r"D:\HT9045\web\JSON"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

PORT = 8086
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


def read_json(name):
    p = os.path.join(MBOX, name + ".json")
    try:
        with io.open(p, encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:                                   # noqa: BLE001
        return {"__error__": str(e)}


def wait_for(fn, want, timeout=6.0, tick=0.2):
    """輪詢到 fn() 的結果符合 want()，回 (值, 花了幾秒, 試了幾次)。

    ⚠ 為什麼不是「睡一下再讀一次」：
      這是**非同步的檔案信箱**，對方（dialog-bridge.js）本來就每 100 ms 輪詢。
      用單次讀去斷言，等於在測「我的 sleep 猜得準不準」——
      20260922 就是這樣得到兩次執行結果不同的 flaky。
      ⇒ 正確的斷言是「在合理時間內會變成那樣」，並且**把花了多久印出來**：
        如果每次都要 3 秒，那本身就是一個要查的訊號。
    """
    t0 = time.monotonic()
    n = 0
    v = None
    while time.monotonic() - t0 < timeout:
        n += 1
        v = fn()
        if want(v):
            return v, time.monotonic() - t0, n
        time.sleep(tick)
    return v, time.monotonic() - t0, n


def read_shim(name):
    """模擬 file: 模式的瀏覽器：讀墊片、抽出 __HT9045_DATA__[name]。

    ⚠ 這一步是承重的 —— 量產啟動器是 file: 協定，瀏覽器**只讀墊片、
      完全不讀 .json**。只驗 .json 等於沒驗到量產那條路。
    """
    p = os.path.join(MBOX, "js", name + ".js")
    try:
        with io.open(p, encoding="utf-8") as f:
            txt = f.read()
    except Exception as e:                                   # noqa: BLE001
        return {"__error__": str(e)}
    key = '__HT9045_DATA__["%s"]=' % name
    i = txt.find(key)
    if i < 0:
        return {"__error__": "墊片裡找不到 %s" % key}
    body = txt[i + len(key):].rstrip()
    if body.endswith(";"):
        body = body[:-1]
    try:
        return json.loads(body)
    except Exception as e:                                   # noqa: BLE001
        return {"__error__": "墊片 JSON 解析失敗: %s" % e}


def pump(sock, leftover, want_id, deadline, seen):
    for op, payload in read_frames(sock, leftover, deadline):
        if op != 1:
            continue
        try:
            j = json.loads(payload.decode("utf-8", "replace"))
        except ValueError:
            continue
        if j.get("type") == "query":
            seen.append(j)
        elif j.get("type") == "ack" and j.get("id") == want_id:
            return j
    return None


def main():
    exe = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build", "wb_serve.exe")
    print("wb_serve : %s" % exe)
    print("   mtime : %s" % time.strftime("%H:%M:%S",
                                          time.localtime(os.path.getmtime(exe))))

    # 自備份：這支要動同事的部署樹，跑完要還原。
    #
    # ⚠⚠ **備份要在 wb_serve 起來之前做完。**
    #   20260922 第一次跑時備份與伺服器寫檔重疊，`copytree` 開著那些檔
    #   撞出 sharing violation，害墊片那一半沒寫成 ——
    #   **我的測量動作破壞了被測的東西**。
    #   （C++ 那側的重試預算也因此從 100 ms 加到 1 秒。）
    bak = os.path.join(ROOT, "_q308_mbox_bak")
    if os.path.isdir(bak):
        shutil.rmtree(bak, ignore_errors=True)
    shutil.copytree(MBOX, bak)
    time.sleep(0.5)                       # 讓檔案 handle 確實關掉
    print("   信箱備份 -> %s\n" % bak)

    log = open(os.path.join(ROOT, "_q308_serve.log"), "wb")
    proc = subprocess.Popen([exe, "--allow-cmd", "--seconds", "180",
                             "--port", str(PORT)],
                            cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    sock = None
    try:
        deadline = time.monotonic() + 45.0
        leftover = b""
        while time.monotonic() < deadline:
            try:
                sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045",
                                              time.monotonic() + 5.0)
                break
            except Exception:                                # noqa: BLE001
                time.sleep(0.4)
        if sock is None:
            print("連不上 wb_serve")
            return 3

        print("[1] 取操作權杖")
        send_text(sock, json.dumps({"type": "cmd", "id": 1, "cmd": "control.acquire"}))
        seen = []
        ack = pump(sock, leftover, 1, time.monotonic() + 15.0, seen)
        check(bool(ack) and ack.get("ok") is True, "control.acquire ok")

        print("\n[2] ★ 觸發一個真的警報（sys.echoErrorModal -> ShowErrorMessage）")
        print("    ⚠ 這個指令會**阻塞**直到有人答，所以不等它的 ack")
        send_text(sock, json.dumps({"type": "cmd", "id": 2,
                                    "cmd": "sys.echoErrorModal",
                                    "tag": "JAM9999", "value": 3}))  # K_RETRY|K_SKIP
        print("\n[3] ★ A：信箱兩個檔都寫了嗎（輪詢到逾時，不用固定 sleep）")
        req, t1, n1 = wait_for(lambda: read_json("Alarm-dialog-request"),
                               lambda v: v.get("state") == "pending")
        shim, t2, n2 = wait_for(lambda: read_shim("Alarm-dialog-request"),
                                lambda v: v.get("state") == "pending")
        print("    .json 等了 %.1f s（%d 次）、墊片等了 %.1f s（%d 次）"
              % (t1, n1, t2, n2))
        check(req.get("state") == "pending",
              ".json state=pending（實際 %r）" % req.get("state"))
        check(bool(req.get("requestId")),
              ".json requestId 非空（實際 %r）" % req.get("requestId"))
        check(isinstance(req.get("seq"), int) and req.get("seq") >= 1,
              ".json seq>=1（實際 %r）—— 0 的話瀏覽器永遠不會開框" % req.get("seq"))
        check(req.get("arguments", {}).get("code") == "JAM9999",
              "arguments.code 帶到了（實際 %r）" % req.get("arguments", {}).get("code"))
        check(req.get("arguments", {}).get("kCode") == 3,
              "arguments.kCode=3（實際 %r）" % req.get("arguments", {}).get("kCode"))
        check("__error__" not in shim,
              "js 墊片解析得出來（%s）" % shim.get("__error__", "ok"))
        check(shim.get("seq") == req.get("seq") and shim.get("state") == req.get("state"),
              "★ 墊片與 .json 內容一致 —— 量產是 file: 協定，只讀墊片")
        if shim.get("seq") != req.get("seq") or shim.get("state") != req.get("state"):
            # ⚠ 不要猜，也**不要用 mtime 判斷**。
            #   20260922 第一次跑就是被 mtime 誤導：墊片內容明明是新的，
            #   但 `ls -l` 顯示的 mtime 還是套包時的 09:42 ——
            #   `MoveFileExA` 之後的 mtime 不反映寫入時間。
            #   ⇒ 判準一律是**內容**。
            print("       .json : seq=%r state=%r requestId=%r"
                  % (req.get("seq"), req.get("state"), req.get("requestId")))
            print("       墊片  : seq=%r state=%r requestId=%r"
                  % (shim.get("seq"), shim.get("state"), shim.get("requestId")))
            # ⚠ 分辨「沒寫進去」與「我的解析器讀錯」：把原始前 160 字元印出來。
            sp = os.path.join(MBOX, "js", "Alarm-dialog-request.js")
            with io.open(sp, encoding="utf-8", errors="replace") as fh:
                raw = fh.read()
            print("       墊片原始(%d bytes) 前 160 字元:" % len(raw))
            print("       %r" % raw[:160])
            print("       墊片裡有 JAM9999 嗎: %s" % ("JAM9999" in raw))

        print("\n[4] ★★ B：模擬「關掉網頁再開」")
        print("    dialog-bridge 的 lastSeq 是記憶體狀態，重開就歸 0。")
        print("    條件是 state=='pending' && requestId && seq > lastSeq(0)。")
        fresh_would_show = (shim.get("state") == "pending"
                            and bool(shim.get("requestId"))
                            and isinstance(shim.get("seq"), int)
                            and shim.get("seq") > 0)
        check(fresh_would_show,
              "★★ 新開的瀏覽器會再彈出這個警報框（使用者最在意的那一條）")

        print("\n[5] 答它（走 dialog.response，就是 HTDialogHost 會送的那條）")
        qid = req.get("requestId")
        send_text(sock, json.dumps({"type": "cmd", "id": 3,
                                    "cmd": "dialog.response",
                                    "tag": str(qid), "value": "RETRY:BtnStart"}))
        ack = pump(sock, b"", 3, time.monotonic() + 20.0, seen)
        check(bool(ack) and ack.get("ok") is True,
              "dialog.response 被接受（實際 %s）" % json.dumps(ack, ensure_ascii=False)[:90])

        print("\n[6] ★ C：答完之後 request 退役了嗎（同樣輪詢）")
        req2, t3, n3 = wait_for(lambda: read_json("Alarm-dialog-request"),
                                lambda v: v.get("state") == "idle")
        shim2, t4, n4 = wait_for(lambda: read_shim("Alarm-dialog-request"),
                                 lambda v: v.get("state") == "idle")
        print("    .json 等了 %.1f s（%d 次）、墊片等了 %.1f s（%d 次）"
              % (t3, n3, t4, n4))
        if "__error__" in req2:
            print("       ⚠ .json 讀不到：%s" % req2["__error__"])
        if "__error__" in shim2:
            print("       ⚠ 墊片讀不到：%s" % shim2["__error__"])
        check(req2.get("state") == "idle",
              ".json 已退役成 idle（實際 %r）" % req2.get("state"))
        check(shim2.get("state") == "idle",
              "★ 墊片也退役了 —— 只退一邊的話，file: 模式下按 F5 會再彈一次")
        check(isinstance(req2.get("seq"), int) and req2.get("seq") > req.get("seq", 0),
              "退役的 seq 比原本大（單調遞增，實際 %r -> %r）"
              % (req.get("seq"), req2.get("seq")))

        print("\n[7] 伺服器自己說了什麼")
        return 0
    finally:
        try:
            if sock:
                sock.close()
        except Exception:                                    # noqa: BLE001
            pass
        try:
            proc.terminate(); proc.wait(timeout=20)
        except Exception:                                    # noqa: BLE001
            proc.kill()
        log.close()
        # 還原同事的部署樹
        for f in os.listdir(bak):
            s = os.path.join(bak, f)
            d = os.path.join(MBOX, f)
            if os.path.isfile(s):
                shutil.copy2(s, d)
        js = os.path.join(bak, "js")
        if os.path.isdir(js):
            for f in os.listdir(js):
                shutil.copy2(os.path.join(js, f), os.path.join(MBOX, "js", f))
        shutil.rmtree(bak, ignore_errors=True)
        print("   信箱已還原、備份已刪")
        try:
            with io.open(os.path.join(ROOT, "_q308_serve.log"),
                         encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    if ("query qid=" in line or "dialog" in line.lower()
                            or "echoErrorModal" in line or "信箱" in line):
                        print("   [serve] %s" % line.rstrip())
        except OSError:
            pass
        print("\n==== %d PASS / %d FAIL ====" % (g_pass, g_fail))


if __name__ == "__main__":
    sys.exit(main())
