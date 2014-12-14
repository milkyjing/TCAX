/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-4-17
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_COLOR__2003_04_17__H__
#define __FOO_COLOR__2003_04_17__H__
#include "StdDefine.h"

//=============================================================================
/**
 *  Color helper class.
 *  all function has no param check.
 */
class FCColor
{
public:
    /**
     *  Combine two pixel, put result into pDest.
     *  @param pDest : must be 32bpp to receive result
     *  @param cr1 : background
     *  @param cr2 : foreground
     *  @param nAlpha2 : alpha value of  cr2
     */
    static void CombineAlphaPixel (void* pDest, RGBQUAD cr1, const void* cr2, BYTE nAlpha2) ;

    /**
     *  AlphaBlend two pixel, put result into pDest.
     */
    static void AlphaBlendPixel (BYTE* pDest, const BYTE* pSrc, BYTE nAlphaSrc)
    {
        if (nAlphaSrc == 0xFF)
        {
            *(WORD*)pDest = *(WORD*)pSrc ;
            ((BYTE*)pDest)[2] = ((BYTE*)pSrc)[2] ;
            return ;
        }
        if (nAlphaSrc == 0)
            return ;

        // needn't bound in [0,0xFF], i have checked :-)
        pDest[0] = (pSrc[0] - pDest[0]) * nAlphaSrc / 0xFF + pDest[0] ;
        pDest[1] = (pSrc[1] - pDest[1]) * nAlphaSrc / 0xFF + pDest[1] ;
        pDest[2] = (pSrc[2] - pDest[2]) * nAlphaSrc / 0xFF + pDest[2] ;
    }

    /**
     *  Calculate pixel's grayscale value.
     *  @param prgb : point a 24bpp or 32bpp pixel address.
     */
    static BYTE GetGrayscale (const void* prgb)
    {
        return (30*PCL_R(prgb) + 59*PCL_G(prgb) + 11*PCL_B(prgb)) / 100 ;
    }

    /** 
     *  Rapid pixel copy.
     *  @param nBytes : can be 1,2,3,4
     */
    static void CopyPixel (void* pDest, const void* pSrc, int nBytes)
    {
        if (nBytes == 4)
            *(DWORD*)pDest = *(DWORD*)pSrc ;
        else if (nBytes == 3)
        {
            *(WORD*)pDest = *(WORD*)pSrc ;
            ((BYTE*)pDest)[2] = ((BYTE*)pSrc)[2] ;
        }
        else if (nBytes == 1)
            *(BYTE*)pDest = *(BYTE*)pSrc ;
        else if (nBytes == 2)
            *(WORD*)pDest = *(WORD*)pSrc ;
        else
        {
            assert(false) ;
            memcpy (pDest, pSrc, nBytes) ;
        }
    }

    /**
     *  @name RGB <==> HLS (Hue, Lightness, Saturation).
     */
    //@{
    /// RGB ==> HLS
    static void     RGBtoHLS (const void* prgb, double* H, double* L, double* S) ;
    /// HLS ==> RGB
    static RGBQUAD  HLStoRGB (const double& H, const double& L, const double& S) ;
    //@}

    /**
     *  @name RGB <==> HSV (Hue, Saturation, Value).
     */
    //@{
    /// RGB ==> HSV
    static void     RGBtoHSV (const void* prgb, double* H, double* S, double* V) ;
    /// HSV ==> RGB
    static RGBQUAD  HSVtoRGB (double H, const double& S, const double& V) ;
    //@}

