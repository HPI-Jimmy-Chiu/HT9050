# -*- coding: utf-8 -*-
# ===========================================================================
#  tools/websync/sync_web.py -- mirror the HT9050 HTML HMI into D:\HT9045\web
#
#  AI(W906-FW2-WEBSYNC) 20260911. User ruling 20260911: D:\HT9045\web becomes
#  the runtime root for the hand-built HMI whose source lives in the HT9050
#  workspace, and web/ must hold exactly that set with no stale pages left
#  behind. This script is the mechanism for both halves.
#
#  ---------------------------------------------------------------------------
#  WHY web/ IS GITIGNORED AND THIS SCRIPT IS NOT
#  ---------------------------------------------------------------------------
#  The source is 586 files / 91 MB, and MEASURED, not estimated:
#      34 MB  two .mp4 banners under IMG/ScreenShot
#      16 MB  Production-update.json plus a byte-duplicate .js shim of it
#      14 MB  Production-runtime / Setup-current / IO-runtime -- RUNTIME DATA,
#             rewritten every time the machine or the simulator runs
#  Committing that is irreversible: git keeps every blob forever, and the
#  runtime files would add megabytes per run to the history.
#
#  It also buys nothing here. (AI(W906-BA-LEAVE) 20260914 CORRECTION: this
#  paragraph used to say `git -C D:\HT9050 ls-files "docs/機台介面 HTML/HT9045"`
#  returns ZERO and the source is untracked. That WAS true when written on
#  20260911 -- and stopped being true a few hours later, when commit aba09af
#  ("把 HT9045 HTML HMI 的「人寫的部分」納入版控", 20260911 14:03, 365 files)
#  put it under version control. Re-measured 20260914: 364 files tracked.
#  The conclusion is unchanged -- 91 MB of runtime-rewritten payload still does
#  not belong in THIS repo -- but the reason had to change, and a stale reason
#  in a file people read while deciding whether to retire D:\HT9050 is worse
#  than no reason at all.)
#
#  ⚠ aba09af is UNPUSHED. Its origin is D:\HT9050_git\root.git, which lives
#  OUTSIDE D:\HT9050, and `git -C D:\HT9050_git\root.git cat-file -t aba09af`
#  returns fatal. A bundle of the full history is preserved at
#  D:\HT9045\backup\HT9050_history_20260914\ (restore-tested, not just
#  `git bundle verify`d). See docs/LEAVE_TASK_PLAN.md §1.1 and §2.1.
#
#  So: the SCRIPT and its manifest are tracked, the PAYLOAD is not. Drift is
#  detected by re-running with --check, not by git.
#
#  ---------------------------------------------------------------------------
#  WHY THE COPY IS VERBATIM
#  ---------------------------------------------------------------------------
#  It is tempting to drop JSON/js/ -- 10.9 MB of JSONP shims that exist only so
#  the pages work under the file: protocol, where fetch() is CORS-blocked. Over
#  http they are pure duplication: 34 pairs, and --report parses both sides and
#  finds all 34 carry IDENTICAL DATA (they are not identical BYTES -- the .json
#  is pretty-printed and the .js minified, which is why the first version of
#  that check compared sizes and wrongly called 15 of them out of step).
#
#  They are copied anyway, because HT9045_Debug.cmd launches the UI as
#  file:///... today. Dropping them would silently break the only launch path
#  that currently exists. The duplication is reported by --report instead, so
#  the decision stays visible rather than being made here by omission.
#
#  ---------------------------------------------------------------------------
#  DELETION RULE
#  ---------------------------------------------------------------------------
#  "No stale pages" means files in web/ that the source does not have are
#  removed. Two guards, because this deletes:
#    1. Anything git TRACKS under web/ is never deleted; it aborts and names
#       the files. web/ is meant to be ignored, so a tracked file there means
#       someone's assumption is wrong and a script is the wrong place to
#       resolve that.
#    2. --check makes no changes at all and exits 1 on any difference.
#
#  Usage:
#    python sync_web.py --check      report drift, write nothing, exit 1 if any
#    python sync_web.py --apply      mirror source -> web/, delete stale
#    python sync_web.py --report     source composition only (size breakdown)
# ===========================================================================
import hashlib
import io
import json
import os
import re
import shutil
import subprocess
import sys

