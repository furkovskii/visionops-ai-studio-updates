@app.post('/api/workspace/connect/scan')
def visionops_connect_scan():
    module=session.get('active_module')
    if module not in _ip_cameras or not _wall_authorized():
        raise CloudError('Kamera keşfi için yetkili çalışma alanı gerekli.',403)
    form=request.get_json(silent=True)
    if not isinstance(form,dict) or type(form.get('extended',True)) is not bool:
        raise CloudError('Geçersiz tarama isteği.',400)
    extended=form.get('extended',True)
    _vo_rate_limit(_wall_owner(),'scan',60,6)
    interfaces=_vo_interfaces()
    found=_discover_onvif(interfaces=interfaces)
    if extended:
        if not _VO_FALLBACK_LOCK.acquire(blocking=False):
            raise CloudError('Genişletilmiş tarama zaten çalışıyor.',429)
        try:
            for rtsp in _vo_scan_rtsp(interfaces=interfaces):
                if not any(d['ip']==rtsp['ip'] for d in found):
                    found.append(rtsp)
        finally:_VO_FALLBACK_LOCK.release()
    found=found[:64]
    response=jsonify(devices=found,interfaces=len(interfaces),
                     mode='onvif_and_optin_rtsp' if extended else 'onvif')
    response.headers['Cache-Control']='no-store'
    return response

