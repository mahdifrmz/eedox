#ifndef PATHBUF_H
#define PATHBUF_H

#include <vec.h>

typedef struct
{
    vec_t fields;
    uint8_t is_absolute;
    uint8_t is_expldir;
    uint32_t back;
} pathbuf_t;

pathbuf_t pathbuf_root();
void pathbuf_free(pathbuf_t *buf);
pathbuf_t pathbuf_parse(const char *address);
pathbuf_t pathbuf_join(pathbuf_t *buf1, pathbuf_t *buf2);
pathbuf_t pathbuf_copy(pathbuf_t *buf);

#endif