# The source path carries a space and CJK characters. It is a CONSTANT here on
# purpose: passing it through a shell argv on this machine (ACP 950) mangles
# it. Change it here, not on the command line.
# AI(W906-BA) 20260915: was D:\HT9050\docs\<CJK> HTML\HT9045, i.e. inside the
# tree being retired. Now an inbound landing area under D:\HT9045.
#
# This folder may legitimately NOT EXIST yet. The guard below (see main(),
# "source not found") aborts before the delete pass when SRC is not a
# directory, so a missing SRC is safe and self-explaining. An EMPTY but
# existing SRC would NOT be safe: it passes the guard and then everything
# in web/ that is neither git-tracked nor in OURS becomes a delete
# candidate. So do not pre-create this folder just to tidy things up --
# let the web author's next delivery create it.
# ⛔ AI(W906-WEBDRIFT) 20260922: **不要把 SRC 改指到 D:\HT9045_Client\web。**
#
#   那是 20260922 一次分析給出的建議，理由是「指到真的有東西的那個」。
#   它是錯的，而且錯得很貴。main():433 的刪除條件是
#       rel not in src_files and rel not in RUNTIME_OWNED and rel not in OURS
#   而實測 DEPLOY 722 檔 / D:\HT9045_Client\web 358 檔，排掉 IMG/ 與 JSON/
#   之後仍有 **85 個檔只存在於部署側**。改了 SRC 再跑一次 --apply，
#   那 85 個會全部被刪 —— 那是機台正在服務的 HMI，其中包含
#   ht9045_lotstart.js 與 ht9045_dialog_host.js（少了它們 START 會停在
#   "Please Enter LotID and Operator ID!!"、警報框關不掉）。
#
#   ⇒ SRC 維持指向一個**不存在**的目錄。不存在 = main() 的 "source not found"
#     提前 abort（exit 2）= 安全。這不是壞掉，是刻意的安全態。
#   ⇒ 漂移偵測改由 tools/websync/web_drift_check.py 負責（唯讀，零刪除路徑）。
#   ⇒ 使用者 20260922 已裁決走**方案 A**（讓 D:\HT9045\web 成為
#     ht9045_web.git 的 checkout）。A 落地後本檔與 web_drift_check.py 一起退休，
#     由 git 接手。
SRC = os.path.join("D:\\", "HT9045", "incoming", "web_html")
DST = os.path.join("D:\\", "HT9045", "web")
REPO = os.path.join("D:\\", "HT9045")
MANIFEST = os.path.join(
    REPO, "HT9011UC_Cpp_V3.33.906.0", "tools", "websync", "web_manifest.json")

# Not part of the runtime payload: repo metadata of the source workspace.
EXCLUDE_DIRS = {".github", ".git"}

# ---------------------------------------------------------------------------
#  RUNTIME-OWNED DOCUMENTS (user ruling 20260911)
#
#  These are written by whatever is feeding the HMI -- the JSON Simulator today,
#  the C++ runtime bridge later -- while the machine runs. The source copies in
#  the HT9050 workspace are SEED data, not truth. Mirroring them on every sync
#  would do two bad things:
#    1. overwrite live state with a stale seed (a shift's production count
#       replaced by the seed's zeros), and
#    2. make --check report drift forever, which trains everyone to ignore it.
#
#  So: copied when ABSENT (seeding a fresh deploy is right), never overwritten,
#  never deleted. Chosen by what the document IS, not by size -- each one's own
#  `runtime` block declares a pollIntervalMs (200 ms for Motor/IO, 2000 ms for
#  System), which is the tell: a document with a poll interval is a feed, not an
#  asset. 14.6 MB across these 7.
#
#  Their JSON/js/*.js JSONP twins are runtime-owned for the same reason: the
#  generator writes both sides, so re-seeding one and not the other would put
#  file:-mode and http-mode into disagreement -- exactly the failure --report
#  warns about.
RUNTIME_OWNED = {
    "JSON/Production-runtime.json",
    "JSON/Production-update.json",
    "JSON/Setup-current.json",
    "JSON/IO-runtime.json",
    "JSON/Motor-runtime.json",
    "JSON/System-runtime.json",
    "JSON/Task-runtime.json",
}
RUNTIME_OWNED |= set("JSON/js/%s.js" % p[len("JSON/"):-len(".json")]
                     for p in list(RUNTIME_OWNED))

