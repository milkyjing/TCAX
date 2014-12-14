#include "mapfile.h"
#include <stdio.h>


int main() {
    HANDLE hMapFile;
    unsigned long *buf;
    buf = libtcas_mapfile("test.tcas", &hMapFile, NULL);
    if (buf) {
        printf("%08X\n", buf[0]);
        libtcas_unmapfile(hMapFile, buf);
    }
    return 0;
}

