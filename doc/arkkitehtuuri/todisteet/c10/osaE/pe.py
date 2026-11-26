import struct,sys
def imports(p):
    b=open(p,'rb').read()
    pe=struct.unpack_from('<I',b,0x3C)[0]
    nsec=struct.unpack_from('<H',b,pe+6)[0]
    osz=struct.unpack_from('<H',b,pe+20)[0]
    opt=pe+24
    magic=struct.unpack_from('<H',b,opt)[0]
    dd=opt+(96 if magic==0x10b else 112)
    secs=[]
    s=opt+osz
    for i in range(nsec):
        o=s+i*40
        vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
        secs.append((va,max(vs,rs),rp))
    def off(r):
        for va,sz,rp in secs:
            if va<=r<va+sz: return r-va+rp
        return None
    def names(rva,step,namefield):
        out=[]
        if not rva: return out
        d=off(rva)
        while True:
            n=struct.unpack_from('<I',b,d+namefield)[0]
            if n==0:break
            o=off(n);e=b.index(b'\0',o);out.append(b[o:e].decode())
            d+=step
        return out
    imp=struct.unpack_from('<I',b,dd+8)[0]
    dl=struct.unpack_from('<I',b,dd+13*8)[0]
    ver=struct.unpack_from('<HH',b,opt+40)  # os ver
    subver=struct.unpack_from('<HH',b,opt+48)
    sub=struct.unpack_from('<H',b,opt+68)[0]
    return names(imp,20,12),names(dl,32,4),ver,subver,sub
for p in sys.argv[1:]:
    i,d,v,sv,sub=imports(p)
    print(p);print(' imports:',i);print(' delay:',d);print(' osver',v,'subsysver',sv,'subsys',sub)
