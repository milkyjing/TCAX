/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2001-4-27
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_OBJECT_IMAGE__2001_04_27__H__
#define __FOO_OBJECT_IMAGE__2001_04_27__H__
#include "ObjBase.h"
#include "FColor.h"
#include "ObjProgress.h"
#include "oxo_helper.h"
#include "imagefile/Interface_ImageHandle.h"
#include "imagefile/Interface_ImageHandleFactory.h"
#include "pixelprocessor/Interface_PixelProcessor.h"

//=============================================================================
/**
 *  Basic image object.
@verbatim
  1) the origin (0,0) of image lies at left-top point.
  2) all coordinate in this class (POINT, RECT...) relative to image's (0,0).
  3) after image be created, pixel's data are automatically initialized to 0. and if
     image's bpp <= 8, a gray palette will be set automatically.
  4) if you want to use Save / Load, you must call SetImageHandleFactory to set a handle factory.
@endverbatim
 */
class FCObjImage : public FCObjGraph
{
public:
    // mask for 16bpp image
    enum
    {
        MASK16_RED_565 = 0xF800,
        MASK16_GREEN_565 = 0x07E0,
        MASK16_BLUE_565 = 0x001F,
        MASK16_RED_555 = 0x7C00,
        MASK16_GREEN_555 = 0x03E0,
        MASK16_BLUE_555 = 0x001F,
    };

    /**
     *  @name Constructor.
     */
    //@{
    /// Create an empty image object.
    FCObjImage () ;
    /// Copy constructor.
    FCObjImage (const FCObjImage& img) ;
    /// Create an image.
    FCObjImage (int nWidth, int nHeight, int nColorBit) ;
    /// Create an image from a image file.
    FCObjImage (const char* szFileName) ;
    FCObjImage& operator= (const FCObjImage& img) ;
    virtual ~FCObjImage() {Destroy();}
    //@}

    /**
     *  Serialize image object in memory (bpp > 8).
     *  @see FCObject::Serialize
     */
    virtual int Serialize (bool bSave, BYTE* pSave) ;

    /**
     *  @name Create/Destroy image.
     */
    //@{
    /**
     *  Create image.
@verbatim
hint:
  1) if image's bpp <= 8, a gray palette will be set automatically.
  2) after image be created, all pixels are automatically initialized to 0.
@endverbatim
    */
    bool Create (const BITMAPINFOHEADER* pBmif) ;
    ///  Create image.
    bool Create (int nWidth, int nHeight, int nColorBit) ;
    /// Destroy image object.
    void Destroy() ;
    //@}

    /**
     *  @name Basic attributes.
     */
    //@{
    /// Is current image object is valid.
    bool IsValidImage() const {return (m_pByte != 0);}
    /// this function don't do boundary check, <B>so crash if iLine exceed</B>.
    BYTE* GetBits (int iLine) const ;
    /// this function don't do boundary check, <B>so crash if y exceed</B>.
    BYTE* GetBits (int x, int y) const ;
    /// Get image's pixel start-address (address of left-bottom point).
    BYTE* GetMemStart() const {return m_pByte;}
    /// Pixel width of image.
    int Width() const {return m_DibInfo.biWidth;}
    /// Pixel height of image.
    int Height() const {return m_DibInfo.biHeight;}
    /// Image's bpp (bit per pixel), available : 1,4,8,16,24,32.
    int ColorBits() const {return m_DibInfo.biBitCount;}
    /// Bytes pitch between two lines (the value is 4-bytes rounded).
    int GetPitch() const {return 4 * ((Width() * ColorBits() + 31) / 32);}
    /**
     *  Create BITMAPINFOHEADER struct.
     *  palette(<=8bit) or 3-bit-fields(16bit) was appended. <BR><BR>
     *  you must use <B>delete[]</B> to delete returned point, high recommended to use PCL_Array wrap returned pointer.
@code
    PCL_array<BITMAPINFOHEADER>   imgInfo (img.NewImgInfoWithPalette()) ;
    imgInfo.get()->biWidth ;
    pPalette = imgInfo.get() + 1 ;
@endcode
     */
    BITMAPINFOHEADER* NewImgInfoWithPalette() const ;
    /// bound rc into image.
    /// @param rc : coordinate in image.
    void BoundRect (RECT& rc) const ;
    //@}

    /**
     *  @name Pixel access.
     */
    //@{
    /// Is point (x,y) in image.
    bool IsInside (int x, int y) const {return (x>=0) && (x<Width()) && (y>=0) && (y<Height());}
    /// Get pixel data at (x,y).
    DWORD GetPixelData (int x, int y) const ;
    /// Set pixel data at (x,y).
    void SetPixelData (int x, int y, DWORD dwPixel) ;
    //@}

    /**
     *  @name Palette operations (bpp : 1,4,8)
     */
    //@{
    /// Get palette of image.
    /// the iFirstIndex is A <B>zero-based</B> color table index.
    bool GetColorTable (int iFirstIndex, int iNumber, RGBQUAD* pColors) const ;
    /// Set palette of image.
    /// the iFirstIndex is A <B>zero-based</B> color table index.
    bool SetColorTable (int iFirstIndex, int iNumber, const RGBQUAD* pColors) ;
    /// Duplicate palette from imgSrc.
    void CopyPalette (const FCObjImage& imgSrc) ;
    //@}

    /**
     *  @name Color convert.
     */
    //@{
    /// Convert current image's bpp to 24.
    void ConvertTo24Bit() {__ConvertToTrueColor(24);}
    /// Convert current image's bpp to 32 (alpha channel will set 0xFF).
    void ConvertTo32Bit() {__ConvertToTrueColor(32);}
    //@}

