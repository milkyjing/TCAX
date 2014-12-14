/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-4-8
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef	__FOO_COMMAND__2003_04_08__H__
#define	__FOO_COMMAND__2003_04_08__H__
#include "../ObjCanvas.h"

    // base class of normal image effect command
    class FCCmdImgEffect ;
        class FCCmdImgPixelProcessor ; // receive a FCSinglePixelProcessBase object
        class FCCmdImgSetAlpha ; // set image's alpha
        class FCCmdImgAddShadow ; // add shadow
        class FCCmdImageMask ; // create image to mask
            class FCCmdMaskColor ; // mask color on layer
            class FCCmdMaskGradientBase ; // base class of mask gradient on layer
                class FCCmdMaskGradientLine ; // mask linear gradient on layer
                class FCCmdMaskGradientBiLine ; // mask bilinear gradient on layer
                class FCCmdMaskGradientConicalSym ; // mask symmetric conical gradient on layer
                class FCCmdMaskGradientConicalASym ; // mask anti-symmetric conical gradient on layer
                class FCCmdMaskGradientRect ; // mask rectangle gradient on layer
                class FCCmdMaskGradientRadial ; // mask radial gradient on layer
            class FCCmdMaskLineBase ; // base class of mask line on layer

    // auto enlarge layer to fill up canvas
    class FCCmdAutoEnlargeLayer ;

    // selection operation
    class FCCmdSelectionSetBase ; // base class to set canvas's region
        class FCCmdSelectionInvert ; // invert current region
        class FCCmdSelectionCanvasAll ; // select all canvas
        class FCCmdSelectionClear ; // clear selection
        class FCCmdSelectionSmooth ; // smooth selection
        class FCCmdSelectionBlendHaloBase ;
            class FCCmdSelectionExpand ; // expand selection
            class FCCmdSelectionShrink ; // shrink selection
            class FCCmdSelectionBorder ; // border selection
        class FCCmdSelectionCreateBase ;
            class FCCmdSelectionCreateRect ; // rectangle region
            class FCCmdSelectionCreateEllipse ; // ellipse region
            class FCCmdSelectionCreatePolygonBase ; // polygon region
            class FCCmdSelectionCreateMagicWand ; // magic wand

    // Layer's operation
    class FCCmdLayerOperation ;
        class FCCmdLayerStretch ; // stretch layer
        class FCCmdLayerRotate ; // rotate layer
        class FCCmdLayerRotate90 ; // clockwise rotate 90'
        class FCCmdLayerRotate270 ; // clockwise rotate 270'
        class FCCmdLayerSkew ; // skew transform
        class FCCmdLayerLens ; // lens transform
        class FCCmdLayerCrop ; // crop layer
    class FCCmdLayerMove ; // move layer
    class FCCmdLayerAdd ; // add new layer to canvas
    class FCCmdLayerRemove ; // remove layer from canvas
    class FCCmdLayerSaveProperty ; // save layer's property
    class FCCmdLayerExchange ; // change layer's sequence in canvas

    // 组合命令，可以把频繁使用的cmd（如：layer move组合成为一个cmd）
//  class FCCmdComposite ;
        class FCCmdLayerMerge ; // merge layers
        class FCCmdLayerListRemove ; // delete layers from canvas
        class FCCmdCanvasOperation ;
            class FCCmdCanvasResize ; // resize canvas
            class FCCmdCanvasRotate90 ; // canvas rotate 90'
            class FCCmdCanvasRotate270 ; // canvas rotate 270'
            class FCCmdCanvasRotate ; // rotate canvas
            class FCCmdCanvasStretch ; // stretch canvas
            class FCCmdCanvasCrop ; // crop canvas
            class FCCmdCanvasAutoCrop ; // 自动裁减画布/auto crop canvas

//=============================================================================
/**
 *  Base class of image effect command.
 */
class FCCmdImgEffect : public FCCmdArtPrider
{
public:
    FCCmdImgEffect() : m_pLayer(0) {}
protected:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_pLayer = canvas.GetCurrentLayer() ;
        if (!m_pLayer || !m_pLayer->IsValidImage())
        {
            m_pLayer = 0 ;
            return ;
        }

        // start
        if (pProgress)
            pProgress->ResetProgress() ;

        if (!canvas.HasSelected())
        {
            // save whole layer
            m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
            this->Implement (*m_pLayer, pProgress) ;

            // don't change alpha channel
            if (IsLayerLimited(*m_pLayer))
                for (int y=0 ; y < m_Undo.Height() ; y++)
                    for (int x=0 ; x < m_Undo.Width() ; x++)
                        PCL_A(m_pLayer->GetBits(x,y)) = PCL_A(m_Undo.GetBits(x,y)) ;
        }
        else
        {
            RECT   rcOnSel ;
            {
                RECT   rcOnLayer = {0, 0, canvas.GetSelection().Width(), canvas.GetSelection().Height()} ;
                canvas.GetSelection().Layer_to_Canvas (rcOnLayer) ;
                m_pLayer->Canvas_to_Layer (rcOnLayer) ;
                m_pLayer->BoundRect (rcOnLayer) ;
                if (IsRectEmpty (&rcOnLayer))
                    return ;

                m_pRectOnLayer = std::auto_ptr<RECT>(new RECT) ;
                rcOnSel = *m_pRectOnLayer = rcOnLayer ;
                m_pLayer->Layer_to_Canvas (rcOnSel) ;
                canvas.GetSelection().Canvas_to_Layer (rcOnSel) ;
            }

            // save region
            m_pLayer->GetSubBlock (&m_Undo, *m_pRectOnLayer) ;
            m_Undo.SetGraphObjPos (m_pLayer->GetGraphObjPos().x + m_pRectOnLayer->left,
                                   m_pLayer->GetGraphObjPos().y + m_pRectOnLayer->top) ;

            // set alpha == 0 out of selection, some effect need (such as box blur)
            FCObjImage     block(m_Undo) ;
            {
                for (int y=0 ; y < block.Height() ; y++)
                    for (int x=0 ; x < block.Width() ; x++)
                        if (*canvas.GetSelection().GetBits (x+rcOnSel.left, y+rcOnSel.top) == 0)
                            *(RGBQUAD*)block.GetBits(x,y) = PCL_RGBA(0xFF,0xFF,0xFF,0) ;
            }

            // process
            this->Implement (block, pProgress) ;

            // put back
            for (int y=0 ; y < block.Height() ; y++)
                for (int x=0 ; x < block.Width() ; x++)
                    if (*canvas.GetSelection().GetBits (x+rcOnSel.left, y+rcOnSel.top))
                        FCColor::CopyPixel (m_pLayer->GetBits (x+m_pRectOnLayer->left, y+m_pRectOnLayer->top),
                                            block.GetBits(x,y),
                                            IsLayerLimited(*m_pLayer) ? 3 : 4) ;
        }
        if (pProgress)
            pProgress->SetProgress (100) ; // set 100%
    }

    virtual void Undo (FCObjCanvas& canvas)
    {
        if (!m_Undo.IsValidImage() || (canvas.FindLayer(m_pLayer) == -1))
            {assert(false); return;}

        if (!m_pRectOnLayer.get())
        {
            // whole image
            m_Redo = *static_cast<FCObjImage*>(m_pLayer) ;
            *static_cast<FCObjImage*>(m_pLayer) = m_Undo ;
        }
        else
        {
            m_pLayer->GetSubBlock (&m_Redo, *m_pRectOnLayer) ;
            m_pLayer->CoverBlock (m_Undo, m_pRectOnLayer->left, m_pRectOnLayer->top) ;
        }
        m_Undo.Destroy() ;
    }

    virtual void Redo (FCObjCanvas& canvas)
    {
        if (!m_Redo.IsValidImage() || (canvas.FindLayer(m_pLayer) == -1))
            {assert(false); return;}

        if (!m_pRectOnLayer.get())
        {
            // whole image
            m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
            *static_cast<FCObjImage*>(m_pLayer) = m_Redo ;
        }
        else
        {
            m_pLayer->GetSubBlock (&m_Undo, *m_pRectOnLayer) ;
            m_pLayer->CoverBlock (m_Redo, m_pRectOnLayer->left, m_pRectOnLayer->top) ;
        }
        m_Redo.Destroy() ;
    }

    /// Process image, don't change img's size & position.
    virtual void Implement (FCObjImage& img, FCObjProgress* pProgress) {}

    virtual bool IsLayerLimited (const FCObjLayer& rLayer) const
    {
        return rLayer.IsLayerLimited() ? true : false ;
    }

    FCObjLayer      * m_pLayer ;
    FCObjImage      m_Undo ;
    FCObjImage      m_Redo ;
    std::auto_ptr<RECT>   m_pRectOnLayer ;
};

//=============================================================================
/**
 *  Process image use a FCSinglePixelProcessBase command.
 */
class FCCmdImgPixelProcessor : public FCCmdImgEffect
{
public:
    /// Constructor (pProcessor must use <B>new</B> to create).
    FCCmdImgPixelProcessor (FCSinglePixelProcessBase* pProcessor) : m_pProcessor(pProcessor) {}
    virtual void Implement (FCObjImage& img, FCObjProgress* pProgress)
    {
        if (m_pProcessor.get())
            img.SinglePixelProcessProc (*m_pProcessor, pProgress) ;
    }
private:
    std::auto_ptr<FCInterface_PixelProcess>   m_pProcessor ;
};

//=============================================================================
/**
 *  Set layer's alpha, ignore layer's limited.
 */
