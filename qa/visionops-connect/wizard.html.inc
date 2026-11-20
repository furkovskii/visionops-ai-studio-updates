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
</style>
<div class="vo-connect-header"><div><h3>VisionOps Connect · Akıllı kamera kurulumu</h3><p class="vo-connect-hint">Önce kameraları bul, sonra bağlantı yetkisini doğrula. RTSP adresi tek başına görüntünün açıldığını kanıtlamaz.</p></div><button type="button" class="btn primary" id="vo-connect-scan">1 · Kameraları Bul</button></div>
<ol class="vo-connect-stepper" id="vo-connect-steps" aria-label="Kamera ekleme adımları"><li data-state="active"><b>1</b> Kamerayı bul</li><li data-state="pending"><b>2</b> Erişimi doğrula</li><li data-state="pending"><b>3</b> Görüntüyü başlat</li></ol>
<label class="vo-connect-scan-extra vo-connect-hint"><input type="checkbox" id="vo-connect-extended"> Gelişmiş yerel ağ taramasına izin ver (yalnızca bağlı olduğum ağların /24 adreslerinde RTSP 554/8554 kontrolü; daha fazla trafik oluşturur)</label>
<p id="vo-connect-status" role="status" aria-live="polite">Tarama başlatılmadı. Kameraların ve bilgisayarın yerel ağda erişilebilir olması gerekir.</p>
<div id="vo-connect-devices" class="vo-connect-devices" aria-label="Bulunan kameralar"></div>
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
<p class="vo-connect-hint">ONVIF açık değilse veya cihaz ağdan keşfedilemiyorsa aşağıdaki manuel bağlantı ayarlarını kullanın. Kamera şifresi tarama sırasında denenmez ve buluta gönderilmez. Gerekli kamera hesabı mevcut kamera sisteminizin yöneticisinden alınır.</p>
</section>
