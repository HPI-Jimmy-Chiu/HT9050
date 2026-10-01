# -*- coding: utf-8 -*-
# =============================================================================
#  P3 完成條件的端到端量測（AI(W906-P3-OLP) 20260919）
#
#  完成條件原文：「每個命令在 wb_serve 的指令通道上各有一條對應的 cmd，
#  逐條端到端驗過，且每一條的 ack 反映真實結果不是罐頭 ok」。
#
#  ## 兩條腿，刻意分開
#
#  **腿 1 — 可達性（35/35，零寫入）**
#    每一條白名單指令故意送**錯的欄數**。回 kWebOlpBadArity(-2) 就證明
#    「它在表上、派發到得了」；回 kWebOlpNotWhitelisted(-1) 就是沒接上。
#    這條腿不寫任何檔，所以可以對全部 35 條跑。
#    ⚠ 兩個例外：olp.mapping / olp.dutOnOff 是**動態欄數**（依機台站數），
#      欄數檢查對它們只檢查上限，所以用「送 41 個欄位」來探 —— 超過 golden
#      訊框的 40 格，必然回 BadArity 而**不會**執行。
#
#  **腿 2 — 真的寫得進去（代表性子集，snapshot → 擾動 → 驗 → 還原）**
#    可達性證明不了「值真的落地」。所以挑三條各自寫不同檔的指令，
#    真的改值、真的看檔案內容變了、再從 snapshot 還原並驗位元組全同。
#    ⚠ 這一段**會寫真實配方檔**，這是刻意的（CLAUDE.md 20260918 裁決：
#      一律確實讀寫檔案，用「備份 → 驗證 → 刪備份」取代擋寫）。
#
#  ## 這支不問什麼
#  不問「ack 是不是 ok」。ack 回 true 只代表 status==0；本檔要的是
#  **status 的值**與**detail 帶不帶 golden 分支名** —— 那才是「不是罐頭」。
# =============================================================================
import hashlib
import json
import os
import re
import subprocess
import sys
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
sys.path.insert(0, os.path.join(ROOT, "tools", "webprobe"))
from cmd_probe import ws_handshake, send_text, wait_ack   # noqa: E402

PORT = 8079

# WebOlp.cpp 的白名單，連欄數一起 —— 與那張表同一份量測。
# （不做成 olp.list 指令：多開一條指令只為了測試，是把測試成本轉嫁成攻擊面。）
WHITELIST = [
    ("olp.fixTrayDefine", 5), ("olp.mapping", -1), ("olp.dutOnOff", -1),
    ("olp.soakTime", 1), ("olp.temperature", 1),
    ("olp.lowYield", 3), ("olp.byArmPerSiteDiffYield", 3),
    ("olp.consecFailAlarmByHead", 2), ("olp.consecFailAlarmBySocket", 2),
    ("olp.allSiteFailFor9045", 1), ("olp.contactModeFor9045", 1),
    ("olp.contactVacuumMode", 1), ("olp.contactDropWait", 1),
    ("olp.slowContactSpeed", 1), ("olp.shuttleWaitOutSideCamber", 1),
    ("olp.pickShuttleDeviceAfterTested", 1),
    ("olp.pickShuttleDeviceThenWait", 1),
    ("olp.pickShuttleDeviceTogether32SiteN", 1),
    ("olp.indexArm1Height", 4), ("olp.indexArm2Height", 4),
    ("olp.testICCheckMode", 1), ("olp.aboveSocket", 1),
    ("olp.hotPlate1", 1), ("olp.hotPlate2", 1),
    ("olp.testerInitialMaximumTest", 1), ("olp.testerMaximumTest", 1),
    ("olp.testerDummyTest", 1), ("olp.testerStartDelay", 1),
    ("olp.hotZ1Down", 1), ("olp.hotShuttleSoakMode", 1),
    ("olp.ambientCheck", 1), ("olp.ambientCheckTemp", 1),
    ("olp.temperatureOffset", 1), ("olp.contactCountForOffsetPeriod", 1),
    ("olp.contactCountForCoolDown", 1),
]

