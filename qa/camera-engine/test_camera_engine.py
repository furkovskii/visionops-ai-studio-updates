import ast, threading, time, sys, unittest, json, os, struct
from pathlib import Path
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
import numpy as np, cv2
source=Path(__file__).resolve().parents[2]/'camera-qa-build'/'VisionOps_AI_Studio'/'app.py'
tree=ast.parse(source.read_text(encoding='utf-8'))
want={'_NETWORK_CAPTURE_CHILD','_CameraCaptureError','_IsolatedNetworkCapture','_open_universal_camera','_private_ipv4','_safe_xml','_local_endpoint','_camera_public_status','_camera_diagnostics','_WALL_STALE_SECONDS','_WALL_OPEN_SECONDS','_ip_camera_worker','_clear_wall_frames','_CAM_PRIVACY_LOCK'}
nodes=[n for n in tree.body if getattr(n,'name',None) in want or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id in want for t in n.targets)]
assert len(nodes)==len(want)
exec(compile(ast.Module(body=nodes,type_ignores=[]),str(source),'exec'),globals())
FAKE="""
import sys,time,struct,json
config=json.loads(sys.stdin.buffer.readline())
assert config['source'].startswith('rtsp://tester:')
assert config['source'] not in ' '.join(sys.argv)
"""
URL='rtsp://tester:fake-secret@192.168.1.20:554/stream'
FRAME="sys.stdout.buffer.write(struct.pack('!II',8,10)+bytes([123])*240);sys.stdout.buffer.flush()\n"
class CameraEngineTests(unittest.TestCase):
    def test_stream_and_bounded_queue(self):
        p=_IsolatedNetworkCapture(URL,threading.Event(),child_code=FAKE+'\nwhile True:\n '+FRAME+' time.sleep(.01)\n',deadline_seconds=4)
        try:
            self.assertTrue(p.isOpened());self.assertEqual(p.initial.shape,(8,10,3));self.assertEqual(int(p.initial[0,0,0]),123)
            time.sleep(.15);self.assertLessEqual(p.frames.qsize(),1);self.assertTrue(p.read()[0]);self.assertNotIn('fake-secret',' '.join(p.proc.args))
        finally:p.release()
        self.assertIsNotNone(p.proc.poll());self.assertFalse(p.reader.is_alive())
    def test_stalled_open(self):
        now=time.monotonic()
        with self.assertRaisesRegex(_CameraCaptureError,'decoder_timeout'):_IsolatedNetworkCapture(URL,threading.Event(),child_code=FAKE+'\ntime.sleep(60)',deadline_seconds=.5)
        self.assertLess(time.monotonic()-now,4)
    def test_stalled_read_and_termination(self):
        p=_IsolatedNetworkCapture(URL,threading.Event(),child_code=FAKE+FRAME+'time.sleep(60)',deadline_seconds=4)
        p.timeout=.3
        try:
            with self.assertRaisesRegex(_CameraCaptureError,'decoder_timeout'):p.read()
        finally:p.release()
        self.assertIsNotNone(p.proc.poll());self.assertFalse(p.reader.is_alive())
    def test_bad_dimensions(self):
        with self.assertRaisesRegex(_CameraCaptureError,'decoder_exit'):_IsolatedNetworkCapture(URL,threading.Event(),child_code=FAKE+"sys.stdout.buffer.write(struct.pack('!II',20000,20000));sys.stdout.buffer.flush();time.sleep(60)",deadline_seconds=4)
    def test_cancel(self):
        stop=threading.Event();t=threading.Timer(.15,stop.set);t.start();start=time.monotonic()
        try:
            with self.assertRaisesRegex(_CameraCaptureError,'stopped'):_IsolatedNetworkCapture(URL,stop,child_code=FAKE+'time.sleep(60)',deadline_seconds=9)
        finally:t.cancel()
        self.assertLess(time.monotonic()-start,4)
    def test_decoder_exit(self):
        with self.assertRaisesRegex(_CameraCaptureError,'decoder_exit'):_IsolatedNetworkCapture(URL,threading.Event(),child_code=FAKE+'sys.exit(7)',deadline_seconds=4)
    def test_onvif_local_restrictions(self):
        self.assertEqual(_private_ipv4('192.168.1.42'),'192.168.1.42')
        for ip in ['8.8.8.8','127.0.0.1','169.254.169.254']:
            with self.assertRaises(ValueError):_private_ipv4(ip)
        for url in ['http://192.168.1.43/a','http://a:b@192.168.1.42/a','file:///tmp/a']:
            with self.assertRaises(ValueError):_local_endpoint(url,'192.168.1.42')
        with self.assertRaises(ValueError):_safe_xml(b'<!DOCTYPE x><x/>')
    def test_stale_status(self):
        c={'status':'Canlı','last_frame_at':1,'frame':b'0','protocol':'rtsp','decoder_timeouts':2};self.assertEqual(_camera_public_status(c,14),'Görüntü gecikiyor');self.assertEqual(_camera_public_status(c,10),'Canlı');self.assertEqual(_camera_diagnostics(c,10)['decoder_timeouts'],2)
    def test_worker_automatic_reconnect_and_stop(self):
        from unittest.mock import patch
        stop=threading.Event(); opens=[]; alerts=[]; ready=threading.Event()
        class Driver:
            def __init__(self,first):self.first=first;self.closed=False;self.n=0
            def isOpened(self):return True
            def read(self):
                self.n+=1
                if self.first and self.n>=5:raise _CameraCaptureError('decoder_timeout')
                time.sleep(.13);return True,np.zeros((24,32,3),np.uint8)
            def release(self):self.closed=True
        def opener(protocol,source,stop_event):
            driver=Driver(not opens);opens.append(driver);return driver,None
        def alert(c,live):
            alerts.append(live);c['alert_outage']=not live
            if live:ready.set()
        cam={'stop':stop,'protocol':'rtsp','source':URL,'analysis':False,'status':'Bağlanıyor'}
        with patch.dict(globals(),{'_open_universal_camera':opener,'redact_faces':lambda f,p:f,'_wall_connection_alert':alert}):
            worker=threading.Thread(target=_ip_camera_worker,args=(cam,),daemon=True);cam['worker']=worker;worker.start()
            try:
                self.assertTrue(ready.wait(8),'worker never reconnected')
                self.assertGreaterEqual(cam['reconnects'],1);self.assertEqual(cam['decoder_timeouts'],1);self.assertEqual(alerts[:2],[False,True]);self.assertIsNotNone(cam['frame'])
            finally:stop.set();worker.join(3)
        self.assertFalse(worker.is_alive());self.assertEqual(cam['status'],'Durduruldu');self.assertIsNone(cam['frame']);self.assertTrue(all(d.closed for d in opens))
    def test_real_four_mjpeg_decoders(self):
        class Handler(BaseHTTPRequestHandler):
            def log_message(self,*args):pass
            def do_GET(self):
                self.send_response(200);self.send_header('Content-Type','multipart/x-mixed-replace; boundary=frame');self.end_headers()
                try:
                    for i in range(400):
                        f=np.full((64,96,3),(i*7)%240,np.uint8);ok,j=cv2.imencode('.jpg',f);blob=j.tobytes();self.wfile.write(b'--frame\r\nContent-Type: image/jpeg\r\nContent-Length: '+str(len(blob)).encode()+b'\r\n\r\n'+blob+b'\r\n');self.wfile.flush();time.sleep(.04)
                except (OSError,ConnectionError):pass
        srv=ThreadingHTTPServer(('127.0.0.1',0),Handler);srv.daemon_threads=True
        thread=threading.Thread(target=srv.serve_forever,daemon=True);thread.start(); captures=[]; errors=[];lock=threading.Lock()
        def connect(i):
            try:
                c=_IsolatedNetworkCapture(f'http://127.0.0.1:{srv.server_port}/camera{i}',threading.Event(),deadline_seconds=9)
                with lock:captures.append(c)
            except Exception as exc:
                with lock:errors.append(str(exc))
        threads=[threading.Thread(target=connect,args=(i,)) for i in range(4)]
        try:
            for t in threads:t.start()
            for t in threads:t.join(12)
            self.assertFalse(errors,errors);self.assertEqual(len(captures),4)
            for c in captures:self.assertEqual(c.initial.shape,(64,96,3));self.assertTrue(c.read()[0])
            c=captures[0];c.proc.kill();c.proc.wait();self.assertTrue(c.done.wait(2))
            with self.assertRaisesRegex(_CameraCaptureError,'decoder_exit'):c.read()
            for c in captures[1:]:self.assertTrue(c.read()[0])
            c=captures[0];c.release();replacement=_IsolatedNetworkCapture(f'http://127.0.0.1:{srv.server_port}/replacement',threading.Event());captures.append(replacement);self.assertTrue(replacement.isOpened())
        finally:
            for c in captures:c.release()
            srv.shutdown();srv.server_close();thread.join(2)
        self.assertTrue(all(c.proc.poll() is not None and not c.reader.is_alive() for c in captures))
    @unittest.skipUnless(os.name=='nt','Windows launcher only')
    def test_pythonw_piped_decoder(self):
        from unittest.mock import patch
        exe=Path(sys.executable).with_name('pythonw.exe')
        self.assertTrue(exe.is_file())
        with patch.object(sys,'executable',str(exe)):
            self.test_stream_and_bounded_queue()
if __name__=='__main__':unittest.main(verbosity=2)
