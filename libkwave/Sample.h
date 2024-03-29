/***************************************************************************
               Sample.h  -  definition of the sample type
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

#ifndef SAMPLE_H
#define SAMPLE_H

//***************************************************************************

#include <QtGlobal>

#include <limits>

/** use an unsigned integer for sample offset/count calculations */
typedef quint64 sample_index_t;

/** the highest possible sample index (currently limited to 32bit integer */
#define SAMPLE_INDEX_MAX (static_cast<sample_index_t>( \
        std::numeric_limits<qint32>::max()) )

/**
 * Currently a "sample" is defined as a 32 bit integer
 * with 24 significant bits
 */
typedef qint32 sample_t;

/** native data type used for storing a sample_t */
typedef qint32 sample_storage_t;

/** number of significant bits per sample */
#define SAMPLE_BITS 24

/** number of bits used for storing samples in integer representation */
#define SAMPLE_STORAGE_BITS (sizeof(sample_storage_t) * 8)

/** lowest sample value */
#define SAMPLE_MIN (-(1 << (SAMPLE_BITS - 1)) + 1)

/** highest sample value */
#define SAMPLE_MAX (+(1 << (SAMPLE_BITS - 1)) - 1)

/**
 * Simple conversion from float to sample_t
 */
static inline sample_t float2sample(const float f) {
    return static_cast<sample_t>(
        f * static_cast<float>(1 << (SAMPLE_BITS - 1)));
}

/**
 * Simple conversion from sample_t to float
 */
static inline float sample2float(const sample_t s) {
    return static_cast<float>(
        static_cast<float>(s) / static_cast<float>(1 << (SAMPLE_BITS - 1)));
}

/**
 * Simple conversion from sample_t to double
 */
static inline double sample2double(const sample_t s) {
    return static_cast<double>(
        static_cast<double>(s) / static_cast<double>(1 << (SAMPLE_BITS - 1)));
}

/**
 * Simple conversion from double to sample_t
 */
static inline sample_t double2sample(const double f) {
    return static_cast<sample_t>(
        f * static_cast<double>(1 << (SAMPLE_BITS - 1)));
}

#endif /* SAMPLE_H */

//***************************************************************************
//***************************************************************************
