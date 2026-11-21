import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {JSDOM} from 'jsdom';
const raw=readFileSync(new URL('../../brands-restart-build/VisionOps_AI_Studio/templates/activate.html',import.meta.url),'utf8');
const start=raw.indexOf('<section id="vo-connect"'),end=raw.indexOf('<details class="wall-add"',start);
assert.ok(start>=0&&end>start,'Compiled camera wizard must exist');
const wizard=raw.slice(start,end);
const i=raw.indexOf('// VisionOps Connect wizard.'),j=raw.indexOf('async function onvifRequest(path,body){',i);
assert.ok(i>=0&&j>i);
const js=raw.slice(i,j).replaceAll('{{ csrf|tojson }}','"fixture"');
const dom=new JSDOM('<!doctype html><html><body>'+wizard+'<details class="wall-add"><summary>Manual</summary></details></body></html>',
 {runScripts:'outside-only',url:'http://127.0.0.1'});
const w=dom.window,d=w.document,get=id=>d.getElementById(id);
w.HTMLElement.prototype.scrollIntoView=()=>{};
w.cameras=[];w.loadCameras=async()=>true;
w.fetch=async()=>({ok:true,json:async()=>({devices:[
    {ip:'192.168.11.45',port:554,path:'',scheme:'rtsp',discovered_via:'rtsp'}],interfaces:1})});
w.eval(js);
const extra=JSON.parse(readFileSync(new URL('brands.json',import.meta.url),'utf8'));
const help=get('vo-help-brand'),rtsp=get('vo-connect-device-brand');
for(const b of extra){
  assert.ok([...help.options].some(o=>o.value===b.id&&o.textContent===b.label),b.id);
  assert.ok([...rtsp.options].some(o=>o.value===b.id),b.id);
  help.value=b.id;help.dispatchEvent(new w.Event('change'));
  assert.ok(get('vo-help-brand-guide').textContent.length>50,b.id);
}
get('vo-connect-scan').click();await new Promise(done=>setTimeout(done,0));
d.querySelector('.vo-connect-device').click();
rtsp.value='axis';rtsp.dispatchEvent(new w.Event('change'));
assert.equal(get('vo-connect-path').value,'/axis-media/media.amp');
rtsp.value='uniview';rtsp.dispatchEvent(new w.Event('change'));
assert.equal(get('vo-connect-path').value,'','Unknown-model streams must not use invented paths');
assert.equal(get('vo-connect-stream-details').open,true);
assert.equal(get('vo-connect-add').disabled,true);
assert.ok(get('vo-connect-manual-slot').contains(d.querySelector('.wall-add')));
console.log(extra.length+' manufacturer guides and 4 RTSP preset safeguards passed.');
