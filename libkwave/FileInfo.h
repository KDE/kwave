/***************************************************************************
             FileInfo.h  -  information about an audio file
			     -------------------
    begin                : Mar 13 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

#include "config.h"
#include <qmap.h>
#include <qstring.h>
#include <qvariant.h>

class FileInfo
{
public:

    /** Constructor */
    FileInfo();

    /** Destructor */
    virtual ~FileInfo();

    /** returns the number of samples */
    inline unsigned int length() { return m_length; };

    /** Sets the length in samples */
    inline void setLength(unsigned int length) { m_length = length; };

    /** returns the sample rate [samples/second] */
    inline double rate() { return m_rate; };

    /** sets a new sample rate */
    inline void setRate(double rate) { m_rate = rate; };

    /** returns the number of bits per sample */
    inline unsigned int bits() { return m_bits; };

    /** sets a new resolution in bits per sample */
    inline void setBits(unsigned int bits) { m_bits = bits; };

    /** returns the number of tracks */
    inline unsigned int tracks() { return m_tracks; };

    /** Sets the number of tracks */
    inline void setTracks(unsigned int tracks) { m_tracks = tracks; };

    /**
     * Sets a property to a new value. If the property does not already
     * exist, a new one will be added to the info. If an empty value is
     * passed, the property will be removed if exists.
     * @param name string with the property's name, case-sensitive
     * @param value a QVariant with the new value
     */
    virtual void set(const QString &name, const QVariant &value);

    /**
     * Returns the value of a property. If the property does not exist,
     * an empty value will be returned.
     * @param name string with the property's name, case-sensitive
     * @return value of the property or empty if not found
     */
    virtual const QVariant &get(const QString &name);

    /** Clears the list of all properties. */
    virtual void clear();

protected:

    /** length in samples */
    unsigned int m_length;

    /** sample rate */
    double m_rate;

    /** bits per sample */
    unsigned int m_bits;

    /** number of tracks */
    unsigned int m_tracks;

    /** list of properties */
    QMap<QString, QVariant> m_properties;
};

#endif /* _FILE_INFO_H_ */
