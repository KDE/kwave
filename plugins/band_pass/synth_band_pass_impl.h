	/*

	Copyright (C) 2000 Jeff Tranter
			           tranter@pobox.com

			  (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    --
    Copyright (C) 2003 Thomas Eschenbacher <thomas.eschenbacher@gmx.de> (THE)

    2003-06-06, THE, copied into the Kwave project for fixing a serious buffer
                     overflow bug in calculateBlock.
    */

#include <math.h>
#include <assert.h>
#include "artsmodules.h"
#include "stdsynthmodule.h"

#include "synth_band_pass.h"
#include "c_filter_stuff.h"

using namespace Arts;

class Synth_BAND_PASS_impl:
    virtual public Synth_BAND_PASS_skel,
    virtual public Arts::StdSynthModule
{
protected:
	float _bw, _frequency;
	filter f;

public:
	Synth_BAND_PASS_impl() : _bw(10), _frequency(500)
	{
	}

	float bw() { return _bw; }
	void bw(float newBw) { _bw = newBw; }

	float frequency() { return _frequency; }
	void frequency(float newFrequency) { _frequency = newFrequency; }

	void streamInit()
	{
		initfilter(&f);
	}

	void calculateBlock(unsigned long samples)
	{
		setfilter_2polebp(&f,_frequency,_bw);

		unsigned long i;

		for(i=0;i<samples;i++)
		{
			// do the filtering
  			f.x = invalue[i];
  			f.y = f.cx * f.x + f.cx1 * f.x1 + f.cx2 * f.x2
	    			+ f.cy1 * f.y1 + f.cy2 * f.y2;
  			f.x2 = f.x1;
  			f.x1 = f.x;
  			f.y2 = f.y1;
  			f.y1 = f.y;
  			outvalue[i] = 0.95 * f.y;
		}
	}
};

// REGISTER_IMPLEMENTATION(Synth_BAND_PASS_impl);
