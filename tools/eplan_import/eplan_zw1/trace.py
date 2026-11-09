import os
import sys, json, collections, glob; sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from eodlib import *
def load_types(extra={}):
    V=collections.defaultdict(collections.Counter)
    for f in glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','votes_*.json')):
        for k,c in json.load(open(f)).items():
            for t,n in c.items(): V[int(k,16)][t]+=n
    t={k:c.most_common(1)[0][0] for k,c in V.items()}
    t.update({int(k,16):v for k,v in json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'daten_typen','seed.json'))).items()})
    t.update(extra); return t
def trace(buf,types,maxn=400):
    pos=0; depth=0; npin=0; out=[]
    while pos+2<=len(buf) and len(out)<maxn:
        pid=struct.unpack_from('<H',buf,pos)[0]
        if pid==0 and depth==0: out.append(('END',pos)); break
        if pid==0xffff:
            if depth>0: depth-=1; out.append(('  '*depth+'}',)); pos+=2; continue
            out.append(('SEP',)); pos+=2; continue
        if pid==0x0380:
            x=struct.unpack_from('<I',buf,pos+2)[0]; n=struct.unpack_from('<I',buf,pos+7)[0]; p2=pos+11; vals=[]
            for _ in range(n):
                v,p2=rd(buf,p2,'str'); vals.append(v)
            out.append(('  '*depth+'0x380 arrS %r'%((x,buf[pos+6],vals),),)); pos=p2; continue
        t=types.get(pid)
        if t is None: out.append(('UNKNOWN',hex(pid),pos,buf[pos:pos+24].hex())); break
        if t=='obj':
            oc=struct.unpack_from('<H',buf,pos+2)[0]; out.append(('  '*depth+'%s obj %s {'%(hex(pid),hex(oc)),)); depth+=1; pos+=4; continue
        if t=='dN':
            v=[];p2=pos+2
            while True:
                nx=struct.unpack_from('<H',buf,p2)[0]
                if nx>=0xe000 and nx!=0xffff and nx in types and types[nx]!='dN': break
                v.append(struct.unpack_from('<d',buf,p2)[0]); p2+=8
        else:
            try: v,p2=rd(buf,pos+2,t)
            except Fail: out.append(('FAIL',hex(pid),t,pos,buf[pos:pos+24].hex())); break
        out.append(('  '*depth+'%s %s %r'%(hex(pid),t,v),)); pos=p2
    return out
if __name__=='__main__':
    f=sys.argv[1]; idx=int(sys.argv[2])
    types=load_types(json.loads(sys.argv[3]) if len(sys.argv)>3 else {})
    r=list(records(f))[idx]
    for o in trace(r['stream'],types): print(*o)
