// VisionOps AI customer command center. Same-origin, owner-authorized camera inventory only.
// No live-stream guesses, third-party endpoints, raw video, PII or camera credentials.
(function(){
'use strict';
const root=document.getElementById('vo-command');if(!root)return;
const tabs=Array.from(root.querySelectorAll('[data-vc-tab]'));
function show(id,focus){
 for(const tab of tabs){const active=tab.dataset.vcTab===id;tab.setAttribute('aria-selected',String(active));tab.tabIndex=active?0:-1;}
 for(const view of root.querySelectorAll('.vc-view'))view.hidden=view.id!=='vc-view-'+id;
 if(focus){const tab=tabs.find(x=>x.dataset.vcTab===id);if(tab)tab.focus();}
}
for(const tab of tabs){tab.addEventListener('click',()=>show(tab.dataset.vcTab,false));tab.addEventListener('keydown',event=>{
 if(!['ArrowDown','ArrowRight','ArrowLeft','ArrowUp','Home','End'].includes(event.key))return;
 event.preventDefault();const i=tabs.indexOf(tab),next=event.key==='Home'?0:event.key==='End'?tabs.length-1:(i+(['ArrowRight','ArrowDown'].includes(event.key)?1:-1)+tabs.length)%tabs.length;
 show(tabs[next].dataset.vcTab,true);
});}
const checks=Array.from(root.querySelectorAll('[data-vc-check]')),progress=root.querySelector('#vc-check-progress');
function updateChecks(){if(progress)progress.textContent=checks.filter(box=>box.checked).length+'/'+checks.length;}
for(const box of checks)box.addEventListener('change',updateChecks);
updateChecks();
const list=root.querySelector('#vc-camera-list'),count=root.querySelector('#vc-camera-count'),status=root.querySelector('#vc-camera-status');
const filter=root.querySelector('#vc-camera-search'),reload=root.querySelector('#vc-camera-reload'),updated=root.querySelector('#vc-camera-updated');
let inventory=[],busy=false;
function empty(message){if(!list)return;list.replaceChildren();const p=document.createElement('p');p.className='vc-empty';p.textContent=message;list.append(p);}
function normalize(data){
 let value=data&&typeof data==='object'?(data.cameras??data.devices??data.items??[]):[];
 if(value&&!Array.isArray(value)&&typeof value==='object')value=Object.entries(value).map(([id,c])=>({id,...(c&&typeof c==='object'?c:{})}));
 if(!Array.isArray(value))return [];
 return value.filter(x=>x&&typeof x==='object'&&typeof x.id==='string'&&x.id.length<81).slice(0,16);
}
function render(){
 const query=filter?filter.value.trim().toLocaleLowerCase('tr-TR'):'';
 const cameras=inventory.filter(cam=>String(cam.name||cam.label||'').toLocaleLowerCase('tr-TR').includes(query));
 list.replaceChildren();
 if(!inventory.length){empty('Bu oturumda kayıtlı kamera bulunamadı. Önce IP kamera ekleyin.');return;}
 if(!cameras.length){empty('Bu ada uygun kayıtlı kamera yok. Filtreyi temizleyin.');return;}
 cameras.forEach((cam,i)=>{
  const item=document.createElement('div'),name=document.createElement('strong'),detail=document.createElement('small');
  name.textContent=String(cam.name||cam.label||('Kamera '+String(i+1).padStart(2,'0'))).slice(0,80);
  detail.textContent='Kayıtlı kamera · Canlı durum bu ekranda doğrulanmıyor';
  item.append(name,detail);list.append(item);
 });
}
async function load(){
 if(busy)return;
 busy=true;if(reload)reload.disabled=true;if(updated)updated.textContent='Kamera kayıtları kontrol ediliyor…';
 try{
  const response=await fetch('/api/workspace/ip-cameras',{method:'GET',credentials:'same-origin',cache:'no-store',headers:{'Accept':'application/json'}});
  if(!response.ok)throw Error('Bu oturum için kamera listesi alınamadı.');
  const data=await response.json();
  inventory=normalize(data);
  count.textContent=String(inventory.length);
  status.textContent='Yetkili kameraların kayıt sayısı; çevrimiçi olma bilgisi değildir.';
  if(updated)updated.textContent='Liste son kontrolü: '+new Date().toLocaleString('tr-TR')+' · canlı görüntü doğrulaması değildir.';
  render();
 }catch(_err){
  inventory=[];count.textContent='—';status.textContent='Kamera durumu bilinmiyor';
  if(updated)updated.textContent='Liste doğrulanamadı. Bağlantıyı kontrol edip yeniden deneyin.';
  empty('Liste okunamadı. Kamera bağlantısı bölümünden tekrar deneyin.');
 }finally{busy=false;if(reload)reload.disabled=false;}
}
if(filter)filter.addEventListener('input',render);
if(reload)reload.addEventListener('click',()=>{void load()});
void load();
})();