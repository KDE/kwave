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

#ifndef _COMPRESSION_H_
#define _COMPRESSION_H_

#include "config.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>

#include <kdemacros.h>

#include "libkwave/SampleFormat.h"

namespace Kwave
{

    class KDE_EXPORT Compression
    {
    public:

	/** supported compression types */
	enum {
	    NONE         = 0,
	    G711_ULAW    = AF_COMPRESSION_G711_ULAW,
	    G711_ALAW    = AF_COMPRESSION_G711_ALAW,
	    MS_ADPCM     = AF_COMPRESSION_MS_ADPCM,
	    GSM          = AF_COMPRESSION_GSM,
	    MPEG_LAYER_I = 600,
	    MPEG_LAYER_II,
	    MPEG_LAYER_III,
	    OGG_VORBIS,
	    OGG_OPUS,
	    FLAC
	};

	/**
	 * default constructor
	 */
	Compression();

	/**
	 * Constructor, from "int"
	 * @param value the integer representation of a compression
	 */
	Compression(int value);

	/**
	 * Copy constructor
	 *
	 * @param other another compression to copy from
	 */
	Compression(const Kwave::Compression &other);

	/**
	 * Complete constructor
	 *
	 * @param value numeric integer value
	 * @param name descriptive name of the compression, non-localized
	 * @param sample_formats list of supported sample formats
	 * @param has_abr whether average bitrate mode is supported
	 * @param has_vbr whether variable bitrate mode is supported
	 */
	explicit Compression(int value,
	                     const QString &name,
	                     const QList<Kwave::SampleFormat> &sample_formats,
	                     bool has_abr,
	                     bool has_vbr);

	/** destructor */
	virtual ~Compression();

	/**
	 * Returns the descriptive name of the compression, already localized
	 * @return localized name
	 */
	QString name() const;

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

	/**
	 * Gets the integer representation of this compression type
	 * @return integer value or -1 for "invalid"
	 */
	int toInt() const;

    private:

	/** fills the static map of known compression types on first use */
	void fillMap();

    private:

	/** internal container class with meta data */
	class CompressionInfo: public QSharedData
	{
	public:

	    /** constructor */
	    CompressionInfo();

	    /** copy constructor */
	    CompressionInfo(const CompressionInfo &other);

	    /** destructor */
	    virtual ~CompressionInfo();

	    /** integer representation */
	    int m_as_int;

	    /** non-localized descriptive name */
	    QString m_name;

	    /** list of supported sample formats */
	    QList<Kwave::SampleFormat> m_sample_formats;

	    /** true if ABR mode is supported */
	    bool m_has_abr;

	    /** true if VBR mode is supported */
	    bool m_has_vbr;
	};

    private:

	/** pointer to the shared meta data */
	QSharedDataPointer<CompressionInfo> m_data;

	/** map with all known compression types */
	static QMap<int, Kwave::Compression> m_map;
    };

}

#endif /* _COMPRESSION_H_ */

//***************************************************************************
//***************************************************************************
