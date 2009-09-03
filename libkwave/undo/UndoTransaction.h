/***************************************************************************
      UndoTransaction.h  -  groups moulitple UndoAction objects together
			     -------------------
    begin                : Fri May 25 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _UNDO_TRANSACTION_H_
#define _UNDO_TRANSACTION_H_

#include "config.h"

#include <QList>
#include <QString>

class UndoAction;

/**
 * @class UndoTransaction
 * Groups multiple UndoAction objects together to one transaction. As most
 * user actions consist of a number of small actions that belong together
 * and don't make sense or leave an inconsistent state if separated, they
 * get grouped together to one transaction.
 */
class UndoTransaction: public QList<UndoAction *>
{

public:

    /**
     * Constructor.
     * @param name description of the undo transaction as a user-readable
     * localized string.
     */
    UndoTransaction(const QString &name);

    /** Destructor */
    virtual ~UndoTransaction();

    /** Returns the size in bytes summed up over all undo actions */
    unsigned int undoSize();

    /** Returns the additional memory needed for storing redo data */
    unsigned int redoSize();

    /**
     * Returns the description of the undo transaction as a user-readable
     * localized string. If no name has been passed at initialization
     * time, a list of all action's descriptions will be generated.
     * @todo avoid duplicates, give a useful name/description
     */
    QString description();

    /**
     * Loops over all undo actions to determine wheter there is at least
     * one undo action that contains a modification of the signal.
     * @see UndoAction::containsModification()
     * @return true if a modification is contained, false if not.
     */
    bool containsModification() const;

    /**
     * aborts the undo transaction
     */
    void abort();

    /**
     * Returns true if the undo transaction has been aborted
     */
    bool isAborted() const { return m_aborted; }

        /** dump, for debugging purposes */
    virtual void dump(const QString &indent);

private:

    /** name of the action */
    QString m_description;

    /** if true, the transaction has been aborted */
    bool m_aborted;

};

#endif /* _UNDO_TRANSACTION_H_ */