class FCCmdImgSetAlpha : public FCCmdImgEffect
{
public:
    /// Constructor.
    FCCmdImgSetAlpha (int nAlpha) : m_nAlpha(FClamp0255(nAlpha)) {}
private:
    virtual void Implement (FCObjImage& img, FCObjProgress* pProgress)
    {
        if (img.IsValidImage() && (img.ColorBits() == 32))
            img.SetAlphaChannelValue (m_nAlpha) ;
    }
    virtual bool IsLayerLimited (const FCObjLayer& rLayer) const
    {
        return false ;
    }
private:
    int     m_nAlpha ;
};

//=============================================================================
/**
 *  Add shadow.
 */
class FCCmdImgAddShadow : public FCCmdImgEffect
{
public:
    /// Constructor.
    FCCmdImgAddShadow (SHADOWDATA ShData)
    {
        m_ShadowData = ShData ;
        m_ShadowData.nSmooth = FMax (2, (int)m_ShadowData.nSmooth) ;
        m_ShadowData.nAlpha = FClamp ((int)m_ShadowData.nAlpha, 1, 100) ;
    }
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_pLayer = canvas.GetCurrentLayer() ;
        if (!m_pLayer || !m_pLayer->IsValidImage())
            return ;

        // start
        if (pProgress)
            pProgress->ResetProgress() ;

        if (!canvas.HasSelected())
        {
            // save whole layer
            m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
            this->Implement (*m_pLayer, pProgress) ;
        }
        else
        {
            RECT   rcOnSel ;
            {
                RECT   rcOnLayer = {0, 0, canvas.GetSelection().Width(), canvas.GetSelection().Height()} ;
                canvas.GetSelection().Layer_to_Canvas (rcOnLayer) ;
                m_pLayer->Canvas_to_Layer (rcOnLayer) ;
                m_pLayer->BoundRect (rcOnLayer) ;
                if (IsRectEmpty (&rcOnLayer))
                    return ;

                m_pRectOnLayer = std::auto_ptr<RECT>(new RECT) ;
                rcOnSel = *m_pRectOnLayer = rcOnLayer ;
                m_pLayer->Layer_to_Canvas (rcOnSel) ;
                canvas.GetSelection().Canvas_to_Layer (rcOnSel) ;
            }

            // set alpha == 0 out of selection
            FCObjImage     imgBlock ;
            m_pLayer->GetSubBlock (&imgBlock, *m_pRectOnLayer) ;
            {
                for (int y=0 ; y < imgBlock.Height() ; y++)
                    for (int x=0 ; x < imgBlock.Width() ; x++)
                        if (*canvas.GetSelection().GetBits (x+rcOnSel.left, y+rcOnSel.top) == 0)
                            *(RGBQUAD*)imgBlock.GetBits(x,y) = PCL_RGBA(0xFF,0xFF,0xFF,0) ;
            }

            // calculate shadowed image size
            RECT     rcShadow = *m_pRectOnLayer ;
            ::OffsetRect (&rcShadow, m_ShadowData.nOffsetX, m_ShadowData.nOffsetY) ;
            ::InflateRect (&rcShadow, m_ShadowData.nSmooth, m_ShadowData.nSmooth) ;
            ::UnionRect (&rcShadow, m_pRectOnLayer.get(), &rcShadow) ;

            // calculate the expand size
            const RECT   rcLayer = {0, 0, m_pLayer->Width(), m_pLayer->Height()} ;
            if (IsRectInRect (rcLayer, rcShadow))
            {
                *m_pRectOnLayer = rcShadow ;
                m_pLayer->GetSubBlock (&m_Undo, rcShadow) ;
            }
            else
            {
                m_pRectOnLayer = std::auto_ptr<RECT>() ;
                m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
                int   nExpL = (rcShadow.left >= 0) ? 0 : -rcShadow.left,
                      nExpT = (rcShadow.top >= 0) ? 0 : -rcShadow.top,
                      nExpR = (rcShadow.right > rcLayer.right) ? (rcShadow.right-rcLayer.right) : 0,
                      nExpB = (rcShadow.bottom > rcLayer.bottom) ? (rcShadow.bottom-rcLayer.bottom) : 0 ;
                m_pLayer->ExpandFrame (false, nExpL, nExpT, nExpR, nExpB) ;
                OffsetRect (&rcShadow, nExpL, nExpT) ;
            }

            // make shadow image
            FCCmdImgSetAlpha   cmdClear(0) ;
            static_cast<FCCmdArtPrider&>(cmdClear).Execute (canvas, 0) ; // clear region and combine later
            this->Implement (imgBlock, pProgress) ;
            m_pLayer->CombineImage (imgBlock, rcShadow.left, rcShadow.top) ;
        }
        if (pProgress)
            pProgress->SetProgress (100) ; // set 100%
    }

    virtual void Implement (FCObjImage& img, FCObjProgress* pProgress)
    {
        FCPixelAddShadow   aCmd(m_ShadowData) ;
        img.SinglePixelProcessProc(aCmd, pProgress) ;
    }
private:
    SHADOWDATA   m_ShadowData ;
};

//=============================================================================
/**
 *  Create image to mask.
 */
class FCCmdImageMask : public FCCmdImgEffect
{
    virtual void Implement (FCObjImage& img, FCObjProgress* pProgress)
    {
        FCObjImage   imgMask ;
        CreateMaskImage (img, imgMask, pProgress) ;
        img.CombineImage (imgMask) ;
    }
protected:
    /// Create 32bpp mask image as imgMask.
    virtual void CreateMaskImage (const FCObjImage& img, FCObjImage& imgMask, FCObjProgress* pProgress) =0 ;
};

//=============================================================================
/**
 *  Mask color on layer.
 */
class FCCmdMaskColor : public FCCmdImageMask
{
    class FCPixelCombineColor : public FCSinglePixelProcessBase
    {
        virtual bool ValidateColorBits (const FCObjImage* pImg)
        {
            return pImg->IsValidImage() && (pImg->ColorBits() == 32) ;
        }
        virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
        {
            RGBQUAD   * p = (RGBQUAD*)pPixel ;
            FCColor::CombineAlphaPixel (p, *p, &m_crFill, PCL_A(&m_crFill)) ;
        }
        RGBQUAD	  m_crFill ;
    public:
        FCPixelCombineColor (RGBQUAD crFill) : m_crFill(crFill) {}
    };

    virtual void CreateMaskImage (const FCObjImage& img, FCObjImage& imgMask, FCObjProgress* pProgress)
    {
        imgMask.Create (img.Width(), img.Height(), 32) ;
        
        FCPixelCombineColor   aCmd(m_crFill) ;
        imgMask.SinglePixelProcessProc (aCmd, pProgress) ;
    }
    RGBQUAD   m_crFill ;
public:
    /// Constructor (combine base on alpha of crFill).
    FCCmdMaskColor (RGBQUAD crFill) : m_crFill(crFill) {}
};

//=============================================================================
/**
 *  Base class of mask gradient on layer.
 */
class FCCmdMaskGradientBase : public FCCmdImageMask
{
    virtual void CreateMaskImage (const FCObjImage& img, FCObjImage& imgMask, FCObjProgress* pProgress)
    {
        imgMask.Create (img.Width(), img.Height(), 32) ;

        std::auto_ptr<FCPixelGradientBase>   m_pCmd (CreateGradientCommand (img, m_cr1, m_cr2, m_nRepeat)) ;
        imgMask.SinglePixelProcessProc (*m_pCmd, pProgress) ;
        imgMask.SetAlphaChannelValue (m_nAlpha) ;
    }

    RGBQUAD      m_cr1, m_cr2 ;
    REPEAT_MODE  m_nRepeat ;
    int          m_nAlpha ;

public:
    /// Constructor.
    FCCmdMaskGradientBase (int nAlpha, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : m_nAlpha(nAlpha), m_cr1(cr1), m_cr2(cr2), m_nRepeat(nRepeat) {}
    /// New a gradient fill command.
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) =0 ;
};

//=============================================================================
/**
 *  Mask linear gradient on layer.
 */
class FCCmdMaskGradientLine : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        POINT   pt1=m_pt1, pt2=m_pt2 ;
        img.Canvas_to_Layer (pt1) ;
        img.Canvas_to_Layer (pt2) ;
        return new FCPixelGradientLine (pt1, pt2, cr1, cr2, nRepeat) ;
    }
    POINT     m_pt1, m_pt2 ; // on canvas
public:
    /// Constructor.
    FCCmdMaskGradientLine (int nAlpha, POINT pt1, POINT pt2, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_pt1=pt1 ; m_pt2=pt2 ;
    }
};

//=============================================================================
/**
 *  Mask bilinear gradient on layer.
 */
class FCCmdMaskGradientBiLine : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        POINT   pt1=m_pt1, pt2=m_pt2 ;
        img.Canvas_to_Layer (pt1) ;
        img.Canvas_to_Layer (pt2) ;
        return new FCPixelGradientBiLine (pt1, pt2, cr1, cr2, nRepeat) ;
    }
    POINT     m_pt1, m_pt2 ; // on canvas
public:
    /// Constructor.
    FCCmdMaskGradientBiLine (int nAlpha, POINT pt1, POINT pt2, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_pt1=pt1 ; m_pt2=pt2 ;
    }
};

//=============================================================================
/**
 *  Mask symmetric conical gradient on layer.
 */
class FCCmdMaskGradientConicalSym : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        POINT   pt1=m_pt1, pt2=m_pt2 ;
        img.Canvas_to_Layer (pt1) ;
        img.Canvas_to_Layer (pt2) ;
        return new FCPixelGradientConicalSym (pt1, pt2, cr1, cr2, nRepeat) ;
    }
    POINT     m_pt1, m_pt2 ; // on canvas
