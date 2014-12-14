/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-12-23
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_HISTOGRAM__2003_12_23__H__
#define __FOO_HISTOGRAM__2003_12_23__H__
#include "ObjImage.h"

//=============================================================================
/**
 *  Calculate histogram of image.
 */
class FCHistogram
{
public:
    /// Calculate img's histogram, img's bpp >= 24.
    FCHistogram (const FCObjImage& img) ;

    /// Analyze image (only 24bpp or 32bpp image can be analyzed).
    void AnalyzeImage (const FCObjImage& img) ;

    /**
     *  @name Get/Set histogram Stat. range.
     */
    //@{
    /// Get start position.
    int GetStart() const {return m_nStart;}
    /// Get end position.
    int GetEnd() const {return m_nEnd;}
    /// Set start position.
    void SetStart (int nStart) {m_nStart = FClamp0255(nStart);}
    /// Set end position.
    void SetEnd (int nEnd) {m_nEnd = FClamp0255(nEnd);}
    //@}

    /**
     *  @name Query attributes.
     *  nChannel can be CHANNEL_GRAY, CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE
     */
    //@{
    /// Get pixel count of Stat. image.
    int GetPixelNumber() const {return m_nPixelNum;}
    /// Pixel count of selected region (nChannel).
    int GetCount (IMAGE_CHANNEL nChannel = CHANNEL_GRAY) const ;
    /// Pixel count of nValue position (nChannel).
    int GetValueCount (int nValue, IMAGE_CHANNEL nChannel = CHANNEL_GRAY) const ;
    /// Pixel average of selected region (nChannel).
    int GetAverage (IMAGE_CHANNEL nChannel = CHANNEL_GRAY) const ;
    /// Get max count in selected region (nChannel).
    int GetMaxCount (IMAGE_CHANNEL nChannel = CHANNEL_GRAY) const ;
    /// Received img is 24bpp, size is (256,nHeight)
    void GetHistogramImage (int nHeight, FCObjImage* img, IMAGE_CHANNEL nChannel = CHANNEL_GRAY) const ;
    //@}

private:
    const PCL_array<int>& GetChannelHistogram (IMAGE_CHANNEL nChannel) const
    {
        switch (nChannel)
        {
            case CHANNEL_RED :   return m_HisRed ;
            case CHANNEL_GREEN : return m_HisGreen ;
            case CHANNEL_BLUE :  return m_HisBlue ;
        }
        return m_HisGray ;
    }

private:
    PCL_array<int>   m_HisGray, m_HisRed, m_HisGreen, m_HisBlue ;
    int     m_nStart ;
    int     m_nEnd ;
    int     m_nPixelNum ;
};

//=============================================================================
// inline Implement
//=============================================================================
inline FCHistogram::FCHistogram (const FCObjImage& img) : m_HisGray(256), m_HisRed(256), m_HisGreen(256), m_HisBlue(256)
{
    m_nStart = 0 ;
    m_nEnd = 255 ;
    m_nPixelNum = 0 ;
    AnalyzeImage (img) ;
}
//-----------------------------------------------------------------------------
inline void FCHistogram::AnalyzeImage (const FCObjImage& img)
{
    if (!img.IsValidImage() || (img.ColorBits() < 24))
        {assert(false); return;}

    for (int i=0 ; i < 256 ; i++)
    {
        m_HisGray[i] = m_HisRed[i] = m_HisGreen[i] = m_HisBlue[i] = 0 ;
    }

    m_nPixelNum = img.Width()*img.Height() ;
    for (int y=0 ; y < img.Height() ; y++)
        for (int x=0 ; x <img.Width() ; x++)
        {
            BYTE   * p = img.GetBits(x,y) ;
            m_HisBlue[PCL_B(p)]++ ;
            m_HisGreen[PCL_G(p)]++ ;
            m_HisRed[PCL_R(p)]++ ;
            m_HisGray[FCColor::GetGrayscale(p)]++ ;
        }
}
//-----------------------------------------------------------------------------
inline int FCHistogram::GetCount (IMAGE_CHANNEL nChannel) const
{
    int     nCount = 0 ;
    for (int i=m_nStart ; i <= m_nEnd ; i++)
        nCount += GetChannelHistogram(nChannel)[i] ;
    return nCount ;
}
//-----------------------------------------------------------------------------
inline int FCHistogram::GetValueCount (int nValue, IMAGE_CHANNEL nChannel) const
{
    return GetChannelHistogram(nChannel)[FClamp0255(nValue)] ;
}
//-----------------------------------------------------------------------------
inline int FCHistogram::GetAverage (IMAGE_CHANNEL nChannel) const
{
    double   fAverage = 0 ;
    int      nCount = 0 ;
    for (int i=m_nStart ; i <= m_nEnd ; i++)
    {
        int   n = GetChannelHistogram(nChannel)[i] ;
        nCount += n ;
        fAverage = fAverage + n * i ;
    }
    return nCount ? (int)(fAverage/nCount) : 0 ;
}
//-----------------------------------------------------------------------------
inline int FCHistogram::GetMaxCount (IMAGE_CHANNEL nChannel) const
{
    int     nMaxCount = 0 ;
    for (int i=m_nStart ; i <= m_nEnd ; i++)
    {
        int   n = GetChannelHistogram(nChannel)[i] ;
        if (n > nMaxCount)
            nMaxCount = n ;
    }
    return nMaxCount ;
}
//-----------------------------------------------------------------------------
inline void FCHistogram::GetHistogramImage (int nHeight, FCObjImage* img, IMAGE_CHANNEL nChannel) const
{
    if (!img || !img->Create (256, nHeight, 24))
        return ;

    // fill white back
    memset (img->GetMemStart(), 0xFF, img->GetPitch() * img->Height()) ;

    int     nMaxCount = GetMaxCount(nChannel) ;
    if (nMaxCount == 0)
        return ;

    for (int x=0 ; x < img->Width() ; x++)
    {
        int   nFill = FClamp (GetChannelHistogram(nChannel)[x]*img->Height()/nMaxCount, 0, img->Height()) ;
        for (int i=0 ; i < nFill ; i++)
        {
            BYTE   * p = img->GetBits(x, img->Height() - 1 - i) ;
            PCL_R(p) = PCL_G(p) = PCL_B(p) = 0 ;
        }
    }
}

#endif
