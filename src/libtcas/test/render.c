#include "../tcas/tcas.h"
#include "../tcas/hla_z_comp.h"
#include "../tcas/tcas.c"
#include "../tcas/vector.c"
#include "../tcas/hla_z_comp.c"

#include <Windows.h>    // where GetTickCount comes from


// global variables, TIP: to implement a real tcas renderer, pack these variables as class data members (C++) or struct data members (C)
TCAS_File g_file;
//HANDLE g_hMapFile;
//tcas_unit *g_fileMapBuf;
TCAS_Header g_header;
//TCAS_pIndex g_pIndexs;
//TCAS_pCompactedIndex g_pCompactedIndexs;
TCAS_pOrderedIndex g_pOrderedIndexs;
TCAS_IndexStreamsPtr g_indexStreams;
TCAS_ChunksCache g_chunksCache;
//ThreadpoolPtr g_tp;
/* */
tcas_u32 g_fpsNumerator;
tcas_u32 g_fpsDenominator;
tcas_u32 g_minFrame;
tcas_u32 g_maxFrame;
tcas_u32 g_width;
tcas_u32 g_height;

int tcas_renderer_init(const char *filename, double fps) {
    tcas_u32 x, y, t;
    if (libtcas_open_file(&g_file, filename, tcas_file_open_existing) != tcas_error_success) {
        printf("Error: can not open the TCAS file.\n");
        return -1;
    }
    if (libtcas_read_header(&g_file, &g_header, 0) != tcas_error_success) {
        libtcas_close_file(&g_file);
        printf("Error: can not read the TCAS file.\n");
        return -1;
    }
    //libtcas_mapfile(g_filename, &g_hMapFile, &g_fileMapBuf);
    //memcpy(&g_header, g_fileMapBuf, sizeof(TCAS_Header));
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
    g_minFrame = (tcas_u32)((tcas_s64)g_header.minTime * g_fpsNumerator / (g_fpsDenominator * 1000)) + 1;
    g_maxFrame = (tcas_u32)((tcas_s64)g_header.maxTime * g_fpsNumerator / (g_fpsDenominator * 1000)) + 1;
    //g_pIndexs = NULL;
    //g_pCompactedIndexs = NULL;
    g_pOrderedIndexs = NULL;
    g_indexStreams = NULL;
    //g_tp = NULL;
    //g_tp = tp_create(NULL, 5);
    //tp_initialize(g_tp);
    // Initialize
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(g_header.flag)) {
        if (libtcas_create_ordered_index(&g_file, &g_header, g_fpsNumerator, g_fpsDenominator, &g_pOrderedIndexs, NULL) != tcas_error_success) {
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(g_header.flag)) {
        if (libtcas_create_ordered_index_z(&g_file, &g_header, g_fpsNumerator, g_fpsDenominator, &g_pOrderedIndexs, NULL) != tcas_error_success) {
            printf("Error: can not parse the TCAS file, step1.\n");
            return -1;
        }
    } else {
        printf("Error: tcasfilter does no support this TCAS file type yet.\n");
        return -1;
    }
    if ((g_indexStreams = libtcas_linearize_ordered_indexs(g_pOrderedIndexs, g_header.chunks, NULL)) == NULL) {
        printf("Error: can not parse the TCAS file, step2.\n");
        return -1;
    }
    free(g_pOrderedIndexs);
    g_pOrderedIndexs = NULL;
    libtcas_init_chunks_cache(&g_chunksCache, vector_get_count(g_indexStreams));
    g_width = 1280;
    g_height = 720;
    g_file.temp = 0;
    return 0;
}

void tcas_renderer_fin() {
    //free(g_pIndexs);
    //g_pIndexs = NULL;
    //free(g_pCompactedIndexs);
    //g_pCompactedIndexs = NULL;
    if (g_indexStreams)
        vector_destroy(g_indexStreams);
    g_indexStreams = NULL;
    if (g_chunksCache.streamCount > 0)
        libtcas_free_chunks_cache(&g_chunksCache);
    /*if (g_tp)
        tp_destroy(g_tp, 0);
    if (g_fileMapBuf)
        libtcas_unmapfile(g_hMapFile, g_fileMapBuf);*/
    libtcas_close_file(&g_file);
}

