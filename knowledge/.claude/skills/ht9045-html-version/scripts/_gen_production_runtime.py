"""Generate the HT9045 production snapshot consumed by the HTML simulator."""
import csv
import ctypes
import hashlib
import json
import os
import struct
from datetime import datetime


ROOT = r"D:\HT9045"
SYSTEM = os.path.join(ROOT, "system")
CONFIG = os.path.join(ROOT, "config", "config.ini")
SETUP_CURRENT = os.path.join(ROOT, "JSON", "Setup-current.json")
GENERAL = os.path.join(ROOT, "JSON", "General-config.json")
OUTPUT = os.path.join(ROOT, "JSON", "Production-runtime.json")
UPDATE_OUTPUT = os.path.join(ROOT, "JSON", "Production-update.json")
UPDATE_ACK_OUTPUT = os.path.join(ROOT, "JSON", "Production-update-ack.json")
BRIDGE_CONTRACT_OUTPUT = os.path.join(ROOT, "JSON", "Runtime-bridge-contract.json")
ROWS = 4
COLS = 8
MAX_BINS = 256
MAX_X_ITEM = 30
MAX_Y_ITEM = 70
MAX_MGZ_TRAY = 14
ARM_FORMATS = {
    2432: {"valuesPerSite": 19, "fixedBinCount": 15},
    12800: {"valuesPerSite": 100},
    13312: {"valuesPerSite": 104},
    33280: {"valuesPerSite": MAX_BINS + 4},
}

ARM_GROUPS = {
    "current": "Arm",
    "history": "ArmHis",
    "byLot": "ArmByLot",
    "autoClean": "ArmAutoClean",
}

Int32 = ctypes.c_int32
Bool8 = ctypes.c_uint8
TrayMatrix = (Int32 * MAX_Y_ITEM) * MAX_X_ITEM
SocketMatrix = (Int32 * COLS) * ROWS
HotMatrix = ((Int32 * 50) * 50) * 2


class MachineRecord(ctypes.Structure):
    _fields_ = [
        ("bInitialStart", Bool8), ("Start", Int32), ("fHasTray", Bool8 * 20),
        ("TrayData", TrayMatrix * 20), ("TrayXItem", Int32 * 20), ("TrayYItem", Int32 * 20),
        ("FLCarryKitItem", SocketMatrix), ("FRCarryKitItem", SocketMatrix),
        ("BLCarryKitItem", SocketMatrix), ("BRCarryKitItem", SocketMatrix),
        ("SortCarryKitItem", SocketMatrix), ("FTestSuckItem", SocketMatrix),
        ("BTestSuckItem", SocketMatrix), ("InArmSuckItem", SocketMatrix),
        ("OutArmSuckItem", SocketMatrix), ("TestSocketItem", SocketMatrix),
        ("CatchTrayItem", Int32), ("iWhichSht", Int32), ("iWhichKit", Int32), ("iInArmOrder", Int32),
        ("iPickPlate", Int32 * 2), ("iPlatePickX", Int32 * 2), ("iPlatePickY", Int32 * 2),
        ("iPlacePlate", Int32 * 2), ("iPlatePlaceX", Int32 * 2), ("iPlatePlaceY", Int32 * 2),
        ("iBackInArmHotCount", Int32), ("iHotCount", Int32),
        ("iHotPlateCount", HotMatrix), ("iHotInArmOrder", HotMatrix),
        ("iHotWhichKit", HotMatrix), ("iHotWhichShuttle", HotMatrix),
        ("iInRotateUnit", Int32), ("iOutRotateUnit", Int32),
        ("bPlaceToHotplate", Bool8), ("bPickFromHotplate", Bool8),
        ("bSiteMappingCHKOK", Bool8), ("iKyecSiteMapStatus", Int32 * 32),
        ("bBackupCleanOut", Bool8), ("iHotRecBuf", HotMatrix), ("LoaderBuf", TrayMatrix),
        ("iRowOnHotPlate", HotMatrix), ("iWhichSite", TrayMatrix * 20),
        ("fHasMagazineTray", Bool8 * MAX_MGZ_TRAY), ("MagazineTrayData", TrayMatrix * MAX_MGZ_TRAY),
        ("MagazineTrayXItem", Int32 * 20), ("MagazineTrayYItem", Int32 * 20),
        ("iAuto3MagazineIndex", Int32), ("iBinData", TrayMatrix * 3), ("iWhichAuto", TrayMatrix * 3),
        ("fHasTray_6", Bool8 * 9), ("TrayData_6", TrayMatrix * 9),
        ("TrayXItem_6", Int32 * 9), ("TrayYItem_6", Int32 * 9),
        ("iWhichSite_6", TrayMatrix * 9), ("iBinData_6", TrayMatrix * 3),
        ("iWhichAuto_6", TrayMatrix * 3), ("cTrayID", (ctypes.c_char * 2048) * 20),
    ]


