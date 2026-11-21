// VisionOps AI customer command center: authorized same-origin camera listing only.
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
const list=root.querySelector('#vc-camera-list'),count=root.querySelector('#vc-camera-count'),status=root.querySelector('#vc-camera-status');
function empty(message){if(!list)return;list.replaceChildren();const p=document.createElement('p');p.className='vc-empty';p.textContent=message;list.append(p);}
function normalize(data){let value=data&&typeof data==='object'?(data.cameras??data.devices??data.items??[]):[];
 if(value&&!Array.isArray(value)&&typeof value==='object')value=Object.entries(value).map(([id,c])=>({id,...(c&&typeof c==='object'?c:{})}));
 if(!Array.isArray(value))return [];
 return value.filter(x=>x&&typeof x==='object'&&typeof x.id==='string'&&x.id.length<81).slice(0,16);
}
async function load(){
 try{
  const response=await fetch('/api/workspace/ip-cameras',{method:'GET',credentials:'same-origin',cache:'no-store',headers:{'Accept':'application/json'}});
  if(!response.ok)throw Error('Bu oturum için kamera listesi alınamadı.');
  const data=await response.json();const cameras=normalize(data);
  count.textContent=String(cameras.length);status.textContent='Yetkili kameraların kayıt sayısı; çevrimiçi olma bilgisi değildir.';
  list.replaceChildren();if(!cameras.length){empty('Bu oturumda kayıtlı kamera bulunamadı. Önce IP kamera ekleyin.');return;}
  cameras.forEach((cam,i)=>{const item=document.createElement('div');const name=document.createElement('strong');const detail=document.createElement('small');name.textContent=String(cam.name||cam.label||('Kamera '+String(i+1).padStart(2,'0'))).slice(0,80);detail.textContent='Kayıtlı kamera · Canlı durum bu ekranda doğrulanmıyor';item.append(name,detail);list.append(item);});
 }catch(_err){count.textContent='—';status.textContent='Kamera durumu bilinmiyor';empty('Liste okunamadı. Kamera bağlantısı bölümünden tekrar deneyin.');}
}
void load();
})();