# Debug JSON Command Log

## Scope

`IDE.JsonCommandLog.html` is a Debug-only independent window registered by `background.html` as `jsoncommandlog`. Open it from `Main.html` Debug menu: `JSON Command Log`.

The window polls the current JSON mailbox contents every 500 ms and renders formatted JSON. It additionally receives successful Main/Site request notifications through `background.html`, then adds every unique request or acknowledgement to a persistent browser timeline. The timeline uses `localStorage`, so reloading the Debug window does not clear it.

On every Debug window load, the timeline also adds one `HTML-startup.json` metadata event. It lists the seven JSON documents loaded by `HTSettings`, the current Recipe, and the current Production sequence. This makes the timeline useful before the first command is issued, while avoiding a duplicate copy of large startup payloads.

| Panel | Input JSON | Direction |
|---|---|---|
| Input: Main Command | `Main-command-request.json` | HTML -> BCB6 |
| Input: Site Toggle | `Site-toggle-request.json` | HTML -> BCB6 |
| Output: BCB6 Command Acknowledgement | `Main-command-ack.json` | BCB6 -> HTML |

## Save Log Checkbox

The checkbox writes `Debug-json-log-config.json` through `HTJsonWriter` in Debug mode. Its fixed setting is:

```json
{
  "enabled": true,
  "logDirectory": "D:\\HT9045_Log\\Debug_JSON_Log",
  "relativeDirectoryTemplate": "{YYYY}\\{MM}\\{DD}"
}
```

## Direct Browser Archive

`HTDebugLogWriter` provides direct browser-side archive writing for Debug mode. Select **Choose Log Directory** once and select exactly `D:\HT9045_Log\Debug_JSON_Log` in the browser directory dialog. Chromium requires this explicit user gesture; JavaScript cannot silently grant itself access to an absolute Windows path.

After the directory is granted, **Save and Clear** writes directly to `D:\HT9045_Log\Debug_JSON_Log\YYYY\MM\DD\timeline-YYYYMMDD-HHMMSS.json`. The archive contains the complete retained timeline in one JSON document and its archive timestamp. The UI clears only after the write completes. If no directory has been granted or writing fails, the timeline remains visible and the page displays an error; it never falls back to a browser download.

Browser code must not attempt to create that directory or write log files directly: File System Access requires an explicit user-selected directory and cannot safely enforce an absolute Windows path. The BCB6 runtime bridge owns physical logging. When `enabled:true`, it creates `D:\HT9045_Log\Debug_JSON_Log\YYYY\MM\DD\` using the C++ local date.

Each daily directory contains `command-events.jsonl`, one valid JSON envelope per line. The Debug window retains events in its local timeline and only writes `Debug-json-log-flush-request.json` after the user selects **Save and Clear**. C++ atomically persists the complete requested timeline, then writes `Debug-json-log-flush-ack.json` with the same `flushId`. HTML clears its local timeline only after `accepted:true` is received. This prevents duplicate log entries caused by the viewer's 500 ms polling and prevents data loss when saving fails. Each envelope contains `schemaVersion`, `loggedAt`, `direction`, `mailbox`, `requestId` when supplied, and the unmodified `payload` from the mailbox.

The bridge also overwrites three quick-inspection files in the same daily directory: `latest-main-command-request.json`, `latest-site-toggle-request.json`, and `latest-main-command-ack.json`. **Clear only** removes the browser timeline after confirmation and never deletes C++ log files. Do not automatically delete history; the retention policy is manual. When disabled, the Save and Clear command is unavailable and C++ stops creating new archives without deleting existing logs.

## Startup Before the Browser Exists

The `HTML-startup.json` observation describes what the browser received after its iframe has loaded. It cannot prove which source files existed before the browser process started. To cover this earlier period, the C++ launcher reads the persisted `Debug-json-log-config.json` **before** it starts `background.html` or Edge. When logging is enabled, it creates the daily directory and writes one `startup-manifest.json` for the handler run.

The manifest is deliberately metadata-only to avoid duplicating large `General`/`Config`/`Setup`/`Production` payloads. It contains a generated `runId`, timestamps, and for every JSON provided to the HTML startup loader: its name, byte length, last-write time, and SHA-256. C++ includes the same `runId` in each later `command-events.jsonl` envelope, so the startup input set and subsequent command sequence can be matched exactly.

Do not make this bootstrap step depend on the Debug page, browser postMessage, or a 500 ms poll. A checkbox change affects the next C++ bridge read; after it has been persisted, the next startup is captured before any HTML is visible.

All four JSON files require matching `JSON/js/*.js` shims for `file://` transport. After schema changes, run `_gen_json_shim.py`.
