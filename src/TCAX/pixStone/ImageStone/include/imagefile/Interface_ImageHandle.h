/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-4-2
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_INTERFACE_IMAGEHANDLE__2005_04_02__H__
#define __FOO_INTERFACE_IMAGEHANDLE__2005_04_02__H__

#include "../ImageProperty.h"
class FCObjImage ; // external class

//=============================================================================
/**
 *  Interface of image handle.
 *  Used by FCObjImage/FCObjMultiFrame object to implement Load/Save image.
 */
class FCImageHandleBase
{
public:
    /**
     *  @name Load image.
     */
    //@{
    /**
     *  Load image from file.
     *  This function default load file into memory and then call FCImageHandleBase::LoadImageMemory, you can override it to load directly.
     *  @param rImageList : You must <B>new</B> FCObjImage and add them into rImageList, after add you can't delete them later.
     *  @param rImageProp : optional, you can <B>new</B> and add property.
     */
    virtual bool LoadImageFile (const char* szFileName,
                                PCL_Interface_Composite<FCObjImage>& rImageList,
                                std::auto_ptr<FCImageProperty>& rImageProp)
    {
        FILE   * pf = fopen (szFileName, "rb") ;
        if (!pf)
            return false ;

        // get file length
        fseek (pf, 0, SEEK_END) ;
        const int   nFileSize = (int)ftell(pf) ;
        if (nFileSize <= 0)
        {
            fclose (pf) ;
            return false ;
        }

        // read file into memory
        PCL_array<BYTE>   pStart (nFileSize) ;
        fseek (pf, 0, SEEK_SET) ;
        fread (pStart.get(), 1, nFileSize, pf) ;
        fclose (pf) ;
        return LoadImageMemory (pStart.get(), nFileSize, rImageList, rImageProp) ;
    }

    /**
     *  Load image from memory.
     *  This function default return false, derived class should implement it.
     *  @param rImageList : You must <B>new</B> FCObjImage and add them into rImageList, after add you can't delete them later.
     *  @param rImageProp : optional, you can <B>new</B> and add property.
     */
    virtual bool LoadImageMemory (const BYTE* pStart, int nMemSize,
                                  PCL_Interface_Composite<FCObjImage>& rImageList,
                                  std::auto_ptr<FCImageProperty>& rImageProp)
    {
        return false ;
    }
    //@}

    /**
     *  @name Save image.
     */
    //@{
    /**
     *  Save image to file.
     *  This function default return false, derived class should implement it.
     */
    virtual bool SaveImageFile (const char* szFileName,
                                const std::deque<const FCObjImage*>& rImageList,
                                const FCImageProperty& rImageProp)
    {
        return false ;
    }
    //@}

    virtual ~FCImageHandleBase() {}

protected:
    /// Jpeg's quality on saving jpeg (default is 80).
    static int JpegSaveQuality (const FCImageProperty& rImageProp)
    {
        std::string   s = rImageProp.QueryPropertyValue(IMAGE_TAG_SAVE_FLAG) ;
        if (s.empty())
            s = rImageProp.QueryPropertyValue(IMAGE_TAG_JPEG_QUALITY) ;

        int     nFlag = 80 ;
        if (!s.empty() && (s != "-1"))
            FCOXOHelper::A2X (s, nFlag) ;

        return FClamp(nFlag, 1, 100) ;
    }

    /// Gif's transparent color index in palette.
    static int GifSaveTransparentIndex (const FCImageProperty& rImageProp)
    {
        std::string   s = rImageProp.QueryPropertyValue(IMAGE_TAG_SAVE_FLAG) ;
        if (s.empty())
            s = rImageProp.QueryPropertyValue(IMAGE_TAG_GIF_TRANSPARENT_INDEX) ;

        int     nFlag = -1 ;
        if (!s.empty() && (s != "-1"))
            FCOXOHelper::A2X (s, nFlag) ;

        return nFlag ;
    }
};

#endif
