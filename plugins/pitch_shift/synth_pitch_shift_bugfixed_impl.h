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

#include "synth_pitch_shift_bugfixed.h"

using namespace Arts;

class Synth_PITCH_SHIFT_bugfixed_impl:
    virtual public Synth_PITCH_SHIFT_bugfixed_skel,
    virtual public Arts::StdSynthModule
{
protected:
	float _speed, _frequency;

	static const int MAXDELAY = 44100;
	float *dbuffer;
	float lfopos, b1pos, b2pos, b1inc, b2inc;
	bool b1reset, b2reset, initialized;
	int dbpos;

public:
	Synth_PITCH_SHIFT_bugfixed_impl() : _speed(1.0), _frequency(5.0)
	{
	}

	float speed() { return _speed; }
	void speed(float newSpeed) { _speed = newSpeed; }

	float frequency() { return _frequency; }
	void frequency(float newFrequency) { _frequency = newFrequency; }


	void streamInit()
	{
		dbuffer = new float[MAXDELAY];
		for (dbpos=0; dbpos<MAXDELAY; dbpos++)
			dbuffer[dbpos] = 0;

		dbpos = 0;
		initialized = false;
		lfopos = 0;
	}
	void streamEnd()
	{
		delete[] dbuffer;
	}

	void calculateBlock(unsigned long samples)
	{
		float *outend = outvalue + samples;
		float fsr = (float)samplingRate;
		float pi2 = 2*M_PI;
		float lfo, b1value, b2value;
		float lfoposinc = _frequency / fsr;

		if (!initialized)
		{
			if (_speed <= 1.0) {
				b1pos = b2pos = 0.0;
				b1inc = b2inc = 1.0 - _speed;
			} else {
				/* not yet sure what would be a nice initialization here? */
				b1pos = b2pos = 0.0;
				b1inc = b2inc = 0.0;
			}
			initialized = true;
		}

		while (outvalue < outend)
		{
			/*
			 * fill delay buffer with the input signal
			 */
			dbuffer[dbpos] = *invalue++;

			lfopos += lfoposinc;
			lfopos -= floor(lfopos);

			if (lfopos < 0.25) {
				b1reset = b2reset = false;
			}

			/*
			 * _speed < 1.0 (downpitching)
			 *
			 *  start with current sample and increase delay slowly
			 *
			 * _speed > 1.0 (uppitching)
			 *
			 *  start with a sample from long ago and slowly decrease delay
			 */
			if (!b1reset && lfopos > 0.25) {
				if (_speed <= 1.0) {
					b1pos = 0;
					b1inc = 1 - _speed;
				} else {
					b1inc = 1 - _speed;
					b1pos = 10 + ((-b1inc) * (1 / lfoposinc));
					/* 10+ are not strictly necessary */
				}
				b1reset = true;
			}

			if (!b2reset && lfopos > 0.75) {
				if (_speed <= 1.0) {
					b2pos = 0;
					b2inc = 1 - _speed;
				} else{
					b2inc = 1 - _speed;
					b2pos = 10 + ((-b2inc) * (1/lfoposinc));
					/* 10+ are not strictly necessary */
				}
				b2reset = true;
			}

			b1pos += b1inc;
			b2pos += b2inc;

			int position, position1;
			double error,int_pos;

			/*
			 * Interpolate value from buffer position 1
			 */
			error = modf(b1pos, &int_pos);

			position = (dbpos + MAXDELAY - ((int)int_pos % MAXDELAY)) % MAXDELAY;
			position1 = ((MAXDELAY-1) + position) % MAXDELAY;
			b1value = dbuffer[position] * (1 - error) + dbuffer[position1] * error;

			/*
			 * Interpolate value from buffer position 2
			 */
			error = modf(b2pos,&int_pos);

			position = (dbpos + MAXDELAY - ((int)int_pos % MAXDELAY)) % MAXDELAY;
			position1 = ((MAXDELAY-1) + position) % MAXDELAY;
			b2value = dbuffer[position]*(1-error) + dbuffer[position1]*error;

			/*
			 * Calculate output signal from these two buffers
			 */

			lfo = (sin(pi2 * lfopos) + 1) / 2;

			/*             position    sin   lfo variable
			 *------------------------------------------------------------------
			 * lfo value:    0.25       1         1        => buffer 2 is used
			 *               0.75      -1         0        => buffer 1 is used
			 */

			*outvalue++ = b1value * (1.0 - lfo) + b2value * lfo;

			/*
			 * increment delay buffer position
			 */
			dbpos++;
			if (dbpos == MAXDELAY)
				dbpos = 0;
		}
	}
};

// REGISTER_IMPLEMENTATION(Synth_PITCH_SHIFT_bugfixed_impl);