    /**
     *  @name Channel operations (bpp == 32)
     */
    //@{
    /// Get alpha channel of 32bpp image (imgAlpha's bpp == 8).
    void GetAlphaChannel (FCObjImage* imgAlpha) const ;
    /// Set alpha channel of 32bpp image (alpha8's bpp == 8).
    void AppendAlphaChannel (const FCObjImage& alpha8) ;
    /// Set 32bpp image's alpha channel value.
    void SetAlphaChannelValue (int nValue) ;
    //@}

    /**
     *  @name Basic transform.
     */
    //@{

    /**
     *  Get block of image (bpp >= 8).
     *  if rcBlock exceed image, it will be bounded in image. and SubImg's position is on image.
     *  @param rcBlock : rect on image.
     */
    bool GetSubBlock (FCObjImage* SubImg, RECT rcBlock) const ;

    /**
     *  Cover img on position (x,y) (bpp >= 8).
     */
    bool CoverBlock (const FCObjImage& img, int x, int y) ;
    void TileBlock (const FCObjImage& img, int x, int y) ;

    /**
     *  Alpha blend image (bpp >= 24).
     *  @param rcSrc is rect of img32 must inside.
     *  @param rcDest is dest rect can be intersect.
     */
    void AlphaBlend (const FCObjImage& img32, RECT rcDest, RECT rcSrc, int nAlphaPercent) ;

    /**
     *  Combine image (bpp == 32).
     */
    void CombineImage (const FCObjImage& img32, int x=0, int y=0, int nAlphaPercent=100) ;

    // inner, for selection process.
	void LogicalBlend (const FCObjImage& img, LOGICAL_OP LogOP, int x=0, int y=0) ;

    /**
     *  Expand image (bpp >= 8).
     *  @param bCopyEdge : duplicate edge pixels if true.
     *  @param iLeft/iTop/iRight/iBottom must >= 0
     */
    void ExpandFrame (bool bCopyEdge, int iLeft, int iTop, int iRight, int iBottom) ;

    /// bpp >= 8.
    void	Stretch (int nNewWidth, int nNewHeight) ; // stretch
    /// bpp >= 24.
    void	Stretch_Smooth (int nNewWidth, int nNewHeight, FCObjProgress * progress=0) ; // stretch
    /// Perform a processor. more detail refer PixelProcessorBase.h
    void    SinglePixelProcessProc (FCInterface_PixelProcess& rProcessor, FCObjProgress* pProgress=0) ;
    //@}

    /**
     *  @name Read/Write image file.
     */
    //@{
    /**
     *  Set image handle factory.
     *  You must use <B>new</B> to create object and after the object be setted, you can't delete it later.<BR><BR>
     *  it's a global setting, you can set at startup of program.
@code
    FCObjImage::SetImageHandleFactory (new FCImageHandleFactory_FreeImage) ;
@endcode
     */
    static void SetImageHandleFactory (FCImageHandleFactory* pFactory) {__ManageImageHandleFactory(false,pFactory);}
    /// Get image handle factory.
    static FCImageHandleFactory* GetImageHandleFactory() {return __ManageImageHandleFactory(true,0);}
    /**
     *  Load image file.
     *  this function determine image format by file's ext name.
     *  @param pProperty : optional, you can pass NULL if you don't care.
     */
    bool Load (const char* szFileName, FCImageProperty* pProperty=0) ;
    /**
     *  Load image from memory.
     *  @param pProperty : optional, you can pass NULL if you don't care.
     */
    bool Load (BYTE* pStart, int nMemSize, IMAGE_TYPE imgType, FCImageProperty* pProperty=0) ;
    /// Load from DIB format memory.
    bool LoadDIBStream (const void* pDIB, int nBufferSize) ;
    /**
     *  Save image to file.
     *  this function determine image format by file's ext name.
     *  @param nFlag : depends on the image format. <BR>
@verbatim
     JPG : compress quality [1..100], default(82)
     GIF : transparent color's index in palette
@endverbatim
     */
    bool Save (const char* szFileName, int nFlag = -1) const
    {
        // save property
        FCImageProperty   imgProp ;
        imgProp.SetPropertyValue (IMAGE_TAG_SAVE_FLAG, FCOXOHelper::X2A(nFlag).c_str()) ;
        return Save (szFileName, imgProp) ;
    }
    /// Save image to file.
    bool Save (const char* szFileName, const FCImageProperty& rProp) const ;
    //@}

private:
    BITMAPINFOHEADER   m_DibInfo ; // DIB Info
    BYTE             * m_pByte ;   // Bitmap start bits, from left-bottom start
    BYTE            ** m_ppLine ;  // Line-pointer, ppLine[] ; from top to bottom
    DWORD              m_dwBitFields[3] ; // only 16bit image, order R,G,B
    RGBQUAD          * m_pPalette ; // palette

private:
    void __SetGrayPalette() ;
    void __InitClassMember() ; // initialize the member variant
    void __ConvertToTrueColor (int iColor) ; // iColor == 24 or 32
    static RGBQUAD __Split16Bit_565 (WORD wPixel) ;
    static RGBQUAD __Split16Bit_555 (WORD wPixel) ;
    static void __FillImageRect (const FCObjImage& img, const RECT& rcBlock, const void* pSrc) ;
    static FCImageHandleFactory* __ManageImageHandleFactory (bool bGet, FCImageHandleFactory* pFactory) ;
} ;

