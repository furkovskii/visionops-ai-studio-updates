import ast, json, sqlite3, tempfile, unittest, uuid
from pathlib import Path
from datetime import datetime,date,timedelta
from contextlib import closing

ROOT=Path(__file__).resolve().parents[2]
APP=ROOT/'smart-qa-build'/'VisionOps_AI_Studio'/'app.py'
wanted={'_smart_validate','_smart_step','_smart_heat','_smart_trend','_smart_report','_wall_rule_defaults','_wall_rules','_wall_db','_wall_alert','_wall_ai_alerts','_wall_record'}
nodes=[n for n in ast.parse(APP.read_text(encoding='utf-8')).body if getattr(n,'name',None) in wanted]
assert len(nodes)==len(wanted)
class CloudError(Exception):pass
exec(compile(ast.Module(body=nodes,type_ignores=[]),str(APP),'exec'),globals())
REG=[{'name':'Mama','kind':'food','bounds':[.4,.4,.2,.2]}]
def rule(**kwargs):
    return dict(name='Senaryo',kind='absence',zone='Mama',duration=10,threshold=2,cooldown=60,enabled=True,**{}) | kwargs
def frame(state,rules,stamp,points=(),rows=(),events=(),token=(1,1)):
    return _smart_step(state,rules,REG,rows,events,points,stamp,token)
def row(day,n,total,hour=10):
    return dict(day=day.isoformat(),hour=hour,n=n,total=total,heat=json.dumps([total]+[0]*95))
class SmartTests(unittest.TestCase):
    def test_validation_and_legacy_defaults(self):
        self.assertEqual(_wall_rules({},REG)['scenarios'],[])
        self.assertEqual(_wall_rules({'scenarios':[rule()]},REG)['scenarios'],[rule()])
        for bad in ([rule(duration=True)],[rule(duration=43201)],[rule(zone='Missing')],[rule(threshold=0)],[rule(cooldown=59)],[rule(name='')],[rule(),rule(name='senaryo')],[rule(name=str(i)) for i in range(9)]):
            with self.assertRaises(CloudError):_smart_validate(bad,REG)
    def test_observed_absence_and_latch(self):
        state={};alerts=[]
        for t in range(71):
            state,n=frame(state,[rule()],t);alerts+=n
        self.assertEqual(len(alerts),1);self.assertIn('Gerçek yokluk kanıtı değildir',alerts[0][1])
        state,n=frame(state,[rule()],71,[('Kedi',.5,.5)]);self.assertFalse(n)
        for t in range(72,83):state,n=frame(state,[rule()],t);alerts+=n
        self.assertEqual(len(alerts),2)
    def test_gap_and_reconfiguration_reset(self):
        state={};alerts=[]
        for t in list(range(6))+list(range(100,110)):
            state,n=frame(state,[rule()],t);alerts+=n
        self.assertFalse(alerts)
        state,n=frame(state,[rule()],110,token=(1,2));self.assertFalse(n)
        state,n=frame(state,[rule()],110,token=(1,2));self.assertFalse(n)
        state,n=frame(state,[rule(duration=20)],111,token=(1,2));self.assertFalse(n)
    def test_no_cross_camera_or_disabled_rules(self):
        s1={};s2={}
        for t in range(11):s1,n=frame(s1,[rule()],t)
        self.assertEqual(len(n),1)
        s2,n=frame(s2,[rule()],10);self.assertFalse(n)
        for t in range(11,25):s2,n=frame(s2,[rule(enabled=False)],t)
        self.assertFalse(n)
    def test_independent_scenarios_and_dwell(self):
        rules=[rule(name='Kalabalık',kind='count',zone='',duration=10),rule(name='Kalış',kind='dwell')]
        s={};out=[]
        for t in range(11):
            s,n=frame(s,rules,t,[('Kedi',.1,.1),('Kedi',.8,.8)],rows=[{'name':'Mama','count':1,'seconds':t}]);out+=n
        self.assertEqual(len(out),2)
    def test_entry_uses_observed_event_and_cooldown(self):
        r=rule(kind='entry');s,n=frame({},[r],0,events=[('Mama','Giriş','abc','Kedi',None)]);self.assertEqual(len(n),1)
        s,n=frame(s,[r],1);self.assertFalse(n)
        s,n=frame(s,[r],2,events=[('Mama','Giriş','def','Kedi',None)]);self.assertFalse(n)
        s,n=frame(s,[r],63,events=[('Mama','Giriş','ghi','Kedi',None)]);self.assertEqual(len(n),1)
    def test_heat_rates_missing_vs_zero_and_range(self):
        today=date(2026,11,20);rs=[row(today,10,20),row(today-timedelta(days=6),10,0),row(today-timedelta(days=29),20,40),row(today-timedelta(days=30),10,90),row(today+timedelta(days=1),1,5)]
        heat=_smart_heat(rs,today)
        self.assertEqual(heat['1']['rates'][0],2);self.assertEqual(heat['7']['rates'][0],1);self.assertEqual(heat['30']['rates'][0],1.5)
        self.assertEqual(heat['7']['covered_days'],2);self.assertEqual(_smart_heat([],today)['1']['rates'],[None]*96)
        self.assertEqual(_smart_heat([row(today,10,0)],today)['1']['rates'],[0]*96)
        self.assertEqual(_smart_heat([dict(row(today,3,1),heat='[99]')],today)['1']['samples'],0)
    def test_trend_uses_same_completed_hour_and_coverage(self):
        today=date(2026,11,20)
        rs=[row(today-timedelta(days=i),20,40) for i in range(1,8)]+[row(today,20,20),row(today,500,50000,11)]
        t=_smart_trend(rs,today,11);self.assertEqual(t['status'],'lower');self.assertEqual(t['percent'],-50);self.assertEqual(t['baseline_days'],7)
        self.assertEqual(_smart_trend(rs[:4]+[row(today,20,20)],today,11)['status'],'insufficient')
        self.assertEqual(_smart_trend(rs,today,0)['status'],'insufficient')
        zero=[row(today-timedelta(days=i),20,0) for i in range(1,6)]+[row(today,20,10)]
        t=_smart_trend(zero,today,11);self.assertEqual(t['status'],'higher');self.assertIsNone(t['percent'])
    def test_integrated_alert_persistence_and_isolated_heat(self):
        global DATA_DIR
        with tempfile.TemporaryDirectory() as d:
            DATA_DIR=Path(d);cam={'module':'pet','owner':'a','id':'cam','name':'Test','config':{'regions':REG,'rules':_wall_rules({'scenarios':[rule()]},REG)}}
            for t in range(11):_wall_ai_alerts(cam,[],[],[],t,(1,1))
            with closing(_wall_db('pet','a')) as c:self.assertEqual(c.execute("SELECT COUNT(*) FROM alerts WHERE kind='smart'").fetchone()[0],1)
            _wall_record(cam,[],[('Kedi',.5,.5)],10)
            self.assertEqual(_smart_report('pet','a','cam')['heatmaps']['1']['samples'],1)
            self.assertEqual(_smart_report('pet','b','cam')['heatmaps']['1']['samples'],0)
            self.assertEqual(_smart_report('pet','a','different')['heatmaps']['1']['samples'],0)
    def test_ui_hooks_present_in_both_packages(self):
        t=(APP.parent/'templates'/'activate.html').read_text(encoding='utf-8')
        for x in ['id="vo-smart-list"','id="vo-heat-period"','id="vo-smart-trend"','r.scenarios=smartRead()','renderSmartReport(data);','X-CSRF-Token']:
            self.assertIn(x,t)
if __name__=='__main__':unittest.main(verbosity=2)
