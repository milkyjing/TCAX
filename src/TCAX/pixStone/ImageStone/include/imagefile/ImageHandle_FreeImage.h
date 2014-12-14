/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-4-9
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_IMAGE_HANDLE_FREEIMAGE__2004_04_09__H__
#define __FOO_IMAGE_HANDLE_FREEIMAGE__2004_04_09__H__

//class FCImageHandle ;
    class FCImageHandle_FreeImage ;

//=============================================================================
/**
 *  Read/Write image via FreeImage library.
 *  It's a free image library, you can download from <a href='http://sourceforge.net/projects/freeimage' target='_blank'>http://sourceforge.net/projects/freeimage</a><BR><BR>
 *  Load from file can load all frames of gif/tiff, load from memory only load first frame.
 */
class FCImageHandle_FreeImage : public FCImageHandleBase
{
public:
    static bool FreeImage_to_FCImage (FIBITMAP* pFIimg, FCObjImage& img)
    {
        if (!pFIimg)
            return false ;

        // prepare image info
        BYTE              __bufImgInfo[sizeof(BITMAPINFOHEADER) + 16] = {0} ;
        BITMAPINFOHEADER  * pInfo = (BITMAPINFOHEADER*)__bufImgInfo ;
        memcpy (pInfo, FreeImage_GetInfoHeader(pFIimg), sizeof(BITMAPINFOHEADER)) ;
        if (pInfo->biCompression == BI_BITFIELDS)
        {
            DWORD   * pMask = (DWORD*)(pInfo + 1) ;
            pMask[0] = FreeImage_GetRedMask (pFIimg) ;
            pMask[1] = FreeImage_GetGreenMask (pFIimg) ;
            pMask[2] = FreeImage_GetBlueMask (pFIimg) ;
        }

        // create image
        if (!img.Create(pInfo))
            return false ;

        // set palette
        if (img.ColorBits() <= 8)
            img.SetColorTable (0, FreeImage_GetColorsUsed(pFIimg), FreeImage_GetPalette(pFIimg)) ;

        // set pixel
        assert (img.GetPitch() == FreeImage_GetPitch(pFIimg)) ;
        memcpy (img.GetMemStart(), FreeImage_GetBits(pFIimg), img.GetPitch()*img.Height()) ;

        // transparency
        if ((img.ColorBits() <= 8) && FreeImage_IsTransparent(pFIimg))
        {
            FCObjImage   idxImg (img) ;

            int    n = (int)FreeImage_GetTransparencyCount(pFIimg) ;
            BYTE   * p = FreeImage_GetTransparencyTable(pFIimg) ;

            img.ConvertTo32Bit() ;
            for (int y=0 ; y < img.Height() ; y++)
                for (int x=0 ; x < img.Width() ; x++)
                {
                    int   nIndex = *idxImg.GetBits(x,y) ;
                    if (nIndex < n)
                        PCL_A(img.GetBits(x,y)) = p[nIndex] ;
                }
        }
        return true ;
    }

    static FIBITMAP* AllocateFreeImage (const FCObjImage& img)
    {
        if (!img.IsValidImage())
            return 0 ;

        // create FreeImage object
        DWORD     dwBitFields[3] = {0, 0, 0} ;
        if (img.ColorBits() == 16)
        {
            PCL_array<BITMAPINFOHEADER>   info (img.NewImgInfoWithPalette()) ;
            memcpy (dwBitFields, info.get() + 1, 12) ;
        }
        FIBITMAP   * pFIimg = FreeImage_AllocateT (FIT_BITMAP, img.Width(), img.Height(), img.ColorBits(), dwBitFields[0], dwBitFields[1], dwBitFields[2]) ;
        if (!pFIimg)
            return 0 ;

        // set pixel
        assert (img.GetPitch() == FreeImage_GetPitch(pFIimg)) ;
        memcpy (FreeImage_GetBits(pFIimg), img.GetMemStart(), img.GetPitch()*img.Height()) ;

        // set palette
        if (img.ColorBits() <= 8)
        {
            RGBQUAD     pPal[256] = {0} ;
            img.GetColorTable (0, 1<<img.ColorBits(), pPal) ;
            memcpy (FreeImage_GetPalette(pFIimg), pPal, FreeImage_GetColorsUsed(pFIimg)*sizeof(RGBQUAD)) ;
        }
        return pFIimg ;
    }

private:
    static void StoreFrame (FIBITMAP* pFIimage,
                            PCL_Interface_Composite<FCObjImage>& rImageList,
                            std::auto_ptr<FCImageProperty>& rImageProp)
    {
        // create frame
        FCObjImage   * pImg = new FCObjImage ;
        FreeImage_to_FCImage (pFIimage, *pImg) ;
        if (pImg->IsValidImage())
        {
            rImageList.PCL_PushObject (pImg) ;
        }
        else
        {
            delete pImg ;
        }
    }

