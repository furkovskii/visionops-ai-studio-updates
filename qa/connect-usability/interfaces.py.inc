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
            startup=subprocess.STARTUPINFO()
            startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            startup.wShowWindow = getattr(subprocess,'SW_HIDE',0)
            r=subprocess.run(['powershell.exe','-NoLogo','-NoProfile','-NonInteractive','-Command',command],
                             stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,stdin=subprocess.DEVNULL,
                             startupinfo=startup,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0),
                             timeout=4,check=True)
            if len(r.stdout)<=8192:
                result=json.loads(r.stdout.decode('utf-8-sig'))
                for ip in (result if isinstance(result,list) else [result]):add(ip)
        except (OSError,ValueError,subprocess.SubprocessError):pass
    return sorted(candidates)[:12]
