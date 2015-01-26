/***************************************************************************
      RateConverter.cpp  -  single channel sample rate converter
                             -------------------
    begin                : Sat Jul 11 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#include <math.h>

#include "libkwave/Utils.h"
#include "libkwave/modules/RateConverter.h"

//***************************************************************************
Kwave::RateConverter::RateConverter()
    :Kwave::SampleSource(), m_ratio(1.0), m_converter(0),
     m_converter_in(), m_converter_out()
{
    int error = 0;
    m_converter = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
    Q_ASSERT(m_converter);
    if (!m_converter) qWarning("creating converter failed: '%s",
	src_strerror(error));
}

//***************************************************************************
Kwave::RateConverter::~RateConverter()
{
    if (m_converter) src_delete(m_converter);
}

//***************************************************************************
void Kwave::RateConverter::goOn()
{
}

//***************************************************************************
void Kwave::RateConverter::input(Kwave::SampleArray data)
{
    // shortcut for ratio == 1:1
    if ((m_ratio == 1.0) || data.isEmpty()) {
	emit output(data);
	return;
    }

    // normal processing
    Kwave::SampleArray samples_out;
    const unsigned int in_len = data.size();

    // convert the input buffer into an array of floats
    m_converter_in.resize(in_len);
    float          *f_in = m_converter_in.data();
    const sample_t *s_in = data.constData();
    Q_ASSERT(f_in);
    Q_ASSERT(s_in);

    // work blockwise to allow loop unrolling
    unsigned int remaining = in_len;
    const unsigned int block_size = 16;
    while (remaining >= block_size) {
	for (unsigned int i = 0; i < block_size; i++)
	    f_in[i] = sample2float(s_in[i]);
	f_in      += block_size;
	s_in      += block_size;
	remaining -= block_size;
    }
    for (; remaining; remaining--)
	(*f_in++) = sample2float(*(s_in++));

    // prepare the output buffer (estimated size, rounded up)
    // worst case would be factor 2, which means that there was a 100%
    // leftover remaining from the previous pass
    // just for safety we limit the extra output space to some
    // (hopefully) reasonable range between 4096 and 16384
    const unsigned int out_len = Kwave::toUint(
	ceil(static_cast<double>(in_len) * m_ratio)
    );
    const unsigned int extra = qBound<unsigned int>(4096, out_len, 16384);
    m_converter_out.resize(out_len + extra);

    // set up the sample rate converter input
    SRC_DATA src;
    src.data_in           = m_converter_in.data();
    src.data_out          = m_converter_out.data();
    src.input_frames      = in_len;
    src.output_frames     = out_len + extra;
    src.input_frames_used = 0;
    src.output_frames_gen = 0;
    src.end_of_input      = (in_len == 0) ? 1 : 0;
    src.src_ratio         = m_ratio;

    // let the converter run...
    int error = src_process(m_converter, &src);
    if (error) qWarning("SRC error: '%s'", src_strerror(error));
    Q_ASSERT(!error);

    // convert the result back from floats to sample_t
    unsigned int gen = src.output_frames_gen;
    Kwave::SampleArray out(gen);
    const float *f_out = src.data_out;
    sample_t    *s_out = out.data();

    // work blockwise to allow loop unrolling
    remaining = gen;
    while (remaining >= block_size) {
	for (unsigned int i = 0; i < block_size; i++)
	    s_out[i] = float2sample(f_out[i]);
	s_out     += block_size;
	f_out     += block_size;
	remaining -= block_size;
    }
    for (; remaining; remaining--)
	*(s_out++) = float2sample(*(f_out++));

    emit output(out);
}

//***************************************************************************
void Kwave::RateConverter::setRatio(const QVariant ratio)
{
    m_ratio = QVariant(ratio).toDouble();
}

//***************************************************************************
#include "RateConverter.moc"
//***************************************************************************
//***************************************************************************
