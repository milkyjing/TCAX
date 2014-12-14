/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __IMAGESTONE_HEADER__9711__H__
#define __IMAGESTONE_HEADER__9711__H__

//=============================================================================

#include "include/PCL_interface_lazyobj.h"
#include "include/FTimeCount.h"

#include "include/FColor.h"
#include "include/FHistogram.h"

#include "include/command/Command.h"
#include "include/ObjBase.h"
#include "include/ObjProgress.h"
#include "include/ObjImage.h"
#include "include/ObjMultiFrame.h"
#include "include/pixelprocessor/PixelProcessorBase.h"
#include "include/ObjSelect.h"
#include "include/ObjLayer.h"
#include "include/ObjCanvas.h"

#include "include/FWin32.h"

#include "include/imagefile/ImageFactory_Mini.h"
#include "include/imagefile/ImageFactory_Gdiplus.h"
#include "include/imagefile/ImageFactory_FreeImage.h"

//=============================================================================
inline FCImageHandleFactory* FCObjImage::__ManageImageHandleFactory (bool bGet, FCImageHandleFactory* pFactory)
{
#ifdef WIN32
    static std::auto_ptr<FCImageHandleFactory>   s_pFactory (new FCImageHandleFactory_Gdiplus) ;
#else
    static std::auto_ptr<FCImageHandleFactory>   s_pFactory (new FCImageHandleFactory_Mini) ;
#endif

    if (!bGet && pFactory)
        s_pFactory = std::auto_ptr<FCImageHandleFactory>(pFactory) ;
    return s_pFactory.get() ;
}
//-----------------------------------------------------------------------------
// stretch (>=24 bit)
inline void FCObjImage::Stretch_Smooth (int nNewWidth, int nNewHeight, FCObjProgress * progress)
{
#ifdef WIN32
    FCWin32::GDIPlus_ResizeImage (*this, nNewWidth, nNewHeight) ;
#else
    // parameter check
    if (!IsValidImage() || (nNewWidth <= 0) || (nNewHeight <= 0) || (ColorBits() < 24))
    {
        assert(false) ; return ;
    }
    if ((nNewWidth == Width()) && (nNewHeight == Height()))
        return ;

    // in order to bilinear, the source image's W/H must larger than 2
    if ((Width() == 1) || (Height() == 1))
    {
        ExpandFrame (true, 0, 0, (Width()==1) ? 1 : 0, (Height()==1) ? 1 : 0) ;
    }

    // first backup image
    const FCObjImage     imgOld(*this) ;
    if (!Create (nNewWidth, nNewHeight, imgOld.ColorBits()))
    {
        assert(false) ; return ;
    }

    // initialize index table, to accelerate
    PCL_array<int>   pTabX (Width()),
                     pXMod (Width()) ;
    for (int i=0 ; i < Width() ; i++)
    {
        pTabX[i] = i * imgOld.Width() / Width() ;
        pXMod[i] = (i * imgOld.Width()) % Width() ;

        // approximate to last col
        if (pTabX[i] >= imgOld.Width()-1)
        {
            pTabX[i] = imgOld.Width() - 2 ;
            pXMod[i] = Width() - 1 ;
        }
    }

    // stretch pixel
    const int   nSpan = ColorBits() / 8,
                nPitch = imgOld.GetPitch() ;
    if (progress)
        progress->ResetProgress() ; // reset to 0
    for (int y=0 ; y < Height() ; y++)
    {
        int     nSrcY = y * imgOld.Height() / Height(),
                nYMod = (y * imgOld.Height()) % Height() ;
        if (nSrcY >= imgOld.Height()-1) // approximate to last row
        {
            nSrcY = imgOld.Height() - 2 ;
            nYMod = Height() - 1 ;
        }

        const double   un_y = nYMod / (double)Height() ;
        BYTE           * pWrite = GetBits(y) ;
        for (int x=0 ; x < Width() ; x++, pWrite += nSpan)
        {
            // 计算原图对应点
            const int    nSrcX = pTabX[x],
                         nXMod = pXMod[x] ;

            const BYTE   * pOldPix = imgOld.GetBits (nSrcX, nSrcY) ;
            if ((nXMod == 0) && (nYMod == 0))
            {
                FCColor::CopyPixel (pWrite, pOldPix, nSpan) ;
            }
            else
            {
                const BYTE  * pcrPixel[4] =
                {
                    pOldPix, pOldPix + nSpan,
                    pOldPix - nPitch, pOldPix - nPitch + nSpan
                } ;

                RGBQUAD   crRet = FCColor::Get_Bilinear_Pixel (nXMod/(double)Width(), un_y, ColorBits() == 32, pcrPixel) ;
                FCColor::CopyPixel (pWrite, &crRet, nSpan) ;
            }
        }
        if (progress)
            progress->SetProgress (y * 100 / Height()) ;
    }
#endif
}

#endif
