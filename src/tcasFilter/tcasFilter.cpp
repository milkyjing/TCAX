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

#include "tcasFilter.h"


TcasFilter::TcasFilter(TCHAR *tszName, LPUNKNOWN pUnk, HRESULT *phr) : CTransInPlaceFilter(tszName, pUnk, CLSID_TcasFilter, phr) {
    m_isTcasInitialized = FALSE;
    m_colorSpace = TCAS_NONE;
    ::memset(m_filename, 0, MAX_PATH * sizeof(char));
    ::memset(&m_file, 0, sizeof(TCAS_File));
    //m_hMapFile = NULL;
    //m_fileMapBuf = NULL;
    ::memset(&m_header, 0, sizeof(TCAS_Header));
    m_minTime = 0;
    m_maxTime = 0;
    m_offTime = 0;
    m_hasSetOffTime = FALSE;
    m_pSeeking = NULL;
    m_pAviDecompressor = NULL;
    m_indexStreams = NULL;
    ::memset(&m_chunksCache, 0, sizeof(TCAS_ChunksCache));
    //m_tp = NULL;
}

TcasFilter::~TcasFilter() {
    if (m_pSeeking)
        m_pSeeking->Release();
    if (m_pAviDecompressor)
        m_pAviDecompressor->Release();
    if (m_indexStreams)
        ::vector_destroy(m_indexStreams);
    if (m_chunksCache.streamCount > 0)
        ::libtcas_free_chunks_cache(&m_chunksCache);
    /*if (m_tp)
        ::tp_destroy(m_tp, 0);
    if (m_fileMapBuf)
        libtcas_unmapfile(m_hMapFile, m_fileMapBuf);*/
    ::libtcas_close_file(&m_file);
}

CUnknown *TcasFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr) {
    TcasFilter *pNewObject = new TcasFilter(TCASFILTER_NAME, pUnk, phr);
    if (NULL == pNewObject)
        *phr = E_OUTOFMEMORY;
    return pNewObject;
}

HRESULT TcasFilter::CheckInputType(const CMediaType *mtIn) {
    CAutoLock lock(m_pLock);
    CheckPointer(mtIn, E_POINTER);
    if (!::IsEqualGUID(*mtIn->FormatType(), FORMAT_VideoInfo)) return E_INVALIDARG;
    if (::IsEqualGUID(*mtIn->Type(), MEDIATYPE_Video)) {
        if (::IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_YV12)) {  // preferred
            if (TCAS_NONE == m_colorSpace)
                m_colorSpace = TCAS_YV12;
            else if (TCAS_YV12 != m_colorSpace) return VFW_E_TYPE_NOT_ACCEPTED;
            return Initialize();
        } else if (::IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_YUY2)) {
            if (TCAS_NONE == m_colorSpace)
                m_colorSpace = TCAS_YUY2;
            else if (TCAS_YUY2 != m_colorSpace) return VFW_E_TYPE_NOT_ACCEPTED;
            return Initialize();
        } else if (::IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_RGB24)) {
            if (TCAS_NONE == m_colorSpace)
                m_colorSpace = TCAS_RGB24;
            else if (TCAS_RGB24 != m_colorSpace) return VFW_E_TYPE_NOT_ACCEPTED;
            return Initialize();
        } else if (::IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_RGB32)) {
            if (TCAS_NONE == m_colorSpace)
                m_colorSpace = TCAS_RGB32;
            else if (TCAS_RGB32 != m_colorSpace) return VFW_E_TYPE_NOT_ACCEPTED;
            return Initialize();
        } else return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return VFW_E_TYPE_NOT_ACCEPTED;
}

void TcasFilter::ErrorInfo(const char *info) {
    char buf[1024];
    ::sprintf_s(buf, 1024, "%s - \"%s\"", info, m_filename);
    ::MessageBoxA(NULL, buf, "TcasFilter info", MB_OK | MB_ICONINFORMATION);
}

