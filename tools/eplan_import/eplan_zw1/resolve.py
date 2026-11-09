import pickle,collections,sys
import os
P=pickle.load(open(os.environ.get('EOD_OUT','parsed.pkl'),'rb'))
F=P['Function.eod']; C=P['Connection.eod']
def pd(e):
    d=collections.defaultdict(list)
    if e and e['props']:
        for pid,t,v in e['props']: d[pid].append(v)
    return d
def kvd(d,pid):
    out={}
    for lst in d.get(pid,[]):
        for k,i,s in lst: out.setdefault((k,i),s)
    return out
def pins(d):
    p=[v for v in d.get(0xeb19,[])]
    if not p and 0x0380 in d: p=d[0x0380][0][2]
    return p
def bmk(fid):
    e=F.get(fid); d=pd(e); kv=kvd(d,0xeaf6)
    s=''
    if (1100,0) in kv: s+='='+kv[(1100,0)]
    if (1101,0) in kv: s+='.'+kv[(1101,0)]
    if (1200,0) in kv: s+='+'+kv[(1200,0)]
    if (1201,0) in kv: s+='.'+kv[(1201,0)]
    if (20013,0) in kv or (20014,0) in kv: s+='-'+kv.get((20012,0),'')+kv.get((20013,0),'')+kv.get((20014,0),'')
    if (20015,0) in kv: s+='.'+kv[(20015,0)]
    if (20017,0) in kv or (20018,0) in kv: s+='-'+kv.get((20017,0),'')+kv.get((20018,0),'')
    return s,d
if __name__=='__main__':
    stat=collections.Counter(); bad=[]
    for cid,e in C.items():
        d=pd(e)
        for side,(fr,pr,txt) in {'A':(0xeb2b,0xeb2d,0xfa03),'B':(0xeb2e,0xeb30,0xfa04)}.items():
            if fr not in d: stat[side+':noref']+=1; continue
            fid=d[fr][0][1]; pin=d[pr][0] if pr in d else None
            b,fd=bmk(fid); pl=pins(fd)
            pn=pl[pin-1] if pin and 0<pin<=len(pl) else None
            k6=kvd(fd,0xea6b); x=k6.get((20406,0))
            cand=b+(':'+x if x else '')+(':'+pn if pn else '')
            if txt in d:
                t=d[txt][0]
                if t==cand: stat[side+':match']+=1
                else: stat[side+':mismatch']+=1; bad.append((side,hex(cid),t,cand,pin,pl,F.get(fid,{}).get('cls')))
            else: stat[side+':notxt']+=1
    print(dict(stat))
    for b in bad[:25]: print(b)
