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

#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "config.h"

#include <QtGlobal>
#include <QFlags>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"
#include "libkwave/TypesMap.h"

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
	INF_ESTIMATED_LENGTH,    /**< estimated length in samples */
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
    class Q_DECL_EXPORT FileInfo: public Kwave::MetaData
    {
    public:
	/**
	 * @enum Flag
	 * flags additional meta information about file info entries
	 */
	typedef enum {
	    /** no flags */
	    FP_NONE           = 0,

	    /** for internal usage only, do not show to the user */
	    FP_INTERNAL       = 1,

	    /** readonly, cannot be modified by the user */
	    FP_READONLY       = 2,

	    /** available for the GUI, but not for loading/saving */
	    FP_NO_LOAD_SAVE   = 4,

	    /** represents a numeric value, otherwise handled as a string */
	    FP_FORMAT_NUMERIC = 8

	} Flag;

	Q_DECLARE_FLAGS(Flags, Flag)

	/** Default constructor, creates an empty file info object */
	FileInfo();

	/**
	 * Constructor, filters out all file global data from
	 * a list of meta data objects.
	 * @param meta_data_list a meta data list to copy from
	 */
	explicit FileInfo(const Kwave::MetaDataList &meta_data_list);

	/** Destructor */
	virtual ~FileInfo();

	/** returns the identifier of the "type" of this meta data object */
	static QString metaDataType() {
	    return QString::fromLatin1("FILE INFO");
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
	 * returns the flags of a property (meta data)
	 */
	inline Flags flags(FileProperty key) const {
	    return m_property_map.data(key);
	}

	/**
	 * Returns true if a property is only internal.
	 */
	inline bool isInternal(FileProperty key) const {
	    return (flags(key) & FP_INTERNAL);
	}

	/**
	 * Returns true if a property is intended to be saved into or
	 * loaded from a file.
	 */
	inline bool canLoadSave(FileProperty key) const {
	    return !(flags(key) & FP_NO_LOAD_SAVE);
	}

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

	/**
	 * Returns a file property key from a property name
	 */
	inline FileProperty fromName(const QString &name) const {
	    return m_property_map.findFromName(name);
	}

	/** Returns a list with all properties */
	const QMap<FileProperty, QVariant> properties() const;

	/** Returns a list of all known non-standard properties */
	QList<FileProperty> allKnownProperties() const;

	/** dumps all properties to stdout, useful for debugging */
        virtual void dump() const Q_DECL_OVERRIDE;

    private:

	/**
	 * Pre-filled map with property names and descriptions
	 */
	class PropertyTypesMap: public Kwave::TypesMap<FileProperty, Flags>
	{
	public:
	    /** constructor */
	    explicit PropertyTypesMap()
		:Kwave::TypesMap<FileProperty, Flags>()
	    {
		fill();
	    }

	    /** fills the list */
            virtual void fill() Q_DECL_OVERRIDE;

	    /** returns a list of all properties */
	    virtual QList<FileProperty> all() const;
	};

	/** map with properties and their names and descriptions */
	PropertyTypesMap m_property_map;

    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Kwave::FileInfo::Flags)

#endif /* FILE_INFO_H */

//***************************************************************************
//***************************************************************************
