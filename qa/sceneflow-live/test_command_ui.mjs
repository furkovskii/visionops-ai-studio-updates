// Safe, isolated UI test against mocked authorized camera inventory. No live credentials.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {JSDOM} from 'jsdom';
const root=readFileSync(new URL('./command-center.html.inc',import.meta.url),'utf8').replace('__VC_HERO_DATA__','UklGRg==');
const js=readFileSync(new URL('./command-center.js.inc',import.meta.url),'utf8');
const dom=new JSDOM('<!doctype html><html><body>'+root+'</body></html>',{runScripts:'outside-only',url:'http://127.0.0.1/'});
const w=dom.window,document=w.document;
let requests=[];
w.fetch=async (url,options)=>{requests.push({url,options});return {ok:true,json:async()=>({cameras:[
 {id:'cam01',name:'Giriş'},
 {id:'cam02',name:'<img src=x onerror=alert(1)>'}
]})}};
w.eval(js);
await new Promise(r=>setTimeout(r,70));
assert.equal(requests.length,1);
assert.equal(requests[0].url,'/api/workspace/ip-cameras');
assert.equal(requests[0].options.credentials,'same-origin');
assert.equal(requests[0].options.cache,'no-store');
assert.equal(document.getElementById('vc-camera-count').textContent,'2');
assert.equal(document.querySelector('#vc-camera-list').children.length,2);
assert.equal(document.querySelector('#vc-camera-list img'),null,'camera names must never parse HTML');
assert.match(document.querySelector('#vc-camera-list').textContent,/onerror=alert/);
const tab=document.getElementById('vc-tab-cameras');tab.click();
assert.equal(tab.getAttribute('aria-selected'),'true');
assert.equal(document.getElementById('vc-view-cameras').hidden,false);
assert.equal(document.getElementById('vc-view-overview').hidden,true);
const k=new w.KeyboardEvent('keydown',{key:'ArrowRight',bubbles:true});tab.dispatchEvent(k);
assert.equal(document.getElementById('vc-tab-areas').getAttribute('aria-selected'),'true');
assert.equal(document.querySelector('[href="#vo-sf-live"]')!==null,true,'real link to existing authorized SceneFlow');
assert.equal(document.querySelector('[href="#vo-connect"]')!==null,true,'real link to existing camera UI');
assert(!/\b98[.,]7\b/.test(root),'no invented accuracy KPI');
console.log('VisionOps customer dashboard tabs, keyboard, authorized inventory and XSS safety passed');