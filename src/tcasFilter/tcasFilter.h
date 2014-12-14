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

#ifndef TCASFILTER_H
#define TCASFILTER_H
#pragma once

#include <Windows.h>
#include <InitGuid.h>
#include <streams.h>
#include <string.h>

//#include "../../../../../libtcas/trunk/src/alpha/tcas/tcas.h"
#include "../../../../../libtcas/trunk/src/alpha/tcas/hla_frame_cache.h"

#define TCASFILTER_NAME_W L"TcasFilter"
#define TCASFILTER_NAME_A "TcasFilter"
#define TCASFILTER_NAME_T TEXT("TcasFilter")
#define TCASFILTER_NAME TCASFILTER_NAME_T

#define TCAS_NONE  0
#define TCAS_YV12  1  // preferred order
#define TCAS_YUY2  2
#define TCAS_RGB24 3
#define TCAS_RGB32 4


// {B3A5A510-3053-47F8-BC09-114089D28741}
DEFINE_GUID(CLSID_TcasFilter, 
0xb3a5a510, 0x3053, 0x47f8, 0xbc, 0x9, 0x11, 0x40, 0x89, 0xd2, 0x87, 0x41);

////////////////////////////
#include <wchar.h>
void Message(long i) {
    WCHAR wcs[65];
    wsprintfW(wcs, L"%i", i);
    MessageBoxW(NULL, wcs, L"INFO", MB_OK);
}
//////////////////////////

class TcasFilter : public CTransInPlaceFilter {
public:
    DECLARE_IUNKNOWN;
    TcasFilter(TCHAR *tszName, LPUNKNOWN pUnk, HRESULT *phr);
    virtual ~TcasFilter();
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT Transform(IMediaSample *pSample);
private:
    void ErrorInfo(const char *info);
    BOOL GetPotentialTcasFile(char *filename);
    void GetTfnFromVfn(const char *vfn, char *tfn);
    HRESULT InitializeIndex();
    HRESULT Initialize();
    void RenderRGB24(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart);
    void RenderRGB32(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart);
    void RenderYUY2(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart);
    void RenderYV12(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart);  // preferred
    // data members
    BOOL m_isTcasInitialized;
    int m_colorSpace;
    char m_filename[MAX_PATH];
    TCAS_File m_file;
    //HANDLE m_hMapFile;
    //tcas_unit *m_fileMapBuf;
    TCAS_Header m_header;
    CRefTime m_minTime;
    CRefTime m_maxTime;
    CRefTime m_offTime;  // to synchronize the timing of audio and video, because TCAS FX's timing is actually based on the audio
    BOOL m_hasSetOffTime;  // a flag indicating whether we've got the offset time or not
    IMediaSeeking *m_pSeeking;  // to use GetCurrentPosition function when playback
    IBaseFilter *m_pAviDecompressor;
    TCAS_IndexStreamsPtr m_indexStreams;
    TCAS_ChunksCache m_chunksCache;
    //ThreadpoolPtr m_tp;
};

#endif  // TCASFILTER_H

