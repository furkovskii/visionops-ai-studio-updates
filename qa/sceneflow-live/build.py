"""Build VisionOps v0.13.9: local SceneFlow rules and TruthMeter, ActionLab.
Extends the v0.13.6 signed-release source. No new runtime dependencies,
no camera credential persistence, no change to vendor ONVIF/RTSP driver.
"""
import ast,json,re,subprocess,sys,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]; HERE=Path(__file__).resolve().parent
OUT=ROOT/'sceneflow-live-build';OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/';VERSION='0.13.9'
def src(name):return (HERE/name).read_text(encoding='utf-8')
def once(text,old,new):
    count=text.count(old)
    assert count==1,('Source anchor changed: '+old[:80],count)
    return text.replace(old,new)
def core():
    text=(ROOT/'qa/sceneflow/sceneflow.py').read_text(encoding='utf-8')
    text=once(text,'from __future__ import annotations\n','')
    return text
def patch_app(text):
    text=once(text,'def _wall_db(module, owner):',
        '# ---- SceneFlow rules, TruthMeter coverage & ActionLab experiment evidence ----\n'
        +core()+'\n\n'+src('runtime.py.inc')+'\n\ndef _wall_db(module, owner):')
    text=once(text,'                            _wall_ai_alerts(camera,rows,events,detections,stamp,token)\n',
        '                            _wall_ai_alerts(camera,rows,events,detections,stamp,token)\n'
        '                            _sf_frame(camera,events,stamp,token)\n')
    text=once(text,'if live:live.update(config=config,alert_token=None,smart_state={})',
        'if live:live.update(config=config,alert_token=None,smart_state={},sf_state={})')
    # Verify the engine is not reachable from unauthenticated public endpoint.
    tree=ast.parse(text)
    entry=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='sceneflow_api')
    assert any(isinstance(n,ast.Name) and n.id=='_sf_scope_for' for n in ast.walk(entry))
    return text
def patch_html(text):
    # A self-contained, accessible customer command center before the existing live tools.
    hero=src('command-hero.b64').strip()
    panel=src('command-center.html.inc').replace('__VC_HERO_DATA__',hero)
    assert '__VC_HERO_DATA__' not in panel and hero.startswith('UklGR')
    text=once(text,'<section class="vo-scene"',panel+'\n<section class="vo-scene"')
    start=text.index('<section class="vo-scene"')
    stop=text.index('\n</section>',start)
    text=text[:stop]+'\n'+src('live-ui.html.inc')+text[stop:]
    # Inject beside already verified inline camera installation script.
    js=src('live-ui.js.inc').replace("'__SCENEFLOW_CSRF__'","{{ csrf|tojson }}")
    assert '__SCENEFLOW_CSRF__' not in js
    text=once(text,'async function onvifRequest(path,body){',src('command-center.js.inc')+'\n'+js+'\nasync function onvifRequest(path,body){')
    return text
subprocess.run([sys.executable,str(ROOT/'qa/sceneflow-desktop/build.py')],check=True,cwd=ROOT)
subprocess.run([sys.executable,str(ROOT/'qa/sceneflow-desktop/test_desktop.py')],check=True,cwd=ROOT)
for package_name in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'sceneflow-desktop-build'/package_name) as old,zipfile.ZipFile(OUT/package_name,'w',zipfile.ZIP_DEFLATED) as new:
        for entry in old.infolist():
            if entry.is_dir():continue
            assert entry.filename.startswith(PREFIX) and '..' not in Path(entry.filename).parts and '\\' not in entry.filename
            rel=entry.filename[len(PREFIX):];data=old.read(entry)
            if rel in ('app.py','templates/activate.html'):
                text=data.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
                data=(patch_app(text) if rel=='app.py' else patch_html(text)).encode('utf-8')
            elif rel=='version.json':
                data=(json.dumps({'version':VERSION})+'\n').encode('utf-8')
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(entry,data)
    with zipfile.ZipFile(OUT/package_name) as package:
        assert package.testzip() is None
        assert json.loads(package.read(PREFIX+'version.json'))['version']==VERSION
        if package_name.endswith('Setup.zip'):package.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as u,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as s:
    for entry in u.namelist():assert u.read(entry)==s.read(entry),entry
print('VisionOps SceneFlow v0.13.9 live-rule Windows update and installer prepared; no claim of physical camera calibration.')
