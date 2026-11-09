import os
import sys,os,pickle; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from trace import load_types
from eodlib import *
from resolve3 import C,pd
types=load_types(); ids=set()
for cid,e in C.items():
    d=pd(e)
    for fr in (0xeb2b,0xeb2e):
        if fr in d and d[fr][0][0]==130: ids.add(d[fr][0][1])
INS={}
for r in records('Instance.eod'):
    if r['id'] in ids and r['stream']:
        try: INS[r['id']]=parse_stream(r['stream'],types,tail=8 if r['mode']=='inline' else 0,budget=50000)[0]
        except Exception: INS[r['id']]=None
pickle.dump(INS,open(os.environ['EOD_INS'],'wb')); print('ins130',len(ids),sum(1 for v in INS.values() if v))
