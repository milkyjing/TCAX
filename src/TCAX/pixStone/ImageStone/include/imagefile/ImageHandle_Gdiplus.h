/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-6-18
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
*/
#ifndef __FOO_IMAGE_HANDLE_GDIPLUS__2004_06_18__H__
#define __FOO_IMAGE_HANDLE_GDIPLUS__2004_06_18__H__
#include "../FWin32.h"

//class FCImageHandle ;
    class FCImageHandle_Gdiplus ;

//=============================================================================
/**
 *  Auto load GDI+ module.
 *  load GDI+ module at constructor and unload at destructor.
 */
class FCAutoInitGDIPlus
{
    ULONG_PTR   m_GdiplusToken ;
public:
    /// Constructor (load GDI+ module).
    FCAutoInitGDIPlus()
    {
        Gdiplus::GdiplusStartupInput   gpSI ;
        Gdiplus::GdiplusStartup (&m_GdiplusToken, &gpSI, NULL) ;
    }
    /// Destructor (unload GDI+ module).
    virtual ~FCAutoInitGDIPlus()
    {
        Gdiplus::GdiplusShutdown (m_GdiplusToken) ;
    }

    /// Get bmp/jpeg/gif/tiff/png image's CLSID.
    static bool GetImageEncoderClsid (IMAGE_TYPE imgType, CLSID* pClsid)
    {
        bstr_t     strType ;
        switch (imgType)
        {
            case IMG_BMP : strType = "image/bmp"  ; break;
            case IMG_JPG : strType = "image/jpeg" ; break;
            case IMG_GIF : strType = "image/gif"  ; break;
            case IMG_TIF : strType = "image/tiff" ; break;
            case IMG_PNG : strType = "image/png"  ; break;
            default : return false ;
        }

        UINT     nNum=0, nSize=0 ;
        Gdiplus::GetImageEncodersSize (&nNum, &nSize) ;
        if (nSize <= 0)
            return false ;

        PCL_array<Gdiplus::ImageCodecInfo>   pICI (new BYTE[nSize]) ;
        Gdiplus::GetImageEncoders (nNum, nSize, pICI.get()) ;

        for (UINT i=0 ; i < nNum; i++)
        {
            if (bstr_t(pICI[i].MimeType) == strType)
            {
                *pClsid = pICI[i].Clsid ;
                return true ;
            }
        }
        return false ;
    }
};

//=============================================================================
/**
 *  Read/Write image via Gdi+.
 *  if you load a gif image with transparent color, you will get a 32bpp image object, transparent color be converted to alpha=0 pixel
 */
