/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-3-3
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_TIME_COUNT__2005_03_03__H__
#define __FOO_TIME_COUNT__2005_03_03__H__
#include <time.h>

//=============================================================================
/**
 *  Calculate time interval between two time stamper.
 */
class FCTimeCount
{
    clock_t   m_nTick ;
public:
    /// Constructor (will set start time stamper).
    FCTimeCount() {SetStartTag();}

    /// Set start time stamper.
    void SetStartTag() {m_nTick = ::clock();}
    /// Get passed millisecond from start time stamper.
    int GetPassMillisecond() const {return (int)tick_to_ms(::clock() - m_nTick);}

    static clock_t tick_to_ms (clock_t nTick)
    {
        return (nTick * 1000 / CLOCKS_PER_SEC) ;
    }
};

#endif
