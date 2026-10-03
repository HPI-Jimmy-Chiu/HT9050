#!/bin/bash
# AI(W906-FW-GATEVERDICT) 20260827: 把「逐項比對失敗集合」從紀律變成工具。
#
# 為什麼需要這支：20260827 凌晨查出 **W34 是在紅燈上 commit 的，而記錄寫成綠燈**。
# 機制不是粗心，是 marker 檔本身沒有鑑別力——
#
#   ctest 對「有任何測試失敗」一律回 exit 8，不論失敗的是 5 個還是 7 個。
#   所以 _<tag>_gate_{g,done}.txt 裡的 `G_EXIT=8` / `R_EXIT=8`
#   在 w23/w26/w28/w29/w30/w31/w32（真的 5 個失敗）
#   與 w34（Debug 7 個、Release 6 個）**逐位元組相同**。
#
# 政策裡「比對失敗集合逐項，不看數字」那條規則存在的理由就是為了擋這件事，
# 而 exit code 恰好長得像一個確認訊號。**把判定寫進工具，不要靠人每次記得做。**
#
# 用法：
#   bash tools/gateverdict.sh <tag>        # 判定 build_<tag>g 與 build_<tag>r
#   bash tools/gateverdict.sh <tag> g      # 只判定 Debug 側
#
# 輸出（每側一段）：
#   <side>_TOTAL=<n 個失敗>
#   <side>_EXTRA=<超出常駐清單的項目>      ← 非空就是 RED
#   <side>_ABSENT=<常駐清單裡這次沒失敗的>  ← 非空不代表壞，但要知道（可能是被跳過）
#   <side>_VERDICT=GREEN|RED|NO-LOG
#
# 離開碼：0=兩側都 GREEN；1=任一側 RED；2=用法錯或缺 log。

ROOT=/d/HT9045/HT9011UC_Cpp_V3.33.906.0

# 常駐失敗清單（docs/PT_CAMPAIGN_PLAN.md §7）。
# dfm2rc_idempotent 歷史上曾是第六項，目前實測為通過，故不列入基準——
# 若它再度失敗，會以 EXTRA 出現，那正是我們要看到的。
#
# AI(W906-P8) 20260920: **IniFiles 移除，清單從五項縮成四項。**
#   它的常駐失敗根因不是 TIniFile 的缺陷，是那支測試把**這台機器的組態值**
#   釘死成斷言（IO_CARD_TYPE==2 / TTL_CARD_TYPE==2 / HEATER_CTRL_TYPE==4，
#   而這台實際是 1 / 0 / 2）。⇒ 它在**任何組態不同的機器上都會紅**，
#   包含同事的機台 —— 那實際效果是訓練所有人忽略紅燈。
#   20260920 把它改成只驗讀取機制（鍵存在、型別解析、大小寫不敏感、
#   重讀一致），實測 50/50 通過。「值等於多少」搬到
#   `tools/machine_config_expect.py`（機台驗收，不進 gate）。
#   ⚠ 從現在起 IniFiles 若失敗，會以 EXTRA 出現 —— 那是真回歸，要查。
RESIDENT="GA1_ReadGeneralIni
config_db
config_loaders
ini_helpers"

TAG="$1"
[ -z "$TAG" ] && { echo "用法: bash tools/gateverdict.sh <tag> [g|r]" >&2; exit 2; }
SIDES="${2:-g r}"

rc=0
for s in $SIDES; do
  log="$ROOT/build_${TAG}${s}/ctest.log"
  up=$(echo "$s" | tr 'gr' 'GR')
  if [ ! -f "$log" ]; then
    echo "${up}_VERDICT=NO-LOG"
    rc=2
    continue
  fi

  # **這個守衛是承重的。** 一個還在寫的 ctest.log 沒有 "The following tests FAILED"
  # 區段，失敗集合會抽成空的 → EXTRA 空 → 判成 GREEN。
  # 那正是這支工具存在要擋的那類假綠燈，所以先要求整輪跑完的摘要行存在。
  if ! grep -q 'tests passed' "$log"; then
    echo "${up}_VERDICT=INCOMPLETE (ctest.log 尚無 'tests passed' 摘要行，該輪還在跑或被中斷)"
    rc=2
    continue
  fi

  # 失敗清單長這樣（前導 tab/空白）：	 14 - config_db (Failed)
  # 剝成純名字。註：ctest 的 log 是 CRLF，正則用 $ 搭 re.M 會靜默失配，
  # 這裡先 tr -d '\r'（與 scratchpad/link_closure.py 同一個坑）。
  names=$(sed -n '/The following tests FAILED/,$p' "$log" \
          | tr -d '\r' \
          | sed -nE 's/^[[:space:]]*[0-9]+ - (.+) \([A-Za-z]+\)$/\1/p' \
          | sort)

  n=$(printf '%s' "$names" | grep -c . )
  extra=$(comm -13 <(printf '%s\n' "$RESIDENT") <(printf '%s\n' "$names") | grep . | tr '\n' ' ')
  absent=$(comm -23 <(printf '%s\n' "$RESIDENT") <(printf '%s\n' "$names") | grep . | tr '\n' ' ')

  # AI(W906-GATE-SIMCFG) 20260918: 把「這一輪 gate 的是哪一種組態」印出來。
  # 沒有這一行，一個在模擬組態下跑出來的 GREEN 跟出貨組態的 GREEN 逐位元組相同,
  # 而它們的意義完全不同（模擬組態下 14 支是必然失敗的）。這跟本檔開頭 W34 的
  # 形狀一樣：證據的缺席長得跟證據一樣。
  cache="$ROOT/build_${TAG}${s}/CMakeCache.txt"
  cfg="UNKNOWN"
  if [ -f "$cache" ]; then
    if grep -q '^W906_NO_SOFT_SIMULTE:BOOL=ON' "$cache"; then
      cfg="SHIPPING (SOFT_SIMULTE undefined)"
    else
      cfg="SIMULATION (SOFT_SIMULTE defined) -- ⚠ harness 是對 SHIPPING 推導的，預期 14 支額外失敗"
    fi
  fi
  echo "${up}_CONFIG=$cfg"
  echo "${up}_TOTAL=$n"
  echo "${up}_FAILSET=$(printf '%s' "$names" | tr '\n' ' ')"
  echo "${up}_EXTRA=$extra"
  echo "${up}_ABSENT=$absent"
  if [ -n "$extra" ]; then
    echo "${up}_VERDICT=RED"
    rc=1
  else
    echo "${up}_VERDICT=GREEN"
  fi
