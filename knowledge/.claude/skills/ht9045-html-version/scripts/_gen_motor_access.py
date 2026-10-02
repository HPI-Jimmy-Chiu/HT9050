# 由 uteach.cpp 抽出 TECH_PARA / TECH_TWOPARA / TECH_MotorAxle，並產生馬達動作指令協定 JSON：
#   motor-access.json      HTML→C++ 指令（互斥，一次一筆）
#   motor-access-ack.json  C++→HTML 完成回報
#   teach-access.json      Teaching Set/Go 按鈕對照表
import json
import os
import re
from datetime import datetime

CPP = (r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"
       r"\uteach.cpp")
OUT = r"D:\HT9045\JSON"

RE_AXLE = re.compile(
    r"TechMotorAxle\.push_back\(\s*new\s+TECH_MotorAxle\(\s*(\w+)\s*,\s*(\w+)")
RE_PARA = re.compile(
    r"TechPara\.push_back\(\s*new\s+TECH_PARA\(\s*&([\w.\[\]]+)\s*,\s*(\w+)\s*,"
    r"\s*(\w+)\s*,\s*\"([^\"]+)\"\s*,\s*(\w+)\s*,\s*(\w+)")
RE_TWO = re.compile(
    r"TechTwoPara\.push_back\(\s*new\s+TECH_TWOPARA\(\s*&([\w.\[\]]+)\s*,\s*&([\w.\[\]]+)\s*,"
    r"\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*\"([^\"]+)\"\s*,\s*\"([^\"]+)\"\s*,"
    r"\s*(\w+)\s*,\s*(\w+)")

NOW = datetime.now().astimezone().isoformat(timespec="seconds")


def nn(v):
    """NULL / 空字串 → None"""
    return None if v in ("NULL", "", None) else v


# 指令目錄：kind=motion 者執行中必須鎖定其他按鈕，只留 btnStop
COMMANDS = [
    # ---- uMotorTest / Panel20 ----
    dict(source="uMotorTest", button="sbMotorTest_JogP", action="jogP", kind="motion",
         trigger="mousedown", release="stop", params=["motorId", "speed"]),
    dict(source="uMotorTest", button="sbMotorTest_JogN", action="jogN", kind="motion",
         trigger="mousedown", release="stop", params=["motorId", "speed"]),
    dict(source="uMotorTest", button="sbMotorTest_MoveP", action="moveRelative", kind="motion",
         trigger="click", params=["motorId", "speed", "currentPos", "interval", "targetPos"]),
    dict(source="uMotorTest", button="sbMotorTest_MoveN", action="moveRelative", kind="motion",
         trigger="click", params=["motorId", "speed", "currentPos", "interval", "targetPos"]),
    dict(source="uMotorTest", button="btnGo", action="moveAbsolute", kind="motion",
         trigger="click", params=["motorId", "speed", "targetPos"]),
    dict(source="uMotorTest", button="btnGoSoftP", action="moveSoftLimitP", kind="motion",
         trigger="click", params=["motorId", "speed", "targetPos"]),
    dict(source="uMotorTest", button="btnGoSoftN", action="moveSoftLimitN", kind="motion",
         trigger="click", params=["motorId", "speed", "targetPos"]),
    dict(source="uMotorTest", button="btnHome", action="home", kind="motion",
         trigger="click", params=["motorId", "speed"]),
    dict(source="uMotorTest", button="btnLoopMove", action="loopMove", kind="motion",
         trigger="click", params=["motorId", "speed", "pos1", "pos2", "waitTime"]),
    dict(source="uMotorTest", button="btnStop", action="stop", kind="control",
         trigger="click", params=["motorId"], allowedWhileBusy=True),
    dict(source="uMotorTest", button="btnServoOff", action="servoToggle", kind="control",
         trigger="click", params=["motorId", "servoOn"]),
    dict(source="uMotorTest", button="btnMotorPower", action="motorPowerToggle", kind="control",
         trigger="click", params=["powerOn"]),
    dict(source="uMotorTest", button="btnSetPosP", action="setPos1", kind="edit",
         trigger="click", params=["motorId", "currentPos"]),
    dict(source="uMotorTest", button="btnSetPosN", action="setPos2", kind="edit",
         trigger="click", params=["motorId", "currentPos"]),
    # ---- uMotorTest / Panel23（參數設定；BCB6 btnXxxClick：讀現值→寫 P* 參數→UpdateMotorParameter）----
    dict(source="uMotorTest", button="btnHighSpeed", action="setJogHighSpeed", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnLowSpeed", action="setJogLowSpeed", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnHomeHigh", action="setHomeHighSpeed", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnHomeLow", action="setHomeLowSpeed", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnSoftPPos", action="setSoftLimitP", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnSoftNPos", action="setSoftLimitN", kind="edit",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnRange", action="refreshParameter", kind="edit",
         trigger="click", params=["motorId"]),
    dict(source="uMotorTest", button="btnRate", action="refreshParameter", kind="edit",
         trigger="click", params=["motorId"]),
    dict(source="uMotorTest", button="btnSetRange", action="setRangeAndInit", kind="control",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnSetRate", action="setRateAndInit", kind="control",
         trigger="click", params=["motorId", "value"]),
    dict(source="uMotorTest", button="btnReloadMotorData", action="reloadMotorData", kind="control",
         trigger="click", params=[]),
    dict(source="uMotorTest", button="btResetMNet", action="resetMNet", kind="control",
         trigger="click", params=["confirm"], requiresSystemStop=True),
    # ---- uteach / pnlMotion ----
    dict(source="uteach", button="btnJogP", action="jogP", kind="motion",
         trigger="mousedown", release="stop", params=["motorId", "speed"]),
    dict(source="uteach", button="btnJogN", action="jogN", kind="motion",
         trigger="mousedown", release="stop", params=["motorId", "speed"]),
    dict(source="uteach", button="btnMoveP", action="moveRelative", kind="motion",
         trigger="click", params=["motorId", "speed", "currentPos", "interval", "targetPos"]),
    dict(source="uteach", button="btnMoveN", action="moveRelative", kind="motion",
         trigger="click", params=["motorId", "speed", "currentPos", "interval", "targetPos"]),
    dict(source="uteach", button="btnMoveTo", action="moveAbsolute", kind="motion",
         trigger="click", params=["motorId", "speed", "targetPos"]),
    dict(source="uteach", button="btnHome", action="home", kind="motion",
         trigger="click", params=["motorId", "speed"]),
    dict(source="uteach", button="btnStop", action="stop", kind="control",
         trigger="click", params=["motorId"], allowedWhileBusy=True),
    dict(source="uteach", button="btnServo", action="servoToggle", kind="control",
         trigger="click", params=["motorId", "servoOn"]),
    dict(source="uteach", button="btnSetTo", action="setTeachFromCurrent", kind="edit",
         trigger="click", params=["motorId", "currentPos", "edit"]),
    dict(source="uteach", button="btnSetToOffset", action="setTeachFromOffset", kind="edit",
         trigger="click", params=["motorId", "offset", "edit"]),
    # ---- uteach / TechPara・TechTwoPara（實際按鈕由 teach-access.json 展開）----
    dict(source="uteach", button="SetButton*", action="teachSet", kind="edit",
         trigger="click", params=["motorIds", "edits", "keys", "currentPos"]),
    dict(source="uteach", button="GoButton*", action="teachGo", kind="motion",
         trigger="click", params=["motorIds", "speed", "targetPos"]),
]


