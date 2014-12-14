/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-1-23
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __PCL_INTERFACE_COMPOSITE__2005_01_23__H__
#define __PCL_INTERFACE_COMPOSITE__2005_01_23__H__
#include <assert.h>
#include <deque>

template<class T> class PCL_Interface_Composite ;

//=============================================================================
/**
    composite support.
    enable your class contain composite object, for example:
@code
class CMultiImage : public CObject,
                    private PCL_Interface_Composite<CImage>
{
    int GetImageCount() const { return PCL_GetObjectCount(); }
    CImage* GetImage (int nIndex) const { return PCL_GetObject(nIndex); }
}
@endcode
*/
template<class T>
class PCL_Interface_Composite
{
public:
    virtual ~PCL_Interface_Composite() {PCL_DeleteAllObjects();}

    /**
     *  Get count of object.
     */
    int PCL_GetObjectCount() const {return (int)m_objList.size();}

    /**
     *  Add an object (must created by new) into list (add a NULL object is permitted).
     *  after the object be added, you can't delete it later.
     */
    void PCL_PushObject (T* pObj) {m_objList.push_back (pObj);}

    /**
     *  Insert an object (must created by new) into list (add a NULL object is permitted).
     *  after the object be added, you can't delete it later.
     */
    void PCL_InsertObject (int nIndex, T* pObj)
    {
        if ((nIndex >= 0) && (nIndex <= PCL_GetObjectCount()))
            m_objList.insert (m_objList.begin()+nIndex, pObj) ; // push NULL is permitted
        else
            {assert(false);}
    }

    /**
     *  Get object in list, nIndex is a zero-based index.
     */
    T* PCL_GetObject (int nIndex) const
    {
        if ((nIndex >= 0) && (nIndex < PCL_GetObjectCount()))
            return m_objList[nIndex] ;
        else
            {assert(false); return 0;}
    }

    /**
     *  Get zero-based index which pObj in list, return -1 if not found.
     */
    int PCL_GetObjectIndex (const T* pObj) const
    {
        for (size_t i=0 ; i < m_objList.size() ; i++)
        {
            if (m_objList[i] == pObj)
                return (int)i ;
        }
        return -1 ;
    }

    /**
     *  Throw objects' ownership. you must delete objects yourself.
     */
    void PCL_ThrowOwnership (std::deque<T*>& listObj)
    {
        listObj = m_objList ;
        m_objList.clear() ;
    }

    /**
     *  Delete all objects in list.
     */
    void PCL_DeleteAllObjects()
    {
        while (!m_objList.empty())
        {
            T   * p = m_objList.back() ;
            m_objList.pop_back() ;
            if (p)
                delete p ;
        }
    }

    /**
     *  Delete nIndex object from list.
     */
    void PCL_DeleteObject (int nIndex)
    {
        if ((nIndex >= 0) && (nIndex < PCL_GetObjectCount()))
        {
            T   * p = m_objList[nIndex] ;
            m_objList.erase(m_objList.begin() + nIndex) ;
            if (p)
                delete p ;
        }
        else
        {
            assert(false) ;
        }
    }

    /**
     *  Delete object in list.
     */
    void PCL_DeleteObject (T* pObj)
    {
        if (pObj)
        {
            PCL_DeleteObject (PCL_GetObjectIndex(pObj)) ;
        }
        assert (PCL_GetObjectIndex(pObj) == -1) ;
    }

private:
    std::deque<T*>   m_objList ;
};

//=============================================================================
// inline implement
//=============================================================================

#endif
