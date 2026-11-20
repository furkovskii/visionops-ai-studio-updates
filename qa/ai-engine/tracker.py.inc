def _wall_track(previous, detections, regions, stamp, module=None):
    """Anonymous, per-camera motion-aware tracks. No biometric or cross-camera ID.
    Durations count only consecutive observations, never missing frames or downtime.
    """
    from math import hypot, isfinite
    if not isfinite(stamp): raise ValueError('Invalid observation time')
    max_gap=2.75
    tracks={k:dict(v,visible=False,candidate={}) for k,v in previous.items() if 0<stamp-v['last']<=5}
    available=set(tracks); assigned=[]; events=[]; matches={}
    pairs=[]
    for i,(label,x,y) in enumerate(detections):
        if not all(isfinite(v) and 0<=v<=1 for v in (x,y)):raise ValueError('Invalid observation coordinates')
        for k,t in tracks.items():
            if t['label']!=label:continue
            dt=stamp-t['last']; horizon=min(dt,2.)
            vx,vy=t.get('velocity',(0.,0.))
            px=max(0.,min(1.,t['x']+vx*horizon));py=max(0.,min(1.,t['y']+vy*horizon))
            dist=hypot(x-px,y-py);raw=hypot(x-t['x'],y-t['y'])
            # Predict through crossings; bound extrapolation and reject large jumps.
            if dist<=min(.20,.075+.045*dt) and raw<=min(.35,.12+.09*dt):
                pairs.append((dist+.12*raw,i,k))
    for _,i,k in sorted(pairs):
        if i not in matches and k in available:matches[i]=k;available.remove(k)
    for i,(label,x,y) in enumerate(detections):
        k=matches.get(i) or uuid.uuid4().hex[:16]
        old=previous.get(k,{})
        dt=stamp-old.get('last',stamp)
        continuous=bool(old.get('visible',True) and old and 0<dt<=max_gap)
        inside=dict(old.get('inside',{})); candidate=dict(old.get('candidate',{})) if continuous else {}
        elapsed=dict(old.get('elapsed',{}))
        if not continuous:elapsed={name:0. for name in inside}
        for r in regions:
            name=r['name']; a,b,w,h=r['bounds']; present=a<=x<=a+w and b<=y<=b+h
            # A gap is not proof of crossing a region boundary. Re-establish
            # membership silently if an existing track reappears elsewhere.
            if old and not continuous and present!=(name in inside):
                if not present:inside.pop(name,None);elapsed.pop(name,None)
                else:candidate[name]=(True,stamp,True)
                continue
            if present==(name in inside):
                if present and continuous and name not in candidate:elapsed[name]=elapsed.get(name,0.)+dt
                candidate.pop(name,None);continue
            prior=candidate.get(name)
            if prior is None or prior[0]!=present:candidate[name]=(present,stamp);continue
            candidate.pop(name,None)
            if present:
                inside[name]=prior[1];elapsed[name]=stamp-prior[1]
                if len(prior)<3 or not prior[2]:events.append((name,'Giriş',k,label,None))
            else:
                inside.pop(name,None);seconds=round(elapsed.pop(name,0.),1);events.append((name,'Çıkış',k,label,seconds))
        velocity=(0.,0.)
        if continuous:
            dx=(x-old['x'])/dt;dy=(y-old['y'])/dt
            velocity=(max(-.14,min(.14,dx)),max(-.14,min(.14,dy)))
        motion={'anchor':(x,y),'still':0.,'latched':False,'move_hits':0}
        if continuous:
            motion={**motion,**old.get('motion',{})}
            ax,ay=motion['anchor'];moved=hypot(x-ax,y-ay)>.025
            if moved:
                motion['still']=0.;motion['move_hits']+=1
                if motion['move_hits']>=2:
                    if motion['latched'] and module in ('pet','farm'):
                        events.append(('Hareket analizi','Hareket yeniden gözlendi',k,label,None))
                    motion.update(anchor=(x,y),latched=False,move_hits=0)
            else:
                motion['move_hits']=0;motion['still']+=dt
                if motion['still']>=30 and not motion['latched'] and module in ('pet','farm'):
                    motion['latched']=True
                    events.append(('Hareket analizi','Hareketsizlik adayı',k,label,round(motion['still'],1)))
        tracks[k]={'label':label,'x':x,'y':y,'last':stamp,'inside':inside,'candidate':candidate,'elapsed':elapsed,
                   'sample_index':i,'velocity':velocity,'visible':True,'continuous':continuous,'motion':motion}
        assigned.append(tracks[k])
    rows=[{'name':r['name'],'kind':r['kind'],'count':sum(r['name'] in t['inside'] for t in assigned),
           'seconds':round(max((t['elapsed'].get(r['name'],0.) for t in assigned if r['name'] in t['inside']),default=0.),1)} for r in regions]
    return tracks,rows,events
