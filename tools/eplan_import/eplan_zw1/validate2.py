import csv,collections,sys,re,json
tot=collections.Counter(); res={}
for p in sys.argv[1:]:
    R=list(csv.DictReader(open(f'/home/claude/{p}/out/Verbindungen_aufgeloest.csv',encoding='utf-8-sig'),delimiter=';'))
    st=collections.Counter(); ex=collections.defaultdict(list)
    for r in R:
        a=bool(r['A_berechnet']); b=bool(r['B_berechnet'])
        st['verbindungen']+=1; st['beidseitig' if a and b else 'einseitig' if a or b else 'keine']+=1
        for s in 'AB':
            t=r[s+'_eplan_text']; c=r[s+'_berechnet']
            if not t: continue
            st['enden_mit_text']+=1
            if t==c: st['identisch']+=1
            elif t.endswith(c) and t[:len(t)-len(c)] in ('=','=+','+'): st['nur_leeres_praefix']+=1
            elif not c and 'nicht parsebar' in r[s+'_typ']: st['verweis_ins_leere']+=1
            else: st['abweichend']+=1; ex[p].append((r['connection_id'],s,t,c))
    res[p]=dict(st); tot.update(st)
    print(p,dict(st)); [print('   ',x) for x in ex[p][:6]]
print('GESAMT',dict(tot))
json.dump({'projekte':res,'gesamt':dict(tot)},open('/home/claude/validierung.json','w'),indent=1)
