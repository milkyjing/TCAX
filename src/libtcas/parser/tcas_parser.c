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
        printf("Error: can not open the TCAS file.\n");
        return -1;
    }
    if (libtcas_read_header(&pTcas->file, &pTcas->header, 0) != tcas_error_success) {
        libtcas_close_file(&pTcas->file);
        printf("Error: can not read the TCAS file.\n");
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
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pTcas->header.flag)) {
        if (0 == pTcas->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pTcas->file, &pTcas->header.minTime, &pTcas->header.maxTime, &pTcas->header.chunks);
        if (libtcas_create_ordered_index_z(&pTcas->file, &pTcas->header, pTcas->fpsNumerator, pTcas->fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&pTcas->file);
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else {
        libtcas_close_file(&pTcas->file);
        printf("Error: tcas_parser does no support this TCAS file type yet.\n");
        return -1;
    }
    pTcas->minFrame = libtcas_MulDiv(pTcas->header.minTime, pTcas->fpsNumerator, pTcas->fpsDenominator * 1000) + 1;
    pTcas->maxFrame = libtcas_MulDiv(pTcas->header.maxTime, pTcas->fpsNumerator, pTcas->fpsDenominator * 1000) + 1;
    pTcas->indexStreams = NULL;
    if ((pTcas->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pTcas->header.chunks, NULL)) == NULL) {
        libtcas_close_file(&pTcas->file);
        free(pOrderedIndexs);
        printf("Error: can not parse the TCAS file, step2.\n");
        return -1;
    }
    free(pOrderedIndexs);
    libtcas_init_chunks_cache(&pTcas->chunksCache, vector_get_count(pTcas->indexStreams));
    printf("index stream count: %i\n", vector_get_count(pTcas->indexStreams));
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

static tcas_bool tcas_render(TcasStPtr pTcas, TCAS_pFile pFile, int n) {
    TCAS_Chunk chunk;
    tcas_byte *rgba;
    if (libtcas_create_frame_with_linearized_index(&pTcas->file, &pTcas->header, pTcas->indexStreams, n, &pTcas->chunksCache, pTcas->width, pTcas->height, &rgba) != tcas_error_success)
        printf("Error: can not create TCAS frame.\n");
    chunk.startTime = libtcas_MulDiv(n, pTcas->header.fpsDenominator * 1000, pTcas->header.fpsNumerator);
    chunk.endTime = libtcas_MulDiv(n + 1, pTcas->header.fpsDenominator * 1000, pTcas->header.fpsNumerator);
    chunk.cltp = 0;
    libtcas_convert_rgba_to_chunk(rgba, pTcas->width, pTcas->height, TCAS_FALSE, &chunk);
    free(rgba);
    if (GETCOUNT(chunk.cltp) > 0) {
        libtcas_write_chunk(pFile, &chunk);
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_TRUE;
    } else {
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_FALSE;
    }
}

static tcas_bool tcas_render_z(TcasStPtr pTcas, TCAS_pFile pFile, int n) {
    TCAS_Chunk chunk;
    tcas_unit *z_pos_and_color;
    tcas_byte *rgba;
    if (libtcas_create_frame_with_linearized_index_z(&pTcas->file, &pTcas->header, pTcas->indexStreams, n, &pTcas->chunksCache, pTcas->width, pTcas->height, &rgba) != tcas_error_success)
        printf("Error: can not create TCAS frame.\n");
    chunk.startTime = libtcas_MulDiv(n, pTcas->header.fpsDenominator * 1000, pTcas->header.fpsNumerator);
    chunk.endTime = libtcas_MulDiv(n + 1, pTcas->header.fpsDenominator * 1000, pTcas->header.fpsNumerator);
    chunk.cltp = 0;
    libtcas_convert_rgba_to_chunk(rgba, pTcas->width, pTcas->height, TCAS_FALSE, &chunk);
    free(rgba);
    if (GETCOUNT(chunk.cltp) > 0) {
        z_pos_and_color = libtcas_compress_pos_and_color(chunk.pos_and_color, GETCOUNT(chunk.cltp));
        free(chunk.pos_and_color);
        chunk.pos_and_color = z_pos_and_color;
        libtcas_write_chunk_z(pFile, &chunk);
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
    char szFilename[256];
    char szOutFilename[256];
    tcas_s32 i;
    TcasSt tcas;
    TCAS_File outFile;
    TCAS_Header outHeader;
    /* Show progress */
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (argc > 1) {
        strcpy(szFilename, argv[1]);
        printf("filename: %s\n", szFilename);
    } else {
        printf("filename: ");
        fgets(szFilename, 256, stdin);
        szFilename[strlen(szFilename) - 1] = 0;
    }
    len = strlen(szFilename);
    if (stricmp(szFilename + len - 5, ".tcas") != 0) {
        printf("Error: invalid TCAS file name.\n");
        system("PAUSE");
        return 0;
    }
    memcpy(szOutFilename, szFilename, len - 5);
    memcpy(szOutFilename + len - 5, "_parsed.tcas", 13);
    if (libtcas_open_file(&outFile, szOutFilename, tcas_file_create_new) != tcas_error_success) {
        printf("Error: cannot create the output file.\n");
        system("PAUSE");
        return 0;
    }
    printf("Initializing...\n");
    t1 = GetTickCount();
    if (tcas_renderer_init(&tcas, szFilename, 0) != 0) {
        libtcas_close_file(&outFile);
        system("PAUSE");
        return 0;
    }
    t2 = GetTickCount();
    printf("Info: initialization takes %i mm\n", t2 - t1);
    outHeader = tcas.header;
    outHeader.chunks = 0;
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(tcas.header.flag))
        outHeader.flag = MAKE16B16B(TCAS_KEY_FRAMING_DISABLED, TCAS_FILE_TYPE_PARSED);
    else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(tcas.header.flag))
        outHeader.flag = MAKE16B16B(TCAS_KEY_FRAMING_DISABLED, TCAS_FILE_TYPE_PARSED_Z);
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    /* get the current cursor position */
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y + 1;
    t1 = GetTickCount();
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(tcas.header.flag)) {
        for (i = tcas.minFrame; i < tcas.maxFrame; i++) {
            if (tcas_render(&tcas, &outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - tcas.minFrame + 1) / (double)(tcas.maxFrame - tcas.minFrame + 1));
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(tcas.header.flag)) {
        for (i = tcas.minFrame; i < tcas.maxFrame; i++) {
            if (tcas_render_z(&tcas, &outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - tcas.minFrame + 1) / (double)(tcas.maxFrame - tcas.minFrame + 1));
        }
    }
    SetConsoleCursorPosition(hStdout, coord);
    printf("progress: %.1f%%\n", 100.0);
    printf("Task completed!\n");
    t2 = GetTickCount();
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    libtcas_close_file(&outFile);
    tcas_renderer_fin(&tcas);
    printf("Info: total time for parsing %i frames %i mm\n\n", tcas.maxFrame - tcas.minFrame, t2 - t1);
    system("PAUSE");
    return 0;
}

