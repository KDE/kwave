/*************************************************************************
         StreamObject.h  -  base class with a generic sample source/sink
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

#ifndef STREAM_OBJECT_H
#define STREAM_OBJECT_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QList>
#include <QObject>
#include <QRecursiveMutex>

#include "libkwave/SampleArray.h"

class QVariant;

namespace Kwave
{
    class SampleSink;
    class SampleSource;

    class LIBKWAVE_EXPORT StreamObject: public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         *
         * @param parent a parent object, passed to QObject (optional)
         */
        explicit StreamObject(QObject *parent = nullptr);

        /** Destructor */
        ~StreamObject() override;

        /**
         * Returns the default number of tracks that the source provides
         * @return number of tracks, default is 1
         */
        virtual unsigned int tracks() const { return 1; }

        virtual Kwave::StreamObject * operator [] (unsigned int track)
        {
            return (track == 0) ? this : nullptr;
        }

        /**
         * Returns the number of tracks used as input, usually defaults to
         * the number of tracks
         * @return number of input tracks
         */
        virtual unsigned int tracksIn() const { return tracks(); }

        /**
         * Returns the number of tracks used as output, usually defaults to
         * the number of tracks
         * @return number of output tracks
         */
        virtual unsigned int tracksOut() const { return tracks(); }

        /**
         * Returns the sink that corresponds to one specific track
         * if the object has multiple tracks. For single-track objects
         * it returns "this" for the first index and 0 for all others
         * @param track index of the track
         * @return a stream object or nullptr
         */
        virtual Kwave::StreamObject *in(unsigned int track);

        /**
         * Returns the source that corresponds to one specific track
         * if the object has multiple tracks. For single-track objects
         * it returns "this" for the first index and 0 for all others
         * @param track index of the track
         * @return a stream object or nullptr
         */
        virtual Kwave::StreamObject *out(unsigned int track);

        /**
         * Returns the number of input. Defaults to the number of
         * tracks, but can be overwritten for objects that have different
         * numbers of tracks / inputs.
         * @return number of inputs
         */
        virtual unsigned int inputs() const
        {
            return tracks();
        }

        /**
         * Returns the number of outputs. Defaults to the number of
         * tracks, but can be overwritten for objects that have different
         * numbers of tracks / outputs.
         * @return number of outputs
         */
        virtual unsigned int outputs() const
        {
            return tracks();
        }

        /**
         * Returns the block size used for producing data.
         * @return currently 32k [samples]
         */
        virtual unsigned int blockSize() const;

        /**
         * Sets an attribute of a Kwave::StreamObject.
         * @param attribute name of the attribute, with the signature of
         *        a Qt SLOT(\<name\>(QVariant value))
         * @param value the new value of the attribute, stored in a QVariant
         */
        void setAttribute(const char *attribute,
                          const QVariant &value);

        /**
         * Switch interactive mode on or off. In interactive mode we
         * use a smaller block size for creating objects to get better
         * response time to parameter changes. In non-interactive mode
         * the block size is higher for better performance.
         */
        static void setInteractive(bool interactive);

        /** returns true if the transfer has been canceled */
        virtual bool isCanceled() const { return m_canceled; }

        /**
         * Connect the output of this stream object to another stream
         * object that acts as a sink.
         *
         * @param sink pointer to a StreamObject
         * @param port index of the input or 0
         */
        virtual void connectTo(Kwave::StreamObject *sink,
                               unsigned int port);

        /**
         * Receive input data, version without port.
         *
         * @param data the sample data to process
         */
        virtual void input(Kwave::SampleArray &data);

        /**
         * Receive input data, default version which just calls
         * input(0, data).
         * @param port index of the input, use zero if only
         *                    one input exists
         * @param data the sample data to process
         */
        virtual void input(unsigned int port,
                           Kwave::SampleArray &data)
        {
            Q_UNUSED(port);
            Q_ASSERT(port == 0);
            input(data);
        }

        /**
         * Should be called from the worker function of the stream
         * object to publish resulting data to the next stage.
         * Can be connected to one or more inputs of of other stream
         * objects.
         */
        virtual void output(Kwave::SampleArray &data);

    public slots:

        /**
         * Can be connected to other stream objects to cancel the current
         * transfer.
         */
        void cancel();

    signals:

        /**
         * Emitted by setAttribute and connected to the corresponding
         * slot.
         */
        void attributeChanged(const QVariant value);

        /**
         * emitted when cancel() is called, can be connected
         * to the cancel() slot of child objects
         */
        void sigCancel();

    private:

        /** info about a sink that is connected to one of our outputs */
        struct ConnectedSink
        {
            /** pointer to the sink, must not be nullptr */
            Kwave::StreamObject *m_sink;

            /** zero based index of the input (default is 0) */
            unsigned int         m_port;
        };

        /** List of connected sinks which receive our output */
        QList<ConnectedSink> m_sinks;

        /** Mutex for locking access to setAttribute (recursive) */
        QRecursiveMutex m_lock_set_attribute;

        /** interactive mode: if enabled, use smaller block size */
        static bool m_interactive;

        /**
         * Initialized as false, will be true if the transfer has
         * been canceled
         */
        bool m_canceled;
    };
}

#endif /* STREAM_OBJECT_H */

//***************************************************************************
//***************************************************************************
