/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-4-9
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_IMAGE_HANDLE_BMP__2004_04_09__H__
#define __FOO_IMAGE_HANDLE_BMP__2004_04_09__H__
#include "../ObjImage.h"

//class FCImageHandle ;
    class FCImageHandle_Bmp ;

//=============================================================================
/**
 *  Read/Write BMP image.
 */
class FCImageHandle_Bmp : public FCImageHandleBase
{
    // inner struct
    #pragma pack(2)
        struct PCL_BMPHEADER
        {
            WORD    bfType;
            DWORD   bfSize;
            WORD    bfReserved1;
            WORD    bfReserved2;
            DWORD   bfOffBits;
        };
    #pragma pack()

    /// Load BMP image.
    virtual bool LoadImageMemory (const BYTE* pStart, int nMemSize,
                                  PCL_Interface_Composite<FCObjImage>& rImageList,
                                  std::auto_ptr<FCImageProperty>& rImageProp)
    {
        if (!pStart || (nMemSize <= sizeof(PCL_BMPHEADER)))
            return false ;
        if (((PCL_BMPHEADER*)pStart)->bfType != 0x4D42)
            return false ;

        // size check
        const int   nOffset = (int)((PCL_BMPHEADER*)pStart)->bfOffBits ;
        if (nMemSize <= nOffset)
            return false ;

        // create image
        BITMAPINFOHEADER   * pBmih = (BITMAPINFOHEADER*)(pStart + sizeof(PCL_BMPHEADER)) ;
        FCObjImage         * pImg = new FCObjImage ;
        if (!pImg->Create(pBmih))
        {
            delete pImg; return false;
        }

        // set palette
        if (pImg->ColorBits() <= 8)
        {
            BYTE   * pPal = (BYTE*)pBmih + pBmih->biSize ;
            pImg->SetColorTable (0, 1<<pImg->ColorBits(), (RGBQUAD*)pPal) ;
        }

        // set pixel
        int     nCopyByte = pImg->GetPitch()*pImg->Height() ;
        assert (nMemSize - nOffset >= nCopyByte) ;
        nCopyByte = FMin (nCopyByte, nMemSize - nOffset) ;
        memcpy (pImg->GetMemStart(), pStart + nOffset, nCopyByte) ;

        // add to list
        rImageList.PCL_PushObject (pImg) ;
        return true ;
    }

    /// Save BMP image.
    virtual bool SaveImageFile (const char* szFileName,
                                const std::deque<const FCObjImage*>& rImageList,
                                const FCImageProperty& rImageProp)
    {
        if (rImageList.empty() || !rImageList[0]->IsValidImage())
            return false ;
        const FCObjImage   &img = *rImageList[0] ;

        // create image file, if the file already exists, its contents are discarded.
        FILE   * pf = fopen (szFileName, "wb") ;
        if (!pf)
            return false ;

        // calculate total size of bmp file.
        const int   nPixs = img.GetPitch() * img.Height() ;
        int   nTotalSize = sizeof(PCL_BMPHEADER) + sizeof(BITMAPINFOHEADER) + nPixs ;

        PCL_array<BITMAPINFOHEADER>   info (img.NewImgInfoWithPalette()) ;
        if (img.ColorBits() <= 8)
            nTotalSize += (4 * (1<<img.ColorBits())) ;
        else if (info.get()->biCompression == BI_BITFIELDS)
            nTotalSize += 12 ;

        // write bmp file header
        PCL_BMPHEADER     bmHeader ;
        bmHeader.bfType = 0x4D42 ; // "BM"
        bmHeader.bfSize = nTotalSize ;
        bmHeader.bfReserved1 = bmHeader.bfReserved2 = 0 ;
        bmHeader.bfOffBits = nTotalSize - nPixs ;
        fwrite (&bmHeader, 1, sizeof(bmHeader), pf) ;

        // write BMP file info / bitfields / palette
        fwrite (info.get(), 1, nTotalSize - sizeof(PCL_BMPHEADER) - nPixs, pf) ;

        // write pixels value
        fwrite (img.GetMemStart(), 1, nPixs, pf) ;
        fclose (pf) ;
        return true ;
    }
};

#endif
