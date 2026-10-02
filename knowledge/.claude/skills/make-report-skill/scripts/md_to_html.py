#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
md_to_html.py — Markdown to HTML/PDF converter using honprec-red/blue-template

Usage:
    python md_to_html.py <input.md> [--template red|blue|weekly] [--out <output.html>] [--pdf]

Examples:
    python md_to_html.py "D:/00_ReleaseNote/HPI-TW/921_KYEC_LEE/report.md"
    python md_to_html.py report.md --template blue
    python md_to_html.py report.md --template weekly
    python md_to_html.py report.md --template red --pdf
    python md_to_html.py report.md --template red --pdf --out custom.pdf

Requirements:
    pip install markdown playwright
    (Playwright uses existing Edge — no extra browser download needed)
"""

import sys
import os
import re
import argparse

# --------------------------------------------------------------------------- #
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SKILL_DIR = os.path.dirname(SCRIPT_DIR)

TEMPLATE_MAP = {
    "red":    os.path.join(SKILL_DIR, "templates", "honprec-red-template"),
    "blue":   os.path.join(SKILL_DIR, "templates", "honprec-blue-template"),
    "weekly": os.path.join(SKILL_DIR, "templates", "personal-weekly-report-template"),
}

# --------------------------------------------------------------------------- #
def load_logo_base64(template_dir):
    """Read Logo Base64 — 優先直接讀取 PNG（快速路徑），略過 58KB 參考 md 檔。"""
    import base64

    # 快速路徑：直接對 PNG 做 base64（21KB → 28KB，不需解析 58KB md）
    png_path = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "references", "HonPrec鴻勁精密商標_CS2_2.png"
    )
    if os.path.exists(png_path):
        with open(png_path, "rb") as f:
            b64 = base64.b64encode(f.read()).decode("ascii")
        print(f"[INFO] Logo Base64 loaded from PNG ({len(b64):,} chars)")
        return b64

    # 備用路徑：從 references/HonPrec_Logo_Base64_Reference.md 解析
    ref_path = os.path.join(template_dir, "references", "HonPrec_Logo_Base64_Reference.md")
    if not os.path.exists(ref_path):
        print(f"[WARN] Logo not found (PNG nor reference md): {png_path}")
        return ""
    with open(ref_path, encoding="utf-8") as f:
        content = f.read()
    match = re.search(r"`\n(iVBORw0[^\n`]+(?:\n[^\n`]+)*)\n`", content, re.DOTALL)
    if match:
        b64 = match.group(1).strip()
        print(f"[INFO] Logo Base64 loaded from reference md ({len(b64):,} chars)")
        return b64
    for line in content.splitlines():
        line = line.strip()
        if line.startswith("iVBORw0") and len(line) > 1000:
            print(f"[INFO] Logo Base64 loaded from reference md ({len(line):,} chars)")
            return line
    print("[WARN] Could not extract Base64 from reference file")
    return ""


# --------------------------------------------------------------------------- #
CSS_RED = """
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: "微軟正黑體", "Calibri", sans-serif;
  background: #f4f6f9;
  padding: 32px 24px;
  color: #2c2c2c;
  line-height: 1.7;
}
.page { max-width: 900px; margin: 0 auto; }
header {
  display: flex;
  align-items: center;
  gap: 18px;
  border-left: 6px solid #c0392b;
  padding: 14px 18px;
  margin-bottom: 28px;
  background: #fff;
  border-radius: 0 6px 6px 0;
  box-shadow: 0 1px 4px rgba(0,0,0,.08);
}
header img { height: 56px; object-fit: contain; }
header h1 { font-size: 20px; color: #c0392b; margin-bottom: 4px; }
header p  { font-size: 12px; color: #888; }
h1 { font-size: 22px; color: #c0392b; margin: 24px 0 12px; }
h2 {
  font-size: 15px;
  background: #c0392b;
  color: #fff;
  padding: 6px 14px;
  border-radius: 4px;
  margin: 28px 0 12px 0;
  page-break-after: avoid;   /* 防止標題孤立在頁尾 */
  break-after: avoid;
}
h3 { font-size: 14px; color: #c0392b; margin: 18px 0 8px; }
p { margin-bottom: 10px; font-size: 14px; }
table {
  width: 100%;
  border-collapse: collapse;
  background: #fff;
  margin-bottom: 16px;
  page-break-inside: auto;
  break-inside: auto;
}
th, td {
  border: 1px solid #e0e0e0;
  padding: 9px 14px;
  font-size: 14px;
  vertical-align: top;
}
th {
  background: #f4f4f4;
  font-weight: bold;
  white-space: nowrap;
  color: #444;
}
code {
  background: #f4f4f4;
  padding: 2px 6px;
  border-radius: 3px;
  font-family: Consolas, "Courier New", monospace;
  font-size: 13px;
}
pre {
  background: #f8f8f8;
  border: 1px solid #ddd;
  border-left: 4px solid #c0392b;
  padding: 14px 16px;
  border-radius: 4px;
  overflow-x: auto;
  font-family: Consolas, "Courier New", monospace;
  font-size: 13px;
  line-height: 1.6;
  margin-bottom: 16px;
}
pre code { background: none; padding: 0; }
ul, ol { padding-left: 24px; margin-bottom: 10px; font-size: 14px; }
li { margin-bottom: 4px; }
blockquote {
  border-left: 4px solid #c0392b;
  background: #fff8f8;
  padding: 10px 16px;
  margin: 12px 0;
  color: #555;
  font-size: 14px;
  page-break-inside: auto;
  break-inside: auto;
}
.flow-box {
  background: #fff;
  border: 1px solid #ddd;
  border-left: 4px solid #c0392b;
  padding: 16px 20px;
  font-family: Consolas, "Courier New", monospace;
  font-size: 14px;
  line-height: 2.2;
  border-radius: 4px;
  margin: 12px 0 16px 0;
}
.arr  { color: #c0392b; font-weight: bold; }
.node { color: #2980b9; font-weight: bold; }
.time { color: #27ae60; font-style: italic; }
.cmd  { color: #8e44ad; font-weight: bold; }
hr { border: none; border-top: 1px solid #e0e0e0; margin: 20px 0; }
footer {
  margin-top: 36px;
  font-size: 11px;
  color: #bbb;
  border-top: 1px solid #ddd;
  padding-top: 10px;
  text-align: center;
}
/* Mermaid diagram container */
.mermaid {
  background: transparent;
  padding: 0;
  margin: 16px 0;
  text-align: center;
  page-break-inside: auto;
  break-inside: auto;
}
/* 讓 SVG 本身保持自適應，不要強制拉滿 100% 導致高度過度等比放大產生空白 */
.mermaid svg {
  max-width: 100% !important;
  height: auto !important;
  display: block;
  margin: 0 auto;
}
"""

CSS_BLUE = CSS_RED.replace("#c0392b", "#1a6fa8").replace("#fff8f8", "#f0f7ff")

CSS_WEEKLY = """
* { box-sizing: border-box; }
body { font-family: "Microsoft JhengHei", Arial, sans-serif; font-size: 13px; margin: 24px 40px; color: #222; }
h1 { font-size: 20px; color: #c00; border-bottom: 2px solid #c00; padding-bottom: 4px; margin-bottom: 12px; margin-top: 16px; }
h2 { font-size: 15px; color: #005a9e; border-left: 4px solid #005a9e; padding-left: 8px; margin-top: 22px; margin-bottom: 8px; }
h3 { font-size: 13px; color: #333; margin-top: 14px; margin-bottom: 6px; }
p { margin-bottom: 8px; font-size: 13px; line-height: 1.7; }
.meta { background: #f5f5f5; border: 1px solid #ddd; padding: 8px 14px; border-radius: 4px; margin-bottom: 16px; line-height: 1.8; font-size: 13px; }
.meta strong { color: #005a9e; }
table { border-collapse: collapse; width: 100%; margin-top: 8px; font-size: 12px; }
th { background: #003e7e; color: #fff; padding: 5px 7px; text-align: center; white-space: nowrap; }
td { border: 1px solid #bbb; padding: 4px 7px; vertical-align: top; }
tr:nth-child(even) td { background: #f0f4fb; }
tr:hover td { background: #dce8f8; }
.summary { background: #fffbf0; border: 1px solid #e8c840; padding: 10px 16px; border-radius: 4px; margin-top: 8px; line-height: 1.7; }
ul { margin: 4px 0 4px 18px; padding: 0; }
li { margin-bottom: 3px; font-size: 13px; }
blockquote { border-left: 4px solid #005a9e; background: #f0f7ff; padding: 8px 14px; margin: 10px 0; color: #444; }
hr { border: none; border-top: 1px solid #ddd; margin: 16px 0; }
code { background: #f4f4f4; padding: 2px 4px; border-radius: 3px; font-size: 12px; font-family: Consolas, monospace; }
pre { background: #f8f8f8; border: 1px solid #ddd; border-left: 4px solid #005a9e; padding: 10px 14px; border-radius: 4px; overflow-x: auto; font-size: 12px; }
footer { margin-top: 32px; color: #999; font-size: 11px; border-top: 1px solid #ddd; padding-top: 6px; }
"""

# --------------------------------------------------------------------------- #
# Mermaid JS — 使用 UMD 版本放在 body 底部
# 原因：ESM type="module" 的 import 為非同步載入，且 CDN 載完時 DOMContentLoaded 已觸發，
# 導致 startOnLoad 無法偵測到 .mermaid 元素。
# 正確做法：在 </body> 前載入 UMD + 明確呼叫 mermaid.run()
MERMAID_JS_BODY = """
<script src="https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js"></script>
<script>
  mermaid.initialize({ theme: 'default', securityLevel: 'loose' });
  mermaid.run({ nodes: document.querySelectorAll('.mermaid') });
</script>
"""


def extract_title_from_md(content):
    """Extract first H1 as page title."""
    for line in content.splitlines():
        line = line.strip()
        if line.startswith("# "):
            return line[2:].strip()
    return "報告"


def extract_yaml_frontmatter(content):
    """Parse YAML frontmatter and return (meta_dict, body)."""
    meta = {}
    if not content.startswith("---"):
        return meta, content
    end_idx = content.find("\n---", 3)
    if end_idx == -1:
        return meta, content
    frontmatter = content[3:end_idx].strip()
    body = content[end_idx + 4:].lstrip("\n")
    for line in frontmatter.splitlines():
        if ":" in line:
            key, _, val = line.partition(":")
            meta[key.strip()] = val.strip()
    return meta, body


def create_exportable_md(md_path, template="red", out_path=None):
    """
    產生包含完整 Logo、CSS 設定與標題列的 .md 檔案，
    專為給 Yzane.Markdown-PDF VS Code 外掛直接輸出 PDF 設計。
    """
    template_dir = TEMPLATE_MAP.get(template, TEMPLATE_MAP["red"])
    
    with open(md_path, encoding="utf-8-sig") as f:
        raw = f.read()

    meta, body = extract_yaml_frontmatter(raw)
    page_title = meta.get("title") or extract_title_from_md(body)
    
    logo_path = "C:/Users/steven/.github/skills/make-report-skill/references/HonPrec鴻勁精密商標_CS2_2.png"
    logo_html = f'<img src="{logo_path}" alt="Logo">'
    
    # 移除原本可能存在的 report-header 區塊，防止重複產生
    import re
    # 用明確的註解保護，避免處理巢狀 div 問題，確保可以安全移除舊的檔頭
    body = re.sub(r'<!-- REPORT_HEADER_START -->.*?<!-- REPORT_HEADER_END -->\n*', '', body, flags=re.DOTALL)
    
    # 決定使用的 CSS
    css_path = os.path.join(template_dir, f"honprec-{template}.css").replace("\\", "/")
    
    # 建立適合 Markdown PDF 讀取的檔頭
    header_html = f"""<!-- REPORT_HEADER_START -->
<div class="report-header">
  {logo_html}
  <div>
    <h1 style="border:none; padding:0; margin-bottom:4px;">{page_title}</h1>
        <p style="margin:0;">鴻勁精密股份有限公司 Hon. Precision, Inc.</p>
  </div>
</div>
<!-- REPORT_HEADER_END -->
"""
    
    # 組合新的 YAML Frontmatter
    new_yaml = f"""---
"markdown-pdf":
  includeDefaultStyles: false
  displayHeaderFooter: false
  printBackground: true
  styles:
    - "{css_path}"
---
"""
    
    new_content = new_yaml + "\n" + header_html + "\n\n" + body
    
    if not out_path:
        base = os.path.splitext(md_path)[0]
        out_path = base + "_printable.md"

    with open(out_path, encoding="utf-8", mode="w") as f:
        f.write(new_content)
        
    print(f"[OK] Exportable MD saved: {out_path}")
    return out_path


# --------------------------------------------------------------------------- #
def convert_md_to_html(md_path, template="red", out_path=None):
    """Main conversion function."""
    try:
        import markdown
        from markdown.extensions.tables import TableExtension
        from markdown.extensions.fenced_code import FencedCodeExtension
    except ImportError:
        print("[ERROR] Required package missing. Run: pip install markdown")
        sys.exit(1)

    template_dir = TEMPLATE_MAP.get(template, TEMPLATE_MAP["red"])
    if not os.path.isdir(template_dir):
        print(f"[WARN] Template dir not found: {template_dir}, using inline CSS only")

    # Read source MD（utf-8-sig 自動剝除 BOM \ufeff，避免 front matter 解析失敗）
    with open(md_path, encoding="utf-8-sig") as f:
        raw = f.read()

    meta, body = extract_yaml_frontmatter(raw)
    page_title = meta.get("title") or extract_title_from_md(body)
    author = meta.get("author", "鴻勁精密 HONPREC")
    date_str = meta.get("date", "")

    # Load Logo
    logo_b64 = load_logo_base64(template_dir)
    logo_html = (
        f'<img src="data:image/png;base64,{logo_b64}" alt="鴻勁精密 Logo">'
        if logo_b64 else ""
    )

    # Choose CSS
    css = CSS_WEEKLY if template == "weekly" else (CSS_BLUE if template == "blue" else CSS_RED)

    # Pre-process Mermaid blocks: replace ```mermaid ... ``` with <div class="mermaid">
    def replace_mermaid(m):
        code = m.group(1).strip()
        return f'\n<div class="mermaid">\n{code}\n</div>\n'

    body = re.sub(r"```mermaid\s*\n([\s\S]*?)```", replace_mermaid, body)

    # Convert Markdown → HTML
    md = markdown.Markdown(extensions=[
        TableExtension(),
        FencedCodeExtension(),
        "markdown.extensions.nl2br",
    ])
    content_html = md.convert(body)

    # Add id attributes to section headings so URL fragments work (#A01, #N07-1)
    content_html = re.sub(
        r'<(h[23])>([A-Z]\d{2,3}(?:-\d+[a-z]?)*)\s*(?:—|-)',
        lambda m: f'<{m.group(1)} id="{m.group(2)}">{m.group(2)} —',
        content_html
    )

    # Post-process for weekly template
    if template == "weekly":
        # 1. Convert metadata paragraph (contains **人員**) → .meta div
        content_html = re.sub(
            r'<p>(<strong>人員</strong>[\s\S]*?)</p>',
            r'<div class="meta">\1</div>',
            content_html
        )
        # 2. Wrap 摘要 section in .summary div (until <hr> or next <h2>)
        content_html = re.sub(
            r'(<h2>[^<]*摘要[^<]*</h2>\n?)([\s\S]*?)(<hr\s*/?> |(?=<h2>))',
            lambda m: m.group(1) + '<div class="summary">\n' + m.group(2) + '</div>\n' + m.group(3),
            content_html
        )

    # Build full HTML
    footer_text = f"鴻勁精密股份有限公司 Hon. Precision, Inc. | No.11, Ln. 758, Sec. 3, Zhongqing Rd., Daya Dist., Taichung City 42878 | Tel: +886-4-25608752 | www.honprec.com | ISO 9001 | {author} | {date_str}" if date_str else f"鴻勁精密股份有限公司 Hon. Precision, Inc. | No.11, Ln. 758, Sec. 3, Zhongqing Rd., Daya Dist., Taichung City 42878 | Tel: +886-4-25608752 | www.honprec.com | ISO 9001 | {author}"

    if template == "weekly":
        html = f"""<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{page_title}</title>
  <style>{css}</style>
</head>
<body>
  <div style="padding:0 0 8px 0;">
    <img width="100%" alt="鴻勁精密 Logo" src="data:image/png;base64,{logo_b64}">
  </div>
  {content_html}
  <footer>{footer_text}</footer>
{MERMAID_JS_BODY}
</body>
</html>"""
    else:
        html = f"""<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{page_title}</title>
  <style>{css}</style>
</head>
<body>
<div class="page">
  <header>
    {logo_html}
    <div>
      <h1>{page_title}</h1>
            <p>鴻勁精密股份有限公司 Hon. Precision, Inc.</p>
    </div>
  </header>

  {content_html}

  <footer>{footer_text}</footer>
</div>
{MERMAID_JS_BODY}
</body>
</html>"""

    # Output path
    if not out_path:
        base = os.path.splitext(md_path)[0]
        out_path = base + ".html"

    with open(out_path, encoding="utf-8", mode="w") as f:
        f.write(html)

    print(f"[OK] HTML saved: {out_path}")
    return out_path


# --------------------------------------------------------------------------- #
def export_html_to_pdf(html_path, pdf_path=None):
    """
    使用 Playwright headless Edge 將 HTML 轉成 PDF。
    不需額外下載瀏覽器——直接使用系統已安裝的 Microsoft Edge。
    需要先執行：pip install playwright
    """
    try:
        from playwright.sync_api import sync_playwright
    except ImportError:
        print("[ERROR] playwright not installed. Run: pip install playwright")
        return None

    if not pdf_path:
        pdf_path = os.path.splitext(html_path)[0] + ".pdf"

    abs_html = os.path.abspath(html_path)
    file_url = "file:///" + abs_html.replace("\\", "/")

    edge_path = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
    if not os.path.exists(edge_path):
        edge_path = r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"

    print(f"[INFO] Launching headless Edge: {edge_path}")
    print(f"[INFO] Rendering: {file_url}")

    with sync_playwright() as pw:
        browser = pw.chromium.launch(
            executable_path=edge_path,
            headless=True,
            args=["--no-sandbox", "--disable-dev-shm-usage"]
        )
        # 寬 viewport 讓 Mermaid 以足夠寬度完整渲染 SVG
        page = browser.new_page(viewport={"width": 1280, "height": 900})
        page.goto(file_url, wait_until="networkidle", timeout=30000)
        # 等待 Mermaid 渲染完成（最多 10 秒）
        try:
            page.wait_for_function(
                "() => document.querySelectorAll('.mermaid svg').length > 0",
                timeout=10000
            )
            page.wait_for_timeout(500)
            print("[INFO] Mermaid diagrams rendered")
        except Exception:
            print("[WARN] Mermaid wait timeout (diagram may not render in PDF)")

        # ── 核心修正：把每個 .mermaid div 截圖成 PNG，替換掉 SVG ──
        # 原因：Chromium PDF 引擎對超高 SVG 的分頁計算有已知 bug，
        # 會在 h2 標題之後插入空白頁。改用 <img> 可完全避免此問題。
        mermaid_elements = page.query_selector_all(".mermaid")
        
        # Step 1: Fix Mermaid's trailing empty space bug by explicitly cropping to actual bounding box
        for el in mermaid_elements:
            try:
                el.evaluate(r"""(el) => {
                    const svg = el.querySelector('svg');
                    if (svg) {
                        const bbox = svg.getBBox();
                        // Add some padding to the bounding box
                        let realHeight = bbox.height + Math.max(0, bbox.y) + 30; 
                        
                        let origBox = svg.getAttribute('viewBox');
                        let originW = svg.clientWidth || 800;
                        if (origBox) {
                            let parts = origBox.split(/[\s,]+/);
                            if (parts.length >= 3) originW = parts[2];
                        }
                        
                        // If Mermaid's drawn height is much smaller than the viewBox, crop it
                        svg.style.height = realHeight + 'px';
                        svg.setAttribute('height', realHeight);
                        svg.setAttribute('viewBox', '0 0 ' + originW + ' ' + realHeight);
                        
                        // Ensure wrapper is also shrunk
                        el.style.height = (realHeight + 40) + 'px';
                    }
                }""")
            except Exception as e:
                print(f"[WARN] Failed to crop SVG bounding box: {e}")

        page.wait_for_timeout(300)

        import base64
        for idx, el in enumerate(mermaid_elements):
            try:
                png_bytes = el.screenshot(type="png")
                b64 = base64.b64encode(png_bytes).decode()
                # Use Promise to wait for image load and slice it for pagination
                js_code = f"""
                    (el) => {{
                        return new Promise((resolve) => {{
                            const containerWidth = el.clientWidth || 800;
                            const img = new Image();
                            img.onload = () => {{
                                el.innerHTML = '';
                                el.style.overflow = 'visible'; 
                                el.style.padding = '0';
                                el.style.border = 'none';
                                
                                let nw = img.naturalWidth;
                                let nh = img.naturalHeight;
                                let ratio = containerWidth / nw;
                                let renderedHeight = nh * ratio;
                                
                                const SLICE_H = 20; // Thin strips for natural page breaks
                                let slices = Math.ceil(renderedHeight / SLICE_H);
                                
                                for(let i=0; i<slices; i++) {{
                                    let sliceDiv = document.createElement('div');
                                    sliceDiv.style.width = '100%';
                                    let thisSliceH = Math.min(SLICE_H, renderedHeight - i*SLICE_H);
                                    sliceDiv.style.height = thisSliceH + 'px';
                                    sliceDiv.style.backgroundImage = 'url(' + img.src + ')';
                                    sliceDiv.style.backgroundSize = '100% auto';
                                    sliceDiv.style.backgroundPosition = '0 -' + (i * SLICE_H) + 'px'; 
                                    sliceDiv.style.backgroundRepeat = 'no-repeat';
                                    sliceDiv.style.pageBreakInside = 'auto';
                                    sliceDiv.style.breakInside = 'auto';
                                    el.appendChild(sliceDiv);
                                }}
                                resolve();
                            }};
                            img.src = 'data:image/png;base64,{b64}';
                        }});
                    }}
                """
                el.evaluate(js_code)
                print(f"[INFO] Mermaid[{idx}] sliced to PNG ({len(png_bytes):,} bytes)")
            except Exception as e:
                print(f"[WARN] Mermaid[{idx}] screenshot failed: {e}")

        page.wait_for_timeout(300)
        page.emulate_media(media="print")
        page.pdf(
            path=pdf_path,
            format="A4",
            print_background=True,
            margin={"top": "15mm", "bottom": "15mm",
                    "left": "20mm", "right": "20mm"}
        )
        browser.close()

    size = os.path.getsize(pdf_path)
    print(f"[OK] PDF saved: {pdf_path} ({size:,} bytes)")
    return pdf_path


# --------------------------------------------------------------------------- #
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert Markdown report to HTML/PDF (honprec template)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python md_to_html.py report.md
  python md_to_html.py report.md --template blue
  python md_to_html.py report.md --pdf
  python md_to_html.py report.md --template red --pdf --out my_report.pdf
""")
    parser.add_argument("input", help="Input .md file path")
    parser.add_argument("--template", choices=["red", "blue", "weekly"], default="red",
                        help="Template colour scheme: red=提案表/報告, blue=工程報告, weekly=個人週報 (default: red)")
    parser.add_argument("--out", default=None,
                        help="Output path (.html or .pdf, default: same dir as input)")
    parser.add_argument("--pdf", action="store_true",
                        help="Also generate PDF using headless Edge (requires playwright)")
    parser.add_argument("--md", action="store_true",
                        help="Generate a _printable.md file optimized for VS Code Markdown PDF extension")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"[ERROR] File not found: {args.input}")
        sys.exit(1)

    if args.md:
        out_path = args.out if args.out and args.out.endswith(".md") else None
        create_exportable_md(args.input, template=args.template, out_path=out_path)
        sys.exit(0)

    # Determine output paths
    if args.out and args.out.endswith(".pdf"):
        html_out = None   # temp html same dir
        pdf_out = args.out
        force_pdf = True
    else:
        html_out = args.out
        pdf_out = None
        force_pdf = args.pdf

    html_path = convert_md_to_html(args.input, template=args.template, out_path=html_out)

    if force_pdf:
        export_html_to_pdf(html_path, pdf_path=pdf_out)
