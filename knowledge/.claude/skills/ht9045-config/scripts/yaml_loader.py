"""
yaml_loader.py
==============
Minimal YAML reader (subset) for Config 工具鏈，避免依賴 PyYAML。
僅支援我們自己 md_to_yaml.py 產出的格式：
  - dict / list / scalar
  - double-quoted strings
  - literal block (|)
  - bool true/false / null / numbers
"""

import re


def _unquote(s):
    s = s.strip()
    if s.startswith('"') and s.endswith('"'):
        s = s[1:-1].replace('\\"', '"').replace('\\\\', '\\')
        return s
    return s


def _parse_scalar(s):
    s = s.strip()
    if s == '' or s.lower() == 'null':
        return None
    if s.lower() == 'true':
        return True
    if s.lower() == 'false':
        return False
    if s.startswith('"'):
        return _unquote(s)
    # int / float
    if re.match(r'^-?\d+$', s):
        return int(s)
    if re.match(r'^-?\d+\.\d+$', s):
        return float(s)
    return s


def load_yaml(path):
    with open(path, encoding='utf-8') as f:
        text = f.read()
    return loads(text)


def loads(text):
    lines = text.split('\n')
    # Strip comments (lines starting with #) and trailing blank
    cleaned = []
    for ln in lines:
        if ln.lstrip().startswith('#'):
            cleaned.append('')
        else:
            cleaned.append(ln.rstrip('\r'))

    parser = _Parser(cleaned)
    return parser.parse_block(0)


class _Parser:
    def __init__(self, lines):
        self.lines = lines
        self.idx = 0

    def _peek(self):
        while self.idx < len(self.lines) and self.lines[self.idx].strip() == '':
            self.idx += 1
        if self.idx >= len(self.lines):
            return None
        return self.lines[self.idx]

    def _indent(self, line):
        return len(line) - len(line.lstrip(' '))

    def parse_block(self, base_indent):
        """Return dict or list or scalar at current position with base_indent."""
        first = self._peek()
        if first is None:
            return None
        ind = self._indent(first)
        stripped = first.strip()
        if stripped.startswith('-'):
            return self._parse_list(ind)
        return self._parse_dict(ind)

    def _parse_dict(self, base_indent):
        result = {}
        while self.idx < len(self.lines):
            line = self.lines[self.idx]
            if line.strip() == '':
                self.idx += 1
                continue
            ind = self._indent(line)
            if ind < base_indent:
                break
            if ind > base_indent:
                # shouldn't happen at top of dict
                break
            stripped = line.strip()
            if ':' not in stripped:
                break
            key, _, rest = stripped.partition(':')
            key = key.strip()
            rest = rest.strip()
            self.idx += 1
            if rest == '':
                # nested or list
                nxt = self._peek()
                if nxt is None:
                    result[key] = None
                    continue
                nind = self._indent(nxt)
                if nind <= base_indent:
                    result[key] = None
                else:
                    nstripped = nxt.strip()
                    if nstripped.startswith('-'):
                        result[key] = self._parse_list(nind)
                    else:
                        result[key] = self._parse_dict(nind)
            elif rest == '|':
                # literal block scalar
                block_lines = []
                block_indent = None
                while self.idx < len(self.lines):
                    bl = self.lines[self.idx]
                    if bl.strip() == '':
                        block_lines.append('')
                        self.idx += 1
                        continue
                    bind = self._indent(bl)
                    if block_indent is None:
                        if bind <= base_indent:
                            break
                        block_indent = bind
                    if bind < block_indent:
                        break
                    block_lines.append(bl[block_indent:])
                    self.idx += 1
                # trim trailing empties
                while block_lines and block_lines[-1] == '':
                    block_lines.pop()
                result[key] = '\n'.join(block_lines) + ('\n' if block_lines else '')
            elif rest == '[]':
                result[key] = []
            else:
                result[key] = _parse_scalar(rest)
        return result

    def _parse_list(self, base_indent):
        result = []
        while self.idx < len(self.lines):
            line = self.lines[self.idx]
            if line.strip() == '':
                self.idx += 1
                continue
            ind = self._indent(line)
            if ind < base_indent:
                break
            stripped = line.strip()
            if not stripped.startswith('-'):
                break
            after_dash = stripped[1:].strip()
            self.idx += 1
            if after_dash == '':
                # nested object on next lines
                nxt = self._peek()
                if nxt is None:
                    result.append(None)
                    continue
                nind = self._indent(nxt)
                if nind <= base_indent:
                    result.append(None)
                else:
                    nstripped = nxt.strip()
                    if nstripped.startswith('-'):
                        result.append(self._parse_list(nind))
                    else:
                        result.append(self._parse_dict(nind))
            else:
                # inline scalar
                result.append(_parse_scalar(after_dash))
        return result


if __name__ == '__main__':
    import sys
    import json
    data = load_yaml(sys.argv[1])
    print(json.dumps(data, indent=2, ensure_ascii=False))