DENY = ["olp.start", "olp.cleanOut", "olp.homeAndStart", "olp.oneCycle",
        "olp.category", "olp.binDefine", "olp.setupFileName", "olp.tempMode",
        "olp.connection", "olp.testMode", "olp.lotInfo", "olp.startMode",
        "olp.ppUl", "olp.ppDl", "olp.clearReport", "olp.pause", "olp.resume",
        "olp.NotAThing"]

K_NOT_WHITELISTED = -1
K_BAD_ARITY = -2

g_pass = 0
g_fail = 0


def check(cond, msg):
    global g_pass, g_fail
    if cond:
        print("  PASS: %s" % msg)
        g_pass += 1
    else:
        print("  FAIL: %s" % msg)
        g_fail += 1


ACK_STATUS = re.compile(r'^status=(-?\d+)\s*')


def ack_body(a):
    """照 AckJson 的**真形狀**讀（WebBridgeServer.cpp:1215-1245，量過的）。

        ok=false -> {"type":"ack","id":N,"ok":false,"error":"<字串>"}
                    字串開頭固定 `status=<N> `（wb_serve 的 olp.* 分支保證）
        ok=true  -> 第三參數若是 JSON 物件，欄位被**攤平內嵌**在 ack 上，
                    所以直接讀 a["status"] / a["golden"] / a["detail"]

    ⚠ 我第一版讀 a["message"] —— **那個欄位根本不存在**。
      結果 35 條全部誤判成 status=None，還印出「這個 wb_serve 是舊的」
      這種完全錯誤的診斷。那是我按照「我以為的契約」寫探針，
      而不是按照被測物實際送出的東西（memory: selftest 要走同一條存取路徑）。
    """
    if a is None:
        return None, None
    ok = a.get("ok")
    if ok:
        return True, {"status": a.get("status"), "golden": a.get("golden"),
                      "detail": a.get("detail"), "accepted": a.get("accepted")}
    err = a.get("error")
    st = None
    if isinstance(err, str):
        m = ACK_STATUS.match(err)
        if m:
            st = int(m.group(1))
    return False, {"status": st, "detail": err, "golden": None}


GUARD_TAG = "p3probe"


def guard(action, tag):
    """呼叫 tools/realfile_guard.py。回傳它的 exit code。

    ⚠ 一定要帶 PYTHONIOENCODING —— 這台的 console 是 cp950，
    工具印中文會炸（memory: python-console-cp950-use-pythonioencoding）。
    """
    env = dict(os.environ)
    env["PYTHONIOENCODING"] = "utf-8:replace"
    r = subprocess.run(
        [sys.executable, os.path.join(ROOT, "tools", "realfile_guard.py"),
         action, tag],
        cwd=ROOT, env=env, capture_output=True)
    out = r.stdout.decode("utf-8", "replace")
    for ln in out.splitlines():
        if ("CHANGED" in ln or "MISSING" in ln or "結論" in ln
                or "備份" in ln):
            print("   [guard %s] %s" % (action, ln.strip()))
    return r.returncode


def recipe_dir():
    with open(os.path.join(r"D:\HT9045", "SetUp.inf"), "rb") as f:
        name = f.read().decode("cp950", "replace").splitlines()[0].strip()
    return os.path.join(r"D:\HT9045", "IniData", "Data", name)


def hash_dir(d):
    out = {}
    for fn in sorted(os.listdir(d)):
        p = os.path.join(d, fn)
        if os.path.isfile(p):
            with open(p, "rb") as f:
                out[fn] = hashlib.md5(f.read()).hexdigest()
    return out


