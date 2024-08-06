/***************************************************************************
       GotoPluginBase.h  -  base class for the goto plugin
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

#ifndef GOTO_PLUGIN_BASE_H
#define GOTO_PLUGIN_BASE_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libgui/SelectTimeWidget.h"
#include "libkwave/Plugin.h"

namespace Kwave
{

    class GotoPluginBase: public Kwave::Plugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        GotoPluginBase(QObject *parent, const QVariantList &args);

        /** Destructor */
        virtual ~GotoPluginBase() override;

        /**
         * Shows a dialog for selecting the range and emits a command
         * for applying the selection if OK has been pressed.
         * @see Kwave::Plugin::setup
         */
        virtual QStringList *setup(QStringList &previous_params)
            override;

        /**
         * selects the position
         * @see Kwave::Plugin::start()
         */
        virtual int start(QStringList &params) override;

    protected:

        /** Returns the command to be emitted */
        virtual QString command() const = 0;

        /** Returns the title of the dialog */
        virtual QString title() const = 0;

        /** Reads values from the parameter list */
        int interpreteParameters(QStringList &params);

    private:

        /** selected mode for position: by time, samples, percentage */
        Kwave::SelectTimeWidget::Mode m_mode;

        /** position in milliseconds, samples or percents */
        unsigned int m_position;
    };
}

#endif /* GOTO_PLUGIN_BASE_H */

//***************************************************************************
//***************************************************************************
