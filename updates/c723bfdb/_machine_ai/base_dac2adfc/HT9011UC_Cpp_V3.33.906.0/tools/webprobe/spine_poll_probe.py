# -*- coding: utf-8 -*-
# =============================================================================
#  spine 有沒有在輪詢？—— 用 gdb 中斷點數次數（AI(W906-P0-4) 20260920）
#
#  ## 這支在問什麼
#
#  使用者 20260920 說明了他的驗收方式：
#      「我主要是會在 ainarm9045 攔中斷，在模擬狀態下，
#        是否有重複兩次以上進來此 function」
#
#  ⇒ 判準不是「有沒有當掉」，是**狀態機有沒有被反覆驅動**。
#     進得去而且**重複進入** = spine 真的在輪詢 = START 成立。
#     這比任何「回傳 true」的斷言都嚴格。
#
#  本檔把那個手動動作變成**可重跑的量測**，理由是：
#  「某天某人看到過」不是回歸保護，會當掉的檢查才是。
#
#  ## 量法
#
#  gdb 批次模式下斷點，命中就印一行然後 continue（不停住），
#  最後數那一行出現幾次。等同於手動按 F5 + 看它中斷幾次。
#
#  ## ★ 20260920 16:xx 起：它會**自己按 START**
#
#  在那之前這支只是把 wb_serve 跑起來然後數。那是**量不到東西的**，
#  因為 20260920 補上了 golden 的 `if(SystemStart)` 守衛
#  （csystem.cpp:17130；使用者 Q15 確認真實機台就是這樣）——
#  沒按 START 就什麼都不會跑，這正是我們要的行為。
#
#  ⇒ 現在它會在背景開一條執行緒，握手之後送
#     `control.acquire` + `start.run`，再數命中。
#
#  ## ⚠ 它也會告訴你**卡在哪一關**
#
#  鏈路是：
#      MainProc -> if(SystemStart)                    csystem.cpp:17130（golden）
#               -> W906_MainProcHomeDispatch() 若回 true 就 return
#                  -> DoHomeProcess -> ProcessMotorHome()   uhome.cpp:1180
#               -> DoAllProcess()        csystem.cpp
#                  -> DoInArm()          ainarm2.cpp
#                     -> DoInArm_9045()  ainarm9045.cpp
#
#  所以同時在**四個**點下斷點。哪一個有命中、哪一個沒有，
#  直接指出鏈路斷在哪 —— 而不是只知道「最後那個沒進去」。
#
#  已知的第一道關卡：`ainarm2.cpp` 的 `bInitialStartIndexCheckDone`，
#  設它的是 `atester.cpp`（要先跑過 Index Check）。
# =============================================================================
import json
import os
import re
import subprocess
import sys
import threading
import time

ROOT = r"D:\HT9045\HT9011UC_Cpp_V3.33.906.0"
GDB = os.path.join(ROOT, "..", "install", "mingw32-16.2.0", "mingw32", "bin", "gdb.exe")
PORT = 8093
# 探針用的工單／操作員。`StartFromWeb()` 會擋空的 LotID/OperatorID，
# 所以探針要先開工單才能按 START（見 Starter.run 的說明）。
LOT_ID = "SPINEPROBE"
LOT_OPERATOR = "PROBE"

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, wait_ack   # noqa: E402

# (顯示名, 斷點位置, 這個點代表什麼)
# ⚠ 用**函式名**，不要用 `file:line`。
#   20260920 實測：`break csystem.cpp:3190` 這種形式在這顆二進位上
#   **綁不上**（`info breakpoints` 回 "No breakpoints"），於是探針
#   三個點都回 0 —— 而那個 0 是**測不到**，不是沒發生。
#   一個永遠回 0 的探針比沒有探針更糟，因為它看起來像有在量。
#   函式名形式已驗證會綁：DoAllProcess -> 0x68c048、DoInArm_9045 -> 0x49caea。
POINTS = [
    ("ProcessMotorHome", "ProcessMotorHome", "歸零狀態機 —— 有命中代表 START 之後歸零真的起跑"),
    ("DoAllProcess",     "DoAllProcess",     "spine 本體 —— 有命中代表歸零臂沒有攔截"),
    ("DoInArm",          "DoInArm",          "InArm 派發 —— 有命中代表 spine 走到了 InArm"),
    ("DoInArm_9045",     "DoInArm_9045",     "★ 使用者的判準 —— 重複 >=2 次才算輪詢"),
]


