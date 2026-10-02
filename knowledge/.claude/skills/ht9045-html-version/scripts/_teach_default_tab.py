# HW.teach.html：最外層 PageControl2 預設分頁改為 tsAxleCtrl（對應 BCB6 uteach.cpp FormShow 需求）
import re
p = r'D:\HT9045\page\HW.teach.html'
s = open(p, encoding='utf-8').read()

# 頁籤：tsIndex 去 act、tsAxleCtrl 加 act
s, n1 = re.subn(r'<div class="tab act" data-t="0" title="tsIndex : TTabSheet">', '<div class="tab" data-t="0" title="tsIndex : TTabSheet">', s)
s, n2 = re.subn(r'<div class="tab" data-t="8" title="tsAxleCtrl : TTabSheet">', '<div class="tab act" data-t="8" title="tsAxleCtrl : TTabSheet">', s)
# 內容：tsIndex pane 隱藏、tsAxleCtrl pane 顯示
s, n3 = re.subn(r'(<div class="pcPane" data-p="0" title="tsIndex" style=")display:block;', r'\1display:none;', s)
s, n4 = re.subn(r'(<div class="pcPane" data-p="8" title="tsAxleCtrl" style=")display:none;', r'\1display:block;', s)
print('tab off/on', n1, n2, 'pane off/on', n3, n4)
assert n1 == n2 == n3 == n4 == 1
open(p, 'w', encoding='utf-8').write(s)
print('ok')
