const fs = require('fs')
const { exit, stdout } = require('process')
const crypto = require('crypto')

const imgpath = process.argv[2]
const buffer = fs.readFileSync(imgpath)

const NODEKIND_FILE =  1
const NODEKIND_DIR  = 2

let errorlist = []

const Error = {
    InvalidChildName:1,
    ChildTableOutOfRange:2,
    HeaderInvalid:4,
    HeaderChildren:5,
    HeaderAlloc:6,
    HeaderFormatValidity:7
}

function readh(idx)
{
    const content = buffer.slice(idx*512,(idx+1)*512);
    
    const validity = content.readInt32LE(0);
    if(validity != 1 && validity != 0)
    {
        return {header:null,error:Error.HeaderFormatValidity}
    }
    const index = content.readInt32LE(4);
    const size = content.readInt32LE(8);
    const alloc = content.readInt32LE(12);
    const children = content.readInt32LE(16);
    const kind = content.readInt32LE(20) == 1 ? NODEKIND_DIR : NODEKIND_FILE;

    return {header:{index,validity,kind,size,alloc,children},error:0}
}

function readch(idx,size)
{
    const children = [];
    const end = idx * 512 + size;
    let ptr = idx * 512;
    let name = '';
    while(ptr < end)
    {
        while(1)
        {
            const b = buffer[ptr++]
            if(b == 0)
                break;
            name += String.fromCharCode(b);
        }
        if(name == '')
            return {error:Error.InvalidChildName,children:null};
        children.push({
            name,
            idx:buffer.readInt32LE(ptr)
        });
        ptr += 4;
        name='';
    }
    if(ptr > end)
        return {error:Error.InvalidChildName,children:null};
    return {error:0,children};
}

function readn(idx,path)
{
    const {header,error} = readh(idx);
    if(error)
    {
        const r = {error,path}
        errorlist.push(r)
        return r
    }
    if(!header.validity)
    {
        const r = {error:Error.HeaderInvalid,path};
        errorlist.push(r)
        return r
    }
    if(header.kind == NODEKIND_DIR)
    {
        let dir = {};
        const {children,error} = readch(idx+1,header.size);
        if(error > 0)
        {
            const r = {error,path};
            errorlist.push(r)
            return r
        }
        let ret = null;
        for(const c of children)
        {
            const { node, error, path : cpath} = readn(c.idx,`${path}/${c.name}`);
            if(error > 0)
            {
                ret = {error,path:cpath};
            }
            dir[c.name] = node;
        }
        return ret || {node:dir,error:0};
    }
    else{
        const beg = (idx+1)*512;
        return {
            node: buffer.slice(beg, beg+header.size),
            error:0
        };
    }
}

function space(n)
{
    s=''
    for(let i=0;i<n;i++) s += ' '
    return s
}

function showNode(node,depth)
{
    if(node instanceof Buffer)
    {
        const hash = crypto.createHash('md5').update(node).digest('hex').slice(0,6)
        stdout.write(`(${node.length}) hash = ${hash}`)
    }
    else{
        for (const c in node)
        {   
            stdout.write(`\n${space(depth*4)}+${c}: `);
            showNode(node[c],depth+1);
        }
    }
}

const allocptr = buffer.readInt32LE(0);
const rootptr = buffer.readInt32LE(512);

let {error , node, path} = readn(rootptr,'');
if(error > 0)
{
    console.error('error:',errorlist);
    exit(1)
}
if(process.argv.length > 3)
{
    let path = process.argv[3]
    path = path.split('/')
    for(const p of path)
    {
        node = node[p]
    }
}
if(node instanceof Buffer)
    stdout.write(node)
else{
    console.log({allocptr,rootptr})
    showNode(node,0)
}