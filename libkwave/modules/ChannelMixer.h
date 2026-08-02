/*************************************************************************
         ChannelMixer.h  -  matrix based mixer for multiple channels
                             -------------------
    begin                : Sun Oct 10 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#ifndef CHANNEL_MIXER_H
#define CHANNEL_MIXER_H

#include "config.h"
#include "libkwave_export.h"

#include <QPointer>
#include <QQueue>
#include <QString>
#include <QVector>

#include "libkwave/Connect.h"
#include "libkwave/SampleSource.h"
#include "libkwave/modules/SampleBuffer.h"
#include "libkwave/modules/StreamObject.h"

//***************************************************************************
namespace Kwave
{
    class MixerMatrix;

    class LIBKWAVE_EXPORT ChannelMixer: public Kwave::SampleSource
    {
        Q_OBJECT
        using StreamObject::input;
    public:
        /**
         * Constructor
         * @param inputs number of input channels
         * @param outputs number of output channels
         */
        ChannelMixer(unsigned int inputs, unsigned int outputs);

        /** Destructor */
        ~ChannelMixer() override;

        /**
         * Init function, you must call it once after creating and before
         * using this object. If the return value is false, you should
         * delete this object.
         * @return true if succeeded, false if failed
         */
        virtual bool init();

        /**
         * Returns the number of inputs.
         * @return number of inputs
         */
        unsigned int inputs() const override
        {
            return m_inputs;
        }

        /**
         * Returns the number of outputs.
         * @return number of outputs
         */
        unsigned int outputs() const override
        {
            return m_outputs;
        }

        /**
         * returns the number of tracks
         * @return always 0 -> use inputs() or outputs()
         */
        unsigned int tracks() const override { return 0; }

        /**
         * Returns the number of tracks used as output, usually defaults to
         * the number of tracks
         * @return number of output tracks
         */
        unsigned int tracksOut() const override { return m_outputs; }

        /**
         * Returns this for track 0, otherwise a nullptr
         *
         * @param track index of the track
         * @return a stream object or nullptr
         */
        Kwave::StreamObject *in(unsigned int track) override
        {
            return (track == 0) ? this : nullptr;
        }

        /**
         * Returns the source that corresponds to one specific track
         * if the object has multiple tracks. For single-track objects
         * it returns "this" for the first index and 0 for all others
         * @param track index of the track
         * @return a stream object or nullptr
         */
        Kwave::StreamObject *out(unsigned int track) override;

        /** does nothing, work is done automatically in mix() */
        void goOn() override
        {
        }

        /** receives a block with index + input data */
        virtual void input(unsigned int port,
                           Kwave::SampleArray &data) override;

    private:

        /** does the calculation */
        virtual void mix();

    private:

        /** mixer matrix */
        Kwave::MixerMatrix *m_matrix;

        /** number of inputs */
        unsigned int m_inputs;

        /** number of outputs */
        unsigned int m_outputs;

        /** queues for input data */
        QVector< QQueue<Kwave::SampleArray> > m_input_queue;

        /** buffers with output data */
        QVector< QPointer<Kwave::SampleBuffer> > m_output_buffer;

        /** mutex for locking access to the queues */
        QMutex m_lock;
    };

}

#endif /* CHANNEL_MIXER_H */

//***************************************************************************
//***************************************************************************
