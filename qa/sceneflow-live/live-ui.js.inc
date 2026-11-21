// SceneFlow: same-origin owner-scoped API. No camera URLs, face data, passwords or third-party services.
(function(){
'use strict';
const prefix='vo-sf-';
const node=id=>document.getElementById(prefix+id);
if(!node('camera'))return;
const csrf='__SCENEFLOW_CSRF__';
const picker=node('camera'),list=node('rule-list'),status=node('status');
const add=node('add'),save=node('save'),refresh=node('refresh'),reload=node('load');
let zoneOptions=[],definitions=[],latest=[],busy=false;
function message(s){status.textContent=s;}
async function request(url,body){
 const opts={credentials:'same-origin',cache:'no-store',
   headers:{'Accept':'application/json','X-CSRF-Token':csrf}};
 if(body!==undefined){opts.method='POST';opts.headers['Content-Type']='application/json';opts.body=JSON.stringify(body);}
 const response=await fetch(url,opts);
 const data=await response.json().catch(()=>({error:'Yanıt okunamadı.'}));
 if(!response.ok||data.error)throw Error(data.error||'İstek başarısız oldu.');
 return data;
}
function buttonState(){
 const selected=!!picker.value;
 add.disabled=busy||!selected||!zoneOptions.length||definitions.length>=8;
 save.disabled=busy||!selected;
 refresh.disabled=busy||!selected;
 reload.disabled=busy;
 picker.disabled=busy;
}
function inputLabel(title,type,value,options=[]){
 const label=document.createElement('label');label.textContent=title;
 const el=document.createElement(type==='select'?'select':'input');
 if(type==='select'){for(const [key,name] of options){const opt=document.createElement('option');opt.value=key;opt.textContent=name;el.append(opt)}el.value=String(value);}
 else{el.type='number';el.min=type==='threshold'?'0':'30';el.max=type==='threshold'?'100000':'1800';el.step=type==='threshold'?'1':'30';el.value=String(value);}
 label.append(el);return {label,el};
}
function idFor(){return 'flow_'+Date.now().toString(36)+'_'+Math.floor(Math.random()*4096).toString(36)}
function render(){
 list.replaceChildren();
 for(const [i,r] of definitions.entries()){
  const wrap=document.createElement('div');wrap.className='vo-sf-rule';
  const head=document.createElement('div');head.className='vo-sf-rule-head';
  const heading=document.createElement('strong');heading.textContent='Gözlem kuralı '+String(i+1).padStart(2,'0');
  const remove=document.createElement('button');remove.type='button';remove.textContent='Kuralı kaldır';remove.onclick=()=>{definitions.splice(i,1);latest=[];render();};
  head.append(heading,remove);wrap.append(head);
  const fields=document.createElement('div');fields.className='vo-sf-grid';
  const specs=[
    ['Alan','zone_id','select',zoneOptions.map(z=>[z.id,z.name])],
    ['Olay','event','select',[['zone_enter','Alana giriş'],['zone_exit','Alandan çıkış']]],
    ['Koşul','condition','select',[['at_least','En az'],['at_most','En fazla']]],
    ['Olay sayısı eşiği','threshold','threshold',[]],
    ['İzleme süresi (saniye)','window_seconds','duration',[]]
  ];
  for(const [label,key,type,opts] of specs){
   const f=inputLabel(label,type,r[key],opts);
   f.el.onchange=()=>{r[key]=type==='select'?f.el.value:Number(f.el.value);latest=[];render();};
   fields.append(f.label);
  }
  wrap.append(fields);
  const seen=latest.find(v=>v.rule?.rule_id===r.rule_id);
  if(seen){
   const e=seen.evidence||{},card=document.createElement('div');card.className='vo-sf-evidence';
   const title=document.createElement('strong');
   title.textContent=e.verdict==='triggered'?'Eşik gözlendi':e.verdict==='not_triggered'?'Gözlemde eşik oluşmadı':'Gözlem yetersiz / bilinmiyor';
   card.append(title);
   const meta=document.createElement('p');
   const ratio=typeof e.coverage_ratio==='number'?Math.round(e.coverage_ratio*100)+'% gözlem süresi':'Ölçüm yok';
   const count=Number.isInteger(e.observed_events)?String(e.observed_events)+' kaydedilmiş olay':'Sayım doğrulanamadı';
   meta.textContent=ratio+' · '+count+'. '+String(e.reason||'');
   card.append(meta);
   const exp=e.comparison;
   if(exp){
    const p=document.createElement('p');
    p.textContent='ActionLab: '+(exp.enough_evidence?(exp.relative_change===null?'Temel dönemde sıfır olay olduğundan yüzde karşılaştırması yapılamaz.':'İki eşit gözlem dönemi arasında '+exp.relative_change+'% gözlenen olay oranı farkı.'): 'Karşılaştırma için iki dönemde de yeterli gözlem gerekli.')+' '+String(exp.interpretation||'');
    card.append(p);
   }
   const note=document.createElement('p');note.textContent='Model sayım doğruluğu: Henüz bağımsız doğrulama yapılmadı.';card.append(note);
   wrap.append(card);
  }
  list.append(wrap);
 }
 if(!definitions.length){const empty=document.createElement('p');empty.textContent=zoneOptions.length?'Henüz kural yok. “+ Gözlem kuralı” ile başlayın.':'Bu kamerada analiz alanı tanımlanmamış. Önce mevcut Analiz Alanları bölümünden bir alan belirleyin.';list.append(empty);}
 buttonState();
}
async function loadRules(){
 definitions=[];latest=[];zoneOptions=[];render();
 if(!picker.value){message('Önce kayıtlı bir kamera seçin.');return;}
 busy=true;buttonState();message('Kamera analiz alanları ve kurallar doğrulanıyor…');
 try{
  const data=await request('/api/workspace/sceneflow/'+encodeURIComponent(picker.value));
  zoneOptions=Array.isArray(data.zones)?data.zones.filter(z=>z&&typeof z.id==='string'&&typeof z.name==='string'):[];
  latest=Array.isArray(data.rules)?data.rules:[];
  definitions=latest.map(x=>({...x.rule})).filter(x=>x&&x.rule_id&&zoneOptions.some(z=>z.id===x.zone_id)).slice(0,8);
  message(data.online?'Kamera analizi açık. TruthMeter yalnızca işlenmiş görüntü aralıklarını ölçer.':'Kamera analizi şu anda çevrimdışı. Kuralları düzenleyebilirsiniz; sonuçlar bilinmiyor olarak gösterilir.');
 }catch(err){message('Kurallar okunamadı: '+String(err.message||'Erişim engellendi.'));}
 finally{busy=false;render();}
}
function parseCameras(data){
 let value=data?.cameras??data?.devices??data?.items??[];
 if(value&&!Array.isArray(value)&&typeof value==='object')value=Object.entries(value).map(([id,c])=>({id,...c}));
 if(!Array.isArray(value))return [];
 return value.filter(x=>x&&typeof x==='object'&&typeof x.id==='string'&&x.id.length<81)
             .slice(0,16).map(x=>({id:x.id,name:String(x.name||x.label||'Kamera').slice(0,55)}));
}
async function loadCameras(){
 busy=true;buttonState();message('Yetkili kameralarınız kontrol ediliyor…');
 try{
  const result=await request('/api/workspace/ip-cameras');
  const cameras=parseCameras(result);const previous=picker.value;
  picker.replaceChildren();
  const empty=document.createElement('option');empty.value='';empty.textContent='Kamera seçin';picker.append(empty);
  for(const cam of cameras){const item=document.createElement('option');item.value=cam.id;item.textContent=cam.name;picker.append(item);}
  if(cameras.length)picker.value=cameras.some(c=>c.id===previous)?previous:cameras[0].id;
  message(cameras.length?cameras.length+' kamera kaydı bulundu.':'Kayıtlı kamera bulunamadı. Önce Kamera Ekle bölümüne gidin.');
 }catch(err){message('Kamera listesi alınamadı: '+String(err.message||'Erişim engellendi.'));}
 finally{busy=false;buttonState();}
 if(picker.value)await loadRules();
}
add.addEventListener('click',()=>{
 if(busy||!picker.value||!zoneOptions.length||definitions.length>=8)return;
 definitions.push({rule_id:idFor(),zone_id:zoneOptions[0].id,event:'zone_enter',
                  condition:'at_least',threshold:1,window_seconds:120,minimum_coverage:.9});
 latest=[];render();
});
save.addEventListener('click',async()=>{
 if(busy||!picker.value)return;
 if(definitions.some(r=>!Number.isInteger(r.threshold)||r.threshold<0||r.threshold>100000||
                       !Number.isInteger(r.window_seconds)||r.window_seconds<30||r.window_seconds>1800)){
  message('Eşik sayısı 0–100000, süre 30–1800 saniye arasında olmalı.');return;
 }
 busy=true;buttonState();message('Kurallar güvenli yerel depoya kaydediliyor…');
 try{await request('/api/workspace/sceneflow/'+encodeURIComponent(picker.value),{rules:definitions});message('Kurallar kaydedildi. Yeni güvenilir kamera gözlemleri toplanınca TruthMeter sonuçları oluşacak.');await loadRules();}
 catch(err){message('Kaydedilemedi: '+String(err.message||'Geçersiz işlem.'));}
 finally{busy=false;buttonState();}
});
picker.addEventListener('change',()=>{void loadRules()});
reload.addEventListener('click',()=>{void loadCameras()});
refresh.addEventListener('click',()=>{void loadRules()});
document.addEventListener('visibilitychange',()=>{if(document.visibilityState==='visible'&&picker.value&&!busy)void loadRules()});
void loadCameras();
})();
