// Simulated browser UX regression. The real camera adapter remains unmocked
// in the Windows decoder suite; UI below is intentionally fixture-only.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {JSDOM} from 'jsdom';
const ROOT=new URL('./',import.meta.url);
const html=readFileSync(new URL('wizard.html.inc',ROOT),'utf8');
const js=readFileSync(new URL('wizard.js.inc',ROOT),'utf8');
const dom=new JSDOM('<!doctype html><html><body>'+html+'<details class="wall-add"><summary>Manuel</summary></details></body></html>',
 {runScripts:'outside-only',url:'http://127.0.0.1'});
const {window}=dom,d=window.document;
window.HTMLElement.prototype.scrollIntoView=()=>{};
let cameras=[],added=[],requests=[],scenario='onvif';
window.cameras=cameras;
window.loadCameras=async change=>{
  added.push(change);
  if(change?.action==='add'){
    cameras=[...cameras,{id:'added'+added.length,name:change.label,status:'Canlı'}];
    window.cameras=cameras;
  }
  return true;
};
window.fetch=async (url,opts)=>{
  requests.push([url,JSON.parse(opts.body),opts.headers['X-CSRF-Token']]);
  let body;
  if(url.endsWith('/scan')){
    body={interfaces:2,devices:scenario==='onvif'?
      [{ip:'192.168.1.20',port:80,scheme:'http',path:'/onvif/device_service',discovered_via:'onvif'}]:
      [{ip:'192.168.1.21',port:554,scheme:'rtsp',path:'',discovered_via:'rtsp'}]};
  }else if(scenario==='onvif'){
    body={reachable:true,stream_verified:false,code:'profiles',help:'Media2 profile available',
          profiles:[{name:'Old channel',token:'m1',mode:'media1'},{name:'New channel',token:'m2',mode:'media2'}]};
  }else if(scenario==='network_error'){
    body={reachable:false,stream_verified:false,code:'network',help:'Cihaz erişilemiyor',profiles:[]};
  }else body={reachable:true,stream_verified:false,code:'rtsp',help:'RTSP yanıt veriyor',profiles:[]};
  return {ok:true,json:async()=>body};
};
window.eval(js.replace('__VO_CSRF_VALUE__','fixture_csrf'));
const get=id=>d.getElementById(id);
assert.equal(get('vo-connect-steps').children[0].dataset.state,'active');
const tick=()=>new Promise(done=>setTimeout(done,0));
get('vo-connect-scan').click();await tick();
assert.equal(requests[0][0],'/api/workspace/connect/scan');
assert.equal(requests[0][1].extended,false);
assert.equal(requests[0][2],'fixture_csrf');
const device=d.querySelector('.vo-connect-device');
assert.ok(device);device.click();
assert.equal(get('vo-connect-steps').children[1].dataset.state,'active');
assert.equal(get('vo-connect-form').hidden,false);
get('vo-connect-user').value='camera-viewer';
get('vo-connect-pass').value='secret-only-for-camera';
get('vo-connect-check').click();await tick();
assert.equal(get('vo-connect-profile').options.length,2);
assert.equal(get('vo-connect-add').disabled,false);
assert.equal(get('vo-connect-steps').children[2].dataset.state,'active');
get('vo-connect-profile').value='m2';
get('vo-connect-profile').dispatchEvent(new window.Event('input',{bubbles:true}));
assert.equal(get('vo-connect-profile').value,'m2','Selecting another Media2 profile must not invalidate or erase options');
assert.equal(get('vo-connect-add').disabled,false);
get('vo-connect-form').dispatchEvent(new window.Event('submit',{bubbles:true,cancelable:true}));
await tick();
const req=added.find(x=>x?.action==='add');
assert.equal(req.onvif_mode,'media2');
assert.equal(req.profile,'m2');
assert.equal(req.remember,true);
assert.equal(get('vo-connect-pass').value,'');
assert.ok(get('vo-connect-help').textContent.includes('Canlı görüntü doğrulandı'));
assert.equal(get('vo-connect-steps').children[2].dataset.state,'done');
assert.ok(!requests.some(([u,body])=>String(u).includes('secret-only-for-camera')));
assert.ok(!window.localStorage.length);
scenario='rtsp';
get('vo-connect-extended').checked=true;
get('vo-connect-scan').click();await tick();
assert.equal(requests.at(-1)[1].extended,true);
d.querySelector('.vo-connect-device').click();
get('vo-connect-check').click();await tick();
assert.equal(get('vo-connect-path-wrap').hidden,false);
assert.equal(get('vo-connect-add').disabled,false);
scenario='network_error';
get('vo-connect-scan').click();await tick();
d.querySelector('.vo-connect-device').click();
get('vo-connect-check').click();await tick();
assert.equal(get('vo-connect-add').disabled,true);
get('vo-connect-manual').click();
assert.equal(d.querySelector('.wall-add').open,true);
console.log('7 UI scenarios passed: auto discovery, stepper, CSRF, Media2 selection, live-frame state, opt-in RTSP and failure/manual fallback.');
