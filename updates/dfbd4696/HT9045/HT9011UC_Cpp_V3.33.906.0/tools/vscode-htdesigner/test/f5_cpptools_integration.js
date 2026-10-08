const vscode=require('vscode'),fs=require('fs'),assert=require('assert');
exports.run=async()=>{const trace=[];try{
 await vscode.extensions.getExtension('ms-vscode.cpptools').activate();
  const vm=require('vm'),path=require('path'),root=path.resolve(__dirname,'..');
  const source=fs.readFileSync(path.join(root,'extension.js'),'utf8');
  const opts={'run.debug':false,'run.simulation':true};
  const api=Object.assign({},vscode,{workspace:{getConfiguration:()=>({get:k=>opts[k]})},tasks:{
   onDidEndTaskProcess:f=>vscode.tasks.onDidEndTaskProcess(e=>{}),onDidEndTask:f=>vscode.tasks.onDidEndTask(e=>{}),
   executeTask:t=>{t.execution=new vscode.ProcessExecution(t.execution.process,t.execution.args,t.execution.options);return vscode.tasks.executeTask(t);}
  }});
  const ctx={vscode:api,path,fs,console,__dirname:root,require:p=>p==='./lib/runmode'?require('../lib/runmode'):require(p)};vm.createContext(ctx);
  vm.runInContext(source.slice(source.indexOf('const RUN_DIRS ='),source.indexOf('\nclass CppNav'))+'\nthis.RunBar=RunBar;',ctx);
  const bar=Object.create(ctx.RunBar.prototype);bar.ctx={extensionPath:root};bar.hub={logEv(){},projSearch:{areas:()=>[]}};bar.freeProgram=async()=>{};bar.update=()=>{};ctx.runBar=bar;
  const marker='resolveDebugConfiguration: (folder, cfg) => {';const a=source.indexOf(marker),b=source.indexOf('\n        },',a);
  const early=vm.runInContext('((folder,cfg)=>{'+source.slice(a+marker.length,b)+'})',ctx);
  const tree=path.resolve(root,'../..'),folder=vscode.workspace.workspaceFolders[0];
  const original={type:'cppdbg',request:'launch',program:path.join(folder.uri.fsPath,'build_dbg','wb_serve.exe'),cwd:folder.uri.fsPath,environment:[],args:[],MIMode:'gdb',miDebuggerPath:'D:/HT9045/install/mingw32-16.2.0/mingw32/bin/gdb.exe',externalConsole:false};

 opts['run.debug']=false;opts['run.simulation']=true;

 const fixture=folder.uri.fsPath,fixtureTools=path.join(fixture,'tools');fs.mkdirSync(fixtureTools,{recursive:true});
 const portRoot=path.resolve(root,'../..');
 fs.copyFileSync(path.join(portRoot,'tools','build_with_status.ps1'),path.join(fixtureTools,'build_with_status.ps1'));
 let boot=fs.readFileSync(path.join(portRoot,'tools','open_boot_wait.ps1'),'utf8');boot=boot.replace('Start-Process -FilePath $shExe','Start-Process -WindowStyle Hidden -FilePath $shExe');fs.writeFileSync(path.join(fixtureTools,'open_boot_wait.ps1'),boot);
 const webDir=path.resolve(fixture,'../web');fs.mkdirSync(webDir,{recursive:true});fs.writeFileSync(path.join(webDir,'boot_wait.html'),'fixture');
 const hmiDir=path.join(fixture,'build_hmi_shell');fs.mkdirSync(hmiDir,{recursive:true});const hmiCpp=path.join(hmiDir,'hmi.cpp');
 fs.writeFileSync(hmiCpp,'#include <windows.h>\nint WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){Sleep(8000);return 0;}');
 require('child_process').execFileSync('C:/MinGW/bin/g++.exe',[hmiCpp,'-static','-mwindows','-o',path.join(hmiDir,'ht9045_hmi.exe')]);fs.writeFileSync(path.join(hmiDir,'WebView2Loader.dll'),'');
 const actual=require('../lib/runmode').resolve(original,fixture,true,false),mark=path.join(path.dirname(process.env.HTD_IT_REPORT),'dummy_started.txt');
 const cpp=path.join(fixture,'dummy.cpp');
 fs.writeFileSync(cpp,'#include <windows.h>\n#include <stdio.h>\nint main(){FILE*f=fopen('+JSON.stringify(mark.replace(/\\/g,'/'))+',"w");if(f){fputs("ok",f);fclose(f);}Sleep(2000);return 0;}');
 fs.writeFileSync(path.join(fixture,'CMakeLists.txt'),'cmake_minimum_required(VERSION 3.10)\nproject(F5Fixture CXX)\noption(W906_NO_SOFT_SIMULTE "fixture" OFF)\noption(BUILD_TESTING "fixture" OFF)\nadd_executable(wb_serve dummy.cpp)\ntarget_link_libraries(wb_serve -static)\n');
 const seedDir=path.dirname(original.program);fs.mkdirSync(seedDir,{recursive:true});fs.writeFileSync(path.join(seedDir,'CMakeCache.txt'),fs.readFileSync('D:/HT9045/Obj/V906/build_dbg/CMakeCache.txt','utf8'));
 const provider=vscode.debug.registerDebugConfigurationProvider('cppdbg',{resolveDebugConfiguration:async(f,c)=>{trace.push('resolve-enter');fs.writeFileSync(process.env.HTD_IT_REPORT+'.trace',JSON.stringify(trace));const v=await early(f,c);trace.push('resolve-return');fs.writeFileSync(process.env.HTD_IT_REPORT+'.trace',JSON.stringify(trace));return v;}});
 const deadline=new Promise((_,rej)=>setTimeout(()=>rej(new Error('timeout '+trace.join(','))),150000));
 const started=await Promise.race([vscode.debug.startDebugging(folder,Object.assign({},original,{name:'F5-real-cpptools-dummy'})),deadline]);
 assert(started);
 const end=Date.now()+10000;while(!fs.existsSync(mark)&&Date.now()<end)await new Promise(r=>setTimeout(r,100));
 assert(fs.existsSync(mark),'dummy main did not start');
 await vscode.debug.stopDebugging();
 fs.unlinkSync(mark);fs.writeFileSync(path.join(fixture,'CMakeLists.txt'),'cmake_minimum_required(VERSION 3.10)\nmessage(FATAL_ERROR "Intentional regression failure")\n');
 const failedStart=await vscode.debug.startDebugging(folder,Object.assign({},original,{name:'F5-failed-build-must-not-run-old-exe'}));
 assert.strictEqual(failedStart,false);assert(!fs.existsSync(mark),'old exe ran after failed build');
 provider.dispose();
 fs.writeFileSync(process.env.HTD_IT_REPORT,JSON.stringify({pass:true,trace,mainReached:true,taskEndEventsSuppressed:true,failedBuildBlocksOldExe:true}));
}catch(e){fs.writeFileSync(process.env.HTD_IT_REPORT,JSON.stringify({pass:false,trace,error:String(e.stack||e)}));throw e;}};
