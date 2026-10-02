#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Release Note Generator for HT-9xxx Handler Software

Extracts data from customer requirement proposal documents and generates
professional HTML release notes.

Usage:
    python generate_release_note.py --proposal <path_to_proposal.md> --version <V3.33.x.x> --output <output_dir>
"""

import os
import re
import sys
from datetime import datetime
from pathlib import Path
import argparse


def extract_version_from_folder(folder_path):
    """
    Extract software version from folder name pattern.
    
    Pattern: HT9011UC_Code_V{Major}.{Minor}.{Build}.{Revision}_{YYYYMMDD}_...
    Example: HT9011UC_Code_V3.33.899.1_20260325_JC_RY_S_SH → V3.33.899.1
    
    Args:
        folder_path: Path to the folder (or folder name)
    
    Returns:
        Version string (e.g., 'V3.33.899.1') or None if not found
    """
    folder_name = os.path.basename(folder_path)
    match = re.search(r'V(\d+\.\d+\.\d+\.\d+)', folder_name)
    if match:
        return f"V{match.group(1)}"
    return None


def extract_proposal_data(proposal_path):
    """Extract metadata from proposal Markdown file."""
    data = {
        'version': None,
        'engineer': None,
        'customer': None,
        'machine_model': None,
        'issue_summary': None,
        'fix_description': None,
        'proposal_date': None,
    }
    
    with open(proposal_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Extract version (V3.33.x.x pattern) from content
    # If folder name extraction will be used, this becomes secondary
    version_match = re.search(r'V3\.33\.\d+\.\d+', content)
    if version_match:
        data['version'] = version_match.group(0)
    
    # Extract engineer from "軟體工程師" or "Engineer" field
    engineer_match = re.search(r'(?:軟體工程師|Engineer)[:\s]+([A-Za-z0-9]+)', content)
    if engineer_match:
        data['engineer'] = engineer_match.group(1).strip()
    
    # Extract customer from "指定客戶" or "Customer" field
    customer_match = re.search(r'(?:指定客戶|Customer)[:\s]+(\d+_[A-Za-z0-9_]+)', content)
    if customer_match:
        data['customer'] = customer_match.group(1).strip()
    
    # Extract machine model
    model_match = re.search(r'(?:機台型號|Machine Model)[:\s]+([^\n|]+)', content)
    if model_match:
        data['machine_model'] = model_match.group(1).strip()
    
    # Extract issue summary
    issue_match = re.search(r'InArm[^\n]*out[^\n]*limit[^\n]*', content, re.IGNORECASE)
    if issue_match:
        data['issue_summary'] = issue_match.group(0).strip()
    else:
        # Fallback: extract from "問題概述" section
        summary_match = re.search(r'(?:問題概述|Issue)[:\s]+([^\n]+)', content)
        if summary_match:
            data['issue_summary'] = summary_match.group(1).strip()
    
    # Extract fix description (from "修改內容" or "Detailed Changes")
    fix_match = re.search(r'(?:修改內容說明|Detailed Changes).*?(?:^##|$)', content, re.DOTALL | re.MULTILINE)
    if fix_match:
        desc_text = fix_match.group(0).strip()
        # Clean up markdown formatting
        desc_text = re.sub(r'\n.*X Pitch.*\n', '', desc_text)  # Remove code blocks
        desc_text = re.sub(r'```.*?```', '', desc_text, flags=re.DOTALL)  # Remove backticks
        data['fix_description'] = desc_text[:200].strip() + "..."  # Truncate to 200 chars
    
    # Extract proposal date
    date_match = re.search(r'(?:提出日期|提案日期|Date)[:\s]+(\d{4}/\d{2}/\d{2})', content)
    if date_match:
        date_str = date_match.group(1)
        data['proposal_date'] = date_str
    
    return data


def append_to_release_note(data, output_path):
    """
    Append a new fix to existing Release Note for the same version.
    Supports multiple customers' fixes for the same version.
    """
    from pathlib import Path
    
    output_file = Path(output_path)
    
    # If file doesn't exist, create new one
    if not output_file.exists():
        return generate_html_release_note(data, str(output_file))
    
    # Read existing file
    with open(output_path, 'r', encoding='utf-8') as f:
        existing_content = f.read()
    
    # Extract customer and fix information for new entry
    customer = data['customer'] or 'Unknown'
    engineer = data['engineer'] or 'Unknown'
    issue_summary = data['issue_summary'] or 'Software bug fix'
    fix_description = data['fix_description'] or 'Fixed software issue'
    
    # Create new fix item HTML
    new_fix_item = f'''
            <div style="margin-bottom: 20px; padding-bottom: 15px; border-bottom: 1px solid #eee;">
                <div class="version-box">
                    <strong>Customer:</strong> {customer}<br>
                    <strong>Engineer:</strong> {engineer}<br>
                    <strong>Issue:</strong> {issue_summary}<br>
                    <strong>Status:</strong> FIXED ✓
                </div>
                <p>{fix_description}</p>
            </div>
'''
    
    # Insert new fix before footer
    modified_content = existing_content.replace(
        '        <div class="footer">',
        f'''        <!-- FIXES SECTION -->
        <div class="fixes-section">
{new_fix_item}
        </div>

        <div class="footer">'''
    )
    
    # Update footer timestamp
    modified_content = re.sub(
        r'<p><strong>Release Note Generated:</strong> \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}</p>',
        f'<p><strong>Release Note Last Updated:</strong> {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>',
        modified_content
    )
    
    # Write back to file
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(modified_content)
    
    return output_path


def generate_html_release_note(data, output_path):
    """Generate HTML release note from extracted data."""
    
    version = data['version'] or 'V3.33.999.0'
    engineer = data['engineer'] or 'Unknown'
    customer = data['customer'] or '000_Unknown'
    machine_model = data['machine_model'] or 'HT9045'
    issue_summary = data['issue_summary'] or 'Software bug fix'
    fix_description = data['fix_description'] or 'Fixed software issue'
    proposal_date = data['proposal_date'] or datetime.now().strftime('%Y/%m/%d')
    
    # Convert proposal date format
    try:
        date_obj = datetime.strptime(proposal_date, '%Y/%m/%d')
        release_date = date_obj.strftime('%Y-%m-%d')
    except:
        release_date = datetime.now().strftime('%Y-%m-%d')
    
    html_content = f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HT-9xxx Software Release Note {version}</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        body {{
            font-family: "Segoe UI", "Tahoma", Arial, sans-serif;
            background-color: #f5f5f5;
            color: #333;
            line-height: 1.6;
            padding: 20px;
        }}
        .container {{
            max-width: 900px;
            margin: 0 auto;
            background: white;
            padding: 40px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        .header {{
            border-bottom: 3px solid #0066cc;
            padding-bottom: 20px;
            margin-bottom: 30px;
        }}
        h1 {{
            color: #0066cc;
            font-size: 28px;
            margin: 10px 0;
        }}
        .subtitle {{
            color: #666;
            font-size: 14px;
            margin-top: 5px;
        }}
        .section {{
            margin: 25px 0;
        }}
        h2 {{
            color: #0066cc;
            font-size: 18px;
            border-left: 4px solid #0066cc;
            padding-left: 15px;
            margin-bottom: 15px;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 15px 0;
        }}
        th {{
            background-color: #0066cc;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 600;
        }}
        td {{
            border: 1px solid #ddd;
            padding: 12px;
        }}
        tr:nth-child(even) {{
            background-color: #f9f9f9;
        }}
        .version-box {{
            background-color: #e7f3ff;
            border: 1px solid #0066cc;
            padding: 15px;
            margin: 15px 0;
            border-radius: 4px;
        }}
        .version-box strong {{
            color: #0066cc;
        }}
        .footer {{
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            font-size: 12px;
            color: #666;
            text-align: right;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>HT-9xxx Software Release Note</h1>
            <p class="subtitle">IC Test Handler Software Release</p>
        </div>

        <div class="section">
            <h2>Release Information</h2>
            <table>
                <tr>
                    <th style="width: 30%">Item</th>
                    <th>Details</th>
                </tr>
                <tr>
                    <td><strong>Software Version</strong></td>
                    <td>{version}</td>
                </tr>
                <tr>
                    <td><strong>Release Date</strong></td>
                    <td>{release_date}</td>
                </tr>
                <tr>
                    <td><strong>Engineer</strong></td>
                    <td>{engineer}</td>
                </tr>
                <tr>
                    <td><strong>Customer</strong></td>
                    <td>{customer}</td>
                </tr>
                <tr>
                    <td><strong>Machine Model</strong></td>
                    <td>{machine_model}</td>
                </tr>
            </table>
        </div>

        <div class="section">
            <h2>Fix Summary</h2>
            <div class="version-box">
                <strong>Issue:</strong> {issue_summary}<br>
                <strong>Status:</strong> FIXED ✓
            </div>
        </div>

        <div class="section">
            <h2>Detailed Changes</h2>
            <p>{fix_description}</p>
        </div>

        <div class="footer">
            <p><strong>Release Note Generated:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
            <p><strong>For Support:</strong> Contact HonPrec RD5 Support Team</p>
        </div>
    </div>
</body>
</html>'''
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html_content)
    
    return output_path


def main():
    parser = argparse.ArgumentParser(
        description='Generate HTML release note from customer proposal document',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python generate_release_note.py \\
    --proposal u:\\共用區\\客戶需求單\\947_SCK\\20260210_軟體功能新增提案表_HT9046LS_InarmMotorOutLimit.md \\
    --output D:\\00_ReleaseNote\\outputs

  python generate_release_note.py \\
    --proposal proposal.md \\
    --version V3.33.899.1 \\
    --customer SCK \\
    --output ./release_notes
        '''
    )
    
    parser.add_argument('--proposal', type=str, required=True,
                       help='Path to proposal Markdown file')
    parser.add_argument('--version', type=str, default=None,
                       help='Software version (auto-detected if not provided)')
    parser.add_argument('--customer', type=str, default=None,
                       help='Customer code (auto-detected if not provided)')
    parser.add_argument('--output', type=str, default='D:\\00_ReleaseNote\\outputs',
                       help='Output directory for HTML file')
    
    args = parser.parse_args()
    
    # Validate input
    proposal_path = Path(args.proposal)
    if not proposal_path.exists():
        print(f'✗ Error: Proposal file not found: {proposal_path}', file=sys.stderr)
        sys.exit(1)
    
    # Extract data
    print(f'Extracting data from: {proposal_path}')
    data = extract_proposal_data(str(proposal_path))
    
    # Try to extract version from folder name first (highest priority for auto-detection)
    folder_version = extract_version_from_folder(str(proposal_path.parent.parent))
    if folder_version and not args.version:
        data['version'] = folder_version
        print(f'  ✓ Version auto-detected from folder: {folder_version}')
    
    # Override with command line args if provided
    if args.version:
        data['version'] = args.version
        print(f'  • Version overridden by parameter: {args.version}')
    if args.customer:
        data['customer'] = args.customer
    
    # Prepare output
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    version = data['version'] or 'V3.33.999.0'
    customer = data['customer'] or '000_Unknown'
    
    # Generate output filename without customer code
    # Format: HT-9xxx_Software_Release_Note_{VERSION}.html
    output_file = output_dir / f'HT-9xxx_Software_Release_Note_{version}.html'
    
    # Generate HTML with append support (multiple customers for same version)
    print(f'Generating/Updating release note...')
    if output_file.exists():
        print(f'  ℹ Existing Release Note found, appending new fix...')
        html_path = append_to_release_note(data, str(output_file))
    else:
        print(f'  ✓ Creating new Release Note...')
        html_path = generate_html_release_note(data, str(output_file))
    
    # Report results
    print(f'✓ Release note updated: {html_path}')
    print(f'  Version: {data["version"]}')
    print(f'  Engineer: {data["engineer"]}')
    print(f'  Customer: {data["customer"]}')
    print(f'  Machine: {data["machine_model"]}')
    print(f'\n  📌 Note: Release Note is version-based, not customer-specific.')
    print(f'     Multiple customers\' fixes for {version} will be consolidated here.')


if __name__ == '__main__':
    main()

