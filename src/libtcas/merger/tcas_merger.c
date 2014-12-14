/* for simplicity consideration, the two TCAS files should have the same type, resolution and fps
*/

#include "../tcas/tcas.h"
#include "../tcas/hla_z_comp.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"
#include "../tcas/rb.c"
#include "../tcas/hla_z_comp.c"

#include <Windows.h>


typedef struct _tcas_st {
    TCAS_File file;
    TCAS_Header header;
    TCAS_IndexStreamsPtr indexStreams;
    TCAS_ChunksCache chunksCache;
    tcas_u32 fpsNumerator;
    tcas_u32 fpsDenominator;
    tcas_s32 minFrame;
    tcas_s32 maxFrame;
    tcas_u16 width;
    tcas_u16 height;
} TcasSt, *TcasStPtr;


static int tcas_renderer_init(TcasStPtr pTcas, const char *filename, double fps) {
    tcas_u32 x, y, t;
    TCAS_pOrderedIndex pOrderedIndexs;
    if (libtcas_open_file(&pTcas->file, filename, tcas_file_open_existing) != tcas_error_success) {
        printf("Error: can not open the TCAS file - \"%s\"\n", filename);
        return -1;
    }
    if (libtcas_read_header(&pTcas->file, &pTcas->header, 0) != tcas_error_success) {
        libtcas_close_file(&pTcas->file);
        printf("Error: can not read the TCAS file - \"%s\"\n", filename);
        return -1;
    }
    if (fps <= 0.0) {
        pTcas->fpsNumerator = pTcas->header.fpsNumerator;
        pTcas->fpsDenominator = pTcas->header.fpsDenominator;
    } else {
        pTcas->fpsDenominator = 100000;
        pTcas->fpsNumerator   = (tcas_u32)(fps * pTcas->fpsDenominator);
        x = pTcas->fpsNumerator;
        y = pTcas->fpsDenominator;
        while (y) {   // find gcd
            t = x % y;
            x = y;
            y = t;
        }
        pTcas->fpsNumerator /= x;
        pTcas->fpsDenominator /= x;
    }
    // Initialize
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_create_ordered_index(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            printf("Error: can not parse the TCAS file - \"%s\"\n", filename);
            return -1;
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_create_ordered_index_z(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            printf("Error: can not parse the TCAS file - \"%s\"\n", filename);
            return -1;
        }
    } else {
        libtcas_close_file(&pTcas->file);
        printf("Error: tcas_parser does no support this TCAS file type yet - \"%s\"\n", filename);
        return -1;
    }
    pTcas->minFrame = libtcas_MulDiv(pTcas->header.minTime, pTcas->fpsNumerator, pTcas->fpsDenominator * 1000) + 1;
    pTcas->maxFrame = libtcas_MulDiv(pTcas->header.maxTime, pTcas->fpsNumerator, pTcas->fpsDenominator * 1000) + 1;
    pTcas->indexStreams = NULL;
    if ((pTcas->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pTcas->header.chunks, NULL)) == NULL) {
        libtcas_close_file(&pTcas->file);
        free(pOrderedIndexs);
        printf("Error: can not parse the TCAS file, step2 - \"%s\"\n", filename);
        return -1;
    }
    free(pOrderedIndexs);
    libtcas_init_chunks_cache(&pTcas->chunksCache, vector_get_count(pTcas->indexStreams));
    pTcas->width = GETPOSX(pTcas->header.resolution);
    pTcas->height = GETPOSY(pTcas->header.resolution);
    return 0;
}

static void tcas_renderer_fin(TcasStPtr pTcas) {
    if (pTcas->indexStreams)
        vector_destroy(pTcas->indexStreams);
    pTcas->indexStreams = NULL;
    if (pTcas->chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&pTcas->chunksCache);
    libtcas_close_file(&pTcas->file);
}

