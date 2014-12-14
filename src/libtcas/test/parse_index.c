#include "../tcas/tcas.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"

void traverse(const TCAS_pOrderedIndex pOrderedIndex) {
    printf("%i %i\n", pOrderedIndex->first, pOrderedIndex->last);
}

int main() {
    TCAS_LinearIndexsPtr linearIndexs;
    TCAS_OrderedIndex indexStart;
    TCAS_OrderedIndex indexEnd;
    long result;
    TCAS_OrderedIndex key;
    vector_create(&linearIndexs, sizeof(TCAS_OrderedIndex), 2, NULL, NULL);
    indexStart.first = 3;
    indexStart.last = 3;
    indexStart.id_pair = MAKEINDEXIP(1, 0);
    indexEnd.first = 7;
    indexEnd.last = 7;
    indexEnd.id_pair = MAKEINDEXIP(2, 1);
    vector_assign(linearIndexs, 0, &indexStart);
    vector_assign(linearIndexs, 1, &indexEnd);
    indexStart.first = 12;
    indexStart.last = 12;
    indexStart.id_pair = MAKEINDEXIP(3, 0);
    indexEnd.first = 21;
    indexEnd.last = 21;
    indexEnd.id_pair = MAKEINDEXIP(4, 1);
    vector_push_back(linearIndexs, &indexStart);
    vector_push_back(linearIndexs, &indexEnd);
    key.first = 14;
    key.last = 14;
    vector_traverse(linearIndexs, (vector_traverse_func)traverse);
    result = vector_binary_search_interval(linearIndexs, &key, (vector_compare_interval_func)_check_key_index_validity);
    printf("%i\n", result);
    return 0;
}

