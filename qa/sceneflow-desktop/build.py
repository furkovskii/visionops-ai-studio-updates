"""Build unreleased SceneFlow desktop preview v0.13.7 QA on verified v0.13.6.
Actual rule engine remains prototype-only until calibrated live camera integration.
"""
import json,subprocess,sys,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).resolve().parent
OUT=ROOT/'sceneflow-desktop-build';OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/';VERSION='0.13.7'
def inc(name):return (HERE/name).read_text(encoding='utf-8')
def once(s,old,new):
    assert s.count(old)==1,('Unrecognized desktop anchor',old[:120],s.count(old))
    return s.replace(old,new)
def patch_html(s):
    s=once(s,'<section id="vo-connect"',
           inc('preview.html.inc')+'\n<section id="vo-connect"')
    s=once(s,'async function onvifRequest(path,body){',
           inc('preview.js.inc')+'\nasync function onvifRequest(path,body){')
    return s
subprocess.run([sys.executable,str(ROOT/'qa/brands-restart/build.py')],check=True,cwd=ROOT)
subprocess.run([sys.executable,str(ROOT/'qa/sceneflow/test_sceneflow.py')],check=True,cwd=ROOT)
for name in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'brands-restart-build'/name) as old, zipfile.ZipFile(OUT/name,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX) and '..' not in Path(item.filename).parts
            rel=item.filename[len(PREFIX):]
            blob=old.read(item)
            if rel=='templates/activate.html':
                blob=patch_html(blob.decode('utf-8').replace('\r\n','\n').replace('\r','\n')).encode('utf-8')
            elif rel=='version.json':
                blob=(json.dumps({'version':VERSION})+'\n').encode('utf-8')
            if rel.endswith(('.py','.pyw')):compile(blob,rel,'exec')
            new.writestr(item,blob)
    with zipfile.ZipFile(OUT/name) as package:
        assert package.testzip() is None
        assert json.loads(package.read(PREFIX+'version.json'))['version']==VERSION
        if name.endswith('Setup.zip'):package.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as u,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as s:
    for name in u.namelist():assert u.read(name)==s.read(name),name
print('SceneFlow desktop design preview v0.13.7 QA built; NOT released to customer auto-updates.')
