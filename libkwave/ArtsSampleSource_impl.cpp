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
    :ArtsSampleSource_skel(), Arts::StdSynthModule(), m_reader(0)
{
}

//***************************************************************************
ArtsSampleSource_impl::ArtsSampleSource_impl(SampleReader *rdr)
    :ArtsSampleSource_skel(), Arts::StdSynthModule(), m_reader(rdr)
{
}

//***************************************************************************
void ArtsSampleSource_impl::calculateBlock(unsigned long samples)
{
    unsigned long i = 0;

    if (m_reader && !(m_reader->eof()) && samples) {
	// fill the buffer with samples
	QMemArray<sample_t> buffer(samples);
	*m_reader >> buffer;
	unsigned int read_samples = buffer.size();
	for (i=0; i < read_samples; i++)
	    source[i] = sample2float(buffer[i]);
    }

    // pad the rest with zeroes
    while (i < samples) source[i++] = 0.0;
}

//***************************************************************************
REGISTER_IMPLEMENTATION(ArtsSampleSource_impl);

//***************************************************************************
//***************************************************************************
