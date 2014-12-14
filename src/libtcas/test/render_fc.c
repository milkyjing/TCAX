#include "../tcas/hla_frame_cache.h"
#include "../tcas/tcas.c"
#include "../tcas/hla_file_cache.c"
#include "../tcas/hla_frame_cache.c"
#include "../tcas/hla_z_comp.c"
#include "../tcas/vector.c"
#include "../tcas/queue.c"

TCAS_FrameCache g_frameCache;

tcas_u32 g_fpsNumerator;
tcas_u32 g_fpsDenominator;
tcas_u32 g_minFrame;
tcas_u32 g_maxFrame;
tcas_u32 g_width;
tcas_u32 g_height;

int tcas_renderer_init(const char *filename, double fps) {
    tcas_u32 x, y, t;
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
    g_width = 1280;
    g_height = 720;
    libtcas_frame_cache_init(filename, g_fpsNumerator, g_fpsDenominator, g_width, g_height, TCAS_MAX_FRAME_CACHED, &g_frameCache);
    //g_frameCache.fcpArgs.fileCache.temp = 0;
    libtcas_frame_cache_run(&g_frameCache);
    return 0;
}

void tcas_renderer_fin() {
    libtcas_frame_cache_fin(&g_frameCache);
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
    while (!libtcas_frame_cache_get(&g_frameCache, n, &tcasFrameBuf))
        continue;
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
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\tcas_test_illusion\\toradora_op.tcas", 23.976) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\tcas_test_bezier\\test.tcas", 25) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\11eyes_op_reborn\\11eyes_op.tcas", 23.976) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\11eyes_op_reborn\\11eyes_op_720p.tcas", 23.976) != 0) {
    //if (tcas_renderer_init("F:\\Downloads\\test2.tcas", 25) != 0) {
    if (tcas_renderer_init("E:\\杂项\\test1.tcas", 25) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\sophisticated\\tcas_volume_test\\test.tcas", 25) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\FXs\\horizon\\op\\horizon_op.tcas", 23.976) != 0) {
    //if (tcas_renderer_init("I:\\RIXE\\examples\\FXs\\working\\op\\working_II_op.tcas", 23.976) != 0) {
        printf("error!");
        return 0;
    }
    t2 = GetTickCount();
    printf("initialization takes %i mm\n\n", t2 - t1);
    count = 200;
    t0 = GetTickCount();
    for (i = g_frameCache.minFrame; i < g_frameCache.maxFrame; i++) {
        //t1 = GetTickCount();
        tcas_render_rgb32(i);
        //t2 = GetTickCount();
        //printf("tcas_render_rgb32 frame %i takes %i mm\n", i, t2 - t1);
    }
    t3 = GetTickCount();
    tcas_renderer_fin();
    printf("\ntotal time for rendering %i frames %i\n", g_frameCache.maxFrame - g_frameCache.minFrame, t3 - t0);
    //printf("\nI/O time %i\n", g_frameCache.fcpArgs.file.temp);
    return 0;
}
