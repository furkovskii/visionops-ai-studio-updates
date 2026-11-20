# ---- Smart analytics: local, anonymous, per camera ----
def _smart_validate(values, regions):
    if not isinstance(values,list) or len(values)>8:raise CloudError('En fazla 8 senaryo tanımlanabilir.',400)
    names=set();clean=[];zones={r['name'] for r in regions}
    for v in values:
        if not isinstance(v,dict):raise CloudError('Geçersiz senaryo.',400)
        name=v.get('name');kind=v.get('kind');zone=v.get('zone','');duration=v.get('duration');threshold=v.get('threshold');cooldown=v.get('cooldown');enabled=v.get('enabled')
        if not isinstance(name,str) or not 1<=len(name.strip())<=40 or name.strip().casefold() in names or any(ord(ch)<32 for ch in name):
            raise CloudError('Senaryo adı benzersiz ve 1–40 karakter olmalı.',400)
        if kind not in ('count','absence','dwell','entry') or not isinstance(zone,str) or type(enabled) is not bool:
            raise CloudError('Geçersiz senaryo türü.',400)
        if type(duration) is not int or not 10<=duration<=43200 or type(threshold) is not int or not 1<=threshold<=100 or type(cooldown) is not int or not 60<=cooldown<=86400:
            raise CloudError('Süre veya eşik geçersiz.',400)
        if kind=='count':zone=''
        elif zone not in zones:raise CloudError('Önce kayıtlı bir analiz alanı seçin.',400)
        names.add(name.strip().casefold())
        clean.append(dict(name=name.strip(),kind=kind,zone=zone,duration=duration,threshold=threshold,cooldown=cooldown,enabled=enabled))
    return clean

def _smart_step(previous, rules, regions, rows, events, detections, stamp, token):
    """Never count missing camera time, detector gaps or unconfirmed occupancy as absence.
    State is bounded to currently configured rules and resets after camera/config changes.
    Alerts are candidates, not facts or safety-critical decisions."""
    from math import isfinite
    if not isfinite(stamp):raise ValueError('Invalid sample time')
    reset=previous.get('token')!=token
    last=previous.get('last')
    if not reset and last is not None and stamp<=last:return previous,[]
    dt=stamp-last if not reset and last is not None and 0<stamp-last<=2.75 else 0.
    old={} if reset else previous.get('rules',{})
    result={};notices=[];bounds={r['name']:r['bounds'] for r in regions}
    for rule in rules:
        if not rule['enabled']:continue
        key=rule['name'];prior=old.get(key,{})
        if prior.get('definition')!=rule:prior={}
        item=dict(definition=dict(rule),elapsed=0.,active=False,latched=False,sent=prior.get('sent',-1e20))
        kind=rule['kind'];zone=rule['zone']
        if kind!='count' and zone not in bounds:continue
        if kind=='count':active=len(detections)>=rule['threshold'];desc=f"Algılama eşiği ({rule['threshold']}) gözlendi."
        elif kind=='absence':
            x,y,w,h=bounds[zone]
            active=not any(x<=px<=x+w and y<=py<=y+h for _,px,py in detections)
            desc=f"{zone} alanında {rule['duration']} sn kesintisiz örneklemede hedef algılanmadı. Gerçek yokluk kanıtı değildir."
        elif kind=='dwell':
            row=next((r for r in rows if r['name']==zone),None)
            active=bool(row and row['count']>0 and row['seconds']>=rule['duration'])
            desc=f"{zone} alanında gözlemlenen kalış eşiği ({rule['duration']} sn) aşıldı."
        else:
            active=any(e[0]==zone and e[1]=='Giriş' for e in events)
            desc=f"{zone} alanına giriş adayı gözlendi."
        item['active']=active
        if active:
            item['elapsed']=prior.get('elapsed',0.)+dt if dt and prior.get('active') else 0.
            item['latched']=bool(dt and prior.get('active') and prior.get('latched'))
            reached=kind in ('entry','dwell') or item['elapsed']>=rule['duration']
            if reached and not item['latched'] and stamp-item['sent']>=rule['cooldown']:
                notices.append(('smart',key+' · '+desc))
                item.update(sent=stamp,latched=True)
        result[key]=item
    return {'token':token,'last':stamp,'rules':result},notices

def _smart_heat(rows,today):
    from datetime import timedelta
    out={}
    for days in (1,7,30):
        start=(today-timedelta(days=days-1)).isoformat();end=today.isoformat()
        cells=[0]*96;n=0;covered=set()
        for r in rows:
            if not start<=r['day']<=end:continue
            try:
                h=json.loads(r['heat']);samples=r['n']
                if type(samples) is not int or samples<=0 or len(h)!=96 or any(type(v) is not int or v<0 for v in h) or not 0<=r['total']<=100*samples or sum(h)!=r['total']:continue
            except (ValueError,TypeError,KeyError):continue
            n+=samples;covered.add(r['day'])
            for i,v in enumerate(h):cells[i]+=v
        out[str(days)]={'days':days,'covered_days':len(covered),'samples':n,'cells':cells,'rates':[round(v/n,4) if n else None for v in cells]}
    return out

def _smart_trend(rows,today,hour):
    from datetime import timedelta
    from statistics import median
    # Compare only the latest completed local hour with the same hour on previous days.
    if hour<=0:return {'status':'insufficient','reason':'Bugünün ilk tam saati henüz tamamlanmadı.'}
    target=hour-1;start=today-timedelta(days=7);baseline=[];current=None
    for r in rows:
        if r['hour']!=target or r['n']<20 or not 0<=r['total']<=100*r['n']:continue
        day=r['day'];rate=r['total']/r['n']
        if day==today.isoformat():current=(rate,r['n'])
        elif start.isoformat()<=day<today.isoformat():baseline.append(rate)
    out={'status':'insufficient','hour':target,'baseline_days':len(baseline),'current_samples':current[1] if current else 0}
    if current is None or len(baseline)<5:
        out['reason']='Aynı tamamlanmış saat için bugün ve en az 5 geçmiş günde 20 analiz örneği gerekli.';return out
    ref=median(baseline);value,n=current;diff=value-ref
    out.update(current=round(value,3),baseline=round(ref,3),delta=round(diff,3),percent=round(100*diff/ref,1) if ref>0 else None,status='stable')
    if abs(diff)>=.2 and ((ref>0 and abs(diff)/ref>=.3) or (ref==0 and value>=.2)):
        out['status']='higher' if diff>0 else 'lower'
    out['reason']='Örnek başına algılama değişimi; kamera açısı, örnekleme veya model değişebilir. Sağlık, müşteri sayısı ve gerçek hareket teşhisi değildir.'
    return out

def _smart_report(module,owner,key,today=None,hour=None):
    from contextlib import closing
    from datetime import timedelta
    now=datetime.now().astimezone();today=today or now.date();hour=now.hour if hour is None else hour
    with closing(_wall_db(module,owner)) as c:
        rows=[dict(r) for r in c.execute('SELECT day,hour,n,total,heat FROM samples WHERE camera=? AND day BETWEEN ? AND ? ORDER BY day,hour',(key,(today-timedelta(days=29)).isoformat(),today.isoformat()))]
    return {'heatmaps':_smart_heat(rows,today),'trend':_smart_trend(rows,today,hour)}
