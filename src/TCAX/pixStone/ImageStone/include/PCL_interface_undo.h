/*
 *   Copyright (C) =USTC= Fu Li
 *
 *   Author   :  Fu Li
 *   Create   :  2005-3-6
 *   Home     :  http://www.crazy-bit.com/
 *   Mail     :  crazybitwps@hotmail.com
 *   History  :  
 */
#ifndef __FOO_PCL_INTERFACE_UNDO_MANAGER_H__
#define __FOO_PCL_INTERFACE_UNDO_MANAGER_H__
#include <assert.h>
#include <deque>

template<class T> class PCL_Interface_Undo ;

//=============================================================================
/**
 *  Undo/Redo manage.
 */
template<class T>
class PCL_Interface_Undo
{
public:
    /// Constructor.
    PCL_Interface_Undo (int nUndoLevel = 20) : m_nUndoLevel(nUndoLevel) {}
    virtual ~PCL_Interface_Undo()
    {
        ClearRedoList() ;
        ClearUndoList() ;
    }

    /**
     *  @name Basic Undo/Redo operation.
     */
    //@{
    /// Test undo enable.
    bool IsUndoEnable() const {return !m_UndoList.empty();}
    /// Test redo enable.
    bool IsRedoEnable() const {return !m_RedoList.empty();}
    /// Undo
    void Undo()
    {
        if (IsUndoEnable())
        {
            T   * p = m_UndoList.back() ; // get the latest undo-command
            this->PCL_Implement_Undo (p) ;
            m_UndoList.pop_back() ;
            m_RedoList.push_front(p) ;
        }
    }
    /// Redo
    void Redo()
    {
        if (IsRedoEnable())
        {
            T   * p = m_RedoList.front() ; // get the first redo-command
            this->PCL_Implement_Redo (p) ;
            m_RedoList.pop_front() ;
            m_UndoList.push_back(p) ;
        }
    }
    //@}

    /**
     *  @name Clear operation.
     */
    //@{
    /// Clear redo command.
    void ClearRedoList()
    {
        while (IsRedoEnable())
        {
            T   * p = m_RedoList.back() ;
            m_RedoList.pop_back() ;
            delete p ;
        }
    }
    /// Clear undo command.
    void ClearUndoList()
    {
        while (IsUndoEnable())
        {
            T   * p = m_UndoList.back() ;
            m_UndoList.pop_back() ;
            delete p ;
        }
    }
    //@}

    /**
     *  @name Undo level.
     */
    //@{
    /// Get undo level.
    /// if the undo level ==0, it means undo disable..
    int GetUndoLevel() const {return m_nUndoLevel;}
    /// Set undo level.
    /// it will delete overflowed command.
    void SetUndoLevel (int nLevel)
    {
        if (nLevel < 0)
        {
            assert(false); return;
        }

        m_nUndoLevel = nLevel ; // >=0
        // calculate current number of command in list
        int     nRemove = (int)(m_UndoList.size() + m_RedoList.size()) - m_nUndoLevel ;
        if (nRemove <= 0)
            return ;

        // first : delete command from Redo-list back
        while (!m_RedoList.empty() && (nRemove > 0))
        {
            T   * p = m_RedoList.back() ;
            m_RedoList.pop_back() ;
            delete p ; nRemove-- ;
        }

        // then : delete command from Undo-list front
        while (!m_UndoList.empty() && (nRemove > 0))
        {
            T   * p = m_UndoList.front() ;
            m_UndoList.pop_front() ;
            delete p ; nRemove-- ;
        }
    }
    //@}

protected:
    /**
     *  @name Handle command.
     */
    //@{
    /// Undo's implementation (must implement).
    /// @param : pCmd is the last command in undo-list.
    virtual void PCL_Implement_Undo (T* pCmd) =0 ;
    /// Redo's implementation (must implement).
    /// @param : pCmd is the first command in redo-list.
    virtual void PCL_Implement_Redo (T* pCmd) =0 ;
    //@}

    /**
     *  Add a command object into undo-list.
     *    1) it will clear redo-list.  <BR>
     *    2) if the undo list is full, it will delete oldest undo command.
     */
    bool AddCommand (T* pCmd)
    {
        if ((m_nUndoLevel <= 0) || !pCmd)
        {
            assert(false); return false;
        }

        // clear redo-list, ennnnnnn. this behavior like a bintree.
        ClearRedoList() ;

        // single command
        assert ((int)m_UndoList.size() <= m_nUndoLevel) ;
        if ((int)m_UndoList.size() >= m_nUndoLevel) // never '>'
        {
            T   * p = m_UndoList.front() ;
            m_UndoList.pop_front() ;
            delete p ;
        }

        // add to undo list
        m_UndoList.push_back (pCmd) ;
        return true ;
    }

    T* GetLatestUndoCommand() const {return IsUndoEnable() ? m_UndoList.back() : 0;}

private:
    std::deque<T*>   m_UndoList ;
    std::deque<T*>   m_RedoList ;
    // the level of undo operation. the ZERO means disable undo
    int              m_nUndoLevel ;
};

//=============================================================================
// inline implement
//=============================================================================

#endif