class FCImageHandle_Gdiplus : public FCImageHandleBase,
                              public FCAutoInitGDIPlus
{
    // create image property object base on Bitmap, must delete returned object
    static FCImageProperty* CreatePropertyFromBitmap (Gdiplus::Bitmap& gp_Bmp)
    {
        // get all property
        UINT   nBytes, nNum ;
        gp_Bmp.GetPropertySize (&nBytes, &nNum) ;
        PCL_array<Gdiplus::PropertyItem>   gp_Item (new BYTE[nBytes]) ;
        gp_Bmp.GetAllPropertyItems (nBytes, nNum, gp_Item.get()) ;

        // fill tag
        FCImageProperty   * pProp = new FCImageProperty ;
        for (UINT i=0 ; i < nNum ; i++)
        {
            Gdiplus::PropertyItem   & rItem = gp_Item[i] ;
            FIMAGE_TAG     nTag = (FIMAGE_TAG)rItem.id ;

            // convert all type to string
            if (rItem.type == PropertyTagTypeASCII)
            {
                std::string   s ((char*)rItem.value, rItem.length) ;
                pProp->SetPropertyValue (nTag, s.c_str()) ;
            }
            else if (rItem.type == PropertyTagTypeRational)
            {
                if (rItem.length == 8)
                {
                    std::string   s = FCOXOHelper::X2A (*(unsigned long*)rItem.value) ;
                    s += "/" ;
                    s += FCOXOHelper::X2A (((unsigned long*)rItem.value)[1]) ;
                    pProp->SetPropertyValue (nTag, s.c_str()) ;
                }
            }
            else if (rItem.type == PropertyTagTypeSRational)
            {
                if (rItem.length == 8)
                {
                    std::string   s = FCOXOHelper::X2A (*(long*)rItem.value) ;
                    s += "/" ;
                    s += FCOXOHelper::X2A (((long*)rItem.value)[1]) ;
                    pProp->SetPropertyValue (nTag, s.c_str()) ;
                }
            }
            else if (rItem.type == PropertyTagTypeShort)
            {
                if (rItem.length == 2)
                {
                    std::string   s = FCOXOHelper::X2A (*(unsigned short*)rItem.value) ;
                    pProp->SetPropertyValue (nTag, s.c_str()) ;
                }
            }
            else if (rItem.type == PropertyTagTypeLong)
            {
                if (rItem.id == PropertyTagFrameDelay)
                {
                    assert (rItem.length % 4 == 0) ;
                    for (ULONG i=0 ; i < rItem.length/4 ; i++)
                    {
                        unsigned long   lv = ((unsigned long*)rItem.value)[i] ;
                        pProp->PutFrameDelay ((int)lv * 10) ;
                    }
                }
                else
                {
                    if (rItem.length == 4)
                    {
                        std::string   s = FCOXOHelper::X2A (*(unsigned long*)rItem.value) ;
                        pProp->SetPropertyValue (nTag, s.c_str()) ;
                    }
                }
            }
        }
        return pProp ;
    }

    // property tag <==> type
    static void GetGdiplusPropertyTypeTab (PCL_TT_Convertor<PROPID, WORD>& aTab)
    {
        aTab.Clear() ;
        aTab.AddElement (PropertyTagEquipMake, PropertyTagTypeASCII) ;
        aTab.AddElement (PropertyTagEquipModel, PropertyTagTypeASCII) ;
        aTab.AddElement (PropertyTagExifDTOrig, PropertyTagTypeASCII) ;
        aTab.AddElement (PropertyTagExifExposureTime, PropertyTagTypeRational) ;
        aTab.AddElement (PropertyTagExifFNumber, PropertyTagTypeRational) ;
        aTab.AddElement (PropertyTagExifFocalLength, PropertyTagTypeRational) ;
        aTab.AddElement (PropertyTagExifISOSpeed, PropertyTagTypeShort) ;
        aTab.AddElement (PropertyTagExifExposureBias, PropertyTagTypeSRational) ;
        aTab.AddElement (PropertyTagExifMaxAperture, PropertyTagTypeRational) ;
        aTab.AddElement (PropertyTagExifFlash, PropertyTagTypeShort) ;
        aTab.AddElement (PropertyTagExifMeteringMode, PropertyTagTypeShort) ;
        aTab.AddElement (PropertyTagExifExposureProg, PropertyTagTypeShort) ;
    }

    // Add property into bitmap object.
    static void AddPropertyInBitmap (const FCImageProperty& rImageProp, Gdiplus::Bitmap& gpBmp)
    {
        PCL_TT_Convertor<PROPID, WORD>   tabType ;
        GetGdiplusPropertyTypeTab (tabType) ;

        // put image's property
        for (int i=0 ; i < rImageProp.GetElementCount() ; i++)
        {
            Gdiplus::PropertyItem     aItem ;
            aItem.id = rImageProp.GetT1(i) ;
            aItem.type = tabType.First_to_Second (aItem.id, PropertyTagTypeUndefined) ;
            if (aItem.type == PropertyTagTypeUndefined)
                continue ;

            const std::string   & s = rImageProp.GetT2(i) ;
            if (aItem.type == PropertyTagTypeASCII)
            {
                PCL_array<char>   pBuf (s.length() + 1) ;
                ZeroMemory (pBuf.get(), s.length() + 1) ;
                CopyMemory (pBuf.get(), s.c_str(), s.length()) ;
                aItem.length = s.length() + 1 ;
                aItem.value = pBuf.get() ;
                gpBmp.SetPropertyItem (&aItem) ;
            }
            else if (aItem.type == PropertyTagTypeRational)
            {
                size_t   nPos = s.find ("/") ;
                if (nPos != std::string::npos)
                {
                    unsigned long   p[2] ;
                    FCOXOHelper::A2X (s.substr(0,nPos), p[0]) ;
                    FCOXOHelper::A2X (s.substr(nPos+1), p[1]) ;
                    aItem.length = 8 ;
                    aItem.value = p ;
                    gpBmp.SetPropertyItem (&aItem) ;
                }
            }
            else if (aItem.type == PropertyTagTypeSRational)
            {
                size_t   nPos = s.find ("/") ;
                if (nPos != std::string::npos)
                {
                    long   p[2] ;
                    FCOXOHelper::A2X (s.substr(0,nPos), p[0]) ;
                    FCOXOHelper::A2X (s.substr(nPos+1), p[1]) ;
                    aItem.length = 8 ;
                    aItem.value = p ;
                    gpBmp.SetPropertyItem (&aItem) ;
                }
            }
            else if (aItem.type == PropertyTagTypeShort)
            {
                unsigned short   n ;
                FCOXOHelper::A2X (s, n) ;
                aItem.length = 2 ;
                aItem.value = &n ;
                gpBmp.SetPropertyItem (&aItem) ;
            }
            else if (aItem.type == PropertyTagTypeLong)
            {
                unsigned long   n ;
                FCOXOHelper::A2X (s, n) ;
                aItem.length = 4 ;
                aItem.value = &n ;
                gpBmp.SetPropertyItem (&aItem) ;
            }
        }
    }

    // Load image file via GDIPlus.
    virtual bool LoadImageFile (const char* szFileName,
                                PCL_Interface_Composite<FCObjImage>& rImageList,
                                std::auto_ptr<FCImageProperty>& rImageProp)
    {
        // load image file
        Gdiplus::Bitmap   gpBmp (bstr_t(szFileName), FALSE) ;
        return StoreMultiFrame (gpBmp, rImageList, rImageProp) ;
    }

    // Load image memory via GDIPlus.
    virtual bool LoadImageMemory (const BYTE* pStart, int nMemSize,
                                  PCL_Interface_Composite<FCObjImage>& rImageList,
                                  std::auto_ptr<FCImageProperty>& rImageProp)
    {
        if (!pStart || (nMemSize <= 0))
            return false ;

        // copy to HGLOBAL then load
        HGLOBAL   hBuffer = ::GlobalAlloc (GMEM_MOVEABLE, nMemSize) ;
        ::CopyMemory (::GlobalLock(hBuffer), pStart, nMemSize) ;
        ::GlobalUnlock (hBuffer) ;

        bool      bRet = false ;
        IStream   * pStream = NULL ;
        if (::CreateStreamOnHGlobal (hBuffer, TRUE, &pStream) == S_OK)
        {
            Gdiplus::Bitmap   gpBmp (pStream, FALSE) ;
            bRet = StoreMultiFrame (gpBmp, rImageList, rImageProp) ;
        }
        if (pStream)
            pStream->Release() ;
        return bRet ;
    }

    // Get all frames in gp_Bmp, add into rImageList.
    static bool StoreMultiFrame (Gdiplus::Bitmap& gp_Bmp,
                                 PCL_Interface_Composite<FCObjImage>& rImageList,
                                 std::auto_ptr<FCImageProperty>& rImageProp)
    {
        if (gp_Bmp.GetLastStatus() != Gdiplus::Ok)
        {
            assert(false); return false;
        }

        // get frame dimensions
        const UINT        nDim = gp_Bmp.GetFrameDimensionsCount() ;
        PCL_array<GUID>   listID (nDim) ;
        gp_Bmp.GetFrameDimensionsList (listID.get(), nDim) ;

        // get frame and store
        for (UINT i=0 ; i < nDim ; i++)
        {
            UINT   nFrame = gp_Bmp.GetFrameCount (&listID[i]) ;
            for (UINT j=0 ; j < nFrame ; j++)
            {
                gp_Bmp.SelectActiveFrame (&listID[i], j) ;

                FCObjImage   * pImg = new FCObjImage ;
                FCWin32::GDIPlus_LoadBitmap (gp_Bmp, *pImg) ;
                if (pImg->IsValidImage())
                {
                    rImageList.PCL_PushObject(pImg) ;
                }
                else
                {
                    delete pImg ; assert(false);
                }
            }
        }

        // store image property
        rImageProp = std::auto_ptr<FCImageProperty>(CreatePropertyFromBitmap(gp_Bmp)) ;
        return (rImageList.PCL_GetObjectCount() != 0) ;
    }

    // Save image to file via GDI+.
    virtual bool SaveImageFile (const char* szFileName,
                                const std::deque<const FCObjImage*>& rImageList,
                                const FCImageProperty& rImageProp)
    {
        if (rImageList.empty() || !rImageList[0]->IsValidImage())
            return false ;
        const FCObjImage   &img = *rImageList[0] ;

        // get encoder's CLSID
        CLSID        clsID ;
        IMAGE_TYPE   imgType = FCObjImage::GetImageHandleFactory()->QueryImageFileType(szFileName) ;
        if (!GetImageEncoderClsid (imgType, &clsID))
            return false ;

        // if image is jpeg format, set save quality
        std::auto_ptr<Gdiplus::EncoderParameters>   pEnParas ;
        ULONG      nQuality = JpegSaveQuality(rImageProp) ;
        if (imgType == IMG_JPG)
        {
            pEnParas = std::auto_ptr<Gdiplus::EncoderParameters>(new Gdiplus::EncoderParameters) ;
            pEnParas->Count = 1 ;
            pEnParas->Parameter[0].Guid = Gdiplus::EncoderQuality ;
            pEnParas->Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong ;
            pEnParas->Parameter[0].NumberOfValues = 1 ;
            pEnParas->Parameter[0].Value = &nQuality ;
        }

        // create a GDI+ bitmap and put property
        std::auto_ptr<Gdiplus::Bitmap>   pBmp (FCWin32::GDIPlus_CreateBitmap(img)) ;
        if (!pBmp.get())
            return false ;

        AddPropertyInBitmap (rImageProp, *pBmp) ;
        return (pBmp->Save (bstr_t(szFileName), &clsID, pEnParas.get()) == Gdiplus::Ok) ;
    }
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
