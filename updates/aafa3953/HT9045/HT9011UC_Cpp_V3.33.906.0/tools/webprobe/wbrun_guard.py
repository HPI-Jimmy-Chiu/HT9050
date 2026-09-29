#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
wbrun_guard.py -- Q50 option B: back up the real machine folders, run the SIM wb_serve with the web probes,
then diff, restore and verify every file.        AI(W906-Q50-GUARD) 20260928, St01-owned tool, NOT in golden.

README
======
Ruling    Steven 20260928 「Q50 你可以在晚上的時候做」 = option B (decisions-pending Q50).
Spec      D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md
          §2 what a run writes, §4 B-1 backup scope, §6 the four rounds, §6.7 / §7 what "clean" and "done" mean.
Precedent tools\flowcmp\flow_run.py :216-246 (baseline + private copies) and :360-424 (restore).  Added here:
          changed / appended log files are restored too (flow_run only removed new ones), a foreign wb_serve.exe
          aborts the round, a marker file + --restore-only recovers after a crash, round 1 stops on unexpected files.

One round
  1. (once per invocation) preflight -> copy + SHA256 every file under ROOTS into <run>\bk\ -> marker file
  2. wb_serve --seconds N --port 8046 --root <run>\web, env cleaned (every W906_* / HT9045_* removed), then
     HT9045_TESTERCOMM=0 HT9045_ELA=0 W906_NO_BROWSER_WAKE=1 (+ W906_PWBOOK_PATH, W906_LEVELSET_PATH from r2 on)
  3. wait for the port; GET /api/struct/machine.defines?all=1 must say SOFT_SIMULTE on; run the round's probes one
     after another, each with a timeout; every 5 s look for a foreign wb_serve / machine-file writer / ctest -> abort
  4. wb_serve stops by itself when --seconds expires: the only path that runs the golden FormClose saves
     (Ctrl-C / Ctrl-Break only write Program Close=1 -- tools\wb_serve.cpp W906_ConsoleCtrl; there is no WS
     command for a normal close, see act.main.closeProgram "存完不關站")
  5. rescan + SHA256 -> changed / new / deleted (an appended log shows as changed, with the appended bytes);
     copies of what the run wrote go to <run>\r\<id>\ev\ (evidence), text diffs to <run>\r\<id>\diffs\;
     restore changed + deleted from bk, delete new files and new folders; rescan: every SHA256 = baseline?
     -> prints "ROUND <id> RESULT: CLEAN" or "... NOT-CLEAN"; report.json + report.md per round
  6. r1 only: every change must match EXPECTED_BOOT (the spec's §6.2 list) -> otherwise STOP (exit 4)

Rounds  r1 boot only | r2 six read-only probes, one boot | r3a..r3k write probes, one boot each |
        r4a regression read-only probes, one boot | r4b..r4g regression writers, one boot each (--dry-plan lists all)

Commands (PY = C:\Users\steven\AppData\Local\Programs\Python\Python314\python.exe, G = this file, EXE = wb_serve.exe)
  PY G --dry-plan --all --wb-serve EXE           print everything it would do; writes nothing
  PY G --round 1 --wb-serve EXE --label <commit> one round; also --round 2 / 3 / 4 / 3c (one sub-round) / --all
  PY G --restore-only D:\AI_TempFile\q50-run\<ts>   emergency: put every file back from that backup, verify
  PY G --verify-only  D:\AI_TempFile\q50-run\<ts>   read-only: is every file still equal to that backup?
  PY G --drop-backup  D:\AI_TempFile\q50-run\<ts>   verify, then delete that run's bk\ web\ bin\ (reports stay)

Markers (all in D:\AI_TempFile)
  wbrun_ACTIVE.json  written by this guard (exclusive create) before the backup, deleted only when every file is
                     verified back.  While it exists do nothing else (no build, no ctest, no F5); if no guard is
                     running, run --restore-only <the backup it names>.  Gate / proxy-build tooling should check it.
  GATE_ACTIVE* / BUILD_ACTIVE*   agreed marker for gates and proxy builds: create before, delete after; the guard
                     refuses while one exists.  Independently it refuses while a st01-*-gate.log / st02-*-build.log
                     written in the last --gate-log-window minutes (default 360) lacks its "=== gate done" /
                     "=== ctest exit ... done" line, or while ctest / cc1plus / ninja / cmake / make / g++ / cl run.
  q50-run\r1_result_<exe sha8>.json  round 1 verdict for that exe; rounds 2-4 refuse unless it says pass.

Exit codes  0 all CLEAN and every probe passed | 6 CLEAN, a probe failed / timed out / was skipped |
  7 files CLEAN but residue outside the backup set (recycle bin entry, new top-level entry) needs a hand |
  3 aborted (foreign process, not SIM, boot failed, Ctrl-C) and CLEAN | 4 round 1 found an unexpected file (CLEAN) |
  5 NOT-CLEAN: marker kept, run --restore-only | 2 refused before touching anything | 1 usage / internal error

Test mode (never touches D:\): --root R maps every D:\... path to R\D\...; --probe-dir DIR; --test-skip-build-check;
  a --wb-serve ending in .py is started with this Python (a fake server).
Never: runs git; writes anywhere except the ROOTS it restores, its own run folder and the marker; deletes anything
  outside ROOTS (recycle-bin and top-level residue is only reported).
"""
import argparse
import csv
import datetime
import difflib
import fnmatch
import glob
import hashlib
import json
import os
import re
import shlex
import shutil
import signal
import socket
import stat
import subprocess
import sys
import time
import traceback
import urllib.request

for _s in (sys.stdout, sys.stderr):
    try:
        _s.reconfigure(encoding='utf-8', errors='replace')
    except (AttributeError, ValueError):
        pass

HERE = os.path.dirname(os.path.abspath(__file__))
PY = sys.executable
SPEC = r'D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md'
AI_TEMP = r'D:\AI_TempFile'
MARKER = AI_TEMP + r'\wbrun_ACTIVE.json'
RUN_BASE = AI_TEMP + r'\q50-run'
WEB_SRC = r'D:\HT9045\web'
RECYCLE = 'D:\\$RECYCLE.BIN'
TOPLEVEL = ['D:\\', r'D:\HT9045']

# ------------------------------------------------------------------------------------------------ what gets backed up
# (real path, kind, class).  Spec §4 B-1 "備份範圍" (wbserve-sandbox-run.md lines 254-259) and §2.5 line 198.
ROOTS = [
    # whole folder + per-file SHA256, restored after every round (§4 B-1 first bullet)
    (r'D:\HT9045\system', 'dir', 'machine'),
    (r'D:\HT9045\config', 'dir', 'machine'),
    (r'D:\HT9045\IniData', 'dir', 'machine'),
    (r'D:\HT9045\Error', 'dir', 'machine'),
    (r'D:\HT9045\MDB', 'dir', 'machine'),
    (r'D:\HT9045\CFG', 'dir', 'machine'),
    (r'D:\HT9045\SECS', 'dir', 'machine'),
    (r'D:\HT9045\PMAlarm', 'dir', 'machine'),
    (r'D:\HT9045\setup.inf', 'file', 'machine'),
    (r'D:\HT9045\CurrentSetupData.txt', 'file', 'machine'),
    (r'D:\GPIB9045\system', 'dir', 'machine'),
    (r'D:\RS232Standard\system', 'dir', 'machine'),
    (r'D:\UnloaderInfo', 'dir', 'machine'),
    # log folders (§4 B-1 second bullet): new files copied to evidence then removed; changed / appended restored
    (r'D:\HT9045_Log', 'dir', 'log'),
    (r'D:\GPIBLOG', 'dir', 'log'),
    (r'D:\RS232Log', 'dir', 'log'),
    (r'D:\HandlerLog', 'dir', 'log'),
    (r'D:\SECS_GEM_LOGS', 'dir', 'log'),
    (r'D:\RMS', 'dir', 'log'),
    (r'D:\SaveRecord', 'dir', 'log'),
    (r'D:\PrecautionRecord', 'dir', 'log'),
    # absent on this machine (§2.5 line 198: 跑完若出現＝這次跑出來的) -- if a run creates one, it goes too
    (r'D:\HT9045_StateRecord', 'dir', 'absent-ok'),
    (r'D:\HT9045_Backup', 'dir', 'absent-ok'),
    (r'D:\AutoCleanLogs', 'dir', 'absent-ok'),
    # guard's own addition: wb_serve runs with --root <copy>, so the real mailbox must not move (§3 row 5)
    (r'D:\HT9045\web\JSON\runtime', 'dir', 'canary'),
]

# ------------------------------------------------------------------------------------ round 1: expected boot changes
# Spec §6.2 (wbserve-sandbox-run.md lines 341-342): 「§2.2 表中標「開機」「正常關站」的那幾列（Gerneral.ini、teach.ini、
# lastdata 三檔、config.ini、ContactInfo.ini、machinerecord、Arm* 18 檔、MachineLife.ini、LotSummary.csv、RunMode.txt、
# BootLog.txt、AlarmCodeList.txt、D:\HT9045_Log 的 JamRate_Daily／MDB_UpdateLog／SiteUseMgr）」; the file names are the
# §2.2 rows (lines 131-154) and the laptop measurement docs\NIGHT_REPORT.md line 204 (row 47).  Deliberately NOT on
# the list: D:\GPIB9045 / D:\GPIBLOG (row 47 saw them, but HT9045_TESTERCOMM=0 keeps the engine off -- a change
# there means the switch did not take), Error\English\JAM0000.dat (HT9045_ELA=0), web\JSON\runtime (--root copy).
# Pattern = fnmatch on the lower-cased real path; kinds = change kinds that are allowed.
_FILE = 'changed new'
_TREE = 'changed new dirnew'
EXPECTED_BOOT = [
    (r'd:\ht9045\system\gerneral.ini', _FILE, '§2.2 Gerneral.ini: missing keys; Program Close 0 at serve start, 1 at normal close'),
    (r'd:\ht9045\system\teach.ini', _FILE, '§2.2 teach.ini: contact-height values, bAOAMatrix, blank lines'),
    (r'd:\ht9045\system\lastdata.dat', _FILE, '§2.2 lastdata three files (WriteLastDataFile)'),
    (r'd:\ht9045\system\lastdata_backup.dat', _FILE, '§2.2 lastdata three files'),
    (r'd:\ht9045\system\lastdata_backup2.dat', _FILE, '§2.2 lastdata three files'),
    (r'd:\ht9045\config\config.ini', _FILE, '§2.2 config.ini: WriteLastDataFile always writes it'),
    (r'd:\ht9045\system\contactinfo.ini', _FILE, '§2.2 ContactInfo.ini: 128 keys on this machine (725038a6)'),
    (r'd:\ht9045\system\machinerecord.dat', _FILE, '§2.2 machinerecord written back at boot'),
    (r'd:\ht9045\system\machinerecordrealccd.dat', _FILE, '§2.2 machinerecordRealCCD'),
    (r'd:\ht9045\system\arm[012].dat', _FILE, '§2.2 Arm* 18 files (normal close WriteCTInfo)'),
    (r'd:\ht9045\system\arm[012]_backup.dat', _FILE, '§2.2 Arm* 18 files'),
    (r'd:\ht9045\system\armhis[012].dat', _FILE, '§2.2 Arm* 18 files'),
    (r'd:\ht9045\system\armhis[012]_backup.dat', _FILE, '§2.2 Arm* 18 files'),
    (r'd:\ht9045\system\armbylot[012].dat', _FILE, '§2.2 Arm* 18 files'),
    (r'd:\ht9045\system\armbylot[012]_backup.dat', _FILE, '§2.2 Arm* 18 files'),
    (r'd:\ht9045\system\machinelife.ini', _FILE, '§2.2 MachineLife.ini: missing keys'),
    (r'd:\ht9045\system\lotsummary.csv', _FILE, '§2.2 LotSummary.csv: normal close rewrites it'),
    (r'd:\ht9045\system\runmode.txt', _FILE, '§2.2 RunMode.txt: normal close'),
    (r'd:\ht9045\error\bootlog.txt', _FILE, '§2.2 BootLog.txt: one line per boot'),
    (r'd:\ht9045\error\alarmcodelist.txt', _FILE, '§2.2 AlarmCodeList.txt: saved after the alarm catalogue loads'),
    (r'd:\ht9045_log\jamrate_daily', 'dirnew', '§2.2 D:\\HT9045_Log JamRate_Daily (normal close)'),
    (r'd:\ht9045_log\jamrate_daily\*', _TREE, '§2.2 D:\\HT9045_Log JamRate_Daily'),
    (r'd:\ht9045_log\mdb_updatelog', 'dirnew', '§2.2 D:\\HT9045_Log MDB_UpdateLog (boot)'),
    (r'd:\ht9045_log\mdb_updatelog\*', _TREE, '§2.2 D:\\HT9045_Log MDB_UpdateLog'),
    (r'd:\ht9045_log\siteusemgr', 'dirnew', '§2.2 D:\\HT9045_Log SiteUseMgr (hourly file)'),
    (r'd:\ht9045_log\siteusemgr\*', _TREE, '§2.2 D:\\HT9045_Log SiteUseMgr'),
]

# ----------------------------------------------------------------------------------------------------- probe table
# name -> script, args (placeholders below), timeout (hard, s), budget (s, sizes --seconds), declared writes
# (patterns as above: what spec §1.3 / §2.2 says the probe itself may write), spec citation.
# Placeholders: {port} {user} {password} {console} {levelset} {pwbook} {D} (= D:\ or the test root's D\) {run}
#               {baseline_sha} (sha256sum list of D:\HT9045 before the run) {offsets} (--offsets-file)
_U = ['--user', '{user}', '--password', '{password}']
_ANY = 'changed new deleted dirnew dirgone'
_RCP = r'd:\ht9045\inidata\data\*'
_LAST = r'd:\ht9045\system\lastdata*.dat'


def _P(script, args, timeout, budget, declares, cite, needs=()):
    return {'script': script, 'args': args, 'timeout': timeout, 'budget': budget,
            'declares': [(p, _ANY, 'declared: ' + cite) for p in declares], 'cite': cite, 'needs': list(needs)}


PROBES = {
    # ---- r2: read-only probes, spec §6.3 items 1-6 (exact arguments from there)
    'q3_dio_owner': _P('q3_dio_owner_probe.py', ['--port', '{port}'], 180, 90, [], '§6.3 #1'),
    'q41_open_gate': _P('q41_open_gate_probe.py', ['--port', '{port}', '--levelset', '{levelset}', '--no-open'],
                        300, 120, [], '§6.3 #2'),
    'contactct_live': _P('data_contactct_live_probe.py', ['--port', '{port}'] + _U + ['--serve-log', '{console}'],
                         600, 300, [], '§6.3 #3'),
    'observer_token': _P('data_observer_token_probe.py', ['--port', '{port}'] + _U, 600, 300, [], '§6.3 #4 (no --mutate)'),
    'formevent': _P('formevent_probe.py', ['--port', '{port}'] + _U, 300, 150,
                    [r'd:\ht9045\inidata\data\*\hotplate.data', r'd:\ht9045\inidata\data\*\handlercondition.data'],
                    '§6.3 #5 (opening may re-write recipe HotPlate.Data / HandlerCondition.Data, §1.3)'),
    'formevent_cc': _P('formevent_cc_probe.py', ['--port', '{port}'] + _U, 300, 150, [], '§6.3 #6 (no --allow-save)'),
    # ---- r3: write probes, one boot each, spec §6.4
    'formevent_ta_ts': _P('formevent_ta_ts_probe.py', ['--port', '{port}'] + _U, 300, 150,
                          [r'd:\ht9045\inidata\data\*\tray.data', r'd:\ht9045\inidata\data\*\temperature.data',
                           r'd:\ht9045\inidata\data\*\tester.data'], '§6.4 #1 without --allow-save (fills missing keys)'),
    'formevent_ta_ts_save': _P('formevent_ta_ts_probe.py', ['--port', '{port}'] + _U + ['--allow-save'], 300, 150,
                               [_RCP, r'd:\ht9045\inidata\definetemp\*', r'd:\ht9045\config\atc.ini'],
                               '§6.4 #1 with --allow-save (Tray/Temperature/Tester.Data, DefineTemp, ATC.ini)'),
    'formevent_cc_save': _P('formevent_cc_probe.py', ['--port', '{port}'] + _U + ['--allow-save'], 300, 150,
                            [r'd:\ht9045\system\gerneral.ini'],
                            '§6.4 #1 formevent_cc --allow-save (answers NO; close fills Gerneral.ini keys)'),
    'closetail_s1': _P('q41_closetail_probe.py', ['--port', '{port}', '--console', '{console}'] + _U, 300, 150, [],
                       '§6.4 #2 S1 only (no --save)'),
    'closetail_save': _P('q41_closetail_probe.py', ['--port', '{port}', '--console', '{console}'] + _U + ['--save'],
                         300, 150, [_RCP, r'd:\ht9045\system\runmode.txt', _LAST, r'd:\ht9045\system\gerneral.ini'],
                         '§6.4 #2 --save (UdUld/ArmCondition/Binasgn*/Tray/Tester.Data, RunMode.txt, lastdata, [Shuttle])'),
    'closetail_hotplate': _P('q41_closetail_probe.py',
                             ['--port', '{port}', '--console', '{console}'] + _U + ['--hotplate'], 300, 150,
                             [_RCP, r'd:\ht9045\system\runmode.txt', _LAST, r'd:\ht9045\system\gerneral.ini'],
                             '§6.4 #2 --hotplate (HotPlate.Data, HandlerCondition.Data); --config is NOT run (§9)'),
    'countersel_write': _P('status_countersel_probe.py',
                           ['--port', '{port}', '--write', '--file', '{D}HT9045\\config\\config.ini', '--edit', 'cbUPH'] + _U,
                           600, 300, [r'd:\ht9045\config\config.ini'], '§6.4 #3 (+ test password book login)'),
    'smartdiag': _P('data_smartdiag_probe.py', ['--port', '{port}'] + _U, 600, 300,
                    [r'd:\ht9045\system\smartdiagnostic*'], '§6.4 #4 / §1.3 group 2'),
    'builder': _P('data_builder_probe.py', ['--port', '{port}'] + _U, 600, 300,
                  [r'd:\ht9045\inidata\data\w906prb*', r'd:\ht9045\inidata\data\w906imp*',
                   r'd:\ht9045\inidata\offset\w906prb*', r'd:\ht9045\inidata\offset\w906imp*'],
                  '§6.4 #4 (its deletes go to the recycle bin -> reported as residue)'),
    'sortct': _P('data_sortct_probe.py', ['--port', '{port}'] + _U, 900, 420,
                 [_LAST, r'd:\ht9045_log\qtydata\*', r'd:\ht9045_log\qtydata'], '§6.4 #4 (calls C:\\MinGW\\bin\\g++.exe)'),
    's12_form_write': _P('s12_form_probe.py',
                         ['--port', '{port}', '--page', 'Setup.HotPlate.html', '--write', 'XST1=13.360'] + _U, 600, 300,
                         [_RCP], '§6.4 #4 "--write ..." -- args from the probe usage (s12_form_probe.py lines 19-21)'),
    # ---- r4: regression (spec §1.3 group 3, §6.5)
    'contactct': _P('data_contactct_probe.py', ['--port', '{port}'] + _U + ['--baseline', '{baseline_sha}'], 600, 240,
                    [], '§1.3 group 3, read-only; --baseline = the guard\'s pre-run sha256sum list'),
    'observer': _P('data_observer_probe.py', ['--port', '{port}'] + _U, 600, 300, [], '§1.3 group 3, read-only'),
    'testcategory': _P('data_testcategory_probe.py', ['--port', '{port}'], 600, 240, [], '§1.3 group 3, read-only'),
    's1_eventlog': _P('s1_eventlog_probe.py', ['--port', '{port}'], 300, 120, [], '§1.3 group 3 (default port 8192!)'),
    's7_thermo': _P('s7_thermo_probe.py', ['--port', '{port}'], 300, 120, [], '§1.3 group 3 (default port 8192!)'),
    's12c_config': _P('s12c_config_probe.py', ['--port', '{port}'] + _U, 600, 240, [], '§1.3 group 3, no --write'),
    's12c_offset': _P('s12c_offset_probe.py', ['--port', '{port}'] + _U, 600, 240, [], '§1.3 group 3, no --write'),
    's12c_page': _P('s12c_page_probe.py', ['--port', '{port}', '--page', 'Setup.Ld_ULd.html', '--struct', 'Ld_UldDelayTime'] + _U,
                    600, 240, [], '§1.3 group 3, no --write; page/struct from the probe usage line 17'),
    'counterclear': _P('data_counterclear_probe.py', ['--port', '{port}'] + _U, 600, 240, [], '§1.3 group 3, no --write'),
    'lotinfo': _P('data_lotinfo_probe.py', ['--port', '{port}'] + _U, 600, 300,
                  [r'd:\ht9045\config\config.ini', r'd:\ht9045\system\armbylot*', r'd:\ht9045\config\security_new.def'],
                  '§1.3 group 3; default S step sends lot.start (writes [Lot Info], ArmByLot*.dat)'),
    'showmymessage': _P('showmymessage_probe.py', ['--port', '{port}'], 900, 420, [],
                        '§1.3 group 3 (partly run before); holds the operator token -> own boot'),
    'towerlight': _P('status_towerlight_probe.py', ['--port', '{port}'] + _U + ['--offsets', '{offsets}'], 600, 300,
                     [_LAST, r'd:\ht9045\config\config.ini'], '§1.3 group 3; needs the MinGW offset list (--offsets-file)',
                     needs=['offsets']),
    's12c_config_write': _P('s12c_config_probe.py', ['--port', '{port}', '--write'] + _U, 600, 240,
                            [r'd:\ht9045\config\config.ini', _LAST], '§1.3 group 3 with --write'),
    's12c_offset_write': _P('s12c_offset_probe.py', ['--port', '{port}', '--write'] + _U, 600, 240,
                            [r'd:\ht9045\inidata\defineoffset\*'], '§1.3 group 3 with --write'),
    'counterclear_write': _P('data_counterclear_probe.py', ['--port', '{port}', '--write'] + _U, 600, 240,
                             [_LAST, r'd:\ht9045_log\qtydata\*', r'd:\ht9045_log\qtydata'], '§1.3 group 3 with --write'),
}


def _R(rid, group, title, probes, seconds=None, env='probe', cite=''):
    return {'id': rid, 'group': group, 'title': title, 'probes': probes, 'seconds': seconds, 'env': env, 'cite': cite}


ROUNDS = [
    _R('r1', 1, 'boot only, no probes', [], 120, 'boot', '§6.2'),
    _R('r2', 2, 'read-only probes, one boot',
       ['q3_dio_owner', 'q41_open_gate', 'contactct_live', 'observer_token', 'formevent', 'formevent_cc'], 1800, cite='§6.3'),
    _R('r3a', 3, 'formevent_ta_ts, no --allow-save', ['formevent_ta_ts'], cite='§6.4 #1'),
    _R('r3b', 3, 'formevent_ta_ts --allow-save', ['formevent_ta_ts_save'], cite='§6.4 #1'),
    _R('r3c', 3, 'formevent_cc --allow-save', ['formevent_cc_save'], cite='§6.4 #1'),
    _R('r3d', 3, 'q41_closetail S1 only', ['closetail_s1'], cite='§6.4 #2'),
    _R('r3e', 3, 'q41_closetail --save', ['closetail_save'], cite='§6.4 #2'),
    _R('r3f', 3, 'q41_closetail --hotplate', ['closetail_hotplate'], cite='§6.4 #2'),
    _R('r3g', 3, 'status_countersel --write', ['countersel_write'], cite='§6.4 #3'),
    _R('r3h', 3, 'data_smartdiag', ['smartdiag'], cite='§6.4 #4'),
    _R('r3i', 3, 'data_builder (check the recycle bin)', ['builder'], cite='§6.4 #4'),
    _R('r3j', 3, 'data_sortct', ['sortct'], cite='§6.4 #4'),
    _R('r3k', 3, 's12_form --write', ['s12_form_write'], cite='§6.4 #4'),
    _R('r4a', 4, 'regression, read-only probes, one boot',
       ['contactct', 'observer', 'testcategory', 's1_eventlog', 's7_thermo', 's12c_config', 's12c_offset', 's12c_page',
        'counterclear'], cite='§6.5'),
    _R('r4b', 4, 'regression: data_lotinfo (lot.start writes)', ['lotinfo'], cite='§6.5'),
    _R('r4c', 4, 'regression: showmymessage', ['showmymessage'], cite='§6.5'),
    _R('r4d', 4, 'regression: status_towerlight (needs --offsets-file)', ['towerlight'], cite='§6.5'),
    _R('r4e', 4, 'regression: s12c_config --write', ['s12c_config_write'], cite='§6.5'),
    _R('r4f', 4, 'regression: s12c_offset --write', ['s12c_offset_write'], cite='§6.5'),
    _R('r4g', 4, 'regression: data_counterclear --write', ['counterclear_write'], cite='§6.5'),
]

# processes: refuse to start while any runs; abort mid-run when one appears (not our own wb_serve pid)
WRITER_PROCS = ['wb_serve.exe', 'wb_publish.exe', 'ht9045.exe', 'h9046_32gpib.exe', 'rs232standard.exe',
                'eventloganalyzer.exe']
BUILD_PROCS = ['ctest.exe', 'cc1plus.exe', 'ninja.exe', 'cmake.exe', 'mingw32-make.exe', 'make.exe', 'collect2.exe',
               'ld.exe', 'g++.exe', 'gcc.exe', 'cl.exe', 'msbuild.exe']
MIDRUN_BUILD_PROCS = ['ctest.exe']          # sortct itself runs g++, so only a new ctest aborts mid-run
GATE_LOGS = [('st01-*-gate.log', r'^=== gate done'), ('st02-*-build.log', r'^=== ctest exit .*done')]
GATE_MARKERS = ['GATE_ACTIVE*', 'BUILD_ACTIVE*']
PWBOOK_LINE = 'S12TEST 3 S12PW'            # spec §6.1 step 4 (format: user level password, WebLogin.cpp:574)


class GuardError(Exception):
    pass


# ------------------------------------------------------------------------------------------------------- utilities
class Log:
    def __init__(self):
        self.f = None

    def open(self, path):
        self.f = open(path, 'a', encoding='utf-8')

    def __call__(self, *a):
        s = ' '.join(str(x) for x in a)
        print(s, flush=True)
        if self.f:
            self.f.write(time.strftime('%H:%M:%S ') + s + '\n')
            self.f.flush()


LOG = Log()


def iso(t=None):
    return datetime.datetime.fromtimestamp(t or time.time()).strftime('%Y-%m-%d %H:%M:%S')


def sha256_of(p):
    h = hashlib.sha256()
    try:
        with open(p, 'rb') as f:
            while True:
                b = f.read(1 << 20)
                if not b:
                    break
                h.update(b)
        return h.hexdigest(), None
    except OSError as e:
        return None, str(e)


def copy_hash(src, dst):
    """Copy src -> dst while hashing what was read; copy the timestamps / read-only bit; return (sha, size)."""
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    h, n = hashlib.sha256(), 0
    with open(src, 'rb') as fi, open(dst, 'wb') as fo:
        while True:
            b = fi.read(1 << 20)
            if not b:
                break
            h.update(b)
            fo.write(b)
            n += len(b)
    shutil.copystat(src, dst)
    return h.hexdigest(), n


def make_writable(p):
    try:
        if os.path.exists(p) and not os.access(p, os.W_OK):
            os.chmod(p, stat.S_IWRITE | stat.S_IREAD)
    except OSError:
        pass


def json_dump(obj, path):
    tmp = path + '.tmp'
    with open(tmp, 'w', encoding='utf-8') as f:
        json.dump(obj, f, ensure_ascii=False, indent=1)
    os.replace(tmp, path)


def read_tail(p, n=8192):
    try:
        with open(p, 'rb') as f:
            f.seek(0, 2)
            sz = f.tell()
            f.seek(max(0, sz - n))
            return f.read().decode('utf-8', 'replace')
    except OSError:
        return ''


def _toolhelp():
    """(image name, pid) of every process via CreateToolhelp32Snapshot -- fast and independent of tasklist.exe,
    which took more than 60 s on this machine while a gate was compiling (test T19, 20260928)."""
    import ctypes
    from ctypes import wintypes as wt

    class PE(ctypes.Structure):
        _fields_ = [('dwSize', wt.DWORD), ('cntUsage', wt.DWORD), ('th32ProcessID', wt.DWORD),
                    ('th32DefaultHeapID', ctypes.c_void_p), ('th32ModuleID', wt.DWORD), ('cntThreads', wt.DWORD),
                    ('th32ParentProcessID', wt.DWORD), ('pcPriClassBase', ctypes.c_long), ('dwFlags', wt.DWORD),
                    ('szExeFile', ctypes.c_wchar * 260)]
    k = ctypes.WinDLL('kernel32', use_last_error=True)
    k.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
    k.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
    k.Process32FirstW.argtypes = [ctypes.c_void_p, ctypes.POINTER(PE)]
    k.Process32NextW.argtypes = [ctypes.c_void_p, ctypes.POINTER(PE)]
    k.CloseHandle.argtypes = [ctypes.c_void_p]
    h = k.CreateToolhelp32Snapshot(2, 0)                   # TH32CS_SNAPPROCESS
    if not h or h == ctypes.c_void_p(-1).value:
        return None
    rows = []
    e = PE()
    e.dwSize = ctypes.sizeof(PE)
    try:
        ok = k.Process32FirstW(h, ctypes.byref(e))
        while ok:
            rows.append((e.szExeFile, int(e.th32ProcessID)))
            ok = k.Process32NextW(h, ctypes.byref(e))
    finally:
        k.CloseHandle(h)
    return rows or None


def tasklist():
    try:
        rows = _toolhelp()
        if rows:
            return rows
    except (OSError, AttributeError, ValueError):
        pass
    for _ in range(2):                                      # fallback: tasklist.exe, generous timeout, one retry
        try:
            o = subprocess.run(['tasklist', '/FO', 'CSV', '/NH'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                               timeout=120).stdout.decode('utf-8', 'replace')
        except (OSError, subprocess.SubprocessError):
            continue
        rows = []
        for row in csv.reader(o.splitlines()):
            if len(row) >= 2 and row[1].strip().isdigit():
                rows.append((row[0].strip(), int(row[1])))
        if rows:
            return rows
    return None


def pid_alive(pid):
    rows = tasklist() or []
    return any(p == pid for _, p in rows)


def port_busy(port):
    """Listening sockets on :port (netstat, foreign 0.0.0.0:0 / [::]:0 -- TIME_WAIT leftovers do not count) or a
    connect to 127.0.0.1:port that is accepted."""
    hits = []
    for proto in ('TCP', 'TCPv6'):
        try:
            o = subprocess.run(['netstat', '-ano', '-p', proto], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                               timeout=60).stdout.decode('utf-8', 'replace')
        except (OSError, subprocess.SubprocessError):
            continue
        for line in o.splitlines():
            p = line.split()
            if len(p) >= 4 and p[0].upper().startswith('TCP') and p[1].endswith(':%d' % port) \
                    and p[2] in ('0.0.0.0:0', '[::]:0'):
                hits.append(' '.join(p))
    s = socket.socket()
    s.settimeout(0.5)
    try:
        s.connect(('127.0.0.1', port))
        hits.append('connect to 127.0.0.1:%d accepted' % port)
    except OSError:
        pass
    finally:
        s.close()
    return hits


def kill_tree(pid):
    subprocess.run(['taskkill', '/T', '/F', '/PID', str(pid)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def ctrl_break(proc):
    try:
        os.kill(proc.pid, signal.CTRL_BREAK_EVENT)
        return True
    except (OSError, ValueError, AttributeError) as e:
        LOG('   Ctrl-Break to pid %d failed (%s)' % (proc.pid, e))
        return False


def wait_proc(proc, sec):
    try:
        proc.wait(timeout=sec)
        return True
    except subprocess.TimeoutExpired:
        return False


# ------------------------------------------------------------------------------------------------------ path space
class Space:
    """The real D:\\ paths, or (test mode, --root R) the same paths under R\\D\\..."""

    def __init__(self, root):
        self.root = os.path.abspath(root) if root else None

    def m(self, real):
        if not self.root:
            return real
        drive, rest = os.path.splitdrive(real)
        return os.path.join(self.root, drive.rstrip(':').upper(), rest.lstrip('\\/'))


def real_of(rb, rel):
    return rb['path'] if rb['kind'] == 'file' else rb['path'] + '\\' + rel


def under(base, real):
    """base\\<drive letter>\\<rest of real> (backup / evidence layout)."""
    drive, rest = os.path.splitdrive(real)
    return os.path.join(base, drive.rstrip(':').upper(), rest.lstrip('\\/'))


def scan_root(space, rb, hashing=True):
    base = space.m(rb['path'])
    res = {'exists': False, 'files': {}, 'dirs': {}, 'errs': []}
    if rb['kind'] == 'file':
        if os.path.isfile(base):
            res['exists'] = True
            st = os.stat(base)
            sha, err = sha256_of(base) if hashing else (None, None)
            res['files'][''] = ['', sha, st.st_size, st.st_mtime_ns, err]
        elif os.path.exists(base):
            res['exists'] = True
            res['errs'].append('not a file: ' + base)
        return res
    if not os.path.isdir(base):
        if os.path.exists(base):
            res['exists'] = True
            res['errs'].append('not a folder: ' + base)
        return res
    res['exists'] = True
    for dp, dn, fn in os.walk(base, onerror=lambda e: res['errs'].append('walk: %s' % e)):
        rd = os.path.relpath(dp, base)
        if rd != '.':
            res['dirs'][rd.lower()] = rd
        for f in fn:
            p = os.path.join(dp, f)
            rel = os.path.relpath(p, base)
            try:
                st = os.stat(p)
            except OSError as e:
                res['files'][rel.lower()] = [rel, None, -1, 0, str(e)]
                continue
            sha, err = sha256_of(p) if hashing else (None, None)
            res['files'][rel.lower()] = [rel, sha, st.st_size, st.st_mtime_ns, err]
    return res


def toplevel(space):
    out = {}
    for t in TOPLEVEL:
        try:
            out[t] = sorted(os.listdir(space.m(t)))
        except OSError:
            out[t] = None
    return out


def recycle_list(space):
    base = space.m(RECYCLE)
    out = []
    try:
        sids = os.listdir(base)
    except OSError:
        return out
    for sid in sids:
        sp = os.path.join(base, sid)
        try:
            if os.path.isdir(sp):
                out.extend(sid + '\\' + n for n in os.listdir(sp))
        except OSError:
            pass                        # other users' bins are not readable
    return sorted(out)


def recycle_origin(path):
    """Original path stored in a $I file (Windows 10+ format 2, or format 1)."""
    try:
        with open(path, 'rb') as f:
            b = f.read(4096)
        ver = int.from_bytes(b[0:8], 'little')
        if ver == 2:
            n = int.from_bytes(b[24:28], 'little')
            return b[28:28 + 2 * n].decode('utf-16-le', 'replace').rstrip('\0')
        if ver == 1:
            return b[24:24 + 520].decode('utf-16-le', 'replace').split('\0')[0]
    except OSError:
        pass
    return '?'


# ------------------------------------------------------------------------------------------------------ the context
class Ctx:
    def __init__(self, a):
        self.a = a
        self.space = Space(a.root)
        self.test = bool(a.root)
        self.probe_dir = os.path.abspath(a.probe_dir) if a.probe_dir else HERE
        self.stop_flag = False
        self.dirty = False          # True from the moment wb_serve starts until a verify says every file is back
        self.rundir = None
        self.man = None
        self.exe = None             # copied exe
        self.exe_info = {}
        self.pwbook = None
        self.levelset = None
        self.baseline_sha = None
        self.webcopy = None
        self.expected = list(EXPECTED_BOOT) + [(p.lower(), _TREE, 'added by operator (--expect-extra)')
                                               for p in (a.expect_extra or [])]

    def P(self, real):
        return self.space.m(real)

    def in_set(self, mapped):
        pl = os.path.normcase(os.path.abspath(mapped))
        for path, kind, _ in ROOTS:
            b = os.path.normcase(os.path.abspath(self.P(path)))
            if pl == b or (kind == 'dir' and pl.startswith(b.rstrip('\\') + '\\')):
                return True
        return False


CTX = None


def on_sigint(signum, frame):
    if CTX is not None:
        CTX.stop_flag = True
    print('\n!! Ctrl-C: the round stops, wb_serve gets Ctrl-Break, then every file is restored -- keep this window open',
          flush=True)


# ----------------------------------------------------------------------------------------------------- preflight
def gate_activity(ctx):
    found = []
    ai = ctx.P(AI_TEMP)
    win = ctx.a.gate_log_window
    if win > 0:
        for pat, term in GATE_LOGS:
            for p in glob.glob(os.path.join(ai, pat)):
                try:
                    mt = max(os.path.getmtime(x) for x in [p] + glob.glob(glob.escape(p) + '.*'))
                except (OSError, ValueError):
                    continue
                if time.time() - mt > win * 60:
                    continue
                if not re.search(term, read_tail(p), re.M):
                    found.append('gate/proxy log still open: %s (last write %s, no "%s" line)' % (p, iso(mt), term))
    for pat in GATE_MARKERS:
        for p in glob.glob(os.path.join(ai, pat)):
            found.append('agreed build marker present: %s' % p)
    return found


def proc_problems(ctx, mine=(), build=True, midrun=False):
    rows = tasklist()
    if rows is None:
        return [] if midrun else ['tasklist failed (cannot tell what runs)']
    seen = {}
    blist = MIDRUN_BUILD_PROCS if midrun else BUILD_PROCS
    for name, pid in rows:
        n = name.lower()
        if pid in mine:
            continue
        if n in WRITER_PROCS:
            seen.setdefault((n, ''), []).append(pid)
        elif build and n in blist:
            seen.setdefault((n, ' -- a gate / build / ctest'), []).append(pid)
    out = []
    for (n, why), pids in sorted(seen.items()):
        more = (' +%d more' % (len(pids) - 5)) if len(pids) > 5 else ''
        out.append('%s is running (pid %s%s)%s' % (n, ', '.join(str(p) for p in pids[:5]), more, why))
    return out


def preflight(ctx, full=True):
    """Reasons to refuse.  Read-only."""
    why = []
    mk = ctx.P(MARKER)
    if os.path.exists(mk):
        try:
            j = json.load(open(mk, encoding='utf-8'))
        except (OSError, ValueError):
            j = {}
        why.append('marker %s exists (state %s, backup %s): a previous run is not verified back -- run --restore-only %s'
                   % (mk, j.get('state'), j.get('rundir'), j.get('rundir')))
    skip_build = ctx.test and ctx.a.test_skip_build_check     # test only: the process part, not the gate logs
    why += proc_problems(ctx, build=not skip_build)
    for port in sorted({8045, ctx.a.port}):
        for h in port_busy(port):
            why.append('port %d busy: %s' % (port, h))
    why += gate_activity(ctx)
    if full and ctx.a.wb_serve and not os.path.isfile(ctx.a.wb_serve):
        why.append('--wb-serve %s: no such file' % ctx.a.wb_serve)
    return why


def r1_result_path(ctx, sha):
    return os.path.join(ctx.P(RUN_BASE), 'r1_result_%s.json' % sha[:8])


# ------------------------------------------------------------------------------------------------------- marker
def marker_create(ctx, rundir):
    mk = ctx.P(MARKER)
    os.makedirs(os.path.dirname(mk), exist_ok=True)
    fd = os.open(mk, os.O_CREAT | os.O_EXCL | os.O_WRONLY)       # exclusive: a second guard cannot pass
    os.close(fd)
    marker_update(ctx, 'preparing', rundir=rundir)


def marker_update(ctx, state, rundir=None, rnd=None):
    mk = ctx.P(MARKER)
    j = {}
    try:
        j = json.load(open(mk, encoding='utf-8'))
    except (OSError, ValueError):
        pass
    j.update({'guard': 'wbrun_guard.py', 'state': state, 'updated': iso(), 'pid': os.getpid()})
    if rundir:
        j['rundir'] = rundir
    if rnd:
        j['round'] = rnd
    j['restore'] = '"%s" "%s" --restore-only "%s"%s' % (PY, os.path.abspath(__file__), j.get('rundir'),
                                                      (' --root "%s"' % ctx.space.root) if ctx.test else '')
    json_dump(j, mk)


def marker_remove(ctx, rundir):
    mk = ctx.P(MARKER)
    try:
        j = json.load(open(mk, encoding='utf-8'))
    except (OSError, ValueError):
        return
    if os.path.normcase(j.get('rundir') or '') == os.path.normcase(rundir):
        os.remove(mk)
        LOG('marker removed:', mk)
    else:
        LOG('marker %s belongs to %s -- left alone' % (mk, j.get('rundir')))


# ------------------------------------------------------------------------------------------------------ snapshot
def snapshot(ctx):
    rd = ctx.rundir
    bk = os.path.join(rd, 'bk')
    roots, total, nfiles = [], 0, 0
    t0 = time.time()
    for path, kind, cls in ROOTS:
        rb = {'path': path, 'kind': kind, 'cls': cls}
        lst = scan_root(ctx.space, rb, hashing=False)
        if lst['errs']:
            raise GuardError('cannot list %s: %s' % (path, lst['errs'][:3]))
        rb['exists'] = lst['exists']
        rb['dirs'] = lst['dirs']
        rb['files'] = {}
        for k, (rel, _s, _z, _m, err) in sorted(lst['files'].items()):
            if err:
                raise GuardError('cannot stat %s: %s' % (real_of(rb, rel), err))
            real = real_of(rb, rel)
            src, dst = ctx.P(real), under(bk, real)
            if len(dst) > 255:
                raise GuardError('backup path would be %d characters: %s' % (len(dst), dst))
            try:
                sha, n = copy_hash(src, dst)
            except OSError as e:
                raise GuardError('cannot back up %s: %s' % (src, e))
            sha2, err2 = sha256_of(dst)
            if sha2 != sha:
                raise GuardError('backup copy of %s does not verify (%s)' % (src, err2 or 'hash differs'))
            rb['files'][k] = [rel, sha, n, os.stat(src).st_mtime_ns]
            total += n
            nfiles += 1
        LOG('   backed up %-34s %s  %5d files %9.1f MB' % (path, 'exists' if rb['exists'] else 'ABSENT',
                                                            len(rb['files']), sum(v[2] for v in rb['files'].values()) / 1e6))
        roots.append(rb)
    man = {'guard': 'wbrun_guard.py', 'complete': True, 'created': iso(), 'test_root': ctx.space.root,
           'rundir': rd, 'roots': roots, 'toplevel': toplevel(ctx.space), 'recycle': recycle_list(ctx.space),
           'files': nfiles, 'bytes': total, 'exe': ctx.exe_info, 'label': ctx.a.label, 'spec': SPEC}
    json_dump(man, os.path.join(rd, 'manifest.json'))
    # sha256sum-style list of D:\HT9045 (data_contactct_probe --baseline reads "./system/Arm*.dat" lines)
    lines = []
    for rb in roots:
        if rb['path'].lower().startswith('d:\\ht9045\\'):
            for k, v in sorted(rb['files'].items()):
                rel = real_of(rb, v[0])[len('D:\\HT9045\\'):].replace('\\', '/')
                lines.append('%s  ./%s\n' % (v[1], rel))
    ctx.baseline_sha = os.path.join(rd, 'baseline_HT9045.sha256')
    with open(ctx.baseline_sha, 'w', encoding='utf-8', newline='\n') as f:
        f.writelines(lines)
    LOG('backup complete: %d files, %.1f MB, %.0f s -> %s' % (nfiles, total / 1e6, time.time() - t0, bk))
    return man


def load_manifest(rundir):
    p = os.path.join(rundir, 'manifest.json')
    try:
        man = json.load(open(p, encoding='utf-8'))
    except (OSError, ValueError) as e:
        raise GuardError('no readable manifest in %s (%s)' % (rundir, e))
    if man.get('guard') != 'wbrun_guard.py' or not man.get('complete'):
        raise GuardError('%s is not a complete wbrun_guard manifest' % p)
    return man


# --------------------------------------------------------------------------------------------------- diff/classify
def diff_all(ctx, man):
    """Rescan every root; return (entries, scans).  entry = dict(kind, real, root, key)."""
    entries, scans = [], {}
    for rb in man['roots']:
        cur = scan_root(ctx.space, rb)
        scans[rb['path']] = cur
        bf, cf = rb['files'], cur['files']
        for k, v in bf.items():
            real = real_of(rb, v[0])
            if k not in cf:
                entries.append({'kind': 'deleted', 'real': real, 'root': rb['path'], 'key': k, 'before': v[2]})
            elif cf[k][1] is None:
                entries.append({'kind': 'unreadable', 'real': real, 'root': rb['path'], 'key': k, 'before': v[2],
                                'err': cf[k][4]})
            elif cf[k][1] != v[1]:
                entries.append({'kind': 'changed', 'real': real, 'root': rb['path'], 'key': k, 'before': v[2],
                                'after': cf[k][2]})
        for k, v in cf.items():
            if k not in bf:
                entries.append({'kind': 'new', 'real': real_of(rb, v[0]), 'root': rb['path'], 'key': k, 'after': v[2]})
        if rb['kind'] == 'dir':
            for k, rel in cur['dirs'].items():
                if k not in rb['dirs']:
                    entries.append({'kind': 'dirnew', 'real': rb['path'] + '\\' + rel, 'root': rb['path'], 'key': k})
            if rb['exists'] and cur['exists']:
                for k, rel in rb['dirs'].items():
                    if k not in cur['dirs']:
                        entries.append({'kind': 'dirgone', 'real': rb['path'] + '\\' + rel, 'root': rb['path'], 'key': k})
        if rb['exists'] and not cur['exists']:
            entries.append({'kind': 'rootgone', 'real': rb['path'], 'root': rb['path'], 'key': ''})
        elif not rb['exists'] and cur['exists']:
            entries.append({'kind': 'rootnew', 'real': rb['path'], 'root': rb['path'], 'key': ''})
        for e in cur['errs']:
            entries.append({'kind': 'scan-error', 'real': rb['path'], 'root': rb['path'], 'key': '', 'err': e})
    return entries, scans


def match(patterns, e):
    low = e['real'].lower()
    for pat, kinds, cite in patterns:
        if e['kind'] not in kinds.split():
            continue
        if fnmatch.fnmatchcase(low, pat):
            return pat, cite
        if e['kind'] in ('dirnew', 'dirgone') and pat.endswith('*') and fnmatch.fnmatchcase(low + '\\x', pat):
            return pat, cite
    return None


def classify(ctx, entries, probes):
    for e in entries:
        m = match(ctx.expected, e)
        e['boot'] = m[0] if m else None
        e['declared_by'] = [p['name'] for p in probes if match(p['declares'], e)]
        e['class'] = 'boot-expected' if m else ('declared by ' + ','.join(e['declared_by']) if e['declared_by']
                                                else 'UNDECLARED')


def change_detail(bkfile, evfile, e, diffdir):
    """Short human text for a changed file: appended bytes, or a unified diff (text files <= 4 MB)."""
    try:
        old = open(bkfile, 'rb').read() if os.path.getsize(bkfile) <= 4 << 20 else None
        new = open(evfile, 'rb').read() if os.path.getsize(evfile) <= 4 << 20 else None
    except OSError:
        return 'size %s -> %s' % (e.get('before'), e.get('after'))
    if old is None or new is None:
        return 'size %s -> %s (large file, no diff)' % (e.get('before'), e.get('after'))

    def dec(b):
        try:
            return b.decode('utf-8')
        except UnicodeDecodeError:
            return b.decode('cp950', 'replace')
    if b'\0' in old[:4096] or b'\0' in new[:4096]:
        nd = sum(1 for i in range(min(len(old), len(new))) if old[i] != new[i]) + abs(len(old) - len(new))
        return 'binary, %d -> %d bytes, %d bytes differ' % (len(old), len(new), nd)
    if new.startswith(old):
        add = dec(new[len(old):]).splitlines()
        return 'appended %d bytes / %d lines: %s' % (len(new) - len(old), len(add), ' | '.join(add[:3])[:300])
    d = list(difflib.unified_diff(dec(old).splitlines(), dec(new).splitlines(), 'before', 'after', n=0, lineterm=''))
    os.makedirs(diffdir, exist_ok=True)
    fn = os.path.join(diffdir, re.sub(r'[\\/:]+', '__', e['real']) + '.diff')
    with open(fn, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(d) + '\n')
    body = [x for x in d[2:] if not x.startswith('@@')]
    return '%d line(s) differ: %s' % (len(body), ' | '.join(body[:4])[:300])


# ------------------------------------------------------------------------------------------------------- restore
def restore(ctx, man, entries, evdir, diffdir):
    """Put the backup set back: evidence first, then restore changed/deleted, delete new files and folders."""
    bk = os.path.join(man['rundir'], 'bk')
    acts = []
    roots = dict((rb['path'], rb) for rb in man['roots'])

    def act(what, real, ok, detail=''):
        acts.append({'action': what, 'real': real, 'ok': ok, 'detail': detail})
        if not ok:
            LOG('   RESTORE FAILED: %s %s %s' % (what, real, detail))

    def evidence(real):
        src = ctx.P(real)
        if not evdir or not os.path.isfile(src):
            return None
        dst = under(evdir, real)
        try:
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(src, dst)
            return dst
        except OSError as e:
            LOG('   (evidence copy of %s failed: %s)' % (real, e))
            return None

    # 1. folders that went away (root first), so files can be put back into them
    for e in sorted([x for x in entries if x['kind'] in ('rootgone', 'dirgone')], key=lambda x: len(x['real'])):
        rb = roots[e['root']]
        if e['kind'] == 'rootgone' and rb['kind'] == 'file':
            continue
        dst = ctx.P(e['real'])
        assert ctx.in_set(dst), dst
        try:
            os.makedirs(dst, exist_ok=True)
            act('mkdir', e['real'], True)
        except OSError as x:
            act('mkdir', e['real'], False, str(x))
    if any(x['kind'] == 'rootgone' and roots[x['root']]['kind'] == 'dir' for x in entries):
        for rb in man['roots']:              # sub-folders of a root that disappeared completely
            if rb['kind'] == 'dir' and any(x['kind'] == 'rootgone' and x['root'] == rb['path'] for x in entries):
                for rel in sorted(rb['dirs'].values(), key=len):
                    try:
                        os.makedirs(ctx.P(rb['path'] + '\\' + rel), exist_ok=True)
                    except OSError as x:
                        act('mkdir', rb['path'] + '\\' + rel, False, str(x))
    # 2. changed / unreadable / deleted files <- backup
    for e in entries:
        if e['kind'] not in ('changed', 'unreadable', 'deleted'):
            continue
        rb = roots[e['root']]
        if e['kind'] != 'deleted':
            ev = evidence(e['real'])
            if ev and e['kind'] == 'changed':
                e['detail'] = change_detail(under(bk, e['real']), ev, e, diffdir)
        src, dst = under(bk, e['real']), ctx.P(e['real'])
        assert ctx.in_set(dst), dst
        want = rb['files'][e['key']][1]
        try:
            make_writable(dst)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copyfile(src, dst)
            shutil.copystat(src, dst)
            got, err = sha256_of(dst)
            act('restore', e['real'], got == want, '' if got == want else (err or 'hash differs after copy'))
        except OSError as x:
            act('restore', e['real'], False, str(x))
    # 3. new files -> evidence, then delete
    for e in entries:
        if e['kind'] != 'new':
            continue
        dst = ctx.P(e['real'])
        assert ctx.in_set(dst), dst
        evidence(e['real'])
        try:
            make_writable(dst)
            os.remove(dst)
            act('delete-new', e['real'], True)
        except OSError as x:
            act('delete-new', e['real'], False, str(x))
    # 4. new folders, deepest first; then roots the run created
    news = [x for x in entries if x['kind'] == 'dirnew'] + [x for x in entries if x['kind'] == 'rootnew']
    for e in sorted(news, key=lambda x: (x['kind'] == 'rootnew', -x['real'].count('\\'))):
        rb = roots[e['root']]
        if e['kind'] == 'rootnew' and rb['kind'] == 'file':
            continue                          # the file itself was handled as 'new'
        dst = ctx.P(e['real'])
        assert ctx.in_set(dst), dst
        try:
            os.rmdir(dst)
            act('rmdir-new', e['real'], True)
        except OSError as x:
            act('rmdir-new', e['real'], False, str(x))
    return acts


def verify(ctx, man):
    """Every file / folder of the backup set equal to the baseline?  Returns the remaining entries."""
    entries, _ = diff_all(ctx, man)
    return entries


def residue(ctx, man, port):
    out = []
    now = toplevel(ctx.space)
    for t, before in (man.get('toplevel') or {}).items():
        cur = now.get(t)
        if before is None or cur is None:
            continue
        for n in sorted(set(cur) - set(before)):
            out.append({'kind': 'top-level new', 'what': os.path.join(t, n)})
        for n in sorted(set(before) - set(cur)):
            out.append({'kind': 'top-level gone', 'what': os.path.join(t, n)})
    before = set(man.get('recycle') or [])
    base = ctx.P(RECYCLE)
    for n in recycle_list(ctx.space):
        if n not in before:
            leaf = n.split('\\')[-1]
            org = recycle_origin(os.path.join(base, n)) if leaf.upper().startswith('$I') else ''
            out.append({'kind': 'recycle-bin new', 'what': os.path.join(RECYCLE, n), 'origin': org})
    for p in proc_problems(ctx, build=False):
        out.append({'kind': 'process', 'what': p})
    for h in port_busy(port):
        out.append({'kind': 'port', 'what': h})
    return out


# -------------------------------------------------------------------------------------------------------- probes
def build_env(ctx, rnd):
    drop = sorted(k for k in os.environ if k.upper().startswith(('W906_', 'HT9045_')))
    env = dict((k, v) for k, v in os.environ.items() if k not in drop)
    add = {'HT9045_TESTERCOMM': '0', 'HT9045_ELA': '0', 'W906_NO_BROWSER_WAKE': '1'}
    if rnd['env'] == 'probe':
        add['W906_PWBOOK_PATH'] = ctx.pwbook
        add['W906_LEVELSET_PATH'] = ctx.levelset
    env.update(add)
    return env, add, drop


def probe_args_override(ctx):
    out = {}
    for s in ctx.a.probe_args or []:
        if '=' not in s:
            raise GuardError('--probe-args wants NAME=ARGS, got %r' % s)
        n, rest = s.split('=', 1)
        out[n.strip()] = [t[1:-1] if len(t) > 1 and t[0] == t[-1] == '"' else t for t in shlex.split(rest, posix=False)]
    return out


def resolve_probes(ctx, rnd, rdir):
    over = probe_args_override(ctx)
    vals = {'port': str(ctx.a.port), 'user': ctx.a.user, 'password': ctx.a.password,
            'console': os.path.join(rdir, 'console.txt'), 'levelset': ctx.levelset, 'pwbook': ctx.pwbook,
            'D': ctx.P('D:\\'), 'run': ctx.rundir, 'baseline_sha': ctx.baseline_sha,
            'offsets': os.path.abspath(ctx.a.offsets_file) if ctx.a.offsets_file else None}
    out = []
    for name in rnd['probes']:
        spec = PROBES[name]
        pr = {'name': name, 'script': os.path.join(ctx.probe_dir, spec['script']), 'timeout': spec['timeout'],
              'budget': spec['budget'], 'declares': spec['declares'], 'cite': spec['cite'], 'skip': None}
        if ctx.a.probe_timeout:
            pr['timeout'] = min(pr['timeout'], ctx.a.probe_timeout)
        args = over.get(name, spec['args'])
        cmd = [PY, pr['script']]
        for t in args:
            for ph in re.findall(r'\{(\w+)\}', t):
                if vals.get(ph) is None:
                    pr['skip'] = 'needs {%s}%s' % (ph, ' (--offsets-file)' if ph == 'offsets' else '')
            try:
                cmd.append(t.format(**dict((k, v if v is not None else '<missing>') for k, v in vals.items())))
            except (KeyError, IndexError, ValueError) as x:
                raise GuardError('probe %s argument %r: %s' % (name, t, x))
        if not os.path.isfile(pr['script']):
            pr['skip'] = 'script not found: %s' % pr['script']
        pr['cmd'] = cmd
        out.append(pr)
    return out


def round_seconds(ctx, rnd, probes):
    if ctx.a.serve_seconds:
        return ctx.a.serve_seconds
    if rnd['seconds']:
        return rnd['seconds']
    return 60 + sum(p['budget'] for p in probes if not p['skip'])


def fail_lines(path):
    try:
        txt = open(path, encoding='utf-8', errors='replace').read().splitlines()
    except OSError:
        return []
    f = [x for x in txt if re.search(r'\bFAIL', x)][:20]
    return f or txt[-8:]


def watch(ctx, proc, state):
    """Every 5 s: a foreign wb_serve / writer / new ctest?  Returns an abort reason or None."""
    if time.time() < state.get('next', 0):
        return None
    state['next'] = time.time() + 5
    bad = proc_problems(ctx, mine={proc.pid} if proc else set(), build=not (ctx.test and ctx.a.test_skip_build_check),
                        midrun=True)
    return ('foreign process: ' + '; '.join(bad)) if bad else None


def run_probe(ctx, pr, idx, rdir, proc, deadline, env):
    out = os.path.join(rdir, 'probes', '%02d_%s.txt' % (idx, pr['name']))
    pr['out'] = out
    if pr['skip']:
        pr.update(status='SKIPPED', rc=None, secs=0)
        open(out, 'w', encoding='utf-8').write('SKIPPED: %s\n' % pr['skip'])
        return None
    left = deadline - time.time() - 10
    if left < 15:                       # --seconds was sized too small for this round: say so, do not start it
        pr.update(status='NO-TIME', rc=None, secs=0, skip='only %d s of wb_serve --seconds left' % left)
        open(out, 'w', encoding='utf-8').write('NOT RUN: %s\n' % pr['skip'])
        return None
    limit = min(pr['timeout'], left)
    penv = dict(env, PYTHONUTF8='1', PYTHONIOENCODING='utf-8')
    t0 = time.time()
    with open(out, 'wb') as f:
        f.write(('# %s\n# started %s, timeout %d s\n' % (subprocess.list2cmdline(pr['cmd']), iso(), limit)).encode('utf-8'))
        f.flush()
        p = subprocess.Popen(pr['cmd'], stdout=f, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, cwd=rdir,
                             env=penv, creationflags=subprocess.CREATE_NEW_PROCESS_GROUP)
        LOG('   probe %d %-22s pid %d, timeout %d s' % (idx, pr['name'], p.pid, limit))
        status, abort, st = None, None, {}
        gone_at = None
        while True:
            rc = p.poll()
            if rc is not None:
                status = 'PASS' if rc == 0 else 'FAIL'
                break
            if time.time() - t0 > limit:
                kill_tree(p.pid)
                status = 'TIMEOUT'
                break
            if ctx.stop_flag:
                kill_tree(p.pid)
                status, abort = 'ABORTED', 'operator Ctrl-C'
                break
            if proc.poll() is not None:
                gone_at = gone_at or time.time()
                if time.time() - gone_at > 10:
                    kill_tree(p.pid)
                    status = 'SERVER-GONE'
                    break
            abort = watch(ctx, proc, st)
            if abort:
                kill_tree(p.pid)
                status = 'ABORTED'
                break
            time.sleep(0.5)
        p.wait()
    pr.update(status=status, rc=p.returncode, secs=round(time.time() - t0, 1))
    pr['fail'] = fail_lines(out) if status != 'PASS' else []
    LOG('   probe %d %-22s %s rc=%s %.0f s' % (idx, pr['name'], status, p.returncode, pr['secs']))
    return abort


# --------------------------------------------------------------------------------------------------------- round
def uniq_dir(p):
    q, i = p, 2
    while os.path.exists(q):
        q = '%s_%d' % (p, i)
        i += 1
    return q


def serve_cmd(ctx, seconds):
    exe = ctx.exe or ctx.a.wb_serve or '<wb_serve.exe>'
    cmd = [exe, '--seconds', str(seconds), '--port', str(ctx.a.port), '--root', ctx.webcopy or '<run>\\web']
    if exe.lower().endswith('.py'):
        cmd = [PY] + cmd                      # test mode: fake server
    return cmd


def wait_port(ctx, proc, port, timeout):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if proc.poll() is not None:
            return None
        if ctx.stop_flag:
            return None
        s = socket.socket()
        s.settimeout(0.5)
        try:
            s.connect(('127.0.0.1', port))
            return time.time()
        except OSError:
            time.sleep(0.5)
        finally:
            s.close()
    return None


def sim_check(port):
    try:
        opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))      # never via a system proxy
        j = json.loads(opener.open('http://127.0.0.1:%d/api/struct/machine.defines?all=1' % port,
                                   timeout=15).read().decode('utf-8', 'replace'))
        d = (j.get('defines') or {}).get('SOFT_SIMULTE')
        return None if d is None else bool(d.get('on'))
    except Exception as e:          # noqa: BLE001 -- any failure = unknown, reported
        LOG('   SIM check: GET machine.defines failed (%s)' % e)
        return None


def stop_serve(ctx, proc, deadline, early, abort):
    """Normal: wait for --seconds to expire (golden FormClose saves run).  Returns (mode, abort)."""
    if proc.poll() is not None:
        return 'exited by itself, rc=%s' % proc.returncode, abort
    if abort or early or ctx.stop_flag:
        why = abort or ('operator Ctrl-C' if ctx.stop_flag else '--stop-early')
        ok = ctrl_break(proc)
        if ok and wait_proc(proc, 60):
            return 'ctrl-break (%s), rc=%s' % (why, proc.returncode), abort
        kill_tree(proc.pid)
        wait_proc(proc, 30)
        return 'killed (%s)' % why, abort
    limit = deadline + ctx.a.grace
    st = {}
    while proc.poll() is None:
        if ctx.stop_flag:
            return stop_serve(ctx, proc, deadline, False, 'operator Ctrl-C')
        a = watch(ctx, proc, st)
        if a:
            return stop_serve(ctx, proc, deadline, False, a)
        if time.time() > limit:
            m, _ = stop_serve(ctx, proc, deadline, False, 'still running %d s after --seconds' % ctx.a.grace)
            return m, 'wb_serve did not stop by itself'
        time.sleep(0.5)
    return 'natural (--seconds expired), rc=%s' % proc.returncode, abort


def run_round(ctx, rnd):
    rid = rnd['id']
    rdir = uniq_dir(os.path.join(ctx.rundir, 'r', rid))
    os.makedirs(os.path.join(rdir, 'probes'))
    probes = resolve_probes(ctx, rnd, rdir)
    seconds = round_seconds(ctx, rnd, probes)
    env, added, dropped = build_env(ctx, rnd)
    cmd = serve_cmd(ctx, seconds)
    rep = {'round': rid, 'title': rnd['title'], 'cite': rnd['cite'], 'rdir': rdir, 'started': iso(),
           'serve': {'cmd': cmd, 'cwd': os.path.join(rdir, 'cwd'), 'seconds': seconds, 'env_set': added,
                     'env_removed': dropped, 'console': os.path.join(rdir, 'console.txt')},
           'exe': ctx.exe_info, 'label': ctx.a.label, 'test_root': ctx.space.root, 'probes': probes,
           'aborted': None, 'unexpected': [], 'changes': [], 'restore': [], 'remaining': [], 'residue': []}
    LOG('=== ROUND %s -- %s (%s)' % (rid, rnd['title'], rnd['cite']))
    if probes and all(p['skip'] for p in probes):            # e.g. r4d without --offsets-file: do not boot for nothing
        for i, p in enumerate(probes, 1):
            run_probe(ctx, p, i, rdir, None, 0, {})
            LOG('   probe %d %s SKIPPED: %s' % (i, p['name'], p['skip']))
        rep.update(set_clean=True, result='CLEAN', wb_serve_started=False, finished=iso())
        write_report(rep)
        LOG('=== ROUND %s RESULT: CLEAN (not booted: every probe skipped)' % rid)
        return rep
    pf = preflight(ctx, full=False)
    pf = [x for x in pf if not x.startswith('marker ')]        # our own marker is expected here
    if pf:
        rep['aborted'] = 'preflight: ' + '; '.join(pf)
        LOG('   refused: ' + rep['aborted'])
        rep['set_clean'], rep['result'] = True, 'CLEAN'
        rep['wb_serve_started'] = False
        write_report(rep)
        return rep
    marker_update(ctx, 'running', rnd=rid)
    os.makedirs(rep['serve']['cwd'])
    LOG('   wb_serve: %s' % subprocess.list2cmdline(cmd))
    LOG('   env set: %s; removed: %s' % (' '.join('%s=%s' % kv for kv in added.items()), ' '.join(dropped) or '-'))
    proc, abort, t0 = None, None, time.time()
    cf = open(rep['serve']['console'], 'wb')
    try:
        ctx.dirty = True            # from here on the real files may change until a verify says otherwise
        proc = subprocess.Popen(cmd, stdout=cf, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, env=env,
                                cwd=rep['serve']['cwd'], creationflags=subprocess.CREATE_NEW_PROCESS_GROUP)
        rep['serve']['pid'] = proc.pid
        rep['wb_serve_started'] = True
        t_open = wait_port(ctx, proc, ctx.a.port, ctx.a.boot_timeout)
        if t_open is None:
            abort = 'operator Ctrl-C during boot' if ctx.stop_flag else (
                'wb_serve exited during boot (rc %s)' % proc.returncode if proc.poll() is not None
                else 'port %d not open after %d s' % (ctx.a.port, ctx.a.boot_timeout))
            deadline = time.time()
        else:
            rep['serve']['boot_s'] = round(t_open - t0, 1)
            deadline = t_open + seconds
            LOG('   port %d open after %.1f s; wb_serve stops by itself at about %s' % (ctx.a.port, t_open - t0, iso(deadline)))
            sim = sim_check(ctx.a.port)
            rep['serve']['sim'] = sim
            if sim is False:
                abort = 'NOT a SIM build (machine.defines SOFT_SIMULTE off)'
            elif sim is None:
                LOG('   WARNING: could not confirm SOFT_SIMULTE; continuing (the report says so)')
            else:
                LOG('   SIM build confirmed (SOFT_SIMULTE on)')
            for i, pr in enumerate(probes, 1):
                if abort:
                    pr.update(status='NOT-RUN', rc=None, secs=0, out=None, fail=[])
                    continue
                abort = run_probe(ctx, pr, i, rdir, proc, deadline, env)
        mode, abort = stop_serve(ctx, proc, deadline, ctx.a.stop_early, abort)
        rep['serve']['stop'] = mode
    except Exception as x:          # noqa: BLE001 -- whatever happened, stop wb_serve and restore
        rep['exception'] = traceback.format_exc()
        abort = abort or 'guard exception: %s' % x
        LOG('   EXCEPTION: %s' % x)
    finally:
        if proc is not None and proc.poll() is None:
            ctrl_break(proc)
            if not wait_proc(proc, 30):
                kill_tree(proc.pid)
                wait_proc(proc, 30)
            rep['serve']['stop'] = rep['serve'].get('stop') or 'forced'
        cf.close()
    rep['serve']['rc'] = proc.returncode if proc else None
    rep['serve']['elapsed_s'] = round(time.time() - t0, 1)
    rep['aborted'] = abort
    LOG('   wb_serve %s, %.0f s' % (rep['serve'].get('stop'), rep['serve']['elapsed_s']))
    if abort:
        LOG('   ABORTED: %s -- no further round runs after this one; restoring now' % abort)
    time.sleep(ctx.a.settle)
    marker_update(ctx, 'restoring', rnd=rid)
    finish_round(ctx, rep, probes, rid == 'r1')
    return rep


def finish_round(ctx, rep, probes, is_r1):
    rdir = rep['rdir']
    entries, _ = diff_all(ctx, ctx.man)
    classify(ctx, entries, [p for p in probes if p.get('status') not in (None,)])
    if is_r1:
        rep['unexpected'] = [e for e in entries if not e['boot']]
    acts = restore(ctx, ctx.man, entries, os.path.join(rdir, 'ev'), os.path.join(rdir, 'diffs'))
    rep['changes'] = entries
    rep['restore'] = acts
    rem = verify(ctx, ctx.man)
    rep['remaining'] = rem
    rep['set_clean'] = not rem
    if not rem:
        ctx.dirty = False
    rep['residue'] = residue(ctx, ctx.man, ctx.a.port)
    rep['result'] = 'CLEAN' if rep['set_clean'] and not rep['residue'] else 'NOT-CLEAN'
    rep['finished'] = iso()
    n = dict((k, sum(1 for e in entries if e['kind'] == k)) for k in ('changed', 'new', 'deleted', 'unreadable'))
    LOG('   changes: changed %(changed)d, new %(new)d, deleted %(deleted)d, unreadable %(unreadable)d' % n +
        ', folders +%d/-%d' % (sum(1 for e in entries if e['kind'] in ('dirnew', 'rootnew')),
                               sum(1 for e in entries if e['kind'] in ('dirgone', 'rootgone'))))
    for e in entries:
        LOG('     %-10s %-58s %s' % (e['kind'], e['real'], e['class']))
    if is_r1:
        LOG('   round 1 check against the expected boot list: %d unexpected' % len(rep['unexpected']))
    LOG('   restore: %d action(s), %d failed; re-verify: %d difference(s) left' % (
        len(acts), sum(1 for x in acts if not x['ok']), len(rem)))
    for r in rep['residue']:
        LOG('   residue: %s %s %s' % (r['kind'], r['what'], r.get('origin', '')))
    write_report(rep)
    LOG('=== ROUND %s RESULT: %s%s' % (rep['round'], rep['result'],
                                       '' if rep['result'] == 'CLEAN' else
                                       ' (files %s; %d residue item(s))' % ('CLEAN' if rep['set_clean'] else 'NOT restored',
                                                                            len(rep['residue']))))


def write_report(rep):
    rdir = rep['rdir']
    json_dump(rep, os.path.join(rdir, 'report.json'))
    L = ['# Q50 guard round %s -- %s' % (rep['round'], rep['title']), '',
         '- result: **%s**%s' % (rep.get('result'), (' (aborted: %s)' % rep['aborted']) if rep.get('aborted') else ''),
         '- spec: %s %s' % (SPEC, rep['cite']),
         '- started %s, finished %s' % (rep['started'], rep.get('finished', '-'))]
    if rep.get('test_root'):
        L.append('- TEST MODE: every D:\\ path below is under %s\\D\\' % rep['test_root'])
    ex = rep.get('exe') or {}
    L.append('- exe: %s (copied from %s, modified %s, sha256 %s); label %s' % (
        ex.get('copy'), ex.get('source'), ex.get('mtime'), ex.get('sha256'), rep.get('label') or '-'))
    s = rep['serve']
    L.append('- wb_serve: `%s`' % subprocess.list2cmdline(s['cmd']))
    L.append('- env set: %s; removed from the inherited env: %s' % (
        ', '.join('%s=%s' % kv for kv in s['env_set'].items()), ', '.join(s['env_removed']) or '-'))
    L.append('- boot %s s, SIM %s, stop %s, rc %s, %s s total; console %s' % (
        s.get('boot_s'), s.get('sim'), s.get('stop'), s.get('rc'), s.get('elapsed_s'), s['console']))
    L += ['', '## Probes', '', '| # | probe | result | rc | s | output |', '|---|---|---|---|---|---|']
    for i, p in enumerate(rep['probes'], 1):
        L.append('| %d | %s | %s | %s | %s | %s |' % (i, p['script'], p.get('status'), p.get('rc'), p.get('secs'),
                                                      p.get('out') or p.get('skip') or ''))
        for x in p.get('fail') or []:
            L.append('|  |  | `%s` |  |  |  |' % x.replace('|', '/')[:200])
    ch = rep['changes']
    L += ['', '## Changes the run made (%d)' % len(ch), '', '| kind | path | class | detail |', '|---|---|---|---|']
    for e in ch:
        L.append('| %s | %s | %s | %s |' % (e['kind'], e['real'], e['class'],
                                            (e.get('detail') or e.get('err') or '').replace('|', '/')[:300]))
    if rep['round'] == 'r1':
        L += ['', '## Round 1 check (expected boot list, %d unexpected)' % len(rep['unexpected'])]
        L += ['- UNEXPECTED %s %s' % (e['kind'], e['real']) for e in rep['unexpected']] or ['- none']
    bad = [a for a in rep['restore'] if not a['ok']]
    L += ['', '## Restore: %d action(s), %d failed' % (len(rep['restore']), len(bad))]
    L += ['- FAILED %s %s %s' % (a['action'], a['real'], a['detail']) for a in bad]
    L += ['', '## Re-verify: %s' % ('every file and folder = baseline' if rep.get('set_clean') else
                                    '%d difference(s) left' % len(rep['remaining']))]
    L += ['- LEFT %s %s' % (e['kind'], e['real']) for e in rep['remaining']]
    L += ['', '## Residue outside the backup set (%d)' % len(rep['residue'])]
    L += ['- %s %s %s' % (r['kind'], r['what'], r.get('origin', '')) for r in rep['residue']] or ['- none']
    with open(os.path.join(rdir, 'report.md'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(L) + '\n')


# ------------------------------------------------------------------------------------------------------ commands
def _round_matches(r, w):
    rid = w if w.startswith('r') else 'r' + w
    return r['id'] == rid or (rid[1:].isdigit() and r['group'] == int(rid[1:]))


def select_rounds(a):
    if a.all:
        sel = list(ROUNDS)
    else:
        want = [w.strip().lower() for x in (a.round or []) for w in x.split(',') if w.strip()]
        if not want:
            raise GuardError('give --round N (1-4 or a sub-round id such as 3c) or --all')
        bad = [w for w in want if not any(_round_matches(r, w) for r in ROUNDS)]
        if bad:
            raise GuardError('unknown round(s): %s' % bad)
        sel = [r for r in ROUNDS if any(_round_matches(r, w) for w in want)]
    if a.probes:
        keep = set(x.strip() for x in a.probes.split(',') if x.strip())
        unknown = keep - set(PROBES)
        if unknown:
            raise GuardError('unknown probe name(s): %s (known: %s)' % (sorted(unknown), ', '.join(PROBES)))
        sel = [dict(r, probes=[p for p in r['probes'] if p in keep]) for r in sel]
        sel = [r for r in sel if r['probes'] or r['id'] == 'r1']
    return sel


def exe_info(path):
    st = os.stat(path)
    sha, _ = sha256_of(path)
    return {'source': os.path.abspath(path), 'mtime': iso(st.st_mtime), 'size': st.st_size, 'sha256': sha}


def prepare(ctx, rounds):
    a, rd = ctx.a, ctx.rundir
    info = exe_info(a.wb_serve)
    os.makedirs(os.path.join(rd, 'bin'))
    ctx.exe = os.path.join(rd, 'bin', os.path.basename(a.wb_serve))
    shutil.copy2(a.wb_serve, ctx.exe)
    info['copy'] = ctx.exe
    if sha256_of(ctx.exe)[0] != info['sha256']:
        raise GuardError('copied exe does not verify')
    ctx.exe_info = info
    LOG('exe: %s (modified %s, sha256 %s) -> %s' % (info['source'], info['mtime'], info['sha256'][:16], ctx.exe))
    src = a.web_src or ctx.P(WEB_SRC)
    ctx.webcopy = os.path.join(rd, 'web')
    if os.path.isdir(src):
        t0 = time.time()
        shutil.copytree(src, ctx.webcopy)
        LOG('web copy: %s -> %s (%.0f s)' % (src, ctx.webcopy, time.time() - t0))
    else:
        raise GuardError('web folder %s not found' % src)
    ctx.pwbook = os.path.join(rd, 'pwbook.txt')
    with open(ctx.pwbook, 'w', encoding='ascii', newline='') as f:
        f.write(PWBOOK_LINE + '\r\n')
    ctx.levelset = os.path.join(rd, 'q41_levelset.dat')
    if any(r['env'] == 'probe' for r in rounds):
        gen = os.path.join(ctx.probe_dir, 'q41_open_gate_probe.py')
        r = subprocess.run([PY, gen, 'make-levelset', ctx.levelset], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           timeout=120, cwd=rd)
        LOG('levelset: %s' % r.stdout.decode('utf-8', 'replace').strip())
        if r.returncode != 0 or not os.path.isfile(ctx.levelset) or os.path.getsize(ctx.levelset) != 1024:
            raise GuardError('make-levelset did not produce a 1024-byte %s' % ctx.levelset)


def check_r1_gate(ctx, rounds, sha):
    if rounds[0]['id'] == 'r1' or ctx.a.skip_r1_check:
        return None
    p = r1_result_path(ctx, sha)
    try:
        j = json.load(open(p, encoding='utf-8'))
    except (OSError, ValueError):
        return 'round 1 has not passed for this exe (sha256 %s...): no %s -- run --round 1 first' % (sha[:8], p)
    if j.get('verdict') != 'pass':
        return 'round 1 for this exe ended "%s" (%s): Steven decides before rounds 2-4 (%s)' % (
            j.get('verdict'), j.get('when'), p)
    return None


def cmd_run(a):
    global CTX
    ctx = CTX = Ctx(a)
    rounds = select_rounds(a)
    if not a.wb_serve:
        raise GuardError('--wb-serve <exe> is required')
    why = preflight(ctx)
    if not why:
        g = check_r1_gate(ctx, rounds, sha256_of(a.wb_serve)[0] or '')
        if g:
            why.append(g)
    if why:
        LOG('REFUSED (nothing touched):')
        for w in why:
            LOG('  - ' + w)
        return 2
    base = ctx.P(RUN_BASE)
    os.makedirs(base, exist_ok=True)
    ctx.rundir = uniq_dir(os.path.join(base, time.strftime('%Y%m%d_%H%M%S')))
    os.makedirs(ctx.rundir)
    try:
        marker_create(ctx, ctx.rundir)
    except FileExistsError:
        LOG('REFUSED: %s appeared meanwhile (another guard?)' % ctx.P(MARKER))
        return 2
    LOG.open(os.path.join(ctx.rundir, 'guard.log'))
    signal.signal(signal.SIGINT, on_sigint)
    LOG('wbrun_guard %s: rounds %s; run folder %s%s' % (iso(), ' '.join(r['id'] for r in rounds), ctx.rundir,
                                                     ('; TEST ROOT ' + ctx.space.root) if ctx.test else ''))
    reps, code = [], 0
    try:
        prepare(ctx, rounds)
        ctx.man = snapshot(ctx)
        for rnd in rounds:
            if ctx.stop_flag:
                break
            rep = run_round(ctx, rnd)
            reps.append(rep)
            if not rep['set_clean']:
                code = 5
                break
            if not rep.get('wb_serve_started') and (rep.get('aborted') or '').startswith('preflight'):
                code = 2
                break
            if rnd['id'] == 'r1':
                verdict = 'pass' if not rep['unexpected'] and not rep['aborted'] else (
                    'unexpected' if rep['unexpected'] else 'aborted')
                json_dump({'verdict': verdict, 'when': iso(), 'exe': ctx.exe_info, 'rundir': ctx.rundir,
                           'unexpected': [e['real'] for e in rep['unexpected']]},
                          r1_result_path(ctx, ctx.exe_info['sha256']))
                if rep['unexpected']:
                    LOG('STOP: round 1 changed file(s) outside the expected boot list -- restored; report to Steven, '
                        'do not run round 2 (spec §6.2)')
                    code = 4
                    break
            if rep['aborted']:
                code = 3
                break
            if not ctx.stop_flag:
                marker_update(ctx, 'between rounds (files verified back)')
    except GuardError as x:
        LOG('ERROR: %s' % x)
        code = code or 1
    except Exception:              # noqa: BLE001
        LOG('INTERNAL ERROR:\n' + traceback.format_exc())
        code = code or 1
    finally:
        if ctx.dirty:               # wb_serve ran and no verify has said every file is back (NOT-CLEAN or a crash)
            code = 5
            marker_update(ctx, 'NOT-CLEAN -- run --restore-only')
            LOG('NOT-CLEAN: marker %s kept.  Next: %s' % (ctx.P(MARKER), json.load(open(ctx.P(MARKER), encoding='utf-8'))['restore']))
        else:
            marker_remove(ctx, ctx.rundir)
        summary(ctx, reps)
    if code == 0:
        if any(r['result'] != 'CLEAN' for r in reps):
            code = 7
        elif any(p.get('status') != 'PASS' for r in reps for p in r['probes']):
            code = 6
        elif ctx.stop_flag:
            code = 3
    LOG('exit %d' % code)
    return code


def summary(ctx, reps):
    if not ctx.rundir or not os.path.isdir(ctx.rundir):
        return
    rows = []
    for r in reps:
        for p in r['probes']:
            rows.append({'round': r['round'], 'probe': p['script'], 'status': p.get('status'), 'rc': p.get('rc'),
                         'secs': p.get('secs'), 'fail': p.get('fail') or [], 'out': p.get('out'),
                         'console': r['serve']['console']})
    j = {'rundir': ctx.rundir, 'exe': ctx.exe_info, 'label': ctx.a.label,
         'rounds': [{'round': r['round'], 'result': r.get('result'), 'aborted': r.get('aborted'),
                     'changes': len(r['changes']), 'unexpected': len(r['unexpected']), 'residue': r['residue']}
                    for r in reps], 'probes': rows}
    json_dump(j, os.path.join(ctx.rundir, 'summary.json'))
    ex = ctx.exe_info or {}
    L = ['# Q50 guard summary -- %s' % ctx.rundir, '',
         '- exe %s (modified %s, sha256 %s), label %s' % (ex.get('source'), ex.get('mtime'), ex.get('sha256'),
                                                          ctx.a.label or '-'), '',
         '| round | result | aborted | changes | unexpected | residue |', '|---|---|---|---|---|---|']
    for r in reps:
        L.append('| %s | %s | %s | %d | %d | %d |' % (r['round'], r.get('result'), r.get('aborted') or '',
                                                      len(r['changes']), len(r['unexpected']), len(r['residue'])))
    L += ['', '| round | probe | result | rc | s | first failure lines | output | console |',
          '|---|---|---|---|---|---|---|---|']
    for x in rows:
        L.append('| %s | %s | %s | %s | %s | %s | %s | %s |' % (
            x['round'], x['probe'], x['status'], x['rc'], x['secs'],
            ' / '.join(x['fail'][:3]).replace('|', '/')[:300], x['out'], x['console']))
    with open(os.path.join(ctx.rundir, 'summary.md'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(L) + '\n')
    LOG('summary: %s' % os.path.join(ctx.rundir, 'summary.md'))


def cmd_restore_only(a):
    global CTX
    ctx = CTX = Ctx(a)
    rd = os.path.abspath(a.restore_only)
    man = load_manifest(rd)
    if (man.get('test_root') or None) != (ctx.space.root or None):
        raise GuardError('manifest was made with --root %r, this call has --root %r' % (man.get('test_root'), ctx.space.root))
    bad = proc_problems(ctx, build=False)
    if bad:
        LOG('REFUSED: a machine-file writer is still running (stop it first): ' + '; '.join(bad))
        return 2
    ctx.man, ctx.rundir = man, rd
    signal.signal(signal.SIGINT, signal.SIG_IGN)       # never half-restore
    rdir = uniq_dir(os.path.join(rd, 'r', 'restore_only_' + time.strftime('%Y%m%d_%H%M%S')))
    os.makedirs(rdir)
    LOG.open(os.path.join(rdir, 'guard.log'))
    LOG('restore-only from %s (backup made %s)' % (rd, man['created']))
    entries, _ = diff_all(ctx, man)
    classify(ctx, entries, [])
    acts = restore(ctx, man, entries, os.path.join(rdir, 'ev'), os.path.join(rdir, 'diffs'))
    rem = verify(ctx, man)
    res = residue(ctx, man, a.port)
    rep = {'round': 'restore-only', 'title': 'emergency restore', 'cite': '§6.8', 'rdir': rdir, 'started': iso(),
           'finished': iso(), 'serve': {'cmd': [], 'env_set': {}, 'env_removed': [], 'console': '-'},
           'exe': man.get('exe'), 'label': man.get('label'), 'test_root': man.get('test_root'), 'probes': [],
           'aborted': None, 'unexpected': [], 'changes': entries, 'restore': acts, 'remaining': rem,
           'residue': res, 'set_clean': not rem, 'result': 'CLEAN' if not rem and not res else 'NOT-CLEAN'}
    write_report(rep)
    LOG('   %d difference(s) found, %d action(s), %d failed, %d left' % (
        len(entries), len(acts), sum(1 for x in acts if not x['ok']), len(rem)))
    for e in rem:
        LOG('   LEFT %s %s' % (e['kind'], e['real']))
    for r in res:
        LOG('   residue: %s %s %s' % (r['kind'], r['what'], r.get('origin', '')))
    if not rem:
        marker_remove(ctx, rd)
    LOG('=== RESTORE-ONLY RESULT: %s' % rep['result'])
    return 0 if not rem and not res else (7 if not rem else 5)


def cmd_verify_only(a):
    ctx = Ctx(a)
    man = load_manifest(os.path.abspath(a.verify_only))
    if (man.get('test_root') or None) != (ctx.space.root or None):
        raise GuardError('manifest was made with --root %r, this call has --root %r' % (man.get('test_root'), ctx.space.root))
    rem = verify(ctx, man)
    for e in rem:
        print('   DIFF %s %s' % (e['kind'], e['real']))
    print('=== VERIFY-ONLY (%s, backup of %s): %s' % (a.verify_only, man['created'], 'CLEAN' if not rem else
                                                     'NOT-CLEAN (%d difference(s))' % len(rem)))
    return 0 if not rem else 5


def cmd_drop_backup(a):
    ctx = Ctx(a)
    rd = os.path.abspath(a.drop_backup)
    man = load_manifest(rd)
    try:
        j = json.load(open(ctx.P(MARKER), encoding='utf-8'))
        if os.path.normcase(j.get('rundir') or '') == os.path.normcase(rd):
            LOG('REFUSED: the marker still points at %s' % rd)
            return 2
    except (OSError, ValueError):
        pass
    rem = verify(ctx, man)
    if rem:
        LOG('REFUSED: %d difference(s) against this backup -- restore first' % len(rem))
        return 5

    def onerr(func, path, exc):
        make_writable(path)
        func(path)
    for sub in ('bk', 'web', 'bin'):
        p = os.path.join(rd, sub)
        if os.path.isdir(p):
            shutil.rmtree(p, onexc=onerr) if sys.version_info >= (3, 12) else shutil.rmtree(p, onerror=onerr)
            LOG('deleted %s' % p)
    LOG('backup dropped; reports kept in %s' % rd)
    return 0


def cmd_dry_plan(a):
    ctx = Ctx(a)
    rounds = select_rounds(a)
    print('wbrun_guard --dry-plan (%s) -- nothing is written' % iso())
    print('mode: %s' % (('TEST, every D:\\ path is under %s\\D\\' % ctx.space.root) if ctx.test else 'REAL (D:\\)'))
    why = preflight(ctx)
    if a.wb_serve and os.path.isfile(a.wb_serve):
        g = check_r1_gate(ctx, rounds, sha256_of(a.wb_serve)[0])
        if g:
            why.append(g)
        ex = exe_info(a.wb_serve)
        print('exe: %s modified %s sha256 %s' % (ex['source'], ex['mtime'], ex['sha256']))
    print('preflight now: %s' % ('OK' if not why else 'WOULD REFUSE'))
    for w in why:
        print('  - ' + w)
    print('\nbackup set (listing only):')
    tot = 0
    for path, kind, cls in ROOTS:
        lst = scan_root(ctx.space, {'path': path, 'kind': kind, 'cls': cls}, hashing=False)
        mb = sum(max(v[2], 0) for v in lst['files'].values()) / 1e6
        tot += mb
        print('  %-34s %-9s %-7s %6d files %8.1f MB' % (path, cls, 'exists' if lst['exists'] else 'ABSENT',
                                                        len(lst['files']), mb))
    print('  total %.1f MB -> %s\\<ts>\\bk ; web copy of %s -> <ts>\\web ; marker %s' % (
        tot, ctx.P(RUN_BASE), a.web_src or ctx.P(WEB_SRC), ctx.P(MARKER)))
    print('  only listed for additions: %s ; %s' % (', '.join(TOPLEVEL), RECYCLE))
    print('\nround 1 expected boot changes (%s §6.2):' % SPEC)
    for pat, kinds, cite in ctx.expected:
        print('  %-44s %-20s %s' % (pat, kinds, cite))
    ctx.rundir = ctx.P(RUN_BASE) + '\\<ts>'
    ctx.webcopy = ctx.rundir + '\\web'
    ctx.pwbook = ctx.rundir + '\\pwbook.txt'
    ctx.levelset = ctx.rundir + '\\q41_levelset.dat'
    ctx.baseline_sha = ctx.rundir + '\\baseline_HT9045.sha256'
    ctx.exe = ctx.rundir + '\\bin\\' + os.path.basename(a.wb_serve or 'wb_serve.exe')
    total = 0
    for rnd in rounds:
        rdir = ctx.rundir + '\\r\\' + rnd['id']
        probes = resolve_probes(ctx, rnd, rdir)
        sec = round_seconds(ctx, rnd, probes)
        env, added, dropped = build_env(ctx, rnd)
        if probes and all(p['skip'] for p in probes):
            print('\n%s  %s (%s)  NOT BOOTED: every probe skipped (%s)' % (rnd['id'], rnd['title'], rnd['cite'],
                                                                         probes[0]['skip']))
            continue
        est = sec + 60 + 240
        total += est
        print('\n%s  %s (%s)  wb_serve --seconds %d, about %d min with backup compare/restore' % (
            rnd['id'], rnd['title'], rnd['cite'], sec, est // 60))
        print('   env: %s   (removed from the inherited env: %s)' % (
            ' '.join('%s=%s' % kv for kv in added.items()), ' '.join(dropped) or '-'))
        print('   serve: %s' % subprocess.list2cmdline(serve_cmd(ctx, sec)))
        for i, p in enumerate(probes, 1):
            print('   probe %d (timeout %d s%s): %s' % (i, p['timeout'], (', SKIP: ' + p['skip']) if p['skip'] else '',
                                                        subprocess.list2cmdline(p['cmd'])))
            if p['declares']:
                print('            may write: %s' % ', '.join(d[0] for d in p['declares']))
    print('\nestimated wall time %d min (plus the backup, 1-3 min) ; stop: %s' % (
        total // 60, 'Ctrl-Break after the probes (--stop-early)' if a.stop_early else 'natural, when --seconds expires'))
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description='Q50 option B guard -- see the README at the top of this file.')
    g = ap.add_mutually_exclusive_group()
    g.add_argument('--round', action='append', help='1 | 2 | 3 | 4 | a sub-round id (r3c / 3c); comma list allowed')
    g.add_argument('--all', action='store_true', help='r1 .. r4g in order (stops on the rules above)')
    g.add_argument('--restore-only', metavar='RUNDIR', help='emergency restore from that run folder\'s backup')
    g.add_argument('--verify-only', metavar='RUNDIR', help='read-only compare against that backup')
    g.add_argument('--drop-backup', metavar='RUNDIR', help='verify, then delete that run\'s bk / web / bin')
    ap.add_argument('--dry-plan', action='store_true', help='print the plan for --round/--all; write nothing')
    ap.add_argument('--wb-serve', help='the SIM wb_serve.exe to run (copied into the run folder first)')
    ap.add_argument('--probes', help='comma list: only these probes (names: see PROBES)')
    ap.add_argument('--probe-args', action='append', help='NAME=ARGS replaces a probe\'s argument list (placeholders ok)')
    ap.add_argument('--offsets-file', help='MinGW LAST_GENERAL_SET offset list for status_towerlight_probe --offsets')
    ap.add_argument('--label', default='', help='free text for the reports, e.g. the commit the exe was built from')
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--user', default='S12TEST')
    ap.add_argument('--password', default='S12PW')
    ap.add_argument('--serve-seconds', type=int, help='override wb_serve --seconds for every round')
    ap.add_argument('--stop-early', action='store_true',
                    help='Ctrl-Break wb_serve right after the probes (skips the golden close saves)')
    ap.add_argument('--probe-timeout', type=int, help='cap every probe timeout (s)')
    ap.add_argument('--boot-timeout', type=int, default=300)
    ap.add_argument('--grace', type=int, default=240, help='s to wait past --seconds before Ctrl-Break')
    ap.add_argument('--settle', type=float, default=3.0, help='s to wait after wb_serve exits before the rescan')
    ap.add_argument('--expect-extra', action='append', help='add a pattern to the round-1 expected list (Steven OK)')
    ap.add_argument('--gate-log-window', type=int, default=360, help='minutes; 0 = do not read gate logs')
    ap.add_argument('--skip-r1-check', action='store_true', help='run rounds 2-4 without a passed round 1 (Steven OK)')
    ap.add_argument('--web-src', help='web folder to copy for --root (default D:\\HT9045\\web)')
    ap.add_argument('--probe-dir', help='folder with the probe scripts (default: this folder)')
    ap.add_argument('--root', help='TEST ONLY: map every D:\\ path under this folder')
    ap.add_argument('--test-skip-build-check', action='store_true', help='TEST ONLY (needs --root)')
    a = ap.parse_args(argv)
    if a.test_skip_build_check and not a.root:
        ap.error('--test-skip-build-check is only allowed with --root')
    try:
        if a.restore_only:
            return cmd_restore_only(a)
        if a.verify_only:
            return cmd_verify_only(a)
        if a.drop_backup:
            return cmd_drop_backup(a)
        if a.dry_plan:
            return cmd_dry_plan(a)
        return cmd_run(a)
    except GuardError as x:
        LOG('ERROR: %s' % x)
        return 1


if __name__ == '__main__':
    sys.exit(main())
