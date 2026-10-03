# -*- coding: utf-8 -*-
# =============================================================================
#  tools/realfile_guard.py  --  AI(W906-TEACH-W1) 20260919
#
#  把 `docs/PLAN_START_TO_RUN.md` §0.6 的備份紀律變成工具。
#
#  使用者 20260919 原話：「安全作法可以寫前備份，寫後檢查有問題還原，
#  最終測試沒問題後刪除備份，**這規則所有讀寫測試功能**」。
#
#  為什麼要工具而不是靠紀律：這件事每次驗證都要做，而「記得做」本身就是
#  會失敗的環節。而且「比對檔案內容」如果靠肉眼，等於沒比對。
#
#  用法
#  ---------------------------------------------------------------------------
#    python tools/realfile_guard.py snap  <標籤>      # 動檔之前
#    python tools/realfile_guard.py check <標籤>      # 跑完之後：逐檔比對內容
#    python tools/realfile_guard.py restore <標籤>    # 有問題時還原
#    python tools/realfile_guard.py drop  <標籤>      # 確認沒問題後刪備份
#
#  `check` 的輸出是**逐檔差異**，不是一句 OK：
#    - 沒變                    -> `same`
#    - 變了但只是多了幾個鍵    -> 印出實際多出來的行
#    - 變了而且有行被改寫/刪除 -> `CHANGED` 並印出 unified diff
#
#  ⚠ 這支只看**內容**，不看 mtime。mtime 會被「開檔即寫入同樣內容」改掉，
#    而那不是我們要抓的東西；我們要抓的是「值被改了」。
# =============================================================================
import difflib
import hashlib
import io
import os
import shutil
import sys

ROOT = r"D:\HT9045"
BK = os.path.join(ROOT, "HT9011UC_Cpp_V3.33.906.0", "_realfile_guard")

# §0.6 列的備份清單。刻意寫死 —— 「這次動到哪些檔」不該由跑的人臨時判斷。
TARGETS = [
    r"system\teach.ini",
    r"system\tech.dat",
    r"system\Gerneral.ini",
    r"system\Mot_Table.csv",
    r"system\IO_Table.csv",
    # AI(W906-LOT-W1) 20260919: P0-2 的 ReadWriteLotInfo 會讀寫
    # AuthPath+"config.ini"（AuthPath = D:\HT9045\config\，common.cpp:102）。
    r"config\config.ini",
    # AI(W906-P2a-CF) 20260919: ContactForceLoad 讀它（golden ContactForce.cpp:428）。
    #   現在是唯讀，但 golden 的 WriteFile()（:1180-1374）會把整份重寫，
    #   那一半落地的那一天這裡就是第一道防線。
    r"system\ContactInfo.ini",
    r"CurrentSetupData.txt",
    r"setup.inf",
    # AI(W906-T6-MAINPROC) 20260923: lot.start（wb_serve 的 SetLotStart）會把每批手臂統計清空並
    #   存檔 —— system\ArmByLot{0,1,2}.dat 與各自的 _backup.dat（cSocket.cpp:221 的 TArm("ArmByLot"+i)）。
    #   20260923 夜間比對 mtime 才發現：每一次用 lot.start 的探針都覆寫過這 6 個檔，而本清單原本沒有它們，
    #   所以探針前的內容無從還原。補上之後 snap 會先備份。
    r"system\ArmByLot0.dat",
    r"system\ArmByLot0_backup.dat",
    r"system\ArmByLot1.dat",
    r"system\ArmByLot1_backup.dat",
    r"system\ArmByLot2.dat",
    r"system\ArmByLot2_backup.dat",
    # AI(W906-MERGE-S11) 20260923: Steven 8deffd1 的 S11 把 fMain->Clarn_Data 接成活的（golden V912
    #   main.cpp:15458-15648 逐字翻），28 個呼叫點中 20 個從 no-op 變成真的清資料並寫這個檔
    #   （硬編路徑），其中含網頁 START 路徑 WebStart.cpp:1352/2355/2367。合併後的 START 探針會走到它。
    r"system\lastdata.dat",
    # AI(W906-A4-0) 20260924: A4（開機照 golden 呼叫 InitialHandler）之後開機會碰到這三個檔：
    #   LoadMachineRecord→SaveMachineRecord 在 MachRec.bInitialStart==false 時寫 machinerecord.dat（golden cinitial.cpp:8061；
    #   RealCCD 版同路徑），LoadCylinderLife 的 ReadWriteIni 會把缺的預設值寫進 MachineLife.ini（common.cpp:334）。
    r"system\machinerecord.dat",
    r"system\machinerecordRealCCD.dat",
    r"system\MachineLife.ini",
]


def active_recipe():
    """SetUp.inf 第一行 = 目前作用中的工單名。

    golden 的 GetLastOpenFN()（common.cpp:1252-1281）就是讀這個檔，
    所以這裡跟機台看到的是同一個答案，不是猜的。
    """
    p = os.path.join(ROOT, "SetUp.inf")
    if not os.path.isfile(p):
        return None
    try:
        first = open(p, "rb").read().decode("cp950", "replace").splitlines()
    except Exception:
        return None
    return first[0].strip() if first and first[0].strip() else None