def _fhome_addr(exe):
    """用 `nm` 當場問 `fHome` 這個全域指標的位址；問不到就回 None。

    ⚠ 回 None 時 `build_script` 會**不印 STEP**，而報告那邊會明說
      「沒有 iHomeStep 資料」—— 不要退化成印 0，那會被當成「步號是 0」。
    """
    nm = os.path.join(ROOT, "..", "install", "mingw32-16.2.0", "mingw32", "bin", "nm.exe")
    if not os.path.isfile(nm):
        return None
    try:
        out = subprocess.run([nm, "--defined-only", exe],
                             capture_output=True, timeout=120).stdout.decode("utf-8", "replace")
    except Exception:                                            # noqa: BLE001
        return None
    for line in out.splitlines():                                # ⚠ 本機 nm 輸出是 CRLF，用 splitlines
        parts = line.split()
        if len(parts) == 3 and parts[2] in ("_fHome", "fHome"):
            try:
                return int(parts[0], 16)
            except ValueError:
                return None
    return None


def build_script(seconds, exe):
    """產生 gdb 批次腳本。命中就印、不停住。

    ★ AI(W906-P0-5) 20260920: `ProcessMotorHome` 那一點**另外印 `iHomeStep`**。
      P0-5 的完成條件 #3 問的就是「退樁之後 iHomeStep 走到哪」，
      而原本這支只回答「有沒有進去」。命中次數答不了「卡在哪一步」。

      ⚠ Release build 沒有 DWARF，寫不了 `fHome->iHomeStep`。
        改用位址：`fHome` 是 `TfHome*`，`iHomeStep` 在 vptr 之後的第一個 int
        ⇒ offset 4（20260920 用記憶體 dump 驗過，值是 0x00000001）。
      ⚠ `fHome` 的位址**每次重建都會變**，所以這裡用 `nm` 當場問，
        不要寫死 —— 寫死的那一刻它就開始腐壞，而且腐壞之後印出來的是
        某個無關位址的內容，看起來像個合理的步號。
    """
    fhome = _fhome_addr(exe)
    lines = ["set pagination off", "set confirm off"]
    for name, loc, _ in POINTS:
        extra = []
        if name == "ProcessMotorHome" and fhome:
            extra = ['  printf "STEP %%d\\n", *(int*)(*(int*)0x%08x + 4)' % fhome]
        lines += [
            "break %s" % loc,
            "commands",
            "  silent",
            '  printf "HIT %s\\n"' % name,
        ] + extra + [
            "  continue",
            "end",
        ]
    # ★ 自我檢查：先印出斷點清單。綁不上就會看到 "No breakpoints"，
    #   而那時的 0 命中代表「量不到」，不是「沒發生」。
    lines += ["info breakpoints"]
    lines += ["run --allow-cmd --seconds %d --port %d" % (seconds, PORT), "quit"]
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
#  背景執行緒：等服務起來，然後真的按 START
# ---------------------------------------------------------------------------
class Starter(threading.Thread):
    def __init__(self, delay):
        threading.Thread.__init__(self)
        self.daemon = True
        self.delay = delay
        self.log = []

    def say(self, s):
        self.log.append(s)

    def run(self):
        time.sleep(self.delay)
        # ⚠ **要重試**。gdb 底下 wb_serve 起得比裸跑慢很多（要載符號），
        #   第一次寫死「睡 8 秒就連」得到的是 WinError 10061（連線被拒），
        #   而那會讓探針回報四個 0 —— 又一個「量不到卻看起來像沒發生」。
        sock = None
        leftover = b""
        deadline = time.monotonic() + 60.0
        attempt = 0
        while time.monotonic() < deadline:
            attempt += 1
            try:
                sock, leftover = ws_handshake("127.0.0.1", PORT, "/ht9045",
                                              time.monotonic() + 10.0)
                break
            except Exception as e:                               # noqa: BLE001
                last = e
                time.sleep(1.0)
        if sock is None:
            self.say("握手失敗（試了 %d 次）：%s" % (attempt, last))
            return
        self.say("握手成功（第 %d 次嘗試）" % attempt)
        pending = [leftover]
        cid = [0]

        def cmd(name, value=None, tag=None, tmo=15.0):
            cid[0] += 1
            m = {"type": "cmd", "id": cid[0], "cmd": name}
            if value is not None:
                m["value"] = value
            if tag is not None:
                m["tag"] = tag
            send_text(sock, json.dumps(m))
            lo = pending[0]
            pending[0] = b""
            return wait_ack(sock, lo, cid[0], time.monotonic() + tmo)

        a = cmd("control.acquire")
        self.say("control.acquire -> %s" % (a if a is None else a.get("ok")))
        if a is None or a.get("ok") is not True:
            self.say("⚠ 沒拿到操作權 —— start.run 會被拒絕，下面的 0 是「沒按到」")
            return

        # ★ 先開工單。這**不是**繞過檢查，是滿足 golden 自己的前置條件：
        #   20260920 實測，少了這一步 `StartFromWeb()` 會回 false 並印
        #   `[ShowMyMessage] Please Enter LotID and Operator ID!!`
        #   （golden main.cpp:5213-5217 的同一條拒絕）。
        #   ⚠ `lot.start` 會寫真實的工單資料 —— 跑這支探針之前要先
        #     `python tools/realfile_guard.py snap <tag>`（PLAN §0.6）。
        c = cmd("lot.start", value=LOT_OPERATOR, tag=LOT_ID, tmo=30.0)
        self.say("lot.start(%s/%s) -> %s" % (LOT_ID, LOT_OPERATOR,
                                             c if c is None else c.get("ok")))
        if c is None or c.get("ok") is not True:
            self.say("⚠ 工單沒開成 —— start.run 仍會被 LotID/OperatorID 擋下")

        b = cmd("start.run", tmo=30.0)
        if b is None:
            self.say("start.run -> 沒有 ack（逾時）")
        else:
            self.say("start.run -> ok=%s  %s"
                     % (b.get("ok"), {k: v for k, v in b.items() if k != "ok"}))


