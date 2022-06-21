const fs = require('fs')
const path = require('path')

const NODEKIND_FILE =  1
const NODEKIND_DIR  = 2

function sizeToAlloc(s)
{
    if(s % 512 == 0)
    {
        return s / 512
    }
    else
    {
        return (s - s % 512) / 512 + 1
    }
}

function loadFile(file)
{
    let buf;
    if(file.bin != undefined)
    {
        buf = fs.readFileSync(`./build/user/${file.bin}`)
    }
    else{
        buf = Buffer.from(file.text)
    }
    const size = buf.length
    buf = Buffer.concat([buf,Buffer.alloc(sizeToAlloc(buf.length)*512 - buf.length)])
    return {
        kind:NODEKIND_FILE,
        content:buf,
        size
    }
}

const binlist = [
	'sh',
	'echo',
	'cat',
	'ls',
	'tee',
	'mkdir',
	'wc',
	'stat',
	'cp',
]

let bins = {}

for(const b of binlist)
{
    bins[b] = {kind:NODEKIND_FILE,bin:b}
}

let fs_tree = {
    children:{
        home:{kind:NODEKIND_DIR,children:{
            sample:{kind:NODEKIND_FILE,bin:'sample'},
            upclnt:{kind:NODEKIND_FILE,bin:'upclnt'},
        }},
        bin:{kind:NODEKIND_DIR,children:bins},
        etc:{kind:NODEKIND_DIR,children:{
            init:{kind:NODEKIND_FILE,bin:'init'},
            upcd:{kind:NODEKIND_FILE,bin:'upcd'},
            services:{kind:NODEKIND_FILE,text:'/etc/upcd\n'},
        }}
    },
    kind:NODEKIND_DIR
}

function loadTree(node)
{
    if(node.kind == NODEKIND_FILE)
    {
        return loadFile(node)
    }
    else
    {
        for(const e in node.children)
        {
            node.children[e] = loadTree(node.children[e])
        }
        return node
    }
}

fs_tree = loadTree(fs_tree)

let nodes = []

function collect(tree)
{
    nodes.push(tree)
    if(tree.kind == NODEKIND_DIR)
    {
        for(c in tree.children)
        {
            collect(tree.children[c])
        }
    }
}

collect(fs_tree)

for (let i=0;i<nodes.length;i++)
{
    let node = nodes[i]
    if(node.kind == NODEKIND_DIR)
    {
        node.size = 0
        for(c in node.children)
        {
            node.size += c.length + 5
        }
    }
    node.alloc = sizeToAlloc(node.size)
}

let allocPtr = 2

for(let i=0;i<nodes.length;i++)
{
    let node = nodes[i]
    node.index = allocPtr
    allocPtr += (node.alloc + 1)
}

for(let i=0;i<nodes.length;i++)
{
    let node = nodes[i]
    if(node.kind == NODEKIND_DIR)
    {
        let content = Buffer.alloc(0)
        for(c in node.children)
        {
            const chnamelen = c.length
            const chindex = node.children[c].index
            let entry = Buffer.alloc(chnamelen + 5)
            entry.write(c,'ascii')
            entry.writeUInt8(0,chnamelen)
            entry.writeUInt32LE(chindex,chnamelen + 1)
            content = Buffer.concat([content,entry])
        }
        content = Buffer.concat([content,Buffer.alloc(sizeToAlloc(content.length)*512 - content.length)])
        node.content = content
    }
}

for(let i=0;i<nodes.length;i++)
{
    let node = nodes[i]
    let header = Buffer.alloc(512)
    header.writeUInt32LE(1,0); // validity
    header.writeUInt32LE(node.index,4); // index
    header.writeUInt32LE(node.size,8); // size
    header.writeUInt32LE(node.alloc,12); // size
    header.writeUInt32LE(node.kind == NODEKIND_DIR ? Object.keys(node.children).length : 0,16); // child count
    header.writeUInt32LE(node.kind == NODEKIND_DIR ? 1 : 0,20); // child count
    node.header = header
}

nodes.sort((n1,n2)=>{
    if(n1.index < n2.index)
    {
        return -1
    }
    else if(n1.index == n2.index)
    {
        return 0
    }
    else{
        return 1
    }
})

let allocPtrBlock = Buffer.alloc(512)
allocPtrBlock.writeUInt32LE(allocPtr,0)

let rootPtrBlock = Buffer.alloc(512)
rootPtrBlock.writeUInt32LE(fs_tree.index,0)

let buffers = []

buffers.push(allocPtrBlock)
buffers.push(rootPtrBlock)

for(let i=0;i<nodes.length;i++)
{
    let node = nodes[i]
    buffers.push(node.header)
    buffers.push(node.content)
}

let fsBuffer = Buffer.concat(buffers)
fs.writeFileSync(process.argv[2],fsBuffer)