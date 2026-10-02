from pathlib import Path
from datetime import datetime, timedelta
import hashlib
import re
import sys


def week_text(report_date):
    monday = report_date - timedelta(days=report_date.weekday())
    sunday = monday + timedelta(days=6)
    weekday_text = ['一', '二', '三', '四', '五', '六', '日']
    return f'{monday:%Y/%m/%d}（週{weekday_text[monday.weekday()]}）～ {sunday:%Y/%m/%d}（週{weekday_text[sunday.weekday()]}）'


def normalize_text_dates(text, default_year):
    if text is None:
        return ''

    normalized = str(text)
    normalized = normalized.replace('～', '~').replace('；', '、')

    def repl_datetime(match):
        year, month, day = match.groups()
        return f'{int(year):04d}/{int(month):02d}/{int(day):02d}'

    def repl_compact_range(match):
        start_text, end_text = match.groups()
        start_date = datetime.strptime(start_text, '%Y%m%d')
        end_date = datetime.strptime(end_text, '%Y%m%d')
        return f'{start_date:%Y/%m/%d}~{end_date:%Y/%m/%d}'

    def repl_full_range(match):
        year1, month1, day1, year2, month2, day2 = match.groups()
        return f'{int(year1):04d}/{int(month1):02d}/{int(day1):02d}~{int(year2):04d}/{int(month2):02d}/{int(day2):02d}'

    def repl_shared_year_range(match):
        year, month1, day1, month2, day2 = match.groups()
        return f'{int(year):04d}/{int(month1):02d}/{int(day1):02d}~{int(year):04d}/{int(month2):02d}/{int(day2):02d}'

    def repl_month_day_range(match):
        month1, day1, month2, day2 = match.groups()
        return f'{int(default_year):04d}/{int(month1):02d}/{int(day1):02d}~{int(default_year):04d}/{int(month2):02d}/{int(day2):02d}'

    def repl_month_day_range_with_bullet(match):
        prefix, month1, day1, month2, day2, bullet = match.groups()
        return f'{prefix}{int(default_year):04d}/{int(month1):02d}/{int(day1):02d}~{int(default_year):04d}/{int(month2):02d}/{int(day2):02d}<br>{bullet}'

    def repl_month_day_single(match):
        prefix, month, day = match.groups()
        return f'{prefix}{int(default_year):04d}/{int(month):02d}/{int(day):02d}'

    def repl_single(match):
        year, month, day = match.groups()
        return f'{int(year):04d}/{int(month):02d}/{int(day):02d}'

    def repl_compact_single(match):
        value = match.group(1)
        date_value = datetime.strptime(value, '%Y%m%d')
        return f'{date_value:%Y/%m/%d}'

    def repl_numbered_compact_single(match):
        prefix, number, value = match.groups()
        date_value = datetime.strptime(value, '%Y%m%d')
        return f'{prefix}{number}{date_value:%Y/%m/%d}'

    def repl_year_month_days(match):
        parts = re.findall(r'\d{1,4}', match.group(1))
        if len(parts) < 3:
            return match.group(1)
        year = int(parts[0])
        month = int(parts[1])
        return '、'.join(f'{year:04d}/{month:02d}/{int(day):02d}' for day in parts[2:])

    def repl_month_day_with_prefix(match):
        prefix, month, day = match.groups()
        return f'{prefix}{int(default_year):04d}/{int(month):02d}/{int(day):02d}'

    def repl_month_day_with_paren(match):
        month, day = match.groups()
        return f'{int(default_year):04d}/{int(month):02d}/{int(day):02d}'

    def repl_seqnum_month_day(match):
        groups = match.groups()
        if len(groups) == 4:
            prefix, seqnum, month, day = groups
            return f'{prefix}{seqnum}. {int(default_year):04d}/{int(month):02d}/{int(day):02d}'
        elif len(groups) == 3:
            seqnum, month, day = groups
            return f'{seqnum}. {int(default_year):04d}/{int(month):02d}/{int(day):02d}'
        return match.group(0)

    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{2})-(\d{1,2})-(\d{1,2})\s+\d{2}:\d{2}:\d{2}(?![A-Za-z0-9_.])', repl_datetime, normalized)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{6})\s*[~-]\s*(20\d{6})(?![A-Za-z0-9_.])', repl_compact_range, normalized)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{2})[./、_-](\d{1,2})[./、_-](\d{1,2})\s*[~-]\s*(20\d{2})[./、_-](\d{1,2})[./、_-](\d{1,2})(?![A-Za-z0-9_.])', repl_full_range, normalized)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{2})/(\d{1,2})/(\d{1,2})\s*[~-]\s*(\d{1,2})/(\d{1,2})(?![A-Za-z0-9_.])', repl_shared_year_range, normalized)
    normalized = re.sub(r'(^|<br>|[\[(、\s>])(\d{1,2})[./](\d{1,2})\s*[~-]\s*(\d{1,2})[./](\d{1,2})([A-Za-z]\.)', repl_month_day_range_with_bullet, normalized, flags=re.MULTILINE)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(\d{1,2})[./](\d{1,2})\s*[~-]\s*(\d{1,2})[./](\d{1,2})(?![A-Za-z0-9_.])', repl_month_day_range, normalized)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{2}[、]\d{1,2}(?:[、]\d{1,2}){2,})(?![A-Za-z0-9_.])', repl_year_month_days, normalized)
    normalized = re.sub(r'(^|<br>|[\[(、\s>])(\d{1,2})[./](\d{1,2})(?=<br>|\)|\]|、|\s|<|$)', repl_month_day_single, normalized, flags=re.MULTILINE)
    normalized = re.sub(r'(^|[\[(、\s])(\d{1,2})[./](\d{1,2})(?=\)|\]|、|\s|$)', repl_month_day_with_prefix, normalized)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{2})[./、_-](\d{1,2})[./、_-](\d{1,2})(?![A-Za-z0-9_.])', repl_single, normalized)
    normalized = re.sub(r'(\d{1,2})/(\d{1,2})\([^)]*\)', lambda m: repl_month_day_with_paren(m), normalized)
    normalized = re.sub(r'(^|<br>|>|[\s(])(\d+)\.(\d{1,2})/(\d{1,2})', lambda m: repl_seqnum_month_day(m), normalized, flags=re.MULTILINE)
    normalized = re.sub(r'(\d+)\.(\d{1,2})/(\d{1,2})(?=\s|<|$)', lambda m: repl_seqnum_month_day(m), normalized)
    normalized = re.sub(r'(^|<br>|[\[(、\s])(\d+\.)(20\d{6})(?![A-Za-z0-9_.])', repl_numbered_compact_single, normalized, flags=re.MULTILINE)
    normalized = re.sub(r'(?<![A-Za-z0-9_.])(20\d{6})(?![A-Za-z0-9_.])', repl_compact_single, normalized)

    legacy_misparsed_year = 1911 + (int(default_year) % 1000)
    if legacy_misparsed_year != int(default_year):
        normalized = re.sub(
            rf'(?<![A-Za-z0-9_.]){legacy_misparsed_year}[./、_-](\d{{1,2}})[./、_-](\d{{1,2}})(?![A-Za-z0-9_.])',
            lambda match: f'{int(default_year):04d}/{int(match.group(1)):02d}/{int(match.group(2)):02d}',
            normalized
        )

    return normalized


