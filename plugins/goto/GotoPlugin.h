/***************************************************************************
           GotoPlugin.h  -  Plugin for moving the view to a certain position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#ifndef GOTO_PLUGIN_H
#define GOTO_PLUGIN_H

#include "config.h"

#include <QString>

#include "GotoPluginBase.h"

namespace Kwave
{


    class GotoPlugin: public Kwave::GotoPluginBase
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        GotoPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~GotoPlugin() override;

    protected:

        /** Returns the command to be emitted */
        QString command() const override;

        /** Returns the title of the dialog */
        QString title() const override;

    };
}

#endif /* GOTO_PLUGIN_H */

//***************************************************************************
//***************************************************************************
