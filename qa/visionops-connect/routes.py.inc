# User-initiated local camera discovery; all entry points use the existing
# authenticated workspace, ownership and global POST CSRF checks.
_VO_FALLBACK_LOCK=threading.Lock()

@app.post('/api/workspace/connect/scan')
def visionops_connect_scan():
    module=session.get('active_module')
    if module not in _ip_cameras or not _wall_authorized():
        raise CloudError('Kamera keşfi için yetkili çalışma alanı gerekli.',403)
    form=request.get_json(silent=True)
    if not isinstance(form,dict) or type(form.get('extended',False)) is not bool:
        raise CloudError('Geçersiz tarama isteği.',400)
    found=_discover_onvif()
    if form.get('extended') is True:
        if not _VO_FALLBACK_LOCK.acquire(blocking=False):
            raise CloudError('Genişletilmiş tarama zaten çalışıyor.',429)
        try:
            for rtsp in _vo_scan_rtsp():
                if not any(d['ip']==rtsp['ip'] for d in found):
                    found.append(rtsp)
        finally:_VO_FALLBACK_LOCK.release()
    found=found[:64]
    response=jsonify(devices=found,interfaces=len(_vo_interfaces()),
                     mode='onvif_and_optin_rtsp' if form['extended'] else 'onvif')
    response.headers['Cache-Control']='no-store'
    return response

@app.post('/api/workspace/connect/check')
def visionops_connect_check():
    module=session.get('active_module')
    if module not in _ip_cameras or not _wall_authorized():
        raise CloudError('Kamera bağlantı kontrolü için yetkili çalışma alanı gerekli.',403)
    form=request.get_json(silent=True)
    try:result=_vo_connect_check(form)
    except (ValueError,TypeError,OverflowError):
        raise CloudError('Yerel ağ IPv4 adresi veya port geçersiz.',400) from None
    response=jsonify(result)
    response.headers['Cache-Control']='no-store'
    return response