public:
    /// Constructor.
    FCCmdMaskGradientConicalSym (int nAlpha, POINT pt1, POINT pt2, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_pt1=pt1 ; m_pt2=pt2 ;
    }
};

//=============================================================================
/**
 *  Mask anti-symmetric conical gradient on layer.
 */
class FCCmdMaskGradientConicalASym : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        POINT   pt1=m_pt1, pt2=m_pt2 ;
        img.Canvas_to_Layer (pt1) ;
        img.Canvas_to_Layer (pt2) ;
        return new FCPixelGradientConicalASym (pt1, pt2, cr1, cr2, nRepeat) ;
    }
    POINT     m_pt1, m_pt2 ; // on canvas
public:
    /// Constructor.
    FCCmdMaskGradientConicalASym (int nAlpha, POINT pt1, POINT pt2, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_pt1=pt1 ; m_pt2=pt2 ;
    }
};

//=============================================================================
/**
 *  Mask rectangle gradient on layer.
 */
class FCCmdMaskGradientRect : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        RECT    rc = m_rcOnCanvas ;
        img.Canvas_to_Layer (rc) ;
        return new FCPixelGradientRect (rc, cr1, cr2, nRepeat) ;
    }
    RECT      m_rcOnCanvas ;
public:
    /// Constructor.
    FCCmdMaskGradientRect (int nAlpha, RECT rcOnCanvas, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_rcOnCanvas = rcOnCanvas ;
    }
};

//=============================================================================
/**
 *  Mask radial gradient on layer.
 */
class FCCmdMaskGradientRadial : public FCCmdMaskGradientBase
{
    virtual FCPixelGradientBase* CreateGradientCommand (const FCObjImage& img, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat)
    {
        RECT    rc = m_rcOnCanvas ;
        img.Canvas_to_Layer (rc) ;
        return new FCPixelGradientRadial (rc, cr1, cr2, nRepeat) ;
    }
    RECT      m_rcOnCanvas ;
public:
    /// Constructor.
    FCCmdMaskGradientRadial (int nAlpha, RECT rcOnCanvas, RGBQUAD cr1, RGBQUAD cr2, REPEAT_MODE nRepeat) : FCCmdMaskGradientBase(nAlpha, cr1, cr2, nRepeat)
    {
        m_rcOnCanvas = rcOnCanvas ;
    }
};

//=============================================================================
/**
 *  Base class of mask line on layer.
 */
class FCCmdMaskLineBase : public FCCmdImageMask
{
    virtual void CreateMaskImage (const FCObjImage& img, FCObjImage& imgMask, FCObjProgress* pProgress)
    {
        FCObjImage     imgLine ;
        imgLine.Create (img.Width(), img.Height(), 24) ;
        MaskDrawLine (img, imgLine, m_nPenWidth, m_LineStyle) ;
        FCPixelConvertTo8BitGray   aCmd ;
        imgLine.SinglePixelProcessProc (aCmd) ;

        imgMask.Create (img.Width(), img.Height(), 32) ;
        for (int y=0 ; y < img.Height() ; y++)
            for (int x=0 ; x < img.Width() ; x++)
            {
                RGBQUAD     cr = m_cr ;
                PCL_A(&cr) = PCL_B(imgLine.GetBits(x,y)) * m_nAlpha / 0xFF ;
                *(RGBQUAD*)imgMask.GetBits(x,y) = cr ;
            }
    }

    int          m_nAlpha ;
    RGBQUAD      m_cr ;
    int          m_nPenWidth ;
    LINE_STYLE   m_LineStyle ;

public:
    /// Constructor.
    FCCmdMaskLineBase (int nAlpha, RGBQUAD cr, int nPenWidth, LINE_STYLE LineStyle) : m_nAlpha(nAlpha), m_cr(cr), m_nPenWidth(nPenWidth), m_LineStyle(LineStyle) {}
    /// Draw line on imgLine.
    virtual void MaskDrawLine (const FCObjImage& img, FCObjImage& imgLine, int nPenWidth, LINE_STYLE LineStyle) =0 ;
};

//=============================================================================
/**
 *  Auto enlarge layer to fill up canvas.
 */
class FCCmdAutoEnlargeLayer : public FCCmdArtPrider
{
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_pLayer = canvas.GetCurrentLayer() ;
        if (!m_pLayer || !m_pLayer->IsValidImage())
            return ;

        // not handle when limited
        if (m_pLayer->IsLayerLimited())
            return ;

        // enlarge to full up canvas
        RECT   rcLayer = {0, 0, m_pLayer->Width(), m_pLayer->Height()} ;
        m_pLayer->Layer_to_Canvas (rcLayer) ;

        SIZE   sizeCanvas = canvas.GetCanvasDimension() ;
        int    nExpL = (rcLayer.left <= 0) ? 0 : rcLayer.left,
               nExpT = (rcLayer.top <= 0) ? 0 : rcLayer.top,
               nExpR = (rcLayer.right < sizeCanvas.cx) ? (sizeCanvas.cx-rcLayer.right) : 0,
               nExpB = (rcLayer.bottom < sizeCanvas.cy) ? (sizeCanvas.cy-rcLayer.bottom) : 0 ;
        if (nExpL || nExpT || nExpR || nExpB)
        {
            m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
            m_pLayer->ExpandFrame (false, nExpL, nExpT, nExpR, nExpB) ;
        }
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        if (m_pLayer && m_Undo.IsValidImage())
        {
            m_Redo = *static_cast<FCObjImage*>(m_pLayer) ;
            *static_cast<FCObjImage*>(m_pLayer) = m_Undo ;
            m_Undo.Destroy() ;
        }
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        if (m_pLayer && m_Redo.IsValidImage())
        {
            m_Undo = *static_cast<FCObjImage*>(m_pLayer) ;
            *static_cast<FCObjImage*>(m_pLayer) = m_Redo ;
            m_Redo.Destroy() ;
        }
    }

    FCObjLayer   * m_pLayer ;
    FCObjImage   m_Undo ;
    FCObjImage   m_Redo ;

public:
    FCCmdAutoEnlargeLayer() : m_pLayer(0) {}
};

//=============================================================================
/**
 *  Base class to set canvas's region.
 */
class FCCmdSelectionSetBase : public FCCmdArtPrider
{
protected:
    /// Derived class put new selection into newSelect.
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect) =0 ;
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        this->OnCalculateNewSelection (canvas, m_Select) ;
        m_Select.SelectionOptimize() ;
        m_Select.RecalculateEdge (canvas.GetZoomScale(), true) ;
        this->Redo (canvas) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        this->Redo (canvas) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        FSwap (canvas.m_CurrSel, m_Select) ;
    }
private:
    FCObjSelect   m_Select ;
};

//=============================================================================
/**
 *  Invert current region.
 */
class FCCmdSelectionInvert : public FCCmdSelectionSetBase
{
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        const SIZE     sizeCanvas = canvas.GetCanvasDimension() ;
        if (!canvas.HasSelected() || (sizeCanvas.cx <= 0) || (sizeCanvas.cy <= 0))
        {
            assert(false); return;
        }

        // create selection & combine with old selection
        newSelect.Create (sizeCanvas.cx, sizeCanvas.cy, 8) ;
        const POINT     ptPos = canvas.GetSelection().GetGraphObjPos() ;
        newSelect.CoverBlock (canvas.GetSelection(), ptPos.x, ptPos.y) ;

        // start invert operation
        for (int y=0 ; y < newSelect.Height() ; y++)
            for (int x=0 ; x < newSelect.Width() ; x++)
            {
                BYTE   * p = newSelect.GetBits (x,y) ;
                *p = ~*p ;
            }

        // position
        newSelect.SetGraphObjPos (0, 0) ;
    }
};

//=============================================================================
/**
 *  Select all canvas.
 */
class FCCmdSelectionCanvasAll : public FCCmdSelectionSetBase
{
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        const SIZE     sizeCanvas = canvas.GetCanvasDimension() ;
        if ((sizeCanvas.cx <= 0) || (sizeCanvas.cy <= 0))
        {
            assert(false); return;
        }

        // create selection
        newSelect.Create (sizeCanvas.cx, sizeCanvas.cy, 8) ;
        memset (newSelect.GetMemStart(), 0xFF, newSelect.GetPitch()*newSelect.Height()) ;
        newSelect.SetGraphObjPos (0, 0) ;
    }
};

//=============================================================================
/**
 *  Clear selection.
 */
class FCCmdSelectionClear : public FCCmdSelectionSetBase
{
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        newSelect = FCObjSelect() ;
    }
};

//=============================================================================
/**
 *  Smooth selection.
 */
class FCCmdSelectionSmooth : public FCCmdSelectionSetBase
{
public:
    /// Constructor.
    FCCmdSelectionSmooth (int nStep) : m_nStep(nStep) {}
private:
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        newSelect = canvas.GetSelection() ;
        if (!newSelect.HasSelected())
        {
            assert(false); return;
        }

        // smooth
        newSelect.ConvertTo24Bit() ;
        FCPixelBlur_Box   cmdSmooth (m_nStep, false) ;
        newSelect.SinglePixelProcessProc (cmdSmooth) ;

        FCPixelConvertTo8BitGray   aCmd ;
        newSelect.SinglePixelProcessProc(aCmd) ;

        // convert to 2-state
        for (int y=0 ; y < newSelect.Height() ; y++)
            for (int x=0 ; x < newSelect.Width() ; x++)
            {
                BYTE   * p = newSelect.GetBits (x,y) ;
                *p = ((*p < 128) ? 0 : 0xFF) ;
            }
    }
private:
    int     m_nStep ;
};

//=============================================================================
/**
 *  Base class of blend halo.
 */
