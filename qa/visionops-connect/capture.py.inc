# Camera Engine 1.0 foundation: isolate native network decoder calls. No external
# dependency or new update file. Source credentials travel only over a private pipe.
_NETWORK_CAPTURE_CHILD = r"""
import os, sys, json, struct, threading, time

def parent_watch():
    if os.name == 'nt':
        # Avoid blocking the Windows CRT with concurrent stdin reads.
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
    finally: os._exit(0)

config = json.loads(sys.stdin.buffer.readline(8192))
threading.Thread(target=parent_watch, daemon=True).start()
import cv2
import numpy as np
cap = None
try:
    cap = cv2.VideoCapture(config['source'], cv2.CAP_FFMPEG, [
        cv2.CAP_PROP_OPEN_TIMEOUT_MSEC, 5000,
        cv2.CAP_PROP_READ_TIMEOUT_MSEC, 5000])
    if not cap.isOpened(): sys.exit(2)
    next_frame = 0.0
    failures = 0
    while True:
        ok, frame = cap.read()
        if not ok or frame is None or not frame.size:
            failures += 1
            if failures >= 3: break
            time.sleep(.12)
            continue
        failures = 0
        now = time.monotonic()
        if now < next_frame:
            time.sleep(.002)
            continue
        next_frame = now + .125
        h, w = frame.shape[:2]
        scale = min(1., 960/max(1,w),720/max(1,h))
        if scale < 1:
            frame = cv2.resize(frame,(max(1,round(w*scale)),max(1,round(h*scale))), interpolation=cv2.INTER_AREA)
        if frame.ndim != 3 or frame.shape[2] != 3 or frame.dtype != np.uint8: break
        h,w = frame.shape[:2]
        blob = np.ascontiguousarray(frame).tobytes()
        # Length derived from validated dimensions by parent; bounded 2 MiB/frame.
        sys.stdout.buffer.write(struct.pack('!II', h,w))
        sys.stdout.buffer.write(blob)
        sys.stdout.buffer.flush()
finally:
    if cap is not None: cap.release()
"""

class _CameraCaptureError(RuntimeError):
    # No underlying exception/source URL in user-visible errors or logs.
    def __init__(self, reason):
        self.reason = reason
        super().__init__(reason)


class _IsolatedNetworkCapture:
    def __init__(self, source, stop, *, child_code=None, deadline_seconds=9.0):
        import subprocess, sys, queue
        self.stop = stop
        self.timeout = deadline_seconds
        self.frames = queue.Queue(maxsize=1)
        self.done = threading.Event()
        self.closed = threading.Event()
        self.proc = None
        self.reader = None
        self.initial = None
        try:
            # -u ensures stdout carries only framed binary data. No camera secret in
            # the process command line, environment, log file, or stdout diagnostics.
            self.proc = subprocess.Popen([sys.executable, '-u', '-c', child_code or _NETWORK_CAPTURE_CHILD],
                stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                bufsize=0, creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            self.proc.stdin.write((json.dumps({'source':source})+'\n').encode('utf-8'))
            self.reader = threading.Thread(target=self._receive, name='visionops-decoder-pipe', daemon=True)
            self.reader.start()
            self.initial = self._next(self.timeout)
        except BaseException:
            self.release()
            raise

    def _receive(self):
        import struct, numpy as np, queue
        def exact(n):
            out = bytearray(n)
            view = memoryview(out)
            offset = 0
            while offset < n:
                size = self.proc.stdout.readinto(view[offset:])
                if not size: raise EOFError()
                offset += size
            return out
        try:
            while not self.closed.is_set():
                h,w = struct.unpack('!II',exact(8))
                if not 1<=h<=720 or not 1<=w<=960: raise ValueError('bad frame')
                frame = np.frombuffer(exact(h*w*3),dtype=np.uint8).reshape(h,w,3)
                # Only the freshest frame survives; pipe is drained independently
                # of inference/face redaction, preventing growing decoder backlogs.
                try: self.frames.put_nowait(frame)
                except queue.Full:
                    try: self.frames.get_nowait()
                    except queue.Empty: pass
                    try: self.frames.put_nowait(frame)
                    except queue.Full: pass
        except Exception:
            pass
        finally:
            self.done.set()

    def _next(self, timeout):
        import queue
        deadline = time.monotonic()+timeout
        while not self.stop.is_set() and not self.closed.is_set():
            if self.done.is_set(): raise _CameraCaptureError('decoder_exit')
            remaining=deadline-time.monotonic()
            if remaining<=0: raise _CameraCaptureError('decoder_timeout')
            try: return self.frames.get(timeout=min(.1,remaining))
            except queue.Empty: continue
        raise _CameraCaptureError('stopped')

    def isOpened(self):
        return self.initial is not None and not self.closed.is_set() and not self.done.is_set()

    def read(self):
        return True, self._next(self.timeout)

    def release(self):
        if self.closed.is_set(): return
        self.closed.set()
        p = self.proc
        if p is not None:
            # Explicitly terminate a hung C/FFmpeg decoder; never invoke cap.release
            # on the same blocked driver from the supervising application thread.
            if p.stdin is not None:
                try: p.stdin.close()
                except OSError: pass
            try:
                if p.poll() is None: p.terminate()
                p.wait(timeout=1.0)
            except Exception:
                try: p.kill(); p.wait(timeout=1.0)
                except Exception: pass
            if self.reader is not None: self.reader.join(timeout=1.0)
            if p.stdout is not None and (self.reader is None or not self.reader.is_alive()):
                try: p.stdout.close()
                except OSError: pass
        self.initial=None


def _open_universal_camera(protocol, source, stop=None):
    if protocol == 'usb':
        cap, initial, _, _ = open_camera(source)
        return cap, initial
    cap = _IsolatedNetworkCapture(source, stop if stop is not None else threading.Event())
    initial = cap.initial
    return cap, initial
