/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-4-9
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_IMAGE_HANDLE_TGA__2004_04_09__H__
#define __FOO_IMAGE_HANDLE_TGA__2004_04_09__H__
#include "../ObjImage.h"

//class FCImageHandle ;
    class FCImageHandle_Tga ;

//=============================================================================
/**
 *  Read/Write TGA image.
 */
class FCImageHandle_Tga : public FCImageHandleBase
{
    // Definitions for TGA image types.
    enum
    {
        TGA_NULL    = 0,
        TGA_UCPAL   = 1,
        TGA_UCRGB   = 2,
        TGA_UCMONO  = 3,
        TGA_RLEPAL  = 9,
        TGA_RLERGB  = 10,
        TGA_RLEMONO = 11,
    };

    // TGA file header (same as BMP format, pixel store from left-bottom)
#pragma pack(1)
    struct TGAHEAD
    {
        BYTE  byID_Length ; // Number of Characters in Identification Field
        BYTE  byPalType ;   // 00 : no-palette     01 : has-palette
        BYTE  byImageType ;
        WORD  wPalFirstNdx ; // 调色板起始索引
        WORD  wPalLength ;   // number of color in palette
        BYTE  byPalBits ;    // 调色板中每一颜色所占位数
        WORD  wLeft ;
        WORD  wBottom ;
        WORD  wWidth ;      // width
        WORD  wHeight ;     // height
        BYTE  byColorBits ; // bpp
        struct
        {
            BYTE AlphaBits : 4 ; // number of attribute bits associated with each pixel
            BYTE HorMirror : 1 ; // 1: from right to left
            BYTE VerMirror : 1 ; // 为1表示图像数据上下颠倒存储
            BYTE Reserved : 2 ;
        } Descriptor ;
    }; // 18 - Bytes
#pragma pack()

    /// Load TGA image.
    virtual bool LoadImageMemory (const BYTE* pStart, int nMemSize,
                                  PCL_Interface_Composite<FCObjImage>& rImageList,
                                  std::auto_ptr<FCImageProperty>& rImageProp)
    {
        const TGAHEAD   * pTga = (TGAHEAD*)pStart ;
        if (!pStart || (nMemSize <= sizeof(TGAHEAD)))
            return false ;
        if ((pTga->byPalBits == 15) || (pTga->byPalBits == 16))
            return false ; // not support 15 or 16 bit palette

        // create image
        FCObjImage   * pImg = new FCObjImage ;
        if (!pImg->Create (pTga->wWidth, pTga->wHeight, (pTga->byColorBits == 15) ? 16 : pTga->byColorBits))
        {
            delete pImg; return false ;
        }

        // set palette
        const BYTE   * pCurr = pStart + sizeof(TGAHEAD) + pTga->byID_Length ;
        if (pTga->byPalType == 1)
        {
            if (pTga->wPalFirstNdx + pTga->wPalLength > 256)
            {
                delete pImg; return false ;
            }

            RGBQUAD   pPal[256] = {0} ;
            for (int i=0 ; i < pTga->wPalLength ; i++)
            {
                PCL_B(&pPal[pTga->wPalFirstNdx + i]) = *pCurr++ ;
                PCL_G(&pPal[pTga->wPalFirstNdx + i]) = *pCurr++ ;
                PCL_R(&pPal[pTga->wPalFirstNdx + i]) = *pCurr++ ;
                if (pTga->byPalBits == 32)
                    PCL_A(&pPal[pTga->wPalFirstNdx + i]) = *pCurr++ ;
            }
            pImg->SetColorTable (0, 256, pPal) ;
        }

        // start decode
        for (int i=0 ; i < pImg->Height() ; i++)
        {
            BYTE     * pDest ;
            if (pStart[17] & 0x20)
                pDest = pImg->GetBits(i) ; // top to bottom
            else
                pDest = pImg->GetBits(pImg->Height() - 1 - i) ; // bottom to top

            if ((pTga->byImageType == TGA_RLEPAL) || (pTga->byImageType == TGA_RLERGB) || (pTga->byImageType == TGA_RLEMONO))
            {
                pCurr = RLE_TGA_DecodeLine (pCurr, pImg->ColorBits(), pImg->Width(), pDest) ;
            }
            else // not-compressed
            {
                int   nPitch = pImg->Width() * pImg->ColorBits() / 8 ;
                memcpy (pDest, pCurr, nPitch) ;
                pCurr += nPitch ;
            }
        }

        // image's property
        rImageProp = std::auto_ptr<FCImageProperty>(new FCImageProperty) ;
        if ((pTga->byImageType == TGA_RLEPAL) || (pTga->byImageType == TGA_RLERGB) || (pTga->byImageType == TGA_RLEMONO))
        {
            rImageProp->SetPropertyValue (IMAGE_TAG_TGA_USERLE, "1") ;
        }
        else
        {
            rImageProp->SetPropertyValue (IMAGE_TAG_TGA_USERLE, "-1") ;
        }

        // add to list
        rImageList.PCL_PushObject (pImg) ;
        return true ;
    }

