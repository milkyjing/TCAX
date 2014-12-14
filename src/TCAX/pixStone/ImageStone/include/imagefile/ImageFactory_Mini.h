/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-7-29
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_IMAGEFACTORY_MINI__2005_07_29__H__
#define __FOO_IMAGEFACTORY_MINI__2005_07_29__H__
#include "Interface_ImageHandleFactory.h"
#include "ImageHandle_Bmp.h"
#include "ImageHandle_Tga.h"

//class FCImageHandleFactory ;
    class FCImageHandleFactory_Mini ;

//=============================================================================
/**
 *  Read/Write image depend on nothing (<B>All OS</B>).
@verbatim
              BMP      TGA
    Read       O        O
    Write      O        O
@endverbatim
 */
class FCImageHandleFactory_Mini : public FCImageHandleFactory
{
protected:
    virtual FCImageHandleBase* CreateImageHandle (IMAGE_TYPE imgType)
    {
        switch (imgType)
        {
            case IMG_BMP : return new FCImageHandle_Bmp ;
            case IMG_TGA : return new FCImageHandle_Tga ;
        }
        return 0 ;
    }
    virtual ~FCImageHandleFactory_Mini() {}
};

#endif
