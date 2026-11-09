import os
import sys,collections; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from resolve import *
I=P['IPoint.eod']; Pot=P['Potential.eod']
SIDES={'A':(0xeb2b,0xeb2c,0xfa03),'B':(0xeb2e,0xeb2f,0xfa04)}
def strp(d,pid):
    return d[pid][0] if pid in d else None
def resolve_ep(d,s):
    fr,ix,txt=SIDES[s]
    out=dict(txt=strp(d,txt))
    if fr not in d: out.update(kind='keine Referenz'); return out
    cls,tid=d[fr][0]; i=d[ix][0] if ix in d else 0
    out.update(refcls=cls,refid=tid,pinidx=i)
    if not isinstance(i,int):
        # Strom falsch zerlegt (beobachtet bei verlegten 3D-Verbindungen: an der Stelle steht eine x/y/z-Wegliste)
        out.update(kind='Anschlussindex nicht lesbar',pinidx=None,calc=None); return out
    if cls==17 or tid in F:
        e=F.get(tid)
        if not e or not e['props']: out.update(kind='Funktion (nicht parsebar)'); return out
        b,fd=bmk(tid); pl=pins(fd); k6=kvd(fd,0xea6b)
        x=(k6.get((20447,0)) or '')+(k6.get((20406,0)) or k6.get((20030,0)) or strp(fd,0xeb22) or '')
        pn=pl[i] if 0<=i<len(pl) else None
        out.update(kind='Funktion',bmk=b,x=x,pin=pn,pins=pl,fcls=e['cls'])
        bb=b[1:] if b.startswith('-') else b   # EPLAN-Endpunkttext: ohne Strukturkennzeichen kein '-' (in 13/13 Fällen, EPLAN 2.9)
        out['calc']=bb+(':'+x if x else '')+(':'+pn if pn else '')
    elif cls==93:
        e=Pot.get(tid); fd=pd(e); out.update(kind='Potential',calc=strp(fd,0xf0fb))
    elif cls==70:
        e=I.get(tid); fd=pd(e); out.update(kind='IPoint',ipprops={hex(p[0]):p[2] for p in (e['props'] or []) if p[1] in('str','kv','ref')} if e else None)
        out['calc']=None
    else:
        out.update(kind='Instance-Objekt (Klasse %d)'%cls,calc=None)
    return out
if __name__=='__main__':
    st=collections.Counter(); bad=[]
    for cid,e in C.items():
        d=pd(e)
        for s in 'AB':
            ep=resolve_ep(d,s)
            k=ep['kind']
            if ep['txt'] is None: st[(k,'kein EPLAN-Text','berechnet' if ep.get('calc') else 'nicht berechnet')]+=1
            elif ep.get('calc')==ep['txt']: st[(k,'Text','MATCH')]+=1
            else: st[(k,'Text','MISMATCH')]+=1; bad.append((s,hex(cid),ep['txt'],ep.get('calc'),ep.get('pinidx'),ep.get('pins')))
    for k,v in sorted(st.items()): print(v,k)
    for b in bad[:30]: print(b)
