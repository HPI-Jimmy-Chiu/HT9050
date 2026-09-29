/*
 * AI(W906-ELA-XLSD) 20260928 (St02-E helper): ★ 第五題 = D (Steven 0928)
 *
 * HtXlsx -- a small .xlsx writer that runs in the page: no library, no CDN, no server code, no new route.
 * Used by eventlog.html for golden's four XLS buttons (EventlogAnalyzer Rev891 Analyzer.cpp :2245-2297 -> SGDToXLS ->
 * XLSfile.pas StringGridToXLS :91-111: a BIFF2 file whose every cell is a LABEL record, i.e. text, :178-192).
 * Here every cell is text as well: an inline string (c t="inlineStr") styled with the built-in Text number format
 * (numFmtId 49 = "@", ECMA-376 Part 1 §18.8.30 numFmt), so Excel converts nothing: "0316" stays "0316", "1E5" stays
 * "1E5", "2026/09/28" stays the string.
 *
 * ---- The ZIP container (PKWARE APPNOTE.TXT 6.3.x; ECMA-376 Part 2 = OPC, whose physical package is a ZIP) ----
 * Written by hand, every entry STORED (compression method 0, no compression), in this order per entry:
 *   4.3.7   local file header: signature 0x04034b50, version needed 10 (1.0 is enough for stored data, 4.4.3.2),
 *           flags, method 0, DOS time, DOS date, CRC-32, compressed size, uncompressed size (equal when stored),
 *           name length, extra length 0 -- 30 bytes, then the name, then the entry's bytes
 * and after all entries:
 *   4.3.12  central directory: one file header per entry, signature 0x02014b50, version made by 20 (host 0 =
 *           MS-DOS / FAT), the same fields as the local header, comment length 0, disk 0, attributes 0, and the
 *           offset of the entry's local header -- 46 bytes, then the name
 *   4.3.16  end of central directory record: signature 0x06054b50, disk 0 / 0, entry count (twice), central
 *           directory size and offset, comment length 0 -- 22 bytes
 * Fields are little-endian (4.4.1.1).  General purpose bit 11 (UTF-8 names, 4.4.4) is set only for a non-ASCII name;
 * every part name here is ASCII.  Bit 3 (data descriptor) is never set: sizes and CRC are known before each header.
 * No extra fields, no ZIP64 (a grid is at most 5,000 rows, far below 4 GiB), no archive comment.
 * DOS date / time (4.4.6): one fixed stamp, 1980-01-01 00:00:00 (date 0x0021, time 0x0000), so the same rows always
 * give the same bytes.
 * CRC-32 (4.4.7): the standard CRC-32 -- reflected polynomial 0xEDB88320 (0x04C11DB7), register preset to 0xFFFFFFFF,
 * result complemented; one 256-entry table.
 *
 * ---- The parts (ECMA-376 Part 1 §12.3 SpreadsheetML part types, §18 SpreadsheetML markup; Part 2 for the first two) ----
 *   [Content_Types].xml          Part 2: Default for rels / xml, one Override per workbook, styles and worksheet part
 *   _rels/.rels                  Part 2: the package relationship officeDocument -> xl/workbook.xml
 *   xl/workbook.xml              §18.2 workbook / sheets / sheet (name, sheetId, r:id)
 *   xl/_rels/workbook.xml.rels   worksheet relationships rId1..rIdN, then styles rId(N+1)
 *   xl/styles.xml                §18.8 the minimum Excel expects: 2 fonts (regular, bold), the 2 reserved fills
 *                                (none, gray125), 1 empty border, 1 cellStyleXf, cellXfs 0 = default, 1 = Text,
 *                                2 = Text + bold (the header row), the "Normal" cell style
 *   xl/worksheets/sheetN.xml     §18.3 worksheet, children in schema order: dimension, cols (a width per column),
 *                                sheetData (row r= / c r= s= t="inlineStr" / is / t), ignoredErrors
 *                                numberStoredAsText over the used range (every cell is text on purpose, so Excel's
 *                                green "number stored as text" triangles -- which golden's BIFF2 files showed -- are off)
 * No sharedStrings part (inline strings need none), no docProps (optional parts).  An empty cell is left out, and a
 * value longer than Excel's 32,767 characters per cell is cut there (no event-log field comes near it).
 *
 * Text: XML-escaped (& < >, and " in attributes).  ST_Xstring (ECMA-376 Part 1, shared simple types): a character XML 1.0
 * cannot carry (C0 controls other than TAB / LF, CR which XML would turn into LF, U+FFFE / U+FFFF) is written _xHHHH_,
 * and a literal "_xHHHH_" in the data gets its underscore written _x005F_, so Excel reads the text back unchanged.
 * Bytes are UTF-8 (TextEncoder, or the same encoding by hand; a lone surrogate becomes U+FFFD).
 * Sheet names (Excel's rules, not the schema's): at most 31 characters, none of [ ] : * ? / \, no apostrophe at either
 * end, not empty, unique ignoring case.
 * Column width: the longest value in the column (an East Asian wide character counts 2), + 2, between 4 and 60
 * character widths (customWidth="1").
 *
 * API (window.HtXlsx):
 *   build([{ name, head: [..], rows: [[..], ..] }, ..]) -> Uint8Array, the .xlsx bytes (row 1 = head, bold)
 *   save(fileName, sheets)                              -> builds it and downloads it (Blob + <a download>); returns bytes
 *   crc32(Uint8Array)                                   -> the unsigned CRC-32
 */
