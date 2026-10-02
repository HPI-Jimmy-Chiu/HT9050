import re
import sys
import os
import html
from datetime import datetime, timedelta

def split_and_group_red_text_html(html_content, report_date):
    # Normalize to \n
    s = html_content.replace('<br>', '\n').replace('<br/>', '\n')
    lines = s.split('\n')
    
    current_lines = []
    history_lines = []
    
    # Calculate Monday of the report week
    # Assuming report_date is the Sunday equivalent or end of week.
    mon = report_date - timedelta(days=report_date.weekday())
    mon = datetime(mon.year, mon.month, mon.day)
    
    current_date_block_is_history = False
    
    def clean_html(s):
        s = html.unescape(s)
        return re.sub(r'<[^>]+>', '', s).strip()

    for line in lines:
        text = clean_html(line)
        # Find dates 20YY/MM/DD or 20YYMMDD
        raw_dates = re.findall(r'20\d{2}/\d{2}/\d{2}|20\d{6}', text)
        if raw_dates:
            latest_date = None
            for d_str in raw_dates:
                d_str = d_str.replace('/', '')
                try:
                    d = datetime.strptime(d_str, '%Y%m%d')
                    if not latest_date or d > latest_date:
                        latest_date = d
                except:
                    pass
            if latest_date:
                if latest_date < mon:
                    current_date_block_is_history = True
                else:
                    current_date_block_is_history = False
        
        if current_date_block_is_history:
            history_lines.append(line)
        else:
            current_lines.append(line)

    def group_machines(lines_to_group):
        final_list = []
        groups = {}
        non_groupable = []
        
        for l in lines_to_group:
            txt = clean_html(l)
            if not txt: 
                non_groupable.append(l)
                continue
            
            # Pattern: HT-xxxx Description
            match = re.search(r'(HT-[\w\d]+|POLB-[\w\d]+|PQ-[\w\d]+)', txt)
            if match and ('改機' in txt or '升級' in txt):
                machine = match.group(1)
                desc = txt.replace(machine, '').strip()
                desc = re.sub(r'^[0-9]+\.', '', desc).strip()
                desc = re.sub(r'^[\d\s]+', '', desc).strip()
                
                if len(desc) > 5:
                    if desc not in groups:
                        groups[desc] = []
                    if machine not in groups[desc]:
                        groups[desc].append(machine)
                    continue
            
            non_groupable.append(l)
            
        final_list.extend(non_groupable)
        for desc, machines in groups.items():
            if len(machines) > 1:
                sorted_machines = sorted(machines)
                machines_str = '、'.join(sorted_machines)
                final_list.append(f'- {machines_str} {desc}')
            else:
                final_list.append(f'- {machines[0]} {desc}')
            
        return final_list

    new_current = group_machines(current_lines)
    all_lines = new_current + history_lines
    return '<br>'.join(all_lines)


def process_summary_file(file_path):
    print(f"Processing {file_path}")
    
    # Try to extract date from filename
    # Pattern: 115-3-15 -> 2026-03-15
    # Or YYYYMMDD
    filename = os.path.basename(file_path)
    
    report_date = datetime.now()
    
    # Match ROC date: 115-3-15
    m_roc = re.search(r'(\d{3})[-_](\d{1,2})[-_](\d{1,2})', filename)
    if m_roc:
        year = int(m_roc.group(1)) + 1911
        month = int(m_roc.group(2))
        day = int(m_roc.group(3))
        report_date = datetime(year, month, day)
    else:
        # Match YYYYMMDD
        m_ymd = re.search(r'(20\d{2})(\d{2})(\d{2})', filename)
        if m_ymd:
             report_date = datetime(int(m_ymd.group(1)), int(m_ymd.group(2)), int(m_ymd.group(3)))
    
    print(f"Using report date: {report_date.strftime('%Y-%m-%d')}")

    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    replacement_count = 0

    def row_processor(match):
        nonlocal replacement_count
        row_html = match.group(0)
        parts = re.split(r'(<td.*?>.*?</td>)', row_html, flags=re.DOTALL)
        td_parts_indices = [i for i, p in enumerate(parts) if p.startswith('<td')]
        
        # Action column is index 4 (5th column: No, Dept, Customer, Issue, Action...)
        if len(td_parts_indices) > 4:
            idx = td_parts_indices[4]
            original_td = parts[idx]
            
            if 'color:#ff0000' in original_td:
                start_match = re.search(r'(<span style="color:#ff0000">)', original_td)
                if start_match:
                    prefix = original_td[:start_match.end()]
                    remainder = original_td[start_match.end():]
                    
                    term_match = re.search(r'(</span>|<a\s|</td>)', remainder, re.DOTALL)
                    
                    if term_match:
                        inner = remainder[:term_match.start()]
                        terminator = term_match.group(1)
                        suffix = remainder[term_match.start():]
                        
                        new_inner = split_and_group_red_text_html(inner, report_date)
                        
                        if new_inner != inner:
                            replacement_count += 1
                            if terminator == '</span>':
                                new_td_content = prefix + new_inner + suffix
                            else:
                                new_td_content = prefix + new_inner + '</span>' + suffix
                                
                            parts[idx] = new_td_content

        return ''.join(parts)
    
    tbody_pattern = re.compile(r'<tbody>(.*?)</tbody>', re.DOTALL)
    new_content = tbody_pattern.sub(lambda m: f'<tbody>{re.sub(r'<tr.*?>(.*?)</tr>', row_processor, m.group(1), flags=re.DOTALL)}</tbody>', content)
    
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    print(f"Updates saved. Total replacements: {replacement_count}")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        target = sys.argv[1]
        process_summary_file(target)
    else:
        print("Usage: python script.py <path_to_summary_md>")
