#include "strmap.h"


int libtcc_write_unicode_to_file(const char *filename, const wchar_t *buffer, int count) {
    FILE *fp;
    wchar_t bom[1];
    fp = fopen(filename, "wb");
    if (!fp) return -1;
    bom[0] = 0xFEFF;
    if (buffer[0] != bom[0]) {
        if (fwrite(bom, sizeof(wchar_t), 1, fp) != 1) {
            fclose(fp);
            return -1;
        }
    }
    if (fwrite(buffer, sizeof(wchar_t), count, fp) != count) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int main() {
    wchar_t *key;
    wchar_t *value;
    StrMap map;
    libmap_read_map_file("test3.tcc", &map);
    //libtcc_write_unicode_to_file("sdfsd.txt", map.buffer + 1, map.count - 2);
    libmap_parse(&map);
    libmap_polish(&map);
    //key = (wchar_t *)malloc(100);
    //wcscpy(key, L"font face name");
    key = L"font file";
    //tcc_map_set_offset(map.map, 3);
    if (tcc_map_retrieve(map.map, &key, &value))
        MessageBoxW(NULL, value, L"D", MB_OK);
    key = L"font face name";
    //tcc_map_set_offset(map.map, 3);
    if (tcc_map_retrieve(map.map, &key, &value))
        MessageBoxW(NULL, value, L"D", MB_OK);
        //wprintf(value);
    libmap_destroy(&map);
    //free(key);
    return 0;
}
