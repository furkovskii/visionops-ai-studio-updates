# VisionOps Connect · safe local discovery and explicitly consented RTSP fallback.
# All traffic originates on the customer PC. No camera addresses or credentials
# are sent to an external service. The scan never tries default passwords.
def _vo_interfaces():
    import socket,ipaddress,subprocess
    candidates=set()
    def add(value):
        try:ip=_private_ipv4(str(value))
        except (ValueError,TypeError):return
        candidates.add(ip)
    try:
        for item in socket.getaddrinfo(socket.gethostname(),None,socket.AF_INET,socket.SOCK_DGRAM):add(item[4][0])
    except OSError:pass
    # The UDP connect call determines a source interface without sending packets.
    for gateway in ('192.0.2.1','198.51.100.1'):
        try:
            with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
                s.connect((gateway,9));add(s.getsockname()[0])
        except OSError:pass
    # Windows multi-NIC discovery: read OS adapter information, not an internet
    # scan or any user-provided command. Failure falls back to standard sockets.
    if os.name=='nt':
        try:
            command=('Get-NetIPAddress -AddressFamily IPv4 | '
                     'Where-Object { $_.AddressState -eq "Preferred" } | '
                     'Select-Object -ExpandProperty IPAddress | ConvertTo-Json -Compress')
            r=subprocess.run(['powershell.exe','-NoProfile','-NonInteractive','-Command',command],
                             stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,timeout=4,check=True)
            if len(r.stdout)<=8192:
                result=json.loads(r.stdout.decode('utf-8-sig'))
                for ip in (result if isinstance(result,list) else [result]):add(ip)
        except (OSError,ValueError,subprocess.SubprocessError):pass
    return sorted(candidates)[:12]

def _vo_parse_probe(body,peer,message_id):
    # Only consume replies to our one-time Probe and endpoints on the sender's IP.
    try:
        ip=_private_ipv4(peer)
        root=_safe_xml(body)
        if root.findtext('.//{*}RelatesTo')!=message_id:return []
        found=[]
        for match in root.findall('.//{*}ProbeMatch')[:16]:
            for url in (match.findtext('{*}XAddrs') or '').split()[:16]:
                if len(url)>1024:continue
                try:part=_local_endpoint(url,ip)
                except (ValueError,TypeError):continue
                path=part.path+('?' + part.query if part.query else '')
                if len(path)>512:continue
                found.append(dict(ip=ip,port=part.port or (443 if part.scheme=='https' else 80),
                                  path=path,scheme=part.scheme,discovered_via='onvif'))
        return found[:32]
    except (ValueError,TypeError,AttributeError,IndexError):return []

def _discover_onvif(timeout=3.5):
    import socket,select
    if not _DISCOVERY_LOCK.acquire(blocking=False):
        raise CloudError('Kamera keşfi zaten çalışıyor.',429)
    sockets=[]
    try:
        timeout=min(6.,max(1.,float(timeout)))
        identity='urn:uuid:'+str(uuid.uuid4())
        packet=(f'<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" '
                'xmlns:a="http://schemas.xmlsoap.org/ws/2004/08/addressing" '
                'xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery" '
                'xmlns:dn="http://www.onvif.org/ver10/network/wsdl"><s:Header>'
                f'<a:MessageID>{identity}</a:MessageID>'
                '<a:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>'
                '<a:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action>'
                '</s:Header><s:Body><d:Probe><d:Types>dn:NetworkVideoTransmitter</d:Types>'
                '</d:Probe></s:Body></s:Envelope>').encode()
        interfaces=_vo_interfaces()
        for ip in interfaces or ['']:
            sock=None
            try:
                sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM,socket.IPPROTO_UDP)
                sock.setsockopt(socket.IPPROTO_IP,socket.IP_MULTICAST_TTL,1)
                if ip:sock.setsockopt(socket.IPPROTO_IP,socket.IP_MULTICAST_IF,socket.inet_aton(ip))
                sock.bind((ip or '',0))
                sock.sendto(packet,('239.255.255.250',3702))
                sockets.append(sock)
            except OSError:
                if sock is not None:
                    try:sock.close()
                    except OSError:pass
        if not sockets:return []
        end=time.monotonic()+timeout;found={}
        while len(found)<64 and time.monotonic()<end:
            ready,_,_=select.select(sockets,[],[],min(.35,max(0,end-time.monotonic())))
            for sock in ready:
                try:body,peer=sock.recvfrom(65535)
                except OSError:continue
                for dev in _vo_parse_probe(body,peer[0],identity):
                    key=(dev['ip'],dev['port'],dev['path'],dev['scheme'])
                    found[key]=dev
                    if len(found)>=64:break
        return sorted(found.values(),key=lambda x:(x['ip'],x['port'],x['path']))
    finally:
        for sock in sockets:
            try:sock.close()
            except OSError:pass
        _DISCOVERY_LOCK.release()