def main():
    # ⚠ 不要寫死某個 build 目錄。這棵樹同時有幾十個 build_*，其中不乏
    #   半途被殺掉那幾輪留下來的（刪不掉，EBUSY），而它們裡面的 wb_serve.exe
    #   是**舊源碼**建的。拿舊二進位驗新程式碼，結果看起來會像成功。
    #   ⇒ 預設挑最新的那一個，並把 mtime 印出來讓人對帳。
    if len(sys.argv) > 1:
        exe = sys.argv[1]
        if not os.path.isfile(exe):
            print("指定的 wb_serve.exe 不存在：%s" % exe)
            return 2
    else:
        cands = []
        for d in os.listdir(ROOT):
            if not d.startswith("build"):
                continue
            p = os.path.join(ROOT, d, "wb_serve.exe")
            if os.path.isfile(p):
                cands.append((os.path.getmtime(p), p))
        if not cands:
            print("找不到任何 wb_serve.exe —— 先跑 gate 或 simbuild")
            return 2
        cands.sort()
        exe = cands[-1][1]
        if len(cands) > 1:
            print("（共 %d 個 wb_serve.exe，挑最新的）" % len(cands))
    print("wb_serve : %s" % exe)
    print("   mtime : %s" % time.strftime("%Y-%m-%d %H:%M:%S",
                                          time.localtime(os.path.getmtime(exe))))

    rdir = recipe_dir()
    print("作用中配方: %s" % rdir)

    # ---- 備份：用 realfile_guard，不自己手刻 -------------------------------
    #   §0.6 的紀律有工具了（tools/realfile_guard.py），而它自 commit e15be87
    #   起涵蓋作用中工單的整個配方樹 —— 正是本探針會寫到的地方。
    #   兩份備份邏輯並存遲早會有一份被改而另一份沒有，所以這裡呼叫它。
    if guard("snap", GUARD_TAG) != 0:
        print("snap 失敗（可能上一次的備份還在，先 drop）")
        return 2
    before = hash_dir(rdir)
    print("已 snap（realfile_guard 標籤 %s），配方樹 %d 個檔" % (GUARD_TAG, len(before)))

    log = open(os.path.join(ROOT, "_p3olp_probe_serve.log"), "wb")
    proc = subprocess.Popen([exe, "--allow-cmd", "--seconds", "180",
                             "--port", str(PORT)],
                            cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    ok_overall = False
    try:
        time.sleep(3.0)
        sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045",
                                      time.monotonic() + 20.0)
        cid = [0]
        # ⚠ handshake 會連帶讀進一些已經到達的位元組（leftover）。
        #   第一次 wait_ack **必須**把它帶進去，否則第一個指令的 ack
        #   就躺在那個緩衝裡永遠讀不到。
        #   原本每次都傳 b""，所以 acquire 永遠回 null ——
        #   這一輪沒釀成誤判純屬運氣（第一個指令剛好是 acquire，
        #   它成不成功由後面 35 條反證），但只要有人把一條真的斷言
        #   排到第一個，它就會無聲失敗。
        pending = [leftover]

        def cmd(name, value=None, tmo=10.0):
            cid[0] += 1
            m = {"type": "cmd", "id": cid[0], "cmd": name}
            if value is not None:
                m["value"] = value
            send_text(sock, json.dumps(m))
            lo = pending[0]
            pending[0] = b""
            return wait_ack(sock, lo, cid[0], time.monotonic() + tmo)

        a = cmd("control.takeover")  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
        # 這是後面每一條的**前提**，不是給人看的日誌。
        check(a is not None and a.get("ok") is True,
              "control.takeover 拿到操作權（後面 53 條指令的前提）")
        if a is None or a.get("ok") is not True:
            print("   沒有操作權，後面的結果沒有意義 —— 中止")
            raise SystemExit(1)

        # ------------------------------------------------------------------
        #  腿 1 -- 可達性（零寫入）
        # ------------------------------------------------------------------
        print("\n[1] 可達性：35 條白名單各送一次錯欄數，看它分派得到")
        reach = 0
        for name, arity in WHITELIST:
            # 固定欄數的：送 arity+1 個；動態欄數的：送 41 個（超過 golden 的 40 格）
            n = (arity + 1) if arity >= 0 else 41
            a = cmd(name, ",".join(["0"] * n))
            _, body = ack_body(a)
            st = body.get("status") if body else None
            if body and body.get("status") is None:
                print("     !! ack 沒帶 status —— 這個 wb_serve 比 P3 的"
                      " ack 契約舊，先 rebuild 再跑")
            if st == K_BAD_ARITY:
                reach += 1
            else:
                print("     !! %-40s status=%s detail=%s"
                      % (name, st, (body or {}).get("detail")))
        check(reach == len(WHITELIST),
              "35 條白名單全部分派得到（回 BadArity 而不是 NotWhitelisted）：%d/%d"
              % (reach, len(WHITELIST)))

        print("\n[2] 拒絕名單：%d 條都必須被擋" % len(DENY))
        denied = 0
        for name in DENY:
            a = cmd(name, "1")
            okv, body = ack_body(a)
            d = str((body or {}).get("detail"))
            if okv is False and (body or {}).get("status") == K_NOT_WHITELISTED:
                denied += 1
            else:
                print("     !! %-24s ok=%s status=%s detail=%s"
                      % (name, okv, (body or {}).get("status"), d))
        check(denied == len(DENY), "拒絕名單一條都沒漏：%d/%d" % (denied, len(DENY)))

        # ------------------------------------------------------------------
        #  腿 2 -- 真的寫得進去
        # ------------------------------------------------------------------
        print("\n[3] 真實寫入：三條各自寫不同檔，驗 ack + 驗檔案真的變了")
        WRITES = [
            ("olp.testerStartDelay", "3.5", "Tester.Data"),
            ("olp.ambientCheckTemp", "27",  "Temperature.Data"),
            ("olp.contactDropWait",  "12",  "Contact.Data"),
        ]
        for name, val, fn in WRITES:
            a = cmd(name, val)
            okv, body = ack_body(a)
            st = (body or {}).get("status")
            det = str((body or {}).get("detail", ""))
            gold = (body or {}).get("golden")
            print("   %-28s ok=%-5s status=%s golden=%s" % (name, okv, st, gold))
            if st == 2:
                print("     (機台正在跑 -> golden 不准改設定。這是正確行為，"
                      "但這一輪量不到寫入。)")
                check(det.find("CheckSystemStart") >= 0,
                      "%s 的 ack 指名 CheckSystemStart，不是罐頭字串" % name)
                continue
            if st == 1:
                print("     (機台裡還有 IC -> golden 的 cleanout 守衛。同上。)")
                continue
            check(st == 0, "%s -> status 0" % name)
            check(gold is not None and gold.endswith("_REQUEST"),
                  "%s 的 ack 帶 golden 分支名（%s）" % (name, gold))
            after1 = hash_dir(rdir)
            check(after1.get(fn) != before.get(fn),
                  "%s 真的改了 %s（檔案內容變了，不是只有 ack 說變了）" % (name, fn))

        sock.close()
        ok_overall = True
    finally:
        try:
            proc.terminate()
        except Exception:
            pass
        time.sleep(1.5)
        log.close()

        # ---- 還原並驗證（不管成功失敗都做）-------------------------------
        #   restore 是**無條件**的：腿 2 是故意去改真實配方的，所以跑完一定要還原。
        #   還原之後再 check 一次 —— 那一步問的是「還原得乾不乾淨」，
        #   不是「有沒有被改」。兩者是不同的問題，不可以只做一個。
        print("\n[4] 還原配方並驗證（realfile_guard restore -> check -> drop）")
        guard("restore", GUARD_TAG)
        rc = guard("check", GUARD_TAG)
        after = hash_dir(rdir)
        same = (after == before)
        check(rc == 0 and same,
              "還原後 realfile_guard check 全綠，且配方樹 %d 個檔位元組全同"
              % len(before))
        if rc == 0 and same:
            guard("drop", GUARD_TAG)
            print("   備份已刪（CLAUDE.md：驗證通過就不要留備份垃圾）")
        else:
            print("   ⚠ 備份保留（標籤 %s）—— 有差異，請人工看" % GUARD_TAG)
            for k in sorted(set(before) | set(after)):
                if before.get(k) != after.get(k):
                    print("     %-24s %s -> %s" % (k, before.get(k), after.get(k)))

    print("\n==== 結果: %d PASS, %d FAIL ====" % (g_pass, g_fail))
    return 0 if (g_fail == 0 and ok_overall) else 1


if __name__ == "__main__":
    sys.exit(main())
