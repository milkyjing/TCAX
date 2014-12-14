/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-7-29
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_PIXEL_PROCESSOR_BASE__2005_07_29__H__
#define __FOO_PIXEL_PROCESSOR_BASE__2005_07_29__H__
#include "../FHistogram.h"

//class FCInterface_PixelProcess ;
class FCSinglePixelProcessBase ;
    class FCPixelConvertTo16Bit ; // 1, 4, 8, 24, 32 ==> 16
    class FCPixelConvertTo8BitGray ; // 1, 4, 8, 16, 24, 32 ==> 8bit gray
    class FCPixelGrayscale ; // gray scale (>=24 bit)
    class FCPixelFillColor ; // fill color (>=24 bit)
    class FCPixelFillPattern ; // fill pattern (32 bit)
    class FCPixelHueSaturation ; // hue saturation (>=24 bit)]
    class FCPixelMirror ; // mirror (>=8 bit)
    class FCPixelFlip ; // flip (>=8 bit)
    class FCPixelShift ; // shift (>=24 bit)
    class FCPixelAutoContrast ; // auto contrast (>=24 bit)
    class FCPixelAutoColorEnhance ; // auto color enhance (>=24 bit)
    class FCPixelEmboss ; // emboss (>=24 bit)
    class FCPixelIllusion ; // illusion (>=24 bit)
    class FCPixelBlinds ; // blinds (>=24 bit)
    class FCPixelMosaic ; // mosaic (32 bit)
    class FCPixelAdjustRGB ; // adjust RGB (>=24 bit)
    class FCPixelColorLevel ; // color level (>=24 bit)
    class FCPixelThreshold ; // threshold (>=24 bit)
    class FCPixelRotate90 ; // clockwise rotate 90' (>=8 bit)
        class FCPixelRotate270 ; // clockwise rotate 270' (>=8 bit)
    class FCPixelHalftoneM3 ; // halftone (>=24 bit)
    class FCPixelOilPaint ; // oil paint (>=24 bit)
    class FCPixelColorTone ; // color tone (>=24 bit)
    class FCPixelNoisify ; // noisify (>=24 bit)
    class FCPixelSplash ; // splash (>=24 bit)
    class FCPixelVideo ; // video (>=24 bit)
    class FCPixelColorBalance ; // color balance (>=24 bit)
    class FCPixelFillGrid ; // fill grid (>=24 bit)
    class FCPixel3DGrid ; // add 3D grid (>=24 bit)
    class FCPixelMedianFilter ; // Median filter (>=24 bit)
    class FCPixelLensFlare ; // lens flare effect (>=24 bit)
    class FCPixelTileReflection ; // tile reflection effect (>=24 bit)
    class FCPixelConvolute ; // image convolute (>= 24 bit)
        class FCPixelDetectEdges ; // Detect edges (>=24 bit)
        class FCPixelSharp ; // Sharp (laplacian template) (>=24 bit)
    class FCPixelGradientBase ; // base class of gradient fill (>=24 bit)
        class FCPixelGradientLine ; // gradient fill linear (>=24 bit)
            class FCPixelGradientBiLine ; // gradient fill bilinear (>=24 bit)
            class FCPixelGradientConicalSym ; // gradient fill symmetric conical (>=24 bit)
            class FCPixelGradientConicalASym ; // gradient fill Anti-symmetric conical (>=24 bit)
        class FCPixelGradientRect ; // gradient fill rect (>=24 bit)
        class FCPixelGradientRadial ; // gradient fill radial (>=24 bit)
    class FCPixelBilinearDistord ; // bilinear distord (>=24 bit)
        class FCPixelCylinder ; // cylinder (>=24 bit)
        class FCPixelWave ; // wave (>=24 bit)
        class FCPixelWhirlPinch ; // whirl & pinch (>=24 bit)
        class FCPixelFractalTrace ; // Fractal trace (>=24 bit)
        class FCPixelLens ; // lens (>=24 bit)
        class FCPixelSkew ; // skew transform (>=24 bit)
        class FCPixelPerspective ; // perspective transform (>=24 bit)
        class FCPixelRotate ; // rotate (>=24 bit)
        class FCPixelRibbon ; // ribbon (>=24 bit)
        class FCPixelRipple ; // ripple (>=24 bit)
    class FCPixelLUTRoutine ; // LUT(look up table) routine (>=24 bit)
        class FCPixelBrightness ; // adjust brightness (>=24 bit)
        class FCPixelContrast ; // adjust contrast (>=24 bit)
        class FCPixelGamma ; // adjust gamma (>=24 bit)
        class FCPixelInvert ; // negate (>=24 bit)
        class FCPixelSolarize ; // Solarize (>=24 bit)
        class FCPixelPosterize ; // posterize (>=24 bit)
    class FCPixelColorsCount ; // count image's number of color (>=24 bit)
        class FCPixelGetKeyColor ; // Find a color unused in image (>=24 bit)
    class FCPixelWholeImageBase ; // process whole image.
        class FCPixelExportAscII ; // save a ASCII text file (>=24 bit)
        class FCPixelBlur_Box ; // blur box (>=24 bit)
        class FCPixelBlur_Zoom ; // blur zoom (>=24 bit)
        class FCPixelBlur_Radial ; // blur radial (>=24 bit)
        class FCPixelBlur_Motion ; // blur motion (>=24 bit)
        class FCPixelBlur_Gauss_IIR ; // blur IIR gauss (>=24 bit)
        class FCPixelSoftGlow ; // soft glow (>=24 bit)
        class FCPixelInnerBevel ; // add inner bevel frame (>=24 bit)
        class FCPixelSmoothEdge ; // smooth edge (32 bit)
        class FCPixelAddShadow ; // add shadow (32 bit)
        class FCPixelPatternFrame ; // frame image with pattern (>=24 bit)
        class FCPixelFillGradientFrame ; // fill a gradient frame (>=24 bit)

//=============================================================================
/**
 *  Base class of processor.
 */
class FCSinglePixelProcessBase : public FCInterface_PixelProcess
{
public:
    FCSinglePixelProcessBase() : m_pImgOld(0) {}
    virtual ~FCSinglePixelProcessBase() {if(m_pImgOld)  delete m_pImgOld;}

    /// whether the image can be disposed by this processor.
    /// default test image's bpp >= 24
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() >= 24) ;
    }

protected:
    void SetBackupImage (const FCObjImage* pImg)
    {
        if (pImg)
        {
            if (m_pImgOld)
                delete m_pImgOld ;
            m_pImgOld = new FCObjImage(*pImg) ;
        }
    }
    FCObjImage* GetBackupImage() const {return m_pImgOld;}

private:
    FCObjImage  * m_pImgOld ; // backup image
};

//=============================================================================
/**
 *  1, 4, 8, 24, 32 ==> 16.
@verbatim
    example:
        FCPixelConvertTo16Bit   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelConvertTo16Bit : public FCSinglePixelProcessBase
{
    virtual bool ValidateColorBits (const FCObjImage* pImg) {return pImg->IsValidImage() && (pImg->ColorBits() != 16) ;}
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage(pImg) ;

        // make it easier, now we only need 24(32) ==> 16
        if (pImg->ColorBits() <= 8)
            GetBackupImage()->ConvertTo24Bit() ;
        pImg->Create (pImg->Width(), pImg->Height(), 16) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        *(WORD*)pPixel = Combine16Bit_555 (GetBackupImage()->GetBits(x,y)) ; // 24,32 ==> 16
    }
    static WORD Combine16Bit_555 (const void* pRGB)
    {
        const WORD   wR = ((PCL_R(pRGB) >> 3) << 10),
                     wG = ((PCL_G(pRGB) >> 3) << 5),
                     wB =  (PCL_B(pRGB) >> 3) ;
        return (wR | wG | wB) ;
    }
};

//=============================================================================
/**
 *  1, 4, 8, 16, 24, 32 ==> 8bit gray.
@verbatim
    example:
        FCPixelConvertTo8BitGray   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelConvertTo8BitGray : public FCSinglePixelProcessBase
{
    virtual bool ValidateColorBits (const FCObjImage* pImg) {return pImg->IsValidImage();}
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage(pImg) ;

        // make it easier, now we only need 24(32) ==> 8bit gray
        if (pImg->ColorBits() <= 16)
            GetBackupImage()->ConvertTo24Bit() ;
        pImg->Create (pImg->Width(), pImg->Height(), 8) ; // default to set a gray palette
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        *pPixel = FCColor::GetGrayscale (GetBackupImage()->GetBits(x,y)) ;
    }
};

//=============================================================================
/**
 *  Gray scale image (>=24 bit).
 *  all channel are same after process.
@verbatim
    example:
        FCPixelGrayscale   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGrayscale : public FCSinglePixelProcessBase
{
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        PCL_R(pPixel) = PCL_G(pPixel) = PCL_B(pPixel) = FCColor::GetGrayscale(pPixel) ;
    }
};

//=============================================================================
/**
 *  Fill color (>=24 bit).
@verbatim
    example:
        const RGBQUAD      cr = PCL_RGBA(0,0,255) ;
        FCPixelFillColor   aCmd (cr, 192) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFillColor : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    /// @param nAlpha == -1, not fill alpha
    FCPixelFillColor (RGBQUAD crFill, int nAlpha=-1) : m_crFill(crFill), m_nAlpha(nAlpha), m_bIsFillAlpha(false) {}
private:
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        if (pImg->IsValidImage() && (pImg->ColorBits() >= 24))
        {
            m_bIsFillAlpha = ((m_nAlpha != -1) && (pImg->ColorBits() == 32)) ;
            PCL_A(&m_crFill) = m_bIsFillAlpha ? FClamp0255(m_nAlpha) : 0 ;
            return true ;
        }
        return false ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        FCColor::CopyPixel (pPixel, &m_crFill, m_bIsFillAlpha ? 4 : 3) ;
    }

    RGBQUAD   m_crFill ;
    int       m_nAlpha ;
    bool      m_bIsFillAlpha ;
};

//=============================================================================
/**
 *  Fill pattern image (32 bit).
@verbatim
    example:
        FCObjImage           * pMask = new FCObjImage ("c:\\test.jpg") ; // mustn't delete
        FCPixelFillPattern   aCmd (pMask, 192, false) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFillPattern : public FCSinglePixelProcessBase
{
public:
    /**
     *  Constructor.
     *  @param pPattern : pattern image, must use new to create and don't delete.
     */
    FCPixelFillPattern (FCObjImage* pPattern, int nAlpha, bool bOnlyTexture) : m_pPattern(pPattern), m_nAlpha(FClamp0255(nAlpha)), m_bOnlyTexture(bOnlyTexture)
    {
        if (pPattern)
            pPattern->ConvertTo24Bit() ;
    }
private:
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() == 32) && m_pPattern.get() && (m_pPattern->ColorBits() >= 24) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        BYTE   * pPat = m_pPattern->GetBits (x % m_pPattern->Width(), y % m_pPattern->Height()) ;
        if (m_bOnlyTexture)
        {
            // calculate texture
            int   n = (PCL_B(pPat)+PCL_G(pPat)+PCL_R(pPat) - 384) * m_nAlpha / 765 ;
            PCL_B(pPixel) = FClamp0255 (PCL_B(pPixel) - n) ;
            PCL_G(pPixel) = FClamp0255 (PCL_G(pPixel) - n) ;
            PCL_R(pPixel) = FClamp0255 (PCL_R(pPixel) - n) ;
        }
        else
        {
            FCColor::CombineAlphaPixel (pPixel, *(RGBQUAD*)pPixel, pPat, m_nAlpha) ;
        }
    }

    std::auto_ptr<FCObjImage>   m_pPattern ; // 24bit or 32bit
    int      m_nAlpha ;
    bool     m_bOnlyTexture ;
};

//=============================================================================
/**
 *  Adjust image's hue & saturation (>=24 bit).
@verbatim
    example:
        FCPixelHueSaturation   aCmd (100, 150) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelHueSaturation : public FCSinglePixelProcessBase
{
public:
    /**
     *  Constructor.
     *  nHue & nSaturation : between 0 and 200
     */
    FCPixelHueSaturation (int nHue, int nSaturation)
    {
        nHue = FClamp(nHue, 0, 200) ;
        nHue -= 100 ;
        nHue = 180 * nHue / 100 ;
        nSaturation = FClamp(nSaturation, 0, 200) ;
        nSaturation -= 100 ;

        for (int hue=0 ; hue < 6 ; hue++)
            for (int i=0 ; i < 256 ; i++)
            {
                int   value = nHue * 255 / 360 ;
                if ((i + value) < 0)
                    hue_transfer[hue][i] = 255 + (i + value) ;
                else if ((i + value) > 255)
                    hue_transfer[hue][i] = i + value - 255 ;
                else
                    hue_transfer[hue][i] = i + value ;

                // saturation
                value = nSaturation * 255 / 100 ;
                value = FClamp (value, -255, 255) ;
                saturation_transfer[hue][i] = FClamp0255 (i * (255 + value) / 255) ;
            }
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double   h, l, s ;
        FCColor::RGBtoHLS (pPixel, &h, &l, &s) ;
        h *= 255 ;
        s *= 255 ;

        int   hue = 0 ;
        if (h < 21)
            hue = 0 ;
        else if (h < 64)
            hue = 1 ;
        else if (h < 106)
            hue = 2 ;
        else if (h < 149)
            hue = 3 ;
        else if (h < 192)
            hue = 4 ;
        else if (h < 234)
            hue = 5 ;
        else
            hue = 0 ;

        h = hue_transfer[hue][FClamp0255((int)h)] ;
        s = saturation_transfer[hue][FClamp0255((int)s)] ;
        
        RGBQUAD   cr = FCColor::HLStoRGB(h/255.0, l, s/255.0) ;
        FCColor::CopyPixel (pPixel, &cr, 3) ;
    }

    int   hue_transfer[6][256] ;
    int   saturation_transfer[6][256] ;
};

