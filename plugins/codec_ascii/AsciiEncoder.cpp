/*************************************************************************
       AsciiEncoder.cpp  -  encoder for ASCII data
                             -------------------
    begin                : Sun Nov 26 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include "config.h"

#include <math.h>
#include <stdlib.h>

#include <new>

#include <QApplication>
#include <QList>
#include <QTextCodec>
#include <QVariant>

#include <KLocalizedString>

#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"

#include "AsciiCodecPlugin.h"
#include "AsciiEncoder.h"

/***************************************************************************/
Kwave::AsciiEncoder::AsciiEncoder()
    :Kwave::Encoder(), m_dst()
{
    m_dst.setCodec(QTextCodec::codecForName("UTF-8"));
    LOAD_MIME_TYPES
    REGISTER_COMPRESSION_TYPES
}

/***************************************************************************/
Kwave::AsciiEncoder::~AsciiEncoder()
{
}

/***************************************************************************/
Kwave::Encoder *Kwave::AsciiEncoder::instance()
{
    return new(std::nothrow) Kwave::AsciiEncoder();
}

/***************************************************************************/
QList<Kwave::FileProperty> Kwave::AsciiEncoder::supportedProperties()
{
    // default is to support all known properties
    Kwave::FileInfo info;
    return info.allKnownProperties();
}

/***************************************************************************/
bool Kwave::AsciiEncoder::encode(QWidget *widget,
                                 Kwave::MultiTrackReader &src,
                                 QIODevice &dst,
                                 const Kwave::MetaDataList &meta_data)
{
    bool result = true;

    qDebug("AsciiEncoder::encode()");

    // get info: tracks, sample rate
    const Kwave::FileInfo info(meta_data);
    unsigned int tracks = info.tracks();
    unsigned int bits   = info.bits();
    sample_index_t length = info.length();

    do {
	// open the output device
	if (!dst.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
	    Kwave::MessageBox::error(widget,
		i18n("Unable to open the file for saving."));
	    result = false;
	    break;
	}

	// output device successfully opened
	m_dst.setDevice(&dst);

	// write out the default properties:
	// sample rate, bits, tracks, length
	m_dst << META_PREFIX << "'rate'="   << info.rate() << endl;
	m_dst << META_PREFIX << "'tracks'=" << tracks << endl;
	m_dst << META_PREFIX << "'bits'="   << bits   << endl;
	m_dst << META_PREFIX << "'length'=" << length << endl;

	// write out all other, non-standard properties that we have
	QMap<Kwave::FileProperty, QVariant> properties = info.properties();
	QMap<Kwave::FileProperty, QVariant>::Iterator it;
	QList<Kwave::FileProperty> supported = supportedProperties();
	for (it=properties.begin(); it != properties.end(); ++it) {
	    Kwave::FileProperty p = it.key();
	    QVariant            v = it.value();

	    if (!supported.contains(p))
		continue;
	    if (!info.canLoadSave(p))
		continue;

	    // write the property
	    m_dst << META_PREFIX << "'" << info.name(p) << "'='"
	          << Kwave::Parser::escape(v.toString()).toUtf8()
	          << "'" << endl;
	}

	// write out all labels
	Kwave::LabelList labels(meta_data);
	foreach (const Kwave::Label &label, labels) {
	    m_dst << META_PREFIX << "'label["
	    << QString::number(label.pos()) << "]'='"
	    << Kwave::Parser::escape(label.name()).toUtf8()
	    << "'" << endl;
	}

	sample_index_t rest = length;
	sample_index_t pos  = 0;
	while (rest-- && !src.isCanceled()) {
	    // write out one track per line
	    for (unsigned int track=0; track < tracks; track++) {
		Kwave::SampleReader *reader = src[track];
		Q_ASSERT(reader);
		if (!reader) break;

		// read one single sample
		sample_t sample = 0;
		if (!reader->eof()) (*reader) >> sample;

		// print out the sample value
		m_dst.setFieldWidth(9);
		m_dst << sample;

		// comma as separator between the samples
		if (track != tracks-1)
		    m_dst << ", ";
	    }

	    // as comment: current position [samples]
	    m_dst << " # ";
	    m_dst.setFieldWidth(12);
	    m_dst << pos;
	    pos++;

	    // end of line
	    m_dst << endl;
	}

    } while (false);

    // end of file
    m_dst << "# EOF " << endl << endl;

    m_dst.setDevice(Q_NULLPTR);
    dst.close();

    return result;
}

/***************************************************************************/
/***************************************************************************/
