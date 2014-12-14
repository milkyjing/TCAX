/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-7-29
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifdef WIN32
#ifndef __FOO_IMAGEFACTORY_GDIPLUS__2005_07_29__H__
#define __FOO_IMAGEFACTORY_GDIPLUS__2005_07_29__H__
#include "ImageFactory_Mini.h"
#include "ImageHandle_Gdiplus.h"

//class FCImageHandleFactory ;
    class FCImageHandleFactory_Gdiplus ;

//=============================================================================
/**
 *  Read/Write image via GDI+ (<B>Need Gdiplus.dll</B>).
@verbatim
              BMP      TGA      Jpg      Gif      Tif      Png      Ico
    Read       O        O        O        O        O        O        O
    Write      O        O        O        O        O        O        X
@endverbatim
 */
class FCImageHandleFactory_Gdiplus : public FCImageHandleFactory,
                                     public FCAutoInitGDIPlus
{
	virtual FCImageHandleBase* CreateImageHandle (IMAGE_TYPE imgType)
    {
        switch (imgType)
        {
            case IMG_BMP : return new FCImageHandle_Bmp ;
            case IMG_TGA : return new FCImageHandle_Tga ;
            case IMG_JPG : return new FCImageHandle_Gdiplus ;
            case IMG_GIF : return new FCImageHandle_Gdiplus ;
            case IMG_TIF : return new FCImageHandle_Gdiplus ;
            case IMG_PNG : return new FCImageHandle_Gdiplus ;
            case IMG_ICO : return new FCImageHandle_Gdiplus ;
        }
        return 0 ;
    }
    virtual ~FCImageHandleFactory_Gdiplus() {}
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
#endif // WIN32
