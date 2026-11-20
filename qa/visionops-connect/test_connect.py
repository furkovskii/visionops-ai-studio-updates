"""Offline safety and compatibility tests. Real devices are a separate release gate."""
import ast,json,os,socket,threading,unittest,uuid,xml.etree.ElementTree as ET,zipfile
from unittest import mock
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'connect-qa-build'/'VisionOps_AI_Studio'/'app.py'
TREE=ast.parse(SOURCE.read_text(encoding='utf-8'))
NAMES={'_private_ipv4','_safe_xml','_local_endpoint','_vo_interfaces','_vo_parse_probe',
       '_discover_onvif','_vo_probe_rtsp','_vo_scan_rtsp','_vo_connect_check',
       '_vo_media2_profiles','_vo_media2_source','_onvif_source','_camera_source','_vo_rate_limit'}
NODES=[node for node in TREE.body if getattr(node,'name',None) in NAMES]
assert len(NODES)==len(NAMES),(len(NODES),len(NAMES))
class CloudError(Exception):pass
exec(compile(ast.Module(body=NODES,type_ignores=[]),str(SOURCE),'exec'),globals())
_DISCOVERY_LOCK=threading.Lock()
_VO_RATE_LOCK=threading.Lock()
_VO_RATE={}
def probe_xml(mid,peer='192.168.2.15',host=None):
    host=host or peer
    return (f'<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" '
            f'xmlns:a="http://schemas.xmlsoap.org/ws/2004/08/addressing" '
            f'xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery">'
            f'<s:Header><a:RelatesTo>{mid}</a:RelatesTo></s:Header>'
            f'<s:Body><d:ProbeMatches><d:ProbeMatch><d:XAddrs>'
            f'http://{host}:8899/onvif/device_service'
            f'</d:XAddrs></d:ProbeMatch></d:ProbeMatches></s:Body></s:Envelope>').encode()
class FakeTcp:
    def __init__(self,result=b'RTSP/1.0 401 Unauthorized\r\n'):self.result=result;self.sent=b''
    def __enter__(self):return self
    def __exit__(self,*a):return False
    def settimeout(self,*a):pass
    def sendall(self,data):self.sent=data
    def recv(self,n):return self.result

