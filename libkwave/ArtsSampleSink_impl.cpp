/***************************************************************************
    ArtsSampleSink_impl.cpp  -  adapter for converting from aRts to Kwave
			     -------------------
    begin                : Nov 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "Sample.h"
#include "SampleWriter.h"
#include "ArtsSampleSink_impl.h"

#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/flowsystem.h>
#include <arts/stdsynthmodule.h>

//***************************************************************************
ArtsSampleSink_impl::ArtsSampleSink_impl()
    :ArtsSampleSink_skel(), Arts::StdSynthModule(),
    m_writer(0), m_done(false)
{
}

//***************************************************************************
ArtsSampleSink_impl::ArtsSampleSink_impl(SampleWriter *writer)
    :ArtsSampleSink_skel(), Arts::StdSynthModule(),
    m_writer(writer), m_done(false)
{
}

//***************************************************************************
void ArtsSampleSink_impl::calculateBlock(unsigned long samples)
{
    if (!m_writer) return;

    unsigned long i;
    sample_t sample = 0;

    // fill the sink with our input
    for(i=0;i < samples;i++) {
	sample = sample_t(sink[i] * (1 << 23));
	*m_writer << sample;
    }
}

//***************************************************************************
void ArtsSampleSink_impl::goOn()
{
    _node()->requireFlow();
}

//***************************************************************************
bool ArtsSampleSink_impl::done()
{
    return m_writer->eof();
}

//***************************************************************************
REGISTER_IMPLEMENTATION(ArtsSampleSink_impl);

//***************************************************************************
//***************************************************************************
