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
char *pathbuf_field(pathbuf_t *buf, uint32_t index);
char *pathbuf_name(pathbuf_t *buf);
uint8_t pathbuf_cmp(pathbuf_t *buf1, pathbuf_t *buf2);
uint32_t pathbuf_len(pathbuf_t *buf);
pathbuf_t pathbuf_parent(pathbuf_t *buf);
pathbuf_t pathbuf_child(pathbuf_t *buf, const char *name, uint8_t expldir);

#endif