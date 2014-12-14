
//=============================================================================
/**
 *  Box blur effect (>=24 bit).
@verbatim
    example:
        FCPixelBlur_Box   aCmd (5, true) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlur_Box : public FCPixelWholeImageBase
{
public:
    /**
     *  Constructor.
     *  @param bCopyEdge true : blur border with alpha==0 pixel
     */
    FCPixelBlur_Box (int iBlockLen, bool bCopyEdge)
    {
        m_iBlock = FMax(2, iBlockLen) ;
        m_bCopyEdge = bCopyEdge ;
    }
private:
    // if m_iBlock>200, the sum maybe exceed int32
    void in_UpdateSum (int& R, int& G, int& B, int& A, const void* pAdd, const void* pSub)
    {
        int   nAddA = (GetBackupImage()->ColorBits() == 32) ? PCL_A(pAdd) : 0xFF,
              nSubA = (GetBackupImage()->ColorBits() == 32) ? PCL_A(pSub) : 0xFF ;
        B = B + PCL_B(pAdd)*nAddA - PCL_B(pSub)*nSubA ;
        G = G + PCL_G(pAdd)*nAddA - PCL_G(pSub)*nSubA ;
        R = R + PCL_R(pAdd)*nAddA - PCL_R(pSub)*nSubA ;
        A = A + nAddA - nSubA ;
    }
    void in_SetPixel (int R, int G, int B, int A, void* pPixel)
    {
        PCL_B(pPixel) = A ? (B/A) : 0 ;
        PCL_G(pPixel) = A ? (G/A) : 0 ;
        PCL_R(pPixel) = A ? (R/A) : 0 ;
        if (GetBackupImage()->ColorBits() == 32)
            PCL_A(pPixel) = A / FSquare(m_iBlock) ; // pixel number of block
    }
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage (pImg) ;

        // expand edge
        int   nLeftTop = m_iBlock/2, // left & top
              nRightDown = nLeftTop; // right & bottom :  -((m_iBlock % 2)^1)
        GetBackupImage()->ExpandFrame (m_bCopyEdge, nLeftTop, nLeftTop, nRightDown, nRightDown) ;
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        // RGBA sum of every scanline start
        int     iFirstR=0, iFirstG=0, iFirstB=0, iFirstA=0 ;
        for (int y=0 ; y < pImg->Height() ; y++)
        {
            if (y == 0) // first line
            {
                for (int ny=0 ; ny < m_iBlock ; ny++)
                    for (int nx=0 ; nx < m_iBlock ; nx++)
                    {
                        BYTE   * pPixel = GetBackupImage()->GetBits (nx,ny) ;
                        int    nA = (GetBackupImage()->ColorBits() == 32) ? PCL_A(pPixel) : 0xFF ;
                        iFirstB += PCL_B(pPixel) * nA ;
                        iFirstG += PCL_G(pPixel) * nA ;
                        iFirstR += PCL_R(pPixel) * nA ;
                        iFirstA += nA ;
                    }
            }
            else // Y move down
            {
                // sub up line & add down line
                for (int i=0 ; i < m_iBlock ; i++)
                    in_UpdateSum (iFirstR, iFirstG, iFirstB, iFirstA,
                                  GetBackupImage()->GetBits (i, y-1+m_iBlock),
                                  GetBackupImage()->GetBits (i, y-1)) ;
            }

            // set first pixel per scanline
            in_SetPixel (iFirstR, iFirstG, iFirstB, iFirstA, pImg->GetBits(y)) ;

            // X move
            int     iCurrR=iFirstR, iCurrG=iFirstG, iCurrB=iFirstB, iCurrA=iFirstA ;
            for (int x=1 ; x < pImg->Width() ; x++)
            {
                // sub left pixel & add right pixel
                for (int i=0 ; i < m_iBlock ; i++)
                    in_UpdateSum (iCurrR, iCurrG, iCurrB, iCurrA,
                                  GetBackupImage()->GetBits (x-1+m_iBlock, y+i),
                                  GetBackupImage()->GetBits (x-1, y+i)) ;

                in_SetPixel (iCurrR, iCurrG, iCurrB, iCurrA, pImg->GetBits(x,y)) ;
            }
            if (pProgress)
                pProgress->SetProgress ((y+1) * 100 / pImg->Height()) ;
        } // end of for(Y)
    }

    int     m_iBlock ;
    bool    m_bCopyEdge ;
};
