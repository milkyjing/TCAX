/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-4-8
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_OBJECT_SELECTION__2003_04_08__H__
#define __FOO_OBJECT_SELECTION__2003_04_08__H__
#include "ObjImage.h"
#include "PCL_interface_zoom.h"

//=============================================================================
/**
 *  Selection object (bitmap mask).
 *  it's a 8-bit gray image, 0xFF means pixel selected, otherwise unselected.
 */
class FCObjSelect : public FCObjImage
{
public:
    /** handle type. */
    enum RGN_TYPE
    {
        RGN_CREATE=0, /**< create a new selection */
        RGN_ADD=1,    /**< add selection into current selection */
        RGN_SUB=2,    /**< sub selection from current selection */
    };
public:
    /**
     *  @name Constructor.
     */
    //@{
    /// Create an empty selection.
    FCObjSelect ()
    {
        m_nZoom = ZOOM_UNSCAN ;
        m_nCurAnt = 0 ;
    }
    /// Copy constructor.
    FCObjSelect (const FCObjSelect& sel)
    {
        m_nZoom = ZOOM_UNSCAN ;
        m_nCurAnt = 0 ;
        *this = sel ;
    }
    //@}

    FCObjSelect& operator= (const FCObjSelect& sel)
    {
        if (&sel == this)
        {
            assert(false); return *this;
        }

        __SetSelectionEmpty(); // enable empty copy
        if (sel.HasSelected())
        {
            m_nZoom = sel.m_nZoom;
            m_nCurAnt = sel.m_nCurAnt;
            m_ptEdge = sel.m_ptEdge; // copy the edge point
            FCObjImage::operator=(sel);
        }
        return *this;
    }

    /// Is selection object valid.
    bool HasSelected() const {return IsValidImage();}

    /**
     *  Optimize selection.
     *  erase un-selected point around. the position of selection will be adjusted automatically.
     */
    void SelectionOptimize()
    {
        if (!HasSelected())
            return ;

        RECT     rcBound = {Width(), Height(), 0, 0} ;
        for (int y=0 ; y < Height() ; y++)
            for (int x=0 ; x < Width() ; x++)
                if (*GetBits(x,y) == 0xFF)
                {
                    if (x < rcBound.left)    rcBound.left = x ;
                    if (x > rcBound.right)   rcBound.right = x ;
                    if (y < rcBound.top)     rcBound.top = y ;
                    if (y > rcBound.bottom)  rcBound.bottom = y ;
                }

        // the selection is open, so we ++
        rcBound.right++ ; rcBound.bottom++ ;

        // the whole selection is empty
        if (IsRectEmpty(&rcBound))
        {
            __SetSelectionEmpty() ;
            return ;
        }

        // the selection can't be optimized
        const RECT   rcSel = {0, 0, Width(), Height()} ;
        if (memcmp (&rcSel, &rcBound, sizeof(RECT)) == 0) // rect is equal
            return ;

        // erase == get a sub block
        const POINT     ptOldPos = GetGraphObjPos() ;
        FCObjImage      imgSub ;
        GetSubBlock (&imgSub, rcBound) ;
        *static_cast<FCObjImage*>(this) = imgSub ;
        SetGraphObjPos (ptOldPos.x+rcBound.left, ptOldPos.y+rcBound.top) ;

        m_ptEdge.clear() ;
        m_nCurAnt = 0 ;
        m_nZoom = ZOOM_UNSCAN ; // force to re-calculate edge
    }

