# `.res` Binary 版本號 Patch（Step 2-d）

> **BCB6 限制**：`make -f HT9045.mak -B` 不會從 `.bpr` 重新產生 `HT9045.res`（只有 IDE 開啟專案才會）。
> `.bpr` 的 `Build=` / `FileVersion=` 設定必須**額外 patch `.res` binary** 才能反映到 EXE。
>
> NSIS 的 `GetFileVersion` 讀取 **VS_FIXEDFILEINFO 二進位 DWORD**；若只更新字串表，installer 檔名仍顯示舊版本號。

兩個 patch 均需執行。

---

## Patch 1 — StringFileInfo UTF-16LE 字串

腳本：`d:\AI_TempFile\_patch_res_version.py`

```python
old_str = "3.33.OLD.0".encode('utf-16-le')
new_str = "3.33.NEW.1".encode('utf-16-le')

res_path = r"D:\HT9045\<版本>\HT9045.res"
with open(res_path, 'rb') as f:
    data = bytearray(f.read())

idx = 0
count = 0
while True:
    pos = data.find(old_str, idx)
    if pos == -1:
        break
    data[pos:pos + len(old_str)] = new_str
    idx = pos + len(new_str)
    count += 1

with open(res_path, 'wb') as f:
    f.write(data)
print(f"Patched {count} occurrence(s) in .res")
```

---

## Patch 2 — VS_FIXEDFILEINFO `dwFileVersionLS` DWORD

腳本：`d:\AI_TempFile\_patch_res_fixedfileinfo.py`（同時 patch `.res` 與 `.exe`）

```python
import struct

SIG = struct.pack('<I', 0xFEEF04BD)  # VS_FIXEDFILEINFO 簽章

# dwFileVersionLS offset = SIG_offset + 12
# 公式：(Release << 16) | Build
# 例：903,0 => 0x03870000  →  903,1 => 0x03870001

old_ls = struct.pack('<I', 0x03870000)
new_ls = struct.pack('<I', 0x03870001)

for path in [r"D:\HT9045\<版本>\HT9045.res", r"D:\HT9045\EXE\HT9045.exe"]:
    with open(path, 'rb') as f:
        data = bytearray(f.read())
    pos = data.find(SIG)
    if pos == -1:
        print(f"  [SKIP] VS_FIXEDFILEINFO not found in {path}")
        continue
    ls_offset = pos + 12
    found = data[ls_offset:ls_offset + 4]
    if found == old_ls:
        data[ls_offset:ls_offset + 4] = new_ls
        with open(path, 'wb') as f:
            f.write(data)
        print(f"  [OK] Patched {path}")
    else:
        print(f"  [WARN] Expected {old_ls.hex()}, found {found.hex()} in {path}")
```

---

## patch 後驗證

```powershell
$v = (Get-Item "D:\HT9045\EXE\HT9045.exe").VersionInfo
Write-Host "$($v.FileVersion) | Build=$($v.FilePrivatePart)"
# 預期：3.33.903.1 | Build=1
```

> patch 後**不需重新全編譯**；若 EXE 已連結，直接 patch EXE 即可（Patch 2）。若需更新 `.res` 再重連結，執行 `make -f HT9045.mak`（incremental，不加 `-B`）。
