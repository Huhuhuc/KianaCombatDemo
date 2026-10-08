import struct, zipfile
z=zipfile.ZipFile('C:/Users/Huhuhu/Downloads/战斗策划【miHoYo-2027实习】.zip')
b=z.read('KianaResources/Model/Avatar_Kiana_C2_Model.FBX')
wide=struct.unpack_from('<I',b,23)[0]>=7500
def node(o):
    end,n,pl=struct.unpack_from('<QQQ' if wide else '<III',b,o);o+=24 if wide else 12
    if not end:return None,o+1
    ln=b[o];o+=1;name=b[o:o+ln].decode();o+=ln;p=[]
    for _ in range(n):
        t=chr(b[o]);o+=1
        if t in 'SR':
            l=struct.unpack_from('<I',b,o)[0];o+=4;p.append(b[o:o+l].decode(errors='replace') if t=='S' else None);o+=l
        elif t in 'fdlibc':
            _,_,l=struct.unpack_from('<III',b,o);o+=12+l;p.append(None)
        else:
            f={'L':'q','I':'i','D':'d','F':'f','C':'?','Y':'h'}[t];p.append(struct.unpack_from('<'+f,b,o)[0]);o+=struct.calcsize(f)
    children=[]
    while o<end-(25 if wide else 13):
        c,o=node(o)
        if c:children.append(c)
    return (name,p,children),end
nodes=[];o=27
while o<len(b):
    c,o=node(o)
    if not c:break
    nodes.append(c)
objs=next(x for x in nodes if x[0]=='Objects')[2]
cons=next(x for x in nodes if x[0]=='Connections')[2]
ids={x[1][0]:x for x in objs if x[1]}
for x in objs:
    if x[0]=='Material':
        print('MATERIAL',repr(x[1][0]))
        for c in cons:
            if len(c[1])>2 and c[1][2]==x[1][0]:
                src=ids.get(c[1][1])
                if src and src[0]=='Texture': print(c[1][3:],[ch[1] for ch in src[2] if ch[0]=='RelativeFilename'])
