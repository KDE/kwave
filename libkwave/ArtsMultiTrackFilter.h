/***************************************************************************
 ArtsMultiTrackFilter.h  -  interface for multi-track aRts filters
                             -------------------
    begin                : Sun Dec 9 2001
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

#ifndef _ARTS_MULTI_TRACK_FILTER_H_
#define _ARTS_MULTI_TRACK_FILTER_H_

#include <string.h>
#include "libkwave/ArtsMultiSource.h"
#include "libkwave/ArtsMultiSink.h"

class ArtsMultiTrackFilter
    :public ArtsMultiSource, public ArtsMultiSink
{

public:

    /**
     * Returns a pointer to one of the filter objects. (normally used for
     * connecting it to another Arts::Object).
     * @param i index of the track [0..count-1]
     * @return pointer to the object or 0 if index is out of range
     */
    virtual Arts::Object *operator[](unsigned int i) = 0;

    /**
     * Connects the input of the filters to an ArtsMultiSource.
     * @param source reference to a source (or previous filter)
     * @param output_name name of the output streams of the source
     * @param input_name name of the input streams of our filters (sink)
     */
    virtual void connectInput(ArtsMultiSource &source,
	const std::string &output_name = "output",
	const std::string &input_name = "input") = 0;

    /**
     * Connects the input of the filters to the output of a single
     * aRts object.
     * @param source reference to the Arts::Object used as source
     * @param output_name name of the output streams of the source
     * @param input_name name of the input streams of our filters (sink)
     */
    virtual void connectInput(Arts::Object &source,
	const std::string &output_name = "output",
	const std::string &input_name = "input") = 0;

    /**
     * Connects the output of the filters to an ArtsMultiSink.
     * @param sink reference to a sink (or next filter)
     * @param input_name name of the input streams of the sinks
     * @param output_name name of the output streams of our filters (source)
     */
    virtual void connectOutput(ArtsMultiSink &sink,
	const std::string &input_name = "input",
	const std::string &output_name = "output") = 0;

    /** Starts all filters. */
    virtual void start() = 0;

    /** Stops all filters. */
    virtual void stop() = 0;

};

#endif /* _ARTS_MULTI_TRACK_FILTER_H_ */
