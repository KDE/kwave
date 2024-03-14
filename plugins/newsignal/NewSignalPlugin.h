/***************************************************************************
      NewSignalPlugin.h  -  plugin for creating a new signal
                             -------------------
    begin                : Wed Jul 18 2001
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

#ifndef NEW_SIGNAL_PLUGIN_H
#define NEW_SIGNAL_PLUGIN_H

#include "config.h"
#include "libkwave/Plugin.h"
#include <QObject>

namespace Kwave
{
    class NewSignalPlugin: public Kwave::Plugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        NewSignalPlugin(QObject *parent, const QVariantList &args);

        /** virtual Destructor */
        virtual ~NewSignalPlugin() Q_DECL_OVERRIDE;

        /**
         * Shows a dialog for creating a new signal and emits sigCommand if
         * OK has been pressed.
         * @see Kwave::Plugin::setup
         */
        virtual QStringList *setup(QStringList &previous_params)
            Q_DECL_OVERRIDE;

    protected:

        /** Reads values from the parameter list */
        int interpreteParameters(QStringList &params);

    private:
        /** number of samples */
        unsigned int m_samples;

        /** samples rate */
        unsigned int m_rate;

        /** bits per sample */
        unsigned int m_bits;

        /** number of tracks */
        unsigned int m_tracks;

        /** select by time or by samples */
        bool m_bytime;

    };
}

#endif /* NEW_SIGNAL_PLUGIN_H */

//***************************************************************************
//***************************************************************************