class FCCmdSelectionBlendHaloBase : public FCCmdSelectionSetBase
{
protected:
    /// Constructor.
    FCCmdSelectionBlendHaloBase (int nPixel) : m_nPixel(nPixel) {}
    void BlendEdgeHalo (FCObjSelect& newSelect, LOGICAL_OP logOp, int nHaloValue, bool bClear)
    {
        if (!newSelect.HasSelected() || (m_nPixel <= 0))
        {
            assert(false) ; return ;
        }

        newSelect.ExpandFrame (false, m_nPixel, m_nPixel, m_nPixel, m_nPixel) ;

        // find edge point
        std::deque<POINT>     ptList ;
        GetEdgePointList (newSelect, ptList) ;

        // clear image
        if (bClear)
            memset (newSelect.GetMemStart(), 0, newSelect.GetPitch()*newSelect.Height()) ;

        // create RECT halo (round halo better ?)
        FCObjImage     imgMask (2*m_nPixel+1, 2*m_nPixel+1, 8) ;
        memset (imgMask.GetMemStart(), nHaloValue, imgMask.GetPitch()*imgMask.Height()) ;

        // blend halo
        for (size_t i=0 ; i < ptList.size() ; i++)
        {
            newSelect.LogicalBlend (imgMask, logOp, ptList[i].x-m_nPixel, ptList[i].y-m_nPixel) ;
        }
    }
private:
    // find the points of edge
    static void GetEdgePointList (const FCObjSelect& sel, std::deque<POINT>& ptList)
    {
        ptList.clear() ;

        // is edge point : current is selected around is non-selected
        // brim point must be edge point
        for (int y=0 ; y < sel.Height() ; y++)
            for (int x=0 ; x < sel.Width() ; x++)
                if (*sel.GetBits(x,y) == 0xFF)
                    if ((y == 0)              || (*sel.GetBits(x,y-1) == 0) || // up
                        (y == sel.Height()-1) || (*sel.GetBits(x,y+1) == 0) || // down
                        (x == 0)              || (*sel.GetBits(x-1,y) == 0) || // left
                        (x == sel.Width()-1)  || (*sel.GetBits(x+1,y) == 0) || // right
                        (*sel.GetBits(x+1,y-1) == 0) || // right-up
                        (*sel.GetBits(x+1,y+1) == 0) || // right-down
                        (*sel.GetBits(x-1,y+1) == 0) || // left-down
                        (*sel.GetBits(x-1,y-1) == 0)) // left-up
                    {
                        POINT   pt = {x,y} ;
                        ptList.push_back (pt) ;
                    }
    }
private:
    int     m_nPixel ;
};

//=============================================================================
/**
 *  Expand selection.
 */
class FCCmdSelectionExpand : public FCCmdSelectionBlendHaloBase
{
public:
    /// Constructor.
    FCCmdSelectionExpand (int nPixel) : FCCmdSelectionBlendHaloBase(nPixel) {}
private:
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        newSelect = canvas.GetSelection() ;
        BlendEdgeHalo (newSelect, LOGI_OR, 0xFF, false) ;
    }
};

//=============================================================================
/**
 *  Shrink selection.
 */
class FCCmdSelectionShrink : public FCCmdSelectionBlendHaloBase
{
public:
    /// Constructor.
    FCCmdSelectionShrink (int nPixel) : FCCmdSelectionBlendHaloBase(nPixel) {}
private:
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        newSelect = canvas.GetSelection() ;
        BlendEdgeHalo (newSelect, LOGI_AND, 0, false) ;
    }
};

//=============================================================================
/**
 *  Border selection.
 */
class FCCmdSelectionBorder : public FCCmdSelectionBlendHaloBase
{
public:
    /// Constructor.
    FCCmdSelectionBorder (int nPixel) : FCCmdSelectionBlendHaloBase(nPixel) {}
private:
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        newSelect = canvas.GetSelection() ;
        BlendEdgeHalo (newSelect, LOGI_OR, 0xFF, true) ;
    }
};

//=============================================================================
/**
 *  Base class to handle selection.
 */
class FCCmdSelectionCreateBase : public FCCmdSelectionSetBase
{
protected:
    /// Constructor.
    FCCmdSelectionCreateBase (FCObjSelect::RGN_TYPE nType) : m_nType(nType) {}
    virtual void CreateSelection (FCObjCanvas& canvas, FCObjSelect& selCreate) =0 ;
private:
    FCObjSelect::RGN_TYPE   m_nType ;
private:
    virtual void OnCalculateNewSelection (FCObjCanvas& canvas, FCObjSelect& newSelect)
    {
        if (!canvas.HasSelected() || (m_nType == FCObjSelect::RGN_CREATE))
        {
            // create new create
            CreateSelection (canvas, newSelect) ;
        }
        else
        {
            FCObjSelect     selCreate ;
            CreateSelection (canvas, selCreate) ;
            newSelect = canvas.GetSelection() ;        
            if (m_nType == FCObjSelect::RGN_ADD)
                AddSelection (newSelect, selCreate) ;
            else if (m_nType == FCObjSelect::RGN_SUB)
                SubSelection (newSelect, selCreate) ;
        }
    }
    void AddSelection (FCObjSelect& selCurr, const FCObjSelect& sel)
    {
        if (!selCurr.HasSelected() || !sel.HasSelected())
            {assert(false); return;}

        // selection's canvas position 
        RECT     rcSel = {0, 0, sel.Width(), sel.Height()},
                 rcCurr = {0, 0, selCurr.Width(), selCurr.Height()} ;
        selCurr.Layer_to_Canvas (rcCurr) ;
        sel.Layer_to_Canvas (rcSel) ;

        // merge
        if (!::IsRectInRect (rcCurr, rcSel))
        {
            // we need create new bitmap
            RECT        rcDest ;
            ::UnionRect (&rcDest, &rcSel, &rcCurr) ;
            if (IsRectEmpty(&rcDest))
                {assert(false); return;}

            FCObjImage     imgOld (selCurr) ;
            selCurr.Create (RECTWIDTH(rcDest), RECTHEIGHT(rcDest), 8) ;
            selCurr.CoverBlock (imgOld, rcCurr.left-rcDest.left, rcCurr.top-rcDest.top) ; // 覆盖原图
            selCurr.SetGraphObjPos (rcDest.left, rcDest.top) ;
            rcCurr = rcDest ; // rcCurr now put rect of new selection on canvas
        }

        // cover sel
        selCurr.LogicalBlend (sel, LOGI_SEL_ADD, rcSel.left - rcCurr.left,
                                                 rcSel.top - rcCurr.top) ;
    }
    void SubSelection (FCObjSelect& selCurr, const FCObjSelect& sel)
    {
        if ((&selCurr == &sel) || !selCurr.HasSelected() || !sel.HasSelected())
            {assert(false); return;}

        // selection's canvas position 
        RECT     rcSel = {0, 0, sel.Width(), sel.Height()} ,
                 rcCurr = {0, 0, selCurr.Width(), selCurr.Height()} ;
        selCurr.Layer_to_Canvas (rcCurr) ;
        sel.Layer_to_Canvas (rcSel) ;

        // cover sel
        selCurr.LogicalBlend (sel, LOGI_SEL_SUB, rcSel.left - rcCurr.left,
                                                 rcSel.top - rcCurr.top) ;
    }
};

//=============================================================================
/**
 *  Rectangle region.
 */
class FCCmdSelectionCreateRect : public FCCmdSelectionCreateBase
{
public:
    /// Constructor.
    /// @param rcRect : rect on canvas.
    FCCmdSelectionCreateRect (RECT rcRect, FCObjSelect::RGN_TYPE nType) : m_rcRect(rcRect), FCCmdSelectionCreateBase(nType) {}
private:
    virtual void CreateSelection (FCObjCanvas& canvas, FCObjSelect& selCreate)
    {
        // ::NormalizeRect (m_rcRect) ;
        if (::IsRectEmpty(&m_rcRect))
            return ;

        // create new rect region
        selCreate.Create (RECTWIDTH(m_rcRect), RECTHEIGHT(m_rcRect), 8) ;
        memset (selCreate.GetMemStart(), 0xFF, selCreate.GetPitch()*selCreate.Height()) ;
        selCreate.SetGraphObjPos (m_rcRect.left, m_rcRect.top) ;
    }
private:
    RECT     m_rcRect ;
};

//=============================================================================
/**
 *  Ellipse region.
 */
class FCCmdSelectionCreateEllipse : public FCCmdSelectionCreateBase
{
public:
    /// Constructor.
    /// @param rcRect : rect on canvas.
    FCCmdSelectionCreateEllipse (RECT rcEllipse, FCObjSelect::RGN_TYPE nType) : m_rcEllipse(rcEllipse), FCCmdSelectionCreateBase(nType) {}
private:
    virtual void CreateSelection (FCObjCanvas& canvas, FCObjSelect& selCreate)
    {
        // ::NormalizeRect (m_rcEllipse) ;
        if (::IsRectEmpty(&m_rcEllipse))
            return ;

        // create new elliptic region
        selCreate.Create (RECTWIDTH(m_rcEllipse), RECTHEIGHT(m_rcEllipse), 8) ;
        double  fCenX = selCreate.Width() / 2.0,
                fCenY = selCreate.Height() / 2.0 ;
        for (int y=0 ; y < selCreate.Height() ; y++)
            for (int x=0 ; x < selCreate.Width() ; x++)
            {
                double   fX = (x+0.5-fCenX) / fCenX,
                         fY = (y+0.5-fCenY) / fCenY ;
                *selCreate.GetBits(x,y) = (FHypot(fX, fY) > 1.0) ? 0 : 0xFF ;
            }
        selCreate.SetGraphObjPos (m_rcEllipse.left, m_rcEllipse.top) ;
    }
private:
    RECT     m_rcEllipse ;
};