def recipe_files():
    """作用中工單底下的所有檔（Contact.Data / Temperature.Data / Tester.Data ...）。

    ⚠ AI(W906-P3-OLP) 20260920 加的，起因是 P3 量到一個結構性缺口：
      OLP 的 35 支 setter **每一支**最後都是
          WriteIniData(DataPath + GetLastOpenFN() + "\\X.Data", ...)
      也就是說，這棵樹最常被程式碼寫到的真實檔就是配方樹，
      而 TARGETS 一個都沒涵蓋到。P3 當時只能在 probe 裡自己另外
      hash 一次目錄 —— 那正是「同一個坑要靠每個呼叫端各自記得」的形狀。

    ⚠ 這是**動態**的：換工單就換一組檔。所以 snap 與 check 之間如果有人
      換了工單，兩邊的集合會不一樣 —— check 對這件事有明確處理（見下面
      的 SKIPPED 與 MISSING 兩種輸出），不會靜默混過去。
    """
    name = active_recipe()
    if not name:
        return []
    d = os.path.join(ROOT, "IniData", "Data", name)
    if not os.path.isdir(d):
        return []
    out = []
    for fn in sorted(os.listdir(d)):
        full = os.path.join(d, fn)
        if os.path.isfile(full):
            out.append((os.path.join("IniData", "Data", name, fn), full))
    return out


def paths():
    out = []
    for rel in TARGETS:
        p = os.path.join(ROOT, rel)
        if os.path.isfile(p):
            out.append((rel, p))
    out.extend(recipe_files())
    return out


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


def cmd_snap(tag):
    d = os.path.join(BK, tag)
    if os.path.isdir(d):
        print("⚠ 備份已存在，不覆蓋：%s" % d)
        print("  （要重新開始請先 drop）")
        return 1
    os.makedirs(d)
    for rel, p in paths():
        dst = os.path.join(d, rel.replace("\\", "__"))
        shutil.copy2(p, dst)
        print("  snap %-28s %s  %d bytes" % (rel, md5(p)[:12], os.path.getsize(p)))
    print("備份於 %s" % d)
    return 0


def cmd_check(tag):
    d = os.path.join(BK, tag)
    if not os.path.isdir(d):
        print("找不到備份：%s" % d)
        return 2
    bad = 0
    for rel, p in paths():
        src = os.path.join(d, rel.replace("\\", "__"))
        if not os.path.isfile(src):
            # AI(W906-LOT-W1) 20260919: 這裡原本無條件算 bad，但有第二種可能：
            # **TARGETS 在 snap 之後被加了新檔** —— 那不是檔變了，
            # 是備份沒涵蓋到。兩者混為一談會讓這支工具報假紅，
            # 而一個會報假紅的哨兵等於沒有哨兵。
            print("  %-28s SKIPPED (不在這份備份裡，snap 之後才加進 TARGETS)" % rel)
            continue
        if md5(src) == md5(p):
            print("  %-28s same" % rel)
            continue
        bad += 1
        print("  %-28s CHANGED" % rel)
        try:
            a = io.open(src, encoding="cp950", errors="replace").read().splitlines()
            b = io.open(p, encoding="cp950", errors="replace").read().splitlines()
        except Exception:
            print("      （二進位檔，只比 md5）")
            continue
        for ln in list(difflib.unified_diff(a, b, "備份", "現在", lineterm=""))[:40]:
            print("      %s" % ln)
    # AI(W906-P3-OLP) 20260920: 反向掃描 —— 備份裡有、現在沒有的檔。
    # 上面的迴圈走的是「現在有的檔」，所以**被刪掉的檔它看不見**。
    # 加了動態的配方樹之後這不再是假想：換工單就會整組檔對不上，
    # 而「安靜地少了 16 個檔」跟「全部未變」在舊版輸出裡長得一模一樣。
    now = set(rel.replace("\\", "__") for rel, _ in paths())
    for fn in sorted(os.listdir(d)):
        if fn not in now:
            bad += 1
            print("  %-28s MISSING（備份裡有，現在的清單裡沒有 ——"
                  " 檔被刪了，或工單被換過）" % fn.replace("__", "\\"))

    print()
    print("結論：%s" % ("全部未變，可以 drop" if bad == 0
                        else "有 %d 個檔變了/不見了 —— 逐條確認是不是預期中的那一個" % bad))
    return 0 if bad == 0 else 1


def cmd_restore(tag):
    d = os.path.join(BK, tag)
    if not os.path.isdir(d):
        print("找不到備份：%s" % d)
        return 2
    for rel, p in paths():
        src = os.path.join(d, rel.replace("\\", "__"))
        if os.path.isfile(src):
            shutil.copy2(src, p)
            print("  restore %s" % rel)
    return 0


def cmd_drop(tag):
    d = os.path.join(BK, tag)
    if os.path.isdir(d):
        shutil.rmtree(d)
        print("已刪備份 %s" % d)
    else:
        print("沒有備份可刪")
    return 0


def main():
    if len(sys.argv) < 3:
        print(__doc__ or "用法：python tools/realfile_guard.py "
                         "{snap|check|restore|drop} <標籤>")
        return 2
    cmd, tag = sys.argv[1], sys.argv[2]
    fn = {"snap": cmd_snap, "check": cmd_check,
          "restore": cmd_restore, "drop": cmd_drop}.get(cmd)
    if fn is None:
        print("未知指令 %s" % cmd)
        return 2
    return fn(tag)


if __name__ == "__main__":
    sys.exit(main())
