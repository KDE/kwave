/*************************************************************************
       AsciiDecoder.cpp  -  decoder for ASCII data
                             -------------------
    begin                : Sun Dec 03 2006
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

#include <QDateTime>
#include <QTextStream>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include "libkwave/CompressionType.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "AsciiCodecPlugin.h"
#include "AsciiDecoder.h"

//***************************************************************************
AsciiDecoder::AsciiDecoder()
    :Decoder(), m_source(0), m_dest(0)
{
    LOAD_MIME_TYPES;
}

//***************************************************************************
AsciiDecoder::~AsciiDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Decoder *AsciiDecoder::instance()
{
    return new AsciiDecoder();
}

//***************************************************************************
bool AsciiDecoder::open(QWidget *widget, QIODevice &src)
{
    info().clear();
    Q_ASSERT(!m_source);
    if (m_source) qWarning("AsciiDecoder::open(), already open !");

    // try to open the source
    if (!src.open(IO_ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    // take over the source
    m_source = &src;

    /********** Decoder setup ************/
    qDebug("--- AsciiDecoder::open() ---");

    QTextStream source(m_source);

    // read in all metadata until start of samples, EOF or user cancel
    qDebug("AsciiDecoder::open(...)");
    unsigned int linenr = 0;
    while (!source.atEnd()) {
	QString line = source.readLine().simplified();
	qDebug("META %5u %s", linenr++, line.toLocal8Bit().data());
	if (!line.length())
	    continue; // skip empty line
	if (line.startsWith("#") && !line.startsWith(META_PREFIX))
	    continue; // skip comment lines

	if (!line.startsWith(META_PREFIX)) {
	    // reached end of metadata
	    break;
	}
    }

    qDebug("--- THE ASCII DECODER IS NOT FUNCTIONAL YET ---");
    qDebug("---           sorry :-(                     ---");
    KMessageBox::sorry(widget,
	i18n("not implemented yet"),
	i18n("Sorry"));
    return false;

//     return true;
}

//***************************************************************************
bool AsciiDecoder::decode(QWidget * /* widget */, MultiTrackWriter &dst)
{
    Q_ASSERT(m_source);
    if (!m_source) return false;

    m_dest = &dst;
    QTextStream source(m_source);

    // read in all remaining data until EOF or user cancel
    qDebug("AsciiDecoder::decode(...)");
    unsigned int linenr = 0;
    while (!source.atEnd() && !dst.isCancelled()) {
	QString line = source.readLine();
	qDebug("DATA %5u %s", linenr++, line.toLocal8Bit().data());

	if (linenr > 50) break;
    }

    m_dest = 0;
    m_info.setLength(dst.last() ? dst.last()+1 : 0);

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void AsciiDecoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