//=============================================================================
/**
 *  Polygon region.
 */
class FCCmdSelectionCreatePolygonBase : public FCCmdSelectionCreateBase
{
public:
    /// Constructor.
    /// @param PointList : point on canvas.
    FCCmdSelectionCreatePolygonBase (std::deque<POINT> PointList, FCObjSelect::RGN_TYPE nType) : FCCmdSelectionCreateBase(nType), m_PointList(PointList) {}
protected:
    /// @param img : is a 24bpp image.
    /// @param ppt : point on img.
    virtual void ImplementDrawPolygon (FCObjImage& img, const POINT* ppt, size_t cNum) =0 ;
private:
    RECT FindPolygonBoundRect() const
    {
        RECT   rc ;
        rc.left = rc.top = 0x7FFFFFFF ;
        rc.right = rc.bottom = -0x7FFFFFFF ;
        for (size_t i=0 ; i < m_PointList.size() ; i++)
        {
            if (m_PointList[i].x < rc.left)    rc.left = m_PointList[i].x ;
            if (m_PointList[i].x > rc.right)   rc.right = m_PointList[i].x ;
            if (m_PointList[i].y < rc.top)     rc.top = m_PointList[i].y ;
            if (m_PointList[i].y > rc.bottom)  rc.bottom = m_PointList[i].y ;
        }
        return rc ;
    }
    virtual void CreateSelection (FCObjCanvas& canvas, FCObjSelect& selCreate)
    {
        const int          nNum = (int)m_PointList.size() ;
        PCL_array<POINT>   ppt (nNum) ;
        if (nNum)
            for (int i=0 ; i < nNum ; i++)
                ppt[i] = m_PointList[i] ;

        RECT     rcBound = FindPolygonBoundRect() ;
        if (::IsRectEmpty(&rcBound))
            return ;

        // create new polygon region
        FCObjImage   img ;
        img.Create (RECTWIDTH(rcBound), RECTHEIGHT(rcBound), 24) ;

        // draw polygon
        for (int i=0 ; i < nNum ; i++)
        {
            ppt[i].x = ppt[i].x - rcBound.left ;
            ppt[i].y = ppt[i].y - rcBound.top ;
        }
        ImplementDrawPolygon (img, ppt.get(), nNum) ;

        selCreate.Create (img.Width(), img.Height(), 8) ;
        for (int y=0 ; y < img.Height() ; y++)
            for (int x=0 ; x < img.Width() ; x++)
                *selCreate.GetBits(x,y) = PCL_B(img.GetBits(x,y)) ;
        selCreate.SetGraphObjPos (rcBound.left, rcBound.top) ;
    }
private:
    std::deque<POINT>   m_PointList ;
};

//=============================================================================
/**
 *  Magic wand selection.
 */
class FCCmdSelectionCreateMagicWand : public FCCmdSelectionCreateBase
{
public:
    /// Constructor.
    FCCmdSelectionCreateMagicWand (POINT ptCanvas, int nTolerance, bool bContinuous, FCObjSelect::RGN_TYPE nType) : FCCmdSelectionCreateBase(nType)
    {
        m_ptCanvas = ptCanvas ;
        m_nTolerance = nTolerance ;
        m_bContinuous = bContinuous ;
    }
private:
    virtual void CreateSelection (FCObjCanvas& canvas, FCObjSelect& selCreate)
    {
        FCObjLayer   * pLayer = canvas.GetCurrentLayer() ;
        if (!pLayer)
            {assert(false); return;}

        POINT     ptLayer = m_ptCanvas ;
        pLayer->Canvas_to_Layer (ptLayer) ;
        if (!pLayer->IsInside (ptLayer.x, ptLayer.y)) // click point out of current layer
        {
            // select region that alpha==0
            const SIZE     sizeCanvas = canvas.GetCanvasDimension() ;
            // create selection
            selCreate.Create (sizeCanvas.cx, sizeCanvas.cy, 8) ;
            selCreate.SetGraphObjPos (0, 0) ;
            for (int y=0 ; y < selCreate.Height() ; y++)
                for (int x=0 ; x < selCreate.Width() ; x++)
                {
                    POINT     ptCur = {x, y} ;
                    pLayer->Canvas_to_Layer (ptCur) ;
                    if (!pLayer->IsInside (ptCur.x,ptCur.y) || !PCL_A(pLayer->GetBits(ptCur.x,ptCur.y)))
                        *selCreate.GetBits(x,y) = 0xFF ;
                }
        }
        else
            if (m_bContinuous)
            {
                SeedFillMask (ptLayer, m_nTolerance, *pLayer, selCreate) ;
            }
            else
            {
                selCreate.Create (pLayer->Width(), pLayer->Height(), 8) ;
                selCreate.SetGraphObjPos (pLayer->GetGraphObjPos()) ;
                const RGBQUAD     crClick = *(RGBQUAD*)pLayer->GetBits (ptLayer.x, ptLayer.y) ; // color of click point
                for (int y=0 ; y < pLayer->Height() ; y++)
                    for (int x=0 ; x < pLayer->Width() ; x++)
                        if (IsInTolerance (pLayer->GetBits(x,y), crClick, m_nTolerance))
                            *selCreate.GetBits(x,y) = 0xFF ;
            }
    }
    static bool IsInTolerance (const void* pCurr, RGBQUAD crOrigin, int nTolerance)
    {
        return (abs(PCL_B(pCurr) - PCL_B(&crOrigin)) <= nTolerance) &&
               (abs(PCL_G(pCurr) - PCL_G(&crOrigin)) <= nTolerance) &&
               (abs(PCL_R(pCurr) - PCL_R(&crOrigin)) <= nTolerance) &&
               (PCL_A(pCurr) == PCL_A(&crOrigin)) ;
    }
    static void SeedFillMask (POINT ptImg, int nTolerance, const FCObjImage& img, FCObjSelect& sel)
    {
        if (!img.IsInside (ptImg.x, ptImg.y))
            return ;

        // color of click
        const RGBQUAD   crOrigin = *(RGBQUAD*)img.GetBits (ptImg.x, ptImg.y) ;
        sel.Create (img.Width(), img.Height(), 8) ; // same size
        sel.SetGraphObjPos (img.GetGraphObjPos()) ; // canvas's coordinate

        std::deque<POINT>   ptUnfilled ;
        POINT               ptCurr = {ptImg.x, ptImg.y} ;
        ptUnfilled.push_back (ptCurr) ;
        while (!ptUnfilled.empty())
        {
            ptCurr = ptUnfilled.back() ; ptUnfilled.pop_back() ;
            *sel.GetBits (ptCurr.x, ptCurr.y) = 0xFF ; // set filled

            // up
            if ((ptCurr.y > 0) && (*sel.GetBits (ptCurr.x, ptCurr.y-1) == 0) &&
                IsInTolerance (img.GetBits (ptCurr.x, ptCurr.y-1), crOrigin, nTolerance))
            {
                POINT     pt = {ptCurr.x, ptCurr.y-1} ;
                ptUnfilled.push_back (pt) ;
            }
            // right
            if ((ptCurr.x < img.Width()-1) && (*sel.GetBits (ptCurr.x+1, ptCurr.y) == 0) &&
                IsInTolerance (img.GetBits (ptCurr.x+1, ptCurr.y), crOrigin, nTolerance))
            {
                POINT     pt = {ptCurr.x+1, ptCurr.y} ;
                ptUnfilled.push_back (pt) ;
            }
            // bottom
            if ((ptCurr.y < img.Height()-1) && (*sel.GetBits (ptCurr.x, ptCurr.y+1) == 0) &&
                IsInTolerance (img.GetBits (ptCurr.x, ptCurr.y+1), crOrigin, nTolerance))
            {
                POINT     pt = {ptCurr.x, ptCurr.y+1} ;
                ptUnfilled.push_back (pt) ;
            }
            // left
            if ((ptCurr.x > 0) && (*sel.GetBits (ptCurr.x-1, ptCurr.y) == 0) &&
                IsInTolerance (img.GetBits (ptCurr.x-1, ptCurr.y), crOrigin, nTolerance))
            {
                POINT     pt = {ptCurr.x-1, ptCurr.y} ;
                ptUnfilled.push_back (pt) ;
            }
        }
    }
private:
    POINT     m_ptCanvas ;
    int       m_nTolerance ;
    bool      m_bContinuous ;
};

//=============================================================================
/**
 *  Handle layer.
 */
class FCCmdLayerOperation : public FCCmdArtPrider
{
protected:
    /// Constructor.
    FCCmdLayerOperation (FCObjLayer* pLayer) : m_pLayer(pLayer) {}
    /// Process layer.
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress) =0 ;
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        if (!m_pLayer)
            {assert(false); return;}

        m_UndoLayer = *m_pLayer ;
        m_pUndoMemo = std::auto_ptr<FCMemoLayer>(m_pLayer->CreateMemoObj()) ;
        this->ProcessLayer (canvas, *m_pLayer, pProgress) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        if (!m_pLayer)
            {assert(false); return;}

        m_RedoLayer = *m_pLayer ;
        m_pRedoMemo = std::auto_ptr<FCMemoLayer>(m_pLayer->CreateMemoObj()) ;
        *m_pLayer = m_UndoLayer ;
        m_pLayer->SetMemoObj (m_pUndoMemo.get()) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        if (!m_pLayer)
            {assert(false); return;}

        m_UndoLayer = *m_pLayer ;
        m_pUndoMemo = std::auto_ptr<FCMemoLayer>(m_pLayer->CreateMemoObj()) ;
        *m_pLayer = m_RedoLayer ;
        m_pLayer->SetMemoObj (m_pRedoMemo.get()) ;
    }
