/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2003-4-10
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef	__FOO_OBJECT_PROGRESS__2003_04_10__H__
#define	__FOO_OBJECT_PROGRESS__2003_04_10__H__

//=============================================================================
/**
 *  Progress interface.
 */
class FCObjProgress
{
    int   m_nProgress ;
public:
    FCObjProgress() : m_nProgress(0) {}
    virtual ~FCObjProgress() {}

    /// Set current progress to 0
    virtual void ResetProgress() {this->SetProgress(0);}

    /// Get current progress.
    virtual int GetProgress() const {return m_nProgress;}

    /// Set current progress.
    virtual void SetProgress(int nNew) {m_nProgress=nNew;}
};

#endif
