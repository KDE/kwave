/***************************************************************************
       FileInfoPlugin.h  -  plugin for editing file properties
                             -------------------
    begin                : Fri Jul 19 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef FILE_INFO_PLUGIN_H
#define FILE_INFO_PLUGIN_H

#include "config.h"
#include "libkwave/Plugin.h"
#include <QObject>

namespace Kwave
{
    class FileInfoPlugin: public Kwave::Plugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        FileInfoPlugin(QObject *parent, const QVariantList &args);

        /** virtual Destructor */
        ~FileInfoPlugin() override;

        /**
         * Shows a dialog for editing file properties.
         * @see Kwave::Plugin::setup
         */
        QStringList *setup(QStringList &) override;

    protected:

        /** Applies the new settings */
        void apply(Kwave::FileInfo &new_info);

    };
}

#endif /* FILE_INFO_PLUGIN_H */

//***************************************************************************
//***************************************************************************
