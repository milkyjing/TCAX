/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2004-10-5
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_INTERFACE_COMMAND__2004_10_05__H__
#define __FOO_INTERFACE_COMMAND__2004_10_05__H__

class FCObjCanvas ; // external class

class FCCmdArtPrider ; // interface for FCObjCanvas::Execute
    class FCCmdOnceExecute ; // this command execute once and can't be UNDO/REDO

//=============================================================================
/**
 *  Interface for FCObjCanvas::Execute.
 */
class FCCmdArtPrider
{
public:
    FCCmdArtPrider() : m_bDeleteAfterExecute(false) {}
    virtual ~FCCmdArtPrider() {}
    void SetDeleteFlagAfterExecute (bool bFlag) {m_bDeleteAfterExecute=bFlag;}
    bool IsNeedDeleteAfterExecute() const {return m_bDeleteAfterExecute ? true : false;}

    /// Operation on canvas.
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress) =0 ;
    /// Undo.
    virtual void Undo (FCObjCanvas& canvas) =0 ;
    /// Redo.
    virtual void Redo (FCObjCanvas& canvas) =0 ;

private:
    int   m_bDeleteAfterExecute ;
};

//=============================================================================
/**
 *  This command execute once and can't be UNDO/REDO.
 */
class FCCmdOnceExecute : public FCCmdArtPrider
{
    virtual void Undo (FCObjCanvas& canvas) {assert(false);}
    virtual void Redo (FCObjCanvas& canvas) {assert(false);}
public:
    FCCmdOnceExecute() {SetDeleteFlagAfterExecute(true);}
};

//=============================================================================
/**
 *  Composite command.
 */
class FCCmdComposite : public FCCmdArtPrider,
                       private PCL_Interface_Composite<FCCmdArtPrider>
{
protected:
    virtual void Execute (FCObjCanvas& canvas, FCObjProgress* pProgress)
    {
        for (int i=0 ; i < PCL_GetObjectCount() ; i++)
            PCL_GetObject(i)->Execute (canvas, pProgress) ;
    }
    virtual void Undo (FCObjCanvas& canvas)
    {
        for (int i=PCL_GetObjectCount()-1 ; i >= 0 ; i--)
            PCL_GetObject(i)->Undo(canvas) ;
    }
    virtual void Redo (FCObjCanvas& canvas)
    {
        for (int i=0 ; i < PCL_GetObjectCount() ; i++)
            PCL_GetObject(i)->Redo(canvas) ;
    }
public:
    /// Add a command to composite command.
    void PushImgCommand (FCCmdArtPrider* pCmd)
    {
        assert (pCmd) ;
        if (pCmd)
            PCL_PushObject (pCmd) ;
    }
};

//=============================================================================
// inline Implement
//=============================================================================

#endif
