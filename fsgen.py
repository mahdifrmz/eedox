import sys

class Writer:

    def __init__(self):
        self.buffer = bytearray()

    def write_buf(self,index:int,arr:bytearray):
        while len(arr) + index >= len(self.buffer):
            self.buffer.append(0)
        for i in range(0,len(arr)):
            self.buffer[index+i] = arr[i]

    def write_byte(self,index:int,value:int):
        while index >= len(self.buffer):
            self.buffer.append(0)
        self.buffer[index] = value.to_bytes(1,'big')[0]

    def write_dword(self,index:int,value:int):
        while index+3 >= len(self.buffer):
            self.buffer.append(0)
        self.buffer[index+0] = value & 0x000000ff
        self.buffer[index+1] = (value & 0x0000ff00) >> 8
        self.buffer[index+2] = (value & 0x00ff0000) >> 16
        self.buffer[index+3] = (value & 0xff000000) >> 24

    def expand(self,size):
        while len(self.buffer) < size:
            self.buffer.append(0)
    def align(self,alignment):
        while len(self.buffer) % alignment != 0:
            self.buffer.append(0)

sector_alloc = 2

def alloc(s:int):
    if s%512 != 0:
        s -= s%512
        s += 512
    return int(s/512)

def gen_header(size:int,index:int,kind:int,child_count:int):
    writer = Writer()
    writer.write_dword(0,1)
    writer.write_dword(4,index)
    writer.write_dword(8,size)
    writer.write_dword(12,alloc(size))
    writer.write_dword(16,child_count)
    writer.write_dword(20,kind)
    writer.expand(512)
    return writer.buffer

if len(sys.argv) < 2:
    print('not enough args!')
    exit(1)

writer = Writer()
root_ptr = 3 * 512
index_indexes = []

for fn in sys.argv[2:]:
    fn = fn.split('/').pop()
    fnlen = len(fn)
    index_indexes.append(root_ptr + fnlen + 1)
    writer.write_buf(root_ptr,bytearray(fn,'ASCII'))
    root_ptr += fnlen + 5

index_indexes.reverse()
root_size = root_ptr - 3 * 512
root_alloc = alloc(root_size)
sector_alloc += (root_alloc + 1)

for fn in sys.argv[2:]:
    content = bytearray(open(fn,'rb').read())
    size = len(content)
    index = sector_alloc
    writer.write_buf(index*512,gen_header(size,sector_alloc,0,0))
    sector_alloc += 1
    writer.write_buf(sector_alloc*512,content)
    sector_alloc += alloc(size)
    writer.write_dword(index_indexes.pop(),index)    

writer.write_dword(0,sector_alloc)
writer.write_dword(1 * 512,2)
writer.write_buf(2*512,gen_header(root_size,2,1,len(sys.argv)-2))
writer.align(512)

open(sys.argv[1],'wb+').write(writer.buffer)