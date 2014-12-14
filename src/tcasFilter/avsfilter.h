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

#ifndef AVSFILTER_H
#define AVSFILTER_H
#pragma once

#include <windows.h>
#include "avisynth.h"
//#include "../../../../../libtcas/trunk/src/alpha/tcas/tcas.h"
#include "../../../../../libtcas/trunk/src/alpha/tcas/hla_frame_cache.h"
#include "../../../../../libtcas/trunk/src/alpha/tcas/hla_framer.h"

class TcasRenderer : public GenericVideoFilter {
private:
    char m_filename[MAX_PATH];
    BOOL m_bUseFrameCache;      // use m_framer or m_frameCache
    TCAS_Framer m_framer;
    TCAS_FrameCache m_frameCache;
    tcas_u32 m_fpsNumerator;
    tcas_u32 m_fpsDenominator;
    tcas_s32 m_minFrame;
    tcas_s32 m_maxFrame;
public:
    TcasRenderer(PClip _child, IScriptEnvironment *env, const char *filename, double fps, int maxFrameCount, int maxMemory);
    ~TcasRenderer();
    void RenderRGB24(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env);
    void RenderRGB32(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env);
    void RenderYUY2(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env);
    void RenderYV12(const PVideoFrame *pSrc, PVideoFrame *pDst, int n, IScriptEnvironment *env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
};

#endif    /* AVSFILTER_H */

