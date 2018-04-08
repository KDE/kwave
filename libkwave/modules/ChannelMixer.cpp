/*************************************************************************
       ChannelMixer.cpp  -  matrix based mixer for multiple channels
                             -------------------
    begin                : Sun Oct 10 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#include <limits>
#include <new>

#include <QtGlobal>
#include <QByteArray>
#include <QMetaObject>
#include <QMutexLocker>
#include <QObject>
#include <QVarLengthArray>

#include "libkwave/MixerMatrix.h"
#include "libkwave/Sample.h"
#include "libkwave/Utils.h"
#include "libkwave/modules/ChannelMixer.h"
#include "libkwave/modules/Indexer.h"
#include "libkwave/modules/StreamObject.h"

//***************************************************************************
Kwave::ChannelMixer::ChannelMixer(unsigned int inputs, unsigned int outputs)
    :Kwave::SampleSource(),
     m_matrix(Q_NULLPTR),
     m_inputs(inputs),
     m_outputs(outputs),
     m_indexer(),
     m_input_queue(),
     m_output_buffer(),
     m_lock()
{
}

//***************************************************************************
bool Kwave::ChannelMixer::init()
{
    if (!m_inputs || !m_outputs) return false;

    // create queues for the input data
    m_input_queue.resize(m_inputs);
    Q_ASSERT(m_input_queue.count() == Kwave::toInt(m_inputs));
    if (m_input_queue.count() != Kwave::toInt(m_inputs)) return false;

    // create the buffers for the output data
    for (unsigned int index = 0; index < m_outputs; ++index) {
	// create a buffer for the input
	Kwave::SampleBuffer *out_buffer =
	    new(std::nothrow) Kwave::SampleBuffer();
	Q_ASSERT(out_buffer);
	if (!out_buffer) return false;
	m_output_buffer.append(out_buffer);
    }

    // create indexing proxies and connect their output to this mixer
    for (unsigned int index = 0; index < m_inputs; ++index) {
	Kwave::StreamObject *indexer =
	    new(std::nothrow) Kwave::Indexer(index);
	Q_ASSERT(indexer);
	if (!indexer) return false;

	m_indexer.append(indexer);
	bool ok = Kwave::connect(
	    *indexer, SIGNAL(output(uint,Kwave::SampleArray)),
	    *this,    SLOT(idxInput(uint,Kwave::SampleArray)));
	Q_ASSERT(ok);
	if (!ok) return false;
    }

    // create the mixer matrix
    // create a translation matrix for mixing up/down to the desired
    // number of output channels
    m_matrix = new(std::nothrow) Kwave::MixerMatrix(m_inputs, m_outputs);
    Q_ASSERT(m_matrix);
    if (!m_matrix) return false;

    // everything succeeded
    return true;
}

//***************************************************************************
Kwave::ChannelMixer::~ChannelMixer()
{
    QMutexLocker _lock(&m_lock);

    while (!m_indexer.isEmpty()) {
	Kwave::StreamObject *indexer = m_indexer[0];
	if (indexer) delete indexer;
	m_indexer.remove(0);
    }

    m_input_queue.clear();

    while (!m_output_buffer.isEmpty()) {
	Kwave::SampleBuffer *buffer = m_output_buffer[0];
	if (buffer) delete buffer;
	m_output_buffer.remove(0);
    }
}

//***************************************************************************
static inline QByteArray _sig(const char *sig)
{
    return QMetaObject::normalizedSignature(sig);
}

//***************************************************************************
unsigned int Kwave::ChannelMixer::tracksOfPort(const char *port) const
{
    unsigned int retval = 0;
    QMutexLocker _lock(const_cast<QMutex *>(&m_lock));

    if (_sig(port) == _sig(SLOT(input(Kwave::SampleArray)))) {
	// input ports
	retval = m_inputs; // init is done
    } else if (_sig(port) == _sig(SIGNAL(output(Kwave::SampleArray)))) {
	// output ports
	retval = m_outputs;
    } else if (_sig(port) ==
               _sig(SLOT(idxInput(uint,Kwave::SampleArray)))) {
	retval = 1;
    } else {
	qFatal("unknown port");
    }

    return retval;
}

//***************************************************************************
Kwave::StreamObject *Kwave::ChannelMixer::port(const char *port,
                                               unsigned int track)
{
    Kwave::StreamObject *retval = Q_NULLPTR;
    QMutexLocker _lock(&m_lock);

    if (_sig(port) == _sig(SLOT(input(Kwave::SampleArray)))) {
	// input proxy
	Q_ASSERT(Kwave::toInt(track) < m_indexer.count());
        if (Kwave::toInt(track) >= m_indexer.count()) return Q_NULLPTR;
	retval = m_indexer.at(track);
    } else if (_sig(port) == _sig(SIGNAL(output(Kwave::SampleArray)))) {
	// output proxy
	Q_ASSERT(Kwave::toInt(track) < m_output_buffer.count());
        if (Kwave::toInt(track) >= m_output_buffer.count()) return Q_NULLPTR;
	retval = m_output_buffer[track];
    } else if (_sig(port) ==
	       _sig(SLOT(idxInput(uint,Kwave::SampleArray)))) {
	retval = this;
    } else {
	qFatal("unknown port");
    }

    return retval;
}

//***************************************************************************
void Kwave::ChannelMixer::idxInput(unsigned int index, Kwave::SampleArray data)
{
    QMutexLocker _lock(&m_lock);

    // put the data into the corresponding input queue
    Q_ASSERT(index < m_inputs);
    Q_ASSERT(Kwave::toInt(index) < m_input_queue.count());
    if (Kwave::toInt(index) < m_input_queue.count())
	m_input_queue[index].enqueue(data);

    // check: if there is one empty queue we are not yet ready for mixing
    bool ready = true;
    foreach (const QQueue<Kwave::SampleArray> &queue, m_input_queue) {
	if (queue.isEmpty()) {
	    ready = false;
	    break;
	}
    }

    // mix if we are ready
    if (ready && m_matrix) mix();
}

//***************************************************************************
void Kwave::ChannelMixer::mix()
{
    Q_ASSERT(m_matrix);

    // all inputs should contain a buffer, dequeue them into a vector
    // and form an array of pointers to the raw data, for speeding up
    QVector<Kwave::SampleArray> v_input(m_inputs);
    QVarLengthArray<const sample_t *> input(m_inputs);
    unsigned int min_len = std::numeric_limits<unsigned int>::max();
    for (unsigned int track = 0; track < m_inputs; track++) {
	// dequeue the buffer with input data
	QQueue<Kwave::SampleArray> &queue = m_input_queue[track];
	Q_ASSERT(!queue.isEmpty());
	Kwave::SampleArray buffer = queue.dequeue();
	v_input[track] = buffer;

	// get a pointer for quick access
	const sample_t *raw_data = v_input[track].constData();
	input[track] = raw_data;

	// detect minimum input length
	min_len = qMin(min_len, buffer.size());
    }
    Q_ASSERT(min_len);
    if (!min_len) return; // zero length buffer in the queue, data underrun?

    // make sure all output buffers are large enough
    // and build an array of pointers to the raw data, for speeding up
    QVarLengthArray<sample_t *> output(m_outputs);
    for (unsigned int track = 0; track < m_outputs; track++) {
	Kwave::SampleBuffer *buffer = m_output_buffer[track];
	Q_ASSERT(buffer);
	if (!buffer) return;
	bool ok = true;
	if (buffer->constData().size() < min_len)
	    ok &= buffer->data().resize(min_len);
	if (!ok) {
	    qWarning("ChannelMixer: failed to increase buffer size to %u",
		     min_len);
	    return; // OOM ?
	}
	output[track] = buffer->data().data();
    }

    // mix all channels together, using the mixer matrix
    for (unsigned int y = 0; y < m_outputs; y++) {
	sample_t *out = output[y];
	for (unsigned int pos = 0; pos < min_len; pos++) {
	    double sum = 0.0;
	    for (unsigned int x = 0; x < m_inputs; x++) {
		const double f = (*m_matrix)[x][y];
		const double i = static_cast<double>(input[x][pos]);
		sum += (f * i);
	    }
	    out[pos] = static_cast<sample_t>(sum);
	}

	// emit the output
	Kwave::SampleBuffer *out_buf = m_output_buffer[y];
	if (Q_UNLIKELY(out_buf->constData().size() > min_len)) {
	    bool ok = out_buf->data().resize(min_len);
	    Q_ASSERT(ok);
	    Q_UNUSED(ok);
	}
	out_buf->finished();
    }

}

//***************************************************************************
//***************************************************************************
