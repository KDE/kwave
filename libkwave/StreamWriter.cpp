/***************************************************************************
        StreamWriter.cpp - adapter between writers and sample source
			     -------------------
    begin                : Sun Aug 23 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#include "config.h"

#include "libkwave/SampleArray.h"
#include "libkwave/StreamWriter.h"

//***************************************************************************
Kwave::StreamWriter::StreamWriter()
    :Kwave::Writer()
{
}

//***************************************************************************
Kwave::StreamWriter::~StreamWriter()
{
    // If this assert gets hit, you deleted a writer without calling
    // flush() before. Flushing in the destructor is problematic and
    // might be too late when the derived classes' destructor was
    // already done and signal/slot connections were already released!
    Q_ASSERT(!m_buffer_used);
    if (m_buffer_used) qWarning("StreamWriter was not flushed!?");
}

//***************************************************************************
bool Kwave::StreamWriter::write(const Kwave::SampleArray &buffer,
                                unsigned int &count)
{
    // NOTE: even zero length input has to be passed, needed for flushing!

    if (count < buffer.size()) {
	// have to work with a resized copy - slow :-(
	Kwave::SampleArray data = buffer;
	if (!data.resize(count))
	    return false; // out-of-memory ?

	// emit the resized copy
	emit output(data);
    } else {
	// directly emit the buffer - fast :-)
	emit output(buffer);
    }

    count = 0;
    return true;
}

//***************************************************************************
//***************************************************************************
