import glob, io
for f in glob.glob(r'D:\HT9045\page\*.html'):
    s = io.open(f, encoding='utf-8').read()
    if 'theme.css' in s or 'ht9xxx.css' not in s:
        continue
    s = s.replace('<link rel="stylesheet" href="ht9xxx.css">',
                  '<link rel="stylesheet" href="ht9xxx.css">\n<link rel="stylesheet" href="theme.css">', 1)
    s = s.replace('</body>', '<script src="theme.js"></script>\n</body>', 1)
    io.open(f, 'w', encoding='utf-8').write(s)
    print('themed:', f.rsplit('\\', 1)[-1])
