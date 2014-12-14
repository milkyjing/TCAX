/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-3-30
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef	__FOO_OBJECT_CANVAS__2003_03_30__H__
#define	__FOO_OBJECT_CANVAS__2003_03_30__H__
#include "ObjLayer.h"
#include "ObjSelect.h"
#include "pixelprocessor/PixelProcessorBase.h"
#include "command/Interface_Command.h"
#include "PCL_interface_zoom.h"
#include "PCL_interface_undo.h"
#include <algorithm>

//=============================================================================
/**
 *  Canvas object - container of layers.
 */
class FCObjCanvas : public PCL_Interface_ZoomScale,
                    public PCL_Interface_Undo<FCCmdArtPrider>
{
public:
    /// Constructor.
    FCObjCanvas (int nUndoLevel = 20) : PCL_Interface_Undo<FCCmdArtPrider>(nUndoLevel)
    {
        m_sizeCanvas.cx = m_sizeCanvas.cy = 0 ;
        m_pCurrentLayer = 0 ;
        m_nResX = m_nResY = 72 ;
    }

    virtual ~FCObjCanvas()
    {
        Clear() ;
    }

    /**
     *  Clear canvas, command history, layer...
     */
    void Clear()
    {
        // first, maybe command hold layer's pointer
        ClearUndoList() ;
        ClearRedoList() ;

        // layers
        while (!m_LayerList.empty())
        {
            FCObjLayer   * p = m_LayerList.back() ;
            m_LayerList.pop_back() ;
            delete p ;
        }

        // removed layers
        while (!m_RemovedLayerList.empty())
        {
            FCObjLayer   * p = m_RemovedLayerList.back() ;
            m_RemovedLayerList.pop_back() ;
            delete p ;
        }
        m_pCurrentLayer = 0 ;

        m_sizeCanvas.cx = m_sizeCanvas.cy = 0 ;
        m_nResX = m_nResY = 72 ;
        m_CurrSel = FCObjSelect() ;
    }

    /// Set DPI (dot per inch) resolution.
    void SetCanvasResolution (int nResX, int nResY) {m_nResX=nResX ; m_nResY=nResY ;}
    /// Get DPI (dot per inch) resolution.
    void GetCanvasResolution (int& nResX, int& nResY) const {nResX=m_nResX ; nResY=m_nResY ;}

	/// Set canvas's size.
    void SetCanvasDimension (SIZE sz) {m_sizeCanvas=sz; assert(sz.cx>=1 && sz.cy>=1);}
    /// Get canvas's size.
    SIZE GetCanvasDimension () const {return m_sizeCanvas;}
    /// Get canvas's scaled size.
    SIZE GetCanvasScaledDimension() const
    {
        POINT   pt = {m_sizeCanvas.cx, m_sizeCanvas.cy} ;
        Actual_to_Scaled (pt) ;
        SIZE    sz = {pt.x, pt.y} ;
        return sz ;
    }

    /// Get current selection object on canvas.
    const FCObjSelect& GetSelection() const {return m_CurrSel;}
    /// Has a selection object on canvas.
    bool HasSelected() const {return m_CurrSel.HasSelected();}

    /// Bound rect in canvas (rc is coordinate of canvas).
    void BoundRect (RECT& rc) const
    {
        RECT   rcCanvas = {0, 0, m_sizeCanvas.cx, m_sizeCanvas.cy} ;
        ::IntersectRect (&rc, &rcCanvas, &rc) ;
    }

    /**
     *  @name Layers manage.
     */
    //@{
    /// Get number of layers on canvas.
    int GetLayerNumber() const {return (int)m_LayerList.size();}
    /// Get zero-based index layer.
    FCObjLayer* GetLayer (int nIndex) const
    {
        if ((nIndex >= 0) && (nIndex < GetLayerNumber()))
            return m_LayerList[nIndex] ;
        else
            return 0 ;
    }
    /// Get current layer of canvas.
    FCObjLayer* GetCurrentLayer() const {return m_pCurrentLayer;}
    /// Get current layer's index.
    int GetCurrentLayerIndex() const {return FindLayer(m_pCurrentLayer);}
    /// Set current layer of canvas.
    void SetCurrentLayer (int nIndex)
    {
        FCObjLayer   * pLayer = GetLayer(nIndex) ; assert(pLayer) ;
        if (pLayer)
            m_pCurrentLayer = pLayer ;
    }
    /**
     *  Add a new layer into canvas.
     *   1) you must use <B>new</B> to create a layer.   <BR>
     *   2) after the layer be added, you can't delete it any more.
     *  @param nIndex - insert position, -1 add to last
     */
    void AddLayer (FCObjLayer* pLayer, int nIndex = -1)
    {
        if (!pLayer || (pLayer->ColorBits() != 32)) // now layer must 32-bpp
            {assert(false); return;}

        // ensure the insert position is valid
        if ((nIndex < -1) || (nIndex > GetLayerNumber()))
        {
            assert(false) ;
            nIndex = -1 ;
        }

        // ensure the new layer isn't in current layer list
        std::deque<FCObjLayer*>::iterator   pt = __pcl_FindDeque (m_LayerList, pLayer) ;
        if (pt != m_LayerList.end()) // already in layer list
            {assert(false); return;}

        // add into layer list
        if (nIndex == -1)
            m_LayerList.push_back (pLayer) ;
        else
            m_LayerList.insert (m_LayerList.begin() + nIndex, pLayer) ;
        m_pCurrentLayer = pLayer ;
        
        // remove from <Removed LayerList>
        pt = __pcl_FindDeque (m_RemovedLayerList, pLayer) ;
        if (pt != m_RemovedLayerList.end())
            m_RemovedLayerList.erase (pt) ;
    }
    /**
     *  Remove layer from canvas (but not delete layer).
     *  when only one layer in canvas, you can't remove it.
     */
    void RemoveLayer (FCObjLayer* pLayer)
    {
        if (!pLayer || (GetLayerNumber() <= 1))
            {assert(false); return;}

        // find layer in current layer list
        std::deque<FCObjLayer*>::iterator   pt = __pcl_FindDeque (m_LayerList, pLayer) ;
        if (pt == m_LayerList.end()) // not in layer list
            {assert(false); return;}

        // remember position then remove from list
        int     nIndex = (int)(pt - m_LayerList.begin()) ;
        m_LayerList.erase (pt) ;

        // update current layer
        if (m_pCurrentLayer == pLayer)
        {
            nIndex = FMin (nIndex, GetLayerNumber()-1) ;
            SetCurrentLayer (nIndex) ;
        }

        // add to removed-layer list
        if (__pcl_FindDeque (m_RemovedLayerList, pLayer) == m_RemovedLayerList.end())
            m_RemovedLayerList.push_back (pLayer) ;
        else
            {assert(false);}
    }
    /// Get the layer's index in canvas.
    int FindLayer (const FCObjLayer* pLayer) const
    {
        for (int i=0 ; i < GetLayerNumber() ; i++)
        {
            if (pLayer == GetLayer(i))
                return i ;
        }
        return -1 ;
    }
    /// Delete layer from removed list.
    void DeleteLayerFromRemoveList (FCObjLayer* pLayer)
    {
        if (!pLayer)
            {assert(false); return;}

        std::deque<FCObjLayer*>::iterator   pt = __pcl_FindDeque (m_RemovedLayerList, pLayer) ;
        if (pt != m_RemovedLayerList.end()) // in removed layer list
        {
            m_RemovedLayerList.erase (pt) ;
            delete pLayer ;
        }
    }
    //@}

    /// Execute a command, mustn't delete command after execute.
    void ExecuteEffect (FCCmdArtPrider* cmd, FCObjProgress* pProgress=0)
    {
        if (!cmd)
            {assert(false); return;}

        // It's very important to call ClearRedoList before cmd->Execute !!!
        ClearRedoList() ;

        cmd->Execute (*this, pProgress) ;

        // then we manipulate the command according undo-list
        if ((GetUndoLevel() > 0) && !cmd->IsNeedDeleteAfterExecute())
        {
            AddCommand (cmd) ;
        }
        else
        {
            delete cmd ;
        }
    }

    /**
     *  @name Make image.
     */
    //@{
    /// Get canvas's block view.
    /// created imgRegion's bpp same to imgBack
    /// @param rcRegion : RECT on canvas
    void MakeViewWindow (double fScale,
                         std::deque<FCObjLayer*> layerList,
                         const FCObjImage& imgBack,
                         RECT rcRegion,
                         FCObjImage& imgRegion) const
    {
        imgRegion.Destroy() ;

        // must in canvas
        {
            SIZE   sz = GetCanvasDimension() ;
            RECT   rcCanvas = {0, 0, sz.cx, sz.cy} ;
            if (!IsRectInRect (rcCanvas, rcRegion))
                {assert(false); return;}
        }

        // create image & draw back
        int   nL = (int)(fScale * rcRegion.left),
              nT = (int)(fScale * rcRegion.top),
              nR = (int)(fScale * rcRegion.right),
              nB = (int)(fScale * rcRegion.bottom) ;
        if ((nR <= nL) || (nB <= nT))
            return ;

        if (!imgRegion.Create (nR-nL, nB-nT, imgBack.ColorBits()))
            return ;

        imgRegion.TileBlock (imgBack, -nL, -nT) ;

        // draw layers
        for (size_t i=0 ; i < layerList.size() ; i++)
        {
            FCObjLayer   * pLayer = layerList[i] ;
            if (!pLayer->IsLayerVisible())
                continue ;

            RECT     rcOnLayer = rcRegion ;
            pLayer->Canvas_to_Layer (rcOnLayer) ;
            pLayer->BoundRect (rcOnLayer) ;
            if (IsRectEmpty(&rcOnLayer))
                continue ;

            // RECT of layer on imgRegion
            RECT     rcOnImg = rcOnLayer ;
            pLayer->Layer_to_Canvas (rcOnImg) ;
            rcOnImg.left   = (int)(fScale * rcOnImg.left) ;
            rcOnImg.top    = (int)(fScale * rcOnImg.top) ;

            int   nSR = (int)(fScale * rcOnImg.right),
                  nSB = (int)(fScale * rcOnImg.bottom) ;

            // a little error when zoom out
            if (fScale < 1.0)
            {
                int   nSFrontR = (int)(fScale * (rcOnImg.right-1)) ;
                rcOnImg.right = nSR ;
                if (nSFrontR == nSR)
                    rcOnImg.right++ ;
                
                int   nSUpB = (int)(fScale * (rcOnImg.bottom-1)) ;
                rcOnImg.bottom = nSB ;
                if (nSUpB == nSB)
                    rcOnImg.bottom++ ;
            }
            else
            {
                rcOnImg.right  = nSR ;
                rcOnImg.bottom = nSB ;
            }

            OffsetRect (&rcOnImg, -nL, -nT) ;
            if (!IsRectEmpty(&rcOnImg))
            {
                imgRegion.AlphaBlend (*pLayer, rcOnImg, rcOnLayer, pLayer->GetLayerTransparent()) ;
            }
        }
    }

    /// imgThumb's bpp same to imgBack.
    void MakeThumbnail (double fScale,
                        FCObjLayer& rLayer,
                        const FCObjImage& imgBack,
                        FCObjImage& imgThumb) const
    {
        std::auto_ptr<FCMemoLayer>   bakMemo (rLayer.CreateMemoObj()),
                                     newMemo (rLayer.CreateMemoObj()) ;
        newMemo->m_bLayerVisible = true ;
        newMemo->m_nAlphaPercent = 100 ;
        rLayer.SetMemoObj (newMemo.get()) ;

        RECT     rcRegion = {0, 0, GetCanvasDimension().cx, GetCanvasDimension().cy} ;

        std::deque<FCObjLayer*>   layerList ;
        layerList.push_back (&rLayer) ;

        MakeViewWindow (fScale, layerList, imgBack, rcRegion, imgThumb) ;

        rLayer.SetMemoObj (bakMemo.get()) ;
    }

    /// Make canvas image (imgCanvas's bpp is 32).
    void GetCanvasImage (FCObjImage& imgCanvas) const
    {
        if (!imgCanvas.Create(m_sizeCanvas.cx, m_sizeCanvas.cy, 32))
            {assert(false); return;}

        FCPixelFillColor   aCmd (FCColor::crWhite(), 0) ;
        imgCanvas.SinglePixelProcessProc (aCmd) ; // alpha init 0

        for (int i=0 ; i < GetLayerNumber() ; i++)
        {
            FCObjLayer   * pLayer = GetLayer(i) ;
            if (pLayer->IsLayerVisible())
                imgCanvas.CombineImage (*pLayer,
                                        pLayer->GetGraphObjPos().x,
                                        pLayer->GetGraphObjPos().y,
                                        pLayer->GetLayerTransparent()) ;
        }
    }

    /// Make layer's region.
    void MakeRegion (const FCObjLayer& rLayer, FCObjImage& imgRegion) const
    {
        imgRegion.Destroy() ;

        if (!HasSelected())
        {
            imgRegion = static_cast<const FCObjImage&>(rLayer) ;
        }
        else
        {
            RECT     rc = {0, 0, m_CurrSel.Width(), m_CurrSel.Height()} ;
            m_CurrSel.Layer_to_Canvas (rc) ;
            rLayer.Canvas_to_Layer (rc) ;
            rLayer.BoundRect (rc) ;
            if (IsRectEmpty (&rc))
                return ;

            // rect on layer & selection
            RECT     rcOnLayer = rc,
                     rcOnSel = rc ;
            rLayer.Layer_to_Canvas (rcOnSel) ;
            m_CurrSel.Canvas_to_Layer (rcOnSel) ;

            // out of selection
            rLayer.GetSubBlock (&imgRegion, rcOnLayer) ;
            for (int y=0 ; y < imgRegion.Height() ; y++)
                for (int x=0 ; x < imgRegion.Width() ; x++)
                    if (*m_CurrSel.GetBits(x+rcOnSel.left, y+rcOnSel.top) == 0)
                    {
                        *(RGBQUAD*)imgRegion.GetBits(x,y) = PCL_RGBA(0xFF,0xFF,0xFF,0) ;
                    }
        }
    }
    //@}

    /**
     *  @name Access .oxo file.
     */
    //@{
    /// Read .oxo file.
    bool Load_oXo (const char* szFileName)
    {
        char     * _pCurr = 0 ;
        int      nFileSize = 0 ;
        // load file into memory
        FCOXOHelper::LoadFileToBuffer (szFileName, _pCurr, nFileSize) ;
        if (!_pCurr)
            {assert(false); return false;}

        BYTE              * pCurr = (BYTE*)_pCurr ;
        PCL_array<BYTE>   _aAutoDelete (pCurr) ;

        // read file's header TAG : "oXo " , version 1
        if ((*(DWORD*)pCurr != 0x206F586F) || (*(DWORD*)(pCurr + 4) != 1))
            return false ;
        pCurr += 8 ;

        // read block
        while (*(DWORD*)pCurr != OXO_BLOCK_END)
        {
            BYTE      * pBak = pCurr ;
            DWORD     nBlockSize = *(DWORD*)(pCurr + 4) ; // block size, exclude TAG & SIZE (8-bytes)

            if (*(DWORD*)pCurr == OXO_BLOCK_CANVAS) // canvas
            {
                pCurr += 8 ;
                m_sizeCanvas = *(SIZE*)pCurr ;
            }
            else if (*(DWORD*)pCurr == OXO_BLOCK_CANVAS_DPI) // canvas's DPI
            {
                pCurr += 8 ;
                m_nResX = *(DWORD*)pCurr ; pCurr += 4 ;
                m_nResY = *(DWORD*)pCurr ; pCurr += 4 ;
            }
            else if (*(DWORD*)pCurr == OXO_BLOCK_LAYER) // layer
            {
                pCurr += 8 ;
                FCObjLayer   * pLayer = new FCObjLayer ;
                pLayer->Serialize (false, pCurr) ;
                this->AddLayer(pLayer) ;
            }
            else if (*(DWORD*)pCurr == OXO_BLOCK_TEXTLAYER) // text layer
            {
                pCurr += 8 ;
                // has discarded after PhoXo V1.6
                // first skip text info
                pCurr += strlen((char*)pCurr) + 1 + 84 ;

                FCObjLayer   * pLayer = new FCObjLayer ;
                pLayer->Serialize (false, pCurr) ;
                this->AddLayer(pLayer) ;
            }
            else if (*(DWORD*)pCurr == OXO_BLOCK_LAYER_NAME) // layer's name
            {
                pCurr += 4 ;
                DWORD     n = *(DWORD*)pCurr ; pCurr += 4 ;
                // name string
                PCL_array<char>   p(n+8) ;
                memset (p.get(), 0, n+8) ;
                memcpy (p.get(), pCurr, n) ;
                if (GetLayerNumber())
                {
                    GetLayer(GetLayerNumber()-1)->LayerName() = p.get() ;
                }
            }
            else
            {
                // unknow block, do nothing
            }
            pCurr = pBak + nBlockSize + 8 ;
        }
        return true ;
    }
    /// Write .oxo file.
    bool Save_oXo (const char* szFileName) const
    {
        if (GetLayerNumber() <= 0)
            {assert(false); return false;}

        // estimate size to malloc memory
        int     nMaxSize = 1024 * 32, i ;
        for (i=0 ; i < GetLayerNumber() ; i++)
        {
            nMaxSize += sizeof(BITMAPINFOHEADER) + 12 ;
            nMaxSize = nMaxSize + GetLayer(i)->GetPitch() * GetLayer(i)->Height() ;
        }

        PCL_array<BYTE>   pStart (nMaxSize) ;
        BYTE              * pCurr = pStart.get() ;

        // write signature
        *(DWORD*)pCurr = 0x206F586F ; pCurr += 4 ; // "oXo "
        // oXo file's version
        *(DWORD*)pCurr = 0x01       ; pCurr += 4 ;

        // store canvas basic info
        *(DWORD*)pCurr = OXO_BLOCK_CANVAS ; pCurr += 4 ;
        *(DWORD*)pCurr = sizeof(SIZE)     ; pCurr += 4 ;
        *(SIZE*)pCurr = m_sizeCanvas     ; pCurr += sizeof(SIZE) ;

        // store canvas DPI
        *(DWORD*)pCurr = OXO_BLOCK_CANVAS_DPI ; pCurr += 4 ;
        *(DWORD*)pCurr = 8                    ; pCurr += 4 ;
        *(DWORD*)pCurr = m_nResX              ; pCurr += 4 ;
        *(DWORD*)pCurr = m_nResY              ; pCurr += 4 ;

        // store layer
        for (i=0 ; i < GetLayerNumber() ; i++)
        {
            *(DWORD*)pCurr = OXO_BLOCK_LAYER ; pCurr += 4 ;
            *(DWORD*)pCurr = 0               ; pCurr += 4 ; // write size later
            int     nWrite = GetLayer(i)->Serialize (true, pCurr) ;
            *(DWORD*)(pCurr - 4) = nWrite    ; pCurr += nWrite ;

            // save layer's name
            std::string   s = GetLayer(i)->LayerName() ;
            if (!s.empty())
            {
                *(DWORD*)pCurr = OXO_BLOCK_LAYER_NAME ; pCurr += 4 ;
                *(DWORD*)pCurr = (DWORD)s.length() ; pCurr += 4 ;
                memcpy (pCurr, s.c_str(), s.length()) ; pCurr += s.length() ;
            }
        }

        // terminator
        *(DWORD*)pCurr = OXO_BLOCK_END ; pCurr += 4 ;
        *(DWORD*)pCurr = 0 ;           ; pCurr += 4 ;

        // write file
        return FCOXOHelper::SaveBufferToFile (szFileName, pStart.get(), (int)(pCurr - pStart.get())) ;
    }
    //@}

private:
    virtual void PCL_Implement_Undo (FCCmdArtPrider* pCmd) {pCmd->Undo (*this);}
    virtual void PCL_Implement_Redo (FCCmdArtPrider* pCmd) {pCmd->Redo (*this);}

    enum
    {
        OXO_BLOCK_UNKNOW = 0,     // unknow
        OXO_BLOCK_CANVAS = 1,     // canvas
        OXO_BLOCK_LAYER = 2,      // layer
        OXO_BLOCK_TEXTLAYER = 3,  // text layer -- has discarded after PhoXo V1.6
        OXO_BLOCK_CANVAS_DPI = 4, // canvas DPI
        OXO_BLOCK_LAYER_NAME = 5, // name of layer, this block follow layer block
        OXO_BLOCK_END = 0xFF,     // end
    };

private:
	SIZE				m_sizeCanvas ;
	FCObjLayer			* m_pCurrentLayer ;
	std::deque<FCObjLayer*>   m_LayerList ;
	std::deque<FCObjLayer*>   m_RemovedLayerList ; // 被移除的layer，和m_LayerList一起释放

    int                 m_nResX, m_nResY ; // image's DPI resolution (Dot Per Inch)

	FCObjSelect			m_CurrSel ; // 当前选取的区域对象

    static std::deque<FCObjLayer*>::iterator __pcl_FindDeque (std::deque<FCObjLayer*>& listLayer, const FCObjLayer* pLayer)
    {
        return std::find (listLayer.begin(), listLayer.end(), pLayer) ;
    }

friend class FCCmdLayerExchange ;
friend class FCCmdSelectionSetBase ;
};

//=============================================================================
// inline implement
//=============================================================================

#endif