def idle_request():
    return {
        "seq": 0, "id": None, "source": None, "button": None, "action": None,
        "kind": "idle", "motors": [], "params": {},
        "issuedAt": None, "state": "idle"
    }


def main():
    with open(CPP, "r", encoding="cp950", errors="replace") as f:
        src = f.read()

    axle = [{"button": b, "motorId": m} for m, b in RE_AXLE.findall(src)]

    para = []
    for param, motor, edit, key, setb, gob in RE_PARA.findall(src):
        para.append({
            "setButton": nn(setb), "goButton": nn(gob),
            "motorIds": [motor], "edits": [edit], "keys": [key],
            "params": [param], "type": "TECH_PARA"
        })

    two = []
    for p1, p2, m1, m2, e1, e2, k1, k2, setb, gob in RE_TWO.findall(src):
        two.append({
            "setButton": nn(setb), "goButton": nn(gob),
            "motorIds": [m1, m2], "edits": [e1, e2], "keys": [k1, k2],
            "params": [p1, p2], "type": "TECH_TWOPARA"
        })

    teach = {
        "schemaVersion": "1.0.0",
        "generatedAt": NOW,
        "source": {"toolchain": "BCB6", "cpp": CPP.replace("\\", "/")},
        "summary": {"motorAxle": len(axle), "techPara": len(para), "techTwoPara": len(two)},
        "motorAxle": axle,
        "techPoints": para + two,
    }

    access = {
        "schemaVersion": "1.0.0",
        "generatedAt": NOW,
        "protocol": {
            "requestFile": "JSON/motor-access.json",
            "ackFile": "JSON/motor-access-ack.json",
            "mutex": True,
            "busyPolicy": "kind=motion 執行中鎖定所有按鈕，只有 allowedWhileBusy=true（btnStop）可操作",
            "unlockOn": ["done", "error", "aborted"],
            "ackPollMs": 200,
            "offlineAutoAckMs": 700
        },
        "commands": COMMANDS,
        "request": idle_request(),
    }

    ack = {
        "schemaVersion": "1.0.0",
        "generatedAt": NOW,
        "ack": {
            "seq": 0, "id": None, "state": "idle", "result": "",
            "message": "", "motorId": None,
            "position": {"cmdPos": None, "encPos": None, "targetPos": None},
            "completedAt": None
        },
    }

    os.makedirs(OUT, exist_ok=True)
    for name, data in (("teach-access", teach), ("motor-access", access),
                       ("motor-access-ack", ack)):
        with open(os.path.join(OUT, name + ".json"), "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=1)
        print("ok:", name + ".json")

    print("motorAxle=%d techPara=%d techTwoPara=%d commands=%d"
          % (len(axle), len(para), len(two), len(COMMANDS)))


if __name__ == "__main__":
    main()
