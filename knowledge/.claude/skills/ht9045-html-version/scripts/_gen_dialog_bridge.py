"""Generate bidirectional ShowErrorMessage/ShowMyMessage bridge skeletons."""
import json
import os


ROOT = r"D:\HT9045"
OUTPUT = os.path.join(ROOT, "JSON")
SOURCE_ROOT = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2"

ACTION_CODES = [
    ("RETRY", 0x0001),
    ("SKIP", 0x0002),
    ("CLEAN_OUT", 0x0004),
    ("TRAY_FEED", 0x0008),
    ("TRAY_END", 0x0010),
    ("RESET", 0x0020),
    ("HOME", 0x0040),
    ("TRAIN", 0x0080),
    ("FIX", 0x0100),
    ("ONECYCLE", 0x0200),
    ("PAUSE", 0x0400),
    ("START", 0x0800),
]


def source(files, symbols):
    return {
        "toolchain": "BCB6",
        "encoding": "CP950/Big5",
        "root": SOURCE_ROOT,
        "files": files,
        "symbols": symbols,
    }


def write(name, value):
    path = os.path.join(OUTPUT, name)
    with open(path, "w", encoding="utf-8", newline="\n") as target:
        json.dump(value, target, ensure_ascii=False, indent=2)
        target.write("\n")
    print("ok:", path)


def auth_block():
    # C++ 在發佈 request 時已知是否需密碼（fSecurity->GetJamLevel / bNeedPassWord / bMBoxNeedPassword / bAlarmUnlockPassWord）
    return {
        "required": False,
        "kind": "none",          # access-level | unlock-password | special-note | employee-id | mbox-password
        "level": None,
        "title": "Password",
        "prompt": "",
        "userIdRequired": True,
        "defaultUserId": None,
    }


def auth_result_block():
    return {
        "required": False,
        "verified": False,
        "authId": None,
        "accessLevel": None,
        "bypassedByCloseRequest": False,
    }


def auth_verify():
    return {
        "schemaVersion": "1.0.0",
        "channel": "dialog-auth",
        "seq": 0,
        "authId": "",
        "state": "idle",
        "requestedAt": None,
        "target": {"channel": "", "requestId": "", "requestSeq": 0},
        "kind": "access-level",
        "level": None,
        "pendingAction": {"name": "NONE", "code": None, "pressedButton": None},
        "credentials": {"userId": None, "password": ""},
    }


def auth_result():
    return {
        "schemaVersion": "1.0.0",
        "channel": "dialog-auth",
        "seq": 0,
        "authId": "",
        "state": "idle",
        "accepted": False,
        "accessLevel": None,
        "userId": None,
        "message": None,
        "verifiedAt": None,
        "error": None,
    }


def alarm_request():
    return {
        "schemaVersion": "1.0.0",
        "channel": "show-error-message",
        "seq": 0,
        "requestId": "",
        "state": "idle",
        "requestedAt": None,
        "function": "ShowErrorMessage",
        "blocking": True,
        "arguments": {
            "code": "",
            "kCode": 0,
            "position": 0,
            "duplicateError": False,
            "errorPart": "",
        },
        "display": {
            "alarmType": None,
            "unitName": "",
            "message": "",
            "jamArea": "",
            "description": "",
            "flushPanel": None,
        },
        "auth": auth_block(),
        "buttons": [],
        "closePolicy": "acknowledge-only",
        "error": None,
    }


def alarm_response():
    return {
        "schemaVersion": "1.0.0",
        "channel": "show-error-message",
        "seq": 0,
        "requestId": "",
        "requestSeq": 0,
        "state": "idle",
        "accepted": False,
        "selectedAction": {"name": "NONE", "code": 0},
        "pressedButton": None,
        "auth": auth_result_block(),
        "closedBy": None,
        "completedAt": None,
        "durationMs": None,
        "error": None,
    }


def message_request():
    return {
        "schemaVersion": "1.0.0",
        "channel": "show-my-message",
        "seq": 0,
        "requestId": "",
        "state": "idle",
        "requestedAt": None,
        "function": "ShowMyMessage",
        "blocking": True,
        "arguments": {
            "s1": "",
            "s2": "",
            "s3": "",
            "ok": False,
            "servoOff": False,
        },
        "display": {
            "primaryText": "",
            "secondaryText": "",
            "buttonLabel": "Pause",
            "showAlarmReset": False,
            "buttonEnabled": True,
        },
        "runtime": {
            "secsGemAlarm": False,
            "haltHandler": False,
            "systemInitialOK": False,
            "employeeIdCheck": False,
        },
        "requestedSideEffects": {
            "pauseHandler": True,
            "stopAllMotor": False,
            "servoOffInArmXY": False,
        },
        "auth": auth_block(),
        "error": None,
    }


