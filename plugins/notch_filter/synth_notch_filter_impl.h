/***************************************************************************
    synth_notch_filter_impl.h  -  aRts plugin for a notch filter
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>
#include <assert.h>
#include "artsmodules.h"
#include "stdsynthmodule.h"

#include "synth_notch_filter.h"
#include "c_filter_stuff.h"

using namespace Arts;

class Synth_NOTCH_FILTER_impl:
    virtual public Synth_NOTCH_FILTER_skel,
    virtual public Arts::StdSynthModule
{
protected:
    float _bw, _frequency;
    filter _f;

public:
    Synth_NOTCH_FILTER_impl()
        : _bw(1.0), _frequency(5.0)
    {
    }

    float bw() { return _bw; }
    void bw(float newBw) { _bw = newBw; }

    float frequency() {
	return _frequency;
    }

    void frequency(float newFrequency) {
	_frequency = newFrequency;
    }

    void streamInit() {
	initfilter(&_f);
    }

    void calculateBlock(unsigned long samples)
    {
	setfilter_peaknotch2(&_f,_frequency,-100.0,_bw);
	unsigned long i;

	for(i=0; i < samples; i++) {
	    // do the filtering
	    _f.x = invalue[i];
	    _f.y = _f.cx * _f.x + _f.cx1 * _f.x1 + _f.cx2 * _f.x2
	    	+ _f.cy1 * _f.y1 + _f.cy2 * _f.y2;
	    _f.x2 = _f.x1;
	    _f.x1 = _f.x;
	    _f.y2 = _f.y1;
	    _f.y1 = _f.y;
	    outvalue[i] = 0.95 * _f.y;
	}
    }
    
};

/* REGISTER_IMPLEMENTATION(Synth_NOTCH_FILTER_impl); */