class LastSetObserverPrefix(ctypes.Structure):
    _fields_ = [
        ("LastOpenFilename", ctypes.c_char * 512), ("iAutoTeachStep", Int32), ("SendCT", Int32 * 4),
        ("handlerCounters", Int32 * 12), ("fullTrayCount", Int32 * 12),
        ("partialTrayCount", Int32 * 12), ("partialTrayICCount", Int32 * 12),
        ("inHandlerICCount", Int32 * 12), ("SystemDateRecord", ctypes.c_char * 512),
        ("SystemAccSecond", (Int32 * 8) * 4),
    ]


class LastSetObserverCounters(ctypes.Structure):
    _fields_ = LastSetObserverPrefix._fields_ + [
        ("MessageLight", (Int32 * 3) * 8), ("MusicSelect", Int32 * 9), ("bMusicEnable", Bool8),
        ("lMaxAlarmCT", Int32), ("lMaxMessageCT", Int32), ("lMaxUserCT", Int32),
        ("iLanguageCountry", Int32), ("bShowCountType", Bool8),
        ("iTemperature", Int32), ("iStartMode", Int32), ("iTester", Int32), ("iScanner", Int32),
        ("iRealDummy", Int32), ("bUT150InstallReserved", Bool8 * 8), ("TrayLoaderSpeed_Old", Int32 * 12),
        ("SoakTime", ctypes.c_double), ("JamSoakTime", ctypes.c_double),
        ("InitialWaitTime", ctypes.c_double), ("CollingTime", ctypes.c_double),
        ("LoopMovePos", Int32 * 180), ("Index1Torue", ctypes.c_float * 12),
        ("ContactSet", Int32 * 8), ("ContactCountReserved", Int32 * 28),
        ("Hot_time", ctypes.c_double), ("ShuttleSpeed2", Int32 * 17),
        ("BinCT", (ctypes.c_uint32 * 256) * 4), ("BinCT_ART", (ctypes.c_uint32 * 256) * 4),
        ("bUseOutArmCheckIndex", Bool8),
        ("fInArmSuckDeleyTime", ctypes.c_double), ("fInArmDestroyDeleyTime", ctypes.c_double),
        ("fIndexArmSuckDeleyTime", ctypes.c_double), ("fIndexArmDestroyDeleyTime", ctypes.c_double),
        ("fOutArmSuckDeleyTime", ctypes.c_double), ("fOutArmDestroyDeleyTime", ctypes.c_double),
        ("bUseFinishHomeCanGo", Bool8), ("bUT150Install", Bool8 * 20),
        ("iUnloadFixTray", Int32), ("iCCDPurgeSetupCount", Int32),
        ("iCCDPurgeSetupTime", ctypes.c_double), ("iCCDPurgeRunCount", Int32),
        ("BinCT_Old", (Int32 * 20) * 4), ("bHT8040_SACNNER", Int32),
        ("bBigFan", Bool8), ("bInArmPlaceToShuttleCheck", Bool8), ("bC03UseCatchTray", Bool8),
        ("iShuttlePitchMode", Int32), ("bPasswordCanInputByMouse", Bool8), ("iTemptureRange", Int32),
        ("bD41TestSocketICCheckSkip", Bool8), ("bTemperatureSelectByDevice", Bool8),
        ("iScannerSelect", Int32), ("bAutoAdjustTestZDown", Bool8),
        ("iWaitIndexDestroyTime", ctypes.c_double),
        ("bClearContactOffset", Bool8), ("bClearPickOffset", Bool8), ("bTesterFinishCanHome", Bool8),
        ("bIndexICFallDownMustPressFMotorDown", Bool8), ("bUseSingleTenmpertureLimit", Bool8),
        ("iSingleTempLimit", Int32 * 20), ("iOutArmPickPlaceInterval", Int32),
        ("bCheckIndexICDestroy", Bool8), ("iTestHeadCheckVacuumTime", ctypes.c_double),
        ("bEnableSiteModeSelect", Bool8), ("bShakeShuttleWhenJam", Bool8),
        ("iInArmQuickModeWaitTime", Int32), ("bOutputShuttleSkipICMiss", Bool8),
        ("bLoaderTraySplitFailCanSkip", Bool8), ("HeadTestCT", Int32 * 192),
        ("bHT8080PlaceToShuttleDualDropModeAndUp", Bool8), ("bRotateInMachine", Bool8),
        ("bC02InstallCCD", Bool8), ("szSupervisor", ctypes.c_char * 32),
        ("iInOutArmZSpeedScale", Int32), ("StartDelayTime", ctypes.c_double),
        ("bEnableReadTorque", Bool8), ("iReadTorqueTimeCount", Int32), ("iRTMUpdateMin", Int32),
        ("bInitialICCheck", Bool8), ("bCheckOutputShuttleContiFail", Bool8),
        ("iCheckOutputShuttleContiFailCount", Int32), ("bResetGPIBAfterOneCycleCleanOut", Bool8),
        ("bEnableAutoTorqueOfRunTime", Bool8), ("iAutoTorqueOfRunTimeCount", Int32),
        ("bEnableFinishTestUpWait", Bool8), ("iFinishTestUpWaitHeight", Int32),
        ("iD21FinishTestUpWaitTime", ctypes.c_double), ("iTTLPulseWidth", Int32),
        ("bUseOutPutSHJamSetInterFaceErrorBin", Bool8), ("iShuttleShakeSpeed", Int32),
        ("iMachineModel", Int32), ("InOutArmZSafe", Int32),
        ("bTesterTimerOutNotNeedReTest", Bool8), ("bEnableMainScreenShowAlarmUse", Bool8),
        ("iCCDTypeSelect", Int32), ("bAutoSwitchToOperatorMode", Bool8),
        ("bManualHeightComptibleWithNS", Bool8), ("iJamCount", Int32 * 3),
    ]


