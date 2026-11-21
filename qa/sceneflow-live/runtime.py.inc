# SceneFlow live integration: local, owner-scoped, anonymous zone events.
# Embedded into existing signed app.py package; intentionally NO new runtime imports.
import hashlib as _sf_hashlib
from contextlib import closing as _sf_closing

_SF_LABELS = {
    'cafe': {'insan','kişi','person','human','people'},
    'pet': {'kedi','köpek','cat','dog','bird','kuş','pet'},
    'farm': {'inek','sığır','koyun','keçi','at','domuz','manda','cow','cattle','sheep','goat','horse','pig','buffalo','animal','hayvan'}
}
_SF_HISTORY_SECONDS = 3900
_SF_MAX_EVENTS = 12000
_SF_MAX_COVERAGE = 5000

def _sf_zone(name):
    # Stable opaque identifier; never leak human names/individual identities.
    return _sf_hashlib.sha256(str(name).encode('utf-8')).hexdigest()[:20]

def _sf_camera(module,owner,camera_id):
    if not isinstance(camera_id,str) or not 1<=len(camera_id)<=80 or not CAPTURE.fullmatch(camera_id):
        raise CloudError('Geçersiz kamera seçimi.',400)
    with _LOCK:
        active=_ip_cameras.get(module,{}).get(camera_id)
        if active is not None and active.get('owner')!=owner:active=None
        if active is None and camera_id not in _load_saved_cameras(module,owner):
            raise CloudError('Bu kameraya erişim izniniz yok.',404)
        return active

def _sf_database(module,owner):
    conn=_wall_db(module,owner)
    conn.execute('CREATE TABLE IF NOT EXISTS sceneflow_rules(camera TEXT NOT NULL, rule_id TEXT NOT NULL, payload TEXT NOT NULL, PRIMARY KEY(camera,rule_id))')
    conn.commit()
    return conn

def _sf_saved(module,owner,camera_id):
    with _sf_closing(_sf_database(module,owner)) as conn:
        values=[json.loads(r['payload']) for r in conn.execute('SELECT payload FROM sceneflow_rules WHERE camera=? ORDER BY rule_id',(camera_id,))]
    return [Rule.from_dict(v) for v in values]

def _sf_save(module,owner,camera_id,form):
    if not isinstance(form,dict) or set(form)!={'rules'} or not isinstance(form['rules'],list) or len(form['rules'])>8:
        raise CloudError('En fazla sekiz güvenli SceneFlow kuralı kaydedilebilir.',400)
    zones={_sf_zone(z.get('name')):str(z.get('name')) for z in _wall_config(module,owner,camera_id).get('regions',[])
           if isinstance(z,dict) and isinstance(z.get('name'),str)}
    clean=[]
    for item in form['rules']:
        if not isinstance(item,dict) or set(item)-{'rule_id','zone_id','event','condition','threshold','window_seconds','minimum_coverage'}:
            raise CloudError('SceneFlow kuralı beklenmeyen alanlar içeriyor.',400)
        if item.get('zone_id') not in zones or item.get('event') not in ('zone_enter','zone_exit'):
            raise CloudError('Önce geçerli bir alan seçin. Yalnızca alan giriş/çıkış olayları desteklenir.',400)
        try:
            rule=Rule.from_dict({**item,'camera_id':camera_id,'module':module,
                'subject':'person' if module=='cafe' else 'animal'})
        except (TypeError,ValueError,KeyError) as exc:
            raise CloudError('SceneFlow kuralındaki eşik, süre veya alan geçersiz.',400) from None
        if rule.window_seconds>1800:
            raise CloudError('Karşılaştırma için kural süresi en fazla 30 dakika olabilir.',400)
        clean.append(rule)
    if len({r.rule_id for r in clean})!=len(clean):
        raise CloudError('Kural kodları benzersiz olmalıdır.',400)
    with _sf_closing(_sf_database(module,owner)) as conn,conn:
        conn.execute('DELETE FROM sceneflow_rules WHERE camera=?',(camera_id,))
        conn.executemany('INSERT INTO sceneflow_rules(camera,rule_id,payload) VALUES(?,?,?)',
                         [(camera_id,r.rule_id,json.dumps(asdict(r),ensure_ascii=False,sort_keys=True)) for r in clean])
    with _LOCK:
        live=_ip_cameras.get(module,{}).get(camera_id)
        if live is not None and live.get('owner')==owner:
            live['sf_state']={}  # no stale observations across rule/zone edits
    return clean

