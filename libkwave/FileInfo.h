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

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>

#include <kdemacros.h>

#include "libkwave/Sample.h"
#include "libkwave/TypesMap.h"
#include "libkwave/LabelList.h"

/**
 * @enum FileProperty
 * Enumeration type that lists all file properties. If you extend this list,
 * please don't forget to add a verbose name and a description of it in
 * FileInfo.cpp
 */
typedef enum {
    // please sort in new items alphabetically...
    INF_ALBUM = 0,           /**< name of the album */
    INF_ANNOTATION,          /**< annotation/comment */
    INF_ARCHIVAL,            /**< archival location */
    INF_AUTHOR,              /**< name of the author/artist */
    INF_BITRATE_LOWER,       /**< lower bitrate limit */
    INF_BITRATE_NOMINAL,     /**< nominal bitrate */
    INF_BITRATE_UPPER,       /**< upper bitrate limit */
    INF_BITS_PER_SAMPLE,     /**< number of bits per sample */
    INF_CD,                  /**< number of the CD in an album */
    INF_COMMISSIONED,        /**< commissioned */
    INF_COMMENTS,            /**< comments */
    INF_COMPRESSION,         /**< compression mode (libaudiofile) */
    INF_CONTACT,             /**< contact information for creator */
    INF_COPYRIGHT,           /**< copyright text */
    INF_COPYRIGHTED,         /**< "copyright" flag */
    INF_CREATION_DATE,       /**< creation date */
    INF_ENGINEER,            /**< engineer */
    INF_FILENAME,            /**< name of the file */
    INF_FILESIZE,            /**< size of the file in bytes */
    INF_GENRE,               /**< genre */
    INF_ISRC,                /**< International Standard Recording Code */
    INF_KEYWORDS,            /**< keywords */
    INF_LABELS,              /**< labels/markers */
    INF_LENGTH,              /**< length of the file in samples */
    INF_LICENSE,             /**< license information */
    INF_MEDIUM,              /**< medium */
    INF_MIMETYPE,            /**< mime type of the file format */
    INF_MPEG_EMPHASIS,       /**< MPEG emphasis mode */
    INF_MPEG_LAYER,          /**< MPEG Layer, I/II/III */
    INF_MPEG_MODEEXT,        /**< MPEG mode extension */
    INF_MPEG_VERSION,        /**< MPEG version */
    INF_NAME,                /**< name of the song or whatever */
    INF_ORGANIZATION,        /**< organization/label */
    INF_ORIGINAL,            /**< true=original, false=copy */
    INF_PERFORMER,           /**< name ot the performer */
    INF_PRIVATE,             /**< "private" bit */
    INF_PRODUCT,             /**< product */
    INF_SAMPLE_FORMAT,       /**< sample format (libaudiofile) */
    INF_SAMPLE_RATE,         /**< sample rate (bits/sample) */
    INF_SOFTWARE,            /**< software */
    INF_SOURCE,              /**< source */
    INF_SOURCE_FORM,         /**< source form */
    INF_SUBJECT,             /**< subject */
    INF_TECHNICAN,           /**< technican */
    INF_TRACK,               /**< track of the CD */
    INF_TRACKS,              /**< number of tracks of the signal */
    INF_VBR_QUALITY,         /**< base quality of an ogg file in VBR mode */
    INF_VERSION              /**< version/remix */
} FileProperty;

/** increment operator for FileProperty */
static inline FileProperty operator ++ (FileProperty &prop) {
    prop = static_cast<FileProperty>(static_cast<unsigned int>(prop) + 1);
    return prop;
}

/**
 * @class FileInfo
 * Holds various properties of a file.
 */
class KDE_EXPORT FileInfo
{
public:

    /** Default constructor, creates an empty file info object */
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
    inline FileInfo & operator = (const FileInfo &source) {
	copy(source);
	return *this;
    }

    /** Compare operator */
    inline bool operator == (const FileInfo &other) {
	return equals(other);
    }

    /** returns the number of samples */
    inline sample_index_t length() const { return m_length; }

    /** Sets the length in samples */
    inline void setLength(unsigned int length) { m_length = length; }

    /** returns the sample rate [samples/second] */
    inline double rate() const { return m_rate; }

    /** sets a new sample rate */
    inline void setRate(double rate) { m_rate = rate; }

    /** returns the number of bits per sample */
    inline unsigned int bits() const { return m_bits; }

    /** sets a new resolution in bits per sample */
    inline void setBits(unsigned int bits) { m_bits = bits; }

    /** returns the number of tracks */
    inline unsigned int tracks() const { return m_tracks; }

    /** Sets the number of tracks */
    inline void setTracks(unsigned int tracks) { m_tracks = tracks; }

    /** Returs the list of labels. @note can be modified! */
    inline LabelList &labels() { return m_labels; }

    /** Returs the list of labels. @note cannot be modified! */
    inline const LabelList &labels() const { return m_labels; }

    /** Returns true if the given property exists */
    inline bool contains(const FileProperty property) const {
	return m_properties.contains(property);
    }

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
    QVariant get(FileProperty key) const;

    /**
     * Returns true if a property is only internal.
     */
    bool isInternal(FileProperty key) const;

    /**
     * Returns true if a property is intended to be saved into or
     * loaded from a file.
     */
    bool canLoadSave(FileProperty key) const;

    /**
     * Returns the name of a property.
     */
    inline QString name(FileProperty key) const {
	return m_property_map.name(key);
    }

    /**
     * Returns the localized description of a property.
     */
    inline QString description(FileProperty key) const {
	return m_property_map.description(key, false);
    }

    /** Returns a reference to the list of non-standard properties */
    const QMap<FileProperty, QVariant> &properties() const {
	return m_properties;
    }

    /** Returns a list of all known non-standard properties */
    QList<FileProperty> allKnownProperties();

    /** Clears the list of all properties. */
    void clear();

    /** dumps all properties to stdout, useful for debugging */
    void dump() const;

private:

    /** length in samples */
    unsigned int m_length;

    /** sample rate */
    double m_rate;

    /** bits per sample */
    unsigned int m_bits;

    /** number of tracks */
    unsigned int m_tracks;

    /** list of labels */
    LabelList m_labels;

    /** list of properties */
    QMap<FileProperty, QVariant> m_properties;

    /**
     * Pre-filled map with property names and descriptions
     */
    class PropertyTypesMap: public TypesMap<FileProperty, int>
    {
    public:
	/** constructor */
	explicit PropertyTypesMap()
	    :TypesMap<FileProperty, int>()
	{
	    fill();
	}

	/** Destructor */
	virtual ~PropertyTypesMap() {}

	/** fills the list with */
	virtual void fill();

	/** returns a list of all properties */
	virtual QList<FileProperty> all() const;
    };

    /** map with properties and their names and descriptions */
    PropertyTypesMap m_property_map;

};

#endif /* _FILE_INFO_H_ */

