// VisionOps Connect wizard. Does not store credentials in web storage, logs or URLs.
(function(){
'use strict';
const root=document.getElementById('vo-connect');
if(!root)return;
const $=id=>document.getElementById(id);
const scan=$('vo-connect-scan'), extended=$('vo-connect-extended'), status=$('vo-connect-status');
const list=$('vo-connect-devices'), form=$('vo-connect-form'), note=$('vo-connect-help');
const username=$('vo-connect-user'), password=$('vo-connect-pass'), label=$('vo-connect-label');
const path=$('vo-connect-path'), profile=$('vo-connect-profile'), add=$('vo-connect-add'), check=$('vo-connect-check');
const csrf='__VO_CSRF_VALUE__';
let selected=null,verified=false,nonce=0,busy=false;
function message(value){note.textContent=value;}
function resetCheck(){verified=false;add.disabled=true;profile.replaceChildren(new Option('Önce bağlantıyı denetle',''));$('vo-connect-profile-wrap').hidden=true;}
function manual(){const old=document.querySelector('.wall-add');if(old){old.open=true;old.scrollIntoView({block:'start',behavior:'smooth'});}}
$('vo-connect-manual').onclick=manual;
function selectDevice(item){
  nonce++;
  selected=item;
  verified=false;busy=false;
  password.value='';
  username.value='';
  label.value=('Kamera '+item.ip).slice(0,40);
  path.value=item.scheme==='rtsp'?'/stream1':item.path;
  $('vo-connect-path-wrap').hidden=item.scheme!=='rtsp';
  $('vo-connect-selected').textContent=item.ip+':'+item.port+' · '+(item.scheme==='rtsp'?'RTSP cihazı':'ONVIF kamera/kayıt cihazı');
  form.hidden=false;
  resetCheck();
  message(item.scheme==='rtsp'
    ?'RTSP servisi algılandı. Yayın yolunu cihazın modeline göre kontrol edin. Bağlantı denetimi gerçek görüntüyü henüz doğrulamaz.'
    :'ONVIF hizmeti bulundu. Kamera hesabını girin ve video profillerini alın. NVR kullanıyorsanız kanallardan birini seçebilirsiniz.');
  form.scrollIntoView({block:'nearest',behavior:'smooth'});
}
async function api(url,body){
  const response=await fetch(url,{method:'POST',credentials:'same-origin',cache:'no-store',
    headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify(body)});
  const data=await response.json().catch(()=>({}));
  if(!response.ok)throw Error(data.error||'İşlem başarısız. Bağlantı ve yetki durumunu kontrol edin.');
  return data;
}
scan.onclick=async()=>{
  if(busy)return;
  busy=true;scan.disabled=true;status.textContent='Yerel ağda ONVIF kameralar aranıyor'+(extended.checked?' · izinli RTSP taraması da yapılacak':'')+'…';
  form.hidden=true;selected=null;nonce++;password.value='';list.replaceChildren();
  try{
    const result=await api('/api/workspace/connect/scan',{extended:extended.checked});
    const devices=Array.isArray(result.devices)?result.devices:[];
    if(!devices.length){
      const p=document.createElement('p');p.textContent='Kamera bulunamadı. ONVIF ayarını kontrol edin veya gelişmiş taramayı etkinleştirerek yeniden deneyin. Yine bulunmazsa manuel bağlantıyı açın.';
      list.append(p);
      const btn=document.createElement('button');btn.type='button';btn.className='btn';btn.textContent='Manuel bağlantıyı aç';btn.onclick=manual;list.append(btn);
    }
    for(const device of devices){
      const button=document.createElement('button');button.type='button';button.className='vo-connect-device';
      const name=document.createElement('strong'),detail=document.createElement('small');
      name.textContent=device.ip+':'+device.port;
      detail.textContent=device.discovered_via==='onvif'?'ONVIF · '+device.scheme.toUpperCase():'RTSP servisi · yayın yolu doğrulanmalı';
      button.append(name,detail);
      button.onclick=()=>selectDevice(device);
      list.append(button);
    }
    status.textContent=devices.length+' erişilebilir cihaz/servis adayı bulundu · '+(result.interfaces||0)+' yerel IPv4 arayüzü görüldü. Görüntü ve giriş bilgileri henüz doğrulanmadı.';
  }catch(e){status.textContent=e.message||'Kamera keşfi başarısız.';}
  finally{busy=false;scan.disabled=false;}
};
for(const el of [username,password,path,profile]){
  el.addEventListener('input',resetCheck);
  el.addEventListener('change',()=>{if(el!==profile)resetCheck();});
}
check.onclick=async()=>{
  if(!selected||busy||!form.reportValidity())return;
  busy=true;check.disabled=true;resetCheck();const ticket=nonce;
  message('Cihaza güvenli bağlantı kontrolü yapılıyor; yetkiler ve video profilleri inceleniyor…');
  try{
    const protocol=selected.scheme==='rtsp'?'rtsp':'onvif';
    const request={protocol,ip:selected.ip,port:selected.port,path:protocol==='onvif'?selected.path:path.value,
       onvif_scheme:selected.scheme,username:username.value,password:password.value};
    const result=await api('/api/workspace/connect/check',request);
    if(ticket!==nonce||selected===null)return;
    message(result.help||'Bağlantı durumu alınamadı.');
    if(!result.reachable)return;
    if(protocol==='onvif'){
      if(result.code!=='profiles'||!result.profiles?.length)return;
      profile.replaceChildren(...result.profiles.map(p=>{const option=new Option(p.name,p.token);option.dataset.mode=p.mode||'media1';return option;}));
      $('vo-connect-profile-wrap').hidden=false;
    }else if(result.code!=='rtsp'){return;}
    verified=true;add.disabled=false;
  }catch(e){if(ticket===nonce)message(e.message||'Cihaza erişilemedi.');}
  finally{busy=false;check.disabled=false;}
};
form.onsubmit=async e=>{
  e.preventDefault();
  if(!verified||!selected||busy||!form.reportValidity())return;
  const ticket=nonce,device=selected;
  busy=true;add.disabled=true;
  message('Kamera ekleniyor. Gerçek görüntünün açılması ayrıca doğrulanacak…');
  const before=new Set(cameras.map(c=>c.id));
  const data={action:'add',label:label.value.trim(),protocol:device.scheme==='rtsp'?'rtsp':'onvif',
      ip:device.ip,port:device.port,path:device.scheme==='rtsp'?path.value:device.path,
      username:username.value,password:password.value,onvif_scheme:device.scheme,
      profile:device.scheme==='rtsp'?'':profile.value,
      onvif_mode:device.scheme==='rtsp'?'media1':(profile.selectedOptions[0]?.dataset.mode||'media1'),remember:$('vo-connect-remember').checked};
  try{
    const ok=await loadCameras(data);
    if(ticket!==nonce)return;
    if(!ok){message('Kamera eklenemedi. Bağlantı bilgilerini kontrol edin; manuel bağlantı da kullanılabilir.');return;}
    const camera=cameras.find(c=>!before.has(c.id));
    if(!camera){message('Kamera kaydı yenilendi; kamera listesini kontrol edin.');return;}
    message('Kamera eklendi · görüntü bağlantısı bekleniyor. Bağlanma süresi kameraya göre değişebilir.');
    verified=false;
    async function watch(attempt){
      if(ticket!==nonce)return;
      const current=cameras.find(c=>c.id===camera.id);
      if(current?.status==='Canlı'){message('Canlı görüntü doğrulandı · '+current.name+' bağlı. AI analizi kamera kartından etkinleştirilebilir.');return;}
      if(attempt>=10){message('Kamera kaydı oluşturuldu, fakat canlı görüntü henüz doğrulanamadı. Ağ, profil ve yayın yolunu kontrol edin; kamera listesinde yeniden deneyin.');return;}
      await new Promise(done=>setTimeout(done,2500));
      if(ticket!==nonce)return;
      await loadCameras();
      watch(attempt+1);
    }
    watch(0);
  }finally{password.value='';busy=false;add.disabled=!verified;}
};
window.addEventListener('pagehide',()=>{password.value='';username.value='';nonce++;});
})();
