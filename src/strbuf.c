#include <strbuf.h>
#include <kutil.h>

void strbuf_push(strbuf *buf, char c)
{
    if (buf->size >= buf->cap)
    {
        buf->cap *= 2;
        char *newbuffer = kmalloc(buf->cap);
        memcpy(newbuffer, buf->buffer, buf->size);
        kfree(buf->buffer);
        buf->buffer = newbuffer;
    }
    buf->buffer[buf->size++] = c;
}
char strbuf_pop(strbuf *buf)
{
    char c = buf->buffer[0];
    for (uint32_t i = 0; i < buf->size - 1; i++)
    {
        buf->buffer[i] = buf->buffer[i + 1];
    }
    buf->size--;
    return c;
}
void strbuf_init(strbuf *buf)
{
    buf->size = 0;
    buf->cap = 4;
    buf->buffer = kmalloc(buf->cap);
}
void strbuf_free(strbuf *buf)
{
    kfree(buf->buffer);
}
void strbuf_pushstr(strbuf *buf, char *buffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        strbuf_push(buf, buffer[i]);
    }
}
void strbuf_popstr(strbuf *buf, char *buffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        buffer[i] = strbuf_pop(buf);
    }
}