# AI(W906-BA-C1CLIENT) 20260911: files THIS TOOL deploys, which the mirror
# source does not have.
#
# Without this they would be deletion candidates on the very next run -- the
# delete pass removes anything present in DST and absent from SRC -- so the
# client would be deleted and re-copied on every sync, churning the counts and
# making "deleted 0" stop meaning what it says.
#
# They are also excluded from the asset digest below, for the same reason the
# runtime documents are: that digest answers "is this the same MIRROR?", and
# folding our own deployed file into it would make the answer change when we
# ship a new client rather than when the mirror changes.
OURS = {
    "page/ht9045_recipe_client.js",
    "page/ht9045_wire_startcond.js",
    "page/ht9045_wire_contact.js",
    "page/ht9045_home_refresh.js",
    "page/ht9045_startcond_wire.js",
    "page/ht9045_yieldmon_wire.js",
    "page/ht9045_sckart_wire.js",
    "page/ht9045_testerif_wire.js",
    # AI(W906-FW-CONTACT-WIRE) 20260914: the Setup.Contact page wiring. Same
    # reasoning as the client above -- we deploy it, the mirror source does not
    # have it, so without this entry the delete pass would remove it on the next
    # sync and "deleted 0" would stop meaning anything.
    #
    # NOTE this does NOT protect the two <script> lines inside
    # page/Setup.Contact.html. That file DOES exist in the mirror source, so a
    # --apply overwrites it and the wiring goes dark with no error. The durable
    # fix is for the web author to carry those two lines in their own source;
    # until then, re-add them after every sync. See
    # D:\HT9045_Client\docs\REPLICATE.md.
    "page/ht9045_contact_wire.js",
    # AI(W906-BA-MERGE) 20260915：網頁同事 20260915 交回的接線檔。他那份 sync_web.py 把
    # 我們上面 5 筆從 OURS 刪掉了，我們這份原本也沒有他這 21 筆 —— 兩邊
    # 任一份跑 --apply 都會刪掉對方的檔。OURS 必須是聯集。
    # 注意：這同樣**不**保護各 Setup.*.html 裡的 <script> 行。
    "page/ht9045_hotplate_wire.js",
    "page/ht9045_wire_engine.js",
    "page/ht9045_wire_barcode.js",
    "page/ht9045_wire_cleaning.js",
    "page/ht9045_wire_configuration.js",
    "page/ht9045_wire_diointerfacecfg.js",
    "page/ht9045_wire_hwhandlersys.js",
    "page/ht9045_wire_hwiosetview.js",
    "page/ht9045_wire_hwmotortest.js",
    "page/ht9045_wire_hwteach.js",
    "page/ht9045_wire_lduld.js",
    # AI(W906-B25-OURS) 20261001: INBOX 102 (b) -- the wiring js St01 / St02 added 0925-1001 that the web pages
    #   in git load but OURS did not list (the next --apply would have deleted them). Measured with the
    #   check_deployed.py section-3 rule over the git web/page, not over the main checkout.
    "page/ht9045_binsel_wire.js",
    "page/ht9045_contactct_wire.js",
    "page/ht9045_counterclear_wire.js",
    "page/ht9045_lotinfo_wire.js",
    "page/ht9045_mainrecord_wire.js",
    "page/ht9045_observer_wire.js",
    "page/ht9045_offset_wire.js",
    "page/ht9045_setup_c_wire.js",
    "page/ht9045_showbinselect_wire.js",
    "page/ht9045_sortct_wire.js",
    "page/ht9045_testcategory_wire.js",
    "page/ht9045_testerif_c_wire.js",
    "page/ht9045_towerlight_wire.js",
    "page/ht9045_wire_dataobserver.js",
    "page/ht9045_wire_hwshuttlemove.js",
    "page/ht9045_wire_livesettings.js",
    "page/ht9045_wire_setupcontact.js",
    "page/ht9045_wire_setuphotplate.js",
    "page/ht9045_wire_setupoffset.js",
    "page/ht9045_wire_setupsckart.js",
    "page/ht9045_wire_setupsetup.js",
    "page/ht9045_wire_setupspeed.js",
    "page/ht9045_wire_setuptempset.js",
    "page/ht9045_wire_setuptesterif.js",
    "page/ht9045_wire_setuptrayassignment.js",
    "page/ht9045_wire_setupyieldmonitoring.js",
    "page/ht9045_wire_statusshowbinselect.js",
    # AI(W906-B27-OURS) 20261001: INBOX 102 (c) -- the NON-wiring helper js the web pages in git load (St01 / St02
    #   hand-written: *_c.js, *_ev.js, ht9045_busy_util.js ...; St02 MR !83 adds ht9045_dio_delete.js). The batch-25
    #   block above used check_deployed.py section 3's rule, which only matches wiring js, so these were never counted.
    #   Rule here: every src="ht9045_*.js" in web/page/*.html. Note :514-524 already aborts a run that would delete a
    #   tracked file; listing them here also covers a checkout older than the file (it is then untracked locally).
    "page/ht9045_agv_c.js",
    "page/ht9045_alarm_motionview.js",
    "page/ht9045_aoaoffset_c.js",
    "page/ht9045_barcode_c.js",
    "page/ht9045_barcode_ev.js",
    "page/ht9045_builder_web.js",
    "page/ht9045_busy_util.js",
    "page/ht9045_cleaning_c.js",
    "page/ht9045_cleaning_ev.js",
    "page/ht9045_config_q41.js",
    "page/ht9045_config_st01_ev.js",
    "page/ht9045_config_trayplate.js",
    "page/ht9045_contact_ev.js",
    "page/ht9045_contact_q41.js",
    "page/ht9045_contact_slk.js",
    "page/ht9045_countersel_c.js",
    "page/ht9045_dio_delete.js",
    "page/ht9045_ela_xlsx.js",
    "page/ht9045_groundman_c.js",
    "page/ht9045_hotplate_ev.js",
    "page/ht9045_hsys_heater_c.js",
    "page/ht9045_iniconfig_auth_c.js",
    "page/ht9045_iniconfig_p26_c.js",
    "page/ht9045_io_do.js",
    "page/ht9045_lduld_q41.js",
    "page/ht9045_main_close.js",
    "page/ht9045_main_st01_ev.js",
    "page/ht9045_menu_icons.js",
    "page/ht9045_mv_motor.js",
    "page/ht9045_mv_trays.js",
    "page/ht9045_nonstop_alarm.js",
    "page/ht9045_nonstop_page.js",
    "page/ht9045_offset_ev.js",  "page/ht9045_mt_gearratio.js",   # AI(W906-GEARRATIO) 20261002: HW.MotorTest.html loads it (the Motor Test Gear Ratio tab, RULINGS_20261002 #22; same line: line numbers kept)
    "page/ht9045_qamode_c.js",
    "page/ht9045_setup_cosflags.js",
    "page/ht9045_setup_sitemap.js",
    "page/ht9045_shuttlemove_c.js",
    "page/ht9045_smartdiag_web.js",
    "page/ht9045_speed_c.js",
    "page/ht9045_startcondition_c.js",  "page/ht9045_teach_armcell.js",  "page/ht9045_teach_st02.js",   # AI(W906-ARMCELL) 20261002: HW.teach.html loads it (the Teach Arm Cell tab, RULINGS_20261002 #18; same line: line numbers kept)   # AI(W906-ST02-C9) 20261002 (St02 claim): HW.teach.html:81 loads it (card ST02-C9 G2 / G1; same line, line numbers kept)
    "page/ht9045_temp_set_c.js",
    "page/ht9045_temp_set_ts1.js",
    "page/ht9045_trayassign_ev.js",
    "page/ht9045_trayedit.js",
    "page/ht9045_trayform_ev.js",
    "page/ht9045_trayform_q41.js",
    "page/ht9045_vacuumunit_c.js",
    "page/ht9045_yield_ev.js",
    "page/ht9045_yieldmon_q41.js",
    "page/ht9045_yieldmonitoring_c.js",
    "page/ht9045_wire_offset.js",
    "page/ht9045_wire_qamode.js",
    "page/ht9045_wire_sckart.js",
    "page/ht9045_wire_setup.js",
    "page/ht9045_wire_speed.js",
    "page/ht9045_wire_tempset.js",
    "page/ht9045_wire_testerif.js",
    "page/ht9045_wire_trayassignment.js",
    "page/ht9045_wire_trayform.js",
    "page/ht9045_wire_yieldmonitoring.js",  "page/ht9045_observer_ev.js",   # AI(W906-B43-OURS) 20261002: St01 q59 E-021 -- Data.Observer.html loads it (St01 1002 11:58 mail); own line, St02 LI-9 F1 appends to :297
    # AI(W906-B29-OURS) 20261002: St02 MR !89 (LI-12) -- Data.LotInfo.html now loads it.
    "page/ht9045_lotinfo_testertcp.js",  "page/ht9045_kb_generic.js",  "page/ht9045_lotinfo_ftp.js",  "page/ht9045_ftpclient.js",  "page/Data.FTPClient.html",   # AI(W906-B38-OURS) 20261002: St02 MR !97 (C-2) -- Main.CommView.html now loads ht9045_kb_generic.js (same line: line numbers kept)  AI(W906-LI9-F1) 20261002 (St02 claim): LI-9 F1 -- Data.LotInfo.html loads ht9045_lotinfo_ftp.js; the ftpclient window = Data.FTPClient.html + ht9045_ftpclient.js (same line)
    # AI(W906-WIRE-SYS) 20260917：Steven 0916-0917 新增／改名的 12 支。
    #   少一筆 = 下次 --apply 靜默刪掉線上那個檔（check_deployed.py 第 3 項就是
    #   在報這個）。本檔第 170 行自己寫著「OURS 必須是聯集」。
    #   ⚠ 其中 configconfiguration / configdiointerfacecfg 是改名後的新名字；
    #     舊名 ht9045_wire_configuration.js / _diointerfacecfg.js 上面還留著，
    #     **刻意不刪** —— 線上還有 Setup.Configuration.html / Setup.DIOInterFaceCFG.html
    #     兩個舊頁在引用它們。等確認舊頁可以退場再一起清。
    "page/ht9045_wire_alertnote.js",
    "page/ht9045_wire_configconfiguration.js",
    "page/ht9045_wire_configdiointerfacecfg.js",
    "page/ht9045_wire_databuilder.js",
    "page/ht9045_wire_datasmartdiagnostic.js",
    "page/ht9045_wire_datastartcondition.js",
    "page/ht9045_wire_hwmycclinksensor.js",
    "page/ht9045_wire_hwomronej1n.js",
    "page/ht9045_wire_main.js",
    "page/ht9045_wire_maincommview.js",
    "page/ht9045_wire_statusgroundman.js",
    "page/ht9045_wire_statussecurity.js",
    # AI(W906-Q27-LOTSTART) 20260921：控制面板的 Lot Start 區塊（我們寫的）。
    #   ⚠ 少這一筆的後果是**靜默的而且會咬人**：`web/` 在 .gitignore:239 裡，
    #     所以這個檔既不被 git 追蹤、（在這一筆之前）也不在 OURS ——
    #     本檔第 91 行的規則「既不在 git 也不在 OURS 就是刪除候選」兩個條件
    #     同時成立，下一次 --apply 會把它刪掉，而且 "deleted N" 看起來很正常。
    #   ★ 它刪掉的後果不只是少一個檔：WebStart.cpp 的 LotID 互鎖（Q29）
    #     已經解閘了，而這個檔是網頁上**唯一**送得出 `lot.start` 的入口。
    #     沒有它，START 會永遠停在 "Please Enter LotID and Operator ID!!"。
    #   ⚠ 這同樣**不**保護 page/Main.gbControlBtn.html 裡那一行 <script>。
    #     那個檔在同事的來源裡有，--apply 會整檔覆蓋、接線靜默失效。
    #     完整檔與 .patch 留在 HT9011UC_Cpp_V3.33.906.0/web-overlay/。
    "page/ht9045_lotstart.js",
    # AI(W906-Q30-8) 20260922：警報對話框的回應通道（我們寫的）。
    #   ⚠ 與 lotstart 同一個道理，但後果更嚴重：少這一筆、下次 --apply 刪掉它，
    #     `dialog-bridge.js` 的 submit() 就會落回第四段 reject
    #     ⇒ **警報框跳得出來、按得下去，但關不掉、C++ 永遠等** = hang up。
    #   ⚠ 這同樣**不**保護 background.html 裡那一行 <script>。
    #     那個檔在同事的來源裡有，--apply 會整檔覆蓋、接線靜默失效。
    #     完整檔與 .patch 留在 HT9011UC_Cpp_V3.33.906.0/web-overlay/。
    "page/ht9045_dialog_host.js",
    # AI(W906-OPBTN-ACK) 20260922：控制面板 START / PAUSE 的接線（我們寫的）。
    #   與 lotstart / dialog_host 完全同一個道理：`web/` 在 .gitignore:239，
    #   同事的來源裡也沒有同名檔 ⇒ 本檔第 91 行那兩個刪除條件同時成立，
    #   少這一筆的話下一次 --apply 會靜默刪掉它，"deleted N" 看起來很正常。
    #   ★ 後果：那 8 顆鈕本來就有一個 classList.toggle('on') 的 handler，
    #     所以 START 按下去**照樣變色**，只是一個 frame 都不送 ——
    #     「壞掉」與「正常」在畫面上長得一模一樣。
    #   ⚠ 這同樣**不**保護 page/Main.gbControlBtn.html 裡那一行 <script>；
    #     那個檔在同事的來源裡有，--apply 會整檔覆蓋。
    #     那一行由 reapply_overlay.py 的第 3 個 SCRIPT_JOB 負責補回。
    "page/ht9045_opbuttons.js",
    # AI(W906-IO-POINTS) 20260924: IO 表整張的即時狀態頁（/api/struct/io/config＋runtime）。
    # 只在我們這側、鏡像來源沒有 ⇒ 不列進來，下次 --apply 的刪除輪會把它刪掉。
    "page/IoLive.html",
}


