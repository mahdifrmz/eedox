#ifndef DGT_H
#define DGT_H

typedef struct
{
    unsigned short length;
    unsigned short base_l;
    unsigned char base_m;
    unsigned char flags;
    unsigned char fllen;
    unsigned char base_h;
} __attribute__((packed)) gdtrec;

typedef struct
{
    unsigned short len;
    gdtrec *ptr;
} __attribute__((packed)) gdtarray;

void load_gdt_recs(gdtrec *gdt_records);

#endif