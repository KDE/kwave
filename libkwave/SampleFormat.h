/***************************************************************************
        SampleFormat.h  -  Map for all known sample formats
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef SAMPLE_FORMAT_H
#define SAMPLE_FORMAT_H

#include "config.h"

#include <QtGlobal>

#include "libkwave/TypesMap.h"

namespace Kwave
{

    class Q_DECL_EXPORT SampleFormat
    {
    public:
        /**
         * numeric representation of a sample format
         * @note for compatibility with older settings these values are
         *       the same as defined in audiofile.h.
         */
        typedef enum {
            Unknown  =  -1, /**< unknown/invalid format */
            Signed   = 401, /**< signed integer         */
            Unsigned = 402, /**< unsigned integer       */
            Float    = 403, /**< 32 bit floating point  */
            Double   = 404  /**< 64 bit floating point  */
        } Format;

        /** Default constructor */
        SampleFormat() { assign(Unknown); }

        /** Constructor, from SampleFormat::xxx */
        explicit SampleFormat(const Format x) { assign(x); }

        /** Copy constructor */
        SampleFormat(const SampleFormat &f) { assign(f); }

        /** Destructor */
        virtual ~SampleFormat() {}

        /** conversion operator to Format */
        inline operator Format() const { return m_format; }

        /** assignment operator from Format */
        inline void assign(Format f) { m_format = f; }

        /** compare operator */
        inline bool operator == (const Format &f) const {
            return (f == m_format);
        }

        /** conversion to int (e.g. for use in plugin parameters) */
        inline int toInt() const { return static_cast<int>(m_format); }

        /** conversion from int  (e.g. for use in plugin parameters) */
        void fromInt(int i);

    private:

        /** internal storage of the sample format, see Format */
        Format m_format;

    public:

        /** map for translating between index, sample format and name */
        class Q_DECL_EXPORT Map: public Kwave::TypesMap<int, Format>
        {
        public:
            /** Constructor */
            explicit Map();

            /** Destructor */
            virtual ~Map() Q_DECL_OVERRIDE;

            /** fills the list */
            virtual void fill() Q_DECL_OVERRIDE;
        };

    };
}

#endif /* SAMPLE_FORMAT_H */

//***************************************************************************
//***************************************************************************
