/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-4-3
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __PCL_LAZY_OBJECT__2005_04_03__H__
#define __PCL_LAZY_OBJECT__2005_04_03__H__

template<class T> class PCL_Interface_Lazy_Object ;

//=============================================================================
/**
 *  Lazy object, initialize object until get .
 */
template<class T>
class PCL_Interface_Lazy_Object
{
public:
    PCL_Interface_Lazy_Object() : m_pObj(0) {}
    virtual ~PCL_Interface_Lazy_Object() {if(m_pObj) delete m_pObj;}

    /// Get lazy object (object will be created at first call).
    T* PCL_GetLazyObject()
    {
        if (!m_pObj)
        {
            m_pObj = new T ;
            PCL_Initialize_Lazy_Object (m_pObj) ;
        }
        return m_pObj ;
    }

protected:
    /// Initialize lazy object.
    virtual void PCL_Initialize_Lazy_Object (T* pObj) {}
private:
    T   * m_pObj ;
};

//=============================================================================
// inline implement
//=============================================================================

#endif