def parse_report_date_from_path(path):
    name = path.name

    for pattern in [r'(20\d{2})_(\d{2})_(\d{2})', r'(20\d{2})-(\d{2})-(\d{2})', r'(20\d{2})(\d{2})(\d{2})']:
        match = re.search(pattern, name)
        if match:
            return datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)))

    roc_match = re.search(r'(?<!\d)(\d{3})-(\d{1,2})-(\d{1,2})(?!\d)', name)
    if roc_match:
        return datetime(int(roc_match.group(1)) + 1911, int(roc_match.group(2)), int(roc_match.group(3)))

    return None


def parse_personal_target_name(path):
    if not path.name.startswith('RD5_個人週報_'):
        return None

    suffix = ''
    stem = path.stem
    if stem.endswith('_一頁版'):
        suffix = '_一頁版'
        stem = stem[:-len(suffix)]

    rest = stem[len('RD5_個人週報_'):]
    patterns = [
        r'^(?P<person>.+?)_(?P<year>20\d{2})_(?P<month>\d{2})_(?P<day>\d{2})$',
        r'^(?P<person>.+?)-(?P<year>20\d{2})-(?P<month>\d{2})-(?P<day>\d{2})$',
        r'^(?P<person>.+?)_(?P<compact>20\d{6})$'
    ]

    for pattern in patterns:
        match = re.match(pattern, rest)
        if match:
            person = match.group('person')
            if match.groupdict().get('compact'):
                report_date = datetime.strptime(match.group('compact'), '%Y%m%d')
            else:
                report_date = datetime(int(match.group('year')), int(match.group('month')), int(match.group('day')))
            return path.with_name(f'RD5_個人週報_{person}_{report_date:%Y_%m_%d}{suffix}{path.suffix}')

    return None


