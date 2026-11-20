<section id="vo-connect" class="vo-connect" aria-label="VisionOps Connect kamera kurulum sihirbazı">
<style>
.vo-connect{margin:16px 0;padding:18px;border:1px solid var(--stroke,#595466);border-radius:16px;background:var(--card,#27262d);color:inherit}
.vo-connect-header{display:flex;flex-wrap:wrap;gap:12px;justify-content:space-between;align-items:flex-start}
.vo-connect-header h3{margin:0 0 6px}.vo-connect p{line-height:1.5}
.vo-connect button{cursor:pointer}.vo-connect button:disabled{cursor:wait;opacity:.6}
.vo-connect-device{display:flex;justify-content:space-between;align-items:center;gap:12px;text-align:left;width:100%;border:1px solid var(--stroke,#595466);border-radius:10px;background:var(--paper,#34343d);color:inherit;padding:12px;margin:6px 0}
.vo-connect-device:focus-visible{outline:3px solid #b7a3dd;outline-offset:2px}
.vo-connect-form{margin-top:12px;padding:14px;border:1px solid var(--stroke,#595466);border-radius:12px}
.vo-connect-fields{display:grid;grid-template-columns:repeat(auto-fit,minmax(165px,1fr));gap:10px}
.vo-connect-fields label{display:flex;flex-direction:column;gap:6px;font-size:13px}
.vo-connect-fields input,.vo-connect-fields select{width:100%;min-width:0}
.vo-connect-actions{display:flex;flex-wrap:wrap;gap:10px;margin:12px 0}
.vo-connect-hint{font-size:13px;opacity:.86}
.vo-connect-note{border-left:3px solid #b7a3dd;padding:8px 12px;margin-top:12px}

/* Resilient, mobile-width-safe charcoal/violet onboarding */
.vo-connect{background:linear-gradient(135deg,rgba(155,132,215,.14),transparent 49%),var(--card,#24232b);box-shadow:0 14px 42px rgba(9,8,18,.12);padding:clamp(14px,2vw,22px)}
.vo-connect-header h3{font-size:clamp(19px,2vw,25px);letter-spacing:-.025em}
.vo-connect-header p{max-width:650px;margin:4px 0 12px}
.vo-connect button{min-height:40px}
.vo-connect-stepper{list-style:none;margin:8px 0 18px;padding:12px;display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px;background:rgba(135,123,166,.08);border:1px solid var(--stroke,#595466);border-radius:12px}
.vo-connect-stepper li{display:flex;align-items:center;gap:9px;font-size:12px;min-width:0;line-height:1.4;color:var(--muted,#bcb5cb);opacity:.75}
.vo-connect-stepper b{display:grid;place-items:center;width:25px;height:25px;min-width:25px;border-radius:50%;border:1px solid var(--stroke,#777);font-size:12px;font-weight:700}
.vo-connect-stepper li[data-state=active]{color:var(--white,#f4effc);opacity:1;font-weight:650}
.vo-connect-stepper li[data-state=active] b{border-color:#b6a4e7;background:#514567;color:#fff}
.vo-connect-stepper li[data-state=done]{color:var(--white,#f4effc);opacity:1}
.vo-connect-stepper li[data-state=done] b{background:#665787;color:#fff;border-color:#8173a0}
.vo-connect-scan-extra{display:flex;gap:10px;align-items:flex-start;border:1px dashed var(--stroke,#6a6078);border-radius:10px;padding:11px;margin:7px 0 12px}
.vo-connect-scan-extra input{margin-top:3px;flex:none}
.vo-connect-devices{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(240px,100%),1fr));gap:9px}
.vo-connect-device{margin:0;min-height:73px;transition:border-color .15s,background .15s;overflow-wrap:anywhere}
.vo-connect-device:hover{border-color:#a18bc8;background:rgba(160,140,200,.13)}
.vo-connect-device small{color:var(--muted,#c2b9cc);font-size:11px}
.vo-connect-form{background:rgba(125,112,157,.06)}
.vo-connect-fields input,.vo-connect-fields select{min-height:40px}
.vo-connect-note{background:rgba(127,112,168,.12);border-radius:0 9px 9px 0}
.vo-connect #vo-connect-status{font-size:13px;line-height:1.5}
@media(max-width:640px){.vo-connect-header{display:block}.vo-connect-header button{width:100%;margin:5px 0 8px}.vo-connect-stepper{gap:5px;padding:9px}.vo-connect-stepper li{font-size:11px}.vo-connect-stepper b{width:20px;min-width:20px;height:20px}.vo-connect-actions>*{width:100%}}
@media(prefers-reduced-motion:reduce){.vo-connect-device{transition:none}}

.vo-scan-disclosure{margin:6px 0 12px;max-width:770px}
.vo-scanning{display:flex;align-items:center;gap:24px;margin:16px 0;padding:26px 22px;border:1px solid rgba(168,146,228,.35);border-radius:14px;background:linear-gradient(125deg,rgba(96,83,148,.2),rgba(63,88,108,.08))}
.vo-scanning[hidden],.vo-fallback[hidden],#vo-connect-manual-slot[hidden]{display:none!important}
.vo-radar{width:78px;height:78px;flex:none;display:grid;place-items:center;border:2px solid rgba(154,136,212,.7);border-radius:50%;position:relative;box-shadow:0 0 26px rgba(145,115,214,.16)}
.vo-radar:before,.vo-radar:after{content:'';position:absolute;border:2px solid rgba(170,146,231,.68);border-radius:50%;inset:10px;animation:vo-pulse 2s ease-out infinite}
.vo-radar:after{animation-delay:1s}
.vo-radar-core{width:17px;height:17px;border-radius:50%;background:#b7a8ed;box-shadow:0 0 18px #b7a8ed;z-index:1}
@keyframes vo-pulse{0%{transform:scale(.35);opacity:.9}100%{transform:scale(1.6);opacity:0}}
.vo-scanning-text{flex:1;min-width:0}
.vo-scanning-text strong{font-size:18px}
.vo-scanning-text p{font-size:13px;opacity:.84;margin:6px 0 12px}
.vo-loading-line{height:5px;border-radius:6px;background:rgba(160,157,184,.22);overflow:hidden}
.vo-loading-line span{height:100%;width:35%;display:block;background:linear-gradient(90deg,transparent,#a58adf,transparent);animation:vo-sweep 1.75s ease-in-out infinite}
@keyframes vo-sweep{from{transform:translateX(-100%)}to{transform:translateX(320%)}}
.vo-fallback{margin:18px 0;padding:16px;border:1px solid rgba(165,146,219,.36);border-radius:14px;background:rgba(155,138,208,.06)}
.vo-fallback-title strong{font-size:17px}.vo-fallback-title p{font-size:13px;color:var(--muted,#c3bccc)}
.vo-help-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:10px;margin:14px 0}
.vo-help-card{padding:14px;border-radius:12px;border:1px solid var(--stroke,#575261);background:rgba(126,114,168,.08)}
.vo-help-card h4{margin:8px 0}.vo-help-card p{font-size:12px;margin:0;color:var(--muted,#c8bdce)}
.vo-help-number{display:grid;place-items:center;width:27px;height:27px;background:rgba(155,133,210,.28);border:1px solid rgba(165,148,210,.45);border-radius:50%;font-weight:700}
.vo-guide-group,.vo-helper{padding:14px;border:1px solid var(--stroke,#595466);border-radius:11px;margin:12px 0}
.vo-guide-group label,.vo-helper label{display:block;margin:5px 0;font-weight:600;font-size:13px}
.vo-guide-group select,.vo-helper select{width:100%;min-height:40px;margin:5px 0}
.vo-helper h4{margin:0}.vo-helper p{font-size:12px;color:var(--muted,#c8bdce)}
.vo-answered{background:rgba(90,84,126,.19);border-left:3px solid #a78cde;border-radius:4px;margin-top:10px;padding:12px;font-size:13px;white-space:pre-line;line-height:1.6}
.vo-terms{margin-top:12px;font-size:12px}.vo-terms summary{cursor:pointer;text-decoration:underline}
.vo-help-actions{display:flex;flex-wrap:wrap;gap:10px;margin-top:14px}
#vo-connect-manual-slot{margin-top:16px;border-top:1px solid var(--stroke,#595466);padding-top:10px}
#vo-connect-manual-slot .wall-add{margin:0}
@media(max-width:720px){.vo-help-grid{grid-template-columns:1fr}.vo-scanning{gap:12px;padding:16px 12px}.vo-radar{width:57px;height:57px}.vo-scanning-text strong{font-size:15px}}
@media(prefers-reduced-motion:reduce){.vo-radar:before,.vo-radar:after,.vo-loading-line span{animation:none!important}}
</style>
<div class="vo-connect-header"><div><h3>VisionOps Connect · Akıllı kamera kurulumu</h3><p class="vo-connect-hint">Önce kameraları bul, sonra bağlantı yetkisini doğrula. RTSP adresi tek başına görüntünün açıldığını kanıtlamaz.</p></div><button type="button" class="btn primary" id="vo-connect-scan">1 · Kameraları Bul</button></div>
<ol class="vo-connect-stepper" id="vo-connect-steps" aria-label="Kamera ekleme adımları"><li data-state="active"><b>1</b> Kamerayı bul</li><li data-state="pending"><b>2</b> Erişimi doğrula</li><li data-state="pending"><b>3</b> Görüntüyü başlat</li></ol>
<p class="vo-connect-hint vo-scan-disclosure">Tek tıklamayla bu bilgisayarın bağlı olduğu yerel ağda erişilebilir kameraları ve kayıt cihazlarını ararız. Kamera parolası denenmez, internetteki cihazlar taranmaz.</p>
<div id="vo-scan-progress" class="vo-scanning" hidden role="status" aria-live="polite" aria-busy="true"><span class="vo-radar" aria-hidden="true"><span class="vo-radar-core"></span></span><div class="vo-scanning-text"><strong id="vo-scan-state">Kameralar aranıyor…</strong><p id="vo-scan-detail">Güvenli yerel ağ taraması devam ediyor. Bu ekranı açık tutabilirsiniz.</p><div class="vo-loading-line" aria-hidden="true"><span></span></div></div></div>
<p id="vo-connect-status" role="status" aria-live="polite">Tarama başlatılmadı. Kameraların ve bilgisayarın yerel ağda erişilebilir olması gerekir.</p>
<div id="vo-connect-devices" class="vo-connect-devices" aria-label="Bulunan kameralar"></div>
<section id="vo-connect-fallback" class="vo-fallback" hidden aria-label="Kamera bulunamadığında kurulum rehberi">
  <div class="vo-fallback-title"><strong>Kamera bulunamadı — endişelenmeyin</strong><p>Henüz kameranız yoksa bu sonuç normaldir. Kamera bağlıysa aşağıdaki adımları deneyin; teknik terimleri bilmeniz gerekmiyor.</p></div>
  <div class="vo-help-grid">
    <article class="vo-help-card"><span class="vo-help-number">1</span><h4>Kamera açık mı?</h4><p>Kamera veya kayıt cihazı çalışıyor olmalı. Üreticinin kendi uygulamasından görüntü açılabildiğini kontrol edin.</p></article>
    <article class="vo-help-card"><span class="vo-help-number">2</span><h4>Bilgisayarla aynı ağda mı?</h4><p>Windows bilgisayarı ve kamera aynı modem ya da erişilebilir yerel ağda olmalı. Misafir Wi-Fi ve ayrı kamera ağları keşfi engelleyebilir.</p></article>
    <article class="vo-help-card"><span class="vo-help-number">3</span><h4>Kamera erişimine izin verildi mi?</h4><p>Bazı kameralarda yerel görüntü paylaşımı kapalıdır. Markanızı seçin; gerekli adımı burada göstereceğiz.</p></article>
  </div>
  <div class="vo-guide-group">
    <label for="vo-help-brand">Kamera veya kayıt cihazının markası</label>
    <select id="vo-help-brand"><option value="general">Markasını bilmiyorum / diğer</option><option value="tapo">TP-Link Tapo</option><option value="mercusys">Mercusys</option><option value="ezviz">EZVIZ</option><option value="imou">Imou</option><option value="hikvision">Hikvision / HiLook</option><option value="dahua">Dahua</option><option value="nvr">NVR / DVR kayıt cihazı</option></select>
    <div id="vo-help-brand-guide" class="vo-answered" role="status"></div>
    <details class="vo-terms"><summary>“Yerel kamera erişimi”, ONVIF ve RTSP ne anlama geliyor?</summary><p><b>Yerel kamera erişimi</b>, VisionOps'un aynı ağdaki cihazdan görüntü alabilmesidir. <b>ONVIF</b> kamerayı bulup yayın seçeneklerini öğrenmeye, <b>RTSP</b> canlı görüntüyü aktarmaya yarar. Bunları bilmeniz şart değil; marka rehberini izleyebilirsiniz.</p></details>
  </div>
  <div class="vo-helper">
    <div><h4>Kurulum yardımcısı</h4><p>Size en yakın sorunu seçin; hangi ayarı kontrol edeceğinizi adım adım anlatalım. Kamera parolanızı bu yardım alanına yazmayın.</p></div>
    <label for="vo-help-question">Nerede takıldınız?</label>
    <select id="vo-help-question"><option value="notfound">Kamera bulunamıyor</option><option value="access">Kullanıcı adı veya parola soruyor</option><option value="protocol">Kamerada yerel erişim ayarını bulamıyorum</option><option value="nvr">Kayıt cihazına bağlı kameralarım var</option><option value="picture">Kamera eklendi ama görüntü açılmıyor</option><option value="network">Kameram farklı Wi-Fi/ağ üzerinde</option></select>
    <button id="vo-help-ask" type="button" class="btn">Bana ne yapacağımı göster</button>
    <div id="vo-help-answer" class="vo-answered" role="status" aria-live="polite"></div>
  </div>
  <div class="vo-help-actions"><button type="button" class="btn primary" id="vo-scan-retry">Tekrar tara</button><button type="button" class="btn" id="vo-manual-reveal">Elle eklemeyi göster (son seçenek)</button></div>
</section>
<form id="vo-connect-form" class="vo-connect-form" hidden autocomplete="off">
<h4 id="vo-connect-selected">Seçilen kamera</h4>
<div class="vo-connect-fields">
<label>Kamera adı<input id="vo-connect-label" required maxlength="40" autocomplete="off"></label>
<label>Kamera kullanıcı adı<input id="vo-connect-user" maxlength="128" autocomplete="off"></label>
<label>Kamera parolası<input id="vo-connect-pass" type="password" maxlength="256" autocomplete="new-password"></label>
<label id="vo-connect-path-wrap" hidden>RTSP yayın yolu<input id="vo-connect-path" maxlength="512" placeholder="/stream1" autocomplete="off"></label>
<label id="vo-connect-profile-wrap" hidden>ONVIF video profili<select id="vo-connect-profile"><option value="">Önce bağlantıyı denetle</option></select></label>
</div>
<label class="vo-connect-hint"><input id="vo-connect-remember" type="checkbox" checked> Kamerayı bu Windows hesabında güvenli biçimde kaydet</label>
<div class="vo-connect-actions"><button type="button" class="btn" id="vo-connect-check">2 · Bağlantıyı denetle</button><button type="submit" class="btn primary" id="vo-connect-add" disabled>3 · Kamerayı ekle ve görüntüyü doğrula</button><button type="button" class="btn" id="vo-connect-manual">Manuel ayarlar</button></div>
<p class="vo-connect-note" id="vo-connect-help" role="status">Cihazı seçtikten sonra bağlantı adımlarını takip edin.</p>
</form>
<div id="vo-connect-manual-slot" hidden></div>
<p class="vo-connect-hint vo-footer-note">Bulunan kamerayı seçtiğinizde erişimi adım adım kontrol ederiz. Görüntüyü doğrulamadan “bağlandı” demeyiz.</p>
</section>
