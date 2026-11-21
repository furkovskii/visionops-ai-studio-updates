import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {JSDOM} from 'jsdom';
const html=readFileSync(new URL('./live-ui.html.inc',import.meta.url),'utf8');
const src=readFileSync(new URL('./live-ui.js.inc',import.meta.url),'utf8').replace("'__SCENEFLOW_CSRF__'","'fixture'");
const dom=new JSDOM('<!doctype html><html><body>'+html+'</body></html>',
  {runScripts:'outside-only',url:'http://127.0.0.1/'});
const w=dom.window,d=w.document,by=id=>d.getElementById('vo-sf-'+id);
let posts=[],reads=0;
w.fetch=async(url,opts)=>{
  if(url==='/api/workspace/ip-cameras')return {ok:true,json:async()=>({cameras:[{id:'cam01',name:'Kasa kamerası'}]})};
  if(url==='/api/workspace/sceneflow/cam01'&&opts.method==='POST'){
    assert.equal(opts.headers['X-CSRF-Token'],'fixture');
    assert.equal(opts.credentials,'same-origin');
    posts.push(JSON.parse(opts.body));
    return {ok:true,json:async()=>({ok:true,saved:1})};
  }
  if(url==='/api/workspace/sceneflow/cam01'){
    reads++;
    return {ok:true,json:async()=>({ok:true,online:reads>1,zones:[{id:'abcdef01234567890123',name:'Giriş'}],rules:[]})};
  }
  throw Error('Unexpected URL '+url);
};
w.eval(src);
async function flush(){await new Promise(r=>setTimeout(r,50));}
await flush();
assert.equal(by('camera').value,'cam01');
assert.equal(by('camera').selectedOptions[0].textContent,'Kasa kamerası');
assert.equal(by('add').disabled,false);
assert.match(by('status').textContent,/çevrimdışı/);
by('add').click();
assert.equal(d.querySelectorAll('.vo-sf-rule').length,1);
assert.equal(by('save').disabled,false);
by('save').click();
await flush();
assert.equal(posts.length,1);
assert.equal(posts[0].rules.length,1);
assert.equal(posts[0].rules[0].zone_id,'abcdef01234567890123');
assert.equal(posts[0].rules[0].event,'zone_enter');
assert.equal(posts[0].rules[0].minimum_coverage,.9);
assert.equal(reads>=2,true);
assert.ok(!d.body.textContent.includes('password'));
console.log('SceneFlow camera selection, explicit rule save, CSRF and no-credentials UI verified');
