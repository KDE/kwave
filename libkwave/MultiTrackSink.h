/*************************************************************************
       MultiTrackSink.h  -  template for multi-track sinks
                            -------------------
    begin                : Sat Oct 20 2007
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

#ifndef MULTI_TRACK_SINK_H
#define MULTI_TRACK_SINK_H

#include "config.h"

#include <new>

#include <QList>
#include <QObject>

#include "libkwave/SampleSink.h"

namespace Kwave
{

    template <class SINK, const bool INITIALIZE>
    class MultiTrackSink: public Kwave::SampleSink,
                          private QList<SINK *>
    {
    public:
        /**
         * Constructor
         * @param tracks number of tracks
         * @param parent a parent object, passed to QObject (optional)
         */
        MultiTrackSink(unsigned int tracks, QObject *parent = nullptr)
            :Kwave::SampleSink(parent),
            QList<SINK *>()
        {
            Q_UNUSED(tracks)
            Q_ASSERT(INITIALIZE || (tracks == 0));
            Q_ASSERT(QList<SINK *>::size() == static_cast<int>(tracks));
        }

        /** Destructor */
        ~MultiTrackSink() override
        {
            clear();
        }

        /** Returns true when all sinks are done */
        bool done() const override
        {
            foreach (Kwave::SampleSink *s,
                     static_cast< QList<SINK *> >(*this))
                if (s && !s->done()) return false;
            return true;
        }

        /**
         * Returns the number of tracks that the sink provides
         * @return number of tracks
         */
        unsigned int tracks() const override
        {
            return static_cast<unsigned int>(QList<SINK *>::size());
        }

        /**
         * Returns the sink that corresponds to one specific track
         * if the object has multiple tracks. For single-track objects
         * it returns "this" for the first index and 0 for all others
         */
        inline virtual SINK *at(unsigned int track) const {
            return QList<SINK *>::at(track);
        }

        /** @see the Kwave::MultiTrackSink.at()... */
        inline virtual SINK * operator [] (unsigned int track) override {
            return at(track);
        }

        /**
         * Insert a new track with a sink.
         *
         * @param track index of the track [0...N-1]
         * @param sink pointer to a Kwave::SampleSink
         * @return true if successful, false if failed
         */
        virtual bool insert(unsigned int track, SINK *sink) {
            QList<SINK *>::insert(track, sink);
            QObject::connect(this, SIGNAL(sigCancel()),
                             sink, SLOT(cancel()), Qt::DirectConnection);
            return (at(track) == sink);
        }

        /** Remove all tracks / sinks */
        virtual void clear() {
            while (!QList<SINK *>::isEmpty())
                delete QList<SINK *>::takeLast();
        }

        /**
         * overloaded function, in most cases the sink objects are connected
         * to some sigCancel() signals of other stream objects, so that the
         * cancel() slot of this object is not called.
         */
        bool isCanceled() const override
        {
            if (Kwave::SampleSink::isCanceled())
                return true;

            unsigned int tracks = this->tracks();
            for (unsigned int track = 0; track < tracks; track++) {
                const SINK *sink = at(track);
                if (sink && sink->isCanceled())
                    return true;
            }

            return false;
        }

    };

    /**
     * Specialized version that internally initializes all objects
     * by generating them through their default constructor.
     */
    template <class SINK>
    class MultiTrackSink<SINK, true>
        :public Kwave::MultiTrackSink<SINK, false>
    {
    public:
        /**
         * Constructor
         *
         * @param tracks number of tracks
         * @param parent a parent object, passed to QObject (optional)
         */
        MultiTrackSink(unsigned int tracks, QObject *parent = nullptr)
            :Kwave::MultiTrackSink<SINK, false>(0, parent)
        {
            for (unsigned int i = 0; i < tracks; i++)
                this->insert(i, new(std::nothrow) SINK());
        }

        /** Destructor */
        virtual ~MultiTrackSink() { }
    };

}

#endif /* _MULTI_TRACK_SINK_H */

//***************************************************************************
//***************************************************************************
