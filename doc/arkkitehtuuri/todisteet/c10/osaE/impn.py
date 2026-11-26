import struct,sys
p=sys.argv[1];want=[w.lower() for w in sys.argv[2:]]
b=open(p,'rb').read()
pe=struct.unpack_from('<I',b,0x3C)[0];nsec=struct.unpack_from('<H',b,pe+6)[0];osz=struct.unpack_from('<H',b,pe+20)[0];opt=pe+24
dd=opt+96
secs=[]
for i in range(nsec):
    o=opt+osz+i*40;vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8);secs.append((va,max(vs,rs),rp))
def off(r):
    for va,sz,rp in secs:
        if va<=r<va+sz:return r-va+rp
d=off(struct.unpack_from('<I',b,dd+8)[0])
while True:
    oft,_,_,nm,ft=struct.unpack_from('<IIIII',b,d)
    if nm==0:break
    o=off(nm);name=b[o:b.index(b'\0',o)].decode()
    if name.lower() in want:
        t=off(oft or ft);out=[]
        while True:
            v=struct.unpack_from('<I',b,t)[0]
            if v==0:break
            if v&0x80000000: out.append('#%d'%(v&0xffff))
            else:
                q=off(v)+2;out.append(b[q:b.index(b'\0',q)].decode())
            t+=4
        print(name,len(out));
        for x in out[:12]:print('   ',x)
    d+=20
