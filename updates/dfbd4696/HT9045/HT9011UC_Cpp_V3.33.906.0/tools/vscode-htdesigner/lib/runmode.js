'use strict';
const path = require('path');
// One choice for the native F5 resolver and the toolbar (which invokes F5).
function resolve(cfg, tree, sim, debug) {
  // (1007: the old ▶ (run.likeF5 off) built its own folder already: __htdOwnBuild -- not taken over and built twice)
  if (!cfg || cfg.request !== 'launch' || !/wb_serve\.exe$/i.test(String(cfg.program || '')) || !tree || cfg.__htdOwnBuild) return null;
  const expand = s => String(s).split('${workspaceFolder}').join(tree);
  const original = expand(cfg.program), baseline = cfg.__htdBaseline || path.dirname(original);
  const mode = (sim ? 'sim' : 'ship') + '_' + (debug ? 'debug' : 'release');
  // (1007 audit D6: one folder per launch configuration's own build folder too -- the plain and the -O2 entry shared
  //  build_f5_<mode>, and whichever configured it first decided the flags of both)
  const dir = path.join(path.dirname(baseline), 'build_f5_' + mode + '__' + path.basename(baseline));
  const env = cfg.environment || [];
  const url = (env.find(e => e.name === 'W906_HMI_URL') || {}).value || '';
  // (1007 audit D12: the port after "host:" -- not a ":" inside [::1]; else --port N of the arguments)
  const argv = Array.isArray(cfg.args) ? cfg.args.map(String) : [];
  const pi = argv.indexOf('--port');
  const port = (/^[a-z]+:\/\/(?:\[[^\]]*\]|[^/:]*):(\d+)/i.exec(url) || [])[1] || (pi >= 0 && /^\d+$/.test(argv[pi + 1] || '') ? argv[pi + 1] : '') || '8045';
  const taskName = 'HT9045 F5 ' + mode + ' [' + tree + ']';
  // (Jimmy 1007: the extension runs this task itself before the start (execute below) and takes preLaunchTask away --
  //  VS Code could not find an extension task by its plain name)
  const next = Object.assign({}, cfg, { program: path.join(dir, 'wb_serve.exe'), noDebug: !debug,
    preLaunchTask: taskName, __htdSelectedMode: true, __htdMode: mode, __htdBaseline: baseline });
  if (!debug) next.ignoreRunWithoutDebuggingWarnings = true;
  return { cfg: next, tree, baseline, dir, sim, debug, taskName,
    // (1007 audit D11: the target URL encoded -- its own "&machine=HT9050" was cut off by the wait page's split on "&")
    query: 'port=' + port + '&built=1&target=' + encodeURIComponent(url || 'http://127.0.0.1:' + port + '/background.html?mode=debug') };
}
// Execute the actual Task object; do not rely on a late-discovered provider name.
function execute(vscode, task, options) {
  const o=options||{},fs=require('fs');
  return new Promise((resolve, reject) => {
    let execution, ended, done=false, stopSub, timer;
    const matches=e=>e&&e.execution&&((execution&&e.execution===execution)||e.execution.task===task||
      (o.runId&&e.execution.task&&e.execution.task.definition&&e.execution.task.definition.runId===o.runId));
    const cleanup=()=>{sub.dispose();if(stopSub)stopSub.dispose();if(timer)clearInterval(timer);};
    const finish=(code,via)=>{if(done)return;done=true;cleanup();if(o.log)o.log('F5 建置結果：'+String(code)+'（'+via+'）');resolve(code);};
    const sub=vscode.tasks.onDidEndTaskProcess(e=>{
      if(matches(e))finish(e.exitCode,'工作事件');else if(!execution)ended=e;
    });
    if(vscode.tasks.onDidEndTask)stopSub=vscode.tasks.onDidEndTask(e=>{if(matches(e))setTimeout(()=>{if(!readResult())finish(undefined,'工作結束但沒有成功結果');},200);});
    const readResult=()=>{
      if(!o.resultFile||!o.runId||done)return false;
      try{const result=JSON.parse(fs.readFileSync(o.resultFile,'utf8'));if(result.runId===o.runId&&Number.isInteger(result.code)){
        finish(result.code,'本次建置結果檔');return true;
      }}catch(e){/* absent or incomplete file: never assume success */}
      return false;
    };
    let absentSince=0;
    if(o.resultFile)timer=setInterval(()=>{
      if(readResult()||done||!execution)return;
      const active=vscode.tasks.taskExecutions;
      if(Array.isArray(active)&&!active.some(e=>matches({execution:e}))){
        if(!absentSince)absentSince=Date.now();
        if(Date.now()-absentSince>2000)finish(undefined,'工作已結束但沒有本次成功結果');
      }else absentSince=0;
    },200);
    Promise.resolve().then(()=>vscode.tasks.executeTask(task)).then(e=>{
      execution=e;if(ended&&ended.execution===e)finish(ended.exitCode,'工作事件');readResult();
    },e=>{cleanup();reject(e);});
  });
}
module.exports = { resolve, execute };
