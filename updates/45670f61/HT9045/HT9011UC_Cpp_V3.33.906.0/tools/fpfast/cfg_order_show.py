import json, sys
db = json.load(open(sys.argv[1], encoding='utf-8'))
for e in db:
    c = e['command']
    if c.endswith('cContact.cpp') or c.endswith('cJSON.c'):
        t = c.split(' ')
        print(t[0].split(chr(92))[-1], [x for x in t if x.startswith('-f') or x.startswith('-O') or x.startswith('-std')])