class ConnectTests(unittest.TestCase):
    def test_safe_discovery_reply(self):
        data=_vo_parse_probe(probe_xml('urn:uuid:123'),'192.168.2.15','urn:uuid:123')
        self.assertEqual(data[0]['ip'],'192.168.2.15')
        self.assertEqual(data[0]['port'],8899)
        self.assertEqual(data[0]['discovered_via'],'onvif')
        self.assertEqual(_vo_parse_probe(probe_xml('urn:uuid:123'),'192.168.2.15','wrong'),[])
    def test_prevent_cross_host_and_redirection(self):
        for host in ['192.168.2.99','127.0.0.1','evil.example']:
            self.assertEqual(_vo_parse_probe(probe_xml('a',host=host),'192.168.2.15','a'),[])
    def test_reject_xml_entities_and_public_hosts(self):
        xml=b'<!DOCTYPE x [<!ENTITY e SYSTEM "file:///etc/passwd">]><x>&e;</x>'
        self.assertEqual(_vo_parse_probe(xml,'192.168.2.15','x'),[])
        self.assertEqual(_vo_parse_probe(probe_xml('a',peer='8.8.8.8'),'8.8.8.8','a'),[])
        for ip in ['127.0.0.1','169.254.1.1','8.8.8.8']:
            with self.assertRaises(ValueError):_private_ipv4(ip)
    def test_rtsp_signature_and_no_password_attempt(self):
        tcp=FakeTcp()
        with mock.patch('socket.create_connection',return_value=tcp):
            self.assertTrue(_vo_probe_rtsp('192.168.2.12',554))
        self.assertIn(b'OPTIONS ',tcp.sent)
        self.assertNotIn(b'Authorization:',tcp.sent)
        with mock.patch('socket.create_connection',return_value=FakeTcp(b'HTTP/1.1 200 OK')):
            self.assertFalse(_vo_probe_rtsp('192.168.2.12',80))
    def test_rtsp_outside_private_denied_without_connect(self):
        with mock.patch('socket.create_connection') as conn:
            self.assertFalse(_vo_probe_rtsp('8.8.8.8',554))
            conn.assert_not_called()
    def test_scanner_only_local_24_and_known_port(self):
        calls=[]
        def fake(ip,port,timeout):
            calls.append((ip,port))
            return ip=='192.168.19.20'
        with mock.patch.dict(globals(),{'_vo_interfaces':lambda:['192.168.19.5'],'_vo_probe_rtsp':fake}):
            devices=_vo_scan_rtsp()
        self.assertEqual(len(devices),1);self.assertEqual(devices[0]['ip'],'192.168.19.20')
        self.assertTrue(all(ip.startswith('192.168.19.') and port==554 for ip,port in calls))
        self.assertLessEqual(len(calls),254)
    def test_check_denies_external_ip(self):
        with mock.patch('socket.create_connection') as conn:
            with self.assertRaises(CloudError):_vo_connect_check({'ip':'9.9.9.9','protocol':'rtsp'})
            conn.assert_not_called()
    def test_onvif_check_returns_media1_without_claiming_video(self):
        profiles=[{'name':'Sub stream','token':'p1'},{'name':'Main stream','token':'p2'}]
        with mock.patch('socket.create_connection',return_value=FakeTcp()),mock.patch.dict(globals(),{'_onvif_profiles':lambda form:('192.168.19.2','http://192.168.19.2','u','p',profiles)}):
            r=_vo_connect_check({'ip':'192.168.19.2','port':80,'protocol':'onvif'})
        self.assertTrue(r['reachable']);self.assertFalse(r['stream_verified'])
        self.assertEqual(len(r['profiles']),2);self.assertEqual(r['profiles'][0]['mode'],'media1')
    def test_media2_profiles_without_media1(self):
        def soap(url,ip,action,body,user,password):
            if action.endswith('GetServices'):
                return ET.fromstring('<root><Service><Namespace>http://www.onvif.org/ver20/media/wsdl</Namespace><XAddr>http://192.168.19.2/onvif/media2</XAddr></Service></root>')
            if action.endswith('GetProfiles'):
                self.assertIn('<tr2:GetProfiles/>',body)
                return ET.fromstring('<root><Profiles token="channel_1"><Name>Channel 1</Name></Profiles><Profiles token="channel_2"><Name>Channel 2</Name></Profiles></root>')
            if action.endswith('GetStreamUri'):
                self.assertIn('channel_2',body)
                return ET.fromstring('<root><Uri>rtsp://192.168.19.2:554/Streaming/Channels/201</Uri></root>')
            raise AssertionError(action)
        form={'ip':'192.168.19.2','port':80,'onvif_scheme':'http','path':'/onvif/device_service','username':'u','password':'p','profile':'channel_2','onvif_mode':'media2'}
        with mock.patch.dict(globals(),{'_onvif_soap':soap}):
            out=_vo_media2_profiles(form)
            self.assertEqual(len(out[-1]),2);self.assertEqual(out[-1][1]['mode'],'media2')
            protocol,source=_onvif_source(form)
            self.assertEqual(protocol,'onvif')
            self.assertIn('/Streaming/Channels/201',source)
    def test_media2_rejects_remote_stream_host(self):
        def soap(url,ip,action,body,user,password):
            if action.endswith('GetServices'):return ET.fromstring('<root><Service><Namespace>http://www.onvif.org/ver20/media/wsdl</Namespace><XAddr>http://192.168.19.2/onvif/media2</XAddr></Service></root>')
            if action.endswith('GetProfiles'):return ET.fromstring('<root><Profiles token="abc"/></root>')
            return ET.fromstring('<root><Uri>rtsp://10.10.10.10/stream</Uri></root>')
        form={'ip':'192.168.19.2','onvif_mode':'media2','profile':'abc'}
        with mock.patch.dict(globals(),{'_onvif_soap':soap}):
            with self.assertRaises(CloudError):_onvif_source(form)
    def test_media2_check_fallback(self):
        forms=[{'token':'ch1','name':'Channel 1','mode':'media2'}]
        def fail(form):raise ValueError('Media1 absent')
        with mock.patch('socket.create_connection',return_value=FakeTcp()),mock.patch.dict(globals(),{
            '_onvif_profiles':fail,'_vo_media2_profiles':lambda form:('192.168.19.2','media','u','p',forms)}):
            result=_vo_connect_check({'ip':'192.168.19.2','port':80,'protocol':'onvif'})
        self.assertEqual(result['profiles'][0]['mode'],'media2')
    def test_no_profile_claims_stream_verified(self):
        with mock.patch('socket.create_connection',return_value=FakeTcp()),mock.patch.dict(globals(),{'_vo_probe_rtsp':lambda *args:True}):
            out=_vo_connect_check({'ip':'192.168.19.2','protocol':'rtsp'})
        self.assertFalse(out['stream_verified']);self.assertEqual(out['code'],'rtsp')
    def test_discovery_uses_each_local_adapter(self):
        import select,time
        sockets=[]
        class DummyUdp:
            def __init__(self,*args):self.bound=None;self.packet=None;self.responded=False;sockets.append(self)
            def __enter__(self):return self
            def __exit__(self,*args):self.close()
            def setsockopt(self,*args):pass
            def bind(self,where):self.bound=where[0]
            def sendto(self,packet,destination):
                self.packet=packet
                self.assert_destination=destination
            def recvfrom(self,limit):
                self.responded=True
                mid=ET.fromstring(self.packet).findtext('.//{*}MessageID')
                ip='192.168.'+('11' if self.bound.endswith('.2') else '12')+'.35'
                return probe_xml(mid,peer=ip),(ip,3702)
            def close(self):pass
        clock=[0.]
        def tick():clock[0]+=.15;return clock[0]
        with mock.patch.dict(globals(),{'_vo_interfaces':lambda:['192.168.11.2','192.168.12.2']}),\
             mock.patch('socket.socket',side_effect=DummyUdp),\
             mock.patch('select.select',side_effect=lambda items,*args:([x for x in items if not x.responded],[],[])),\
             mock.patch('time.monotonic',side_effect=tick):
            found=_discover_onvif(timeout=1)
        self.assertEqual(len(sockets),2)
        self.assertTrue(all(s.assert_destination==('239.255.255.250',3702) for s in sockets))
        self.assertEqual(len(found),2)
    def test_rate_limit_is_bound_to_owner_and_window(self):
        _VO_RATE.clear()
        with mock.patch('time.monotonic',return_value=20):
            for _ in range(3):_vo_rate_limit('owner-a','check',60,3)
            with self.assertRaises(CloudError):_vo_rate_limit('owner-a','check',60,3)
            _vo_rate_limit('owner-b','check',60,3)
        with mock.patch('time.monotonic',return_value=81):
            _vo_rate_limit('owner-a','check',60,3)
    def test_route_scope_guards(self):
        for func in ['visionops_connect_scan','visionops_connect_check']:
            method=next(n for n in TREE.body if isinstance(n,ast.FunctionDef) and n.name==func)
            self.assertIn('_wall_authorized',[n.id for n in ast.walk(method) if isinstance(n,ast.Name)])
            self.assertIn('session',[n.id for n in ast.walk(method) if isinstance(n,ast.Name)])
    def test_wizard_ui_safety_and_fallback(self):
        t=(SOURCE.parent/'templates'/'activate.html').read_text(encoding='utf-8')
        for word in ['id="vo-connect-scan"','id="vo-connect-extended"','id="vo-connect-form"','id="vo-connect-pass"','/api/workspace/connect/check','/api/workspace/connect/scan','X-CSRF-Token','profile.selectedOptions[0]?.dataset.mode','window.addEventListener(\'pagehide\'']:
            self.assertIn(word,t)
        self.assertNotIn('__VO_CSRF_VALUE__',t)
        self.assertIn('<details class="wall-add">',t)
    def test_package_matches_update_and_installer(self):
        prefix='VisionOps_AI_Studio/'
        with zipfile.ZipFile(ROOT/'connect-qa-build'/'VisionOps_AI_Studio_update.zip') as a,zipfile.ZipFile(ROOT/'connect-qa-build'/'VisionOps_AI_Studio_Setup.zip') as b:
            self.assertEqual(json.loads(a.read(prefix+'version.json'))['version'],'0.13.4')
            self.assertEqual(a.read(prefix+'app.py'),b.read(prefix+'app.py'))
            self.assertEqual(a.read(prefix+'templates/activate.html'),b.read(prefix+'templates/activate.html'))
if __name__=='__main__':unittest.main(verbosity=2)
