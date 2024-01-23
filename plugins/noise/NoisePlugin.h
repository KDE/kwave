/*************************************************************************
    NoisePlugin.h  -  overwrites the selected range of samples with noise
                             -------------------
    begin                : Wed Dec 12 2001
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

#ifndef NOISE_PLUGIN_H
#define NOISE_PLUGIN_H

#include "config.h"

#include <QString>
#include <QStringList>

#include "libgui/FilterPlugin.h"

namespace Kwave
{
    class NoisePlugin: public Kwave::FilterPlugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        NoisePlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        virtual ~NoisePlugin() Q_DECL_OVERRIDE;

        /** Creates the setup dialog and connects it's signals */
        virtual Kwave::PluginSetupDialog *createDialog(QWidget *parent)
            Q_DECL_OVERRIDE;

        /**
         * Creates a multi-track filter with the given number of tracks
         * @param tracks number of tracks that the filter should have
         * @return pointer to the filter or null if failed
         */
        virtual Kwave::SampleSource *createFilter(unsigned int tracks)
            Q_DECL_OVERRIDE;

        /**
         * Returns true if the parameters have changed during pre-listen.
         */
        virtual bool paramsChanged() Q_DECL_OVERRIDE;

        /**
         * Update the filter with new parameters if it has changed
         * changed during the pre-listen.
         * @param filter the Kwave::SampleSource to be updated, should be the
         *               same one as created with createFilter()
         * @param force if true, even update if no settings have changed
         */
        virtual void updateFilter(Kwave::SampleSource *filter,
                                  bool force = false) Q_DECL_OVERRIDE;

        /**
         * Returns a verbose name of the performed action. Used for giving
         * the undo action a readable name.
         */
        virtual QString actionName() Q_DECL_OVERRIDE;

    protected:

        /** Reads values from the parameter list */
        virtual int interpreteParameters(QStringList &params) Q_DECL_OVERRIDE;

    protected slots:

        /**
         * called when the noise level setting changed during pre-listen
         * @param level noise level, as a factor between 0 and 1
         */
        void setNoiseLevel(double level);

    private:

        /** noise level, as linear factor ]0 ... 1.0] */
        double m_level;

        /** last value of m_level */
        double m_last_level;

    };
}

#endif /* NOISE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