    /// @param nFlag : 1(use RLE compress) / -1(not use), default(not use)
    virtual bool SaveImageFile (const char* szFileName,
                                const std::deque<const FCObjImage*>& rImageList,
                                const FCImageProperty& rImageProp)
    {
        if (rImageList.empty() || !rImageList[0]->IsValidImage())
            return false ;
        const FCObjImage   &img = *rImageList[0] ;

        // validate
        if ((img.ColorBits() != 8) && (img.ColorBits() != 16) && (img.ColorBits() != 24) && (img.ColorBits() != 32))
            return false ;

        // create image file
        FILE   * pf = fopen (szFileName, "wb") ;
        if (!pf)
            return false ;

        // Initialize TGA Header
        const char   * pTgaInfo = "PhoXo -- TGA" ;
        TGAHEAD      TgaHead ;
        memset (&TgaHead, 0, sizeof(TgaHead)) ;
        TgaHead.byID_Length     = (BYTE)strlen(pTgaInfo) ; // tga size
        TgaHead.byPalType       = ((img.ColorBits() == 8) ? 1 : 0) ;
        TgaHead.byImageType = ((img.ColorBits() == 8) ? TGA_UCPAL : TGA_UCRGB) ;
        TgaHead.wPalFirstNdx    = 0 ;
        TgaHead.wPalLength      = 256 ;
        TgaHead.byPalBits       = 24 ; // palette's bit
        TgaHead.wWidth          = img.Width() ;
        TgaHead.wHeight         = img.Height() ;
        TgaHead.byColorBits     = (BYTE)img.ColorBits() ;
        ((BYTE*)&TgaHead)[17]   = 0x20 ; // top to bottom
        fwrite (&TgaHead, 1, sizeof(TgaHead), pf) ;
        fwrite (pTgaInfo, 1, TgaHead.byID_Length, pf) ;

        // write palette
        if (img.ColorBits() == 8)
        {
            PCL_array<RGBQUAD>   pPal (256) ;
            img.GetColorTable (0, 256, pPal.get()) ;
            for (int i=0 ; i < 256 ; i++)
                fwrite (&pPal[i], 1, 3, pf) ;
        }

        // write pixels
        const int   nLineByte = img.ColorBits() * img.Width() / 8 ;

        // not compress
        for (int y=0 ; y < img.Height() ; y++)
            fwrite (img.GetBits(y), 1, nLineByte, pf) ;

        fclose (pf) ;
        return true ;
    }

    /*
     *  pOutBuffer must large enough.
     *  @param iColorBit : bpp - 8, 16, 24, 32
     *  @param iNumPixel : pixel width of image line
     *  @return return current pInBuffer position.
     */
    static BYTE* RLE_TGA_DecodeLine (const BYTE* InBuffer, int iColorBit, int iNumPixel, BYTE* OutBuffer)
    {
        iColorBit /= 8 ; // convert to bytes : 1,2,3,4
        while (iNumPixel > 0)
        {
            BYTE   byData = *InBuffer++ ; // Next Byte
            if (byData & 0x80) // Data >= 0x80
            {
                int   nNum = (byData & 0x7F) + 1 ; // number of repeat pixel
                iNumPixel -= nNum ;
                for (int i=0 ; i < nNum ; i++, OutBuffer += iColorBit)
                    FCColor::CopyPixel (OutBuffer, InBuffer, iColorBit) ;
                InBuffer += iColorBit ;
            }
            else
            {
                // copy directly
                int   n = byData + 1, // non-repeat pixel
                      nByte = n * iColorBit ; // calculate copy bytes
                iNumPixel -= n ;
                ::memcpy (OutBuffer, InBuffer, nByte) ;
                OutBuffer += nByte ;
                InBuffer += nByte ;
            }
        }
        assert (iNumPixel == 0) ;
        return const_cast<BYTE*>(InBuffer) ;
    }
};

#endif
