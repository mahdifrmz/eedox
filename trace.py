import os,sys

class Writer:
    def __init__(self):
        self.buffer = bytearray()
    def write_dword(self,index:int,value:int):
        while index + 4 > len(self.buffer):
            self.buffer.append(0)
        bs = value.to_bytes(4,'little')
        for i in range(0,4):
            self.buffer[index + i] = bs[i]
    def write_str(self,index:int,value:str):
        while index + len(value) + 1 > len(self.buffer):
            self.buffer.append(0)
        bar = value.encode('ASCII')
        for i in range(0,len(bar)):
            self.buffer[index + i] = bar[i]
        self.buffer[index + len(bar)] = 0

class Table:
    def __init__(self,count):
        self.writer = Writer()
        self.rec_index = 4
        self.sym_index = count * 12 + self.rec_index
    def alloc(self,text:str):
        ptr = self.sym_index
        self.sym_index += len(text)+1
        self.writer.write_str(ptr,text)
        return ptr
    def addrec(self,ptr:int,name:int,address:int):
        self.writer.write_dword(self.rec_index, ptr)
        self.writer.write_dword(self.rec_index+4,name)
        self.writer.write_dword(self.rec_index+8,address)
        self.rec_index += 12

nm_out = os.popen('nm -nl build/kernel').readlines()
count = len(nm_out)
table = Table(count)

table.writer.write_dword(0,count)

for i in range(0,count):
    rec = nm_out[i].split()
    ty = rec[1]
    if ty == 'T' or ty == 't' or ty == 'R' or ty == 'r' :
        address = 0
        if(len(rec) > 3):
            address = table.alloc(rec[3].split('/').pop())
        table.addrec(
            int(rec[0],16),
            table.alloc(rec[2].rstrip()),
            address,
        )

output = table.writer.buffer
while len(output) < 0x8000:
    output.append(0)

os.write(sys.stdout.fileno(),output)