def walk(root):
    """Relative paths of every file under root, excluding EXCLUDE_DIRS."""
    out = {}
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = sorted(d for d in dirnames if d not in EXCLUDE_DIRS)
        for fn in sorted(filenames):
            full = os.path.join(dirpath, fn)
            rel = os.path.relpath(full, root)
            try:
                out[rel.replace("\\", "/")] = os.path.getsize(full)
            except OSError as exc:
                print("  !! cannot stat %s: %s" % (rel, exc))
    return out


def sha256(path, chunk=1 << 20):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        while True:
            b = fh.read(chunk)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def tracked_under_web():
    """Files git tracks under web/. Deleting one would be a real loss."""
    try:
        out = subprocess.check_output(
            ["git", "-C", REPO, "ls-files", "web/"],
            stderr=subprocess.STDOUT)
    except Exception as exc:  # noqa: BLE001
        print("  !! git ls-files failed (%s); refusing to delete anything" % exc)
        return None
    return [l.strip() for l in out.decode("utf-8", "replace").splitlines() if l.strip()]


def human(n):
    for unit in ("B", "KB", "MB", "GB"):
        if n < 1024 or unit == "GB":
            return "%.1f %s" % (n, unit)
        n /= 1024.0


def report(src_files):
    print("=" * 78)
    print("source composition: %s" % SRC)
    print("=" * 78)
    total = sum(src_files.values())
    print("files %d   total %s" % (len(src_files), human(total)))
    by_top = {}
    for rel, size in src_files.items():
        top = rel.split("/")[0] if "/" in rel else "(root)"
        e = by_top.setdefault(top, [0, 0])
        e[0] += 1
        e[1] += size
    print()
    print("%-24s %7s %12s" % ("subtree", "files", "bytes"))
    for top, (n, size) in sorted(by_top.items(), key=lambda t: -t[1][1]):
        print("%-24s %7d %12s" % (top, n, human(size)))
    print()
    print("largest 10:")
    for rel, size in sorted(src_files.items(), key=lambda t: -t[1])[:10]:
        print("  %10s  %s" % (human(size), rel))

    twin_report(src_files)


