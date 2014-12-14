#include <stdio.h>
#include <Windows.h>

int main() {
    DWORD t1, t2;
    int i, size;
    char *buf;
    /*FILE *fp;
    unsigned char *buf;
    fp = fopen("J:\\Program Copies\\ubuntu-10.10-desktop-i386.iso", "rb");
    if (!fp)
        return 0;
    size = 64 * 1024 * 1024;    /* 64 MB 
    buf = (unsigned char *)malloc(size);
    t1 = GetTickCount();
    for (i = 0; i < 1; i++)    /* 300 MB, can be read within about 3000 sec! 
        fread(buf, sizeof(unsigned char), size, fp);
    t2 = GetTickCount();
    printf("%i\n", t2 - t1);
    fclose(fp);
    free(buf); */
    size = 100 * 1024;  /* 100 KB */
    t1 = GetTickCount();
    for (i = 0; i < 10000; i++) {
        buf = (char *)malloc(size);
        free(buf);
    }
    t2 = GetTickCount();
    printf("%i\n", t2 - t1);
    return 0;
}