private:
    FCObjLayer     * m_pLayer ;
    FCObjLayer     m_UndoLayer, m_RedoLayer ;
    std::auto_ptr<FCMemoLayer>     m_pUndoMemo, m_pRedoMemo ;
};

//=============================================================================
/**
 *  Stretch layer.
 */
class FCCmdLayerStretch : public FCCmdLayerOperation
{
public:
    /// Constructor.
    FCCmdLayerStretch (FCObjLayer* pLayer, int nNewW, int nNewH, INTERPOLATION_TYPE InterplType) : FCCmdLayerOperation(pLayer)
    {
        m_nNewWidth = nNewW ;
        m_nNewHeight = nNewH ;
        m_InterplType = InterplType ;
    }
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        // don't change layer's position
        const POINT     ptLayer = rLayer.GetGraphObjPos() ;
        switch (m_InterplType)
        {
            case INTERPOLATION_NONE : rLayer.Stretch (m_nNewWidth, m_nNewHeight) ; break ;
            case INTERPOLATION_BILINEAR : rLayer.Stretch_Smooth (m_nNewWidth, m_nNewHeight, pProgress) ; break ;
            default : assert(false) ;
        }
        rLayer.SetGraphObjPos (ptLayer) ;
    }
private:
    int      m_nNewWidth ;
    int      m_nNewHeight ;
    INTERPOLATION_TYPE   m_InterplType ;
};

//=============================================================================
/**
 *  Rotate layer.
 */
class FCCmdLayerRotate : public FCCmdLayerOperation
{
public:
    /// Constructor.
    /// @param nAngle : 0 -- 360
    FCCmdLayerRotate (FCObjLayer* pLayer, int nAngle) : FCCmdLayerOperation(pLayer)
    {
        m_nAngle = nAngle ;
    }
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        // the central point is fixed in rotate
        POINT     ptCenter = rLayer.GetGraphObjPos() ;
        ptCenter.x += rLayer.Width()/2 ; ptCenter.y += rLayer.Height()/2 ;

        // sawtooth :-(
        FCPixelRotate     aCmd(m_nAngle) ;
        rLayer.SinglePixelProcessProc (aCmd, pProgress) ;

        // calculate the position of rotated layer
        ptCenter.x -= rLayer.Width()/2 ; ptCenter.y -= rLayer.Height()/2 ;
        rLayer.SetGraphObjPos (ptCenter) ;
    }
private:
    int     m_nAngle ;
};

//=============================================================================
/**
 *  Clockwise rotate 90'.
 */
class FCCmdLayerRotate90 : public FCCmdLayerOperation
{
public:
    /// Constructor.
    FCCmdLayerRotate90 (FCObjLayer* pLayer) : FCCmdLayerOperation(pLayer) {}
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        // the central point is fixed in rotate
        POINT     ptCenter = rLayer.GetGraphObjPos() ;
        ptCenter.x += rLayer.Width()/2 ; ptCenter.y += rLayer.Height()/2 ;

        FCPixelRotate90     cmdRotateImg ;
        rLayer.SinglePixelProcessProc (cmdRotateImg, pProgress) ;

        // calculate the position of rotated layer
        ptCenter.x -= rLayer.Width()/2 ; ptCenter.y -= rLayer.Height()/2 ;
        rLayer.SetGraphObjPos (ptCenter) ;
    }
};

//=============================================================================
/**
 *  Clockwise rotate 270'.
 */
class FCCmdLayerRotate270 : public FCCmdLayerOperation
{
public:
    /// Constructor.
    FCCmdLayerRotate270 (FCObjLayer* pLayer) : FCCmdLayerOperation(pLayer) {}
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        // the central point is fixed in rotate
        POINT    ptCenter = rLayer.GetGraphObjPos() ;
        ptCenter.x += rLayer.Width()/2 ; ptCenter.y += rLayer.Height()/2 ;

        FCPixelRotate270     cmdRotateImg ;
        rLayer.SinglePixelProcessProc (cmdRotateImg, pProgress) ;

        // calculate the position of rotated layer
        ptCenter.x -= rLayer.Width()/2 ; ptCenter.y -= rLayer.Height()/2 ;
        rLayer.SetGraphObjPos (ptCenter) ;
    }
};

//=============================================================================
/**
 *  Skew transform.
 */
class FCCmdLayerSkew : public FCCmdLayerOperation
{
public:
    /// Constructor.
    /// @param ptPos : new position of left-up, right-up, right-down, left-down point.
    FCCmdLayerSkew (FCObjLayer* pLayer, const POINT ptPos[4]) : FCCmdLayerOperation(pLayer)
    {
        memcpy (m_ptNewPos, ptPos, sizeof(POINT) * 4) ;
    }
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        FCPixelSkew     aCmd (m_ptNewPos) ;
        rLayer.SinglePixelProcessProc (aCmd, pProgress) ;
    }
private:
    POINT     m_ptNewPos[4] ; // left-up, right-up, right-down, left-down
};

//=============================================================================
/**
 *  Lens transform.
 */
class FCCmdLayerLens : public FCCmdLayerOperation
{
public:
    /// Constructor.
    /// @param ptPos : new position of left-up, right-up, right-down, left-down point.
    FCCmdLayerLens (FCObjLayer* pLayer, const POINT ptPos[4]) : FCCmdLayerOperation(pLayer)
    {
        memcpy (m_ptNewPos, ptPos, sizeof(POINT) * 4) ;
    }
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        FCPixelPerspective     aCmd (m_ptNewPos) ;
        rLayer.SinglePixelProcessProc (aCmd, pProgress) ;
    }
private:
    POINT     m_ptNewPos[4] ; // left-up, right-up, right-down, left-down
};

//=============================================================================
/**
 *  Crop layer.
 */
class FCCmdLayerCrop : public FCCmdLayerOperation
{
public:
    /// Constructor.
    FCCmdLayerCrop (FCObjLayer* pLayer, FCObjImage* pImgMask, RECT rcLayer, int nPercent) : FCCmdLayerOperation(pLayer), m_pImgMask(pImgMask)
    {
        m_rcLayer = rcLayer ;
        m_nPercent = nPercent ;
    }
private:
    virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
    {
        if (!m_pImgMask.get() || (m_pImgMask->ColorBits() != 24))
            {assert(false); return;}

        const POINT   ptOldImg = rLayer.GetGraphObjPos() ;

        // get selected region
        FCObjImage     imgBlock ;
        rLayer.GetSubBlock (&imgBlock, m_rcLayer) ;
        if (!rLayer.IsValidImage())
            {assert(false); return;}

        m_pImgMask->Stretch_Smooth (imgBlock.Width(), imgBlock.Height()) ;

        // process
        for (int y=0 ; y < imgBlock.Height() ; y++)
            for (int x=0 ; x < imgBlock.Width() ; x++)
            {
                int   nGray = 0xFF - FCColor::GetGrayscale (m_pImgMask->GetBits(x,y)) ;
                BYTE  * p = imgBlock.GetBits(x,y) ;
                PCL_A(p) = PCL_A(p) * nGray * (100-m_nPercent) / 0xFF / 100 ;
            }

        static_cast<FCObjImage&>(rLayer) = imgBlock ;
        rLayer.SetGraphObjPos (ptOldImg.x + m_rcLayer.left, ptOldImg.y + m_rcLayer.top) ;
    }
private:
    std::auto_ptr<FCObjImage>   m_pImgMask ; // 24-bit mask image
    RECT         m_rcLayer ; // RECT on layer
    int          m_nPercent ;
};

//=============================================================================
/**
 *  Move graph object.
 */
class FCCmdLayerMove : public FCCmdArtPrider
{
public:
    /// Constructor.
    FCCmdLayerMove (FCObjGraph* pGraph, POINT NewPoint) : m_pGraph(pGraph), m_ptOld(NewPoint) {}
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        this->Redo(canvas) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        this->Redo(canvas) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        if (!m_pGraph)
            {assert(false); return;}

        const POINT     pt = m_pGraph->GetGraphObjPos() ;
        m_pGraph->SetGraphObjPos (m_ptOld) ;
        m_ptOld = pt ;
    }
private:
    POINT       m_ptOld ;
    FCObjGraph  * m_pGraph ;
};

//=============================================================================
/**
 *  Insert a new layer into canvas.
 */
class FCCmdLayerAdd : public FCCmdArtPrider
{
public:
    /**
     *  Insert a new layer into canvas (<B>after be added, you can't delete the layer any more</B>).
     *  @param nPos : the pAddLayer's 0-base index in canvas.
     */
    FCCmdLayerAdd (FCObjLayer* pAddLayer, int nPos)
    {
        m_pAddLayer = pAddLayer ; assert(pAddLayer);
        m_nPos = nPos ;
        m_pCanvas = 0 ;
        // why we need this flag : think about this scene
        // 1)undo level set(2)  2)add a layer  3)merge layer include it  4)add a layer  5)undo
        // the step 2, first layer will be removed, but the step 3 hold its pointer
        m_bRemoved = false ;
    }
    virtual ~FCCmdLayerAdd()
    {
        // 其实图层remove时就该被删除，只是为了节省内存，才暂时放到remove列表中
        if (m_bRemoved)
            m_pCanvas->DeleteLayerFromRemoveList (m_pAddLayer) ;
    }
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_pCanvas = &canvas ;
        this->Redo(canvas) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        m_bRemoved = true ;
        canvas.RemoveLayer (m_pAddLayer) ; // not delete
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        m_bRemoved = false ;
        canvas.AddLayer (m_pAddLayer, m_nPos) ;
    }
