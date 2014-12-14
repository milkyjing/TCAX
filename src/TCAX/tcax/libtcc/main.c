#include "string.h"
#include <stdio.h>

int main() {
    String s;
    string_init(&s, 0);
    string_append(&s, L"GoFuck!", wcslen(L"GoFuck!"));
    string_append_int(&s, 111);
    string_append_double(&s, 43425.402349200);
    string_push_back(&s, L'\n');
    wprintf(L"%i\n", string_get_size(&s));
    wprintf(s.buffer);
    return 0;
}

