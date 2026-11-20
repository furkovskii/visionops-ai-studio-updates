// VisionOps Connect wizard. Does not store credentials in web storage, logs or URLs.
(function(){
'use strict';
const root=document.getElementById('vo-connect');
if(!root)return;
const $=id=>document.getElementById(id);
const scan=$('vo-connect-scan'), status=$('vo-connect-status');
const activity=$('vo-scan-progress'),activityText=$('vo-scan-state'),activityDetail=$('vo-scan-detail');
const fallback=$('vo-connect-fallback'),brand=$('vo-help-brand'),brandGuide=$('vo-help-brand-guide');
const topic=$('vo-help-question'),answer=$('vo-help-answer'),slot=$('vo-connect-manual-slot');
const legacy=document.querySelector('.wall-add');
if(legacy&&slot){slot.append(legacy);legacy.open=false;}
const guide={
 general:'1. Kamerayı veya kayıt cihazını açın ve üretici uygulamasından çalıştığını doğrulayın.\n2. Bilgisayarı ve kamerayı aynı yerel ağa bağlayın.\n3. Kamera uygulamasında “Yerel erişim”, “Üçüncü taraf entegrasyonu”, “ONVIF” veya “RTSP” benzeri bir ayar olup olmadığına bakın.\n4. Desteklemiyorsa kamera modelini doğrulayın; her kamera bu özelliği sunmaz.',
 tapo:'1. Tapo uygulamasından kamerayı açın.\n2. Desteklenen modellerde Kamera Ayarları → Gelişmiş Ayarlar → Kamera Hesabı bölümünde ayrı bir kamera hesabı oluşturun.\n3. VisionOps ile yeniden tarayın. Bu menü bulunmuyorsa cihaz modeliniz yerel akış desteklemeyebilir.',
 mercusys:'1. MERCUSYS uygulamasında kameranın çalıştığını doğrulayın.\n2. Desteklenen modelin kamera ayarlarında RTSP/ONVIF veya kamera hesabı seçeneklerini kontrol edin.\n3. VisionOps ile yeniden tarayın; menü adları cihaz yazılımına göre değişebilir.',
 ezviz:'1. EZVIZ uygulamasından kameranın çevrimiçi olduğunu ve görüntü verdiğini doğrulayın.\n2. Modelinizin yerel RTSP destekleyip desteklemediğini üretici açıklamasından kontrol edin.\n3. Bazı modellerde yerel bağlantı için doğrulama kodu/ayrı yetki gerekebilir. ONVIF her modelde bulunmaz.',
 imou:'1. Imou uygulamasında cihazı açın ve modelin RTSP/ONVIF desteğini kontrol edin.\n2. Kamera veya kayıt cihazında yerel görüntü erişimi kapalıysa üreticinin resmî yönergesine göre açın.\n3. Aynı ağda tekrar tarayın; bazı modeller yalnızca uygulama üzerinden çalışabilir.',
 hikvision:'1. Yetkili olarak kameranın veya kayıt cihazının yerel yönetim paneline giriş yapın.\n2. Destekleyen modellerde Ağ → Gelişmiş → Entegrasyon Protokolü bölümünde ONVIF’i etkinleştirin.\n3. Gerekirse sadece görüntüleme yetkili ONVIF kullanıcısı oluşturun.\n4. Cihaz saatini kontrol edip yeniden tarayın. Menü adları modele göre farklıdır.',
 dahua:'1. Yetkili olarak Dahua kamera/NVR yönetim panelini açın.\n2. Ağ/Entegrasyon/ONVIF ayarlarını ve görüntüleme hesabını kontrol edin.\n3. NVR kullanıyorsanız bağlantıyı tek tek kameralara değil NVR kanallarına yapabilirsiniz.\n4. Aynı ağdan yeniden tarayın. Menü isimleri yazılıma bağlıdır.',
 nvr:'1. NVR/DVR kayıt cihazının kendi ekranında görüntü olup olmadığını kontrol edin.\n2. Kayıt cihazının IP adresinin Windows bilgisayarından erişilebilir olduğundan emin olun.\n3. Cihazda ONVIF ya da RTSP kanal yayını destekleniyorsa bunu yetkili hesapla etkinleştirin.\n4. PoE portlarındaki kameralar ayrı ağda olabilir; VisionOps kayıt cihazını bulsa bile tek tek kameraları göremeyebilir.'
};
const help={
 notfound:'1. Kameranın elektrik ve ağ bağlantısını kontrol edin.\n2. Kameranın kendi uygulamasından görüntü açmayı deneyin.\n3. Aynı modem veya erişilebilir yerel ağa bağlı olduğunuzdan emin olun.\n4. Yukarıdan kameranızın markasını seçin, yerel erişim yönergesini izleyin ve “Tekrar tara”ya basın.',
 access:'Bu şifre, VisionOps hesabınızın şifresi değildir. Kameranın ya da kayıt cihazının görüntüleme hesabı gerekir. Üretici uygulamasından veya cihaz yöneticisinden yalnızca görüntüleme yetkili bir kullanıcı hesabı isteyin. Şifrenizi bu yardım alanına yazmayın.',
 protocol:'Teknik terimleri bilmeniz gerekmiyor. Markanızı seçip yukarıdaki rehberi izleyin. Destekleyen modellerde ONVIF kamerayı bulmak, RTSP görüntüyü almak için kullanılır; bazı kameralar bu bağlantıları sunmaz.',
 nvr:'Kayıt cihazında birden çok kamera varsa önce NVR/DVR cihazını bulmayı deneyin. Kameralar kayıt cihazının ayrı PoE ağında olabilir. NVR destekliyorsa her kanala ait görüntü yayınını seçebilirsiniz. Gerekirse cihaz yöneticisinden kanal ve görüntüleme hesabını alın.',
 picture:'Kamera bulunması, görüntünün açıldığı anlamına gelmez. Kamera hesabını ve yayın profilini kontrol edin; mümkünse alt/ana akışı değiştirin. Desteklenmeyen codec, yanlış yayın yolu ve kamera erişim yetkisi görüntüyü engelleyebilir.',
 network:'Windows bilgisayarı başka bir ağda, kameralar misafir Wi-Fi/VLAN ya da ayrı PoE ağındaysa otomatik arama sonuç vermeyebilir. Bağlantıyı yetkili ağ yöneticisi kurmalı veya NVR üzerinden erişim sağlamalısınız. İnternetin açık olması tek başına yeterli değil.'
};
function updateHelp(){brandGuide.textContent=guide[brand.value]||guide.general;}
function askHelp(){answer.textContent=help[topic.value]||help.notfound;answer.hidden=false;}
brand.addEventListener('change',updateHelp);
$('vo-help-ask').onclick=askHelp;updateHelp();askHelp();
function showHelp(msg,question='notfound'){fallback.hidden=false;if(msg)fallback.querySelector('.vo-fallback-title strong').textContent=msg;topic.value=question;askHelp();}
$('vo-scan-retry').onclick=()=>scan.click();
$('vo-manual-reveal').onclick=()=>manual();
const list=$('vo-connect-devices'), form=$('vo-connect-form'), note=$('vo-connect-help');
const username=$('vo-connect-user'), password=$('vo-connect-pass'), label=$('vo-connect-label');
const path=$('vo-connect-path'), profile=$('vo-connect-profile'), add=$('vo-connect-add'), check=$('vo-connect-check');
const rtspBrand=$('vo-connect-device-brand');
const rtspPresets={auto:'/stream1',tapo:'/stream1',mercusys:'/stream1',
    hikvision:'/Streaming/Channels/101',dahua:'/cam/realmonitor?channel=1&subtype=0',
    imou:'/cam/realmonitor?channel=1&subtype=0',reolink:'/h264Preview_01_main'};
rtspBrand.addEventListener('change',()=>{
  if(rtspPresets[rtspBrand.value])path.value=rtspPresets[rtspBrand.value];
  if(rtspBrand.value==='other')$('vo-connect-stream-details').open=true;
  resetCheck();
});

const csrf='__VO_CSRF_VALUE__';
let selected=null,verified=false,nonce=0,busy=false;
const steps=[...document.querySelectorAll('#vo-connect-steps li')];
function stage(current){
  for(const [i,li] of steps.entries()){
    li.dataset.state=i<current?'done':i===current?'active':'pending';
    if(i===current)li.setAttribute('aria-current','step');
    else li.removeAttribute('aria-current');
  }
}
stage(0);
function message(value){note.textContent=value;}
function resetCheck(){nonce++;stage(selected?1:0);verified=false;add.disabled=true;profile.replaceChildren(new Option('Önce bağlantıyı denetle',''));$('vo-connect-profile-wrap').hidden=true;}
function manual(){
  const old=legacy||document.querySelector('.wall-add');
  if(!old)return;
  slot.hidden=false;old.open=true;old.scrollIntoView({block:'start',behavior:'smooth'});
}
$('vo-connect-manual').onclick=manual;
function selectDevice(item){
  nonce++;
  selected=item;
  verified=false;busy=false;
  password.value='';
  username.value='';
  label.value=('Kamera '+item.ip).slice(0,40);
  rtspBrand.value='auto';
  path.value=item.scheme==='rtsp'?rtspPresets.auto:item.path;
  $('vo-connect-brand-wrap').hidden=item.scheme!=='rtsp';
  $('vo-connect-stream-details').hidden=item.scheme!=='rtsp';
  $('vo-connect-stream-details').open=false;
  $('vo-connect-path-wrap').hidden=item.scheme!=='rtsp';
  $('vo-connect-selected').textContent=item.ip+':'+item.port+' · '+(item.scheme==='rtsp'?'RTSP cihazı':'ONVIF kamera/kayıt cihazı');
  form.hidden=false;
  resetCheck();
  stage(1);
  message(item.scheme==='rtsp'
    ?'Bu cihazdan görüntü alınabilecek bir servis bulundu. Kamera kullanıcı bilgilerini girin; yayın adresi gerekirse alttaki rehber yardımcı olabilir.'
    :'Kamera veya kayıt cihazı bulundu. Kamera hesabını girin; VisionOps kullanılabilir yayınları otomatik kontrol edecek.');
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
  busy=true;scan.disabled=true;status.textContent='Kameralarınız yerel ağda aranıyor…';
  form.hidden=true;fallback.hidden=true;selected=null;nonce++;stage(0);password.value='';list.replaceChildren();
  activity.hidden=false;activityText.textContent='Kameralarınız aranıyor…';
  activityDetail.textContent='Önce yerel ağdaki kamera ve kayıt cihazları kontrol ediliyor.';
  const phase=setTimeout(()=>{if(!activity.hidden){activityText.textContent='Alternatif kamera yayınları kontrol ediliyor…';activityDetail.textContent='Bilgisayarın bağlı olduğu yerel ağda desteklenen yayınlar aranıyor. Teknik ayar yapmanız gerekmiyor.';}},3800);
  try{
    const result=await api('/api/workspace/connect/scan',{extended:true});
    const devices=Array.isArray(result.devices)?result.devices:[];
    if(!devices.length){
      showHelp('Bu ağda kamera bulunamadı','notfound');
    }
    for(const device of devices){
      const button=document.createElement('button');button.type='button';button.className='vo-connect-device';
      const name=document.createElement('strong'),detail=document.createElement('small');
      name.textContent=device.ip+':'+device.port;
      detail.textContent=device.discovered_via==='onvif'?'Kamera veya kayıt cihazı · bağlanmaya hazır aday':'Görüntü servisi bulundu · yetki ve yayın yolu doğrulanmalı';
      button.append(name,detail);
      button.onclick=()=>selectDevice(device);
      list.append(button);
    }
    status.textContent=devices.length
      ?devices.length+' kamera veya kayıt cihazı adayı bulundu. Bağlamak istediğiniz cihazı seçin. Görüntü ve erişim henüz doğrulanmadı.'
      :'Bu ağda kamera bulamadık. Aşağıda kontrol edebileceğiniz basit adımları hazırladık.';
  }catch(e){status.textContent='Tarama tamamlanamadı. Aşağıdan adımları kontrol edip yeniden deneyebilirsiniz.';showHelp('Tarama tamamlanamadı','network');}
  finally{clearTimeout(phase);activity.hidden=true;busy=false;scan.disabled=false;}
};
for(const el of [username,password,path]){
  el.addEventListener('input',resetCheck);
  el.addEventListener('change',resetCheck);
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
    if(!result.reachable){showHelp('Kamera bulundu, bağlantı kurulamadı','network');return;}
    if(protocol==='onvif'){
      if(result.code!=='profiles'||!result.profiles?.length){showHelp('Kamera bulundu, görüntü izni gerekiyor','access');return;}
      profile.replaceChildren(...result.profiles.map(p=>{const option=new Option(p.name,p.token);option.dataset.mode=p.mode||'media1';return option;}));
      $('vo-connect-profile-wrap').hidden=false;
    }else if(result.code!=='rtsp'){showHelp('Kamera bulundu, yayın yolu doğrulanamadı','picture');return;}
    verified=true;stage(2);add.disabled=false;
  }catch(e){if(ticket===nonce){message(e.message||'Cihaza erişilemedi.');showHelp('Kamera bulundu, bağlantı denetimi başarısız','access');}}
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
    if(!ok){message('Kamera henüz eklenemedi. Aşağıdaki rehberden bağlantı adımlarını kontrol edin.');showHelp('Kameranın eklenmesi tamamlanamadı','picture');return;}
    const camera=cameras.find(c=>!before.has(c.id));
    if(!camera){message('Kamera kaydı yenilendi; kamera listesini kontrol edin.');return;}
    message('Kamera eklendi · görüntü bağlantısı bekleniyor. Bağlanma süresi kameraya göre değişebilir.');
    verified=false;
    async function watch(attempt){
      if(ticket!==nonce)return;
      const current=cameras.find(c=>c.id===camera.id);
      if(current?.status==='Canlı'){stage(3);message('Canlı görüntü doğrulandı · '+current.name+' bağlı. AI analizi kamera kartından etkinleştirilebilir.');return;}
      if(attempt>=10){message('Kamera kaydı oluşturuldu, fakat canlı görüntü henüz doğrulanamadı. Aşağıdaki yardım kartlarıyla bağlantıyı kontrol edebilirsiniz.');showHelp('Kamera eklendi, görüntü henüz gelmedi','picture');return;}
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
