#!/usr/bin/env python3
"""
HT9011UC Merge Report MD -> HTML Converter
用法: md2html.py <input.md> [output.html]
"""
import html as html_mod
import re
import sys
import os

def fmt(s):
    """Format inline markdown: **bold**, `code`"""
    s = html_mod.escape(s)
    s = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', s)
    s = re.sub(r'`(.+?)`', r'<code>\1</code>', s)
    return s

def md_to_html(md_text, title='HT9011UC Merge Report'):
    lines = md_text.split('\n')
    o = []
    o.append('<!DOCTYPE html><html lang="zh-TW"><head><meta charset="UTF-8">')
    o.append('<meta name="viewport" content="width=device-width, initial-scale=1.0">')
    o.append(f'<title>{html_mod.escape(title)}</title>')
    o.append('<style>')
    o.append('body{font-family:"Microsoft JhengHei","Segoe UI",sans-serif;'
             'max-width:1100px;margin:2em auto;padding:0 1em;line-height:1.6;'
             'color:#222;background:#fafbfc}')
    o.append('h1{color:#1a5276;border-bottom:3px solid #2e86c1;padding-bottom:.3em}')
    o.append('h2{color:#2e86c1;border-bottom:1px solid #aed6f1;padding-bottom:.2em;margin-top:1.8em}')
    o.append('h3{color:#1a7a4c;margin-top:1.2em}')
    o.append('table{border-collapse:collapse;width:100%;margin:.8em 0}')
    o.append('th,td{border:1px solid #bdc3c7;padding:6px 10px;text-align:left}')
    o.append('th{background:#2e86c1;color:#fff;font-weight:600}')
    o.append('tr:nth-child(even){background:#eaf2f8}')
    o.append('code{background:#f0f0f0;padding:1px 4px;border-radius:3px;font-size:0.92em}')
    o.append('strong{color:#c0392b}')
    o.append('hr{border:none;border-top:2px solid #d5dbdb;margin:1.5em 0}')
    o.append('em{font-style:italic;color:#666}')
    o.append('pre{background:#f4f4f4;border:1px solid #ddd;padding:10px;'
             'overflow-x:auto;font-size:0.9em;border-radius:4px}')
    o.append('</style></head><body>')

    in_table = False
    in_list = False
    in_code = False
    code_buf = []
    i = 0

    while i < len(lines):
        s = lines[i].strip()
        raw = lines[i]

        # Code blocks
        if s.startswith('```'):
            if in_code:
                o.append(html_mod.escape('\n'.join(code_buf)))
                o.append('</pre>')
                in_code = False
                code_buf = []
            else:
                if in_list:
                    o.append('</ul>'); in_list = False
                in_code = True
            i += 1; continue

        if in_code:
            code_buf.append(raw.rstrip())
            i += 1; continue

        # Headings
        if s.startswith('# ') and not s.startswith('## '):
            if in_list: o.append('</ul>'); in_list = False
            o.append(f'<h1>{fmt(s[2:])}</h1>'); i += 1; continue
        if s.startswith('## '):
            if in_list: o.append('</ul>'); in_list = False
            o.append(f'<h2>{fmt(s[3:])}</h2>'); i += 1; continue
        if s.startswith('### '):
            if in_list: o.append('</ul>'); in_list = False
            o.append(f'<h3>{fmt(s[4:])}</h3>'); i += 1; continue

        # HR
        if s == '---':
            if in_list: o.append('</ul>'); in_list = False
            o.append('<hr>'); i += 1; continue

        # Table
        if s.startswith('|') and '|' in s[1:]:
            if in_list: o.append('</ul>'); in_list = False
            cells = [c.strip() for c in s.split('|')[1:-1]]
            if all(set(c) <= set('-: ') for c in cells):
                i += 1; continue
            if not in_table and i + 1 < len(lines) and '---' in lines[i + 1]:
                o.append('<table><thead><tr>')
                for c in cells:
                    o.append(f'<th>{fmt(c)}</th>')
                o.append('</tr></thead><tbody>')
                in_table = True; i += 1; continue
            if in_table:
                o.append('<tr>')
                for c in cells:
                    o.append(f'<td>{fmt(c)}</td>')
                o.append('</tr>')
                if i + 1 >= len(lines) or not lines[i + 1].strip().startswith('|'):
                    o.append('</tbody></table>'); in_table = False
                i += 1; continue

        # List
        if s.startswith('- '):
            if not in_list: o.append('<ul>'); in_list = True
            o.append(f'<li>{fmt(s[2:])}</li>'); i += 1; continue
        else:
            if in_list: o.append('</ul>'); in_list = False

        # Italic paragraph
        if s.startswith('*') and s.endswith('*') and not s.startswith('**'):
            o.append(f'<p><em>{fmt(s[1:-1])}</em></p>'); i += 1; continue

        # Blockquote
        if s.startswith('> '):
            o.append(f'<blockquote>{fmt(s[2:])}</blockquote>'); i += 1; continue

        # Empty
        if not s:
            i += 1; continue

        # Paragraph
        o.append(f'<p>{fmt(s)}</p>'); i += 1

    if in_list: o.append('</ul>')
    if in_table: o.append('</tbody></table>')
    if in_code: o.append('</pre>')
    o.append('</body></html>')
    return '\n'.join(o)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input.md> [output.html]")
        sys.exit(1)

    md_path = sys.argv[1]
    if len(sys.argv) >= 3:
        html_path = sys.argv[2]
    else:
        html_path = os.path.splitext(md_path)[0] + '.html'

    with open(md_path, 'r', encoding='utf-8') as f:
        md_text = f.read()

    # Extract title from first H1
    title_match = re.search(r'^# (.+)$', md_text, re.MULTILINE)
    title = title_match.group(1) if title_match else 'Merge Report'

    html_text = md_to_html(md_text, title)

    with open(html_path, 'w', encoding='utf-8') as f:
        f.write(html_text)

    print(f"OK: {html_path}")

if __name__ == '__main__':
    main()