private:
    int          m_nPos ;
    FCObjLayer   * m_pAddLayer ;
    FCObjCanvas  * m_pCanvas ;
    bool         m_bRemoved ;
};

//=============================================================================
/**
 *  Remove a layer from canvas.
 */
class FCCmdLayerRemove : public FCCmdArtPrider
{
public:
    /// Remove pRemoveLayer from canvas.
    FCCmdLayerRemove (FCObjLayer* pRemoveLayer)
    {
        m_pRemoveLayer = pRemoveLayer; assert(pRemoveLayer);
        m_pCanvas = 0 ;
        m_bRemoved = true ;
    }
    virtual ~FCCmdLayerRemove()
    {
        // 其实图层remove时就该被删除，只是为了节省内存，才暂时放到remove列表中
        if (m_bRemoved)
            m_pCanvas->DeleteLayerFromRemoveList (m_pRemoveLayer) ;
    }
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_pCanvas = &canvas ;
        this->Redo(canvas) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        m_bRemoved = false ;
        canvas.AddLayer (m_pRemoveLayer, m_nPos) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        m_bRemoved = true ;
        m_nPos = canvas.FindLayer(m_pRemoveLayer) ;
        canvas.RemoveLayer (m_pRemoveLayer) ; // not delete
    }
private:
    int         m_nPos ;
    FCObjLayer  * m_pRemoveLayer ;
    FCObjCanvas * m_pCanvas ;
    bool        m_bRemoved ;
};

//=============================================================================
/**
 *  Save layer's current property.
 */
class FCCmdLayerSaveProperty : public FCCmdArtPrider
{
public:
    /// Constructor (save layer's current property when constructor).
    FCCmdLayerSaveProperty (FCObjLayer* pLayer)
    {
        m_pLayer = pLayer ;
        m_pMemo = m_pLayer ? m_pLayer->CreateMemoObj() : 0 ;
    }
    virtual ~FCCmdLayerSaveProperty() {if(m_pMemo)  delete m_pMemo;}
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress) {}
    virtual void Undo (FCObjCanvas& canvas)
    {
        this->Redo(canvas) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        if (m_pLayer && m_pMemo)
        {
            FCMemoLayer   * pCurr = m_pLayer->CreateMemoObj() ;
            m_pLayer->SetMemoObj (m_pMemo) ;
            delete m_pMemo ;
            m_pMemo = pCurr ;
        }
        else
            {assert(false);}
    }
private:
    FCMemoLayer  * m_pMemo ;
    FCObjLayer   * m_pLayer ;
};

//=============================================================================
/**
 *  Change layer's sequence in canvas.
 */
class FCCmdLayerExchange : public FCCmdArtPrider
{
public:
    FCCmdLayerExchange (std::deque<FCObjLayer*> NewList) : m_LayerList(NewList) {}
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        this->Redo(canvas) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        this->Redo(canvas) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        FSwap (canvas.m_LayerList, m_LayerList) ;
    }
private:
    std::deque<FCObjLayer*>   m_LayerList ;
};

//=============================================================================
/**
 *  Merge layers.
 */
class FCCmdLayerMerge : public FCCmdComposite
{
public:
    /// Constructor.
    /// @param IndexList : list of layer's index.
    FCCmdLayerMerge (std::deque<int> IndexList)
    {
        if (IndexList.empty())
            {assert(false); return;}

        int     nNum = (int)IndexList.size(), i ;
        PCL_array<int>   arr(nNum) ;
        for (i=0 ; i < nNum ; i++)
            arr[i] = IndexList[i] ;

        // sort by small to big, delete layer from big
        FCOXOHelper::BubbleSort (arr.get(), nNum) ;

        for (i=0 ; i < nNum ; i++)
            m_IndexList.push_back (arr[i]) ;
    }
private:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        if (m_IndexList.empty())
            {assert(false); return;}

        // calculate merged layer size
        RECT      rcNewLayer = {0,0,0,0} ;
        int       i ;
        for (i=0 ; i < (int)m_IndexList.size() ; i++)
        {
            FCObjLayer   * pLayer = canvas.GetLayer(m_IndexList[i]) ; assert(pLayer);
            if (!pLayer)
                continue ;

            // layer's RECT
            RECT      rcLayer = {0, 0, pLayer->Width(), pLayer->Height()} ;
            POINT     pt = pLayer->GetGraphObjPos() ;
            OffsetRect (&rcLayer, pt.x, pt.y) ;
            if (i == 0)
                rcNewLayer = rcLayer ;
            else
                UnionRect (&rcNewLayer, &rcNewLayer, &rcLayer) ;
        }

        // make new merged layer
        FCObjLayer   * pNewLayer = new FCObjLayer ;
        if (!pNewLayer->Create (RECTWIDTH(rcNewLayer), RECTHEIGHT(rcNewLayer), 32))
        {
            delete pNewLayer ;
            assert(false); return;
        }

        // merge layers (from bottom to top)
        for (i=0 ; i < (int)m_IndexList.size() ; i++)
        {
            FCObjLayer   * pLayer = canvas.GetLayer(m_IndexList[i]) ; assert(pLayer);
            if (!pLayer)
                continue ;

            // position at new layer
            pNewLayer->CombineImage (*pLayer, 
                                     pLayer->GetGraphObjPos().x - rcNewLayer.left,
                                     pLayer->GetGraphObjPos().y - rcNewLayer.top,
                                     pLayer->GetLayerTransparent()) ;
            if (pProgress)
                pProgress->SetProgress (100 * (i + 1) / (int)m_IndexList.size()) ;
        }
        // set new merged layer's position
        pNewLayer->SetGraphObjPos (rcNewLayer.left, rcNewLayer.top) ;

        // insert new layer at min index
        PushImgCommand (new FCCmdLayerAdd (pNewLayer, m_IndexList[0])) ;
        // delete layers
        for (i = (int)m_IndexList.size()-1 ; i >=0 ; i--)
        {
            FCObjLayer   * pLayer = canvas.GetLayer(m_IndexList[i]) ; assert(pLayer);
            if (pLayer)
                PushImgCommand (new FCCmdLayerRemove (pLayer)) ;
        }
        FCCmdComposite::Execute (canvas, pProgress) ;
    }
    std::deque<int>   m_IndexList ;
};

//=============================================================================
/**
 *  Delete layers from canvas
 */
class FCCmdLayerListRemove : public FCCmdComposite
{
public:
    /// Constructor.
    /// @param IndexList : list of layer's index.
    FCCmdLayerListRemove (const FCObjCanvas& canvas, std::deque<int> IndexList)
    {
        if (IndexList.empty())
            {assert(false); return;}

        int     nNum = (int)IndexList.size(), i ;
        PCL_array<int>   arr (nNum) ;
        for (i=0 ; i < nNum ; i++)
            arr[i] = IndexList[i] ;

        // sort by small to big, delete layer from big to small
        FCOXOHelper::BubbleSort (arr.get(), nNum) ;

        for (i=nNum-1 ; i >= 0 ; i--)
            PushImgCommand (new FCCmdLayerRemove(canvas.GetLayer(arr[i]))) ;
    }
};

//=============================================================================
/**
 *  Handle canvas.
 *  <B>derived class must initialize member m_NewCanvasSize at constructor</B>
 */
class FCCmdCanvasOperation : public FCCmdComposite
{
public:
    FCCmdCanvasOperation()
    {
        m_OldCanvasSize.cx = m_OldCanvasSize.cy = -1 ;
        m_NewCanvasSize.cx = m_NewCanvasSize.cy = -1 ;
    }
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        m_OldCanvasSize = canvas.GetCanvasDimension() ;
        FCCmdComposite::Execute (canvas, pProgress) ;
        canvas.SetCanvasDimension (m_NewCanvasSize) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        FCCmdComposite::Undo(canvas) ;
        canvas.SetCanvasDimension (m_OldCanvasSize) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        FCCmdComposite::Redo(canvas) ;
        canvas.SetCanvasDimension (m_NewCanvasSize) ;
    }
private:
    SIZE     m_OldCanvasSize ;
protected:
    SIZE     m_NewCanvasSize ;
};

//=============================================================================
/**
 *  Resize canvas.
 */
class FCCmdCanvasResize : public FCCmdCanvasOperation
{
public:
    /// Constructor.
    /// @param nLeft,nTop,nRight,nBottom : canvas's delta.
    FCCmdCanvasResize (const FCObjCanvas& rCanvas, int nLeft, int nTop, int nRight, int nBottom)
    {
        const SIZE     OldSize = rCanvas.GetCanvasDimension() ;
        m_NewCanvasSize.cx = OldSize.cx + nLeft + nRight ;
        m_NewCanvasSize.cy = OldSize.cy + nTop + nBottom ;
        if ((m_NewCanvasSize.cx < 1) || (m_NewCanvasSize.cy < 1))
        {
            m_NewCanvasSize = OldSize ;
            assert(false) ; return ;
        }

        // update position of all layers
        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer   * pLayer = rCanvas.GetLayer(i) ;
            POINT        ptNew = pLayer->GetGraphObjPos() ;
            ptNew.x += nLeft ; ptNew.y += nTop ;
            PushImgCommand (new FCCmdLayerMove (pLayer, ptNew)) ;
        }

        // clear selection
        PushImgCommand (new FCCmdSelectionClear()) ;
    }
};

//=============================================================================
/**
 *  Canvas rotate 90'
 */
