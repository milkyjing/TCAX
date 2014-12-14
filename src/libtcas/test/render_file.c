#include "../tcas/tcas.h"
#include "../tcas/hla_z_comp.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"
#include "../tcas/hla_z_comp.c"

#include <Windows.h>


// global variables, TIP: to implement a real tcas renderer, pack these variables as class data members (C++) or struct data members (C)
TCAS_File g_file;
TCAS_Header g_header;
TCAS_IndexStreamsPtr g_indexStreams;
TCAS_ChunksCache g_chunksCache;
tcas_u32 g_fpsNumerator;
tcas_u32 g_fpsDenominator;
tcas_u32 g_minFrame;
tcas_u32 g_maxFrame;
tcas_u16 g_width;
tcas_u16 g_height;


static int tcas_renderer_init(const char *filename, double fps) {
    tcas_u32 x, y, t;
    TCAS_pOrderedIndex pOrderedIndexs;
    if (libtcas_open_file(&g_file, filename, tcas_file_open_existing) != tcas_error_success) {
        printf("Error: can not open the TCAS file.\n");
        return -1;
    }
    if (libtcas_read_header(&g_file, &g_header, 0) != tcas_error_success) {
        libtcas_close_file(&g_file);
        printf("Error: can not read the TCAS file.\n");
        return -1;
    }
    if (fps <= 0.0) {
        g_fpsNumerator = g_header.fpsNumerator;
        g_fpsDenominator = g_header.fpsDenominator;
    } else {
        g_fpsDenominator = 100000;
        g_fpsNumerator   = (tcas_u32)(fps * g_fpsDenominator);
        x = g_fpsNumerator;
        y = g_fpsDenominator;
        while (y) {   // find gcd
            t = x % y;
            x = y;
            y = t;
        }
        g_fpsNumerator /= x;
        g_fpsDenominator /= x;
    }
    g_minFrame = (tcas_u32)((tcas_s64)g_header.minTime * g_fpsNumerator / (g_fpsDenominator * 1000)) + 1;
    g_maxFrame = (tcas_u32)((tcas_s64)g_header.maxTime * g_fpsNumerator / (g_fpsDenominator * 1000)) + 1;
    g_indexStreams = NULL;
    // Initialize
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(g_header.flag)) {
        if (0 == g_header.chunks)
            libtcas_get_min_max_time_and_chunks(&g_file, &g_header.minTime, &g_header.maxTime, &g_header.chunks);
        if (libtcas_create_ordered_index(&g_file, &g_header, g_fpsNumerator, g_fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&g_file);
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(g_header.flag)) {
        if (0 == g_header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&g_file, &g_header.minTime, &g_header.maxTime, &g_header.chunks);
        if (libtcas_create_ordered_index_z(&g_file, &g_header, g_fpsNumerator, g_fpsDenominator, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&g_file);
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else {
        libtcas_close_file(&g_file);
        printf("Error: tcasfilter does no support this TCAS file type yet.\n");
        return -1;
    }
    if ((g_indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, g_header.chunks, NULL)) == NULL) {
        libtcas_close_file(&g_file);
        free(pOrderedIndexs);
        printf("Error: can not parse the TCAS file, step2.\n");
        return -1;
    }
    free(pOrderedIndexs);
    libtcas_init_chunks_cache(&g_chunksCache, vector_get_count(g_indexStreams));
    g_width = GETPOSX(g_header.resolution);
    g_height = GETPOSY(g_header.resolution);
    return 0;
}

static void tcas_renderer_fin() {
    if (g_indexStreams)
        vector_destroy(g_indexStreams);
    g_indexStreams = NULL;
    if (g_chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&g_chunksCache);
    libtcas_close_file(&g_file);
}

static tcas_bool tcas_render(TCAS_pFile pFile, int n) {
    TCAS_Chunk chunk;
    if (libtcas_create_chunk_with_linearized_index(&g_file, &g_header, g_indexStreams, n, &g_chunksCache, g_width, g_height, &chunk) != tcas_error_success)
        printf("Error: can not create TCAS frame.\n");
    if (GETCOUNT(chunk.cltp) > 0) {
        libtcas_write_chunk(pFile, &chunk);
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_TRUE;
    } else {
        free(chunk.pos_and_color);  /* free(z_pos_and_color) */
        return TCAS_FALSE;
    }
}

static tcas_bool tcas_render_z(TCAS_pFile pFile, int n) {
    TCAS_Chunk chunk;
    tcas_unit *z_pos_and_color;
    if (libtcas_create_chunk_with_linearized_index_z(&g_file, &g_header, g_indexStreams, n, &g_chunksCache, g_width, g_height, &chunk) != tcas_error_success)
        printf("Error: can not create TCAS frame.\n");
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
    int i, len;
    DWORD t1, t2;
    char szFilename[256];
    char szOutFilename[256];
    TCAS_File outFile;
    TCAS_Header outHeader;
    /* Show progress */
    HANDLE hStdout;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (argc > 1) {
        strcpy(szFilename, argv[1]);
    } else {
        printf("filename: ");
        fgets(szFilename, 256, stdin);
        szFilename[strlen(szFilename) - 1] = 0;
    }
    len = strlen(szFilename);
    if (stricmp(szFilename + len - 5, ".tcas") != 0) {
        printf("invalid TCAS file name.\n");
        return 0;
    }
    memcpy(szOutFilename, szFilename, len - 5);
    memcpy(szOutFilename + len - 5, "_parsed.tcas", 13);
    if (libtcas_open_file(&outFile, szOutFilename, tcas_file_create_new) != tcas_error_success) {
        printf("Error: cannot create the output file.\n");
        return 0;
    }
    t1 = GetTickCount();
    if (tcas_renderer_init(szFilename, 0) != 0) {
        libtcas_close_file(&outFile);
        return 0;
    }
    t2 = GetTickCount();
    printf("Info: initialization takes %i mm\n", t2 - t1);
    outHeader = g_header;
    outHeader.chunks = 0;
    //outHeader.flag = MAKE16B16B(TCAS_KEY_FRAMING_ENABLED, TCAS_FILE_TYPE_COMPRESSED_Z);
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    /* get the current cursor position */
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y + 1;
    t1 = GetTickCount();
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(g_header.flag)) {
        for (i = g_minFrame; i <= g_maxFrame; i++) {
            if (tcas_render(&outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - g_minFrame + 1) / (double)(g_maxFrame - g_minFrame + 1));
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(g_header.flag)) {
        for (i = g_minFrame; i <= g_maxFrame; i++) {
            if (tcas_render_z(&outFile, i))
                outHeader.chunks++;
            SetConsoleCursorPosition(hStdout, coord);
            printf("progress: %.1f%%\n", 100 * (i - g_minFrame + 1) / (double)(g_maxFrame - g_minFrame + 1));
        }
    }
    SetConsoleCursorPosition(hStdout, coord);
    printf("progress: %.1f%%\n", 100.0);
    t2 = GetTickCount();
    libtcas_write_header(&outFile, &outHeader, TCAS_FALSE);
    libtcas_close_file(&outFile);
    tcas_renderer_fin();
    printf("Info: total time for parsing %i frames %i\n", g_maxFrame - g_minFrame, t2 - t1);
    return 0;
}

