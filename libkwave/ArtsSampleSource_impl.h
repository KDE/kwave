/***************************************************************************
    ArtsSampleSource_impl.h  -  adapter for converting from Kwave to aRts
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

#ifndef _ARTS_SAMPLE_SOURCE_H_
#define _ARTS_SAMPLE_SOURCE_H_

#include "config.h"
#include <arts/artsflow.h>
#include <arts/stdsynthmodule.h>

#include "libkwave/SampleReader.h"
#include "libkwave/ArtsSampleSource.h"

//***************************************************************************
class ArtsSampleSource_impl
    :virtual public ArtsSampleSource_skel,
     virtual public Arts::StdSynthModule
{
public:

    /** Default constructor. Should never be used */
     ArtsSampleSource_impl();

     /**
      * Constructor.
      * @param rdr SampleReader that deliveres a stream of samples
      *            from Kwave
      */
     ArtsSampleSource_impl(SampleReader *rdr);

    /**
     * Receiver and data processing function.
     * @see aRts/MCOP documentation
     */
    void calculateBlock(unsigned long samples);

    /** Returns true if no more input data is available */
    inline bool done() { return ((!m_reader) || (m_reader->eof())); };

protected:

    /** Source of samples from Kwave */
    SampleReader *m_reader;

};

#endif /* _ARTS_SAMPLE_SOURCE_H_ */