//=============================================================================
/**
 *  Left-Right mirror image (>=8 bit).
@verbatim
    example:
        FCPixelMirror   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelMirror : public FCSinglePixelProcessBase
{
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() >= 8) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        if (x < pImg->Width()/2)
        {
            BYTE   * pRight = pImg->GetBits (pImg->Width()-1-x, y) ;
            for (int i=0 ; i < pImg->ColorBits()/8 ; i++)
                FSwap (pPixel[i], pRight[i]) ; // bytes of per pixel
        }
    }
};

//=============================================================================
/**
 *  Top-Bottom flip image (>=8 bit).
@verbatim
    example:
        FCPixelFlip   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFlip : public FCSinglePixelProcessBase
{
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() >= 8) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        if (y < pImg->Height()/2)
        {
            BYTE   * pBottom = pImg->GetBits (x, pImg->Height()-1-y) ;
            for (int i=0 ; i < pImg->ColorBits()/8 ; i++)
                FSwap (pPixel[i], pBottom[i]) ; // bytes of per pixel
        }
    }
};

//=============================================================================
/**
 *  Shift (>=24 bit).
@verbatim
    example:
        FCPixelShift   aCmd(5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelShift : public FCSinglePixelProcessBase
{
public:
    FCPixelShift (int nAmount) : m_nAmount (FMax(0,nAmount)) {srand((unsigned int)time(0));}
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        const int    nSpan = pImg->ColorBits() / 8 ;
        int          nShift = rand() % (m_nAmount+1) ;

        // first pixel every line
        if (x || !m_nAmount || !nShift)
            return ;

        BYTE     crLeft[4], crRight[4] ; // L/R edge pixel color
        FCColor::CopyPixel (crLeft, pImg->GetBits(y), nSpan) ;
        FCColor::CopyPixel (crRight, pImg->GetBits(pImg->Width()-1,y), nSpan) ;
        if (rand() % 2)
        {
            // shift right
            if (pImg->Width() > nShift)
                memmove (pImg->GetBits(nShift,y), pPixel, (pImg->Width()-nShift)*nSpan) ;
            else
                nShift = pImg->Width() ;
            for (int i=0 ; i < nShift ; i++, pPixel+=nSpan)
                FCColor::CopyPixel (pPixel, crLeft, nSpan) ;
        }
        else
        {
            // shift left
            if (pImg->Width() > nShift)
                memmove (pPixel, pImg->GetBits(nShift,y), (pImg->Width()-nShift)*nSpan) ;
            else
                nShift = pImg->Width() ;
            pPixel = pImg->GetBits(pImg->Width()-1, y) ;
            for (int i=0 ; i < nShift ; i++, pPixel-=nSpan)
                FCColor::CopyPixel (pPixel, crRight, nSpan) ;
        }
    }

    int   m_nAmount ; // max shift pixel
};

//=============================================================================
/**
 *  Auto contrast (>=24 bit).
@verbatim
    example:
        FCPixelAutoContrast   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelAutoContrast : public FCSinglePixelProcessBase
{
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        BYTE     byCmin[3] = {255, 255, 255},
                 byCmax[3] = {0, 0, 0} ;

        // Get minimum and maximum values for each channel
        for (int y=0 ; y < pImg->Height() ; y++)
            for (int x=0 ; x < pImg->Width() ; x++)
            {
                BYTE   * pPixel = pImg->GetBits(x,y) ;
                for (int b=0 ; b < 3 ; b++)
                {
                    if (pPixel[b] < byCmin[b])  byCmin[b] = pPixel[b] ;
                    if (pPixel[b] > byCmax[b])  byCmax[b] = pPixel[b] ;
                }
            }

        // Calculate LUTs with stretched contrast
        for (int b=0 ; b < 3 ; b++)
        {
            const int     nRange = byCmax[b] - byCmin[b] ;
            if (nRange)
            {
                for (int x=byCmin[b] ; x <= byCmax[b] ; x++)
                    m_byLut[x][b] = 255 * (x - byCmin[b]) / nRange ;
            }
            else
                m_byLut[byCmin[b]][b] = byCmin[b] ;
        }
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        for (int b=0 ; b < 3 ; b++)
            pPixel[b] = m_byLut[pPixel[b]][b] ;
    }

    BYTE     m_byLut[256][3] ;
};

//=============================================================================
/**
 *  Auto color enhance (>=24 bit).
@verbatim
    example:
        FCPixelAutoColorEnhance   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelAutoColorEnhance : public FCSinglePixelProcessBase
{
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        m_vhi = 0.0 ; m_vlo = 1.0 ;

        // Get minimum and maximum values
        for (int y=0 ; y < pImg->Height() ; y++)
            for (int x=0 ; x < pImg->Width() ; x++)
            {
                BYTE    * pPixel = pImg->GetBits(x,y) ;
                int     c = 255 - PCL_B(pPixel),
                        m = 255 - PCL_G(pPixel),
                        y = 255 - PCL_R(pPixel),
                        k = c ;
                if (m < k)  k = m ;
                if (y < k)  k = y ;

                BYTE    byMap[4] = { c-k, m-k, y-k } ;
                double  h, z, v ;
                FCColor::RGBtoHSV (byMap, &h, &z, &v) ;
                if (v > m_vhi)  m_vhi = v ;
                if (v < m_vlo)  m_vlo = v ;
            }
    }
    virtual void ProcessPixel (FCObjImage* pImg, int nx, int ny, BYTE* pPixel)
    {
        int     c = 255 - PCL_B(pPixel),
                m = 255 - PCL_G(pPixel),
                y = 255 - PCL_R(pPixel),
                k = c ;
        if (m < k)  k = m ;
        if (y < k)  k = y ;

        BYTE    byMap[4] = { c-k, m-k, y-k } ;
        double  h, z, v ;
        FCColor::RGBtoHSV (byMap, &h, &z, &v) ;
        if (m_vhi != m_vlo)
            v = (v-m_vlo) / (m_vhi-m_vlo) ;
        *(RGBQUAD*)byMap = FCColor::HSVtoRGB (h, z, v) ;
        c = byMap[0] ; m = byMap[1] ; y = byMap[2] ;
        c += k ; if (c > 255)  c = 255 ;
        m += k ; if (m > 255)  m = 255 ;
        y += k ; if (y > 255)  y = 255 ;
        PCL_B(pPixel) = 255 - c ;
        PCL_G(pPixel) = 255 - m ;
        PCL_R(pPixel) = 255 - y ;
    }

    double     m_vhi, m_vlo ;
};

//=============================================================================
/**
 *  Emboss effect (>=24 bit).
@verbatim
    example:
        FCPixelEmboss   aCmd(5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelEmboss : public FCSinglePixelProcessBase
{
public:
    /**
     *  Constructor.
     *  @param nAngle : [0,360]
     */
    FCPixelEmboss (int nAngle) : m_nAngle(nAngle)
    {
        double   r = AngleToRadian(nAngle),
                 dr = LIB_PI / 4.0 ;
        m_Weights[0][0] = cos(r + dr) ;
        m_Weights[0][1] = cos(r + 2.0*dr) ;
        m_Weights[0][2] = cos(r + 3.0*dr) ;

        m_Weights[1][0] = cos(r) ;
        m_Weights[1][1] = 0 ;
        m_Weights[1][2] = cos(r + 4.0*dr) ;

        m_Weights[2][0] = cos(r - dr) ;
        m_Weights[2][1] = cos(r - 2.0*dr) ;
        m_Weights[2][2] = cos(r - 3.0*dr) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
        GetBackupImage()->ExpandFrame (true, 1, 1, 1, 1) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double   sum = 0 ;
        for (int dy=0 ; dy < 3 ; dy++)
            for (int dx=0 ; dx < 3 ; dx++)
            {
                int   n = FCColor::GetGrayscale(GetBackupImage()->GetBits(x+dx,y+dy)) ;
                sum += m_Weights[dy][dx] * n ;
            }

        PCL_R(pPixel) = PCL_G(pPixel) = PCL_B(pPixel) = FClamp0255((int)sum + 128) ;
    }

    int     m_nAngle ;
    double  m_Weights[3][3] ;
};

//=============================================================================
/**
 *  Illusion effect (>=24 bit).
@verbatim
    example:
        FCPixelIllusion   aCmd(3) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelIllusion : public FCSinglePixelProcessBase
{
public:
    FCPixelIllusion (int nAmount) : m_nAmount(nAmount) {}
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;

        m_fScale = FHypot (pImg->Width(), pImg->Height()) / 2.0 ;
        m_fOffset = (int)(m_fScale / 2.0) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        const double   fXCen = pImg->Width() / 2.0,
                       fYCen = pImg->Height() / 2.0,
                       cx = (x - fXCen) / m_fScale,
                       cy = (y - fYCen) / m_fScale,
                       fTmp = LIB_PI / (double)m_nAmount ;

        double   angle = floor (atan2(cy,cx) / 2.0 / fTmp) * 2.0 * fTmp + fTmp ;
        double   radius = FHypot (cx, cy) ;
        
        int   xx = (int)(x - m_fOffset * cos (angle)),
              yy = (int)(y - m_fOffset * sin (angle)) ;
        xx = FClamp (xx, 0, pImg->Width()-1) ;
        yy = FClamp (yy, 0, pImg->Height()-1) ;

        const BYTE   * pPixel2 = GetBackupImage()->GetBits (xx, yy) ;
        for (int i=0 ; i < pImg->ColorBits()/8 ; i++)
            pPixel[i] = FClamp0255 (pPixel[i] + (int)(radius * (pPixel2[i] - pPixel[i]))) ;
    }

    int     m_nAmount ;
    double  m_fScale, m_fOffset ;
};

//=============================================================================
/**
 *  Blind effect (>=24 bit).
@verbatim
    example:
        FCPixelBlinds   aCmd (FCPixelBlinds::BLIND_X, 10, 50, PCL_RGBA(0,0,255)) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlinds : public FCSinglePixelProcessBase
{
public:
    enum BLIND_TYPE
    {
        BLIND_X,
        BLIND_Y,
    };

    /**
     *  Constructor.
     *  @param nDirect : can be FCPixelBlinds::BLIND_X or FCPixelBlinds::BLIND_Y
     *  @param nOpacity : percentage of crBlind [1,100]
     */
    FCPixelBlinds (BLIND_TYPE nDirect, int nWidth, int nOpacity, RGBQUAD crBlind)
    {
        m_nDirect = nDirect ;
        m_nWidth = nWidth ;
        m_crBlind = crBlind ;
        m_nOpacity = FClamp (nOpacity, 1, 100) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        int     nMaxWidth = FMax (pImg->Width(), pImg->Height()) ;
        m_nWidth = FClamp (m_nWidth, 2, nMaxWidth) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int     nMod ;
        if (m_nDirect == BLIND_X) // horizontal direction
            nMod = y % m_nWidth ;
        else if (m_nDirect == BLIND_Y) // vertical direction
            nMod = x % m_nWidth ;

        double     fAlphaAdd = 255.0 * m_nOpacity/100.0 / (m_nWidth-1.0) ;
        FCColor::AlphaBlendPixel (pPixel, (BYTE*)&m_crBlind, FClamp0255((int)(nMod * fAlphaAdd))) ;
    }

    BLIND_TYPE  m_nDirect ;
    int         m_nWidth ;
    int         m_nOpacity ;
    RGBQUAD     m_crBlind ;
};

