'use strict';
// AI(W906-HTDESIGNER) 20261008 (EastSun「你自己編譯與驗證 不要又是我做之後 才發現問題 請修正後歸類到外掛」 / 「你不能自己偵測喔?」):
// when the debugger stops on an exception, what it means -- read from the exception text and the stack the debug
// adapter returns. The first rule is the one ES02 hit on 1008: WbMutex::lock -> EnterCriticalSection writing 0x00000014 =
// a CRITICAL_SECTION that is all zeros (its DebugInfo is NULL and EnterCriticalSection bumps DebugInfo->ContentionCount,
// at 0x14 on 32-bit Windows): the lock's constructor has not run yet, or its destructor already did -- global objects'
// order. Plain Node (no vscode).
//   frames: [{ name, path, line }] top first.  -> { title, why, phase, focus: frame | null, text }

const SYS = /(^|[\\/])(ntdll|kernel32|kernelbase|msvcrt|ucrtbase)\b|\bRtl\w+|^(Enter|Leave)CriticalSection$|[\\/]WebBridge[\\/]Sync\.h$|[\\/](mingw32|mingw64|include)[\\/](c\+\+|bits)[\\/]/i;

function addrOf(text) {
  const m = /(reading|writing|executing)\s+(?:location\s+)?(0x[0-9a-f]+)/i.exec(String(text || ''));
  return m ? { op: m[1].toLowerCase(), addr: parseInt(m[2], 16) } : null;
}