static tcas_bool tcas_render(TcasStPtr pTcas1, TcasStPtr pTcas2, TCAS_pFile pOutFile, int n) {
    TCAS_Chunk chunk;
    tcas_u32 pitch, size;
    tcas_byte *rgba;
    pitch = pTcas1->width * (sizeof(tcas_byte) << 2);
    size = pTcas1->height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    if (libtcas_fill_frame_with_linearized_index(&pTcas1->file, &pTcas1->header, pTcas1->indexStreams, n, &pTcas1->chunksCache, rgba, pTcas1->width, pTcas1->height) != tcas_error_success)
        printf("Error: can not create TCAS frame 1.\n");
    if (libtcas_fill_frame_with_linearized_index(&pTcas2->file, &pTcas2->header, pTcas2->indexStreams, n, &pTcas2->chunksCache, rgba, pTcas2->width, pTcas2->height) != tcas_error_success)
        printf("Error: can not create TCAS frame 2.\n");
    chunk.startTime = libtcas_MulDiv(n, pTcas1->fpsDenominator * 1000, pTcas1->fpsNumerator);
    chunk.endTime = libtcas_MulDiv(n + 1, pTcas1->fpsDenominator * 1000, pTcas1->fpsNumerator);
    chunk.cltp = 0;
    libtcas_convert_rgba_to_chunk(rgba, pTcas1->width, pTcas1->height, TCAS_FALSE, &chunk);
    free(rgba);
    if (GETCOUNT(chunk.cltp) > 0) {
        libtcas_write_chunk(pOutFile, &chunk);
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_TRUE;
    } else {
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_FALSE;
    }
}

static tcas_bool tcas_render_z(TcasStPtr pTcas1, TcasStPtr pTcas2, TCAS_pFile pOutFile, int n) {
    TCAS_Chunk chunk;
    tcas_unit *z_pos_and_color;
    tcas_u32 pitch, size;
    tcas_byte *rgba;
    pitch = pTcas1->width * (sizeof(tcas_byte) << 2);
    size = pTcas1->height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    if (libtcas_fill_frame_with_linearized_index_z(&pTcas1->file, &pTcas1->header, pTcas1->indexStreams, n, &pTcas1->chunksCache, rgba, pTcas1->width, pTcas1->height) != tcas_error_success)
        printf("Error: can not create TCAS frame 1.\n");
    if (libtcas_fill_frame_with_linearized_index_z(&pTcas2->file, &pTcas2->header, pTcas2->indexStreams, n, &pTcas2->chunksCache, rgba, pTcas2->width, pTcas2->height) != tcas_error_success)
        printf("Error: can not create TCAS frame 1.\n");
    chunk.startTime = libtcas_MulDiv(n, pTcas1->fpsDenominator * 1000, pTcas1->fpsNumerator);
    chunk.endTime = libtcas_MulDiv(n + 1, pTcas1->fpsDenominator * 1000, pTcas1->fpsNumerator);
    chunk.cltp = 0;
    libtcas_convert_rgba_to_chunk(rgba, pTcas1->width, pTcas1->height, TCAS_FALSE, &chunk);
    free(rgba);
    if (GETCOUNT(chunk.cltp) > 0) {
        z_pos_and_color = libtcas_compress_pos_and_color(chunk.pos_and_color, GETCOUNT(chunk.cltp));
        free(chunk.pos_and_color);
        chunk.pos_and_color = z_pos_and_color;
        libtcas_write_chunk_z(pOutFile, &chunk);
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_TRUE;
    } else {
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_FALSE;
    }
}

