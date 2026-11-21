import importlib.util, math, pathlib, sys, unittest
from dataclasses import asdict
P=pathlib.Path(__file__).with_name("sceneflow.py")
spec=importlib.util.spec_from_file_location("sceneflow",P); m=importlib.util.module_from_spec(spec);sys.modules[spec.name]=m;spec.loader.exec_module(m)
R,E,C=m.Rule,m.Event,m.Coverage
def rule(mod="cafe",**kw):
    v=dict(rule_id="queue_exit",module=mod,camera_id="cam01",zone_id="queue",subject="person" if mod=="cafe" else "animal",event="zone_exit",condition="at_least",threshold=2,window_seconds=120,minimum_coverage=.9)
    v.update(kw);return R.from_dict(v)
def ev(t,**kw):
    v=dict(timestamp=t,camera_id="cam01",module="cafe",zone_id="queue",subject="person",kind="zone_exit");v.update(kw);return E(**v)
def cov(a,b,**kw):
    v=dict(start=a,end=b,camera_id="cam01",usable=True);v.update(kw);return C(**v)
class SceneFlowTests(unittest.TestCase):
    def test_valid_events_and_zone_filter(self):
        v=m.evaluate(rule(),[ev(10),ev(20),ev(70,zone_id="other"),ev(55,kind="zone_enter")],[cov(0,120)],120)
        self.assertEqual((v.verdict,v.observed_events),("triggered",2))
    def test_empty_good_window_is_not_triggered(self):
        self.assertEqual(m.evaluate(rule(),[],[cov(0,120)],120).verdict,"not_triggered")
    def test_missing_window_never_zero(self):
        v=m.evaluate(rule(condition="at_most",threshold=0),[],[cov(0,40)],120)
        self.assertEqual(v.verdict,"unknown")
        self.assertAlmostEqual(v.coverage_ratio,1/3,places=4)
    def test_overlapping_coverage_counts_once(self):
        v=m.evaluate(rule(),[ev(20),ev(45)],[cov(0,100),cov(30,120),cov(20,110)],120)
        self.assertEqual(v.covered_seconds,120)
        self.assertEqual(v.observed_events,2)
    def test_camera_outage_events_excluded(self):
        v=m.evaluate(rule(minimum_coverage=.5,threshold=1),[ev(22),ev(95)],[cov(0,70)],120)
        self.assertEqual((v.observed_events,v.verdict),(1,"triggered"))
    def test_time_bounds(self):
        v=m.evaluate(rule(threshold=1),[ev(0),ev(119.9),ev(120)],[cov(0,120)],120)
        self.assertEqual(v.observed_events,2)
    def test_pet_farm_deny_person(self):
        for mod in ("pet","farm"):
            with self.assertRaises(ValueError):rule(mod,subject="person")
            with self.assertRaises(ValueError):ev(10,module=mod,subject="person").validate()
            rule(mod).validate()
    def test_identity_and_arbitrary_code_fields_forbidden(self):
        for field in ("face_id","person_name","password","embedding","python_code","full_name"):
            data=asdict(rule());data[field]="secret"
            with self.assertRaises(ValueError):R.from_dict(data)
    def test_invalid_rule_and_identifier(self):
        for v in (dict(rule_id="../../etc"),dict(zone_id="with spaces"),dict(camera_id="8.8.8.8"),dict(threshold=True),dict(threshold=-1),dict(window_seconds=1),dict(window_seconds=100000),dict(minimum_coverage=.3),dict(minimum_coverage=float("nan")),dict(event="face_match"),dict(condition="eval"),dict(subject="animal")):
            with self.subTest(v=v),self.assertRaises(ValueError):rule(**v)
    def test_invalid_source_data(self):
        for v in (ev(float("nan")),ev(10,kind="face_match"),ev(10,camera_id="8.8.8.8")):
            with self.assertRaises(ValueError):v.validate()
        for v in (cov(0,math.inf),cov(10,0),cov(0,120,usable=1)):
            with self.assertRaises(ValueError):v.validate()
    def test_no_fabricated_calibration(self):
        with self.assertRaises(ValueError):m.evaluate(rule(),[],[cov(0,120)],120,validated_accuracy=.99)
        v=m.evaluate(rule(),[],[cov(0,120)],120)
        self.assertIsNone(v.validated_accuracy)
    def test_receipts_deterministic_and_aggregate(self):
        a=m.evaluate(rule(),[ev(12),ev(40)],[cov(0,120)],120)
        b=m.evaluate(rule(),[ev(40),ev(12)],[cov(0,120)],120)
        self.assertEqual(a.proof_sha256,b.proof_sha256)
        self.assertRegex(a.proof_sha256,"^[a-f0-9]{64}$")
        self.assertNotIn("person_id",str(a.public()))
    def test_experiment_normalized_not_causal(self):
        r=rule(threshold=1)
        before=m.evaluate(r,[ev(10),ev(20)],[cov(0,120)],120)
        after=m.evaluate(r,[ev(150)],[cov(120,240)],240)
        result=m.compare(before,after)
        self.assertEqual(result.relative_change,-50.)
        self.assertIn("nedensel",result.interpretation)
    def test_experiment_missing_evidence(self):
        r=rule()
        before=m.evaluate(r,[],[cov(0,60)],120)
        after=m.evaluate(r,[],[cov(120,240)],240)
        out=m.compare(before,after)
        self.assertFalse(out.enough_evidence)
        self.assertIsNone(out.relative_change)
    def test_zero_baseline_no_percent(self):
        r=rule()
        before=m.evaluate(r,[],[cov(0,120)],120)
        after=m.evaluate(r,[ev(150)],[cov(120,240)],240)
        self.assertIsNone(m.compare(before,after).relative_change)
    def test_incompatible_experiments_rejected(self):
        before=m.evaluate(rule(),[],[cov(0,120)],120)
        after=m.evaluate(rule(rule_id="other"),[],[cov(120,240)],240)
        with self.assertRaises(ValueError):m.compare(before,after)
    def test_overlapping_experiments_rejected(self):
        before=m.evaluate(rule(),[],[cov(0,120)],120)
        after=m.evaluate(rule(),[],[cov(90,210)],210)
        with self.assertRaises(ValueError):m.compare(before,after)
if __name__=="__main__":unittest.main(verbosity=2)
