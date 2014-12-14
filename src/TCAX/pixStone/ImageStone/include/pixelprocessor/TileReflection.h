
//=============================================================================
/**
 *  Tile reflection effect (>=24 bit).
@verbatim
    example:
        FCPixelTileReflection   aCmd (45, 20, 8) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelTileReflection : public FCSinglePixelProcessBase
{
    struct POINT_FLOAT
    {
        double   x ;
        double   y ;
    };

public:
    /**
     *  Constructor.
     */
    FCPixelTileReflection (int nAngle, int nSquareSize, int nCurvature)
    {
        nAngle = FClamp (nAngle, -45, 45) ;
        m_sin = sin (AngleToRadian(nAngle)) ;
        m_cos = cos (AngleToRadian(nAngle)) ;

        nSquareSize = FClamp (nSquareSize, 2, 200) ;
        m_scale = LIB_PI / (double)nSquareSize ;

        nCurvature = FClamp (nCurvature, -20, 20) ;
        if (nCurvature == 0)
            nCurvature = 1 ;
        m_curvature = nCurvature * nCurvature / 10.0 * (abs(nCurvature)/nCurvature) ;

        for (int i=0 ; i < aasamples ; i++)
        {
            double  x = (i * 4) / (double)aasamples,
                    y = i / (double)aasamples ;
            x = x - (int)x ;
            m_aapt[i].x = m_cos * x + m_sin * y ;
            m_aapt[i].y = m_cos * y - m_sin * x ;
        }
    }
private:
    virtual void OnEnterProcess (FCObjImage* pImg)
    {
        SetBackupImage(pImg) ;
    }
    virtual void ProcessPixel (FCObjImage* pImg, int x, int y, BYTE* pPixel)
    {
        double   hw = pImg->Width() / 2.0,
                 hh = pImg->Height() / 2.0,
                 i = x - hw,
                 j = y - hh ;

        int   b=0, g=0, r=0, a=0 ;
        for (int mm=0 ; mm < aasamples ; mm++)
        {
            double   u = i + m_aapt[mm].x ;
            double   v = j - m_aapt[mm].y ;

            double   s =  m_cos * u + m_sin * v ;
            double   t = -m_sin * u + m_cos * v ;

            s += m_curvature * tan(s * m_scale) ;
            t += m_curvature * tan(t * m_scale) ;
            u = m_cos * s - m_sin * t ;
            v = m_sin * s + m_cos * t ;
            
            int   xSample = (int)(hw + u) ;
            int   ySample = (int)(hh + v) ;

            xSample = FClamp (xSample, 0, GetBackupImage()->Width()-1) ;
            ySample = FClamp (ySample, 0, GetBackupImage()->Height()-1) ;

            BYTE   * p = GetBackupImage()->GetBits(xSample, ySample) ;
            b += PCL_B(p) ;
            g += PCL_G(p) ;
            r += PCL_R(p) ;
            a += PCL_A(p) ;
        }

        PCL_B(pPixel) = FClamp0255 (b / aasamples) ;
        PCL_G(pPixel) = FClamp0255 (g / aasamples) ;
        PCL_R(pPixel) = FClamp0255 (r / aasamples) ;
        PCL_A(pPixel) = FClamp0255 (a / aasamples) ;
    }

    enum
    {
        aasamples = 17,
    };

    double   m_sin, m_cos ;
    double   m_scale ;
    double   m_curvature ;
    POINT_FLOAT   m_aapt[aasamples] ;
};
