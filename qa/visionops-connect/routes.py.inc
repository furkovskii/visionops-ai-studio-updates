# User-initiated local camera discovery; all entry points use the existing
# authenticated workspace, ownership and global POST CSRF checks.
_VO_FALLBACK_LOCK=threading.Lock()
_VO_RATE_LOCK=threading.Lock()
_VO_RATE={}
def _vo_rate_limit(owner,action,window=60,maximum=20):
    # Per authenticated workspace; bounded memory and no camera credentials.
    now=time.monotonic()
    key=(str(owner),action)
    with _VO_RATE_LOCK:
        for k,times in list(_VO_RATE.items()):
            fresh=[t for t in times if 0<=now-t<window]
            if fresh:_VO_RATE[k]=fresh
            else:_VO_RATE.pop(k,None)
        if len(_VO_RATE)>512:
            _VO_RATE.clear()
        values=_VO_RATE.setdefault(key,[])
        if len(values)>=maximum:
            raise CloudError('Çok fazla bağlantı denemesi. Biraz sonra yeniden deneyin.',429)
        values.append(now)


@app.post('/api/workspace/connect/scan')
def visionops_connect_scan():
    module=session.get('active_module')
    if module not in _ip_cameras or not _wall_authorized():
        raise CloudError('Kamera keşfi için yetkili çalışma alanı gerekli.',403)
    form=request.get_json(silent=True)
    if not isinstance(form,dict) or type(form.get('extended',False)) is not bool:
        raise CloudError('Geçersiz tarama isteği.',400)
    _vo_rate_limit(_wall_owner(),'scan',60,6 if not form['extended'] else 3)
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
    _vo_rate_limit(_wall_owner(),'check',60,20)
    try:result=_vo_connect_check(form)
    except (ValueError,TypeError,OverflowError):
        raise CloudError('Yerel ağ IPv4 adresi veya port geçersiz.',400) from None
    response=jsonify(result)
    response.headers['Cache-Control']='no-store'
    return response
