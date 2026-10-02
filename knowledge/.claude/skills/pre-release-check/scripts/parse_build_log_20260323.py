import re
from pathlib import Path

p = Path(r"D:\HT9045\Merge Report\2026\BuildVerify_ChangeToFloat_Restore_20260323.log")
b = p.read_bytes()
text = b.decode("utf-16le", errors="ignore")
lines = [x for x in text.splitlines() if x.strip()]
errors = [x for x in lines if re.search(r"\b(Error|Fatal|MAKE0000|Unable to open)\b", x, re.I)]
warnings = [x for x in lines if re.search(r"\bWarning\b", x, re.I)]
print(f"TOTAL_LINES={len(lines)}")
print(f"ERRORS={len(errors)}")
print(f"WARNINGS={len(warnings)}")
for i, e in enumerate(errors, 1):
	print(f"ERROR_LINE_{i}={e}")
print(f"LAST_LINE={lines[-1] if lines else ''}")