# ---------------------------------------------------------------------------
#  The JSON/js twin analysis.
#
#  The first version of this compared FILE SIZES and reported 15 pairs as
#  "already out of step". That was wrong, and wrong in the direction that
#  invents work: JSON/*.json is pretty-printed and JSON/js/*.js is minified, so
#  a 4x size difference is the NORMAL state, not drift. Checked by parsing both
#  sides: 34 of 34 pairs carry identical data.
#
#  Size cannot answer this question, so this compares parsed values. Extracting
#  the shim's payload has one trap of its own -- line 1 is
#  `window.__HT9045_DATA__=window.__HT9045_DATA__||{};`, and taking the first
#  brace grabs THAT empty object and reports every pair as different. Anchor on
#  the keyed assignment, and accept either quote style: Production-update.js
#  uses ' where the rest use ".
# ---------------------------------------------------------------------------
def _shim_payload(path):
    txt = io.open(path, encoding="utf-8-sig", errors="replace").read()
    m = re.search(r"""__HT9045_DATA__\s*\[\s*['"][^'"]*['"]\s*\]\s*=\s*""", txt)
    if not m:
        return None, "no __HT9045_DATA__[...] assignment"
    i = m.end()
    if i >= len(txt) or txt[i] not in "{[":
        return None, "assignment RHS is not an object/array"
    depth, instr, esc = 0, False, False
    for k in range(i, len(txt)):
        c = txt[k]
        if instr:
            if esc:
                esc = False
            elif c == "\\":
                esc = True
            elif c == '"':
                instr = False
            continue
        if c == '"':
            instr = True
        elif c in "{[":
            depth += 1
        elif c in "}]":
            depth -= 1
            if depth == 0:
                try:
                    return json.loads(txt[i:k + 1]), None
                except Exception as exc:  # noqa: BLE001
                    return None, "parse: %r" % (exc,)
    return None, "unbalanced"


