<section class="vo-scene" id="vo-scene" aria-labelledby="vo-scene-title">
<style>
.vo-scene{background:radial-gradient(circle at 88% 10%,#4c6a4930,transparent 40%),#152624;color:#eaf1e5;border-radius:15px;padding:clamp(16px,2.5vw,27px);margin:16px 0;border:1px solid #cbe0b943;font-family:inherit}
.vo-scene *{box-sizing:border-box}
.vo-scene-eyebrow{display:flex;gap:8px;align-items:center;font-size:10px;font-weight:750;letter-spacing:.14em;color:#cce2bb}
.vo-scene-dot{width:7px;height:7px;border-radius:50%;background:#def49d}
.vo-scene header{display:flex;justify-content:space-between;gap:14px;align-items:flex-start;margin:16px 0 20px}
.vo-scene h3{font-size:clamp(22px,2.4vw,30px);color:#f3f8ed;letter-spacing:-.04em;margin:0 0 7px}
.vo-scene header p{color:#c2d3c5;font-size:12px;line-height:1.65;max-width:650px;margin:0}
.vo-scene header a{color:#dff39e;border:1px solid #dff39e8e;border-radius:8px;padding:11px 13px;min-height:43px;font-size:12px;text-decoration:none;white-space:nowrap}
.vo-scene-intro{display:grid;grid-template-columns:minmax(220px,.7fr) minmax(0,1.3fr);gap:17px}
.vo-scene-side{border:1px solid #aac8b440;border-radius:10px;padding:15px;min-width:0}
.vo-scene-side label{font-size:11px;display:block;color:#ccdbc7;margin:0 0 7px}
.vo-scene-side select{display:block;width:100%;border:1px solid #8bb09f7a;background:#273d36;color:#f2f6ec;border-radius:7px;padding:10px;font:inherit;min-height:44px}
.vo-scene-side p{font-size:11px;color:#c2d3c3;line-height:1.75;margin-top:15px}
.vo-scene-side .vo-scene-notice{border-top:1px solid #c5d9ca40;padding-top:15px;margin-top:19px;color:#e2edc0}
.vo-scene-steps{background:#1e3631;border:1px solid #b7d2ba4d;border-radius:10px;padding:14px}
.vo-scene-flow{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}
.vo-scene-flow>div{min-width:0;border:1px solid #b5ccbc45;background:#27413a;padding:13px 10px;border-radius:9px;display:grid;gap:8px;align-content:start}
.vo-scene-flow b{font-size:10px;color:#dcf1a3}
.vo-scene-flow strong{font-size:12px;line-height:1.45;color:#ecf7e9}
.vo-scene-flow span{font-size:11px;color:#c2d6c4;line-height:1.5;overflow-wrap:anywhere}
.vo-scene-flow>div:nth-child(2){background:#dff39e;border-color:#dff39e}
.vo-scene-flow>div:nth-child(2) b,.vo-scene-flow>div:nth-child(2) strong,.vo-scene-flow>div:nth-child(2) span{color:#23372d}
.vo-scene-foot{font-size:10px!important;color:#aacbb0;margin:13px 0 0!important}
@media(max-width:800px){.vo-scene-intro{grid-template-columns:1fr}.vo-scene-flow{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:540px){.vo-scene header{display:block}.vo-scene header a{display:inline-block;margin-top:13px}.vo-scene-flow{grid-template-columns:1fr}}
</style>
<div class="vo-scene-eyebrow"><span class="vo-scene-dot"></span> VISIONOPS SCENEFLOW / GELİŞTİRME ÖNİZLEMESİ</div>
<header><div><h3 id="vo-scene-title">İş akışınızı kameraya öğretin.</h3><p>Kamera görüntüsüne yeni anlamlar katmak için seçilebilir örnek kurallar. TruthMeter ölçüm belirsizliğini, ActionLab değişikliğin sonucunu görünür kılmayı hedefler.</p></div><a href="#vo-connect" id="vo-scene-connect">Kamera bağlantısına git</a></header>
<div class="vo-scene-intro">
 <div class="vo-scene-side"><label for="vo-scene-template">İzlemek istediğiniz olay</label><select id="vo-scene-template">
 <option value="0">Alan hareketinde değişim</option><option value="1">Süre eşiği ve alan kullanımı</option><option value="2">Önce–sonra düzen karşılaştırması</option>
 </select><p id="vo-scene-description">Gözlenebilen olay sayısı, kamera çalışma süresiyle birlikte değerlendirilir.</p>
 <p class="vo-scene-notice">Bu ekran şimdilik yeni sistemin nasıl çalışacağını gösterir. Kamerayı bağlamadan kural etkinleşmez; mevcut analizler değişmez.</p></div>
 <div class="vo-scene-steps" role="group" aria-label="SceneFlow işleme süreci">
   <div class="vo-scene-flow">
    <div><b>01 / GÖZLEM</b><strong>Mevcut kamera</strong><span id="vo-scene-input">Anonim alan hareketi</span></div>
    <div><b>02 / KURAL</b><strong id="vo-scene-rule">Değişim işareti</strong><span>Kullanıcının doğrulayacağı alan</span></div>
    <div><b>03 / TRUTHMETER</b><strong>Gözlem denetimi</strong><span>Eksik görüntü kesin sonuç sayılmaz.</span></div>
    <div><b>04 / ACTIONLAB</b><strong>Ölç–karşılaştır</strong><span id="vo-scene-action">İyileştirme denemesi oluştur</span></div>
   </div>
   <p class="vo-scene-foot">Gerçek kamera ile ölçüm doğruluğu doğrulanana kadar burada başarı yüzdesi veya müşteri sonucu gösterilmez.</p>
 </div>
</div>
</section>
