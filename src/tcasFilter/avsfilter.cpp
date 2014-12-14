/* 
 *  Copyright (C) 2009-2011 milkyjing <milkyjing@gmail.com>
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "avsfilter.h"


TcasRenderer::TcasRenderer(PClip _child, IScriptEnvironment *env, const char *filename, double fps, int maxFrameCount, int maxMemory) : GenericVideoFilter(_child) {
    ::GetFullPathNameA(filename, MAX_PATH, m_filename, NULL);
    if (fps < 0.0) {        // use TCAS default frame rate
        m_fpsNumerator = (tcas_u32)(-1);
        m_fpsDenominator = 1;
    } else if (0.0 == fps) {
        m_fpsNumerator   = vi.fps_numerator;
        m_fpsDenominator = vi.fps_denominator;
    } else {
        m_fpsDenominator = 100000;
        m_fpsNumerator   = (tcas_u32)(fps * m_fpsDenominator);
        tcas_u32 x = m_fpsNumerator;
        tcas_u32 y = m_fpsDenominator;
        tcas_u32 t;
        while (y) {   // find gcd
            t = x % y;
            x = y;
            y = t;
        }
        m_fpsNumerator /= x;
        m_fpsDenominator /= x;
    }
    if (maxMemory < 32) maxMemory = 32;   // if memory_max value is not set then iMemoryMax is 0
    else if (maxMemory > 1024) maxMemory = 1024;
    env->SetMemoryMax(maxMemory);
    if (maxFrameCount > 0)
        m_bUseFrameCache = TRUE;
    else
        m_bUseFrameCache = FALSE;
    if (m_bUseFrameCache) {
        if (::libtcas_frame_cache_init(m_filename, m_fpsNumerator, m_fpsDenominator, vi.width, vi.height, maxFrameCount, &m_frameCache) != tcas_error_success)
            env->ThrowError("Error: failed to initialize the frame cache caused by access failure to the TCAS file.\n");
        m_minFrame = m_frameCache.minFrame;
        m_maxFrame = m_frameCache.maxFrame;
        ::libtcas_frame_cache_run(&m_frameCache);
    } else {
        if (::libtcas_framer_init(m_filename, m_fpsNumerator, m_fpsDenominator, vi.width, vi.height, &m_framer) != tcas_error_success)
            env->ThrowError("Error: failed to initialize the frame cache caused by access failure to the TCAS file.\n");
        m_minFrame = m_framer.minFrame;
        m_maxFrame = m_framer.maxFrame;
    }
}

TcasRenderer::~TcasRenderer() {
    if (m_bUseFrameCache)
        ::libtcas_frame_cache_fin(&m_frameCache);
    else
        ::libtcas_framer_fin(&m_framer);
}

void TcasRenderer::RenderRGB24(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    const tcas_byte *srcp = (*pSrc)->GetReadPtr();
    tcas_byte *dstp       = (*pDst)->GetWritePtr();
    int srcPitch = (*pSrc)->GetPitch();
    int dstPitch = (*pDst)->GetPitch();
    int width    = (*pDst)->GetRowSize() / 3;
    int height   = (*pDst)->GetHeight();
    // copy data from source video frame to target video frame
    if (dstPitch == srcPitch) ::memcpy(dstp, srcp, dstPitch * height);
    else {
        for (int h = 0; h < height; h ++) {
            int yy = h * dstPitch;
            int yySrc = h * srcPitch;
            for (int w = 0; w < width; w ++) {
                int xx = 3 * w;
                dstp[yy + xx]     = srcp[yySrc + xx];
                dstp[yy + xx + 1] = srcp[yySrc + xx + 1];
                dstp[yy + xx + 2] = srcp[yySrc + xx + 2];
            }
        }
    }
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    if (m_bUseFrameCache)
        while (!::libtcas_frame_cache_get(&m_frameCache, n, &tcasFrameBuf)) continue;
    else
        ::libtcas_framer_get(&m_framer, n, &tcasFrameBuf);
    // Render
    int tcasPitch = (width << 2) * sizeof(tcas_byte);
    for (int h = 0; h < height; h ++) {
        y = height - h - 1;
        for (int w = 0; w < width; w ++) {
            x = w;
            int w4 = w << 2;
            int yy = h * tcasPitch;
            r = tcasFrameBuf[yy + w4];
            g = tcasFrameBuf[yy + w4 + 1];
            b = tcasFrameBuf[yy + w4 + 2];
            a = tcasFrameBuf[yy + w4 + 3];
            if (0 == a) continue;
            yy = y * dstPitch;
            int yySrc = y * srcPitch;
            int xx = x * 3;
            dstp[yy + xx]     = (b * a + srcp[yySrc + xx] * (255 - a)) / 255;
            dstp[yy + xx + 1] = (g * a + srcp[yySrc + xx + 1] * (255 - a)) / 255;
            dstp[yy + xx + 2] = (r * a + srcp[yySrc + xx + 2] * (255 - a)) / 255;
        }
    }
    delete[] tcasFrameBuf;
}

void TcasRenderer::RenderRGB32(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    const tcas_byte *srcp = (*pSrc)->GetReadPtr();
    tcas_byte *dstp       = (*pDst)->GetWritePtr();
    int srcPitch = (*pSrc)->GetPitch();
    int dstPitch = (*pDst)->GetPitch();
    int width    = (*pDst)->GetRowSize() >> 2;
    int height   = (*pDst)->GetHeight();
    // copy data from source video frame to target video frame
    if (dstPitch == srcPitch) ::memcpy(dstp, srcp, dstPitch * height);
    else {
        for (int h = 0; h < height; h ++) {
            int yy = h * dstPitch;
            int yySrc = h * srcPitch;
            for (int w = 0; w < width; w ++) {
                int xx = w << 2;
                dstp[yy + xx]     = srcp[yySrc + xx];
                dstp[yy + xx + 1] = srcp[yySrc + xx + 1];
                dstp[yy + xx + 2] = srcp[yySrc + xx + 2];
                dstp[yy + xx + 3] = srcp[yySrc + xx + 3];
            }
        }
    }
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    if (m_bUseFrameCache)
        while (!::libtcas_frame_cache_get(&m_frameCache, n, &tcasFrameBuf)) continue;
    else
        ::libtcas_framer_get(&m_framer, n, &tcasFrameBuf);
    // Render
    int tcasPitch = (width << 2) * sizeof(tcas_byte);
    for (int h = 0; h < height; h ++) {
        y = height - h - 1;
        for (int w = 0; w < width; w ++) {
            x = w;
            int w4 = w << 2;
            int yy = h * tcasPitch;
            r = tcasFrameBuf[yy + w4];
            g = tcasFrameBuf[yy + w4 + 1];
            b = tcasFrameBuf[yy + w4 + 2];
            a = tcasFrameBuf[yy + w4 + 3];
            if (0 == a) continue;
            yy = y * dstPitch;
            int yySrc = y * srcPitch;
            int xx = x << 2;
            dstp[yy + xx]     = (b * a + srcp[yySrc + xx] * (255 - a)) / 255;
            dstp[yy + xx + 1] = (g * a + srcp[yySrc + xx + 1] * (255 - a)) / 255;
            dstp[yy + xx + 2] = (r * a + srcp[yySrc + xx + 2] * (255 - a)) / 255;
            dstp[yy + xx + 3] = (a * a + srcp[yySrc + xx + 3] * (255 - a)) / 255;
        }
    }
    delete[] tcasFrameBuf;
}

void TcasRenderer::RenderYUY2(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    const tcas_byte *srcp = (*pSrc)->GetReadPtr();
    tcas_byte *dstp       = (*pDst)->GetWritePtr();
    int srcPitch = (*pSrc)->GetPitch();
    int dstPitch = (*pDst)->GetPitch();
    int width    = (*pDst)->GetRowSize() >> 1;
    int height   = (*pDst)->GetHeight();
    // copy data from source video frame to target video frame
    if (dstPitch == srcPitch) ::memcpy(dstp, srcp, dstPitch * height);
    else {
        for (int h = 0; h < height; h ++) {
            int yy = h * dstPitch;
            int yySrc = h * srcPitch;
            for (int w = 0; w < width; w ++) {
                int xx = w << 2;
                dstp[yy + xx]     = srcp[yySrc + xx];
                dstp[yy + xx + 1] = srcp[yySrc + xx + 1];
                dstp[yy + xx + 2] = srcp[yySrc + xx + 2];
                dstp[yy + xx + 3] = srcp[yySrc + xx + 3];
            }
        }
    }
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    if (m_bUseFrameCache)
        while (!::libtcas_frame_cache_get(&m_frameCache, n, &tcasFrameBuf)) continue;
    else
        ::libtcas_framer_get(&m_framer, n, &tcasFrameBuf);
    // Render
    int tcasPitch = (width << 2) * sizeof(tcas_byte);
    // Y
    for (int h = 0; h < height; h ++) {
        y = h;
        for (int w = 0; w < width; w ++) {
            x = w;
            int w4 = w << 2;
            int yy = h * tcasPitch;
            r = tcasFrameBuf[yy + w4];
            g = tcasFrameBuf[yy + w4 + 1];
            b = tcasFrameBuf[yy + w4 + 2];
            a = tcasFrameBuf[yy + w4 + 3];
            if (0 == a) continue;
            tcas_byte Y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            yy = y * dstPitch;
            int yySrc = y * srcPitch;
            int xx = x << 1;
            dstp[yy + xx] = (Y * a + srcp[yySrc + xx] * (255 - a)) / 255;
        }
    }
    // UV
    for (int h = 0; h < height; h ++) {
        y = h;
        for (int w = 0; w < width; w += 2) {
            x = w;
            int w4 = w << 2;
            int yy = h * tcasPitch;
            tcas_byte r1, g1, b1, a1;
            tcas_byte r2, g2, b2, a2;
            tcas_byte r3, g3, b3, a3;
            if (0 == w) {
                r1 = tcasFrameBuf[yy + w4];
                g1 = tcasFrameBuf[yy + w4 + 1];
                b1 = tcasFrameBuf[yy + w4 + 2];
                a1 = tcasFrameBuf[yy + w4 + 3];
            } else {
                r1 = tcasFrameBuf[yy + w4 - 4];
                g1 = tcasFrameBuf[yy + w4 - 3];
                b1 = tcasFrameBuf[yy + w4 - 2];
                a1 = tcasFrameBuf[yy + w4 - 1];
            }
            r2 = tcasFrameBuf[yy + w4];
            g2 = tcasFrameBuf[yy + w4 + 1];
            b2 = tcasFrameBuf[yy + w4 + 2];
            a2 = tcasFrameBuf[yy + w4 + 3];
            if (width - 2 == w) {
                r3 = tcasFrameBuf[yy + w4];
                g3 = tcasFrameBuf[yy + w4 + 1];
                b3 = tcasFrameBuf[yy + w4 + 2];
                a3 = tcasFrameBuf[yy + w4 + 3];
            } else {
                r3 = tcasFrameBuf[yy + w4 + 4];
                g3 = tcasFrameBuf[yy + w4 + 5];
                b3 = tcasFrameBuf[yy + w4 + 6];
                a3 = tcasFrameBuf[yy + w4 + 7];
            }
            if (0 == a1 && 0 == a2 && 0 == a3) continue;
            // V
            tcas_byte V1 = ((112 * r1 - 94 * g1 - 18 * b1 + 128) / 256) + 128;
            tcas_byte V2 = ((112 * r2 - 94 * g2 - 18 * b2 + 128) / 256) + 128;
            tcas_byte V3 = ((112 * r3 - 94 * g3 - 18 * b3 + 128) / 256) + 128;
            tcas_byte V  = (V1 + (V2 << 1) + V3) >> 2;
            // U
            tcas_byte U1 = ((-38 * r1 - 74 * g1 + 112 * b1 + 128) / 256) + 128;
            tcas_byte U2 = ((-38 * r2 - 74 * g2 + 112 * b2 + 128) / 256) + 128;
            tcas_byte U3 = ((-38 * r3 - 74 * g3 + 112 * b3 + 128) / 256) + 128;
            tcas_byte U  = (U1 + (U2 << 1) + U3) >> 2;
            // A
            tcas_byte A  = (a1 + (a2 << 1) + a3) >> 2;
            // Blend
            yy = y * dstPitch;
            int yySrc = y * srcPitch;
            int xx = x << 1;
            dstp[yy + xx + 1] = (U * A + srcp[yySrc + xx + 1] * (255 - A)) / 255;
            dstp[yy + xx + 3] = (V * A + srcp[yySrc + xx + 3] * (255 - A)) / 255;
        }
    }
    delete[] tcasFrameBuf;
}

void TcasRenderer::RenderYV12(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    const tcas_byte *srcpY = (*pSrc)->GetReadPtr(PLANAR_Y);
    const tcas_byte *srcpV = (*pSrc)->GetReadPtr(PLANAR_V);
    const tcas_byte *srcpU = (*pSrc)->GetReadPtr(PLANAR_U);
    tcas_byte *dstpY = (*pDst)->GetWritePtr(PLANAR_Y);
    tcas_byte *dstpV = (*pDst)->GetWritePtr(PLANAR_V);
    tcas_byte *dstpU = (*pDst)->GetWritePtr(PLANAR_U);
    int src_pitchY  = (*pSrc)->GetPitch(PLANAR_Y);
    int src_pitchUV = (*pSrc)->GetPitch(PLANAR_V);
    int dst_pitchY  = (*pDst)->GetPitch(PLANAR_Y);
    int dst_pitchUV = (*pDst)->GetPitch(PLANAR_V);
    int width   = (*pDst)->GetRowSize(PLANAR_Y);
    int height  = (*pDst)->GetHeight(PLANAR_Y);
    // copy data from source video frame to target video frame
    if (dst_pitchY == src_pitchY) {
        int sizeY  = height * src_pitchY;
        int sizeUV = sizeY >> 2;
        ::memcpy(dstpY, srcpY, sizeY);
        ::memcpy(dstpV, srcpV, sizeUV);
        ::memcpy(dstpU, srcpU, sizeUV);
    } else {
        for (int h = 0; h < height; h ++) {
            for (int w = 0; w < width; w ++) {
                dstpY[h * dst_pitchY + w] = srcpY[h * src_pitchY + w];
            }
        }
        int hUV = height >> 1;
        int wUV = width >> 1;
        for (int h = 0; h < hUV; h ++) {
            for (int w = 0; w < wUV; w ++) {
                dstpV[h * dst_pitchUV + w] = srcpV[h * src_pitchUV + w];
                dstpU[h * dst_pitchUV + w] = srcpU[h * src_pitchUV + w];
            }
        }
    }
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    if (m_bUseFrameCache)
        while (!::libtcas_frame_cache_get(&m_frameCache, n, &tcasFrameBuf)) continue;
    else
        ::libtcas_framer_get(&m_framer, n, &tcasFrameBuf);
    // Render
    int tcasPitch = (width << 2) * sizeof(tcas_byte);
    // Y
    for (int h = 0; h < height; h ++) {
        y = h;
        for (int w = 0; w < width; w ++) {
            x = w;
            int w4 = w << 2;
            int yy = h * tcasPitch;
            r = tcasFrameBuf[yy + w4];
            g = tcasFrameBuf[yy + w4 + 1];
            b = tcasFrameBuf[yy + w4 + 2];
            a = tcasFrameBuf[yy + w4 + 3];
            if (0 == a) continue;
            tcas_byte Y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            yy = y * dst_pitchY;
            int yySrc = y * src_pitchY;
            int xx = x;
            dstpY[yy + xx] = (Y * a + srcpY[yySrc + xx] * (255 - a)) / 255;
        }
    }
    // UV
    for (int h = 0; h < height; h += 2) {
        y = h >> 1;
        for (int w = 0; w < width; w += 2) {
            x = w >> 1;
            int w4  = w << 2;
            int yy1 = h * tcasPitch;
            int yy2 = (h + 1) * tcasPitch;
            tcas_byte r11, g11, b11, a11, r12, g12, b12, a12, r13, g13, b13, a13;
            tcas_byte r21, g21, b21, a21, r22, g22, b22, a22, r23, g23, b23, a23;
            if (0 == w) {
                r11 = tcasFrameBuf[yy1 + w4];
                g11 = tcasFrameBuf[yy1 + w4 + 1];
                b11 = tcasFrameBuf[yy1 + w4 + 2];
                a11 = tcasFrameBuf[yy1 + w4 + 3];
                r21 = tcasFrameBuf[yy2 + w4];
                g21 = tcasFrameBuf[yy2 + w4 + 1];
                b21 = tcasFrameBuf[yy2 + w4 + 2];
                a21 = tcasFrameBuf[yy2 + w4 + 3];
            } else {
                r11 = tcasFrameBuf[yy1 + w4 - 4];
                g11 = tcasFrameBuf[yy1 + w4 - 3];
                b11 = tcasFrameBuf[yy1 + w4 - 2];
                a11 = tcasFrameBuf[yy1 + w4 - 1];
                r21 = tcasFrameBuf[yy2 + w4 - 4];
                g21 = tcasFrameBuf[yy2 + w4 - 3];
                b21 = tcasFrameBuf[yy2 + w4 - 2];
                a21 = tcasFrameBuf[yy2 + w4 - 1];
            }
            r12 = tcasFrameBuf[yy1 + w4];
            g12 = tcasFrameBuf[yy1 + w4 + 1];
            b12 = tcasFrameBuf[yy1 + w4 + 2];
            a12 = tcasFrameBuf[yy1 + w4 + 3];
            r22 = tcasFrameBuf[yy2 + w4];
            g22 = tcasFrameBuf[yy2 + w4 + 1];
            b22 = tcasFrameBuf[yy2 + w4 + 2];
            a22 = tcasFrameBuf[yy2 + w4 + 3];
            if (width - 2 == w) {
                r13 = tcasFrameBuf[yy1 + w4];
                g13 = tcasFrameBuf[yy1 + w4 + 1];
                b13 = tcasFrameBuf[yy1 + w4 + 2];
                a13 = tcasFrameBuf[yy1 + w4 + 3];
                r23 = tcasFrameBuf[yy2 + w4];
                g23 = tcasFrameBuf[yy2 + w4 + 1];
                b23 = tcasFrameBuf[yy2 + w4 + 2];
                a23 = tcasFrameBuf[yy2 + w4 + 3];
            } else {
                r13 = tcasFrameBuf[yy1 + w4 + 4];
                g13 = tcasFrameBuf[yy1 + w4 + 5];
                b13 = tcasFrameBuf[yy1 + w4 + 6];
                a13 = tcasFrameBuf[yy1 + w4 + 7];
                r23 = tcasFrameBuf[yy2 + w4 + 4];
                g23 = tcasFrameBuf[yy2 + w4 + 5];
                b23 = tcasFrameBuf[yy2 + w4 + 6];
                a23 = tcasFrameBuf[yy2 + w4 + 7];
            }
            if (0 == a11 && 0 == a12 && 0 == a13 && 0 == a21 && 0 == a22 && 0 == a23) continue;
            // V
            tcas_byte V11 = ((112 * r11 - 94 * g11 - 18 * b11 + 128) / 256) + 128;
            tcas_byte V12 = ((112 * r12 - 94 * g12 - 18 * b12 + 128) / 256) + 128;
            tcas_byte V13 = ((112 * r13 - 94 * g13 - 18 * b13 + 128) / 256) + 128;
            tcas_byte V21 = ((112 * r21 - 94 * g21 - 18 * b21 + 128) / 256) + 128;
            tcas_byte V22 = ((112 * r22 - 94 * g22 - 18 * b22 + 128) / 256) + 128;
            tcas_byte V23 = ((112 * r23 - 94 * g23 - 18 * b23 + 128) / 256) + 128;
            tcas_byte V1  = (V11 + (V12 << 1) + V13) >> 2;
            tcas_byte V2  = (V21 + (V22 << 1) + V23) >> 2;
            tcas_byte V   = (V1 + V2) >> 1;   // progressvie
            // U
            tcas_byte U11 = ((-38 * r11 - 74 * g11 + 112 * b11 + 128) / 256) + 128;
            tcas_byte U12 = ((-38 * r12 - 74 * g12 + 112 * b12 + 128) / 256) + 128;
            tcas_byte U13 = ((-38 * r13 - 74 * g13 + 112 * b13 + 128) / 256) + 128;
            tcas_byte U21 = ((-38 * r21 - 74 * g21 + 112 * b21 + 128) / 256) + 128;
            tcas_byte U22 = ((-38 * r22 - 74 * g22 + 112 * b22 + 128) / 256) + 128;
            tcas_byte U23 = ((-38 * r23 - 74 * g23 + 112 * b23 + 128) / 256) + 128;
            tcas_byte U1  = (U11 + (U12 << 1) + U13) >> 2;
            tcas_byte U2  = (U21 + (U22 << 1) + U23) >> 2;
            tcas_byte U   = (U1 + U2) >> 1;
            // A
            tcas_byte A1  = (a11 + (a12 << 1) + a13) >> 2;
            tcas_byte A2  = (a21 + (a22 << 1) + a23) >> 2;
            tcas_byte A   = (A1 + A2) >> 1;
            // Blend
            int yy = y * dst_pitchUV;
            int yySrc = y * src_pitchUV;
            int xx = x;
            dstpV[yy + xx] = (V * A + srcpV[yySrc + xx] * (255 - A)) / 255;
            dstpU[yy + xx] = (U * A + srcpU[yySrc + xx] * (255 - A)) / 255;
        }
    }
    delete[] tcasFrameBuf;
}

PVideoFrame __stdcall TcasRenderer::GetFrame(int n, IScriptEnvironment *env) {
    PVideoFrame src = child->GetFrame(n, env);
    if (n < m_minFrame || n >= m_maxFrame)
        return src;
    PVideoFrame dst = env->NewVideoFrame(vi);
    if (vi.IsRGB24()) RenderRGB24(&src, &dst, n, env);
    else if (vi.IsRGB32()) RenderRGB32(&src, &dst, n, env);
    else if (vi.IsYUY2()) RenderYUY2(&src, &dst, n, env);
    else if (vi.IsYV12()) RenderYV12(&src, &dst, n, env);
    else env->ThrowError("Error: tcasfilter only supports RGB24 or RGB32 or YUY2 or YV12 color space!\n");
    return dst;
}

AVSValue __cdecl Create_TcasRenderer(AVSValue args, void *user_data, IScriptEnvironment *env) {
    return new TcasRenderer(args[0].AsClip(), env, args[1].AsString(), args[2].AsFloat(0.0), args[3].AsInt(TCAS_MAX_FRAME_CACHED), args[4].AsInt(0));
}

extern "C" __declspec(dllexport) const char * __stdcall AvisynthPluginInit2(IScriptEnvironment *env) {
    env->AddFunction("tcasSub", "c[FILE]s[FPS]f[MAX_FRAME]i[MEMORY_MAX]i", Create_TcasRenderer, 0);
    return "`TcasFilter' function `tcasSub(file, fps, max_frame, memory_max)'";
}

