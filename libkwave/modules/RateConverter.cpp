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

#include "RateConverter.h"

//***************************************************************************
Kwave::RateConverter::RateConverter()
    :Kwave::SampleSource(), m_ratio(1.0), m_converter(0),
     m_converter_in(), m_converter_out()
{
    int error = 0;
    m_converter = src_new(SRC_SINC_BEST_QUALITY, 1, &error);
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
    if (m_ratio == 1.0) {
	emit output(data);
	return;
    }

    // normal processing
    Kwave::SampleArray samples_out;
    const unsigned int in_len = data.size();

    // convert the input buffer into an array of floats
    m_converter_in.resize(in_len);
    float          *f_in = m_converter_in.data();
    const sample_t *s_in = data.data();
    for (unsigned int i = 0; i < in_len; i++)
	(*f_in++) = sample2float(*(s_in++));

    // prepare the output buffer (estimated size, rounded up)
    const unsigned int out_len =
	ceil(static_cast<double>(in_len) * m_ratio) + 1;
    m_converter_out.resize(out_len);

    // set up the sample rate converter input
    SRC_DATA src;
    src.data_in           = m_converter_in.data();
    src.data_out          = m_converter_out.data();
    src.input_frames      = in_len;
    src.output_frames     = out_len;
    src.input_frames_used = 0;
    src.output_frames_gen = 0;
    src.end_of_input      = (data.isEmpty() ? 1 : 0);
    src.src_ratio         = m_ratio;

    // let the converter run...
    int error = src_process(m_converter, &src);
    Q_ASSERT(!error);
    if (error) qWarning("SRC error: '%s'", src_strerror(error));

    // convert the result back from floats to sample_t
    unsigned int gen = src.output_frames_gen;
    Kwave::SampleArray out(gen);
    const float *f_out = src.data_out;
    sample_t    *s_out = out.data();
    for (unsigned int i = 0; i < gen; i++)
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
