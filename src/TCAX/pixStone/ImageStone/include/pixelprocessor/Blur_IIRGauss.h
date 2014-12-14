
//=============================================================================
/**
 *  Blur IIR gauss (>=24 bit).
@verbatim
    example:
        FCPixelBlur_Gauss_IIR   aCmd (10, 10) ;
        img.SinglePixelProcessProc (aCmd) ;
@endverbatim
 */
class FCPixelBlur_Gauss_IIR : public FCPixelWholeImageBase
{
public:
    /**
     *  Constructor (nHorz & nVert must >= 2).
     */
    FCPixelBlur_Gauss_IIR (int nHorz, int nVert)
    {
        m_nHorz = FMax(2,nHorz) ;
        m_nVert = FMax(2,nVert) ;
    }
private:
    // Convert from separated to premultiplied alpha, on a single scan line.
    static void multiply_alpha (BYTE* buf, int width, int bytes)
    {
        for (int i=0 ; i < width * bytes ; i += bytes)
        {
            double   alpha = buf[i + bytes - 1] * (1.0 / 255.0) ;
            for (int j = 0; j < bytes - 1; j++)
                buf[i + j] = (int)(buf[i + j] * alpha) ;
        }
    }
    // Convert from premultiplied to separated alpha, on a single scan line.
    static void separate_alpha (BYTE* buf, int width, int bytes)
    {
        int   i, j;
        BYTE alpha;
        double recip_alpha;
        int    new_val;

        for (i = 0; i < width * bytes; i += bytes)
        {
            alpha = buf[i + bytes - 1];
            if (alpha != 0 && alpha != 255)
            {
                recip_alpha = 255.0 / alpha;
                for (j = 0; j < bytes - 1; j++)
                {
                    new_val = (int)(buf[i + j] * recip_alpha);
                    buf[i + j] = FMin (255, new_val);
                }
            }
        }
    }
    static void find_constants (double n_p[], double n_m[], double d_p[], double d_m[], double bd_p[], double bd_m[], double std_dev)
    {
        int    i;
        double constants [8];
        double div;
        
        // The constants used in the implemenation of a casual sequence
        // using a 4th order approximation of the gaussian operator
        div = sqrt(LIB_2PI) * std_dev;
        constants [0] = -1.783 / std_dev;
        constants [1] = -1.723 / std_dev;
        constants [2] = 0.6318 / std_dev;
        constants [3] = 1.997  / std_dev;
        constants [4] = 1.6803 / div;
        constants [5] = 3.735 / div;
        constants [6] = -0.6803 / div;
        constants [7] = -0.2598 / div;
        
        n_p [0] = constants[4] + constants[6];
        n_p [1] = exp (constants[1]) *
            (constants[7] * sin (constants[3]) -
            (constants[6] + 2 * constants[4]) * cos (constants[3])) +
            exp (constants[0]) *
            (constants[5] * sin (constants[2]) -
            (2 * constants[6] + constants[4]) * cos (constants[2]));
        n_p [2] = 2 * exp (constants[0] + constants[1]) *
            ((constants[4] + constants[6]) * cos (constants[3]) * cos (constants[2]) -
            constants[5] * cos (constants[3]) * sin (constants[2]) -
            constants[7] * cos (constants[2]) * sin (constants[3])) +
            constants[6] * exp (2 * constants[0]) +
            constants[4] * exp (2 * constants[1]);
        n_p [3] = exp (constants[1] + 2 * constants[0]) *
            (constants[7] * sin (constants[3]) - constants[6] * cos (constants[3])) +
            exp (constants[0] + 2 * constants[1]) *
            (constants[5] * sin (constants[2]) - constants[4] * cos (constants[2]));
        n_p [4] = 0.0;
        
        d_p [0] = 0.0;
        d_p [1] = -2 * exp (constants[1]) * cos (constants[3]) -
            2 * exp (constants[0]) * cos (constants[2]);
        d_p [2] = 4 * cos (constants[3]) * cos (constants[2]) * exp (constants[0] + constants[1]) +
            exp (2 * constants[1]) + exp (2 * constants[0]);
        d_p [3] = -2 * cos (constants[2]) * exp (constants[0] + 2 * constants[1]) -
            2 * cos (constants[3]) * exp (constants[1] + 2 * constants[0]);
        d_p [4] = exp (2 * constants[0] + 2 * constants[1]);
        
        for (i = 0; i <= 4; i++)
            d_m [i] = d_p [i];
        
        n_m[0] = 0.0;
        for (i = 1; i <= 4; i++)
            n_m [i] = n_p[i] - d_p[i] * n_p[0];
        
        {
            double sum_n_p = 0.0;
            double sum_n_m = 0.0;
            double sum_d = 0.0;
            for (i = 0; i <= 4; i++)
            {
                sum_n_p += n_p[i];
                sum_n_m += n_m[i];
                sum_d += d_p[i];
            }
            
            double a = sum_n_p / (1.0 + sum_d);
            double b = sum_n_m / (1.0 + sum_d);
            
            for (i = 0; i <= 4; i++)
            {
                bd_p[i] = d_p[i] * a;
                bd_m[i] = d_m[i] * b;
            }
        }
    }
    void GaussIIRBlurImage (FCObjImage& img, FCObjProgress* pProgress)
    {
        const int   bytes = img.ColorBits() / 8 ; // 3 or 4

        PCL_array<double>  val_p (FMax(img.Width(),img.Height()) * bytes),
                           val_m (FMax(img.Width(),img.Height()) * bytes) ;

        PCL_array<BYTE>    src (FMax(img.Width(),img.Height()) * bytes) ;

        double   n_p[5], n_m[5], d_p[5], d_m[5], bd_p[5], bd_m[5] ;

        // First the vertical pass
        {
            int     v = m_nVert + 1 ;
            double  std_dev = sqrt (-(v * v) / (2 * log (1.0 / 255.0))) ;

            // derive the constants for calculating the gaussian from the std dev
            find_constants (n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev) ;
        }

        for (int x=0 ; x < img.Width() ; x++)
        {
            memset (val_p.get(), 0, val_p.GetArrayBytes()) ;
            memset (val_m.get(), 0, val_m.GetArrayBytes()) ;

            // get col
            {
                for (int y=0 ; y < img.Height() ; y++)
                    FCColor::CopyPixel (&src[bytes*y], img.GetBits(x,y), bytes) ;
            }

            if (img.ColorBits() == 32)
                multiply_alpha (src.get(), img.Height(), bytes) ;

            BYTE    * sp_p = src.get() ;
            BYTE    * sp_m = src.get() + (img.Height() - 1) * bytes ;
            double  * vp = val_p.get() ;
            double  * vm = val_m.get() + (img.Height() - 1) * bytes ;
            
            // Set up the first vals
            int     initial_p[4],
                    initial_m[4] ;
            for (int i=0 ; i < bytes ; i++)
            {
                initial_p[i] = sp_p[i] ;
                initial_m[i] = sp_m[i] ;
            }

            for (int y=0 ; y < img.Height() ; y++)
            {
                double  * vpptr,
                        * vmptr ;
                int     terms = (y < 4) ? y : 4 ;

                for (int b=0 ; b < bytes ; b++)
                {
                    vpptr = vp + b; vmptr = vm + b;
                    int   i ;
                    for (i=0 ; i <= terms ; i++)
                    {
                        *vpptr += n_p[i] * sp_p[(-i * bytes) + b] - d_p[i] * vp[(-i * bytes) + b] ;
                        *vmptr += n_m[i] * sp_m[(i * bytes) + b] - d_m[i] * vm[(i * bytes) + b] ;
                    }
                    for (int j=i ; j <= 4 ; j++)
                    {
                        *vpptr += (n_p[j] - bd_p[j]) * initial_p[b] ;
                        *vmptr += (n_m[j] - bd_m[j]) * initial_m[b] ;
                    }
                }

                sp_p += bytes ;
                sp_m -= bytes ;
                vp += bytes ;
                vm -= bytes ;
            }

            double  * pTmp_p = val_p.get(), * pTmp_m = val_m.get() ;
            for (int mm=0 ; mm < img.Height() ; mm++)
            {
                BYTE   * p = img.GetBits (x, mm) ;
                for (int b=0 ; b < bytes ; b++)
                {
                    p[b] = FClamp0255 (int(*pTmp_p + *pTmp_m)) ; pTmp_p++ ; pTmp_m++ ;
                }
            }
            if (pProgress)
                pProgress->SetProgress (50 * x / img.Width()) ;
        }

        // Now the horizontal pass
        {
            int     h = m_nHorz + 1 ;
            double  std_dev = sqrt (-(h * h) / (2 * log (1.0 / 255.0))) ;

            // derive the constants for calculating the gaussian from the std dev
            find_constants (n_p, n_m, d_p, d_m, bd_p, bd_m, std_dev) ;
        }

        for (int y=0 ; y < img.Height() ; y++)
        {
            memset (val_p.get(), 0, val_p.GetArrayBytes()) ;
            memset (val_m.get(), 0, val_m.GetArrayBytes()) ;

            // get row
            memcpy (src.get(), img.GetBits(y), img.Width() * bytes) ;

            BYTE    * sp_p = src.get() ;
            BYTE    * sp_m = src.get() + (img.Width() - 1) * bytes ;
            double  * vp = val_p.get() ;
            double  * vm = val_m.get() + (img.Width() - 1) * bytes ;

            // Set up the first vals
            int     initial_p[4],
                    initial_m[4] ;
            for (int i=0 ; i < bytes ; i++)
            {
                initial_p[i] = sp_p[i] ;
                initial_m[i] = sp_m[i] ;
            }

            for (int x=0 ; x < img.Width() ; x++)
            {
                double  * vpptr,
                        * vmptr ;
                int     terms = (x < 4) ? x : 4 ;

                for (int b=0 ; b < bytes ; b++)
                {
                    vpptr = vp + b; vmptr = vm + b;
                    int   i ;
                    for (i=0 ; i <= terms ; i++)
                    {
                        *vpptr += n_p[i] * sp_p[(-i * bytes) + b] - d_p[i] * vp[(-i * bytes) + b] ;
                        *vmptr += n_m[i] * sp_m[(i * bytes) + b] - d_m[i] * vm[(i * bytes) + b] ;
                    }
                    for (int j=i ; j <= 4 ; j++)
                    {
                        *vpptr += (n_p[j] - bd_p[j]) * initial_p[b] ;
                        *vmptr += (n_m[j] - bd_m[j]) * initial_m[b] ;
                    }
                }

                sp_p += bytes ;
                sp_m -= bytes ;
                vp += bytes ;
                vm -= bytes ;
            }
            
            double  * pTmp_p = val_p.get(), * pTmp_m = val_m.get() ;
            for (int mm=0 ; mm < img.Width() ; mm++)
            {
                BYTE   * p = img.GetBits (mm, y) ;
                for (int b=0 ; b < bytes ; b++)
                {
                    p[b] = FClamp0255 (int(*pTmp_p + *pTmp_m)) ; pTmp_p++ ; pTmp_m++ ;
                }
            }

            if (bytes == 4)
                separate_alpha (img.GetBits(y), img.Width(), bytes) ;

            if (pProgress)
                pProgress->SetProgress (50 + 50 * y / img.Height()) ;
        }
    }
    virtual void ProcessWholeImage (FCObjImage* pImg, FCObjProgress* pProgress)
    {
        GaussIIRBlurImage (*pImg, pProgress) ;
    }

    int   m_nHorz, m_nVert ;
};
