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

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>

#include <kdemacros.h>

#include "libkwave/Sample.h"
#include "libkwave/TypesMap.h"
#include "libkwave/MetaData.h"

namespace Kwave
{

    class MetaDataList;

    /**
    * @enum FileProperty
    * Enumeration type that lists all file properties. If you extend this list,
    * please don't forget to add a verbose name and a description of it in
    * FileInfo.cpp
    */
    typedef enum {
	INF_UNKNOWN = -1,        /**< dummy for unknown/unsupported property */
	// please sort in new items alphabetically...
	INF_ALBUM = 0,           /**< name of the album */
	INF_ANNOTATION,          /**< annotation/comment */
	INF_ARCHIVAL,            /**< archival location */
	INF_AUTHOR,              /**< name of the author/artist */
	INF_BITRATE_LOWER,       /**< lower bitrate limit */
	INF_BITRATE_MODE,        /**< bitrate mode (ABR, VBR, CBR, etc...) */
	INF_BITRATE_NOMINAL,     /**< nominal bitrate */
	INF_BITRATE_UPPER,       /**< upper bitrate limit */
	INF_BITS_PER_SAMPLE,     /**< number of bits per sample */
	INF_CD,                  /**< number of the CD in an album */
	INF_CDS,                 /**< number of CDs in an album */
	INF_CHANNELS,            /**< number of tracks of the signal */
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
	INF_OPUS_FRAME_LEN,      /**< Opus codec: frame length in ms */
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
	INF_TRACKS,              /**< number of tracks of the CD */
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
    class KDE_EXPORT FileInfo: public Kwave::MetaData
    {
    public:

	/** Default constructor, creates an empty file info object */
	FileInfo();

	/**
	 * Constructor, filters out all file global data from
	 * a list of meta data objects.
	 * @param meta_data_list a meta data list to copy from
	 */
	FileInfo(const Kwave::MetaDataList &meta_data_list);

	/** Destructor */
	virtual ~FileInfo();

	/** returns the identifier of the "type" of this meta data object */
	static QString metaDataType() {
	    return QString::fromAscii("FILE INFO");
	}

	/** returns the number of samples */
	sample_index_t length() const;

	/** Sets the length in samples */
	void setLength(sample_index_t length);

	/** returns the sample rate [samples/second] */
	double rate() const;

	/** sets a new sample rate */
	void setRate(double rate);

	/** returns the number of bits per sample */
	unsigned int bits() const;

	/** sets a new resolution in bits per sample */
	void setBits(unsigned int bits);

	/** returns the number of tracks */
	unsigned int tracks() const;

	/** Sets the number of tracks */
	void setTracks(unsigned int tracks);

	/** Returns true if the given property exists */
	bool contains(const FileProperty property) const;

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

	/** Returns a list with all properties */
	const QMap<FileProperty, QVariant> properties() const;

	/** Returns a list of all known non-standard properties */
	QList<FileProperty> allKnownProperties() const;

	/** dumps all properties to stdout, useful for debugging */
	void dump() const;

    private:

	/**
	 * Pre-filled map with property names and descriptions
	 */
	class PropertyTypesMap: public Kwave::TypesMap<FileProperty, int>
	{
	public:
	    /** constructor */
	    explicit PropertyTypesMap()
		:Kwave::TypesMap<FileProperty, int>()
	    {
		fill();
	    }

	    /** fills the list */
	    virtual void fill();

	    /** returns a list of all properties */
	    virtual QList<FileProperty> all() const;
	};

	/** map with properties and their names and descriptions */
	PropertyTypesMap m_property_map;

    };

}

#endif /* _FILE_INFO_H_ */

//***************************************************************************
//***************************************************************************