(function (root) {
  'use strict';

  var MIME = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet';
  var XML_HEAD = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\r\n';
  var NS_MAIN = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main';
  var NS_R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships';
  var NS_PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships';
  var NS_CT = 'http://schemas.openxmlformats.org/package/2006/content-types';
  var CT_SML = 'application/vnd.openxmlformats-officedocument.spreadsheetml.';
  var DOS_TIME = 0x0000;                         // 00:00:00  ((h << 11) | (m << 5) | (s >> 1))
  var DOS_DATE = 0x0021;                         // 1980-01-01 (((y - 1980) << 9) | (m << 5) | d)
  var STYLE_TEXT = 1, STYLE_HEAD = 2;            // cellXfs indexes in styles.xml below

  // ---- CRC-32, APPNOTE 4.4.7 ----
  var CRC_TABLE = (function () {
    var t = new Array(256), n, k, c;
    for (n = 0; n < 256; n++) {
      c = n;
      for (k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
      t[n] = c >>> 0;
    }
    return t;
  })();
  function crc32(bytes) {
    var c = 0xFFFFFFFF, i;
    for (i = 0; i < bytes.length; i++) c = CRC_TABLE[(c ^ bytes[i]) & 0xFF] ^ (c >>> 8);
    return (c ^ 0xFFFFFFFF) >>> 0;
  }

  // ---- UTF-8 ----
  function utf8(s) {
    if (typeof TextEncoder !== 'undefined') return new TextEncoder().encode(s);
    var out = [], i, c, d;
    for (i = 0; i < s.length; i++) {
      c = s.charCodeAt(i);
      if (c >= 0xD800 && c <= 0xDBFF && i + 1 < s.length && (d = s.charCodeAt(i + 1)) >= 0xDC00 && d <= 0xDFFF) {
        c = 0x10000 + ((c - 0xD800) << 10) + (d - 0xDC00);
        i++;
      } else if (c >= 0xD800 && c <= 0xDFFF) c = 0xFFFD;
      if (c < 0x80) out.push(c);
      else if (c < 0x800) out.push(0xC0 | (c >> 6), 0x80 | (c & 63));
      else if (c < 0x10000) out.push(0xE0 | (c >> 12), 0x80 | ((c >> 6) & 63), 0x80 | (c & 63));
      else out.push(0xF0 | (c >> 18), 0x80 | ((c >> 12) & 63), 0x80 | ((c >> 6) & 63), 0x80 | (c & 63));
    }
    return new Uint8Array(out);
  }

  // ---- XML text (ST_Xstring) ----
  function hex4(n) { return ('000' + n.toString(16).toUpperCase()).slice(-4); }
  function xmlText(v) {
    return String(v == null ? '' : v)
      .replace(/_(?=x[0-9A-Fa-f]{4}_)/g, '_x005F_')
      .replace(/[\x00-\x08\x0B-\x1F\uFFFE\uFFFF]/g, function (ch) { return '_x' + hex4(ch.charCodeAt(0)) + '_'; })
      .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }
  function xmlAttr(v) { return xmlText(v).replace(/"/g, '&quot;'); }

  // ---- cell references, widths, sheet names ----
  function colName(i) {                          // 0 -> A, 25 -> Z, 26 -> AA
    var s = '';
    for (i = i + 1; i > 0; i = Math.floor((i - 1) / 26)) s = String.fromCharCode(65 + (i - 1) % 26) + s;
    return s;
  }
  function isWide(c) {
    return (c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) || (c >= 0xAC00 && c <= 0xD7A3) ||
           (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFE30 && c <= 0xFE4F) || (c >= 0xFF00 && c <= 0xFF60) ||
           (c >= 0xFFE0 && c <= 0xFFE6) || (c >= 0xD800 && c <= 0xDBFF);   // an astral pair counts once, as wide
  }
  function textWidth(s) {
    var w = 0, line = 0, i, c;
    for (i = 0; i < s.length; i++) {
      c = s.charCodeAt(i);
      if (c === 10) { if (line > w) w = line; line = 0; continue; }       // a multi-line value: its longest line
      if (c >= 0xDC00 && c <= 0xDFFF) continue;                            // low half of a pair, counted above
      line += isWide(c) ? 2 : 1;
    }
    return line > w ? line : w;
  }
  function sheetNames(list) {
    var used = {}, out = [];
    list.forEach(function (n, i) {
      var s = String(n == null ? '' : n).replace(/[\[\]:*?\/\\]/g, '_').replace(/^'+|'+$/g, '').slice(0, 31).replace(/'+$/, '');
      if (!s) s = 'Sheet' + (i + 1);
      var base = s, k = 2, suf;
      while (used[s.toLowerCase()]) { suf = '(' + (k++) + ')'; s = base.slice(0, 31 - suf.length) + suf; }
      used[s.toLowerCase()] = true;
      out.push(s);
    });
    return out;
  }

  // ---- the parts ----
  function worksheetXml(head, rows) {
    var ncol = head.length, cols = [], widths = [], out = [], nrow = 0, k, r;
    for (k = 0; k < ncol; k++) { cols[k] = colName(k); widths[k] = 0; }
    function rowXml(cells, style) {
      var rn = ++nrow, h = '', v, w;
      for (k = 0; k < ncol; k++) {
        v = cells[k] == null ? '' : String(cells[k]);
        if (v === '') continue;
        if (v.length > 32767) v = v.slice(0, 32767);                      // Excel's per-cell limit (else: repair)
        w = textWidth(v);
        if (w > widths[k]) widths[k] = w;
        h += '<c r="' + cols[k] + rn + '" s="' + style + '" t="inlineStr"><is><t xml:space="preserve">' + xmlText(v) + '</t></is></c>';
      }
      if (h) out.push('<row r="' + rn + '">' + h + '</row>');
    }
    rowXml(head, STYLE_HEAD);
    for (r = 0; r < rows.length; r++) rowXml(rows[r] || [], STYLE_TEXT);
    var ref = ncol ? 'A1' + (ncol > 1 || nrow > 1 ? ':' + cols[ncol - 1] + nrow : '') : 'A1';
    var colXml = '';
    for (k = 0; k < ncol; k++)
      colXml += '<col min="' + (k + 1) + '" max="' + (k + 1) + '" width="' + Math.min(60, Math.max(4, widths[k] + 2)) + '" customWidth="1"/>';
    return XML_HEAD + '<worksheet xmlns="' + NS_MAIN + '" xmlns:r="' + NS_R + '">' +
      '<dimension ref="' + ref + '"/>' +
      (colXml ? '<cols>' + colXml + '</cols>' : '') +
      '<sheetData>' + out.join('') + '</sheetData>' +
      (ncol ? '<ignoredErrors><ignoredError sqref="' + ref + '" numberStoredAsText="1"/></ignoredErrors>' : '') +
      '</worksheet>';
  }
  var STYLES_XML = XML_HEAD + '<styleSheet xmlns="' + NS_MAIN + '">' +
    '<fonts count="2">' +
      '<font><sz val="11"/><name val="Calibri"/><family val="2"/></font>' +
      '<font><b/><sz val="11"/><name val="Calibri"/><family val="2"/></font>' +
    '</fonts>' +
    '<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>' +
    '<borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>' +
    '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>' +
    '<cellXfs count="3">' +
      '<xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>' +
      '<xf numFmtId="49" fontId="0" fillId="0" borderId="0" xfId="0" applyNumberFormat="1"/>' +
      '<xf numFmtId="49" fontId="1" fillId="0" borderId="0" xfId="0" applyNumberFormat="1" applyFont="1"/>' +
    '</cellXfs>' +
    '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>' +
    '</styleSheet>';

  // ---- the store-only ZIP (APPNOTE 4.3.7 / 4.3.12 / 4.3.16) ----
  function zipStore(entries) {
    var parts = [], central = [], offset = 0, cdSize = 0, i;
    for (i = 0; i < entries.length; i++) {
      var e = entries[i], name = utf8(e.name), data = e.data, crc = crc32(data), size = data.length;
      var flag = /[^\x00-\x7F]/.test(e.name) ? 0x0800 : 0;
      var lh = new Uint8Array(30 + name.length), v = new DataView(lh.buffer);
      v.setUint32(0, 0x04034b50, true);          // local file header signature
      v.setUint16(4, 10, true);                  // version needed to extract (1.0)
      v.setUint16(6, flag, true);                // general purpose bit flag
      v.setUint16(8, 0, true);                   // compression method: stored
      v.setUint16(10, DOS_TIME, true);
      v.setUint16(12, DOS_DATE, true);
      v.setUint32(14, crc, true);
      v.setUint32(18, size, true);               // compressed size
      v.setUint32(22, size, true);               // uncompressed size
      v.setUint16(26, name.length, true);
      v.setUint16(28, 0, true);                  // extra field length
      lh.set(name, 30);
      var ch = new Uint8Array(46 + name.length), w = new DataView(ch.buffer);
      w.setUint32(0, 0x02014b50, true);          // central file header signature
      w.setUint16(4, 20, true);                  // version made by: 2.0, host 0 (MS-DOS / FAT)
      w.setUint16(6, 10, true);                  // version needed to extract
      w.setUint16(8, flag, true);
      w.setUint16(10, 0, true);                  // stored
      w.setUint16(12, DOS_TIME, true);
      w.setUint16(14, DOS_DATE, true);
      w.setUint32(16, crc, true);
      w.setUint32(20, size, true);
      w.setUint32(24, size, true);
      w.setUint16(28, name.length, true);
      w.setUint16(30, 0, true);                  // extra field length
      w.setUint16(32, 0, true);                  // file comment length
      w.setUint16(34, 0, true);                  // disk number start
      w.setUint16(36, 0, true);                  // internal file attributes
      w.setUint32(38, 0, true);                  // external file attributes
      w.setUint32(42, offset, true);             // relative offset of local header
      ch.set(name, 46);
      parts.push(lh, data);
      central.push(ch);
      offset += lh.length + size;
      cdSize += ch.length;
    }
    if (offset + cdSize + 22 > 0xFFFFFFFF || entries.length > 0xFFFF) throw new Error('HtXlsx: too large for a ZIP without ZIP64');
    var end = new Uint8Array(22), x = new DataView(end.buffer);
    x.setUint32(0, 0x06054b50, true);            // end of central dir signature
    x.setUint16(4, 0, true);                     // number of this disk
    x.setUint16(6, 0, true);                     // disk where the central directory starts
    x.setUint16(8, entries.length, true);        // entries on this disk
    x.setUint16(10, entries.length, true);       // entries in total
    x.setUint32(12, cdSize, true);               // size of the central directory
    x.setUint32(16, offset, true);               // offset of the central directory
    x.setUint16(20, 0, true);                    // comment length
    var all = parts.concat(central, [end]), total = 0, pos = 0, out;
    for (i = 0; i < all.length; i++) total += all[i].length;
    out = new Uint8Array(total);
    for (i = 0; i < all.length; i++) { out.set(all[i], pos); pos += all[i].length; }
    return out;
  }

  function build(sheets) {
    if (!sheets || !sheets.length) throw new Error('HtXlsx.build: no sheet');
    var n = sheets.length, names = sheetNames(sheets.map(function (s) { return s.name; })), i;
    var ct = XML_HEAD + '<Types xmlns="' + NS_CT + '">' +
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>' +
      '<Default Extension="xml" ContentType="application/xml"/>' +
      '<Override PartName="/xl/workbook.xml" ContentType="' + CT_SML + 'sheet.main+xml"/>' +
      '<Override PartName="/xl/styles.xml" ContentType="' + CT_SML + 'styles+xml"/>';
    var wb = XML_HEAD + '<workbook xmlns="' + NS_MAIN + '" xmlns:r="' + NS_R + '"><sheets>';
    var wbRels = XML_HEAD + '<Relationships xmlns="' + NS_PKG_R + '">';
    for (i = 1; i <= n; i++) {
      ct += '<Override PartName="/xl/worksheets/sheet' + i + '.xml" ContentType="' + CT_SML + 'worksheet+xml"/>';
      wb += '<sheet name="' + xmlAttr(names[i - 1]) + '" sheetId="' + i + '" r:id="rId' + i + '"/>';
      wbRels += '<Relationship Id="rId' + i + '" Type="' + NS_R + '/worksheet" Target="worksheets/sheet' + i + '.xml"/>';
    }
    ct += '</Types>';
    wb += '</sheets></workbook>';
    wbRels += '<Relationship Id="rId' + (n + 1) + '" Type="' + NS_R + '/styles" Target="styles.xml"/></Relationships>';
    var rels = XML_HEAD + '<Relationships xmlns="' + NS_PKG_R + '">' +
      '<Relationship Id="rId1" Type="' + NS_R + '/officeDocument" Target="xl/workbook.xml"/></Relationships>';
    var entries = [
      { name: '[Content_Types].xml', data: utf8(ct) },
      { name: '_rels/.rels', data: utf8(rels) },
      { name: 'xl/workbook.xml', data: utf8(wb) },
      { name: 'xl/_rels/workbook.xml.rels', data: utf8(wbRels) },
      { name: 'xl/styles.xml', data: utf8(STYLES_XML) }
    ];
    for (i = 0; i < n; i++)
      entries.push({ name: 'xl/worksheets/sheet' + (i + 1) + '.xml', data: utf8(worksheetXml(sheets[i].head || [], sheets[i].rows || [])) });
    return zipStore(entries);
  }

  function save(fileName, sheets) {
    var bytes = build(sheets);
    var url = URL.createObjectURL(new Blob([bytes], { type: MIME }));
    var a = document.createElement('a');
    a.href = url; a.download = fileName; a.style.display = 'none';
    document.body.appendChild(a); a.click();
    setTimeout(function () { URL.revokeObjectURL(url); document.body.removeChild(a); }, 2000);
    return bytes.length;
  }

  root.HtXlsx = { build: build, save: save, crc32: crc32 };
})(window);
