/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-8-31
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_OBJECT_MULTI_IMAGE__2004_08_31__H__
#define __FOO_OBJECT_MULTI_IMAGE__2004_08_31__H__
#include "ObjImage.h"

//=============================================================================
/**
 *  Image container.
 */
class FCObjMultiFrame : public FCObject
{
    PCL_Interface_Composite<FCObjImage>     m_ImgList ;
public:
    /**
     *  Add frame to object.
     *  pImg must use new to create, after the pImg be added, you can't delete it later.
     */
    void AddFrame (FCObjImage* pImg)
    {
        m_ImgList.PCL_PushObject(pImg) ;
    }

    /// Get image count.
    int GetFrameCount() const {return m_ImgList.PCL_GetObjectCount();}

    /// Get zero-based index image.
    FCObjImage* GetFrame (int nFrame) const {return m_ImgList.PCL_GetObject(nFrame);}

    /// Delete all frame.
    void DeleteAllFrame() {m_ImgList.PCL_DeleteAllObjects();}

    /**
     *  @name Read/Write image file.
     */
    //@{
    /**
     *  Load image file.
     *  this function determine image format by file's ext name.
     *  @param pProperty : optional, you can pass NULL if you don't care.
     */
    bool Load (const char* szFileName, FCImageProperty* pProperty = 0)
    {
        IMAGE_TYPE     imgType = FCObjImage::GetImageHandleFactory()->QueryImageFileType(szFileName) ;
        std::auto_ptr<FCImageHandleBase>  pHandler (FCObjImage::GetImageHandleFactory()->CreateImageHandle(imgType)) ;
        if (!pHandler.get())
            return false ;

        PCL_Interface_Composite<FCObjImage>   listImage ;
        std::auto_ptr<FCImageProperty>        pImgProp ;
        bool     bRet = pHandler->LoadImageFile (szFileName, listImage, pImgProp) ;
        if (bRet)
        {
            // get frames
            std::deque<FCObjImage*>   ls ;
            listImage.PCL_ThrowOwnership (ls) ;
            for (size_t i=0 ; i < ls.size() ; i++)
                AddFrame (ls[i]) ;

            // get property
            if (pImgProp.get() && pProperty)
                *pProperty = *pImgProp ;
        }
        assert (bRet) ;
        return bRet ;
    }

    /**
     *  Load image from memory.
     *  @param pProperty : optional, you can pass NULL if you don't care.
     */
    bool Load (BYTE* pStart, int nMemSize, IMAGE_TYPE imgType, FCImageProperty* pProperty = 0)
    {
        std::auto_ptr<FCImageHandleBase>  pHandler (FCObjImage::GetImageHandleFactory()->CreateImageHandle(imgType)) ;
        if (!pHandler.get())
            return false ;

        PCL_Interface_Composite<FCObjImage>   listImage ;
        std::auto_ptr<FCImageProperty>        pImgProp ;
        bool     bRet = pHandler->LoadImageMemory (pStart, nMemSize, listImage, pImgProp) ;
        if (bRet)
        {
            // get frames
            std::deque<FCObjImage*>   ls ;
            listImage.PCL_ThrowOwnership (ls) ;
            for (size_t i=0 ; i < ls.size() ; i++)
                AddFrame (ls[i]) ;
            
            // get property
            if (pImgProp.get() && pProperty)
                *pProperty = *pImgProp ;
        }
        assert (bRet) ;
        return bRet ;
    }

    /**
     *  Save image to file.
     *  this function determine image format by file's ext name.
     */
    bool Save (const char* szFileName, const FCImageProperty& rProp) const
    {
        if (!GetFrameCount() || !szFileName)
            return false ;

        IMAGE_TYPE     imgType = FCObjImage::GetImageHandleFactory()->QueryImageFileType(szFileName) ;
        std::auto_ptr<FCImageHandleBase>  pHandler (FCObjImage::GetImageHandleFactory()->CreateImageHandle(imgType)) ;
        if (!pHandler.get())
            return false ;

        // save list
        std::deque<const FCObjImage*>   saveList ;
        for (int i=0 ; i < GetFrameCount() ; i++)
            saveList.push_back (GetFrame(i)) ;

        return pHandler->SaveImageFile (szFileName, saveList, rProp) ;
    }
    //@}
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
