/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-3-9
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __PCL_TT_CONVERTOR__2005_03_09__H__
#define __PCL_TT_CONVERTOR__2005_03_09__H__
#ifdef _MSC_VER
    #pragma warning (disable : 4786) // identifier was truncated to '255' characters in the browser information
#endif
#include <deque>
#include <assert.h>

//=============================================================================
/**
 *  Map table.
 */
template<class T1, class T2>
class PCL_TT_Convertor
{
public:
    /// T1 ==> T2
    T2 First_to_Second (const T1& t1, const T2& t2Default) const
    {
        int   i = GetIndexT1(t1) ;
        return (i==-1) ? t2Default : m_t2Tab[i] ;
    }

    /// T2 ==> T1
    T1 Second_to_First (const T2& t2, const T1& t1Default) const
    {
        int   i = GetIndexT2(t2) ;
        return (i==-1) ? t1Default : m_t1Tab[i] ;
    }

    /// Get element count.
    int GetElementCount() const
    {
        if (m_t1Tab.size() == m_t2Tab.size())
            return (int)m_t1Tab.size();
        assert(false) ;
        return 0 ;
    }

    /// Get 0-based index in array by t1, return -1 if not found.
    int GetIndexT1 (const T1& t1) const
    {
        for (size_t i=0 ; i < m_t1Tab.size() ; i++)
        {
            if (m_t1Tab[i] == t1)
                return (int)i ;
        }
        return -1 ;
    }
    /// Get 0-based index in array by t2, return -1 if not found.
    int GetIndexT2 (const T2& t2) const
    {
        for (size_t i=0 ; i < m_t2Tab.size() ; i++)
        {
            if (m_t2Tab[i] == t2)
                return (int)i ;
        }
        return -1 ;
    }

    /// Get first element of n index (<B>no bound check</B>)).
    const T1& GetT1 (int n) const {assert(IsValidIndex(n)); return m_t1Tab[n];}
          T1& GetT1 (int n)       {assert(IsValidIndex(n)); return m_t1Tab[n];}
    /// Get second element of n index (<B>no bound check</B>)).
    const T2& GetT2 (int n) const {assert(IsValidIndex(n)); return m_t2Tab[n];}
          T2& GetT2 (int n)       {assert(IsValidIndex(n)); return m_t2Tab[n];}

    /// Add a new element into table.
    void AddElement (const T1& t1, const T2& t2)
    {
        m_t1Tab.push_back (t1) ;
        m_t2Tab.push_back (t2) ;
    }

    /// Remove an element from table by 0-based index.
    void RemoveElement (int nIndex)
    {
        if (IsValidIndex(nIndex))
        {
            m_t1Tab.erase(m_t1Tab.begin() + nIndex) ;
            m_t2Tab.erase(m_t2Tab.begin() + nIndex) ;
        }
        else
            {assert(false);}
    }
    /// Remove an element from table.
    void RemoveElement (const T1& t1, const T2& t2)
    {
        for (int i=0 ; i < GetElementCount() ; i++)
            if ((m_t1Tab[i] == t1) && (m_t2Tab[i] == t2))
            {
                RemoveElement(i) ;
                return ; // must return immediately
            }
    }

    /// Clear all elements.
    void Clear()
    {
        m_t1Tab.clear() ;
        m_t2Tab.clear() ;
    }

    /// Pop a element from front of list.
    void PopFront()
    {
        if (GetElementCount())
        {
            m_t1Tab.pop_front() ;
            m_t2Tab.pop_front() ;
        }
    }

private:
    bool IsValidIndex (int nIndex) const
    {
        return (nIndex >= 0) && (nIndex < GetElementCount()) ;
    }

private:
    std::deque<T1>   m_t1Tab ;
    std::deque<T2>   m_t2Tab ;
};

#endif
