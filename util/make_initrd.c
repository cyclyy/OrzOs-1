#include "stdio.h"
#include "string.h"
#include "memory.h"

#define INITRD_MAGIC 0xFD
// 1 mb buffer length
#define BUF_LEN (1<<20)

typedef struct {
    char magic;
    char name[128]; 
    int offset;
    int length;
} __attribute__((packed)) initrd_file_header_t;

int main(int argc, char *argv[])
{
    char buf[BUF_LEN];
    int nfiles = argc-1;

    initrd_file_header_t header[100];
    
    FILE *ifile;
    int i;
    int offset;

    offset = sizeof(int) + sizeof(initrd_file_header_t)*nfiles;

    for (i=0; i<nfiles; i++) {
        header[i].magic =INITRD_MAGIC;
        strcpy(header[i].name, argv[i+1]);
        header[i].offset = offset;
        ifile = fopen(argv[i+1],"r");
        fseek(ifile, 0, SEEK_END);
        header[i].length = ftell(ifile);
        fclose(ifile);
        offset += header[i].length;
    }

    FILE *ofile = fopen("initrd.img","w");

    fwrite(&nfiles,sizeof(int),1,ofile);

    fwrite(header,sizeof(initrd_file_header_t),nfiles,ofile);

    for (i=0; i<nfiles; i++) {
        memset(buf, 0, BUF_LEN);

        ifile = fopen(argv[i+1],"r");
        fread(buf, header[i].length, 1, ifile);
        fclose(ifile);

        fwrite(buf, header[i].length, 1, ofile);
    }

    fclose(ofile);

    return 0;
}