    /// Computes bilinear interpolation of four pixels.
    /// The pixels in 'crPixel' in the following (x,y) order: [0,0], [1,0], [0,1], [1,1].
    static RGBQUAD Get_Bilinear_Pixel (double x, double y, bool bHasAlpha, const BYTE* crPixel[4]) ;
    /// Color black.
    static RGBQUAD crBlack() {return PCL_RGBA(0,0,0);}
    /// Color white.
    static RGBQUAD crWhite() {return PCL_RGBA(0xFF,0xFF,0xFF);}

private:
    static double __HLS_Value (const double& m1, const double& m2, double h) ;
    static bool __IsRGBEqual (const void* p1, const void* p2)
    {
        return (PCL_B(p1) == PCL_B(p2)) && (PCL_G(p1) == PCL_G(p2)) && (PCL_R(p1) == PCL_R(p2)) ;
    }
    static RGBQUAD __DoubleRGB_to_RGB (const double& r, const double& g, const double& b)
    {
        return PCL_RGBA (FClamp0255((int)(r*255)), FClamp0255((int)(g*255)), FClamp0255((int)(b*255))) ;
    }
};

//=============================================================================
// inline implement
//=============================================================================
inline void FCColor::CombineAlphaPixel (void* pDest, RGBQUAD cr1, const void* cr2, BYTE nAlpha2)
{
    if (PCL_A(&cr1) || nAlpha2)
    {
        if (nAlpha2 == 0)
        {
            *(RGBQUAD*)pDest = cr1 ;
            return ;
        }
        if ((PCL_A(&cr1) == 0) || (nAlpha2 == 0xFF))
        {
            CopyPixel (pDest, cr2, 3) ;
            PCL_A(pDest) = nAlpha2 ;
            return ;
        }
        // needn't bound in [0,0xFF], i have checked :-)
        int   nTmp1 = 0xFF * PCL_A(&cr1), nTmp2 = 0xFF * nAlpha2,
              nTmp12 = PCL_A(&cr1) * nAlpha2,
              nTemp = nTmp1 + nTmp2 - nTmp12 ;
        PCL_B(pDest) = (nTmp2*PCL_B(cr2) + (nTmp1 - nTmp12)*PCL_B(&cr1))/nTemp ;
        PCL_G(pDest) = (nTmp2*PCL_G(cr2) + (nTmp1 - nTmp12)*PCL_G(&cr1))/nTemp ;
        PCL_R(pDest) = (nTmp2*PCL_R(cr2) + (nTmp1 - nTmp12)*PCL_R(&cr1))/nTemp ;
        PCL_A(pDest) = nTemp / 0xFF ;

/*      // un-optimized, easier to read
        int    nTemp = 0xFF*(PCL_A(&cr1) + nAlpha2) - PCL_A(&cr1)*nAlpha2 ;
        PCL_B(pDest) = (0xFF*PCL_B(cr2)*nAlpha2 + (0xFF - nAlpha2)*PCL_B(&cr1)*PCL_A(&cr1))/nTemp ;
        PCL_G(pDest) = (0xFF*PCL_G(cr2)*nAlpha2 + (0xFF - nAlpha2)*PCL_G(&cr1)*PCL_A(&cr1))/nTemp ;
        PCL_R(pDest) = (0xFF*PCL_R(cr2)*nAlpha2 + (0xFF - nAlpha2)*PCL_R(&cr1)*PCL_A(&cr1))/nTemp ;
        PCL_A(pDest) = nTemp / 0xFF ;*/
    }
    else
    {
        PCL_B(pDest) = PCL_G(pDest) = PCL_R(pDest) = 0xFF ;
        PCL_A(pDest) = 0 ;
    }
}
//-----------------------------------------------------------------------------
inline void FCColor::RGBtoHLS (const void* prgb, double* H, double* L, double* S)
{
    const int   n_cmax = FMax (PCL_R(prgb), FMax (PCL_G(prgb), PCL_B(prgb))),
                n_cmin = FMin (PCL_R(prgb), FMin (PCL_G(prgb), PCL_B(prgb))) ;

    *L = (n_cmax + n_cmin) / 2.0 / 255.0 ;
    if (n_cmax == n_cmin)
    {
        *S = *H = 0.0 ;
        return ;
    }

    const double   r = PCL_R(prgb) / 255.0,
                   g = PCL_G(prgb) / 255.0,
                   b = PCL_B(prgb) / 255.0,
                   cmax = n_cmax / 255.0,
                   cmin = n_cmin / 255.0,
                   delta = cmax - cmin ;

    if (*L < 0.5) 
        *S = delta / (cmax+cmin) ;
    else
        *S = delta / (2.0-cmax-cmin) ;

    if (PCL_R(prgb) == n_cmax)
        *H = (g-b) / delta ;
    else if (PCL_G(prgb) == n_cmax)
        *H = 2.0 + (b-r) / delta ;
    else
        *H = 4.0 + (r-g) / delta ;
    *H /= 6.0 ;
    if (*H < 0.0)
        *H += 1.0 ;
}
//-----------------------------------------------------------------------------
inline double FCColor::__HLS_Value (const double& m1, const double& m2, double h)
{
    if (h > 6.0)
        h -= 6.0 ;
    else if (h < 0.0)
        h += 6.0 ;

    if (h < 1.0)
        return m1 + (m2 - m1) * h ;
    else if (h < 3.0)
        return m2 ;
    else if (h < 4.0)
        return m1 + (m2 - m1) * (4.0 - h) ;
    return m1 ;
}
inline RGBQUAD FCColor::HLStoRGB (const double& H, const double& L, const double& S)
{
    if (S < FLT_EPSILON) // == 0
        return __DoubleRGB_to_RGB (L, L, L) ;

    double     m1, m2 ;
    if (L < 0.5)
        m2 = L * (1.0 + S) ;
    else
        m2 = L + S - L*S ;
    m1 = 2.0*L - m2 ;

    return __DoubleRGB_to_RGB (__HLS_Value (m1, m2, H*6.0 + 2.0),
                               __HLS_Value (m1, m2, H*6.0),
                               __HLS_Value (m1, m2, H*6.0 - 2.0)) ;
}
//-----------------------------------------------------------------------------
inline void FCColor::RGBtoHSV (const void* prgb, double* H, double* S, double* V)
{
    const int      n_cmax = FMax (PCL_R(prgb), FMax (PCL_G(prgb), PCL_B(prgb))),
                   n_cmin = FMin (PCL_R(prgb), FMin (PCL_G(prgb), PCL_B(prgb))) ;
    const double   r = PCL_R(prgb) / 255.0,
                   g = PCL_G(prgb) / 255.0,
                   b = PCL_B(prgb) / 255.0,
                   delta = (n_cmax - n_cmin) / 255.0 ;
    *V = n_cmax / 255.0 ;
    if (n_cmax == n_cmin)
    {
        *S=0.0 ; *H=0.0;
        return ;
    }

    *S = (n_cmax - n_cmin) / (double)n_cmax ;
    if (PCL_R(prgb) == n_cmax)
        *H = (g - b) / delta ;
    else if (PCL_G(prgb) == n_cmax)
        *H = 2.0 + (b - r) / delta ;
    else if (PCL_B(prgb) == n_cmax)
        *H = 4.0 + (r - g) / delta ;

    *H /= 6.0 ;
    if (*H < 0.0)
        *H += 1.0 ;
    else if (*H > 1.0)
        *H -= 1.0 ;
}
//-----------------------------------------------------------------------------
inline RGBQUAD FCColor::HSVtoRGB (double h, const double& s, const double& v)
{
    if (s < FLT_EPSILON) // == 0
        return __DoubleRGB_to_RGB (v, v, v) ;

    if (h == 1.0)
        h = 0.0 ;
    h *= 6.0 ;

    const double  f = h - (int)h,
                  p = v * (1.0 - s),
                  q = v * (1.0 - s * f),
                  t = v * (1.0 - s * (1.0 - f)) ;
    switch ((int)h)
    {
        case 0 : return __DoubleRGB_to_RGB (v, t, p) ;
        case 1 : return __DoubleRGB_to_RGB (q, v, p) ;
        case 2 : return __DoubleRGB_to_RGB (p, v, t) ;
        case 3 : return __DoubleRGB_to_RGB (p, q, v) ;
        case 4 : return __DoubleRGB_to_RGB (t, p, v) ;
        case 5 : return __DoubleRGB_to_RGB (v, p, q) ;
    }
    return __DoubleRGB_to_RGB (0, 0, 0) ;
}
//-----------------------------------------------------------------------------
// Computes bilinear interpolation of four pixels.
// The pixels in 'crPixel' in the following order: [0,0], [1,0], [0,1], [1,1].
// !!! the pointer must 24bit or 32bit color
inline RGBQUAD FCColor::Get_Bilinear_Pixel (double x, double y, bool bHasAlpha, const BYTE* crPixel[4])
{
    const BYTE    * pPixel0 = crPixel[0], * pPixel1 = crPixel[1],
                  * pPixel2 = crPixel[2], * pPixel3 = crPixel[3] ;
    RGBQUAD       crRet = {0xFF, 0xFF, 0xFF, 0xFF} ;

    if ( bHasAlpha && (*(DWORD*)pPixel0 == *(DWORD*)pPixel1) &&
                      (*(DWORD*)pPixel0 == *(DWORD*)pPixel2) &&
                      (*(DWORD*)pPixel0 == *(DWORD*)pPixel3) )
    {
        return *(RGBQUAD*)pPixel0 ;
    }

    if ( !bHasAlpha && __IsRGBEqual(pPixel0, pPixel1) &&
                       __IsRGBEqual(pPixel0, pPixel2) &&
                       __IsRGBEqual(pPixel0, pPixel3) )
    {
        FCColor::CopyPixel (&crRet, pPixel0, 3) ;
        return crRet ;
    }

    // test x-[0,1] y-[0,1]
    assert ((x > -FLT_EPSILON) && (x < 1.0) && (y > -FLT_EPSILON) && (y < 1.0)) ;

    // x && y is zero
    if ((x < FLT_EPSILON) && (y < FLT_EPSILON))
    {
        FCColor::CopyPixel (&crRet, pPixel0, bHasAlpha ? 4 : 3) ;
        return crRet ;
    }

    if (!bHasAlpha || ((PCL_A(pPixel0) & PCL_A(pPixel1) & PCL_A(pPixel2) & PCL_A(pPixel3)) == 0xFF))
    {
        // if 24bit color
        for (int i=0 ; i < 3 ; i++)
        {
            const double   m0 = pPixel0[i] + x * (pPixel1[i] - pPixel0[i]),
                           m1 = pPixel2[i] + x * (pPixel3[i] - pPixel2[i]),
                           my = m0 + y * (m1 - m0) ;
            ((BYTE*)&crRet)[i] = (int)my ; // needn't bound
        }
    }
    else
    {
        // under is 32bit color with alpha
        int       nAlpha[4] = {pPixel0[3], pPixel1[3], pPixel2[3], pPixel3[3]} ;
        // calc dest alpha value
        double    m0 = nAlpha[0] + x * (nAlpha[1] - nAlpha[0]),
                  m1 = nAlpha[2] + x * (nAlpha[3] - nAlpha[2]),
                  my = m0 + y * (m1 - m0) ;
        PCL_A(&crRet) = (int)my ;
        if (PCL_A(&crRet)) // has alpha
            for (int i=0 ; i < 3 ; i++)
            {
                int   nSum0 = nAlpha[0] * pPixel0[i],
                      nSum2 = nAlpha[2] * pPixel2[i] ;

                m0 = nSum0 + x * (pPixel1[i] * nAlpha[1] - nSum0) ;
                m1 = nSum2 + x * (pPixel3[i] * nAlpha[3] - nSum2) ;
                my = m0 + y * (m1 - m0) ;
                ((BYTE*)&crRet)[i] = int(my) / PCL_A(&crRet) ;
            }
    }
    return crRet ;
}

#endif