BOOL TcasFilter::GetPotentialTcasFile(char *filename) {
    IEnumFilters *pEnumFilters = NULL;
    if (m_pGraph && S_OK == m_pGraph->EnumFilters(&pEnumFilters)) {
        IBaseFilter *pBaseFilter = NULL;
        IFileSourceFilter *pFileSourceFilter = NULL;
        LPOLESTR pszFileName = NULL;  // WCHAR *pszFileName
        while (pEnumFilters->Next(1, &pBaseFilter, NULL) == S_OK) {
            if (pBaseFilter->QueryInterface(IID_IFileSourceFilter, (void **)&pFileSourceFilter) == S_OK) {
                if (pFileSourceFilter->GetCurFile(&pszFileName, NULL) == S_OK) {
                    if (pszFileName) {
			            int size = ::WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, NULL, 0, NULL, NULL);
                        char *sz = (char *)::malloc(size * sizeof(char));
                        ::WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, sz, size, NULL, NULL);
			            ::CoTaskMemFree(pszFileName);
                        GetTfnFromVfn(sz, filename);
                        ::free(sz);
			            return TRUE;
                    }
                }
            }
            pBaseFilter->Release();
		}
        pEnumFilters->Release();
	}
    return FALSE;
}

void TcasFilter::GetTfnFromVfn(const char *vfn, char *tfn) {
    const char *pos = ::strrchr(vfn, '.');
    int len = (int)(pos - vfn);
    ::memcpy(tfn, vfn, len * sizeof(char));
    ::memcpy(tfn + len, ".tcas\0", 6 * sizeof(char));
}

HRESULT TcasFilter::InitializeIndex() {
    TCAS_pOrderedIndex pOrderedIndexs;
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(m_header.flag)) {
        if (0 == m_header.chunks)
            libtcas_get_min_max_time_and_chunks(&m_file, &m_header.minTime, &m_header.maxTime, &m_header.chunks);
        if (libtcas_create_ordered_index(&m_file, &m_header, 0, 0, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&m_file);
            ErrorInfo("Failed to create the TCAS index");
            return E_FAIL;
        }
        m_indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, m_header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&m_chunksCache, vector_get_count(m_indexStreams));
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(m_header.flag)) {
        if (0 == m_header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&m_file, &m_header.minTime, &m_header.maxTime, &m_header.chunks);
        if (libtcas_create_ordered_index_z(&m_file, &m_header, 0, 0, &pOrderedIndexs, NULL) != tcas_error_success) {
            libtcas_close_file(&m_file);
            ErrorInfo("Failed to create the TCAS index");
            return E_FAIL;
        }
        m_indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, m_header.chunks, NULL);
        free(pOrderedIndexs);
        libtcas_init_chunks_cache(&m_chunksCache, vector_get_count(m_indexStreams));
    } else {
        ErrorInfo("The input TCAS file must be a compressed TCAS file");
        return E_FAIL;
    }
    return S_OK;
}

