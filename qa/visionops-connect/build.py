"""Build VisionOps Connect v0.13.4 QA from the verified v0.13.3 candidate.
No additional runtime files or dependencies are introduced into the updater.
"""
from pathlib import Path
import json,zipfile,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).resolve().parent
OUT=ROOT/'connect-qa-build';OUT.mkdir(exist_ok=True)
VERSION='0.13.4'; PREFIX='VisionOps_AI_Studio/'
def inc(name):return (HERE/name).read_text(encoding='utf-8')
def one(s,before,after):
    assert s.count(before)==1,('Unexpected release source anchor',before[:100],s.count(before))
    return s.replace(before,after)
def patch_app(s):
    a=s.index('def _discover_onvif(timeout=3.0):');b=s.index('\ndef _onvif_soap(',a)
    s=s[:a]+inc('discovery.py.inc')+'\n'+s[b:]
    # Media2 (Profile T) SOAP namespace is opt-in, no dynamic endpoint expansion.
    s=one(s,"        'xmlns:tt=\"http://www.onvif.org/ver10/schema\">'",
          "        'xmlns:tr2=\"http://www.onvif.org/ver20/media/wsdl\" '\n        'xmlns:tt=\"http://www.onvif.org/ver10/schema\">'")
    a=s.index('def _onvif_source(form):');b=s.index('\ndef _camera_store_path(',a)
    s=s[:a]+inc('media2.py.inc')+'\n\n'+s[b:]
    s=one(s,"@app.post('/api/workspace/onvif/discover')",
          inc('routes.py.inc')+"\n\n@app.post('/api/workspace/onvif/discover')")
    return s
def patch_html(s):
    s=one(s,'<details class="wall-add" open>',
          inc('wizard.html.inc')+'\n<details class="wall-add">')
    js=inc('wizard.js.inc').replace("'__VO_CSRF_VALUE__'","{{ csrf|tojson }}")
    s=one(s,'async function onvifRequest(path,body){',
          js+'\nasync function onvifRequest(path,body){')
    return s
subprocess.run([sys.executable,str(ROOT/'qa/smart-analytics/build.py')],cwd=ROOT,check=True)
subprocess.run([sys.executable,str(ROOT/'qa/smart-analytics/test_smart.py')],cwd=ROOT,check=True)
for name in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'smart-qa-build'/name) as old,zipfile.ZipFile(OUT/name,'w',zipfile.ZIP_DEFLATED) as new:
        for info in old.infolist():
            if info.is_dir():continue
            assert info.filename.startswith(PREFIX) and '..' not in Path(info.filename).parts
            rel=info.filename[len(PREFIX):]
            data=old.read(info)
            if rel in ('app.py','templates/activate.html'):
                text=data.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
                data=(patch_app(text) if rel=='app.py' else patch_html(text)).encode('utf-8')
            elif rel=='version.json':data=(json.dumps({'version':VERSION})+'\n').encode('utf-8')
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(info,data)
    with zipfile.ZipFile(OUT/name) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX+'version.json'))['version']==VERSION
        assert b'__VO_CSRF_VALUE__' not in z.read(PREFIX+'templates/activate.html')
        if name.endswith('Setup.zip'):z.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as update,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as setup:
    for entry in update.namelist():assert update.read(entry)==setup.read(entry),entry
print('VisionOps Connect 0.13.4 QA packages built; Media1/2, wizard and existing AI tests retained.')
