"""Reproducible candidate extending the verified AI Engine v0.13.2 builder."""
import hashlib,json,zipfile,subprocess,sys,re
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
OUT=ROOT/'smart-qa-build';OUT.mkdir(exist_ok=True)
VERSION='0.13.3'
PREFIX='VisionOps_AI_Studio/'
def one(s,a,b):
    assert s.count(a)==1, 'Unexpected source: '+a[:100]
    return s.replace(a,b)
def inc(name):return (HERE/name).read_text(encoding='utf-8')
def patch_app(s):
    s=one(s,'def _wall_rule_defaults():',inc('engine.py.inc')+'\n\ndef _wall_rule_defaults():')
    s=one(s,"'auto_near':False}","'auto_near':False,'scenarios':[]}")
    s=one(s,"    return rules\n\ndef _wall_alert(", "    rules['scenarios']=_smart_validate(rules['scenarios'],regions)\n    return rules\n\ndef _wall_alert(")
    # Existing settings API preserves independently configured scenarios from older clients.
    s=one(s,"            config['rules']=_wall_rules(request.get_json(silent=True),config['regions'])","            form=request.get_json(silent=True)\n            if isinstance(form,dict) and 'scenarios' not in form:form={**form,'scenarios':config['rules'].get('scenarios',[])}\n            config['rules']=_wall_rules(form,config['regions'])")
    # Zone deletion explicitly removes now-invalid zone-dependent scenarios.
    s=one(s,"                old_config=_wall_config(module,owner,camera_id)","                if isinstance(form.get('rules'),dict) and isinstance(form.get('regions'),list):\n                    names=[r.get('name') for r in form['regions'] if isinstance(r,dict)]\n                    rules=form['rules']\n                    if isinstance(rules.get('scenarios'),list):\n                        form={**form,'rules':{**rules,'scenarios':[v for v in rules['scenarios'] if not isinstance(v,dict) or v.get('kind')=='count' or v.get('zone') in names]}}\n                old_config=_wall_config(module,owner,camera_id)")
    s=one(s,"    camera['alert_previous']=(stamp,count)","    camera['alert_previous']=(stamp,count)\n    smart,notices=_smart_step(camera.get('smart_state',{}),rules.get('scenarios',[]),camera['config']['regions'],rows,events,detections,stamp,token)\n    for kind,message in notices:_wall_alert(camera,kind,message)\n    camera['smart_state']=smart")
    # Rule edits reset state even when the tracking generation is unchanged.
    s=one(s,"if live:live.update(config=config,alert_token=None)","if live:live.update(config=config,alert_token=None,smart_state={})")
    s=one(s,"return {'insights':_wall_insights(module,owner,key,today)","return {'smart':_smart_report(module,owner,key,today),'insights':_wall_insights(module,owner,key,today)")
    return s
def patch_template(s):
    start=s.index('{% macro wall_settings_markup() %}');end=s.index('{% endmacro %}',start)
    block=s[start:end];match=re.search(r'id="wall-rule-fields"[^>]*>',block)
    assert match,'Missing rule editor'
    block=block[:match.end()]+inc('rules.html.inc')+block[match.end():]
    s=s[:start]+block+s[end:]
    s=one(s,'function writeRules(r,regions=[]){r={...ruleDefaults,...r};syncRuleZones(regions);',inc('rules.js.inc')+'\nfunction writeRules(r,regions=[]){r={...ruleDefaults,...r};syncRuleZones(regions);smartWrite(r.scenarios??[],regions);')
    a=s.index('function readRules(){');b=s.index('function ruleEnabled(',a)
    s=s[:a]+one(s[a:b],'return r;','r.scenarios=smartRead();return r;')+s[b:]
    s=one(s,'<h3>Yoğunluk haritası</h3>',inc('report.html.inc')+'<h3>VisionOps Heatmap · Yoğunluk haritası</h3>')
    s=one(s,'Son 7 günün örneklenmiş algılama merkezleri. Kişi/hayvan tekil sayısı değildir.','Seçili dönemde örneklenmiş algılama merkezleri. Kişi/hayvan tekil sayısı değildir.')
    s=one(s,'function renderReport(data){latestReport=data;renderInsights();',inc('report.js.inc')+'\nfunction renderReport(data){latestReport=data;renderInsights();')
    s=one(s,"byId('wall-heat-label').textContent=max?'En yoğun hücre: '+max+' örneklenmiş algılama':'Henüz algılama yok.';","renderSmartReport(data);")
    return s
subprocess.run([sys.executable,str(ROOT/'qa/ai-engine/build.py')],cwd=ROOT,check=True)
subprocess.run([sys.executable,str(ROOT/'qa/ai-engine/test_ai_engine.py')],cwd=ROOT,check=True)
for name in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'ai-qa-build'/name) as old,zipfile.ZipFile(OUT/name,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX)
            rel=item.filename[len(PREFIX):];assert '..' not in Path(rel).parts and '\\' not in rel
            data=old.read(item)
            if rel in ('app.py','templates/activate.html'):
                text=data.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
                data=(patch_app(text) if rel=='app.py' else patch_template(text)).encode('utf-8')
            elif rel=='version.json':data=(json.dumps({'version':VERSION})+'\n').encode()
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(item,data)
    with zipfile.ZipFile(OUT/name) as z:
        assert z.testzip() is None and json.loads(z.read(PREFIX+'version.json'))['version']==VERSION
        if name.endswith('Setup.zip'):z.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as u,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as s:
    for rel in u.namelist():assert u.read(rel)==s.read(rel),rel
print('Smart analytics QA packages verified; existing AI tests retained; no production publish.')