//=============================================================================
// inline Implement
//=============================================================================
inline void FCObjImage::__InitClassMember()
{
    memset (&m_DibInfo, 0, sizeof(m_DibInfo)) ;
    m_dwBitFields[0]=m_dwBitFields[1]=m_dwBitFields[2]=0 ;
    m_pByte=0 ; m_ppLine=0 ; m_pPalette=0 ;
}
//-----------------------------------------------------------------------------
inline FCObjImage::FCObjImage()
{
    __InitClassMember() ;
}
inline FCObjImage::FCObjImage (const FCObjImage& img)
{
    __InitClassMember() ;
    *this = img ;
}
inline FCObjImage::FCObjImage (int nWidth, int nHeight, int nColorBit)
{
    __InitClassMember() ;
    Create (nWidth, nHeight, nColorBit) ;
}
inline FCObjImage::FCObjImage (const char* szFileName)
{
    __InitClassMember() ;
    Load (szFileName) ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::Create (const BITMAPINFOHEADER* pBmif)
{
    // unsupported store format
    if (!pBmif || (pBmif->biHeight <= 0) || (pBmif->biWidth <= 0))
        {assert(false); return false;}
    if (!((pBmif->biCompression == BI_RGB) || (pBmif->biCompression == BI_BITFIELDS)))
        {assert(false); return false;}
    switch (pBmif->biBitCount) // validate bpp
    {
        case 1 :
        case 4 :
        case 8 :
        case 16 :
        case 24 :
        case 32 : break ;
        default : assert(false); return false;
    }

    if (IsValidImage())
        Destroy() ;

    // init struct
    memset (&m_DibInfo, 0, sizeof(m_DibInfo)) ;
    m_DibInfo.biSize          = sizeof(BITMAPINFOHEADER) ;
    m_DibInfo.biWidth         = pBmif->biWidth ;
    m_DibInfo.biHeight        = pBmif->biHeight ;
    m_DibInfo.biPlanes        = 1 ;
    m_DibInfo.biBitCount      = pBmif->biBitCount ;
    m_DibInfo.biCompression   = pBmif->biCompression ;
    m_DibInfo.biXPelsPerMeter = pBmif->biXPelsPerMeter ;
    m_DibInfo.biYPelsPerMeter = pBmif->biYPelsPerMeter ;

    // now flag BI_BITFIELDS is only valid in 16bit image
    if (pBmif->biBitCount == 16)
    {
        m_dwBitFields[0] = MASK16_RED_555 ; // 16-bit default format : 5-5-5
        m_dwBitFields[1] = MASK16_GREEN_555 ;
        m_dwBitFields[2] = MASK16_BLUE_555 ;
        if (pBmif->biCompression == BI_BITFIELDS) // custom
            memcpy (m_dwBitFields, pBmif + 1, 12) ;
    }
    else
    {
        // i think it's unnecessary to use mask in 32bit image
        m_DibInfo.biCompression = BI_RGB ;
    }

    // create pixel buffer, pixel must must must initialized to zero !!!
    m_pByte = FCOXOHelper::ZeroMalloc (GetPitch()*Height()) ;
    assert (((int)m_pByte % 4) == 0) ; // DWORD align

    // create a line pointer, to accelerate pixel access
    m_ppLine = (BYTE **) new BYTE [sizeof(BYTE*) * Height()] ;
    const int     nPitch = GetPitch() ;
    m_ppLine[0] = m_pByte + (Height() - 1) * nPitch ;
    for (int y = 1 ; y < Height() ; y++)
        m_ppLine[y] = m_ppLine[y - 1] - nPitch ;

    // 8bit color image default set a gray palette
    if (ColorBits() <= 8)
    {
        m_pPalette = new RGBQUAD[1 << ColorBits()] ;
        __SetGrayPalette() ;
    }
    return true ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::Create (int nWidth, int nHeight, int nColorBit)
{
    BITMAPINFOHEADER     bmih ;
    memset (&bmih, 0, sizeof(bmih)) ;
    bmih.biWidth = nWidth ;
    bmih.biHeight = nHeight ;
    bmih.biBitCount = nColorBit ;
    bmih.biCompression = BI_RGB ;
    return this->Create (&bmih) ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::Destroy()
{
    if (m_ppLine)
        delete[] m_ppLine ;

    FCOXOHelper::ZeroFree (m_pByte) ;

    if (m_pPalette)
        delete[] m_pPalette ;

    __InitClassMember() ;
}
//-----------------------------------------------------------------------------
inline BYTE* FCObjImage::GetBits (int iLine) const
{
    assert (IsInside(0,iLine)) ;
    return m_ppLine[iLine] ;
}
//-----------------------------------------------------------------------------
inline BYTE* FCObjImage::GetBits (int x, int y) const
{
    assert (IsInside(x,y)) ;
    if (ColorBits() == 32)
        return (m_ppLine[y] + x * 4) ;
    if (ColorBits() == 8)
        return (m_ppLine[y] + x) ;
    return (m_ppLine[y] + x * ColorBits() / 8) ;
}
//-----------------------------------------------------------------------------
inline BITMAPINFOHEADER* FCObjImage::NewImgInfoWithPalette() const
{
    // prepare info
    const int          nColorNum = 1 << ColorBits(),
                       nPalBytes = ((ColorBits() <= 8) ? (4*nColorNum) : 0) ;
    BITMAPINFOHEADER   * pBmfh = (BITMAPINFOHEADER*) new BYTE[16 + sizeof(BITMAPINFOHEADER) + nPalBytes] ;
    *pBmfh = m_DibInfo ;

    // append palette(<=8bit) or bit-fields(16bit)
    if (ColorBits() <= 8)
        GetColorTable (0, nColorNum, (RGBQUAD*)(pBmfh + 1)) ;
    else
        memcpy (pBmfh + 1, m_dwBitFields, 12) ;
    return pBmfh ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::BoundRect (RECT& rc) const
{
    RECT   rcImg = {0, 0, Width(), Height()} ;
    ::IntersectRect (&rc, &rcImg, &rc) ;
}
//-----------------------------------------------------------------------------
inline DWORD FCObjImage::GetPixelData (int x, int y) const
{
    if (!IsInside(x, y))
        {assert(false); return 0;}

    const BYTE   * pPixel = GetBits (x,y) ;
    switch (ColorBits())
    {
        case  1 : return 0x01 & (*pPixel >> (7 - (x & 7))) ;
        case  4 : return 0x0F & (*pPixel >> (x & 1 ? 0 : 4)) ;
        case  8 : return *(BYTE*)pPixel ;
        case 16 : return *(WORD*)pPixel ;
        case 24 :
            {
                DWORD     dwrgb = 0 ;
                FCColor::CopyPixel (&dwrgb, pPixel, 3) ;
                return dwrgb ;
            }
        case 32 : return *(DWORD*)pPixel ;
        default : assert(false) ;
    }
    return 0 ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::SetPixelData (int x, int y, DWORD dwPixel)
{
    if (!IsInside(x, y))
        {assert(false); return;}

    BYTE   * pPixel = GetBits (x,y) ;
    switch (ColorBits())
    {
        case  1 : *pPixel &= ~(1     << (7 - (x & 7))) ;
                  *pPixel |= dwPixel << (7 - (x & 7)) ;
                  break ;
        case  4 : *pPixel &= 0x0F    << (x & 1 ? 4 : 0) ;
                  *pPixel |= dwPixel << (x & 1 ? 0 : 4) ;
                  break ;
        case  8 :
        case 16 :
        case 24 :
        case 32 : FCColor::CopyPixel (pPixel, &dwPixel, ColorBits() / 8) ;
            break ;
        default : assert(false) ;
    }
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::GetColorTable (int iFirstIndex, int iNumber, RGBQUAD* pColors) const
{
    if (!IsValidImage() || (ColorBits() > 8) || (iFirstIndex < 0) || !pColors || !m_pPalette)
        {assert(false); return false;}

    const int   nColorNum = 1 << ColorBits() ;
    for (int i=0 ; i < iNumber ; i++)
    {
        int     nIndex = iFirstIndex + i ;
        if (nIndex < nColorNum)
            pColors[i] = m_pPalette[nIndex] ;
    }
    return true ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::SetColorTable (int iFirstIndex, int iNumber, const RGBQUAD* pColors)
{
    if (!IsValidImage() || (ColorBits() > 8) || (iFirstIndex < 0) || !pColors || !m_pPalette)
        {assert(false); return false;}

    const int   nColorNum = 1 << ColorBits() ;
    for (int i=0 ; i < iNumber ; i++)
    {
        int     nIndex = iFirstIndex + i ;
        if (nIndex < nColorNum)
            m_pPalette[nIndex] = pColors[i] ;
    }
    return true ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::CopyPalette (const FCObjImage& imgSrc)
{
    if (!IsValidImage() || (ColorBits() > 8) || (ColorBits() != imgSrc.ColorBits()))
        {assert(false); return;}

    RGBQUAD   pPal[256] ;
    int       nNum = 1 << imgSrc.ColorBits() ;
    imgSrc.GetColorTable (0, nNum, pPal) ;
    SetColorTable (0, nNum, pPal) ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::__SetGrayPalette()
{
    if (!IsValidImage() || (ColorBits() > 8))
        {assert(false); return;}

    // calculate palette
    RGBQUAD     pPal[256] ;
    const int   nNum = 1 << ColorBits(),
                nSpan = 255 / (nNum - 1) ;
    for (int i=0 ; i < nNum ; i++)
    {
        PCL_R(&pPal[i]) = PCL_G(&pPal[i]) = PCL_B(&pPal[i]) = i * nSpan ;
    }
    SetColorTable (0, nNum, pPal) ;
}
//-----------------------------------------------------------------------------
inline RGBQUAD FCObjImage::__Split16Bit_565 (WORD wPixel)
{
    RGBQUAD     rgb ;
    PCL_R(&rgb) = (MASK16_RED_565 & wPixel) >> 8 ;
    PCL_G(&rgb) = (MASK16_GREEN_565 & wPixel) >> 3 ;
    PCL_B(&rgb) = (MASK16_BLUE_565 & wPixel) << 3 ;
    return rgb ;
}
inline RGBQUAD FCObjImage::__Split16Bit_555 (WORD wPixel)
{
    RGBQUAD     rgb ;
    PCL_R(&rgb) = (MASK16_RED_555 & wPixel) >> 7 ;
    PCL_G(&rgb) = (MASK16_GREEN_555 & wPixel) >> 2 ;
    PCL_B(&rgb) = (MASK16_BLUE_555 & wPixel) << 3 ;
    return rgb ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::__ConvertToTrueColor (int iColor)
{
    if (!IsValidImage() || (ColorBits() == iColor))
        return ;
    if ((iColor != 24) && (iColor != 32))
        {assert(false); return;}

    // backup image
    const FCObjImage     OldPic (*this) ;
    if (!Create (OldPic.Width(), OldPic.Height(), iColor))
        return ;

    // get palette
    RGBQUAD     pPal[256] ;
    if (OldPic.ColorBits() <= 8)
        OldPic.GetColorTable (0, 1 << OldPic.ColorBits(), pPal) ;

    // start color convert
    const int   nNewSpan = this->ColorBits() / 8, // 3 or 4
                nOldSpan = OldPic.ColorBits() / 8 ;
    for (int y=0 ; y < Height() ; y++)
    {
        const BYTE   * pOld = OldPic.GetBits (y) ;
              BYTE   * pNew = this->GetBits (y) ;
        for (int x=0 ; x < Width() ; x++, pNew+=nNewSpan, pOld+=nOldSpan)
        {
            switch (OldPic.ColorBits())
            {
                case 1 :
                case 4 :
                case 8 : // 1,4,8 ==> 24,32
                    FCColor::CopyPixel (pNew, &pPal[OldPic.GetPixelData(x,y)], 3) ;
                    break ;
                case 16 : // 16 ==> 24,32
                    {
                        RGBQUAD     crTrans ;
                        if (OldPic.m_dwBitFields[1] == MASK16_GREEN_555)
                            crTrans = __Split16Bit_555 (*(WORD*)pOld) ;
                        else if (OldPic.m_dwBitFields[1] == MASK16_GREEN_565)
                            crTrans = __Split16Bit_565 (*(WORD*)pOld) ;
                        FCColor::CopyPixel (pNew, &crTrans, 3) ;
                    }
                    break ;
                case 24 :
                case 32 : // 24,32 ==> 32,24
                    FCColor::CopyPixel (pNew, pOld, 3) ;
                    break ;
            }
        }
    }
    if (iColor == 32)
        SetAlphaChannelValue (0xFF) ; // set alpha to 0xFF
}
//-----------------------------------------------------------------------------
inline void FCObjImage::GetAlphaChannel (FCObjImage* imgAlpha) const
{
    // create alpha-channel image, it's a 8-bit color image
    if (!imgAlpha || !IsValidImage() || (ColorBits() != 32) || (imgAlpha == this)
        || !imgAlpha->Create (Width(), Height(), 8))
    {
        assert(false) ; return ;
    }

    // get alpha channel
    for (int y=0 ; y < Height() ; y++)
        for (int x=0 ; x < Width() ; x++)
            *imgAlpha->GetBits(x,y) = PCL_A(GetBits(x,y)) ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::AppendAlphaChannel (const FCObjImage& alpha)
{
    if (!IsValidImage() || !alpha.IsValidImage() || (ColorBits() != 32) || (alpha.ColorBits() != 8) ||
        (Width() != alpha.Width()) || (Height() != alpha.Height()))
    {
        assert(false) ; return ;
    }

    // append alpha channel
    for (int y=0 ; y < Height() ; y++)
        for (int x=0 ; x < Width() ; x++)
            PCL_A(GetBits(x,y)) = *alpha.GetBits(x,y) ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::SetAlphaChannelValue (int nValue)
{
    if (!IsValidImage() || (ColorBits() != 32))
    {
        assert(false) ; return ;
    }

    for (int y=0 ; y < Height() ; y++)
        for (int x=0 ; x < Width() ; x++)
            PCL_A(GetBits(x,y)) = nValue ;
}
//-----------------------------------------------------------------------------
inline FCObjImage& FCObjImage::operator= (const FCObjImage& img)
{
    if (!img.IsValidImage() || (&img == this))
        return (*this) ;

    PCL_array<BITMAPINFOHEADER>   bmfh (img.NewImgInfoWithPalette()) ;
    if (Create (bmfh.get()))
    {
        // copy the pixels
        memcpy (GetMemStart(), img.GetMemStart(), img.GetPitch()*img.Height()) ;
        // copy the palette
        if (img.ColorBits() <= 8)
            CopyPalette (img) ;

        // copy position
        FCObjGraph::operator=(img) ;
    }
    return *this ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::GetSubBlock (FCObjImage* SubImg, RECT rcBlock) const
{
    if (!IsValidImage() || !SubImg || (SubImg == this) || (ColorBits() < 8))
    {
        assert(false); return false;
    }

    const RECT   rcImage = {0, 0, Width(), Height()} ;
          RECT   rcD ;
    assert (IsRectInRect (rcImage, rcBlock)) ;
    if (::IntersectRect (&rcD, &rcImage, &rcBlock) == 0)
    {
        assert(false); return false; // rect of destination is empty
    }

    if (!SubImg->Create (RECTWIDTH(rcD), RECTHEIGHT(rcD), ColorBits()))
        return false ;

    // copy pixel
    const int     nSubPitch = SubImg->Width() * ColorBits() / 8 ;
    for (int i=0 ; i < SubImg->Height() ; i++)
        memcpy (SubImg->GetBits(i), GetBits(rcD.left, rcD.top + i), nSubPitch) ;

    // copy palette
    if (ColorBits() <= 8)
        SubImg->CopyPalette (*this) ;

    // set relative position
    SubImg->SetGraphObjPos (rcD.left, rcD.top) ;
    return true ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::CoverBlock (const FCObjImage& img, int x, int y)
{
    if (!IsValidImage() || !img.IsValidImage() || (ColorBits() != img.ColorBits()) || (ColorBits() < 8))
    {
        assert(false); return false;
    }

    // calculate covered RECT
    const RECT   rcImage = {0, 0, Width(), Height()},
                 rcCover = {x, y, x+img.Width(), y+img.Height()} ;
    RECT         rcD ;
    if (::IntersectRect (&rcD, &rcImage, &rcCover) == 0)
        return false ; // rect of destination is empty

    // copy pixel
    const int     nSubPitch = RECTWIDTH(rcD) * img.ColorBits() / 8 ;
    for (int cy=rcD.top ; cy < rcD.bottom ; cy++) // copy
    {
        const BYTE   * pS = img.GetBits (rcD.left-x, cy-y) ; // calculate edge
              BYTE   * pD = this->GetBits (rcD.left, cy) ;
        memcpy (pD, pS, nSubPitch) ;
    }
    return true ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::TileBlock (const FCObjImage& img, int x, int y)
{
    int   nYStart = y ;
    while (nYStart < Height())
    {
        int    nXStart = x ;
        while (nXStart < Width())
        {
            CoverBlock (img, nXStart, nYStart) ; // security ensured by CoverBlock
            nXStart += img.Width() ;
        }
        nYStart += img.Height() ;
    }
}
//-----------------------------------------------------------------------------
inline void FCObjImage::CombineImage (const FCObjImage& Img32, int x, int y, int nAlphaPercent)
{
    RECT   rcD ;
    {
        RECT   rcImg = {0, 0, Width(), Height()},
               rcMask = {x, y, x+Img32.Width(), y+Img32.Height()} ;
        ::IntersectRect (&rcD, &rcImg, &rcMask) ;
    }
    if ((Img32.ColorBits() != 32) || (ColorBits() != 32) || IsRectEmpty(&rcD))
    {
        assert(false); return;
    }

    nAlphaPercent = FClamp (nAlphaPercent, 0, 100) ;
    for (int cy=rcD.top ; cy < rcD.bottom ; cy++)
    {
        RGBQUAD   * pDest = (RGBQUAD*)this->GetBits (rcD.left, cy),
                  * pSrc = (RGBQUAD*)Img32.GetBits (rcD.left-x, cy-y) ; // calculate edge
        for (int cx=rcD.left ; cx < rcD.right ; cx++, pDest++, pSrc++)
            FCColor::CombineAlphaPixel(pDest, *pDest, pSrc,
                                       (nAlphaPercent == 100) ? PCL_A(pSrc)
                                                              : (PCL_A(pSrc)*nAlphaPercent/100)) ;
    }
}
//-----------------------------------------------------------------------------
inline void FCObjImage::AlphaBlend (const FCObjImage& Img32, RECT rcDest, RECT rcSrc, int nAlphaPercent)
{
    if (nAlphaPercent == 0)
        return ;

    // parameter check, rcSrc must inside of Img32
    RECT     rcT ;
    {
        RECT   rcMask = {0, 0, Img32.Width(), Img32.Height()},
               rcImg = {0, 0, Width(), Height()} ;
        IntersectRect(&rcT, &rcDest, &rcImg) ;
        if (!IsValidImage() || (ColorBits() < 24) || !Img32.IsValidImage() || (Img32.ColorBits() != 32) ||
            !IsRectInRect (rcMask, rcSrc) || IsRectEmpty(&rcT))
        {
            assert(false); return;
        }
    }

    nAlphaPercent = FClamp (nAlphaPercent, 0, 100) ;
    const int   nSpan = ColorBits() / 8 ; // 3 or 4
    if ((RECTWIDTH(rcDest) == RECTWIDTH(rcSrc)) && (RECTHEIGHT(rcDest) == RECTHEIGHT(rcSrc)))
    {
        const int   nSrcX = rcSrc.left + rcT.left - rcDest.left ;
        for (int y=rcT.top ; y < rcT.bottom ; y++)
        {
            const BYTE  * pSrc = Img32.GetBits (nSrcX, rcSrc.top + y - rcDest.top) ; // notice edge
            BYTE        * pDest = this->GetBits (rcT.left, y) ;
            for (int x=rcT.left ; x < rcT.right ; x++, pDest+=nSpan, pSrc+=4)
                FCColor::AlphaBlendPixel (pDest, pSrc, (nAlphaPercent==100) ? PCL_A(pSrc)
                                                                            : PCL_A(pSrc)*nAlphaPercent/100) ;
        }
    }
    else
    {
        PCL_array<int>   pX (RECTWIDTH(rcT)),
                         pY (RECTHEIGHT(rcT)) ;

        {
            // get coordinate on source image
            for (int y=rcT.top ; y < rcT.bottom ; y++)
            {
                int   n = rcSrc.top+(y-rcDest.top)*RECTHEIGHT(rcSrc)/RECTHEIGHT(rcDest) ;

                // get max which same scaled
                while(true)
                {
                    int   yn1 = rcDest.top + (n+1-rcSrc.top) * RECTHEIGHT(rcDest) / RECTHEIGHT(rcSrc) ;
                    if (yn1 != y)
                        break ;
                    n++ ;
                }

                pY[y-rcT.top] = FClamp(n, 0, Img32.Height()-1) ;
            }
            for (int x=rcT.left ; x < rcT.right ; x++)
            {
                int   n = rcSrc.left+(x-rcDest.left)*RECTWIDTH(rcSrc)/RECTWIDTH(rcDest) ;

                // get max which same scaled
                while(true)
                {
                    int   xn1 = rcDest.left + (n+1-rcSrc.left) * RECTWIDTH(rcDest) / RECTWIDTH(rcSrc) ;
                    if (xn1 != x)
                        break ;
                    n++ ;
                }

                pX[x-rcT.left] = FClamp(n, 0, Img32.Width()-1) ;
            }
        }

        for (int y=rcT.top ; y < rcT.bottom ; y++)
        {
            BYTE   * pDest = this->GetBits (rcT.left, y) ;
            for (int x=rcT.left ; x < rcT.right ; x++, pDest+=nSpan)
            {
                const BYTE   * pSrc = Img32.GetBits (pX[x-rcT.left], pY[y-rcT.top]) ;
                FCColor::AlphaBlendPixel (pDest, pSrc, (nAlphaPercent==100) ? PCL_A(pSrc)
                                                                            : PCL_A(pSrc)*nAlphaPercent/100) ;
            }
        }
    }
}
//-----------------------------------------------------------------------------
inline void FCObjImage::LogicalBlend (const FCObjImage& MaskImg, LOGICAL_OP LogOP, int x, int y)
{
    if (!IsValidImage() || !MaskImg.IsValidImage() || (ColorBits() != MaskImg.ColorBits()) || (this == &MaskImg) || (ColorBits() != 8))
    {
        assert(false) ; return ;
    }

    // inner rect
    RECT   rc ;
    {
        RECT   rc1 = {0, 0, Width(), Height()},
               rc2 = {x, y, x+MaskImg.Width(), y+MaskImg.Height()} ;
        if (::IntersectRect (&rc, &rc1, &rc2) == 0)
            return ; // no intersected rect
    }

    for (int cy=rc.top ; cy < rc.bottom ; cy++)
        for (int cx=rc.left ; cx < rc.right ; cx++)
        {
            BYTE   * pSrc = MaskImg.GetBits (cx-x, cy-y) ; // notice edge
            BYTE   * pDest = this->GetBits (cx, cy) ;

            switch (LogOP)
            {
                case LOGI_OR  : *pDest |= *pSrc ; break ;
                case LOGI_AND : *pDest &= *pSrc ; break ;
                case LOGI_XOR : *pDest ^= *pSrc ; break ;
                case LOGI_SEL_ADD :
                    if (*pSrc == 0xFF)
                        *pDest = 0xFF ;
                    break ;
                case LOGI_SEL_SUB :
                    if (*pSrc == 0xFF)
                        *pDest = 0 ;
                    break ;
                default : assert(false);
            }
        }
}
//-----------------------------------------------------------------------------
inline int FCObjImage::Serialize (bool bSave, BYTE* pSave)
{
    const BYTE   * pBak = pSave ;
    if (bSave) // save
    {
        assert (ColorBits() > 8) ; // must true color image
        memcpy (pSave, &m_DibInfo, sizeof(m_DibInfo)) ; pSave += sizeof(m_DibInfo) ;
        memcpy (pSave, m_dwBitFields, 12) ; pSave += 12 ;

        int     nWrite = GetPitch() * Height() ;
        memcpy (pSave, GetMemStart(), nWrite) ; pSave += nWrite ;
    }
    else // load
    {
        Create ((BITMAPINFOHEADER*)pSave) ;
        pSave += sizeof(m_DibInfo) + 12 ;

        int     nWrite = GetPitch() * Height() ;
        memcpy (GetMemStart(), pSave, nWrite) ;
        pSave += nWrite ;
    }
    pSave += FCObjGraph::Serialize (bSave, pSave) ;
    return (int)(pSave - pBak) ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::Load (const char* szFileName, FCImageProperty* pProperty)
{
    IMAGE_TYPE     imgType = GetImageHandleFactory()->QueryImageFileType(szFileName) ;
    std::auto_ptr<FCImageHandleBase>  pHandler (GetImageHandleFactory()->CreateImageHandle(imgType)) ;
    if (!pHandler.get())
        return false ;

    PCL_Interface_Composite<FCObjImage>   listImage ;
    std::auto_ptr<FCImageProperty>        pImgProp ;
    bool     bRet = pHandler->LoadImageFile (szFileName, listImage, pImgProp) ;
    if (bRet)
    {
        if (listImage.PCL_GetObjectCount())
            *this = *listImage.PCL_GetObject(0) ;

        if (pImgProp.get() && pProperty)
            *pProperty = *pImgProp ;
    }
    assert (bRet) ;
    return bRet ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::Load (BYTE* pStart, int nMemSize, IMAGE_TYPE imgType, FCImageProperty* pProperty)
{
    std::auto_ptr<FCImageHandleBase>  pHandler (GetImageHandleFactory()->CreateImageHandle(imgType)) ;
    if (!pHandler.get())
        return false ;

    PCL_Interface_Composite<FCObjImage>   listImage ;
    std::auto_ptr<FCImageProperty>        pImgProp ;
    bool     bRet = pHandler->LoadImageMemory (pStart, nMemSize, listImage, pImgProp) ;
    if (bRet)
    {
        if (listImage.PCL_GetObjectCount())
            *this = *listImage.PCL_GetObject(0) ;
        
        if (pImgProp.get() && pProperty)
            *pProperty = *pImgProp ;
    }
    assert (bRet) ;
    return bRet ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::LoadDIBStream (const void* pDIB, int nBufferSize)
{
    const BITMAPINFOHEADER   * pBmif = (const BITMAPINFOHEADER*)pDIB ;
    if (!Create(pBmif))
        {assert(false); return false;}

    const BYTE   * p = (const BYTE*)pDIB + pBmif->biSize ;
    if (ColorBits() <= 8)
    {
        int   n = 1 << ColorBits() ;
        SetColorTable (0, n, (const RGBQUAD*)p) ;
        p += (4 * n) ;
    }
    else if (pBmif->biCompression == BI_BITFIELDS)
    {
        p += 12 ;
    }

    // copy pixel
    int   nLeave = nBufferSize - (int)(p - (BYTE*)pDIB) ;
    assert (nLeave >= GetPitch()*Height()) ;
    memcpy (GetMemStart(), p, FMin (nLeave, GetPitch()*Height())) ;
    return true ;
}
//-----------------------------------------------------------------------------
inline bool FCObjImage::Save (const char* szFileName, const FCImageProperty& rProp) const
{
    if (!IsValidImage() || !szFileName)
        return false ;

    IMAGE_TYPE     imgType = GetImageHandleFactory()->QueryImageFileType(szFileName) ;
    std::auto_ptr<FCImageHandleBase>  pHandler (GetImageHandleFactory()->CreateImageHandle(imgType)) ;
    if (!pHandler.get())
        return false ;

    // save list
    std::deque<const FCObjImage*>   saveList ;
    saveList.push_back (this) ;

    return pHandler->SaveImageFile (szFileName, saveList, rProp) ;
}
//-----------------------------------------------------------------------------
inline void FCObjImage::__FillImageRect (const FCObjImage& img, const RECT& rcBlock, const void* pSrc)
{
    RECT     rc = {0, 0, img.Width(), img.Height()} ;
    IntersectRect (&rc, &rc, &rcBlock) ;
    if (IsRectEmpty(&rc))
        return  ;

    const int   nSpan = img.ColorBits() / 8 ; // 1, 2, 3, 4
    for (int y=rc.top ; y < rc.bottom ; y++)
    {
        BYTE   * pPixel = img.GetBits (rc.left, y) ;
        for (int x=rc.left ; x < rc.right ; x++, pPixel += nSpan)
            FCColor::CopyPixel (pPixel, pSrc, nSpan) ;
    }
}
// add frame
// bCopyEdge: duplicate edge during copying
inline void FCObjImage::ExpandFrame (bool bCopyEdge, int iLeft, int iTop, int iRight, int iBottom)
{
    if ((ColorBits() < 8) || (iLeft < 0) || (iTop < 0) || (iRight < 0) || (iBottom < 0))
    {
        assert(false) ; return ;
    }
    if ((iLeft == 0) && (iTop == 0) && (iRight == 0) && (iBottom == 0))
        return ;

    // backup image then create expanded image
    const FCObjImage     imgOld(*this) ;
    if (!Create (imgOld.Width()+iLeft+iRight, imgOld.Height()+iTop+iBottom, imgOld.ColorBits()))
    {
        assert(false) ; return ;
    }

    // adjust image's position
    SetGraphObjPos (imgOld.GetGraphObjPos().x - iLeft, imgOld.GetGraphObjPos().y - iTop) ;

    // duplicate source image
    CoverBlock (imgOld, iLeft, iTop) ;

    // edge disposal
    if (!bCopyEdge)
        return ;

    // duplicate corner
    const RECT   rcUL = {0, 0, iTop, iLeft},
                 rcUR = {Width()-iRight, 0, Width(), iTop},
                 rcDL = {0, Height()-iBottom, iLeft, Height()},
                 rcDR = {Width()-iRight, Height()-iBottom, Width(), Height()} ;
    __FillImageRect (*this, rcUL, imgOld.GetBits (0, 0)) ;
    __FillImageRect (*this, rcUR, imgOld.GetBits (imgOld.Width()-1, 0)) ;
    __FillImageRect (*this, rcDL, imgOld.GetBits (0, imgOld.Height()-1)) ;
    __FillImageRect (*this, rcDR, imgOld.GetBits (imgOld.Width()-1, imgOld.Height()-1)) ;

    // duplicate four-edge
    const int   dwPitch = GetPitch(),
                nSpan = ColorBits() / 8,
                nOldLineBytes = imgOld.Width() * nSpan ;
    int         m ;
    BYTE        * pSrc, * pDest ;

	// up
    pSrc = GetBits (iLeft, iTop) ;
    pDest = pSrc + dwPitch ;
    for (m=0 ; m < iTop ; m++, pDest += dwPitch)
        memcpy (pDest, pSrc, nOldLineBytes) ;
    // bottom
    pSrc = GetBits (iLeft, imgOld.Height() + iTop - 1) ;
    pDest = pSrc - dwPitch ;
    for (m=0 ; m < iBottom ; m++, pDest -= dwPitch)
        memcpy (pDest, pSrc, nOldLineBytes) ;

    // left
    pSrc = GetBits (iLeft, iTop) ;
    pDest = GetBits (0, iTop) ;
    for (m=0 ; m < imgOld.Height() ; m++, pDest -= dwPitch, pSrc -= dwPitch)
    {
        BYTE     * pTemp = pDest ;
        for (int i=0 ; i < iLeft ; i++, pTemp += nSpan)
            FCColor::CopyPixel (pTemp, pSrc, nSpan) ;
    }
    // right
    pSrc = GetBits (iLeft + imgOld.Width() - 1, iTop) ;
    pDest = pSrc + nSpan ;
    for (m=0 ; m < imgOld.Height() ; m++, pDest -= dwPitch, pSrc -= dwPitch)
    {
        BYTE     * pTemp = pDest ;
        for (int i=0 ; i < iRight ; i++, pTemp += nSpan)
            FCColor::CopyPixel (pTemp, pSrc, nSpan) ;
    }
}
//-----------------------------------------------------------------------------
// stretch (>=8 bit)
inline void FCObjImage::Stretch (int nNewWidth, int nNewHeight)
{
    // parameter check
    if (!IsValidImage() || (nNewWidth <= 0) || (nNewHeight <= 0) || (ColorBits() < 8))
    {
//        assert(false) ;
        return ;
    }
    if ((nNewWidth == Width()) && (nNewHeight == Height()))
        return ;

    // first backup image
    const FCObjImage     imgOld(*this) ;
    if (!Create (nNewWidth, nNewHeight, imgOld.ColorBits()))
    {
        assert(false) ; return ;
    }

    // duplicate palette
    if (ColorBits() <= 8)
        CopyPalette (imgOld) ;

    // initialize index table
    const int        nSpan = ColorBits() / 8 ;
    PCL_array<int>   pTabX (Width()) ;
    for (int xx=0 ; xx < Width() ; xx++)
    {
        pTabX[xx] = xx * imgOld.Width() / Width() ; // force to omit float
        assert (pTabX[xx] < imgOld.Width()) ;
    }
    for (int mm=0 ; mm < (Width() - 1) ; mm++)
        pTabX[mm] = (pTabX[mm+1] - pTabX[mm]) * nSpan ;
    // pTabX[i] put X byte span

    for (int y=0 ; y < Height() ; y++)
    {
        const BYTE   * pOld = imgOld.GetBits (y * imgOld.Height() / Height()) ;
              BYTE   * pPixel = GetBits (y) ;
        for (int x=0 ; x < Width() ; x++)
        {
            FCColor::CopyPixel (pPixel, pOld, nSpan) ;
            pOld += pTabX[x] ;
            pPixel += nSpan ;
        }
    }
}
//-----------------------------------------------------------------------------
inline void FCObjImage::SinglePixelProcessProc (FCInterface_PixelProcess& rProcessor, FCObjProgress* pProgress)
{
    if (!rProcessor.ValidateColorBits (this))
        {assert(false); return;}

    // before
    rProcessor.OnEnterProcess (this) ;
    if (pProgress)
        pProgress->ResetProgress() ; // reset to 0

    switch (rProcessor.QueryProcessType())
    {
        case FCInterface_PixelProcess::PROCESS_TYPE_PIXEL :
            {
                for (int y=0 ; y < Height() ; y++)
                {
                    for (int x=0 ; x < Width() ; x++)
                    {
                        rProcessor.ProcessPixel (this, x, y, GetBits(x,y)) ;
                    }
                    if (pProgress)
                        pProgress->SetProgress ((y+1) * 100 / Height()) ;
                }
            }
            break ;

        case FCInterface_PixelProcess::PROCESS_TYPE_WHOLE :
            rProcessor.ProcessWholeImage (this, pProgress) ;
            break ;
    }

    // after
    rProcessor.OnLeaveProcess (this) ;
}

#endif