def message_response():
    return {
        "schemaVersion": "1.0.0",
        "channel": "show-my-message",
        "seq": 0,
        "requestId": "",
        "requestSeq": 0,
        "state": "idle",
        "accepted": False,
        "selectedAction": "NONE",
        "auth": auth_result_block(),
        "closedBy": None,
        "completedAt": None,
        "durationMs": None,
        "sideEffects": {
            "handlerPaused": None,
            "motorsStopped": None,
            "servoOffInArmXY": None,
            "secsGemEventReported": None,
            "formCloseCleanupCompleted": None,
        },
        "error": None,
    }


def close_request():
    return {
        "schemaVersion": "1.0.0",
        "channel": "dialog-close",
        "seq": 0,
        "closeRequestId": "",
        "state": "idle",
        "requestedAt": None,
        "target": {
            "channel": "",
            "requestId": "",
            "requestSeq": 0,
        },
        "trigger": {
            "source": "io",
            "inputName": "",
            "detectedAt": None,
        },
        "resolvedAction": {
            "name": "NONE",
            "code": None,
        },
        "closeReason": "external-io",
        "error": None,
    }


def close_response():
    return {
        "schemaVersion": "1.0.0",
        "channel": "dialog-close",
        "seq": 0,
        "closeEventId": "",
        "closeRequestId": None,
        "closeRequestSeq": None,
        "state": "idle",
        "accepted": False,
        "target": {
            "channel": "",
            "requestId": "",
            "requestSeq": 0,
        },
        "trigger": {
            "source": "html-action",
            "inputName": None,
        },
        "selectedAction": {
            "name": "NONE",
            "code": None,
        },
        "closedBy": None,
        "dialogWasOpen": False,
        "closedAt": None,
        "normalResponse": {
            "file": "",
            "seq": 0,
        },
        "error": None,
    }


