
'use strict';
const vscode=require('vscode'),fs=require('fs'),assert=require('assert');
const {execute}=require('../lib/runmode');
exports.run=async()=>{
 const results=[];
 const events=[];function record(kind,e){events.push({kind,time:Date.now(),name:e.execution.task.name,rc:e.exitCode});fs.writeFileSync(process.env.HTD_IT_REPORT+'.events',JSON.stringify(events));}
 const listeners=[vscode.tasks.onDidStartTaskProcess(e=>record('start',e)),vscode.tasks.onDidEndTaskProcess(e=>record('processEnd',e)),vscode.tasks.onDidEndTask(e=>record('taskEnd',e))];
 async function one(name,code,cancel){
  const t=new vscode.Task({type:'ht9045-selected-mode',mode:name},vscode.TaskScope.Workspace,name,'ht9045',new vscode.ProcessExecution('powershell.exe',['-NoProfile','-Command',code]),[]);
  let sub;if(cancel)sub=vscode.tasks.onDidStartTaskProcess(e=>{if(e.execution.task.name===name)setTimeout(()=>e.execution.terminate(),300)});
  const rc=await execute(vscode,t);if(sub)sub.dispose();results.push({name,rc});return rc;
 }
 try{
  assert.strictEqual(await one('F5-real-success','exit 0'),0);
  assert.strictEqual(await one('F5-real-failure','exit 7'),7);
  assert.notStrictEqual(await one('F5-real-cancel','Start-Sleep -Seconds 30; exit 0',true),0);

  const vm=require('vm'),path=require('path'),root=path.resolve(__dirname,'..');
  const source=fs.readFileSync(path.join(root,'extension.js'),'utf8');
  const opts={'run.debug':false,'run.simulation':true};
  const api=Object.assign({},vscode,{workspace:{getConfiguration:()=>({get:k=>opts[k],inspect:k=>({globalValue:opts[k]})})},tasks:{
   onDidEndTaskProcess:vscode.tasks.onDidEndTaskProcess,onDidEndTask:vscode.tasks.onDidEndTask,
   executeTask:t=>{t.execution=new vscode.ProcessExecution(t.execution.process,t.execution.args.concat(['-PlanOnly']),t.execution.options);return vscode.tasks.executeTask(t);}
  }});
  const ctx={vscode:api,path,fs,console,__dirname:root,require:p=>p==='./lib/runmode'?require('../lib/runmode'):require(p)};vm.createContext(ctx);
  vm.runInContext(source.slice(source.indexOf('const RUN_DIRS ='),source.indexOf('\nclass CppNav'))+'\nthis.RunBar=RunBar;',ctx);
  const bar=Object.create(ctx.RunBar.prototype);bar.ctx={extensionPath:root};bar.hub={logEv(){},projSearch:{areas:()=>[]}};bar.freeProgram=async()=>{};bar.update=()=>{};ctx.runBar=bar;
  const marker='resolveDebugConfiguration: (folder, cfg) => {';const a=source.indexOf(marker),b=source.indexOf('\n        },',a);
  const early=vm.runInContext('((folder,cfg)=>{'+source.slice(a+marker.length,b)+'})',ctx);
  const tree=path.resolve(root,'../..'),folder=vscode.workspace.workspaceFolders[0];
  const original={type:'cppdbg',request:'launch',program:path.resolve(tree,'../Obj/V906/build_dbg/wb_serve.exe'),environment:[],args:[]};
  for(const sim of [true,false])for(const dbg of [true,false]){
   opts['run.simulation']=sim;opts['run.debug']=dbg;
   const cfg=await early(folder,original);
   assert(cfg);assert.strictEqual(cfg.noDebug,!dbg);assert.strictEqual(cfg.preLaunchTask,undefined);
   assert(cfg.program.includes('build_f5_'+(sim?'sim':'ship')+'_'+(dbg?'debug':'release')));
   results.push({name:'actual F5 resolver '+sim+'/'+dbg,rc:0});
  }

  // Real debug pipeline: use a fake DAP adapter, never start Handler/hardware.
  opts['run.simulation']=true;opts['run.debug']=false;
  const childRoot=require('path').dirname(process.env.HTD_IT_REPORT);
  const guiCpp=require('path').join(childRoot,'gui_child.cpp'),guiExe=require('path').join(childRoot,'gui_child.exe');
  const markerGui=require('path').join(childRoot,'gui_started.txt');
  fs.writeFileSync(guiCpp,'#include <windows.h>\n#include <stdio.h>\nint WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){FILE*f=fopen('+JSON.stringify(markerGui.replace(/\\/g,'/'))+',"w");if(f){fputs("started",f);fclose(f);}Sleep(8000);return 0;}');
  require('child_process').execFileSync('C:/MinGW/bin/g++.exe',[guiCpp,'-mwindows','-o',guiExe]);
  const guiPs=require('path').join(childRoot,'open_gui.ps1');fs.writeFileSync(guiPs,"Start-Process -FilePath '"+guiExe+"' -WindowStyle Hidden");
  const startGui=Date.now();
  assert.strictEqual(await one('F5-GUI-child',"& powershell.exe -NoProfile -File '"+guiPs+"'; exit 0"),0);
  assert(fs.existsSync(markerGui),'GUI fixture did not run');
  results.push({name:'GUI child task elapsed',elapsed:Date.now()-startGui});
  const trace=[];let adapterLaunched=false;
  const provider=vscode.debug.registerDebugConfigurationProvider('cppdbg',{resolveDebugConfiguration:async(f,c)=>{trace.push('resolver-enter');const out=await early(f,c);trace.push('resolver-return');return out;}});
  const adapter=await vscode.extensions.getExtension('local.f5-task-verification').activate();
  const deadline=new Promise((_,reject)=>setTimeout(()=>reject(new Error('debug pipeline timeout: '+trace.join(','))),20000));
  try{
   const started=await Promise.race([vscode.debug.startDebugging(folder,Object.assign({},original,{name:'F5-pipeline-verification'})),deadline]);
   assert(started);assert(adapter.getLaunched());await vscode.debug.stopDebugging();
   results.push({name:'actual debug pipeline reaches fake adapter',rc:0,trace:trace.concat(adapter.trace)});
  }finally{provider.dispose();}
  fs.writeFileSync(process.env.HTD_IT_REPORT,JSON.stringify({pass:true,results}));
 }catch(e){fs.writeFileSync(process.env.HTD_IT_REPORT,JSON.stringify({pass:false,error:String(e.stack||e),results}));throw e;}
};
