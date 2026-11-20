"""Build AI Engine candidate from signed-release input, leaving Camera Engine PR independent."""
import hashlib,json,zipfile,os
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1] if HERE.parent.name=='qa' else HERE
OUT=ROOT/'ai-qa-build'; OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/'
BASE={'release-input':('VisionOps_AI_Studio_update.zip','469711d2d36b619feeb44a8350034fd563c691f8563a0df9dad036373a5882de'),'setup-input':('VisionOps_AI_Studio_Setup.zip','2a4cf84a5fc7749ff2b4b8ea315334056e248879dc53331cb503961fd32eff9a')}
def one(s,a,b):
    assert s.count(a)==1, 'Unexpected source: '+a[:85]
    return s.replace(a,b)
def change(rel,data):
    if rel=='version.json':return b'{"version": "0.13.2"}\n'
    if rel!='app.py':return data
    s=data.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
    a=s.index('def _wall_track(');b=s.index('\ndef _wall_record(',a)
    s=s[:a]+(HERE/'tracker.py.inc').read_text(encoding='utf-8')+'\n'+s[b:]
    s=one(s,"_wall_track(previous,detections,config['regions'],stamp)","_wall_track(previous,detections,config['regions'],stamp,camera['module'])")
    s=one(s,"v['hits']+=1;v['last']=stamp;v['label']=label", "if not t.get('continuous',False):v.update(candidate=None,since=stamp,seat_since=None,seat_logged=False,object_candidate=None,object_since=stamp,table=None,object_contact=None)\n        v['observed']=v.get('observed',0.)+(max(0.,stamp-v.get('last',stamp)) if t.get('continuous',False) else 0.)\n        v['hits']+=1;v['last']=stamp;v['label']=label")
    s=one(s,"round(max(0,v['last']-v['first']),1)","round(v.get('observed',0.),1)")
    s=one(s,"max(0,int(stamp-v[\"first\"]))","max(0,int(v.get('observed',0.)))")
    # Reject degenerate model boxes before counts or tracking state can be polluted.
    s=one(s,"if cls==0:people.append({'x':int(x1)","if not all(__import__('math').isfinite(v) for v in (x1,y1,x2,y2)) or bounds[2]<=bounds[0] or bounds[3]<=bounds[1]:continue\n                        if cls==0:people.append({'x':int(x1)")
    return s.encode('utf-8')
for folder,(name,digest) in BASE.items():
    src=Path(os.environ.get('VISIONOPS_BASE',str(ROOT)))/folder/'v0.13.0.zip'
    assert hashlib.sha256(src.read_bytes()).hexdigest()==digest, 'Base release hash mismatch'
    with zipfile.ZipFile(src) as old,zipfile.ZipFile(OUT/name,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX)
            rel=item.filename[len(PREFIX):];assert '..' not in Path(rel).parts
            data=change(rel,old.read(item))
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(item,data)
    with zipfile.ZipFile(OUT/name) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX+'version.json'))['version']=='0.13.2'
        if folder=='setup-input':z.extractall(OUT)
print('AI candidate built from verified v0.13.0 input; both archives validated')
