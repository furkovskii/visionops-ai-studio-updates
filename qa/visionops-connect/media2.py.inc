# ONVIF Media2 (Profile T) compatibility, keeping legacy Media1 for older NVRs.
# The device may advertise an endpoint only on its own local IP; redirects,
# remote hosts, credentials in the URL and unsupported protocols are refused.
def _vo_media2_profiles(form):
    ip=_private_ipv4(str(form.get('ip','')).strip())
    user,password=str(form.get('username','')),str(form.get('password',''))
    if len(user)>128 or len(password)>256 or (password and not user):
        raise ValueError('invalid credentials')
    scheme=str(form.get('onvif_scheme','http'))
    if scheme not in ('http','https'):raise ValueError('invalid scheme')
    port=int(form.get('port') or (443 if scheme=='https' else 80))
    path=str(form.get('path','')).strip() or '/onvif/device_service'
    if len(path)>512 or not path.startswith('/'):raise ValueError('invalid path')
    endpoint=f'{scheme}://{ip}:{port}{path}'
    _local_endpoint(endpoint,ip)
    dev='http://www.onvif.org/ver10/device/wsdl/'
    media_ns='http://www.onvif.org/ver20/media/wsdl/'
    services=_onvif_soap(endpoint,ip,dev+'GetServices',
       '<tds:GetServices><tds:IncludeCapability>false</tds:IncludeCapability></tds:GetServices>',user,password)
    media=None
    for service in services.findall('.//{*}Service')[:64]:
        if service.findtext('{*}Namespace')==media_ns.rstrip('/'):
            media=service.findtext('{*}XAddr');break
    if not media:raise ValueError('no Media2 service')
    _local_endpoint(media,ip)
    profiles=_onvif_soap(media,ip,media_ns+'GetProfiles','<tr2:GetProfiles/>',user,password)
    items=[]
    for item in profiles.findall('.//{*}Profiles')[:64]:
        token=item.get('token','')
        if token and len(token)<=256:
            items.append({'token':token,'name':(item.findtext('{*}Name') or token)[:100],'mode':'media2'})
    if not items:raise ValueError('no Media2 profiles')
    return ip,media,user,password,items

def _vo_media2_source(form):
    from xml.sax.saxutils import escape
    ip,media,user,password,items=_vo_media2_profiles(form)
    token=str(form.get('profile','')) or items[0]['token']
    if token not in {item['token'] for item in items}:raise ValueError('unknown Media2 profile')
    body=('<tr2:GetStreamUri><tr2:Protocol>RTSP</tr2:Protocol>'
          f'<tr2:ProfileToken>{escape(token)}</tr2:ProfileToken></tr2:GetStreamUri>')
    reply=_onvif_soap(media,ip,'http://www.onvif.org/ver20/media/wsdl/GetStreamUri',body,user,password)
    uri=reply.findtext('.//{*}Uri') or ''
    part=_local_endpoint(uri,ip,('rtsp',))
    route=part.path+('?' + part.query if part.query else '')
    _,source=_camera_source({'protocol':'rtsp','ip':ip,'port':part.port or 554,
                            'path':route,'username':user,'password':password})
    return 'onvif',source

def _onvif_source(form):
    from xml.sax.saxutils import escape
    mode=form.get('onvif_mode','media1')
    if mode not in ('media1','media2'):
        raise CloudError('Geçersiz ONVIF medya türü.',400)
    if mode=='media2':
        try:return _vo_media2_source(form)
        except Exception:
            raise CloudError('ONVIF Media2 akışı alınamadı. Yetkileri, saat ayarını ve profili kontrol edin.',400) from None
    try:
        ip,media,user,password,items=_onvif_profiles(form)
        token=str(form.get('profile','')) or items[0]['token']
        if token not in {item['token'] for item in items}:raise ValueError('unknown profile')
        body=('<trt:GetStreamUri><trt:StreamSetup><tt:Stream>RTP-Unicast</tt:Stream>'
              '<tt:Transport><tt:Protocol>RTSP</tt:Protocol></tt:Transport></trt:StreamSetup>'
              f'<trt:ProfileToken>{escape(token)}</trt:ProfileToken></trt:GetStreamUri>')
        reply=_onvif_soap(media,ip,'http://www.onvif.org/ver10/media/wsdl/GetStreamUri',body,user,password)
        uri=reply.findtext('.//{*}Uri') or ''
        part=_local_endpoint(uri,ip,('rtsp',))
        path=part.path+('?' + part.query if part.query else '')
        _,source=_camera_source({'protocol':'rtsp','ip':ip,'port':part.port or 554,'path':path,
                                 'username':user,'password':password})
        return 'onvif',source
    except Exception:
        # Some newer Profile T devices expose Media2 but no Media1.
        # Do not substitute another profile for a user-selected Media1 token.
        if not form.get('profile'):
            try:return _vo_media2_source(form)
            except Exception:pass
        raise CloudError('ONVIF yayın adresi alınamadı. Kullanıcı, profil, cihaz saati ve Media1/Media2 desteğini kontrol edin; gerekirse RTSP kullanın.',400) from None
