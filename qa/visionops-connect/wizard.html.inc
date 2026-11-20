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
</style>
<div class="vo-connect-header"><div><h3>VisionOps Connect · Akıllı kamera kurulumu</h3><p class="vo-connect-hint">Önce kameraları bul, sonra bağlantı yetkisini doğrula. RTSP adresi tek başına görüntünün açıldığını kanıtlamaz.</p></div><button type="button" class="btn primary" id="vo-connect-scan">1 · Kameraları Bul</button></div>
<label class="vo-connect-hint"><input type="checkbox" id="vo-connect-extended"> Gelişmiş yerel ağ taramasına izin ver (yalnızca bağlı olduğum ağların /24 adreslerinde RTSP 554/8554 kontrolü; daha fazla trafik oluşturur)</label>
<p id="vo-connect-status" role="status" aria-live="polite">Tarama başlatılmadı. Kameraların ve bilgisayarın yerel ağda erişilebilir olması gerekir.</p>
<div id="vo-connect-devices" aria-label="Bulunan kameralar"></div>
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
<p class="vo-connect-hint">ONVIF açık değilse veya cihaz ağdan keşfedilemiyorsa aşağıdaki manuel bağlantı ayarlarını kullanın. Kamera şifresi tarama sırasında denenmez ve buluta gönderilmez.</p>
</section>
