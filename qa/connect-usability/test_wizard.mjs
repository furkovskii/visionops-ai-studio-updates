// Visual behavior regression for VisionOps Connect v0.13.5.
// No physical cameras, browser services, user accounts or cloud requests.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {JSDOM} from 'jsdom';
const root=new URL('./',import.meta.url);
const html=readFileSync(new URL('wizard.html.inc',root),'utf8');
const script=readFileSync(new URL('wizard.js.inc',root),'utf8');
const dom=new JSDOM('<!doctype html><html><body>'+html+'<details class="wall-add"><summary>Eski manuel ayarlar</summary><form id="ip-form"></form></details></body></html>',
  {url:'http://127.0.0.1',runScripts:'outside-only'});
const w=dom.window,d=w.document,get=id=>d.getElementById(id),tick=()=>new Promise(done=>setTimeout(done,0));
w.HTMLElement.prototype.scrollIntoView=()=>{};
let cameras=[],requests=[],added=[],mode='none',pendingCheck=null;
w.cameras=cameras;
w.loadCameras=async data=>{
  added.push(data);
  if(data?.action==='add'){
    cameras=[...cameras,{id:'camera'+added.length,name:data.label,status:'Canlı'}];w.cameras=cameras;
  }
  return true;
};
w.fetch=async(url,opts)=>{
  const body=JSON.parse(opts.body);requests.push({url,body,csrf:opts.headers['X-CSRF-Token']});
  if(url.endsWith('/scan')){
    const devices=mode==='none'||mode==='failure'?[]:
      mode==='rtsp'?[{ip:'192.168.10.44',port:8554,path:'',scheme:'rtsp',discovered_via:'rtsp'}]:
      [{ip:'192.168.10.23',port:80,path:'/onvif/device_service',scheme:'http',discovered_via:'onvif'}];
    if(mode==='failure')return {ok:false,json:async()=>({error:'tarama hatası'})};
    return {ok:true,json:async()=>({devices,interfaces:1})};
  }
  if(mode==='hold'){
    return await new Promise(resolve=>{pendingCheck=resolve;});
  }
  if(mode==='rtsp')return {ok:true,json:async()=>({reachable:true,stream_verified:false,code:'rtsp',help:'RTSP servisi yanıt verdi',profiles:[]})};
  if(mode==='network')return {ok:true,json:async()=>({reachable:false,stream_verified:false,code:'network',help:'Cihaz erişilemiyor',profiles:[]})};
  return {ok:true,json:async()=>({reachable:true,stream_verified:false,code:'profiles',help:'Yayın profilleri',
      profiles:[{token:'a',name:'Kanal A',mode:'media1'},{token:'b',name:'Kanal B',mode:'media2'}]})};
};
w.eval(script.replace('__VO_CSRF_VALUE__','csrf_fixture'));
assert.ok(get('vo-scan-progress').hidden);
assert.ok(get('vo-connect-fallback').hidden);
assert.ok(get('vo-connect-manual-slot').contains(d.querySelector('.wall-add')),'Manual form must remain inside wizard');
assert.ok(get('vo-connect-manual-slot').hidden);
get('vo-connect-scan').click();
assert.equal(get('vo-scan-progress').hidden,false,'Animation must appear immediately, without popup');
await tick();
assert.equal(get('vo-scan-progress').hidden,true);
assert.equal(get('vo-connect-fallback').hidden,false,'Missing camera should show inline recovery');
assert.equal(d.querySelectorAll('.vo-help-card').length,3);
assert.equal(requests[0].body.extended,true,'One click must scan ONVIF and bounded RTSP');
assert.equal(requests[0].csrf,'csrf_fixture');
assert.equal(d.querySelector('.wall-add').open,false);
get('vo-help-brand').value='tapo';get('vo-help-brand').dispatchEvent(new w.Event('change'));
assert.match(get('vo-help-brand-guide').textContent,/Kamera Hesabı/);
get('vo-help-question').value='nvr';get('vo-help-ask').click();
assert.match(get('vo-help-answer').textContent,/NVR\/DVR/);
get('vo-manual-reveal').click();
assert.equal(get('vo-connect-manual-slot').hidden,false);
assert.equal(d.querySelector('.wall-add').open,true);
mode='rtsp';get('vo-scan-retry').click();await tick();
assert.equal(get('vo-connect-fallback').hidden,true);
assert.equal(d.querySelectorAll('.vo-connect-device').length,1);
d.querySelector('.vo-connect-device').click();
assert.equal(get('vo-connect-path-wrap').hidden,false);
assert.equal(get('vo-connect-stream-details').open,false);
get('vo-connect-device-brand').value='hikvision';
get('vo-connect-device-brand').dispatchEvent(new w.Event('change'));
assert.equal(get('vo-connect-path').value,'/Streaming/Channels/101');
get('vo-connect-user').value='operator';get('vo-connect-pass').value='private-test-secret';
get('vo-connect-check').click();await tick();
assert.equal(get('vo-connect-add').disabled,false);
get('vo-connect-form').dispatchEvent(new w.Event('submit',{bubbles:true,cancelable:true}));await tick();
assert.equal(added.at(-1).action,'add');
assert.equal(added.at(-1).port,8554);
assert.equal(added.at(-1).path,'/Streaming/Channels/101');
assert.equal(get('vo-connect-pass').value,'');
assert.match(get('vo-connect-help').textContent,/Canlı görüntü doğrulandı/);
mode='onvif';get('vo-connect-scan').click();await tick();
d.querySelector('.vo-connect-device').click();
get('vo-connect-user').value='operator';get('vo-connect-pass').value='different-test-secret';
get('vo-connect-check').click();await tick();
assert.equal(get('vo-connect-profile').options.length,2);
get('vo-connect-profile').value='b';
get('vo-connect-form').dispatchEvent(new w.Event('submit',{bubbles:true,cancelable:true}));await tick();
assert.equal(added.at(-1).onvif_mode,'media2');
assert.equal(added.at(-1).profile,'b');
// Editing credentials while a real network request is pending invalidates that result.
get('vo-connect-scan').click();await tick();
d.querySelector('.vo-connect-device').click();
get('vo-connect-user').value='operator';get('vo-connect-pass').value='test-scrub-secret';
mode='hold';get('vo-connect-check').click();await tick();
assert.ok(pendingCheck,'Expected pending permission verification');
get('vo-connect-pass').value='changed-password';
get('vo-connect-pass').dispatchEvent(new w.Event('input',{bubbles:true}));
pendingCheck({ok:true,json:async()=>({reachable:true,code:'profiles',profiles:[{token:'x',name:'Should not appear'}]})});
await tick();
assert.equal(get('vo-connect-add').disabled,true);
assert.equal(get('vo-connect-profile').options.length,1);
assert.equal(get('vo-connect-stepper')===null,true);
mode='failure';get('vo-connect-scan').click();await tick();
assert.equal(get('vo-connect-fallback').hidden,false);
assert.equal(get('vo-scan-progress').hidden,true,'Progress indicator must clear even on error');
assert.equal(get('vo-connect-manual-slot').hidden,false,'Manual controls should stay reachable');
assert.equal(w.localStorage.length,0);
assert.ok(!requests.some(x=>x.url.includes('secret')),'Camera credentials must not leak into URL');
assert.ok(!script.includes('window.open('),'No external OS/browser window from wizard');
console.log('10 UI scenarios passed: inline animated scanning, automatic RTSP, empty state, brand help, local assistant, embedded manual, RTSP and Media2 install, stale credential response, error fallback.');
