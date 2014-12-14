/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-7-19
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __PCL_ARRAY__2004_07_19__H__
#define __PCL_ARRAY__2004_07_19__H__
#include <assert.h>

//=============================================================================
/**
 *  Auto delete array.
@code
// attach user alloc memory
PCL_array<POINT>   arrUser (new POINT[10]) ;
arrUser[0].x = 0 ;

// alloc memory self
PCL_array<POINT>   arrOur (10) ;
arrOur[0].x = 0 ;
@endcode
 */
template<class T>
class PCL_array
{
public:
    /**
     *  @name Constructor.
     */
    //@{
    /**
     *  Attch array (no DEBUG-time bound check).
     *  After attached, you can't delete pArray later.
     *  @param pArray : must use <B>new []</B> to create
     */
    PCL_array (void* pArray)
    {
        m_pArray = (T*)pArray; assert(pArray);
        m_nNumberT = -1 ;
    }
    /// Alloc nNumberT T array (with DEBUG-time bound check).
    PCL_array (int nNumberT)
    {
        if (nNumberT > 0)
        {
            m_pArray = new T[nNumberT] ;
            m_nNumberT = nNumberT ;
        }
        else
        {
            m_pArray = 0 ;
            m_nNumberT = -1 ; assert(false);
        }
    }
    virtual ~PCL_array()
    {
        if (m_pArray)
            delete[] m_pArray ;
    }
    //@}

    /// Get element.
    T& operator[](int n) const
    {
        assert ((m_nNumberT == -1) ? true : (n < m_nNumberT)) ;
        return m_pArray[n] ;
    }

    /// Get array base start pointer.
    T* get() const {return m_pArray;}

    // operator T*() const {return m_pArray;} // don't release, otherwise you can use delete[] to delete this object.

    /// Get bytes of array.
    int GetArrayBytes() const
    {
        if (m_nNumberT == -1)
        {
            // attached, we don't know array's size
            assert(false) ;
            return 0 ;
        }
        return (sizeof(T) * m_nNumberT) ;
    }

private:
    T     * m_pArray ;
    int   m_nNumberT ;
};

#endif
