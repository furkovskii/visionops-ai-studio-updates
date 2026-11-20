"""Reproducible Windows VisionOps v0.13.5 UI + automatic camera scan overlay.
Existing v0.13.4 builder/source are untouched for reproducible rollback.
"""
import ast,json,zipfile,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];HERE=Path(__file__).resolve().parent
OUT=ROOT/'connect-usability-build';OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/';VERSION='0.13.5'
def inc(name):return (HERE/name).read_text(encoding='utf-8')
def one(s,old,new):
    assert s.count(old)==1,('Source changed, refusing to patch',old[:120],s.count(old))
    return s.replace(old,new)
def patch_app(s):
    a=s.index('def _vo_interfaces():');b=s.index('\ndef _vo_parse_probe(',a)
    s=s[:a]+inc('interfaces.py.inc')+'\n'+s[b:]
    a=s.index("@app.post('/api/workspace/connect/scan')")
    b=s.index("@app.post('/api/workspace/connect/check')",a)
    s=s[:a]+inc('routes.py.inc')+'\n'+s[b:]
    s=one(s,'def _discover_onvif(timeout=3.5):','def _discover_onvif(timeout=3.5, interfaces=None):')
    s=one(s,'        interfaces=_vo_interfaces()\n        for ip in interfaces or [\'\']:',
          '        if interfaces is None:interfaces=_vo_interfaces()\n        for ip in interfaces or [\'\']:')
    s=one(s,'def _vo_scan_rtsp():','def _vo_scan_rtsp(interfaces=None):')
    s=one(s,'    for ip in _vo_interfaces()[:2]:',
          '    for ip in (interfaces if interfaces is not None else _vo_interfaces())[:2]:')
    assert "CREATE_NO_WINDOW" in s and '_vo_scan_rtsp(interfaces=interfaces)' in s
    return s
def patch_html(s):
    a=s.index('<section id="vo-connect"')
    b=s.index('<details class="wall-add"',a)
    assert s[a:b].count('<section id="vo-connect"')==1
    s=s[:a]+inc('wizard.html.inc')+'\n'+s[b:]
    a=s.index('// VisionOps Connect wizard.')
    b=s.index('async function onvifRequest(path,body){',a)
    src=inc('wizard.js.inc').replace("'__VO_CSRF_VALUE__'","{{ csrf|tojson }}")
    s=s[:a]+src+'\n'+s[b:]
    assert "id=\"vo-scan-progress\"" in s and 'id="vo-connect-fallback"' in s
    assert 'vo-connect-extended' not in s
    return s
subprocess.run([sys.executable,str(ROOT/'qa/visionops-connect/build.py')],cwd=ROOT,check=True)
subprocess.run([sys.executable,str(ROOT/'qa/visionops-connect/test_connect.py')],cwd=ROOT,check=True)
for archive in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'connect-qa-build'/archive) as old, zipfile.ZipFile(OUT/archive,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX) and '..' not in Path(item.filename).parts
            rel=item.filename[len(PREFIX):]
            blob=old.read(item)
            if rel in ('app.py','templates/activate.html'):
                src=blob.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
                blob=(patch_app(src) if rel=='app.py' else patch_html(src)).encode('utf-8')
            elif rel=='version.json':blob=(json.dumps({'version':VERSION})+'\n').encode('utf-8')
            if rel.endswith(('.py','.pyw')):compile(blob,rel,'exec')
            new.writestr(item,blob)
    with zipfile.ZipFile(OUT/archive) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX+'version.json'))['version']==VERSION
        if archive.endswith('Setup.zip'):z.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as update,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as setup:
    for name in update.namelist():assert update.read(name)==setup.read(name),name
print('v0.13.5 user-friendly camera setup packages rebuilt and matched, v0.13.4 retained.')