int main(int argc, char *argv[]) {
    int len;
    DWORD t1, t2;
    char szFilename1[256];
    char szFilename2[256];
    char szOutFilename[256];
    tcas_s32 i;
    TcasSt tcas[2];
    TCAS_File outFile;
    TCAS_Header outHeader;
    tcas_s32 minFrame;
    tcas_s32 maxFrame;
    /* Show progress */
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (argc > 2) {
        strcpy(szFilename1, argv[1]);
        printf("filename1: %s\n", szFilename1);
        strcpy(szFilename2, argv[2]);
        printf("filename2: %s\n", szFilename2);
    } else {
        printf("filename1: ");
        fgets(szFilename1, 256, stdin);
        szFilename1[strlen(szFilename1) - 1] = 0;
        printf("filename2: ");
        fgets(szFilename2, 256, stdin);
        szFilename2[strlen(szFilename2) - 1] = 0;
    }
    len = strlen(szFilename1);
    if (stricmp(szFilename1 + len - 5, ".tcas") != 0) {
        printf("Error: invalid TCAS file name.\n");
        system("PAUSE");
        return 0;
    }
    memcpy(szOutFilename, szFilename1, len - 5);
    memcpy(szOutFilename + len - 5, "_parsed.tcas", 13);
    printf("Initializing...\n");
    t1 = GetTickCount();
    if (tcas_renderer_init(&tcas[0], szFilename1, 0) != 0) {
        system("PAUSE");
        return 0;
    }
    if (tcas_renderer_init(&tcas[1], szFilename2, 0) != 0) {
        tcas_renderer_fin(&tcas[0]);
        system("PAUSE");
        return 0;
    }
    if (!(tcas[0].header.resolution == tcas[1].header.resolution && 
        tcas[0].header.flag == tcas[1].header.flag && 
        tcas[0].header.fpsNumerator / tcas[0].header.fpsDenominator == tcas[1].header.fpsNumerator / tcas[1].header.fpsDenominator)) {
            tcas_renderer_fin(&tcas[0]);
            tcas_renderer_fin(&tcas[1]);
            printf("Error: the two TCAS files does not have the same resolution type and fps.\n");
            system("PAUSE");
            return 0;
    }
    t2 = GetTickCount();
    printf("Info: initialization takes %i mm\n", t2 - t1);
    if (libtcas_open_file(&outFile, szOutFilename, tcas_file_create_new) != tcas_error_success) {
        tcas_renderer_fin(&tcas[0]);
        tcas_renderer_fin(&tcas[1]);
        printf("Error: cannot create the output file.\n");
        system("PAUSE");
        return 0;
    }
    outHeader = tcas[0].header;
    outHeader.minTime = __min(tcas[0].header.minTime, tcas[1].header.minTime);
    outHeader.maxTime = __max(tcas[0].header.maxTime, tcas[1].header.maxTime);
    minFrame = __min(tcas[0].minFrame, tcas[1].minFrame);
    maxFrame = __max(tcas[0].maxFrame, tcas[1].maxFrame);
    outHeader.chunks = 0;
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(tcas[0].header.flag))
        outHeader.flag = MAKE16B16B(TCAS_KEY_FRAMING_DISABLED, TCAS_FILE_TYPE_PARSED);
    else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(tcas[0].header.flag))
        outHeader.flag = MAKE16B16B(TCAS_KEY_FRAMING_DISABLED, TCAS_FILE_TYPE_PARSED_Z);
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    /* get the current cursor position */
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y + 1;
    t1 = GetTickCount();
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(tcas[0].header.flag)) {
        for (i = minFrame; i < maxFrame; i++) {
            if (tcas_render(&tcas[0], &tcas[1], &outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - minFrame + 1) / (double)(maxFrame - minFrame + 1));
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(tcas[0].header.flag)) {
        for (i = minFrame; i < maxFrame; i++) {
            if (tcas_render_z(&tcas[0], &tcas[1], &outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - minFrame + 1) / (double)(maxFrame - minFrame + 1));
        }
    }
    SetConsoleCursorPosition(hStdout, coord);
    printf("progress: %.1f%%\n", 100.0);
    printf("Task completed!\n");
    t2 = GetTickCount();
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    libtcas_close_file(&outFile);
    tcas_renderer_fin(&tcas[0]);
    tcas_renderer_fin(&tcas[1]);
    printf("Info: total time for parsing %i frames %i mm\n\n", maxFrame - minFrame, t2 - t1);
    system("PAUSE");
    return 0;
}

