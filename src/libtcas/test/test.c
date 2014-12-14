#include <stdio.h>
#include <wchar.h>

int main() {
    //signed short i;
    //i = 0x8000;
    //printf("%i\n", i);
    wchar_t *s = L"; Welcome to TCAX forum http://tcax.rhacg.com\r\n";
    printf("%i\n", wcslen(s));
    return 0;
}

