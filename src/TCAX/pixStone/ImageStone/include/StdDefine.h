#ifndef __IMAGESTONE_STANDARD_DEFINE_HEADER_H__
#define __IMAGESTONE_STANDARD_DEFINE_HEADER_H__

//=============================================================================
// compiler interrelated
//=============================================================================
#ifdef _MSC_VER
    #pragma once
    #pragma warning (disable : 4786) // identifier was truncated to '255' characters in the browser information
#endif

//=============================================================================
// basic function
//=============================================================================
template<class T> inline const T& FMax (const T& _X, const T& _Y) {return (_X < _Y ? _Y : _X);}
template<class T> inline const T& FMin (const T& _X, const T& _Y) {return (_Y < _X ? _Y : _X);}
template<class T> inline void FSwap (T& t1, T& t2) { const T tmp=t1 ; t1=t2 ; t2=tmp ;}
template<class T> inline T FSquare (const T& t) {return t*t ;}
// bound in [tLow, tHigh]
template<class T> inline T FClamp (const T& t, const T& tLow, const T& tHigh) {return FMax (tLow, FMin (tHigh, t)) ;}
inline int FClamp0255 (int n) {return FClamp (n, 0, 0xFF) ;}
// round double to int
inline int FRound (const double& x)
{
    if (x > 0.0)
        return (int)(x + 0.5) ;
    else
        return (int)(x - 0.5) ;
}
#define   LIB_PI    3.1415926535897932384626433832795
#define   LIB_2PI   (2.0*LIB_PI)
/// angle ==> radian
inline double AngleToRadian (int nAngle) {return LIB_PI * nAngle / 180.0;}
/// radian ==> angle
inline int RadianToAngle (double fRadian) {return (int)(180.0 * fRadian / LIB_PI);}

//=============================================================================
// include header
//=============================================================================
#ifdef WIN32
	#include <windows.h>
    #include <comdef.h>
    #include <TCHAR.H>

    // define for GDI+
    #if _MSC_VER <= 1200
        // for VC6
        #define ULONG_PTR ULONG
    #endif

    #include <GdiPlus.h>
    #pragma comment (lib, "GdiPlus.lib")

#else
    #include "FLib_Macro.h"
#endif // WIN32

#ifdef PCL_3RD_LIBRARY_USE_FREEIMAGE
    #include "../lib/FreeImage/Dist/FreeImage.h"
#endif // PCL_3RD_LIBRARY_USE_FREEIMAGE

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <memory.h>
#include <string.h>
#include <string>
#include <sstream>
#include "PCL_array.h"
#include "PCL_TT_Convertor.h"
#include "PCL_interface_composite.h"
#include "PCL_interface_lazyobj.h"
#include "PCL_interface_undo.h"
#include "FTimeCount.h"
#include "ObjProgress.h"

//=============================================================================
// avoid memory big/little endian
//=============================================================================
#define   PCL_R(p)   (((RGBQUAD*)(p))->rgbRed)
#define   PCL_G(p)   (((RGBQUAD*)(p))->rgbGreen)
#define   PCL_B(p)   (((RGBQUAD*)(p))->rgbBlue)
#define   PCL_A(p)   (((RGBQUAD*)(p))->rgbReserved)
template<class T> inline RGBQUAD PCL_RGBA (T r, T g, T b, T a=0xFF)
{
    RGBQUAD   cr ;
    PCL_R(&cr)=r ; PCL_G(&cr)=g ; PCL_B(&cr)=b ; PCL_A(&cr)=a ;
    return cr ;
}

//=============================================================================
inline long RECTWIDTH(const RECT& rc) {return rc.right - rc.left;}
inline long RECTHEIGHT(const RECT& rc) {return rc.bottom - rc.top;}
inline bool IsRectInRect (RECT rcOut, RECT rcIn)
{
	return (rcIn.left >= rcOut.left) && (rcIn.top >= rcOut.top) && (rcIn.right <= rcOut.right) && (rcIn.bottom <= rcOut.bottom) ;
}
// hypotenuse, c2 = a2 + b2
inline double FHypot (const double& x, const double& y)
{
    return sqrt (x*x + y*y) ;
}


