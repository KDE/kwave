/***************************************************************************
    VolumePlugin.h  -  Plugin for adjusting a signal's volume
                             -------------------
    begin                : Sun Sep 02 2001
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

#ifndef VOLUME_PLUGIN_H
#define VOLUME_PLUGIN_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"

namespace Kwave
{
    class VolumePlugin: public Kwave::Plugin
    {
        Q_OBJECT

    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        VolumePlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~VolumePlugin() override;

        /**
         * Shows a dialog for selecting a volume.
         * @see Kwave::Plugin::setup
         */
        virtual QStringList *setup(QStringList &previous_params)
            override;

        /**
         * Does the amplification operation
         * @param params list of strings with parameters
         */
        void run(QStringList params) override;

    protected:

        /** Reads values from the parameter list */
        int interpreteParameters(QStringList &params);

    private:
        /** List of parameters */
        QStringList m_params;

        /** amplification factor */
        float m_factor;
    };
}

#endif /* VOLUME_PLUGIN_H */

//***************************************************************************
//***************************************************************************
