# 產生 JSON 的 <script> 傳輸墊片：Edge/Chromium 在 file:// 下封鎖 XHR/fetch，
# script 標籤不受限制。JSON 仍是唯一資料來源，本檔僅為傳輸備援（自動生成，勿手改）。
import json
import os

SRC = r"D:\HT9045\JSON"
OUT = os.path.join(SRC, "js")

FILES = [
    "Machine-type-index.json",
    "IO-config.json",
    "IO-runtime.json",
    "Motor-config.json",
    "Motor-runtime.json",
    "MotionView-i18n.json",
    "Teach-config.json",
    "motor-access.json",
    "motor-access-ack.json",
    "teach-access.json",
    "Sim-scale.json",
    "state-record.json",
    "state-record-ack.json",
    "Task-runtime.json",
    "System-runtime.json",
    "General-config.json",
    "Config.json",
    "Config-help.json",
    "Define-index.json",
    "Security-access.json",
    "Security-access-update.json",
    "Security-visibility-runtime.json",
    "View-rules.json",
    "Setup-index.json",
    "Setup-current.json",
    "Production-runtime.json",
    "Production-update.json",
    "Production-update-ack.json",
    "Main-command-request.json",
    "Main-command-ack.json",
    "Debug-json-log-config.json",
    "Debug-json-log-flush-request.json",
    "Debug-json-log-flush-ack.json",
    "Site-toggle-request.json",
    "Runtime-bridge-contract.json",
    "Dialog-bridge-contract.json",
    "Alarm-dialog-request.json",
    "Alarm-dialog-response.json",
    "Message-dialog-request.json",
    "Message-dialog-response.json",
    "Dialog-close-request.json",
    "Dialog-close-response.json",
    "Dialog-auth-verify.json",
    "Dialog-auth-result.json",
    "Html-load-metrics.json",
    "BinSel-mode-validation.json",
    "Setup-offline-debug.json",
    os.path.join("offline", "SitePanel-runtime.offline.json"),
    os.path.join("offline", "IO-config.offline.json"),
    os.path.join("offline", "IO-runtime.offline.json"),
    os.path.join("offline", "Motor-config.offline.json"),
    os.path.join("offline", "Motor-runtime.offline.json"),
    os.path.join("offline", "Teach-config.offline.json"),
    # 2026-09-09：ht9045-html-json 表③剩餘讀寫檔補完轉換（_gen_misc_config_json.py）
    "ARMS-config.json",
    "ContactInfo-config.json",
    "ColorSensorType-config.json",
    "MVData-config.json",
    "AutoTemperature-config.json",
    "Barcode-config.json",
    "PadInterfacePara-config.json",
    "EventLogLevel-config.json",
    "TrayStepSpeed-config.json",
    "SpecialErrNote-config.json",
    "TrayForm-config.json",
    "PlateForm-config.json",
    "ATC-config.json",
    "DioCfg-index.json",
    "Security-def.json",
    "CriticalParaControl-config.json",
    "ESDconfig-config.json",
    "RPDefault-config.json",
    "SocketCount-config.json",
    "Offset-index.json",
    "SaveByMachine-index.json",
    "DeviceCorrespond-config.json",
    "AlarmCodeList-index.json",
    "AlarmDescription-config.json",
    "AlarmDescriptionOverride-index.json",
    "SecsGem-config.json",
    "PMAlarm-config.json",
    "ProductionInfo-config.json",
    "NSKit-flag.json",
    "SitMap-config.json",
    "LastSet-projection-contract.json",
]


def main():
    os.makedirs(OUT, exist_ok=True)
    made = []
    for rel in FILES:
        src = os.path.join(SRC, rel)
        if not os.path.exists(src):
            print("skip (missing):", rel)
            continue
        with open(src, "r", encoding="utf-8-sig") as f:
            data = json.load(f)
        key = os.path.basename(rel)[:-5]          # 去掉 .json
        dst = os.path.join(OUT, key + ".js")
        body = json.dumps(data, ensure_ascii=False, separators=(",", ":"))
        with open(dst, "w", encoding="utf-8") as f:
            f.write("window.__HT9045_DATA__=window.__HT9045_DATA__||{};\n")
            f.write('window.__HT9045_DATA__["%s"]=%s;\n' % (key, body))
        made.append((key, len(body)))
        print("ok:", key, len(body))
    print("total:", len(made))


if __name__ == "__main__":
    main()
