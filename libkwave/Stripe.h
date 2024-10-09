/***************************************************************************
               Stripe.h  -  continuous block of samples
                             -------------------
    begin                : Feb 09 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STRIPE_H
#define STRIPE_H

#include "config.h"
#include "libkwave_export.h"

#include <QByteArray>
#include <QtGlobal>
#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QMutex>
#include <QSharedData>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

//***************************************************************************
namespace Kwave
{
    class LIBKWAVE_EXPORT Stripe
    {
    public:

        /**
         * Default constructor. Creates an empty stripe with zero-length.
         */
        Stripe();

        /** Copy constructor */
        Stripe(const Stripe &other);

        /** Move constructor */
        Stripe(Stripe &&other);

        /**
         * Constructor. Creates a new zero-length stripe.
         * @param start position within the track
         */
        Stripe(sample_index_t start);

        /**
         * Constructor. Creates a stripe that already contains samples,
         * copied from a buffer with samples.
         *
         * @param start position within the track
         * @param samples array with initial samples
         */
        Stripe(sample_index_t start, const Kwave::SampleArray &samples);

        /**
         * Constructor. Creates a stripe that already contains samples,
         * copied from another stripe with offset.
         *
         * @param start position within the track
         * @param stripe source stripe to copy from
         * @param offset offset within the source stripe
         */
        Stripe(sample_index_t start, Stripe &stripe, unsigned int offset);

        /**
         * Destructor.
         */
        virtual ~Stripe();

        /** move assignment */
        Stripe & operator = (Stripe &&other) noexcept;

        /**
         * Returns the start position of the stripe within the track.
         */
        sample_index_t start() const;

        /**
         * Sets a new start position for the stripe
         */
        void setStart(sample_index_t start);

        /**
         * Returns the current length of the stripe in samples.
         */
        unsigned int length() const;

        /**
         * Returns the position of the last sample of the stripe,
         * or zero if the length is zero
         */
        sample_index_t end() const;

        /**
         * Resizes the stripe to a new number of samples. If the array
         * size is reduced, samples from the end are thrown away. If
         * the size is increased, samples with zero value will be added
         * to the end.
         * @param length new length of the array [samples]
         * @return new length [samples]
         */
        unsigned int resize(unsigned int length);

        /**
         * Appends an array of samples to the end of the stripe.
         * @param samples array with the samples
         * @param offset the offset within the array
         * @param count number of samples in the array
         * @return number of samples appended
         */
        unsigned int append(const Kwave::SampleArray &samples,
                            unsigned int offset,
                            unsigned int count);

        /**
         * Deletes a range of samples
         * @param offset index of the first sample, relative to the start of
         *        the stripe [0...length()-1]
         * @param length number of samples
         */
        void deleteRange(unsigned int offset, unsigned int length);

        /**
         * Copies the content of an array of samples into the stripe.
         * @param offset the offset within the stripe (target)
         * @param source array of samples to be copied
         * @param srcoff offset within the source array
         * @param srclen length of the data in the source array
         * @warning this method is intended to be used only internally
         *          and lacks any error-checking in order to be fast!
         */
        void overwrite(unsigned int offset, const Kwave::SampleArray &source,
                       unsigned int srcoff, unsigned int srclen);

        /**
         * Combine this stripe with another stripe
         * @param offset the offset within this stripe to put the other stripe
         * @param other reference to another stripe
         * @return true if succeeded or false if failed (e.g. out of memory)
         */
        bool combine(unsigned int offset, Kwave::Stripe &other);

        /**
         * Reads out samples from the stripe into a buffer
         *
         * @param buffer array for samples to be read (destination)
         * @param dstoff offset within the destination buffer
         * @param offset the offset within the stripe (source)
         * @param length number of samples to read
         * @return number of samples read
         * @warning this method is intended to be used only internally
         *          and lacks any error-checking in order to be fast!
         */
        unsigned int read(Kwave::SampleArray &buffer, unsigned int dstoff,
                          unsigned int offset, unsigned int length);

        /**
         * Returns the minimum and maximum sample value within a range
         * of samples.
         * @param first index of the first sample
         * @param last index of the last sample
         * @param min receives the lowest value (must be initialized)
         * @param max receives the highest value (must be initialized)
         */
        void minMax(unsigned int first, unsigned int last,
                    sample_t &min, sample_t &max);

        /**
         * Operator for appending an array of samples to the
         * end of the stripe.
         */
        Stripe &operator << (const Kwave::SampleArray &samples);

        /** compare operator */
        bool operator == (const Stripe &other) const;

        /** assignment operator */
        Stripe &operator = (const Stripe &other);

        /**
         * Container for a list of stripes that covers a range of samples.
         * @note the first/last list entry may contain some samples
         *       before/after the selected range
         */
        class List: public QList<Kwave::Stripe>
        {
        public:

            /** Default constructor */
            List()
                :QList<Kwave::Stripe>(), m_left(0), m_right(0)
            {
            }

            /** Constructor */
            List(sample_index_t left, sample_index_t right)
                :QList<Kwave::Stripe>(), m_left(left), m_right(right)
            {
            }

            /** Destructor */
            virtual ~List()
            {
            }

            /** returns the index of the first sample */
            inline sample_index_t left() const { return m_left; }

            /** returns the index of the last sample */
            inline sample_index_t right() const { return m_right; }

        private:
            /** index of the first sample */
            sample_index_t m_left;

            /** index of the last sample */
            sample_index_t m_right;
        };

    private:

        /** mutex for locking some operations */
        QMutex m_lock;

        /** start position within the track */
        sample_index_t m_start;

        /** pointer to the shared data */
        Kwave::SampleArray m_data;

    };
}

#endif /* STRIPE_H */

//***************************************************************************
//***************************************************************************
