/***************************************************************************
 ArtsKwaveMultiTrackFilter.h  -  template for multi-track Kwave filters
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _ARTS_KWAVE_MULTI_TRACK_FILTER_H_
#define _ARTS_KWAVE_MULTI_TRACK_FILTER_H_

#include <string.h>
#include <qglobal.h> // for warning()
#include <qvector.h>

#include <arts/artsflow.h>
#include <arts/connect.h>

#include "libkwave/ArtsMultiTrackFilter.h"

template <class FILTER, class FILTER_IMPL>
class ArtsKwaveMultiTrackFilter: public ArtsMultiTrackFilter
{
public:

    /**
     * Constructor. Creates all filter objects.
     * @param tracks the number of tracks
     */
    ArtsKwaveMultiTrackFilter(unsigned int tracks)
	:m_count(tracks), m_filter(tracks)
    {
	// create all filters
	unsigned int i;
	for (i=0; i < m_count; ++i) {
	    m_filter.insert(i, 0);
	
	    FILTER_IMPL *f = new FILTER_IMPL();
	    ASSERT(f);
	    if (f) m_filter.insert(i, new FILTER(FILTER::_from_base(f)));
	
	    if (!m_filter[i]) {
		warning("ArtsMultiTrackFilter: filter creation failed!!!");
		m_count = i;
		break;
	    }
	}
    };

    /**
     * Destructor. Removes all filters in reverse order.
     */
    virtual ~ArtsKwaveMultiTrackFilter()
    {
	m_filter.setAutoDelete(true);
	while (m_count--) {
	    m_filter.remove(m_count);
	}
    };

    /**
     * Returns a pointer to one of the filter objects. (normally used for
     * connecting it to another Arts::Object).
     * @param i index of the track [0..count-1]
     * @return pointer to the object or 0 if index is out of range
     */
    virtual Arts::Object *operator[](unsigned int i) {
	return m_filter[i];
    };

    /**
     * Connects the input of the filters to an ArtsMultiSource.
     * @param source reference to a source (or previous filter)
     * @param output_name name of the output streams of the source
     * @param input_name name of the input streams of our filters (sink)
     */
    virtual void connectInput(ArtsMultiSource &source,
	const std::string &output_name = "output",
	const std::string &input_name = "input")
    {
	for (unsigned int i=0; i < m_count; ++i)
	    Arts::connect(*(source[i]),   output_name,
	                  *(m_filter[i]), input_name);
    };

    /**
     * Connects the input of the filters to the output of a single
     * aRts object.
     * @param source reference to the Arts::Object used as source
     * @param output_name name of the output streams of the source
     * @param input_name name of the input streams of our filters (sink)
     */
    virtual void connectInput(Arts::Object &source,
	const std::string &output_name = "output",
	const std::string &input_name = "input")
    {
	for (unsigned int i=0; i < m_count; ++i)
	    Arts::connect(source,         output_name,
	                  *(m_filter[i]), input_name);
    };

    /**
     * Connects the output of the filters to an ArtsMultiSink.
     * @param sink reference to a sink (or next filter)
     * @param input_name name of the input streams of the sinks
     * @param output_name name of the output streams of our filters (source)
     */
    virtual void connectOutput(ArtsMultiSink &sink,
	const std::string &input_name = "input",
	const std::string &output_name = "output")
    {
	for (unsigned i=0; i < m_count; ++i)
	    Arts::connect(*(m_filter[i]), output_name,
	                  *(sink[i]),     input_name);
    };

    /** Starts all filters. */
    virtual void start() {
	for (unsigned int i=0; i < m_count; ++i) m_filter[i]->start();
    };

    /** Stops all filters. */
    virtual void stop() {
	for (unsigned int  i=0; i < m_count; ++i) m_filter[i]->stop();
    };

private:

    /** number of fiters (tracks) */
    unsigned int m_count;

    /** list of the filters */
    QVector<FILTER> m_filter;
};

#endif /* _ARTS_KWAVE_MULTI_TRACK_FILTER_H_ */
