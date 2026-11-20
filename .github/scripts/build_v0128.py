"""Reproduce the tested 0.12.8 inline personalization from pinned 0.12.7 inputs."""
from pathlib import Path
import json, zipfile, hashlib, tempfile
HASHES = {'release-input':'eb05156bf5b0eef4231882a108026e0b635a232518fcbf014319f3e6fb2a6f10','setup-input':'22f96edfbb813ff352df5d78d5a23b2df8d020a5c1b893a958d9ffe157f484ee'}
EXPECTED={'templates/activate.html':'2336cd9da6a4cbe463606f5686416a205fc438cebcae5f80b406e71d1a62f311','templates/index.html':'565abd477acb947cc153522f36d78729d3bd6d8eda345ca36aaaed6a1d75c69f','static/dashboard.js':'6bf8bbd049a00f6edb98fd5d9db726fce65d513d217a84621fd3a78993b4f5e4','version.json':'28d61061f5793342f32531b8cccec7ed603d031918105450cb8cf5f250713963'}
def digest(b):return hashlib.sha256(b).hexdigest()
def patch(p):
 t=p/'templates/activate.html';s=t.read_text();a=s.index('{% macro appearance_control');b=s.index('{% endmacro %}',a);m=s[a:b]
 def rep(x,y):
  nonlocal m
  assert x in m,x[:100];m=m.replace(x,y,1)
 rep('</style><dialog id="vo-preferences" aria-labelledby="vo-pref-title">','''
/* Appearance is a normal workspace page, never a modal. The existing sidebar/topbar remains visible. */
#vo-preferences{width:100%;height:auto;max-width:none;max-height:none;box-shadow:none;border-radius:12px;border:1px solid #daddE3;overflow:visible}
#vo-preferences .vp-window{height:auto;min-height:0}#vo-preferences .vp-layout{grid-template-columns:190px minmax(0,1fr);min-height:0}#vo-preferences .vp-rail,#vo-preferences .vp-chrome{display:none}#vo-preferences .vp-main{overflow:visible;min-width:0;border-radius:0 12px 0 0}#vo-preferences .vp-side{border-radius:12px 0 0 0}#vo-preferences .vp-footer{position:static;border-radius:0 0 12px 12px}body:has(#page-personalization.active) #vo-assist-open,body:has(#personalization-panel.active) #vo-assist-open{display:none!important}#vo-preferences :is(.vp-modes,.vp-presets,.vp-colors,.vp-layout-options){min-width:0}#vo-preferences .vp-side-note{margin-top:30px}#vo-preferences .vp-search{margin:0}#vo-preferences .vp-modes button,#vo-preferences .vp-presets button{margin:0}#vo-preferences .vp-main section{scroll-margin-top:14px}#vo-preferences .vp-color-row code{overflow-wrap:anywhere}
@media(max-width:1200px){ #vo-preferences .vp-layout{grid-template-columns:156px minmax(0,1fr)}#vo-preferences .vp-modes{grid-template-columns:repeat(2,minmax(0,1fr))}#vo-preferences .vp-presets{grid-template-columns:repeat(2,minmax(0,1fr))}#vo-preferences .vp-colors{grid-template-columns:1fr}}@media(max-width:850px){ #vo-preferences .vp-layout{grid-template-columns:minmax(0,1fr)}#vo-preferences .vp-side{display:none}#vo-preferences .vp-main{padding:16px 12px}#vo-preferences .vp-footer{flex-wrap:wrap;padding:10px}}
</style><section class="page" id="{{ 'page-personalization' if area=='pet' else 'personalization-panel' }}"><div id="vo-preferences" aria-labelledby="vo-pref-title">''')
 rep('<header class="vp-chrome"><span><b>V</b> VisionOps <i></i> Ayarlar</span><button type="button" id="vo-pref-close" aria-label="Kapat">×</button></header>','')
 rep('Kaydet ve kapat','Kaydet')
 rep('</footer></div></dialog>','</footer></div></div></section>')
 rep("dialog=$('vo-preferences'),{area,csrf}=config;","dialog=$('vo-preferences'),{area,csrf}=config,page=dialog.closest('.page');")
 rep("apply(dialog.open?draft:saved)","apply(page.classList.contains('active')?draft:saved)")
 rep("function close(){if(!busy){draft={...saved};apply(saved);dialog.close()}}dialog.addEventListener('cancel',e=>{e.preventDefault();close()});dialog.addEventListener('close',()=>{draft={...saved};apply(saved)});$('vo-pref-close').onclick=close;$('vo-pref-cancel').onclick=close;","function cancel(){if(!busy){draft={...saved};refresh();$('vo-pref-status').textContent='Kaydedilmemiş değişiklikler geri alındı.'}}$('vo-pref-cancel').onclick=cancel;")
 rep("saved=clean(d.preferences);apply(saved);dialog.close()","saved=clean(d.preferences);draft={...saved};refresh();$('vo-pref-status').textContent='Değişiklikler kaydedildi.'")
 start=m.index("const btn=document.createElement('button');btn.type='button';btn.className='vo-preference-trigger'");end=m.index('apply(saved)})();',start)
 m=m[:start]+"let wasActive=page.classList.contains('active');new MutationObserver(()=>{const active=page.classList.contains('active');if(active===wasActive)return;wasActive=active;if(active){draft={...saved};refresh();$('vo-pref-search').value='';filter();$('vo-pref-status').textContent='';}else{draft={...saved};apply(saved);}}).observe(page,{attributes:true,attributeFilter:['class']});"+m[end:]
 s=s[:a]+m+s[b:]
 act='{{appearance_control(appearance_key,module,appearance_values.get(module,{}),appearance_csrf)}}'
 assert s.count(act)==1;s=s.replace(act,'');pos=s.index('</div></main></div>\n<script>\n(function(){const tabs=');s=s[:pos]+act+'\n'+s[pos:]
 nav='<button type="button" data-tab="settings" class="">';assert nav in s;s=s.replace(nav,'{% if appearance_key %}<button type="button" data-tab="personalization"><svg><use href="#i-settings"/></svg>Kişiselleştirme</button>{% endif %}'+nav,1)
 old="settings:'settings-panel'};let current=";assert old in s;s=s.replace(old,"settings:'settings-panel',...(document.getElementById('personalization-panel')?{personalization:'personalization-panel'}:{})};let current=",1)
 t.write_text(s)
 t=p/'templates/index.html';s=t.read_text();act='{{appearance_control(appearance_key,"pet",appearance_values.get("pet",{}),appearance_csrf)}}';assert s.count(act)==1;s=s.replace(act,'');pos=s.index('<div id="toast"');s=s[:pos]+act+'\n'+s[pos:];nav='<button data-page="settings">';assert nav in s;s=s.replace(nav,'{% if appearance_key %}<button data-page="personalization"><svg><use href="#i-settings"/></svg>Kişiselleştirme</button>{% endif %}'+nav,1);t.write_text(s)
 t=p/'static/dashboard.js';s=t.read_text();old="settings:'Ayarlar & Güncellemeler'}";assert old in s;s=s.replace(old,"settings:'Ayarlar & Güncellemeler',personalization:'Kişiselleştirme'}",1);t.write_text(s)
 (p/'version.json').write_text(json.dumps({'version':'0.12.8'})+'\n')
 for name,expected in EXPECTED.items():
  actual=digest((p/name).read_bytes());assert actual==expected,(name,actual,expected)
for folder,expected in HASHES.items():
 src=Path(folder)/'v0.12.7.zip';dst=Path(folder)/'v0.12.8.zip'
 assert src.exists() and digest(src.read_bytes())==expected, f'Unverified base package: {src}'
 with tempfile.TemporaryDirectory() as tmp:
  dest=Path(tmp)
  with zipfile.ZipFile(src) as z:
   assert z.testzip() is None
   for n in z.namelist():
    assert n.startswith('VisionOps_AI_Studio/') and '..' not in Path(n).parts and '\\' not in n,n
   z.extractall(dest)
  patch(dest/'VisionOps_AI_Studio')
  with zipfile.ZipFile(dst,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for f in sorted(dest.rglob('*')):
    if f.is_file():z.write(f,f.relative_to(dest).as_posix())
  with zipfile.ZipFile(dst) as z:assert z.testzip() is None
 print(dst,digest(dst.read_bytes()))