//=============================================================================
/**
 *  Mosaic effect (32 bit).
@verbatim
    example:
        FCPixelMosaic   aCmd(5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelMosaic : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    /// @param nBlock : pixel width of block
    FCPixelMosaic (int nBlock) : m_nBlock(FMax(2,nBlock)) {}
private:
    virtual bool ValidateColorBits (const FCObjImage* pImg) {return pImg->IsValidImage() && (pImg->ColorBits() == 32);}
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        if ((x % m_nBlock == 0) && (y % m_nBlock == 0))
        {
            RGBQUAD   cr = GetBlockAverage(x, y) ;
            FCColor::CopyPixel (pPixel, &cr, 4) ;
        }
        else
            FCColor::CopyPixel (pPixel, pImg->GetBits(x/m_nBlock*m_nBlock, y/m_nBlock*m_nBlock), 4) ;
    }
    RGBQUAD GetBlockAverage (int x, int y)
    {
        RECT   rc = {x, y, x+m_nBlock, y+m_nBlock} ;
        GetBackupImage()->BoundRect(rc) ;

        int      nNum = RECTWIDTH(rc) * RECTHEIGHT(rc) ;
        double   nSumR=0, nSumG=0, nSumB=0, nSumA=0 ;
        for (int yy=rc.top ; yy < rc.bottom ; yy++)
        {
            for (int xx=rc.left ; xx < rc.right ; xx++)
            {
                BYTE   * p = GetBackupImage()->GetBits (xx, yy) ;
                nSumB += PCL_B(p)*PCL_A(p) ; nSumG += PCL_G(p)*PCL_A(p) ;
                nSumR += PCL_R(p)*PCL_A(p) ; nSumA += PCL_A(p) ;
            }
        }
        return PCL_RGBA (nSumA ? (int)(nSumR / nSumA) : 0,
                         nSumA ? (int)(nSumG / nSumA) : 0,
                         nSumA ? (int)(nSumB / nSumA) : 0,
                         nNum ? (int)(nSumA / nNum) : 0xFF) ;
    }

private:
    int     m_nBlock ;
};

//=============================================================================
/**
 *  Adjust image's RGB value (>=24 bit).
@verbatim
    example:
        FCPixelAdjustRGB   aCmd (-100, 50, 220) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelAdjustRGB : public FCSinglePixelProcessBase
{
public:
    /// Constructor (param's unit is delta).
    FCPixelAdjustRGB (int nR, int nG, int nB) : m_R(nR), m_G(nG), m_B(nB) {}
    void AdjustRGB (void* p)
    {
        PCL_B(p) = FClamp0255 (PCL_B(p) + m_B) ;
        PCL_G(p) = FClamp0255 (PCL_G(p) + m_G) ;
        PCL_R(p) = FClamp0255 (PCL_R(p) + m_R) ;
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        AdjustRGB (pPixel) ;
    }
    int   m_R, m_G, m_B ;
};

//=============================================================================
/**
 *  Color level (>=24 bit).
@verbatim
    example:
        FCPixelColorLevel   aCmd (false, 20, 230) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelColorLevel : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    FCPixelColorLevel (bool bAuto, int nInLow, int nInHigh, IMAGE_CHANNEL nChannel = CHANNEL_RGB)
    {
        m_bAuto = bAuto ;
        m_nInputLow[0] = m_nInputLow[1] = m_nInputLow[2] = FClamp0255(nInLow) ;
        m_nInputHigh[0] = m_nInputHigh[1] = m_nInputHigh[2] = FClamp0255(nInHigh) ;
        m_nOutputLow = 0 ; m_nOutputHigh = 255 ;
        m_bChannelR = (nChannel & CHANNEL_RED) ? true : false ;
        m_bChannelG = (nChannel & CHANNEL_GREEN) ? true : false ;
        m_bChannelB = (nChannel & CHANNEL_BLUE) ? true : false ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
        if (m_bAuto)
        {
            FCHistogram     histo(*pImg) ;
            this->AutoColorLevelChannel (histo, CHANNEL_RED) ;
            this->AutoColorLevelChannel (histo, CHANNEL_GREEN) ;
            this->AutoColorLevelChannel (histo, CHANNEL_BLUE) ;
        }
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double      fInten ;
        if (m_bChannelB)
        {
            fInten = PCL_B(pPixel) - m_nInputLow[0] ;
            if (m_nInputHigh[0] != m_nInputLow[0])
                fInten /= (double)(m_nInputHigh[0] - m_nInputLow[0]) ;
            PCL_B(pPixel) = FClamp0255(FRound(fInten * 255.0)) ;
        }
        if (m_bChannelG)
        {
            fInten = PCL_G(pPixel) - m_nInputLow[1] ;
            if (m_nInputHigh[1] != m_nInputLow[1])
                fInten /= (double)(m_nInputHigh[1] - m_nInputLow[1]) ;
            PCL_G(pPixel) = FClamp0255(FRound(fInten * 255.0)) ;
        }
        if (m_bChannelR)
        {
            fInten = PCL_R(pPixel) - m_nInputLow[2] ;
            if (m_nInputHigh[2] != m_nInputLow[2])
                fInten /= (double)(m_nInputHigh[2] - m_nInputLow[2]) ;
            PCL_R(pPixel) = FClamp0255(FRound(fInten * 255.0)) ;
        }
    }
    void AutoColorLevelChannel (const FCHistogram& histo, IMAGE_CHANNEL nChannel)
    {
        int     nIndex = 0 ;
        switch (nChannel)
        {
            case CHANNEL_RED   : nIndex = 2 ; break ;
            case CHANNEL_GREEN : nIndex = 1 ; break ;
            case CHANNEL_BLUE  : nIndex = 0 ; break ;
            default : assert(false) ; break ;
        }

        const int     nCount = histo.GetCount (nChannel) ;
        if (nCount == 0)
        {
            m_nInputLow[nIndex] = m_nInputHigh[nIndex] = 0 ;
        }
        else
        {
            m_nInputLow[nIndex] = 0 ;
            m_nInputHigh[nIndex] = 255 ;
            // Set the low input
            int     new_count = 0, i ;
            for (i=0 ; i < 255 ; i++)
            {
                new_count += histo.GetValueCount (i, nChannel) ;
                double   percentage = new_count / (double)nCount ;
                double   next_percentage = (new_count + histo.GetValueCount (i+1, nChannel)) / (double)nCount ;
                if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
                {
                    m_nInputLow[nIndex] = i + 1 ;
                    break ;
                }
            }
            // Set the high input
            new_count = 0 ;
            for (i=255 ; i > 0 ; i--)
            {
                new_count += histo.GetValueCount (i, nChannel) ;
                double   percentage = new_count / (double)nCount ;
                double   next_percentage = (new_count + histo.GetValueCount (i-1, nChannel)) / (double)nCount ;
                if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
                {
                    m_nInputHigh[nIndex] = i - 1 ;
                    break ;
                }
            }
        }
    }
private:
    int      m_nInputLow[3], m_nInputHigh[3] ;
    int      m_nOutputLow, m_nOutputHigh ;
    bool     m_bAuto, m_bChannelR, m_bChannelG, m_bChannelB ;
};

//=============================================================================
/**
 *  Threshold image (>=24 bit).
@verbatim
    example:
        FCPixelThreshold   aCmd (128) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelThreshold : public FCSinglePixelProcessBase
{
public:
    /// Constructor (nLevel range [0,255]).
    FCPixelThreshold (int nLevel) : m_nLevel(FClamp0255(nLevel)) {}
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        RGBQUAD   cr = (FCColor::GetGrayscale(pPixel) > m_nLevel) ? FCColor::crWhite() : FCColor::crBlack() ;
        FCColor::CopyPixel (pPixel, &cr, 3) ;
    }
    int     m_nLevel ;
};

//=============================================================================
/**
 *  Clockwise rotate 90' (>=8 bit).
@verbatim
    example:
        FCPixelRotate90   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelRotate90 : public FCSinglePixelProcessBase
{
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() >= 8) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        FCColor::CopyPixel (pPixel,
                            GetBackupImage()->GetBits (y, pImg->Width()-1-x),
                            pImg->ColorBits()/8) ;
    }
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;

        // create new rotated image
        pImg->Create (pImg->Height(), pImg->Width(), pImg->ColorBits()) ;
        if (pImg->ColorBits() <= 8)
            pImg->CopyPalette(*GetBackupImage()) ;
    }
};

//=============================================================================
/**
 *  Clockwise rotate 270' (>=8 bit).
@verbatim
    example:
        FCPixelRotate270   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelRotate270 : public FCPixelRotate90
{
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        FCColor::CopyPixel (pPixel,
                            GetBackupImage()->GetBits (pImg->Height()-1-y, x),
                            pImg->ColorBits()/8) ;
    }
};

//=============================================================================
/**
 *  Halftone (>=24 bit), use Limb Pattern M3 algorithm.
@verbatim
    example:
        FCPixelHalftoneM3   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelHalftoneM3 : public FCSinglePixelProcessBase
{
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        static BYTE  s_BayerPattern[8][8] = // 64 level gray
        {
            0,32,8,40,2,34,10,42,
            48,16,56,24,50,18,58,26,
            12,44,4,36,14,46,6,38,
            60,28,52,20,62,30,54,22,
            3,35,11,43,1,33,9,41,
            51,19,59,27,49,17,57,25,
            15,47,7,39,13,45,5,37,
            63,31,55,23,61,29,53,21
        };
        BYTE   gr = FCColor::GetGrayscale (pPixel) ;
        PCL_R(pPixel) = PCL_G(pPixel) = PCL_B(pPixel) = (((gr>>2) > s_BayerPattern[y&7][x&7]) ? 0xFF : 0) ;
    }
};

//=============================================================================
/**
 *  Oil paint (>=24 bit).
@verbatim
    example:
        FCPixelOilPaint   aCmd (2) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelOilPaint : public FCSinglePixelProcessBase
{
public:
    /// Constructor (nRadius >= 1).
    FCPixelOilPaint (int nRadius)
    {
        m_nRadius = FMax(1,nRadius) ;
        m_nLength = 2*m_nRadius + 1 ; // Stat. block
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
        GetBackupImage()->ExpandFrame (true, m_nRadius, m_nRadius, m_nRadius, m_nRadius) ;

        // calculate block gray
        m_ImgGray = *GetBackupImage() ;
        FCPixelConvertTo8BitGray   aCmd ;
        m_ImgGray.SinglePixelProcessProc(aCmd) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        BYTE    * pSelPixel = 0 ;
        int     nMaxNum = 0,
                pHistogram[256] ;
        memset (pHistogram, 0, sizeof(int) * 256) ;

        // replace every pixel use most frequency
        for (int ny=0 ; ny < m_nLength ; ny++)
            for (int i=0 ; i < m_nLength ; i++)
            {
                BYTE   * pGray = m_ImgGray.GetBits (x+i, y+ny) ;
                if (++pHistogram[*pGray] > nMaxNum)
                {
                    nMaxNum = pHistogram[*pGray] ;
                    pSelPixel = GetBackupImage()->GetBits (x+i, y+ny) ;
                }
            }
        FCColor::CopyPixel (pPixel, pSelPixel, 3) ; // leave alpha channel
    }

    int     m_nRadius ; // >= 1
    int     m_nLength ; // 2*m_nRadius + 1
    FCObjImage   m_ImgGray ;
};

//=============================================================================
/**
 *  Color tone (>=24 bit).
@verbatim
    example:
        FCPixelColorTone   aCmd (PCL_RGBA(0,0,255)) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelColorTone : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    FCPixelColorTone (RGBQUAD crTone)
    {
        double   L, S ;
        FCColor::RGBtoHLS (&crTone, &m_nHue, &L, &S) ;
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int   n = FCColor::GetGrayscale(pPixel) ;
        PCL_B(pPixel) = (BYTE)(255 * pow(n/255.0, 1.2)) ;
        PCL_G(pPixel) = (BYTE)(255 * pow(n/255.0, 1)) ;
        PCL_R(pPixel) = (BYTE)(255 * pow(n/255.0, 0.8)) ;

        double   H, L, S ;
        FCColor::RGBtoHLS (pPixel, &H, &L, &S) ;
        RGBQUAD   cr = FCColor::HLStoRGB (m_nHue, L, S * 150 / 100) ;
        FCColor::CopyPixel (pPixel, &cr, 3) ;
    }
    double   m_nHue ;
};

//=============================================================================
/**
 *  Noisify (>=24 bit).
@verbatim
    example:
        FCPixelNoisify   aCmd (5, false) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelNoisify : public FCSinglePixelProcessBase
{
public:
    /**
     *  Constructor.
     *  @param nLevel : level (0 <= nLevel <= 100).
     */
    FCPixelNoisify (int nLevel, bool bRandom = false)
    {
        m_nLevel = FClamp (nLevel,0,100) ;
        srand ((unsigned int)time(0)) ;
        m_bRandom = bRandom ;
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int     n1, n2, n3 ;
        if (m_bRandom)
        {
            n1=GenGauss(); n2=GenGauss(); n3=GenGauss();
        }
        else
        {
            n1=n2=n3=GenGauss() ;
        }
        PCL_B(pPixel) = FClamp0255 (PCL_B(pPixel) + n1) ;
        PCL_G(pPixel) = FClamp0255 (PCL_G(pPixel) + n2) ;
        PCL_R(pPixel) = FClamp0255 (PCL_R(pPixel) + n3) ;
    }
    int GenGauss()
    {
        double   d = (rand() + rand() + rand() + rand()) * 5.28596089837e-5 - 3.46410161514 ;
        return (int)(m_nLevel * d * 127.0 / 100.0) ;
    }

    int    m_nLevel ;
    bool   m_bRandom ;
};

//=============================================================================
/**
 *  Splash image (>=24 bit).
@verbatim
    example:
        FCPixelSplash   aCmd (5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelSplash : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    /// @param nBlock : splash level (>=3).
    FCPixelSplash (int nBlock)
    {
        m_nBlock = FMax (3, nBlock) ;
        srand((unsigned int)time(0)) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int   xCopy = x - m_nBlock/2 + (rand() % m_nBlock),
              yCopy = y - m_nBlock/2 + (rand() % m_nBlock) ;
        xCopy = FClamp (xCopy, 0, pImg->Width()-1) ;
        yCopy = FClamp (yCopy, 0, pImg->Height()-1) ;
        BYTE   * pSrc = GetBackupImage()->GetBits(xCopy,yCopy) ;
        FCColor::CopyPixel (pPixel, pSrc, pImg->ColorBits()/8) ;
    }
    int     m_nBlock ;
};

//=============================================================================
/**
 *  Video (>=24 bit).
@verbatim
    example:
        FCPixelVideo   aCmd (FCPixelVideo::VIDEO_STAGGERED) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelVideo : public FCSinglePixelProcessBase
{
public:
    enum VIDEO_TYPE {VIDEO_STAGGERED=0, VIDEO_TRIPED=1, VIDEO_3X3=2, VIDEO_DOTS=3} ;
    /// Constructor.
    /// @param nVideoType : VIDEO_STAGGERED, VIDEO_TRIPED, VIDEO_3X3, VIDEO_DOTS
    FCPixelVideo (VIDEO_TYPE nVideoType) : m_VideoType(nVideoType)
    {
        assert(nVideoType>=VIDEO_STAGGERED && nVideoType<=VIDEO_DOTS);
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        static const int   pattern_width[] = {2, 1, 3, 5} ;
        static const int   pattern_height[] = {6, 3, 3, 15} ;
        static const int   video_pattern[4][15 * 5/* max pattern size */] =
        {
            {
                0, 1,
                0, 2,
                1, 2,
                1, 0,
                2, 0,
                2, 1,
            },
            {
                0,
                1,
                2,
            },
            {
                0, 1, 2,
                2, 0, 1,
                1, 2, 0,
            },
            {
                0, 1, 2, 0, 0,
                1, 1, 1, 2, 0,
                0, 1, 2, 2, 2,
                0, 0, 1, 2, 0,
                0, 1, 1, 1, 2,
                2, 0, 1, 2, 2,
                0, 0, 0, 1, 2,
                2, 0, 1, 1, 1,
                2, 2, 0, 1, 2,
                2, 0, 0, 0, 1,
                1, 2, 0, 1, 1,
                2, 2, 2, 0, 1,
                1, 2, 0, 0, 0,
                1, 1, 2, 0, 1,
                1, 2, 2, 2, 0,
            }
        };

        int   nWidth = pattern_width[m_VideoType],
              nHeight = pattern_height[m_VideoType] ;
        for (int i=0 ; i < 3 ; i++)
            if (video_pattern[m_VideoType][nWidth * (y%nHeight) + (x%nWidth)] == i)
                pPixel[i] = FClamp0255 (2 * pPixel[i]) ;
    }

    VIDEO_TYPE   m_VideoType ;
};

