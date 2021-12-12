#ifndef STRBUF_H
#define STRBUF_H

#include <stdint.h>

typedef struct
{
    char *buffer;
    uint32_t size;
    uint32_t cap;
} strbuf;

void strbuf_push(strbuf *buf, char c);
char strbuf_pop(strbuf *buf);
void strbuf_init(strbuf *buf);
void strbuf_free(strbuf *buf);
void strbuf_pushstr(strbuf *buf, char *buffer, uint32_t len);
void strbuf_popstr(strbuf *buf, char *buffer, uint32_t len);

#endif