void tcas_render_rgb32(int n) {
    // Create TCAS frame
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    tcas_byte *srcp;
    tcas_byte *dstp;
    int h, w, xx, yy, yySrc, w4, srcPitch, dstPitch, width, height, tcasPitch;
    tcas_byte *tcasFrameBuf;
    srcp = (tcas_byte *)malloc(1280 * 720 * 4 * sizeof(tcas_byte));
    dstp = (tcas_byte *)malloc(1280 * 720 * 4 * sizeof(tcas_byte));
    srcPitch = 1280 * 4 * sizeof(tcas_byte);
    dstPitch = 1280 * 4 * sizeof(tcas_byte);
    width    = 1280;
    height   = 720;
    // copy data from source video frame to target video frame
    if (dstPitch == srcPitch)
        memcpy(dstp, srcp, dstPitch * height);
    else {
        for (h = 0; h < height; h ++) {
            yy = h * dstPitch;
            yySrc = h * srcPitch;
            for (w = 0; w < width; w ++) {
                xx = 3 * w;
                dstp[yy + xx]     = srcp[yySrc + xx];
                dstp[yy + xx + 1] = srcp[yySrc + xx + 1];
                dstp[yy + xx + 2] = srcp[yySrc + xx + 2];
            }
        }
    }
    // Create TCAS frame
    //if (libtcas_create_frame_with_index(&g_file, &g_header, g_pIndexs, n, width, height, &tcasFrameBuf) != tcas_error_success)
    if (libtcas_create_frame_with_linearized_index_z(&g_file, &g_header, g_indexStreams, n, &g_chunksCache, g_width, g_height, &tcasFrameBuf) != tcas_error_success)
        printf("Error: can not create TCAS frame.\n");
    // Render
    tcasPitch = (width << 2) * sizeof(tcas_byte);
    for (h = 0; h < height; h ++) {
        y = height - h - 1;
        for (w = 0; w < width; w ++) {
            x = w;
            w4 = w << 2;
            yy = h * tcasPitch;
            r = tcasFrameBuf[yy + w4];
            g = tcasFrameBuf[yy + w4 + 1];
            b = tcasFrameBuf[yy + w4 + 2];
            a = tcasFrameBuf[yy + w4 + 3];
            if (0 == a) continue;
            yy = y * dstPitch;
            yySrc = y * srcPitch;
            xx = x * 3;
            dstp[yy + xx]     = (b * a + srcp[yySrc + xx] * (255 - a)) / 255;
            dstp[yy + xx + 1] = (g * a + srcp[yySrc + xx + 1] * (255 - a)) / 255;
            dstp[yy + xx + 2] = (r * a + srcp[yySrc + xx + 2] * (255 - a)) / 255;
        }
    }
    free(srcp);
    free(dstp);
    free(tcasFrameBuf);
}

int main() {
    int i, count;
    DWORD t0, t1, t2, t3;
    t1 = GetTickCount();
    if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\tcas_test_bezier\\test.tcas", 25) != 0) {
    //if (tcas_renderer_init("F:\Downloads\\fuck2.tcas", 25) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\tcas_test_keyframing_shape\\test_c.tcas", 25) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\FXs\\horizon\\op\\horizon_op.tcas", 23.976) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\FXs\\working\\op\\working_II_op.tcas", 23.976) != 0) {
        printf("error!");
        return 0;
    }
    t2 = GetTickCount();
    printf("initialization takes %i mm\n\n", t2 - t1);
    printf("chunk streams %i\n\n", g_chunksCache.streamCount);
    count = 200;
    t0 = GetTickCount();
    for (i = g_minFrame; i < g_maxFrame; i++) {
        //t1 = GetTickCount();
        tcas_render_rgb32(i);
        //t2 = GetTickCount();
        //printf("tcas_render_rgb32 frame %i takes %i mm\n", i, t2 - t1);
    }
    t3 = GetTickCount();
    tcas_renderer_fin();
    printf("\ntotal time for rendering %i frames %i\n", g_maxFrame - g_minFrame, t3 - t0);
    printf("\nI/O time %i\n", g_file.temp);
    return 0;
}