done

# AI(W906-RELDEFER) 20260918: 「只跑 Debug」必須大聲，不可以靜默省略。
#
# 使用者 20260918 裁決：結案時間逼近，先把 Debug 穩定下來，Release 之後再補。
# 那個裁決本身是合理的——實測 build/（F5 跑的、機台上測的那顆）的
# CMAKE_BUILD_TYPE 是空字串，跟 gate 的 Debug 側是**同一種組態**，
# 而 Release 是目前沒有人在跑的組態。
#
# ⚠ 但它有一個明確的失效模式，而這支工具存在的理由就是擋這一類：
#   `gateverdict.sh <tag> g` 原本只印 G_ 那一段然後 exit 0。
#   三週後有人看到 exit 0 與一片 GREEN，會合理地以為兩側都驗過了。
#   這跟 w34「在紅燈上 commit 而記成綠燈」是同一個形狀——
#   **證據的缺席長得跟證據一樣。**
#
# 所以：少跑哪一側就明講哪一側沒跑，並讓最終判定字串自己帶著這個事實。
# 離開碼維持 0（依裁決，Debug 綠就是合格），鑑別力放在字串上。
case " $SIDES " in
  *" r "*) ;;
  *)
    echo "R_VERDICT=NOT-RUN"
    echo "R_NOTE=⚠ Release 側本輪未執行（20260918 使用者裁決：先穩定 Debug，之後補驗）。"
    echo "R_NOTE=⚠ 補驗條件：這棵樹出任何東西之前必須跑一次 Release，且第一個要看的是"
    echo "R_NOTE=  那 8 支用 epsilon 比對浮點的測試——-O3 會改變 x87 中間精度，而"
    echo "R_NOTE=  CMakeLists.txt 沒有 -ffloat-store / -fexcess-precision 保護。"
    echo "R_NOTE=  清單：ContactForce ExternFunction SCK_ART SCK_ART_Remainder"
    echo "R_NOTE=       cContact contactct_core cpublic_foundation ptw1_textprocess"
    ;;
esac

if [ $rc -eq 0 ]; then
  case " $SIDES " in
    *" r "*)
      # AI(W906-P4-SECS) 20260920: **不可以只憑「這次問的是 r」就說 BOTH。**
      #
      #   原本這裡直接 echo GREEN-BOTH，理由是 dualgate 第二次呼叫帶的是 r。
      #   但那等於**宣告一個本次呼叫從來沒有量過的事實** —— g 側的判定。
      #
      #   20260920 p4 實測撞到：g 側**建置失敗**（G_VERDICT=NO-LOG），
      #   r 側綠，於是 _p4_gate_done.txt 印出 OVERALL=GREEN-BOTH。
      #   只讀那個檔的人會直接 push，而 push 的判準（SHIPPING 側的失敗集合）
      #   根本沒被量過。一個會說謊的哨兵比沒有哨兵更糟。
      #
      #   ⇒ 去讀 g 側的 sentinel 對帳。
      _G_SENT="$ROOT/_${TAG}_gate_g.txt"
      if [ -f "$_G_SENT" ] && grep -q '^G_VERDICT=GREEN$' "$_G_SENT"; then
        echo "OVERALL=GREEN-BOTH"
      elif [ -f "$_G_SENT" ]; then
        _GV=$(grep -m1 '^G_VERDICT=' "$_G_SENT" | cut -d= -f2-)
        echo "OVERALL=RED  (Release 綠，但 Debug/SHIPPING 側是 ${_GV:-未知} -- ⛔ 不可 push)"
      else
        echo "OVERALL=UNKNOWN  (找不到 _${TAG}_gate_g.txt -- Debug 側沒有判定，⛔ 不可 push)"
      fi
      ;;
    *)       echo "OVERALL=GREEN-DEBUG-ONLY  (Release 未驗，見 R_NOTE)" ;;
  esac
fi
exit $rc