//=============================================================================
/**
 *  Color balance (>=24 bit).
@verbatim
    example:
        FCPixelColorBalance   aCmd (true, TONE_SHADOWS, -30, 20, 30) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelColorBalance : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    FCPixelColorBalance (bool bPreLum, TONE_REGION ToneRgn, int cyan_red, int magenta_green, int yellow_blue)
    {
        m_bPreserveLuminosity = bPreLum ;

        int     cyan_red_rgn[3] = {0,0,0},
                magenta_green_rgn[3] = {0,0,0},
                yellow_blue_rgn[3] = {0,0,0} ;
        cyan_red_rgn[ToneRgn] = cyan_red ;
        magenta_green_rgn[ToneRgn] = magenta_green ;
        yellow_blue_rgn[ToneRgn] = yellow_blue ;

        // add for lightening, sub for darkening
        PCL_array<double>  highlights_add(256), midtones_add(256), shadows_add(256),
                           highlights_sub(256), midtones_sub(256), shadows_sub(256) ;
        int     i ;
        for (i=0 ; i < 256 ; i++)
        {
            highlights_add[i] = shadows_sub[255 - i] = (1.075 - 1 / (i / 16.0 + 1)) ;
            midtones_add[i] = midtones_sub[i] = 0.667 * (1 - FSquare ((i - 127.0) / 127.0)) ;
            shadows_add[i] = highlights_sub[i] = 0.667 * (1 - FSquare ((i - 127.0) / 127.0)) ;
        }

        // Set the transfer arrays (for speed)
        double   * cyan_red_transfer[3], * magenta_green_transfer[3], * yellow_blue_transfer[3] ;
        cyan_red_transfer[TONE_SHADOWS] = (cyan_red_rgn[TONE_SHADOWS] > 0) ? shadows_add.get() : shadows_sub.get() ;
        cyan_red_transfer[TONE_MIDTONES] = (cyan_red_rgn[TONE_MIDTONES] > 0) ? midtones_add.get() : midtones_sub.get() ;
        cyan_red_transfer[TONE_HIGHLIGHTS] = (cyan_red_rgn[TONE_HIGHLIGHTS] > 0) ? highlights_add.get() : highlights_sub.get() ;
        magenta_green_transfer[TONE_SHADOWS] = (magenta_green_rgn[TONE_SHADOWS] > 0) ? shadows_add.get() : shadows_sub.get() ;
        magenta_green_transfer[TONE_MIDTONES] = (magenta_green_rgn[TONE_MIDTONES] > 0) ? midtones_add.get() : midtones_sub.get() ;
        magenta_green_transfer[TONE_HIGHLIGHTS] = (magenta_green_rgn[TONE_HIGHLIGHTS] > 0) ? highlights_add.get() : highlights_sub.get() ;
        yellow_blue_transfer[TONE_SHADOWS] = (yellow_blue_rgn[TONE_SHADOWS] > 0) ? shadows_add.get() : shadows_sub.get() ;
        yellow_blue_transfer[TONE_MIDTONES] = (yellow_blue_rgn[TONE_MIDTONES] > 0) ? midtones_add.get() : midtones_sub.get() ;
        yellow_blue_transfer[TONE_HIGHLIGHTS] = (yellow_blue_rgn[TONE_HIGHLIGHTS] > 0) ? highlights_add.get() : highlights_sub.get() ;

        for (i=0 ; i < 256 ; i++)
        {
            int     r_n = i, g_n = i, b_n = i ;

            r_n += (int)(cyan_red_rgn[TONE_SHADOWS] * cyan_red_transfer[TONE_SHADOWS][r_n]);        r_n = FClamp0255(r_n);
            r_n += (int)(cyan_red_rgn[TONE_MIDTONES] * cyan_red_transfer[TONE_MIDTONES][r_n]);      r_n = FClamp0255(r_n);
            r_n += (int)(cyan_red_rgn[TONE_HIGHLIGHTS] * cyan_red_transfer[TONE_HIGHLIGHTS][r_n]);  r_n = FClamp0255(r_n);

            g_n += (int)(magenta_green_rgn[TONE_SHADOWS] * magenta_green_transfer[TONE_SHADOWS][g_n]);        g_n = FClamp0255(g_n);
            g_n += (int)(magenta_green_rgn[TONE_MIDTONES] * magenta_green_transfer[TONE_MIDTONES][g_n]);      g_n = FClamp0255(g_n);
            g_n += (int)(magenta_green_rgn[TONE_HIGHLIGHTS] * magenta_green_transfer[TONE_HIGHLIGHTS][g_n]);  g_n = FClamp0255(g_n);

            b_n += (int)(yellow_blue_rgn[TONE_SHADOWS] * yellow_blue_transfer[TONE_SHADOWS][b_n]);        b_n = FClamp0255(b_n);
            b_n += (int)(yellow_blue_rgn[TONE_MIDTONES] * yellow_blue_transfer[TONE_MIDTONES][b_n]);      b_n = FClamp0255(b_n);
            b_n += (int)(yellow_blue_rgn[TONE_HIGHLIGHTS] * yellow_blue_transfer[TONE_HIGHLIGHTS][b_n]);  b_n = FClamp0255(b_n);

            m_pLookupR[i] = r_n ;
            m_pLookupG[i] = g_n ;
            m_pLookupB[i] = b_n ;
        }
    }
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        RGBQUAD     rgb ;
        PCL_B(&rgb) = m_pLookupB[PCL_B(pPixel)] ;
        PCL_G(&rgb) = m_pLookupG[PCL_G(pPixel)] ;
        PCL_R(&rgb) = m_pLookupR[PCL_R(pPixel)] ;
        if (m_bPreserveLuminosity) // preserve luminosity
        {
            double   H, L, S ;
            FCColor::RGBtoHLS (&rgb, &H, &L, &S) ;

            // calculate L value
            int   cmax = FMax (PCL_R(pPixel), FMax (PCL_G(pPixel), PCL_B(pPixel))),
                  cmin = FMin (PCL_R(pPixel), FMin (PCL_G(pPixel), PCL_B(pPixel))) ;
            L = (cmax+cmin) / 2.0 / 255.0 ;

            rgb = FCColor::HLStoRGB (H, L, S) ;
        }
        PCL_B(pPixel) = PCL_B(&rgb) ;
        PCL_G(pPixel) = PCL_G(&rgb) ;
        PCL_R(pPixel) = PCL_R(&rgb) ;
    }

    BYTE     m_pLookupR[256], m_pLookupG[256], m_pLookupB[256] ;
    bool     m_bPreserveLuminosity ;
};

//=============================================================================
/**
 *  Fill grid (>=24 bit).
@verbatim
    example:
        FCPixelFillGrid   aCmd (PCL_RGBA(0,255,0), PCL_RGBA(0,0,255), 5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFillGrid : public FCSinglePixelProcessBase
{
public:
    /**
     *  Constructor.
     *  set alpha to 0xFF if image color is 32bpp.
     *  @param nPitch : pixel width of grid.
     */
    FCPixelFillGrid (RGBQUAD cr1, RGBQUAD cr2, int nPitch) : m_cr1(cr1), m_cr2(cr2), m_nPitch(FMax(1,nPitch)) {}
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int     nX = x / m_nPitch, nY = y / m_nPitch ;
        FCColor::CopyPixel (pPixel, ((nX + nY) % 2 == 0) ? &m_cr1 : &m_cr2, 3) ;

        if (pImg->ColorBits() == 32)
            PCL_A(pPixel) = 0xFF ;
    }
    RGBQUAD   m_cr1, m_cr2 ;
    int       m_nPitch ;
};

//=============================================================================
/**
 *  Add 3D grid (>=24 bit).
@verbatim
    example:
        FCPixel3DGrid   aCmd (32, 60) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixel3DGrid : public FCSinglePixelProcessBase
{
public:
    FCPixel3DGrid (int nSize, int nDepth) : m_nSize(FMax(1,nSize)), m_nDepth(nDepth) {}
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int     nDelta = 0 ;
        if (((y-1) % m_nSize == 0) && (x % m_nSize != 0) && ((x+1) % m_nSize != 0))
            nDelta = -m_nDepth ; // top
        else if (((y+2) % m_nSize == 0) && (x % m_nSize != 0) && ((x+1) % m_nSize != 0))
            nDelta = m_nDepth ; // bottom
        else if (((x-1) % m_nSize == 0) && (y % m_nSize != 0) && ((y+1) % m_nSize != 0))
            nDelta = m_nDepth ; // left
        else if (((x+2) % m_nSize == 0) && (y % m_nSize != 0) && ((y+1) % m_nSize != 0))
            nDelta = -m_nDepth ; // right

        PCL_R(pPixel) = FClamp0255 (PCL_R(pPixel) + nDelta) ;
        PCL_G(pPixel) = FClamp0255 (PCL_G(pPixel) + nDelta) ;
        PCL_B(pPixel) = FClamp0255 (PCL_B(pPixel) + nDelta) ;
    }

    int     m_nSize, m_nDepth ;
};

//=============================================================================
/**
 *  Median filter (>=24 bit).
@verbatim
    example:
        FCPixelMedianFilter   aCmd (3) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelMedianFilter : public FCSinglePixelProcessBase
{
public:
    /// Constructor (nSize >= 2).
    FCPixelMedianFilter (int nSize)
    {
        m_nSize = FMax(2, nSize) ;
        m_pBlock = new BlockElem [m_nSize * m_nSize] ;
    }
    virtual ~FCPixelMedianFilter()
    {
        delete[] m_pBlock ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;

        // duplicate edge
        int     nLeftTop = m_nSize/2,
                nRightDown = nLeftTop ;
        GetBackupImage()->ExpandFrame (true, nLeftTop, nLeftTop, nRightDown, nRightDown) ;

        // backup image
        m_BakImage = *GetBackupImage() ;

        // calculate gray image
        FCPixelConvertTo8BitGray   aCmd ;
        GetBackupImage()->SinglePixelProcessProc (aCmd) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        int     i = 0 ;
        for (int m=0 ; m < m_nSize ; m++)
            for (int n=0 ; n < m_nSize ; n++)
            {
                m_pBlock[i].nGray = *GetBackupImage()->GetBits (x+n, y+m) ;
                FCColor::CopyPixel (&m_pBlock[i].crOrigin, m_BakImage.GetBits(x+n, y+m), 3) ;
                i++ ;
            }

        ::qsort (m_pBlock, i, sizeof(BlockElem), __CompareGray) ;
        FCColor::CopyPixel (pPixel, &m_pBlock[i/2], 3) ;
    }

    struct BlockElem
    {
        RGBQUAD  crOrigin ;
        int      nGray ;
    };

    static int __CompareGray (const void* arg1, const void* arg2)
    {
        return ((BlockElem*)arg1)->nGray - ((BlockElem*)arg2)->nGray ;
    }

    int         m_nSize ;
    BlockElem   * m_pBlock ;
    FCObjImage  m_BakImage ;
};

//=============================================================================
#include "LensFlare.h"

//=============================================================================
#include "TileReflection.h"

//=============================================================================
/// Image convolute (>= 24 bit)
class FCPixelConvolute : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    FCPixelConvolute() : m_pElement(0)
    {
        m_iBlock=0 ; m_nOffset=0 ; m_iDivisor=1 ;
    }
    virtual ~FCPixelConvolute()
    {
        if (m_pElement)
            delete[] m_pElement ;
    }

    /**
     *  Set convolute kernel.
     *  @param nElements : array from top-left of matrix.
     *  @param iBlockLen : width of matrix.
     */
    void SetKernel (const int* nElements, int iBlockLen, int iDivisor, int nOffset=0)
    {
        if (!nElements || (iBlockLen < 1))
            {assert(false); return;}

        if (m_pElement)
            delete[] m_pElement ;
        m_pElement = new int[FSquare(iBlockLen)] ;

        for (int i=0 ; i < FSquare(iBlockLen) ; i++)
            m_pElement[i] = nElements[i] ;

        m_iBlock = iBlockLen ;
        m_iDivisor = FMax(1,iDivisor) ;
        m_nOffset = nOffset ;
    }

private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;

        // duplicate edge, easier to processs
        int   nLeftTop = m_iBlock/2,
              nRightDown = nLeftTop ;
        GetBackupImage()->ExpandFrame (true, nLeftTop, nLeftTop, nRightDown, nRightDown) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        // calculate the sum of sub-block
        int      nSumR=0, nSumG=0, nSumB=0, i=0 ;
        for (int iy=0 ; iy < m_iBlock ; iy++)
            for (int ix=0 ; ix < m_iBlock ; ix++, i++)
            {
                BYTE   * pOld = GetBackupImage()->GetBits (x+ix,y+iy) ;
                nSumB += PCL_B(pOld) * m_pElement[i] ;
                nSumG += PCL_G(pOld) * m_pElement[i] ;
                nSumR += PCL_R(pOld) * m_pElement[i] ;
            }
        // set pixel
        PCL_B(pPixel) = FClamp0255 (m_nOffset + nSumB / m_iDivisor) ;
        PCL_G(pPixel) = FClamp0255 (m_nOffset + nSumG / m_iDivisor) ;
        PCL_R(pPixel) = FClamp0255 (m_nOffset + nSumR / m_iDivisor) ;
    }

    int     * m_pElement ;
    int     m_iBlock, m_iDivisor, m_nOffset ;
};

//=============================================================================
/**
 *  Detect edges (>=24 bit).
@verbatim
    example:
        FCPixelDetectEdges   aCmd(3) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelDetectEdges : public FCPixelConvolute
{
public:
    /// Constructor (nRadius >= 1).
    FCPixelDetectEdges (int nRadius = 3)
    {
        int   nBlock = 2*FMax(1,nRadius) + 1,
              nDivisor = 1,
              nOffset = 0,
              nWidth = nBlock * nBlock ;

        PCL_array<int>   pKernel (nWidth) ;
        for (int i=0 ; i < nWidth ; i++)
            pKernel[i] = -1 ;
        pKernel[nWidth/2] = nWidth - 1 ;
        SetKernel (pKernel.get(), nBlock, nDivisor, nOffset) ;
    }
};

//=============================================================================
/**
 *  Sharp (laplacian template) (>=24 bit).
@verbatim
    example:
        FCPixelSharp   aCmd(3) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelSharp : public FCPixelConvolute
{
public:
    /// Constructor (nStep >= 1).
    FCPixelSharp (int nStep)
    {
        int   arKernel[] = {-1,-1,-1,-1,8+nStep,-1,-1,-1,-1},
              nBlock = 3,
              nDivisor = FMax (1,nStep),
              nOffset = 0 ;
        SetKernel (arKernel, nBlock, nDivisor, nOffset) ;
    }
};

//=============================================================================
/// Base class of gradient fill (>=24 bit)
class FCPixelGradientBase : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    FCPixelGradientBase (RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat=REPEAT_NONE) : m_crStart(crStart), m_crEnd(crEnd), m_nRepeat(nRepeat) {}
protected:
    /// calculate factor of point(x,y)
    virtual double CalculateFactor (int nX, int nY) =0 ;
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double     fFac = this->CalculateFactor (x, y) ;
        switch (m_nRepeat)
        {
        case REPEAT_NONE : fFac = FClamp (fFac, 0.0, 1.0); break;
        case REPEAT_SAWTOOTH :
            if (fFac < 0.0)
                fFac = 1.0 - FDoubleMod1 (-fFac) ;
            else
                fFac = FDoubleMod1 (fFac) ;
            break ;
        case REPEAT_TRIANGULAR :
            if (fFac < 0.0)
                fFac = -fFac ;
            if ( ((int)fFac) & 1 )
                fFac = 1.0 - FDoubleMod1 (fFac) ;
            else
                fFac = FDoubleMod1 (fFac) ;
            break ;
        }
        PCL_B(pPixel) = (BYTE)(PCL_B(&m_crStart) + (PCL_B(&m_crEnd)-PCL_B(&m_crStart)) * fFac) ;
        PCL_G(pPixel) = (BYTE)(PCL_G(&m_crStart) + (PCL_G(&m_crEnd)-PCL_G(&m_crStart)) * fFac) ;
        PCL_R(pPixel) = (BYTE)(PCL_R(&m_crStart) + (PCL_R(&m_crEnd)-PCL_R(&m_crStart)) * fFac) ;
    }
    double FDoubleMod1 (const double &x)
    {
        // the function <fmod> is extreme slow :-(, so we just do it.
        // the function == fmod (x, 1.0)
        return x - (int)x ;
    }

    RGBQUAD      m_crStart, m_crEnd ;
    REPEAT_MODE  m_nRepeat ; // type of repeat
};

//=============================================================================
/**
 *  Gradient fill linear (>=24 bit).
@verbatim
    example:
        POINT     ptStart={0,0}, ptEnd={100,100} ;
        FCPixelGradientLine  aCmd (ptStart, ptEnd, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientLine : public FCPixelGradientBase
{
public:
    /**
     *  Constructor.
     *  @param ptStart : start coordinate on image.
     *  @param ptEnd : end coordinate on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientLine (POINT ptStart, POINT ptEnd, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientBase (crStart, crEnd, nRepeat)
    {
        m_ptStart = ptStart; m_ptEnd = ptEnd;
        m_fDist = FHypot (m_ptStart.x-m_ptEnd.x, m_ptStart.y-m_ptEnd.y) ;
        m_fRatX = (m_ptEnd.x-m_ptStart.x) / m_fDist ;
        m_fRatY = (m_ptEnd.y-m_ptStart.y) / m_fDist ;
    }
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   rat = m_fRatX * (nX-m_ptStart.x) + m_fRatY * (nY-m_ptStart.y) ;
        rat = rat / m_fDist ;
        return (rat < 0.0) ? 0.0 : rat ;
    }
protected:
    POINT      m_ptStart, m_ptEnd ; // coordinate on image
    double     m_fRatX, m_fRatY ;
    double     m_fDist ;
};

//=============================================================================
/**
 *  Gradient fill bilinear (>=24 bit).
@verbatim
    example:
        POINT     ptStart={0,0}, ptEnd={100,100} ;
        FCPixelGradientBiLine  aCmd (ptStart, ptEnd, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientBiLine : public FCPixelGradientLine
{
public:
    /**
     *  Constructor.
     *  @param ptStart : start coordinate on image.
     *  @param ptEnd : end coordinate on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientBiLine (POINT ptStart, POINT ptEnd, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientLine (ptStart, ptEnd, crStart, crEnd, nRepeat) {}
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   rat = m_fRatX * (nX-m_ptStart.x) + m_fRatY * (nY-m_ptStart.y) ;
        rat = rat / m_fDist ;
        return fabs(rat) ;
    }
};

//=============================================================================
/**
 *  Gradient fill symmetric conical (>=24 bit).
@verbatim
    example:
        POINT     ptStart={0,0}, ptEnd={100,100} ;
        FCPixelGradientConicalSym  aCmd (ptStart, ptEnd, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientConicalSym : public FCPixelGradientLine
{
public:
    /**
     *  Constructor.
     *  @param ptStart : start coordinate on image.
     *  @param ptEnd : end coordinate on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientConicalSym (POINT ptStart, POINT ptEnd, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientLine (ptStart, ptEnd, crStart, crEnd, nRepeat) {}
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   rat ;
        double   dx = nX-m_ptStart.x, dy = nY-m_ptStart.y ;
        if ((dx != 0) || (dy != 0))
        {
            double    dr = FHypot (dx, dy) ;
            rat = m_fRatX * dx / dr + m_fRatY * dy / dr ;
            rat = FClamp (rat, -1.0, 1.0) ;
            rat = acos(rat) / LIB_PI ;
            rat = FClamp (rat, 0.0, 1.0) ;
        }
        else
            rat = 0.5 ;
        return rat ;
    }
};

//=============================================================================
/**
 *  Gradient fill Anti-symmetric conical (>=24 bit).
@verbatim
    example:
        POINT     ptStart={0,0}, ptEnd={100,100} ;
        FCPixelGradientConicalSym  aCmd (ptStart, ptEnd, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientConicalASym : public FCPixelGradientLine
{
public:
    /**
     *  Constructor.
     *  @param ptStart : start coordinate on image.
     *  @param ptEnd : end coordinate on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientConicalASym (POINT ptStart, POINT ptEnd, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientLine (ptStart, ptEnd, crStart, crEnd, nRepeat) {}
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   rat ;
        double   dx = nX-m_ptStart.x, dy = nY-m_ptStart.y ;
        if ((dx != 0) || (dy != 0))
        {
            double   ang0, ang1, ang ;
            ang0 = atan2 (m_fRatX, m_fRatY) + LIB_PI ;
            ang1 = atan2 (dx, dy) + LIB_PI ;
            ang = ang1 - ang0 ;
            if (ang < 0.0)
                ang += LIB_2PI ;

            rat = ang / LIB_2PI ;
            rat = FClamp (rat, 0.0, 1.0) ;
        }
        else
            rat = 0.5 ;
        return rat ;
    }
};

//=============================================================================
/**
 *  Gradient fill rect (>=24 bit).
@verbatim
    example:
        RECT     rc = {0, 0, 100, 100} ;
        FCPixelGradientRect  aCmd (rc, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientRect : public FCPixelGradientBase
{
public:
    /**
     *  Constructor.
     *  @param rcRect : rect on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientRect (RECT rcRect, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientBase (crStart, crEnd, nRepeat)
    {
        assert (!IsRectEmpty(&rcRect)) ;
        m_fCenX = (rcRect.left + rcRect.right) / 2.0 ;
        m_fCenY = (rcRect.top + rcRect.bottom) / 2.0 ;
        m_fRadiusX = RECTWIDTH(rcRect) / 2.0 ;
        m_fRadiusY = RECTHEIGHT(rcRect) / 2.0 ;
    }
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   ratX = fabs((nX-m_fCenX) / m_fRadiusX),
                 ratY = fabs((nY-m_fCenY) / m_fRadiusY) ;
        return FMax(ratX, ratY) ;
    }
protected:
    double     m_fCenX, m_fCenY ;
    double     m_fRadiusX, m_fRadiusY ;
};

//=============================================================================
/**
 *  Gradient fill radial (>=24 bit).
@verbatim
    example:
        RECT     rc = {0, 0, 100, 100} ;
        FCPixelGradientRadial  aCmd (rc, PCL_RGBA(0,0,0), PCL_RGBA(0,0,255), REPEAT_NONE) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGradientRadial : public FCPixelGradientBase
{
public:
    /**
     *  Constructor.
     *  @param rcEllipse : rect on image.
     *  @param nRepeat : REPEAT_NONE, REPEAT_SAWTOOTH, REPEAT_TRIANGULAR
     */
    FCPixelGradientRadial (RECT rcEllipse, RGBQUAD crStart, RGBQUAD crEnd, REPEAT_MODE nRepeat = REPEAT_NONE) : FCPixelGradientBase (crStart, crEnd, nRepeat)
    {
        assert (!IsRectEmpty(&rcEllipse)) ;
        m_fCenX = (rcEllipse.left + rcEllipse.right) / 2.0 ;
        m_fCenY = (rcEllipse.top + rcEllipse.bottom) / 2.0 ;
        m_fRadiusX = RECTWIDTH(rcEllipse) / 2.0 ;
        m_fRadiusY = RECTHEIGHT(rcEllipse) / 2.0 ;
    }