def _sf_frame(camera,events,stamp,token):
    """Adapt only confirmed anonymous zone events from the existing tracker.

    Coverage counts only successive processed frames within 2.75 seconds;
    stream loss, stopped analysis and reconfiguration never count as observation.
    """
    module=camera['module']
    if module not in _SF_LABELS or not math.isfinite(stamp):return
    previous=camera.get('sf_state') or {}
    if previous.get('token')!=token or stamp<=previous.get('last',-float('inf')):
        previous=dict(token=token,last=None,coverage=[],events=[],overflow_time=None)
    last=previous.get('last')
    intervals=previous['coverage']
    if last is not None and 0<stamp-last<=2.75:
        if intervals and abs(intervals[-1].end-last)<1e-5:
            old=intervals[-1]
            intervals[-1]=Coverage(old.start,stamp,camera['id'],True)
        else:
            intervals.append(Coverage(last,stamp,camera['id'],True))
    previous['last']=stamp
    # An event is only usable if it happened during a confirmed frame interval.
    # The first frame after an outage has no continuity, so its events are not counted.
    zone_map={z['name']:_sf_zone(z['name']) for z in camera.get('config',{}).get('regions',[])
              if isinstance(z,dict) and isinstance(z.get('name'),str)}
    if last is not None and 0<stamp-last<=2.75:
        subject='person' if module=='cafe' else 'animal'
        for raw in events:
            if not isinstance(raw,(tuple,list)) or len(raw)<4:continue
            zone,kind,_,label=raw[:4]
            event_type={'Giriş':'zone_enter','Çıkış':'zone_exit'}.get(kind)
            if event_type and isinstance(label,str) and label.strip().casefold() in _SF_LABELS[module] and zone in zone_map:
                previous['events'].append(Event(stamp-min(0.001,(stamp-last)/2),camera['id'],module,zone_map[zone],subject,event_type))
    cutoff=stamp-_SF_HISTORY_SECONDS
    previous['events']=[e for e in previous['events'] if e.timestamp>=cutoff]
    previous['coverage']=[c for c in intervals if c.end>=cutoff]
    if len(previous['events'])>_SF_MAX_EVENTS or len(previous['coverage'])>_SF_MAX_COVERAGE:
        previous['overflow_time']=stamp
        previous['events']=previous['events'][-_SF_MAX_EVENTS:]
        previous['coverage']=previous['coverage'][-_SF_MAX_COVERAGE:]
    if previous.get('overflow_time') is not None and previous['overflow_time']<cutoff:
        previous['overflow_time']=None
    camera['sf_state']=previous

def _sf_result(rule, state, now):
    if not state or state.get('last') is None:
        return {'verdict':'unknown','reason':'Kamera analizi henüz yeterli veri üretmedi.',
                'coverage_ratio':0.,'observed_events':None,'validated_accuracy':None,
                'comparison':None,'proof_sha256':None}
    newest=state['last']
    # Compare only completed, non-overlapping rolling windows.
    current=evaluate(rule,state.get('events',()),state.get('coverage',()),now)
    prior=evaluate(rule,state.get('events',()),state.get('coverage',()),now-rule.window_seconds)
    if state.get('overflow_time') is not None and now-2*rule.window_seconds<=state['overflow_time']<now:
        return {'verdict':'unknown','reason':'Olay yoğunluğu ölçüm belleği sınırını aştı; sonuç doğrulanamıyor.',
                'coverage_ratio':current.coverage_ratio,'observed_events':None,
                'validated_accuracy':None,'comparison':None,'proof_sha256':None}
    # A digest cannot be taken as model validity, only as local transcript fingerprint.
    comparison=compare(prior,current)
    return {**current.public(),'comparison':{'enough_evidence':comparison.enough_evidence,
       'relative_change':comparison.relative_change,'interpretation':comparison.interpretation},
       'observed_at':'işlenen kamera kareleri','measured_accuracy':None}

def _sf_scope_for(camera_id):
    module,owner=_wall_alert_scope()
    live=_sf_camera(module,owner,camera_id)
    return module,owner,live

@app.route('/api/workspace/sceneflow/<camera_id>',methods=['GET','POST'])
def sceneflow_api(camera_id):
    module,owner,live=_sf_scope_for(camera_id)
    if request.method=='POST':
        # The existing app-wide anti-CSRF policy applies to every POST route.
        _vo_rate_limit(owner,'sceneflow',60,20)
        rules=_sf_save(module,owner,camera_id,request.get_json(silent=True))
        return jsonify(ok=True,saved=len(rules),note='Yerel olarak kaydedildi; yeni kamera örnekleri bekleniyor.')
    saved=_sf_saved(module,owner,camera_id)
    zones=[{'id':_sf_zone(r['name']),'name':r['name']} for r in _wall_config(module,owner,camera_id).get('regions',[])
           if isinstance(r,dict) and isinstance(r.get('name'),str)]
    # Snapshot live state under camera lock; never carry track IDs, frame bytes or credentials.
    with _LOCK:
        state=live.get('sf_state',{}) if live is not None else {}
        if state:state={**state,'events':list(state.get('events',())),'coverage':list(state.get('coverage',()))}
        online=bool(live and _camera_public_status(live)=='Canlı' and live.get('analysis'))
    now=time.monotonic()
    results=[{'rule':asdict(rule),'evidence':_sf_result(rule,state,now) if online else
              {'verdict':'unknown','reason':'Kamera bağlantısı veya analiz etkin değil.',
               'coverage_ratio':0.,'observed_events':None,'validated_accuracy':None,
               'comparison':None,'proof_sha256':None}} for rule in saved]
    response=jsonify(ok=True,online=online,engine='observed_zone_events_only',
          accuracy_calibrated=False,raw_video_uploaded=False,zones=zones,rules=results)
    response.headers['Cache-Control']='no-store'
    return response
