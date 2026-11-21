"""v0.13.6 signed updater restart and brand coverage regression.
All tests are offline; physical cameras and installed Windows GUI remain separate.
"""
import ast,json,os,sys,threading,tempfile,unittest,zipfile
from pathlib import Path
from unittest import mock
ROOT=Path(__file__).resolve().parents[2]
BUILD=ROOT/'brands-restart-build'
APP=BUILD/'VisionOps_AI_Studio'
def funcs(name,wanted):
    tree=ast.parse((APP/name).read_text(encoding='utf-8'))
    nodes=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name in wanted]
    assert len(nodes)==len(wanted),(name,wanted,[n.name for n in nodes])
    return compile(ast.Module(body=nodes,type_ignores=[]),str(APP/name),'exec')
class RestartTests(unittest.TestCase):
    def test_native_only_restart_signal(self):
        import types
        env={'os':types.SimpleNamespace(name='posix'),'threading':threading,
             '_DESKTOP_HOSTED':False,'_DESKTOP_AUTO_RESTART':threading.Event()}
        exec(funcs('update_manager.py',{'_vo_request_desktop_restart'}),env)
        self.assertFalse(env['_vo_request_desktop_restart']())
        self.assertFalse(env['_DESKTOP_AUTO_RESTART'].is_set())
        env['os']=types.SimpleNamespace(name='nt')
        self.assertFalse(env['_vo_request_desktop_restart']())
        env['_DESKTOP_HOSTED']=True
        self.assertTrue(env['_vo_request_desktop_restart']())
        self.assertTrue(env['_DESKTOP_AUTO_RESTART'].is_set())
    def test_download_verifies_then_stages_before_restart(self):
        s=(APP/'update_manager.py').read_text(encoding='utf-8')
        d=s[s.index('    def updates_download():'):s.index("    @app.route('/api/updates/upload'")]
        m=s[s.index('    def updates_upload():'):s.index("\nif __name__ ==")]
        self.assertLess(d.index("version=stage_package"),d.index("automatic=_vo_request_desktop_restart()"))
        self.assertLess(m.index("version=stage_package"),m.index("automatic=_vo_request_desktop_restart()"))
        self.assertIn("automatic_restart",d)
        self.assertIn("automatic_restart",m)
        self.assertIn("if not admin():",d)
        self.assertIn("if not admin():",m)
        self.assertEqual(s.count("automatic=_vo_request_desktop_restart()"),2)
    def test_gui_close_after_update_signal(self):
        import logging,time
        env={'logging':logging,'time':time}
        exec(funcs('studio.py',{'_vo_watch_desktop_restart'}),env)
        event=threading.Event();event.set()
        window=mock.Mock()
        with mock.patch.object(time,'sleep'):
            env['_vo_watch_desktop_restart'](window,event)
        window.destroy.assert_called_once()
    def test_no_relaunch_on_normal_exit_or_missing_stage(self):
        import subprocess,logging
        env={'subprocess':subprocess,'logging':logging}
        exec(funcs('studio.py',{'_vo_spawn_update_launcher'}),env)
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder); data=root/'data';data.mkdir()
            exe=root/'.venv'/'Scripts'/'pythonw.exe';exe.parent.mkdir(parents=True);exe.touch()
            (root/'launcher.pyw').touch()
            event=threading.Event()
            with mock.patch('subprocess.Popen') as p:
                self.assertFalse(env['_vo_spawn_update_launcher'](event,root,data,3456))
                event.set()
                self.assertFalse(env['_vo_spawn_update_launcher'](event,root,data,3456))
                p.assert_not_called()
                (data/'update_pending.json').write_text('{"version":"0.13.7"}',encoding='utf-8')
                self.assertTrue(env['_vo_spawn_update_launcher'](event,root,data,3456))
                self.assertEqual(p.call_count,1)
                args=p.call_args.args[0]
                self.assertEqual(args[1],str(root/'launcher.pyw'))
                self.assertEqual(args[-2:],['--wait-pid','3456'])
                self.assertNotIn('shell',p.call_args.kwargs)
                self.assertEqual(p.call_args.kwargs['cwd'],str(root))
    @unittest.skipUnless(os.name=='nt','Requires native Windows API mock')
    def test_launcher_waits_for_previous_pid_before_replacing_files(self):
        import ctypes,types
        kernel=mock.MagicMock()
        kernel.OpenProcess.return_value=44
        kernel.WaitForSingleObject.return_value=0
        env={'os':types.SimpleNamespace(name='nt',getpid=lambda:1234),'ctypes':ctypes}
        exec(funcs('launcher.pyw',{'_vo_wait_for_previous_process'}),env)
        with mock.patch('ctypes.WinDLL',return_value=kernel):
            self.assertTrue(env['_vo_wait_for_previous_process'](5678))
            self.assertEqual(kernel.OpenProcess.call_args.args,(0x00100000,False,5678))
            kernel.WaitForSingleObject.assert_called_once_with(44,60000)
            kernel.CloseHandle.assert_called_once_with(44)
            kernel.WaitForSingleObject.return_value=258  # WAIT_TIMEOUT
            with self.assertRaises(RuntimeError):env['_vo_wait_for_previous_process'](5678)
        with self.assertRaises(RuntimeError):env['_vo_wait_for_previous_process'](1234)
    def test_parent_wait_before_update_installation(self):
        s=(APP/'launcher.pyw').read_text(encoding='utf-8')
        self.assertLess(s.index("_vo_wait_for_previous_process(int(sys.argv[2]))"),
                        s.index("if (DATA/'update_pending.json').exists():"))
        self.assertIn("creationflags=subprocess.CREATE_NO_WINDOW",s)
        self.assertIn("if len(sys.argv)!=3 or sys.argv[1]!='--wait-pid'",s)
        studio=(APP/'studio.py').read_text(encoding='utf-8')
        self.assertIn("_vo_updates._DESKTOP_HOSTED=True",studio)
        self.assertLess(studio.index("server.shutdown()"),studio.index("_vo_spawn_update_launcher(_vo_updates._DESKTOP_AUTO_RESTART"))
        self.assertIn("window.events.shown += _on_window_shown",studio)
    def test_21_additional_brand_guides_and_model_safety(self):
        html=(APP/'templates'/'activate.html').read_text(encoding='utf-8')
        brands=json.loads((ROOT/'qa/brands-restart/brands.json').read_text(encoding='utf-8'))
        self.assertGreaterEqual(len(brands),20)
        for b in brands:
            self.assertIn('<option value="'+b['id']+'">'+b['label']+'</option>',html)
            self.assertIn('"'+b['id']+'":',html)
        self.assertIn("/axis-media/media.amp",html)
        self.assertIn("else {path.value='';",html)
        self.assertIn("id=\"vo-connect-device-brand\"",html)
        self.assertIn("id=\"vo-help-brand\"",html)
    def test_update_and_installer_contents_match_and_keep_previous_features(self):
        with zipfile.ZipFile(BUILD/'VisionOps_AI_Studio_update.zip') as update,zipfile.ZipFile(BUILD/'VisionOps_AI_Studio_Setup.zip') as setup:
            self.assertEqual(json.loads(update.read('VisionOps_AI_Studio/version.json'))['version'],'0.13.6')
            for name in update.namelist():self.assertEqual(update.read(name),setup.read(name),name)
            ui=update.read('VisionOps_AI_Studio/templates/activate.html')
            self.assertIn(b'vo-connect-scan',ui)
            self.assertIn(b'vo-scan-progress',ui)
if __name__=='__main__':unittest.main(verbosity=2)