protected:
    virtual double CalculateFactor (int nX, int nY)
    {
        double   rat = FHypot((nX-m_fCenX)/m_fRadiusX, (nY-m_fCenY)/m_fRadiusY) ;
        return rat ;
    }
private:
    double     m_fCenX, m_fCenY ;
    double     m_fRadiusX, m_fRadiusY ;
};

//=============================================================================
/// Bilinear distord (>=24 bit).
/// if derived class override OnEnterProcess, it must call OnEnterProcess of base.
class FCPixelBilinearDistord : public FCSinglePixelProcessBase
{
protected:
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return FCSinglePixelProcessBase::ValidateColorBits(pImg) && (pImg->Width() >= 2) && (pImg->Height() >= 2) ;
    }
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double   un_x, un_y ;
        if (!calc_undistorted_coord (x, y, un_x, un_y))
            return ;

        // this is ugly
        un_x = FClamp (un_x, (double)-0x7FFFFF00, (double)0x7FFFFF00) ;
        un_y = FClamp (un_y, (double)-0x7FFFFF00, (double)0x7FFFFF00) ;

        static RGBQUAD   s_crNull = PCL_RGBA(0xFF,0xFF,0xFF,0) ;

        int   nSrcX = (int)((un_x < 0) ? (un_x-1) : un_x),
              nSrcY = (int)((un_y < 0) ? (un_y-1) : un_y),
              nSrcX_1 = nSrcX + 1,
              nSrcY_1 = nSrcY + 1 ;

        const BYTE   * pcrPixel[4] =
        {
            GetBackupImage()->IsInside(nSrcX,nSrcY) ? GetBackupImage()->GetBits(nSrcX,nSrcY) : (BYTE*)&s_crNull,
            GetBackupImage()->IsInside(nSrcX_1,nSrcY) ? GetBackupImage()->GetBits(nSrcX_1,nSrcY) : (BYTE*)&s_crNull,
            GetBackupImage()->IsInside(nSrcX,nSrcY_1) ? GetBackupImage()->GetBits(nSrcX,nSrcY_1) : (BYTE*)&s_crNull,
            GetBackupImage()->IsInside(nSrcX_1,nSrcY_1) ? GetBackupImage()->GetBits(nSrcX_1,nSrcY_1) : (BYTE*)&s_crNull,
        } ;

        RGBQUAD   cr = FCColor::Get_Bilinear_Pixel (un_x-nSrcX, un_y-nSrcY, pImg->ColorBits() == 32, pcrPixel) ;
        FCColor::CopyPixel (pPixel, &cr, pImg->ColorBits()/8) ;
    }

    // returned bool variable to declare continue or not
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y) =0 ;
};

//=============================================================================
/**
 *  Cylinder (>=24 bit).
@verbatim
    example:
        FCPixelCylinder  aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelCylinder : public FCPixelBilinearDistord
{
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        FCPixelBilinearDistord::OnEnterProcess(pImg) ;

        // position on origin image.
        for (int x=0 ; x < pImg->Width() ; x++)
        {
            double   R = pImg->Width() / 2.0 ;
            m_ColIndex.push_back (2 * R * acos ((R-x)/R) / LIB_PI) ;
        }
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        un_x = m_ColIndex[x] ;
        un_y = y ;
        return true ;
    }
    std::deque<double>   m_ColIndex ;
};

//=============================================================================
/**
 *  Wave (>=24 bit).
@verbatim
    example:
        FCPixelWave  aCmd (25, 30) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelWave : public FCPixelBilinearDistord
{
public:
    /// Constructor.
    FCPixelWave (int nWavelength, int nAmplitude, double fPhase=0) : m_nWavelength(2*FMax(1,nWavelength)), m_nAmplitude(FMax(1,nAmplitude)), m_fPhase(fPhase) {}
private:
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        const int  nImgWidth = GetBackupImage()->Width(),
                   nImgHeight = GetBackupImage()->Height() ;
        double     fScaleX = 1.0, fScaleY = 1.0 ;
        if (nImgWidth < nImgHeight)
            fScaleX = nImgHeight / (double)nImgWidth ;
        else if (nImgWidth > nImgHeight)
            fScaleY = nImgWidth / (double)nImgHeight ;

        // Distances to center, scaled
        double   fCenX = GetBackupImage()->Width() / 2.0,
                 fCenY = GetBackupImage()->Height() / 2.0,
                 dx = (x - fCenX) * fScaleX,
                 dy = (y - fCenY) * fScaleY,
                 amnt = m_nAmplitude * sin (LIB_2PI * FHypot(dx,dy) / (double)m_nWavelength + m_fPhase) ;
        un_x = (amnt + dx) / fScaleX + fCenX ;
        un_y = (amnt + dy) / fScaleY + fCenY ;
        un_x = FClamp (un_x, 0.0, GetBackupImage()->Width()-1.0) ;
        un_y = FClamp (un_y, 0.0, GetBackupImage()->Height()-1.0) ;
        return true ;
    }

    double  m_fPhase ; // [0..2n]
    int     m_nAmplitude ;
    int     m_nWavelength ;
};

//=============================================================================
/**
 *  Whirl & Pinch (>=24 bit).
@verbatim
    example:
        FCPixelWhirlPinch  aCmd (1.5, 0.5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelWhirlPinch : public FCPixelBilinearDistord
{
public:
    /// Constructor.
    /// @param fWhirl : [-2n,2n]
    /// @param fPinch : [-1.0,1.0]
    FCPixelWhirlPinch (double fWhirl, double fPinch)
    {
        m_fWhirl = FClamp (fWhirl, -LIB_2PI, LIB_2PI) ;
        m_fPinch = FClamp (fPinch, -1.0, 1.0) ;
    }
private:
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        
        int      nImgWidth = GetBackupImage()->Width(),
                 nImgHeight = GetBackupImage()->Height() ;
        double   fScaleX = 1.0, fScaleY = 1.0 ;
        if (nImgWidth < nImgHeight)
            fScaleX = nImgHeight / (double)nImgWidth ;
        else if (nImgWidth > nImgHeight)
            fScaleY = nImgWidth / (double)nImgHeight ;

        // Distances to center, scaled
        double   fCenX = GetBackupImage()->Width() / 2.0,
                 fCenY = GetBackupImage()->Height() / 2.0,
                 fRadius = FMax (fCenX, fCenY),
                 dx = (x - fCenX) * fScaleX,
                 dy = (y - fCenY) * fScaleY ;
        double   d = dx*dx + dy*dy ; // Distance^2 to center of *circle* (scaled ellipse)
        double   fSqrtD = sqrt (d) ;

        // If we are inside circle, then distort, else, just return the same position
        bool     bInside = (fSqrtD < fRadius) ;
        // exclude center point
        if (fSqrtD < FLT_EPSILON)
            bInside = false ;
        if (bInside)
        {
            //        double  fDist = sqrt (d / m_fRadiusScale) / m_fRadius ;
            double  fDist = fSqrtD / fRadius ;

            // Pinch
            double  fFactor = pow (sin (LIB_PI / 2.0 * fDist), -m_fPinch) ;
            dx *= fFactor ; dy *= fFactor ;

            // Whirl
            double   fAng = m_fWhirl * FSquare (1.0 - fDist) ;
            double   sina = sin (fAng), cosa = cos (fAng) ;
            un_x = (cosa * dx - sina * dy) / fScaleX + fCenX ;
            un_y = (sina * dx + cosa * dy) / fScaleY + fCenY ;
        }
        else
        {
            un_x = x ; un_y = y ;
        }
        return bInside;
    }

    double  m_fWhirl ; // radian of whirl
    double  m_fPinch ; // [-1.0, 1.0]
    //    double  m_fRadiusScale ; // [0.0, 2.0]
};

//=============================================================================
/**
 *  Fractal trace (>=24 bit).
@verbatim
    example:
        FCPixelFractalTrace  aCmd (2) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFractalTrace : public FCPixelBilinearDistord
{
public:
    /// Constructor (nDepth >= 1).
    FCPixelFractalTrace (int nDepth) : m_nDepth(FMax(1,nDepth)) {}
private:
    void mandelbrot (double x, double y, double* u, double* v) const
    {
        int     iter = 0 ;
        double  xx = x, yy = y ;
        double  x2 = xx * xx,
                y2 = yy * yy ;

        while (iter++ < m_nDepth)
        {
            double   tmp = x2 - y2 + x ;
            yy = 2 * xx * yy + y ;
            xx = tmp ;
            x2 = xx * xx ;
            y2 = yy * yy ;
        }
        *u = xx ; *v = yy ;
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        double   fImgWidth = GetBackupImage()->Width(),
                 fImgHeight = GetBackupImage()->Height(),
                 fScaleX = 1.5 / fImgWidth,
                 fScaleY = 2.0 / fImgHeight,
                 cy = -1.0 + y * fScaleY,
                 cx = -1.0 + x * fScaleX,
                 px, py ;
        mandelbrot (cx, cy, &px, &py) ;
        un_x = (px + 1.0) / fScaleX ;
        un_y = (py + 1.0) / fScaleY ;

        if ( !(0 <= un_x && un_x < fImgWidth && 0 <= un_y && un_y < fImgHeight) )
        {
            un_x = fmod (un_x, fImgWidth) ;
            un_y = fmod (un_y, fImgHeight) ;
            if (un_x < 0.0)  un_x += fImgWidth ;
            if (un_y < 0.0)  un_y += fImgHeight ;
        }
        return true ;
    }
    int     m_nDepth ; // >=1
};

//=============================================================================
/**
 *  Lens (>=24 bit).
@verbatim
    example:
        FCPixelLens  aCmd (1.5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelLens : public FCPixelBilinearDistord
{
public:
    /// Constructor (fRefraction >= 1.0).
    FCPixelLens (double fRefraction, bool bKeepBk) : m_fRefraction(FMax(1.0,fRefraction)), m_bKeepBk(bKeepBk) {}
private:
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        if (m_bKeepBk)
        {
            un_x=x ; un_y=y ;
        }

        double   fCenX = GetBackupImage()->Width() / 2.0,
                 fCenY = GetBackupImage()->Height() / 2.0,
                 asqr = fCenX * fCenX,
                 bsqr = fCenY * fCenY,
                 csqr = FSquare(FMin(fCenX,fCenY)),
                 dy = fCenY - y,
                 ysqr = FSquare(dy),
                 dx = x - fCenX,
                 xsqr = FSquare(dx) ;
        if (ysqr < (bsqr - (bsqr * xsqr) / asqr))
        {
            double  fTmp = sqrt ((1 - xsqr/asqr - ysqr/bsqr) * csqr) ;
            double  fTmpsqr = FSquare(fTmp) ;

            double  nxangle = acos (dx / sqrt(xsqr+fTmpsqr)) ;
            double  theta2 = asin (sin (LIB_PI/2.0 - nxangle) / m_fRefraction) ;
            theta2 = LIB_PI/2.0 - nxangle - theta2 ;
            double  xx = dx - tan (theta2) * fTmp ;

            double  nyangle = acos (dy / sqrt(ysqr+fTmpsqr)) ;
            theta2 = asin (sin (LIB_PI/2.0 - nyangle) / m_fRefraction) ;
            theta2 = LIB_PI/2.0 - nyangle - theta2 ;
            double  yy = dy - tan (theta2) * fTmp ;

            un_x = xx + fCenX ;
            un_y = fCenY - yy ;
        }
        return true ;
    }
    double   m_fRefraction ; // >= 1.0
    bool     m_bKeepBk ;
};

//=============================================================================
/**
 *  Skew transform (>=24 bit).
 */
