import ast, uuid, unittest, math, tempfile, sqlite3, json
from pathlib import Path
from datetime import datetime
from contextlib import closing
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1] if HERE.parent.name=='qa' else HERE
source=ROOT/'ai-qa-build'/'VisionOps_AI_Studio'/'app.py'
tree=ast.parse(source.read_text(encoding='utf-8'))
want={'_wall_track','_auto_scene','_auto_overlap','_auto_near','_wall_db','_wall_record','_wall_insights','_WORLD_FOR','_AUTO_ENTER','_AUTO_SEAT','_AUTO_EDGE','_AUTO_DOOR','_AUTO_SIT'}
nodes=[n for n in tree.body if getattr(n,'name',None) in want or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id in want for t in n.targets)]
assert len(nodes)==len(want), (len(nodes),len(want))
exec(compile(ast.Module(body=nodes,type_ignores=[]),str(source),'exec'),globals())
REG=[{'name':'Mama','kind':'food','bounds':[.45,.4,.25,.3]}]
def active(t): return {v['sample_index']:k for k,v in t.items() if v['visible']}
def step(t,stamp,points,regions=(),module='pet'):
    return _wall_track(t,points,regions,stamp,module)
class TrackingTests(unittest.TestCase):
    def test_classes_and_camera_isolation(self):
        ds=[('Kedi',.4,.5),('Köpek',.4,.5)]
        t,_,_=step({},0,ds);u,_,_=step({},0,ds)
        self.assertEqual(len(t),2);self.assertFalse(set(t)&set(u))
        q,_,_=step(t,1,list(reversed(ds)));self.assertEqual(set(q),set(t));self.assertEqual(q[active(q)[0]]['label'],'Köpek')
    def test_crossing_same_class(self):
        t,_,_=step({},0,[('Kişi',.25,.5),('Kişi',.75,.5)],module='cafe');ids=active(t)
        for n,x in enumerate([.34,.43,.52,.61,.70],1):
            t,_,_=step(t,n,[('Kişi',1-x,.5),('Kişi',x,.5)],module='cafe')
        self.assertEqual(active(t),{0:ids[1],1:ids[0]})
    def test_debounce_entry_exit(self):
        t,_,_=step({},0,[('Kedi',.4,.5)],REG)
        t,_,e=step(t,1,[('Kedi',.5,.5)],REG);self.assertFalse(e)
        t,r,e=step(t,2,[('Kedi',.5,.5)],REG);self.assertEqual([x[1] for x in e],['Giriş']);self.assertEqual(r[0]['count'],1)
        t,r,e=step(t,3,[('Kedi',.5,.5)],REG);self.assertEqual(r[0]['seconds'],2)
        t,_,e=step(t,4,[('Kedi',.4,.5)],REG);self.assertFalse(e)
        t,r,e=step(t,5,[('Kedi',.4,.5)],REG);self.assertEqual([x[1] for x in e],['Çıkış']);self.assertEqual(e[0][-1],2);self.assertEqual(r[0]['count'],0)
    def test_missing_not_exit_or_dwell(self):
        t,_,_=step({},0,[('Kedi',.5,.5)],REG);t,_,_=step(t,1,[('Kedi',.5,.5)],REG);key=active(t)[0]
        t,r,e=step(t,2,[],REG);self.assertFalse(e);self.assertEqual(r[0]['count'],0)
        t,r,e=step(t,3,[('Kedi',.5,.5)],REG);self.assertFalse(e);self.assertEqual(active(t)[0],key);self.assertEqual(r[0]['seconds'],0)
        t,_,_=step(t,4,[],REG);t,r,e=step(t,5,[('Kedi',.4,.5)],REG);self.assertFalse(e);self.assertEqual(r[0]['count'],0)
    def test_gap_crossing_not_entry(self):
        t,_,_=step({},0,[('Kedi',.4,.5)],REG);t,_,_=step(t,1,[],REG)
        t,r,e=step(t,2,[('Kedi',.5,.5)],REG);self.assertFalse(e);self.assertEqual(r[0]['count'],0)
        t,r,e=step(t,3,[('Kedi',.5,.5)],REG);self.assertEqual(r[0]['count'],1);self.assertFalse(e)
    def test_long_gap_expires(self):
        t,_,_=step({},0,[('İnek',.5,.5)],module='farm');key=active(t)[0]
        t,_,e=step(t,6,[('İnek',.5,.5)],module='farm');self.assertNotIn(key,t);self.assertFalse(e)
    def test_stationary_latched_and_recovery(self):
        for module,label in [('pet','Kedi'),('farm','İnek')]:
            t={};events=[]
            for n in range(35):
                t,_,e=step(t,n,[(label,.5,.5)],module=module);events+=e
            self.assertEqual([x[1] for x in events],['Hareketsizlik adayı']);self.assertEqual(events[0][-1],30)
            t,_,e=step(t,35,[(label,.54,.5)],module=module);self.assertFalse(e)
            t,_,e=step(t,36,[(label,.56,.5)],module=module);self.assertEqual([x[1] for x in e],['Hareket yeniden gözlendi'])
    def test_missing_resets_stationary_and_no_cafe_alarm(self):
        t={};events=[]
        for n in range(60):
            t,_,e=step(t,n,[] if n==25 else [('Kedi',.5,.5)]);events+=e
        # 30 continuous seconds only after the camera sees the animal again.
        self.assertEqual(len(events),1)
        t={};events=[]
        for n in range(35):t,_,e=step(t,n,[('Kişi',.5,.5)],module='cafe');events+=e
        self.assertFalse(events)
    def test_invalid_data_no_mutation(self):
        t,_,_=step({},0,[('Kedi',.5,.5)]);before=repr(t)
        for x in [float('nan'),float('inf'),-1,2]:
            with self.assertRaises(ValueError):step(t,1,[('Kedi',x,.5)])
        self.assertEqual(repr(t),before)
    def test_auto_scene_observed_duration_excludes_gap(self):
        t={};scene={};events=[];det=[('Kedi',.5,.5)];boxes=[('Kedi',(.4,.4,.6,.6))]
        for n,visible in [(0,True),(1,True),(2,False),(3,True),(4,True),(10,False)]:
            d=det if visible else [];t,_,_=step(t,n,d)
            scene,_,e=_auto_scene(scene,[],boxes if visible else [],t,n,'pet');events+=e
        self.assertEqual([e[1] for e in events].count('İlk görülme'),1)
        end=[e for e in events if e[1]=='İz sona erdi'];self.assertEqual(len(end),1);self.assertEqual(end[0][-1],2)
    def test_report_storage_and_owner_isolation(self):
        global DATA_DIR
        with tempfile.TemporaryDirectory() as d:
            DATA_DIR=Path(d)
            camera={'module':'farm','owner':'aaa','id':'cam','config':{'auto':True}}
            _wall_record(camera,[('Hareket analizi','Hareketsizlik adayı','abc','İnek',30)],[('İnek',.5,.5)],30)
            with closing(_wall_db('farm','aaa')) as c:
                row=c.execute('select event,seconds from observations').fetchone();self.assertEqual(tuple(row),('Hareketsizlik adayı',30.))
            with closing(_wall_db('farm','bbb')) as c:self.assertEqual(c.execute('select count(*) from observations').fetchone()[0],0)
            self.assertEqual(_wall_insights('farm','aaa','cam')['daily'][-1]['samples'],1)
if __name__=='__main__':unittest.main(verbosity=2)
