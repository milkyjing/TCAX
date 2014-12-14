/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-6-26
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_PHOXO_HELPER__2004_06_26__H__
#define __FOO_PHOXO_HELPER__2004_06_26__H__
#include "StdDefine.h"

//=============================================================================
/**
 *  Some pure C++ helper funtion.
 */
class FCOXOHelper
{
public:
    /**
     *  @name Memory helper.
     */
    //@{
    /// Allocate memory with initialized to zero.
    static BYTE* ZeroMalloc (int nBytes) ;
    /// Free memory alloc by ZeroMalloc.
    static void ZeroFree (void* pPixel) ;
    //@}

    /**
     *  @name File helper.
     */
    //@{
    /**
     *  Save buffer to file.
     *  if the destination file exist, this function will delete it.
     */
    static bool SaveBufferToFile (const char* szFilename, const void* pBuffer, int nLength)
    {
        FILE   * pf = fopen (szFilename, "wb") ;
        if (!pf)
            {assert(false); return false;}

        fwrite (pBuffer, 1, nLength, pf) ;
        fclose (pf) ;
        return true ;
    }

    /// Load file to memory (you must use <B>delete[]</B> to free returned pBuffer).
    static void LoadFileToBuffer (const char* szFilename, char*& pBuffer, int& nLength)
    {
        pBuffer=0; nLength=0;

        FILE   * pf = fopen (szFilename, "rb") ;
        if (!pf)
            {assert(false); return;}

        // get file length
        fseek (pf, 0, SEEK_END) ;
        nLength = (int)ftell(pf) ; assert(nLength > 0);
        if (nLength > 0)
        {
            // read file into memory
            pBuffer = new char[nLength + 8] ;
            memset (&pBuffer[nLength], 0, 8) ;
            fseek (pf, 0, SEEK_SET) ;
            fread (pBuffer, 1, nLength, pf) ;
        }
        fclose (pf) ;
    }
    //@}

    /**
     *  @name String convert helper.
     */
    //@{
    /// int/long/float/double ==> ASCII
    template<class T> static std::string X2A (const T& tNumber, int nWidth=0)
    {
        std::ostringstream     sOutStream ;
        sOutStream.width (nWidth) ;
        sOutStream.fill ('0') ;
        sOutStream << tNumber ;
        return sOutStream.str() ;
    }
    /// ASCII ==> int/long/float/double
    template<class T> static void A2X (const std::string& strNumber, T& tResult)
    {
        tResult = (T)0 ;
        std::stringstream     sTranslation ;
        sTranslation << strNumber ;
        sTranslation >> tResult ;
    }

    /// Get filename's ext name.
    static std::string GetFileExt (const char* pFile)
    {
        if (!pFile)
            {assert(false); return "";}

        std::string   strFile(pFile), strOut ;
        size_t        nPos = strFile.find_last_of (".") ;
        if (nPos != std::string::npos)
            strOut = strFile.substr (nPos + 1) ;
        return strOut ;
    }

    /// Output is from small to big.
    static void BubbleSort (int pArray[], int iNumElement)
    {
        if (!pArray)
            {assert(false); return;}

        for (int i = iNumElement-1 ; i > 0 ; i--)
        {
            bool     bFlag = true ;
            for (int j = 0 ; j < i ; j++)
                if (pArray[j] > pArray[j + 1])
                {
                    FSwap (pArray[j], pArray[j+1]) ;
                    bFlag = false ;
                }
            if (bFlag)
                break ;
        }
    }
};

//=============================================================================
// inline Implement
//=============================================================================
inline BYTE* FCOXOHelper::ZeroMalloc (int nBytes)
{
#ifdef WIN32

    // BoundChecker can's check the memory alloc by <VirtualAlloc>
    #ifdef _DEBUG
        BYTE   * pByte = (BYTE*)malloc (nBytes) ;
        memset (pByte, 0, nBytes) ;
        return pByte ;
    #else
        return (BYTE*)VirtualAlloc (NULL, nBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE) ;
    #endif

#else

        BYTE   * pByte = (BYTE*)malloc (nBytes) ;
        memset (pByte, 0, nBytes) ;
        return pByte ;

#endif
}
//-----------------------------------------------------------------------------
inline void FCOXOHelper::ZeroFree (void* pPixel)
{
    if (!pPixel)
        return ;

#ifdef WIN32

    #ifdef _DEBUG
        free (pPixel) ;
    #else
        ::VirtualFree (pPixel, 0, MEM_RELEASE) ;
    #endif

#else

        free (pPixel) ;

#endif
}
//-----------------------------------------------------------------------------

#endif
