/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-6-21
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_INTERFACE_IMAGEHANDLE_FACTORY__2005_06_21__H__
#define __FOO_INTERFACE_IMAGEHANDLE_FACTORY__2005_06_21__H__
class FCImageHandleBase ; // external class

//=============================================================================
/**
 *  Interface of image handle's factory.
 *  Used by FCObjImage::SetImageHandleFactory.
 */
class FCImageHandleFactory
{
public:
    /**
     *  Get image's format by file's ext name.
     *  you can override this function to judge format by content of file.
     */
    virtual IMAGE_TYPE QueryImageFileType (const char* szFileName)
    {
        if (!szFileName)
            {assert(false); return IMG_UNKNOW;}

        std::string       strExt (FCOXOHelper::GetFileExt(szFileName)) ;
        PCL_array<char>   szConvert (new char[strExt.length() + 8]) ;
        memset (szConvert.get(), 0, strExt.length() + 8) ;
        memcpy (szConvert.get(), strExt.c_str(), strExt.length()) ;

        // convert to lowercase
        for (int i=0 ; i < (int)strExt.length() ; i++)
            szConvert[i] = tolower(szConvert[i]) ;

        PCL_TT_Convertor<IMAGE_TYPE, std::string>   aTab ;
        aTab.AddElement (IMG_JPG, "jpg") ;
        aTab.AddElement (IMG_JPG, "jpeg") ;
        aTab.AddElement (IMG_GIF, "gif") ;
        aTab.AddElement (IMG_PNG, "png") ;
        aTab.AddElement (IMG_BMP, "bmp") ;
        aTab.AddElement (IMG_PCX, "pcx") ;
        aTab.AddElement (IMG_TIF, "tif") ;
        aTab.AddElement (IMG_TIF, "tiff") ;
        aTab.AddElement (IMG_TGA, "tga") ;
        aTab.AddElement (IMG_ICO, "ico") ;
        aTab.AddElement (IMG_PSD, "psd") ;
        aTab.AddElement (IMG_XPM, "xpm") ;
        aTab.AddElement (IMG_PHOXO, "oxo") ;
        return aTab.Second_to_First (szConvert.get(), IMG_UNKNOW) ;
    }

    /**
     *  Create image handle by image type.
     *  Derived class must use <B>new</B> to create handle and return it, user must use <B>delete</B> to delete returned handler.
     *  @return you are high recommended to use std::auto_ptr wrap returned pointer.
@code
    IMAGE_TYPE     imgType = FCObjImage::GetImageHandleFactory()->QueryImageFileType(szFileName) ;
    std::auto_ptr<FCImageHandleBase>  pHandler (FCObjImage::GetImageHandleFactory()->CreateImageHandle(imgType)) ;
@endcode
     */
    virtual FCImageHandleBase* CreateImageHandle (IMAGE_TYPE imgType) =0 ;
    virtual ~FCImageHandleFactory() {}
};

#endif