class FCPixelSkew : public FCPixelBilinearDistord
{
public:
    /**
     *  Constructor.
     *  @param ptNewPos : new position of point LT, RT, RB, LB 
     */
    FCPixelSkew (POINT ptNewPos[4])
    {
        memcpy (m_ptNewPos, ptNewPos, sizeof(POINT) * 4) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        FCPixelBilinearDistord::OnEnterProcess (pImg) ;

        // create skewed image
        m_nNewWidth = FMax (abs(m_ptNewPos[0].x-m_ptNewPos[2].x), abs(m_ptNewPos[1].x-m_ptNewPos[3].x)) ;
        m_nNewHeight = FMax (abs(m_ptNewPos[0].y-m_ptNewPos[2].y), abs(m_ptNewPos[1].y-m_ptNewPos[3].y)) ;
        pImg->Create (m_nNewWidth, m_nNewHeight, pImg->ColorBits()) ;
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        if (m_ptNewPos[0].x != m_ptNewPos[3].x)
        {
            // x axis slope
            int     nDelta = m_ptNewPos[0].x - m_ptNewPos[3].x ;
            un_x = x - ((nDelta > 0) ? (m_nNewHeight - y) : y) * abs(nDelta) / (double)m_nNewHeight ;
            un_y = y * GetBackupImage()->Height() / (double)m_nNewHeight ;
        }
        else if (m_ptNewPos[0].y != m_ptNewPos[1].y)
        {
            // y axis slope
            int     nDelta = m_ptNewPos[0].y - m_ptNewPos[1].y ;
            un_x = x * GetBackupImage()->Width() / (double)m_nNewWidth ;
            un_y = y - ((nDelta > 0) ? (m_nNewWidth - x) : x) * abs(nDelta) / (double)m_nNewWidth ;
        }
        else
        {
            un_x=x ; un_y=y;
        }
        return true ;
    }
private:
    POINT   m_ptNewPos[4] ; // LT, RT, RB, LB
    int     m_nNewWidth ;
    int     m_nNewHeight ;
};

//=============================================================================
/**
 *  Perspective transform (>=24 bit).
 */
class FCPixelPerspective : public FCPixelBilinearDistord
{
public:
    /**
     *  Constructor.
     *  @param ptNewPos : new position of point LT, RT, RB, LB 
     */
    FCPixelPerspective (POINT ptNewPos[4])
    {
        memcpy (m_ptNewPos, ptNewPos, sizeof(POINT) * 4) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        FCPixelBilinearDistord::OnEnterProcess (pImg) ;

        // create sloped image
        m_nNewWidth = FMax (abs(m_ptNewPos[0].x-m_ptNewPos[1].x), abs(m_ptNewPos[2].x-m_ptNewPos[3].x)) ;
        m_nNewHeight = FMax (abs(m_ptNewPos[0].y-m_ptNewPos[3].y), abs(m_ptNewPos[1].y-m_ptNewPos[2].y)) ;
        pImg->Create (m_nNewWidth, m_nNewHeight, pImg->ColorBits()) ;
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        if (m_ptNewPos[0].y != m_ptNewPos[1].y)
        {
            // y axis perspective
            int     nDelta = abs(m_ptNewPos[0].y-m_ptNewPos[3].y) - abs(m_ptNewPos[1].y-m_ptNewPos[2].y) ;
            double  fOffset = fabs(nDelta * ((nDelta > 0) ? x : (m_nNewWidth-x)) / (2.0 * m_nNewWidth)) ;
            un_y = GetBackupImage()->Height() * (y - fOffset) / (m_nNewHeight - 2.0 * fOffset) ;
            un_x = GetBackupImage()->Width() * x / (double)m_nNewWidth ;
        }
        else if (m_ptNewPos[0].x != m_ptNewPos[3].x)
        {
            // x axis perspective
            int     nDelta = abs(m_ptNewPos[0].x-m_ptNewPos[1].x) - abs(m_ptNewPos[2].x-m_ptNewPos[3].x) ;
            double  fOffset = fabs(nDelta * ((nDelta > 0) ? y : (m_nNewHeight-y)) / (2.0 * m_nNewHeight)) ;
            un_x = GetBackupImage()->Width() * (x - fOffset) / (m_nNewWidth - 2.0 * fOffset) ;
            un_y = GetBackupImage()->Height() * y / (double)m_nNewHeight ;
        }
        else
        {
            un_x = x ; un_y = y ;
        }
        return true ;
    }
private:
    POINT   m_ptNewPos[4] ; // LT, RT, RB, LB
    int     m_nNewWidth ;
    int     m_nNewHeight ;
};

//=============================================================================
/**
 *  Rotate image (>=24 bit).
@verbatim
    example:
        FCPixelRotate   aCmd (45) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelRotate : public FCPixelBilinearDistord
{
public:
    /**
     *  Constructor.
     *  @param nAngle : 0 --> 360
     */
    FCPixelRotate (int nAngle)
    {
        while (nAngle < 0)
            nAngle += 360 ;

        nAngle %= 360 ;
        m_fAngle = AngleToRadian(nAngle) ;
        m_fInvAngle = AngleToRadian(360 - nAngle) ;
    }
private:
    static void RotatePoint (double& x, double& y, double center_x, double center_y, double fAngle)
    {
        double   dx = x - center_x, dy = -y + center_y,
                 cost = cos(fAngle), sint = sin(fAngle) ;
        x = center_x + (dx*cost + dy*sint) ;
        y = center_y - (dy*cost - dx*sint) ;
    }
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        FCPixelBilinearDistord::OnEnterProcess (pImg) ;

        // calculate new width & height
        double   cen_x=pImg->Width()/2.0, cen_y=pImg->Height()/2.0,
                 x1=0, y1=0,
                 x2=pImg->Width(), y2=0,
                 x3=0, y3=pImg->Height(),
                 x4=pImg->Width(), y4=pImg->Height() ;

        RotatePoint (x1, y1, cen_x, cen_y, m_fAngle) ;
        RotatePoint (x2, y2, cen_x, cen_y, m_fAngle) ;
        RotatePoint (x3, y3, cen_x, cen_y, m_fAngle) ;
        RotatePoint (x4, y4, cen_x, cen_y, m_fAngle) ;

        double  L = FMin(x1,FMin(x2,FMin(x3,x4))),
                T = FMin(y1,FMin(y2,FMin(y3,y4))),
                R = FMax(x1,FMax(x2,FMax(x3,x4))),
                B = FMax(y1,FMax(y2,FMax(y3,y4))) ;

        m_nNewWidth = (int)ceil(R - L) ;
        m_nNewHeight = (int)ceil(B - T) ;
        pImg->Create (m_nNewWidth, m_nNewHeight, pImg->ColorBits()) ;
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        double   cen_x = (m_nNewWidth-1)/2.0, cen_y = (m_nNewHeight-1)/2.0 ;

        un_x=x ; un_y=y ;
        RotatePoint (un_x, un_y, cen_x, cen_y, m_fInvAngle) ;

        un_x -= (m_nNewWidth - GetBackupImage()->Width()) / 2.0 ;
        un_y -= (m_nNewHeight - GetBackupImage()->Height()) / 2.0 ;
        return true ;
    }

    double   m_fAngle, m_fInvAngle ;
    int      m_nNewWidth, m_nNewHeight ;
};

//=============================================================================
/**
 *  Ribbon (>=24 bit).
@verbatim
    example:
        FCPixelRibbon   aCmd (80, 30) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelRibbon : public FCPixelBilinearDistord
{
public:
    /// Constructor.
    /// @param nSwing : [0..100],  percentage
    /// @param nFrequency : >=0,  a pi every 10
    FCPixelRibbon (int nSwing, int nFrequency)
    {
        m_nSwing = FClamp (nSwing, 0, 100) ;
        m_nFreq = FMax (nFrequency, 0) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        FCPixelBilinearDistord::OnEnterProcess (pImg) ;
        
        // clear image
        memset (pImg->GetMemStart(), 0, pImg->GetPitch()*pImg->Height()) ;

        m_fDelta = m_nSwing * pImg->Height() * 75.0/100/100 ; // upper, max 75%
        for (int i=0 ; i < (int)m_fDelta ; i++)
            memset (GetBackupImage()->GetBits(i), 0, GetBackupImage()->GetPitch()) ;

        double   fAngleSpan = m_nFreq * LIB_PI / 10.0 / pImg->Width() ;
        for (int x=0 ; x < pImg->Width() ; x++)
        {
            double   d = (1-cos(x * fAngleSpan)) * m_fDelta / 2.0 ;
            m_ShiftDown.push_back (d) ;
        }
    }
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        un_x = x ;
        un_y = y + m_fDelta - m_ShiftDown[x] ;
        return true ;
    }

    int      m_nSwing, m_nFreq ;
    double   m_fDelta ;
    std::deque<double>   m_ShiftDown ;
};

//=============================================================================
/**
 *  Ripple (>=24 bit).
@verbatim
    example:
        FCPixelRipple   aCmd (10, 30) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelRipple : public FCPixelBilinearDistord
{
public:
    /// Constructor.
    FCPixelRipple (int nWavelength, int nAmplitude, bool bSinType = true)
    {
        m_nWavelength = FMax (1, nWavelength) ;
        m_nAmplitude = FMax (1, nAmplitude) ;
        m_bSinType = bSinType ;
    }
private:
    virtual bool calc_undistorted_coord (int x, int y, double& un_x, double& un_y)
    {
        double   w = GetBackupImage()->Width() ;
        un_x = fmod (x + w + shift_amount(y), w) ;
        un_x = FClamp (un_x, 0.0, w-1) ;
        un_y = y ;
        return true ;
    }
    double shift_amount (int nPos) const
    {
        if (m_bSinType)
            return m_nAmplitude * sin(nPos*LIB_2PI/(double)m_nWavelength) ;
        else
            return floor (m_nAmplitude * (fabs ((((nPos % m_nWavelength) / (double)m_nWavelength) * 4) - 2) - 1)) ;
    }

    int     m_nWavelength ;
    int     m_nAmplitude ;
    bool    m_bSinType ;
};

//=============================================================================
/// LUT(look up table) routine (>=24 bit)
class FCPixelLUTRoutine : public FCSinglePixelProcessBase
{
public:
    /// Constructor.
    /// @param nChannel : process channel, use OR to combine
    FCPixelLUTRoutine (IMAGE_CHANNEL nChannel = CHANNEL_RGB)
    {
        m_bChannelR = nChannel & CHANNEL_RED ;
        m_bChannelG = nChannel & CHANNEL_GREEN ;
        m_bChannelB = nChannel & CHANNEL_BLUE ;
    }
protected:
    /// Initialize LUT.
    virtual int InitLUTtable (int nLUTIndex) =0 ;
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        for (int i=0 ; i <= 0xFF ; i++)
            m_LUT[i] = this->InitLUTtable (i) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        if (m_bChannelB)  PCL_B(pPixel) = m_LUT[PCL_B(pPixel)] ;
        if (m_bChannelG)  PCL_G(pPixel) = m_LUT[PCL_G(pPixel)] ;
        if (m_bChannelR)  PCL_R(pPixel) = m_LUT[PCL_R(pPixel)] ;
    }
private:
    int     m_LUT[256] ;
    int     m_bChannelR, m_bChannelG, m_bChannelB ;
};

//=============================================================================
/**
 *  Adjust brightness (>=24 bit).
@verbatim
    example:
        FCPixelBrightness   aCmd (150) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBrightness : public FCPixelLUTRoutine
{
public:
    /**
     *  Constructor.
     *  nLevel : between 0 and 200
     */
    FCPixelBrightness (int nLevel, IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel)
    {
        m_nLevel = FClamp(nLevel, 0, 200) ;
        m_nLevel -= 100 ;
    }
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        if (m_nLevel <= 0)
            return FClamp0255 (nLUTIndex + nLUTIndex * m_nLevel / 200) ;
        else
            return FClamp0255 (nLUTIndex + (255-nLUTIndex) * m_nLevel / 200) ;
    }
    int   m_nLevel ;
};

//=============================================================================
/**
 *  Adjust contrast (>=24 bit).
@verbatim
    example:
        FCPixelContrast   aCmd (150) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelContrast : public FCPixelLUTRoutine
{
public:
    /**
     *  Constructor.
     *  nLevel : between 0 and 200
     */
    FCPixelContrast (int nLevel, IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel)
    {
        m_nLevel = FClamp(nLevel, 0, 200) ;
        m_nLevel -= 100 ;
    }
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        double   contrast = (double)m_nLevel / 100.0,
                 value = (double)nLUTIndex / 255.0,
                 n = (value > 0.5) ? (1.0 - value) : value ;

        if (n < 0)
            n = 0 ;

        if (contrast < 0)
        {
            n = 0.5 * pow (2.0 * n, 1.0 + contrast) ;
        }
        else
        {
            double   power = (m_nLevel == 100) ? 127 : (1.0 / (1.0 - contrast)) ;
            n = 0.5 * pow (2.0 * n, power) ;
        }

        value = (value > 0.5) ? (1.0 - n) : n ;
        return FClamp0255 ((int)(value*255)) ;
    }
    int   m_nLevel ;
};

//=============================================================================
/**
 *  Adjust gamma (>=24 bit).
@verbatim
    example:
        FCPixelGamma   aCmd (0.5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelGamma : public FCPixelLUTRoutine
{
public:
    /// Constructor (param must >= 0.0).
    FCPixelGamma (double fGamma, IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel)
    {
        fGamma = FMax (0.0, fGamma) ;
        m_fInvGamma = 1.0 / fGamma ;
    }
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        double   fMax = pow (255.0, m_fInvGamma) / 255.0 ;
        return FClamp0255 (FRound (pow((double)nLUTIndex, m_fInvGamma) / fMax)) ;
    }
    double     m_fInvGamma ;
};

//=============================================================================
/**
 *  Negate image(>=24 bit).
@verbatim
    example:
        FCPixelInvert   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelInvert : public FCPixelLUTRoutine
{
public:
    FCPixelInvert (IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel) {}
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        return (255 - nLUTIndex) ;
    }
};

//=============================================================================
/**
 *  Solarize image(>=24 bit).
@verbatim
    example:
        FCPixelSolarize   aCmd (128) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelSolarize : public FCPixelLUTRoutine
{
public:
    /// Constructor (nThreshold in [0-255]).
    FCPixelSolarize (int nThreshold, IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel), m_nThreshold(FClamp0255(nThreshold)) {}
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        return (nLUTIndex >= m_nThreshold) ? (255 - nLUTIndex) : nLUTIndex ;
    }
    int     m_nThreshold ;
};

//=============================================================================
/**
 *  Posterize image(>=24 bit).
@verbatim
    example:
        FCPixelPosterize   aCmd (2) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelPosterize : public FCPixelLUTRoutine
{
public:
    /// Constructor (nLevel >= 2).
    FCPixelPosterize (int nLevel, IMAGE_CHANNEL nChannel = CHANNEL_RGB) : FCPixelLUTRoutine(nChannel), m_nLevel(FMax(2,nLevel)) {}
private:
    virtual int InitLUTtable (int nLUTIndex)
    {
        double   d = 255.0 / (m_nLevel - 1.0) ;
        return FClamp0255 (FRound (d * FRound (nLUTIndex / d))) ;
    }
    int   m_nLevel ;
};

//=============================================================================
/**
*  Count image's number of color (>=24 bit).
@verbatim
    example:
        FCPixelColorsCount   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
        aCmd.GetColorsNumber() ;
@endverbatim
 */
