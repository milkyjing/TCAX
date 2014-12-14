#include "../tcas/tcas.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"

int main() {
    tcas_u32 i, count;
    /* kinds of chunks */
    TCAS_pRawChunk pRawChunks;
    TCAS_pCompChunk pCompChunks;
    TCAS_ChunkStreamsPtr chunkStreams;
    /* other variables */
    char filename[256];
    TCAS_Header header;
    TCAS_File file;
    TCAS_LinearChunksPtr linearChunks;
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
    /* linear compressed chunks streams */
    libtcas_linearize_chunks(pCompChunks, count, &chunkStreams, tcas_false);
    /* now we have the linear chunk streams, we can write them to files, but here we just want to see the values */
    for (i = 0; i < chunkStreams->count; i++) {
        sprintf(filename, "test.p%03i.tcas", i + 1);
        printf("%s\n", filename);
        libtcas_open_file(&file, filename, tcas_file_create_new);
        libtcas_set_file_position_indicator(&file, tcas_fpi_header);
        vector_retrieve(chunkStreams, i, &linearChunks);
        libtcas_write_chunks(&file, (TCAS_pChunk)linearChunks->datap, linearChunks->count, tcas_false);
        libtcas_set_header(&header, 
            TCAS_FILE_TYPE_LINEAR_TIMING, i + 1, 
            1024, 576, 
            ((TCAS_pChunk)linearChunks->datap)[0].startTime, 
            ((TCAS_pChunk)linearChunks->datap)[linearChunks->count - 1].endTime, 
            linearChunks->count, 24, 1);
        libtcas_write_header(&file, &header, tcas_false);
        libtcas_close_file(&file);
    }
    vector_destroy(chunkStreams);
    return 0;
}
