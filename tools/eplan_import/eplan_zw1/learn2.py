import os
import sys, json, collections; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from eodlib import *
import json
seed={int(k,16):v for k,v in json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','seed.json'))).items()}
votes=collections.defaultdict(collections.Counter)
res={}
files=sys.argv[1:]
for f in files:
    ok=0;fl=0;bd=0;none=0
    for r in records(f):
        if r['stream'] is None: none+=1; continue
        tail=8 if r['mode']=='inline' else 0
        try:
            p,nt=parse_stream(r['stream'],seed,tail=tail,budget=20000)
            for k,v in nt.items(): votes[k][v]+=1
            ok+=1
        except Budget: bd+=1
        except Fail: fl+=1
    print(f,'ok',ok,'fail',fl,'budget',bd,'nostream',none,flush=True)
amb={hex(k):dict(c) for k,c in votes.items() if len(c)>1}
print('pids',len(votes),'ambiguous',len(amb))
for k,c in list(amb.items())[:40]: print(k,c)
json.dump({hex(k):dict(c) for k,c in votes.items()},open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','votes_%s_%s.json')%(os.environ.get('EOD_TAG','p1'),'_'.join(x.split('.')[0] for x in files)),'w'))
