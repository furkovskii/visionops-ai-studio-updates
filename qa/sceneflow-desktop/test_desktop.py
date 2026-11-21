import unittest,zipfile,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'sceneflow-desktop-build'
class SceneFlowDesktopTests(unittest.TestCase):
 def test_camera_wizard_and_privacy_behavior_remain(self):
  with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as z:
   text=z.read('VisionOps_AI_Studio/templates/activate.html').decode('utf-8')
   for marker in ('id="vo-scene"','id="vo-scene-template"','id="vo-connect"','id="vo-connect-scan"',
                  'Kamera bağlantısına git','Gözlem denetimi','Gerçek kamera ile ölçüm doğruluğu'):
    self.assertIn(marker,text)
   self.assertEqual(text.count('id="vo-scene"'),1)
   self.assertEqual(text.count('id="vo-connect"'),1)
   self.assertIn('const guide={',text)
   self.assertIn('id="vo-help-brand"',text)
 def test_no_new_files_or_unverified_data_upload(self):
  with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as p,zipfile.ZipFile(ROOT/'brands-restart-build'/'VisionOps_AI_Studio_update.zip') as base:
   self.assertEqual(set(p.namelist()),set(base.namelist()))
   for name in base.namelist():
    if name not in ('VisionOps_AI_Studio/templates/activate.html','VisionOps_AI_Studio/version.json'):
     self.assertEqual(p.read(name),base.read(name),name)
   text=p.read('VisionOps_AI_Studio/templates/activate.html').decode('utf-8')
   self.assertIn('Visual SceneFlow desktop preview only; no camera stream, PII, storage or external calls.',text)
 def test_update_installer_equal_and_versioned(self):
  with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as a,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as b:
   self.assertEqual(json.loads(a.read('VisionOps_AI_Studio/version.json'))['version'],'0.13.7')
   for name in a.namelist():self.assertEqual(a.read(name),b.read(name))
if __name__=="__main__":unittest.main(verbosity=2)