HRESULT TcasFilter::Initialize() {
    if (!m_isTcasInitialized) {
        if (!GetPotentialTcasFile(m_filename)) return VFW_E_TYPE_NOT_ACCEPTED;
        if (::GetFileAttributesA(m_filename) == INVALID_FILE_ATTRIBUTES) return VFW_E_TYPE_NOT_ACCEPTED;
        if (::libtcas_open_file(&m_file, m_filename, tcas_file_open_existing) != tcas_error_success) {
            ErrorInfo("Cannot open the TCAS file");
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        if (::libtcas_read_header(&m_file, &m_header, tcas_false) != tcas_error_success) {
            ErrorInfo("Cannot read the TCAS file");
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        //::libtcas_mapfile(m_filename, &m_hMapFile, &m_fileMapBuf);
        //::memcpy(&m_header, m_fileMapBuf, sizeof(TCAS_Header));
        if (m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pSeeking) != S_OK) return VFW_E_TYPE_NOT_ACCEPTED;
        if (TCAS_YV12 == m_colorSpace || TCAS_YUY2 == m_colorSpace) {
            if (::CoCreateInstance(CLSID_AVIDec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&m_pAviDecompressor) == S_OK)
                m_pGraph->AddFilter(m_pAviDecompressor, L"AVI Decompressor");  // this is not a must, MSYUV participates in through this interface
        }
        //m_tp = tp_create(NULL, 5);
        //tp_initialize(m_tp);
        if (InitializeIndex() != S_OK) return VFW_E_TYPE_NOT_ACCEPTED;
        m_minTime = CRefTime((long)m_header.minTime);
        m_maxTime = CRefTime((long)m_header.maxTime);
        m_isTcasInitialized = TRUE;
    }
    return S_OK;
}

HRESULT TcasFilter::Transform(IMediaSample *pSample) {
    //CheckPointer(pSample, E_POINTER);
    LONGLONG current;
    m_pSeeking->GetCurrentPosition(&current);
    //CAutoLock lock(m_pLock);  // may deadlock with seeking
    if (!m_hasSetOffTime) {
        CRefTime tTemp;
        pSample->GetTime((REFERENCE_TIME *)&tTemp, (REFERENCE_TIME *)&m_offTime);
        m_hasSetOffTime = TRUE;
    }
    CRefTime tStart = current;
    tStart += m_offTime;
    if (m_minTime <= tStart && tStart < m_maxTime) {
        AM_MEDIA_TYPE *pType = &m_pInput->CurrentMediaType();
        VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pType->pbFormat;
        switch (m_colorSpace) {
        case TCAS_YV12:
            RenderYV12(pSample, pvi, tStart);
            break;
        case TCAS_YUY2:
            RenderYUY2(pSample, pvi, tStart);
            break;
        case TCAS_RGB24:
            RenderRGB24(pSample, pvi, tStart);
            break;
        case TCAS_RGB32:
            RenderRGB32(pSample, pvi, tStart);
            break;
        default:
            return E_FAIL;
        }
    }
    return S_OK;
}

void TcasFilter::RenderRGB24(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    BYTE *pBuffer;
    pSample->GetPointer(&pBuffer);
    const tcas_byte *srcp = pBuffer;
    tcas_byte *dstp       = pBuffer;
    int width    = pvi->bmiHeader.biWidth;
    int height   = pvi->bmiHeader.biHeight;
    int srcPitch = width * 3;
    int dstPitch = srcPitch;
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    int n = tStart.Millisecs();
    //if (::libtcas_create_frame_with_index(&m_file, &m_header, m_pIndexs, n, width, height, &tcasFrameBuf) != tcas_error_success)
    if (::libtcas_create_frame_with_linearized_index(&m_file, &m_header, m_indexStreams, n, &m_chunksCache, width, height, &tcasFrameBuf) != tcas_error_success) return;
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

void TcasFilter::RenderRGB32(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    BYTE *pBuffer;
    pSample->GetPointer(&pBuffer);
    const tcas_byte *srcp = pBuffer;
    tcas_byte *dstp       = pBuffer;
    int width    = pvi->bmiHeader.biWidth;
    int height   = pvi->bmiHeader.biHeight;
    int srcPitch = width << 2;
    int dstPitch = srcPitch;
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    int n = tStart.Millisecs();
    //if (::libtcas_create_frame_with_index(&m_file, &m_header, m_pIndexs, n, width, height, &tcasFrameBuf) != tcas_error_success)
    if (::libtcas_create_frame_with_linearized_index(&m_file, &m_header, m_indexStreams, n, &m_chunksCache, width, height, &tcasFrameBuf) != tcas_error_success) return;
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

void TcasFilter::RenderYUY2(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    BYTE *pBuffer;
    pSample->GetPointer(&pBuffer);
    const tcas_byte *srcp = pBuffer;
    tcas_byte *dstp       = pBuffer;
    int srcPitch = pvi->bmiHeader.biWidth << 1;
    int dstPitch = srcPitch;
    int width    = pvi->bmiHeader.biWidth;
    int height   = pvi->bmiHeader.biHeight;
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    int n = tStart.Millisecs();
    //if (::libtcas_create_frame_with_index(&m_file, &m_header, m_pIndexs, n, width, height, &tcasFrameBuf) != tcas_error_success)
    if (::libtcas_create_frame_with_linearized_index(&m_file, &m_header, m_indexStreams, n, &m_chunksCache, width, height, &tcasFrameBuf) != tcas_error_success) return;
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

void TcasFilter::RenderYV12(IMediaSample *pSample, VIDEOINFOHEADER *pvi, CRefTime &tStart) {
    tcas_u16  x, y;
    tcas_byte r, g, b, a;
    BYTE *pBuffer;
    pSample->GetPointer(&pBuffer);
    const tcas_byte *srcpY = pBuffer;
    const tcas_byte *srcpV = srcpY + pvi->bmiHeader.biHeight * pvi->bmiHeader.biWidth;
    const tcas_byte *srcpU = srcpV + pvi->bmiHeader.biHeight * pvi->bmiHeader.biWidth / 4;
    tcas_byte *dstpY = pBuffer;
    tcas_byte *dstpV = dstpY + pvi->bmiHeader.biHeight * pvi->bmiHeader.biWidth;
    tcas_byte *dstpU = dstpV + pvi->bmiHeader.biHeight * pvi->bmiHeader.biWidth / 4;
    int src_pitchY   = pvi->bmiHeader.biWidth;
    int src_pitchUV  = src_pitchY / 2;
    int dst_pitchY   = src_pitchY;
    int dst_pitchUV  = src_pitchUV;
    int width        = pvi->bmiHeader.biWidth;
    int height       = pvi->bmiHeader.biHeight;
    // Create TCAS frame
    tcas_byte *tcasFrameBuf;
    int n = tStart.Millisecs();
    //if (::libtcas_create_frame_with_index(&m_file, &m_header, m_pIndexs, n, width, height, &tcasFrameBuf) != tcas_error_success)
    if (::libtcas_create_frame_with_linearized_index_z(&m_file, &m_header, m_indexStreams, n, &m_chunksCache, width, height, &tcasFrameBuf) != tcas_error_success) return;
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


// COM things
const AMOVIESETUP_MEDIATYPE sudPinTypes[] = {
    {
        &MEDIATYPE_Video,       // Major type
        &MEDIASUBTYPE_NULL      // Minor type
    }
};

const AMOVIESETUP_PIN sudpPins[] = {
    {
        L"Input",             // Pins string name
        FALSE,                // Is it rendered
        FALSE,                // Is it an output
        FALSE,                // Are we allowed none
        FALSE,                // And allowed many
        &CLSID_NULL,          // Connects to filter
        NULL,                 // Connects to pin
        1,                    // Number of types
        sudPinTypes          // Pin information
    },
    {
        L"Output",            // Pins string name
        FALSE,                // Is it rendered
        TRUE,                 // Is it an output
        FALSE,                // Are we allowed none
        FALSE,                // And allowed many
        &CLSID_NULL,          // Connects to filter
        NULL,                 // Connects to pin
        1,                    // Number of types
        sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER sudTcasFilter = {
    &CLSID_TcasFilter,      // Filter CLSID
    TCASFILTER_NAME_W,      // String name
    MERIT_DO_NOT_USE,       // Filter merit
    2,                      // Number of pins
    sudpPins                // Pin information
};

CFactoryTemplate g_Templates[] = {
    { TCASFILTER_NAME_W, 
      &CLSID_TcasFilter, 
      TcasFilter::CreateInstance, 
      NULL, 
      &sudTcasFilter 
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer() {
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer() {
    return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved) {
	return DllEntryPoint((HINSTANCE)hModule, dwReason, lpReserved);
}

