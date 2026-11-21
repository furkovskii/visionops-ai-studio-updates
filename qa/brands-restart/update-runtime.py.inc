# Signals the native window to close only after a verified update has been staged.
# A standalone Flask server never restarts the user's desktop or launches code.
import threading
_DESKTOP_HOSTED=False
_DESKTOP_AUTO_RESTART=threading.Event()

def _vo_request_desktop_restart():
    if os.name!='nt' or not _DESKTOP_HOSTED:
        return False
    _DESKTOP_AUTO_RESTART.set()
    return True