    /**
     *  Calculate edge point by nZoom.
     *  edge point's coordinate relative to top-left of selection object.
     */
    void RecalculateEdge (int nZoom, bool bForceRecalculate)
    {
        if (!HasSelected())
        {
            m_ptEdge.clear() ;
            m_nCurAnt = 0 ;
            m_nZoom = ZOOM_UNSCAN ;
            return ;
        }

        if (!bForceRecalculate)
            if (m_nZoom == nZoom)
                return ;

        m_ptEdge.clear() ;
        m_nCurAnt = 0 ;
        m_nZoom = nZoom ;
        if ((m_nZoom == ZOOM_UNSCAN) || (m_nZoom == 0))
            return ;

        // search edge point
        FCObjImage   EdgeImg (Width(), Height(), 8) ;
        int          nEdgeNum = 0 ;

        // is edge point : current is selected around is non-selected
        // brim point must be edge point
        for (int y=0 ; y < Height() ; y++)
            for (int x=0 ; x < Width() ; x++)
                if (*GetBits(x,y) == 0xFF)
                    if ((y == 0)          || (*GetBits(x,y-1) == 0) || // up
                        (y == Height()-1) || (*GetBits(x,y+1) == 0) || // down
                        (x == 0)          || (*GetBits(x-1,y) == 0) || // left
                        (x == Width()-1)  || (*GetBits(x+1,y) == 0) || // right
                        (*GetBits(x+1,y-1) == 0) || // right-up
                        (*GetBits(x+1,y+1) == 0) || // right-down
                        (*GetBits(x-1,y+1) == 0) || // left-down
                        (*GetBits(x-1,y-1) == 0)) // left-up
                    {
                        *EdgeImg.GetBits(x,y) = 0xFF ;
                        nEdgeNum++ ;
                    }

        // no edge point
        if (nEdgeNum == 0)
            return ;

        if (m_nZoom <= -1)
        {
            // deflate, only div
            FCObjImage   imgHash (Width()/-m_nZoom+2, Height()/-m_nZoom+2, 8) ;
            for (int y=0 ; y < Height() ; y++)
                for (int x=0 ; x < Width() ; x++)
                    if (*EdgeImg.GetBits(x,y) == 0xFF)
                    {
                        POINT   pt = {x/-m_nZoom, y/-m_nZoom} ;
                        BYTE    * pHash = imgHash.GetBits (pt.x, pt.y) ;
                        if (*pHash == 0) // some point --> one point
                        {
                            *pHash = 0xFF ;
                            m_ptEdge.push_back (pt) ;
                        }
                    }
        }
        else
        {
            // inflate, we should insert some point
            const int   nNewW = Width() * m_nZoom,
                        nNewH = Height() * m_nZoom ;

            // calculate x/y table
            PCL_array<int>   pX (nNewW),
                             pY (nNewH) ;
            int              x, y ;
            for (y=0 ; y < nNewH ; y++)
                {pY[y] = y / m_nZoom; assert (pY[y] < Height());}
            for (x=0 ; x < nNewW ; x++)
                {pX[x] = x / m_nZoom; assert (pX[x] < Width());}

            // search edge
            for (y=0 ; y < nNewH ; y++)
                for (x=0 ; x < nNewW ; x++)
                    if (*EdgeImg.GetBits (pX[x],pY[y]) == 0xFF)
                    {
                        // still edge point after inflate
                        int   cr = *GetBits (pX[x],pY[y]) ;
                        if ((y == 0)       || (*GetBits(pX[x],pY[y-1]) != cr) || // up
                            (y == nNewH-1) || (*GetBits(pX[x],pY[y+1]) != cr) || // down
                            (x == 0)       || (*GetBits(pX[x-1],pY[y]) != cr) || // left
                            (x == nNewW-1) || (*GetBits(pX[x+1],pY[y]) != cr)) // right
    //                      (*(pPixel + 1 + dwPitch) == 0) || // right-up
    //                      (*(pPixel + 1 - dwPitch) == 0) || // right-down
    //                      (*(pPixel - 1 - dwPitch) == 0) || // left-down
    //                      (*(pPixel - 1 + dwPitch) == 0)) // left_up
                        {
                            POINT   pt = {x,y} ;
                            m_ptEdge.push_back (pt) ;
                        }
                    }
        }
    }

private:
    enum
    {
        ZOOM_UNSCAN = 0x7FFF,
    };

    // internal. set selection empty
    void __SetSelectionEmpty()
    {
        FCObjImage::Destroy() ;
        m_ptEdge.clear() ;
        m_nZoom = ZOOM_UNSCAN ;
        m_nCurAnt = 0 ;
    }

private:
    int     m_nZoom ;
    int     m_nCurAnt ; // [0..7]
    std::deque<POINT>   m_ptEdge ; // scaled edge point

friend class FCWin32 ;
};

//===================================================================
// inline implement
//===================================================================

#endif
