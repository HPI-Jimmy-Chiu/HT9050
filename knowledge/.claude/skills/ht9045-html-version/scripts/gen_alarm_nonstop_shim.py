# -*- coding: utf-8 -*-
"""gen_alarm_nonstop_shim.py -- 從 AlarmNonStop.json 產生 file: 協定用的墊片

Steven 20260922
當日完整變更紀錄：<入口網站 repo>\\public\\Docs\\ChangeLog\\Steven\\CHANGES_20260922_Steven.md

為什麼需要這個檔
----------------
量產啟動器走 `file:` 協定：
    HT9045_Release.cmd:16  set "URL=file:///D:/HT9045/background.html?mode=release"
而 Edge/Chromium 在 `file:` 下封鎖 XHR/fetch 讀本地檔。

⇒ 只有 .json 的實作在量產模式 **100% 讀不到，而且不會報錯** ——
  畫面看起來一切正常，只是所有告警都走回會停機的那一頁。

這正是 dialog-bridge.js:22-43 `loadFresh()` 早就在處理的同一件事：
    http(s)  -> fetch .json
    file:    -> <script src="....js">，讀 window.__HT9045_DATA__[name]

所以每條通道都要**兩個檔**。這支負責從 .json 產生那個 .js 墊片，
避免兩份手動維護而漂掉。

⚠ 墊片的 key 必須逐字等於檔名去掉副檔名（`AlarmNonStop`），
  載入端是靠這個 key 取值的。

用法
----
    <python314> gen_alarm_nonstop_shim.py            # 產生
    <python314> gen_alarm_nonstop_shim.py --check    # 只檢查是否同步，不寫檔
                                                      # 不同步回 exit 1
"""
import io
import json
import os
import sys

WEB = r'D:\HT9045\web'
NAME = 'AlarmNonStop'
SRC = os.path.join(WEB, 'config', NAME + '.json')
DST = os.path.join(WEB, 'config', NAME + '.js')

# 墊片一律 CRLF、無 BOM —— 與 web\JSON\js\ 底下既有的墊片一致
NL = '\r\n'


def build(doc):
    body = json.dumps(doc, ensure_ascii=False, separators=(',', ':'))
    return NL.join([
        '/* 自動產生，不要手改 —— 改 %s.json 之後重跑' % NAME,
        '   D:\\HT9045\\HT9011UC_Cpp_V3.33.906.0\\scratchpad\\gen_alarm_nonstop_shim.py',
        '',
        '   這個檔存在的唯一理由：量產啟動器走 file: 協定，',
        '   而瀏覽器在 file: 下讀不到 .json。見產生器檔頭。 */',
        'window.__HT9045_DATA__ = window.__HT9045_DATA__ || {};',
        'window.__HT9045_DATA__[%s] = %s;' % (json.dumps(NAME), body),
        '',
    ])


def main():
    check = '--check' in sys.argv
    if not os.path.exists(SRC):
        print('!! 找不到來源 %s' % SRC)
        return 2
    doc = json.load(io.open(SRC, encoding='utf-8'))
    want = build(doc)

    if check:
        if not os.path.exists(DST):
            print('!! 墊片不存在：%s' % DST)
            return 1
        got = io.open(DST, encoding='utf-8', newline='').read()
        if got != want:
            print('!! 墊片與 .json 不同步：%s' % DST)
            print('   重跑本腳本（不加 --check）即可修好。')
            return 1
        print('OK 墊片與 .json 同步')
        return 0

    b = want.encode('utf-8')          # 先 encode，成功才開檔
    with open(DST, 'wb') as f:
        f.write(b)
    print('%s -> %s  (%d bytes, %d 個 code)'
          % (os.path.basename(SRC), os.path.basename(DST), len(b),
             len(doc.get('codes') or {})))
    return 0


if __name__ == '__main__':
    sys.exit(main())
