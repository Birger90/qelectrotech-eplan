import sys, os, json, pickle, collections; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from trace import load_types
from eodlib import *
types=load_types()
import json
SEED={int(k,16):v for k,v in json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','seed.json'))).items()}
import glob as _g
_PV=collections.defaultdict(collections.Counter)
for _f in _g.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','votes_%s_*.json'%os.environ.get('EOD_TAG','*'))):
    for k,c in json.load(open(_f)).items():
        for t,n in c.items(): _PV[int(k,16)][t]+=n
PTYPES={k:c.most_common(1)[0][0] for k,c in _PV.items()}; PTYPES.update(SEED)
out={}
for f in ['Connection.eod','Function.eod','Page.eod','Location.eod','Potential.eod','ArticleRef.eod','Article.eod','IPoint.eod','Relation.eod','Plc.eod']:
    res={}; st=collections.Counter()
    for r in records(f):
        e=dict(id=r['id'],cls=r['cls'],mode=r['mode'],props=None,err=None)
        if r['stream'] is None: e['err']=r['mode']; st['nostream']+=1
        else:
            tail=8 if r['mode']=='inline' else 0
            try:
                p,nt=parse_stream(r['stream'],types,learn=True,tail=tail,budget=50000)
                e['props']=p; st['ok' if not nt else 'ok_newtypes']+=1
                if nt: e['newtypes']={hex(k):v for k,v in nt.items()}
            except (Fail,Budget) as ex:
                try:
                    try: p,nt=parse_stream(r['stream'],PTYPES,learn=True,tail=tail,budget=50000)
                    except (Fail,Budget): p,nt=parse_stream(r['stream'],SEED,learn=True,tail=tail,budget=50000)
                    e['props']=p; e['fallback']=True; st['ok_fallback']+=1
                except (Fail,Budget) as ex2: e['err']=type(ex2).__name__; st['fail']+=1
        res[r['id']]=e
    out[f]=res; print(f,dict(st),flush=True)
import os
pickle.dump(out,open(os.environ.get('EOD_OUT','parsed.pkl'),'wb'))
