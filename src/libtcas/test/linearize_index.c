#include "../tcas/tcas.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"

int main() {
    tcas_u32 i, count, id, offset;
    /* kinds of chunks */
    TCAS_pRawChunk pRawChunks;
    TCAS_pCompChunk pCompChunks;
    TCAS_pOrderedIndex pOrderedIndexs;
    TCAS_IndexStreamsPtr indexStreams;
    /* other variables */
    TCAS_LinearIndexsPtr linearIndexs;
    /* raw chunks */
    pRawChunks = (TCAS_pRawChunk)malloc(5 * sizeof(TCAS_RawChunk));
    pRawChunks[0].startTime = 0;
    pRawChunks[0].endTime = 100;
    pRawChunks[0].frameType = 0;
    pRawChunks[0].layer = 0;
    pRawChunks[0].posX = 100;
    pRawChunks[0].posY = 0;
    pRawChunks[0].r = (tcas_byte)1;
    pRawChunks[0].g = (tcas_byte)2;
    pRawChunks[0].b = (tcas_byte)3;
    pRawChunks[0].a = (tcas_byte)4;
    pRawChunks[1].startTime = 0;
    pRawChunks[1].endTime = 100;
    pRawChunks[1].frameType = 0;
    pRawChunks[1].layer = 2;
    pRawChunks[1].posX = 100;
    pRawChunks[1].posY = 0;
    pRawChunks[1].r = (tcas_byte)1;
    pRawChunks[1].g = (tcas_byte)2;
    pRawChunks[1].b = (tcas_byte)3;
    pRawChunks[1].a = (tcas_byte)4;
    pRawChunks[2].startTime = 0;
    pRawChunks[2].endTime = 100;
    pRawChunks[2].frameType = 0;
    pRawChunks[2].layer = 0;
    pRawChunks[2].posX = 100;
    pRawChunks[2].posY = 0;
    pRawChunks[2].r = (tcas_byte)1;
    pRawChunks[2].g = (tcas_byte)2;
    pRawChunks[2].b = (tcas_byte)3;
    pRawChunks[2].a = (tcas_byte)4;
    pRawChunks[3].startTime = 0;
    pRawChunks[3].endTime = 100;
    pRawChunks[3].frameType = 0;
    pRawChunks[3].layer = 0;
    pRawChunks[3].posX = 100;
    pRawChunks[3].posY = 0;
    pRawChunks[3].r = (tcas_byte)1;
    pRawChunks[3].g = (tcas_byte)2;
    pRawChunks[3].b = (tcas_byte)3;
    pRawChunks[3].a = (tcas_byte)4;
    pRawChunks[4].startTime = 0;
    pRawChunks[4].endTime = 100;
    pRawChunks[4].frameType = 0;
    pRawChunks[4].layer = 0;
    pRawChunks[4].posX = 100;
    pRawChunks[4].posY = 0;
    pRawChunks[4].r = (tcas_byte)1;
    pRawChunks[4].g = (tcas_byte)2;
    pRawChunks[4].b = (tcas_byte)3;
    pRawChunks[4].a = (tcas_byte)4;
    /* compressed chunks */
    libtcas_convert_raw_chunks(pRawChunks, 5, &pCompChunks, &count);
    free(pRawChunks);
    libtcas_create_ordered_index_from_stream(pCompChunks, count, 0, TCAS_HEADER_OFFSET, 0, 0, &pOrderedIndexs, NULL, &id, &offset);
    indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, count, NULL);
    vector_retrieve(indexStreams, 2, &linearIndexs);
    printf("%i %i %i %i\n", indexStreams->count, indexStreams->elementSize, linearIndexs->count, linearIndexs->elementSize);
    vector_destroy(indexStreams);
    return 0;
}