def _vo_probe_rtsp(ip,port,timeout=.7):
    """No authentication attempts. Only identify an RTSP-speaking endpoint."""
    import socket
    try:
        ip=_private_ipv4(ip);port=int(port)
        if not 1<=port<=65535:raise ValueError()
        with socket.create_connection((ip,port),timeout=timeout) as conn:
            conn.settimeout(timeout)
            req=(f'OPTIONS rtsp://{ip}:{port}/ RTSP/1.0\r\n'
                 'CSeq: 1\r\nUser-Agent: VisionOps-Connect\r\n\r\n').encode('ascii')
            conn.sendall(req)
            reply=conn.recv(128)
        return reply.startswith((b'RTSP/1.0 ',b'RTSP/2.0 '))
    except (OSError,ValueError,TypeError):return False

def _vo_scan_rtsp():
    """Optional owner-approved fallback; scan a bounded /24 of a local interface.
    Does not brute-force passwords, probe arbitrary ports, touch WAN or enumerate
    enterprise /16 ranges. Plain WS-Discovery is always attempted first."""
    import ipaddress,concurrent.futures
    nets=[]
    for ip in _vo_interfaces()[:2]:
        net=ipaddress.ip_network(ip+'/24',strict=False)
        if net not in nets:nets.append(net)
    hosts=[str(host) for net in nets for host in net.hosts()]
    hosts=hosts[:508]
    if not hosts:return []
    found=[]
    def check(ip):
        for port in (554,8554):
            if _vo_probe_rtsp(ip,port,.3):return dict(ip=ip,port=port,path='',scheme='rtsp',discovered_via='rtsp')
        return None
    with concurrent.futures.ThreadPoolExecutor(max_workers=24) as pool:
        for item in pool.map(check,hosts):
            if item:found.append(item)
            if len(found)>=32:break
    return found[:32]

def _vo_connect_check(form):
    """Reachability is never equivalent to a validated or decoded video stream."""
    import socket
    if not isinstance(form,dict):raise CloudError('Geçersiz bağlantı isteği.',400)
    try:ip=_private_ipv4(str(form.get('ip','')).strip())
    except (ValueError,TypeError):raise CloudError('Kamera IP adresi yerel IPv4 ağında olmalı.',400) from None
    protocol=form.get('protocol')
    if protocol not in ('onvif','rtsp','nvr','http','https'):
        raise CloudError('Bu bağlantı türü desteklenmiyor.',400)
    default={'onvif':80,'rtsp':554,'nvr':554,'http':80,'https':443}
    try:
        port=int(form.get('port') or default[protocol])
        if not 1<=port<=65535:raise ValueError()
    except (ValueError,TypeError):raise CloudError('Geçerli bir port seç.',400) from None
    try:
        with socket.create_connection((ip,port),timeout=2.5):pass
    except (OSError,TimeoutError):
        return {'reachable':False,'stream_verified':False,'profiles':[],
                'code':'network','help':'Cihaz erişilemiyor. Bilgisayarın aynı ağda olduğundan, IP adresinden, cihaz gücünden ve güvenlik duvarından emin olun.'}
    if protocol=='onvif':
        try:
            _,_,_,_,profiles=_onvif_profiles(form)
            return {'reachable':True,'stream_verified':False,'profiles':[dict(p,mode='media1') for p in profiles],
                    'code':'profiles','help':'ONVIF profilleri okundu. Bir video profili seçip görüntü bağlantısını başlatın; canlı görüntü ayrıca doğrulanacak.'}
        except Exception:
            try:
                _,_,_,_,profiles=_vo_media2_profiles(form)
                return {'reachable':True,'stream_verified':False,'profiles':profiles,
                        'code':'profiles','help':'ONVIF Media2 (Profile T) profilleri okundu. Video profili seçip görüntü bağlantısını başlatın.'}
            except Exception:pass
            return {'reachable':True,'stream_verified':False,'profiles':[],
                    'code':'onvif_auth','help':'Ağ bağlantısı var ancak ONVIF Media1 profilleri okunamadı. Kamera hesabını, ONVIF ayarını ve cihaz saatini kontrol edin. Gerekirse RTSP/NVR ile ekleyin.'}
    if protocol in ('rtsp','nvr'):
        if _vo_probe_rtsp(ip,port,.7):
            return {'reachable':True,'stream_verified':False,'profiles':[],
                    'code':'rtsp','help':'RTSP servisi yanıt veriyor. Kamera yayın yolu ve yetkileri kaydedildikten sonra gerçek görüntü doğrulanacak.'}
        return {'reachable':True,'stream_verified':False,'profiles':[],
                'code':'unknown_stream','help':'Port açık ancak RTSP yanıtı doğrulanamadı. Üretici yayın yolunu kontrol edin veya manuel eklemeyi deneyin.'}
    return {'reachable':True,'stream_verified':False,'profiles':[],
            'code':'http','help':'Cihaz portu erişilebilir. Bu sonuç görüntü akışının veya kimlik bilgilerinin doğrulandığı anlamına gelmez.'}