class FCCmdCanvasRotate90 : public FCCmdCanvasOperation
{
public:
    /// Constructor.
    FCCmdCanvasRotate90 (const FCObjCanvas& rCanvas)
    {
        const SIZE     OldSize = rCanvas.GetCanvasDimension() ;
        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer   * pLayer = rCanvas.GetLayer(i) ;

            PushImgCommand (new FCCmdLayerRotate90 (pLayer)) ;

            // calculate new position
            POINT     ptOld = pLayer->GetGraphObjPos(),
                      ptLB = {ptOld.x, ptOld.y + pLayer->Height()} ;
            ptOld.x = OldSize.cy - ptLB.y ;
            ptOld.y = ptLB.x ;
            PushImgCommand (new FCCmdLayerMove (pLayer, ptOld)) ;
        }
        PushImgCommand (new FCCmdSelectionClear()) ;
        // new canvas size
        m_NewCanvasSize.cx = OldSize.cy ; m_NewCanvasSize.cy = OldSize.cx ;
    }
};

//=============================================================================
/**
 *  Canvas rotate 270'
 */
class FCCmdCanvasRotate270 : public FCCmdCanvasOperation
{
public:
    /// Constructor.
    FCCmdCanvasRotate270 (const FCObjCanvas& rCanvas)
    {
        const SIZE     OldSize = rCanvas.GetCanvasDimension() ;
        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer   * pLayer = rCanvas.GetLayer(i) ;

            PushImgCommand (new FCCmdLayerRotate270 (pLayer)) ;

            // calculate new position
            POINT     ptOld = pLayer->GetGraphObjPos(),
                      ptRT = {ptOld.x + pLayer->Width(), ptOld.y} ;
            ptOld.x = ptRT.y ;
            ptOld.y = OldSize.cx - ptRT.x ;
            PushImgCommand (new FCCmdLayerMove (pLayer, ptOld)) ;
        }
        PushImgCommand (new FCCmdSelectionClear()) ;
        // new canvas size
        m_NewCanvasSize.cx = OldSize.cy ; m_NewCanvasSize.cy = OldSize.cx ;
    }
};

//=============================================================================
/**
 *  Rotate canvas.
 */
class FCCmdCanvasRotate : public FCCmdCanvasOperation
{
public:
    /// Constructor.
    /// @param nAngle : 0 -- 360
	FCCmdCanvasRotate (const FCObjCanvas& rCanvas, int nAngle)
    {
        nAngle = FClamp (nAngle, 0, 360) ;

        // 计算画布新尺寸
        const SIZE     OldSize = rCanvas.GetCanvasDimension() ;
        const int      nTmpAng = FMax (0, nAngle % 180) ;
        const double   fSin = sin(AngleToRadian(nTmpAng % 90)),
                       fCos = cos(AngleToRadian(nTmpAng % 90)) ;
        // 注意这里的宽高计算
        if (nTmpAng >= 90)
        {
            m_NewCanvasSize.cx = FRound (OldSize.cx * fSin + OldSize.cy * fCos) ;
            m_NewCanvasSize.cy = FRound (OldSize.cy * fSin + OldSize.cx * fCos) ;
        }
        else
        {
            m_NewCanvasSize.cx = FRound (OldSize.cx * fCos + OldSize.cy * fSin) ;
            m_NewCanvasSize.cy = FRound (OldSize.cy * fCos + OldSize.cx * fSin) ;
        }
        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer     * pLayer = rCanvas.GetLayer(i) ;
            
            // 计算新位置
            // 这里特殊点，先移动，后旋转，因为旋转时会绕中心点旋转
            POINT		ptNew = pLayer->GetGraphObjPos() ;
            ptNew.x += (m_NewCanvasSize.cx-OldSize.cx)/2 ;
            ptNew.y += (m_NewCanvasSize.cy-OldSize.cy)/2 ;
            PushImgCommand (new FCCmdLayerMove (pLayer, ptNew)) ;
            
            PushImgCommand (new FCCmdLayerRotate (pLayer, nAngle)) ;
        }
        PushImgCommand (new FCCmdSelectionClear()) ;
    }
};

//=============================================================================
/**
 *  Stretch canvas.
 */
class FCCmdCanvasStretch : public FCCmdCanvasOperation
{
public:
    FCCmdCanvasStretch (const FCObjCanvas& rCanvas, int nNewW, int nNewH, INTERPOLATION_TYPE InterplType)
    {
        const SIZE     OldSize = rCanvas.GetCanvasDimension() ;
        if ((nNewW <= 0) || (nNewH <= 0) || (OldSize.cx <= 0) || (OldSize.cy <= 0))
        {
            m_NewCanvasSize = OldSize ;
            assert(false) ; return ;
        }

        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer   * pLayer = rCanvas.GetLayer(i) ;
            PushImgCommand (new FCCmdLayerStretch (pLayer,
                                                   pLayer->Width() * nNewW / OldSize.cx,
                                                   pLayer->Height() * nNewH / OldSize.cy,
                                                   InterplType)) ;
            // calculate new position
            POINT      ptOld = pLayer->GetGraphObjPos() ;
            ptOld.x = ptOld.x * nNewW / OldSize.cx ;
            ptOld.y = ptOld.y * nNewH / OldSize.cy ;
            PushImgCommand (new FCCmdLayerMove (pLayer, ptOld)) ;
        }
        PushImgCommand (new FCCmdSelectionClear()) ;
        m_NewCanvasSize.cx = nNewW ; m_NewCanvasSize.cy = nNewH ;
    }
};

//=============================================================================
/**
 *  Crop canvas.
 */
class FCCmdCanvasCrop : public FCCmdCanvasOperation
{
public:
    /// Constructor.
    FCCmdCanvasCrop (const FCObjCanvas& rCanvas, RECT rcCanvas)
    {
        RECT     rcSel = rcCanvas ;
        rCanvas.BoundRect (rcSel) ;
        if (IsRectEmpty(&rcSel))
        {
            m_NewCanvasSize = rCanvas.GetCanvasDimension() ;
            return ;
        }

        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            // crop all layers
            PushImgCommand (new FCCmdCanvasCrop_Layer (rCanvas.GetLayer(i), rcSel)) ;
        }
        // clear selection
        PushImgCommand (new FCCmdSelectionClear()) ;
        m_NewCanvasSize.cx = RECTWIDTH(rcSel) ;
        m_NewCanvasSize.cy = RECTHEIGHT(rcSel) ;
    }
private:
    // 裁剪单层逻辑
    class FCCmdCanvasCrop_Layer : public FCCmdLayerOperation
    {
    public:
        FCCmdCanvasCrop_Layer (FCObjLayer* pLayer, RECT rcCanvas) : FCCmdLayerOperation(pLayer), m_rcCanvas(rcCanvas) {}
        virtual void ProcessLayer (FCObjCanvas& canvas, FCObjLayer& rLayer, FCObjProgress* pProgress)
        {
            // calculate RECT in layer
            RECT     rcLayer = m_rcCanvas ;
            rLayer.Canvas_to_Layer (rcLayer) ;
            rLayer.BoundRect (rcLayer) ;
            if (IsRectEmpty (&rcLayer))
            {
                // create transparency layer
                rLayer.Create (RECTWIDTH(m_rcCanvas), RECTHEIGHT(m_rcCanvas), 32) ;
                FCPixelFillColor     cmdFillCr (FCColor::crWhite(), 0) ;
                rLayer.SinglePixelProcessProc (cmdFillCr, pProgress) ;
                rLayer.SetGraphObjPos (0, 0) ;
            }
            else
            {
                FCObjImage      imgCrop ;
                canvas.MakeRegion (rLayer, imgCrop) ;

                // calc new layer's position, notice sequence !!!!!!
                rLayer.Layer_to_Canvas (rcLayer) ;
                (FCObjImage&)rLayer = imgCrop ;
                rLayer.SetGraphObjPos (rcLayer.left-m_rcCanvas.left, rcLayer.top-m_rcCanvas.top) ;
            }
        }
    private:
        RECT     m_rcCanvas ; // rect of canvas
    };
};

//============================================================================
// 自动裁减画布/auto crop canvas，裁掉canvas边上透明的区域
class FCCmdCanvasAutoCrop : public FCCmdCanvasOperation
{
public:
    FCCmdCanvasAutoCrop (const FCObjCanvas& rCanvas)
    {
        const SIZE     sizeOld = rCanvas.GetCanvasDimension() ;
        m_NewCanvasSize = sizeOld ; // must set size before return

        RECT      rcBound = {0,0,0,0} ;
        for (int i=0 ; i < rCanvas.GetLayerNumber() ; i++)
        {
            FCObjLayer     * pLayer = rCanvas.GetLayer(i) ;

            FCPixelGetOptimizedRect   aCmd ;
            pLayer->SinglePixelProcessProc (aCmd) ;
            pLayer->Layer_to_Canvas (aCmd.m_rcOptimized) ;
            if (i == 0)
                rcBound = aCmd.m_rcOptimized ;
            else
                ::UnionRect (&rcBound, &rcBound, &aCmd.m_rcOptimized) ;
        }
        if (IsRectEmpty (&rcBound))
            return ;

        rCanvas.BoundRect (rcBound) ;
        if (IsRectEmpty(&rcBound))
            return ;

        int   nRight = rcBound.right - sizeOld.cx,
              nBottom = rcBound.bottom - sizeOld.cy ;
        PushImgCommand (new FCCmdCanvasResize(rCanvas, -rcBound.left, -rcBound.top, nRight, nBottom)) ;
        m_NewCanvasSize.cx = sizeOld.cx - rcBound.left + nRight ;
        m_NewCanvasSize.cy = sizeOld.cy - rcBound.top + nBottom ;
    }
};

#endif
