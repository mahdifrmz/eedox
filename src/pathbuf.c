#include <pathbuf.h>
#include <kstring.h>
#include <kutil.h>

pathbuf_t pathbuf_root()
{
    pathbuf_t buf;
    buf.fields = vec_new();
    buf.is_absolute = 1;
    buf.is_expldir = 1;
    buf.back = 0;
    return buf;
}
pathbuf_t pathbuf_copy(pathbuf_t *buf)
{
    pathbuf_t newbuf;
    newbuf.back = buf->back;
    newbuf.is_absolute = buf->is_absolute;
    newbuf.is_expldir = buf->is_expldir;
    newbuf.fields = vec_copy(&buf->fields);
    return newbuf;
}
pathbuf_t pathbuf_parse(const char *address)
{
    vec_t fields = vec_new();
    kstring_t buffer = kstring_new();
    pathbuf_t pathbuf;
    pathbuf.back = 0;
    if (*address == '/')
    {
        pathbuf.is_absolute = 1;
    }
    else
    {
        pathbuf.is_absolute = 0;
    }
    for (const char *p = address;; p++)
    {
        char c = *p;
        if (c != '/' && c != 0)
        {
            kstring_push(&buffer, c);
            continue;
        }

        kstring_fit(&buffer);
        if (strcmp(kstring_str(&buffer), "..") == 0)
        {
            if (vec_size(&fields) > 0)
            {
                vec_pop(&fields);
            }
            else if (!pathbuf.is_absolute)
            {
                pathbuf.back++;
            }
            kstring_clear(&buffer);
            pathbuf.is_expldir = 1;
        }
        else if (strcmp(kstring_str(&buffer), ".") == 0)
        {
            pathbuf.is_expldir = 1;
            kstring_clear(&buffer);
            pathbuf.is_expldir = 1;
        }
        else if (buffer.size)
        {
            vec_push(&fields, (uint32_t)kstring_str(&buffer)); // own the Kstring fields
            buffer = kstring_new();                            // release the previous fields
            if (c == 0)
            {
                pathbuf.is_expldir = 0;
            }
        }
        if (c == 0)
        {
            break;
        }
    }
    pathbuf.fields = fields;
    return pathbuf;
}
pathbuf_t pathbuf_join(pathbuf_t *buf1, pathbuf_t *buf2)
{
    if (buf2->is_absolute)
    {
        return pathbuf_root();
    }
    pathbuf_t res = pathbuf_copy(buf1);
    for (uint32_t i = 0; i < buf2->back; i++)
    {
        if (res.fields.size)
        {
            vec_pop(&res.fields);
        }
        else if (!res.is_absolute)
        {
            res.back++;
        }
    }
    for (uint32_t i = 0; i < buf2->fields.size; i++)
    {
        vec_push(&res.fields, buf2->fields.buffer[i]);
    }
    res.is_expldir = buf2->is_expldir;
    return res;
}
void pathbuf_free(pathbuf_t *buf)
{
    for (uint32_t i = 0; i < buf->fields.size; i++)
    {
        kfree((char *)buf->fields.buffer[i]);
    }
    vec_free(&buf->fields);
}