class FCPixelColorsCount : public FCSinglePixelProcessBase
{
public:
    FCPixelColorsCount() : m_pMap(0), m_nCount(0) {}
    virtual ~FCPixelColorsCount() {if(m_pMap) delete[] m_pMap;}
    /// Get used number of color.
    unsigned int GetColorsNumber() const {return m_nCount;}
protected:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        int     iMaxColor = 1 << 24 ;
        m_pMap = new BYTE[iMaxColor+1] ;
        memset (m_pMap, 0, iMaxColor+1) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        DWORD     i = 0 ;
        FCColor::CopyPixel (&i, pPixel, 3) ;
        if (m_pMap[i] == 0)
        {
            m_pMap[i] = 1 ;
            m_nCount++ ;
        }
    }

    unsigned int  m_nCount ;
    BYTE          * m_pMap ;
};

//=============================================================================
/**
*  Find a color unused in image (>=24 bit).
@verbatim
    example:
        FCPixelGetKeyColor   aCmd ;
        img.SinglePixelProcessProc (aCmd) ;
        aCmd.IsFind() ;
        aCmd.GetKeyColor() ;
@endverbatim
 */
class FCPixelGetKeyColor : public FCPixelColorsCount
{
public:
    /// Is found a color unused in image.
    bool IsFind() const {return m_bFind;}
    /// Get the color unused in image.
    RGBQUAD GetKeyColor() const {return m_crKey;}
private:
    virtual void OnLeaveProcess (FCObjImage* pImg)
    {
        m_bFind = false ;
        for (int i=0 ; i <= 0xFFFFFF ; i++)
            if (m_pMap[i] == 0)
            {
                *(DWORD*)&m_crKey = i ;
                m_bFind = true ;
                break ;
            }
    }

    RGBQUAD   m_crKey ;
    bool      m_bFind ;
};

//=============================================================================
/// Base class to process whole image.
class FCPixelWholeImageBase : public FCSinglePixelProcessBase
{
    virtual PROCESS_TYPE QueryProcessType() {return PROCESS_TYPE_WHOLE;}
private:
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel) {assert(false);}
};

