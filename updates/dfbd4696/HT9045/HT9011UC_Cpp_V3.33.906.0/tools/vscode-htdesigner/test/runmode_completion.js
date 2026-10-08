const assert=require('assert'),fs=require('fs'),os=require('os'),path=require('path');
const {execute}=require('../lib/runmode');
(async()=>{
const dir=fs.mkdtempSync(path.join(os.tmpdir(),'htd-completion-'));let serial=0;
async function one(arrange,want){
 const runId='run'+(++serial),resultFile=path.join(dir,runId+'.json'),task={definition:{runId}};
 let cb;const execution={task};let disposed=false;
 const api={tasks:{onDidEndTaskProcess:f=>{cb=f;return{dispose:()=>disposed=true}},executeTask:async()=>{setTimeout(()=>arrange({runId,resultFile,task,execution,event:e=>cb(e)}),10);return execution}}};
 const code=await execute(api,task,{runId,resultFile});assert.strictEqual(code,want);assert(disposed);if(fs.existsSync(resultFile))fs.unlinkSync(resultFile);
}
await one(x=>x.event({execution:{task:{definition:{runId:x.runId}}},exitCode:7}),7);
await one(x=>fs.writeFileSync(x.resultFile,JSON.stringify({runId:x.runId,code:0})),0);
await one(x=>{fs.writeFileSync(x.resultFile,JSON.stringify({runId:'stale',code:0}));setTimeout(()=>fs.writeFileSync(x.resultFile,JSON.stringify({runId:x.runId,code:1})),350)},1);
await one(x=>{fs.writeFileSync(x.resultFile,JSON.stringify({runId:x.runId,code:'0'}));setTimeout(()=>x.event({execution:x.execution,exitCode:7}),350)},7);
await one(x=>x.event({execution:x.execution,exitCode:undefined}),undefined);
fs.rmdirSync(dir);console.log('PASS: cloned task event, missing notification, stale/malformed results rejected, failure and cancel block launch.');
})().catch(e=>{console.error(e);process.exitCode=1;});
