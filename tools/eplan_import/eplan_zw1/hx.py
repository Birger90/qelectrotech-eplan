import sys
p=sys.argv[1]; off=int(sys.argv[2]) if len(sys.argv)>2 else 0; n=int(sys.argv[3]) if len(sys.argv)>3 else 512
d=open(p,'rb').read()[off:off+n]
for i in range(0,len(d),16):
    c=d[i:i+16]
    print(f"{off+i:08x}  {' '.join(f'{b:02x}' for b in c):<48}  {''.join(chr(b) if 32<=b<127 else '.' for b in c)}")