//=============================================================================
/**
 *  Save a ASCII text file (>=24 bit).
@verbatim
    example:
        FCPixelExportAscII   aCmd ("c:\\PhoXo.txt") ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelExportAscII : public FCPixelWholeImageBase
{
public:
    /// Constructor.
    FCPixelExportAscII (const char* szFileName)
    {
        m_pFile = fopen (szFileName, "wb") ; assert(m_pFile);

        char   ch[95] =
        {
            ' ',
            '`','1','2','3','4','5','6','7','8','9','0','-','=','\\',
            'q','w','e','r','t','y','u','i','o','p','[',']',
            'a','s','d','f','g','h','j','k','l',';','\'',
            'z','x','c','v','b','n','m',',','.','/',
            '~','!','@','#','$','%','^','&','*','(',')','_','+','|',
            'Q','W','E','R','T','Y','U','I','O','P','{','}',
            'A','S','D','F','G','H','J','K','L',':','"',
            'Z','X','C','V','B','N','M','<','>','?'
        };
        int   gr[95] =
        {
            0,
            7,22,28,31,31,27,32,22,38,32,40, 6,12,20,38,32,26,20,24,40,
            29,24,28,38,32,32,26,22,34,24,44,33,32,32,24,16, 6,22,26,22,
            26,34,29,35,10, 6,20,14,22,47,42,34,40,10,35,21,22,22,16,14,
            26,40,39,29,38,22,28,36,22,36,30,22,22,36,26,36,25,34,38,24,
            36,22,12,12,26,30,30,34,39,42,41,18,18,22
        };

        // Bubble Sort
        for (int i=0 ; i < 94 ; i++)
            for (int j=i+1 ; j < 95 ; j++)
                if (gr[i] > gr[j])
                {
                    FSwap (ch[i], ch[j]) ;
                    FSwap (gr[i], gr[j]) ;
                }

        memcpy (m_chIndex, ch, 95*sizeof(char)) ;
        memcpy (m_nGray, gr, 95*sizeof(int)) ;
    }
    virtual ~FCPixelExportAscII()
    {
        fclose(m_pFile) ;
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        FCPixelInvert     aCmd ; // most of image is brightness
        GetBackupImage()->SinglePixelProcessProc (aCmd) ;
        FCPixelConvertTo8BitGray   aGray ;
        GetBackupImage()->SinglePixelProcessProc (aGray) ;

        const int     nTransWidth = pImg->Width() / 8,
                      nTransHeight = pImg->Height() / 16 ;
        for (int y=0 ; y < nTransHeight ; y++)
        {
            for (int x=0 ; x < nTransWidth ; x++)
            {
                int     nGray = 0 ;
                for (int k=0 ; k < 16 ; k++)
                    for(int h=0 ; h < 8 ; h++)
                    {
                        BYTE   * pGray = GetBackupImage()->GetBits (8*x+h, y*16+k) ;
                        nGray += *pGray ;
                    }
                    nGray /= 16*8 ;
                    nGray = m_nGray[94] * nGray / 255 ;
                    int   t = 0 ;
                    while (m_nGray[t+1] < nGray)
                        t++ ;
                    fwrite (&m_chIndex[t], 1, sizeof(char), m_pFile) ;
            }
            char    tchar = (char)0x0D ;
            fwrite (&tchar, 1, sizeof(char), m_pFile) ;
            tchar = (char)0x0A ;
            fwrite (&tchar, 1, sizeof(char), m_pFile) ;
        }
    }

    char     m_chIndex[95] ;
    int      m_nGray[95] ;
    FILE     * m_pFile ;
};

//=============================================================================
#include "Blur_Box.h"

//=============================================================================
/**
 *  Blur zoom (>=24 bit).
@verbatim
    example:
        FCPixelBlur_Zoom   aCmd (15) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlur_Zoom : public FCPixelWholeImageBase
{
public:
    /// Constructor.
    FCPixelBlur_Zoom (int nLength) : m_nLength(FMax(0,nLength)) {}
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        for (int y=0 ; y < pImg->Height() ; y++)
        {
            for (int x=0 ; x < pImg->Width() ; x++)
            {
                // Stat.
                int     nSumB=0, nSumG=0, nSumR=0, nSumA=0,
                        i=0 ;
                for (i=0 ; i < m_nLength ; i++)
                {
                    int     nCenX = pImg->Width()/2, nCenY = pImg->Height()/2,
                            xx = (int)(nCenX + (x-nCenX) * (1.0 + 0.02 * i)),
                            yy = (int)(nCenY + (y-nCenY) * (1.0 + 0.02 * i)) ;
                    if (!GetBackupImage()->IsInside(xx,yy))
                        break ;

                    BYTE   * p = GetBackupImage()->GetBits (xx, yy) ;
                    int    nA = (pImg->ColorBits() == 32) ? PCL_A(p) : 0xFF ;
                    nSumA += nA ;
                    nSumB += nA * PCL_B(p) ;
                    nSumG += nA * PCL_G(p) ;
                    nSumR += nA * PCL_R(p) ;
                }

                // set pixel
                BYTE   * pWrite = pImg->GetBits(x,y) ;
                if (nSumA)
                {
                    PCL_B(pWrite) = nSumB/nSumA ;
                    PCL_G(pWrite) = nSumG/nSumA ;
                    PCL_R(pWrite) = nSumR/nSumA ;
                }
                if ((pImg->ColorBits() == 32) && i)
                    PCL_A(pWrite) = nSumA/i ;
            }
            if (pProgress)
                pProgress->SetProgress (100 * (y+1) / pImg->Height()) ;
        }
    }

    int     m_nLength ;
};

//=============================================================================
/**
 *  Blur radial (>=24 bit).
@verbatim
    example:
        FCPixelBlur_Radial   aCmd (30) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlur_Radial : public FCPixelWholeImageBase
{
public:
    /// Constructor.
    FCPixelBlur_Radial (int nAngle) : m_nAngle(abs(nAngle) % 360) {}
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        int   nCenX = pImg->Width()/2, nCenY = pImg->Height()/2,
              R = (int)FHypot (FMax(nCenX,pImg->Width()-nCenX), FMax(nCenY,pImg->Height()-nCenY)),
              n = (int)(4 * AngleToRadian(m_nAngle) * sqrt((double)R) + 2) ;

        PCL_array<double>   ct(n),
                            st(n) ;
        double   theta = (double)AngleToRadian(m_nAngle) / ((double)(n - 1)),
                 offset = double(theta * (n - 1) / 2.0) ;
        for (int i=0 ; i < n ; i++)
        {
            ct[i] = cos (theta * i - offset) ;
            st[i] = sin (theta * i - offset) ;
        }

        for (int y=0 ; y < pImg->Height() ; y++)
        {
            for (int x=0 ; x < pImg->Width() ; x++)
            {
                int   xr = x - nCenX, yr = y - nCenY,
                      r = (int)sqrt ((double)FSquare(xr) + (double)FSquare(yr)),
                      nStep ;
                if (r == 0)
                    nStep = 1 ;
                else
                {
                    nStep = R/r ;
                    if (nStep == 0)
                        nStep = 1 ;
                    else
                        if (nStep > n-1)
                            nStep = n-1 ;
                }

                // Stat.
                int     nSumB=0, nSumG=0, nSumR=0, nSumA=0,
                        nCount = 0 ;
                for (int i=0 ; i < n ; i += nStep)
                {
                    int     xx = (int)(nCenX + xr * ct[i] - yr * st[i]),
                            yy = (int)(nCenY + xr * st[i] + yr * ct[i]) ;
                    if (!GetBackupImage()->IsInside(xx,yy))
                        continue ;

                    nCount++ ;

                    BYTE   * p = GetBackupImage()->GetBits (xx,yy) ;
                    int    nA = (pImg->ColorBits() == 32) ? PCL_A(p) : 0xFF ;
                    nSumA += nA ;
                    nSumB += nA * PCL_B(p) ;
                    nSumG += nA * PCL_G(p) ;
                    nSumR += nA * PCL_R(p) ;
                }

                // set pixel
                BYTE   * pWrite = pImg->GetBits(x,y) ;
                if (nSumA)
                {
                    PCL_B(pWrite) = nSumB/nSumA ;
                    PCL_G(pWrite) = nSumG/nSumA ;
                    PCL_R(pWrite) = nSumR/nSumA ;
                }
                if ((pImg->ColorBits() == 32) && nCount)
                    PCL_A(pWrite) = nSumA/nCount ;
            }
            if (pProgress)
                pProgress->SetProgress (100 * (y+1) / pImg->Height()) ;
        }
    }

    int     m_nAngle ; // [0, 360]
};

//=============================================================================
/**
 *  Blur motion (>=24 bit).
@verbatim
    example:
        FCPixelBlur_Motion   aCmd (15, DIRECT_LEFT) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlur_Motion : public FCPixelWholeImageBase
{
public:
    /// Constructor.
    FCPixelBlur_Motion (int nStep, DIRECT_SYS Direct) : m_nStep(nStep), m_Direct(Direct) {}
private:
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        if (pImg->Width() < 5)
            return ;

        m_nStep = FClamp (m_nStep, 2, (int)pImg->Width()-2) ;

        for (int y=0 ; y < pImg->Height() ; y++)
        {
            int       nCurrX = 0,
                      nSpanX = 1 ;
            RGBQUAD   rgb ; // pixel at edge
            switch (m_Direct)
            {
                case DIRECT_LEFT :
                    FCColor::CopyPixel (&rgb, pImg->GetBits(pImg->Width()-1, y), pImg->ColorBits()/8) ;
                    nCurrX = 0 ;
                    nSpanX = 1 ;
                    break ;
                case DIRECT_RIGHT :
                    FCColor::CopyPixel (&rgb, pImg->GetBits(0, y), pImg->ColorBits()/8) ;
                    nCurrX = pImg->Width()-1 ;
                    nSpanX = -1 ;
                    break ;
            }

            // first block
            int     B=0, G=0, R=0, A=0, i=0 ;
            for (i=0 ; i < m_nStep ; i++)
            {
                BYTE   * p = pImg->GetBits (nCurrX + i*nSpanX, y) ;
                B += PCL_B(p) ;
                G += PCL_G(p) ;
                R += PCL_R(p) ;
                A += PCL_A(p) ;
            }

            // move block
            for (i=0 ; i < pImg->Width() - 2 ; i++, nCurrX+=nSpanX) // leave 2 pixel edge
            {
                int     newB = FClamp0255 (B / m_nStep), // don't set pixel current
                        newG = FClamp0255 (G / m_nStep),
                        newR = FClamp0255 (R / m_nStep),
                        newA = FClamp0255 (A / m_nStep) ;
                BYTE    * p = pImg->GetBits (nCurrX, y) ;

                // step
                if (i >= pImg->Width() - m_nStep) // edge
                {
                    B = B - PCL_B(p) + PCL_B(&rgb) ;
                    G = G - PCL_G(p) + PCL_G(&rgb) ;
                    R = R - PCL_R(p) + PCL_R(&rgb) ;
                    if (pImg->ColorBits() == 32)
                        A = A - PCL_A(p) + PCL_A(&rgb) ;
                }
                else
                {
                    BYTE   * pA = pImg->GetBits (nCurrX + nSpanX*m_nStep, y) ;
                    B = B - PCL_B(p) + PCL_B(pA) ;
                    G = G - PCL_G(p) + PCL_G(pA) ;
                    R = R - PCL_R(p) + PCL_R(pA) ;
                    if (pImg->ColorBits() == 32)
                        A = A - PCL_A(p) + PCL_A(pA) ;
                }
                PCL_B(p) = newB ;
                PCL_G(p) = newG ;
                PCL_R(p) = newR ;
                if (pImg->ColorBits() == 32)
                    PCL_A(p) = newA ;
            }
            if (pProgress)
                pProgress->SetProgress ((y+1) * 100 / pImg->Height()) ;
        }
    }

    DIRECT_SYS  m_Direct ;
    int         m_nStep ; // (>= 2)
};

//=============================================================================
#include "Blur_IIRGauss.h"

//=============================================================================
#include "SoftGlow.h"

//=============================================================================
/**
 *  Add inner bevel frame (>=24 bit).
@verbatim
    example:
        FCPixelInnerBevel   aCmd (20, 10) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelInnerBevel : public FCPixelWholeImageBase
{
public:
    FCPixelInnerBevel (int nSize, int nSmooth)
    {
        m_nSize = FMax (1, nSize) ;
        m_nSmooth = nSmooth ;
    }
private:

    // the temporary object adjust brightness
    class __FCPixelFillInnerBevel : public FCSinglePixelProcessBase
    {
    public :
        __FCPixelFillInnerBevel (FCObjImage* pImg) : m_pImgBright(pImg) {}
    private:
        virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
        {
            BYTE   * p = m_pImgBright->GetBits(x,y) ;
            PCL_B(pPixel) = FClamp0255 (PCL_B(pPixel) * PCL_A(p) / 100) ;
            PCL_G(pPixel) = FClamp0255 (PCL_G(pPixel) * PCL_A(p) / 100) ;
            PCL_R(pPixel) = FClamp0255 (PCL_R(pPixel) * PCL_A(p) / 100) ;
        }
        FCObjImage   * m_pImgBright ;
    };

    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        // image brightness
        const int    nLeft=160, nTop=160,
                     nRight=40, nBottom=40 ;
        FCObjImage   imgBright (pImg->Width(), pImg->Height(), 32) ;
        for (int y=0 ; y < imgBright.Height() ; y++)
            for (int x=0 ; x < imgBright.Width() ; x++)
            {
                BYTE   * p = imgBright.GetBits(x,y) ;
                if ((x < m_nSize) && (y < imgBright.Height()-x) && (y > x))
                    PCL_A(p) = nLeft ;
                else if ((y < m_nSize) && (x < imgBright.Width()-y) && (x > y))
                    PCL_A(p) = nTop ;
                else if ((x > imgBright.Width()-m_nSize) && (y > imgBright.Width()-x) && (y < imgBright.Height()+x-imgBright.Width()))
                    PCL_A(p) = nRight ;
                else if (y > imgBright.Height()-m_nSize)
                    PCL_A(p) = nBottom ;
                else
                    PCL_A(p) = 100 ;
            }

            FCPixelBlur_Box   cmdBlur (m_nSmooth, true) ;
            imgBright.SinglePixelProcessProc (cmdBlur) ;

            __FCPixelFillInnerBevel   cmdBevel (&imgBright) ;
            pImg->SinglePixelProcessProc (cmdBevel, pProgress) ;
    }
    int   m_nSize, m_nSmooth ;
};

//=============================================================================
/**
 *  Smooth edge (32 bit).
@verbatim
    example:
        FCPixelSmoothEdge   aCmd (15) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelSmoothEdge : public FCPixelWholeImageBase
{
public:
    FCPixelSmoothEdge (int iBlock) : m_iBlock(FMax(1,iBlock)) {}
private:
    int     m_iBlock ; // >=1
private:
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() == 32) ;
    }
    static void RecordHaloPoint (const FCObjImage& imgAlpha, std::deque<POINT>& listHalo)
    {
        // expand edge to search easily
        FCObjImage   imgSearch (imgAlpha.Width()+2, imgAlpha.Height()+2, imgAlpha.ColorBits()) ;
        imgSearch.CoverBlock (imgAlpha, 1, 1) ;

        // record halo point
        FCObjImage   imgRecord (imgSearch.Width(), imgSearch.Height(), 8) ;
        POINT        nDirect[4] = {{0,-1}, {0,1}, {-1,0}, {1,0}} ; // up-down-left-right
        for (int y=1 ; y < imgSearch.Height()-1 ; y++)
            for (int x=1 ; x < imgSearch.Width()-1 ; x++)
            {
                BYTE   * p = imgSearch.GetBits(x,y) ;
                for (int i=0 ; i < 4 ; i++)
                {
                    // direction
                    int    nDX = x + nDirect[i].x,
                           nDY = y + nDirect[i].y ;
                    BYTE   * pTmp = imgSearch.GetBits (nDX, nDY) ;
                    if (*pTmp == *p)
                        continue ;

                    // draw halo at smaller alpha
                    POINT   pt ;
                    if (*pTmp < *p)
                    {
                        pt.x=nDX ; pt.y=nDY ;
                    }
                    else
                    {
                        pt.x=x ; pt.y=y ;
                    }
                    *imgRecord.GetBits (pt.x, pt.y) = 0xFF ;
                }
            }

            // halo point
            listHalo.clear() ;
            {
                for (int y=0 ; y < imgRecord.Height() ; y++)
                    for (int x=0 ; x < imgRecord.Width() ; x++)
                        if (*imgRecord.GetBits(x,y) == 0xFF)
                        {
                            POINT   pt = {x-1, y-1} ; // remember -1
                            listHalo.push_back (pt) ;
                        }
            }
    }
    void DrawHalo (FCObjImage& imgDest, FCObjImage& imgAlpha, FCObjImage& imgHalo, POINT ptCenter)
    {
        int    nLT = (imgHalo.Width()-1) / 2 ;
        RECT   rcHalo = {ptCenter.x-nLT, ptCenter.y-nLT, ptCenter.x+nLT+1, ptCenter.y+nLT+1},
               rcAlpha = {0,0,imgAlpha.Width(),imgAlpha.Height()}, rcDest ;
        if (::IntersectRect (&rcDest, &rcHalo, &rcAlpha) == 0)
            return ;

        int    nCenter = 0 ;
        if (imgAlpha.IsInside (ptCenter.x, ptCenter.y))
            nCenter = *imgAlpha.GetBits (ptCenter.x, ptCenter.y) ;

        for (int y=rcDest.top ; y < rcDest.bottom ; y++)
            for (int x=rcDest.left ; x < rcDest.right ; x++)
            {
                BYTE   * pDest = imgDest.GetBits (x, y),
                       * pAlpha = imgAlpha.GetBits (x, y),
                       * pHalo = imgHalo.GetBits (x-rcHalo.left, y-rcHalo.top) ;
                // calculate percentage
                int    nNew = FMax (nCenter, *pAlpha * *pHalo / 0xFF) ;
                if (nNew < *pDest)
                    *pDest = nNew ;
            }
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        // get alpha channel
        FCObjImage   imgAlpha ;
        pImg->GetAlphaChannel (&imgAlpha) ;

        // get draw halo point
        std::deque<POINT>   listHalo ;
        RecordHaloPoint (imgAlpha, listHalo) ;
        if (listHalo.empty())
            return ;

        // create halo image
        FCObjImage          imgHalo ;
        FCPixelCreateHalo   aCmdCreateHalo(m_iBlock, 0, 0xFF) ;
        imgHalo.SinglePixelProcessProc (aCmdCreateHalo) ;

        // make alpha image
        FCObjImage   imgDest = imgAlpha ;
        for (size_t i=0 ; i < listHalo.size() ; i++)
        {
            DrawHalo (imgDest, imgAlpha, imgHalo, listHalo[i]) ;
            if (pProgress)
                pProgress->SetProgress ((int)(100*i/listHalo.size())) ;
        }
        pImg->AppendAlphaChannel (imgDest) ;
    }
private:
    class FCPixelCreateHalo : public FCSinglePixelProcessBase
    {
    public:
        FCPixelCreateHalo (int nRadius, int crCenter, int crEdge) : m_crCenter(crCenter), m_crEdge(crEdge)
        {
            m_fRadius = FMax (1, nRadius) ;
            m_fRadius += 1.0 ;
            m_ptCenter.x = (int)m_fRadius ; m_ptCenter.y = (int)m_fRadius ;
        }
    private:
        virtual bool ValidateColorBits (const FCObjImage* pImg) {return true;}
        virtual void OnEnterProcess (FCObjImage* pImg)
        {
            int     nWidth = (int)(2*m_fRadius + 1) ;
            pImg->Create (nWidth, nWidth, 8) ;
        }
        virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
        {
            double   fDist = FHypot(x-m_ptCenter.x, y-m_ptCenter.y) ;
            if (fDist <= m_fRadius)
                *pPixel = FRound (m_crCenter + (m_crEdge-m_crCenter)*fDist/m_fRadius) ;
            else
                *pPixel = m_crEdge ;
        }
        double   m_fRadius ;
        int      m_crCenter, m_crEdge ;
        POINT    m_ptCenter ;
    };
};

//=============================================================================
/// Calculate optimized image's rect.
class FCPixelGetOptimizedRect : public FCSinglePixelProcessBase
{
public:
    FCPixelGetOptimizedRect()
    {
        memset (&m_rcOptimized, 0, sizeof(m_rcOptimized)) ;
        m_bFirst = true ;
    }
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() == 32) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        if (PCL_A(pPixel))
            if (m_bFirst)
            {
                m_rcOptimized.left = x ;
                m_rcOptimized.right = x+1 ;
                m_rcOptimized.top = y ;
                m_rcOptimized.bottom = y+1 ;
                m_bFirst = false ;
            }
            else
            {
                if (x < m_rcOptimized.left)    m_rcOptimized.left = x ;
                if (x+1 > m_rcOptimized.right)   m_rcOptimized.right = x+1 ;
                if (y < m_rcOptimized.top)     m_rcOptimized.top = y ;
                if (y+1 > m_rcOptimized.bottom)  m_rcOptimized.bottom = y+1 ;
            }
    }
public:
    RECT     m_rcOptimized ;
    bool     m_bFirst ;
};

//=============================================================================
/**
 *  Add shadow (32 bit).
@verbatim
    example:
		SHADOWDATA    ShData ;
        ShData.crShadow = FCColor::crWhite() ;
        ShData.nAlpha = 75 ;
        ShData.nSmooth = 10 ;
        ShData.nOffsetX = 5 ;
        ShData.nOffsetY = 5 ;
        FCPixelAddShadow   aCmd (ShData) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelAddShadow : public FCPixelWholeImageBase
{
public:
    FCPixelAddShadow (SHADOWDATA ShData)
    {
        m_ShadowData = ShData ;
        m_ShadowData.nSmooth = FMax (2, (int)m_ShadowData.nSmooth) ;
        m_ShadowData.nAlpha = FClamp ((int)m_ShadowData.nAlpha, 1, 100) ;
    }
private:
    virtual bool ValidateColorBits (const FCObjImage* pImg)
    {
        return pImg->IsValidImage() && (pImg->ColorBits() == 32) ;
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        // backup image
        const FCObjImage   imgOld(*pImg) ;

        // calculate new image size
        RECT    rcImg = {0, 0, pImg->Width(), pImg->Height()},
                rcShadowOffset = rcImg ;
        ::OffsetRect (&rcShadowOffset, m_ShadowData.nOffsetX, m_ShadowData.nOffsetY) ;
        RECT     rcShadow = rcShadowOffset ;
        ::InflateRect (&rcShadow, m_ShadowData.nSmooth, m_ShadowData.nSmooth) ;
        RECT     rcResult ;
        ::UnionRect (&rcResult, &rcImg, &rcShadow) ;

        // create shadow background and box-blur it
        pImg->Create (RECTWIDTH(rcResult), RECTHEIGHT(rcResult), 32) ;
        int     nStartX = rcShadowOffset.left - rcResult.left,
                nStartY = rcShadowOffset.top - rcResult.top ;
        for (int y=0 ; y < imgOld.Height() ; y++)
            for (int x=0 ; x < imgOld.Width() ; x++)
            {
                RGBQUAD   cr = m_ShadowData.crShadow ;
                PCL_A(&cr) = PCL_A(imgOld.GetBits(x,y)) * m_ShadowData.nAlpha / 100 ;
                *(RGBQUAD*)pImg->GetBits (nStartX + x, nStartY + y) = cr ;
            }

        // box-blur alpha-channel
        FCPixelBlur_Box   cmdSmooth (m_ShadowData.nSmooth, false) ;
        pImg->SinglePixelProcessProc (cmdSmooth, pProgress) ;

        // combine origin image
        pImg->CombineImage (imgOld, rcImg.left-rcResult.left, rcImg.top-rcResult.top) ;

        // adjust new img's position
        pImg->SetGraphObjPos (imgOld.GetGraphObjPos().x - rcImg.left + rcResult.left,
                              imgOld.GetGraphObjPos().y - rcImg.top + rcResult.top) ;
    }
private:
    SHADOWDATA   m_ShadowData ;
};

//=============================================================================
/**
 *  Frame image with pattern (>=24 bit).
@verbatim
    example:
        FCObjImage   img ("border.jpg") ;
        FCPixelPatternFrame   aCmd (img) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelPatternFrame : public FCPixelWholeImageBase
{
public:
    /**
     *  Constructor.
     */
    FCPixelPatternFrame (FCObjImage imgTop)
    {
        imgTop.ConvertTo32Bit() ;
        imgTop.SetAlphaChannelValue(0xFF) ;

        m_imgLeft = m_imgTop = imgTop ;
        FCPixelRotate270   aCmd ;
        m_imgLeft.SinglePixelProcessProc (aCmd) ;
    }
private:
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        if (!m_imgTop.IsValidImage())
            return ;

        int   nBorder = m_imgTop.Height(),
              w = pImg->Width(),
              h = pImg->Height(),
              nSpan = pImg->ColorBits()/8, t ;

        if ((pImg->Width() < 2*nBorder) || (pImg->Height() < 2*nBorder))
            return ;

        for (int i=0 ; i < nBorder ; i++, w-=2, h-=2)
        {
            // top & bottom
            for (t=0 ; t < w ; t++)
            {
                int     x = i + t ;
                BYTE    * pS = m_imgTop.GetBits (x % m_imgTop.Width(), i) ;
                FCColor::CopyPixel (pImg->GetBits(x,i), pS, nSpan) ;
                FCColor::CopyPixel (pImg->GetBits(x,pImg->Height()-1-i), pS, nSpan) ;
            }
            // left & right
            for (t=0 ; t < h ; t++)
            {
                int     y = i + t ;
                BYTE    * pS = m_imgLeft.GetBits (i, y % m_imgLeft.Height()) ;
                FCColor::CopyPixel (pImg->GetBits(i,y), pS, nSpan) ;
                FCColor::CopyPixel (pImg->GetBits(pImg->Width()-1-i,y), pS, nSpan) ;
            }
        }
    }

    FCObjImage   m_imgTop ;
    FCObjImage   m_imgLeft ;
};

//=============================================================================
/**
 *  Fill a gradient frame (>=24 bit).
@verbatim
    example:
        FCPixelFillGradientFrame   aCmd (FCColor::crBlack(), FCColor::crWhite(), 5) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelFillGradientFrame : public FCPixelPatternFrame
{
public:
    /**
     *  Constructor.
     *  @param nWidth : pixel width of border.
     */
    FCPixelFillGradientFrame (RGBQUAD crOut, RGBQUAD crIn, int nWidth) : FCPixelPatternFrame (MakeGradientBorder(crOut,crIn,nWidth)) {}
private:
    static FCObjImage MakeGradientBorder (RGBQUAD crOut, RGBQUAD crIn, int nWidth)
    {
        FCObjImage   img (1, nWidth, 32) ;
        const POINT  ptStart = {0,0},
                     ptEnd = {0,img.Height()-1} ;
        FCPixelGradientLine   aCmd (ptStart, ptEnd, crOut, crIn) ;
        img.SinglePixelProcessProc (aCmd) ;
        return img ;
    }
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
