# Relaunch hand-off: the old Studio process must exit BEFORE staged files
# are replaced. No shell or wildcard command, no user-supplied script paths.
import ctypes
import sys

def _vo_wait_for_previous_process(pid):
    if os.name!='nt' or type(pid) is not int or not 1<=pid<=2147483647 or pid==os.getpid():
        raise RuntimeError('Güvenli yeniden başlatma için geçerli Windows işlem kimliği gerekli.')
    from ctypes import wintypes
    kernel=ctypes.WinDLL('kernel32',use_last_error=True)
    kernel.OpenProcess.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.DWORD]
    kernel.OpenProcess.restype=wintypes.HANDLE
    kernel.WaitForSingleObject.argtypes=[wintypes.HANDLE,wintypes.DWORD]
    kernel.WaitForSingleObject.restype=wintypes.DWORD
    kernel.CloseHandle.argtypes=[wintypes.HANDLE]
    kernel.CloseHandle.restype=wintypes.BOOL
    handle=kernel.OpenProcess(0x00100000,False,pid)  # SYNCHRONIZE only
    if not handle:
        if ctypes.get_last_error()==87: return True # ERROR_INVALID_PARAMETER = already exited
        raise RuntimeError('Önceki uygulama işlemi güvenli şekilde doğrulanamadı.')
    try:
        outcome=kernel.WaitForSingleObject(handle,60000)
        if outcome!=0:raise RuntimeError('Önceki uygulama kapanmadı; güncelleme güvenlik için uygulanmadı.')
        return True
    finally:
        kernel.CloseHandle(handle)
