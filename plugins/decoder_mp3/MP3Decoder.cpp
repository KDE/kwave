/*************************************************************************
         MP3Decoder.cpp  -  decoder for MP3 data
                             -------------------
    begin                : Wed Aug 07 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libgui/ConfirmCancelProxy.h"

#include "MP3Decoder.h"

//***************************************************************************
MP3Decoder::MP3Decoder()
    :Decoder(), m_source(0)
{
    /* included in KDE: */
    addMimeType("audio/x-mpga",   i18n("MPEG layer1 audio"),
                "*.mpga *.mpg *.mp1");
    addMimeType("audio/x-mp2",    i18n("MPEG layer2 audio"), "*.mp2");
    addMimeType("audio/x-mp3",    i18n("MPEG layer3 audio"), "*.mp3");

    /* like defined in RFC3003 */
    addMimeType("audio/mpeg",     i18n("MPEG audio"), "*.mpga *.mpg *.mp1");
    addMimeType("audio/mpeg",     i18n("MPEG layer2 audio"), "*.mp2");
    addMimeType("audio/mpeg",     i18n("MPEG layer3 audio"), "*.mp3");
}

//***************************************************************************
MP3Decoder::~MP3Decoder()
{
    if (m_source) close();
}

//***************************************************************************
Decoder *MP3Decoder::instance()
{
    return new MP3Decoder();
}

//***************************************************************************
bool MP3Decoder::open(QWidget *widget, QIODevice &src)
{
    info().clear();
    ASSERT(!m_source);
    if (m_source) warning("MP3Decoder::open(), already open !");

    return false; // ###
}

//***************************************************************************
bool MP3Decoder::decode(QWidget */*widget*/, MultiTrackWriter &dst)
{
    ASSERT(m_source);
    if (!m_source) return false;

    return false; // ###
}

//***************************************************************************
void MP3Decoder::close()
{
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
