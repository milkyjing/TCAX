#ifndef __FOO_FLIB_MACRO_H__
#define __FOO_FLIB_MACRO_H__

#undef BI_RGB
#undef BI_BITFIELDS
#define BI_RGB        0
#define BI_BITFIELDS  3

typedef struct tagRECT
{
    long left;
    long top;
    long right;
    long bottom;
} RECT;

typedef struct tagPOINT
{
    long x;
    long y;
} POINT;

typedef struct tagSIZE
{
    long cx;
    long cy;
} SIZE;

#ifndef PCL_3RD_LIBRARY_USE_FREEIMAGE
typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned long      DWORD;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER
{
    DWORD      biSize;
    long       biWidth;
    long       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    long       biXPelsPerMeter;
    long       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER ;
#endif // PCL_3RD_LIBRARY_USE_FREEIMAGE

//============================================================================

inline bool IsRectEmpty (const RECT* pRC)
{
    if (pRC)
    {
        return ((pRC->right <= pRC->left) || (pRC->bottom <= pRC->top)) ;
    }
    return true ;
}

inline void SetRect (RECT* pRC, int xLeft, int yTop, int xRight, int yBottom)
{
    if (pRC)
    {
        pRC->left=xLeft ; pRC->top=yTop ; pRC->right=xRight ; pRC->bottom=yBottom ;
    }
}

inline void OffsetRect (RECT* pRC, int dx, int dy)
{
    if (pRC)
    {
        pRC->left+=dx ; pRC->top+=dy ; pRC->right+=dx ; pRC->bottom+=dy ;
    }
}

inline void InflateRect (RECT* pRC, int dx, int dy) {
    if (pRC)
    {
        pRC->left-=dx ; pRC->top-=dy ; pRC->right+=dx ; pRC->bottom+=dy ;
    }
}

inline void UnionRect (RECT* pDst, const RECT* pSrc1, const RECT* pSrc2)
{
    if (pDst && pSrc1 && pSrc2)
    {
        pDst->left = FMin (pSrc1->left, pSrc2->left) ;
        pDst->top = FMin (pSrc1->top, pSrc2->top) ;
        pDst->right = FMax (pSrc1->right, pSrc2->right) ;
        pDst->bottom = FMax (pSrc1->bottom, pSrc2->bottom) ;

        if (IsRectEmpty(pDst))
        {
            // set rect empty
            SetRect (pDst, 0, 0, 0, 0) ;
        }
    }
}

inline bool IntersectRect (RECT* pDst, const RECT* pSrc1, const RECT* pSrc2)
{
    if (pDst && pSrc1 && pSrc2)
    {
        pDst->left = FMax (pSrc1->left, pSrc2->left) ;
        pDst->top = FMax (pSrc1->top, pSrc2->top) ;
        pDst->right = FMin (pSrc1->right, pSrc2->right) ;
        pDst->bottom = FMin (pSrc1->bottom, pSrc2->bottom) ;

        if (IsRectEmpty(pDst))
        {
            // set rect empty
            SetRect (pDst, 0, 0, 0, 0) ;
            return false ;
        }
        return true ;
    }
    return false ;
}

//============================================================================

#endif
