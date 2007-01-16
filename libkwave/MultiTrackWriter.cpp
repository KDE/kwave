/***************************************************************************
    MultiTrackWriter.cpp - writer for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
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

#include <qmemarray.h>
#include <qptrlist.h>

#include "libkwave/Matrix.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"

#ifndef min
#define min(x,y) (( (x) < (y) ) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) (( (x) > (y) ) ? (x) : (y) )
#endif

//***************************************************************************
MultiTrackWriter::MultiTrackWriter()
    :QObject(), QPtrVector<SampleWriter>(), m_cancelled(false)
{
    setAutoDelete(true);
}

//***************************************************************************
MultiTrackWriter::~MultiTrackWriter()
{
    clear();
}

//***************************************************************************
MultiTrackWriter &MultiTrackWriter::operator << (
	const MultiTrackReader &source)
{
    unsigned int src_tracks = source.count();
    unsigned int dst_tracks = count();

    Q_ASSERT(src_tracks);
    Q_ASSERT(dst_tracks);
    if (!src_tracks || !dst_tracks) return *this;

    if (src_tracks != dst_tracks) {
	// create a mixer matrix and pass everything through

	// ### ALPHA: process sample per sample          ###
	// ### still using the same code as in playback  ###

	// create a translation matrix for mixing up/down to the desired
	// number of output channels
	Matrix<double> matrix(src_tracks, dst_tracks);
	unsigned int x, y;
	for (y=0; y < dst_tracks; y++) {
	    unsigned int m1, m2;
	    m1 = y * src_tracks;
	    m2 = (y+1) * src_tracks;

	    for (x=0; x < src_tracks; x++) {
		unsigned int n1, n2;
		n1 = x * dst_tracks;
		n2 = n1 + dst_tracks;

		// get the common area of [n1..n2] and [m1..m2]
		unsigned int l  = max(n1, m1);
		unsigned int r = min(n2, m2);

		matrix[x][y] = (r > l) ?
		    (double)(r-l) / (double)src_tracks : 0.0;
	    }
	}

	QMemArray<sample_t> in_samples(src_tracks);
	QMemArray<sample_t> out_samples(dst_tracks);

	while (!(source.eof())) {
	    // read input vector
	    unsigned int x;
	    for (x=0; x < src_tracks; x++) {
		in_samples[x] = 0;
		SampleReader *stream = source[x];
		Q_ASSERT(stream);
		if (!stream) continue;

		sample_t act;
		(*stream) >> act;
		in_samples[x] = act;
	    }

	    // multiply matrix with input to get output
	    unsigned int y;
	    for (y=0; y < dst_tracks; y++) {
		double sum = 0;
		for (x=0; x < src_tracks; x++) {
		    sum += (double)in_samples[x] * matrix[x][y];
		}
		out_samples[y] = (sample_t)sum;
	    }

	    // write samples to the target stream
	    for (y = 0; y < dst_tracks; y++) {
		if (m_cancelled) break;
		*at(y) << out_samples[y];
	    }

	}

    } else {
	// process 1:1
	unsigned int track;
	for (track = 0; track < src_tracks; ++track) {
	    *at(track) << *(source[track]);
	    if (m_cancelled) break;
	}
    }

    return *this;
}

//***************************************************************************
bool MultiTrackWriter::insert(unsigned int track, const SampleWriter *writer)
{
    if (writer) {
        connect(writer, SIGNAL(proceeded()), this, SLOT(proceeded()));
    }
    return QPtrVector<SampleWriter>::insert(track, writer);
}

//***************************************************************************
void MultiTrackWriter::proceeded()
{
    unsigned int pos = 0;
    unsigned int track;
    for (track=0; track < count(); ++track) {
	SampleWriter *w = at(track);
	if (w) pos += (w->position() - w->first());
    }
    emit progress(pos);
}

//***************************************************************************
void MultiTrackWriter::cancel()
{
    m_cancelled = true;
}

//***************************************************************************
unsigned int MultiTrackWriter::last()
{
    unsigned int last = 0;
    const unsigned int tracks = count();
    for (unsigned int track=0; track < tracks; ++track) {
	SampleWriter *w = (*this)[track];
	if (w && w->last() > last) last = w->last();
    }
    return last;
}

//***************************************************************************
void MultiTrackWriter::clear()
{
    flush();
    setAutoDelete(false);
    while (!isEmpty()) {
	unsigned int last = count()-1;
	SampleWriter *writer = at(last);
	remove(last);
	resize(last);
	if (writer) delete writer;
    }
}

//***************************************************************************
void MultiTrackWriter::flush()
{
    const unsigned int tracks = count();
    for (unsigned int track=0; track < tracks; ++track) {
	SampleWriter *w = (*this)[track];
	if (w) w->flush();
    }
}

//***************************************************************************
//***************************************************************************
