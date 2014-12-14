/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-7-29
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifdef PCL_3RD_LIBRARY_USE_FREEIMAGE
#ifndef __FOO_IMAGEFACTORY_FREEIMAGE__2005_07_29__H__
#define __FOO_IMAGEFACTORY_FREEIMAGE__2005_07_29__H__
#include "ImageFactory_Mini.h"
#include "ImageHandle_FreeImage.h"

//class FCImageHandleFactory ;
    class FCImageHandleFactory_FreeImage ;

//=============================================================================
/**
 *  Read/Write image via FreeImage lib (<B>Need FreeImage lib</B>).
@verbatim
              BMP      TGA      Jpg      Gif      Tif      Png      Pcx      Ico      Xpm      Psd
    Read       O        O        O        O        O        O        O        O        O        O
    Write      O        O        O        O        O        O        O        O        O        X
@endverbatim
 */
class FCImageHandleFactory_FreeImage : public FCImageHandleFactory
{
	virtual FCImageHandleBase* CreateImageHandle (IMAGE_TYPE imgType)
    {
        switch (imgType)
        {
            case IMG_BMP : return new FCImageHandle_Bmp ;
            case IMG_TGA : return new FCImageHandle_Tga ;
            case IMG_GIF : return new FCImageHandle_FreeImage ;
            case IMG_PCX : return new FCImageHandle_FreeImage ;
            case IMG_PNG : return new FCImageHandle_FreeImage ;
            case IMG_TIF : return new FCImageHandle_FreeImage ;
            case IMG_JPG : return new FCImageHandle_FreeImage ;
            case IMG_ICO : return new FCImageHandle_FreeImage ;
            case IMG_XPM : return new FCImageHandle_FreeImage ;
            case IMG_PSD : return new FCImageHandle_FreeImage ;
        }
        return 0 ;
    }
    virtual ~FCImageHandleFactory_FreeImage() {}
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
#endif // PCL_3RD_LIBRARY_USE_FREEIMAGE
