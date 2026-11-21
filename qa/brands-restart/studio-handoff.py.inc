# Native Studio ↔ signed updater coordination.
# Restart cannot run before the current process has fully released app files.
import subprocess
import time
import sys
import update_manager as _vo_updates

def _vo_watch_desktop_restart(window,event):
    event.wait()
    time.sleep(1.2) # allow HTTP response / browser notification to be delivered
    try:
        logging.info('Verified update ready; closing native window for restart')
        window.destroy()
    except Exception:
        logging.exception('Native restart window close failed; user may restart manually')

def _vo_spawn_update_launcher(event,root,data,pid):
    if not event.is_set():return False
    if not (data/'update_pending.json').is_file():
        logging.error('Restart requested without staged package; declining launcher')
        return False
    pythonw=root/'.venv'/'Scripts'/'pythonw.exe'
    launcher=root/'launcher.pyw'
    if not pythonw.is_file() or not launcher.is_file():
        logging.error('Windows updater launcher missing, leaving verified package staged')
        return False
    try:
        subprocess.Popen([str(pythonw),str(launcher),'--wait-pid',str(pid)],
                         cwd=str(root),stdin=subprocess.DEVNULL,
                         stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,
                         creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0),
                         close_fds=True)
        logging.info('Launched safe restart handoff after Studio shutdown')
        return True
    except OSError:
        logging.exception('Desktop update handoff failed, manual restart remains possible')
        return False
