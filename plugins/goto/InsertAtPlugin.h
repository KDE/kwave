/***************************************************************************
       InsertAtPlugin.h  -  plugin for insertin the clipboard at a position
                             -------------------
    begin                : Thu May 12 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#ifndef INSERT_AT_PLUGIN_H
#define INSERT_AT_PLUGIN_H

#include "config.h"

#include <QString>

#include "GotoPluginBase.h"

namespace Kwave
{


    class InsertAtPlugin: public Kwave::GotoPluginBase
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        InsertAtPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~InsertAtPlugin() override;

    protected:

        /** Returns the command to be emitted */
        QString command() const override;

        /** Returns the title of the dialog */
        QString title() const override;

    };
}

#endif /* INSERT_AT_PLUGIN_H */

//***************************************************************************
//***************************************************************************
