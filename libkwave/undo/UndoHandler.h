/***************************************************************************
           UndoHandler.h -  abstract base class for undo saving
                             -------------------
    begin                : Sat Feb 01 2014
    copyright            : (C) 2014 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UNDO_HANDLER_H
#define UNDO_HANDLER_H

#include "config.h"

#include <QtGlobal>

namespace Kwave
{

    class UndoTransaction;

    class Q_DECL_EXPORT UndoHandler
    {
    public:

        /** Default constructor */
        UndoHandler()
        {}

        /** Destructor */
        virtual ~UndoHandler() {}

        /**
         * Called by an undo manager to notify the handler that
         * it is time to save data for undo.
         *
         * @param undo an undo transaction to append some undo data
         * @retval true if successful
         * @retval false if saving undo data failed, e.g. out of memory
         *               or aborted
         */
        virtual bool saveUndoData(Kwave::UndoTransaction &undo) = 0;

    };
}

#endif /* UNDO_HANDLER_H */

//***************************************************************************
//***************************************************************************