def twin_report(src_files):
    pairs = []
    for rel in sorted(src_files):
        if not rel.startswith("JSON/js/") or not rel.endswith(".js"):
            continue
        name = rel[len("JSON/js/"):-len(".js")]
        peer = "JSON/%s.json" % name
        if peer in src_files:
            pairs.append((name, peer, rel))

    same, differ, unreadable = 0, [], []
    for name, peer, rel in pairs:
        try:
            canon = json.load(io.open(os.path.join(SRC, *peer.split("/")),
                                      encoding="utf-8-sig"))
        except Exception as exc:  # noqa: BLE001
            unreadable.append((name, "json: %r" % (exc,)))
            continue
        shim, err = _shim_payload(os.path.join(SRC, *rel.split("/")))
        if err:
            unreadable.append((name, "shim: %s" % err))
        elif shim == canon:
            same += 1
        else:
            differ.append(name)

    twin_bytes = sum(src_files[r] for _n, _p, r in pairs)
    print()
    print("JSON/js JSONP shims paired with a JSON/*.json twin: %d pairs, %s of shims"
          % (len(pairs), human(twin_bytes)))
    print("  They exist only for the file: protocol, where fetch() is CORS-blocked.")
    print("  Over http they are pure duplication.")
    print("  PARSED-VALUE comparison (not size -- size is misleading here,")
    print("  .json is pretty-printed and .js is minified):")
    print("    identical data : %d" % same)
    print("    differing      : %d%s" % (len(differ),
                                         ("  " + ", ".join(differ)) if differ else ""))
    print("    unreadable     : %d" % len(unreadable))
    for n, why in unreadable:
        print("      %-28s %s" % (n, why))
    print("  The shims carry `// Generated by HT9045 JSON Simulator`, so ONE")
    print("  generator writes both sides today and they stay in step by")
    print("  construction. The risk is not present drift; it is that whoever")
    print("  replaces the simulator (the C++ runtime bridge named as `owner` in")
    print("  these documents) must write BOTH, or file:-mode goes stale silently.")


