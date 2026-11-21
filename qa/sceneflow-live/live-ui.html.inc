<div class="vo-sf-live" id="vo-sf-live">
<style>
.vo-sf-live{border-top:1px solid #cbe0b94d;margin-top:23px;padding-top:20px}
.vo-sf-live h4{font-size:19px;margin:0 0 7px;color:#f0f7e9;letter-spacing:-.02em}
.vo-sf-live p{font-size:12px;color:#c1d5c5;line-height:1.7;margin:0 0 13px}
.vo-sf-live label{display:grid;gap:6px;color:#d4e5d1;font-size:12px;font-weight:650}
.vo-sf-live select,.vo-sf-live input{border:1px solid #c7dbbd6e;background:#293e37;color:#fff;border-radius:7px;min-height:42px;padding:9px;width:100%;font:inherit}
.vo-sf-live option{background:#213630}
.vo-sf-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.vo-sf-toolbar{display:flex;flex-wrap:wrap;gap:9px;align-items:end;margin:14px 0}
.vo-sf-toolbar label{min-width:220px;max-width:440px;flex:1}
.vo-sf-live button{min-height:42px;border-radius:7px;padding:10px 13px;font:600 12px/1.3 inherit;cursor:pointer;border:1px solid #cfdfa481;background:#293e37;color:#edfae2}
.vo-sf-live button:hover{background:#365643}
.vo-sf-live button:disabled{opacity:.45;cursor:not-allowed}
.vo-sf-live button.vo-sf-primary{background:#dff39e;color:#1d372e;border-color:#dff39e}
.vo-sf-rule{background:#1b322c;border:1px solid #9fbaa555;border-radius:10px;padding:15px;margin:12px 0}
.vo-sf-rule-head{display:flex;gap:10px;align-items:center;justify-content:space-between;margin-bottom:13px}
.vo-sf-rule-head strong{font-size:13px;color:#edf9e7}
.vo-sf-rule-head button{padding:7px 11px;min-height:32px;font-size:11px}
.vo-sf-evidence{background:#29483c;border:1px solid #b3d49a6b;border-radius:9px;padding:11px;margin-top:12px;font-size:12px;color:#eef5e7;line-height:1.7}
.vo-sf-evidence strong{color:#dff39e}
.vo-sf-evidence p{font-size:11px;margin:7px 0 0}
.vo-sf-note{padding:12px;background:#dff39e13;border:1px solid #dff39e57;border-radius:9px}
.vo-sf-status{min-height:22px;color:#e2edca!important}
@media(max-width:660px){.vo-sf-grid{grid-template-columns:1fr}.vo-sf-toolbar{flex-direction:column;align-items:stretch}.vo-sf-toolbar label{min-width:0;max-width:none;width:100%}}
</style>
<h4>SceneFlow Lab · Kamera olaylarından gerçek kurallar</h4>
<p>Mevcut kameralarınızı değiştirmeden, bir analiz alanı için giriş veya çıkış eşiği oluşturun. Ölçüm yalnızca cihazın gerçekten gördüğü karelere dayanır. Bu aşamada serbest metin yorumlama veya otomatik alan çizimi bulunmaz.</p>
<div class="vo-sf-toolbar">
<label>Kaydedilmiş kamera
<select id="vo-sf-camera"><option value="">Kameralar yükleniyor…</option></select></label>
<button type="button" id="vo-sf-load">Kameraları yenile</button>
</div>
<div id="vo-sf-status" class="vo-sf-status" role="status" aria-live="polite">İş akışı motoru hazır. Kamera seçildiğinde yerel kurallar okunur.</div>
<div id="vo-sf-rule-list"></div>
<div class="vo-sf-toolbar">
<button type="button" id="vo-sf-add" disabled>+ Gözlem kuralı</button>
<button type="button" id="vo-sf-save" class="vo-sf-primary" disabled>Kuralları kaydet</button>
<button type="button" id="vo-sf-refresh" disabled>TruthMeter sonuçlarını yenile</button>
</div>
<p class="vo-sf-note"><strong>Veri doğrulama:</strong> Görüntünün kesilmesi veya analizin kapalı olması bir olayın gerçekleşmediği anlamına gelmez. TruthMeter gözlem kapsamını gösterir; algılama modelinin gerçek doğruluğu ayrıca test edilmeden yüzde olarak sunulmaz. ActionLab karşılaştırması kesin bir kazanç veya sebep-sonuç kanıtı değildir.</p>
</div>
