# ReleaseNote.txt 直接內嵌至 Data.Observer.html 的 #Memo1（依使用者指示 2026-09-09，不建 JSON）
# 內容 = cObserver.cpp ShowVer() 硬編歷史版本清單（asVer 改用代表性版號 V3.33，
#        因目前版號已由同頁 labVersion 動態顯示，避免重覆/失真）+ D:\HT9045\config\ReleaseNote.txt 全文。
import re

CPP = r"D:\HT9045\HT9011UC_Code_V3.33.910.0_20260716_NN Mode 2D + AutoClean V2\cObserver.cpp"
TXT = r"D:\HT9045\config\ReleaseNote.txt"
HTML = r"D:\HT9045\page\Data.Observer.html"

cpp = open(CPP, encoding="cp950", errors="replace").read()
m = re.search(r'Memo1->Lines->Add\(asVer\);(.*?)if\(FileExists\("D:\\\\HT9045\\\\config\\\\ReleaseNote\.txt"\)\)', cpp, re.S)
block = m.group(1)
lines = ["V3.33"]  # asVer 動態版號改用代表性字串；即時版號見同頁 labVersion
for lm in re.finditer(r'Memo1->Lines->Add\("(.*?)"\);', block):
    s = lm.group(1).replace('\\"', '"')
    lines.append(" " if s.strip() == "" else s)

release_note = open(TXT, encoding="cp950", errors="replace").read()
full_text = "\n".join(lines) + "\n" + release_note

def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

html = open(HTML, encoding="utf-8").read()
old = '<textarea class="ed" id="Memo1" title="Memo1 : TMemo" style="position:absolute;left:2px;top:176px;width:621px;height:111px;"></textarea>'
assert old in html, "Memo1 textarea marker not found"
new = ('<textarea class="ed" id="Memo1" title="Memo1 : TMemo" readonly '
       'style="position:absolute;left:2px;top:176px;width:621px;height:111px;">'
       + esc(full_text) + '</textarea>')
html = html.replace(old, new)
with open(HTML, "w", encoding="utf-8") as f:
    f.write(html)
print("embedded", len(full_text), "chars,", full_text.count(chr(10)) + 1, "lines")
