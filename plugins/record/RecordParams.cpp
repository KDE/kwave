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
   :method(RECORD_ALSA),
    pre_record_enabled(false),     pre_record_time(20),
    record_time_limited(false),    record_time(5*60),
    start_time_enabled(false),     start_time(QDateTime::currentDateTime()),
    record_trigger_enabled(false), record_trigger(30),
    amplification_enabled(false),  amplification(+3),
    agc_enabled(false),            agc_decay(50),
    fade_in_enabled(false),        fade_in_time(5),
    fade_out_enabled(false),       fade_out_time(5),
    device_name("plug:dsnoop"),
    tracks(2),
    sample_rate(44100.0),
    compression(0),
    bits_per_sample(16),
    sample_format(SampleFormat::Unknown),
    buffer_count(32),
    buffer_size(13), /* (1 << 13) == 8192 bytes */
    display_level_meter(true),
    display_oscilloscope(false),
    display_fft(false),
    display_overview(false)
{
}

//***************************************************************************
RecordParams::~RecordParams()
{
}

#define GET(value,func) \
	value = list[index++].func(&ok); \
	Q_ASSERT(ok); \
	if (!ok) return -EINVAL;

//***************************************************************************
int RecordParams::fromList(const QStringList &list)
{
    bool ok;
    int index = 0;

    // check number of elements:
    // oldest version has only 26 elements
    // <= v0.8.3 additionally has a recording method (27 entries)
    // >= v0.8.4 additionally has a recording time (29 entries)
    if ((list.size() != 26) &&
        (list.size() != 27) &&
        (list.size() != 29)) return -EINVAL;

    // pre-record
    GET(pre_record_enabled, toUInt);
    GET(pre_record_time, toUInt);

    // record time
    GET(record_time_limited, toUInt);
    GET(record_time, toUInt);

    // record start time
    if (list.size() >= 29) {
	GET(start_time_enabled, toUInt);
	start_time = QDateTime::fromString(list[index++], Qt::ISODate);
    } else {
	start_time_enabled = false;
	start_time = QDateTime::currentDateTime();
    }
    // auto-adjust to same hour as last time but not in past
    if (start_time.date() < QDate::currentDate())
	start_time.setDate(QDate::currentDate());
    if (start_time < QDateTime::currentDateTime())
	start_time = start_time.addDays(1);
    // set seconds to zero
    QTime t = start_time.time();
    t.setHMS(t.hour(), t.minute(), 0, 0);
    start_time.setTime(t);

    // record trigger
    GET(record_trigger_enabled, toUInt);
    GET(record_trigger, toUInt);

    // amplification
    GET(amplification_enabled, toUInt);
    GET(amplification, toInt);

    // AGC
    GET(agc_enabled, toUInt);
    GET(agc_decay, toUInt);

    // fade in
    GET(fade_in_enabled, toUInt);
    GET(fade_in_time, toUInt);

    // fade out
    GET(fade_out_enabled, toUInt);
    GET(fade_out_time, toUInt);

    // device name
    device_name = list[index++];

    // tracks, sample rate, compression, sample format, bits per sample
    GET(tracks, toUInt);
    GET(sample_rate, toDouble);
    GET(compression, toUInt);
    GET(bits_per_sample, toUInt);

    int sf;
    GET(sf, toInt);
    sample_format.fromInt(sf);

    // buffer count and power of buffer size
    GET(buffer_count, toUInt);
    GET(buffer_size, toUInt);

    // various displays: level meter, oscilloscope, FFT, Overview
    GET(display_level_meter, toUInt);
    GET(display_oscilloscope, toUInt);
    GET(display_fft, toUInt);
    GET(display_overview, toUInt);

    // if we have >= 27 entries: new version, we have a recording method
    if (list.size() >= 27) {
	unsigned int method_index;
	GET(method_index, toUInt);
	method = (method_index < RECORD_INVALID) ?
	         static_cast<record_method_t>(method_index) : RECORD_INVALID;
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

    // start time
    PUT(start_time_enabled);
    list += start_time.toString(Qt::ISODate);

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
    PUT(sample_format.toInt());

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