/** When it happened: before main (global constructors), at exit (global destructors / atexit), or while running. */
function phaseOf(frames) {
  const names = (frames || []).map(f => String(f.name || ''));
  if (names.some(n => /__do_global_dtors|__tcf_|_GLOBAL__sub_D_|\batexit\b|\b_?exit\b|__cxa_finalize|_execute_onexit/.test(n))) return 'exit';
  if (names.some(n => /__do_global_ctors|_GLOBAL__sub_I_|__main\b|_pei386_runtime_relocator/.test(n)) && !names.some(n => /^main\b|\bmain\(/.test(n) && !/__main/.test(n))) return 'init';
  return 'run';
}

/** The first frame in the program's own code (not Windows, the C runtime or the lock wrapper itself). */
function focusOf(frames) {
  for (const f of frames || []) {
    if (!f || !f.path) continue;
    if (SYS.test(f.path) || SYS.test(String(f.name || ''))) continue;
    return f;
  }
  return null;
}

function stackText(frames) {
  return (frames || []).map((f, i) => '#' + i + ' ' + (f.name || '?') + (f.path ? '  ' + f.path + (f.line ? ':' + f.line : '') : '')).join('\n');
}

// others: the other threads' stacks ([[frames]]) -- one of them exiting (1008 ES02: main returned early, its global
//  destructors ran, and the ELA worker still took g_mask's lock) makes it an exit-time problem, whatever this thread does
function hintOf(text, frames, others) {
  const a = addrOf(text);
  const exitElsewhere = (others || []).some(fr => phaseOf(fr) === 'exit');
  const ph = exitElsewhere ? 'exit' : phaseOf(frames);
  const focus = focusOf(frames);
  const names = (frames || []).map(f => String(f.name || '') + ' ' + String(f.path || ''));
  const inCs = names.slice(0, 6).some(n => /EnterCriticalSection|LeaveCriticalSection|RtlpEnterCriticalSection|WbMutex::(lock|unlock)|WbGuard|Sync\.h/.test(n));
  const when = ph === 'init' ? '發生在 main 之前（全域物件建立期間）' : exitElsewhere ? '發生在程式結束時：主執行緒正在結束程式、拆掉全域物件，這條背景執行緒還在用它們（結束前要先停掉背景執行緒）' :
    ph === 'exit' ? '發生在程式結束時（全域物件拆除期間）' : '發生在程式執行中';
  let title, why;
  // (1008 full test, audit H1: gdb says "SIGSEGV" with no address; 64-bit builds keep ContentionCount at 0x24, not 0x14)
  const av = /0xc0000005|access violation|SIGSEGV|segmentation fault/i.test(String(text || ''));
  if (av && inCs && (!a || (a.addr >= 0x10 && a.addr <= 0x30))) {
    title = '鎖（CRITICAL_SECTION）還沒建立或已經被拆掉';
    why = (a ? 'EnterCriticalSection 寫到 0x' + a.addr.toString(16) : '在拿鎖（EnterCriticalSection）時存取違規') + '：這個鎖的記憶體全是 0（DebugInfo＝NULL），代表它的建構子還沒跑、或解構子已經跑過。' +
      '通常是全域物件的建立／拆除順序（不同的連結順序──例如新建的 Ninja 資料夾──結果就不同）。' + when + '。' +
      (focus ? '看「' + focus.name + '」（' + focus.path + ':' + focus.line + '）用到的那個物件：改成第一次用到才建立、結束時不拆。' : '');
  } else if (av && a && a.addr < 0x10000) {
    title = '用到 NULL 指標（' + (a.op === 'writing' ? '寫' : a.op === 'reading' ? '讀' : '執行') + ' 0x' + a.addr.toString(16) + '）';
    why = '位址很小＝一個 NULL 指標加上成員偏移。' + when + '。' + (focus ? '看「' + focus.name + '」（' + focus.path + ':' + focus.line + '）裡是哪個指標還沒設好。' : '');
  } else {
    title = '除錯器停在例外';
    why = when + '。' + (focus ? '程式自己的第一層：「' + focus.name + '」（' + focus.path + ':' + focus.line + '）。' : '');
  }
  return { title, why, phase: ph, focus, text: String(text || '') + '\n\n' + title + '\n' + why + '\n\n' + stackText(frames) + '\n' };
}

/**
 * 1008 (EastSun「怎閃退了?」): wb_serve printed "no active recipe; refusing to serve" and quit -- why, from its launch env:
 * the SetUp.inf it reads (W906_SETUPINF_PATH, else D:\HT9045\SetUp.inf) missing / empty, or the recipe it names not in
 * IniData\Data. env: { NAME: value }; io: { exists(p), read(p) } (tests). -> { title, why, setupInf, recipe, fixFrom }
 */
function recipeHint(env, io) {
  const e = env || {};
  const setupInf = e.W906_SETUPINF_PATH || 'D:\\HT9045\\SetUp.inf';
  const dataRoot = (e.W906_INIDATA_ROOT || 'D:\\HT9045\\IniData').replace(/[\\/]+$/, '') + '\\Data';
  const fallback = 'D:\\HT9045\\SetUp.inf';
  let recipe = '';
  if (!io.exists(setupInf)) {
    return { title: '找不到 SetUp.inf，所以沒有作用中的配方', why: '啟動設定指定的 ' + setupInf + ' 不存在（W906_SETUPINF_PATH）。wb_serve 讀不到配方名稱，照設計拒絕服務並結束──不是當機。' +
      (setupInf !== fallback && io.exists(fallback) ? '可以從 ' + fallback + ' 複製一份過去。' : '在那裡放一個 SetUp.inf，第一行寫一個存在的配方名稱。'), setupInf, recipe, fixFrom: setupInf !== fallback && io.exists(fallback) ? fallback : null };
  }
  try { recipe = String(io.read(setupInf) || '').split(/\r?\n/)[0].trim(); } catch (x) { recipe = ''; }
  if (!recipe) return { title: 'SetUp.inf 是空的，所以沒有作用中的配方', why: setupInf + ' 第一行沒有配方名稱。', setupInf, recipe, fixFrom: null };
  if (!io.exists(dataRoot + '\\' + recipe)) return { title: '配方「' + recipe + '」不存在', why: setupInf + ' 指定的配方在 ' + dataRoot + ' 底下找不到。改成一個存在的配方名稱。', setupInf, recipe, fixFrom: null };
  return { title: '沒有作用中的配方', why: setupInf + ' 指定「' + recipe + '」而且資料夾在，可能是配方讀取失敗（例如 "Fail Open"）。看偵錯主控台前面幾行。', setupInf, recipe, fixFrom: null };
}

/**
 * 1008 (EastSun「我軟體又開不了了」: wb_serve ended 9 s after the start, no crash in the event log, nothing said): what a
 * program's exit code means. -> { code, hex, title, why } (title null = a normal end)
 */
function exitHint(code) {
  const n = Number(code);
  if (!isFinite(n)) return { code, hex: '', title: null, why: '' };
  const u = n < 0 ? n + 0x100000000 : n;
  const hex = '0x' + u.toString(16).toUpperCase().padStart(8, '0');
  const K = {
    0x00000000: null,
    0x00000002: ['程式自己結束（exit 2）', 'wb_serve 多半是拒絕啟動：沒有作用中的配方、設定檔讀不到。原因在偵錯主控台最後幾行。'],
    0xE0000027: ['被這台的資安軟體結束', '0xE0000027：端點防護把行程結束了（這台的 gdb 就是這樣被擋）。程式本身沒有錯；請 IT 把這支 exe／這個資料夾加入例外，或用 Release 再試。'],
    0xC0000005: ['存取違規（當機）', '0xC0000005：讀寫了不該碰的記憶體。用 Debug 跑，除錯器會停在那一行。'],
    0xC0000374: ['堆積（heap）損壞', '0xC0000374：某處寫壞了 heap，在 free/new 時才被抓到（CLAUDE.md 記過 x64 wb_publish 這一種）。'],
    0xC0000135: ['少一個 DLL', '0xC0000135：啟動時找不到需要的 DLL（例如 libwinpthread-1.dll、ADVMOT.dll）。'],
    0xC0000409: ['堆疊被寫壞（stack buffer overrun）', '0xC0000409：陣列越界寫到堆疊；用 Debug 跑會停在那裡。'],
    0xC00000FD: ['堆疊用完（stack overflow）', '0xC00000FD：遞迴太深或區域陣列太大。'],
    0xC000013A: ['被 Ctrl+C／關閉視窗結束', '0xC000013A：主控台收到中斷（關掉了終端機或按了 Ctrl+C）。'],
    0x40010004: ['被別的程式結束（TerminateProcess）', '0x40010004：工作管理員、⏹ 或其他程式把它結束了。'],
    0x00000001: ['程式回傳失敗（exit 1）', '程式自己以失敗結束。原因在偵錯主控台或終端機最後幾行。'],
  };
  if (u in K) { const k = K[u]; return k ? { code: n, hex, title: k[0], why: k[1] } : { code: n, hex, title: null, why: '' }; }
  return { code: n, hex, title: '程式結束（' + hex + '）', why: u >= 0xC0000000 ? 'Windows 例外碼 ' + hex + '：程式不正常結束。用 Debug 跑可以停在出事的地方。' : '程式以 ' + n + ' 結束。原因在偵錯主控台最後幾行。' };
}

module.exports = { hintOf, phaseOf, focusOf, addrOf, stackText, recipeHint, exitHint };
