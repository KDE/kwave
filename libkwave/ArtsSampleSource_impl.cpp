/***************************************************************************
    ArtsSampleSource_impl.cpp  -  adapter for converting from Kwave to aRts
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
#include "SampleReader.h"
#include "ArtsSampleSource_impl.h"

#include <arts/connect.h>
#include <arts/objectmanager.h>
#include <arts/flowsystem.h>
#include <arts/stdsynthmodule.h>

//***************************************************************************
ArtsSampleSource_impl::ArtsSampleSource_impl()
    :ArtsSampleSource_skel(), Arts::StdSynthModule(),
     m_reader(0), m_done(false)
{
}

//***************************************************************************
ArtsSampleSource_impl::ArtsSampleSource_impl(SampleReader *rdr)
    :ArtsSampleSource_skel(), Arts::StdSynthModule(),
     m_reader(rdr), m_done(false)
{
}

//***************************************************************************
void ArtsSampleSource_impl::calculateBlock(unsigned long samples)
{
    unsigned long i = 0;
    sample_t sample = 0;
	
    if (m_reader && !(m_reader->eof())) {
	// fill the buffer with samples sample
	for (i=0;i < samples;i++) {
	    *m_reader >> sample;
	    source[i] = sample / double(1 << 23);
	    if (m_reader->eof()) break;
	}
    }

    // pad the rest with zeroes
    while (i < samples) source[i++] = 0;

    // detect eof()	
    if ((!m_reader) || (m_reader->eof())) m_done = true;
}

//***************************************************************************
REGISTER_IMPLEMENTATION(ArtsSampleSource_impl);

//***************************************************************************
//***************************************************************************
