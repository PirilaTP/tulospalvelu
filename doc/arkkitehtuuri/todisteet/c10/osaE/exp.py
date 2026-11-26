import struct,sys
b=open(sys.argv[1],'rb').read()
pe=struct.unpack_from('<I',b,0x3C)[0]
nsec=struct.unpack_from('<H',b,pe+6)[0];osz=struct.unpack_from('<H',b,pe+20)[0];opt=pe+24
dd=opt+96
secs=[]
for i in range(nsec):
    o=opt+osz+i*40;vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8);secs.append((va,max(vs,rs),rp))
def off(r):
    for va,sz,rp in secs:
        if va<=r<va+sz:return r-va+rp
er=struct.unpack_from('<I',b,dd)[0];e=off(er)
n=struct.unpack_from('<I',b,e+24)[0];an=struct.unpack_from('<I',b,e+32)[0]
names=[]
for i in range(n):
    p=struct.unpack_from('<I',b,off(an)+4*i)[0];o=off(p);names.append(b[o:b.index(b'\0',o)].decode())
import re
pat=sys.argv[2]
print(len(names),[x for x in names if re.search(pat,x)])
