/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-3-30
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_OBJECT_BASE__2003_03_30__H__
#define __FOO_OBJECT_BASE__2003_03_30__H__
#include "StdDefine.h"

class FCObject ;
    class FCObjGraph ;

//=============================================================================
/**
 *  The root of all objects
 */
class FCObject
{
public:
    virtual ~FCObject() {}
    /**
     *  Save object to memory / Load object from memory.
     *  @param bSave : save(true) or load(false)
     *  @return written or loaded bytes.
     */
    virtual int Serialize (bool bSave, BYTE* pSave) {return 0;}
};

//=============================================================================
/**
 *  Basic graphic object (encapsulate object's coordinate).
 */
class FCObjGraph : public FCObject
{
public:
    FCObjGraph() {m_ptObj.x = m_ptObj.y = 0;}

    FCObjGraph& operator= (const FCObjGraph& GraphObj)
    {
        m_ptObj = GraphObj.m_ptObj ;
        return *this ;
    }

    /**
     *  Save graphic position to memory / Load graphic position from memory.
     *  @see FCObject::Serialize
     */
    virtual int Serialize (bool bSave, BYTE* pSave)
    {
        if (bSave)
            *(POINT*)pSave = m_ptObj ;
        else
            m_ptObj = *(POINT*)pSave ;
        return sizeof(m_ptObj) ;
    }

    /**
     *  @name Object's position (on canvas).
     */
    //@{
    /// Set position of graph.
    void SetGraphObjPos (int x, int y) {m_ptObj.x=x; m_ptObj.y=y;}
    /// Set position of graph.
    void SetGraphObjPos (POINT pt) {m_ptObj = pt;}
    /// Get position of graph.
    POINT GetGraphObjPos() const {return m_ptObj;}
    /// Offset position of graph.
    void OffsetGraphObj (int xOff, int yOff) {m_ptObj.x += xOff; m_ptObj.y += yOff;}
    //@}

    /**
     *  @name Coordinate transform
     */
    //@{
    /// canvas ==> layer
    void Canvas_to_Layer (POINT& pt) const
    {
        pt.x -= m_ptObj.x ; pt.y -= m_ptObj.y ;
    }
    /// canvas ==> layer
    void Canvas_to_Layer (RECT& rc) const
    {
        rc.left -= m_ptObj.x ;
        rc.top -= m_ptObj.y ;
        rc.right -= m_ptObj.x ;
        rc.bottom -= m_ptObj.y ;
    }

    /// layer ==> canvas
    void Layer_to_Canvas (POINT& pt) const
    {
        pt.x += m_ptObj.x ; pt.y += m_ptObj.y ;
    }
    /// layer ==> canvas
    void Layer_to_Canvas (RECT& rc) const
    {
        rc.left += m_ptObj.x ;
        rc.top += m_ptObj.y ;
        rc.right += m_ptObj.x ;
        rc.bottom += m_ptObj.y ;
    }
    //@}

private:
    POINT     m_ptObj ; // object position on canvas
};

//=============================================================================
// inline implement
//=============================================================================

#endif
