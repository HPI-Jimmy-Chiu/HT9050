'use strict';
// Developer-only, offline: execute real mode resolver and registered F5 path.
const assert=require('assert'),fs=require('fs'),vm=require('vm'),path=require('path');
const root=path.resolve(__dirname,'..'),mode=require('../lib/runmode');
const tree='D:/HT9045/HT9011UC_Cpp_V3.33.906.0';
const base={type:'cppdbg',request:'launch',program:'${workspaceFolder}/../Obj/V906/build_dbg/wb_serve.exe',environment:[{name:'W906_HMI_URL',value:'http://127.0.0.1:8045/background.html?mode=debug'},{name:'W906_TEACH_INI_PATH',value:'simulation/teach.ini'}],args:['--port','8045'],preLaunchTask:'old Debug task'};
for(const sim of [true,false])for(const debug of [true,false]){
 const m=mode.resolve(base,tree,sim,debug);
 assert.strictEqual(m.cfg.noDebug,!debug);assert(m.dir.endsWith('build_f5_'+(sim?'sim':'ship')+'_'+(debug?'debug':'release')+'__build_dbg'));
 assert.strictEqual(m.cfg.environment,base.environment);assert.strictEqual(m.cfg.args,base.args);assert.notEqual(m.cfg.preLaunchTask,base.preLaunchTask);
 assert.strictEqual(mode.resolve(m.cfg,tree,sim,debug).dir,m.dir);
}
assert.strictEqual(mode.resolve({...base,request:'attach'},tree,true,false),null);
assert.strictEqual(mode.resolve({...base,program:'other.exe'},tree,true,false),null);
assert.strictEqual(mode.resolve({...base,__htdOwnBuild:true},tree,true,false),null);
const source=fs.readFileSync(path.join(root,'extension.js'),'utf8'),options={'run.debug':false,'run.simulation':true};let called=[];let explicitSet=false;
class Task{constructor(def,scope,name,src,execution){Object.assign(this,{definition:def,scope,name,source:src,execution});}}
class ProcessExecution{constructor(process,args,options){Object.assign(this,{process,args,options});}}
const vscode={Task,ProcessExecution,TaskScope:{Workspace:2},workspace:{getConfiguration:()=>({get:k=>options[k],inspect:k=>({globalValue:explicitSet?options[k]:undefined})})},commands:{executeCommand:async c=>called.push(c)}};
const context={vscode,path,fs,console,require:p=>p==='./lib/runmode'?mode:require(p),__dirname:root};vm.createContext(context);
vm.runInContext(source.slice(source.indexOf('const RUN_DIRS ='),source.indexOf('\nclass CppNav'))+'\nthis.RunBar=RunBar;',context);
const bar=Object.create(context.RunBar.prototype);bar.ctx={extensionPath:root};bar.hub={logEv(){},projSearch:{areas:()=>[{area:'port',root:tree}]}};bar.state=()=> 'idle';bar.update=()=>{};bar.freeProgram=async()=>{};bar.freeTreeWbServe=async()=>[];
context.runBar=bar;
let ends=[],executed=[],nextExit=0;vscode.tasks={onDidEndTaskProcess:f=>{ends.push(f);return{dispose:()=>{ends=ends.filter(x=>x!==f)}}},executeTask:async task=>{executed.push(task);const e={task};setTimeout(()=>ends.slice().forEach(f=>f({execution:e,exitCode:nextExit})),0);return e;}};
(async()=>{
// Actual early resolver: occurs before VS Code runs preLaunchTask.
const marker='resolveDebugConfiguration: (folder, cfg) => {';const a=source.indexOf(marker),b=source.indexOf('\n        },',a);
const early=vm.runInContext('((folder,cfg)=>{'+source.slice(a+marker.length,b)+'})',context);
const folder={uri:{fsPath:tree}};
// (never set on the toolbar: F5 starts the launch configuration as it is -- EastSun 1007)
assert.strictEqual(await early(folder,base),base);
explicitSet=true;
const cfg=await early(folder,base);
assert(/build_f5_sim_release__build_dbg[\\/]wb_serve\.exe$/.test(cfg.program));assert.strictEqual(cfg.noDebug,true);
const task=executed.at(-1);assert(task);assert(task.execution.args.includes('0'));assert(task.execution.args.includes('1'));
assert(task.execution.args.includes(path.join(root,'media','htd_f5_mode.ps1')));
// Toolbar uses native F5, then the same early resolver above decides the mode.
await bar.buildAndStart();assert.deepStrictEqual(called,['workbench.action.debug.start']);
 options['run.debug']=true;const dbg=await early(folder,base);assert.strictEqual(dbg.noDebug,false);assert(dbg.program.includes('build_f5_sim_debug'));
 options['run.simulation']=false;const ship=await early(folder,base);assert(ship.program.includes('build_f5_ship_debug'));
 options['run.debug']=false;const rel=await early(folder,base);assert(rel.program.includes('build_f5_ship_release'));assert.strictEqual(rel.noDebug,true);
 assert.strictEqual(cfg.preLaunchTask,undefined);
 nextExit=7;assert.strictEqual(await early(folder,base),undefined);
 nextExit=undefined;assert.strictEqual(await early(folder,base),undefined);
 assert.strictEqual(bar.launching,0);
 assert(!base.noDebug && base.preLaunchTask==='old Debug task');
 console.log('PASS: actual F5 early provider and toolbar share selected mode; all four combinations, task/exe agree, original env/args retained, attachment/unrelated executables untouched.');
})().catch(e=>{console.error(e);process.exitCode=1;});

assert(!source.includes("likeF5 ? null : 'ht9045Designer.run.toggle"));
