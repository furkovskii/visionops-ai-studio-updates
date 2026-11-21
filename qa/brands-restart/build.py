"""v0.13.6: model-aware camera brands and safe native automatic update restart.
Build from verified 0.13.5 packages without altering prior update sources.
"""
import ast,html,json,re,subprocess,sys,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).resolve().parent
OUT=ROOT/'brands-restart-build';OUT.mkdir(exist_ok=True)
PREFIX='VisionOps_AI_Studio/';VERSION='0.13.6'
def inc(name):return (HERE/name).read_text(encoding='utf-8')
def one(s,old,new):
    assert s.count(old)==1,('Refusing unexpected source mutation',old[:120],s.count(old))
    return s.replace(old,new)
def patch_html(s):
    brands=json.loads(inc('brands.json'))
    assert len(brands)>=20 and len({b['id'] for b in brands})==len(brands)
    # Brand guides and RTSP-only connection presets are deliberately separate.
    # Unverified models get explanatory help, not fabricated streaming paths.
    options=''.join('<option value="'+html.escape(b['id'],quote=True)+'">'+html.escape(b['label'])+'</option>' for b in brands)
    s=one(s,'<option value="nvr">NVR / DVR kayıt cihazı</option></select>',
          options+'<option value="nvr">NVR / DVR kayıt cihazı</option></select>')
    s=one(s,'<option value="other">Diğer marka (gelişmiş ayarlar)</option></select>',
          options+'<option value="other">Diğer marka (gelişmiş ayarlar)</option></select>')
    guide=''.join(' '+json.dumps(b['id'])+':'+json.dumps(b['help'],ensure_ascii=False)+',\n' for b in brands)
    s=one(s,'const guide={','const guide={\n'+guide)
    s=one(s,"const rtspPresets={auto:'/stream1',",
          "const rtspPresets={axis:'/axis-media/media.amp',auto:'/stream1',")
    s=one(s,"if(rtspPresets[rtspBrand.value])path.value=rtspPresets[rtspBrand.value];\n  if(rtspBrand.value==='other')$('vo-connect-stream-details').open=true;",
          "if(rtspPresets[rtspBrand.value])path.value=rtspPresets[rtspBrand.value];\n"
          "  else {path.value='';$('vo-connect-stream-details').open=true;}\n"
          "  if(rtspBrand.value==='other')$('vo-connect-stream-details').open=true;")
    assert 'axis-media/media.amp' in s
    return s
def patch_update(s):
    s=one(s,'def init_updates(app, data, root):',inc('update-runtime.py.inc')+'\n\ndef init_updates(app, data, root):')
    old="return jsonify({'ok':True,'version':version,'message':'Güncelleme hazır. Uygulamayı kapatıp masaüstü simgesinden yeniden açın.'})"
    new=("automatic=_vo_request_desktop_restart()\n"
         "            return jsonify({'ok':True,'version':version,'automatic_restart':automatic,"
         "'message':'Güncelleme güvenli şekilde hazırlandı. Uygulama otomatik yeniden başlatılıyor…' "
         "if automatic else 'Güncelleme hazır. Masaüstü uygulamasını yeniden açarak kurulumu tamamlayın.'})")
    s=one(s,old,new)
    old="return jsonify({'ok':True,'version':version,'message':'Güncelleme hazır. Uygulamayı kapatıp tekrar açın.'})"
    s=one(s,old,new)
    s=one(s,"'pending':(data/'update_pending.json').exists()})",
          "'pending':(data/'update_pending.json').exists(),"
          "'auto_restart_supported':(os.name=='nt' and _DESKTOP_HOSTED)})")
    return s
def patch_dashboard(s):
    s=one(s,"el('updateDetail').textContent='Uygulamayı kapatıp masaüstü simgesiyle tekrar açın.';",
          "el('updateDetail').textContent=d.auto_restart_supported?"
          "'Güncelleme doğrulandı; uygulama güvenle yeniden başlatılıyor…':"
          "'Güncelleme hazır. Masaüstü uygulamasını yeniden açarak kurulumu tamamlayın.';")
    return s
def patch_launcher(s):
    s=one(s,'try:\n    if not PYTHONW.is_file():',
          inc('launcher-wait.py.inc')+'\ntry:\n    if not PYTHONW.is_file():')
    s=one(s,'    os.chdir(ROOT)\n    # The app is not running yet;',
          "    os.chdir(ROOT)\n"
          "    if len(sys.argv)>1:\n"
          "        if len(sys.argv)!=3 or sys.argv[1]!='--wait-pid':\n"
          "            raise RuntimeError('Geçersiz yeniden başlatma isteği.')\n"
          "        _vo_wait_for_previous_process(int(sys.argv[2]))\n"
          "    # The app is not running yet;")
    return s
def patch_studio(s):
    s=one(s,'try:\n    import webview\n    import app as core',
          inc('studio-handoff.py.inc')+"\ntry:\n    import webview\n    import app as core\n    _vo_updates._DESKTOP_HOSTED=True")
    s=one(s,'    window.events.shown += _on_window_shown\n',
          '    window.events.shown += _on_window_shown\n'
          "    threading.Thread(target=_vo_watch_desktop_restart,\n"
          "                     args=(window,_vo_updates._DESKTOP_AUTO_RESTART),\n"
          "                     daemon=True,name='VisionOpsUpdateRestart').start()\n")
    s=one(s,"        core.state['running'] = False\n        server.shutdown()",
          "        core.state['running'] = False\n"
          "        server.shutdown()\n"
          "        _vo_spawn_update_launcher(_vo_updates._DESKTOP_AUTO_RESTART,ROOT,DATA,os.getpid())")
    return s
subprocess.run([sys.executable,str(ROOT/'qa/connect-usability/build.py')],check=True,cwd=ROOT)
subprocess.run([sys.executable,str(ROOT/'qa/connect-usability/test_usability.py')],check=True,cwd=ROOT)
for archive in ('VisionOps_AI_Studio_update.zip','VisionOps_AI_Studio_Setup.zip'):
    with zipfile.ZipFile(ROOT/'connect-usability-build'/archive) as old, zipfile.ZipFile(OUT/archive,'w',zipfile.ZIP_DEFLATED) as new:
        for item in old.infolist():
            if item.is_dir():continue
            assert item.filename.startswith(PREFIX) and '..' not in Path(item.filename).parts
            rel=item.filename[len(PREFIX):]
            data=old.read(item)
            if rel in ('update_manager.py','launcher.pyw','studio.py','templates/activate.html','static/dashboard.js'):
                text=data.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
                fn={'update_manager.py':patch_update,'launcher.pyw':patch_launcher,
                    'studio.py':patch_studio,'templates/activate.html':patch_html,'static/dashboard.js':patch_dashboard}[rel]
                data=fn(text).encode('utf-8')
            elif rel=='version.json': data=(json.dumps({'version':VERSION})+'\n').encode('utf-8')
            if rel.endswith(('.py','.pyw')):compile(data,rel,'exec')
            new.writestr(item,data)
    with zipfile.ZipFile(OUT/archive) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX+'version.json'))['version']==VERSION
        if archive.endswith('Setup.zip'):z.extractall(OUT)
with zipfile.ZipFile(OUT/'VisionOps_AI_Studio_update.zip') as u,zipfile.ZipFile(OUT/'VisionOps_AI_Studio_Setup.zip') as s:
    for n in u.namelist():assert u.read(n)==s.read(n),n
print('v0.13.6 verified-brand and safe desktop restart update+setup built; previous releases preserved.')