    /// Read file routine
    virtual bool LoadImageFile (const char* szFileName,
                                PCL_Interface_Composite<FCObjImage>& rImageList,
                                std::auto_ptr<FCImageProperty>& rImageProp)
    {
        // get image format
        FREE_IMAGE_FORMAT     imgType = FreeImage_GetFileType (szFileName) ;
        if (imgType == FIF_UNKNOWN)
            return false ;

        // read image file via FreeImage library
        if ((imgType == FIF_GIF) || (imgType == FIF_TIFF))
        {
            FIMULTIBITMAP   * FIMul = FreeImage_OpenMultiBitmap (imgType, szFileName, FALSE, TRUE) ;
            if (FIMul)
            {
                rImageProp = std::auto_ptr<FCImageProperty>(new FCImageProperty) ;
                // get all frames
                for (int i=0 ; i < FreeImage_GetPageCount(FIMul) ; i++)
                {
                    FIBITMAP   * pFI = FreeImage_LockPage (FIMul, i) ;
                    if (pFI)
                    {
                        StoreFrame (pFI, rImageList, rImageProp) ;

                        // get frame delay time
                        FITAG   * tag ;
                        if (FreeImage_GetMetadata (FIMD_ANIMATION, pFI, "FrameTime", &tag))
                        {
                            rImageProp->PutFrameDelay (*(int*)FreeImage_GetTagValue(tag)) ;
                        }
                        FreeImage_UnlockPage (FIMul, pFI, FALSE) ;
                    }
                }
                FreeImage_CloseMultiBitmap (FIMul) ;
            }
        }
        else
        {
            FIBITMAP   * pFI = FreeImage_Load (imgType, szFileName) ;
            if (pFI)
            {
                StoreFrame (pFI, rImageList, rImageProp) ;
                FreeImage_Unload (pFI) ;
            }
        }
        return rImageList.PCL_GetObjectCount() ? true : false ;
    }

    /// Read memory routine
    virtual bool LoadImageMemory (const BYTE* pStart, int nMemSize,
                                  PCL_Interface_Composite<FCObjImage>& rImageList,
                                  std::auto_ptr<FCImageProperty>& rImageProp)
    {
        FIMEMORY   * pMem = FreeImage_OpenMemory (const_cast<BYTE*>(pStart), nMemSize) ;
        if (!pMem)
            return false ;

        // get image format
        FREE_IMAGE_FORMAT   imgType = FreeImage_GetFileTypeFromMemory (pMem) ;
        FIBITMAP            * pFI = FreeImage_LoadFromMemory (imgType, pMem) ;
        if (pFI)
        {
            StoreFrame (pFI, rImageList, rImageProp) ;
            FreeImage_Unload (pFI) ;
        }
        FreeImage_CloseMemory (pMem) ;
        return rImageList.PCL_GetObjectCount() ? true : false ;
    }

    /// save to file routine
    virtual bool SaveImageFile (const char* szFileName,
                                const std::deque<const FCObjImage*>& rImageList,
                                const FCImageProperty& rImageProp)
    {
        if (rImageList.empty() || !rImageList[0]->IsValidImage())
            return false ;
        const FCObjImage   &img = *rImageList[0] ;

        // is the FreeImage library support
        FREE_IMAGE_FORMAT     imgFormat = FreeImage_GetFIFFromFilename (szFileName) ;
        if (imgFormat == FIF_UNKNOWN)
            return false ;
        if (!FreeImage_FIFSupportsWriting (imgFormat) || !FreeImage_FIFSupportsExportBPP (imgFormat, img.ColorBits()))
            return false ;

        // create FreeImage object
        FIBITMAP   * pFI = AllocateFreeImage (img) ;
        if (!pFI)
            return false ;

        // save flag
        int   nFlag = 0 ;
        if ((imgFormat == FIF_GIF) && (img.ColorBits() <= 8))
        {
            // gif transparency
            int     nCount = 1 << img.ColorBits(),
                    nIndex = GifSaveTransparentIndex(rImageProp) ;
            if (nIndex != -1)
            {
                PCL_array<BYTE>   aTab(nCount) ;
                memset (aTab.get(), 0xFF, nCount) ;
                aTab[FClamp(nIndex,0,nCount-1)] = 0 ;
                FreeImage_SetTransparent (pFI, true) ;
                FreeImage_SetTransparencyTable (pFI, aTab.get(), nCount) ;
            }
        }
        else if (imgFormat == FIF_JPEG)
        {
            nFlag = JpegSaveQuality(rImageProp) ;
        }

        // save image file
        bool   bRet = FreeImage_Save (imgFormat, pFI, szFileName, nFlag) ? true : false ;
        FreeImage_Unload (pFI) ;
        return bRet ;
    }

    virtual ~FCImageHandle_FreeImage() {FreeImage_DeInitialise();}
public:
    FCImageHandle_FreeImage() {FreeImage_Initialise();}
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
