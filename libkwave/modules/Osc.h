/***************************************************************************
                  Osc.h  -  simple sine oscillator
                             -------------------
    begin                : Tue Nov 06 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef OSC_H
#define OSC_H

#include "config.h"

#include <QtGlobal>
#include <QObject>
#include <QVariant>

#include "libkwave/SampleSource.h"

namespace Kwave
{

    class Q_DECL_EXPORT Osc: public Kwave::SampleSource
    {
        Q_OBJECT
        public:
            /** Constructor */
            Osc();

            /** Destructor */
            virtual ~Osc() Q_DECL_OVERRIDE;

            /** does the calculation */
            virtual void goOn() Q_DECL_OVERRIDE;

        signals:
            /** emits a block with sine wave data */
            void output(Kwave::SampleArray data);

        public slots:
            /**
             * Sets the frequency of the sine wave, normed to the
             * sample frequency. You should pass the frequency that
             * you want, divided through the sample frequency.
             * If you never call this, the frequency will be undefined!
             */
            void setFrequency(const QVariant &f);

            /**
             * Sets the phase of the sine wave in RAD [0...2*Pi].
             * The default setting is zero.
             */
            void setPhase(const QVariant &p);

            /**
             * Sets the amplitude of the sine wave, normed to the
             * range of [0.0 ... 1.0]. The default is 1.0.
             */
            void setAmplitude(const QVariant &a);

        private:

            /** buffer for output data */
            Kwave::SampleArray m_buffer;

            /** current time multiplied by 2*Pi*f */
            double m_omega_t;

            /** frequency [samples/period] */
            double m_f;

            /** amplitude [0...1] */
            double m_a;
    };
}

#endif /* OSC_H */

//***************************************************************************
//***************************************************************************
