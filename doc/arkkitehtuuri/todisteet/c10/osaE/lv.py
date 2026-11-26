import struct,os
for p in ['Hk/V521/HkMaali.exe','Juk/V52/ViestiMaali.exe','Juk/V52/ViestiLuenta.exe','Hk/V521/HkKisaWin.exe']:
    b=open(os.path.join('C:/dev/tulospalvelu/TPexe',p),'rb').read()
    pe=struct.unpack_from('<I',b,0x3C)[0];opt=pe+24
    print(p,'linker',b[opt+2],b[opt+3],'dllchars',hex(struct.unpack_from('<H',b,opt+70)[0]),'chars',hex(struct.unpack_from('<H',b,pe+22)[0]))
