/***************************************************************************
          Compression.h  -  Wrapper for a compression
                             -------------------
    begin                : Fri Jan 25 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "config.h"

#include <KLazyLocalizedString>
#include <QtGlobal>
#include <QList>
#include <QMap>
#include <QSharedData>
#include <QSharedDataPointer>

#include "libkwave/SampleFormat.h"

namespace Kwave
{

    class Q_DECL_EXPORT Compression
    {
    public:

	/**
	 * supported compression types
	 * @note for compatibility with older settings these values are
	 *       the same as defined in audiofile.h.
	 */
	typedef enum {
	    INVALID         =  -1,
	    NONE            =   0,
	    G722            = 501,
	    G711_ULAW       = 502,
	    G711_ALAW       = 503,
	    APPLE_ACE2      = 504,
	    APPLE_ACE8      = 505,
	    APPLE_MAC3      = 506,
	    APPLE_MAC6      = 507,
	    G726            = 517,
	    G728            = 518,
	    DVI_AUDIO       = 519,
	    GSM             = 520,
	    FS1016          = 521,
	    DV              = 522,
	    MS_ADPCM        = 523,
	    FLAC            = 530,
	    ALAC            = 540,
	    MPEG_LAYER_I    = 600,
	    MPEG_LAYER_II,
	    MPEG_LAYER_III,
	    OGG_VORBIS,
	    OGG_OPUS
	} Type;

	/**
	 * default constructor
	 */
	Compression();

	/**
	 * Constructor, from enum
	 * @param value the enum of an already known compression type
	 */
	explicit Compression(const Type value);

	/**
	 * Copy constructor
	 * @param other another compression to copy from
	 */
	explicit Compression(const Kwave::Compression &other);

	/** destructor */
	virtual ~Compression() {}

	/** assignment operator from sample_format_t */
	inline void assign(Type t) { m_type = t; }

	/**
	 * Returns the descriptive name of the compression, already localized
	 * @return localized name
	 */
	QString name() const;

	/**
	 * Returns the preferred mime type or an empty string
	 */
	QString preferredMimeType() const;

	/**
	 * Returns a list of supported sample formats
	 * @return list of sample formats, or empty list if none supported
	 */
	QList<Kwave::SampleFormat> sampleFormats() const;

	/**
	 * Returns whether average bitrate mode is supported
	 * @return true if supported, false if not
	 */
	bool hasABR() const;

	/**
	 * Returns whether variable bitrate mode is supported
	 * @return true if supported, false if not
	 */
	bool hasVBR() const;

	/** conversion to int (e.g. for use in plugin parameters) */
	inline int toInt() const { return static_cast<int>(m_type); }

	/** conversion from int  (e.g. for use in plugin parameters) */
	static Kwave::Compression::Type fromInt(int i);

	/** conversion to a numeric value from libaudiofile */
	static int toAudiofile(Kwave::Compression::Type compression);

	/** conversion from a numeric value from libaudiofile */
	static Kwave::Compression::Type fromAudiofile(int af_compression);

    private:

	/** fills the map with known compression types (if empty) */
	static void fillMap();

    private:

	/** internal storage of the compression type, see Type */
	Type m_type;

    private:

	/** internal container class with meta data */
	class Info
	{
	public:

	    /** default constructor */
	    Info();

	    /** copy constructor */
	    Info(const Info &other);

	    /**
	     * Constructor
	     *
	     * @param name descriptive name of the compression, non-localized
	     * @param mime_type preferred mime types (optional)
	     * @param sample_formats list of supported sample formats
	     * @param has_abr whether average bitrate mode is supported
	     * @param has_vbr whether variable bitrate mode is supported
	     */
	    Info(const KLazyLocalizedString &name,
	         const QString &mime_type,
	         const QList<Kwave::SampleFormat> &sample_formats,
	         bool has_abr,
	         bool has_vbr
	    );

	    /** destructor */
	    virtual ~Info();

	public:

	    /** non-localized descriptive name */
	    KLazyLocalizedString m_name;

	    /** preferred mime type (optional) */
	    QString m_mime_type;

	    /** list of supported sample formats */
	    QList<Kwave::SampleFormat> m_sample_formats;

	    /** true if ABR mode is supported */
	    bool m_has_abr;

	    /** true if VBR mode is supported */
	    bool m_has_vbr;
	};

    private:

	/** map with all known compression types */
	static QMap<int, Kwave::Compression::Info> m_map;
    };

}

#endif /* COMPRESSION_H */

//***************************************************************************
//***************************************************************************
