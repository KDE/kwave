/*************************************************************************
       RecordParams.cpp  -  holds parameters of the record plugin
                             -------------------
    begin                : Thu Sep 04 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include "config.h"
#include <errno.h>
#include "RecordParams.h"

//***************************************************************************
RecordParams::RecordParams()
   :pre_record_enabled(false),     pre_record_time(20),
    record_time_limited(false),    record_time(5*60),
    record_trigger_enabled(false), record_trigger(30),
    amplification_enabled(false),  amplification(+3),
    agc_enabled(false),            agc_decay(50),
    fade_in_enabled(false),        fade_in_time(5),
    fade_out_enabled(false),       fade_out_time(5),
    sample_rate(44100.0), bits_per_sample(16), tracks(2),
    device_name("[aRts sound daemon]"),
    buffer_size(10), /* (1 << 10) == 1024 bytes */
    display_level_meter(false),
    display_oscilloscope(false),
    display_fft(false),
    display_overview(false)
{
}

//***************************************************************************
RecordParams::~RecordParams()
{
}

#define GET(index,value,func) \
	value = list[index].func(&ok); \
	Q_ASSERT(ok); \
	if (!ok) return -EINVAL;

//***************************************************************************
int RecordParams::fromList(const QStringList &list)
{
    bool ok;

    if (list.size() != 23) return -EINVAL;

    // pre-record
    GET( 0, pre_record_enabled, toUInt);
    GET( 1, pre_record_time, toUInt);

    // record time
    GET( 2, record_time_limited, toUInt);
    GET( 3, record_time, toUInt);

    // record trigger
    GET( 4, record_trigger_enabled, toUInt);
    GET( 5, record_trigger, toUInt);

    // amplification
    GET( 6, amplification_enabled, toUInt);
    GET( 7, amplification, toInt);

    // AGC
    GET( 8, agc_enabled, toUInt);
    GET( 9, agc_decay, toUInt);

    // fade in
    GET(10, fade_in_enabled, toUInt);
    GET(11, fade_in_time, toUInt);

    // fade out
    GET(12, fade_out_enabled, toUInt);
    GET(13, fade_out_time, toUInt);

    // sample rate, bits per sample, track
    GET(14, sample_rate, toDouble);
    GET(15, bits_per_sample, toUInt);
    GET(16, tracks, toUInt);

    // device name
    device_name = list[17];

    // power of buffer size
    GET(18, buffer_size, toUInt);

    // various displays: level meter, oscilloscope, FFT, Overview
    GET(19, display_level_meter, toUInt);
    GET(20, display_oscilloscope, toUInt);
    GET(21, display_fft, toUInt);
    GET(22, display_overview, toUInt);

    return 0;
}

#define PUT(value) list += param.setNum(value)

//***************************************************************************
QStringList RecordParams::toList() const
{
    QStringList list;
    QString param;

    // pre-record
    PUT(pre_record_enabled);
    PUT(pre_record_time);

    // record time
    PUT(record_time_limited);
    PUT(record_time);

    // record trigger
    PUT(record_trigger_enabled);
    PUT(record_trigger);

    // amplification
    PUT(amplification_enabled);
    PUT(amplification);

    // AGC
    PUT(agc_enabled);
    PUT(agc_decay);

    // fade in
    PUT(fade_in_enabled);
    PUT(fade_in_time);

    // fade out
    PUT(fade_out_enabled);
    PUT(fade_out_time);

    // sample rate, bits per sample, track
    PUT(sample_rate);
    PUT(bits_per_sample);
    PUT(tracks);

    // device name
    list += device_name;

    // power of buffer size
    PUT(buffer_size);

    // various displays: level meter, oscilloscope, FFT, Overview
    PUT(display_level_meter);
    PUT(display_oscilloscope);
    PUT(display_fft);
    PUT(display_overview);

    return list;
}

//***************************************************************************
//***************************************************************************
