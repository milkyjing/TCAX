#include "tccmap.h"
#include <stdio.h>
#include <string.h>

typedef struct _tcc_string {
    char *buf;
} TccString, *TccStringPtr;


/* copy functions */
void copy_func(TccStringPtr dst, const TccStringPtr src) {
    int len = strlen(src->buf) + 1;
    dst->buf = (char *)malloc(len);
    memcpy(dst->buf, src->buf, len);
}

/* cleanup functions */
void clean_func(TccStringPtr src) {
    free(src->buf);
}

/* compare functions */
int compare_func(const TccStringPtr dst, const TccStringPtr src) {
    return strcmp(dst->buf, src->buf);
}


int main() {
    TccString key, value, val;
    TccMapPtr map;
    tcc_map_create(&map, sizeof(TccString), sizeof(TccString), 0, (tcc_map_key_copy_func)copy_func, (tcc_map_value_copy_func)copy_func, (tcc_map_key_cleanup_func)clean_func, (tcc_map_value_cleanup_func)clean_func, (tcc_map_key_compare_func)compare_func, (tcc_map_value_compare_func)compare_func);
    key.buf = (char *)malloc(5);
    strcpy(key.buf, "fuck");
    value.buf = (char *)malloc(5);
    strcpy(value.buf, "you");
    tcc_map_push_back(map, &key, &value);
    tcc_map_push_back(map, &key, &value);
    strcpy(value.buf, "youd");
    tcc_map_push_back(map, &key, &value);
    tcc_map_set_offset(map, 2);
    tcc_map_retrieve(map, &key, &val);
    printf(val.buf);
    tcc_map_destroy(map);
    return 0;
}

