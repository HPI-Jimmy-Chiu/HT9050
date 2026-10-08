const vscode=require('vscode');
exports.activate=ctx=>{
 const trace=[];let launched=false;
 ctx.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory('cppdbg',{createDebugAdapterDescriptor:()=>{
  const em=new vscode.EventEmitter();let seq=0;
  return new vscode.DebugAdapterInlineImplementation({onDidSendMessage:em.event,dispose:()=>em.dispose(),handleMessage:m=>{
   trace.push(m.command);em.fire({type:'response',seq:++seq,request_seq:m.seq,command:m.command,success:true,body:m.command==='initialize'?{supportsConfigurationDoneRequest:true}:{}});
   if(m.command==='initialize')em.fire({type:'event',seq:++seq,event:'initialized'});
   if(m.command==='launch')launched=true;
   if(m.command==='disconnect')em.fire({type:'event',seq:++seq,event:'terminated'});
  }});
 }}));
 return {trace,getLaunched:()=>launched};
};