//=============================================================================
// 平面8个方向
//=============================================================================
enum DIRECT_SYS
{
	DIRECT_TOP_LEFT,
	DIRECT_TOP,
	DIRECT_TOP_RIGHT,
	DIRECT_LEFT,
	DIRECT_RIGHT,
	DIRECT_BOTTOM_LEFT,
	DIRECT_BOTTOM,
	DIRECT_BOTTOM_RIGHT,
};
//=============================================================================
// 渐变过渡类型
//=============================================================================
enum REPEAT_MODE
{
	REPEAT_NONE = 0,
	REPEAT_SAWTOOTH = 1, // 锯齿波重复
	REPEAT_TRIANGULAR = 2, // 三角波重复
};
//=============================================================================
// 插值模式
//=============================================================================
enum INTERPOLATION_TYPE
{
    INTERPOLATION_NONE,
    INTERPOLATION_BILINEAR,
};
//=============================================================================
// 逻辑操作
//=============================================================================
enum LOGICAL_OP
{
    LOGI_AND,   // c = a & b
    LOGI_OR,    // c = a | b
    LOGI_XOR,   // c = a ^ b
    LOGI_ADD,   // c = a + b
    LOGI_SUB,   // c = a - b
    LOGI_MUL,   // c = a * b
    LOGI_DIV,   // c = a / b
    LOGI_LOG,   // c = log(a)
    LOGI_EXP,   // c = exp(a)
    LOGI_SQRT,  // c = sqrt(a)
    LOGI_TRIG,  // c = sin/cos/tan(a)
    LOGI_INVERT,// c = (2B - 1) - a

    LOGI_SEL_ADD, // for selection
    LOGI_SEL_SUB, // for selection
};
//=============================================================================
// image format
//=============================================================================
enum IMAGE_TYPE
{
    IMG_UNKNOW,
    IMG_BMP,
    IMG_PCX,
    IMG_JPG,
    IMG_GIF,
    IMG_TGA,
    IMG_TIF,
    IMG_PNG,
    IMG_PSD,
    IMG_ICO,
    IMG_XPM,
    IMG_PHOXO,
    IMG_CUSTOM,
};
//=============================================================================
// R-G-B-A channel
//=============================================================================
enum IMAGE_CHANNEL
{
    CHANNEL_RED   = 1 << 0,// 0x01,
    CHANNEL_GREEN = 1 << 1,// 0x02,
    CHANNEL_BLUE  = 1 << 2,// 0x04,
    CHANNEL_ALPHA = 1 << 3,// 0x08,
    CHANNEL_RGB   = CHANNEL_RED|CHANNEL_GREEN|CHANNEL_BLUE,// 0x07,
    CHANNEL_RGBA  = CHANNEL_RGB|CHANNEL_ALPHA,// 0x0F,
    CHANNEL_GRAY  = 1 << 4,// 0x10
};
//=============================================================================
// 线样式
//=============================================================================
enum LINE_STYLE
{
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DASH = 1,
    LINE_STYLE_DOT = 2,
};
//=============================================================================
// 色调区域（这三个值的顺序一定不能变）
//=============================================================================
enum TONE_REGION
{
    /// shadow region of image.
    TONE_SHADOWS = 0,
    /// midtone region of image.
    TONE_MIDTONES = 1,
    /// highlight region of image.
    TONE_HIGHLIGHTS = 2,
};
//=============================================================================
// 阴影数据结构
//=============================================================================
struct SHADOWDATA
{
    int       nSmooth ; // 模糊度
    RGBQUAD   crShadow ; // 颜色/硬度
    int       nAlpha ; // 透明度
    int       nOffsetX ; // X偏移
    int       nOffsetY ; // Y偏移

    SHADOWDATA()
    {
        nOffsetX = nOffsetY = 5 ;
        crShadow = PCL_RGBA(75,75,75) ;
        nAlpha = 75 ;
        nSmooth = 5 ;
    }
};

//=============================================================================

#endif