def main():
    # ★ AI(W906-P0-5) 20260920: `--seconds N` —— 預設 40 秒答不了「卡住 vs 還沒走到」。
    #   歸零尾段（case 1310）在離線時走的是**模擬移動**：
    #   `GalilTwoY_Move` 的 `Motor->Enable==false` 分支每呼叫一次把 Position
    #   推進 `Speed`，到目標才回 true（myGALILmotor.cpp:5596-5610）。
    #   ⇒ 500 ms 一個 tick，40 秒只有約 80 次機會。看到「同一步重複 33 次」
    #     **不能**直接讀成卡住 —— 要先給它足夠的時間再看。
    argv = [a for a in sys.argv[1:]]
    seconds = 40
    if "--seconds" in argv:
        i = argv.index("--seconds")
        seconds = int(argv[i + 1])
        del argv[i:i + 2]
    exe = argv[0] if argv else None
    if exe is None:
        cands = []
        for d in os.listdir(ROOT):
            if not d.startswith("build"):
                continue
            p = os.path.join(ROOT, d, "wb_serve.exe")
            if os.path.isfile(p):
                cands.append((os.path.getmtime(p), p))
        if not cands:
            print("找不到 wb_serve.exe")
            return 2
        cands.sort()
        exe = cands[-1][1]

    if not os.path.isfile(GDB):
        print("找不到 gdb：%s" % GDB)
        return 2

    print("wb_serve : %s" % exe)
    print("   mtime : %s" % time.strftime("%Y-%m-%d %H:%M:%S",
                                          time.localtime(os.path.getmtime(exe))))
    print("gdb      : %s" % GDB)
    print()

    # seconds 由 main() 開頭的 `--seconds` 決定（預設 40）
    scr = os.path.join(ROOT, "_spine_poll.gdb")
    with open(scr, "w", encoding="ascii") as f:
        f.write(build_script(seconds, exe))

    print("下斷點：")
    for name, loc, why in POINTS:
        print("  %-18s %-20s %s" % (name, loc, why))
    print()
    print("跑 %d 秒，第 8 秒送 control.acquire + start.run …" % seconds)

    starter = Starter(delay=8.0)
    starter.start()

    # ★ AI(W906-P0-7) 20260921: **逾時也要把拿到的東西留下來。**
    #   第一版直接讓 TimeoutExpired 往上炸，於是整支探針的輸出只剩一個
    #   Python traceback —— 而逾時**正是最需要那份資料的時候**：
    #   20260921 03:5x 這一輪就是逾時，416 個樣本一個都沒存到，
    #   只能拿上一輪 01:54 的 raw log（不同的二進位）乾瞪眼。
    #   ⚠ 逾時本身也是一種量測結果：斷點命中率暴增會讓 gdb 追不上
    #     `--seconds N` 的自我結束，而那可能代表**歸零跑完、spine 開始輪詢**。
    #     把它當成錯誤丟掉，等於丟掉一個好消息。
    timed_out = False
    try:
        r = subprocess.run([GDB, "-batch", "-x", scr, exe],
                           cwd=ROOT, capture_output=True, timeout=seconds + 180)
        so, se = r.stdout, r.stderr
    except subprocess.TimeoutExpired as e:                       # noqa: PERF203
        timed_out = True
        so = e.stdout or b""
        se = e.stderr or b""
    out = so.decode("utf-8", "replace") + se.decode("utf-8", "replace")
    if timed_out:
        print()
        print("⚠ gdb 逾時（%d 秒）——下面的數字是**逾時前**收到的，不是完整一輪。"
              % (seconds + 180))
        print("  斷點命中率太高時 gdb 會拖慢受測行程，讓它來不及自己結束。")
        print("  ⇒ 如果 DoInArm_9045 的命中數很高，這其實是**好消息**（spine 在輪詢）。")
    # ★ AI(W906-P0-5) 20260920: 把**原始**輸出留下來。
    #   摘要只答「命中幾次 / 走到哪一步」，答不了「為什麼卡住」——
    #   而 `ShowMyMessage` / `RecordProcess` 印的那些行（例如 case 1310 的
    #   " MTestY1, MTrayX, ... Home Time Out"）正是缺哪個 flag 的直接證據。
    #   20260920 我就是因為沒留它而多跑了一輪 4 分鐘的探針。
    raw = os.path.join(ROOT, "_spine_poll_raw.log")
    with open(raw, "w", encoding="utf-8", errors="replace", newline="\n") as f:
        f.write(out)

    starter.join(timeout=5.0)
    print()
    print("START 執行緒說：")
    for l in starter.log or ["（沒有輸出 —— 它可能還沒跑到）"]:
        print("  " + l)

    # ★ 先確認斷點真的綁上了 —— 沒綁上就不要回報 0
    if re.search(r'No breakpoints, watchpoints', out):
        print()
        print("❌ **斷點沒有綁上** —— 這次量不到，不是「沒發生」。")
        print("   檢查：二進位有沒有除錯符號、函式名對不對。")
        print("   ⚠ 不要把這種情況讀成「鏈路斷了」。")
        return 2
    bound = len(re.findall(r'^\d+\s+breakpoint\s+keep', out, re.M))
    print()
    print("斷點綁上：%d / %d" % (bound, len(POINTS)))
    if bound < len(POINTS):
        print("⚠ 有斷點沒綁上 —— 下面的 0 可能是量不到而不是沒發生")

    counts = {}
    for name, _, _ in POINTS:
        counts[name] = len(re.findall(r'^HIT %s$' % re.escape(name), out, re.M))

    print()
    print("%-18s %8s  %s" % ("斷點", "命中", "判定"))
    print("-" * 64)
    for name, _, _ in POINTS:
        c = counts[name]
        if name == "DoInArm_9045":
            v = "★ 輪詢成立" if c >= 2 else ("只進去一次（不算輪詢）" if c == 1 else "沒進去")
        else:
            v = "有到" if c > 0 else "沒到"
        print("%-18s %8d  %s" % (name, c, v))

    # ★ AI(W906-P0-5) 20260920: iHomeStep 序列 —— P0-5 完成條件 #3 問的就是這個。
    #   「ProcessMotorHome 命中 N 次」答不了「卡在哪一步」，而卡在哪一步才是
    #   歸零有沒有前進的唯一判準（計畫書記的現況是 …600→650→600→700(x43 停住)）。
    steps = [int(m) for m in re.findall(r'^STEP (\d+)$', out, re.M)]
    print()
    if not steps:
        print("iHomeStep：**沒有資料**（nm 問不到 fHome，或 ProcessMotorHome 沒命中）")
        print("  ⚠ 這是「量不到」，不要讀成「步號是 0」。")
    else:
        seq = []
        for s in steps:
            if seq and seq[-1][0] == s:
                seq[-1][1] += 1
            else:
                seq.append([s, 1])
        print("iHomeStep 序列（%d 個樣本）：" % len(steps))
        print("  " + " -> ".join("%d%s" % (v, ("(x%d)" % n) if n > 1 else "")
                                 for v, n in seq))
        mx = max(steps)
        print("  最大步號 = %d；末端 = %d" % (mx, steps[-1]))
        # 末端連續重複 = 卡住（狀態機每個 tick 都回到同一步）
        if seq[-1][1] >= 5:
            print("  ⚠ 末端 %d 連續出現 %d 次 —— **卡在這一步**。" % (seq[-1][0], seq[-1][1]))

    print()
    ok = counts["DoInArm_9045"] >= 2
    if ok:
        print("結論：✅ DoInArm_9045 被重複進入 %d 次 —— spine 在輪詢，"
              "與 BCB6 的形狀一致" % counts["DoInArm_9045"])
    else:
        # 指出鏈路斷在哪 —— 這比「失敗」有用得多
        if counts["ProcessMotorHome"] == 0:
            print("結論：❌ 連 ProcessMotorHome 都沒進去 ——")
            print("      表示 `SystemStart` 沒有變 true（START 沒成立），")
            print("      或 W906_MainProcHomeDispatch 的第一個臂沒有選中歸零。")
            print("      先看上面 START 執行緒說了什麼。")
        elif counts["DoAllProcess"] == 0:
            print("結論：❌ 歸零有起跑，但 DoAllProcess 沒進去 ——")
            print("      歸零還沒跑完（每個 tick 都被歸零臂 return 掉），")
            print("      或 CheckMotorHome() 仍然回 false。看 stdout 有沒有")
            print("      \"Motor not home yet!\"。")
        elif counts["DoInArm"] == 0:
            print("結論：❌ spine 有跑，但沒走到 DoInArm。")
        else:
            print("結論：❌ 進了 DoInArm 但沒進 DoInArm_9045 ——")
            print("      最可能卡在 ainarm2.cpp 的 bInitialStartIndexCheckDone")
            print("      （要先跑過 Index Check，設它的是 atester.cpp），")
            print("      或機型分支沒走到那四個呼叫點之一。")

    tail = [l for l in out.splitlines()
            if "Motor not home yet" in l or "Program received" in l
            or "spine pump" in l or "start.run" in l]
    if tail:
        print()
        print("gdb 另外報告：")
        seen = set()
        for l in tail:
            s = l.strip()[:100]
            if s in seen:
                continue
            seen.add(s)
            print("  " + s)
            if len(seen) >= 8:
                break

    try:
        os.remove(scr)
    except OSError:
        pass
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