def parse_department_target_name(path):
    match = re.match(r'^(鴻勁_研五部週報_)(\d{3})-(\d{1,2})-(\d{1,2})(.*)$', path.stem)
    if not match:
        return None

    prefix, roc_year, month, day, suffix = match.groups()
    return path.with_name(f'{prefix}{roc_year}-{int(month):02d}-{int(day):02d}{suffix}{path.suffix}')


def file_hash(path):
    digest = hashlib.sha256()
    with path.open('rb') as handle:
        for chunk in iter(lambda: handle.read(65536), b''):
            digest.update(chunk)
    return digest.hexdigest()


def rename_if_needed(path):
    target = parse_personal_target_name(path)
    if target is None:
        target = parse_department_target_name(path)
    if target is None or target == path:
        return path, 'unchanged'

    if target.exists():
        if file_hash(path) == file_hash(target):
            path.unlink()
            return target, 'removed-duplicate'
        return path, f'conflict:{target.name}'

    path.rename(target)
    return target, 'renamed'


def normalize_personal_content(content, report_date):
    lines = content.splitlines()
    normalized_lines = []
    for line in lines:
        if line.startswith('**報告日期**：'):
            normalized_lines.append(f'**報告日期**：{report_date:%Y-%m-%d}  ')
            continue
        if line.startswith('**週期**：'):
            normalized_lines.append(f'**週期**：{week_text(report_date)}  ')
            continue
        normalized_lines.append(normalize_text_dates(line, report_date.year))
    return '\n'.join(normalized_lines) + ('\n' if content.endswith('\n') else '')


def normalize_department_content(content, report_date):
    lines = content.splitlines()
    normalized_lines = []
    for line in lines:
        if line.startswith('**週報日期**：'):
            normalized_lines.append(f'**週報日期**：{report_date:%Y/%m/%d}  ')
            continue
        normalized_lines.append(normalize_text_dates(line, report_date.year))
    return '\n'.join(normalized_lines) + ('\n' if content.endswith('\n') else '')


def normalize_file_content(path):
    if path.suffix.lower() not in {'.md', '.html'}:
        return False

    report_date = parse_report_date_from_path(path)
    if report_date is None:
        return False

    original = path.read_text(encoding='utf-8')
    if path.name.startswith('RD5_個人週報_'):
        updated = normalize_personal_content(original, report_date)
    else:
        updated = normalize_department_content(original, report_date)

    if updated == original:
        return False

    path.write_text(updated, encoding='utf-8')
    return True


def iter_report_files(root):
    for path in sorted(root.iterdir()):
        if path.name.startswith('~$'):
            continue
        if path.suffix.lower() not in {'.xlsx', '.md', '.html'}:
            continue
        if path.name.startswith('RD5_個人週報_') or path.name.startswith('鴻勁_研五部週報_'):
            yield path


def main(target_path):
    root = Path(target_path)
    if root.is_file():
        files = [root]
    else:
        files = list(iter_report_files(root))

    renamed_files = []
    for path in files:
        new_path, status = rename_if_needed(path)
        renamed_files.append((path.name, new_path, status))

    changed_content = 0
    normalized_targets = []
    if root.is_file():
        normalized_targets = [renamed_files[0][1]]
    else:
        normalized_targets = [item[1] for item in renamed_files if item[1].exists()]

    for path in normalized_targets:
        if normalize_file_content(path):
            changed_content += 1

    updated_results = []
    for original_name, current_path, status in renamed_files:
        if status.startswith('conflict:') and current_path.exists():
            deduped_path, deduped_status = rename_if_needed(current_path)
            if deduped_status != 'unchanged':
                current_path = deduped_path
                status = deduped_status
        updated_results.append((original_name, current_path, status))
    renamed_files = updated_results

    print('Filename normalization results:')
    for original_name, current_path, status in renamed_files:
        print(f'  {original_name} -> {current_path.name} [{status}]')
    print(f'Content normalization updated: {changed_content} file(s)')


if __name__ == '__main__':
    if len(sys.argv) > 1:
        main(sys.argv[1])
    else:
        print('Usage: python method-e_date_standardizer.py <file-or-folder>')