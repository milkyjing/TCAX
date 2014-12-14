/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2006-9-24
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_IMAGE_PROPERTY__2006_09_24__H__
#define __FOO_IMAGE_PROPERTY__2006_09_24__H__
#include "StdDefine.h"

enum FIMAGE_TAG
{
    // it's only for compatible
    IMAGE_TAG_SAVE_FLAG = 0x01F4,

	// tga format image info
    // int value : 1 means use RLE / -1 means not use
    IMAGE_TAG_TGA_USERLE = 0x03E8,

    // time interval between two frame
    IMAGE_TAG_GIF_FRAMEDELAY = 0x07D0,
    // transparent color index in palette
    IMAGE_TAG_GIF_TRANSPARENT_INDEX = 0x07D2,

    // jpeg format image info
    IMAGE_TAG_JPEG_QUALITY = 0x5010,

    // EXIF info
    IMAGE_TAG_EquipMake = 0x010F,
    IMAGE_TAG_EquipModel = 0x0110,
    IMAGE_TAG_EXIF_DTOrig = 0x9003,
    IMAGE_TAG_EXIF_ExposureTime = 0x829A,
    IMAGE_TAG_EXIF_FNumber = 0x829D,
    IMAGE_TAG_EXIF_FocalLength = 0x920A,
    IMAGE_TAG_EXIF_ISOSpeed = 0x8827,
    IMAGE_TAG_EXIF_ExposureBias = 0x9204,
    IMAGE_TAG_EXIF_MaxAperture = 0x9205,
    IMAGE_TAG_EXIF_Flash = 0x9209,
    IMAGE_TAG_EXIF_MeteringMode = 0x9207,
    IMAGE_TAG_EXIF_ExposureProg = 0x8822,
};
//=============================================================================
/**
 *  Property of image.
 */
class FCImageProperty : public PCL_TT_Convertor<FIMAGE_TAG, std::string>
{
public:
    /**
     *  @name Put/Get multi frame time interval.
     */
    //@{
    /// Get multi frame time interval millisecond.
    void GetFrameDelay (std::deque<int>& listTime) const
    {
        listTime.clear() ;
        std::string   s = QueryPropertyValue(IMAGE_TAG_GIF_FRAMEDELAY) ;
        for (size_t i=0 ; i < s.length() ; i+=16)
        {
            int   n ;
            FCOXOHelper::A2X (s.substr(i,16), n) ;
            listTime.push_back (n) ;
        }
    }

    /// Put multi frame time interval into property package.
    void PutFrameDelay (int nTime)
    {
        std::string   s = QueryPropertyValue(IMAGE_TAG_GIF_FRAMEDELAY) ;
        s += FCOXOHelper::X2A (nTime, 16) ;
        SetPropertyValue (IMAGE_TAG_GIF_FRAMEDELAY, s.c_str()) ;
    }
    //@}

    /// Is exist nTga property.
    bool IsExistProperty (FIMAGE_TAG nTag) const
    {
        return (GetIndexT1(nTag) == -1) ;
    }

    /// Get property value, return "" if property not exist.
    std::string QueryPropertyValue (FIMAGE_TAG nTag) const
    {
        return First_to_Second(nTag, "") ;
    }

    /// Set property value.
    void SetPropertyValue (FIMAGE_TAG nTag, const char* strValue)
    {
        int   n = GetIndexT1(nTag) ;
        if (n == -1)
        {
            AddElement (nTag, strValue) ;
        }
        else
        {
            GetT2(n) = strValue ;
        }
    }
};

#endif
