"""Build the exact locally tested Camera Engine 0.13.1 from v0.13.0 release inputs."""
import ast, hashlib, json, zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).resolve().parent
OUT=ROOT/'camera-qa-build'
OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/'
EXPECTED={'app.py':'ef25c3d228efcd5a998ead252301ffacb587b0b7e31d129d23197643731b4a6f','templates/activate.html':'06648d1c0e15943c7f965554f0991e6c5e2e58c5e98237c4e15e5fbdde8f5161','version.json':'8c44983f143c7c1184acce05f6258b57a29c88ffbd75ffb2467ebf4634423154'}
def one(s,a,b):
    assert s.count(a)==1, 'Unexpected baseline: '+a[:80]
    return s.replace(a,b)
def normalize(a):return a.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
def change(rel, data):
    if rel not in EXPECTED:return data
    s=normalize(data)
    if rel=='app.py':
        a=s.index('def _open_universal_camera(protocol, source):')
        b=s.index('\n\n# ---- Shared live camera inference',a)
        s=s[:a]+(HERE/'capture.py.inc').read_text(encoding='utf-8')+s[b:]
        s=one(s,"'worker_alive':bool(camera.get('worker') and camera['worker'].is_alive())}","'worker_alive':bool(camera.get('worker') and camera['worker'].is_alive()),\n            'decoder_isolated':camera.get('protocol')!='usb',\n            'decoder_timeouts':camera.get('decoder_timeouts',0),\n            'decoder_exits':camera.get('decoder_exits',0)}")
        s=one(s,"_open_universal_camera(camera['protocol'], camera['source'])","_open_universal_camera(camera['protocol'], camera['source'], stop)")
        s=one(s,"            except Exception:\n                # Do not propagate URL/credentials to status or browser logs.","            except Exception as exc:\n                # Do not propagate URL/credentials to status or browser logs.\n                if isinstance(exc,_CameraCaptureError):\n                    if exc.reason=='decoder_timeout':camera['decoder_timeouts']=camera.get('decoder_timeouts',0)+1\n                    elif exc.reason=='decoder_exit':camera['decoder_exits']=camera.get('decoder_exits',0)+1")
    elif rel=='templates/activate.html':
        s=one(s,"'Yeniden bağlantı: '+d.reconnects,...","'Yeniden bağlantı: '+d.reconnects,...(d.decoder_isolated?['İzole ağ motoru','Zaman aşımı: '+(d.decoder_timeouts??0)]:['USB sürücüsü']),...")
        s=one(s,"(c.reconnects??0)+(c.retry_","(c.reconnects??0)+(c.decoder_isolated?' · İzole motor · Zaman aşımı: '+(c.decoder_timeouts??0):' · USB sürücüsü')+(c.retry_")
    else:s='{"version": "0.13.1"}\n'
    result=s.encode('utf-8')
    assert hashlib.sha256(result).hexdigest()==EXPECTED[rel], (rel,hashlib.sha256(result).hexdigest())
    if rel=='app.py':
        # Keep the previous exact-candidate hash gate above. Apply one narrowly
        # scoped runtime fix only after that gate, not a test-only workaround.
        original = """def parent_watch():
    try: sys.stdin.buffer.read(1)
    finally: os._exit(0)"""
        revised = """def parent_watch():
    if os.name == 'nt':
        # A blocking read of Python's buffered stdin during OpenCV/FFmpeg
        # initialization can contend with Windows stdio/CRT locks. Monitor the
        # actual parent process instead; no stdin access after the config line.
        import ctypes
        from ctypes import wintypes
        kernel = ctypes.WinDLL('kernel32', use_last_error=True)
        kernel.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
        kernel.OpenProcess.restype = wintypes.HANDLE
        kernel.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
        kernel.WaitForSingleObject.restype = wintypes.DWORD
        kernel.CloseHandle.argtypes = [wintypes.HANDLE]
        kernel.CloseHandle.restype = wintypes.BOOL
        parent = kernel.OpenProcess(0x00100000, False, os.getppid())
        if not parent: os._exit(3)
        try:
            status = kernel.WaitForSingleObject(parent, 0xffffffff)
        finally:
            kernel.CloseHandle(parent)
        os._exit(0 if status == 0 else 3)
    try: sys.stdin.buffer.read(1)
    finally: os._exit(0)"""
        s=one(s,original,revised)
        result=s.encode('utf-8')
        # Compile the embedded child too: the surrounding app compilation alone
        # cannot catch syntax errors inside a raw Python source string.
        child=next(n.value.value for n in ast.parse(s).body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='_NETWORK_CAPTURE_CHILD' for t in n.targets))
        compile(child,'network_decoder','exec')
        print('Windows decoder watchdog patched; app SHA-256:',hashlib.sha256(result).hexdigest())
    return result
for folder,name,digest in [('release-input','VisionOps_AI_Studio_update.zip','469711d2d36b619feeb44a8350034fd563c691f8563a0df9dad036373a5882de'),('setup-input','VisionOps_AI_Studio_Setup.zip','2a4cf84a5fc7749ff2b4b8ea315334056e248879dc53331cb503961fd32eff9a')]:
    src=ROOT/folder/'v0.13.0.zip'
    assert hashlib.sha256(src.read_bytes()).hexdigest()==digest,'Base archive differs from tested release'
    with zipfile.ZipFile(src) as old,zipfile.ZipFile(OUT/name,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX)
            rel=item.filename[len(PREFIX):]
            assert '..' not in Path(rel).parts
            data=change(rel,old.read(item))
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(item,data)
    with zipfile.ZipFile(OUT/name) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX+'version.json'))['version']=='0.13.1'
        if folder=='setup-input':z.extractall(OUT)
print('Verified original candidate hashes, scoped watchdog patch and both archives')
