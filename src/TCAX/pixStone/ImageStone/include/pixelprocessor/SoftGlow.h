
//=============================================================================
/**
 *  Soft glow effect (>=24 bit).
@verbatim
    example:
        FCPixelSoftGlow   aCmd (10, 60, 110) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelSoftGlow : public FCPixelWholeImageBase
{
public:
    /**
     *  Constructor.
     */
    FCPixelSoftGlow (int nRadius, int nBrightness, int nContrast)
    {
        m_nRadius = nRadius ;
        m_nBrightness = nBrightness ;
        m_nContrast = nContrast ;
    }
private:

    #define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))

    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        // make lightness image
        FCObjImage   img (*pImg) ;

        // gauss blur
        FCPixelBlur_Gauss_IIR   aCmd (m_nRadius, m_nRadius) ;
        img.SinglePixelProcessProc (aCmd, pProgress) ;

        FCPixelBrightness   aBri (m_nBrightness) ;
        img.SinglePixelProcessProc (aBri) ;

        FCPixelContrast   aCon (m_nContrast) ;
        img.SinglePixelProcessProc (aCon) ;

        // blend
        for (int y=0 ; y < img.Height() ; y++)
            for (int x=0 ; x < img.Width() ; x++)
            {
                BYTE   * pSrc = img.GetBits(x,y) ;
                BYTE   * pDst = pImg->GetBits(x,y) ;
                for (int b=0 ; b < 3 ; b++)
                {
                    // screen op
                    int   tmp ;
                    pDst[b] = 255 - INT_MULT((255 - pDst[b]), (255 - pSrc[b]), tmp) ;
                }
            }
    }

    int   m_nRadius ;
    int   m_nBrightness ;
    int   m_nContrast ;
};
