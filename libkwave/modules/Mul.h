/***************************************************************************
                  Mul.h  -  multiplier
                             -------------------
    begin                : Thu Nov 01 2007
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

#ifndef MUL_H
#define MUL_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>

#include "libkwave/SampleArray.h"
#include "libkwave/SampleSink.h"
#include "libkwave/SampleSource.h"

class QVariant;

namespace Kwave
{

    class LIBKWAVE_EXPORT Mul: public Kwave::SampleSource
    {
        Q_OBJECT
        public:
            /** Constructor */
            Mul();

            /** Destructor */
            virtual ~Mul() override;

            /** does nothing, work is done automatically in multiply() */
            virtual void goOn() override;

        signals:
            /** emits a block with the interpolated curve */
            void output(Kwave::SampleArray data);

        public slots:

            /** receives input data for input A */
            void input_a(Kwave::SampleArray data);

            /** receives input data for input B */
            void input_b(Kwave::SampleArray data);

            /** sets input A to a constant value (as float) */
            void set_a(const QVariant &a);

            /** sets input B to a constant value (as float) */
            void set_b(const QVariant &b);

        private:

            /** does the calculation */
            virtual void multiply();

        private:

            /** queue for input A */
            QQueue<Kwave::SampleArray> m_queue_a;

            /** queue for input B */
            QQueue<Kwave::SampleArray> m_queue_b;

            /** semaphore to wait for input on input A */
            QSemaphore m_sem_a;

            /** semaphore to wait for input on input B */
            QSemaphore m_sem_b;

            /** buffer for input A (currently in work) */
            Kwave::SampleArray m_a;

            /** buffer for input B (currently in work) */
            Kwave::SampleArray m_b;

            /** buffer for output data */
            Kwave::SampleArray m_buffer_x;

            /** if true, input A is a constant */
            bool m_a_is_const;

            /** if true, input B is a constant */
            bool m_b_is_const;

            /** if m_a_is_const is set, the value of A */
            float m_value_a;

            /** if m_b_is_const is set, the value of B */
            float m_value_b;

            /** mutex for locking access to the queues */
            QMutex m_lock;
    };
}

#endif /* MUL_H */

/***************************************************************************/
/***************************************************************************/
