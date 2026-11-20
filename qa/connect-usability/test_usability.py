"""v0.13.5 camera scanner + embedded onboarding regression, no physical camera."""
import ast,json,os,socket,subprocess,threading,unittest,zipfile
from pathlib import Path
from unittest import mock
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[2];BASE=ROOT/'connect-usability-build'
SOURCE=BASE/'VisionOps_AI_Studio'/'app.py'
TREE=ast.parse(SOURCE.read_text(encoding='utf-8'))
WANT={'_vo_interfaces','_discover_onvif','_vo_scan_rtsp','visionops_connect_scan'}
NODES=[n for n in TREE.body if getattr(n,'name',None) in WANT]
assert len(NODES)==len(WANT)
for node in NODES:
    if isinstance(node,ast.FunctionDef):node.decorator_list=[]
exec(compile(ast.Module(body=NODES,type_ignores=[]),str(SOURCE),'exec'),globals())
class CloudError(Exception):pass
_VO_FALLBACK_LOCK=threading.Lock()
class Response:
    def __init__(self,**payload):self.payload=payload;self.headers={}
def jsonify(**kwargs):return Response(**kwargs)
class LocalUsabilityTests(unittest.TestCase):
    def test_default_one_click_scan_invokes_both_discovery_modes(self):
        calls=[]
        def onvif(*,interfaces):
            calls.append(('onvif',interfaces))
            return [{'ip':'192.168.10.21','port':80,'path':'/onvif','scheme':'http'}]
        def rtsp(*,interfaces):
            calls.append(('rtsp',interfaces))
            return [{'ip':'192.168.10.21','port':554,'scheme':'rtsp'},
                    {'ip':'192.168.10.22','port':8554,'scheme':'rtsp'}]
        scope={'session':{'active_module':'cafe'},'_ip_cameras':{'cafe':{}},
               '_wall_authorized':lambda:True,'_wall_owner':lambda:'customer',
               'request':SimpleNamespace(get_json=lambda silent: {}),
               '_vo_rate_limit':lambda *args:None,
               '_vo_interfaces':lambda:['192.168.10.5'],'_discover_onvif':onvif,'_vo_scan_rtsp':rtsp}
        with mock.patch.dict(globals(),scope):
            response=visionops_connect_scan()
        self.assertEqual(len(response.payload['devices']),2)
        self.assertEqual(response.payload['devices'][1]['port'],8554)
        self.assertEqual([x[0] for x in calls],['onvif','rtsp'])
        self.assertEqual(calls[1][1],['192.168.10.5'])
        self.assertEqual(response.headers['Cache-Control'],'no-store')
    def test_scan_requires_workspace_authorization(self):
        scope={'session':{'active_module':'cafe'},'_ip_cameras':{'cafe':{}},
               '_wall_authorized':lambda:False}
        with mock.patch.dict(globals(),scope):
            with self.assertRaises(CloudError):visionops_connect_scan()
    def test_scan_does_not_accept_invalid_options(self):
        scope={'session':{'active_module':'cafe'},'_ip_cameras':{'cafe':{}},
               '_wall_authorized':lambda:True,
               'request':SimpleNamespace(get_json=lambda silent:{'extended':'yes'})}
        with mock.patch.dict(globals(),scope):
            with self.assertRaises(CloudError):visionops_connect_scan()
    def test_scanner_local_network_and_ports(self):
        calls=[]
        def check(ip,port,timeout):
            calls.append((ip,port));return ip=='192.168.55.16' and port==8554
        with mock.patch.dict(globals(),{'_vo_probe_rtsp':check}):
            found=_vo_scan_rtsp(interfaces=['192.168.55.5'])
        self.assertEqual(len(found),1)
        self.assertEqual(found[0]['port'],8554)
        self.assertTrue(all(ip.startswith('192.168.55.') and port in (554,8554) for ip,port in calls))
        self.assertLessEqual(len(calls),508)
    def test_windows_console_is_hidden_by_design(self):
        src=SOURCE.read_text(encoding='utf-8')
        section=src[src.index('def _vo_interfaces():'):src.index('def _vo_parse_probe(')]
        for marker in ('subprocess.STARTUPINFO()','subprocess.STARTF_USESHOWWINDOW',
                       "getattr(subprocess,'CREATE_NO_WINDOW',0)",'subprocess.DEVNULL'):
            self.assertIn(marker,section)
        if os.name=='nt':
            with mock.patch('subprocess.run',return_value=SimpleNamespace(stdout=b'[]')) as command:
                _vo_interfaces()
            self.assertTrue(command.called)
            self.assertNotEqual(command.call_args.kwargs['creationflags'] & subprocess.CREATE_NO_WINDOW,0)
            self.assertTrue(command.call_args.kwargs['startupinfo'].dwFlags & subprocess.STARTF_USESHOWWINDOW)
    def test_wizard_empty_state_and_inline_assistant(self):
        markup=(SOURCE.parent/'templates'/'activate.html').read_text(encoding='utf-8')
        for required in ['id="vo-scan-progress"','class="vo-radar"','id="vo-connect-fallback"',
                         'id="vo-help-brand"','id="vo-help-question"','id="vo-help-ask"',
                         'id="vo-connect-manual-slot"',"'/api/workspace/connect/scan',{extended:true}",
                         'id="vo-scan-retry"','vo-help-card']:
            self.assertIn(required,markup)
        self.assertNotIn('vo-connect-extended',markup)
        self.assertNotIn("window.open(",markup[markup.index('// VisionOps Connect wizard.'):])
    def test_old_manual_controls_not_removed(self):
        markup=(SOURCE.parent/'templates'/'activate.html').read_text(encoding='utf-8')
        self.assertIn('<details class="wall-add">',markup)
        self.assertIn('id="ip-form"',markup)
        self.assertIn('slot.append(legacy)',markup)
    def test_zip_matches_safe_update_and_setup(self):
        name='VisionOps_AI_Studio/'
        with zipfile.ZipFile(BASE/'VisionOps_AI_Studio_update.zip') as u,zipfile.ZipFile(BASE/'VisionOps_AI_Studio_Setup.zip') as s:
            self.assertEqual(json.loads(u.read(name+'version.json'))['version'],'0.13.5')
            for i in u.namelist():self.assertEqual(u.read(i),s.read(i),i)
if __name__=='__main__':unittest.main(verbosity=2)