def contract():
    codes = {name: code for name, code in ACTION_CODES}
    return {
        "schemaVersion": "1.2.0",
        "name": "HT9045 modal dialog bridge",
        "source": source(
            ["note.cpp", "note.h", "note.dfm", "mymessbox.cpp", "mymessbox.h", "mymessbox.dfm", "cmydef.cpp"],
            ["ShowErrorMessage", "TfNote::ReturnCode", "TfNote::DoPassword", "TfNote::DoUnlockPassword",
             "ShowMyMessage", "TMyMessageBox::FormClose", "TMyMessageBox::DoPassword_MBox"],
        ),
        "transport": {
            "mailboxPattern": "one request file plus one response file per channel",
            "pollIntervalMs": 100,
            "jsonEncoding": "UTF-8 without BOM",
            "fileWrite": "write .tmp, flush and close, then atomically replace JSON and JSON/js shim",
            "ordering": "seq must increase monotonically; requestId must be unique per invocation",
            "htmlRule": "process only state=pending requests whose seq is newer than the last handled seq",
            "completionRule": "C++ remains blocked until a matching state=completed response is received",
        },
        "displayEnvironment": {
            "machineResolution": {"width": 1920, "height": 1080},
            "mode": "fixed FullHD machine display",
            "responsiveMobileLayoutRequired": False,
            "validationViewports": [{"width": 1920, "height": 1080}],
            "rule": "Do not design or validate phone/tablet layouts; this UI runs only on the fixed machine FullHD display.",
            "pages": {"alarm": "page/Alert.Note.html (972x761, dfm note.dfm)", "message": "page/Alert.MyMessageBox.html (472x219, dfm mymessbox.dfm)"},
            "zOrder": "desktop windows < layout tools 9999 < #dialogBridge 20000 (Alert overlay) < #dialogAuth 21000 (login) < HTQwerty 99999 (keyboard); the Alert always covers every other page",
        },
        "channels": {
            "showErrorMessage": {
                "request": "Alarm-dialog-request.json",
                "response": "Alarm-dialog-response.json",
                "producer": "C++ ShowErrorMessage",
                "consumer": "HTML modal dialog",
                "returnMapping": "response.selectedAction.code is returned unchanged as fNote->ReturnCode",
                "pressedButton": "BtnStart -> fNote::Start() (fMain->Start, SoftStart); BtnPause -> BtnPauseClick (SoftStop). null when closed by C++ close request.",
                "zeroKCode": "show an acknowledge button and return 0",
                "closePolicy": "When kCode is nonzero, browser X/Escape cannot complete the request; an enabled action is required.",
            },
            "showMyMessage": {
                "request": "Message-dialog-request.json",
                "response": "Message-dialog-response.json",
                "producer": "C++ ShowMyMessage",
                "consumer": "HTML modal dialog",
                "returnMapping": "void; response confirms close reason and cleanup completion before C++ continues",
                "buttonMapping": "OK when arguments.ok, runtime.secsGemAlarm, or runtime.haltHandler is true; otherwise Pause",
            },
            "dialogClose": {
                "request": "Dialog-close-request.json",
                "response": "Dialog-close-response.json",
                "producer": "C++ IO/control flow",
                "consumer": "HTML modal dialog",
                "responsibility": "C++ detects and resolves IO; HTML only closes the targeted dialog and acknowledges completion.",
                "normalResponseRule": "HTML also writes the target Alarm/Message response so the blocking C++ function receives its action/result.",
            },
            "dialogAuth": {
                "verify": "Dialog-auth-verify.json",
                "result": "Dialog-auth-result.json",
                "producer": "HTML login layer (verify) / C++ security check (result)",
                "trigger": "request.auth.required==true and operator presses a closing action (Start/Pause/OK); ALARM_RESET never needs auth",
                "kinds": {"access-level": "TfNote::DoPassword -> fSecurity->GetJamLevel vs AccessLevel",
                          "unlock-password": "TfNote::DoUnlockPassword (asUnlockPassword)",
                          "special-note": "TfNote::PanSpecialNoteClick (SpecialErrNote.ini Pwd)",
                          "employee-id": "CheckEmployeeID / SECS GEM S10F3",
                          "mbox-password": "TMyMessageBox::DoPassword_MBox"},
                "verifier": "C++ only. HTML never compares passwords or access levels.",
                "resultRule": "C++ writes state=completed with accepted true/false and the same authId; accepted -> HTML writes the normal response and closes; rejected -> dialog stays open, password cleared, operator may retry (palWrongPW).",
                "timeoutMs": 15000,
                "credentialsHandling": "Plain credentials cross the local bridge only. Prefer WebView2/HTDialogHost.verifyAuth; in file mode C++ must read then immediately overwrite Dialog-auth-verify.json with state=consumed and must never log the password.",
                "closeRequestOverride": "A C++ Dialog-close-request closes the dialog even while auth is pending (auth.bypassedByCloseRequest=true) because C++ already resolved the physical key.",
            },
        },
        "states": {
            "request": ["idle", "pending", "cancelled", "error"],
            "response": ["idle", "completed", "error"],
            "auth": ["idle", "pending", "completed", "consumed", "error"],
            "closedBy": ["action-button", "alarm-reset", "external-io", "host-cancel", "superseded", "error"],
        },
        "alarmActionCodes": codes,
        "alarmButtonRule": "For every bit present in arguments.kCode, publish one button with that exact code and name.",
        "responseCorrelation": ["requestId", "requestSeq"],
        "closeCorrelation": ["closeRequestId", "closeRequestSeq", "target.requestId", "target.requestSeq"],
        "requiredResponseFields": [
            "seq", "requestId", "requestSeq", "state", "accepted", "selectedAction",
            "closedBy", "completedAt", "durationMs", "error",
        ],
        "failureRules": [
            "Never reuse an old response for a new request.",
            "Never unblock ShowErrorMessage with an action code not enabled by kCode.",
            "Do not close the HTML modal until response submission succeeds or the host explicitly cancels it.",
            "A malformed request produces a correlated state=error response when requestId and seq are readable.",
            "HTML must not read, poll, debounce, or interpret machine IO; C++ supplies trigger and resolvedAction.",
            "A close request must match the active target requestId and requestSeq before HTML closes the dialog.",
            "Dialog-close-response.json is emitted after the dialog is visually closed for both HTML actions and C++ close requests.",
            "When request.auth.required is true, the normal response is written only after Dialog-auth-result accepted=true (or after a C++ close request).",
            "The Alert overlay must stay above every other page; only the login layer and the keyboard may appear above it.",
        ],
    }


def main():
    os.makedirs(OUTPUT, exist_ok=True)
    write("Dialog-bridge-contract.json", contract())
    write("Alarm-dialog-request.json", alarm_request())
    write("Alarm-dialog-response.json", alarm_response())
    write("Message-dialog-request.json", message_request())
    write("Message-dialog-response.json", message_response())
    write("Dialog-close-request.json", close_request())
    write("Dialog-close-response.json", close_response())
    write("Dialog-auth-verify.json", auth_verify())
    write("Dialog-auth-result.json", auth_result())


if __name__ == "__main__":
    main()