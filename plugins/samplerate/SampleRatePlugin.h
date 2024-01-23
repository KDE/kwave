/*************************************************************************
     SampleRatePlugin.h  -  sample rate conversion
                             -------------------
    begin                : Tue Jul 07 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef SAMPLE_RATE_PLUGIN_H
#define SAMPLE_RATE_PLUGIN_H

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * @class SampleRatePlugin
     * Change the sample rate of a signal
     */
    class SampleRatePlugin: public Kwave::Plugin
    {
        Q_OBJECT

    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        SampleRatePlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        virtual ~SampleRatePlugin() Q_DECL_OVERRIDE;

        /**
         * changes the sample rate
         * @param params list of strings with parameters
         */
        virtual void run(QStringList params) Q_DECL_OVERRIDE;

    protected:

        /**
         * reads values from the parameter list
         * @param params list of strings with parameters
         * @return 0 if succeeded or negative error code if failed
         */
        int interpreteParameters(QStringList &params);

    private:

        /** list of parameters */
        QStringList m_params;

        /** new sample rate */
        double m_new_rate;

        /** if true, ignore selection and change whole signal */
        bool m_whole_signal;

    };
}

#endif /* SAMPLE_RATE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
