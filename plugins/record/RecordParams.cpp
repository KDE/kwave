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
   :method(RECORD_OSS),
    pre_record_enabled(false),     pre_record_time(20),
    record_time_limited(false),    record_time(5*60),
    record_trigger_enabled(false), record_trigger(30),
    amplification_enabled(false),  amplification(+3),
    agc_enabled(false),            agc_decay(50),
    fade_in_enabled(false),        fade_in_time(5),
    fade_out_enabled(false),       fade_out_time(5),
    device_name("/dev/dsp"),
    tracks(2),
    sample_rate(44100.0),
    compression(0),
    bits_per_sample(16),
    sample_format(SampleFormat::Unknown),
    buffer_count(32),
    buffer_size(13), /* (1 << 13) == 8192 bytes */
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

    if ((list.size() != 26) && (list.size() != 27)) return -EINVAL;

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

    // device name
    device_name = list[14];

    // tracks, sample rate, compression, sample format, bits per sample
    GET(15, tracks, toUInt);
    GET(16, sample_rate, toDouble);
    GET(17, compression, toUInt);
    GET(18, bits_per_sample, toUInt);

    int sf;
    GET(19, sf, toInt);
    sample_format = static_cast<SampleFormat::sample_format_t>(sf);

    // buffer count and power of buffer size
    GET(20, buffer_count, toUInt);
    GET(21, buffer_size, toUInt);

    // various displays: level meter, oscilloscope, FFT, Overview
    GET(22, display_level_meter, toUInt);
    GET(23, display_oscilloscope, toUInt);
    GET(24, display_fft, toUInt);
    GET(25, display_overview, toUInt);

    // if we have 27 entries: new version, we have a recording method
    if (list.size() == 27) {
	unsigned int method_index;
	GET(26, method_index, toUInt);
	method = (method_index < RECORD_INVALID) ?
	         (record_method_t)(method_index) : RECORD_INVALID;
    }

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

    // device name
    list += device_name;

    // tracks, sample rate, compression, sample format, bits per sample
    PUT(tracks);
    PUT(sample_rate);
    PUT(compression);
    PUT(bits_per_sample);
    PUT(static_cast<int>(sample_format));

    // buffer count and power of buffer size
    PUT(buffer_count);
    PUT(buffer_size);

    // various displays: level meter, oscilloscope, FFT, Overview
    PUT(display_level_meter);
    PUT(display_oscilloscope);
    PUT(display_fft);
    PUT(display_overview);

    // record method
    PUT(static_cast<unsigned int>(method));

    return list;
}

//***************************************************************************
//***************************************************************************