def main(argv):
    mode = argv[1] if len(argv) > 1 else ""
    if mode not in ("--check", "--apply", "--report"):
        print(__doc__ or "usage: sync_web.py --check | --apply | --report")
        return 2
    if not os.path.isdir(SRC):
        print("sync_web: source not found: %s" % SRC)
        return 2

    src_files = walk(SRC)
    if mode == "--report":
        report(src_files)
        return 0

    dst_files = walk(DST) if os.path.isdir(DST) else {}

    to_copy, to_delete, held = [], [], []
    for rel, size in sorted(src_files.items()):
        d = os.path.join(DST, *rel.split("/"))
        if rel not in dst_files:
            # Seeding an absent runtime document is right; overwriting one is not.
            to_copy.append((rel, "seed" if rel in RUNTIME_OWNED else "new"))
        elif rel in RUNTIME_OWNED:
            if dst_files[rel] != size:
                held.append(rel)
        elif dst_files[rel] != size:
            to_copy.append((rel, "size"))
        else:
            s = os.path.join(SRC, *rel.split("/"))
            if abs(os.path.getmtime(s) - os.path.getmtime(d)) > 2:
                to_copy.append((rel, "mtime"))
    for rel in sorted(dst_files):
        if rel not in src_files and rel not in RUNTIME_OWNED and rel not in OURS:
            to_delete.append(rel)

    print("=" * 78)
    print("sync_web  %s" % mode)
    print("  source %s   (%d files, %s)" % (SRC, len(src_files), human(sum(src_files.values()))))
    print("  dest   %s   (%d files)" % (DST, len(dst_files)))
    print("=" * 78)
    print("to copy   : %d" % len(to_copy))
    print("to delete : %d" % len(to_delete))
    if held:
        print("held      : %d runtime-owned document(s) differ and are LEFT ALONE"
              % len(held))
        for rel in held:
            print("    %-42s dest %10s   source %10s"
                  % (rel, human(dst_files[rel]), human(src_files[rel])))
        print("    (these are feeds, not assets -- the machine owns them; see")
        print("     RUNTIME_OWNED. Differing here is the NORMAL state once")
        print("     anything has run, and is not drift.)")

    tracked = tracked_under_web()
    if tracked is None:
        return 2
    collisions = sorted(set("web/" + r for r in to_delete) & set(tracked))
    if collisions:
        print()
        print("ABORT: %d file(s) marked for deletion are TRACKED by git:" % len(collisions))
        for c in collisions[:20]:
            print("  " + c)
        print("web/ is meant to be a gitignored deploy target. A tracked file")
        print("there means an assumption is wrong; resolve it before syncing.")
        return 2
    if tracked:
        print()
        print("NOTE: git still tracks %d file(s) under web/. They are not being"
              % len(tracked))
        print("      deleted, but they will be overwritten in place if the source")
        print("      provides the same path -- so a tracked file here can change")
        print("      under git without anyone editing it. web/ should be ignored.")
        for t in tracked[:10]:
            print("  " + t)

    if mode == "--check":
        for rel, why in to_copy[:30]:
            print("  COPY   (%s) %s" % (why, rel))
        if len(to_copy) > 30:
            print("  ... and %d more" % (len(to_copy) - 30))
        for rel in to_delete[:30]:
            print("  DELETE %s" % rel)
        if len(to_delete) > 30:
            print("  ... and %d more" % (len(to_delete) - 30))
        drift = len(to_copy) + len(to_delete)
        print()
        print("IN SYNC" if not drift else "DRIFT: %d difference(s)" % drift)
        return 1 if drift else 0

    copied = 0
    for rel, _why in to_copy:
        s = os.path.join(SRC, *rel.split("/"))
        d = os.path.join(DST, *rel.split("/"))
        os.makedirs(os.path.dirname(d), exist_ok=True)
        shutil.copy2(s, d)          # verbatim bytes + mtime; no text translation
        copied += 1
    deleted = 0
    for rel in to_delete:
        os.remove(os.path.join(DST, *rel.split("/")))
        deleted += 1

    # AI(W906-BA-C1CLIENT) 20260911: deploy OUR side's recipe client alongside
    # the mirror.
    #
    # It lives in this tool's own directory rather than in the HMI source,
    # because the HMI source is a one-way mirror: anything written into DST that
    # SRC does not have is a deletion candidate on the next run, and anything
    # written into SRC is outside this repo's control. Copying it here, after the
    # delete pass, makes it survive every re-sync and keeps the file itself in
    # git where it can be reviewed.
    #
    # Deliberately NOT injected into background.html. The pages belong to their
    # author; a <script> tag this tool silently re-inserted on every sync would
    # be a surprise in someone else's file. He adds the one line when he wires a
    # page -- see docs/BA_MIGRATION_PLAN.md section 6.
    client_src = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                              "ht9045_recipe_client.js")
    client_dst = os.path.join(DST, "page", "ht9045_recipe_client.js")
    client_note = "absent"
    if os.path.isfile(client_src):
        os.makedirs(os.path.dirname(client_dst), exist_ok=True)
        same = (os.path.isfile(client_dst)
                and sha256(client_src) == sha256(client_dst))
        if not same:
            shutil.copy2(client_src, client_dst)
            client_note = "deployed"
        else:
            client_note = "up to date"
    # prune directories the deletions emptied
    for dirpath, dirnames, filenames in os.walk(DST, topdown=False):
        if dirpath != DST and not dirnames and not filenames:
            os.rmdir(dirpath)

    final = walk(DST)
    # The digest covers ASSETS ONLY. Including the runtime-owned documents would
    # make this tracked manifest change every time the machine writes -- so
    # `git status` would go dirty from running the plant, not from deploying
    # anything, and the one number meant to identify a deployment would stop
    # identifying it. Caught by testing the hold: a simulated machine write
    # moved the digest while `copied 0, deleted 0`.
    assets = dict((k, v) for k, v in final.items()
                  if k not in RUNTIME_OWNED and k not in OURS)
    man = {
        "generator": "tools/websync/sync_web.py",
        "source": SRC,
        "dest": DST,
        "files": len(final),
        "bytes": sum(final.values()),
        "asset_files": len(assets),
        "asset_bytes": sum(assets.values()),
        "runtime_owned": sorted(RUNTIME_OWNED),
        "copied_this_run": copied,
        "deleted_this_run": deleted,
        # An identity for the deployed ASSETS that survives the fact that
        # nothing here is in git: same digest == same asset bytes everywhere.
        "asset_sha256": hashlib.sha256(
            "\n".join("%s %d" % (k, v) for k, v in sorted(assets.items()))
            .encode("utf-8")).hexdigest(),
        "entry_point": "background.html",
    }
    os.makedirs(os.path.dirname(MANIFEST), exist_ok=True)
    io.open(MANIFEST, "w", encoding="utf-8", newline="\n").write(
        json.dumps(man, ensure_ascii=False, indent=1) + "\n")

    print()
    print("copied %d, deleted %d" % (copied, deleted))
    print("dest now %d files, %s" % (man["files"], human(man["bytes"])))
    print("asset sha256 %s  (%d asset files, %s)"
          % (man["asset_sha256"][:16], man["asset_files"], human(man["asset_bytes"])))
    print("manifest %s" % MANIFEST)
    entry = os.path.join(DST, "background.html")
    print("entry point %s  %s" % (entry, "OK" if os.path.isfile(entry) else "MISSING"))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
