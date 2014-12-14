/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-2-21
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_INTERFACE_PIXEL_PROCESSOR__2004_02_21__H__
#define __FOO_INTERFACE_PIXEL_PROCESSOR__2004_02_21__H__
#include "../StdDefine.h"
class FCObjImage ; // external class

class FCInterface_PixelProcess ;

//=============================================================================
/**
 *  Pixel processor interface.
 */
class FCInterface_PixelProcess
{
public:
    virtual ~FCInterface_PixelProcess() {}

    /// How to process the image.
    enum PROCESS_TYPE
    {
        /// process whole image.
        PROCESS_TYPE_WHOLE,
        /// process every pixel step.
        PROCESS_TYPE_PIXEL,
    };

    /// Whether the image can be disposed by this processor.
    virtual bool ValidateColorBits (const FCObjImage* pImg) =0 ;

    /// Query process type, default to return PROCESS_TYPE_PIXEL.
    virtual PROCESS_TYPE QueryProcessType() {return PROCESS_TYPE_PIXEL;}

    /// Before process.
    virtual void OnEnterProcess (FCObjImage* pImg) {}
    /// Process (x,y) pixel when QueryProcessType return PROCESS_TYPE_PIXEL.
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel) {}
    /// Process whole image when QueryProcessType return PROCESS_TYPE_WHOLE.
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress) {}
    /// After process.
    virtual void OnLeaveProcess (FCObjImage* pImg) {}
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