def read_cp950(path):
    with open(path, "r", encoding="cp950", errors="replace") as source:
        return source.read()


def read_ini_sections(path):
    sections = {}
    current = None
    for line in read_cp950(path).splitlines():
        text = line.strip()
        if not text or text.startswith(";") or text.startswith("#"):
            continue
        if text.startswith("[") and text.endswith("]"):
            current = text[1:-1]
            sections.setdefault(current, {})
        elif current is not None and "=" in text:
            key, value = text.split("=", 1)
            sections[current][key.strip()] = value.strip()
    return sections


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as source:
        for chunk in iter(lambda: source.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def sparse_matrix(matrix):
    return [[row, col, int(matrix[row][col])]
            for row in range(len(matrix)) for col in range(len(matrix[row]))
            if int(matrix[row][col]) != 0]


def matrix_values(matrix):
    return [[int(value) for value in row] for row in matrix]


def decode_c_string(value):
    return bytes(value).split(b"\0", 1)[0].decode("cp950", errors="replace")


def format_msec_time(milliseconds):
    milliseconds = max(0, int(milliseconds))
    days, remainder = divmod(milliseconds, 86400000)
    hours, remainder = divmod(remainder, 3600000)
    minutes, remainder = divmod(remainder, 60000)
    seconds, millis = divmod(remainder, 1000)
    return "%04d days %02d:%02d:%02d.%03d" % (days, hours, minutes, seconds, millis)


def format_second_spc(seconds):
    seconds = max(0, int(seconds))
    hours = (seconds // 3600) % 24
    minutes = (seconds // 60) % 60
    return "%02d:%02d:%02d" % (hours, minutes, seconds % 60)


def choose_arm_file(name):
    primary = os.path.join(SYSTEM, name + ".dat")
    backup = os.path.join(SYSTEM, name + "_backup.dat")
    if not os.path.isfile(primary):
        return backup if os.path.isfile(backup) else None, "backup"
    if os.path.isfile(backup) and sha256(primary) != sha256(backup):
        return backup, "backup-differs"
    return primary, "primary"


def choose_data_file(name):
    primary = os.path.join(SYSTEM, name + ".dat")
    backup = os.path.join(SYSTEM, name + "_backup.dat")
    if not os.path.isfile(primary):
        return (backup, "backup") if os.path.isfile(backup) else (None, "missing")
    if os.path.isfile(backup) and sha256(primary) != sha256(backup):
        return backup, "backup-differs"
    return primary, "primary"


def machine_record_snapshot(path, spare):
    expected_size = ctypes.sizeof(MachineRecord)
    if not os.path.isfile(path):
        return {"available": False, "spare": spare, "source": {"file": path}, "error": "file missing"}
    actual_size = os.path.getsize(path)
    source_info = {"toolchain": "BCB6", "file": path, "size": actual_size, "expectedSize": expected_size,
                   "sha256": sha256(path), "reader": "LoadMachineRecord(bool)", "encoding": "binary"}
    if actual_size != expected_size:
        return {"available": False, "spare": spare, "source": source_info,
                "error": "MachineRecord size does not match current BCB6 structure"}
    with open(path, "rb") as source:
        record = MachineRecord.from_buffer_copy(source.read())

    trays = [{
        "index": index, "hasTray": bool(record.fHasTray[index]),
        "xItem": int(record.TrayXItem[index]), "yItem": int(record.TrayYItem[index]),
        "trayId": decode_c_string(record.cTrayID[index]),
        "data": sparse_matrix(record.TrayData[index]),
        "whichSite": sparse_matrix(record.iWhichSite[index]),
    } for index in range(20)]
    extra_trays = [{
        "index": index, "hasTray": bool(record.fHasTray_6[index]),
        "xItem": int(record.TrayXItem_6[index]), "yItem": int(record.TrayYItem_6[index]),
        "data": sparse_matrix(record.TrayData_6[index]),
        "whichSite": sparse_matrix(record.iWhichSite_6[index]),
    } for index in range(9)]
    magazines = [{
        "index": index, "hasTray": bool(record.fHasMagazineTray[index]),
        "xItem": int(record.MagazineTrayXItem[index]), "yItem": int(record.MagazineTrayYItem[index]),
        "data": sparse_matrix(record.MagazineTrayData[index]),
    } for index in range(MAX_MGZ_TRAY)]
    hot_cells = []
    for plate in range(2):
        for row in range(50):
            for col in range(50):
                values = [int(record.iHotPlateCount[plate][row][col]), int(record.iHotInArmOrder[plate][row][col]),
                          int(record.iHotWhichKit[plate][row][col]), int(record.iHotWhichShuttle[plate][row][col]),
                          int(record.iHotRecBuf[plate][row][col]), int(record.iRowOnHotPlate[plate][row][col])]
                if any(values):
                    hot_cells.append([plate, row, col] + values)
    return {
        "available": True, "spare": spare, "source": source_info,
        "initialStart": bool(record.bInitialStart), "restoreEligible": bool(record.bInitialStart),
        "start": int(record.Start),
        "trays": trays, "extraTrays": extra_trays, "magazines": magazines,
        "kits": {name: matrix_values(getattr(record, name)) for name in
                 ("FLCarryKitItem", "FRCarryKitItem", "BLCarryKitItem", "BRCarryKitItem", "SortCarryKitItem",
                  "FTestSuckItem", "BTestSuckItem", "InArmSuckItem", "OutArmSuckItem", "TestSocketItem")},
        "loaderBuffer": sparse_matrix(record.LoaderBuf),
        "hotPlate": {"fields": ["plate", "row", "col", "count", "inArmOrder", "whichKit", "whichShuttle", "recordBuffer", "armRow"],
                     "cells": hot_cells, "backInArmHotCount": int(record.iBackInArmHotCount),
                     "hotCount": int(record.iHotCount), "placeToHotplate": bool(record.bPlaceToHotplate),
                     "pickFromHotplate": bool(record.bPickFromHotplate)},
        "routing": {"whichShuttle": int(record.iWhichSht), "whichKit": int(record.iWhichKit),
                    "inArmOrder": int(record.iInArmOrder), "pickPlate": list(record.iPickPlate),
                    "platePickX": list(record.iPlatePickX), "platePickY": list(record.iPlatePickY),
                    "placePlate": list(record.iPlacePlate), "platePlaceX": list(record.iPlatePlaceX),
                    "platePlaceY": list(record.iPlatePlaceY)},
        "state": {"catchTrayItem": int(record.CatchTrayItem), "inRotateUnit": int(record.iInRotateUnit),
                  "outRotateUnit": int(record.iOutRotateUnit), "siteMappingCheckOk": bool(record.bSiteMappingCHKOK),
                  "kyecSiteMapStatus": list(record.iKyecSiteMapStatus), "backupCleanOut": bool(record.bBackupCleanOut),
                  "auto3MagazineIndex": int(record.iAuto3MagazineIndex)},
        "magazineRouting": {"binData": [sparse_matrix(item) for item in record.iBinData],
                            "whichAuto": [sparse_matrix(item) for item in record.iWhichAuto],
                            "binData6": [sparse_matrix(item) for item in record.iBinData_6],
                            "whichAuto6": [sparse_matrix(item) for item in record.iWhichAuto_6]},
    }


def parse_machine_records():
    return {"normal": machine_record_snapshot(os.path.join(SYSTEM, "machinerecord.dat"), False),
            "spare": machine_record_snapshot(os.path.join(SYSTEM, "machinerecordRealCCD.dat"), True)}


def parse_lastset_observer_prefix():
    path, selected = choose_data_file("lastdata")
    prefix_size = ctypes.sizeof(LastSetObserverCounters)
    if not path or os.path.getsize(path) < prefix_size:
        return {"available": False, "source": {"file": path, "minimumSize": prefix_size}}
    with open(path, "rb") as source:
        prefix = LastSetObserverCounters.from_buffer_copy(source.read(prefix_size))
    times = {"start": int(prefix.SystemAccSecond[0][0]), "pause": int(prefix.SystemAccSecond[0][1]),
             "powerOn": int(prefix.SystemAccSecond[0][2]), "product": int(prefix.SystemAccSecond[0][3]),
             "jam": int(prefix.SystemAccSecond[0][4]), "home": int(prefix.SystemAccSecond[0][5]),
             "contactTest": int(prefix.SystemAccSecond[0][6]), "systemNG": int(prefix.SystemAccSecond[0][7])}
    send_count = list(prefix.SendCT)
    jam_count = list(prefix.iJamCount)
    active_seconds = (times["pause"] + times["product"] + times["jam"]) // 1000
    if jam_count[1] == 0:
        mtba = "0 / " + format_second_spc(active_seconds)
        muba = "0 / %d unit" % send_count[1]
    else:
        mtba = "1 / " + format_second_spc(active_seconds // jam_count[1]) if active_seconds else "%d / 0" % jam_count[1]
        muba = "1 / %d unit" % (send_count[1] // jam_count[1]) if send_count[1] else "%d / 0 unit" % jam_count[1]
    return {
        "available": True,
        "source": {"toolchain": "BCB6", "file": path, "selected": selected,
                   "size": os.path.getsize(path), "prefixSize": prefix_size, "sha256": sha256(path),
                   "reader": "ReadLastDataFile()"},
        "lastOpenFilename": decode_c_string(prefix.LastOpenFilename),
        "sendCount": send_count,
        "jamCount": jam_count,
        "systemAccMs": times,
        "panels": {"powerOnTime": format_msec_time(times["powerOn"]),
                   "runningTime": format_msec_time(times["start"]),
                   "productTime": format_msec_time(times["product"]),
                   "loadingCount": send_count[1], "muba": muba, "mtba": mtba,
                   "mtbf": format_second_spc(times["powerOn"] // 1000), "dayJamRate": None},
    }


def parse_observer_panels(observer_record):
    with open(GENERAL, "r", encoding="utf-8") as source:
        general = json.load(source)
    quick = general.get("quick", {})
    runtime_only = {"value": "", "persisted": False,
                    "reason": "Populated only after runtime communication; no shutdown record exists."}
    return {
        "source": {"toolchain": "BCB6", "functions": ["TfObserver::GetMachineData()", "ProcessRunInfo()", "ShowVer()"]},
        "operating": observer_record.get("panels", {}) if observer_record.get("available") else {},
        "version": {"model": quick.get("model", ""), "serialNo": quick.get("serialNo", ""),
                    "machineId": quick.get("machineId", ""), "factory": quick.get("factory", ""),
                    "handlerVersion": quick.get("version", ""), "releaseDate": "",
                    "gpibVersion": dict(runtime_only), "esdVersion": dict(runtime_only),
                    "atcVersion": dict(runtime_only), "ttlRs232Version": dict(runtime_only)},
    }


def site_name(row, col):
    return chr(ord("A") + row) + chr(ord("a") + col)


def read_yield_monitor_ini(name):
    path = os.path.join(SYSTEM, name + ".ini")
    if not os.path.isfile(path):
        return {"available": False, "file": path}
    sections = read_ini_sections(path)
    result = {}
    for section in ("iByBinLowYieldPass", "iByBinArmYieldPass", "iByBinSiteYieldPass"):
        result[section] = [
            int(sections.get(section, {}).get("Site%d-%d" % (row, col), "0") or 0)
            for row in range(ROWS) for col in range(COLS)
        ]
    return {"available": True, "file": path, "sections": result}


def parse_arm(name, recipe_bin_count):
    path, selected = choose_arm_file(name)
    if not path:
        return {"name": name, "available": False}
    size = os.path.getsize(path)
    arm_format = ARM_FORMATS.get(size)
    if not arm_format:
        return {
            "name": name,
            "available": False,
            "source": {"file": path, "selected": selected, "size": size},
            "error": "unsupported TArm size; expected one of %s bytes" % sorted(ARM_FORMATS),
        }
    with open(path, "rb") as source:
        values = struct.unpack("<%dI" % (size // 4), source.read())

    sites = []
    total_pass = 0
    total_fail = 0
    total_if_error = 0
    total_bins = [0] * MAX_BINS
    valid = True
    for row in range(ROWS):
        for col in range(COLS):
            values_per_site = arm_format["valuesPerSite"]
            offset = (row * COLS + col) * values_per_site
            record = values[offset:offset + values_per_site]
            passed, failed, stored_total = record[0:3]
            persisted_bin_count = arm_format.get("fixedBinCount", min(recipe_bin_count, values_per_site - 4))
            bins = list(record[3:3 + persisted_bin_count]) + [0] * (MAX_BINS - persisted_bin_count)
            if_error = record[persisted_bin_count + 3]
            computed_total = passed + failed
            bins_total = sum(bins)
            total_consistent = stored_total == computed_total
            bin_accounting_complete = bins_total + if_error == computed_total
            valid = valid and total_consistent
            total_pass += passed
            total_fail += failed
            total_if_error += if_error
            total_bins = [left + right for left, right in zip(total_bins, bins)]
            sites.append({
                "row": row,
                "col": col,
                "name": site_name(row, col),
                "pass": passed,
                "fail": failed,
                "total": computed_total,
                "storedTotal": stored_total,
                "ifError": if_error,
                "binCounts": bins,
                "passYield": (float(passed) / computed_total) if computed_total else 0.0,
                "consistent": total_consistent,
                "totalConsistent": total_consistent,
                "binAccountingComplete": bin_accounting_complete,
            })

    total = total_pass + total_fail
    return {
        "name": name,
        "available": True,
        "source": {
            "toolchain": "BCB6",
            "file": path,
            "selected": selected,
            "size": size,
            "sha256": sha256(path),
            "format": "uint32[4][8][%d] little-endian" % arm_format["valuesPerSite"],
            "persistedBinCount": arm_format.get("fixedBinCount", min(recipe_bin_count, arm_format["valuesPerSite"] - 4)),
            "layout": "pass,fail,total,bin[persistedBinCount],ifError; remaining values reserved",
        },
        "summary": {
            "pass": total_pass,
            "fail": total_fail,
            "total": total,
            "ifError": total_if_error,
            "binCounts": total_bins,
            "passYield": (float(total_pass) / total) if total else 0.0,
            "consistent": valid,
            "binAccountingComplete": all(site["binAccountingComplete"] for site in sites),
        },
        "sites": sites,
        "yieldMonitorCounters": read_yield_monitor_ini(name),
    }


def parse_lot_summary():
    path = os.path.join(SYSTEM, "LotSummary.csv")
    with open(path, "r", encoding="ascii", errors="strict", newline="") as source:
        rows = [[int(value or 0) for value in row] for row in csv.reader(source)]
    valid = len(rows) == ROWS * COLS + 1 and all(len(row) == MAX_BINS for row in rows)
    if not valid:
        return {"available": False, "source": {"file": path}, "error": "expected 33 rows x 256 bins"}
    sites = [{"site": index, "name": site_name(index // COLS, index % COLS), "binCounts": rows[index]}
             for index in range(ROWS * COLS)]
    computed = [sum(row[bin_index] for row in rows[:ROWS * COLS]) for bin_index in range(MAX_BINS)]
    return {
        "available": True,
        "source": {"toolchain": "BCB6", "file": path, "reader": "TLotSummary::ReadFile()/WriteFile()"},
        "sites": sites,
        "totalCategory": rows[-1],
        "computedTotalCategory": computed,
        "consistent": computed == rows[-1],
    }


def get_setup_context():
    if not os.path.isfile(SETUP_CURRENT):
        return {"recipeName": None, "binCount": MAX_BINS, "siteMap": []}
    with open(SETUP_CURRENT, "r", encoding="utf-8") as source:
        setup = json.load(source)
    documents = setup.get("documents", {})
    tester = documents.get("tester", {}).get("sections", {}).get("RS-232C", {})
    bin_entry = tester.get("Bin Count", {})
    bin_count = int(bin_entry.get("value", MAX_BINS) or MAX_BINS)
    configuration = documents.get("handlerCondition", {}).get("sections", {}).get("Configuration", {})
    site_map = []
    for row in range(ROWS):
        for col in range(COLS):
            key = "Site " + site_name(row, col)
            entry = configuration.get(key, {})
            number = int(entry.get("value", row * COLS + col + 1) or 0)
            site_map.append({"row": row, "col": col, "name": site_name(row, col), "siteNumber": number})
    return {"recipeName": setup.get("recipeName"), "binCount": min(max(bin_count, 1), MAX_BINS), "siteMap": site_map}


def derive_test_category(arms, context):
    bin_count = context["binCount"]
    arm0 = arms[0]
    arm1 = arms[1]
    sites = []
    total_category = [0] * bin_count
    total_socket = 0
    pass_socket = 0
    reject_count = 0
    for mapping in context["siteMap"]:
        index = mapping["row"] * COLS + mapping["col"]
        arm_sites = [arm0["sites"][index], arm1["sites"][index]]
        bins = [sum(site["binCounts"][bin_index] for site in arm_sites) for bin_index in range(bin_count)]
        if_error = sum(site["ifError"] for site in arm_sites)
        socket_total = sum(site["total"] for site in arm_sites)
        socket_pass = sum(site["pass"] for site in arm_sites)
        total_category = [left + right for left, right in zip(total_category, bins)]
        total_socket += socket_total
        pass_socket += socket_pass
        reject_count += if_error
        sites.append({
            **mapping,
            "arms": [{"arm": arm_index, "pass": site["pass"], "fail": site["fail"], "total": site["total"],
                      "ifError": site["ifError"], "binCounts": site["binCounts"][:bin_count]}
                     for arm_index, site in enumerate(arm_sites)],
            "pass": socket_pass,
            "fail": socket_total - socket_pass,
            "total": socket_total,
            "ifError": if_error,
            "binCounts": bins,
            "passYield": (float(socket_pass) / socket_total) if socket_total else 0.0,
        })
    return {
        "source": "Derived from ArmData[0..1], matching TEST_CATEGORY::UpdataCount(true) for non-NN physical sockets",
        "strategy": "physical-socket",
        "binCount": bin_count,
        "sites": sites,
        "totalCategory": total_category,
        "totalSocket": total_socket,
        "passSocket": pass_socket,
        "failSocket": total_socket - pass_socket,
        "rejectCount": reject_count,
        "passYield": (float(pass_socket) / total_socket) if total_socket else 0.0,
        "failYield": (float(total_socket - pass_socket) / total_socket) if total_socket else 0.0,
    }


def parse_lot_info():
    sections = read_ini_sections(CONFIG)
    lot = sections.get("Lot Info", {})
    rfid = sections.get("RFID", {})
    return {
        "source": {"toolchain": "BCB6", "file": CONFIG, "encoding": "cp950",
                   "readers": ["TfLotInfo::ReadWriteLotInfo(bool)", "TfLotInfo::SetLotID(AnsiString,bool)"]},
        "fields": lot,
        "rfid": rfid,
        "quick": {
            "lotId": lot.get("Lot ID") or lot.get("Lot No") or "",
            "lotNo": lot.get("Lot No") or lot.get("Lot ID") or "",
            "customerLotId": lot.get("Customer Lot ID", ""),
            "startTime": lot.get("Start Time", ""),
            "endTime": lot.get("End Time", ""),
            "lotStartTime": lot.get("LotStartTime", ""),
            "operator": lot.get("Operator", ""),
            "customer": lot.get("Customer", ""),
            "testerId": lot.get("Tester ID", ""),
            "testProgram": lot.get("Test Program", ""),
            "deviceName": lot.get("Device Name", ""),
            "runMode": lot.get("Run Mode 2DID") or lot.get("Run Mode", ""),
            "stage": lot.get("Stage", ""),
            "step": lot.get("Step", ""),
            "eventLogFile": lot.get("EventLogFile", ""),
        },
    }


def production_stream_contracts():
    return {
        "socketCounters": {
            "available": False,
            "updateClass": "production-event",
            "trigger": "TMySocket::SetTesterBin() / ClearALLCT()",
            "updatedAt": None,
            "seq": 0,
            "sites": [],
            "siteShape": {
                "identity": ["armIndex", "row", "col", "siteName"],
                "lifetime": ["pass", "fail", "total", "ifError", "binCounts"],
                "bySiteWindow": ["pass", "fail", "total"],
                "byBinMonitor": ["lowYieldPass", "armYieldPass", "siteYieldPass"],
            },
            "note": "Full in-memory TMySocket counters; ClearALLCT publishes zero values for the affected scope.",
        },
        "testStatus": {
            "available": False,
            "updateClass": "production-event",
            "trigger": "ProcessShowTestStatus(int Index)",
            "updatedAt": None,
            "seq": 0,
            "arms": [],
            "siteShape": ["row", "col", "item", "bin", "pass", "hasDevice", "color"],
            "colorValues": ["green", "red", "silver", "white"],
            "note": "Current tester result display state; this is not the accumulated TEST_CATEGORY count.",
        },
        "continuousFail": {
            "available": False,
            "updateClass": "production-event",
            "trigger": "CheckContinuoussFail(int Index)",
            "updatedAt": None,
            "seq": 0,
            "socket": [],
            "arms": [],
            "thresholds": {},
            "alarms": [],
            "fields": ["count", "autoCleanCount", "specialBinCount", "lastBin", "alarmTriggered", "autoCleanTriggered"],
            "note": "Runtime-only consecutive-failure windows for socket, arm, AutoClean, and special-bin decisions.",
        },
        "yieldHistory": {
            "available": False,
            "updateClass": "production-event",
            "trigger": "tester Test Complete; HistroyBin/HistroyPassFail shift",
            "updatedAt": None,
            "seq": 0,
            "rows": [],
            "targets": ["mtRowA", "mtRowB", "mtRowC", "mtRowD"],
            "note": "Runtime-only data. No persisted source exists; do not substitute DFM demo values.",
        },
        "testTiming": {
            "available": False,
            "updateClass": "production-event",
            "trigger": "ProcessRunInfo() after completed production cycle",
            "updatedAt": None,
            "seq": 0,
            "columns": ["site", "startTime", "endTime", "testTime", "indexCycleTime", "indexTime"],
            "rows": [],
            "target": "TimeInfoGrid",
            "note": "Runtime-only Now/Last 1..9/Average data; unavailable before the first completed cycle.",
        },
        "temperature": {
            "available": False,
            "updateClass": "controller-poll",
            "trigger": "temperature controller read; ShowThermo() consumes UN150Read[]",
            "configuredIntervalMs": None,
            "recommendedIntervalMs": 250,
            "updatedAt": None,
            "seq": 0,
            "channels": {},
            "note": "Runtime-only controller data. Poll interval is supplied by the production bridge.",
        },
    }


def runtime_bridge_contract():
    return {
        "schemaVersion": "1.0.0",
        "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
        "source": {"toolchain": "BCB6", "owner": "C++ runtime bridge"},
        "delivery": {
            "mode": "atomic-replace-full-snapshot",
            "pollIntervalMs": 250,
            "sequence": "seq must increase monotonically; eventId must be unique",
            "reason": "A full mutable snapshot prevents lost state when file polling skips intermediate events.",
            "fileTransport": "C++ must atomically replace both JSON and JSON/js shim for file:// delivery.",
            "writerSteps": ["write .tmp", "flush and close", "atomic replace target", "repeat for shim"],
        },
        "immutableStartupOnly": ["Production-runtime.json.machineRecord"],
        "mutableProductionSections": [
            "context", "lotInfo", "observerRecord", "observerPanels", "arms", "lotSummary",
            "testCategory", "productionStreams",
        ],
        "files": {
            "productionUpdate": {"path": "JSON/Production-update.json", "shimPath": "JSON/js/Production-update.js", "producer": "C++", "consumer": "HTML"},
            "productionAck": {"path": "JSON/Production-update-ack.json", "producer": "HTML", "consumer": "C++", "optional": True, "condition": "debug writer authorized"},
            "taskRuntime": {"path": "JSON/Task-runtime.json", "producer": "C++", "consumer": "HTML", "pollIntervalMs": 1000},
            "systemRuntime": {"path": "JSON/System-runtime.json", "producer": "C++", "consumer": "HTML", "pollIntervalMs": 2000},
        },
        "eventTypes": {
            "tester-bin": {"after": "TMySocket::SetTesterBin(int)", "updates": ["arms", "testCategory", "productionStreams.socketCounters"]},
            "test-status": {"after": "ProcessShowTestStatus(int)", "updates": ["testCategory", "productionStreams.testStatus"]},
            "continuous-fail": {"after": "CheckContinuoussFail(int)", "updates": ["observerRecord", "productionStreams.continuousFail"]},
            "clear-count": {"after": "TMySocket::ClearALLCT() / TArm::ClearALLCT()", "updates": ["arms", "testCategory", "productionStreams"]},
            "test-complete": {"after": "ProcessArmCount() and yield/history processing", "updates": ["observerRecord", "observerPanels", "arms", "lotSummary", "testCategory", "productionStreams"]},
            "lot-change": {"after": "Lot Start/End or Lot Info change", "updates": ["context", "lotInfo", "arms", "lotSummary", "testCategory"]},
            "setup-change": {"after": "Setup/Recipe change", "updates": ["context", "arms", "testCategory"]},
            "temperature-poll": {"after": "temperature controller read", "updates": ["productionStreams.temperature"]},
        },
        "eventEnvelope": {
            "required": ["seq", "eventId", "eventType", "trigger", "affected", "occurredAt"],
            "optionalContext": ["armIndex", "row", "col", "binValue", "clearScope"],
            "clearScopeValues": ["socket", "arm", "all-arms", "lot", "yield-window", "all"],
            "dateTimeFormat": "ISO-8601 with timezone",
        },
    }


def production_update(data):
    return {
        "schemaVersion": "1.0.0",
        "generatedAt": data["generatedAt"],
        "source": {"toolchain": "BCB6", "owner": "C++ runtime bridge"},
        "protocol": {
            "contractFile": "JSON/Runtime-bridge-contract.json",
            "ackFile": "JSON/Production-update-ack.json",
            "fullSnapshot": True,
            "machineRecordIncluded": False,
        },
        "event": {
            "seq": 0, "eventId": None, "eventType": "idle", "trigger": None,
            "armIndex": None, "row": None, "col": None, "binValue": None,
            "clearScope": None, "affected": [], "occurredAt": None,
        },
        "state": {key: data[key] for key in (
            "context", "lotInfo", "observerRecord", "observerPanels", "arms", "lotSummary",
            "testCategory", "productionStreams",
        )},
    }


def production_update_ack(generated_at):
    return {
        "schemaVersion": "1.0.0",
        "generatedAt": generated_at,
        "source": {"toolchain": "HTML", "consumer": "Production-update.json"},
        "ack": {
            "seq": 0, "eventId": None, "state": "idle", "appliedAt": None,
            "message": None, "errors": [],
        },
    }


def write_json(path, value):
    with open(path, "w", encoding="utf-8", newline="\n") as target:
        json.dump(value, target, ensure_ascii=False, indent=2)
        target.write("\n")


def generate():
    context = get_setup_context()
    arm_groups = {}
    for group, prefix in ARM_GROUPS.items():
        arm_groups[group] = [parse_arm(prefix + str(index), context["binCount"]) for index in range(3)]
    current_valid = all(arm.get("available") and arm.get("summary", {}).get("consistent")
                        for arm in arm_groups["current"][:2])
    by_lot_valid = all(arm.get("available") and arm.get("summary", {}).get("consistent")
                       for arm in arm_groups["byLot"][:2])
    observer_record = parse_lastset_observer_prefix()
    data = {
        "schemaVersion": "1.3.0",
        "generatedAt": datetime.now().astimezone().isoformat(timespec="seconds"),
        "source": {"toolchain": "BCB6", "modules": ["cSocket.cpp", "cObserver.cpp", "uLotInfo.cpp", "cinitial.cpp", "LastSet.h"]},
        "updatePolicy": {
            "startupOnly": ["machineRecord"],
            "eventMutable": ["context", "lotInfo", "observerRecord", "observerPanels", "arms", "lotSummary", "testCategory", "productionStreams"],
            "operatorEvent": "Setup/Recipe or Lot data change",
            "productionEvent": "tester Test Complete or completed production cycle",
            "controllerPoll": "temperature bridge interval; recommended 250 ms",
        },
        "runtime": {"connected": False, "snapshot": True, "lastPollAt": None, "seq": 0},
        "context": context,
        "lotInfo": parse_lot_info(),
        "machineRecord": parse_machine_records(),
        "observerRecord": observer_record,
        "observerPanels": parse_observer_panels(observer_record),
        "arms": arm_groups,
        "lotSummary": parse_lot_summary(),
        "testCategory": {
            "current": derive_test_category(arm_groups["current"], context) if current_valid else {"available": False},
            "byLot": derive_test_category(arm_groups["byLot"], context) if by_lot_valid else {"available": False},
            "note": "TEST_CATEGORY is runtime-derived and is regenerated from persisted TArm data at snapshot time.",
        },
        "productionStreams": production_stream_contracts(),
    }
    os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
    outputs = {
        OUTPUT: data,
        UPDATE_OUTPUT: production_update(data),
        UPDATE_ACK_OUTPUT: production_update_ack(data["generatedAt"]),
        BRIDGE_CONTRACT_OUTPUT: runtime_bridge_contract(),
    }
    for path, value in outputs.items():
        write_json(path, value)
        print("ok:", path)
    print("lot:", data["lotInfo"]["quick"]["lotId"], "binCount:", context["binCount"],
          "total:", data["testCategory"]["current"].get("totalSocket"),
          "arms valid:", current_valid, "lot summary valid:", data["lotSummary"].get("consistent"))


if __name__ == "__main__":
    generate()