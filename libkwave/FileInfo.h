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
#include "libkwave/TypesMap.h"

/**
 * @enum FileProperty
 * Enumeration type that lists all file properties. If you extend this list,
 * please don't forget to add a verbose name and a description it in
 * FileInfo.cpp
 */
typedef enum {
    // please sort in new items alphabetically...
    INF_ALBUM = 0,           /**< name of the album */
    INF_ANNOTATION,          /**< annotation/comment */
    INF_ARCHIVAL,            /**< archival location */
    INF_ARTIST,              /**< name ot the artist */
    INF_AUTHOR,              /**< name of the author/artist */
    INF_CD,                  /**< number of the CD in an album */
    INF_COMMISSIONED,        /**< commissioned */
    INF_COMMENTS,            /**< comments */
    INF_COMPRESSION,         /**< compression mode (libaudiofile) */
    INF_COPYRIGHT,           /**< copyright */
    INF_CREATION_DATE,       /**< creation date */
    INF_ENGINEER,            /**< engineer */
    INF_FILENAME,            /**< name of the file */
    INF_FILESIZE,            /**< size of the file in bytes */
    INF_GENRE,               /**< genre */
    INF_KEYWORDS,            /**< keywords */
    INF_MEDIUM,              /**< medium */
    INF_MIMETYPE,            /**< mime type of the file format */
    INF_NAME,                /**< name of the song or whatever */
    INF_PRODUCT,             /**< product */
    INF_SAMPLE_FORMAT,       /**< sample format (libaudiofile) */
    INF_SOFTWARE,            /**< software */
    INF_SOURCE,              /**< source */
    INF_SOURCE_FORM,         /**< source form */
    INF_SUBJECT,             /**< subject */
    INF_TECHNICAN,           /**< technican */
    INF_TRACK                /**< track of the CD */
} FileProperty;

/**
 * @class FileInfo
 * Holds various properties of a file.
 */
class FileInfo
{
public:

    /** Constructor */
    FileInfo();

    /** Copy constructor */
    FileInfo(const FileInfo &inf);

    /** Destructor */
    virtual ~FileInfo();

    /** Copy everything from another FileInfo */
    void copy(const FileInfo &source);

    /** Return true if the info is equal to another info */
    bool equals(const FileInfo &other);

    /** Assignment operator */
    inline FileInfo operator = (const FileInfo &source) {
	copy(source);
	return *this;
    };

    /** Compare operator */
    inline bool operator == (const FileInfo &other) {
	return equals(other);
    }

    /** returns the number of samples */
    inline unsigned int length() const { return m_length; };

    /** Sets the length in samples */
    inline void setLength(unsigned int length) { m_length = length; };

    /** returns the sample rate [samples/second] */
    inline double rate() const { return m_rate; };

    /** sets a new sample rate */
    inline void setRate(double rate) { m_rate = rate; };

    /** returns the number of bits per sample */
    inline unsigned int bits() const { return m_bits; };

    /** sets a new resolution in bits per sample */
    inline void setBits(unsigned int bits) { m_bits = bits; };

    /** returns the number of tracks */
    inline unsigned int tracks() const { return m_tracks; };

    /** Sets the number of tracks */
    inline void setTracks(unsigned int tracks) { m_tracks = tracks; };

    /** Returns true if the given property exists */
    inline bool contains(const FileProperty property) const {
	return m_properties.contains(property);
    };

    /**
     * Sets a property to a new value. If the property does not already
     * exist, a new one will be added to the info. If an empty value is
     * passed, the property will be removed if exists.
     * @param key identifies the property
     * @param value a QVariant with the new value
     */
    void set(FileProperty key, const QVariant &value);

    /**
     * Returns the value of a property. If the property does not exist,
     * an empty value will be returned.
     * @param key identifies the property
     * @return value of the property or empty if not found
     */
    const QVariant &get(FileProperty key) const;

    /**
     * Returns true if a property is only internal.
     */
    bool isInternal(FileProperty key);

    /**
     * Returns true if a property is intended to be saved into or
     * loaded from a file.
     */
    bool canLoadSave(FileProperty key);

    /**
     * Returns the name of a property.
     */
    inline QString name(FileProperty key) {
	return m_property_map.name(key);
    };

    /**
     * Returns the localized description of a property.
     */
    inline QString description(FileProperty key) {
	return m_property_map.description(key, false);
    };

    /** Returns a reference to the list of non-standard properties */
    const QMap<FileProperty, QVariant> &properties() const {
	return m_properties;
    };

    /** Clears the list of all properties. */
    void clear();

    /** dumps all properties to stdout, useful for debugging */
    void dump();

private:

    /** length in samples */
    unsigned int m_length;

    /** sample rate */
    double m_rate;

    /** bits per sample */
    unsigned int m_bits;

    /** number of tracks */
    unsigned int m_tracks;

    /** list of properties */
    QMap<FileProperty, QVariant> m_properties;

    /**
     * Pre-filled map with property names and descriptions
     */
    class PropertyTypesMap: public TypesMap<FileProperty, int>
    {
    public:
        virtual void fill();
    };

    /** map with properties and their names and descriptions */
    PropertyTypesMap m_property_map;

};

#endif /* _FILE_INFO_H_ */
