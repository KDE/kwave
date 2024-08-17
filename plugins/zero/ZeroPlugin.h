/*************************************************************************
           ZeroPlugin.h  -  wipes out the selected range of samples to zero
                             -------------------
    begin                : Fri Jun 01 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef ZERO_PLUGIN_H
#define ZERO_PLUGIN_H

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * @class ZeroPlugin
     * This is a very simple plugin that blanks the currently selected range of
     * samples with zeroes.
     */
    class ZeroPlugin: public Kwave::Plugin
    {
        Q_OBJECT

    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        ZeroPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~ZeroPlugin() override;

        /**
         * Fills the selected area with zeroes
         * @param params list of strings with parameters
         */
        void run(QStringList params) override;

    private:

        /** use an array with zeroes for faster filling */
        Kwave::SampleArray m_zeroes;

    };
}

#endif /* ZERO_PLUGIN_H */

//***************************************************************************
//***************************************************************************
