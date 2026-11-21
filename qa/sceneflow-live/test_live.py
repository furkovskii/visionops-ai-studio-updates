"""Release safety tests: real detector-zone events, owner isolation and unknown evidence."""
import ast,contextlib,dataclasses,hashlib,importlib.util,json,math,sqlite3,tempfile,threading,time,unittest,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
APP=ROOT/'sceneflow-live-build'/'VisionOps_AI_Studio'/'app.py'
source=APP.read_text(encoding='utf-8')
tree=ast.parse(source)
names={'_sf_zone','_sf_camera','_sf_database','_sf_saved','_sf_save','_sf_frame','_sf_result','_sf_scope_for'}
nodes=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name in names]
assert len(nodes)==len(names),[n.name for n in nodes]
mod=ast.Module(body=nodes,type_ignores=[])
wanted={n.name for n in nodes}
spec=importlib.util.spec_from_file_location('sf_original',ROOT/'qa/sceneflow/sceneflow.py')
import sys
sf=importlib.util.module_from_spec(spec);sys.modules[spec.name]=sf;spec.loader.exec_module(sf)
globals().update({name:getattr(sf,name) for name in ('Rule','Event','Coverage','evaluate','compare')})
class CloudError(Exception):
 def __init__(self,text,status=400):super().__init__(text);self.status=status
_LOCK=threading.RLock()
_ip_cameras={'cafe':{},'pet':{},'farm':{}}
_SF_LABELS={'cafe': {'insan','kişi','person','human','people'},
            'pet': {'kedi','köpek','cat','dog','bird','kuş','pet'},
            'farm': {'inek','sığır','koyun','keçi','at','domuz','manda','cow','cattle','sheep','goat','horse','pig','buffalo','animal','hayvan'}}
_SF_HISTORY_SECONDS=3900;_SF_MAX_EVENTS=12000;_SF_MAX_COVERAGE=5000
_CAP=sf.CAPTURE
CAPTURE=_CAP
asdict=dataclasses.asdict
_sf_hashlib=hashlib
_sf_closing=contextlib.closing
def _load_saved_cameras(module,owner):return {}
def _wall_config(module,owner,camera_id):return {'regions':[{'name':'Giriş kapısı','bounds':[0,0,1,1]}]}
exec(compile(mod,str(APP),'exec'),globals())
def config_rule(module='cafe',event='zone_enter',duration=30,threshold=1):
 return sf.Rule(rule_id='door',module=module,camera_id='cam01',zone_id=_sf_zone('Giriş kapısı'),
                subject='person' if module=='cafe' else 'animal',
                event=event,condition='at_least',threshold=threshold,window_seconds=duration)
def test_camera(module='cafe',owner='u1'):
 return {'id':'cam01','owner':owner,'module':module,'config':{'regions':[{'name':'Giriş kapısı','bounds':[0,0,1,1]}]}}
class SceneFlowIntegrationTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory()
  self.addCleanup(self.tmp.cleanup)
  def wall_db(module,owner):
   filename=Path(self.tmp.name)/(module+'-'+hashlib.sha256(owner.encode()).hexdigest()[:12]+'.db')
   db=sqlite3.connect(filename);db.row_factory=sqlite3.Row;return db
  globals()['_wall_db']=wall_db
  for key in _ip_cameras:_ip_cameras[key].clear()
 def test_frame_pipeline_real_events_only_with_coverage(self):
  cam=test_camera()
  for sec in range(1,33):
   events=[('Giriş kapısı','Giriş','temporary-track','İnsan',None)] if sec in (8,16) else []
   _sf_frame(cam,events,float(sec),(1,1))
  receipt=_sf_result(config_rule(threshold=2),cam['sf_state'],33.)
  self.assertEqual(receipt['verdict'],'triggered')
  self.assertEqual(receipt['observed_events'],2)
  self.assertAlmostEqual(receipt['coverage_ratio'],.9667,places=2)
  self.assertIsNone(receipt['validated_accuracy'])
  self.assertNotIn('temporary-track',str(receipt))
 def test_outage_does_not_count_events_or_camera_time(self):
  cam=test_camera()
  _sf_frame(cam,[],1.,(1,1))
  _sf_frame(cam,[('Giriş kapısı','Giriş','x','İnsan',None)],120.,(1,1))
  receipt=_sf_result(config_rule(duration=30),cam['sf_state'],121.)
  self.assertEqual(receipt['verdict'],'unknown')
  self.assertIsNone(receipt['observed_events'])
 def test_owner_scoped_rule_storage_and_validation(self):
  cam=test_camera()
  _ip_cameras['cafe']['cam01']=cam
  form={'rules':[dict(rule_id='door',zone_id=_sf_zone('Giriş kapısı'),event='zone_enter',
                       condition='at_least',threshold=1,window_seconds=30,minimum_coverage=.9)]}
  saved=_sf_save('cafe','u1','cam01',form)
  self.assertEqual(len(saved),1)
  self.assertEqual(_sf_saved('cafe','u1','cam01')[0].rule_id,'door')
  self.assertEqual(_sf_saved('cafe','u2','cam01'),[])
  with self.assertRaises(CloudError):_sf_camera('cafe','u2','cam01')
  self.assertEqual(_sf_camera('cafe','u1','cam01')['owner'],'u1')
  with self.assertRaises(CloudError):_sf_save('cafe','u1','cam01',{'rules':[dict(form['rules'][0],face_id='private')]})
  self.assertEqual(len(_sf_saved('cafe','u1','cam01')),1)
  with self.assertRaises(CloudError):_sf_save('cafe','u1','cam01',{'rules':[dict(form['rules'][0],zone_id='bad')]})
 def test_rules_reject_identity_and_third_party_modules(self):
  for module in ('pet','farm'):
   cam=test_camera(module)
   for second in range(1,33):
    ev=[('Giriş kapısı','Giriş','hidden','İnsan',None)] if second==16 else []
    _sf_frame(cam,ev,float(second),(1,1))
   result=_sf_result(config_rule(module=module),cam['sf_state'],33.)
   self.assertEqual(result['observed_events'],0)
   self.assertEqual(result['verdict'],'not_triggered')
  cam=test_camera('pet')
  for second in range(1,33):
   ev=[('Giriş kapısı','Giriş','hidden','Kedi',None)] if second==16 else []
   _sf_frame(cam,ev,float(second),(1,1))
  self.assertEqual(_sf_result(config_rule(module='pet'),cam['sf_state'],33.)['observed_events'],1)
 def test_disabled_or_reconfigured_token_no_stale_coverage(self):
  cam=test_camera()
  for i in range(1,35):_sf_frame(cam,[],float(i),(1,1))
  self.assertEqual(_sf_result(config_rule(),cam['sf_state'],35.)['verdict'],'not_triggered')
  _sf_frame(cam,[],36.,(2,1))
  self.assertEqual(_sf_result(config_rule(),cam['sf_state'],37.)['verdict'],'unknown')
 def test_overflow_refuses_to_invent_count(self):
  cam=test_camera()
  cam['sf_state']={'token':(1,1),'last':30.,'coverage':[],
                   'events':[Event(20.,'cam01','cafe',_sf_zone('Giriş kapısı'),'person','zone_enter')]*_SF_MAX_EVENTS,'overflow_time':None}
  _sf_frame(cam,[('Giriş kapısı','Giriş','hidden','İnsan',None)],31.,(1,1))
  result=_sf_result(config_rule(),cam['sf_state'],32.)
  self.assertEqual(result['verdict'],'unknown')
 def test_actionlab_comparison_only_when_evidence_complete(self):
  cam=test_camera()
  for second in range(1,80):
   event=[('Giriş kapısı','Giriş','anonymous','İnsan',None)] if second in (7,18,34) else []
   _sf_frame(cam,event,float(second),(1,1))
  current=_sf_result(config_rule(duration=30),cam['sf_state'],79.5)
  self.assertTrue(current['comparison']['enough_evidence'])
  self.assertEqual(current['comparison']['relative_change'],-100.)
  self.assertIn('nedensel',current['comparison']['interpretation'])
 def test_camera_selector_source_contract(self):
  body=ast.get_source_segment(source,next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='ip_cameras_api'))
  self.assertIn('return jsonify(cameras=',body)
  self.assertIn('request.method',body)
 def test_release_package_compatibility(self):
  path=ROOT/'sceneflow-live-build'
  with zipfile.ZipFile(path/'VisionOps_AI_Studio_update.zip') as update,zipfile.ZipFile(path/'VisionOps_AI_Studio_Setup.zip') as setup:
   self.assertEqual(json.loads(update.read('VisionOps_AI_Studio/version.json'))['version'],'0.13.7')
   for f in update.namelist():self.assertEqual(update.read(f),setup.read(f),f)
   html=update.read('VisionOps_AI_Studio/templates/activate.html').decode('utf-8')
   app=update.read('VisionOps_AI_Studio/app.py').decode('utf-8')
   for marker in ('id="vo-connect"','id="vo-sf-camera"','id="vo-sf-save"','X-CSRF-Token',
                  '/api/workspace/sceneflow/','id="vo-scene"'):
    self.assertIn(marker,html)
   self.assertIn('def sceneflow_api(camera_id)',app)
   self.assertIn('_sf_frame(camera,events,stamp,token)',app)
   self.assertIn('sf_state={}',app)
   self.assertNotIn('__SCENEFLOW_CSRF__',html)
if __name__=='__main__':unittest.main(verbosity=2)
